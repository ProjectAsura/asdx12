//-----------------------------------------------------------------------------
// File : asdxColorCorrectionEffect.cpp
// Desc : Color Correction Effect.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes.
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include <gfx/asdxColorCorrectionEffect.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxPresetState.h>


namespace {

//-----------------------------------------------------------------------------
// Shaders
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/asdxColorFilterPS.inc"
#include "../res/shaders/Compiled/asdxColorFilterCS.inc"


///////////////////////////////////////////////////////////////////////////////
// Param1 structure
///////////////////////////////////////////////////////////////////////////////
struct Param1
{
    uint32_t        Width;
    uint32_t        Height;
    asdx::Vector2   InvSize;
};

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ColorCorrectionEffect class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ColorCorrectionEffect::ColorCorrectionEffect()
{ ResetValues(); }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ColorCorrectionEffect::~ColorCorrectionEffect()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool ColorCorrectionEffect::Init(DXGI_FORMAT rtvFormat)
{
    auto pDevice = asdx::GetD3D12Device();

    // ルートシグニチャの初期化.
    {
        D3D12_DESCRIPTOR_RANGE range[2] = {};
        range[0].RangeType                          = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range[0].NumDescriptors                     = 1;
        range[0].BaseShaderRegister                 = 0;
        range[0].RegisterSpace                      = 0;
        range[0].OffsetInDescriptorsFromTableStart  = 0;

        range[1].RangeType                          = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        range[1].NumDescriptors                     = 1;
        range[1].BaseShaderRegister                 = 0;
        range[1].RegisterSpace                      = 0;
        range[1].OffsetInDescriptorsFromTableStart  = 0;

        D3D12_ROOT_PARAMETER param[4] = {};
        param[0].ParameterType              = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[0].Constants.Num32BitValues   = 16;
        param[0].Constants.ShaderRegister   = 0;
        param[0].Constants.RegisterSpace    = 0;
        param[0].ShaderVisibility           = D3D12_SHADER_VISIBILITY_ALL;

        param[1].ParameterType              = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[1].Constants.Num32BitValues   = 4;
        param[1].Constants.ShaderRegister   = 1;
        param[1].Constants.RegisterSpace    = 0;
        param[1].ShaderVisibility           = D3D12_SHADER_VISIBILITY_ALL;

        param[2].ParameterType                          = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[2].DescriptorTable.NumDescriptorRanges    = 1;
        param[2].DescriptorTable.pDescriptorRanges      = &range[0];
        param[2].ShaderVisibility                       = D3D12_SHADER_VISIBILITY_ALL;

        param[3].ParameterType                          = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[3].DescriptorTable.NumDescriptorRanges    = 1;
        param[3].DescriptorTable.pDescriptorRanges      = &range[1];
        param[3].ShaderVisibility                       = D3D12_SHADER_VISIBILITY_ALL;

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.NumParameters      = _countof(param);
        desc.pParameters        = param;
        desc.NumStaticSamplers  = _countof(asdx::Preset::StaticSamplers);
        desc.pStaticSamplers    = asdx::Preset::StaticSamplers;
        desc.Flags              = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        desc.Flags             |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
        desc.Flags             |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
        desc.Flags             |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        RefPtr<ID3DBlob> blob;
        RefPtr<ID3DBlob> errorBlob;
        auto hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, blob.GetAddress(), errorBlob.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : D3D12SerializeRootSignature() Failed. errcode = 0x%x", hr);
            if (errorBlob.GetPtr() != nullptr)
            {
                ELOG("Error : Msg = %s", reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
            }
            return false;
        }

        hr = pDevice->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(m_RootSignature.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateRootSignature() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // グラフィックスパイプラインステートの初期化.
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature                 = m_RootSignature.GetPtr();
        desc.VS                             = asdx::Preset::FullScreenVS;
        desc.PS                             = { asdxColorFilterPS, sizeof(asdxColorFilterPS) };
        desc.BlendState                     = asdx::Preset::Opaque;
        desc.SampleMask                     = D3D12_DEFAULT_SAMPLE_MASK;
        desc.RasterizerState                = asdx::Preset::CullNone;
        desc.DepthStencilState              = asdx::Preset::DepthNone;
        desc.InputLayout.NumElements        = _countof(asdx::Preset::QuadElements);
        desc.InputLayout.pInputElementDescs = asdx::Preset::QuadElements;
        desc.PrimitiveTopologyType          = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets               = 1;
        desc.RTVFormats[0]                  = rtvFormat;
        desc.DSVFormat                      = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count               = 1;
        desc.SampleDesc.Quality             = 0;

        auto hr = pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_GraphicsPSO.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // コンピュートパイプラインステートの初期化.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = m_RootSignature.GetPtr();
        desc.CS = { asdxColorFilterCS, sizeof(asdxColorFilterCS) };

        auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_ComputePSO.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void ColorCorrectionEffect::Term()
{
    m_GraphicsPSO  .Reset();
    m_ComputePSO   .Reset();
    m_RootSignature.Reset();
}

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void ColorCorrectionEffect::Draw(ID3D12GraphicsCommandList* pCmd, D3D12_GPU_DESCRIPTOR_HANDLE handleSRV)
{
    if (pCmd == nullptr || handleSRV.ptr == 0)
        return;

    auto matrix = CalcColorMatrix();

    pCmd->SetGraphicsRootSignature(m_RootSignature.GetPtr());
    pCmd->SetPipelineState(m_GraphicsPSO.GetPtr());
    pCmd->SetGraphicsRoot32BitConstants(0, 16, &matrix._11, 0);
    pCmd->SetGraphicsRootDescriptorTable(2, handleSRV);
    DrawQuad(pCmd);
}

//-----------------------------------------------------------------------------
//      コンピュートシェーダを起動します.
//-----------------------------------------------------------------------------
void ColorCorrectionEffect::Dispatch
(
    ID3D12GraphicsCommandList*  pCmd,
    uint32_t                    width,
    uint32_t                    height,
    D3D12_GPU_DESCRIPTOR_HANDLE handleUAV,
    D3D12_GPU_DESCRIPTOR_HANDLE handleSRV
)
{
    if (pCmd == nullptr || width == 0 || height == 0 || handleUAV.ptr == 0 || handleSRV.ptr == 0)
        return;

    auto matrix = CalcColorMatrix();

    Param1 param = {};
    param.Width         = width;
    param.Height        = height;
    param.InvSize.x     = 1.0f / float(width);
    param.InvSize.y     = 1.0f / float(height);

    pCmd->SetComputeRootSignature(m_RootSignature.GetPtr());
    pCmd->SetPipelineState(m_ComputePSO.GetPtr());
    pCmd->SetComputeRoot32BitConstants(0, 16, &matrix._11, 0);
    pCmd->SetComputeRoot32BitConstants(1, 4, &param, 0);
    pCmd->SetComputeRootDescriptorTable(2, handleSRV);
    pCmd->SetComputeRootDescriptorTable(3, handleUAV);

    auto threadX = (width  + 7) / 8;
    auto threadY = (height + 7) / 8;
    pCmd->Dispatch(threadX, threadY, 1);
}

//-----------------------------------------------------------------------------
//      明度調整値を設定します.
//-----------------------------------------------------------------------------
void ColorCorrectionEffect::SetBrightness(float value)
{ m_Brightness = value; }

//-----------------------------------------------------------------------------
//      彩度調整値を設定します.
//-----------------------------------------------------------------------------
void ColorCorrectionEffect::SetSaturation(float value)
{
    m_Saturation.x = value;
    m_Saturation.y = value;
    m_Saturation.z = value;
}

//-----------------------------------------------------------------------------
//      彩度調整値を設定します.
//-----------------------------------------------------------------------------
void ColorCorrectionEffect::SetSaturation(const Vector3& value)
{ m_Saturation = value; }

//-----------------------------------------------------------------------------
//      コントラスト調整値を設定します.
//-----------------------------------------------------------------------------
void ColorCorrectionEffect::SetContrast(float value)
{ m_Contrast = value; }

//-----------------------------------------------------------------------------
//      色相調整値を設定します.
//-----------------------------------------------------------------------------
void ColorCorrectionEffect::SetHueDegree(float value)
{ m_HueDegree = value; }

//-----------------------------------------------------------------------------
//      セピアトーン調整値を設定します.
//-----------------------------------------------------------------------------
void ColorCorrectionEffect::SetSepiaTone(float value)
{ m_SepiaTone = asdx::Saturate(value); }

//-----------------------------------------------------------------------------
//      グレースケール調整値を設定します.
//-----------------------------------------------------------------------------
void ColorCorrectionEffect::SetGrayScale(float value)
{ m_GrayScale = asdx::Saturate(value); }

//-----------------------------------------------------------------------------
//      色反転フラグを設定します.
//-----------------------------------------------------------------------------
void ColorCorrectionEffect::SetReverse(bool value)
{ m_Reverse = value; }

//-----------------------------------------------------------------------------
//      明度調整値を取得します.
//-----------------------------------------------------------------------------
float ColorCorrectionEffect::GetBrightness() const
{ return m_Brightness; }

//-----------------------------------------------------------------------------
//      彩度調整値を取得します.
//-----------------------------------------------------------------------------
const Vector3& ColorCorrectionEffect::GetSaturation() const
{ return m_Saturation; }

//-----------------------------------------------------------------------------
//      コントラスト調整値を取得します.
//-----------------------------------------------------------------------------
float ColorCorrectionEffect::GetContrast() const
{ return m_Contrast; }

//-----------------------------------------------------------------------------
//      色相調整値を取得します.
//-----------------------------------------------------------------------------
float ColorCorrectionEffect::GetHueDegree() const
{ return m_HueDegree; }

//-----------------------------------------------------------------------------
//      セピアトーン調整値を取得します.
//-----------------------------------------------------------------------------
float ColorCorrectionEffect::GetSepiaTone() const
{ return m_SepiaTone; }

//-----------------------------------------------------------------------------
//      グレースケール調整値を取得します.
//-----------------------------------------------------------------------------
float ColorCorrectionEffect::GetGrayScale() const
{ return m_GrayScale; }

//-----------------------------------------------------------------------------
//      色反転フラグを取得します.
//-----------------------------------------------------------------------------
bool ColorCorrectionEffect::IsReverse() const
{ return m_Reverse; }

//-----------------------------------------------------------------------------
//      調整値をリセットします.
//-----------------------------------------------------------------------------
void ColorCorrectionEffect::ResetValues()
{
    m_Saturation = Vector3(1.0f, 1.0f, 1.0f);
    m_AddColor   = Vector3(0.0f, 0.0f, 0.0f);
    m_MulColor   = Vector3(1.0f, 1.0f, 1.0f);
    m_Brightness = 1.0f;
    m_Contrast   = 1.0f;
    m_HueDegree  = 0.0f;
    m_SepiaTone  = 0.0f;
    m_GrayScale  = 0.0f;
    m_Reverse    = false;
}

//-----------------------------------------------------------------------------
//      カラー行列を計算します.
//-----------------------------------------------------------------------------
Matrix ColorCorrectionEffect::CalcColorMatrix() const
{
    Matrix result = Matrix::CreateIdentity();

    auto mtxHue         = Matrix::CreateHueMatrix(m_HueDegree);
    auto mtxSaturation  = Matrix::CreateSaturationMatrix(m_Saturation.x, m_Saturation.y, m_Saturation.z);
    auto mtxBrightness  = Matrix::CreateBrightnessMatrix(m_Brightness);
    auto mtxContrast    = Matrix::CreateContrastMatrix(m_Contrast);
    auto mtxGrayScale   = Matrix::CreateGrayScaleMatrix(m_GrayScale);
    auto mtxSepiaTone   = Matrix::CreateSepiaMatrix(m_SepiaTone);
    auto mtxScale       = Matrix::CreateScale(m_MulColor);
    auto mtxAdd         = Matrix::CreateTranslation(m_AddColor);

    result = Matrix::Multiply(mtxHue,        result);
    result = Matrix::Multiply(mtxSaturation, result);
    result = Matrix::Multiply(mtxBrightness, result);
    result = Matrix::Multiply(mtxContrast,   result);
    result = Matrix::Multiply(mtxGrayScale,  result);
    result = Matrix::Multiply(mtxSepiaTone,  result);

    if (m_Reverse)
    {
        auto mtxReverse = Matrix::CreateReverseColorMatrix();
        result = Matrix::Multiply(mtxReverse, result);
    }

    result = Matrix::Multiply(mtxScale, result);
    result = Matrix::Multiply(mtxAdd,   result);

    return result;
}

} // namespace asdx

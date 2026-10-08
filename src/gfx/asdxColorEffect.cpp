//-----------------------------------------------------------------------------
// File : asdxColorEffect.cpp
// Desc : Color Effect.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes.
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include <gfx/asdxColorEffect.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxPresetState.h>
#include <gfx/asdxScopedMarker.h>


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
    uint16_t    DstW;   //!< 横幅.
    uint16_t    DstH;   //!< 縦幅.
};

//-----------------------------------------------------------------------------
//      カラー行列を計算します.
//-----------------------------------------------------------------------------
template<typename T>
asdx::Matrix4x4 CalcColorMatrix(const T& param)
{
    asdx::Matrix4x4 result = asdx::Matrix4x4::CreateIdentity();

    auto mtxHue         = asdx::Matrix4x4::CreateHueMatrix(param.HueDegree);
    auto mtxSaturation  = asdx::Matrix4x4::CreateSaturationMatrix(param.Saturation.x, param.Saturation.y, param.Saturation.z);
    auto mtxBrightness  = asdx::Matrix4x4::CreateBrightnessMatrix(param.Brightness);
    auto mtxContrast    = asdx::Matrix4x4::CreateContrastMatrix(param.Contrast);
    auto mtxGrayScale   = asdx::Matrix4x4::CreateGrayScaleMatrix(param.GrayScale);
    auto mtxSepiaTone   = asdx::Matrix4x4::CreateSepiaMatrix(param.SepiaTone);
    auto mtxWb          = asdx::Matrix4x4::CreateWhiteBalanceMatrix(param.WhiteBalance, 6504.0f);
    auto mtxScale       = asdx::Matrix4x4::CreateScale(param.MulColor);
    auto mtxAdd         = asdx::Matrix4x4::CreateTranslation(param.AddColor);

    result = asdx::Matrix4x4::Multiply(mtxHue,        result);
    result = asdx::Matrix4x4::Multiply(mtxSaturation, result);
    result = asdx::Matrix4x4::Multiply(mtxBrightness, result);
    result = asdx::Matrix4x4::Multiply(mtxContrast,   result);
    result = asdx::Matrix4x4::Multiply(mtxGrayScale,  result);
    result = asdx::Matrix4x4::Multiply(mtxSepiaTone,  result);
    result = asdx::Matrix4x4::Multiply(mtxScale,      result);
    result = asdx::Matrix4x4::Multiply(mtxAdd,        result);

    if (param.BlackAndWhite)
    {
        auto mtxBlackAndWhite = asdx::Matrix4x4::CreateBlackAndWhiteMatrix();
        result = asdx::Matrix4x4::Multiply(mtxBlackAndWhite, result);
    }

    if (param.Reverse)
    {
        auto mtxReverse = asdx::Matrix4x4::CreateReverseColorMatrix();
        result = asdx::Matrix4x4::Multiply(mtxReverse, result);
    }

    result = asdx::Matrix4x4::Multiply(mtxWb, result);

    return result;
}

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ColorEffectCS class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ColorEffectCS::ColorEffectCS()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ColorEffectCS::~ColorEffectCS()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool ColorEffectCS::Init()
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
        desc.NumStaticSamplers  = _countof(Preset::StaticSamplers);
        desc.pStaticSamplers    = Preset::StaticSamplers;
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

    // コンピュートパイプラインステートの初期化.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = m_RootSignature.GetPtr();
        desc.CS = { asdxColorFilterCS, sizeof(asdxColorFilterCS) };

        auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_PipelineState.GetAddress()));
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
void ColorEffectCS::Term()
{
    m_PipelineState.Reset();
    m_RootSignature.Reset();
}

//-----------------------------------------------------------------------------
//      コンピュートシェーダを起動します.
//-----------------------------------------------------------------------------
void ColorEffectCS::Dispatch(ID3D12GraphicsCommandList* pCmd, const Param& args)
{
    if (pCmd == nullptr 
     || args.UavWidth == 0 || args.UavHeight == 0 
     || args.HandleUAV.ptr == 0 || args.HandleSRV.ptr == 0)
        return;

    ASDX_SCOPED_MARKER(pCmd, ColorEffectCS);

    auto matrix = CalcColorMatrix(args);

    Param1 param = {};
    param.DstW = uint16_t(args.UavWidth);
    param.DstH = uint16_t(args.UavHeight);

    pCmd->SetComputeRootSignature(m_RootSignature.GetPtr());
    pCmd->SetPipelineState(m_PipelineState.GetPtr());
    pCmd->SetComputeRoot32BitConstants(0, 16, &matrix._11, 0);
    pCmd->SetComputeRoot32BitConstants(1, 1, &param, 0);
    pCmd->SetComputeRootDescriptorTable(2, args.HandleSRV);
    pCmd->SetComputeRootDescriptorTable(3, args.HandleUAV);

    auto threadX = (args.UavWidth  + 7u) / 8u;
    auto threadY = (args.UavHeight + 7u) / 8u;
    pCmd->Dispatch(threadX, threadY, 1);
}

///////////////////////////////////////////////////////////////////////////////
// ColorEffectPS class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ColorEffectPS::ColorEffectPS()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ColorEffectPS::~ColorEffectPS()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool ColorEffectPS::Init(DXGI_FORMAT rtvFormat)
{
    auto pDevice = asdx::GetD3D12Device();

    // ルートシグニチャの初期化.
    {
        D3D12_DESCRIPTOR_RANGE range[1] = {};
        range[0].RangeType                          = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range[0].NumDescriptors                     = 1;
        range[0].BaseShaderRegister                 = 0;
        range[0].RegisterSpace                      = 0;
        range[0].OffsetInDescriptorsFromTableStart  = 0;

        D3D12_ROOT_PARAMETER param[3] = {};
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

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.NumParameters      = _countof(param);
        desc.pParameters        = param;
        desc.NumStaticSamplers  = _countof(Preset::StaticSamplers);
        desc.pStaticSamplers    = Preset::StaticSamplers;
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
        desc.VS                             = Preset::FullScreenVS;
        desc.PS                             = { asdxColorFilterPS, sizeof(asdxColorFilterPS) };
        desc.BlendState                     = Preset::Opaque;
        desc.SampleMask                     = D3D12_DEFAULT_SAMPLE_MASK;
        desc.RasterizerState                = Preset::CullNone;
        desc.DepthStencilState              = Preset::DepthNone;
        desc.InputLayout.NumElements        = _countof(Preset::QuadElements);
        desc.InputLayout.pInputElementDescs = Preset::QuadElements;
        desc.PrimitiveTopologyType          = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets               = 1;
        desc.RTVFormats[0]                  = rtvFormat;
        desc.DSVFormat                      = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count               = 1;
        desc.SampleDesc.Quality             = 0;

        auto hr = pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_PipelineState.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void ColorEffectPS::Term()
{
    m_PipelineState.Reset();
    m_RootSignature.Reset();
}

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void ColorEffectPS::Draw(ID3D12GraphicsCommandList* pCmd, const Param& args)
{
    if (pCmd == nullptr || args.HandleSRV.ptr == 0)
        return;

    ASDX_SCOPED_MARKER(pCmd, ColorEffectPS);

    auto matrix = CalcColorMatrix(args);

    pCmd->SetGraphicsRootSignature(m_RootSignature.GetPtr());
    pCmd->SetPipelineState(m_PipelineState.GetPtr());
    pCmd->SetGraphicsRoot32BitConstants(0, 16, &matrix._11, 0);
    pCmd->SetGraphicsRootDescriptorTable(2, args.HandleSRV);
    DrawQuad(pCmd);
}

} // namespace asdx

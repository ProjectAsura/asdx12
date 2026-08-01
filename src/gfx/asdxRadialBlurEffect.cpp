//-----------------------------------------------------------------------------
// File : asdxRadialBlurEffect.cpp
// Desc : Radial Blur Effect.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxRadialBlurEffect.h>
#include <gfx/asdxPresetState.h>
#include <gfx/asdxDevice.h>
#include <fnd/asdxLogger.h>


namespace {

//-----------------------------------------------------------------------------
// Shaders
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/asdxRadialBlurPS.inc"
#include "../res/shaders/Compiled/asdxRadialBlurCS.inc"


///////////////////////////////////////////////////////////////////////////////
// Param0 structure
///////////////////////////////////////////////////////////////////////////////
struct Param0
{
    asdx::Vector2 Center;       //!< ブラー中心.
    float         Strength;     //!< ブラー強度.
    uint32_t      SampleCount;  //!< サンプル数.
};

///////////////////////////////////////////////////////////////////////////////
// Param1 structure
///////////////////////////////////////////////////////////////////////////////
struct Param1
{
    uint32_t    InputWidth;     //!< 入力サイズ横幅
    uint32_t    InputHeight;    //!< 入力サイズ縦幅.
    uint32_t    OutputWidth;    //!< 出力サイズ横幅.
    uint32_t    OutputHeight;   //!< 出力サイズ縦幅.
};

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// RadialBlurEffect class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
RadialBlurEffect::RadialBlurEffect()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
RadialBlurEffect::~RadialBlurEffect()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool RadialBlurEffect::Init(DXGI_FORMAT rtvFormat)
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
        param[0].Constants.Num32BitValues   = 4;
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

    // グラフィックスパイプラインステートの初期化.
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature                 = m_RootSignature.GetPtr();
        desc.VS                             = asdx::Preset::FullScreenVS;
        desc.PS                             = { asdxRadialBlurPS, sizeof(asdxRadialBlurPS) };
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
        desc.CS = { asdxRadialBlurCS, sizeof(asdxRadialBlurCS) };

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
void RadialBlurEffect::Term()
{
    m_GraphicsPSO  .Reset();
    m_ComputePSO   .Reset();
    m_RootSignature.Reset();
}

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void RadialBlurEffect::Draw
(
    ID3D12GraphicsCommandList*  pCmd,
    uint32_t                    inputW,
    uint32_t                    inputH,
    D3D12_GPU_DESCRIPTOR_HANDLE handleSRV)
{
    if (pCmd == nullptr || handleSRV.ptr == 0)
        return;

    assert(inputW != 0);
    assert(inputH != 0);

    Param0 param0 = {};
    param0.Center       = m_Center;
    param0.Strength     = m_Strength;
    param0.SampleCount  = m_SampleCount;

    Param1 param1 = {};
    param1.InputWidth   = inputW;
    param1.InputHeight  = inputH;

    pCmd->SetGraphicsRootSignature(m_RootSignature.GetPtr());
    pCmd->SetPipelineState(m_GraphicsPSO.GetPtr());
    pCmd->SetGraphicsRoot32BitConstants(0, 4, &param0, 0);
    pCmd->SetGraphicsRoot32BitConstants(1, 4, &param1, 0);
    pCmd->SetGraphicsRootDescriptorTable(2, handleSRV);
    DrawQuad(pCmd);
}

//-----------------------------------------------------------------------------
//      コンピュートシェーダを起動します.
//-----------------------------------------------------------------------------
void RadialBlurEffect::Dispatch
(
    ID3D12GraphicsCommandList*  pCmd,
    uint32_t                    outputW,
    uint32_t                    outputH,
    D3D12_GPU_DESCRIPTOR_HANDLE handleUAV,
    uint32_t                    inputW,
    uint32_t                    inputH,
    D3D12_GPU_DESCRIPTOR_HANDLE handleSRV
)
{
    if (pCmd == nullptr || handleSRV.ptr == 0 || handleUAV.ptr == 0)
        return;

    assert(inputW != 0);
    assert(inputH != 0);
    assert(outputW != 0);
    assert(outputH != 0);

    Param0 param0 = {};
    param0.Center       = m_Center;
    param0.Strength     = m_Strength;
    param0.SampleCount  = m_SampleCount;

    Param1 param1 = {};
    param1.InputWidth   = inputW;
    param1.InputHeight  = inputH;
    param1.OutputWidth  = outputW;
    param1.OutputHeight = outputH;

    pCmd->SetComputeRootSignature(m_RootSignature.GetPtr());
    pCmd->SetPipelineState(m_ComputePSO.GetPtr());
    pCmd->SetComputeRoot32BitConstants(0, 4, &param0, 0);
    pCmd->SetComputeRoot32BitConstants(1, 4, &param1, 0);
    pCmd->SetComputeRootDescriptorTable(2, handleSRV);
    pCmd->SetComputeRootDescriptorTable(3, handleUAV);

    uint32_t threadX = (outputW + 7) / 8;
    uint32_t threadY = (outputH + 7) / 8;
    pCmd->Dispatch(threadX, threadY, 1);
}

//-----------------------------------------------------------------------------
//      ブラーの強さを設定します.
//-----------------------------------------------------------------------------
void RadialBlurEffect::SetStrength(float value)
{ m_Strength = value; }

//-----------------------------------------------------------------------------
//      ブラーの強さを取得します.
//-----------------------------------------------------------------------------
float RadialBlurEffect::GetStrength() const
{ return m_Strength; }

//-----------------------------------------------------------------------------
//      ブラー中心を設定します.
//-----------------------------------------------------------------------------
void RadialBlurEffect::SetCenter(const Vector2& value)
{ m_Center = value; }

//-----------------------------------------------------------------------------
//      ブラー中心を取得します.
//-----------------------------------------------------------------------------
const Vector2& RadialBlurEffect::GetCenter() const
{ return m_Center; }

//-----------------------------------------------------------------------------
//      サンプル数を設定します.
//-----------------------------------------------------------------------------
void RadialBlurEffect::SetSampleCount(uint8_t value)
{ m_SampleCount = value; }

//-----------------------------------------------------------------------------
//      サンプル数を取得します.
//-----------------------------------------------------------------------------
uint8_t RadialBlurEffect::GetSampleCount() const
{ return m_SampleCount; }

} // namespace asdx

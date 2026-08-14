//-----------------------------------------------------------------------------
// File : asdxMosaicEffect.cpp
// Desc : Mosaic Effect.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <fnd/asdxLogger.h>
#include <fnd/asdxMath.h>
#include <gfx/asdxMosaicEffect.h>
#include <gfx/asdxPresetState.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxScopedMarker.h>


namespace {

//-----------------------------------------------------------------------------
// Shaders
//-----------------------------------------------------------------------------
#include "../../res/shaders/Compiled/asdxMosaicPS.inc"
#include "../../res/shaders/Compiled/asdxMosaicCS.inc"


///////////////////////////////////////////////////////////////////////////////
// ROOT_PARAM enum
///////////////////////////////////////////////////////////////////////////////
enum ROOT_PARAM
{
    ROOT_CBV0,
    ROOT_SRV0,
    ROOT_UAV0,
};

///////////////////////////////////////////////////////////////////////////////
// Param structure
///////////////////////////////////////////////////////////////////////////////
struct Param
{
    float       Scale;
    uint16_t    DstW;
    uint16_t    DstH;
};

} // namespace 


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// MosaicEffect class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
MosaicEffect::MosaicEffect()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
MosaicEffect::~MosaicEffect()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool MosaicEffect::Init(DXGI_FORMAT format)
{
    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

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

        D3D12_ROOT_PARAMETER param[3] = {};
        param[ROOT_CBV0].ParameterType              = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[ROOT_CBV0].Constants.Num32BitValues   = 4;
        param[ROOT_CBV0].Constants.ShaderRegister   = 0;
        param[ROOT_CBV0].Constants.RegisterSpace    = 0;
        param[ROOT_CBV0].ShaderVisibility           = D3D12_SHADER_VISIBILITY_ALL;

        param[ROOT_SRV0].ParameterType                          = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[ROOT_SRV0].DescriptorTable.NumDescriptorRanges    = 1;
        param[ROOT_SRV0].DescriptorTable.pDescriptorRanges      = &range[0];
        param[ROOT_SRV0].ShaderVisibility                       = D3D12_SHADER_VISIBILITY_ALL;

        param[ROOT_UAV0].ParameterType                          = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[ROOT_UAV0].DescriptorTable.NumDescriptorRanges    = 1;
        param[ROOT_UAV0].DescriptorTable.pDescriptorRanges      = &range[1];
        param[ROOT_UAV0].ShaderVisibility                       = D3D12_SHADER_VISIBILITY_ALL;

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
        desc.PS                             = { asdxMosaicPS, sizeof(asdxMosaicPS) };
        desc.BlendState                     = asdx::Preset::Opaque;
        desc.SampleMask                     = D3D12_DEFAULT_SAMPLE_MASK;
        desc.RasterizerState                = asdx::Preset::CullNone;
        desc.DepthStencilState              = asdx::Preset::DepthNone;
        desc.InputLayout.NumElements        = _countof(asdx::Preset::QuadElements);
        desc.InputLayout.pInputElementDescs = asdx::Preset::QuadElements;
        desc.PrimitiveTopologyType          = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets               = 1;
        desc.RTVFormats[0]                  = format;
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
        desc.CS = { asdxMosaicCS, sizeof(asdxMosaicCS) };

        auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_ComputePSO.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void MosaicEffect::Term()
{
    m_GraphicsPSO .Reset();
    m_ComputePSO   .Reset();
    m_RootSignature.Reset();
}

//-----------------------------------------------------------------------------
//      ピクセルシェーダを用いてエフェクトを適用します.
//-----------------------------------------------------------------------------
void MosaicEffect::Draw
(
    ID3D12GraphicsCommandList*  pCmd,
    uint32_t                    inputW,
    uint32_t                    inputH,
    D3D12_GPU_DESCRIPTOR_HANDLE handleSRV
)
{
    if (pCmd == nullptr || handleSRV.ptr == 0)
        return;

    ASDX_SCOPED_MARKER(pCmd, MosaicEffectPS);

    auto size  = Max(inputW, inputH);
    auto block = size * m_Scale;

    pCmd->SetGraphicsRootSignature(m_RootSignature.GetPtr());
    pCmd->SetPipelineState(m_GraphicsPSO.GetPtr());
    pCmd->SetGraphicsRoot32BitConstants(ROOT_CBV0, 1, &block, 0);
    pCmd->SetGraphicsRootDescriptorTable(ROOT_SRV0, handleSRV);
    DrawQuad(pCmd);
}

//-----------------------------------------------------------------------------
//      コンピュートシェーダを用いてエフェクトを適用します.
//-----------------------------------------------------------------------------
void MosaicEffect::Dispatch
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
    if (pCmd == nullptr || outputW == 0 || outputH == 0 || handleUAV.ptr == 0 || handleSRV.ptr == 0)
        return;

    ASDX_SCOPED_MARKER(pCmd, MosaicEffectCS);

    auto size  = Max(inputW, inputH);
    auto block = size * m_Scale;

    Param param = {};
    param.Scale = block;
    param.DstW  = outputW;
    param.DstH  = outputH;

    auto threadX = (outputW + 7u) / 8u;
    auto threadY = (outputH + 7u) / 8u;

    pCmd->SetComputeRootSignature(m_RootSignature.GetPtr());
    pCmd->SetPipelineState(m_ComputePSO.GetPtr());
    pCmd->SetComputeRoot32BitConstants(ROOT_CBV0, 2, &param, 0);
    pCmd->SetComputeRootDescriptorTable(ROOT_SRV0, handleSRV);
    pCmd->SetComputeRootDescriptorTable(ROOT_UAV0, handleUAV);
    pCmd->Dispatch(threadX, threadY, 1);
}

//-----------------------------------------------------------------------------
//      制御パラメータを設定します.
//-----------------------------------------------------------------------------
void MosaicEffect::SetScale(float value)
{ m_Scale = value; }

//-----------------------------------------------------------------------------
//      制御パラメータを取得します.
//-----------------------------------------------------------------------------
float MosaicEffect::GetScale() const
{ return m_Scale; }

} // namespace asdx

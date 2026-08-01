//-----------------------------------------------------------------------------
// File : asdxFxaa.cpp
// Desc : Fast Approximate Anti-Aliasing.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include <gfx/asdxFxaa.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxPresetState.h>


namespace {

//-----------------------------------------------------------------------------
// Shaders
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/asdxFxaaPS.inc"


///////////////////////////////////////////////////////////////////////////////
// Param structure
///////////////////////////////////////////////////////////////////////////////
struct Param
{
    float InvW;
    float InvH;
};

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Fxaa class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Fxaa::Fxaa()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Fxaa::~Fxaa()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Fxaa::Init(DXGI_FORMAT format)
{
    if (format == DXGI_FORMAT_UNKNOWN)
    {
        ELOGA("Error : Invalid Argument.");
        return false;
    }

    auto pDevice = GetD3D12Device();

    // ルートシグニチャ生成.
    {
        D3D12_DESCRIPTOR_RANGE range = {};
        range.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range.NumDescriptors                    = 1;
        range.BaseShaderRegister                = 0;
        range.RegisterSpace                     = 0;
        range.OffsetInDescriptorsFromTableStart = 0;

        D3D12_ROOT_PARAMETER param[2] = {};
        param[0].ParameterType              = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[0].Constants.Num32BitValues   = 2;
        param[0].Constants.ShaderRegister   = 0;
        param[0].Constants.RegisterSpace    = 0;
        param[0].ShaderVisibility           = D3D12_SHADER_VISIBILITY_PIXEL;

        param[1].ParameterType                          = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[1].DescriptorTable.NumDescriptorRanges    = 1;
        param[1].DescriptorTable.pDescriptorRanges      = &range;
        param[1].ShaderVisibility                       = D3D12_SHADER_VISIBILITY_PIXEL;

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

    // パイプラインステート生成.
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature                 = m_RootSignature.GetPtr();
        desc.VS                             = Preset::FullScreenVS;
        desc.PS                             = { asdxFxaaPS, sizeof(asdxFxaaPS) };
        desc.BlendState                     = Preset::Opaque;
        desc.SampleMask                     = D3D12_DEFAULT_SAMPLE_MASK;
        desc.RasterizerState                = Preset::CullNone;
        desc.DepthStencilState              = Preset::DepthNone;
        desc.InputLayout.NumElements        = _countof(Preset::QuadElements);
        desc.InputLayout.pInputElementDescs = Preset::QuadElements;
        desc.PrimitiveTopologyType          = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets               = 1;
        desc.RTVFormats[0]                  = format;
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

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Fxaa::Term()
{
    m_PipelineState.Reset();
    m_RootSignature.Reset();
}

//-----------------------------------------------------------------------------
//      FXAAを適用します.
//-----------------------------------------------------------------------------
void Fxaa::Apply
(
    ID3D12GraphicsCommandList*  pCmd,
    uint32_t                    width,
    uint32_t                    height,
    D3D12_GPU_DESCRIPTOR_HANDLE handleSRV
)
{
    if (pCmd == nullptr || width == 0 || height == 0 || handleSRV.ptr == 0)
        return;

    Param param = {};
    param.InvW = 1.0f / float(width);
    param.InvH = 1.0f / float(height);

    pCmd->SetGraphicsRootSignature(m_RootSignature.GetPtr());
    pCmd->SetPipelineState(m_PipelineState.GetPtr());
    pCmd->SetGraphicsRoot32BitConstants(0, 2, &param, 0);
    pCmd->SetGraphicsRootDescriptorTable(1, handleSRV);
    DrawQuad(pCmd);
}

} // namespace asdx

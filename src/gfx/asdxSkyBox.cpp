//-----------------------------------------------------------------------------
// File : asdxSkyBox.cpp
// Desc : Sky Box for Background.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include <gfx/asdxSkyBox.h>
#include <gfx/asdxPipelineState.h>
#include <gfx/asdxPresetState.h>
#include <gfx/asdxDevice.h>


namespace {

///////////////////////////////////////////////////////////////////////////////
// ROOT_PARAM enum
///////////////////////////////////////////////////////////////////////////////
enum ROOT_PARAM
{
    ROOT_PARAM_B0,
    ROOT_PARAM_B1,
    ROOT_PARAM_T0,
    ROOT_PARAM_U0,

    MAX_ROOT_PARAM_COUNT
};

///////////////////////////////////////////////////////////////////////////////
// Param structure
///////////////////////////////////////////////////////////////////////////////
struct Param
{
    asdx::Matrix View;
    asdx::Matrix Proj;
};

///////////////////////////////////////////////////////////////////////////////
// Constants structure
///////////////////////////////////////////////////////////////////////////////
struct Constants
{
    uint32_t    Width;
    uint32_t    Height;
    float       InvWidth;
    float       InvHeight;
};

//-----------------------------------------------------------------------------
// Shaders.
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/asdxSkyBoxPS.inc"
#include "../res/shaders/Compiled/asdxSkyBoxCS.inc"
#include "../res/shaders/Compiled/asdxSkySpherePS.inc"
#include "../res/shaders/Compiled/asdxSkySphereCS.inc"


} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// SkyContext class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
SkyContext::SkyContext()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
SkyContext::~SkyContext()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool SkyContext::Init()
{
    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    // ルートシグニチャ生成.
    {
        D3D12_DESCRIPTOR_RANGE ranges[2] = {};
        InitRangeAsSRV(ranges[0], 0);
        InitRangeAsUAV(ranges[1], 0);

        D3D12_ROOT_PARAMETER params[MAX_ROOT_PARAM_COUNT] = {};
        InitAsCBV(params[ROOT_PARAM_B0], 0, D3D12_SHADER_VISIBILITY_ALL);
        InitAsConstants(params[ROOT_PARAM_B1], 1, 4, D3D12_SHADER_VISIBILITY_ALL);
        InitAsTable(params[ROOT_PARAM_T0], 1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
        InitAsTable(params[ROOT_PARAM_U0], 1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);

        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.NumParameters      = _countof(params);
        desc.pParameters        = params;
        desc.NumStaticSamplers  = _countof(Preset::StaticSamplers);
        desc.pStaticSamplers    = Preset::StaticSamplers;
        desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        if (!InitRootSignature(pDevice, &desc, m_RootSignature.GetAddress()))
        {
            ELOG("Error : InitRootSingature() Failed.");
            return false;
        }
    }

    // 定数バッファ生成.
    {
        auto size = RoundUp<uint64_t>(sizeof(Param), 256);
        if (!m_Buffer.Init(size))
        {
            ELOG("Error : ConstantBuffer::Init() Failed.");
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void SkyContext::Term()
{
    m_RootSignature.Reset();
    m_Buffer.Term();
}

//-----------------------------------------------------------------------------
//      ルートシグニチャを取得します.
//-----------------------------------------------------------------------------
ID3D12RootSignature* SkyContext::GetRootSignature() const
{ return m_RootSignature.GetPtr(); }

//-----------------------------------------------------------------------------
//      定数バッファを取得します.
//-----------------------------------------------------------------------------
const DoubledConstantBuffer& SkyContext::GetBuffer() const
{ return m_Buffer; }

//-----------------------------------------------------------------------------
//      定数バッファを更新します.
//-----------------------------------------------------------------------------
void SkyContext::UpdateBuffer(const Matrix& view, const Matrix& proj)
{
    Param param = {};
    param.View = view;
    param.Proj = proj;

    m_Buffer.SwapBuffer();
    m_Buffer.Update(&param, sizeof(param));
}


///////////////////////////////////////////////////////////////////////////////
// SkyBoxPS class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです
//-----------------------------------------------------------------------------
SkyBoxPS::SkyBoxPS()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
SkyBoxPS::~SkyBoxPS()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool SkyBoxPS::Init(const SkyContext& context, DXGI_FORMAT colorFormat)
{
    auto pRootSignature = context.GetRootSignature();
    if (pRootSignature == nullptr)
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
    desc.pRootSignature                 = pRootSignature;
    desc.VS                             = Preset::FullScreenVS;
    desc.PS                             = { asdxSkyBoxPS, sizeof(asdxSkyBoxPS) };
    desc.BlendState                     = Preset::Opaque;
    desc.SampleMask                     = D3D12_DEFAULT_SAMPLE_MASK;
    desc.RasterizerState                = Preset::CullBack;
    desc.DepthStencilState              = Preset::DepthNone;
    desc.InputLayout.NumElements        = _countof(Preset::QuadElements);
    desc.InputLayout.pInputElementDescs = Preset::QuadElements;
    desc.PrimitiveTopologyType          = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    desc.NumRenderTargets               = 1;
    desc.RTVFormats[0]                  = colorFormat;
    desc.DSVFormat                      = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Quality             = 0;
    desc.SampleDesc.Count               = 1;

    auto hr = pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_PipelineState.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. errcode = 0x%x", hr);
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void SkyBoxPS::Term()
{ m_PipelineState.Reset(); }

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void SkyBoxPS::Draw
(
    ID3D12GraphicsCommandList*  pCmd,
    const SkyContext&           context,
    D3D12_GPU_DESCRIPTOR_HANDLE handleSRV
)
{
    if (pCmd == nullptr || handleSRV.ptr == 0)
        return;

    auto pRootSignature = context.GetRootSignature();
    if (pRootSignature == nullptr)
        return;

    auto addressBuffer = context.GetBuffer().GetGpuAddress();
    if (addressBuffer == 0)
        return;

    pCmd->SetGraphicsRootSignature(pRootSignature);
    pCmd->SetPipelineState(m_PipelineState.GetPtr());
    pCmd->SetGraphicsRootConstantBufferView(ROOT_PARAM_B0, addressBuffer);
    pCmd->SetGraphicsRootDescriptorTable(ROOT_PARAM_T0, handleSRV);

    DrawQuad(pCmd);
}


///////////////////////////////////////////////////////////////////////////////
// SkyBoxCS class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
SkyBoxCS::SkyBoxCS()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
SkyBoxCS::~SkyBoxCS()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool SkyBoxCS::Init(const SkyContext& context)
{
    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
    desc.pRootSignature = context.GetRootSignature();
    desc.CS = { asdxSkyBoxCS, sizeof(asdxSkyBoxCS) };

    auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_PipelineState.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void SkyBoxCS::Term()
{ m_PipelineState.Reset(); }

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void SkyBoxCS::Dispatch
(
    ID3D12GraphicsCommandList*  pCmd,
    const SkyContext&           context,
    uint32_t                    width,
    uint32_t                    height,
    D3D12_GPU_DESCRIPTOR_HANDLE handleUAV,
    D3D12_GPU_DESCRIPTOR_HANDLE handleSRV
)
{
    if (pCmd == nullptr || handleSRV.ptr == 0 || width == 0 || height == 0)
        return;

    auto pRootSignature = context.GetRootSignature();
    if (pRootSignature == nullptr)
        return;

    auto addressBuffer = context.GetBuffer().GetGpuAddress();
    if (addressBuffer == 0)
        return;

    Constants constants = {};
    constants.Width     = width;
    constants.Height    = height;
    constants.InvWidth  = 1.0f / float(width);
    constants.InvHeight = 1.0f / float(height);

    pCmd->SetComputeRootSignature(pRootSignature);
    pCmd->SetPipelineState(m_PipelineState.GetPtr());
    pCmd->SetComputeRootConstantBufferView(ROOT_PARAM_B0, addressBuffer);
    pCmd->SetComputeRoot32BitConstants(ROOT_PARAM_B1, 4, &constants, 0);
    pCmd->SetGraphicsRootDescriptorTable(ROOT_PARAM_T0, handleSRV);

    auto x = RoundDiv(width,  8u);
    auto y = RoundDiv(height, 8u);
    pCmd->Dispatch(x, y, 1);
}


///////////////////////////////////////////////////////////////////////////////
// SkySpherePS
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです
//-----------------------------------------------------------------------------
SkySpherePS::SkySpherePS()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
SkySpherePS::~SkySpherePS()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool SkySpherePS::Init(const SkyContext& context, DXGI_FORMAT colorFormat)
{
    auto pRootSignature = context.GetRootSignature();
    if (pRootSignature == nullptr)
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
    desc.pRootSignature                 = pRootSignature;
    desc.VS                             = Preset::FullScreenVS;
    desc.PS                             = { asdxSkySpherePS, sizeof(asdxSkySpherePS) };
    desc.BlendState                     = Preset::Opaque;
    desc.SampleMask                     = D3D12_DEFAULT_SAMPLE_MASK;
    desc.RasterizerState                = Preset::CullBack;
    desc.DepthStencilState              = Preset::DepthNone;
    desc.InputLayout.NumElements        = _countof(Preset::QuadElements);
    desc.InputLayout.pInputElementDescs = Preset::QuadElements;
    desc.PrimitiveTopologyType          = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    desc.NumRenderTargets               = 1;
    desc.RTVFormats[0]                  = colorFormat;
    desc.DSVFormat                      = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Quality             = 0;
    desc.SampleDesc.Count               = 1;

    auto hr = pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_PipelineState.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. errcode = 0x%x", hr);
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void SkySpherePS::Term()
{ m_PipelineState.Reset(); }

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void SkySpherePS::Draw
(
    ID3D12GraphicsCommandList*  pCmd,
    const SkyContext&           context,
    D3D12_GPU_DESCRIPTOR_HANDLE handleSRV
)
{
    if (pCmd == nullptr || handleSRV.ptr == 0)
        return;

    auto pRootSignature = context.GetRootSignature();
    if (pRootSignature == nullptr)
        return;

    auto addressBuffer = context.GetBuffer().GetGpuAddress();
    if (addressBuffer == 0)
        return;

    pCmd->SetGraphicsRootSignature(pRootSignature);
    pCmd->SetPipelineState(m_PipelineState.GetPtr());
    pCmd->SetGraphicsRootConstantBufferView(ROOT_PARAM_B0, addressBuffer);
    pCmd->SetGraphicsRootDescriptorTable(ROOT_PARAM_T0, handleSRV);

    DrawQuad(pCmd);
}


///////////////////////////////////////////////////////////////////////////////
// SkySphereCS class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
SkySphereCS::SkySphereCS()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
SkySphereCS::~SkySphereCS()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool SkySphereCS::Init(const SkyContext& context)
{
    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
    desc.pRootSignature = context.GetRootSignature();
    desc.CS = { asdxSkySphereCS, sizeof(asdxSkySphereCS) };

    auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_PipelineState.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void SkySphereCS::Term()
{ m_PipelineState.Reset(); }

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void SkySphereCS::Dispatch
(
    ID3D12GraphicsCommandList*  pCmd,
    const SkyContext&           context,
    uint32_t                    width,
    uint32_t                    height,
    D3D12_GPU_DESCRIPTOR_HANDLE handleUAV,
    D3D12_GPU_DESCRIPTOR_HANDLE handleSRV
)
{
    if (pCmd == nullptr || handleSRV.ptr == 0 || width == 0 || height == 0)
        return;

    auto pRootSignature = context.GetRootSignature();
    if (pRootSignature == nullptr)
        return;

    auto addressBuffer = context.GetBuffer().GetGpuAddress();
    if (addressBuffer == 0)
        return;

    Constants constants = {};
    constants.Width     = width;
    constants.Height    = height;
    constants.InvWidth  = 1.0f / float(width);
    constants.InvHeight = 1.0f / float(height);

    pCmd->SetComputeRootSignature(pRootSignature);
    pCmd->SetPipelineState(m_PipelineState.GetPtr());
    pCmd->SetComputeRootConstantBufferView(ROOT_PARAM_B0, addressBuffer);
    pCmd->SetComputeRoot32BitConstants(ROOT_PARAM_B1, 4, &constants, 0);
    pCmd->SetGraphicsRootDescriptorTable(ROOT_PARAM_T0, handleSRV);

    auto x = RoundDiv(width,  8u);
    auto y = RoundDiv(height, 8u);
    pCmd->Dispatch(x, y, 1);
}

} // namespace asdx


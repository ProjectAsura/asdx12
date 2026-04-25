//-----------------------------------------------------------------------------
// File : asdxGaussianBlurEffect.cpp
// Desc : Gaussian Blur Effect.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include <gfx/asdxGaussianBlurEffect.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxPresetState.h>


namespace {

//-----------------------------------------------------------------------------
// Shaders.
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/asdxGaussianBlurPS.inc"
#include "../res/shaders/Compiled/asdxGaussianBlurCS.inc"


///////////////////////////////////////////////////////////////////////////////
// Param structure
///////////////////////////////////////////////////////////////////////////////
struct Param
{
    float       Weights[8];
    float       OffsetX;
    float       OffsetY;
    uint32_t    Width;
    uint32_t    Height;
};

//-----------------------------------------------------------------------------
//      ガウスブラーの重みを計算します.
//-----------------------------------------------------------------------------
void ComputeGaussWeights(float dispersion, Param& param)
{
    float total = 0.0f;
    for(auto i=0; i<8; ++i)
    {
        auto pos = 0.5f + 2.0f * i;
        param.Weights[i] = expf(-0.5f * (pos * pos) / dispersion);
        total += 2.0f * param.Weights[i];
    }

    float invTotal = 1.0f / total;
    for(auto i=0; i<8; ++i)
        param.Weights[i] *= invTotal;
}

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// GaussianBlurEffectPS class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
GaussianBlurEffectPS::GaussianBlurEffectPS()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
GaussianBlurEffectPS::~GaussianBlurEffectPS()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool GaussianBlurEffectPS::Init(uint32_t width, uint32_t height, DXGI_FORMAT format)
{
    auto pDevice = asdx::GetD3D12Device();

    // ルートシグニチャの初期化.
    {
        D3D12_DESCRIPTOR_RANGE range = {};
        range.RangeType                          = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range.NumDescriptors                     = 1;
        range.BaseShaderRegister                 = 0;
        range.RegisterSpace                      = 0;
        range.OffsetInDescriptorsFromTableStart  = 0;

        D3D12_ROOT_PARAMETER param[2] = {};
        param[0].ParameterType              = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[0].Constants.Num32BitValues   = 12;
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
        desc.PS                             = { asdxGaussianBlurPS, sizeof(asdxGaussianBlurPS) };
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

        auto hr = pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_PipelineState.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateGraphicsPipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // カラーターゲットを初期化.
    {
        TargetDesc desc = {};
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width              = width;
        desc.Height             = height;
        desc.DepthOrArraySize   = 1;
        desc.MipLevels          = 1;
        desc.Format             = format;
        desc.SampleDesc         = { 1, 0 };
        desc.InitState          = D3D12_RESOURCE_STATE_COMMON;
        desc.ClearColor[0]      = 0.0f;
        desc.ClearColor[1]      = 0.0f;
        desc.ClearColor[2]      = 0.0f;
        desc.ClearColor[3]      = 0.0f;

        for(auto i=0; i<2; ++i)
        {
            if (!m_ColorTarget[i].Init(&desc))
            {
                ELOGA("Error : ColorTarget::Init() Failed.");
                return false;
            }
        }
    }

    m_State = D3D12_RESOURCE_STATE_COMMON;

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void GaussianBlurEffectPS::Term()
{
    for(auto i=0; i<2; ++i)
    { m_ColorTarget[i].Term(); }
    m_PipelineState.Reset();
    m_RootSignature.Reset();
}

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void GaussianBlurEffectPS::Draw
(
    ID3D12GraphicsCommandList*  pCmd,
    float                       dispersion,
    D3D12_GPU_DESCRIPTOR_HANDLE handleSRV
)
{
    assert(pCmd != nullptr);
    assert(dispersion > 0.0f);

    auto desc = GetDesc();

    Param param = {};
    param.OffsetX = 1.0f / float(desc.Width);
    param.OffsetY = 0.0f;
    param.Width   = uint32_t(desc.Width);
    param.Height  = desc.Height;
    ComputeGaussWeights(dispersion, param);

    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX   = 0;
    viewport.TopLeftY   = 0;
    viewport.Width      = float(desc.Width);
    viewport.Height     = float(desc.Height);
    viewport.MinDepth   = 0.0f;
    viewport.MaxDepth   = 1.0f;

    D3D12_RECT rect = {};
    rect.left   = 0;
    rect.right  = LONG(desc.Width);
    rect.top    = 0;
    rect.bottom = LONG(desc.Height);

    D3D12_RESOURCE_BARRIER barriers[2] = {};
    barriers[0].Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[0].Transition.pResource    = m_ColorTarget[0].GetResource();
    barriers[0].Transition.StateBefore  = m_State;
    barriers[0].Transition.StateAfter   = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barriers[0].Transition.Subresource  = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    pCmd->ResourceBarrier(1, barriers);

    // ルートシグニチャとパイプラインステートを設定.
    pCmd->SetGraphicsRootSignature(m_RootSignature.GetPtr());
    pCmd->SetPipelineState(m_PipelineState.GetPtr());

    // 水平方向ブラー.
    auto handleRTV = m_ColorTarget[0].GetCpuHandleRTV();
    pCmd->OMSetRenderTargets(1, &handleRTV, FALSE, nullptr);
    pCmd->RSSetViewports(1, &viewport);
    pCmd->RSSetScissorRects(1, &rect);
    pCmd->SetGraphicsRoot32BitConstants(0, 12, &param, 0);
    pCmd->SetGraphicsRootDescriptorTable(1, handleSRV);
    DrawQuad(pCmd);

    barriers[0].Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[0].Transition.pResource    = m_ColorTarget[0].GetResource();
    barriers[0].Transition.StateBefore  = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barriers[0].Transition.StateAfter   = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barriers[0].Transition.Subresource  = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    barriers[1].Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[1].Transition.pResource    = m_ColorTarget[1].GetResource();
    barriers[1].Transition.StateBefore  = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barriers[1].Transition.StateAfter   = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barriers[1].Transition.Subresource  = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    pCmd->ResourceBarrier(2, barriers);

    param.OffsetX = 0.0f;
    param.OffsetY = 1.0f / float(desc.Height);
    handleSRV = m_ColorTarget[0].GetGpuHandleSRV();
    handleRTV = m_ColorTarget[1].GetCpuHandleRTV();

    // 垂直方向ブラー.
    pCmd->OMSetRenderTargets(1, &handleRTV, FALSE, nullptr);
    pCmd->RSSetViewports(1, &viewport);
    pCmd->RSSetScissorRects(1, &rect);
    pCmd->SetGraphicsRoot32BitConstants(0, 12, &param, 0);
    pCmd->SetGraphicsRootDescriptorTable(1, handleSRV);
    DrawQuad(pCmd);

    barriers[0].Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[0].Transition.pResource    = m_ColorTarget[1].GetResource();
    barriers[0].Transition.StateBefore  = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barriers[0].Transition.StateAfter   = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barriers[0].Transition.Subresource  = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    m_State = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    pCmd->ResourceBarrier(1, barriers);
}

//-----------------------------------------------------------------------------
//      GPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE GaussianBlurEffectPS::GetHandleGPU() const
{ return m_ColorTarget[1].GetGpuHandleSRV(); }

//-----------------------------------------------------------------------------
//      構成設定を取得します.
//-----------------------------------------------------------------------------
TargetDesc GaussianBlurEffectPS::GetDesc() const
{ return m_ColorTarget[0].GetDesc(); }

//-----------------------------------------------------------------------------
//      ターゲットをリサイズします.
//-----------------------------------------------------------------------------
bool GaussianBlurEffectPS::Resize(uint32_t width, uint32_t height)
{
    for(auto i=0; i<2; ++i)
    {
        if (!m_ColorTarget[i].Resize(width, height))
        { return false; }
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// GaussianBlurEffectCS class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
GaussianBlurEffectCS::GaussianBlurEffectCS()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
GaussianBlurEffectCS::~GaussianBlurEffectCS()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool GaussianBlurEffectCS::Init(uint32_t width, uint32_t height, DXGI_FORMAT format)
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

        D3D12_ROOT_PARAMETER param[3] = {};
        param[0].ParameterType              = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[0].Constants.Num32BitValues   = 12;
        param[0].Constants.ShaderRegister   = 0;
        param[0].Constants.RegisterSpace    = 0;
        param[0].ShaderVisibility           = D3D12_SHADER_VISIBILITY_ALL;

        param[1].ParameterType                          = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[1].DescriptorTable.NumDescriptorRanges    = 1;
        param[1].DescriptorTable.pDescriptorRanges      = &range[0];
        param[1].ShaderVisibility                       = D3D12_SHADER_VISIBILITY_ALL;

        param[2].ParameterType                          = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[2].DescriptorTable.NumDescriptorRanges    = 1;
        param[2].DescriptorTable.pDescriptorRanges      = &range[1];
        param[2].ShaderVisibility                       = D3D12_SHADER_VISIBILITY_ALL;

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

    // コンピュートパイプラインステートの初期化.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = m_RootSignature.GetPtr();
        desc.CS = { asdxGaussianBlurCS, sizeof(asdxGaussianBlurCS) };

        auto hr = pDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_PipelineState.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateComputePipelineState() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // コンピュートターゲットを初期化.
    {
        TargetDesc desc = {};
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width              = width;
        desc.Height             = height;
        desc.DepthOrArraySize   = 1;
        desc.MipLevels          = 1;
        desc.Format             = format;
        desc.SampleDesc         = { 1, 0 };
        desc.InitState          = D3D12_RESOURCE_STATE_COMMON;
        desc.ClearColor[0]      = 0.0f;
        desc.ClearColor[1]      = 0.0f;
        desc.ClearColor[2]      = 0.0f;
        desc.ClearColor[3]      = 0.0f;

        for(auto i=0; i<2; ++i)
        {
            if (!m_ComputeTarget[i].Init(&desc))
            {
                ELOGA("Error : ColorTarget::Init() Failed.");
                return false;
            }
        }

        m_State = D3D12_RESOURCE_STATE_COMMON;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void GaussianBlurEffectCS::Term()
{
    for(auto i=0; i<2; ++i)
    { m_ComputeTarget[i].Term(); }
    m_PipelineState.Reset();
    m_RootSignature.Reset();
}

//-----------------------------------------------------------------------------
//      コンピュートシェーダを起動します.
//-----------------------------------------------------------------------------
void GaussianBlurEffectCS::Dispatch(ID3D12GraphicsCommandList* pCmd, float dispersion, D3D12_GPU_DESCRIPTOR_HANDLE handleSRV)
{
    assert(pCmd != nullptr);
    assert(dispersion > 0.0f);

    auto desc = GetDesc();

    Param param = {};
    param.OffsetX = 1.0f / float(desc.Width);
    param.OffsetY = 0.0f;
    param.Width   = uint32_t(desc.Width);
    param.Height  = desc.Height;
    ComputeGaussWeights(dispersion, param);

    pCmd->SetComputeRootSignature(m_RootSignature.GetPtr());
    pCmd->SetPipelineState(m_PipelineState.GetPtr());

    D3D12_RESOURCE_BARRIER barriers[3] = {};
    barriers[0].Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[0].Transition.pResource    = m_ComputeTarget[0].GetResource();
    barriers[0].Transition.StateBefore  = m_State;
    barriers[0].Transition.StateAfter   = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    barriers[0].Transition.Subresource  = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    pCmd->ResourceBarrier(1, barriers);

    auto threadX = (uint32_t(desc.Width)  + 7) / 8;
    auto threadY = (desc.Height + 7) / 8;

    auto handleUAV = m_ComputeTarget[0].GetGpuHandleUAV();

    pCmd->SetComputeRoot32BitConstants(0, 12, &param, 0);
    pCmd->SetComputeRootDescriptorTable(1, handleSRV);
    pCmd->SetComputeRootDescriptorTable(2, handleUAV);
    pCmd->Dispatch(threadX, threadY, 1);

    handleSRV = m_ComputeTarget[0].GetGpuHandleSRV();
    handleUAV = m_ComputeTarget[1].GetGpuHandleUAV();

    barriers[0].Type            = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barriers[0].UAV.pResource   = m_ComputeTarget[0].GetResource();

    barriers[1].Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[1].Transition.pResource    = m_ComputeTarget[0].GetResource();
    barriers[1].Transition.StateBefore  = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    barriers[1].Transition.StateAfter   = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    barriers[1].Transition.Subresource  = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    barriers[2].Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[2].Transition.pResource    = m_ComputeTarget[1].GetResource();
    barriers[2].Transition.StateBefore  = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barriers[2].Transition.StateAfter   = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    barriers[2].Transition.Subresource  = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    pCmd->ResourceBarrier(3, barriers);

    param.OffsetX = 0.0f;
    param.OffsetY = 1.0f / float(desc.Height);
    pCmd->SetComputeRoot32BitConstants(0, 12, &param, 0);
    pCmd->SetComputeRootDescriptorTable(1, handleSRV);
    pCmd->SetComputeRootDescriptorTable(2, handleUAV);
    pCmd->Dispatch(threadX, threadY, 1);

    barriers[0].Type            = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barriers[0].UAV.pResource   = m_ComputeTarget[1].GetResource();

    barriers[1].Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[1].Transition.pResource    = m_ComputeTarget[1].GetResource();
    barriers[1].Transition.StateBefore  = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    barriers[1].Transition.StateAfter   = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barriers[1].Transition.Subresource  = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    pCmd->ResourceBarrier(2, barriers);

    m_State = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
}

//-----------------------------------------------------------------------------
//      構成設定を取得します.
//-----------------------------------------------------------------------------
TargetDesc GaussianBlurEffectCS::GetDesc() const
{ return m_ComputeTarget[0].GetDesc(); }

//-----------------------------------------------------------------------------
//      GPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE GaussianBlurEffectCS::GetHandleGPU() const
{ return m_ComputeTarget[1].GetGpuHandleSRV(); }

//-----------------------------------------------------------------------------
//      ターゲットをリサイズします.
//-----------------------------------------------------------------------------
bool GaussianBlurEffectCS::Resize(uint32_t width, uint32_t height)
{
    for(auto i=0; i<2; ++i)
    {
        if (!m_ComputeTarget->Resize(width, height))
        { return false; }
    }

    return true;
}

} // namespace asdx

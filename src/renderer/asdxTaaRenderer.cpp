//-----------------------------------------------------------------------------
// File : asdxTaaRenderer.cpp
// Desc : Temporal Anti-Aliasing Renderer
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <renderer/asdxTaaRenderer.h>
#include <gfx/asdxGraphicsSystem.h>
#include <gfx/asdxQuad.h>
#include <fnd/asdxMath.h>
#include <fnd/asdxLogger.h>


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/FullScreenVS.inc"
#include "../res/shaders/Compiled/TaaPS.inc"
#include "../res/shaders/Compiled/TaaCS.inc"


///////////////////////////////////////////////////////////////////////////////
// Param structure
///////////////////////////////////////////////////////////////////////////////
struct Param
{
    float           Gamma;
    float           BlendFactor;
    asdx::Vector2   MapSize;
    asdx::Vector2   InvMapSize;
    asdx::Vector2   Jitter;
};

//-----------------------------------------------------------------------------
//      ハルトン数列.
//-----------------------------------------------------------------------------
float HaltonSequence(uint32_t i, uint32_t b)
{
    float f = 1.0f;
    float r = 0.0f;
    while (i > 0)
    {
        f = f / b;
        r = r + f * (i % b);
        i = i / b;
    }
    return r;
}

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// TaaRenderer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      ピクセルシェーダ用初期化処理を行います.
//-----------------------------------------------------------------------------
bool TaaRenderer::InitPS(DXGI_FORMAT format)
{
    auto pDevice = GetD3D12Device();

    if (!Quad::Instance().IsInit())
    {
        if (!Quad::Instance().Init(pDevice))
        {
            ELOGA("Error : Quad::Init() Failed.");
            return false;
        }
    }

    // ルートシグニチャの生成.
    {
        DescriptorSetLayout<5, 2> layout;
        layout.SetContants(0, SV_PS, 8, 0);
        layout.SetTableSRV(1, SV_PS, 0);
        layout.SetTableSRV(2, SV_PS, 1);
        layout.SetTableSRV(3, SV_PS, 2);
        layout.SetTableSRV(4, SV_PS, 3);

        layout.SetStaticSampler(0, SV_PS, STATIC_SAMPLER_POINT_CLAMP, 0);
        layout.SetStaticSampler(1, SV_PS, STATIC_SAMPLER_LINEAR_CLAMP, 1);
        layout.SetFlags(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        if (!m_RootSigPS.Init(pDevice, layout.GetDesc()))
        {
            ELOGA("Error : RootSignature::Init() Failed.");
            return false;
        }
    }

    // パイプラインステートの生成.
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature         = m_RootSigPS.GetPtr();
        desc.VS                     = { FullScreenVS, sizeof(FullScreenVS) };
        desc.PS                     = { TaaPS, sizeof(TaaPS) };
        desc.BlendState             = BLEND_DESC(BLEND_STATE_OPAQUE);
        desc.RasterizerState        = RASTERIZER_DESC(RASTERIZER_STATE_CULL_NONE);
        desc.DepthStencilState      = DEPTH_STENCIL_DESC(DEPTH_STATE_NONE);
        desc.SampleMask             = D3D12_DEFAULT_SAMPLE_MASK;
        desc.SampleDesc.Count       = 1;
        desc.SampleDesc.Quality     = 0;
        desc.InputLayout            = Quad::kInputLayout;
        desc.PrimitiveTopologyType  = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets       = 1;
        desc.RTVFormats[0]          = format;

        if (!m_PipelineStatePS.Init(pDevice, &desc))
        {
            ELOGA("Error : PipelineState::Init() Failed.");
            return false;
        }
    }

    return true;
}


//-----------------------------------------------------------------------------
//      コンピュートシェーダ用初期化処理を行います.
//-----------------------------------------------------------------------------
bool TaaRenderer::InitCS()
{
    auto pDevice = GetD3D12Device();

    // ルートシグニチャの生成.
    {
        DescriptorSetLayout<6, 2> layout;
        layout.SetContants(0, SV_ALL, 8, 0);
        layout.SetTableSRV(1, SV_ALL, 0);
        layout.SetTableSRV(2, SV_ALL, 1);
        layout.SetTableSRV(3, SV_ALL, 2);
        layout.SetTableSRV(4, SV_ALL, 3);
        layout.SetTableUAV(5, SV_ALL, 0);

        layout.SetStaticSampler(0, SV_ALL, STATIC_SAMPLER_POINT_CLAMP, 0);
        layout.SetStaticSampler(1, SV_ALL, STATIC_SAMPLER_LINEAR_CLAMP, 1);

        if (!m_RootSigCS.Init(pDevice, layout.GetDesc()))
        {
            ELOGA("Error : RootSignature::Init() Failed.");
            return false;
        }
    }

    // パイプラインステートの生成.
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = m_RootSigCS.GetPtr();
        desc.CS             = { TaaCS, sizeof(TaaCS) };

        if (!m_PipelineStateCS.Init(pDevice, &desc))
        {
            ELOGA("Error : PipelineState::Init() Failed.");
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void TaaRenderer::Term()
{
    m_RootSigPS      .Term();
    m_RootSigCS      .Term();
    m_PipelineStatePS.Term();
    m_PipelineStateCS.Term();

    if (Quad::Instance().IsInit())
    { Quad::Instance().Term(); }
}

//-----------------------------------------------------------------------------
//      ピクセルシェーダを用いて描画処理を行います.
//-----------------------------------------------------------------------------
void TaaRenderer::RenderPS
(
    ID3D12GraphicsCommandList*  pCmdList,
    const IRenderTargetView*    pRTV,
    const IShaderResourceView*  pCurrentColorSRV,
    const IShaderResourceView*  pHistoryColorSRV,
    const IShaderResourceView*  pVelocitySRV,
    const IShaderResourceView*  pDepthSRV,
    float                       gamma,
    float                       alpha,
    const asdx::Vector2&        jitter
)
{
    auto desc = pHistoryColorSRV->GetResource()->GetDesc();
    auto w = uint32_t(desc.Width);
    auto h = desc.Height;

    Param param = {};
    param.Gamma         = gamma;
    param.BlendFactor   = alpha;
    param.MapSize.x     = float(w);
    param.MapSize.y     = float(h);
    param.InvMapSize.x  = 1.0f / param.MapSize.x;
    param.InvMapSize.y  = 1.0f / param.MapSize.y;
    param.Jitter        = jitter;

    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX   = 0.0f;
    viewport.TopLeftY   = 0.0f;
    viewport.Width      = param.MapSize.x;
    viewport.Height     = param.MapSize.y;
    viewport.MinDepth   = 0.0f;
    viewport.MaxDepth   = 1.0f;

    D3D12_RECT scissor = {};
    scissor.top     = 0;
    scissor.left    = 0;
    scissor.right   = LONG(param.MapSize.x);
    scissor.bottom  = LONG(param.MapSize.y);

    auto handleRTV = pRTV->GetHandleCPU();

    pCmdList->RSSetViewports(1, &viewport);
    pCmdList->RSSetScissorRects(1, &scissor);
    pCmdList->OMSetRenderTargets(1, &handleRTV, FALSE, nullptr);

    pCmdList->SetGraphicsRootSignature(m_RootSigPS.GetPtr());
    pCmdList->SetPipelineState(m_PipelineStatePS.GetPtr());
    pCmdList->SetGraphicsRoot32BitConstants(0, 8, &param, 0);
    pCmdList->SetGraphicsRootDescriptorTable(1, pCurrentColorSRV->GetHandleGPU());
    pCmdList->SetGraphicsRootDescriptorTable(2, pHistoryColorSRV->GetHandleGPU());
    pCmdList->SetGraphicsRootDescriptorTable(3, pVelocitySRV->GetHandleGPU());
    pCmdList->SetGraphicsRootDescriptorTable(4, pDepthSRV->GetHandleGPU());
    Quad::Instance().Draw(pCmdList);
}

//-----------------------------------------------------------------------------
//      コンピュートシェーダを用いて描画処理を行います.
//-----------------------------------------------------------------------------
void TaaRenderer::RenderCS
(
    ID3D12GraphicsCommandList*  pCmdList,
    const IUnorderedAccessView* pUAV,
    const IShaderResourceView*  pCurrentColorSRV,
    const IShaderResourceView*  pHistoryColorSRV,
    const IShaderResourceView*  pVelocitySRV,
    const IShaderResourceView*  pDepthSRV,
    float                       gamma,
    float                       alpha,
    const asdx::Vector2&        jitter
)
{
    auto desc = pUAV->GetResource()->GetDesc();
    auto w = uint32_t(desc.Width);
    auto h = desc.Height;

    Param param = {};
    param.Gamma         = gamma;
    param.BlendFactor   = alpha;
    param.MapSize.x     = float(w);
    param.MapSize.y     = float(h);
    param.InvMapSize.x  = 1.0f / param.MapSize.x;
    param.InvMapSize.y  = 1.0f / param.MapSize.y;
    param.Jitter        = jitter;

    auto threadX = (w + 7) / 8;
    auto threadY = (h + 7) / 8;

    pCmdList->SetComputeRootSignature(m_RootSigCS.GetPtr());
    pCmdList->SetPipelineState(m_PipelineStateCS.GetPtr());
    pCmdList->SetComputeRoot32BitConstants(0, 8, &param, 0);
    pCmdList->SetComputeRootDescriptorTable(1, pCurrentColorSRV->GetHandleGPU());
    pCmdList->SetComputeRootDescriptorTable(2, pHistoryColorSRV->GetHandleGPU());
    pCmdList->SetComputeRootDescriptorTable(3, pVelocitySRV->GetHandleGPU());
    pCmdList->SetComputeRootDescriptorTable(4, pDepthSRV->GetHandleGPU());
    pCmdList->SetComputeRootDescriptorTable(5, pUAV->GetHandleGPU());
    pCmdList->Dispatch(threadX, threadY, 1);
}

//-----------------------------------------------------------------------------
//      ジッタ―値を計算します
//-----------------------------------------------------------------------------
asdx::Vector2 TaaRenderer::CalcJitter(uint32_t jitterIndex, uint32_t w, uint32_t h)
{
    asdx::Vector2 result;
    result.x = (HaltonSequence(jitterIndex + 1, 2) - 0.5f) / float(w);
    result.y = (HaltonSequence(jitterIndex + 1, 3) - 0.5f) / float(h);
    return result;
}

} // namespace asdx

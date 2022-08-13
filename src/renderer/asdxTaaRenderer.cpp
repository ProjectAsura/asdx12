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


///////////////////////////////////////////////////////////////////////////////
// Param structure
///////////////////////////////////////////////////////////////////////////////
struct Param
{
    float           Gamma;
    float           BlendFactor;
    asdx::Vector2   MapSize;
    asdx::Vector2   CurrJitter;
    asdx::Vector2   PrevJitter;
};

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// TaaRenderer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool TaaRenderer::Init(DXGI_FORMAT format)
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
        layout.SetContants(0, SV_PS, 4, 0);
        layout.SetTableSRV(1, SV_PS, 0);
        layout.SetTableSRV(2, SV_PS, 1);
        layout.SetTableSRV(3, SV_PS, 2);
        layout.SetTableSRV(4, SV_PS, 3);

        layout.SetStaticSampler(0, SV_PS, STATIC_SAMPLER_POINT_CLAMP, 0);
        layout.SetStaticSampler(1, SV_PS, STATIC_SAMPLER_LINEAR_CLAMP, 1);
        layout.SetFlags(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        if (!m_RootSig.Init(pDevice, layout.GetDesc()))
        {
            ELOGA("Error : RootSignature::Init() Failed.");
            return false;
        }
    }

    // パイプラインステートの生成.
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature         = m_RootSig.GetPtr();
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

        if (!m_PipelineState.Init(pDevice, &desc))
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
    m_RootSig      .Term();
    m_PipelineState.Term();

    if (Quad::Instance().IsInit())
    { Quad::Instance().Term(); }
}

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void TaaRenderer::Render
(
    ID3D12GraphicsCommandList*  pCmdList,
    const IRenderTargetView*    pRTV,
    const IShaderResourceView*  pCurrentColorSRV,
    const IShaderResourceView*  pHistoryColorSRV,
    const IShaderResourceView*  pVelocitySRV,
    const IShaderResourceView*  pDepthSRV,
    float                       gamma,
    float                       alpha
)
{
    Param param = {};
    param.Gamma         = gamma;
    param.BlendFactor   = alpha;
    param.MapSize.x     = float(pHistoryColorSRV->GetResource()->GetDesc().Width);
    param.MapSize.y     = float(pHistoryColorSRV->GetResource()->GetDesc().Height);

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

    pCmdList->SetGraphicsRootSignature(m_RootSig.GetPtr());
    pCmdList->SetPipelineState(m_PipelineState.GetPtr());
    pCmdList->SetGraphicsRoot32BitConstants(0, 4, &param, 0);
    pCmdList->SetGraphicsRootDescriptorTable(1, pCurrentColorSRV->GetHandleGPU());
    pCmdList->SetGraphicsRootDescriptorTable(2, pHistoryColorSRV->GetHandleGPU());
    pCmdList->SetGraphicsRootDescriptorTable(3, pVelocitySRV->GetHandleGPU());
    pCmdList->SetGraphicsRootDescriptorTable(4, pDepthSRV->GetHandleGPU());
    Quad::Instance().Draw(pCmdList);
}

} // namespace asdx

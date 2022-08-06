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
    float   Gamma;
    float   BlendFactor;
    float   HistoryMapWidth;
    float   HistoryMapHeight;
};

///////////////////////////////////////////////////////////////////////////////
// Jitter structure
///////////////////////////////////////////////////////////////////////////////
struct Jitter
{
    int32_t CurrJitterX;
    int32_t CurrJitterY;
    int32_t PrevJitterX;
    int32_t PrevJitterY;
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
        DescriptorSetLayout<4, 3> layout;
        layout.SetContants(0, SV_PS, 4, 0);
        layout.SetTableSRV(1, SV_PS, 0);
        layout.SetTableSRV(2, SV_PS, 1);
        layout.SetTableSRV(3, SV_PS, 2);

        layout.SetStaticSampler(0, SV_PS, STATIC_SAMPLER_POINT_CLAMP, 0);
        layout.SetStaticSampler(1, SV_PS, STATIC_SAMPLER_POINT_CLAMP, 0);
        layout.SetStaticSampler(2, SV_PS, STATIC_SAMPLER_POINT_CLAMP, 0);

        if (!m_RootSig.Init(pDevice, layout.GetDesc()))
        {
            ELOGA("Error : RootSignature::Init() Failed.");
            return false;
        }
    }

    // パイプラインステートの生成.
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature     = m_RootSig.GetPtr();
        desc.VS                 = { FullScreenVS, sizeof(FullScreenVS) };
        desc.PS                 = { TaaPS, sizeof(TaaPS) };
        desc.BlendState         = BLEND_DESC(BLEND_STATE_OPAQUE);
        desc.RasterizerState    = RASTERIZER_DESC(RASTERIZER_STATE_CULL_NONE);
        desc.DepthStencilState  = DEPTH_STENCIL_DESC(DEPTH_STATE_NONE);
        desc.SampleMask         = D3D12_DEFAULT_SAMPLE_MASK;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.NumRenderTargets   = 1;
        desc.RTVFormats[0]      = format;

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
}

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void TaaRenderer::Render
(
    ID3D12GraphicsCommandList*  pCmdList,
    IRenderTargetView*          pRTV,
    IShaderResourceView*        pCurrentColorSRV,
    IShaderResourceView*        pHistoryColorSRV,
    IShaderResourceView*        pVelocitySRV,
    float                       gamma,
    float                       alpha
)
{
    Param param = {};
    param.Gamma             = gamma;
    param.BlendFactor       = alpha;
    param.HistoryMapWidth   = float(pHistoryColorSRV->GetResource()->GetDesc().Width);
    param.HistoryMapHeight  = float(pHistoryColorSRV->GetResource()->GetDesc().Height);

    pCmdList->SetGraphicsRootSignature(m_RootSig.GetPtr());
    pCmdList->SetPipelineState(m_PipelineState.GetPtr());
    pCmdList->SetGraphicsRoot32BitConstants(0, 4, &param, 0);
    pCmdList->SetGraphicsRootDescriptorTable(1, pCurrentColorSRV->GetHandleGPU());
    pCmdList->SetGraphicsRootDescriptorTable(2, pHistoryColorSRV->GetHandleGPU());
    pCmdList->SetGraphicsRootDescriptorTable(3, pVelocitySRV->GetHandleGPU());
    Quad::Instance().Draw(pCmdList);
}

} // namespace asdx

//-----------------------------------------------------------------------------
// File : TestApp.cpp
// Desc : Test Application.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "TestApp.h"
#include <asdxCmdHelper.h>
#include <asdxMath.h>
#include <asdxGraphicsDevice.h>
#include <asdxLogger.h>
#include <asdxMisc.h>
#include <asdxGuiMgr.h>
#include <d3d12.h>

#include "TestVS.inc"
#include "TestPS.inc"


namespace {

///////////////////////////////////////////////////////////////////////////////
// Vertex structure
///////////////////////////////////////////////////////////////////////////////
struct Vertex
{
    asdx::Vector3 Position;
    asdx::Vector2 TexCoord;
};

///////////////////////////////////////////////////////////////////////////////
// CbMesh structure
///////////////////////////////////////////////////////////////////////////////
struct alignas(256) CbMesh
{
    asdx::Matrix    World;
};

///////////////////////////////////////////////////////////////////////////////
// CbScene structure
///////////////////////////////////////////////////////////////////////////////
struct alignas(256) CbScene
{
    asdx::Matrix    View;
    asdx::Matrix    Proj;
};

} // namespace

///////////////////////////////////////////////////////////////////////////////
// TestApp class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
TestApp::TestApp()
: asdx::Application(L"Test App", 960, 540, nullptr, nullptr, nullptr)
{
}

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
TestApp::~TestApp()
{
}

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool TestApp::OnInit()
{
    m_pGraphicsQueue = asdx::GfxDevice().GetGraphicsQueue();

    auto pDevice = asdx::GfxDevice().GetDevice();

    // 頂点データ
    {
        const Vertex vertices[] = {
            { asdx::Vector3( 0.0f,  0.5f, 0.0f), asdx::Vector2(0.5f, 1.0f) },
            { asdx::Vector3( 0.5f, -0.5f, 0.0f), asdx::Vector2(1.0f, 0.0f) },
            { asdx::Vector3(-0.5f, -0.5f, 0.0f), asdx::Vector2(0.0f, 0.0f) },
        };

        if (!m_TriangleVB.Init(asdx::GfxDevice(), sizeof(vertices), sizeof(Vertex)))
        {
            ELOG("Error : VertexBuffer::Init() Failed.");
            return false;
        }

        auto ptr = m_TriangleVB.Map<Vertex>();
        memcpy(ptr, vertices, sizeof(vertices));
        m_TriangleVB.Unmap();
    }

    // ルートシグニチャ.
    {
        uint32_t flag = asdx::RSF_ALLOW_IA;
        flag |= asdx::RSF_DENY_GS;
        flag |= asdx::RSF_DENY_DS;
        flag |= asdx::RSF_DENY_HS;
        flag |= asdx::RSF_DENY_AS;
        flag |= asdx::RSF_DENY_MS;

        asdx::RootSignatureDesc desc;
        desc.AddFromShader(TestVS, sizeof(TestVS))
            .AddFromShader(TestPS, sizeof(TestPS));
        //m_IndexCBMesh       = desc.AddCBV(asdx::SV_VS, 0);
        //m_IndexCBScene      = desc.AddCBV(asdx::SV_VS, 1);
        //m_IndexColorMap     = desc.AddSRV(asdx::SV_PS, 0);
        //m_IndexLinearClamp  = desc.AddStaticSampler(asdx::SV_PS, asdx::SS_LINEAR_CLAMP, 0);
        desc.SetFlag(flag);

        if (!m_RootSignature.Init(pDevice, desc))
        {
            ELOG("Error : RootSignature::Init() Failed.");
            return false;
        }

        m_IndexCBMesh       = m_RootSignature.Find("CbMesh");
        m_IndexCBScene      = m_RootSignature.Find("CbScene");
        m_IndexColorMap     = m_RootSignature.Find("ColorMap");
        m_IndexLinearClamp  = m_RootSignature.Find("LinearClamp");
    }

    // パイプラインステート.
    {
        D3D12_INPUT_ELEMENT_DESC elements[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature         = m_RootSignature.GetPtr();
        desc.VS                     = { TestVS, sizeof(TestVS) };
        desc.PS                     = { TestPS, sizeof(TestPS) };
        desc.BlendState             = asdx::PipelineState::GetBS(asdx::BLEND_STATE_OPAQUE);
        desc.RasterizerState        = asdx::PipelineState::GetRS(asdx::RASTERIZER_STATE_CULL_BACK);
        desc.DepthStencilState      = asdx::PipelineState::GetDSS(asdx::DEPTH_STATE_DEFAULT);
        desc.InputLayout            = { elements, 2 };
        desc.PrimitiveTopologyType  = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.SampleMask             = UINT_MAX;
        desc.NumRenderTargets       = 1;
        desc.RTVFormats[0]          = m_SwapChainFormat;
        desc.DSVFormat              = DXGI_FORMAT_D32_FLOAT;
        desc.SampleDesc.Count       = 1;
        desc.SampleDesc.Quality     = 0;

        if (!m_PSO.Init(pDevice, &desc))
        {
            ELOG("Error : PipelineState::Init() Failed.");
            return false;
        }
    }

    if (!m_CbMesh.Init(asdx::GfxDevice(), sizeof(CbMesh)))
    {
        ELOG("Error : CbMesh Initialize Failed");
        return false;
    }

    if (!m_CbScene.Init(asdx::GfxDevice(), sizeof(CbScene)))
    {
        ELOG("Error : CbScene Initialize Failed.");
        return false;
    }

    {
        asdx::ResTexture res;
        if (!res.LoadFromFileA("./test.tga"))
        {
            ELOG("Error : ResTexture::LoadFromFileA() Failed.");
            return false;
        }

        asdx::IUploadResource* pUpdateResource = nullptr;
        if (!m_Texture.Init(asdx::GfxDevice(), res, &pUpdateResource))
        {
            ELOG("Error : Texture::Init() Failed.");
            return false;
        }

        m_Uploader.Push(pUpdateResource);

        res.Release();
    }

    {
        if (!m_Sampler.Init(asdx::GfxDevice(), asdx::ST_LINEAR_CLAMP))
        {
            ELOG("Error : Sampler::Init() Failed.");
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void TestApp::OnTerm()
{
    m_TriangleVB    .Term();
    m_PSO           .Term();
    m_RootSignature .Term();
    m_CbMesh        .Term();
    m_CbScene       .Term();
    m_Texture       .Term();
    m_Sampler       .Term();
    m_Disposer      .Clear();
    m_Uploader      .Clear();
}

//-----------------------------------------------------------------------------
//      フレーム描画を行います.
//-----------------------------------------------------------------------------
void TestApp::OnFrameRender(asdx::FrameEventArgs& args)
{
    auto idx  = GetCurrentBackBufferIndex();
    auto pCmd = m_GfxCmdList.Reset();

    m_Uploader.Upload(pCmd);

    asdx::BarrierTransition(
        pCmd,
        m_ColorTarget[idx].GetResource(),
        0,
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);

    auto pRTV = m_ColorTarget[idx].GetRTV();
    auto pDSV = m_DepthTarget.GetDSV();

    asdx::ClearRTV(pCmd, pRTV, m_ClearColor);
    asdx::ClearDSV(pCmd, pDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0);

    asdx::SetRenderTarget(pCmd, pRTV, pDSV);
    asdx::SetViewport(pCmd, m_ColorTarget[idx].GetResource());

    // 描画処理.
    {
        auto vbv = m_TriangleVB.GetView();
        pCmd->SetGraphicsRootSignature(m_RootSignature.GetPtr());
        pCmd->SetPipelineState(m_PSO.GetPtr());
        //pCmd->SetGraphicsRootConstantBufferView(m_IndexCBMesh, m_CbMesh.GetResource()->GetGPUVirtualAddress());
        //pCmd->SetGraphicsRootConstantBufferView(m_IndexCBScene, m_CbScene.GetResource()->GetGPUVirtualAddress());
        pCmd->SetGraphicsRootDescriptorTable(m_IndexColorMap, m_Texture.GetDescriptor()->GetHandleGPU());
        pCmd->SetGraphicsRootDescriptorTable(m_IndexLinearClamp, m_Sampler.GetDescriptor()->GetHandleGPU());

        pCmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        pCmd->IASetVertexBuffers(0, 1, &vbv);
        pCmd->DrawInstanced(3, 1, 0, 0);
    }

    asdx::BarrierTransition(
        pCmd,
        m_ColorTarget[idx].GetResource(),
        0,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);

    pCmd->Close();

    ID3D12CommandList* pCmds[] = {
        pCmd
    };

    // 前フレームの描画の完了を待機.
    m_pGraphicsQueue->WaitIdle();

    // コマンドを実行.
    m_pGraphicsQueue->Execute(1, pCmds);

    // 画面に表示.
    Present(0);

    // フレーム同期.
    m_Disposer.FrameSync();
    m_Uploader.FrameSync();
}

//-----------------------------------------------------------------------------
//      リサイズ処理を行います.
//-----------------------------------------------------------------------------
void TestApp::OnResize(const asdx::ResizeEventArgs& args)
{
}

//-----------------------------------------------------------------------------
//      キー処理を行います.
//-----------------------------------------------------------------------------
void TestApp::OnKey(const asdx::KeyEventArgs& args)
{
}

//-----------------------------------------------------------------------------
//      マウス処理を行います.
//-----------------------------------------------------------------------------
void TestApp::OnMouse(const asdx::MouseEventArgs& args)
{
}

//-----------------------------------------------------------------------------
//      タイピング処理を行います.
//-----------------------------------------------------------------------------
void TestApp::OnTyping(uint32_t keyCode)
{
}

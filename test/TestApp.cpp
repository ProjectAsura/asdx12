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
#include <asdxQuad.h>
#include <d3d12.h>

#include "TestVS.inc"
#include "TestPS.inc"

#include "TestMS.inc"

#include "MeshletTestMS.inc"
#include "MeshletTestPS.inc"

//#define TEST_TRIANGLE     (1)
//#define TEST_MESH_SHADER  (1)
//#define TEST_MESHLET      (1)
#define TEST_QUAD         (1)

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
    m_CameraController.Init(asdx::Vector3(0.0f, 0.0f, 2.0f),
        asdx::Vector3(0.0f, 0.0f, 0.0f),
        asdx::Vector3(0.0f, 1.0f, 0.0f),
        0.1f,
        1000.0f);
    m_CameraController.Present();

#if TEST_TRIANGLE
    if (!TriangleTestInit())
    { return false; }
#endif

#if TEST_MESH_SHADER
    if (!MeshShaderTestInit())
    { return false; }
#endif

#if TEST_MESHLET
    if (!MeshletTestInit())
    { return false; }
#endif

#if TEST_QUAD
    if (!QuadTestInit())
    { return false; }
#endif

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void TestApp::OnTerm()
{
    m_TriangleIndexBuffer.Term();
    m_TriangleVertexBuffer.Term();

    m_Model         .Term();
    m_TriangleVB    .Term();
    m_PSO           .Term();
    m_RootSignature .Term();
    m_CbMesh        .Term();
    m_CbScene       .Term();
    m_Texture       .Term();
    m_Sampler       .Term();
    m_Disposer      .Clear();
}

//-----------------------------------------------------------------------------
//      フレーム描画を行います.
//-----------------------------------------------------------------------------
void TestApp::OnFrameRender(asdx::FrameEventArgs& args)
{
    auto idx  = GetCurrentBackBufferIndex();
    auto pCmd = m_GfxCmdList.Reset();

    asdx::GfxDevice().SetUploadCommand(pCmd);

#if TEST_TRIANGLE
    // 三角形描画.
    TriangleTestRender(pCmd, idx);
#endif

#if TEST_MESH_SHADER
    MeshShaderTestRender(pCmd, idx);
#endif

#if TEST_MESHLET
    // メッシュレット描画.
    MeshletTestRender(pCmd, idx);
#endif

#if TEST_QUAD
    // 矩形の描画
    QuadTestRender(pCmd, idx);
#endif

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
    asdx::GfxDevice().FrameSync();
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
    m_CameraController.OnMouse(
        args.X,
        args.Y,
        args.WheelDelta,
        args.IsLeftButtonDown,
        args.IsRightButtonDown,
        args.IsMiddleButtonDown,
        args.IsSideButton1Down,
        args.IsSideButton2Down);
}

//-----------------------------------------------------------------------------
//      タイピング処理を行います.
//-----------------------------------------------------------------------------
void TestApp::OnTyping(uint32_t keyCode)
{
}

//-----------------------------------------------------------------------------
//      三角形描画用の初期化処理です.
//-----------------------------------------------------------------------------
bool TestApp::TriangleTestInit()
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

        if (!m_TriangleVB.Init(pDevice, sizeof(vertices), sizeof(Vertex)))
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

    if (!m_CbMesh.Init(sizeof(CbMesh)))
    {
        ELOG("Error : CbMesh Initialize Failed");
        return false;
    }

    if (!m_CbScene.Init(sizeof(CbScene)))
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

        if (!m_Texture.Init(res))
        {
            ELOG("Error : Texture::Init() Failed.");
            return false;
        }
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
//      三角形の描画関数です.
//-----------------------------------------------------------------------------
void TestApp::TriangleTestRender(ID3D12GraphicsCommandList6* pCmd, uint8_t idx)
{
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
}


//-----------------------------------------------------------------------------
//      メッシュシェーダ描画用の初期化処理です.
//-----------------------------------------------------------------------------
bool TestApp::MeshShaderTestInit()
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

        if (!m_TriangleVertexBuffer.Init(3, sizeof(vertices[0]), vertices))
        {
            ELOG("Error : StructuredBuffer::Init() Failed.");
            return false;
        }
    }

    // 出力番号データ.
    {
        const uint32_t indices[] = { 0, 1, 2 };

        if (!m_TriangleIndexBuffer.Init(3, sizeof(indices[0]), indices))
        {
            ELOG("Error : StructuredBuffer::Init() Failed.");
            return false;
        }
    }

    // ルートシグニチャ.
    {
        uint32_t flag = 0;
        flag |= asdx::RSF_DENY_VS;
        flag |= asdx::RSF_DENY_GS;
        flag |= asdx::RSF_DENY_DS;
        flag |= asdx::RSF_DENY_HS;
        flag |= asdx::RSF_DENY_AS;

        asdx::RootSignatureDesc desc;
        desc.AddFromShader(TestMS, sizeof(TestMS))
            .AddFromShader(TestPS, sizeof(TestPS));
        desc.SetFlag(flag);

        if (!m_RootSignature.Init(pDevice, desc))
        {
            ELOG("Error : RootSignature::Init() Failed.");
            return false;
        }
    }

    // パイプラインステート.
    {
        asdx::GEOMETRY_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature             = m_RootSignature.GetPtr();
        desc.MS                         = { TestMS, sizeof(TestMS) };
        desc.PS                         = { TestPS, sizeof(TestPS) };
        desc.BlendState                 = asdx::PipelineState::GetBS(asdx::BLEND_STATE_OPAQUE);
        desc.RasterizerState            = asdx::PipelineState::GetRS(asdx::RASTERIZER_STATE_CULL_BACK);
        desc.DepthStencilState          = asdx::PipelineState::GetDSS(asdx::DEPTH_STATE_DEFAULT);
        desc.SampleMask                 = UINT_MAX;
        desc.RTVFormats.NumRenderTargets = 1;
        desc.RTVFormats.RTFormats[0]     = m_SwapChainFormat;
        desc.DSVFormat                  = DXGI_FORMAT_D32_FLOAT;
        desc.SampleDesc.Count           = 1;
        desc.SampleDesc.Quality         = 0;

        if (!m_PSO.Init(pDevice, &desc))
        {
            ELOG("Error : PipelineState::Init() Failed.");
            return false;
        }
    }

    if (!m_CbMesh.Init(sizeof(CbMesh)))
    {
        ELOG("Error : CbMesh Initialize Failed");
        return false;
    }

    if (!m_CbScene.Init(sizeof(CbScene)))
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

        if (!m_Texture.Init(res))
        {
            ELOG("Error : Texture::Init() Failed.");
            return false;
        }

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
//      三角形の描画関数です.
//-----------------------------------------------------------------------------
void TestApp::MeshShaderTestRender(ID3D12GraphicsCommandList6* pCmd, uint8_t idx)
{
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
        auto idxSRV0 = m_RootSignature.Find("Vertices");
        auto idxSRV1 = m_RootSignature.Find("Indices");
        auto idxSRV2 = m_RootSignature.Find("ColorMap");
        auto idxSmp0 = m_RootSignature.Find("LinearClamp");

        auto srv0 = m_TriangleVertexBuffer.GetDescriptor()->GetHandleGPU();
        auto srv1 = m_TriangleIndexBuffer.GetDescriptor()->GetHandleGPU();
        auto srv2 = m_Texture.GetDescriptor()->GetHandleGPU();
        auto smp0 = m_Sampler.GetDescriptor()->GetHandleGPU();

        pCmd->SetGraphicsRootSignature(m_RootSignature.GetPtr());
        pCmd->SetPipelineState(m_PSO.GetPtr());

        pCmd->SetGraphicsRootDescriptorTable(idxSRV0, srv0);
        pCmd->SetGraphicsRootDescriptorTable(idxSRV1, srv1);
        pCmd->SetGraphicsRootDescriptorTable(idxSRV2, srv2);
        pCmd->SetGraphicsRootDescriptorTable(idxSmp0, smp0);

        pCmd->DispatchMesh(1, 1, 1);
    }

    asdx::BarrierTransition(
        pCmd,
        m_ColorTarget[idx].GetResource(),
        0,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
}

//-----------------------------------------------------------------------------
//      メッシュレットの初期化処理です.
//-----------------------------------------------------------------------------
bool TestApp::MeshletTestInit()
{
    m_pGraphicsQueue = asdx::GfxDevice().GetGraphicsQueue();

    auto pDevice = asdx::GfxDevice().GetDevice();

    {
        asdx::ResModel resource;
        if (!asdx::LoadModel("teapot.mdl", resource))
        {
            ELOG("Error : LoadModel() Failed.");
            return false;
        }

        if (!m_Model.Init(resource))
        {
            ELOG("Error : Model::Init() Failed.");
            return false;
        }
    }

    // ルートシグニチャ.
    {
        uint32_t flag = 0;
        flag |= asdx::RSF_DENY_AS;
        flag |= asdx::RSF_DENY_VS;
        flag |= asdx::RSF_DENY_GS;
        flag |= asdx::RSF_DENY_DS;
        flag |= asdx::RSF_DENY_HS;

        asdx::RootSignatureDesc desc;
        desc.AddFromShader(MeshletTestMS, sizeof(MeshletTestMS))
            .AddFromShader(MeshletTestPS, sizeof(MeshletTestPS));
        desc.SetFlag(flag);

        if (!m_RootSignature.Init(pDevice, desc))
        {
            ELOG("Error : RootSignature::Init() Failed.");
            return false;
        }
    }

    // パイプラインステート.
    {
        asdx::GEOMETRY_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = m_RootSignature.GetPtr();
        desc.MS                             = { MeshletTestMS, sizeof(MeshletTestMS) };
        desc.PS                             = { MeshletTestPS, sizeof(MeshletTestPS) };
        desc.BlendState                     = asdx::PipelineState::GetBS(asdx::BLEND_STATE_OPAQUE);
        desc.SampleMask                     = UINT_MAX;
        desc.RasterizerState                = asdx::PipelineState::GetRS(asdx::RASTERIZER_STATE_CULL_NONE);
        desc.DepthStencilState              = asdx::PipelineState::GetDSS(asdx::DEPTH_STATE_DEFAULT);
        desc.RTVFormats.NumRenderTargets    = 1;
        desc.RTVFormats.RTFormats[0]        = m_SwapChainFormat;
        desc.DSVFormat                      = DXGI_FORMAT_D32_FLOAT;
        desc.SampleDesc.Count               = 1;
        desc.SampleDesc.Quality             = 0;

        if (!m_PSO.Init(pDevice, &desc))
        {
            ELOG("Error : PipelineState::Init() Failed.");
            return false;
        }
    }

    if (!m_CbMesh.Init(sizeof(CbMesh)))
    {
        ELOG("Error : CbMesh Initialize Failed");
        return false;
    }

    if (!m_CbScene.Init(sizeof(CbScene)))
    {
        ELOG("Error : CbScene Initialize Failed.");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      メッシュレットの描画処理です.
//-----------------------------------------------------------------------------
void TestApp::MeshletTestRender(ID3D12GraphicsCommandList6* pCmd, uint8_t idx)
{
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

    {
        CbMesh res = {};
        res.World = asdx::Matrix::CreateIdentity();
        m_CbMesh.Update(&res, sizeof(res));
    }

    {
        CbScene res = {};
        res.View = m_CameraController.GetView();

        auto aspect = float(m_Width) / float(m_Height);
        res.Proj = asdx::Matrix::CreatePerspectiveFieldOfView(
            asdx::F_PIDIV4,
            aspect,
            m_CameraController.GetNearClip(),
            m_CameraController.GetFarClip());

        m_CbScene.Update(&res, sizeof(res));
    }

    auto paramSRV0 = m_RootSignature.Find("Positions");
    auto paramSRV1 = m_RootSignature.Find("TangentSpaces");
    auto paramSRV2 = m_RootSignature.Find("TexCoords");
    auto paramSRV3 = m_RootSignature.Find("Indices");
    auto paramSRV4 = m_RootSignature.Find("Primitives");
    auto paramSRV5 = m_RootSignature.Find("Meshlets");
    auto paramCBV0 = m_RootSignature.Find("CbMesh");
    auto paramCBV1 = m_RootSignature.Find("CbScene");

    auto pCBV0 = m_CbMesh .GetResource()->GetGPUVirtualAddress();
    auto pCBV1 = m_CbScene.GetResource()->GetGPUVirtualAddress();

    pCmd->SetGraphicsRootSignature(m_RootSignature.GetPtr());
    pCmd->SetPipelineState(m_PSO.GetPtr());

    for(auto i=0u; i<m_Model.GetMeshCount(); ++i)
    {
        auto& mesh = m_Model.GetMesh(i);
        
        auto pSRV0 = mesh.GetPositions    ().GetDescriptor()->GetHandleGPU();
        auto pSRV1 = mesh.GetTangentSpaces().GetDescriptor()->GetHandleGPU();
        auto pSRV2 = mesh.GetTexCoords   (0).GetDescriptor()->GetHandleGPU();
        auto pSRV3 = mesh.GetInindices    ().GetDescriptor()->GetHandleGPU();
        auto pSRV4 = mesh.GetPrimitives   ().GetDescriptor()->GetHandleGPU();
        auto pSRV5 = mesh.GetMeshlets     ().GetDescriptor()->GetHandleGPU();

        pCmd->SetGraphicsRootDescriptorTable(paramSRV0, pSRV0);
        pCmd->SetGraphicsRootDescriptorTable(paramSRV1, pSRV1);
        pCmd->SetGraphicsRootDescriptorTable(paramSRV2, pSRV2);
        pCmd->SetGraphicsRootDescriptorTable(paramSRV3, pSRV3);
        pCmd->SetGraphicsRootDescriptorTable(paramSRV4, pSRV4);
        pCmd->SetGraphicsRootDescriptorTable(paramSRV5, pSRV5);
        pCmd->SetGraphicsRootConstantBufferView(paramCBV0, pCBV0);
        pCmd->SetGraphicsRootConstantBufferView(paramCBV1, pCBV1);

        pCmd->DispatchMesh(mesh.GetMeshletCount(), 1, 1);
    }

    asdx::BarrierTransition(
        pCmd,
        m_ColorTarget[idx].GetResource(),
        0,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
}



//-----------------------------------------------------------------------------
//      フルスクリーン三角形描画用の初期化処理です.
//-----------------------------------------------------------------------------
bool TestApp::QuadTestInit()
{
    m_pGraphicsQueue = asdx::GfxDevice().GetGraphicsQueue();

    auto pDevice = asdx::GfxDevice().GetDevice();

    if (!asdx::Quad::Instance().Init(asdx::GfxDevice().GetDevice()))
    {
        ELOG("Error : Quad::Init() Failed");
        return false;
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
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature         = m_RootSignature.GetPtr();
        desc.VS                     = { TestVS, sizeof(TestVS) };
        desc.PS                     = { TestPS, sizeof(TestPS) };
        desc.BlendState             = asdx::PipelineState::GetBS(asdx::BLEND_STATE_OPAQUE);
        desc.RasterizerState        = asdx::PipelineState::GetRS(asdx::RASTERIZER_STATE_CULL_BACK);
        desc.DepthStencilState      = asdx::PipelineState::GetDSS(asdx::DEPTH_STATE_DEFAULT);
        desc.InputLayout            = asdx::Quad::InputLayout;
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

    {
        asdx::ResTexture res;
        if (!res.LoadFromFileA("./test.tga"))
        {
            ELOG("Error : ResTexture::LoadFromFileA() Failed.");
            return false;
        }

        if (!m_Texture.Init(res))
        {
            ELOG("Error : Texture::Init() Failed.");
            return false;
        }
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
//      フルスクリーン三角形の描画関数です.
//-----------------------------------------------------------------------------
void TestApp::QuadTestRender(ID3D12GraphicsCommandList6* pCmd, uint8_t idx)
{
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
        pCmd->SetGraphicsRootSignature(m_RootSignature.GetPtr());
        pCmd->SetPipelineState(m_PSO.GetPtr());
        pCmd->SetGraphicsRootDescriptorTable(m_IndexColorMap, m_Texture.GetDescriptor()->GetHandleGPU());
        pCmd->SetGraphicsRootDescriptorTable(m_IndexLinearClamp, m_Sampler.GetDescriptor()->GetHandleGPU());

        asdx::Quad::Instance().Draw(pCmd);
    }

    asdx::BarrierTransition(
        pCmd,
        m_ColorTarget[idx].GetResource(),
        0,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
}

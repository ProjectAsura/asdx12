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

} // namespace

///////////////////////////////////////////////////////////////////////////////
// TestApp class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      �R���X�g���N�^�ł�.
//-----------------------------------------------------------------------------
TestApp::TestApp()
: asdx::Application(L"Test App", 960, 540, nullptr, nullptr, nullptr)
{
}

//-----------------------------------------------------------------------------
//      �f�X�g���N�^�ł�.
//-----------------------------------------------------------------------------
TestApp::~TestApp()
{
}

//-----------------------------------------------------------------------------
//      �������������s���܂�.
//-----------------------------------------------------------------------------
bool TestApp::OnInit()
{
    m_pGraphicsQueue = asdx::GfxDevice().GetGraphicsQueue();

    auto pDevice = asdx::GfxDevice().GetDevice();

    // ���_�f�[�^
    {
        const Vertex vertices[] = {
            { asdx::Vector3( 0.0f, 1.0f, 0.0f), asdx::Vector2(0.5f, 1.0f) },
            { asdx::Vector3( 1.0f, 0.0f, 0.0f), asdx::Vector2(1.0f, 0.0f) },
            { asdx::Vector3(-1.0f, 0.0f, 0.0f), asdx::Vector2(0.0f, 0.0f) },
        };

        if (!m_TriangleVB.Init(pDevice, sizeof(vertices), sizeof(Vertex)))
        {
            ELOG("Error : VertexBuffer::Init() Failed.");
            return false;
        }
    }

    // �p�C�v���C���X�e�[�g.
    {
        D3D12_INPUT_ELEMENT_DESC elements[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.VS                     = { TestVS, sizeof(TestVS) };
        desc.PS                     = { TestPS, sizeof(TestPS) };
        desc.BlendState             = asdx::PipelineState::GetBS(asdx::BLEND_STATE_OPAQUE);
        desc.RasterizerState        = asdx::PipelineState::GetRS(asdx::RASTERIZER_STATE_CULL_NONE);
        desc.DepthStencilState      = asdx::PipelineState::GetDSS(asdx::DEPTH_STATE_DEFAULT);
        desc.InputLayout            = { elements, 2 };
        desc.PrimitiveTopologyType  = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
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

    return true;
}

//-----------------------------------------------------------------------------
//      �I���������s���܂�.
//-----------------------------------------------------------------------------
void TestApp::OnTerm()
{
    m_TriangleVB.Term();
    m_PSO.Term();
}

//-----------------------------------------------------------------------------
//      �t���[���`����s���܂�.
//-----------------------------------------------------------------------------
void TestApp::OnFrameRender(asdx::FrameEventArgs& args)
{
    auto idx  = GetCurrentBackBufferIndex();
    auto pCmd = m_GfxCmdList.Reset();

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

    // �`�揈��.
    {
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

    // �O�t���[���̕`��̊�����ҋ@.
    m_pGraphicsQueue->WaitIdle();

    // �R�}���h�����s.
    m_pGraphicsQueue->Execute(1, pCmds);

    // ��ʂɕ\��.
    Present(0);
}

//-----------------------------------------------------------------------------
//      ���T�C�Y�������s���܂�.
//-----------------------------------------------------------------------------
void TestApp::OnResize(const asdx::ResizeEventArgs& args)
{
}

//-----------------------------------------------------------------------------
//      �L�[�������s���܂�.
//-----------------------------------------------------------------------------
void TestApp::OnKey(const asdx::KeyEventArgs& args)
{
}

//-----------------------------------------------------------------------------
//      �}�E�X�������s���܂�.
//-----------------------------------------------------------------------------
void TestApp::OnMouse(const asdx::MouseEventArgs& args)
{
}

//-----------------------------------------------------------------------------
//      �^�C�s���O�������s���܂�.
//-----------------------------------------------------------------------------
void TestApp::OnTyping(uint32_t keyCode)
{
}

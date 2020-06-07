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


namespace {



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

    return true;
}

//-----------------------------------------------------------------------------
//      �I���������s���܂�.
//-----------------------------------------------------------------------------
void TestApp::OnTerm()
{
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

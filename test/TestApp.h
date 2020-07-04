//-----------------------------------------------------------------------------
// File : TestApp.h
// Desc : Test Application.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxApp.h>
#include <asdxConstantBuffer.h>
#include <asdxVertexBuffer.h>
#include <asdxPipelineState.h>
#include <asdxRootSignature.h>
#include <asdxResourceDisposer.h>


///////////////////////////////////////////////////////////////////////////////
// TestApp class
///////////////////////////////////////////////////////////////////////////////
class TestApp : public asdx::Application
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================
    TestApp();
    ~TestApp();

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    asdx::Queue*            m_pGraphicsQueue = nullptr;
    asdx::VertexBuffer      m_TriangleVB;
    asdx::RootSignature     m_RootSignature;
    asdx::PipelineState     m_PSO;
    asdx::ResourceDisposer  m_Disposer;
    asdx::ConstantBuffer    m_CbMesh;
    asdx::ConstantBuffer    m_CbScene;

    uint32_t                m_IndexCBMesh;
    uint32_t                m_IndexCBScene;
    uint32_t                m_IndexColorMap;
    uint32_t                m_IndexLinearClamp;

    //=========================================================================
    // private methods.
    //=========================================================================
    bool OnInit() override;
    void OnTerm() override;
    void OnFrameRender(asdx::FrameEventArgs& args) override;
    void OnResize(const asdx::ResizeEventArgs& args) override;
    void OnKey(const asdx::KeyEventArgs& args) override;
    void OnMouse(const asdx::MouseEventArgs& args) override;
    void OnTyping(uint32_t keyCode) override;
};

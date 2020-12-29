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
#include <asdxDisposer.h>
#include <asdxResourceUploader.h>
#include <asdxTexture.h>
#include <asdxSampler.h>
#include <asdxModel.h>
#include <asdxCameraController.h>
#include <asdxPassGraph.h>
#include <renderer/asdxColorFilter.h>


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
    asdx::CommandQueue*     m_pGraphicsQueue = nullptr;
    asdx::CommandQueue*     m_pComputeQueue = nullptr;
    asdx::VertexBuffer      m_TriangleVB;
    asdx::RootSignature     m_RootSignature;
    asdx::PipelineState     m_PSO;
    asdx::ConstantBuffer    m_CbMesh;
    asdx::ConstantBuffer    m_CbScene;
    asdx::Texture           m_Texture;
    asdx::Sampler           m_Sampler;
    asdx::Model             m_Model;

    uint32_t                m_IndexCBMesh;
    uint32_t                m_IndexCBScene;
    uint32_t                m_IndexColorMap;
    uint32_t                m_IndexLinearClamp;
    asdx::StructuredBuffer  m_TriangleVertexBuffer;
    asdx::StructuredBuffer  m_TriangleIndexBuffer;
    asdx::CameraController  m_CameraController;
    asdx::WaitPoint         m_WaitPoint;

    asdx::IPassGraph*       m_PassGraph;
    asdx::ColorFilter       m_ColorFilter;

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

    bool TriangleTestInit();
    void TriangleTestRender(ID3D12GraphicsCommandList6* pCmd, uint8_t idx);

    bool MeshShaderTestInit();
    void MeshShaderTestRender(ID3D12GraphicsCommandList6* pCmd, uint8_t idx);

    bool MeshletTestInit();
    void MeshletTestRender(ID3D12GraphicsCommandList6* pCmd, uint8_t idx);

    bool QuadTestInit();
    void QuadTestRender(ID3D12GraphicsCommandList6* pCmd, uint8_t idex);

    bool PassGraphTestInit();
    void PassGraphTestTerm();
    void PassGraphTestRender(ID3D12GraphicsCommandList6* pCmd, uint8_t index);
};

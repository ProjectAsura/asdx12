//-----------------------------------------------------------------------------
// File : ModelViewer.h
// Desc : Model Viewer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fw/asdxApp.h>
#include <fw/asdxAppCamera.h>
#include <gfx/asdxCommandQueue.h>
#include <gfx/asdxModel.h>
#include <gfx/asdxPipelineState.h>
#include <gfx/asdxBuffer.h>
#include <gfx/asdxShape.h>
#include <gfx/asdxMotionPlayer.h>
#include <gfx/asdxLine.h>
#include <imgui.h>
#include <ImGuizmo.h>


///////////////////////////////////////////////////////////////////////////////
// ModelViewer class
///////////////////////////////////////////////////////////////////////////////
class ModelViewer : public asdx::App
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================

public:
    //=========================================================================
    // public variables.
    //=========================================================================


    //=========================================================================
    // public mehtods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    ModelViewer();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~ModelViewer();

private:
    struct ModelInfo
    {
        size_t  MeshCount;
        size_t  MaterialCount;
        size_t  VertexCount;
        size_t  IndexCount;
        size_t  BoneCount;
    };

    //=========================================================================
    // private variables.
    //=========================================================================
    asdx::WaitPoint                     m_FrameWaitPoint;
    std::vector<uint8_t>                m_ModelBinary;
    asdx::RefPtr<asdx::Model>           m_Model;
    std::string                         m_OutputPath;
    asdx::RefPtr<ID3D12RootSignature>   m_RootSignature;
    asdx::GraphicsPipelineState         m_StaticSolidState;
    asdx::GraphicsPipelineState         m_SkeletalSolidState;
    asdx::GraphicsPipelineState         m_StaticWireframeState;
    asdx::GraphicsPipelineState         m_SkeletalWireframeState;
    ModelInfo                           m_ModelInfo = {};
    asdx::ConstantBuffer                m_SceneCB[2];
    asdx::AppCamera                     m_Camera;
    uint32_t                            m_DrawMode           = 0;
    bool                                m_DrawBoundingSphere = false;
    bool                                m_DrawBones          = false;
    bool                                m_EnableWireframe    = false;
    asdx::Vector3                       m_ModelTranslation;
    asdx::Vector3                       m_ModelRotation;
    asdx::Vector3                       m_ModelScale;
    bool                                m_EnableGuizmo = false;
    ImGuizmo::OPERATION                 m_GuizmoOperation;
    asdx::Matrix                        m_Proj;
    asdx::BoneShape                     m_BoneShape;
    asdx::SphereShape                   m_SphereShape;
    asdx::ShapeStates                   m_ShapeStates;
    asdx::ShapeParams                   m_ShapeParams;
    asdx::MotionPlayer                  m_MotionPlayer;
    asdx::MotionBinary                  m_MotionBinary;
    asdx::LineRenderer                  m_LineRenderer;
    size_t                              m_ClipIndex     = 0;
    std::vector<std::string>            m_ClipNames     = {};
    asdx::StructuredBuffer              m_MatrixPalletBuffer[2];

    //=========================================================================
    // private methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //-------------------------------------------------------------------------
    bool OnInit() override;

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void OnTerm() override;

    //-------------------------------------------------------------------------
    //! @brief      フレーム遷移処理です.
    //-------------------------------------------------------------------------
    void OnFrameMove(const asdx::App::FrameEventArgs& args) override;

    //-------------------------------------------------------------------------
    //! @brief      フレーム描画処理です.
    //-------------------------------------------------------------------------
    void OnFrameRender(const asdx::App::FrameEventArgs& args) override;

    //-------------------------------------------------------------------------
    //! @brief      リサイズ処理です.
    //-------------------------------------------------------------------------
    void OnResize(const asdx::App::ResizeEventArgs& args) override;

    //-------------------------------------------------------------------------
    //! @brief      キー処理です.
    //-------------------------------------------------------------------------
    void OnKey(const asdx::App::KeyEventArgs& args) override;

    //-------------------------------------------------------------------------
    //! @brief      マウス処理です.
    //-------------------------------------------------------------------------
    void OnMouse(const asdx::App::MouseEventArgs& args) override;

    //-------------------------------------------------------------------------
    //! @brief      タイピング処理です.
    //-------------------------------------------------------------------------
    void OnTyping(uint32_t keyCode) override;

    //-------------------------------------------------------------------------
    //! @brief      ファイルドロップ処理です.
    //-------------------------------------------------------------------------
    void OnDrop(const wchar_t** dropFiles, uint32_t fileCount) override;

    //-------------------------------------------------------------------------
    //! @brief      ファイルメニュー処理です.
    //-------------------------------------------------------------------------
    void MenuFile(ID3D12GraphicsCommandList* pCmd);

    //-------------------------------------------------------------------------
    //! @brief      表示メニュー処理です.
    //-------------------------------------------------------------------------
    void MenuView();

    //-------------------------------------------------------------------------
    //! @brief      ヘルプメニュー処理です.
    //-------------------------------------------------------------------------
    void MenuHelp();

    //-------------------------------------------------------------------------
    //! @brief      モデルを再生成します.
    //-------------------------------------------------------------------------
    void RecreateModel();

    //-------------------------------------------------------------------------
    //! @brief      モデルバイナリを保存します.
    //-------------------------------------------------------------------------
    void SaveModelBinary(const char* path);

    //-------------------------------------------------------------------------
    //! @brief      モデルを読み込みます.
    //-------------------------------------------------------------------------
    void LoadModel();

    //-------------------------------------------------------------------------
    //! @brief      モーションを読み込みます.
    //-------------------------------------------------------------------------
    void LoadMotion();
};

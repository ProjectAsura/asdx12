//-----------------------------------------------------------------------------
// File : LevelEditor.cpp
// Desc : Level Editor.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <LevelEditor.h>
#include <fnd/asdxMacro.h>
#include <fnd/asdxLogger.h>
#include <edit/asdxGuiMgr.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxCommandQueue.h>
#include <gfx/asdxTextureManager.h>
#include <imgui.h>
#include <ImGuizmo.h>

///////////////////////////////////////////////////////////////////////////////
// LevelEditor class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
LevelEditor::LevelEditor()
: base::App(L"LevelEditor", 1920, 1080, nullptr, nullptr, nullptr)
{
    m_SwapChainFormat    = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    m_DepthStencilFormat = DXGI_FORMAT_D32_FLOAT;

    m_ClearColor[0] = 0.1f;
    m_ClearColor[1] = 0.1f;
    m_ClearColor[2] = 0.1f;
    m_ClearColor[3] = 1.0f;

    m_DeviceDesc.MaxShaderResourceCount = 8192;
    m_DeviceDesc.MaxSamplerCount        = 128;
    m_DeviceDesc.MaxColorTargetCount    = 256;
    m_DeviceDesc.MaxDepthTargetCount    = 256;

#if ASDX_DEBUG
    m_DeviceDesc.EnableDebug   = true;
    m_DeviceDesc.EnableCapture = true;
#endif
}

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
LevelEditor::~LevelEditor()
{
}

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool LevelEditor::OnInit()
{
    auto pDevice = asdx::GetD3D12Device();

    m_GfxCmdList.Reset();
    auto pCmd = m_GfxCmdList.GetD3D12CommandList();

    // GUIマネージャの初期化処理.
    {
        if (!asdx::GuiMgr::Instance().Init(pCmd, m_hWnd, m_Width, m_Height, m_SwapChainFormat, nullptr))
        {
            ELOG("Error : GuiMgr::Init() Failed.");
            return false;
        }
    }

    if (!m_ShapeStates.Init(m_SwapChainFormat, m_DepthStencilFormat))
    {
        ELOG("Error : ShapeStates Init Failed.");
        return false;
    }

    m_Camera.Init(
        asdx::Vector3(0.0f, 0.0f, 5.0f),
        asdx::Vector3(0.0f, 0.0f, 0.0f),
        asdx::Vector3(0.0f, 1.0f, 0.0f),
        0.1f,
        10000.0f);

    // ラインレンダラーの初期化.
    if (!m_LineRenderer.Init(100 * UINT16_MAX, m_SwapChainFormat, DXGI_FORMAT_UNKNOWN))
    {
        ELOG("Error : LineRenderer::Init() Failed.");
        return false;
    }

    // コマンドの記録を終了.
    pCmd->Close();

    // セットアップコマンド実行.
    {
        // グラフィックスキューを取得.
        auto pGraphicsQueue = asdx::GetGraphicsQueue();

        // コマンドを実行.
        if (asdx::TextureManager::Instance().HasCommand())
        {
            auto pTextureCmd = asdx::TextureManager::Instance().Swap();

            ID3D12CommandList* pCmds[] = { pCmd, pTextureCmd };
            pGraphicsQueue->Execute(_countof(pCmds), pCmds);
        }
        else
        {
            ID3D12CommandList* pCmds[] = { pCmd };
            pGraphicsQueue->Execute(_countof(pCmds), pCmds);
        }

        // 待機点を発行.
        m_FrameWaitPoint = pGraphicsQueue->Signal();

        // 完了を待機.
        pGraphicsQueue->Sync(m_FrameWaitPoint);
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void LevelEditor::OnTerm()
{
    m_SceneCB.Term();

    m_AxisVertexBuffer.Term();

    m_LineRenderer.Term();

    m_RootSignature.Reset();

    m_PipelineState.Term();

    m_ShapeStates.Term();

    // GUIマネージャの終了処理.
    asdx::GuiMgr::Instance().Term();
}

//-----------------------------------------------------------------------------
//      フレーム遷移処理です.
//-----------------------------------------------------------------------------
void LevelEditor::OnFrameMove(const base::FrameEventArgs& args)
{
    auto pCmd = m_GfxCmdList.Reset();

    m_LineRenderer.Reset();

    // ImGui関連.
    asdx::GuiMgr::Instance().Update(m_Width, m_Height);

    ImGuizmo::BeginFrame();
    ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());

    // レベル情報を描画.
    DrawLevelInfo();

    // コンテキストメニューを描画.
    DrawContextMenu();

    // プロパティウィンドウを描画.
    DrawPropertyWindow();

    auto aspect = float(m_Width) / float(m_Height);
    auto view = m_Camera.GetView();
    m_Proj = asdx::Matrix::CreatePerspectiveFieldOfView(asdx::ToRadian(37.5f), aspect, 0.1f, 1000.0f);
    m_ShapeStates.SetViewProj(view, m_Proj);

    // グリッド描画.
    if (m_DrawGrid)
    {
        if (m_GridMode == 0)
            asdx::DrawSquareGrid(m_LineRenderer, m_GridHalfRange, m_GridSize, m_GridColor);
        else
            asdx::DrawHexGrid(m_LineRenderer, m_GridHalfRange, m_GridSize, m_GridColor);
    }
}

//-----------------------------------------------------------------------------
//      フレーム描画処理です.
//-----------------------------------------------------------------------------
void LevelEditor::OnFrameRender(const base::FrameEventArgs& args)
{
    auto idx = GetCurrentBackBufferIndex();

    auto pCmd = m_GfxCmdList.GetD3D12CommandList();

    // 書き込み状態に遷移.
    m_ColorTarget[idx].ChangeState(pCmd, D3D12_RESOURCE_STATE_RENDER_TARGET);

    // レンダーターゲット設定.
    auto handleRTV = m_ColorTarget[idx].GetCpuHandleRTV();
    auto handleDSV = m_DepthTarget.GetCpuHandleDSV();
    pCmd->OMSetRenderTargets(1, &handleRTV, FALSE, &handleDSV);
    pCmd->ClearRenderTargetView(handleRTV, m_ClearColor, 0, nullptr);
    pCmd->ClearDepthStencilView(handleDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    pCmd->RSSetViewports(1, &m_Viewport);
    pCmd->RSSetScissorRects(1, &m_ScissorRect);

    // 描画処理.
    {

        auto addr = m_ShapeStates.GetBufferAddress();
        m_LineRenderer.SetPipelineState(pCmd);
        pCmd->SetGraphicsRootConstantBufferView(0, addr);
        m_LineRenderer.Draw(pCmd);
    }

    // GUIを描画.
    asdx::GuiMgr::Instance().Draw(pCmd);

    // 表示状態に遷移.
    m_ColorTarget[idx].ChangeState(pCmd, D3D12_RESOURCE_STATE_PRESENT);

    // コマンド記録終了.
    pCmd->Close();

    // コマンドを実行.
    {
        auto pGraphicsQueue = asdx::GetGraphicsQueue();

        // 前フレームの描画の完了を待機.
        if (m_FrameWaitPoint.IsValid())
        { pGraphicsQueue->Sync(m_FrameWaitPoint); }

        // コマンドを実行.
        if (asdx::TextureManager::Instance().HasCommand())
        {
            auto pTextureCmd = asdx::TextureManager::Instance().Swap();

            ID3D12CommandList* pCmds[] = { pCmd, pTextureCmd };
            pGraphicsQueue->Execute(_countof(pCmds), pCmds);
        }
        else
        {
            ID3D12CommandList* pCmds[] = { pCmd };
            pGraphicsQueue->Execute(_countof(pCmds), pCmds);
        }

        // 待機点を発行.
        m_FrameWaitPoint = pGraphicsQueue->Signal();
    }

    Present(1);

    asdx::FrameSync();
}

//-----------------------------------------------------------------------------
//      リサイズ処理です.
//-----------------------------------------------------------------------------
void LevelEditor::OnResize(const base::ResizeEventArgs& args)
{

}

//-----------------------------------------------------------------------------
//      キー処理です.
//-----------------------------------------------------------------------------
void LevelEditor::OnKey(const base::KeyEventArgs& args)
{
    asdx::GuiMgr::Instance().OnKey(args.KeyCode, args.IsKeyDown, args.IsAltDown);

    m_Camera.OnKey(args.KeyCode, args.IsKeyDown, args.IsAltDown);

    // 文字入力を誤検知しないようにチェック.
    auto itemActive = ImGui::IsAnyItemFocused() || ImGui::IsAnyItemActive();

    if (args.IsKeyDown)
    {
        switch(args.KeyCode)
        {
        // 移動ツール.
        case 'W':
            {
                if (!itemActive)
                {
                    if (m_EnableGuizmo && m_GuizmoOperation == ImGuizmo::OPERATION::TRANSLATE)
                    { m_EnableGuizmo = false; }
                    else
                    {
                        m_EnableGuizmo    = true;
                        m_GuizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
                    }
                }
            }
            break;

        // 回転ツール.
        case 'E':
            {
                if (!itemActive)
                {
                    if (m_EnableGuizmo && m_GuizmoOperation == ImGuizmo::OPERATION::ROTATE)
                    { m_EnableGuizmo = false; }
                    else
                    {
                        m_EnableGuizmo    = true;
                        m_GuizmoOperation = ImGuizmo::OPERATION::ROTATE;
                    }
                }
            }
            break;

        // スケールツール.
        case 'R':
            {
                if (!itemActive)
                {
                    if (m_EnableGuizmo && m_GuizmoOperation == ImGuizmo::OPERATION::SCALE)
                    { m_EnableGuizmo = false; }
                    else
                    {
                        m_EnableGuizmo    = true;
                        m_GuizmoOperation = ImGuizmo::OPERATION::SCALE;
                    }
                }
            }
            break;

        // ワイヤーフレーム.
        case '4':
            {
                m_EnableWireframe = true;
            }
            break;

        // シェーディング
        case '5':
            {
                m_EnableWireframe = false;
            }
            break;

        default:
            break;
        }
    }

}

//-----------------------------------------------------------------------------
//      マウス処理です.
//-----------------------------------------------------------------------------
void LevelEditor::OnMouse(const base::MouseEventArgs& args)
{
    if (args.IsAltDown)
    {
        m_Camera.OnMouse(
            args.X,
            args.Y,
            args.WheelDelta,
            args.IsDownL,
            args.IsDownR,
            args.IsDownM,
            args.IsDownX1,
            args.IsDownX2);
    }
    else
    {
        asdx::GuiMgr::Instance().OnMouse(
            args.X, args.Y, args.WheelDelta, args.IsDownL, args.IsDownM, args.IsDownR);
    }
}

//-----------------------------------------------------------------------------
//      タイピング処理です.
//-----------------------------------------------------------------------------
void LevelEditor::OnTyping(uint32_t keyCode)
{
    asdx::GuiMgr::Instance().OnTyping(keyCode);
}

//-----------------------------------------------------------------------------
//      ファイルドラッグアンドドロップ時の処理です.
//-----------------------------------------------------------------------------
void LevelEditor::OnDrop(const wchar_t** dropFiles, uint32_t fileCount)
{

}


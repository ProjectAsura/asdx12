//-----------------------------------------------------------------------------
// File : ModelViewer.cpp
// Desc : Model Viewer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <ModelViewer.h>
#include <fnd/asdxMacro.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxMisc.h>
#include <fnd/asdxPath.h>
#include <fnd/asdxMisc.h>
#include <fnd/asdxFileIO.h>
#include <edit/asdxGuiMgr.h>
#include <imgui.h>
#include <im3d.h>
#include "ModelConverter.h"


///////////////////////////////////////////////////////////////////////////////
// ModelViewer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ModelViewer::ModelViewer()
: asdx::App(L"ModelViewer", 1920, 1080, nullptr, nullptr, nullptr)
{
    m_SwapChainFormat    = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    m_DepthStencilFormat = DXGI_FORMAT_D32_FLOAT;

    m_ClearColor[0] = 0.2f;
    m_ClearColor[1] = 0.2f;
    m_ClearColor[2] = 0.2f;
    m_ClearColor[3] = 1.0f;

    m_DeviceDesc.MaxShaderResourceCount = 8192;
    m_DeviceDesc.MaxSamplerCount        = 128;
    m_DeviceDesc.MaxColorTargetCount    = 256;
    m_DeviceDesc.MaxDepthTargetCount    = 256;

#if ASDX_DEBUG
    m_DeviceDesc.EnableDebug = true;
#endif
}

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ModelViewer::~ModelViewer()
{
}

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool ModelViewer::OnInit()
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

    // モデルマネージャの初期化.
    if (!asdx::InitModelManager(32))
    {
        ELOGA("Error : InitModelManager() Failed.");
        return false;
    }

    // コマンドの記録を終了.
    pCmd->Close();

    // セットアップコマンド実行.
    {
        ID3D12CommandList* pCmds[] = { pCmd };

        // グラフィックスキューを取得.
        auto pGraphicsQueue = asdx::GetGraphicsQueue();

        // コマンドを実行.
        pGraphicsQueue->Execute(_countof(pCmds), pCmds);

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
void ModelViewer::OnTerm()
{
    // モデルマネージャの終了処理.
    asdx::TermModelManager();

    // GUIマネージャの終了処理.
    asdx::GuiMgr::Instance().Term();
}

//-----------------------------------------------------------------------------
//      フレーム遷移処理です.
//-----------------------------------------------------------------------------
void ModelViewer::OnFrameMove(const asdx::App::FrameEventArgs& args)
{
    auto pCmd = m_GfxCmdList.Reset();

    asdx::GuiMgr::Instance().Update(m_Width, m_Height);

    // 情報表示.
    {
        const auto w = 200.0f;
        const auto h = 100.0f;
        const auto x = 10.0f;
        const auto y = 10.0f;

        ImGui::SetNextWindowPos(ImVec2(m_Width - (w + x), m_Height - (h + y)));
        ImGui::SetNextWindowSize(ImVec2(w, h));

        auto flags = ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoTitleBar;
        if (ImGui::Begin("Info", nullptr, flags))
        {
            ImGui::Text("FPS : %.2f", GetFPS());
            ImGui::Separator();

            uint32_t meshCount     = 0;
            uint32_t materialCount = 0;
            uint32_t vertexCount   = 0;
            uint32_t indexCount    = 0;

            if (!m_ModelBinary.empty() && m_Model.GetPtr() != nullptr)
            {
                meshCount     = uint32_t(m_Model->GetMeshCount());
                materialCount = uint32_t(m_Model->GetMaterialCount());

                for(size_t i=0; i<m_Model->GetMeshCount(); ++i)
                {
                    vertexCount += m_Model->GetMesh(i)->GetVertexCount();
                    indexCount  += m_Model->GetMesh(i)->GetIndexCount();
                }
            }

            ImGui::Text((const char*)u8"メッシュ数     : %u",  meshCount);
            ImGui::Text((const char*)u8"マテリアル数   : %u",  materialCount);
            ImGui::Text((const char*)u8"頂点数         : %u", vertexCount);
            ImGui::Text((const char*)u8"インデックス数 : %u",  indexCount);

            ImGui::End();
        }
    }

    // コンテキストメニュー.
    {
        if (ImGui::IsMouseClicked(1))
        { ImGui::OpenPopup("ContextMenu"); }

        if (ImGui::BeginPopup("ContextMenu"))
        {
            if (ImGui::BeginMenu((const char*)u8"ファイル"))
            {
                MenuFile(pCmd);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu((const char*)u8"ヘルプ"))
            {
                MenuHelp();
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }
    }
}

//-----------------------------------------------------------------------------
//      フレーム描画処理です.
//-----------------------------------------------------------------------------
void ModelViewer::OnFrameRender(const asdx::App::FrameEventArgs& args)
{
    auto idx = GetCurrentBackBufferIndex();

    auto pCmd = m_GfxCmdList.GetD3D12CommandList();

    // 書き込み状態に遷移.
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource    = m_ColorTarget[idx].GetResource();
        barrier.Transition.StateBefore  = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter   = D3D12_RESOURCE_STATE_RENDER_TARGET;
        pCmd->ResourceBarrier(1, &barrier);
    }

    // レンダーターゲット設定.
    auto handleRTV = m_ColorTarget[idx].GetCpuHandleRTV();
    auto handleDSV = m_DepthTarget.GetCpuHandleDSV();
    pCmd->OMSetRenderTargets(1, &handleRTV, FALSE, &handleDSV);
    pCmd->ClearRenderTargetView(handleRTV, m_ClearColor, 0, nullptr);
    pCmd->ClearDepthStencilView(handleDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    pCmd->RSSetViewports(1, &m_Viewport);
    pCmd->RSSetScissorRects(1, &m_ScissorRect);

    // モデルを描画
    {
    }

    // GUIを描画.
    asdx::GuiMgr::Instance().Draw(pCmd);

    // 表示状態に遷移.
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource    = m_ColorTarget[idx].GetResource();
        barrier.Transition.StateBefore  = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter   = D3D12_RESOURCE_STATE_PRESENT;
        pCmd->ResourceBarrier(1, &barrier);
    }

    // コマンド記録終了.
    pCmd->Close();

    // コマンドを実行.
    {
        ID3D12CommandList* pCmds[] = { pCmd };

        auto pGraphicsQueue = asdx::GetGraphicsQueue();

        // 前フレームの描画の完了を待機.
        if (m_FrameWaitPoint.IsValid())
        { pGraphicsQueue->Sync(m_FrameWaitPoint); }

        // コマンドを実行.
        pGraphicsQueue->Execute(_countof(pCmds), pCmds);

        // 待機点を発行.
        m_FrameWaitPoint = pGraphicsQueue->Signal();
    }

    Present(1);
}

//-----------------------------------------------------------------------------
//      リサイズ処理です.
//-----------------------------------------------------------------------------
void ModelViewer::OnResize(const asdx::App::ResizeEventArgs& args)
{
}

//-----------------------------------------------------------------------------
//      キー処理です.
//-----------------------------------------------------------------------------
void ModelViewer::OnKey(const asdx::App::KeyEventArgs& args)
{
    asdx::GuiMgr::Instance().OnKey(args.KeyCode, args.IsKeyDown, args.IsAltDown);
}

//-----------------------------------------------------------------------------
//      マウス処理です.
//-----------------------------------------------------------------------------
void ModelViewer::OnMouse(const asdx::App::MouseEventArgs& args)
{
    asdx::GuiMgr::Instance().OnMouse(
        args.X, args.Y, args.WheelDelta, args.IsDownL, args.IsDownM, args.IsDownR);
}

//-----------------------------------------------------------------------------
//      タイピング処理です.
//-----------------------------------------------------------------------------
void ModelViewer::OnTyping(uint32_t keyCode)
{
    asdx::GuiMgr::Instance().OnTyping(keyCode);
}

//-----------------------------------------------------------------------------
//      ファイルドロップ時の処理です.
//-----------------------------------------------------------------------------
void ModelViewer::OnDrop(const wchar_t** dropFiles, uint32_t fileCount)
{
    if (dropFiles == nullptr || fileCount == 0)
        return;

    // 最初の1つだけ処理する.
}

//-----------------------------------------------------------------------------
//      ファイルメニュー処理です.
//-----------------------------------------------------------------------------
void ModelViewer::MenuFile(ID3D12GraphicsCommandList* pCmd)
{
    // モデルファイルを開く.
    if (ImGui::MenuItem((const char*)u8"ファイルを開く"))
    {
        const char* filter = 
            "モデルファイル(*.mdb, *.dae, *.xml, *.gtlf, *.fbx, *.ply, *.dxf, *.smd, *.vta, *.mdl, *.md2, *.md3, *.md5mesh, *.md5anim, *.x, *.obj, *.ms3d, *.lwo, *.lows)\0*.mdb;*.dae;*.xml;*.gltf;*.fbx;*.ply;*.dxf;*.smd;*.vta;*.mdl;*.md2;*.md3;*.md5mesh;*.md5anim;*.x;*.obj;*.ms3d;*.lwo;*.lws\0"
            "Project Asura Model Binary (*.mdb)\0*.mdb\0"
            "Collada (*.dae, *.xml)\0*.dae;*.xml\0"
            "glTF (*.gltf)\0*.gltf\0"
            "Film Box (*.fbx)\0*.fbx\0"
            "Standard Polygon Library (*.ply)\0*.ply\0"
            "Autodesk DXF (*.dxf)\0*.dxf\0"
            "Valve Model (*.smd, *.vta)\0*.smd;*.vta\0"
            "Quake1 Model (*.mdl)\0*.mdl\0"
            "Quake2 Model (*.md2)\0*.md2\0"
            "Quake3 Model (*.md3)\0*.md3\0"
            "Doom3 Model (*.md5mesh, *.md5anim)\0*.md5mesh;*.md5anim\0"
            "DirectX X File (*.x)\0*.x\0"
            "Wavefront Object (*.obj)\0*.obj\0"
            "Milkshape 3D (*.ms3d)\0*.ms3d\0"
            "LightWave Model (*.lwo)\0*.lwo\0"
            "LightWave Scene (*.lws)\0*.lws\0"
            "全てのファイル (*.*)\0*.*\0\0";

        asdx::fs::path path;
        if (asdx::OpenFileDlg(filter, path))
        {
            auto input = path.string();

            std::vector<uint8_t> modelBinary;
            if (asdx::ModelConverter::Convert(input.c_str(), modelBinary))
            {
                m_ModelBinary = std::move(modelBinary);
                RecreateModel();
            }
        }
    }

    // 保存処理.
    if (!m_ModelBinary.empty())
    {
        if (ImGui::MenuItem((const char*)u8"名前を付けて保存"))
        {
            const char* filter = 
                "Project Asura Model Binary (*.mdb)\0*.mdb\0";
            std::string base;
            std::string ext = ".mdb";

            asdx::fs::path path;
            if (asdx::SaveFileDlg(filter, path))
            {
                m_OutputPath = path.string();
                SaveModelBinary(m_OutputPath.c_str());
            }
        }

        if (ImGui::MenuItem((const char*)u8"上書き保存"))
        {
            SaveModelBinary(m_OutputPath.c_str());
        }
    }
}

//-----------------------------------------------------------------------------
//      ヘルプメニュー処理です.
//-----------------------------------------------------------------------------
void ModelViewer::MenuHelp()
{
    // バージョン情報.
    if (ImGui::MenuItem((const char*)u8"バージョン情報"))
    {
        asdx::InfoDlg("Version Info",
            "ModelViewer ver 0.1\n"
            "Build 0.1\n"
            "Copyright(c) Project Asura.");
    }

    // ライセンス情報.
    if (ImGui::MenuItem((const char*)u8"ライセンス情報"))
    {
    }
}

//-----------------------------------------------------------------------------
//      モデルを再生成します.
//-----------------------------------------------------------------------------
void ModelViewer::RecreateModel()
{
    asdx::IModel* pModel = nullptr;

    // モデルを生成をします.
    if (!asdx::GetModelManager().CreateModel(m_ModelBinary, &pModel))
    {
        ELOGA("Error : ModelManager::CreateModel() Failed.");
        return;
    }

    // 成功したら差し替え.
    m_Model.Attach(pModel);
}

//-----------------------------------------------------------------------------
//      モデルバイナリを保存します.
//-----------------------------------------------------------------------------
void ModelViewer::SaveModelBinary(const char* path)
{
    if (path == nullptr)
        return;

    if (!asdx::SaveA(path, m_ModelBinary))
    {
        ELOG("Error : SaveA() Failed path = %s", path);
        return;
    }

    ILOG("Info : ModelBinary Output success. path = %s", path);
}

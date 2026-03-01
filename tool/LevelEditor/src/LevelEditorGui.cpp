//-----------------------------------------------------------------------------
// File : LevelEditorGui.cpp
// Desc : Gui for Level Editor.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <LevelEditor.h>
#include <fnd/asdxMisc.h>
#include <imgui.h>
#include <ImGuizmo.h>


#ifndef TABLE2
#define TABLE2(c0, c1)              \
    ImGui::TableNextRow();          \
    ImGui::TableSetColumnIndex(0);  \
    (c0);                           \
    ImGui::TableSetColumnIndex(1);  \
    (c1)
#endif//IMGUI_COLUMN2


///////////////////////////////////////////////////////////////////////////////
// LevelEditor class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      レベル情報を描画します.
//-----------------------------------------------------------------------------
void LevelEditor::DrawLevelInfo()
{
    if (!m_ShowInfo)
        return;

    const auto w = 200.0f;
    const auto h = 95.0f;
    const auto x = 10.0f;
    const auto y = 10.0f;

    ImGui::SetNextWindowPos(ImVec2(m_Width - (w + x), m_Height - (h + y)));
    ImGui::SetNextWindowSize(ImVec2(w, h));

    auto flags = ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoTitleBar;
    if (!ImGui::Begin("Info", nullptr, flags))
        return;

    ImGui::Text("FPS : %.2f", GetFPS());
    ImGui::Separator();

    ImGui::BeginTable(asdx::ToChar(u8"レベル情報"), 2);

    TABLE2(
        ImGui::Text(asdx::ToChar(u8"モデル数")),
        ImGui::Text(asdx::ToChar(u8"%zu"), m_Level.Models.size()));

    TABLE2(
        ImGui::Text(asdx::ToChar(u8"ライト数")),
        ImGui::Text(asdx::ToChar(u8"%zu"), m_Level.Lights.size()));

    TABLE2(
        ImGui::Text(asdx::ToChar(u8"ピン数")),
        ImGui::Text(asdx::ToChar(u8"%zu"), m_Level.Pins.size()));

    ImGui::EndTable();

    ImGui::End();
}

//-----------------------------------------------------------------------------
//      コンテキストメニューを描画.
//-----------------------------------------------------------------------------
void LevelEditor::DrawContextMenu()
{
    if (ImGui::IsMouseClicked(1))
    { ImGui::OpenPopup("ContextMenu"); }

    if (ImGui::BeginPopup("ContextMenu"))
    {
        if (ImGui::BeginMenu(asdx::ToChar(u8"ファイル")))
        {
            MenuFile();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(asdx::ToChar(u8"表示")))
        {
            MenuView();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(asdx::ToChar(u8"ヘルプ")))
        {
            MenuHelp();
            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }
}

//-----------------------------------------------------------------------------
//      ファイルメニュー処理です.
//-----------------------------------------------------------------------------
void LevelEditor::MenuFile()
{
    if (ImGui::MenuItem(asdx::ToChar(u8"レベルを開く")))
    {
    }

    if (ImGui::MenuItem(asdx::ToChar(u8"名前を付けてレベルを保存")))
    {
    }
}

//-----------------------------------------------------------------------------
//      表示メニュー処理です.
//-----------------------------------------------------------------------------
void LevelEditor::MenuView()
{
    ImGui::Checkbox(asdx::ToChar(u8"プロパティウィンドウ"), &m_ShowProperty);
    ImGui::Checkbox(asdx::ToChar(u8"情報パネル"), &m_ShowInfo);
}

//-----------------------------------------------------------------------------
//      ヘルプメニュー処理です.
//-----------------------------------------------------------------------------
void LevelEditor::MenuHelp()
{
    if (ImGui::MenuItem(asdx::ToChar(u8"バージョン情報")))
    {
        asdx::InfoDlg("Version Info",
            "LevelEditor ver 0.1\n"
            "Build 0.1\n"
            "Copyright(c) Project Asura.");
    }

    if (ImGui::MenuItem(asdx::ToChar(u8"ライセンス情報")))
    {
        m_ShowLisence = true;
    }
}

//-----------------------------------------------------------------------------
//      プロパティウィンドウを描画.
//-----------------------------------------------------------------------------
void LevelEditor::DrawPropertyWindow()
{
    if (!m_ShowProperty)
        return;

    ImGui::SetNextWindowPos(ImVec2(1510, 10), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_Once);

    if (!ImGui::Begin(asdx::ToChar(u8"プロパティ"), &m_ShowProperty))
        return;

    auto flags = ImGuiTabBarFlags_TabListPopupButton | ImGuiTabBarFlags_FittingPolicyScroll;
    if (ImGui::BeginTabBar("Panels", flags))
    {
        // モデルタブを描画.
        DrawModelTab();

        // ライトタブを描画.
        DrawLightTab();

        // ピンタブを描画.
        DrawPinTab();

        // デバッグタブを描画.
        DrawDebugTab();

        ImGui::EndTabBar();
    }

    ImGui::End();
}

//-----------------------------------------------------------------------------
//      モデルタブを描画.
//-----------------------------------------------------------------------------
void LevelEditor::DrawModelTab()
{
    if (!ImGui::BeginTabItem(asdx::ToChar(u8"モデル")))
        return;

    if (m_Level.Models.empty())
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), asdx::ToChar(u8"モデルがありません"));
    }
    else
    {
    }

    ImGui::EndTabItem();
}

//-----------------------------------------------------------------------------
//      ライトタブを描画.
//-----------------------------------------------------------------------------
void LevelEditor::DrawLightTab()
{
    if (!ImGui::BeginTabItem(asdx::ToChar(u8"ライト")))
        return;

    if (m_Level.Models.empty())
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), asdx::ToChar(u8"ライトがありません"));
    }
    else
    {
    }


    ImGui::EndTabItem();
}

//-----------------------------------------------------------------------------
//      ピンタブを描画.
//-----------------------------------------------------------------------------
void LevelEditor::DrawPinTab()
{
    if (!ImGui::BeginTabItem(asdx::ToChar(u8"ピン")))
        return;

    if (m_Level.Models.empty())
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), asdx::ToChar(u8"ピンがありません"));
    }
    else
    {
    }


    ImGui::EndTabItem();
}

//-----------------------------------------------------------------------------
//      デバッグタブを描画.
//-----------------------------------------------------------------------------
void LevelEditor::DrawDebugTab()
{
    if (!ImGui::BeginTabItem(asdx::ToChar(u8"デバッグ")))
        return;


    ImGui::EndTabItem();
}

//-----------------------------------------------------------------------------
//      ギズモを描画します.
//-----------------------------------------------------------------------------
void LevelEditor::DrawGizmo(asdx::Vector3& position, asdx::Vector3& scale, asdx::Quaternion& rotation)
{
    if (!m_EnableGuizmo)
        return;

    auto& view = m_Camera.GetView();
    auto& proj = m_Proj;
    auto& io = ImGui::GetIO();

    float matrix[16] = {};
    ImGuizmo::RecomposeMatrixFromComponents(&position.x, &rotation.x, &scale.x, matrix);
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    if (ImGuizmo::Manipulate(&view._11, &proj._11, m_GuizmoOperation, ImGuizmo::MODE::LOCAL, matrix))
    {
        ImGuizmo::DecomposeMatrixToComponents(
            matrix, &position.x, &rotation.x, &scale.x);
    }
}

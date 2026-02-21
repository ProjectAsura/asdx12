//-----------------------------------------------------------------------------
// File : ModelViewerGui.cpp
// Desc : Gui for ModelViewer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <ModelViewer.h>
#include <fnd/asdxMisc.h>
#include <fnd/asdxLogger.h>
#include <DirectXMath.h>


namespace {

const char* kDrawModes[] = {
    asdx::ToChar(u8"ライティング"),
    asdx::ToChar(u8"スクリーン空間位置座標"),
    asdx::ToChar(u8"法線ベクトル"),
    asdx::ToChar(u8"接線ベクトル"),
    asdx::ToChar(u8"従接線ベクトル"),
    asdx::ToChar(u8"テクスチャ座標"),
    asdx::ToChar(u8"頂点カラー"),
    asdx::ToChar(u8"ボーン番号"),
    asdx::ToChar(u8"ボーン重み"),
};


//-----------------------------------------------------------------------------
//      コンボボックスを描画します.
//-----------------------------------------------------------------------------
static bool ImGuiCombo(const char* caption, size_t& index, const std::vector<std::string>& items)
{
    if (items.empty())
    {
        int idx = 0;
        ImGui::Combo(caption, &idx, "None");
        return false;
    }

    if (!ImGui::BeginCombo(caption, items[index].c_str())) 
        return false;

    assert(index < items.size());
    bool changed = false;
    for (size_t n=0; n<items.size(); n++)
    {
        auto selected = (n == index);
        if (ImGui::Selectable(items[n].c_str(), selected))
        {
            index   = n;
            changed = true;
        }
        if (selected)
        {
            ImGui::SetItemDefaultFocus();
        }
    }
    ImGui::EndCombo();

    return changed;
}

} // namespace


///////////////////////////////////////////////////////////////////////////////
// ModelViewer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      モデル情報を描画します.
//-----------------------------------------------------------------------------
void ModelViewer::DrawModelInfo()
{
    const auto w = 200.0f;
    const auto h = 190.0f;
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

        ImGui::BeginTable(asdx::ToChar(u8"モデル情報"), 2);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text(asdx::ToChar(u8"メッシュ数"));
        ImGui::TableSetColumnIndex(1);
        ImGui::Text(asdx::ToChar(u8"%zu"), m_ModelInfo.MeshCount);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text(asdx::ToChar(u8"マテリアル数"));
        ImGui::TableSetColumnIndex(1);
        ImGui::Text(asdx::ToChar(u8"%zu"), m_ModelInfo.MaterialCount);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text(asdx::ToChar(u8"ボーン数"));
        ImGui::TableSetColumnIndex(1);
        ImGui::Text(asdx::ToChar(u8"%zu"), m_ModelInfo.BoneCount);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text(asdx::ToChar(u8"頂点数"));
        ImGui::TableSetColumnIndex(1);
        ImGui::Text(asdx::ToChar(u8"%zu"), m_ModelInfo.VertexCount);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text(asdx::ToChar(u8"インデックス数"));
        ImGui::TableSetColumnIndex(1);
        ImGui::Text(asdx::ToChar(u8"%zu"), m_ModelInfo.IndexCount);

        ImGui::EndTable();

        ImGui::Separator();

        ImGui::BeginTable(asdx::ToChar(u8"モーション情報"), 2);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text(asdx::ToChar(u8"クリップ数"));
        ImGui::TableSetColumnIndex(1);
        ImGui::Text(asdx::ToChar(u8"%u"), m_MotionBinary.GetClipCount());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text(asdx::ToChar(u8"所要時間"));
        ImGui::TableSetColumnIndex(1);
        ImGui::Text(asdx::ToChar(u8"%.2f"), m_MotionPlayer.GetDuration());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text(asdx::ToChar(u8"Tick/秒"));
        ImGui::TableSetColumnIndex(1);
        ImGui::Text(asdx::ToChar(u8"%.2f"), m_MotionPlayer.GetTicksPerSecond());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text(asdx::ToChar(u8"再生時間"));
        ImGui::TableSetColumnIndex(1);
        ImGui::Text(asdx::ToChar(u8"%.2f"), m_MotionPlayer.GetTimeInTicks());

        ImGui::EndTable();

        ImGui::End();
    }
}

//-----------------------------------------------------------------------------
//      コンテキストメニューを描画します.
//-----------------------------------------------------------------------------
void ModelViewer::DrawContextMenu(ID3D12GraphicsCommandList* pCmd)
{
    if (ImGui::IsMouseClicked(1))
    { ImGui::OpenPopup("ContextMenu"); }

    if (ImGui::BeginPopup("ContextMenu"))
    {
        if (ImGui::BeginMenu(asdx::ToChar(u8"ファイル")))
        {
            MenuFile(pCmd);
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
//      ギズモを描画します.
//-----------------------------------------------------------------------------
void ModelViewer::DrawGizmo(asdx::Matrix& modelWorld)
{
    if (m_ModelInfo.MeshCount == 0)
        return;

    if (!m_EnableGuizmo)
        return;

    auto& view = m_Camera.GetView();
    auto& proj = m_Proj;
    auto& io = ImGui::GetIO();

    float matrix[16] = {};
    ImGuizmo::RecomposeMatrixFromComponents(&m_ModelTranslation.x, &m_ModelRotation.x, &m_ModelScale.x, matrix);
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    if (ImGuizmo::Manipulate(&view._11, &proj._11, m_GuizmoOperation, ImGuizmo::MODE::LOCAL, matrix))
    {
        ImGuizmo::DecomposeMatrixToComponents(
            matrix, &m_ModelTranslation.x, &m_ModelRotation.x, &m_ModelScale.x);
    }

    modelWorld = asdx::Matrix(matrix);
}

//-----------------------------------------------------------------------------
//      バウンディングスフィアを描画します.
//-----------------------------------------------------------------------------
void ModelViewer::DrawBoundingSphere(const asdx::Matrix& modelWorld)
{
    if (!m_Model)
        return;

    if (!m_DrawBoundingSphere)
        return;

    // メッシュのバウンディングスフィア.
    for(auto i=0u; i<m_Model->GetMeshCount(); ++i)
    {
        auto& sphere = m_Model->GetMesh(i)->GetBoundingSphere();
        auto world = asdx::Matrix::CreateScale(sphere.Radius) * asdx::Matrix::CreateTranslation(sphere.Center) * modelWorld;

        uint32_t index = uint32_t(i);
        m_ShapeParams.SetWorld(index, world);
        m_ShapeParams.SetColor(index, asdx::Vector4(0.0f, 1.0f, 0.0f, 0.1f));
    }

    // モデルのバウンディングスフィア.
    {
        auto& sphere = m_Model->GetBoundingSphere();
        auto world = asdx::Matrix::CreateScale(sphere.Radius) * asdx::Matrix::CreateTranslation(sphere.Center) * modelWorld;

        uint32_t index = uint32_t(m_Model->GetMeshCount());
        m_ShapeParams.SetWorld(index, world);
        m_ShapeParams.SetColor(index, asdx::Vector4(1.0f, 1.0f, 0.0f, 0.1f));
    }
}

//-----------------------------------------------------------------------------
//      ボーンを描画します.
//-----------------------------------------------------------------------------
void ModelViewer::DrawBones(const asdx::Matrix& modelWorld)
{
    if (!m_Model)
        return;

    if (!m_DrawBones)
        return;

    auto count = m_Model->GetBoneCount();

    // モーションがある場合はアニメーション後のボーンを表示
    if (m_MotionBinary.GetClipCount() > 0)
    {
        auto matrices = m_MotionPlayer.GetWorldTransforms();
        for(auto i=0u; i<count; ++i)
        {
            auto& bone = m_Model->GetBone(i);
            auto parentId = asdx::BoneProxy::GetParentId(bone);
            if (parentId < 0)
                continue;

            auto m0 = matrices[parentId] * modelWorld;
            auto p0 = m0.GetPosition();

            auto m1 = matrices[i] * modelWorld;
            auto p1 = m1.GetPosition();

            asdx::DrawWireBone(m_LineRenderer, p0, p1, asdx::Vector4(0.0f, 1.0f, 1.0f, 1.0f));
        }
    }
    // バインドポーズ表示.
    else
    {
        for(auto i=0u; i<count; ++i)
        {
            auto& bone = m_Model->GetBone(i);

            auto parentId = asdx::BoneProxy::GetParentId(bone);
            if (parentId < 0)
                continue;

            auto& parentBone = m_Model->GetBone(uint32_t(parentId));
            auto m0 = asdx::BoneProxy::GetBindPoseMatrix(parentBone) * modelWorld;
            auto p0 = m0.GetPosition();

            auto m1 = asdx::BoneProxy::GetBindPoseMatrix(bone) * modelWorld;
            auto p1 = m1.GetPosition();

            asdx::DrawWireBone(m_LineRenderer, p0, p1, asdx::Vector4(0.0f, 1.0f, 1.0f, 1.0f));
        }
    }
}

//-----------------------------------------------------------------------------
//      プロパティウィンドウを描画します.
//-----------------------------------------------------------------------------
void ModelViewer::DrawPropertyWindow()
{
    ImGui::SetNextWindowPos(ImVec2(1510, 10), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_Once);

    if (!ImGui::Begin(asdx::ToChar(u8"プロパティ")))
        return;

    auto flags = ImGuiTabBarFlags_TabListPopupButton | ImGuiTabBarFlags_FittingPolicyScroll;
    if (ImGui::BeginTabBar("Panels", flags))
    {
        // マテリアルタブを描画.
        DrawMaterialTab();

        // モーションタブを描画.
        DrawMotionTab();

        // デバッグタブを描画.
        DrawDebugTab();

        ImGui::EndTabBar();
    }

    ImGui::End();
}

//-----------------------------------------------------------------------------
//      マテリアルタブを描画します.
//-----------------------------------------------------------------------------
void ModelViewer::DrawMaterialTab()
{
    if (!ImGui::BeginTabItem(asdx::ToChar(u8"マテリアル")))
        return;

    if (!!m_Model && m_Model->GetMaterialCount() > 0)
    {
        auto count = m_Model->GetMaterialCount();
        for(auto i=0u; i<count; ++i)
        {
            if (!ImGui::CollapsingHeader(m_Model->GetMaterialName(i), ImGuiTreeNodeFlags_DefaultOpen))
                continue;


            ImGui::Separator();
        }
    }
    else
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "NO MATERIAL");
    }

    ImGui::EndTabItem();
}

//-----------------------------------------------------------------------------
//      モーションタブを描画します.
//-----------------------------------------------------------------------------
void ModelViewer::DrawMotionTab()
{
    if (!ImGui::BeginTabItem(asdx::ToChar(u8"モーション")))
        return;

    if (ImGuiCombo(asdx::ToChar(u8"再生クリップ"), m_ClipIndex, m_ClipNames))
    {
        // クリップを差し替え.
        m_MotionPlayer.SetClip(m_MotionBinary.GetClip(uint32_t(m_ClipIndex)));
    }

    if (ImGui::Button(asdx::ToChar(u8"再生")))
    {
        m_MotionPlayer.SetPause(false);
    }
    ImGui::SameLine();
    if (ImGui::Button(asdx::ToChar(u8"停止")))
    {
        m_MotionPlayer.SetPause(true);
    }
    ImGui::SameLine();
    if (ImGui::Button(asdx::ToChar(u8"コマ送り")))
    {
        auto root = m_MotionBinary.GetRootTransform();
        m_MotionPlayer.FrameAdvance(root);
    }
    ImGui::SameLine();
    if (ImGui::Button(asdx::ToChar(u8"頭出し")))
    {
        m_MotionPlayer.Cue();
    }
    ImGui::SameLine();
    if (ImGui::Button(asdx::ToChar(u8"クリア")))
    {
        m_MotionPlayer.SetClip(nullptr);
        m_ClipIndex = 0;
        m_ClipNames.clear();
        m_MotionBinary.Term();
    }

    ImGui::SameLine();
    auto loop = m_MotionPlayer.IsLoop();
    if (ImGui::Checkbox(asdx::ToChar(u8"ループ再生"), &loop))
    { m_MotionPlayer.SetLoop(loop); }

    auto speed = m_MotionPlayer.GetPlaySpeed();
    if (ImGui::DragFloat(asdx::ToChar(u8"再生速度"), &speed, 0.1f))
    {
        m_MotionPlayer.SetPlaySpeed(speed);
    }

    ImGui::EndTabItem();
}

//-----------------------------------------------------------------------------
//      デバッグタブを描画します.
//-----------------------------------------------------------------------------
void ModelViewer::DrawDebugTab()
{
    if (!ImGui::BeginTabItem(asdx::ToChar(u8"デバッグ")))
        return;

    int mode = (int)m_DrawMode;
    ImGui::Combo(asdx::ToChar(u8"描画モード"), &mode, kDrawModes, _countof(kDrawModes));
    m_DrawMode = mode;

    ImGui::Checkbox(asdx::ToChar(u8"ワイヤーフレーム"), &m_EnableWireframe);
    ImGui::Checkbox(asdx::ToChar(u8"バウンディングスフィア表示"), &m_DrawBoundingSphere);
    ImGui::Checkbox(asdx::ToChar(u8"ボーン表示"), &m_DrawBones);

    ImGui::EndTabItem();
}
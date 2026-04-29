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
#include <fnd/asdxPath.h>
#include <gfx/asdxGfxMisc.h>
#include <res/asdxResTexture.h>
#include <DirectXMath.h>
#include <TextureConverter.h>


#ifndef TABLE2
#define TABLE2(c0, c1)              \
    ImGui::TableNextRow();          \
    ImGui::TableSetColumnIndex(0);  \
    (c0);                           \
    ImGui::TableSetColumnIndex(1);  \
    (c1)
#endif//IMGUI_COLUMN2


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
const char* kDrawModes[] = {
    asdx::ToChar(u8"ライティング"),
    asdx::ToChar(u8"スクリーン空間位置座標"),
    asdx::ToChar(u8"法線ベクトル"),
    asdx::ToChar(u8"接線ベクトル"),
    asdx::ToChar(u8"従接線ベクトル"),
    asdx::ToChar(u8"テクスチャ座標"),
    asdx::ToChar(u8"頂点カラー"),
    asdx::ToChar(u8"頂点カラー(R)"),
    asdx::ToChar(u8"頂点カラー(G)"),
    asdx::ToChar(u8"頂点カラー(B)"),
    asdx::ToChar(u8"頂点カラー(A)"),
    asdx::ToChar(u8"ボーン番号"),
    asdx::ToChar(u8"ボーン重み"),
    asdx::ToChar(u8"ベースカラー"),
    asdx::ToChar(u8"オクルージョン"),
    asdx::ToChar(u8"ラフネス"),
    asdx::ToChar(u8"メタルネス"),
    asdx::ToChar(u8"アルファ"),
    asdx::ToChar(u8"屈折率"),
    asdx::ToChar(u8"エミッシブカラー"),
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

//-----------------------------------------------------------------------------
//      テクスチャ情報を描画します.
//-----------------------------------------------------------------------------
static void DrawTextureInfo(const char* label, const char* path, const asdx::TextureHolder& holder)
{
    ImGui::PushID(label);
    ImGui::Text(label);
    ImGui::Text("Path : %s", path);

    ImGui::BeginTable("##Table", 2);
    ImGui::TableSetupColumn("##row0", ImGuiTableColumnFlags_WidthFixed);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Image(holder.GetHandleGPU().ptr, ImVec2(64, 64));

    ImGui::TableSetColumnIndex(1);
    auto desc = holder.GetDesc();
    ImGui::Text("Dimension : %s", asdx::ToShortString(desc.Dimension));
    ImGui::Text("Size      : (%llu, %u, %u)", desc.Width, desc.Height, desc.DepthOrArraySize);
    ImGui::Text("MipLevels : %u", desc.MipLevels);
    ImGui::Text("Format    : %s", asdx::ToShortString(desc.Format));

    ImGui::EndTable();
    ImGui::PopID();
}

//-----------------------------------------------------------------------------
//      アルファモードを文字列にします.
//-----------------------------------------------------------------------------
static const char* ToString(asdx::AlphaMode mode)
{
    switch(mode)
    {
    case asdx::AlphaMode::Opaque:
        return "Opaque";

    case asdx::AlphaMode::Mask:
        return "Mask";

    case asdx::AlphaMode::Blend:
       return "Blend";

    default:
        return "Unknown";
    }
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
    if (!m_ShowInfo)
        return;

    const auto w = 200.0f;
    const auto h = 210.0f;
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

        TABLE2(
            ImGui::Text(asdx::ToChar(u8"メッシュ数")),
            ImGui::Text(asdx::ToChar(u8"%zu"), m_ModelInfo.MeshCount));

        TABLE2(
            ImGui::Text(asdx::ToChar(u8"マテリアル数")),
            ImGui::Text(asdx::ToChar(u8"%zu"), m_ModelInfo.MaterialCount));

        TABLE2(
            ImGui::Text(asdx::ToChar(u8"ボーン数")),
            ImGui::Text(asdx::ToChar(u8"%zu"), m_ModelInfo.BoneCount));

        TABLE2(
            ImGui::Text(asdx::ToChar(u8"頂点数")),
            ImGui::Text(asdx::ToChar(u8"%zu"), m_ModelInfo.VertexCount));

        TABLE2(
            ImGui::Text(asdx::ToChar(u8"インデックス数")),
            ImGui::Text(asdx::ToChar(u8"%zu"), m_ModelInfo.IndexCount));

        ImGui::EndTable();

        ImGui::Separator();

        ImGui::BeginTable(asdx::ToChar(u8"モーション情報"), 2);

        TABLE2(
            ImGui::Text(asdx::ToChar(u8"クリップ数")),
            ImGui::Text(asdx::ToChar(u8"%u"), m_MotionBinary.GetClipCount()));

        TABLE2(
            ImGui::Text(asdx::ToChar(u8"所要時間")),
            ImGui::Text(asdx::ToChar(u8"%.2f"), m_MotionPlayer.GetDuration()));

        TABLE2(
            ImGui::Text(asdx::ToChar(u8"Tick/秒")),
            ImGui::Text(asdx::ToChar(u8"%.2f"), m_MotionPlayer.GetTicksPerSecond()));

        TABLE2(
            ImGui::Text(asdx::ToChar(u8"再生時間")),
            ImGui::Text(asdx::ToChar(u8"%.2f"), m_MotionPlayer.GetTimeInTicks()));

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
//      ライセンスを描画します.
//-----------------------------------------------------------------------------
void ModelViewer::DrawLisence()
{
    if (m_ShowLisence)
    {
        ImGui::OpenPopup("Lisence");
        m_ShowLisence = false;
    }

    if (ImGui::BeginPopupModal("Lisence"))
    {
        ImGui::Text("Dear ImGui");
        ImGui::TextLinkOpenURL("https://github.com/ocornut/imgui/blob/master/LICENSE.txt", "https://github.com/ocornut/imgui/blob/master/LICENSE.txt");
        ImGui::NewLine();

        ImGui::Text("simdjson");
        ImGui::TextLinkOpenURL("https://github.com/simdjson/simdjson/blob/master/LICENSE", "https://github.com/simdjson/simdjson/blob/master/LICENSE");
        ImGui::TextLinkOpenURL("https://github.com/simdjson/simdjson/blob/master/LICENSE-MIT", "https://github.com/simdjson/simdjson/blob/master/LICENSE-MIT");
        ImGui::NewLine();

        ImGui::Text("ImGuizmo");
        ImGui::TextLinkOpenURL("https://github.com/CedricGuillemet/ImGuizmo/blob/master/LICENSE", "https://github.com/CedricGuillemet/ImGuizmo/blob/master/LICENSE");
        ImGui::NewLine();

        ImGui::Text("D3D12MemoryAllocator");
        ImGui::TextLinkOpenURL("https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator/blob/master/LICENSE.txt", "https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator/blob/master/LICENSE.txt");
        ImGui::NewLine();

        ImGui::Text("xxHash");
        ImGui::TextLinkOpenURL("https://github.com/Cyan4973/xxHash/blob/dev/LICENSE", "https://github.com/Cyan4973/xxHash/blob/dev/LICENSE");
        ImGui::NewLine();

        ImGui::Text("flatbuffers");
        ImGui::TextLinkOpenURL("https://github.com/google/flatbuffers/blob/master/LICENSE", "https://github.com/google/flatbuffers/blob/master/LICENSE");
        ImGui::NewLine();

        ImGui::Text("MikkTSpace");
        ImGui::TextLinkOpenURL("https://github.com/mmikk/MikkTSpace/blob/master/mikktspace.c", "https://github.com/mmikk/MikkTSpace/blob/master/mikktspace.c");
        ImGui::NewLine();

        ImGui::Text("DirectXTex");
        ImGui::TextLinkOpenURL("https://github.com/microsoft/DirectXTex/blob/main/LICENSE", "https://github.com/microsoft/DirectXTex/blob/main/LICENSE");
        ImGui::NewLine();

        if (ImGui::Button("Close"))
            ImGui::CloseCurrentPopup();

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
        auto& sphere = m_Model->GetMesh(i)->GetSphere();
        auto world = asdx::Matrix::CreateScale(sphere.Radius) * asdx::Matrix::CreateTranslation(sphere.Center) * modelWorld;

        uint32_t index = uint32_t(i);
        m_ShapeParams.SetWorld(index, world);
        m_ShapeParams.SetColor(index, asdx::Vector4(0.0f, 1.0f, 0.0f, 0.1f));
    }

    // モデルのバウンディングスフィア.
    {
        auto& sphere = m_Model->GetSphere();
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

    auto count    = m_Model->GetBoneCount();
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

//-----------------------------------------------------------------------------
//      プロパティウィンドウを描画します.
//-----------------------------------------------------------------------------
void ModelViewer::DrawPropertyWindow()
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
        // マテリアルタブを描画.
        DrawMaterialTab();

        // モーションタブを描画.
        DrawMotionTab();

        // メッシュタブを描画.
        DrawMeshTab();

        // ボーンタブを描画.
        DrawBoneTab();

        // デバッグタブを描画.
        DrawDebugTab();

        ImGui::EndTabBar();
    }

    ImGui::End();
}

//-----------------------------------------------------------------------------
//      メッシュタブを描画します.
//-----------------------------------------------------------------------------
void ModelViewer::DrawMeshTab()
{
    if (!ImGui::BeginTabItem(asdx::ToChar(u8"メッシュ")))
        return;

    if (!!m_Model && m_Model->GetMeshCount() > 0)
    {
        auto count = m_Model->GetMeshCount();
        for(auto i=0u; i<count; ++i)
        {
            ImGui::PushID(i);

            const auto& res = m_Model->GetResMesh(i);
            auto name = asdx::MeshProxy::GetName(res);
            if (!ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::PopID();
                continue;
            }

            auto drawMesh = m_Model->GetMesh(i);
            auto visible = drawMesh->IsVisible();

            if (ImGui::Checkbox(asdx::ToChar(u8"表示"), &visible))
            { drawMesh->SetVisibility(visible); }

            ImGui::BeginTable("##MeshTable", 2);
            ImGui::TableSetupColumn("##MeshRow0", ImGuiTableColumnFlags_WidthFixed);

            auto materialId = asdx::MeshProxy::GetMaterialId(res);

            TABLE2(
                ImGui::Text(asdx::ToChar(u8"マテリアルID")),
                ImGui::Text(asdx::ToChar(u8"%u"), materialId));

            const auto& material = m_Model->GetResMaterial(materialId);
            auto matName = asdx::MaterialProxy::GetName(material);
            TABLE2(
                ImGui::Text(asdx::ToChar(u8"マテリアル名")),
                ImGui::Text(asdx::ToChar(u8"%s"), matName));

            auto positions = asdx::MeshProxy::GetPositions(res);
            TABLE2(
                ImGui::Text(asdx::ToChar(u8"頂点数")),
                ImGui::Text(asdx::ToChar(u8"%llu"), positions.size()));

            auto indices = asdx::MeshProxy::GetVerexIndices(res);
            TABLE2(
                ImGui::Text(asdx::ToChar(u8"ポリゴン数")),
                ImGui::Text(asdx::ToChar(u8"%llu"), indices.size() / 3));

            ImGui::EndTable();

            ImGui::PopID();

            ImGui::Separator();
        }
    }
    else
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), asdx::ToChar(u8"メッシュがありません"));
    }

    ImGui::EndTabItem();
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
            const auto& res = m_Model->GetResMaterial(i);
            ImGui::PushID(i);

            auto name = asdx::MaterialProxy::GetName(res);
            if (!ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::PopID();
                continue;
            }

            const auto mtl = m_Model->GetMaterial(i);

            ImGui::BeginTable("##ParamTable", 2);
            ImGui::TableSetupColumn("##ParamRow0", ImGuiTableColumnFlags_WidthFixed);

            auto baseColor = asdx::MaterialProxy::GetBaseColorFactor(res);
            TABLE2(
                ImGui::Text(asdx::ToChar(u8"ベースカラー")),
                ImGui::Text("%f, %f, %f", baseColor.x, baseColor.y, baseColor.z));

            auto alpha = asdx::MaterialProxy::GetAlpha(res);
            TABLE2(
                ImGui::Text(asdx::ToChar(u8"アルファ値")),
                ImGui::Text("%f", alpha));

            auto occlusion = asdx::MaterialProxy::GetOcclusionFactor(res);
            TABLE2(
                ImGui::Text(asdx::ToChar(u8"オクルージョン")),
                ImGui::Text("%f", occlusion));

            auto roughness = asdx::MaterialProxy::GetRoughnessFactor(res);
            TABLE2(
                ImGui::Text(asdx::ToChar(u8"ラフネス")),
                ImGui::Text("%f", roughness));

            auto ior = asdx::MaterialProxy::GetIor(res);
            TABLE2(
                ImGui::Text(asdx::ToChar(u8"屈折率")),
                ImGui::Text("%f", ior));

            auto emissive = asdx::MaterialProxy::GetEmissiveFactor(res);
            TABLE2(
                ImGui::Text(asdx::ToChar(u8"エミッシブ")),
                ImGui::Text("%f, %f, %f", emissive.x, emissive.y, emissive.z));

            auto alphaCutOff = asdx::MaterialProxy::GetAlphaCutOff(res);
            TABLE2(
                ImGui::Text(asdx::ToChar(u8"アルファカットオフ")),
                ImGui::Text("%f", alphaCutOff));

            auto alphaMode = asdx::MaterialProxy::GetAlphaMode(res);
            TABLE2(
                ImGui::Text(asdx::ToChar(u8"アルファモード")),
                ImGui::Text("%s", ToString(alphaMode)));

            ImGui::EndTable();
            ImGui::Separator();

            auto baseColorMap = asdx::MaterialProxy::GetBaseColorMap(res);
            auto baseColorPath = (baseColorMap.is_null_or_empty() ? "NONE" : baseColorMap.c_str());
            DrawTextureInfo(asdx::ToChar(u8"ベースカラーマップ"), baseColorPath, mtl->GetTexture(asdx::Material::TEXTURE_BASE_COLOR));
            ImGui::Separator();

            auto normalMap = asdx::MaterialProxy::GetNormalMap(res);
            auto normalPath = (normalMap.is_null_or_empty() ? "NONE" : normalMap.c_str());
            DrawTextureInfo(asdx::ToChar(u8"法線マップ"), normalPath, mtl->GetTexture(asdx::Material::TEXTURE_NORMAL));
            ImGui::Separator();

            auto ormMap = asdx::MaterialProxy::GetOrmMap(res);
            auto ormPath = (ormMap.is_null_or_empty() ? "NONE" : ormMap.c_str());
            DrawTextureInfo(asdx::ToChar(u8"ORMマップ"), ormPath, mtl->GetTexture(asdx::Material::TEXTURE_ORM));
            ImGui::Separator();

            auto emissiveMap = asdx::MaterialProxy::GetEmissiveMap(res);
            auto emissivePath = (emissiveMap.is_null_or_empty() ? "NONE" : emissiveMap.c_str());
            DrawTextureInfo(asdx::ToChar(u8"エミッシブマップ"), emissivePath, mtl->GetTexture(asdx::Material::TEXTURE_EMISSIVE));
            ImGui::Separator();

            ImGui::PopID();
        }
    }
    else
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), asdx::ToChar(u8"マテリアルがありません"));
    }

    ImGui::EndTabItem();
}

//-----------------------------------------------------------------------------
//      ボーンタブを描画します.
//-----------------------------------------------------------------------------
void ModelViewer::DrawBoneTab()
{
    if (!ImGui::BeginTabItem(asdx::ToChar(u8"ボーン")))
        return;

    if (!!m_Model && m_Model->GetBoneCount() > 0)
    {
        auto count = m_Model->GetBoneCount();
        for(auto i=0u; i<count; ++i)
        {
            const auto& res = m_Model->GetBone(i);

            ImGui::PushID(i);

            auto name = asdx::BoneProxy::GetName(res);
            if (!ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::PopID();
                continue;
            }

            ImGui::BeginTable("##BoneTable", 2);

            auto parentId = asdx::BoneProxy::GetParentId(res);
            const char* parentName = "NONE";
            if (parentId >= 0)
            {
                const auto& parentBone = m_Model->GetBone(parentId);
                parentName = asdx::BoneProxy::GetName(parentBone).c_str();
            }

            TABLE2(
                ImGui::Text(asdx::ToChar(u8"親ボーン")),
                ImGui::Text(asdx::ToChar(u8"%s"), parentName));

            auto children = asdx::BoneProxy::GetChildren(res);
            for(auto j=0u; j<children.size(); ++j)
            {
                auto childId = children[j];
                const char* childName = "NONE";
                if (childId >= 0)
                {
                    const auto& childBone = m_Model->GetBone(uint32_t(childId));
                    childName = asdx::BoneProxy::GetName(childBone).c_str();
                }

                TABLE2(
                    ImGui::Text(asdx::ToChar(u8"子ボーン[%u]"), j),
                    ImGui::Text(asdx::ToChar(u8"%s"), childName));
            }

            ImGui::EndTable();

            ImGui::PopID();

            ImGui::Separator();
        }
    }
    else
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), asdx::ToChar(u8"ボーンがありません"));
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
        bool loop = m_MotionPlayer.IsLoop();

        // クリップを差し替え.
        m_MotionPlayer.NextClip(m_MotionBinary.GetClip(uint32_t(m_ClipIndex)), loop, 0.5f);
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

    ImGui::ColorEdit4(asdx::ToChar(u8"背景色"), m_ClearColor, ImGuiColorEditFlags_Float);

    int mode = (int)m_DrawMode;
    if (ImGui::Combo(asdx::ToChar(u8"描画モード"), &mode, kDrawModes, _countof(kDrawModes)))
    { m_DrawMode = mode; }

    ImGui::Checkbox(asdx::ToChar(u8"ワイヤーフレーム"), &m_EnableWireframe);
    ImGui::Checkbox(asdx::ToChar(u8"バウンディングスフィア表示"), &m_DrawBoundingSphere);
    ImGui::Checkbox(asdx::ToChar(u8"ボーン表示"), &m_DrawBones);
    ImGui::Checkbox(asdx::ToChar(u8"軸表示"), &m_DrawAxis);
    ImGui::Checkbox(asdx::ToChar(u8"グリッド表示"), &m_DrawGrid);
    ImGui::Checkbox(asdx::ToChar(u8"環境マップ表示"), &m_DrawSky);

    ImGui::Separator();

    const char* filter =
        "読込可能なテクスチャ形式\0*.hdr;*.dds;*.txb;*jxr\0"
        "Project Asura Texture Binary (*.txb)\0*.txb\0"
        "Direct Draw Surface (*.dds)\0*.dds\0"
        "Radiance HDR (*.hdr)\0*.hdr\0"
        "JPEG XR (*.jxr)\0*.jxr\0\0";

    ImGui::BeginTable("##Maps", 2);
    ImGui::TableSetupColumn("##MapRow0", ImGuiTableColumnFlags_WidthFixed);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    if (ImGui::Button(asdx::ToChar(u8"環境マップ")))
    {
        asdx::fs::path path;
        if (asdx::OpenFileDlg(filter, path))
        {
            CreateTexture(path.string().c_str(), &m_EnvMap);
            m_PathEnvMap = path.filename().string();
        }
    }
    ImGui::TableSetColumnIndex(1);
    ImGui::Text(asdx::ToChar(u8"Path : %s"), m_PathEnvMap.empty() ? "NONE" : m_PathEnvMap.c_str());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    if (ImGui::Button(asdx::ToChar(u8"DiffuseLD")))
    {
        asdx::fs::path path;
        if (asdx::OpenFileDlg(filter, path))
        {
            CreateTexture(path.string().c_str(), &m_DiffuseLD);
            m_PathDiffuseLD = path.filename().string();
        }
    }
    ImGui::TableSetColumnIndex(1);
    ImGui::Text(asdx::ToChar(u8"Path : %s"), m_PathDiffuseLD.empty() ? "NONE" : m_PathDiffuseLD.c_str());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    if (ImGui::Button(asdx::ToChar(u8"SpecularLD")))
    {
        asdx::fs::path path;
        if (asdx::OpenFileDlg(filter, path))
        {
            CreateTexture(path.string().c_str(), &m_SpecularLD);
            m_PathSpecularLD = path.filename().string();
        }
    }
    ImGui::TableSetColumnIndex(1);
    ImGui::Text(asdx::ToChar(u8"Path : %s"), m_PathSpecularLD.empty() ? "NONE" : m_PathSpecularLD.c_str());

    ImGui::EndTable();

    ImGui::Separator();

    ImGui::EndTabItem();
}
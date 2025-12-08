//-----------------------------------------------------------------------------
// File : asdxRingMenu.cpp
// Desc : Ring Menu.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <edit/asdxRingMenu.h>
#include <cmath>
#include <imgui_internal.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// RingMenu class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
RingMenu::RingMenu()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int RingMenu::AddCategory(const std::string& name)
{
    m_Categories.push_back(Category{name, {}});
    m_Selection.push_back(0);
    return (int)m_Categories.size() - 1;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void RingMenu::AddItem(int category, ImTextureID icon, const std::string& label)
{
    if (category < 0 || category >= (int)m_Categories.size())
        return;

    m_Categories[category].Items.push_back(Item{icon, label});
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void RingMenu::Open()
{
    m_IsOpen = true;
    m_AnimT  = 0.0f;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void RingMenu::Close()
{
    m_IsOpen = false;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int RingMenu::GetCurrentCategory() const
{ return m_CurrentCategory; }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int RingMenu::GetSelectedIndex(int category) const
{
    if (category < 0 || category >= m_Selection.size())
        return 0;

    return m_Selection[category];
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void RingMenu::Draw()
{
    auto& io = ImGui::GetIO();

    //-----------------------------------------------------
    // (1) Open / Close 入力
    //-----------------------------------------------------
    if (!m_IsOpen && ImGui::IsKeyPressed(ImGuiKey_Space))
    {
        Open();
    }
    else if (m_IsOpen && ImGui::IsKeyPressed(ImGuiKey_Escape))
    {
        Close();
    }

    //-----------------------------------------------------
    // (2) Open / Close アニメ
    //-----------------------------------------------------
    float delta = io.DeltaTime * 5.0f;

    if (m_IsOpen)
        m_AnimT = std::min(1.0f, m_AnimT + delta);
    else
        m_AnimT = std::max(0.0f, m_AnimT - delta);

    if (m_AnimT <= 0.0f)
        return; // 消えたら描画しない

    //-----------------------------------------------------
    // (3) カテゴリ切替（↑↓）
    //-----------------------------------------------------
    if (!m_CategoryChanging && m_Categories.size() > 1)
    {
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
        {
            m_TargetCategory   = (int)((m_CurrentCategory - 1 + m_Categories.size()) % m_Categories.size());
            m_CategoryChanging = true;
            m_CategoryAnimT    = 0.0f;
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
        {
            m_TargetCategory   = (int)((m_CurrentCategory + 1) % m_Categories.size());
            m_CategoryChanging = true;
            m_CategoryAnimT    = 0.0f;
        }
    }

    if (m_CategoryChanging)
    {
        m_CategoryAnimT += io.DeltaTime * 6.0f;
        if (m_CategoryAnimT >= 1.0f)
        {
            m_CategoryAnimT   = 1.0f;
            m_CategoryChanging = false;
            m_CurrentCategory  = m_TargetCategory;
        }
    }

    //-----------------------------------------------------
    // (4) 項目選択（左右）
    //-----------------------------------------------------
    auto& category = m_Categories[m_CurrentCategory];
    int& sel = m_Selection[m_CurrentCategory];
    int itemCount = (int)category.Items.size();

    if (!m_CategoryChanging && itemCount > 0)
    {
        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
            sel = (sel - 1 + itemCount) % itemCount;
        else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
            sel = (sel + 1) % itemCount;
    }

    //-----------------------------------------------------
    // (5) 描画
    //-----------------------------------------------------
    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);

    // Enterアニメ（聖剣2風：外側から内へ移動する）
    float radius = m_RadiusBase * EaseOutBack(m_AnimT);

    //-----------------------------------------------------
    // カテゴリ切替アニメ割合
    //-----------------------------------------------------
    float ct = m_CategoryAnimT;
    float easeCt = EaseOutBack(ct);

    int prevCat = m_CurrentCategory;
    int nextCat = m_TargetCategory;

    // 切替アニメ中
    if (m_CategoryChanging)
    {
        DrawCategoryRing(draw, center, radius, prevCat, 1.0f - easeCt);
        DrawCategoryRing(draw, center, radius, nextCat, easeCt);
    }
    else
    {
        DrawCategoryRing(draw, center, radius, m_CurrentCategory, 1.0f);
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
float RingMenu::EaseOutBack(float t)
{
    float c1 = 1.70158f;
    float c3 = c1 + 1.0f;
    return 1.0f + c3 * powf(t - 1.0f, 3.0f) + c1 * powf(t - 1.0f, 2.0f);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void RingMenu::DrawCategoryRing(ImDrawList* draw, ImVec2 center, float radius, int catIndex, float scale)
{
    if (catIndex < 0 || catIndex >= (int)m_Categories.size())
        return;

    const auto& cat = m_Categories[catIndex];
    int count = (int)cat.Items.size();
    if (count == 0) 
        return;

    //=========================================
    // ★ 半円にするかどうかを決める
    //=========================================
    const int MaxFullRingItems = 12;  // 好みで調整
    bool useHalfRing = (count > MaxFullRingItems);

    float startAng, endAng;

    if (!useHalfRing)
    {
        // 通常：360°リング
        startAng = 0.0f;
        endAng   = IM_PI * 2.0f;
    }
    else
    {
        // 多すぎる場合：180°リング（上半円）
        //  -90° 〜 +90°
        startAng = -IM_PI * 0.5f;
        endAng   = +IM_PI * 0.5f;
    }

    // 角度幅
    float arc = endAng - startAng;

    //=========================================
    //  項目の配置
    //=========================================
    for (int i = 0; i < count; i++)
    {
        float t = (float)i / (float)std::max(count - 1, 1); // 等間隔
        float ang = startAng + arc * t;

        // 半円モード時、下側の描画は不要（念のため Y<0 を条件にするなら）
        if (useHalfRing && sinf(ang) < 0.0f)
            continue;

        float px = center.x + cosf(ang) * radius * scale;
        float py = center.y + sinf(ang) * radius * scale;

        const auto& it = cat.Items[i];
        float iconSize = 48.0f * scale;

        // 通常カラー.
        ImU32 col = IM_COL32(255,255,255,220);

        // 選択時カラーに.
        if (catIndex == m_CurrentCategory && i == m_Selection[catIndex])
            col = IM_COL32(255,255,0,255);

        // アイコン
        if (it.Icon)
        {
            draw->AddImage(
                it.Icon,
                ImVec2(px - iconSize * 0.5f, py - iconSize * 0.5f),
                ImVec2(px + iconSize * 0.5f, py + iconSize * 0.5f),
                ImVec2(0, 0),
                ImVec2(1, 1),
                col);
        }

        // ラベル
        draw->AddText(
            ImVec2(px - 20, py + iconSize * 0.6f),
            IM_COL32(255,255,255,255),
            it.Label.c_str());
    }
}

} // namspace asdx

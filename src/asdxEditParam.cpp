//-----------------------------------------------------------------------------
// File : asdxEditParam.cpp
// Desc : Edit Parameter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxEditParam.h>
#include <asdxParamHistory.h>
#include <asdxAppHistoryMgr.h>

#ifdef ASDX_ENABLE_IMGUI
#include <imgui.h>
#endif//ASDX_ENABLE_IMGUI


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// EditBool class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditBool::EditBool(bool value)
: m_Value(value)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      値を設定します.
//-----------------------------------------------------------------------------
void EditBool::SetValue(bool value, bool history)
{
    if (!history)
    {
        m_Value = value;
        return;
    }

    if (value == m_Value)
    { return; }

    AppHistoryMgr::GetInstance().Add(CreateHistory(value));
}

//-----------------------------------------------------------------------------
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditBool::CreateHistory(bool value)
{ return new ParamHistory<bool>(&m_Value, value); }

//-----------------------------------------------------------------------------
//      値を取得します.
//-----------------------------------------------------------------------------
bool EditBool::GetValue() const
{ return m_Value; }

//-----------------------------------------------------------------------------
//      値へのポインタ取得します.
//-----------------------------------------------------------------------------
const bool* EditBool::GetValuePtr() const
{ return &m_Value; }

#ifdef ASDX_ENABLE_IMGUI
//-----------------------------------------------------------------------------
//      チェックボックスを描画します.
//-----------------------------------------------------------------------------
void EditBool::DrawCheckbox(const char* tag)
{
    auto prev = m_Value;
    if (ImGui::Checkbox(tag, &m_Value))
    {
        AppHistoryMgr::GetInstance().Add(new ParamHistory<bool>(&m_Value, m_Value, prev), false);
    }
}
#endif//ASDX_ENABLE_IMGUI

#ifdef ASDX_ENABLE_TINYXML2
//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* EditBool::Serialize(tinyxml2::XMLDocument* doc, const char* tag)
{
    auto element = doc->NewElement(tag);
    element->SetAttribute("value", m_Value);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void EditBool::Deserialize(tinyxml2::XMLElement* element, const char* tag)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    m_Value = e->BoolAttribute("value");
    element = e;
}
#endif//ASDX_ENABLE_TINYXML2


///////////////////////////////////////////////////////////////////////////////
// EditInt class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditInt::EditInt(int value)
: m_Value   (value)
#ifdef ASDX_ENABLE_IMGUI
, m_Prev    (value)
, m_Dragged (false)
#endif//ASDX_ENABLE_IMGUI
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      値を設定します.
//-----------------------------------------------------------------------------
void EditInt::SetValue(int value, bool history)
{
    if (!history)
    {
        m_Value = value;
        return;
    }

    if (m_Value == value)
    { return; }

    AppHistoryMgr::GetInstance().Add(CreateHistory(value));
}

//-----------------------------------------------------------------------------
//      値を取得します.
//-----------------------------------------------------------------------------
int EditInt::GetValue() const
{ return m_Value; }

//-----------------------------------------------------------------------------
//      値へのポインタを取得します.
//-----------------------------------------------------------------------------
const int* EditInt::GetValuePtr() const
{ return &m_Value; }

//-----------------------------------------------------------------------------
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditInt::CreateHistory(int value)
{ return new ParamHistory<int>(&m_Value, m_Value, value); }

#ifdef ASDX_ENABLE_IMGUI
//-----------------------------------------------------------------------------
//      スライダーを描画します.
//-----------------------------------------------------------------------------
void EditInt::DrawSlider(const char* tag, int step, int mini, int maxi)
{
    auto prev = m_Value;
    auto flag = ImGui::DragInt(tag, &m_Value, float(step), mini, maxi);

    if (!ImGui::IsMouseDragging(0) && !ImGui::IsMouseDown(0))
    {
        if (!m_Dragged && !ImGui::IsItemActive())
        {
            // 一度もドラッグされておらず，アイテムもいじられていない場合.
            m_Prev = m_Value;
        }
        else if (m_Dragged)
        {
            // マウスドラッグ終了時.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<int>(&m_Value, m_Value, m_Prev), false);
            m_Dragged = false;
        }
        else if (flag)
        {
            // キーボード入力.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<int>(&m_Value, m_Value, m_Prev), false);
        }
    }
    else if (ImGui::IsItemActive())
    {
        m_Dragged = ImGui::IsMouseDragging();
    }
}

//-----------------------------------------------------------------------------
//      コンボボックスを描画します.
//-----------------------------------------------------------------------------
void EditInt::DrawCombo(const char* tag, int count, const char** items)
{
    auto value = m_Value;
    if (ImGui::Combo(tag, &value, items, count))
    {
        AppHistoryMgr::GetInstance().Add(new ParamHistory<int>(&m_Value, value, m_Prev));
    }
}

//-----------------------------------------------------------------------------
//      コンボボックスを描画します.
//-----------------------------------------------------------------------------
void EditInt::DrawCombo(const char* tag, bool (*items_getter)(void* data, int idx, const char** out_text), int count)
{
    auto value = m_Value;
    if (ImGui::Combo(tag, &value, items_getter, &value, count))
    {
        AppHistoryMgr::GetInstance().Add(new ParamHistory<int>(&m_Value, value, m_Prev));
    }
}

#endif//ASDX_ENABLE_IMGUI

#ifdef ASDX_ENABLE_TINYXML2
//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* EditInt::Serialize(tinyxml2::XMLDocument* doc, const char* tag)
{
    auto element = doc->NewElement(tag);
    element->SetAttribute("value", m_Value);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void EditInt::Deserialize(tinyxml2::XMLElement* element, const char* tag)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    m_Value = m_Prev = e->IntAttribute("value");
    element = e;
}
#endif//ASDX_ENABLE_TINYXML2


///////////////////////////////////////////////////////////////////////////////
// EditFloat class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditFloat::EditFloat(float value)
: m_Value  (value)
#ifdef ASDX_ENABLE_IMGUI
, m_Prev   (0.0f)
, m_Dragged(false)
#endif//ASDX_ENABLE_IMGUI
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      値を設定します.
//-----------------------------------------------------------------------------
void EditFloat::SetValue(float value, bool history)
{
    if (!history)
    {
        m_Value = value;
        return;
    }

    if (m_Value == value)
    { return; }

    AppHistoryMgr::GetInstance().Add(CreateHistory(value));
}

//-----------------------------------------------------------------------------
//      値を取得します.
//-----------------------------------------------------------------------------
float EditFloat::GetValue() const
{ return m_Value; }

//-----------------------------------------------------------------------------
//      値へのポインタを取得します.
//-----------------------------------------------------------------------------
const float* EditFloat::GetValuePtr() const
{ return &m_Value; }

//-----------------------------------------------------------------------------
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditFloat::CreateHistory(float value)
{ return new ParamHistory<float>(&m_Value, value); }

#ifdef ASDX_ENABLE_IMGUI
//-----------------------------------------------------------------------------
//      スライダーを描画します.
//-----------------------------------------------------------------------------
void EditFloat::DrawSlider(const char* tag, float step, float mini, float maxi)
{
    auto prev = m_Value;
    auto flag = ImGui::DragFloat(tag, &m_Value, step, mini, maxi, "%.5f");

    if (!ImGui::IsMouseDragging(0) && !ImGui::IsMouseDown(0))
    {
        if (!m_Dragged && !ImGui::IsItemActive())
        {
            // 一度もドラッグされておらず，アイテムもいじられていない場合.
            m_Prev = m_Value;
        }
        else if (m_Dragged)
        {
            // マウスドラッグ終了時.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<float>(&m_Value, m_Value, m_Prev), false);
            m_Dragged = false;
        }
        else if (flag)
        {
            // キーボード入力.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<float>(&m_Value, m_Value, m_Prev), false);
        }
    }
    else if (ImGui::IsItemActive())
    {
        m_Dragged = ImGui::IsMouseDragging();
    }
}
#endif//ASDX_ENABLE_IMGUI

#ifdef ASDX_ENABLE_TINYXML2
//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* EditFloat::Serialize(tinyxml2::XMLDocument* doc, const char* tag)
{
    auto element = doc->NewElement(tag);
    element->SetAttribute("value", m_Value);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void EditFloat::Deserialize(tinyxml2::XMLElement* element, const char* tag)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    m_Value = m_Prev = e->FloatAttribute("value");
}
#endif//ASDX_ENABLE_TINYXML2


///////////////////////////////////////////////////////////////////////////////
// EditFloat2 class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditFloat2::EditFloat2(float x, float y)
: m_Value  (x, y)
#ifdef ASDX_ENABLE_IMGUI
, m_Prev   (x, y)
, m_Dragged(false)
#endif//ASDX_ENABLE_IMGUI
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      値を設定します.
//-----------------------------------------------------------------------------
void EditFloat2::SetValue(const asdx::Vector2& value, bool history)
{
    if (!history)
    {
        m_Value = value;
        return;
    }

    if (m_Value == value)
    { return; }

    AppHistoryMgr::GetInstance().Add(CreateHistory(value));
}

//-----------------------------------------------------------------------------
//      値を取得します.
//-----------------------------------------------------------------------------
const asdx::Vector2& EditFloat2::GetValue() const
{ return m_Value; }

//-----------------------------------------------------------------------------
//      値へのポインタを取得します.
//-----------------------------------------------------------------------------
const asdx::Vector2* EditFloat2::GetValuePtr() const
{ return &m_Value; }

//-----------------------------------------------------------------------------
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditFloat2::CreateHistory(const asdx::Vector2& value)
{ return new ParamHistory<asdx::Vector2>(&m_Value, value); }

#ifdef ASDX_ENABLE_IMGUI
//-----------------------------------------------------------------------------
//      スライダーを描画します.
//-----------------------------------------------------------------------------
void EditFloat2::DrawSlider(const char* tag, float step, float mini, float maxi)
{
    auto prev = m_Value;
    auto flag = ImGui::DragFloat2(tag, m_Value, step, mini, maxi);

    if (!ImGui::IsMouseDragging(0) && !ImGui::IsMouseDown(0))
    {
        if (!m_Dragged && !ImGui::IsItemActive())
        {
            // 一度もドラッグされておらず，アイテムもいじられていない場合.
            m_Prev = m_Value;
        }
        else if (m_Dragged)
        {
            // マウスドラッグ終了時.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector2>(&m_Value, m_Value, m_Prev), false);
            m_Dragged = false;
        }
        else if (flag)
        {
            // キーボード入力.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector2>(&m_Value, m_Value, m_Prev), false);
        }
    }
    else if (ImGui::IsItemActive())
    {
        m_Dragged = ImGui::IsMouseDragging();
    }
}
#endif//ASDX_ENABLE_IMGUI

#ifdef ASDX_ENABLE_TINYXML2
//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* EditFloat2::Serialize(tinyxml2::XMLDocument* doc, const char* tag)
{
    auto element = doc->NewElement(tag);
    element->SetAttribute("x", m_Value.x);
    element->SetAttribute("y", m_Value.y);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void EditFloat2::Deserialize(tinyxml2::XMLElement* element, const char* tag)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    m_Value.x = m_Prev.x = e->FloatAttribute("x");
    m_Value.y = m_Prev.y = e->FloatAttribute("y");
}
#endif//ASDX_ENABLE_TINYXML2


///////////////////////////////////////////////////////////////////////////////
// EditFloat3 class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditFloat3::EditFloat3(float x, float y, float z)
: m_Value   (x, y, z)
#ifdef ASDX_ENABLE_IMGUI
, m_Prev    (x, y, z)
, m_Dragged (false)
#endif//ASDX_ENABLE_IMGUI
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      値を設定します.
//-----------------------------------------------------------------------------
void EditFloat3::SetValue(const asdx::Vector3& value, bool history)
{
    if (!history)
    {
        m_Value = value;
        return;
    }

    if (m_Value == value)
    { return; }

    AppHistoryMgr::GetInstance().Add(CreateHistory(value));
}

//-----------------------------------------------------------------------------
//      値を取得します.
//-----------------------------------------------------------------------------
const asdx::Vector3& EditFloat3::GetValue() const
{ return m_Value; }

//-----------------------------------------------------------------------------
//      値へのポインタを取得します.
//-----------------------------------------------------------------------------
const asdx::Vector3* EditFloat3::GetValuePtr() const
{ return &m_Value; }

//-----------------------------------------------------------------------------
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditFloat3::CreateHistory(const asdx::Vector3& value)
{ return new ParamHistory<asdx::Vector3>(&m_Value, value); }

#ifdef ASDX_ENABLE_IMGUI
//-----------------------------------------------------------------------------
//      スライダーを描画します.
//-----------------------------------------------------------------------------
void EditFloat3::DrawSlider(const char* tag, float step, float mini, float maxi)
{
    auto prev = m_Value;
    auto flag = ImGui::DragFloat3(tag, m_Value, step, mini, maxi);

    if (!ImGui::IsMouseDragging(0) && !ImGui::IsMouseDown(0))
    {
        if (!m_Dragged && !ImGui::IsItemActive())
        {
            // 一度もドラッグされておらず，アイテムもいじられていない場合.
            m_Prev = m_Value;
        }
        else if (m_Dragged)
        {
            // マウスドラッグ終了時.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector3>(&m_Value, m_Value, m_Prev), false);
            m_Dragged = false;
        }
        else if (flag)
        {
            // キーボード入力.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector3>(&m_Value, m_Value, m_Prev), false);
        }
    }
    else if (ImGui::IsItemActive())
    {
        m_Dragged = ImGui::IsMouseDragging();
    }
}
#endif//ASDX_ENABLE_IMGUI

#ifdef ASDX_ENABLE_TINYXML2
//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* EditFloat3::Serialize(tinyxml2::XMLDocument* doc, const char* tag)
{
    auto element = doc->NewElement(tag);
    element->SetAttribute("x", m_Value.x);
    element->SetAttribute("y", m_Value.y);
    element->SetAttribute("z", m_Value.z);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void EditFloat3::Deserialize(tinyxml2::XMLElement* element, const char* tag)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    m_Value.x = m_Prev.x = e->FloatAttribute("x");
    m_Value.y = m_Prev.y = e->FloatAttribute("y");
    m_Value.z = m_Prev.z = e->FloatAttribute("z");
}
#endif//ASDX_ENABLE_TINYXML2


///////////////////////////////////////////////////////////////////////////////
// EditFloat4 class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditFloat4::EditFloat4(float x, float y, float z, float w)
: m_Value   (x, y, z, w)
#ifdef ASDX_ENABLE_IMGUI
, m_Prev    (x, y, z, w)
, m_Dragged (false)
#endif//ASDX_ENABLE_IMGUI
{ /* DO_NOTHING */ }


//-----------------------------------------------------------------------------
//      値を設定します.
//-----------------------------------------------------------------------------
void EditFloat4::SetValue(const asdx::Vector4& value, bool history)
{
    if (!history)
    {
        m_Value = value;
        return;
    }

    if (m_Value == value)
    { return; }

    AppHistoryMgr::GetInstance().Add(CreateHistory(value));
}

//-----------------------------------------------------------------------------
//      値を取得します.
//-----------------------------------------------------------------------------
const asdx::Vector4& EditFloat4::GetValue() const
{ return m_Value; }

//-----------------------------------------------------------------------------
//      値へのポインタを取得します.
//-----------------------------------------------------------------------------
const asdx::Vector4* EditFloat4::GetValuePtr() const
{ return &m_Value; }

//-----------------------------------------------------------------------------
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditFloat4::CreateHistory(const asdx::Vector4& value)
{ return new ParamHistory<asdx::Vector4>(&m_Value, value); }

#ifdef ASDX_ENABLE_IMGUI
//-----------------------------------------------------------------------------
//      スライダーを描画します.
//-----------------------------------------------------------------------------
void EditFloat4::DrawSlider(const char* tag, float step, float mini, float maxi)
{
    auto prev = m_Value;
    auto flag = ImGui::DragFloat4(tag, m_Value, step, mini, maxi);

    if (!ImGui::IsMouseDragging(0) && !ImGui::IsMouseDown(0))
    {
        if (!m_Dragged && !ImGui::IsItemActive())
        {
            // 一度もドラッグされておらず，アイテムもいじられていない場合.
            m_Prev = m_Value;
        }
        else if (m_Dragged)
        {
            // マウスドラッグ終了時.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector4>(&m_Value, m_Value, m_Prev), false);
            m_Dragged = false;
        }
        else if (flag)
        {
            // キーボード入力.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector4>(&m_Value, m_Value, m_Prev), false);
        }
    }
    else if (ImGui::IsItemActive())
    {
        m_Dragged = ImGui::IsMouseDragging();
    }
}
#endif

#ifdef ASDX_ENABLE_TINYXML2
//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* EditFloat4::Serialize(tinyxml2::XMLDocument* doc, const char* tag)
{
    auto element = doc->NewElement(tag);
    element->SetAttribute("x", m_Value.x);
    element->SetAttribute("y", m_Value.y);
    element->SetAttribute("z", m_Value.z);
    element->SetAttribute("w", m_Value.w);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void EditFloat4::Deserialize(tinyxml2::XMLElement* element, const char* tag)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    m_Value.x = m_Prev.x = e->FloatAttribute("x");
    m_Value.y = m_Prev.y = e->FloatAttribute("y");
    m_Value.z = m_Prev.z = e->FloatAttribute("z");
    m_Value.w = m_Prev.w = e->FloatAttribute("w");
}
#endif//ASDX_ENABLE_TINYXML2


///////////////////////////////////////////////////////////////////////////////
// EditColor3 class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditColor3::EditColor3(float r, float g, float b)
: m_Value   (r, g, b)
#ifdef ASDX_ENABLE_IMGUI
, m_Prev    (r, g, b)
, m_Dragged (false)
#endif//ASDX_ENABLE_IMGUI
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      値を設定します.
//-----------------------------------------------------------------------------
void EditColor3::SetValue(const asdx::Vector3& value, bool history)
{
    if (!history)
    {
        m_Value = value;
        return;
    }

    if (m_Value == value)
    { return; }

    AppHistoryMgr::GetInstance().Add(CreateHistory(value));
}

//-----------------------------------------------------------------------------
//      値を取得します.
//-----------------------------------------------------------------------------
const asdx::Vector3& EditColor3::GetValue() const
{ return m_Value; }

//-----------------------------------------------------------------------------
//      値へのポインタを取得します.
//-----------------------------------------------------------------------------
const asdx::Vector3* EditColor3::GetValuePtr() const
{ return &m_Value; }

//-----------------------------------------------------------------------------
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditColor3::CreateHistory(const asdx::Vector3& value)
{ return new ParamHistory<asdx::Vector3>(&m_Value, value); }

#ifdef ASDX_ENABLE_IMGUI
//-----------------------------------------------------------------------------
//      スライダーを描画します.
//-----------------------------------------------------------------------------
void EditColor3::DrawPicker(const char* tag)
{
    auto prev = m_Value;
    auto flag = ImGui::ColorEdit3(tag, m_Value, ImGuiColorEditFlags_Float);

    if (!ImGui::IsMouseDragging(0) && !ImGui::IsMouseDown(0))
    {
        if (!m_Dragged && !ImGui::IsItemActive())
        {
            // 一度もドラッグされておらず，アイテムもいじられていない場合.
            m_Prev = m_Value;
        }
        else if (m_Dragged)
        {
            // マウスドラッグ終了時.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector3>(&m_Value, m_Value, m_Prev), false);
            m_Dragged = false;
        }
        else if (flag)
        {
            // キーボード入力.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector3>(&m_Value, m_Value, m_Prev), false);
        }
    }
    else if (ImGui::IsItemActive())
    {
        m_Dragged = ImGui::IsMouseDragging(0);
    }
}
#endif//ASDX_ENABLE_IMGUI

#ifdef ASDX_ENABLE_TINYXML2
//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* EditColor3::Serialize(tinyxml2::XMLDocument* doc, const char* tag)
{
    auto element = doc->NewElement(tag);
    element->SetAttribute("r", m_Value.x);
    element->SetAttribute("g", m_Value.y);
    element->SetAttribute("b", m_Value.z);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void EditColor3::Deserialize(tinyxml2::XMLElement* element, const char* tag)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    m_Value.x = m_Prev.x = e->FloatAttribute("r");
    m_Value.y = m_Prev.y = e->FloatAttribute("g");
    m_Value.z = m_Prev.z = e->FloatAttribute("b");
}
#endif//ASDX_ENABLE_TINYXML2


///////////////////////////////////////////////////////////////////////////////
// EditColor4 class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditColor4::EditColor4(float r, float g, float b, float a)
: m_Value   (r, g, b, a)
#ifdef ASDX_ENABLE_IMGUI
, m_Prev    (r, g, b, a)
, m_Dragged (false)
#endif//ASDX_ENABLE_IMGUI
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      値を設定します.
//-----------------------------------------------------------------------------
void EditColor4::SetValue(const asdx::Vector4& value, bool history)
{
    if (!history)
    {
        m_Value = value;
        return;
    }

    if (m_Value == value)
    { return; }

    AppHistoryMgr::GetInstance().Add(CreateHistory(value));
}

//-----------------------------------------------------------------------------
//      値を取得します.
//-----------------------------------------------------------------------------
const asdx::Vector4& EditColor4::GetValue() const
{ return m_Value; }

//-----------------------------------------------------------------------------
//      値へのポインタを取得します.
//-----------------------------------------------------------------------------
const asdx::Vector4* EditColor4::GetValuePtr() const
{ return &m_Value; }

//-----------------------------------------------------------------------------
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditColor4::CreateHistory(const asdx::Vector4& value)
{ return new ParamHistory<asdx::Vector4>(&m_Value, value); }

#ifdef ASDX_ENABLE_IMGUI
//-----------------------------------------------------------------------------
//      スライダーを描画します.
//-----------------------------------------------------------------------------
void EditColor4::DrawPicker(const char* tag)
{
    auto prev = m_Value;
    auto flag = ImGui::ColorEdit4(tag, m_Value, ImGuiColorEditFlags_Float);

    if (!ImGui::IsMouseDragging(0) && !ImGui::IsMouseDown(0))
    {
        if (!m_Dragged && !ImGui::IsItemActive())
        {
            // 一度もドラッグされておらず，アイテムもいじられていない場合.
            m_Prev = m_Value;
        }
        else if (m_Dragged)
        {
            // マウスドラッグ終了時.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector4>(&m_Value, m_Value, m_Prev), false);
            m_Dragged = false;
        }
        else if (flag)
        {
            // キーボード入力.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector4>(&m_Value, m_Value, m_Prev), false);
        }
    }
    else if (ImGui::IsItemActive())
    {
        m_Dragged = ImGui::IsMouseDragging(0);
    }
}
#endif//ASDX_ENABLE_IMGUI

#ifdef ASDX_ENABLE_TINYXML2
//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* EditColor4::Serialize(tinyxml2::XMLDocument* doc, const char* tag)
{
    auto element = doc->NewElement(tag);
    element->SetAttribute("r", m_Value.x);
    element->SetAttribute("g", m_Value.y);
    element->SetAttribute("b", m_Value.z);
    element->SetAttribute("a", m_Value.w);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void EditColor4::Deserialize(tinyxml2::XMLElement* element, const char* tag)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    m_Value.x = m_Prev.x = e->FloatAttribute("r");
    m_Value.y = m_Prev.y = e->FloatAttribute("g");
    m_Value.z = m_Prev.z = e->FloatAttribute("b");
    m_Value.w = m_Prev.w = e->FloatAttribute("a");
}
#endif//ASDX_ENABLE_TINYXML2

} // namespace asdx

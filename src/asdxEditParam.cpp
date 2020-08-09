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


#ifndef ASDX_UNUSED
#define ASDX_UNUSED(x) ((void)x)
#endif//ASDX_UNUSED


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
//      チェックボックスを描画します.
//-----------------------------------------------------------------------------
void EditBool::DrawCheckbox(const char* tag)
{
#ifdef ASDX_ENABLE_IMGUI
    auto prev = m_Value;
    if (ImGui::Checkbox(tag, &m_Value))
    {
        AppHistoryMgr::GetInstance().Add(new ParamHistory<bool>(&m_Value, m_Value, prev), false);
    }
#else
    ASDX_UNUSED(tag);
#endif
}


///////////////////////////////////////////////////////////////////////////////
// EditInt class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditInt::EditInt(int value)
: m_Value   (value)
, m_Prev    (value)
, m_Dragged (false)
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
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditInt::CreateHistory(int value)
{ return new ParamHistory<int>(&m_Value, m_Value, value); }

//-----------------------------------------------------------------------------
//      スライダーを描画します.
//-----------------------------------------------------------------------------
void EditInt::DrawSlider(const char* tag, int step, int mini, int maxi)
{
#ifdef ASDX_ENABLE_IMGUI
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
        m_Dragged = ImGui::IsMouseDragging(0);
    }
#else
    ASDX_UNUSED(tag);
    ASDX_UNUSED(step);
    ASDX_UNUSED(mini);
    ASDX_UNUSED(maxi);
#endif
}

//-----------------------------------------------------------------------------
//      コンボボックスを描画します.
//-----------------------------------------------------------------------------
void EditInt::DrawCombo(const char* tag, int count, const char** items)
{
#ifdef ASDX_ENABLE_IMGUI
    auto value = m_Value;
    if (ImGui::Combo(tag, &value, items, count))
    {
        AppHistoryMgr::GetInstance().Add(new ParamHistory<int>(&m_Value, value, m_Prev));
    }
#else
    ASDX_UNUSED(tag);
    ASDX_UNUSED(count);
    ASDX_UNUSED(items);
#endif
}

//-----------------------------------------------------------------------------
//      コンボボックスを描画します.
//-----------------------------------------------------------------------------
void EditInt::DrawCombo(const char* tag, bool (*items_getter)(void* data, int idx, const char** out_text), int count)
{
#ifdef ASDX_ENABLE_IMGUI
    auto value = m_Value;
    if (ImGui::Combo(tag, &value, items_getter, &value, count))
    {
        AppHistoryMgr::GetInstance().Add(new ParamHistory<int>(&m_Value, value, m_Prev));
    }
#else
    ASDX_UNUSED(tag);
    ASDX_UNUSED(items_getter);
#endif
}


///////////////////////////////////////////////////////////////////////////////
// EditFloat class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditFloat::EditFloat(float value)
: m_Value  (value)
, m_Prev   (0.0f)
, m_Dragged(false)
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
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditFloat::CreateHistory(float value)
{ return new ParamHistory<float>(&m_Value, value); }

//-----------------------------------------------------------------------------
//      スライダーを描画します.
//-----------------------------------------------------------------------------
void EditFloat::DrawSlider(const char* tag, float step, float mini, float maxi)
{
#ifdef ASDX_ENABLE_IMGUI
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
        m_Dragged = ImGui::IsMouseDragging(0);
    }
#else
    ASDX_UNUSED(tag);
    ASDX_UNUSED(step);
    ASDX_UNUSED(mini);
    ASDX_UNUSED(maxi);
#endif
}


///////////////////////////////////////////////////////////////////////////////
// EditFloat2 class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditFloat2::EditFloat2(float x, float y)
: m_Value  (x, y)
, m_Prev   (x, y)
, m_Dragged(false)
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
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditFloat2::CreateHistory(const asdx::Vector2& value)
{ return new ParamHistory<asdx::Vector2>(&m_Value, value); }

//-----------------------------------------------------------------------------
//      スライダーを描画します.
//-----------------------------------------------------------------------------
void EditFloat2::DrawSlider(const char* tag, float step, float mini, float maxi)
{
#ifdef ASDX_ENABLE_IMGUI
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
        m_Dragged = ImGui::IsMouseDragging(0);
    }
#else
    ASDX_UNUSED(tag);
    ASDX_UNUSED(step);
    ASDX_UNUSED(mini);
    ASDX_UNUSED(maxi);
#endif
}


///////////////////////////////////////////////////////////////////////////////
// EditFloat3 class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditFloat3::EditFloat3(float x, float y, float z)
: m_Value   (x, y, z)
, m_Prev    (x, y, z)
, m_Dragged (false)
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
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditFloat3::CreateHistory(const asdx::Vector3& value)
{ return new ParamHistory<asdx::Vector3>(&m_Value, value); }

//-----------------------------------------------------------------------------
//      スライダーを描画します.
//-----------------------------------------------------------------------------
void EditFloat3::DrawSlider(const char* tag, float step, float mini, float maxi)
{
#ifdef ASDX_ENABLE_IMGUI
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
        m_Dragged = ImGui::IsMouseDragging(0);
    }
#else
    ASDX_UNUSED(tag);
    ASDX_UNUSED(step);
    ASDX_UNUSED(mini);
    ASDX_UNUSED(maxi);
#endif
}


///////////////////////////////////////////////////////////////////////////////
// EditFloat4 class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditFloat4::EditFloat4(float x, float y, float z, float w)
: m_Value   (x, y, z, w)
, m_Prev    (x, y, z, w)
, m_Dragged (false)
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
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditFloat4::CreateHistory(const asdx::Vector4& value)
{ return new ParamHistory<asdx::Vector4>(&m_Value, value); }

//-----------------------------------------------------------------------------
//      スライダーを描画します.
//-----------------------------------------------------------------------------
void EditFloat4::DrawSlider(const char* tag, float step, float mini, float maxi)
{
#ifdef ASDX_ENABLE_IMGUI
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
        m_Dragged = ImGui::IsMouseDragging(0);
    }
#else
    ASDX_UNUSED(tag);
    ASDX_UNUSED(step);
    ASDX_UNUSED(mini);
    ASDX_UNUSED(maxi);
#endif
}


///////////////////////////////////////////////////////////////////////////////
// EditColor3 class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditColor3::EditColor3(float r, float g, float b)
: m_Value   (r, g, b)
, m_Prev    (r, g, b)
, m_Dragged (false)
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
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditColor3::CreateHistory(const asdx::Vector3& value)
{ return new ParamHistory<asdx::Vector3>(&m_Value, value); }

//-----------------------------------------------------------------------------
//      スライダーを描画します.
//-----------------------------------------------------------------------------
void EditColor3::DrawPicker(const char* tag)
{
#ifdef ASDX_ENABLE_IMGUI
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
#else
    ASDX_UNUSED(tag);
#endif
}


///////////////////////////////////////////////////////////////////////////////
// EditColor4 class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditColor4::EditColor4(float r, float g, float b, float a)
: m_Value   (r, g, b, a)
, m_Prev    (r, g, b, a)
, m_Dragged (false)
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
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditColor4::CreateHistory(const asdx::Vector4& value)
{ return new ParamHistory<asdx::Vector4>(&m_Value, value); }

//-----------------------------------------------------------------------------
//      スライダーを描画します.
//-----------------------------------------------------------------------------
void EditColor4::DrawPicker(const char* tag)
{
#ifdef ASDX_ENABLE_IMGUI
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
#else
    ASDX_UNUSED(tag);
#endif
}

} // namespace asdx


#ifdef ASDX_ENABLE_TINYXML2
namespace asdx {

//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditorBool& control)
{
    auto value = control.GetValue();
    auto element = doc->NewElement(tag);
    element->SetAttribute("value", value);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditorInt& control)
{
    auto value = control.GetValue();
    auto element = doc->NewElement(tag);
    element->SetAttribute("value", value);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditorFloat& control)
{
    auto value = control.GetValue();
    auto element = doc->NewElement(tag);
    element->SetAttribute("value", value);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditorFloat2& control)
{
    auto value = control.GetValue();
    auto element = doc->NewElement(tag);
    element->SetAttribute("x", value.x);
    element->SetAttribute("y", value.y);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditorFloat3& control)
{
    auto value = control.GetValue();
    auto element = doc->NewElement(tag);
    element->SetAttribute("x", value.x);
    element->SetAttribute("y", value.y);
    element->SetAttribute("z", value.z);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditorFloat4& control)
{
    auto value = control.GetValue();
    auto element = doc->NewElement(tag);
    element->SetAttribute("x", value.x);
    element->SetAttribute("y", value.y);
    element->SetAttribute("z", value.z);
    element->SetAttribute("w", value.w);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditColor3& control)
{
    auto value = control.GetValue();
    auto element = doc->NewElement(tag);
    element->SetAttribute("r", value.x);
    element->SetAttribute("g", value.y);
    element->SetAttribute("b", value.z);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditColor4& control)
{
    auto value = control.GetValue();
    auto element = doc->NewElement(tag);
    element->SetAttribute("r", value.x);
    element->SetAttribute("g", value.y);
    element->SetAttribute("b", value.z);
    element->SetAttribute("a", value.w);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditorBool& contorl)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    auto value = e->BoolAttribute("value");
    control = EditorBool(value);
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditorInt& contorl)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    auto value = e->IntAttribute("value");
    control = EditorInt(value);
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditorFloat& contorl)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    auto value = e->FloatAttribute("value");
    control = EditorFloat(value);
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditorFloat2& contorl)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    asdx::Vector2 value;
    value.x = e->FloatAttribute("x");
    value.y = e->FloatAttribute("y");
    control = EditorFloat2(value.x, value.y);
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditorFloat3& contorl)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    asdx::Vector3 value;
    value.x = e->FloatAttribute("x");
    value.y = e->FloatAttribute("y");
    value.z = e->FloatAttribute("z");
    control = EditorFloat3(value.x, value.y, value.z);
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditorFloat4& contorl)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    asdx::Vector4 value;
    value.x = e->FloatAttribute("x");
    value.y = e->FloatAttribute("y");
    value.z = e->FloatAttribute("z");
    value.w = e->FloatAttribute("w");
    control = EditorFloat4(value.x, value.y, value.z, value.w);
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditorColor3& control)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    asdx::Vector3 value;
    value.x = e->FloatAttribute("r");
    value.y = e->FloatAttribute("g");
    value.z = e->FloatAttribute("b");
    control = EditColor3(value.x, value.y, value.z);
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void EditColor4::Deserialize(tinyxml2::XMLElement* element, const char* tag, EditorColor4& control)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    asdx::Vector4 value;
    value.x = e->FloatAttribute("r");
    value.y = e->FloatAttribute("g");
    value.z = e->FloatAttribute("b");
    value.w = e->FloatAttribute("a");
    control = EditorColor4(value.x, value.y, value.z, value.w);
}

} // namespace asdx
#endif//ASDX_ENABLE_TINYXML2

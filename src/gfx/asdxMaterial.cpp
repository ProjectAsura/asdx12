//-----------------------------------------------------------------------------
// File : asdxMaterial.cpp
// Desc : Material.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <fnd/asdxMisc.h>
#include <fnd/asdxHash.h>
#include <gfx/asdxMaterial.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Material class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Material::Material()
: m_Hash            (0)
, m_State           (MATERIAL_STATE_OPAQUE)
, m_DisplayFace     (DISPLAY_FACE_BOTH)
, m_ShadowCast      (false)
, m_ShadowReceive   (false)
, m_Dirty           (false)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Material::~Material()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Material::Init(const ResMaterial& value)
{
    m_Hash          = CalcHash(value.Name.c_str());
    m_State         = MATERIAL_STATE(value.State);
    m_DisplayFace   = DISPLAY_FACE(value.DisplayFace);
    m_ShadowCast    = value.ShadowCast;
    m_ShadowReceive = value.ShadowReceive;
    m_Dirty         = false;
    m_Buffer        = value.Buffer;

    // パラメータを設定します.
    for(size_t i=0; i<value.Parameters.size(); ++i)
    {
        auto& p = value.Parameters[i];
        auto hash = CalcHash(value.Parameters[i].Name.c_str());
        m_Params[hash] = p;
    }

    // テクスチャ初期化.
    m_Textures.resize(value.Textures.size());
    for(size_t i=0; i<value.Textures.size(); ++i)
    {
        std::string path;
        if (!SearchFilePathA(value.Textures[i].Path.c_str(), path))
        {
            Term();
            return false;
        }

        ResTexture res;
        if (!res.LoadFromFileA(path.c_str()))
        {
            Term();
            return false;
        }

        if (!m_Textures[i].Texture.Init(res))
        {
            Term();
            return false;
        }

        m_Textures[i].Usage = value.Textures[i].Usage;
    }

    // 定数バッファ初期化.
    if (!m_CB.Init(value.Buffer.size()))
    {
        Term();
        return false;
    }

    // バッファ内容を更新.
    for(auto i=0u; i<2; ++i)
    {
        auto ptr = m_CB.Map(i);
        memcpy(ptr, m_Buffer.data(), m_Buffer.size());
        m_CB.Unmap(i);
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Material::Term()
{
    m_Hash          = 0;
    m_State         = MATERIAL_STATE_OPAQUE;
    m_DisplayFace   = DISPLAY_FACE_BOTH;
    m_ShadowCast    = false;
    m_ShadowReceive = false;
    m_Dirty         = false;

    m_CB.Term();
    m_Buffer.clear();
    m_Params.clear();

    for(size_t i=0; i<m_Textures.size(); ++i)
    { m_Textures[i].Texture.Term(); }
    m_Textures.clear();
}

//-----------------------------------------------------------------------------
//      マテリアル名ハッシュを取得します.
//-----------------------------------------------------------------------------
uint32_t Material::GetHash() const
{ return m_Hash; }

//-----------------------------------------------------------------------------
//      マテリアルステートを取得します.
//-----------------------------------------------------------------------------
MATERIAL_STATE Material::GetState() const
{ return m_State; }

//-----------------------------------------------------------------------------
//      キャストシャドウフラグを取得します.
//-----------------------------------------------------------------------------
bool Material::CastShadow() const
{ return m_ShadowCast; }

//-----------------------------------------------------------------------------
//      シャドウレシーブフラグを取得します.
//-----------------------------------------------------------------------------
bool Material::ReceiveShadow() const
{ return m_ShadowReceive; }

//-----------------------------------------------------------------------------
//      パラメータを設定します.
//-----------------------------------------------------------------------------
void Material::SetParam(uint32_t hash, bool value)
{
    if (m_Params.find(hash) == m_Params.end())
    { return; }

    auto p = m_Params.at(hash);
    if (p.Type != MATERIAL_PARAMETER_BOOL)
    { return; }

    auto v = value ? 1 : 0;
    memcpy(m_Buffer.data() + p.Offset, &v, sizeof(v));
    m_Dirty = true;
}

//-----------------------------------------------------------------------------
//      パラメータを設定します.
//-----------------------------------------------------------------------------
void Material::SetParam(uint32_t hash, int value)
{
    if (m_Params.find(hash) == m_Params.end())
    { return; }

    auto p = m_Params.at(hash);
    if (p.Type != MATERIAL_PARAMETER_INT)
    { return; }

    memcpy(m_Buffer.data() + p.Offset, &value, sizeof(value));
    m_Dirty = true;
}

//-----------------------------------------------------------------------------
//      パラメータを設定します.
//-----------------------------------------------------------------------------
void Material::SetParam(uint32_t hash, float value)
{
    if (m_Params.find(hash) == m_Params.end())
    { return; }

    auto p = m_Params.at(hash);
    if (p.Type != MATERIAL_PARAMETER_FLOAT)
    { return; }

    memcpy(m_Buffer.data() + p.Offset, &value, sizeof(value));
    m_Dirty = true;
}

//-----------------------------------------------------------------------------
//      パラメータを設定します.
//-----------------------------------------------------------------------------
void Material::SetParam(uint32_t hash, uint32_t value)
{
    if (m_Params.find(hash) == m_Params.end())
    { return; }

    auto p = m_Params.at(hash);
    if (p.Type != MATERIAL_PARAMETER_UINT)
    { return; }

    memcpy(m_Buffer.data() + p.Offset, &value, sizeof(value));
    m_Dirty = true;
}

//-----------------------------------------------------------------------------
//      パラメータを設定します.
//-----------------------------------------------------------------------------
void Material::SetParam(uint32_t hash, const Vector2& value)
{
    if (m_Params.find(hash) == m_Params.end())
    { return; }

    auto p = m_Params.at(hash);
    if (p.Type != MATERIAL_PARAMETER_FLOAT2)
    { return; }

    memcpy(m_Buffer.data() + p.Offset, &value, sizeof(value));
    m_Dirty = true;
}

//-----------------------------------------------------------------------------
//      パラメータを設定します.
//-----------------------------------------------------------------------------
void Material::SetParam(uint32_t hash, const Vector3& value)
{
    if (m_Params.find(hash) == m_Params.end())
    { return; }

    auto p = m_Params.at(hash);
    if (p.Type != MATERIAL_PARAMETER_FLOAT3)
    { return; }

    memcpy(m_Buffer.data() + p.Offset, &value, sizeof(value));
    m_Dirty = true;
}

//-----------------------------------------------------------------------------
//      パラメータを設定します.
//-----------------------------------------------------------------------------
void Material::SetParam(uint32_t hash, const Vector4& value)
{
    if (m_Params.find(hash) == m_Params.end())
    { return; }

    auto p = m_Params.at(hash);
    if (p.Type != MATERIAL_PARAMETER_FLOAT4)
    { return; }

    memcpy(m_Buffer.data() + p.Offset, &value, sizeof(value));
    m_Dirty = true;
}

//-----------------------------------------------------------------------------
//      パラメータを取得します.
//-----------------------------------------------------------------------------
bool Material::GetBool(uint32_t hash) const
{
    auto result = false;
    if (m_Params.find(hash) != m_Params.end())
    {
        auto p = m_Params.at(hash);
        if (p.Type == MATERIAL_PARAMETER_BOOL)
        {
            auto val = 0;
            memcpy(&val, m_Buffer.data() + p.Offset, sizeof(val));
            result = (val == 1);
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
//      パラメータを取得します.
//-----------------------------------------------------------------------------
int Material::GetInt(uint32_t hash) const
{
    auto result = 0;
    if (m_Params.find(hash) != m_Params.end())
    {
        auto p = m_Params.at(hash);
        if (p.Type == MATERIAL_PARAMETER_INT)
        { memcpy(&result, m_Buffer.data() + p.Offset, sizeof(result)); }
    }

    return result;
}

//-----------------------------------------------------------------------------
//      パラメータを取得します.
//-----------------------------------------------------------------------------
float Material::GetFloat(uint32_t hash) const
{
    auto result = 0.0f;
    if (m_Params.find(hash) != m_Params.end())
    {
        auto p = m_Params.at(hash);
        if (p.Type == MATERIAL_PARAMETER_FLOAT)
        { memcpy(&result, m_Buffer.data() + p.Offset, sizeof(result)); }
    }

    return result;
}

//-----------------------------------------------------------------------------
//      パラメータを取得します.
//-----------------------------------------------------------------------------
uint32_t Material::GetUint(uint32_t hash) const
{
    auto result = 0u;
    if (m_Params.find(hash) != m_Params.end())
    {
        auto p = m_Params.at(hash);
        if (p.Type == MATERIAL_PARAMETER_UINT)
        { memcpy(&result, m_Buffer.data() + p.Offset, sizeof(result)); }
    }

    return result;
}

//-----------------------------------------------------------------------------
//      パラメータを取得します.
//-----------------------------------------------------------------------------
Vector2 Material::GetVector2(uint32_t hash) const
{
    auto result = Vector2(0.0f, 0.0f);
    if (m_Params.find(hash) != m_Params.end())
    {
        auto p = m_Params.at(hash);
        if (p.Type == MATERIAL_PARAMETER_FLOAT2)
        { memcpy(&result, m_Buffer.data() + p.Offset, sizeof(result)); }
    }

    return result;
}

//-----------------------------------------------------------------------------
//      パラメータを取得します.
//-----------------------------------------------------------------------------
Vector3 Material::GetVector3(uint32_t hash) const
{
    auto result = Vector3(0.0f, 0.0f, 0.0f);
    if (m_Params.find(hash) != m_Params.end())
    {
        auto p = m_Params.at(hash);
        if (p.Type == MATERIAL_PARAMETER_FLOAT3)
        { memcpy(&result, m_Buffer.data() + p.Offset, sizeof(result)); }
    }

    return result;
}

//-----------------------------------------------------------------------------
//      パラメータを取得します.
//-----------------------------------------------------------------------------
Vector4 Material::GetVector4(uint32_t hash) const
{
    auto result = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
    if (m_Params.find(hash) != m_Params.end())
    {
        auto p = m_Params.at(hash);
        if (p.Type == MATERIAL_PARAMETER_FLOAT4)
        { memcpy(&result, m_Buffer.data() + p.Offset, sizeof(result)); }
    }

    return result;
}

//-----------------------------------------------------------------------------
//      定数バッファビューを取得します.
//-----------------------------------------------------------------------------
IConstantBufferView* Material::GetCBV() const
{ return m_CB.GetView(); }

//-----------------------------------------------------------------------------
//      テクスチャ数を取得します.
//-----------------------------------------------------------------------------
uint32_t Material::GetTextureCount() const
{ return uint32_t(m_Textures.size()); }

//-----------------------------------------------------------------------------
//      テクスチャを取得します.
//-----------------------------------------------------------------------------
MaterialTexture Material::GetTexture(uint32_t index) const
{
    MaterialTexture result = {};
    assert(index < GetTextureCount());

    result.Usage = m_Textures[index].Usage;
    result.pSRV  = m_Textures[index].Texture.GetView();
    return result;
}

//-----------------------------------------------------------------------------
//      更新処理を行います.
//-----------------------------------------------------------------------------
void Material::Update()
{
    if (!m_Dirty)
    { return; }

    m_CB.Update(m_Buffer.data(), m_Buffer.size());
    m_Dirty = false;
}

} // namespace asdx
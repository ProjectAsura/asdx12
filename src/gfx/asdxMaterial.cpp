//-----------------------------------------------------------------------------
// File : asdxMaterial.cpp
// Desc : Material.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include <gfx/asdxMaterial.h>
#include <gfx/asdxUpdateCommand.h>
#include <gfx/asdxTexture.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Material class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Material::Material()
: m_RefCount(1)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Material::~Material()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Material::Init(const ResMaterial& res)
{
    // 定数バッファ初期化.
    if (!m_Buffer.Init(res.Buffer.size()))
    {
        ELOGA("Error : ConstantBuffer Initialize Failed.");
        return false;
    }

    // シェーダ名.
    m_Shader.assign(res.Shader.c_str(), res.Shader.size());

    // 定数バッファ更新.
    if (res.Buffer.data() != nullptr)
    {
        auto ptr = m_Buffer.Map();
        memcpy(ptr, res.Buffer.data(), res.Buffer.size());
        m_Buffer.Unmap();
    }

    // テクスチャ初期化.
    m_Textures.resize(res.Textures.size());
    for(size_t i=0; i<res.Textures.size(); ++i)
    {
        m_Textures[i] = TextureManager::Instance().GetOrCreate(res.Textures[i].c_str());
        if (!m_Textures[i].IsValid())
        {
            ELOGA("Error : Texture Initialized Failed. index = %u, path = %s", i, res.Textures[i].c_str());
            return false;
        }
    }

    // アルファモード設定.
    m_AlphaMode = res.AlphaMode;

    // カリングモード設定.
    m_CullMode = res.CullMode;

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Material::Term()
{
    m_Buffer.Term();

    for(size_t i=0; i<m_Textures.size(); ++i)
    { m_Textures[i].Reset(); }

    m_Textures.clear();
    m_Shader  .clear();
}

//-----------------------------------------------------------------------------
//      参照カウンタを上げます.
//-----------------------------------------------------------------------------
void Material::AddRef()
{ m_RefCount++; }

//-----------------------------------------------------------------------------
//      解放処理を行います.
//-----------------------------------------------------------------------------
void Material::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-----------------------------------------------------------------------------
//      参照カウンタを取得します.
//-----------------------------------------------------------------------------
uint32_t Material::GetRefCount() const
{ return m_RefCount; }

//-----------------------------------------------------------------------------
//      シェーダ名を取得します.
//-----------------------------------------------------------------------------
StringView Material::GetShader() const
{ return StringView(m_Shader.c_str()); }

//-----------------------------------------------------------------------------
//      定数バッファを取得します.
//-----------------------------------------------------------------------------
const ConstantBuffer& Material::GetBuffer() const
{ return m_Buffer; }

//-----------------------------------------------------------------------------
//      テクスチャ数を取得します.
//-----------------------------------------------------------------------------
uint32_t Material::GetTextureCount() const
{ return uint32_t(m_Textures.size()); }

//-----------------------------------------------------------------------------
//      テクスチャを取得します.
//-----------------------------------------------------------------------------
const TextureHolder& Material::GetTexture(uint32_t index) const
{
    assert(index < m_Textures.size());
    return m_Textures[index];
}

//-----------------------------------------------------------------------------
//      アルファモードを設定します.
//-----------------------------------------------------------------------------
void Material::SetAlphaMode(AlphaMode value)
{ m_AlphaMode = value; }

//-----------------------------------------------------------------------------
//      アルファモードを取得します.
//-----------------------------------------------------------------------------
AlphaMode Material::GetAlphaMode() const
{ return m_AlphaMode; }

//-----------------------------------------------------------------------------
//      カリングモードを設定します.
//-----------------------------------------------------------------------------
void Material::SetCullMode(CullMode value)
{ m_CullMode = value; }

//-----------------------------------------------------------------------------
//      カリングモードを取得します.
//-----------------------------------------------------------------------------
CullMode Material::GetCullMode() const
{ return m_CullMode; }

//-----------------------------------------------------------------------------
//      マテリアルを生成します.
//-----------------------------------------------------------------------------
bool Material::Create(const ResMaterial& res, Material** ppMaterial)
{
    auto instance = new(std::nothrow) Material();
    if (instance == nullptr)
    {
        ELOGA("Error : Out of memory.");
        return false;
    }

    if (!instance->Init(res))
    {
        instance->Release();
        return false;
    }

    (*ppMaterial) = instance;
    return true;
}

} // namespace asdx

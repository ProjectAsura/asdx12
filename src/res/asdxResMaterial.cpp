//-----------------------------------------------------------------------------
// File : asdxResMaterial.cpp
// Desc : Material Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxMacro.h>
#include <fnd/asdxLogger.h>
#include <res/asdxResMaterial.h>
#include "MaterialBinary_generated.h"


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t CURRENT_VERSION = 1u;     //!< 現在ランタイムでサポートされているバージョン.

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// MaterialBinary class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
MaterialBinary::MaterialBinary()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
MaterialBinary::~MaterialBinary()
{ Term(); }

//-----------------------------------------------------------------------------
//      メモリからロードします.
//-----------------------------------------------------------------------------
void MaterialBinary::Load(std::vector<uint8_t>&& blob)
{
    m_Blob = std::move(blob);

#if ASDX_DEBUG
    // データ整合性をチェック.
    {
        assert(!m_Blob.empty());
        flatbuffers::Verifier::Options options;
        flatbuffers::Verifier verifier(m_Blob.data(), m_Blob.size(), options);
        assert(res::VerifyMaterialBinaryBuffer(verifier));
        ASDX_UNUSED(verifier);
    }
#endif
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void MaterialBinary::Term()
{
    m_Blob.clear();
    m_Blob.shrink_to_fit();
}

//-----------------------------------------------------------------------------
//      プロパティ数を取得します.
//-----------------------------------------------------------------------------
uint32_t MaterialBinary::GetPropertyCount() const
{
    assert(!m_Blob.empty());
    return res::GetMaterialBinary(m_Blob.data())->Props()->size();
}

//-----------------------------------------------------------------------------
//      テクスチャ数を取得します.
//-----------------------------------------------------------------------------
uint32_t MaterialBinary::GetTextureCount() const
{
    assert(!m_Blob.empty());
    return res::GetMaterialBinary(m_Blob.data())->Textures()->size();
}

//-----------------------------------------------------------------------------
//      ブレンドタイプを取得します.
//-----------------------------------------------------------------------------
MaterialBlendType MaterialBinary::GetBlendType() const
{
    assert(!m_Blob.empty());
    return MaterialBlendType(res::GetMaterialBinary(m_Blob.data())->Blend());
}

//-----------------------------------------------------------------------------
//      カリングタイプを取得します.
//-----------------------------------------------------------------------------
MaterialCullType MaterialBinary::GetCullType() const
{
    assert(!m_Blob.empty());
    return MaterialCullType(res::GetMaterialBinary(m_Blob.data())->Cull());
}

//-----------------------------------------------------------------------------
//      深度タイプを取得します.
//-----------------------------------------------------------------------------
MaterialDepthType MaterialBinary::GetDepthType() const
{
    assert(!m_Blob.empty());
    return MaterialDepthType(res::GetMaterialBinary(m_Blob.data())->Depth());
}

//-----------------------------------------------------------------------------
//      定数バッファサイズを取得します.
//-----------------------------------------------------------------------------
uint32_t MaterialBinary::GetBufferSize() const
{
    assert(!m_Blob.empty());
    return res::GetMaterialBinary(m_Blob.data())->BufferSize();
}

//-----------------------------------------------------------------------------
//      プロパティを取得します.
//-----------------------------------------------------------------------------
ResMaterialProperty MaterialBinary::GetProperty(uint32_t index) const
{
    assert(!m_Blob.empty());
    auto prop = res::GetMaterialBinary(m_Blob.data())->Props()->Get(index);

    ResMaterialProperty result;
    result.Name     = StringView(prop->Name()->c_str());
    result.Type     = MaterialDataType(prop->Type());
    result.Elements = prop->Elements();
    result.Offset   = prop->Offset();
    result.Value =   ArrayView<uint8_t>(prop->Value()->data(), prop->Value()->size());

    return result;
}

//-----------------------------------------------------------------------------
//      テクスチャを取得します.
//-----------------------------------------------------------------------------
ResMaterialTexture MaterialBinary::GetTexture(uint32_t index) const
{
    assert(!m_Blob.empty());
    auto tex = res::GetMaterialBinary(m_Blob.data())->Textures()->Get(index);

    ResMaterialTexture result;
    result.BindName = StringView(tex->BindName()->c_str());
    result.Path     = StringView(tex->Path()->c_str());
    return result;
}

//-----------------------------------------------------------------------------
//      プロパティを検索します.
//-----------------------------------------------------------------------------
bool MaterialBinary::FindProperty(const char* name, ResMaterialProperty& result) const
{
    assert(!m_Blob.empty());
    auto prop = res::GetMaterialBinary(m_Blob.data())->Props()->LookupByKey(name);
    if (prop == nullptr)
    { return false; }

    result.Name     = StringView(prop->Name()->c_str());
    result.Type     = MaterialDataType(prop->Type());
    result.Elements = prop->Elements();
    result.Offset   = prop->Offset();
    result.Value =   ArrayView<uint8_t>(prop->Value()->data(), prop->Value()->size());

    return true;
}

//-----------------------------------------------------------------------------
//      テクスチャを検索します.
//-----------------------------------------------------------------------------
bool MaterialBinary::FindTexture(const char* name, ResMaterialTexture& result) const
{
    assert(!m_Blob.empty());
    auto tex = res::GetMaterialBinary(m_Blob.data())->Textures()->LookupByKey(name);
    if (tex == nullptr)
    { return false; }

    result.BindName = StringView(tex->BindName()->c_str());
    result.Path     = StringView(tex->Path()->c_str());

    return true;
}

} // namespace asdx

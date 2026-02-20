//-----------------------------------------------------------------------------
// File : asdxResMaterial.cpp
// Desc : Material Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxMacro.h>
#include <res/asdxResMaterial.h>
#include "MaterialBinary_generated.h"


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t CURRENT_VERSION = 1u; //!< 現在ランタイムでサポートされているバージョン.

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
//      ロード処理を行います.
//-----------------------------------------------------------------------------
void MaterialBinary::Load(std::vector<uint8_t>&& blob)
{
    m_Blob = std::move(blob);

#if ASDX_DEBUG
    // データ整合性をチェック.
    {
        assert(!m_Blob.empty());
        flatbuffers::Verifier verifier(m_Blob.data(), m_Blob.size());
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
//      マテリアル種別を取得します.
//-----------------------------------------------------------------------------
uint32_t MaterialBinary::GetKind() const
{
    assert(!m_Blob.empty());
    return res::GetMaterialBinary(m_Blob.data())->Kind();
}

//-----------------------------------------------------------------------------
//      ブレンドステートを取得します.
//-----------------------------------------------------------------------------
MaterialBlendState MaterialBinary::GetBlendState() const
{
    assert(!m_Blob.empty());
    return MaterialBlendState(res::GetMaterialBinary(m_Blob.data())->BlendState());
}

//-----------------------------------------------------------------------------
//      深度ステートを取得します.
//-----------------------------------------------------------------------------
MaterialDepthState MaterialBinary::GetDepthState() const
{
    assert(!m_Blob.empty());
    return MaterialDepthState(res::GetMaterialBinary(m_Blob.data())->DepthState());
}

//-----------------------------------------------------------------------------
//      ラスタライザーステートを取得します.
//-----------------------------------------------------------------------------
MaterialRasterizerState MaterialBinary::GetRasterizerState() const
{
    assert(!m_Blob.empty());
    return MaterialRasterizerState(res::GetMaterialBinary(m_Blob.data())->RasterizerState());
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
//      バッファを取得します.
//-----------------------------------------------------------------------------
MaterialBuffer MaterialBinary::GetBuffer() const
{
    assert(!m_Blob.empty());
    auto buffer = res::GetMaterialBinary(m_Blob.data())->Buffer();

    MaterialBuffer result = {};
    result.Size  = buffer->Data()->size();
    result.pData = buffer->Data()->data();

    return result;
}

//-----------------------------------------------------------------------------
//      テクスチャを取得します.
//-----------------------------------------------------------------------------
MaterialTexture MaterialBinary::GetTexture(uint32_t index) const
{
    assert(!m_Blob.empty());
    auto textures = res::GetMaterialBinary(m_Blob.data())->Textures();
    assert(index < textures->size());
    auto tex = textures->Get(index);

    MaterialTexture result = {};
    result.Name = StringView(tex->Name()->c_str());
    result.Path = StringView(tex->Path()->c_str());

    return result;
}

//-----------------------------------------------------------------------------
//      テクスチャを検索します.
//-----------------------------------------------------------------------------
bool MaterialBinary::FindTexture(const char* name, uint32_t& index) const
{
    if (name == nullptr)
    {
        index = UINT32_MAX;
        return false;
    }

    assert(!m_Blob.empty());
    auto textures = res::GetMaterialBinary(m_Blob.data())->Textures();

    uint32_t lhs = 0;
    uint32_t rhs = textures->size();

    while(lhs < rhs)
    {
        uint32_t mid = lhs + (rhs - lhs) / 2u;
        auto tex = textures->Get(mid);
        auto ret = strcmp(tex->Name()->c_str(), name);
        if (ret == 0)
        {
            index = mid;
            return true;
        }
        else if (ret < 0)
        {
            lhs = mid + 1;
        }
        else
        {
            rhs = mid;
        }
    }

    index = UINT32_MAX;
    return false;
}

} // namespace asdx

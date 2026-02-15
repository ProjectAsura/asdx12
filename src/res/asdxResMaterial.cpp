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
//      バッファ数を取得します.
//-----------------------------------------------------------------------------
uint32_t MaterialBinary::GetBufferCount() const
{
    assert(!m_Blob.empty());
    return res::GetMaterialBinary(m_Blob.data())->Buffers()->size();
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
//      レンダーステートを取得します.
//-----------------------------------------------------------------------------
MaterialRenderState MaterialBinary::GetRenderState() const
{
    assert(!m_Blob.empty());
    auto state = res::GetMaterialBinary(m_Blob.data())->States();

    MaterialRenderState result = {};
    result.Blend        = MaterialBlendState(state->Blend());
    result.Depth        = MaterialDepthState(state->Depth());
    result.Rasterizer   = MaterialRasterizerState(state->Rasterizer());
    result.UserFlags    = state->UserFlags();

    return result;
}

//-----------------------------------------------------------------------------
//      バッファを取得します.
//-----------------------------------------------------------------------------
MaterialBuffer MaterialBinary::GetBuffer(uint32_t index) const
{
    assert(!m_Blob.empty());
    auto buffers = res::GetMaterialBinary(m_Blob.data())->Buffers();
    assert(index < buffers->size());
    auto buf = buffers->Get(index);

    MaterialBuffer result = {};
    result.Name  = StringView(buf->Name()->c_str());
    result.Size  = buf->Data()->size();
    result.pData = buf->Data()->data();

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
//      バッファを検索します.
//-----------------------------------------------------------------------------
bool MaterialBinary::FindBuffer(const char* name, uint32_t& index) const
{
    if (name == nullptr)
    {
        index = UINT32_MAX;
        return false;
    }

    assert(!m_Blob.empty());
    auto buffers = res::GetMaterialBinary(m_Blob.data())->Buffers();

    uint32_t lhs = 0;
    uint32_t rhs = buffers->size();

    while(lhs < rhs)
    {
        uint32_t mid = lhs + (rhs - lhs) / 2u;
        auto buf = buffers->Get(mid);
        auto ret = strcmp(buf->Name()->c_str(), name);
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

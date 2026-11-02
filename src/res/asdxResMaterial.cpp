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
#include <fnd/asdxPath.h>
#include <res/asdxResMaterial.h>
#include "MaterialBinary_generated.h"


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t CURRENT_VERSION = 1u; //!< 現在ランタイムでサポートされているバージョン.


static_assert((uint8_t)asdx::res::AlphaType_Opaque == (uint8_t)asdx::AlphaMode::Opaque);
static_assert((uint8_t)asdx::res::AlphaType_Mask   == (uint8_t)asdx::AlphaMode::Mask);
static_assert((uint8_t)asdx::res::AlphaType_Blend  == (uint8_t)asdx::AlphaMode::Blend);

static_assert((uint8_t)asdx::res::CullType_Back  == (uint8_t)asdx::CullMode::Back);
static_assert((uint8_t)asdx::res::CullType_Front == (uint8_t)asdx::CullMode::Front);
static_assert((uint8_t)asdx::res::CullType_None  == (uint8_t)asdx::CullMode::None);

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
void MaterialBinary::Load(std::vector<uint8_t>&& blob, const StringView& dir)
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

    auto bin = asdx::res::GetMaterialBinary(m_Blob.data());

    // バイナリバージョンをチェック.
    assert(bin->Version() == CURRENT_VERSION);

    // テクスチャ数取得.
    auto count = bin->Textures()->size();

    // ディレクトリ解決を行う.
    m_ResolvedPaths.resize(count);
    for(uint32_t i=0; i<count; ++i)
    {
        auto src = bin->Textures()->Get(i)->c_str();

        // デフォルトテクスチャは解決しない.
        if (strstr(src, "default.") != nullptr)
        {
            m_ResolvedPaths[i] = src;
            continue;
        }

        // ファイルパスを解決する.
        auto path = std::string(dir.c_str()) + "\\" + src;
        fs::path findPath;
        if (!SearchFilePath(path, findPath))
        {
            ELOGA("Error : SearchFilePath() Failed. path = %s", path.c_str());
            m_ResolvedPaths[i] = src;
            continue;
        }

        // 解決したパスを設定.
        m_ResolvedPaths[i] = findPath.string();
    }

    // テクスチャパスを設定.
    m_Textures.resize(count);
    for(uint32_t i=0; i<count; ++i)
    {
        m_Textures[i] = StringView(m_ResolvedPaths[i].c_str());;
    }
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void MaterialBinary::Term()
{
    m_Blob.clear();
    m_Blob.shrink_to_fit();

    m_Textures.clear();
    m_Textures.shrink_to_fit();

    m_ResolvedPaths.clear();
    m_ResolvedPaths.shrink_to_fit();
}

//-----------------------------------------------------------------------------
//      マテリアルリソースを取得します.
//-----------------------------------------------------------------------------
ResMaterial MaterialBinary::GetResource() const
{
    auto bin = asdx::res::GetMaterialBinary(m_Blob.data());

    ResMaterial result = {};
    result.Shader    = StringView(bin->Shader()->c_str());
    result.Buffer    = ArrayView<uint8_t>(bin->Buffer()->data(), bin->Buffer()->size());
    result.Textures  = ArrayView<StringView>(m_Textures.data(), m_Textures.size());
    result.AlphaMode = AlphaMode(bin->AlphaMode());
    result.CullMode  = CullMode(bin->CullMode());

    return result;
}

} // namespace asdx

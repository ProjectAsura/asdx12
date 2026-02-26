//-----------------------------------------------------------------------------
// File : asdxResTexture.cpp
// Desc : Texture Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes.
//-----------------------------------------------------------------------------
#include <fnd/asdxMacro.h>
#include <fnd/asdxLogger.h>
#include <res/asdxResTexture.h>
#include "TextureBinary_generated.h"


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t CURRENT_VERSION = 1u;     //!< 現在ランタイムでサポートされているバージョン.

// そのままキャストするのでサイズが一致することを確認.
static_assert(sizeof (asdx::ResSubResource) == sizeof (asdx::res::SubResource));
static_assert(alignof(asdx::ResSubResource) == alignof(asdx::res::SubResource));

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// TextureBinary class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
TextureBinary::TextureBinary()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
TextureBinary::~TextureBinary()
{ Term(); }

//-----------------------------------------------------------------------------
//      ファイルからロードします.
//-----------------------------------------------------------------------------
void TextureBinary::Load(std::vector<uint8_t>&& blob)
{
    m_Blob = std::move(blob);
#if ASDX_DEBUG
    // データ整合性をチェック.
    {
        assert(!m_Blob.empty());
        flatbuffers::Verifier verifier(m_Blob.data(), m_Blob.size());
        assert(res::VerifyTextureBinaryBuffer(verifier));
        ASDX_UNUSED(verifier);
    }
#endif
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void TextureBinary::Term()
{
    m_Blob.clear();
    m_Blob.shrink_to_fit();
}

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ResTexture TextureBinary::GetResource() const
{
    assert(!m_Blob.empty());
    auto pTextureBinary = res::GetTextureBinary(m_Blob.data());

    ResTexture result = {};
    result.Dimension        = TEXTURE_DIMENSION(pTextureBinary->Dimension());
    result.Width            = pTextureBinary->Width();
    result.Height           = pTextureBinary->Height();
    result.DepthOrArraySize = pTextureBinary->DepthOrArraySize();
    result.MipLevels        = pTextureBinary->MipLevels();
    result.Format           = pTextureBinary->Format();
    result.SubResources     = ArrayView(reinterpret_cast<const ResSubResource*>(pTextureBinary->SubResources()->data()), pTextureBinary->SubResources()->size());
    result.Pixels           = ArrayView(pTextureBinary->Texels()->data(), pTextureBinary->Texels()->size());

    return result;
}

} // namespace asdx

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

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// TextureBinary class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
TextureBinary::TextureBinary()
: m_pBlob(nullptr)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
TextureBinary::~TextureBinary()
{ Term(); }

//-----------------------------------------------------------------------------
//      ファイルからロードします.
//-----------------------------------------------------------------------------
bool TextureBinary::LoadA(const char* path)
{
    if (!ReadFileToBlobA(path, &m_pBlob))
    { return false; }

#if ASDX_DEBUG
    // データ整合性をチェック.
    {
        assert(m_pBlob != nullptr);
        flatbuffers::Verifier::Options options;
        flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(m_pBlob->GetBuffer()), m_pBlob->GetBufferSize(), options);
        assert(res::VerifyTextureBinaryBuffer(verifier));
        ASDX_UNUSED(verifier);
    }
#endif

    return true;
}

//-----------------------------------------------------------------------------
//      ファイルからロードします.
//-----------------------------------------------------------------------------
bool TextureBinary::LoadW(const wchar_t* path)
{
    if (!ReadFileToBlobW(path, &m_pBlob))
    { return false; }

#if ASDX_DEBUG
    // データ整合性をチェック.
    {
        assert(m_pBlob != nullptr);
        flatbuffers::Verifier::Options options;
        flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(m_pBlob->GetBuffer()), m_pBlob->GetBufferSize(), options);
        assert(res::VerifyTextureBinaryBuffer(verifier));
        ASDX_UNUSED(verifier);
    }
#endif

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void TextureBinary::Term()
{
    if (m_pBlob != nullptr)
    {
        m_pBlob->Release();
        m_pBlob = nullptr;
    }
}

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ResTexture TextureBinary::GetResource() const
{
    assert(m_pBlob != nullptr);
    assert(m_pBlob->GetBuffer() != nullptr);
    auto pTextureBinary = res::GetTextureBinary(m_pBlob->GetBuffer());

    ResTexture result = {};
    result.Dimension        = TEXTURE_DIMENSION(pTextureBinary->Dimension());
    result.Width            = pTextureBinary->Width();
    result.Height           = pTextureBinary->Height();
    result.DepthOrArraySize = pTextureBinary->DepthOrArraySize();
    result.MipLevels        = pTextureBinary->MipLevels();
    result.Format           = pTextureBinary->Format();
    result.SubResourceCount = pTextureBinary->Subresources()->size();

    uint64_t   offset = 0;
    const auto texels = pTextureBinary->Texels()->data();
    for(auto i=0u; i<result.SubResourceCount; ++i)
    {
        const auto res = pTextureBinary->Subresources()->Get(i);

        result.SubResources[i].Width        = res->Width();
        result.SubResources[i].Height       = res->Height();
        result.SubResources[i].RowPitch     = res->RowPitch();
        result.SubResources[i].SlicePitch   = res->SlicePitch();
        result.SubResources[i].pPixels      = texels + offset;

        offset += result.SubResources[i].SlicePitch;
    }

    return result;
}

} // namespace asdx

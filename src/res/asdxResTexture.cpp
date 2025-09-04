//-----------------------------------------------------------------------------
// File : asdxResTexture.cpp
// Desc : Texture Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes.
//-----------------------------------------------------------------------------
#include <cstdio>
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
: m_pBinary(nullptr)
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
    void*  pBuffer    = nullptr;
    size_t bufferSize = 0;

    // テクスチャファイル(*.txb)をロード.
    {
        if (path == nullptr)
        {
            ELOG("Error : Invalid Argument.");
            return false;
        }

        FILE* fp = nullptr;
        auto err = fopen_s(&fp, path, "rb");
        if (err != 0 || fp == nullptr)
        {
            ELOG("Error : File Open Failed. path = %s", path);
            return false;
        }

        auto begin = ftell(fp);
        fseek(fp, 0, SEEK_END);
        auto end = ftell(fp);
        bufferSize = end - begin;
        fseek(fp, 0, SEEK_SET);

        pBuffer = malloc(bufferSize);
        if (pBuffer == nullptr)
        {
            fclose(fp);
            ELOG("Error : Out of Memory.");
            return false;
        }

        fread(pBuffer, bufferSize, 1, fp);
        fclose(fp);
    }

    return LoadFromMemory(pBuffer, bufferSize);
}

//-----------------------------------------------------------------------------
//      ファイルからロードします.
//-----------------------------------------------------------------------------
bool TextureBinary::LoadW(const wchar_t* path)
{
    void*  pBuffer    = nullptr;
    size_t bufferSize = 0;

    // テクスチャファイル(*.txb)をロード.
    {
        if (path == nullptr)
        {
            ELOG("Error : Invalid Argument.");
            return false;
        }

        FILE* fp = nullptr;
        auto err = _wfopen_s(&fp, path, L"rb");
        if (err != 0 || fp == nullptr)
        {
            ELOG("Error : File Open Failed. path = %ls", path);
            return false;
        }

        auto begin = ftell(fp);
        fseek(fp, 0, SEEK_END);
        auto end = ftell(fp);
        bufferSize = end - begin;
        fseek(fp, 0, SEEK_SET);

        pBuffer = malloc(bufferSize);
        if (pBuffer == nullptr)
        {
            fclose(fp);
            ELOG("Error : Out of Memory.");
            return false;
        }

        fread(pBuffer, bufferSize, 1, fp);
        fclose(fp);
    }

    return LoadFromMemory(pBuffer, bufferSize);
}

//-----------------------------------------------------------------------------
//      メモリからロードします.
//-----------------------------------------------------------------------------
bool TextureBinary::LoadFromMemory(void* pBinary, [[maybe_unused]] size_t binarySize)
{
#if ASDX_DEBUG
    // データ整合性をチェック.
    {
        flatbuffers::Verifier::Options options;
        [[maybe_unused]] flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(pBinary), binarySize, options);
        assert(res::VerifyTextureBinaryBuffer(verifier));
    }
#endif

    m_pBinary = pBinary;
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void TextureBinary::Term()
{
    if (m_pBinary != nullptr)
    {
        free(m_pBinary);
        m_pBinary = nullptr;
    }
}

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ResTexture TextureBinary::GetResource() const
{
    assert(m_pBinary != nullptr);
    auto pTextureBinary = res::GetTextureBinary(m_pBinary);

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

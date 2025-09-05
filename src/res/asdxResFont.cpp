//-----------------------------------------------------------------------------
// File : asdxResFont.cpp
// Desc : Font Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdio>
#include <fnd/asdxMacro.h>
#include <fnd/asdxLogger.h>
#include <res/asdxResFont.h>
#include "FontBinary_generated.h"


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t CURRENT_VERSION = 1u; //!< 現在ランタイムでサポートされているバージョン.

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// FontBinary class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
FontBinary::FontBinary()
: m_pBinary(nullptr)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
FontBinary::~FontBinary()
{ Term(); }

//-----------------------------------------------------------------------------
//      ファイルからロードします.
//-----------------------------------------------------------------------------
bool FontBinary::LoadA(const char* path)
{
    void*  pBuffer    = nullptr;
    size_t bufferSize = 0;

    // フォントファイル(*.fnb)をロード.
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
bool FontBinary::LoadW(const wchar_t* path)
{
    void*  pBuffer    = nullptr;
    size_t bufferSize = 0;

    // フォントファイル(*.fnb)をロード.
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
bool FontBinary::LoadFromMemory(void* pBinary, [[maybe_unused]] size_t binarySize)
{
#if ASDX_DEBUG
    // データ整合性をチェック.
    {
        flatbuffers::Verifier::Options options;
        [[maybe_unused]] flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(pBinary), binarySize, options);
        assert(res::VerifySizePrefixedFontBinaryBuffer(verifier));
    }
#endif

    m_pBinary = pBinary;
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void FontBinary::Term()
{
    if (m_pBinary != nullptr)
    {
        free(m_pBinary);
        m_pBinary = nullptr;
    }
}

//-----------------------------------------------------------------------------
//      ピクセル距離範囲を取得します.
//-----------------------------------------------------------------------------
float FontBinary::GetDistanceRange() const
{
    assert(m_pBinary != nullptr);
    return res::GetFontBinary(m_pBinary)->DistanceRange();
}

//-----------------------------------------------------------------------------
//      フォントサイズを取得します.
//-----------------------------------------------------------------------------
uint32_t FontBinary::GetFontSize() const
{
    assert(m_pBinary != nullptr);
    return res::GetFontBinary(m_pBinary)->FontSize();
}

//-----------------------------------------------------------------------------
//      em単位のサイズを取得します.
//-----------------------------------------------------------------------------
float FontBinary::GetEmSize() const
{
    assert(m_pBinary != nullptr);
    return res::GetFontBinary(m_pBinary)->EmSize();
}

//-----------------------------------------------------------------------------
//      1行あたりの縦幅を取得します.
//-----------------------------------------------------------------------------
float FontBinary::GetLineHeight() const
{
    assert(m_pBinary != nullptr);
    return res::GetFontBinary(m_pBinary)->LineHeight();
}

//-----------------------------------------------------------------------------
//      mean lineより上側を取得します.
//-----------------------------------------------------------------------------
float FontBinary::GetAscender() const
{
    assert(m_pBinary != nullptr);
    return res::GetFontBinary(m_pBinary)->Ascender();
}

//-----------------------------------------------------------------------------
//      mean lineより下側を取得します.
//-----------------------------------------------------------------------------
float FontBinary::GetDescender() const
{
    assert(m_pBinary != nullptr);
    return res::GetFontBinary(m_pBinary)->Descender();
}

//-----------------------------------------------------------------------------
//      Y方向を反転するかチェックします.
//-----------------------------------------------------------------------------
bool FontBinary::IsFlipY() const
{
    assert(m_pBinary != nullptr);
    return res::GetFontBinary(m_pBinary)->FlipY();
}

//-----------------------------------------------------------------------------
//      テクスチャの横幅を取得します.
//-----------------------------------------------------------------------------
uint32_t FontBinary::GetTextureWidth() const
{
    assert(m_pBinary != nullptr);
    return res::GetFontBinary(m_pBinary)->TextureWidth();
}

//-----------------------------------------------------------------------------
//      テクスチャの縦幅を取得します.
//-----------------------------------------------------------------------------
uint32_t FontBinary::GetTextureHeight() const
{
    assert(m_pBinary != nullptr);
    return res::GetFontBinary(m_pBinary)->TextureHeight();
}

//-----------------------------------------------------------------------------
//      テクスチャフォーマットを取得します.
//-----------------------------------------------------------------------------
DXGI_FORMAT FontBinary::GetTextureFormat() const
{
    assert(m_pBinary != nullptr);
    return DXGI_FORMAT(res::GetFontBinary(m_pBinary)->TextureFormat());
}

//-----------------------------------------------------------------------------
//      テクセルサイズを取得します.
//-----------------------------------------------------------------------------
size_t FontBinary::GetTexelSize() const
{
    assert(m_pBinary != nullptr);
    return res::GetFontBinary(m_pBinary)->Texels()->size();
}

//-----------------------------------------------------------------------------
//      テクセルデータを取得します.
//-----------------------------------------------------------------------------
const uint8_t* FontBinary::GetTexels() const
{
    assert(m_pBinary != nullptr);
    return res::GetFontBinary(m_pBinary)->Texels()->data();
}

//-----------------------------------------------------------------------------
//      グリフを検索します.
//-----------------------------------------------------------------------------
bool FontBinary::FindGlyph(uint32_t unicode, ResGlyph& result) const
{
    assert(m_pBinary != nullptr);
    auto glyph = res::GetFontBinary(m_pBinary)->Glyphs()->LookupByKey(unicode);
    if (glyph == nullptr)
    { return false; }

    auto& atlasBound = glyph->AtlasBound();
    auto& planeBound = glyph->PlaneBound();

    result.Unicode = glyph->Unicode();
    result.Advance = glyph->Advance();

    result.AtlasBound.Left      = atlasBound.Left();
    result.AtlasBound.Right     = atlasBound.Right();
    result.AtlasBound.Top       = atlasBound.Top();
    result.AtlasBound.Bottom    = atlasBound.Bottom();

    result.PlaneBound.Left      = planeBound.Left();
    result.PlaneBound.Right     = planeBound.Right();
    result.PlaneBound.Top       = planeBound.Top();
    result.PlaneBound.Bottom    = planeBound.Bottom();

    return true;
}

} // namespace asdx

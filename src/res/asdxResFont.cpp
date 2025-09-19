//-----------------------------------------------------------------------------
// File : asdxResFont.cpp
// Desc : Font Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
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
: m_Blob()
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
    if (!m_Blob.LoadA(path))
    { return false; }

#if ASDX_DEBUG
    // データ整合性をチェック.
    {
        flatbuffers::Verifier::Options options;
        flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(m_Blob.GetBuffer()), m_Blob.GetBufferSize(), options);
        assert(res::VerifyFontBinaryBuffer(verifier));
        ASDX_UNUSED(verifier);
    }
#endif

    return true;
}

//-----------------------------------------------------------------------------
//      ファイルからロードします.
//-----------------------------------------------------------------------------
bool FontBinary::LoadW(const wchar_t* path)
{
    if (!m_Blob.LoadW(path))
    { return false; }

#if ASDX_DEBUG
    // データ整合性をチェック.
    {
        flatbuffers::Verifier::Options options;
        flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(m_Blob.GetBuffer()), m_Blob.GetBufferSize(), options);
        assert(res::VerifyFontBinaryBuffer(verifier));
        ASDX_UNUSED(verifier);
    }
#endif

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void FontBinary::Term()
{ m_Blob.Term(); }

//-----------------------------------------------------------------------------
//      ピクセル距離範囲を取得します.
//-----------------------------------------------------------------------------
float FontBinary::GetDistanceRange() const
{
    assert(m_Blob.GetBuffer() != nullptr);
    return res::GetFontBinary(m_Blob.GetBuffer())->DistanceRange();
}

//-----------------------------------------------------------------------------
//      フォントサイズを取得します.
//-----------------------------------------------------------------------------
uint32_t FontBinary::GetFontSize() const
{
    assert(m_Blob.GetBuffer() != nullptr);
    return res::GetFontBinary(m_Blob.GetBuffer())->FontSize();
}

//-----------------------------------------------------------------------------
//      1行あたりの縦幅を取得します.
//-----------------------------------------------------------------------------
float FontBinary::GetLineHeight() const
{
    assert(m_Blob.GetBuffer() != nullptr);
    return res::GetFontBinary(m_Blob.GetBuffer())->LineHeight();
}

//-----------------------------------------------------------------------------
//      mean lineより上側を取得します.
//-----------------------------------------------------------------------------
float FontBinary::GetAscender() const
{
    assert(m_Blob.GetBuffer() != nullptr);
    return res::GetFontBinary(m_Blob.GetBuffer())->Ascender();
}

//-----------------------------------------------------------------------------
//      mean lineより下側を取得します.
//-----------------------------------------------------------------------------
float FontBinary::GetDescender() const
{
    assert(m_Blob.GetBuffer() != nullptr);
    return res::GetFontBinary(m_Blob.GetBuffer())->Descender();
}

//-----------------------------------------------------------------------------
//      テクスチャの横幅を取得します.
//-----------------------------------------------------------------------------
uint32_t FontBinary::GetWidth() const
{
    assert(m_Blob.GetBuffer() != nullptr);
    return res::GetFontBinary(m_Blob.GetBuffer())->Width();
}

//-----------------------------------------------------------------------------
//      テクスチャの縦幅を取得します.
//-----------------------------------------------------------------------------
uint32_t FontBinary::GetHeight() const
{
    assert(m_Blob.GetBuffer() != nullptr);
    return res::GetFontBinary(m_Blob.GetBuffer())->Height();
}

//-----------------------------------------------------------------------------
//      テクスチャの1行あたりのサイズを取得します.
//-----------------------------------------------------------------------------
uint32_t FontBinary::GetRowPitch() const
{
    assert(m_Blob.GetBuffer() != nullptr);
    return res::GetFontBinary(m_Blob.GetBuffer())->RowPitch();
}

//-----------------------------------------------------------------------------
//      テクスチャのスライスサイズを取得します.
//-----------------------------------------------------------------------------
uint32_t FontBinary::GetSlicePitch() const
{
    assert(m_Blob.GetBuffer() != nullptr);
    return res::GetFontBinary(m_Blob.GetBuffer())->SlicePitch();
}

//-----------------------------------------------------------------------------
//      テクスチャフォーマットを取得します.
//-----------------------------------------------------------------------------
DXGI_FORMAT FontBinary::GetFormat() const
{
    assert(m_Blob.GetBuffer() != nullptr);
    return DXGI_FORMAT(res::GetFontBinary(m_Blob.GetBuffer())->TextureFormat());
}

//-----------------------------------------------------------------------------
//      テクセルデータを取得します.
//-----------------------------------------------------------------------------
const uint8_t* FontBinary::GetTexels() const
{
    assert(m_Blob.GetBuffer() != nullptr);
    return res::GetFontBinary(m_Blob.GetBuffer())->Texels()->data();
}

//-----------------------------------------------------------------------------
//      グリフを検索します.
//-----------------------------------------------------------------------------
bool FontBinary::FindGlyph(uint32_t unicode, ResGlyph& result) const
{
    assert(m_Blob.GetBuffer() != nullptr);
    auto glyph = res::GetFontBinary(m_Blob.GetBuffer())->Glyphs()->LookupByKey(unicode);
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

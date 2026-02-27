//-----------------------------------------------------------------------------
// File : asdxResFont.cpp
// Desc : Font Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxMacro.h>
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
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
FontBinary::~FontBinary()
{ Term(); }

//-----------------------------------------------------------------------------
//      メモリからロードします.
//-----------------------------------------------------------------------------
void FontBinary::Load(std::vector<uint8_t>&& blob)
{
    m_Blob = std::move(blob);

#if ASDX_DEBUG
    // データ整合性をチェック.
    {
        assert(!m_Blob.empty());
        flatbuffers::Verifier verifier(m_Blob.data(), m_Blob.size());
        assert(res::VerifyFontBinaryBuffer(verifier));
        ASDX_UNUSED(verifier);
    }
#endif
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void FontBinary::Term()
{
    m_Blob.clear();
    m_Blob.shrink_to_fit();
}

//-----------------------------------------------------------------------------
//      ピクセル距離範囲を取得します.
//-----------------------------------------------------------------------------
float FontBinary::GetDistanceRange() const
{
    assert(!m_Blob.empty());
    return res::GetFontBinary(m_Blob.data())->DistanceRange();
}

//-----------------------------------------------------------------------------
//      フォントサイズを取得します.
//-----------------------------------------------------------------------------
uint32_t FontBinary::GetFontSize() const
{
    assert(!m_Blob.empty());
    return res::GetFontBinary(m_Blob.data())->FontSize();
}

//-----------------------------------------------------------------------------
//      1行あたりの縦幅を取得します.
//-----------------------------------------------------------------------------
float FontBinary::GetLineHeight() const
{
    assert(!m_Blob.empty());
    return res::GetFontBinary(m_Blob.data())->LineHeight();
}

//-----------------------------------------------------------------------------
//      mean lineより上側を取得します.
//-----------------------------------------------------------------------------
float FontBinary::GetAscender() const
{
    assert(!m_Blob.empty());
    return res::GetFontBinary(m_Blob.data())->Ascender();
}

//-----------------------------------------------------------------------------
//      mean lineより下側を取得します.
//-----------------------------------------------------------------------------
float FontBinary::GetDescender() const
{
    assert(!m_Blob.empty());
    return res::GetFontBinary(m_Blob.data())->Descender();
}

//-----------------------------------------------------------------------------
//      テクスチャの横幅を取得します.
//-----------------------------------------------------------------------------
uint32_t FontBinary::GetWidth() const
{
    assert(!m_Blob.empty());
    return res::GetFontBinary(m_Blob.data())->Texture()->Width();
}

//-----------------------------------------------------------------------------
//      テクスチャの縦幅を取得します.
//-----------------------------------------------------------------------------
uint32_t FontBinary::GetHeight() const
{
    assert(!m_Blob.empty());
    return res::GetFontBinary(m_Blob.data())->Texture()->Height();
}

//-----------------------------------------------------------------------------
//      テクスチャを取得します.
//-----------------------------------------------------------------------------
ResTexture FontBinary::GetTexture() const
{
    assert(!m_Blob.empty());
    auto texture = res::GetFontBinary(m_Blob.data())->Texture();

    ResTexture result = {};
    result.Dimension        = TEXTURE_DIMENSION(texture->Dimension());
    result.Width            = texture->Width();
    result.Height           = texture->Height();
    result.DepthOrArraySize = texture->DepthOrArraySize();
    result.Format           = texture->Format();
    result.MipLevels        = texture->MipLevels();
    result.SubResources     = ArrayView(reinterpret_cast<const ResSubResource*>(texture->SubResources()->data()), texture->SubResources()->size());
    result.Pixels           = ArrayView(texture->Texels()->data(), texture->Texels()->size());

    return result;
}

//-----------------------------------------------------------------------------
//      グリフを検索します.
//-----------------------------------------------------------------------------
bool FontBinary::FindGlyph(uint32_t unicode, ResGlyph& result) const
{
    assert(!m_Blob.empty());
    auto glyph = res::GetFontBinary(m_Blob.data())->Glyphs()->LookupByKey(unicode);
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

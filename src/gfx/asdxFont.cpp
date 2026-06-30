//-----------------------------------------------------------------------------
// File : asdxFont.cpp
// Desc : Font Rasterizer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <cstdarg>
#include <fnd/asdxLogger.h>
#include <res/asdxResTexture.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxFont.h>
#include <gfx/asdxSprite.h>
#include <gfx/asdxTextureManager.h>


#define ASDX_FONT_FORMATTING            \
    assert(format != nullptr);          \
    char buffer[1024] = {};             \
    va_list arg;                        \
    va_start(arg, format);              \
    vsprintf_s(buffer, format, arg);    \
    va_end(arg);

#define ASDX_FONT_FORMATTING8                               \
    assert(format != nullptr);                              \
    char8_t buffer[1024] = {};                              \
    va_list arg;                                            \
    va_start(arg, reinterpret_cast<const char*>(format));   \
    vsprintf_u8(buffer, sizeof(buffer), format, arg);       \
    va_end(arg);

namespace {

//----------------------------------------------------------------------------
// Constant Values.
//----------------------------------------------------------------------------
#include "../res/shaders/Compiled/asdxFontPS.inc"

//----------------------------------------------------------------------------
//      UTF-8からUTF-32に変換します.
//----------------------------------------------------------------------------
bool ToUTF32(const char* &p, uint32_t &out)
{
    const uint8_t* s = reinterpret_cast<const uint8_t*>(p);
    if (*s == 0)
        return false;

    uint32_t cp  = 0;
    uint8_t  c   = *s;
    int      len = 0;

    if (c < 0x80) 
    {
        cp  = c;
        len = 1;
    }
    else if ((c >> 5) == 0x6)
    {
        if ((s[1] & 0xC0) != 0x80)
        {
            ++p;
            return false;
        }
        cp = ((c & 0x1F) << 6) | (s[1] & 0x3F);
        len = 2;
    }
    else if ((c >> 4) == 0xE)
    {
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80)
        {
            ++p;
            return false;
        }
        cp = ((c & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        len = 3;
    }
    else if ((c >> 3) == 0x1E)
    {
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 || (s[3] & 0xC0) != 0x80)
        {
            ++p;
            return false;
        }
        cp = ((c & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        len = 4;
    }
    else
    {
        ++p;
        return false;
    }

    p  += len;
    out = cp;
    return true;
}

//-----------------------------------------------------------------------------
//      char8_t 型用 vsprintf_s
//-----------------------------------------------------------------------------
int vsprintf_u8(char8_t* buffer, size_t size, const char8_t* format, va_list args)
{
    return vsprintf_s(
        reinterpret_cast<char*>(buffer),
        size,
        reinterpret_cast<const char*>(format),
        args);
}

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Font class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Font::Font()
: m_Texture(nullptr)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Font::~Font()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Font::Init(std::vector<uint8_t>&& blob)
{
    if (blob.empty())
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    m_Binary.Load(std::move(blob));
    ResTexture res = m_Binary.GetTexture();

    if (!TextureManager::Instance().CreateTexture(res, &m_Texture))
    {
        ELOG("Error : Font Texture Init Failed.");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Font::Term()
{
    if (m_Texture)
    {
        m_Texture->Release();
        m_Texture = nullptr;
    }
    m_Binary .Term();
}

//-----------------------------------------------------------------------------
//      テクスチャを取得します.
//-----------------------------------------------------------------------------
const Texture* Font::GetTexture() const
{ return m_Texture; }

//-----------------------------------------------------------------------------
//      フォントバイナリを取得します.
//-----------------------------------------------------------------------------
const FontBinary& Font::GetBinary() const
{ return m_Binary; }

//-----------------------------------------------------------------------------
//      グリフを検索します.
//-----------------------------------------------------------------------------
bool Font::Find(uint32_t unicode, DrawInfo& info) const
{
    ResGlyph glyph = {};
    if (!m_Binary.FindGlyph(unicode, glyph))
        return false;

    auto unitSize = m_Binary.GetFontSize();
    auto w = glyph.PlaneBound.Right  - glyph.PlaneBound.Left;
    auto h = glyph.PlaneBound.Bottom - glyph.PlaneBound.Top;
    assert(w >= 0.0f);
    assert(h >= 0.0f);

    info.x = unitSize * glyph.PlaneBound.Left;
    info.y = unitSize * glyph.PlaneBound.Top;
    info.w = unitSize * w;
    info.h = unitSize * h;

    info.uv0.x = glyph.AtlasBound.Left   / float(m_Binary.GetWidth ());
    info.uv0.y = glyph.AtlasBound.Top    / float(m_Binary.GetHeight());
    info.uv1.x = glyph.AtlasBound.Right  / float(m_Binary.GetWidth ());
    info.uv1.y = glyph.AtlasBound.Bottom / float(m_Binary.GetHeight());

    info.advance = glyph.Advance * unitSize;

    return true;
}

//-----------------------------------------------------------------------------
//      文字列の幅を計算します.
//-----------------------------------------------------------------------------
int Font::CalcWidth(const char* text, float scale) const
{
    const char* p = text;   // UTF-8.
    int posX = 0;
    int maxX = 0;
    while(*p)
    {
        uint32_t unicode = 0;
        if (!ToUTF32(p, unicode))
            continue;

        if (unicode == '\n')
        {
            maxX = Max(posX, maxX);
            posX = 0;
            continue;
        }

        Font::DrawInfo info;
        if (!Find(unicode, info))
        {
            // フォールバックでリトライする.
            if (!Find(0xFFFD, info))
                continue;
        }

        posX += int(info.advance * scale);
    }

    return Max(posX, maxX);
}

#if _HAS_CXX20
//-----------------------------------------------------------------------------
//      文字列の幅を計算します.
//-----------------------------------------------------------------------------
int Font::CalcWidth(const char8_t* text, float scale) const
{ return CalcWidth(reinterpret_cast<const char*>(text), scale); }
#endif

//-----------------------------------------------------------------------------
//      1行分の縦幅を取得します.
//-----------------------------------------------------------------------------
int Font::CalcLineHeight(float scale) const
{
    const auto& bin = GetBinary();
    return int(bin.GetLineHeight() * bin.GetFontSize() * scale);
}


///////////////////////////////////////////////////////////////////////////////
// FontRenderer class
///////////////////////////////////////////////////////////////////////////////
FontRenderer FontRenderer::s_Instance = {};

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
FontRenderer::FontRenderer()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
FontRenderer::~FontRenderer()
{ Term(); }

//-----------------------------------------------------------------------------
//      シングルトンインスタンスを取得.
//-----------------------------------------------------------------------------
FontRenderer& FontRenderer::Instance()
{ return s_Instance; }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool FontRenderer::Init(SpriteRenderer& renderer)
{
    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    // パイプラインステートを生成します.
    {
        D3D12_SHADER_BYTECODE ps = { asdxFontPS, sizeof(asdxFontPS) };
        if (!renderer.CreatePipelineState(pDevice, ps, true, m_PipelineState.GetAddress()))
        {
            ELOG("Error : PipelineState Init Failed.");
            return false;
        }
    }

    // サンプラーを生成します.
    {
        auto desc = Sampler::LinearClamp;
        if (!m_LinearClamp.Init(&desc))
        {
            ELOG("Error : Sampler::Init() Failed.");
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void FontRenderer::Term()
{
    m_PipelineState.Reset();
    m_LinearClamp  .Term();
}

//-----------------------------------------------------------------------------
//      スプライトフォントを追加します.
//-----------------------------------------------------------------------------
void FontRenderer::Add
(
    SpriteRenderer& renderer,
    const Font&     font,
    int             x,
    int             y,
    int             layer,
    int*            outX,
    int*            outY,
    const char*     text
)
{
    if (text == nullptr)
        return;

    auto lineHeight = font.GetBinary().GetLineHeight() * font.GetBinary().GetFontSize() * m_Scale;
    auto posX = x;
    auto posY = y + int(lineHeight);

    const char* p = text;   // UTF-8.
    while(*p)
    {
        uint32_t unicode = 0;
        if (!ToUTF32(p, unicode))
            continue;

        if (unicode == '\n')
        {
            posX = x;
            posY += int(lineHeight);
            continue;
        }

        Font::DrawInfo info;
        if (!font.Find(unicode, info))
        {
            // フォールバックでリトライする.
            if (!font.Find(0xFFFD, info))
                continue;
        }

        renderer.Add(
            posX + int(info.x * m_Scale),
            posY + int(info.y * m_Scale),
            int(info.w * m_Scale),
            int(info.h * m_Scale),
            layer,
            info.uv0,
            info.uv1);

        posX += int(info.advance * m_Scale);
    }

    if (outX != nullptr)
    { *outX = posX; }

    if (outY != nullptr)
    { *outY = posY; }
}

//-----------------------------------------------------------------------------
//      フォーマットを指定してスプライトフォントを追加します.
//-----------------------------------------------------------------------------
void FontRenderer::AddFormat(SpriteRenderer& renderer, const Font& font, int x, int y, int layer, int* outX, int* outY, const char* format, ...)
{
    ASDX_FONT_FORMATTING
    Add(renderer, font, x, y, layer, outX, outY, buffer);
}

//-----------------------------------------------------------------------------
//      フォーマットを指定してスプライトフォントを追加します.
//-----------------------------------------------------------------------------
void FontRenderer::AddFormat(SpriteRenderer& renderer, const Font& font, int x, int y, int layer, const char* format, ...)
{
    ASDX_FONT_FORMATTING
    Add(renderer, font, x, y, layer, buffer);
}

//-----------------------------------------------------------------------------
//      フォーマットを指定してスプライトフォントを追加します.
//-----------------------------------------------------------------------------
void FontRenderer::AddFormat(SpriteRenderer& renderer, const Font& font, int x, int y, const char* format, ...)
{
    ASDX_FONT_FORMATTING
    Add(renderer, font, x, y, buffer);
}

#if _HAS_CXX20
//-----------------------------------------------------------------------------
//      スプライトフォントを追加します.
//-----------------------------------------------------------------------------
void FontRenderer::Add
(
    SpriteRenderer& renderer,
    const Font&     font,
    int             x,
    int             y,
    int             layer,
    int*            outX,
    int*            outY,
    const char8_t*  text
)
{ return Add(renderer, font, x, y, layer, outX, outY, reinterpret_cast<const char*>(text)); }

//-----------------------------------------------------------------------------
//      フォーマットを指定してスプライトフォントを追加します.
//-----------------------------------------------------------------------------
void FontRenderer::AddFormat(SpriteRenderer& renderer, const Font& font, int x, int y, int layer, int* outX, int* outY, const char8_t* format, ...)
{
    ASDX_FONT_FORMATTING8
    Add(renderer, font, x, y, layer, outX, outY, buffer);
}

//-----------------------------------------------------------------------------
//      フォーマットを指定してスプライトフォントを追加します.
//-----------------------------------------------------------------------------
void FontRenderer::AddFormat(SpriteRenderer& renderer, const Font& font, int x, int y, int layer, const char8_t* format, ...)
{
    ASDX_FONT_FORMATTING8
    Add(renderer, font, x, y, layer, buffer);
}

//-----------------------------------------------------------------------------
//      フォーマットを指定してスプライトフォントを追加します.
//-----------------------------------------------------------------------------
void FontRenderer::AddFormat(SpriteRenderer& renderer, const Font& font, int x, int y, const char8_t* format, ...)
{
    ASDX_FONT_FORMATTING8
    Add(renderer, font, x, y, buffer);
}
#endif

//-----------------------------------------------------------------------------
//      パイプラインステートを設定します.
//-----------------------------------------------------------------------------
void FontRenderer::SetState(SpriteRenderer& renderer, const Font& font)
{
    renderer.SetParam(4, GetParam(), 0);
    renderer.ChangeBatch(m_PipelineState.GetPtr(), font.GetTexture()->GetHandleGPU(), m_LinearClamp.GetHandleGPU());
}

//-----------------------------------------------------------------------------
//      スケールを設定します.
//-----------------------------------------------------------------------------
void FontRenderer::SetScale(float value)
{ m_Scale = value; }

//-----------------------------------------------------------------------------
//      スケールを取得します.
//-----------------------------------------------------------------------------
float FontRenderer::GetScale() const
{ return m_Scale; }

//-----------------------------------------------------------------------------
//      アウター描画の設定を行います.
//-----------------------------------------------------------------------------
void FontRenderer::SetEnableOuter(bool value)
{ m_Param.EnableOuter = value ? 1 : 0; }

//-----------------------------------------------------------------------------
//      アウター描画が有効かどうかチェックします.
//-----------------------------------------------------------------------------
bool FontRenderer::IsEnableOuter() const
{ return m_Param.EnableOuter == 1; }

//-----------------------------------------------------------------------------
//      アウターオフセットの設定を行います.
//-----------------------------------------------------------------------------
void FontRenderer::SetEnableOffset(bool value)
{ m_Param.EnableOffset = value ? 1 : 0; }

//-----------------------------------------------------------------------------
//      アウターオフセットが有効かどうかチェックします.
//-----------------------------------------------------------------------------
bool FontRenderer::IsEnableOffset() const
{ return m_Param.EnableOffset == 1; }

//-----------------------------------------------------------------------------
//      アウターオフセットを設定します.
//-----------------------------------------------------------------------------
void FontRenderer::SetOuterOffset(float x, float y)
{
    m_Param.OuterOffsetX = x;
    m_Param.OuterOffsetY = y;
}

//-----------------------------------------------------------------------------
//      X成分のアウターオフセットを取得します.
//-----------------------------------------------------------------------------
float FontRenderer::GetOuterOffsetX() const
{ return m_Param.OuterOffsetX; }

//-----------------------------------------------------------------------------
//      Y成分のアウターオフセットを取得します.
//-----------------------------------------------------------------------------
float FontRenderer::GetOuterOffsetY() const
{ return m_Param.OuterOffsetY; }

//-----------------------------------------------------------------------------
//      アウターカラーを設定します.
//-----------------------------------------------------------------------------
void FontRenderer::SetOuterColor(float r, float g, float b, float a)
{
    auto ur = uint8_t(r * 255.0f);
    auto ug = uint8_t(g * 255.0f);
    auto ub = uint8_t(b * 255.0f);
    auto ua = uint8_t(a * 255.0f);

    uint32_t color = ((ur << 24) | (ug << 16) | (ub << 8) | (ua));
    m_Param.OuterColor = color;
}

//-----------------------------------------------------------------------------
//      アウターカラーを設定します.
//-----------------------------------------------------------------------------
void FontRenderer::SetOuterColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    uint32_t color = ((r << 24) | (g << 16) | (b << 8) | (a));
    m_Param.OuterColor = color;
}

//-----------------------------------------------------------------------------
//      アウターカラーを設定します.
//-----------------------------------------------------------------------------
void FontRenderer::SetOuterColor(uint32_t color)
{ m_Param.OuterColor = color; }

//-----------------------------------------------------------------------------
//      アウターカラーを取得します.
//-----------------------------------------------------------------------------
uint32_t FontRenderer::GetOuterColor() const
{ return m_Param.OuterColor; }

//-----------------------------------------------------------------------------
//      シェーダパラメータを取得します.
//-----------------------------------------------------------------------------
const void* FontRenderer::GetParam() const
{ return &m_Param; }

//-----------------------------------------------------------------------------
//      LinearClampサンプラーを取得します.
//-----------------------------------------------------------------------------
const Sampler& FontRenderer::GetSampler() const
{ return m_LinearClamp; }

} // namespace asdx

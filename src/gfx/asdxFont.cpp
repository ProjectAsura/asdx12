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
#include <gfx/asdxDevice.h>
#include <gfx/asdxFont.h>
#include <gfx/asdxSprite.h>
#include <fnd/asdxLogger.h>
#include <res/asdxResTexture.h>


namespace { 

//----------------------------------------------------------------------------
// Constant Values.
//----------------------------------------------------------------------------
#include "../res/shaders/Compiled/FontPS.inc"


//----------------------------------------------------------------------------
//      Unicodeを取得します.
//----------------------------------------------------------------------------
bool Utf8Next(const char* &p, uint32_t &out)
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

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Font class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Font::Font()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Font::~Font()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Font::Init(ID3D12GraphicsCommandList* pCmd, const char* path)
{
    if (pCmd == nullptr || path == nullptr)
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    if (!m_Binary.LoadA(path))
    {
        ELOG("Error : FontBinary Load Failed.");
        return false;
    }

    ResTexture res = {};
    res.Dimension           = TEXTURE_DIMENSION_2D;
    res.Width               = m_Binary.GetWidth();
    res.Height              = m_Binary.GetHeight();
    res.DepthOrArraySize    = 1;
    res.MipLevels           = 1;
    res.Format              = m_Binary.GetFormat();
    res.SubResourceCount    = 1;

    res.SubResources[0].Width       = res.Width;
    res.SubResources[0].Height      = res.Height;
    res.SubResources[0].RowPitch    = m_Binary.GetRowPitch();
    res.SubResources[0].SlicePitch  = m_Binary.GetSlicePitch();
    res.SubResources[0].pPixels     = m_Binary.GetTexels();

    if (!m_Texture.Init(pCmd, res))
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
    m_Texture.Term();
    m_Binary .Term();
}

//-----------------------------------------------------------------------------
//      テクスチャを取得します.
//-----------------------------------------------------------------------------
const Texture& Font::GetTexture() const
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
    auto w = glyph.PlaneBound.Right - glyph.PlaneBound.Left;
    auto h = glyph.PlaneBound.Top   - glyph.PlaneBound.Bottom;

    info.x = unitSize * glyph.PlaneBound.Left;
    info.y = unitSize * glyph.PlaneBound.Top;
    info.w = unitSize * w;
    info.h = unitSize * h;

    info.uv0.x = glyph.AtlasBound.Left   / float(m_Binary.GetWidth ());
    info.uv0.y = glyph.AtlasBound.Top    / float(m_Binary.GetHeight());
    info.uv1.x = glyph.AtlasBound.Right  / float(m_Binary.GetWidth ());
    info.uv1.y = glyph.AtlasBound.Bottom / float(m_Binary.GetHeight());

    info.advance = glyph.Advance * unitSize;

    if (m_Binary.IsFlipY())
    {
        auto temp  = info.uv0.y;
        info.uv0.y = info.uv1.y;
        info.uv1.y = temp;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// FontRenderer class
///////////////////////////////////////////////////////////////////////////////

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
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool FontRenderer::Init(SpriteRenderer& renderer)
{
    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    D3D12_SHADER_BYTECODE ps = { FontPS, sizeof(FontPS) };

    if (!renderer.CreateSpritePipelineState(pDevice, ps, m_PSO.GetAddress()))
    {
        ELOG("Error : PipelineState Init Failed.");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void FontRenderer::Term()
{ m_PSO.Reset(); }

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
    auto posX = x;
    auto posY = y + (int)(font.GetBinary().GetAscender() * m_Scale);

    const char* p = text;   // UTF-8.
    while(*p)
    {
        uint32_t unicode = 0;
        if (!Utf8Next(p, unicode))
            continue;

        if (unicode == '\n')
        {
            posX = x;
            posY += int(font.GetBinary().GetLineHeight() * m_Scale);
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
//      書式指定子でフォーマットします.
//-----------------------------------------------------------------------------
char* FontRenderer::Format(char* buffer, size_t bufferSize, const char* format, ...)
{
    assert(buffer != nullptr);
    assert(format != nullptr);

    va_list arg;
    va_start(arg, format);
    vsprintf_s(buffer, bufferSize, format, arg);
    va_end(arg);
    return buffer;
}

//-----------------------------------------------------------------------------
//      パイプラインステートを設定します.
//-----------------------------------------------------------------------------
void FontRenderer::SetPipelineState(ID3D12GraphicsCommandList* pCmdList, SpriteRenderer& renderer)
{ renderer.SetPipelineState(pCmdList, m_PSO.GetPtr()); }

} // namespace asdx

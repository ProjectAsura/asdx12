//-----------------------------------------------------------------------------
// File : asdxMapChip.cpp
// Desc : Map Chip.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxMapChip.h>
#include <fnd/asdxLogger.h>
#include <utility>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// MapChip class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
MapChip::MapChip()
: m_ScrollX     (0)
, m_ScrollY     (0)
, m_TileOffsetX (0)
, m_TileOffsetY (0)
, m_DrawTileW   (0)
, m_DrawTileH   (0)
, m_ScreenWidth (0)
, m_ScreenHeight(0)
, m_DrawRows    (0)
, m_DrawCols    (0)
, m_Clamp       (false)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
MapChip::~MapChip()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool MapChip::Init
(
    ID3D12GraphicsCommandList*  pCmd,
    std::vector<uint8_t>&       blob,
    uint32_t                    screenWidth,
    uint32_t                    screenHeight,
    int                         drawTileWidth,
    int                         drawTileHeight
)
{
    m_Binary.Load(std::move(blob));

    m_Textures.resize(m_Binary.GetTileSetCount());

    for(auto i=0u; i<m_Binary.GetTileSetCount(); ++i)
    {
        auto res = m_Binary.GetTexture(i);
        if (!Texture::Create(pCmd, res, &m_Textures[i]))
        {
            ELOG("Error : Texture Init Failed. index = %u", i);
            return false;
        }
    }

    m_ScrollX = 0;
    m_ScrollY = 0;

    m_ScreenWidth  = screenWidth;
    m_ScreenHeight = screenHeight;

    m_DrawTileW = drawTileWidth;
    m_DrawTileH = drawTileHeight;

    UpdateDrawCount();

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void MapChip::Term()
{
    m_Binary.Term();

    m_ScrollX = 0;
    m_ScrollY = 0;

    m_ScreenWidth  = 0;
    m_ScreenHeight = 0;

    m_DrawTileW = 0;
    m_DrawTileH = 0;

    m_DrawRows = 0;
    m_DrawCols = 0;

    m_Clamp = false;

    for(size_t i=0; i<m_Textures.size(); ++i)
    { 
        m_Textures[i]->Release(); 
        m_Textures[i] = nullptr;
    }

    m_Textures.clear();
    m_Textures.shrink_to_fit();
}

//-----------------------------------------------------------------------------
//      マップチップバイナリを取得します.
//-----------------------------------------------------------------------------
const MapChipBinary& MapChip::GetBinary() const
{ return m_Binary; }

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void MapChip::Draw(SpriteRenderer& renderer, D3D12_GPU_DESCRIPTOR_HANDLE sampler)
{
    renderer.ChangeBatch(renderer.GetDefaultState(), m_Textures[0]->GetHandleGPU(), sampler);

    auto tileSet = m_Binary.GetTileSet(0);

    // レイヤーごとに描画.
    for(auto l=0u; l<m_Binary.GetLayerCount(); ++l)
    {
        auto layer = m_Binary.GetLayer(l);

        for(int y=-1; y<=m_DrawRows; ++y) // 前後1タイル分の余白を考慮.
        {
            for(int x=-1; x<=m_DrawCols; ++x) // 前後1タイル分の余白を考慮.
            {
                auto tx = (x >= 0) ? x : layer.Columns - 1;
                auto ty = (y >= 0) ? y : layer.Rows - 1;

                uint32_t idX, idY;
                if (m_Clamp)
                {
                    idX = asdx::Clamp(tx + m_TileOffsetX, 0u, layer.Columns);
                    idY = asdx::Clamp(ty + m_TileOffsetY, 0u, layer.Rows);
                }
                else
                {
                    idX = (tx + m_TileOffsetX) % layer.Columns;
                    idY = (ty + m_TileOffsetY) % layer.Rows;
                }

                auto id = idX + (idY * layer.Columns);
                auto tileId = layer.Data[id] - tileSet.FirstChipId;

                auto coord = m_Binary.GetCoord(0, tileId);
                renderer.Add(
                    x * m_DrawTileW + m_ScrollX,
                    y * m_DrawTileH + m_ScrollY,
                    m_DrawTileW,
                    m_DrawTileH,
                    coord.Uv0,
                    coord.Uv1);
            }
        }
    }

    // タイルオフセット値を更新します.
    UpdateTileOffset();
}

//-----------------------------------------------------------------------------
//      タイルオフセットを設定します.
//-----------------------------------------------------------------------------
void MapChip::SetTileOffset(uint32_t x, uint32_t y)
{
    m_TileOffsetX = x;
    m_TileOffsetY = y;
}

//-----------------------------------------------------------------------------
//      スクロールオフセットを設定します.
//-----------------------------------------------------------------------------
void MapChip::SetScroll(int x, int y)
{
    assert(abs(x) <= m_DrawTileW);
    assert(abs(y) <= m_DrawTileH);
    m_ScrollX = x;
    m_ScrollY = y;
}

//-----------------------------------------------------------------------------
//      スクリーンサイズを設定します.
//-----------------------------------------------------------------------------
void MapChip::SetScreenSize(uint32_t w, uint32_t h)
{
    m_ScreenWidth  = w;
    m_ScreenHeight = h;

    UpdateDrawCount();
}

//-----------------------------------------------------------------------------
//      描画タイルサイズを設定します.
//-----------------------------------------------------------------------------
void MapChip::SetDrawTileSize(int w, int h)
{
    m_DrawTileW = w;
    m_DrawTileH = h;

    UpdateDrawCount();
}

//-----------------------------------------------------------------------------
//      X方向のタイルオフセットを取得します.
//-----------------------------------------------------------------------------
uint32_t MapChip::GetTileOffsetX() const
{ return m_TileOffsetX; }

//-----------------------------------------------------------------------------
//      Y方向のタイルオフセットを取得します.
//-----------------------------------------------------------------------------
uint32_t MapChip::getTileOffsetY() const
{ return m_TileOffsetY; }

//-----------------------------------------------------------------------------
//      X方向のスクロール値を取得します.
//-----------------------------------------------------------------------------
int MapChip::GetScrollX() const
{ return m_ScrollX; }

//-----------------------------------------------------------------------------
//      Y方向のスクロール値を取得します.
//-----------------------------------------------------------------------------
int MapChip::GetScrollY() const
{ return m_ScrollY; }

//-----------------------------------------------------------------------------
//      描画タイル横幅を取得します.
//-----------------------------------------------------------------------------
int MapChip::GetDrawTileWidth() const
{ return m_DrawTileW; }

//-----------------------------------------------------------------------------
//      描画タイル縦幅を取得します.
//-----------------------------------------------------------------------------
int MapChip::GetDrawTileHeight() const
{ return m_DrawTileH; }

//-----------------------------------------------------------------------------
//      スクリーンの横幅を取得します.
//-----------------------------------------------------------------------------
uint32_t MapChip::GetScreenWidth() const
{ return m_ScreenWidth; }

//-----------------------------------------------------------------------------
//      スクリーンの縦幅を取得します.
//-----------------------------------------------------------------------------
uint32_t MapChip::GetScreenHeight() const
{ return m_ScreenHeight; }

//-----------------------------------------------------------------------------
//      チップIDを取得します.
//-----------------------------------------------------------------------------
uint16_t MapChip::GetChipId(uint32_t x, uint32_t y) const
{ return GetChipId(0, x, y); }

//-----------------------------------------------------------------------------
//      プロパティを持つかどうかチェックします.
//-----------------------------------------------------------------------------
bool MapChip::HasProperty(uint32_t x, uint32_t y, ResChipProperty& prop) const
{
    auto chipId = GetChipId(x, y);
    return m_Binary.FindChipProperty(0, chipId, prop);
}

//-----------------------------------------------------------------------------
//      クランプフラグを設定します.
//-----------------------------------------------------------------------------
void MapChip::SetClamp(bool value)
{ m_Clamp = value; }

//-----------------------------------------------------------------------------
//      クランプフラグを取得します.
//-----------------------------------------------------------------------------
bool MapChip::IsClamp() const
{ return m_Clamp; }

//-----------------------------------------------------------------------------
//      チップIDを取得します.
//-----------------------------------------------------------------------------
uint16_t MapChip::GetChipId(uint32_t layerId, uint32_t x, uint32_t y) const
{
    auto layer = m_Binary.GetLayer(layerId);

    auto idX = x % layer.Columns;
    auto idY = y % layer.Rows;

    auto id = idX + (idY * layer.Columns);
    return layer.Data[id];
}

//-----------------------------------------------------------------------------
//      描画数を更新します.
//-----------------------------------------------------------------------------
void MapChip::UpdateDrawCount()
{
    m_DrawCols = int(ceil(float(m_ScreenWidth)  / float(m_DrawTileW)));
    m_DrawRows = int(ceil(float(m_ScreenHeight) / float(m_DrawTileH)));
}

//-----------------------------------------------------------------------------
//      タイルオフセット値を更新します.
//-----------------------------------------------------------------------------
void MapChip::UpdateTileOffset()
{
    auto layer0 = m_Binary.GetLayer(0);

    // X方向のスクロール値と，オフセットを更新.
    if (m_Clamp)
    {
        if (m_ScrollX >= m_DrawTileW)
        {
            if (int(m_TileOffsetX) - 1 < 0)
            {
                m_ScrollX = m_DrawTileW;
                m_TileOffsetX = 0;
            }
            else
            {
                m_ScrollX = 0;
                m_TileOffsetX--;
            }
        }
        else if (m_ScrollX <= -m_DrawTileW)
        {
            if (m_TileOffsetX + 1 < layer0.Columns)
            {
                m_ScrollX = 0;
                m_TileOffsetX++;
            }
            else
            {
                m_ScrollX = -m_DrawTileW;
                m_TileOffsetX = layer0.Columns - 1;
            }
        }
    }
    else
    {
        if (m_ScrollX >= m_DrawTileW)
        {
            m_ScrollX = 0;
            if (int(m_TileOffsetX) - 1 < 0)
            { m_TileOffsetX = layer0.Columns - 1; }
            else
            { m_TileOffsetX--; }
        }
        else if (m_ScrollX <= -m_DrawTileW)
        {
            m_ScrollX = 0;
            m_TileOffsetX = (m_TileOffsetX + 1) % layer0.Columns;
        }
    }

    // X方向のスクロール値と，オフセットを更新.
    if (m_Clamp)
    {
        if (m_ScrollY >= m_DrawTileH)
        {
            if (int(m_TileOffsetY) - 1 < 0)
            {
                m_ScrollY = m_DrawTileH;
                m_TileOffsetY = 0;
            }
            else
            {
                m_ScrollY = 0;
                m_TileOffsetY--;
            }
        }
        else if (m_ScrollY <= -m_DrawTileH)
        {
            if (m_TileOffsetY + 1 < layer0.Rows)
            {
                m_ScrollY = 0;
                m_TileOffsetY++;
            }
            else 
            {
                m_ScrollY = -m_DrawTileH;
                m_TileOffsetY = layer0.Rows - 1;
            }
        }
    }
    else
    {
        if (m_ScrollY >= m_DrawTileH)
        {
            m_ScrollY = 0;
            if (int(m_TileOffsetY) - 1 < 0)
            { m_TileOffsetY = layer0.Rows - 1; }
            else
            { m_TileOffsetY--; }
        }
        else if (m_ScrollY <= -m_DrawTileH)
        {
            m_ScrollY = 0;
            m_TileOffsetY = (m_TileOffsetY + 1) % layer0.Rows;
        }
    }
}

} // namespace asdx

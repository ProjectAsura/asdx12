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
#include <string>


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
    std::vector<uint8_t>&       blob,
    const char*                 baseDir,
    uint32_t                    screenWidth,
    uint32_t                    screenHeight,
    int                         drawTileWidth,
    int                         drawTileHeight
)
{
    m_Binary.Load(std::move(blob));

    auto path = std::string(baseDir) + "\\" + std::string(m_Binary.GetTexturePath().c_str());

    m_Texture = TextureManager::Instance().GetOrCreate(path.c_str());
    if (!m_Texture.IsValid())
    {
        ELOGA("Error : Texture Init Failed. path = %s", path.c_str());
        return false;
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

    m_Texture.Reset();
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
    renderer.ChangeBatch(renderer.GetDefaultState(), m_Texture.GetHandleGPU(), sampler);

    auto columns = m_Binary.GetMapColumns();
    auto rows    = m_Binary.GetMapRows();
    auto firstId = m_Binary.GetFirstChipId();

    // レイヤーごとに描画.
    for(auto l=0u; l<m_Binary.GetLayerCount(); ++l)
    {
        auto layer = m_Binary.GetLayer(l);

        for(int y=-1; y<=m_DrawRows; ++y) // 前後1タイル分の余白を考慮.
        {
            for(int x=-1; x<=m_DrawCols; ++x) // 前後1タイル分の余白を考慮.
            {
                auto tx = (x >= 0) ? x : columns - 1;
                auto ty = (y >= 0) ? y : rows - 1;

                uint32_t idX, idY;
                if (m_Clamp)
                {
                    idX = asdx::Clamp(tx + m_TileOffsetX, 0u, columns);
                    idY = asdx::Clamp(ty + m_TileOffsetY, 0u, rows);
                }
                else
                {
                    idX = (tx + m_TileOffsetX) % columns;
                    idY = (ty + m_TileOffsetY) % rows;
                }

                auto id = idX + (idY * columns);
                auto tileId = layer.Tiles[id] - firstId;

                auto coord = m_Binary.GetCoord(tileId);
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
uint32_t MapChip::GetTileOffsetY() const
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
uint16_t MapChip::GetChipId(uint32_t layerIndex, uint32_t x, uint32_t y) const
{
    auto idX = x % m_Binary.GetMapColumns();
    auto idY = y % m_Binary.GetMapRows();

    auto id = idX + (idY * m_Binary.GetMapColumns());
    return m_Binary.GetLayer(layerIndex).Tiles[id];
}

//-----------------------------------------------------------------------------
//      タイルプロパティを取得します.
//-----------------------------------------------------------------------------
ResTileProp MapChip::GetTileProp(uint32_t x, uint32_t y) const
{
    auto idX = x % m_Binary.GetMapColumns();
    auto idY = y % m_Binary.GetMapRows();

    auto id = idX + (idY * m_Binary.GetMapColumns());
    return m_Binary.GetTileProp(id);
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
    auto columns = m_Binary.GetMapColumns();
    auto rows    = m_Binary.GetMapRows();

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
            if (m_TileOffsetX + 1 < columns)
            {
                m_ScrollX = 0;
                m_TileOffsetX++;
            }
            else
            {
                m_ScrollX = -m_DrawTileW;
                m_TileOffsetX = columns - 1;
            }
        }
    }
    else
    {
        if (m_ScrollX >= m_DrawTileW)
        {
            m_ScrollX = 0;
            if (int(m_TileOffsetX) - 1 < 0)
            { m_TileOffsetX = columns - 1; }
            else
            { m_TileOffsetX--; }
        }
        else if (m_ScrollX <= -m_DrawTileW)
        {
            m_ScrollX = 0;
            m_TileOffsetX = (m_TileOffsetX + 1) % columns;
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
            if (m_TileOffsetY + 1 < rows)
            {
                m_ScrollY = 0;
                m_TileOffsetY++;
            }
            else 
            {
                m_ScrollY = -m_DrawTileH;
                m_TileOffsetY = rows - 1;
            }
        }
    }
    else
    {
        if (m_ScrollY >= m_DrawTileH)
        {
            m_ScrollY = 0;
            if (int(m_TileOffsetY) - 1 < 0)
            { m_TileOffsetY = rows - 1; }
            else
            { m_TileOffsetY--; }
        }
        else if (m_ScrollY <= -m_DrawTileH)
        {
            m_ScrollY = 0;
            m_TileOffsetY = (m_TileOffsetY + 1) % rows;
        }
    }
}

} // namespace asdx

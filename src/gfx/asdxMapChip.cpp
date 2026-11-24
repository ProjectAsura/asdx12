//-----------------------------------------------------------------------------
// File : asdxMapChip.cpp
// Desc : Map Chip.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxMapChip.h>
#include <gfx/asdxTexture.h>
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

    auto chipCount = m_Binary.GetChipCount();
    for(auto i=0u; i<chipCount; ++i)
    {
        auto chip = m_Binary.GetChip(i);
        if (chip.Frames.empty())
            continue;

        ChipFrame frame = {};
        frame.ElapapsedSec   = 0.0f;
        frame.ArrayIndex     = 0;
        frame.ChipIndex      = i;
        frame.ResolvedChipId = chip.Frames[0].Id;

        m_ChipFrames[chip.Id] = frame;
    }

    m_LayerInfos.resize(m_Binary.GetLayerCount());
    for(size_t i=0; i<m_LayerInfos.size(); ++i)
    {
        auto& info = m_LayerInfos[i];
        info.Offset     = Vector2(0.0f, 0.0f); 
        info.Visibility = true;
    }

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

    m_ChipFrames.clear();

    m_LayerInfos.clear();
    m_LayerInfos.shrink_to_fit();
}

//-----------------------------------------------------------------------------
//      有効かどうかチェックします.
//-----------------------------------------------------------------------------
bool MapChip::IsValid() const
{ return m_Binary.IsValid() && m_Texture.IsValid(); }

//-----------------------------------------------------------------------------
//      マップチップバイナリを取得します.
//-----------------------------------------------------------------------------
const MapChipBinary& MapChip::GetBinary() const
{ return m_Binary; }

//-----------------------------------------------------------------------------
//      更新処理を行います.
//-----------------------------------------------------------------------------
void MapChip::Update(float deltaSec)
{
    if (m_ChipFrames.empty())
        return;

    for(auto& itr : m_ChipFrames)
    {
        auto& frame = itr.second;

        // 経過時間取得.
        auto& elapsedSec = frame.ElapapsedSec;

        // 時間を進める.
        elapsedSec += deltaSec;

        auto chip = m_Binary.GetChip(frame.ChipIndex);
        auto idx = frame.ArrayIndex;

        // 継続時間を超えたら、次のフレームに切り替える.
        if (chip.Frames[idx].Duration < elapsedSec)
        {
            frame.ArrayIndex     = (idx + 1) % uint32_t(chip.Frames.size());
            frame.ElapapsedSec   = fmod(elapsedSec, chip.Frames[idx].Duration);
            frame.ResolvedChipId = chip.Frames[frame.ArrayIndex].Id;
        }
    }
}

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void MapChip::Draw(SpriteRenderer& renderer, D3D12_GPU_DESCRIPTOR_HANDLE sampler)
{
    renderer.ChangeBatch(renderer.GetDefaultState(), m_Texture->GetHandleGPU(), sampler);

    auto columns = m_Binary.GetMapColumns();
    auto rows    = m_Binary.GetMapRows();
    auto firstId = m_Binary.GetFirstChipId();
    auto layers  = m_Binary.GetLayerCount();

    auto prevChipId    = -1;
    auto resolveChipId = -1;

    // レイヤーごとに描画.
    for(auto l=0u; l<layers; ++l)
    {
        auto layer = m_Binary.GetLayer(l);

        // レイヤーが非表示の場合は処理しない.
        if (!m_LayerInfos[l].Visibility)
            continue;

        // オフセット値を計算.
        const auto tileOffset = GetLayerTileOffset(l);
        const auto chipOffset = GetLayerChipOffset(l);

        for(int y = -m_BlankMarginL; y < m_DrawRows + m_BlankMarginR; ++y)
        {
            for(int x = -m_BlankMarginT; x < m_DrawCols + m_BlankMarginB; ++x)
            {
                auto tx = (x >= 0) ? x : columns - 1;
                auto ty = (y >= 0) ? y : rows - 1;

                // 行番号と列番号を算出.
                uint32_t idX, idY;
                if (m_Clamp)
                {
                    idX = asdx::Clamp(tx + m_TileOffsetX + tileOffset.x, 0u, columns);
                    idY = asdx::Clamp(ty + m_TileOffsetY + tileOffset.y, 0u, rows);
                }
                else
                {
                    idX = (tx + m_TileOffsetX + tileOffset.x) % columns;
                    idY = (ty + m_TileOffsetY + tileOffset.y) % rows;
                }

                // 配置タイル番号を取得.
                auto id = idX + (idY * columns);
                if (layer.Tiles[id] < firstId)
                    continue;

                // タイルに対応するチップ番号を求める.
                auto chipId = layer.Tiles[id] - firstId;

                // 無駄に検索しないように抑制.
                if (prevChipId != chipId)
                {
                    prevChipId = chipId;

                    auto itr = m_ChipFrames.find(chipId);
                    if (itr != m_ChipFrames.end())
                        resolveChipId = itr->second.ResolvedChipId;
                    else
                        resolveChipId = chipId;
                }

                // チップ番号からテクスチャ座標を計算.
                auto coord = m_Binary.GetCoord(resolveChipId);

                // スプライトを追加.
                renderer.Add(
                    x * m_DrawTileW + m_ScrollX + chipOffset.x,
                    y * m_DrawTileH + m_ScrollY + chipOffset.y,
                    m_DrawTileW,
                    m_DrawTileH,
                    int(l),
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
    auto id = m_Binary.CalcTileIndex(x, y);
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
                m_ScrollX     = m_DrawTileW;
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
                m_ScrollX     = -m_DrawTileW;
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
            m_ScrollX     = 0;
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
                m_ScrollY     = m_DrawTileH;
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
                m_ScrollY     = -m_DrawTileH;
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
            m_ScrollY     = 0;
            m_TileOffsetY = (m_TileOffsetY + 1) % rows;
        }
    }
}

//-----------------------------------------------------------------------------
//      レイヤーオフセットを設定します.
//-----------------------------------------------------------------------------
void MapChip::SetLayerOffset(uint32_t index, const Vector2& offset)
{
    assert(index < m_LayerInfos.size());
    m_LayerInfos[index].Offset = offset;
}

//-----------------------------------------------------------------------------
//      レイヤーオフセットを取得します.
//-----------------------------------------------------------------------------
const Vector2& MapChip::GetLayerOffset(uint32_t index) const
{
    assert(index < m_LayerInfos.size());
    return m_LayerInfos[index].Offset;
}

//-----------------------------------------------------------------------------
//      レイヤーのタイルオフセットを取得します.
//-----------------------------------------------------------------------------
Int2 MapChip::GetLayerTileOffset(uint32_t index) const
{
    assert(index < m_LayerInfos.size());
    // オフセットは逆方向なのでマイナスになる.
    return Int2(
        -int(m_LayerInfos[index].Offset.x / m_DrawTileW),
        -int(m_LayerInfos[index].Offset.y / m_DrawTileH));
}

//-----------------------------------------------------------------------------
//      レイヤーのチップオフセットを取得します.
//-----------------------------------------------------------------------------
Int2 MapChip::GetLayerChipOffset(uint32_t index) const
{
    assert(index < m_LayerInfos.size());
    return Int2(
        int(fmod(m_LayerInfos[index].Offset.x, float(m_DrawTileW))),
        int(fmod(m_LayerInfos[index].Offset.y, float(m_DrawTileH))));
}

//-----------------------------------------------------------------------------
//      レイヤー表示フラグを設定します.
//-----------------------------------------------------------------------------
void MapChip::SetLayerVisibility(uint32_t index, bool value)
{
    assert(index < m_LayerInfos.size());
    m_LayerInfos[index].Visibility = value;
}

//-----------------------------------------------------------------------------
//      レイヤー表示フラグを取得します.
//-----------------------------------------------------------------------------
bool MapChip::IsLayerVisible(uint32_t index) const
{
    assert(index < m_LayerInfos.size());
    return m_LayerInfos[index].Visibility;
}

//-----------------------------------------------------------------------------
//      タイルマージンを設定します.
//-----------------------------------------------------------------------------
void MapChip::SetBlankMargin(int left, int right, int top, int bottom)
{
    m_BlankMarginL = left;
    m_BlankMarginR = right;
    m_BlankMarginT = top;
    m_BlankMarginB = bottom;
}

//-----------------------------------------------------------------------------
//      左タイルマージンを取得します.
//-----------------------------------------------------------------------------
int MapChip::GetBlankMarginL() const
{ return m_BlankMarginL; }

//-----------------------------------------------------------------------------
//      右タイルマージンを取得します.
//-----------------------------------------------------------------------------
int MapChip::GetBlankMarginR() const
{ return m_BlankMarginR; }

//-----------------------------------------------------------------------------
//      上タイルマージンを取得します.
//-----------------------------------------------------------------------------
int MapChip::GetBlankMarginT() const
{ return m_BlankMarginT; }

//-----------------------------------------------------------------------------
//      下タイルマージンを取得します.
//-----------------------------------------------------------------------------
int MapChip::GetBlankMarginB() const
{ return m_BlankMarginB; }

} // namespace asdx

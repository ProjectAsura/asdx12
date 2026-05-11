//-----------------------------------------------------------------------------
// File : asdxResMapChip.cpp
// Desc : Map Chip Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxMacro.h>
#include <res/asdxResMapChip.h>
#include "MapChipBinary_generated.h"


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// MapChipBinary class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
MapChipBinary::MapChipBinary()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
MapChipBinary::~MapChipBinary()
{ Term(); }

//-----------------------------------------------------------------------------
//      バイナリをロードします.
//-----------------------------------------------------------------------------
void MapChipBinary::Load(std::vector<uint8_t>&& blob)
{
    m_Blob = std::move(blob);

#if ASDX_DEBUG
    // デバッグ整合性をチェック.
    {
        assert(!m_Blob.empty());
        flatbuffers::Verifier verifier(m_Blob.data(), m_Blob.size());
        assert(res::VerifyMapChipBinaryBuffer(verifier));
        ASDX_UNUSED(verifier);
    }
#endif
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void MapChipBinary::Term()
{
    m_Blob.clear();
    m_Blob.shrink_to_fit();
}

//-----------------------------------------------------------------------------
//      行数を取得します.
//-----------------------------------------------------------------------------
uint32_t MapChipBinary::GetMapRows() const
{
    assert(!m_Blob.empty());
    return res::GetMapChipBinary(m_Blob.data())->MapCount()->Y();
}

//-----------------------------------------------------------------------------
//      列数を取得します.
//-----------------------------------------------------------------------------
uint32_t MapChipBinary::GetMapColumns() const
{
    assert(!m_Blob.empty());
    return res::GetMapChipBinary(m_Blob.data())->MapCount()->X();
}

//-----------------------------------------------------------------------------
//      マップチップの行数を取得します.
//-----------------------------------------------------------------------------
uint32_t MapChipBinary::GetChipRows() const
{
    assert(!m_Blob.empty());
    return res::GetMapChipBinary(m_Blob.data())->ChipCount()->Y();
}

//-----------------------------------------------------------------------------
//      マップチップの列数を取得します.
//-----------------------------------------------------------------------------
uint32_t MapChipBinary::GetChipColumns() const
{
    assert(!m_Blob.empty());
    return res::GetMapChipBinary(m_Blob.data( ))->ChipCount()->X();
}

//-----------------------------------------------------------------------------
//      タイルの横幅を取得します.
//-----------------------------------------------------------------------------
uint32_t MapChipBinary::GetChipWidth() const
{
    assert(!m_Blob.empty());
    return res::GetMapChipBinary(m_Blob.data())->ChipSize()->X();
}

//-----------------------------------------------------------------------------
//      タイルの縦幅を取得します.
//-----------------------------------------------------------------------------
uint32_t MapChipBinary::GetChipHeight() const
{
    assert(!m_Blob.empty());
    return res::GetMapChipBinary(m_Blob.data())->ChipSize()->Y();
}

//-----------------------------------------------------------------------------
//      テクスチャの横幅を取得します.
//-----------------------------------------------------------------------------
uint32_t MapChipBinary::GetTextureWidth() const
{
    assert(!m_Blob.empty());
    return res::GetMapChipBinary(m_Blob.data())->TextureSize()->X();
}

//-----------------------------------------------------------------------------
//      テクスチャの縦幅を取得します.
//-----------------------------------------------------------------------------
uint32_t MapChipBinary::GetTextureHeight() const
{
    assert(!m_Blob.empty());
    return res::GetMapChipBinary(m_Blob.data())->TextureSize()->Y();
}

//-----------------------------------------------------------------------------
//      テクスチャのファイルパスを取得します.
//-----------------------------------------------------------------------------
StringView MapChipBinary::GetTexturePath() const
{
    assert(!m_Blob.empty());
    return StringView(res::GetMapChipBinary(m_Blob.data())->TexturePath()->c_str());
}

//-----------------------------------------------------------------------------
//      最初のチップIDを取得します.
//-----------------------------------------------------------------------------
uint32_t MapChipBinary::GetFirstChipId() const
{
    assert(!m_Blob.empty());
    return res::GetMapChipBinary(m_Blob.data())->FirstChipId();
}

//-----------------------------------------------------------------------------
//      レイヤー数を取得します.
//-----------------------------------------------------------------------------
uint32_t MapChipBinary::GetLayerCount() const
{
    assert(!m_Blob.empty());
    return res::GetMapChipBinary(m_Blob.data())->Layers()->size();
}

//-----------------------------------------------------------------------------
//      レイヤーを取得します.
//-----------------------------------------------------------------------------
ResMapLayer MapChipBinary::GetLayer(uint32_t index) const
{
    assert(!m_Blob.empty());
    auto layer = res::GetMapChipBinary(m_Blob.data())->Layers()->Get(index);

    ResMapLayer result = {};
    result.Name  = StringView(layer->Name()->c_str());
    result.Tiles = ArrayView<uint16_t>(layer->Chips()->data(), layer->Chips()->size());
    return result;
}

//-----------------------------------------------------------------------------
//      指定タイルのテクスチャ座標を取得します.
//-----------------------------------------------------------------------------
ResTileCoord MapChipBinary::GetCoord(uint32_t tileId) const
{
    assert(!m_Blob.empty());
    auto bin = res::GetMapChipBinary(m_Blob.data());
    auto x = tileId % bin->ChipCount()->X();
    auto y = (tileId / bin->ChipCount()->X()) % bin->ChipCount()->Y();

    auto u = float(bin->ChipSize()->X()) / float(bin->TextureSize()->X());
    auto v = float(bin->ChipSize()->Y()) / float(bin->TextureSize()->Y());

    ResTileCoord result;
    result.Uv0.x = u * x;
    result.Uv0.y = v * (y + 1);
    result.Uv1.x = u * (x + 1);
    result.Uv1.y = v * y;

    return result;
}

//-----------------------------------------------------------------------------
//      指定タイルのプロパティを取得します
//-----------------------------------------------------------------------------
ResTileProp MapChipBinary::GetTileProp(uint32_t index) const
{
    assert(!m_Blob.empty());
    auto prop = res::GetMapChipBinary(m_Blob.data())->TileProps()->Get(index);

    ResTileProp result = {};
    result.Flags   = prop->Flags();
    result.EventId = prop->EventId();
    return result;
}

//-----------------------------------------------------------------------------
//      タイル番号を求めます.
//-----------------------------------------------------------------------------
uint32_t MapChipBinary::CalcTileIndex(uint32_t x, uint32_t y, bool repeat) const
{
    auto bin = res::GetMapChipBinary(m_Blob.data());
    auto tx  = 0u;
    auto ty  = 0u;
    if (repeat)
    {
        tx = x % bin->MapCount()->X();
        ty = y % bin->MapCount()->Y();
    }
    else
    {
        tx = Clamp(x, 0u, bin->MapCount()->X() - 1u);
        ty = Clamp(y, 0u, bin->MapCount()->Y() - 1u);
    }
    return ty * bin->MapCount()->X() + tx;
}

//-----------------------------------------------------------------------------
//      チップ番号を求めます.
//-----------------------------------------------------------------------------
uint32_t MapChipBinary::CalcChipIndex(uint32_t x, uint32_t y) const
{
    auto bin = res::GetMapChipBinary(m_Blob.data());
    auto tx = Clamp(x, 0u, bin->ChipCount()->X() - 1u);
    auto ty = Clamp(y, 0u, bin->ChipCount()->Y() - 1u);
    return ty * bin->ChipCount()->X() + tx;
}

} // namespace asdx


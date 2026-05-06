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
uint32_t MapChipBinary::GetRows() const
{
    assert(!m_Blob.empty());
    return res::GetMapChipBinary(m_Blob.data())->Rows();
}

//-----------------------------------------------------------------------------
//      列数を取得します.
//-----------------------------------------------------------------------------
uint32_t MapChipBinary::GetColumns() const
{
    assert(!m_Blob.empty());
    return res::GetMapChipBinary(m_Blob.data())->Columns();
}

//-----------------------------------------------------------------------------
//      タイルの横幅を取得します.
//-----------------------------------------------------------------------------
uint32_t MapChipBinary::GetTileWidth() const
{
    assert(!m_Blob.empty());
    return res::GetMapChipBinary(m_Blob.data())->TileWidth();
}

//-----------------------------------------------------------------------------
//      タイルの縦幅を取得します.
//-----------------------------------------------------------------------------
uint32_t MapChipBinary::GetTileHeight() const
{
    assert(!m_Blob.empty());
    return res::GetMapChipBinary(m_Blob.data())->TileHeight();
}

//-----------------------------------------------------------------------------
//      タイルセット数を取得します.
//-----------------------------------------------------------------------------
uint32_t MapChipBinary::GetTileSetCount() const
{
    assert(!m_Blob.empty());
    return res::GetMapChipBinary(m_Blob.data())->TileSets()->size();
}

//-----------------------------------------------------------------------------
//      タイルセットを取得します.
//-----------------------------------------------------------------------------
ResTileSet MapChipBinary::GetTileSet(uint32_t tileSetIndex) const
{
    assert(!m_Blob.empty());
    auto tileSet = res::GetMapChipBinary(m_Blob.data())->TileSets()->Get(tileSetIndex);

    ResTileSet result = {};
    result.Name          = StringView(tileSet->Name()->c_str());
    result.FirstChipId   = tileSet->FirstChipId();
    result.Columns       = tileSet->Columns();
    result.TileCount     = tileSet->TileCount();
    result.TileWidth     = tileSet->TileWidth();
    result.TileHeight    = tileSet->TileHeight();
    result.TextureWidth  = tileSet->TextureWidth();
    result.TextureHeight = tileSet->TextureHeight();
    result.TexturePath   = StringView(tileSet->TexturePath()->c_str());
    return result;
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
ResTileMapLayer MapChipBinary::GetLayer(uint32_t layerIndex) const
{
    assert(!m_Blob.empty());
    auto layer = res::GetMapChipBinary(m_Blob.data())->Layers()->Get(layerIndex);

    ResTileMapLayer result = {};
    result.Name     = StringView(layer->Name()->c_str());
    result.Id       = layer->Id();
    result.Rows     = layer->Rows();
    result.Columns  = layer->Columns();
    result.Data     = ArrayView<uint16_t>(layer->Data()->data(), layer->Data()->size());
    return result;
}

//-----------------------------------------------------------------------------
//      指定タイルのテクスチャ座標を取得します.
//-----------------------------------------------------------------------------
ResTileCoord MapChipBinary::GetCoord(uint32_t tileSetIndex, uint32_t tileId) const
{
    assert(!m_Blob.empty());
    auto tileSet = res::GetMapChipBinary(m_Blob.data())->TileSets()->Get(tileSetIndex);
    auto x = tileId % tileSet->Columns();
    auto y = tileId / tileSet->Columns();

    auto u = float(tileSet->TileWidth ()) / float(tileSet->TextureWidth());
    auto v = float(tileSet->TileHeight()) / float(tileSet->TextureHeight());

    ResTileCoord result;
    result.Uv0.x = u * x;
    result.Uv0.y = v * (y + 1);
    result.Uv1.x = u * (x + 1);
    result.Uv1.y = v * y;

    return result;
}

//-----------------------------------------------------------------------------
//      チッププロパティを検索します.
//-----------------------------------------------------------------------------
bool MapChipBinary::FindChipProperty(uint32_t tileSetIndex, uint16_t chipId, ResChipProperty& result) const
{
    assert(!m_Blob.empty());
    auto chip = res::GetMapChipBinary(m_Blob.data())
                ->TileSets()
                ->Get(tileSetIndex)
                ->Tiles()
                ->LookupByKey(chipId);
    if (chip == nullptr)
    {
        result.Id        = chipId;
        result.Collision = false;
        result.Event     = 0;
        return false;
    }

    result.Id        = chip->Id();
    result.Collision = chip->Collision();
    result.Event     = chip->Event();
    return true;
}

} // namespace asdx


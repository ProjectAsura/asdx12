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
: m_pBlob(nullptr)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
MapChipBinary::~MapChipBinary()
{ Term(); }

//-----------------------------------------------------------------------------
//      バイナリをロードします.
//-----------------------------------------------------------------------------
bool MapChipBinary::LoadA(const char* path)
{
    if (!ReadFileToBlobA(path, &m_pBlob))
    { return false; }

#if ASDX_DEBUG
    // デバッグ整合性をチェック.
    {
        assert(m_pBlob != nullptr);
        flatbuffers::Verifier::Options options;
        flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(m_pBlob->GetBuffer()), m_pBlob->GetBufferSize());
        assert(res::VerifyMapChipBinaryBuffer(verifier));
        ASDX_UNUSED(verifier);
    }
#endif

    return true;
}

//-----------------------------------------------------------------------------
//      バイナリをロードします.
//-----------------------------------------------------------------------------
bool MapChipBinary::LoadW(const wchar_t* path)
{
    if (!ReadFileToBlobW(path, &m_pBlob))
    { return false; }

#if ASDX_DEBUG
    // デバッグ整合性をチェック.
    {
        assert(m_pBlob != nullptr);
        flatbuffers::Verifier::Options options;
        flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(m_pBlob->GetBuffer()), m_pBlob->GetBufferSize());
        assert(res::VerifyMapChipBinaryBuffer(verifier));
        ASDX_UNUSED(verifier);
    }
#endif

    return true;

}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void MapChipBinary::Term()
{
    if (m_pBlob != nullptr)
    {
        m_pBlob->Release();
        m_pBlob = nullptr;
    }
}

//-----------------------------------------------------------------------------
//      行数を取得します.
//-----------------------------------------------------------------------------
uint32_t MapChipBinary::GetRows() const
{
    assert(m_pBlob != nullptr);
    assert(m_pBlob->GetBuffer() != nullptr);
    return res::GetMapChipBinary(m_pBlob->GetBuffer())->Rows();
}

//-----------------------------------------------------------------------------
//      列数を取得します.
//-----------------------------------------------------------------------------
uint32_t MapChipBinary::GetColumns() const
{
    assert(m_pBlob != nullptr);
    assert(m_pBlob->GetBuffer() != nullptr);
    return res::GetMapChipBinary(m_pBlob->GetBuffer())->Columns();
}

//-----------------------------------------------------------------------------
//      タイルの横幅を取得します.
//-----------------------------------------------------------------------------
uint32_t MapChipBinary::GetTileWidth() const
{
    assert(m_pBlob != nullptr);
    assert(m_pBlob->GetBuffer() != nullptr);
    return res::GetMapChipBinary(m_pBlob->GetBuffer())->TileWidth();
}

//-----------------------------------------------------------------------------
//      タイルの縦幅を取得します.
//-----------------------------------------------------------------------------
uint32_t MapChipBinary::GetTileHeight() const
{
    assert(m_pBlob != nullptr);
    assert(m_pBlob->GetBuffer() != nullptr);
    return res::GetMapChipBinary(m_pBlob->GetBuffer())->TileHeight();
}

//-----------------------------------------------------------------------------
//      タイルセット数を取得します.
//-----------------------------------------------------------------------------
uint32_t MapChipBinary::GetTileSetCount() const
{
    assert(m_pBlob != nullptr);
    assert(m_pBlob->GetBuffer() != nullptr);
    return res::GetMapChipBinary(m_pBlob->GetBuffer())->TileSets()->size();
}

//-----------------------------------------------------------------------------
//      タイルセットを取得します.
//-----------------------------------------------------------------------------
ResTileSet MapChipBinary::GetTileSet(uint32_t tileSetIndex) const
{
    assert(m_pBlob != nullptr);
    assert(m_pBlob->GetBuffer() != nullptr);

    auto tileSet = res::GetMapChipBinary(m_pBlob->GetBuffer())->TileSets()->Get(tileSetIndex);

    ResTileSet result = {};
    result.FirstChipId  = tileSet->FirstChipId();
    result.Columns      = tileSet->Columns();
    result.TileCount    = tileSet->TileCount();
    result.TileWidth    = tileSet->TileWidth();
    result.TileHeight   = tileSet->TileHeight();
    return result;
}

//-----------------------------------------------------------------------------
//      マップチップを取得します.
//-----------------------------------------------------------------------------
ResTexture MapChipBinary::GetMapChip(uint32_t tileSetIndex) const
{
    assert(m_pBlob != nullptr);
    assert(m_pBlob->GetBuffer() != nullptr);

    auto tileSet = res::GetMapChipBinary(m_pBlob->GetBuffer())->TileSets()->Get(tileSetIndex);

    ResTexture result = {};
    result.Dimension                    = TEXTURE_DIMENSION_2D;
    result.Width                        = tileSet->Image()->Width();
    result.Height                       = tileSet->Image()->Height();
    result.DepthOrArraySize             = 1;
    result.MipLevels                    = 1;
    result.SubResourceCount             = 1;
    result.SubResources[0].Width        = tileSet->Image()->Width();
    result.SubResources[0].Height       = tileSet->Image()->Height();
    result.SubResources[0].RowPitch     = tileSet->Image()->RowPitch();
    result.SubResources[0].SlicePitch   = tileSet->Image()->SlicePitch();
    result.SubResources[0].pPixels      = tileSet->Image()->Pixels()->data();

    return result;
}

//-----------------------------------------------------------------------------
//      レイヤー数を取得します.
//-----------------------------------------------------------------------------
uint32_t MapChipBinary::GetLayerCount() const
{
    assert(m_pBlob != nullptr);
    assert(m_pBlob->GetBuffer() != nullptr);
    return res::GetMapChipBinary(m_pBlob->GetBuffer())->Layers()->size();
}

//-----------------------------------------------------------------------------
//      レイヤーを取得します.
//-----------------------------------------------------------------------------
ResTileMapLayer MapChipBinary::GetLayer(uint32_t layerIndex) const
{
    assert(m_pBlob != nullptr);
    assert(m_pBlob->GetBuffer() != nullptr);
    auto layer = res::GetMapChipBinary(m_pBlob->GetBuffer())->Layers()->Get(layerIndex);

    ResTileMapLayer result = {};
    result.Id       = layer->Id();
    result.Rows     = layer->Rows();
    result.Columns  = layer->Columns();
    result.Data     = ArrayView<uint32_t>(layer->Data()->data(), layer->Data()->size());
    return result;
}

} // namespace asdx


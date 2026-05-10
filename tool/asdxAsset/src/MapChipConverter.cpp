//-----------------------------------------------------------------------------
// File : MapChipConverter.cpp
// Desc : Map Chip Converter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

#define NOMINMAX

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdio>
#include <vector>
#include <string>
#include <sstream>
#include <MapChipConverter.h>
#include <TextureConverter.h>
#include <tinyxml2.h>
#include <filesystem>
#include "MapChipBinary_generated.h"


#ifndef ELOG
#define ELOG(x, ...) fprintf_s(stderr, "[File:%s, Line:%d] " x "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif//ELOG


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t CURRENT_VERSION = 3u;     //!< 現在のバイナリバージョン.


///////////////////////////////////////////////////////////////////////////////
// TILE_FLAG enumeration
///////////////////////////////////////////////////////////////////////////////
enum TILE_FLAG
{
    TILE_FLAG_COLLISION = 0x1 << 0,   //!< 衝突判定あり.
    TILE_FLAG_EVENT     = 0x1 << 1,   //!< イベントあり.
    TILE_FLAG_DAMAGE    = 0x1 << 2,   //!< ダメージあり.
    TILE_FLAG_TOL       = 0x1 << 3,   //!< 離着陸可能 (TOL: Take Off and Landing).
};

///////////////////////////////////////////////////////////////////////////////
// TileProp structure
///////////////////////////////////////////////////////////////////////////////
struct TileProp
{
    bool    HasCollision;
    bool    HasEvent;
    bool    HasDamage;
    bool    EnableTOL;
};

////////////////////////////////////////////////////////////////////////////////
// Frame structure
////////////////////////////////////////////////////////////////////////////////
struct Frame
{
    uint32_t    Id;
    float       Duration;
};

///////////////////////////////////////////////////////////////////////////////
// Tile structure
///////////////////////////////////////////////////////////////////////////////
struct Tile
{
    uint16_t                Id;
    TileProp                Prop;
    std::vector<Frame>      Frames;
};

///////////////////////////////////////////////////////////////////////////////
// Image structure
///////////////////////////////////////////////////////////////////////////////
struct Image
{
    uint32_t    Width;
    uint32_t    Height;
    std::string Source;
};

///////////////////////////////////////////////////////////////////////////////
// TileSet structure
///////////////////////////////////////////////////////////////////////////////
struct TileSet
{
    std::string         Name;
    uint16_t            FirstChipId;
    uint32_t            ColumnCount;
    uint32_t            TileWidth;
    uint32_t            TileHeight;
    uint32_t            TileCount;
    Image               Image;
    std::vector<Tile>   Tiles;
};

///////////////////////////////////////////////////////////////////////////////
// Layer structure
///////////////////////////////////////////////////////////////////////////////
struct Layer
{
    std::string             Name;
    uint32_t                Id;
    uint32_t                Width;
    uint32_t                Height;
    std::vector<uint16_t>   Data;
};

///////////////////////////////////////////////////////////////////////////////
// Object structure
///////////////////////////////////////////////////////////////////////////////
struct Object
{
    std::string Name;
    uint16_t    Id;
    uint16_t    EventId;
    uint32_t    X;
    uint32_t    Y;
    uint32_t    Width;
    uint32_t    Height;
};

///////////////////////////////////////////////////////////////////////////////
// ObjectGroup structure
///////////////////////////////////////////////////////////////////////////////
struct ObjectGroup
{
    uint32_t            Id;
    std::string         Name;
    bool                Locked;
    std::vector<Object> Objects;
};

///////////////////////////////////////////////////////////////////////////////
// MapData structure
///////////////////////////////////////////////////////////////////////////////
struct MapData
{
    uint32_t                    Width;
    uint32_t                    Height;
    uint32_t                    TileWidth;
    uint32_t                    TileHeight;
    bool                        Infinite;
    uint32_t                    NextLayerId;
    uint32_t                    NextObjectId;
    std::vector<TileSet>        TileSets;
    std::vector<Layer>          Layers;
    std::vector<ObjectGroup>    ObjectGroups;
};


//-----------------------------------------------------------------------------
//      カンマ区切りを配列に変換します.
//-----------------------------------------------------------------------------
std::vector<uint16_t> ParseCsv(const char* csv)
{
    std::istringstream stream(csv);

    std::string field;
    std::vector<uint16_t> result;
    while (std::getline(stream, field, ','))
    {
        auto u = uint16_t(std::stoi(field));
        result.push_back(u);
    }
    return result;
}

//-----------------------------------------------------------------------------
//      ファイルパスを連結します.
//-----------------------------------------------------------------------------
std::string PathCombine(const std::string& lhs, const std::string& rhs)
{
    if (rhs.empty())
        return std::string();
    if (lhs.empty())
        return rhs;

    return lhs + "\\" + rhs;
}

//-----------------------------------------------------------------------------
//      テクスチャコンバートを行います.
//-----------------------------------------------------------------------------
void ConvertTXB(const std::string& input, const std::string& output)
{
    TextureConverter::Desc desc = {};
    desc.InputPath  = input;
    desc.OutputPath = output;

    if (!TextureConverter::Convert(desc))
    { ELOG("Error : TextureConverter::Convert() Failed. path = %s", input.c_str()); }
}

//-----------------------------------------------------------------------------
//      マップデータをロードします.
//-----------------------------------------------------------------------------
bool LoadMapData(const char* path, MapData& outData)
{
    tinyxml2::XMLDocument doc;
    auto err = doc.LoadFile(path);
    if (err != tinyxml2::XML_SUCCESS)
    {
        ELOG("Error : Txm File Load Filed. path = %s, errcode = 0x%x", path, err);
        return false;
    }

    auto map = doc.FirstChildElement("map");
    if (map == nullptr)
    {
        ELOG("Error : Map Element Not Found");
        return false;
    }

    outData.Width        = map->UnsignedAttribute("width");
    outData.Height       = map->UnsignedAttribute("height");
    outData.TileWidth    = map->UnsignedAttribute("tilewidth");
    outData.TileHeight   = map->UnsignedAttribute("tileheight");
    outData.Infinite     = map->BoolAttribute("infinite");
    outData.NextLayerId  = map->UnsignedAttribute("nextlayerid");
    outData.NextObjectId = map->UnsignedAttribute("nextobjectid");

    // tileset
    auto tileSet = map->FirstChildElement("tileset");
    if (tileSet != nullptr)
    {
        TileSet ts;
        ts.FirstChipId = tileSet->UnsignedAttribute("firstgid");
        ts.Name        = tileSet->Attribute("name");
        ts.ColumnCount = tileSet->UnsignedAttribute("columns");
        ts.TileWidth   = tileSet->UnsignedAttribute("tilewidth");
        ts.TileHeight  = tileSet->UnsignedAttribute("tileheight");
        ts.TileCount   = tileSet->UnsignedAttribute("tilecount");

        auto image = tileSet->FirstChildElement("image");
        if (image != nullptr)
        {
            ts.Image.Width  = image->UnsignedAttribute("width");
            ts.Image.Height = image->UnsignedAttribute("height");
            ts.Image.Source = image->Attribute("source");
        }

        for(auto tile = tileSet->FirstChildElement("tile"); tile != nullptr; tile = tile->NextSiblingElement("tile"))
        {
            Tile t;
            t.Id = tile->UnsignedAttribute("id");

            auto props = tile->FirstChildElement("properties");
            if (props != nullptr)
            {
                for(auto prop = props->FirstChildElement("property"); prop != nullptr; prop = prop->NextSiblingElement("property"))
                {
                    TileProp tp = {};
                    auto name = prop->Attribute("name");
                    if (strcmp(name, "collision") == 0)
                    {
                        tp.HasCollision = prop->BoolAttribute("value");
                    }
                    else if (strcmp(name, "event") == 0)
                    {
                        tp.HasEvent = prop->BoolAttribute("value");
                    }
                    else if (strcmp(name, "damage") == 0)
                    {
                        tp.HasDamage = prop->BoolAttribute("value");
                    }
                    else if (strcmp(name, "tol") == 0)
                    {
                        tp.EnableTOL = prop->BoolAttribute("value");
                    }

                    t.Prop = tp;
                }
            }

            auto anims = tile->FirstChildElement("animation");
            if (anims != nullptr)
            {
                for(auto frame = anims->FirstChildElement("frame"); frame != nullptr; frame = frame->NextSiblingElement("frame"))
                {
                    auto tileId   = frame->UnsignedAttribute("tileid");
                    auto duration = frame->FloatAttribute("duration");

                    Frame f;
                    f.Id        = tileId;
                    f.Duration  = duration;
                    t.Frames.push_back(f);
                }
            }

            ts.Tiles.emplace_back(t);
        }

        outData.TileSets.emplace_back(ts);
    }

    // layer
    for(auto layer = map->FirstChildElement("layer"); layer != nullptr; layer = layer->NextSiblingElement("layer"))
    {
        Layer l;
        l.Name   = layer->Attribute("name");
        l.Id     = layer->UnsignedAttribute("id");
        l.Width  = layer->UnsignedAttribute("width");
        l.Height = layer->UnsignedAttribute("height");

        auto data     = layer->FirstChildElement("data");
        auto encoding = data->Attribute("encoding");
        auto ret      = (strcmp(encoding, "csv") == 0);
        assert(ret);
        if (!ret)
        {
            ELOG("Error : data encoding is not \"csv\".");
            return false;
        }

        l.Data = ParseCsv(data->GetText());

        outData.Layers.emplace_back(l);
    }

    // objectgroup
    for(auto objectGroup = map->FirstChildElement("objectgroup"); objectGroup != nullptr; objectGroup = objectGroup->NextSiblingElement("objectgroup"))
    {
        ObjectGroup og;
        og.Id     = objectGroup->UnsignedAttribute("id");
        og.Name   = objectGroup->Attribute("name");
        og.Locked = objectGroup->BoolAttribute("locked");
        for(auto object = objectGroup->FirstChildElement("object"); object != nullptr; object = object->NextSiblingElement("object"))
        {
            uint16_t eventId = UINT16_MAX;

            auto props = object->FirstChildElement("properties");
            if (props != nullptr)
            {
                for(auto prop = props->FirstChildElement("property"); prop != nullptr; prop = prop->NextSiblingElement("property"))
                {
                    auto name = prop->Attribute("name");
                    if (strcmp(name, "event") == 0)
                    {
                        eventId = uint16_t(prop->UnsignedAttribute("value"));
                        break;
                    }
                }
            }

            Object o;
            o.Id        = object->UnsignedAttribute("id");
            o.EventId   = eventId;
            o.Name      = object->Attribute("name");
            o.X         = object->UnsignedAttribute("x");
            o.Y         = object->UnsignedAttribute("y");
            o.Width     = object->UnsignedAttribute("width");
            o.Height    = object->UnsignedAttribute("height");
            og.Objects.emplace_back(o);
        }
        outData.ObjectGroups.emplace_back(og);
    }

    return true;
}

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// MapChipConverter class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      現在のバイナリバージョンを取得します.
//-----------------------------------------------------------------------------
uint32_t MapChipConverter::GetCurrentVersion()
{ return CURRENT_VERSION; }

//-----------------------------------------------------------------------------
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool MapChipConverter::Convert(const Desc& desc)
{
    if (desc.InputPath.empty() || desc.OutputPath.empty())
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    MapData mapData = {};
    if (!LoadMapData(desc.InputPath.c_str(), mapData))
    {
        ELOG("Error : LoadMapData() Failed.");
        return false;
    }

    flatbuffers::FlatBufferBuilder builder(1024);

    const auto& tileSet = mapData.TileSets[0];

    std::vector<flatbuffers::Offset<asdx::res::MapChip>> dstChips;
    for(auto i=0; i<tileSet.Tiles.size(); ++i)
    {
        const auto& tile = tileSet.Tiles[i];

        std::vector<asdx::res::MapChipFrame> dstFrames;
        for(auto j=0; j<tile.Frames.size(); ++j)
        {
            dstFrames.emplace_back(
                asdx::res::MapChipFrame(tile.Frames[j].Id, tile.Frames[j].Duration));
        }

        auto dstChip = asdx::res::CreateMapChipDirect(
            builder,
            tile.Id,
            &dstFrames);
    }

    struct ConvTileProp
    {
        uint16_t    Flags   = 0;
        uint16_t    EventId = 0;
    };

    std::vector<ConvTileProp> convProps;
    convProps.resize(tileSet.TileCount);
    for(auto i=0; i<convProps.size(); ++i)
    {
        convProps[i].Flags   = 0;
        convProps[i].EventId = UINT16_MAX;
    }

    std::vector<flatbuffers::Offset<asdx::res::MapLayer>> dstLayers;
    for(auto i=0; i<mapData.Layers.size(); ++i)
    {
        for(auto j=0u; j<tileSet.TileCount; ++j)
        {
            auto id = mapData.Layers[i].Data[j];
            if (id < tileSet.FirstChipId)
                continue;

            auto itr = std::find_if(tileSet.Tiles.begin(), tileSet.Tiles.end(), [id](const Tile& val)
            {
                return id  == val.Id;
            });

            if (itr == tileSet.Tiles.end())
                continue;

            const auto& tile = (*itr);
            uint16_t flags = 0;
            if (tile.Prop.HasCollision)
            {
                flags |= TILE_FLAG_COLLISION;
            }
            if (tile.Prop.HasEvent)
            {
                flags |= TILE_FLAG_EVENT;
            }
            if (tile.Prop.HasDamage)
            {
                flags |= TILE_FLAG_DAMAGE;
            }
            if (tile.Prop.EnableTOL)
            {
                flags |= TILE_FLAG_TOL;
            }

            convProps[j].Flags |= flags;
        }

        dstLayers.emplace_back(
            asdx::res::CreateMapLayerDirect(
                builder,
                mapData.Layers[i].Name.c_str(),
                &mapData.Layers[i].Data));
    }

    // イベントIDを仕込む
    for(auto i=0; i<mapData.ObjectGroups.size(); ++i)
    {
        const auto& group = mapData.ObjectGroups[i];
        for(auto j=0; j<group.Objects.size(); ++j)
        {
            auto& obj = group.Objects[j];

            if (obj.EventId == UINT16_MAX)
                continue;

            // 配置位置からタイルを算出.
            auto x0 = obj.X / mapData.TileWidth;
            auto y0 = obj.Y / mapData.TileHeight;

            auto x1 = (obj.X + obj.Width  + 0.5f) / mapData.TileWidth;
            auto y1 = (obj.Y + obj.Height + 0.5f) / mapData.TileHeight;

            for(auto y=y0; y<y1; ++y)
            {
                for(auto x=x0; x<x1; ++x)
                {
                    auto tileId = y * mapData.Width + x;
                    if (tileId >= convProps.size())
                        continue;

                    // タイルがイベントを持つかどうかチェック.
                    if (!(convProps[tileId].Flags & TILE_FLAG_EVENT))
                        continue;

                    // タイルにイベントIDを仕込む.
                    convProps[tileId].EventId = obj.EventId;
                }
            }
        }
    }

    std::vector<asdx::res::MapTileProp> dstProps;
    for(auto i=0; i<convProps.size(); ++i)
    {

        dstProps.emplace_back(
            asdx::res::MapTileProp(
                convProps[i].Flags,
                convProps[i].EventId)
        );
    }

    auto tileRows   = tileSet.TileCount / tileSet.ColumnCount;
    auto tileCols   = tileSet.ColumnCount;
    auto chipWidth  = mapData.TileWidth;
    auto chipHeight = mapData.TileHeight;

    asdx::res::Uint2 mapCount    = {mapData.Width, mapData.Height};
    asdx::res::Uint2 chipCount   = {tileCols, tileRows};
    asdx::res::Uint2 chipSize    = {chipWidth, chipHeight};
    asdx::res::Uint2 textureSize = {tileSet.Image.Width, tileSet.Image.Height};

    std::filesystem::path p = tileSet.Image.Source.c_str();
    std::string texturePath = "textures\\" + p.filename().replace_extension(".txb").string();

    auto firstChipId = tileSet.FirstChipId;

    // Binary出力.
    {
        auto bin = asdx::res::CreateMapChipBinaryDirect(
            builder,
            CURRENT_VERSION,
            &mapCount,
            &chipCount,
            &chipSize,
            &textureSize,
            texturePath.c_str(),
            firstChipId,
            &dstLayers,
            &dstProps,
            &dstChips);

        builder.Finish(bin);

        auto buf =builder.GetBufferPointer();
        auto size = builder.GetSize();

        FILE* fp = nullptr;
        auto err = fopen_s(&fp, desc.OutputPath.c_str(), "wb");
        if (err != 0)
        {
            ELOG("Error : Output File Open Failed. path = %s, errcode = 0x%x", desc.OutputPath.c_str(), err);
            return false;
        }

        fwrite(buf, size, 1, fp);
        fclose(fp);
    }

    return true;
}

} // namespace asdx

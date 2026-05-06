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
#include <sstream>
#include <MapChipConverter.h>
#include <TextureConverter.h>
#include <tinyxml2.h>
#include "MapChipBinary_generated.h"
#include <filesystem>


#ifndef ELOG
#define ELOG(x, ...) fprintf_s(stderr, "[File:%s, Line:%d] " x "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif//ELOG


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t CURRENT_VERSION = 2u;     //!< 現在のバイナリバージョン.


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

    tinyxml2::XMLDocument doc;
    auto err = doc.LoadFile(desc.InputPath.c_str());
    if (err != tinyxml2::XML_SUCCESS)
    {
        ELOG("Error : Txm File Load Failed. path = %s, errcode = %d", desc.InputPath.c_str(), err);
        return false;
    }

    auto map = doc.FirstChildElement("map");
    if (map == nullptr)
    {
        ELOG("Error : Map Element Not Found.");
        return false;
    }

    uint32_t rows       = map->UnsignedAttribute("height");
    uint32_t columns    = map->UnsignedAttribute("width");
    uint32_t tileWidth  = map->UnsignedAttribute("tilewidth");
    uint32_t tileHeight = map->UnsignedAttribute("tileheight");

    flatbuffers::FlatBufferBuilder builder(1024);

    std::vector<flatbuffers::Offset<asdx::res::TileSet>> tilesets;
    std::vector<flatbuffers::Offset<asdx::res::Layer>>   layers;

    std::filesystem::path txbOutPath = desc.OutputPath;
    auto txbOutDir = txbOutPath.parent_path().string();

    // tileset
    {
        for(auto tileset = map->FirstChildElement("tileset"); tileset != nullptr; tileset = tileset->NextSiblingElement("tileset"))
        {
            auto image = tileset->FirstChildElement("image");
            uint32_t texW = 0;
            uint32_t texH = 0;
            std::string texPath;
            if (image != nullptr)
            {
                texW    = image->UnsignedAttribute("width");
                texH    = image->UnsignedAttribute("height");

                // 名前を差し替え.
                std::filesystem::path p = image->Attribute("source");
                texPath = "textures\\" + p.filename().replace_extension(".txb").string();

                if (desc.TextureConvert)
                {
                    ConvertTXB(
                        image->Attribute("source"),
                        PathCombine(txbOutDir, texPath));
                }
            }

            auto firstChipId    = tileset->UnsignedAttribute("firstgid", 1);
            auto name           = tileset->Attribute("name");
            auto tileW          = tileset->UnsignedAttribute("tilewidth");
            auto tileH          = tileset->UnsignedAttribute("tileheight");
            auto tileCount      = tileset->UnsignedAttribute("tilecount");
            auto columnCount    = tileset->UnsignedAttribute("columns");

            std::vector<asdx::res::Tile> tiles;
            for(auto tile = tileset->FirstChildElement("tile"); tile != nullptr; tile = tile->NextSiblingElement("tile"))
            {
                uint16_t id = uint16_t(tile->UnsignedAttribute("id"));

                bool     collision = false;
                uint32_t eventId   = 0;

                auto props = tile->FirstChildElement("properties");
                if (props != nullptr)
                {
                    for(auto prop = props->FirstChildElement("property"); prop != nullptr; prop = prop->NextSiblingElement("property"))
                    {
                        auto name = prop->Attribute("name");
                        if (_stricmp(name, "collision") == 0)
                        {
                            collision = prop->BoolAttribute("value");
                        }
                        else if (_stricmp(name, "event") == 0)
                        {
                            eventId = prop->UnsignedAttribute("value");
                        }
                    }
                }

                tiles.emplace_back(id, collision, eventId);
            }

            tilesets.emplace_back(
                asdx::res::CreateTileSetDirect(
                    builder,
                    name,
                    firstChipId,
                    columnCount,
                    tileCount,
                    tileW,
                    tileH,
                    texW,
                    texH,
                    texPath.c_str(),
                    &tiles));
        }
    }

    // layer
    {
        for(auto layer = map->FirstChildElement("layer"); layer != nullptr; layer = layer->NextSiblingElement("layer"))
        {
            auto id         = layer->UnsignedAttribute("id");
            auto name       = layer->Attribute("name");
            auto rowCount   = layer->UnsignedAttribute("height");
            auto colCount   = layer->UnsignedAttribute("width");
            auto data       = layer->FirstChildElement("data");

            auto encoding = data->Attribute("encoding");
            auto ret = (strcmp(encoding, "csv") == 0);
            assert(ret);
            if (!ret)
            {
                ELOG("Error : data encoding is not \"csv\".");
                return false;
            }

            auto datas = ParseCsv(data->GetText());

            layers.emplace_back(
                asdx::res::CreateLayerDirect(
                    builder,
                    name,
                    id,
                    rowCount,
                    colCount,
                    &datas));
        }
    }

    // Binary出力.
    {
        auto bin = asdx::res::CreateMapChipBinaryDirect(
            builder,
            CURRENT_VERSION,
            rows,
            columns,
            tileWidth,
            tileHeight,
            &tilesets,
            &layers);

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

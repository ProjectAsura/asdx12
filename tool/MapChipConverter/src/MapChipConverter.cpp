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
#include <DirectXTex.h>
#include <tinyxml2.h>
#include "MapChipBinary_generated.h"


#ifndef ELOG
#define ELOG(x, ...) fprintf_s(stderr, "[File:%s, Line:%d] " x "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif//ELOG


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t CURRENT_VERSION = 1u;     //!< 現在のバイナリバージョン.


//-----------------------------------------------------------------------------
//      ワイド文字列に変換します.
//-----------------------------------------------------------------------------
std::wstring ToStringW( const std::string& value )
{
    auto length = MultiByteToWideChar(CP_ACP, 0, value.c_str(), int(value.size() + 1), nullptr, 0 );
    auto buffer = new wchar_t[length];

    MultiByteToWideChar(CP_ACP, 0, value.c_str(), int(value.size() + 1),  buffer, length );

    std::wstring result( buffer );
    delete[] buffer;

    return result;
}

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

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// MapChipConverter class
///////////////////////////////////////////////////////////////////////////////

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

    // tileset
    {
        for(auto tileset = map->FirstChildElement("tileset"); tileset != nullptr; tileset = tileset->NextSiblingElement("tileset"))
        {
            auto image = tileset->FirstChildElement("image");
            uint32_t imageWidth  = 0;
            uint32_t imageHeight = 0;
            flatbuffers::Offset<asdx::res::MapChip> mapChip;
            if (image != nullptr)
            {
                imageWidth  = image->UnsignedAttribute("width");
                imageHeight = image->UnsignedAttribute("height");

                std::vector<uint8_t> pixels;

                auto path = image->Attribute("source");

                // テクスチャロード.
                DirectX::TexMetadata  metaData     = {};
                DirectX::ScratchImage scratchImage = {};

                std::wstring inputPath = ToStringW(path);
                auto hr = DirectX::LoadFromWICFile(
                    inputPath.c_str(),
                    DirectX::WIC_FLAGS_NONE,
                    &metaData,
                    scratchImage);

                assert(imageWidth  == metaData.width);
                assert(imageHeight == metaData.height);

                pixels.resize(scratchImage.GetPixelsSize());
                memcpy(pixels.data(), scratchImage.GetPixels(), pixels.size());

                auto images = scratchImage.GetImages();

                std::vector<asdx::res::MapChipSubResource> subResources;
                subResources.push_back(asdx::res::MapChipSubResource(
                    uint32_t(images[0].width),
                    uint32_t(images[0].height),
                    images[0].rowPitch,
                    images[0].slicePitch,
                    0
                ));

                mapChip = asdx::res::CreateMapChipDirect(
                    builder,
                    imageWidth,
                    imageHeight,
                    uint32_t(metaData.format),
                    &subResources,
                    &pixels);

                scratchImage.Release();
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
                    mapChip,
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
            ELOG("Error : Output File Open Failed. path = %s", desc.OutputPath.c_str());
            return false;
        }

        fwrite(buf, size, 1, fp);
        fclose(fp);
    }

    return true;
}

} // namespace asdx

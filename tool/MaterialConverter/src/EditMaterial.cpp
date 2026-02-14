//-----------------------------------------------------------------------------
// File : EditMaterial.cpp
// Desc : Material For Edit Data.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "EditMaterial.h"
#include <simdjson.h>



namespace asdx::edit {

//-----------------------------------------------------------------------------
//      マテリアルを json ファイルに保存します.
//-----------------------------------------------------------------------------
bool SaveToJson(const char* path, const Material& material)
{
    return false;
}

//-----------------------------------------------------------------------------
//      json ファイルからマテリアルを読込します.
//-----------------------------------------------------------------------------
bool LoadfromJson(const char* path, Material& material)
{
    simdjson::ondemand::parser parser;

    auto json = simdjson::padded_string::load(path);
    if (json.error() != simdjson::SUCCESS)
    {
        return false;
    }

    auto doc = parser.iterate(json);
    if (json.error() != simdjson::SUCCESS)
    {
        return false;
    }

    auto name = doc["Name"];
    if (name.error() == simdjson::SUCCESS)
    {
    }

    auto state = doc["State"];
    if (state.error() == simdjson::SUCCESS)
    {
    }

    auto texture = doc["Textures"];
    if (texture.error() == simdjson::SUCCESS)
    {
        for(auto tex : texture.get_array())
        {
        }
    }

    auto buffers = doc["Buffers"];
    if (buffers.error() == simdjson::SUCCESS)
    {
        for(auto buf : buffers.get_array())
        {
        }
    }

    return false;
}

} // namespace asdx::edit
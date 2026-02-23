//-----------------------------------------------------------------------------
// File : EditMaterial.cpp
// Desc : Material For Edit Data.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "EditMaterial.h"
#include <cstdio>
#include <simdjson.h>


#ifndef ELOG
#define ELOG(x, ...) fprintf_s(stderr, "[File: %s, Line: %d] " x "\n", __FILE__, __LINE__, ##__VA_ARGS__ )
#endif//ELOG


namespace edit {

//-----------------------------------------------------------------------------
//      マテリアルを json ファイルに保存します.
//-----------------------------------------------------------------------------
bool SaveToJson(const char* path, const Material& material)
{
    FILE* fp = nullptr;
    auto err = fopen_s(&fp, path, "w");
    if (err != 0)
    {
        ELOG("Error : File Open Failed. path = %s", path);
        return false;
    }

    fprintf_s(fp, "{\n");
    fprintf_s(fp, "    \"Name\": \"%s\",\n", material.Name.c_str());
    fprintf_s(fp, "    \"Kind\": %u,\n", material.Kind);
    if (!material.Textures.empty())
    {
        fprintf_s(fp, ",\n");
        fprintf_s(fp, "    \"Textures\": [\n");
        auto count = material.Textures.size();
        auto index = 0;
        for(auto& texture : material.Textures)
        {
            fprintf_s(fp, "        {\n");
            fprintf_s(fp, "             \"Name\": \"%s\",\n", texture.first.c_str());
            fprintf_s(fp, "             \"Path\": \"%s\"\n", texture.second.c_str());
            fprintf_s(fp, "        }");

            if (index != count - 1)
            { fprintf_s(fp, ",\n"); }
            else
            { fprintf_s(fp, "\n"); }

            index++;
        }

        fprintf_s(fp, "     ]");
    }
    if (!material.Params.empty())
    {
        fprintf_s(fp, ",\n");
        fprintf_s(fp, "    \"Params\": [\n");
        auto count = material.Params.size();
        auto index = 0;
        for(auto& param : material.Params)
        {
            fprintf_s(fp, "        {\n");
            fprintf_s(fp, "             \"Name\": \"%s\",\n", param.first.c_str());
            fprintf_s(fp, "             \"Value\": %f\n", param.second);
            fprintf_s(fp, "        }");

            if (index != count - 1)
            { fprintf_s(fp, ",\n"); }
            else
            { fprintf_s(fp, "\n"); }

            index++;
        }
        fprintf_s(fp, "    ]");
        // 次のデータ無いのでここで改行.
        fprintf_s(fp, "\n");
    }

    fprintf_s(fp, "}\n");

    return true;
}

//-----------------------------------------------------------------------------
//      json ファイルからマテリアルを読込します.
//-----------------------------------------------------------------------------
bool LoadFromJson(const char* path, Material& material)
{
    simdjson::ondemand::parser parser;

    auto json = simdjson::padded_string::load(path);
    if (json.error() != simdjson::SUCCESS)
    {
        ELOG("Error : File Load Failed. path = %s", path);
        return false;
    }

    auto doc = parser.iterate(json);
    if (json.error() != simdjson::SUCCESS)
    {
        ELOG("Error : simdjson parser error.");
        return false;
    }

    auto name = doc["Name"];
    if (name.error() == simdjson::SUCCESS)
    {
        material.Name = name.get_string().value();
    }

    auto kind = doc["Kind"];
    if (kind.error() == simdjson::SUCCESS)
    {
        material.Kind = uint32_t(kind.get_uint64().value());
    }

    auto params = doc["Params"];
    if (params.error() == simdjson::SUCCESS)
    {
        for(auto param : params.get_array())
        {
            std::string paramName;
            float       paramValue = 0.0f;

            auto name = param["Name"];
            if (name.error() == simdjson::SUCCESS)
            {
                paramName = name.get_string().value();
            }

            auto value = param["Value"];
            if (name.error() == simdjson::SUCCESS)
            {
                paramValue = float(name.get_double().value());
            }

            material.Params[paramName] = paramValue;
        }
    }

    auto textures = doc["Textures"];
    if (textures.error() == simdjson::SUCCESS)
    {
        for(auto tex : textures.get_array())
        {
            std::string texName;
            std::string texPath;

            auto name = tex["Name"];
            if (name.error() == simdjson::SUCCESS)
            {
                texName = name.get_string().value();
            }

            auto path = tex["Path"];
            if (path.error() == simdjson::SUCCESS)
            {
                texPath = path.get_string().value();
            }

            material.Textures[texName] = texPath;
        }
    }

    return true;
}

} // namespace edit

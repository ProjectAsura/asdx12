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
//      ブレンドステートを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(BlendState state)
{
    switch(state)
    {
    case BlendState::Opaque:
    default:
        return "Opaque";

    case BlendState::AlphaBlend:
        return "AlphaBlend";

    case BlendState::Additive:
        return "Additive";

    case BlendState::Subtract:
        return "Subtract";

    case BlendState::Premultiplied:
        return "Premultiplied";

    case BlendState::Multiply:
        return "Multiply";

    case BlendState::Screen:
        return "Screen";
    }
}

//-----------------------------------------------------------------------------
//      文字列からブレンドステートに変換します.
//-----------------------------------------------------------------------------
BlendState ToBlendState(const char* state)
{
    if (_stricmp(state, "Opaque") == 0)
    {
        return BlendState::Opaque;
    }
    else if (_stricmp(state, "AlphaBlend") == 0)
    {
        return BlendState::AlphaBlend;
    }
    else if (_stricmp(state, "Additive") == 0)
    {
        return BlendState::Additive;
    }
    else if (_stricmp(state, "Subtract") == 0)
    {
        return BlendState::Subtract;
    }
    else if (_stricmp(state, "Premultiplied") == 0)
    {
        return BlendState::Premultiplied;
    }
    else if (_stricmp(state, "Multiply") == 0)
    {
        return BlendState::Multiply;
    }
    else if (_stricmp(state, "Screen") == 0)
    {
        return BlendState::Screen;
    }

    return BlendState::Opaque;
}

//-----------------------------------------------------------------------------
//      深度ステートから文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(DepthState state)
{
    switch(state)
    {
    case DepthState::ReadWrite:
        return "ReadWrite";

    case DepthState::ReadOnly:
        return "ReadOnly";

    case DepthState::WriteOnly:
        return "WriteOnly";

    case DepthState::None:
        return "None";
    }

    return "ReadWrite";
}

//-----------------------------------------------------------------------------
//      文字列から深度ステートに変換します.
//-----------------------------------------------------------------------------
DepthState ToDepthState(const char* state)
{
    if (_stricmp(state, "ReadWrite") == 0)
    {
        return DepthState::ReadWrite;
    }
    else if (_stricmp(state, "ReadOnly") == 0)
    {
        return DepthState::ReadOnly;
    }
    else if (_stricmp(state, "WriteOnly") == 0)
    {
        return DepthState::WriteOnly;
    }
    else if (_stricmp(state, "None") == 0)
    {
        return DepthState::None;
    }

    return DepthState::ReadWrite;
}

//-----------------------------------------------------------------------------
//      ラスタライザーステートを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(RasterizerState state)
{
    switch(state)
    {
    case RasterizerState::CullNone:
        return "CullNone";

    case RasterizerState::CullBack:
        return "CullBack";

    case RasterizerState::CullFront:
        return "CullFront";

    case RasterizerState::Wireframe:
        return "Wireframe";
    }

    return "CullNone";
}

//-----------------------------------------------------------------------------
//      文字列からラスタライザーステートに変換します.
//-----------------------------------------------------------------------------
RasterizerState ToRasterizerState(const char* state)
{
    if (_stricmp(state, "CullNone") == 0)
    {
        return RasterizerState::CullNone;
    }
    else if (_stricmp(state, "CullBack") == 0)
    {
        return RasterizerState::CullBack;
    }
    else if (_stricmp(state, "CullFront") == 0)
    {
        return RasterizerState::CullFront;
    }
    else if (_stricmp(state, "Wireframe") == 0)
    {
        return RasterizerState::Wireframe;
    }

    return RasterizerState::CullNone;
}


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
    fprintf_s(fp, "    \"BlendState\": \"%s\",\n", ToString(material.BlendState));
    fprintf_s(fp, "    \"DepthState\": \"%s\",\n", ToString(material.DepthState));
    fprintf_s(fp, "    \"RasterizerState\": \"%s\",\n", ToString(material.RasterizerState));
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

    auto blend = doc["BlendState"];
    if (blend.error() == simdjson::SUCCESS)
    {
        material.BlendState = ToBlendState(blend.get_string().value().data());
    }

    auto depth = doc["Depth"];
    if (depth.error() == simdjson::SUCCESS)
    {
        material.DepthState = ToDepthState(depth.get_string().value().data());
    }

    auto rasterizer = doc["Rasterizer"];
    if (rasterizer.error() == simdjson::SUCCESS)
    {
        material.RasterizerState = ToRasterizerState(rasterizer.get_string().value().data());
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
                paramValue = float(name.get_double());
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

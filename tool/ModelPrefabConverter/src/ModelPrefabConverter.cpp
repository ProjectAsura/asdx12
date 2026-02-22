//-----------------------------------------------------------------------------
// File : ModelPrefabConverter.cpp
// Desc : Model Prefab Converter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdio>
#include <ModelPrefabConverter.h>
#include <simdjson.h>
#include "ModelPrefabBinary_generated.h"


#ifndef ELOG
#define ELOG(x, ...) fprintf_s(stderr, "[File: %s, Line: %d] " x "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif//ELOG


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t CURRENT_VERSION = 1u;  //!< 現在サポートされているバージョン.

} // namespace


///////////////////////////////////////////////////////////////////////////////
// ModelPrefabConverter class
///////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
//      変換処理を行います
//------------------------------------------------------------------------------
bool ModelPrefabConverter::Convert(const Desc& desc)
{
    ModelPrefab prefab;
    if (!LoadFromJson(desc.InputPath.c_str(), prefab))
    {
        ELOG("Error : ModelPrefabConverter::LoadFromJson() Failed.");
        return false;
    }

    std::vector<uint8_t> binary;
    if (!Convert(prefab, binary))
    {
        ELOG("Error : ModelPrefabConverter::Convert() Failed.");
        return false;
    }

    FILE* fp = nullptr;
    auto err = fopen_s(&fp, desc.OutputPath.c_str(), "wb");
    if (err != 0)
    {
        ELOG("Error : File open failed. path = %s", desc.OutputPath.c_str());
        return false;
    }

    fwrite(binary.data(), binary.size(), 1, fp);
    fclose(fp);

    return true;
}

//------------------------------------------------------------------------------
//      変換処理を行います.
//------------------------------------------------------------------------------
bool ModelPrefabConverter::Convert(const ModelPrefab& input, std::vector<uint8_t>& binary)
{
    if (input.ModelPath.empty() || input.Materials.empty())
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    flatbuffers::FlatBufferBuilder builder(1024);

    std::vector<flatbuffers::Offset<asdx::res::MeshMaterial>> materials;

    auto bin = asdx::res::CreateModelPrefabBinaryDirect(
        builder,
        CURRENT_VERSION,
        input.ModelPath.c_str(),
        &materials);

    builder.Finish(bin);

    binary.resize(builder.GetSize());
    memcpy(binary.data(), builder.GetBufferPointer(), builder.GetSize());

    return true;
}

//------------------------------------------------------------------------------
//      逆変換処理を行います.
//------------------------------------------------------------------------------
bool ModelPrefabConverter::ReverseConvert(const std::vector<uint8_t>& binary, ModelPrefab& result)
{
    if (binary.empty())
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    auto prefabBin = asdx::res::GetModelPrefabBinary(binary.data());

    result.ModelPath = prefabBin->ModelPath()->c_str();
    auto count = prefabBin->Materials()->size();
    for(auto i=0u; i<count; ++i)
    {
        auto srcMat = prefabBin->Materials()->Get(i);
        MeshMaterial dstMat;
        dstMat.Name = srcMat->Name()->c_str();
        dstMat.Path = srcMat->Path()->c_str();
        result.Materials.emplace_back(dstMat);
    }

    return false;
}

//------------------------------------------------------------------------------
//      jsonファイルからロードします.
//------------------------------------------------------------------------------
bool ModelPrefabConverter::LoadFromJson(const char* path, ModelPrefab& prefab)
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

    auto modelPath = doc["ModelPath"];
    if (modelPath.error() == simdjson::SUCCESS)
    {
        prefab.ModelPath = modelPath.get_string().value();
    }

    auto materials = doc["Materials"];
    if (materials.error() == simdjson::SUCCESS)
    {
        for(auto material : materials.get_array())
        {
            MeshMaterial dstMat;
            auto name = material["Name"];
            if (name.error() == simdjson::SUCCESS)
            {
                dstMat.Name = name.get_string().value();
            }

            auto path = materials["Path"];
            if (path.error() == simdjson::SUCCESS)
            {
                dstMat.Path = name.get_string().value();
            }

            prefab.Materials.emplace_back(dstMat);
        }
    }

    return true;
}

//------------------------------------------------------------------------------
//      jsonファイルにセーブします.
//------------------------------------------------------------------------------
bool ModelPrefabConverter::SaveToJson(const char* path, const ModelPrefab& prefab)
{
    if (path == nullptr || prefab.ModelPath.empty() || prefab.Materials.empty())
    {
        ELOG("Error : Invalid Argument.");
    }

    FILE* fp = nullptr;
    auto err = fopen_s(&fp, path, "w");
    if (err != 0)
    {
        ELOG("Error : File Open Failed. path = %s", path);
        return false;
    }

    fprintf_s(fp, "{\n");
    fprintf_s(fp, "    \"ModelPath\": \"%s\",\n", prefab.ModelPath.c_str());
    fprintf_s(fp, "    \"Materials\": [\n");
    auto count = prefab.Materials.size();
    for(auto i=0u; i<count; ++i)
    {
        auto& mat = prefab.Materials[i];
        fprintf_s(fp, "          { \"Name\": \"%s\", \"Path\": \"%s\" }", mat.Name.c_str(), mat.Path.c_str());
        if (i != count - 1)
        {
            fprintf_s(fp, ",\n");
        }
        else
        {
            fprintf_s(fp, "\n");
        }
    }

    fprintf_s(fp, "    ]\n");
    fprintf_s(fp, "}\n");

    return false;
}


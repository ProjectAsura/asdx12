//-----------------------------------------------------------------------------
// File : MaterialConverter.cpp
// Desc : Material Binary (*.mtb) Converter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdio>
#include <simdjson.h>
#include <filesystem>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <MaterialConverter.h>
#include <MaterialBinary_generated.h>


#ifndef ELOG
#define ELOG(x, ...) fprintf_s(stderr, "[File:%s, Line:%d] " x "\n", __FILE__, __LINE__, ##__VA_ARGS__ )
#endif//ELOG

namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t CURRENT_VERSION = 1u;  //!< 現在サポートされているバージョン.


//-----------------------------------------------------------------------------
//      JSONファイルから変換します.
//-----------------------------------------------------------------------------
bool ConvertFromJSON
(
    const char*                                             inputPath,
    flatbuffers::FlatBufferBuilder&                         builder,
    std::vector<flatbuffers::Offset<asdx::res::Material>>&  materials
)
{
    simdjson::ondemand::parser parser;
    auto json = simdjson::padded_string::load(inputPath);
    auto doc  = parser.iterate(json);

    // {
    //      "materials" : [
    //          {
    //              "bind_name" : string,
    //              "base_color_map" : string,
    //              "normal_map" : string,
    //              "orm_map" : string,
    //              "emissive_map" : string,
    //              "base_color_factor" : [float, float, float],
    //              "occlusion_factor" : float,
    //              "roughness_factor" : float,
    //              "metalness_factor" : float,
    //              "ior" : float,
    //              "alpha_threshold" : float,
    //          },
    //      ]
    // }

    auto mats = doc["materials"].get_array();
    assert(mats.error() == simdjson::SUCCESS);

    for(auto m : mats)
    {
        const char* bindName     = nullptr;
        const char* baseColorMap = nullptr;
        const char* normalMap    = nullptr;
        const char* ormMap       = nullptr;
        const char* emissiveMap  = nullptr;

        asdx::res::Float3 baseColorFactor(1.0f, 1.0f, 1.0f);
        asdx::res::Float3 emissiveFactor (1.0f, 1.0f, 1.0f);
        float occlusionFactor = 1.0f;
        float roughnessFactor = 1.0f;
        float metalnessFactor = 1.0f;
        float ior             = 1.0f;
        float alphaThreshold  = 0.0f;

        // TODO : 変換処理を実装.
        bindName     = m["bind_name"]     .get_string().value().data();
        baseColorMap = m["base_color_map"].get_string().value().data();
        normalMap    = m["normal_map"]    .get_string().value().data();
        ormMap       = m["orm_map"]       .get_string().value().data();
        emissiveMap  = m["emissive_map"]  .get_string().value().data();

        {
            auto param = m["base_color_factor"].get_array();
            baseColorFactor = asdx::res::Float3(
                float(param.at(0).get_double().value()),
                float(param.at(1).get_double().value()),
                float(param.at(2).get_double().value()));
        }

        occlusionFactor = float(m["occlusion_factor"].get_double().value());
        roughnessFactor = float(m["roughness_factor"].get_double().value());
        metalnessFactor = float(m["metalness_factor"].get_double().value());

        {
            auto param = m["emissive_factor"].get_array();
            emissiveFactor = asdx::res::Float3(
                float(param.at(0).get_double().value()),
                float(param.at(1).get_double().value()),
                float(param.at(2).get_double().value()));
        }

        ior             = float(m["ior"]            .get_double().value());
        alphaThreshold  = float(m["alpha_threshold"].get_double().value());

        materials.emplace_back(
            asdx::res::CreateMaterialDirect(
                builder,
                bindName,
                baseColorMap,
                normalMap,
                ormMap,
                emissiveMap,
                &baseColorFactor,
                occlusionFactor,
                roughnessFactor,
                metalnessFactor,
                &emissiveFactor,
                ior,
                alphaThreshold));
    }

    return true;
}

//-----------------------------------------------------------------------------
//      テクスチャファイルパスを取得します.
//-----------------------------------------------------------------------------
bool GetTexturePath(aiMaterial* mat, aiTextureType type, aiString* path)
{
    if (mat->GetTextureCount(type) == 0)
        return false;

    return mat->GetTexture(type, 0, path) == aiReturn_SUCCESS;
}

//-----------------------------------------------------------------------------
//      Assimpを使って変換します.
//-----------------------------------------------------------------------------
bool ConvertByAssimp
(
    const char*                                             inputPath,
    flatbuffers::FlatBufferBuilder&                         builder,
    std::vector<flatbuffers::Offset<asdx::res::Material>>&  materials
)
{
    int flag = aiProcess_RemoveRedundantMaterials;

    Assimp::Importer importer;
    auto pScene = importer.ReadFile(inputPath, flag);

    if (pScene == nullptr)
    {
        ELOG("Error : Importer::ReadFile() Failed. path = %s", inputPath);
        return false;
    }

    if (!pScene->HasMaterials())
    {
        ELOG("Error : Material Not Found. path = %s", inputPath);
        return false;
    }

    for(auto i=0u; i<pScene->mNumMaterials; ++i)
    {
        const auto src = pScene->mMaterials[i];
        const char* bindName     = src->GetName().C_Str();

        asdx::res::Float3 baseColorFactor(1.0f, 1.0f, 1.0f);
        asdx::res::Float3 emissiveFactor (1.0f, 1.0f, 1.0f);
        float occlusionFactor = 1.0f;
        float roughnessFactor = 1.0f;
        float metalnessFactor = 1.0f;
        float ior             = 1.0f;
        float alphaThreshold  = 0.0f;

        aiString baseColorPath;
        GetTexturePath(src, aiTextureType_BASE_COLOR, &baseColorPath);

        aiString normalPath;
        GetTexturePath(src, aiTextureType_NORMALS, &normalPath);

        aiString ormPath;
        GetTexturePath(src, aiTextureType_GLTF_METALLIC_ROUGHNESS, &ormPath);

        aiString emissivePath;
        GetTexturePath(src, aiTextureType_EMISSIVE, &emissivePath);

        {
            aiColor4D param;
            if (src->Get(AI_MATKEY_BASE_COLOR, param) == aiReturn_SUCCESS)
            { 
                baseColorFactor = asdx::res::Float3(param.r, param.g, param.b);
                alphaThreshold  = param.a;
            }
        }

        {
            aiColor4D param;
            if (src->Get(AI_MATKEY_EMISSIVE_INTENSITY, param) == aiReturn_SUCCESS)
            { emissiveFactor = asdx::res::Float3(param.r, param.g, param.b); }
        }

        {
            float param;
            if (src->Get(AI_MATKEY_METALLIC_FACTOR, param) == aiReturn_SUCCESS)
            { metalnessFactor = param; }
        }

        {
            float param;
            if (src->Get(AI_MATKEY_ROUGHNESS_FACTOR, param) == aiReturn_SUCCESS)
            { roughnessFactor = param; }
        }

        {
            float param;
            if (src->Get(AI_MATKEY_REFRACTI, param) == aiReturn_SUCCESS)
            { ior = param; }
        }

        materials.emplace_back(
            asdx::res::CreateMaterialDirect(
                builder,
                bindName,
                baseColorPath.C_Str(),
                normalPath.C_Str(),
                ormPath.C_Str(),
                emissivePath.C_Str(),
                &baseColorFactor,
                occlusionFactor,
                roughnessFactor,
                metalnessFactor,
                &emissiveFactor,
                ior,
                alphaThreshold));
    }

    return true;
}

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// MaterialConverter class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool MaterialConverter::Convert(const Desc& desc)
{
    if (desc.InputPath.empty() || desc.OutputPath.empty())
    {
        ELOG("Error : Invalid Arguments.");
        return false;
    }

    // 変換処理.
    std::vector<uint8_t> binary;
    if (!Convert(desc.InputPath.c_str(), binary))
    {
        ELOG("Error : Convert() Failed.");
        return false;
    }

    // ファイルに出力.
    {
        FILE* fp = nullptr;
        auto err = fopen_s(&fp, desc.OutputPath.c_str(), "wb");
        if (err != 0)
        {
            ELOG("Error : File Open Failed. path = %s", desc.OutputPath.c_str());
            return false;
        }

        fwrite(binary.data(), binary.size(), 1, fp);
        fclose(fp);
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool MaterialConverter::Convert(const char* inputPath, std::vector<uint8_t>& binary)
{
    if (inputPath == nullptr)
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    std::vector<flatbuffers::Offset<asdx::res::Material>> materials;
    flatbuffers::FlatBufferBuilder builder(1024);

    // 拡張子取得.
    std::filesystem::path path(inputPath);
    auto ext = path.extension().string();

    // 拡張子判定.
    if (ext == ".json")
    {
        if (!ConvertFromJSON(inputPath, builder, materials))
        {
            ELOG("Error : ConvertFromJSON() Failed.");
            return false;
        }
    }
    else
    {
        if (!ConvertByAssimp(inputPath, builder, materials))
        {
            ELOG("Error : ConvertByAssimp() Failed.");
            return false;
        }
    }

    // バイナリ生成.
    auto bin = asdx::res::CreateMaterialBinaryDirect(
        builder,
        CURRENT_VERSION,
        &materials);

    // バイナリ作成完了.
    builder.Finish(bin);

    // 出力先にコピー.
    binary.resize(builder.GetSize());
    memcpy(binary.data(), builder.GetBufferPointer(), builder.GetSize());

    // 正常終了.
    return true;
}

} // namespace asdx

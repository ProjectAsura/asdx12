//----------------------------------------------------------------------------
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

enum MATERIAL_FEATURE
{
    MATERIAL_NONE           = 0x0,
    MATERIAL_ANISOTROPY     = 0x1 << 0,
    MATERIAL_CLEARCOAT      = 0x1 << 1,
    MATERIAL_SHEEN          = 0x1 << 2,
    MATERIAL_TRANSMISSION   = 0x1 << 3,
    MATERIAL_IOR            = 0x1 << 4,
    MATERIAL_DISPERSION     = 0x1 << 5,
    MATERIAL_VOLUME         = 0x1 << 6,
    MATERIAL_IRIDESCENCE    = 0x1 << 7,
};

//-----------------------------------------------------------------------------
//      テクスチャファイルパスを取得します.
//-----------------------------------------------------------------------------
bool GetTexturePath(aiMaterial* mat, aiTextureType type, aiString* path, uint32_t index = 0)
{
    if (mat->GetTextureCount(type) == 0)
        return false;

    return mat->GetTexture(type, index, path) == aiReturn_SUCCESS;
}

//-----------------------------------------------------------------------------
//      マテリアルを変換します.
//-----------------------------------------------------------------------------
void ConvertMaterial
(
    aiMaterial*                                             src,
    flatbuffers::FlatBufferBuilder&                         builder,
    std::vector<flatbuffers::Offset<asdx::res::Material>>&  materials
)
{
    std::vector<flatbuffers::Offset<asdx::res::MaterialTexture>> textures;

    const char* bindName = src->GetName().C_Str();

    uint32_t featureMask = MATERIAL_NONE;

    // デフォルト値設定.
    asdx::res::Float3   baseColorFactor(0.0f, 0.0f, 0.0f);
    float               occlusionFactor          = 1.0f;
    float               roughnessFactor          = 1.0f;
    float               metalnessFactor          = 1.0f;
    asdx::res::Float3   emissiveFactor(0.0f, 0.0f, 0.0f);
    float               alphaThreshold           = -1.0f;
    float               anisotropyStrength       = 0.0f;
    asdx::res::Float2   anisotropyRotation(0.0f, 0.0f);
    float               clearCoatFactor          = 1.0f;
    float               clearCoatRoughnessFactor = 1.0f;
    asdx::res::Float3   sheenColorFactor(0.0f, 0.0f, 0.0f);
    float               sheenRoughnessFactor     = 0.0f;
    float               ior                      = 1.5f;
    float               dispersion               = 0.0f;
    float               transmissionFactor       = 0.0f;
    float               iridescenceFactor        = 0.0f;
    float               iridescenceIor           = 1.3f;
    float               iridescenceThicknessMin  = 100.0f;
    float               iridescenceThicknessMax  = 400.0f;

    {
        aiString path;
        if (GetTexturePath(src, aiTextureType_BASE_COLOR, &path))
        {
            textures.emplace_back(
                asdx::res::CreateMaterialTextureDirect(
                    builder,
                    path.C_Str(),
                    asdx::res::TextureKind_BaseColor));
        }
    }

    {
        aiString path;
        if (GetTexturePath(src, aiTextureType_NORMALS, &path))
        {
            textures.emplace_back(
                asdx::res::CreateMaterialTextureDirect(
                    builder,
                    path.C_Str(),
                    asdx::res::TextureKind_Normal));
        }
    }

    {
        aiString path;
        if (GetTexturePath(src, aiTextureType_GLTF_METALLIC_ROUGHNESS, &path))
        {
            textures.emplace_back(
                asdx::res::CreateMaterialTextureDirect(
                    builder,
                    path.C_Str(),
                    asdx::res::TextureKind_Orm));
        }
    }

    {
        aiString path;
        if (GetTexturePath(src, aiTextureType_EMISSIVE, &path))
        {
            textures.emplace_back(
                asdx::res::CreateMaterialTextureDirect(
                    builder,
                    path.C_Str(),
                    asdx::res::TextureKind_Emissive));
        }
    }

    {
        aiString path;
        if (GetTexturePath(src, aiTextureType_ANISOTROPY, &path))
        {
            textures.emplace_back(
                asdx::res::CreateMaterialTextureDirect(
                    builder,
                    path.C_Str(),
                    asdx::res::TextureKind_Anisotropy));

            featureMask |= MATERIAL_ANISOTROPY;
        }
    }

    {
        aiString path;
        if (GetTexturePath(src, aiTextureType_CLEARCOAT, &path))
        {
            textures.emplace_back(
                asdx::res::CreateMaterialTextureDirect(
                    builder,
                    path.C_Str(),
                    asdx::res::TextureKind_ClearCoat));

            featureMask |= MATERIAL_CLEARCOAT;
        }
    }

    {
        aiString path;
        if (GetTexturePath(src, aiTextureType_CLEARCOAT, &path, 1))
        {
            textures.emplace_back(
                asdx::res::CreateMaterialTextureDirect(
                    builder,
                    path.C_Str(),
                    asdx::res::TextureKind_ClearCoatRoughness));

            featureMask |= MATERIAL_CLEARCOAT;
        }
    }

    {
        aiString path;
        if (GetTexturePath(src, aiTextureType_CLEARCOAT, &path, 2))
        {
            textures.emplace_back(
                asdx::res::CreateMaterialTextureDirect(
                    builder,
                    path.C_Str(),
                    asdx::res::TextureKind_ClearCoatNormal));

            featureMask |= MATERIAL_CLEARCOAT;
        }
    }

    {
        aiString path;
        if (GetTexturePath(src, aiTextureType_TRANSMISSION, &path))
        {
            textures.emplace_back(
                asdx::res::CreateMaterialTextureDirect(
                    builder,
                    path.C_Str(),
                    asdx::res::TextureKind_Transmission));

            featureMask |= MATERIAL_TRANSMISSION;
        }
    }

    {
        aiString path;
        if (GetTexturePath(src, aiTextureType_SHEEN, &path))
        {
            textures.emplace_back(
                asdx::res::CreateMaterialTextureDirect(
                    builder,
                    path.C_Str(),
                    asdx::res::TextureKind_SheenColor));

            featureMask |= MATERIAL_SHEEN;
        }
    }

    {
        aiString path;
        if (GetTexturePath(src, aiTextureType_SHEEN, &path, 1))
        {
            textures.emplace_back(
                asdx::res::CreateMaterialTextureDirect(
                    builder,
                    path.C_Str(),
                    asdx::res::TextureKind_SheenRoughness));

            featureMask |= MATERIAL_SHEEN;
        }
    }

    {
        aiString path;
        if (GetTexturePath(src, aiTextureType_TRANSMISSION, &path))
        {
            textures.emplace_back(
                asdx::res::CreateMaterialTextureDirect(
                    builder,
                    path.C_Str(),
                    asdx::res::TextureKind_Transmission));

            featureMask |= MATERIAL_TRANSMISSION;
        }
    }

    {
        aiString path;
        if (GetTexturePath(src, aiTextureType_TRANSMISSION, &path, 1))
        {
            textures.emplace_back(
                asdx::res::CreateMaterialTextureDirect(
                    builder,
                    path.C_Str(),
                    asdx::res::TextureKind_Thickness));

            featureMask |= MATERIAL_VOLUME;
        }
    }

    {
        aiColor4D param;
        if (src->Get(AI_MATKEY_BASE_COLOR, param) == aiReturn_SUCCESS)
        {
            baseColorFactor = asdx::res::Float3(param.r, param.g, param.b);
        }
    }

    {
        float param;
        if (src->Get(AI_MATKEY_ROUGHNESS_FACTOR, param) == aiReturn_SUCCESS)
        {
            roughnessFactor = param;
        }
    }

    {
        float param;
        if (src->Get(AI_MATKEY_METALLIC_FACTOR, param) == aiReturn_SUCCESS)
        {
            metalnessFactor = param;
        }
    }

    {
        float param;
        if (src->Get(AI_MATKEY_REFRACTI, param) == aiReturn_SUCCESS)
        {
            ior = param;
            featureMask |= MATERIAL_IOR;
        }
    }

    {
        float param;
        if (src->Get("$mat.dispersion", 0, 0, param) == aiReturn_SUCCESS)
        {
            dispersion = param;
            featureMask |= MATERIAL_DISPERSION;
        }
    }

    {
        aiColor3D param;
        if (src->Get(AI_MATKEY_COLOR_EMISSIVE, param) == aiReturn_SUCCESS)
        {
            emissiveFactor = asdx::res::Float3(param.r, param.g, param.b);
        }
    }

    {
        aiVector2D param;
        if (src->Get(AI_MATKEY_ANISOTROPY_ROTATION, param) == aiReturn_SUCCESS)
        {
            anisotropyRotation = asdx::res::Float2(param.x, param.y);
            featureMask |= MATERIAL_ANISOTROPY;
        }
    }

    {
        float param;
        if (src->Get(AI_MATKEY_ANISOTROPY_FACTOR, param) == aiReturn_SUCCESS)
        {
            anisotropyStrength = param;
            featureMask |= MATERIAL_ANISOTROPY;
        }
    }

    {
        float param;
        if (src->Get(AI_MATKEY_CLEARCOAT_FACTOR, param) == aiReturn_SUCCESS)
        {
            clearCoatFactor = param;
            featureMask |= MATERIAL_CLEARCOAT;
        }
    }

    {
        float param;
        if (src->Get(AI_MATKEY_CLEARCOAT_ROUGHNESS_FACTOR, param) == aiReturn_SUCCESS)
        {
            clearCoatRoughnessFactor = param;
            featureMask |= MATERIAL_CLEARCOAT;
        }
    }

    {
        aiColor3D param;
        if (src->Get(AI_MATKEY_SHEEN_COLOR_FACTOR, param) == aiReturn_SUCCESS)
        {
            sheenColorFactor = asdx::res::Float3(param.r, param.g, param.b);
            featureMask |= MATERIAL_SHEEN;
        }
    }

    {
        float param;
        if (src->Get(AI_MATKEY_SHEEN_ROUGHNESS_FACTOR, param) == aiReturn_SUCCESS)
        {
            sheenRoughnessFactor = param;
            featureMask |= MATERIAL_SHEEN;
        }
    }

    {
        float param;
        if (src->Get(AI_MATKEY_TRANSMISSION_FACTOR, param) == aiReturn_SUCCESS)
        {
            transmissionFactor = param;
            featureMask |= MATERIAL_TRANSMISSION;
        }
    }

    {
        float param;
        if (src->Get("$mat.iridescence.factor", 0, 0, param) == aiReturn_SUCCESS)
        {
            iridescenceFactor = param;
            featureMask |= MATERIAL_IRIDESCENCE;
        }
    }

    {
        float param;
        if (src->Get("$mat.iridescence.ior", 0, 0, param) == aiReturn_SUCCESS)
        {
            iridescenceIor = param;
            featureMask |= MATERIAL_IRIDESCENCE;
        }
    }

    {
        float param;
        if (src->Get("$mat.iridescence.thicknessMinimum", 0, 0, param) == aiReturn_SUCCESS)
        {
            iridescenceThicknessMin = param;
            featureMask |= MATERIAL_IRIDESCENCE;
        }
    }

    {
        float param;
        if (src->Get("$mat.iridescence.thicknessMaximum", 0, 0, param) == aiReturn_SUCCESS)
        {
            iridescenceThicknessMax = param;
            featureMask |= MATERIAL_IRIDESCENCE;
        }
    }

    materials.emplace_back(
        asdx::res::CreateMaterialDirect(
            builder,
            bindName,
            featureMask,
            textures.empty() ? nullptr : &textures,
            &baseColorFactor,
            occlusionFactor,
            roughnessFactor,
            metalnessFactor,
            &emissiveFactor,
            alphaThreshold,
            anisotropyStrength,
            &anisotropyRotation,
            clearCoatFactor,
            clearCoatRoughnessFactor,
            &sheenColorFactor,
            sheenRoughnessFactor,
            ior,
            dispersion,
            transmissionFactor,
            iridescenceFactor,
            iridescenceIor,
            iridescenceThicknessMin,
            iridescenceThicknessMax));
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
        ConvertMaterial(src, builder, materials);
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

    if (!ConvertByAssimp(inputPath, builder, materials))
    {
        ELOG("Error : ConvertByAssimp() Failed.");
        return false;
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

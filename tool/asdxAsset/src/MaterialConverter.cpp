//-----------------------------------------------------------------------------
// File : MaterialConverter.cpp
// Desc : Material Binary (*.mtb) Converter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <MaterialConverter.h>
#include <MaterialBinary_generated.h>
#include "../../../include/fnd/asdxFlatDoc.h"


#ifndef ELOG
#define ELOG(x, ...) fprintf_s(stderr, "[File:%s, Line:%d] " x "\n", __FILE__, __LINE__, ##__VA_ARGS__ )
#endif//ELOG

namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t CURRENT_VERSION = 1u;


///////////////////////////////////////////////////////////////////////////////
// ShaderParam structure
///////////////////////////////////////////////////////////////////////////////
struct ShaderParam
{
    asdx::Float3    BaseColor;  //!< ベースカラー.
    float           Alpha;      //!< アルファ値.
    float           Occlusion;  //!< オクルージョン.
    float           Roughness;  //!< ラフネス値.
    float           Metalness;  //!< メタルネス値.
    float           Ior;        //!< 屈折率.
    asdx::Float3    Emissive;   //!< エミッシブ.
    uint32_t        Reserved;   //!< 予約領域.
};

} // namespace


///////////////////////////////////////////////////////////////////////////////
// MaterialConverter class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      現在のバイナリバージョンを取得します.
//-----------------------------------------------------------------------------
uint32_t MaterialConverter::GetCurrentVersion()
{ return CURRENT_VERSION; }

//-----------------------------------------------------------------------------
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool MaterialConverter::Convert(const Desc& desc)
{
    // 引数チェック.
    if (desc.InputPath.empty() || desc.OutputPath.empty())
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    // FlatDocロード.
    asdx::FlatDoc doc;
    if (!doc.Load(desc.InputPath.c_str()))
    {
        ELOG("Error : FlatDoc Open Failed. path = %s", desc.InputPath.c_str());
        return false;
    }

    flatbuffers::FlatBufferBuilder builder(1024);

    ShaderParam param = {};
    param.BaseColor = doc.GetFloat3("BaseColor", asdx::Float3(1.0f, 1.0f, 1.0f));
    param.Alpha     = doc.GetFloat("Alpha", 1.0f);
    param.Occlusion = doc.GetFloat("Occlusion", 1.0f);
    param.Roughness = doc.GetFloat("Roughness", 0.5f);
    param.Metalness = doc.GetFloat("Metalness", 0.0f);
    param.Ior       = doc.GetFloat("Ior", 1.0f);
    param.Emissive  = doc.GetFloat3("Emissive", asdx::Float3(0.0f, 0.0f, 0.0f));
    param.Reserved  = 0;

    asdx::res::AlphaType alphaMode = asdx::res::AlphaType(doc.GetUint("AlphaMode", 0));
    asdx::res::CullType  cullMode  = asdx::res::CullType(doc.GetUint("CullMode", 0));

    auto shader = doc.GetString("Shader");

    auto baseColorMap = doc.GetString("BaseColorMap", "default.BaseColor");
    auto normalMap    = doc.GetString("NormalMap"   , "default.Normal");
    auto ormMap       = doc.GetString("OrmMap"      , "default.Orm");
    auto emissiveMap  = doc.GetString("EmissiveMap" , "default.TransparentBlack");

    {
        std::vector<flatbuffers::Offset<flatbuffers::String>> textures;
        textures.push_back(builder.CreateString(baseColorMap.c_str()));
        textures.push_back(builder.CreateString(normalMap.c_str()));
        textures.push_back(builder.CreateString(ormMap.c_str()));
        textures.push_back(builder.CreateString(emissiveMap.c_str()));

        std::vector<uint8_t> buffer;
        buffer.resize(sizeof(param));
        memcpy(buffer.data(), &param, sizeof(param));

        auto material = asdx::res::CreateMaterialBinaryDirect(
            builder,
            CURRENT_VERSION,
            shader.c_str(),
            &buffer,
            &textures,
            alphaMode,
            cullMode);

        builder.Finish(material);

        std::vector<uint8_t> binary;
        binary.resize(builder.GetSize());
        memcpy(binary.data(), builder.GetBufferPointer(), builder.GetSize());

        FILE* fp = nullptr;
        auto err = fopen_s(&fp, desc.OutputPath.c_str(), "wb");
        if (err != 0)
        {
            ELOG("Error : Output File Open Failed. path = %s, errcode = 0x%x", desc.OutputPath.c_str(), err);
            return false;
        }

        fwrite(binary.data(), binary.size(), 1, fp);
        fclose(fp);
    }

    return true;
}

//-----------------------------------------------------------------------------
//      逆変換処理を行います.
//-----------------------------------------------------------------------------
bool MaterialConverter::ReverseConvert(const std::string& path, const std::vector<uint8_t>& binary)
{
    // 引数チェック.
    if (path.empty() || binary.empty())
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    // マテリアルバイナリへ.
    auto bin = asdx::res::GetMaterialBinary(binary.data());

    // シェーダパラメータに変換.
    ShaderParam param = {};
    assert(sizeof(param) == bin->Buffer()->size());
    memcpy(&param, bin->Buffer()->data(), bin->Buffer()->size());

    asdx::FlatDoc doc;
    doc.SetString("Shader", bin->Shader()->c_str());

    doc.SetUint("AlphaMode", uint32_t(bin->AlphaMode()));
    doc.SetUint("CullMode" , uint32_t(bin->CullMode()));

    doc.SetFloat3("BaseColor", param.BaseColor);
    doc.SetFloat ("Alpha"    , param.Alpha);
    doc.SetFloat ("Occlusion", param.Occlusion);
    doc.SetFloat ("Roughness", param.Roughness);
    doc.SetFloat ("Metalness", param.Metalness);
    doc.SetFloat ("Ior"      , param.Ior);
    doc.SetFloat3("Emissive" , param.Emissive);

    assert(bin->Textures()->size() == 4);
    doc.SetString("BaseColorMap", bin->Textures()->Get(0)->c_str());
    doc.SetString("NormalMap"   , bin->Textures()->Get(1)->c_str());
    doc.SetString("OrmMap"      , bin->Textures()->Get(2)->c_str());
    doc.SetString("EmissiveMap" , bin->Textures()->Get(3)->c_str());

    // FlatDocに保存.
    if (!doc.Save(path.c_str()))
    {
        ELOG("Error : FlatDoc Save Failed. path = %s", path.c_str());
        return false;
    }

    // 正常終了.
    return true;
}

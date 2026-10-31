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
#include <simdjson.h>
#include "../../../include/fnd/asdxMath.h"
#include "../../../include/edit/asdxJsonWriter.h"


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

    simdjson::ondemand::parser parser;
    auto json = simdjson::padded_string::load(desc.InputPath.c_str());
    auto doc  = parser.iterate(json);

    auto shader = doc["shader"].get_string().value();

    auto alpha_mode = doc["alpha_mode"].get_uint64().value();
    auto cull_mode  = doc["cull_mode"].get_uint64().value();

    auto param = doc["param"].get_object();
    auto base_color = param["base_color"].get_array();
    auto alpha      = param["alpha"].get_double().value();
    auto occlusion  = param["occlusion"].get_double().value();
    auto roughness  = param["roughness"].get_double().value();
    auto metalness  = param["metalness"].get_double().value();
    auto ior        = param["ior"].get_double().value();
    auto emissive   = param["emissive"].get_array();

    auto textures = doc["textures"].get_object();
    auto map_base_color = textures["base_color"].get_string().value();
    auto map_normal     = textures["normal"].get_string().value();
    auto map_orm        = textures["orm"].get_string().value();
    auto map_emissive   = textures["emissive"].get_string().value();

    ShaderParam shaderParam = {};
    shaderParam.BaseColor  = asdx::Float3(
            float(base_color.at(0).get_double().value()),
            float(base_color.at(1).get_double().value()),
            float(base_color.at(2).get_double().value()));
    shaderParam.Alpha      = float(alpha);
    shaderParam.Occlusion  = float(occlusion);
    shaderParam.Roughness  = float(roughness);
    shaderParam.Metalness  = float(metalness);
    shaderParam.Ior        = float(ior);
    shaderParam.Emissive   = asdx::Float3(
            float(emissive.at(0).get_double().value()),
            float(emissive.at(1).get_double().value()),
            float(emissive.at(2).get_double().value()));

    flatbuffers::FlatBufferBuilder builder(1024);


    asdx::res::AlphaType alphaMode = asdx::res::AlphaType(alpha_mode);
    asdx::res::CullType  cullMode  = asdx::res::CullType(cull_mode);

    {
        std::vector<flatbuffers::Offset<flatbuffers::String>> textures;
        textures.resize(4);
        textures[0] = builder.CreateString(map_base_color.data());
        textures[1] = builder.CreateString(map_normal.data());
        textures[2] = builder.CreateString(map_orm.data());
        textures[3] = builder.CreateString(map_emissive.data());

        std::vector<uint8_t> buffer;
        buffer.resize(sizeof(shaderParam));
        memcpy(buffer.data(), &shaderParam, sizeof(shaderParam));

        auto material = asdx::res::CreateMaterialBinaryDirect(
            builder,
            CURRENT_VERSION,
            shader.data(),
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
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool MaterialConverter::Convert(const Material& material, std::vector<uint8_t>& binary)
{
    flatbuffers::FlatBufferBuilder builder(1024);

    asdx::res::AlphaType alphaMode = asdx::res::AlphaType(material.AlphaMode);
    asdx::res::CullType  cullMode  = asdx::res::CullType(material.CullMode);

    {
        std::vector<flatbuffers::Offset<flatbuffers::String>> textures;
        textures.resize(material.Textures.size());
        for(size_t i=0; i<material.Textures.size(); ++i)
        {
            textures[i] = builder.CreateString(material.Textures[i].c_str());
        }

        ShaderParam shaderParam = {};
        shaderParam.BaseColor = material.Param.BaseColor;
        shaderParam.Alpha     = material.Param.Alpha;
        shaderParam.Occlusion = material.Param.Occlusion;
        shaderParam.Roughness = material.Param.Roughness;
        shaderParam.Metalness = material.Param.Metalness;
        shaderParam.Ior       = material.Param.Ior;
        shaderParam.Emissive  = material.Param.Emissive;

        std::vector<uint8_t> buffer;
        buffer.resize(sizeof(ShaderParam));
        memcpy(buffer.data(), &shaderParam, sizeof(shaderParam));

        auto res = asdx::res::CreateMaterialBinaryDirect(
            builder,
            CURRENT_VERSION,
            material.Shader.c_str(),
            &buffer,
            &textures,
            alphaMode,
            cullMode);

        builder.Finish(res);

        binary.resize(builder.GetSize());
        memcpy(binary.data(), builder.GetBufferPointer(), builder.GetSize());
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

    asdx::JsonWriter writer;
    writer.Begin();

    writer.Write("shader", bin->Shader()->c_str());
    writer.Write("alpha_mode", uint8_t(bin->AlphaMode()));
    writer.Write("cull_mode" , uint8_t(bin->CullMode()));

    writer.WriteBeginSection("param");
    {
        writer.Write("base_color", asdx::Vector3(param.BaseColor.x, param.BaseColor.y, param.BaseColor.z));
        writer.Write("alpha", param.Alpha);
        writer.Write("occlusion", param.Occlusion);
        writer.Write("roughness", param.Roughness);
        writer.Write("metalness", param.Metalness);
        writer.Write("ior", param.Ior);
        writer.Write("emissive", asdx::Vector3(param.Emissive.x, param.Emissive.y, param.Emissive.z));
    }
    writer.WriteEndSection();

    writer.WriteBeginSection("textures");
    {
        writer.Write("base_color", bin->Textures()->Get(0)->c_str());
        writer.Write("normal"    , bin->Textures()->Get(1)->c_str());
        writer.Write("orm"       , bin->Textures()->Get(2)->c_str());
        writer.Write("emissive"  , bin->Textures()->Get(3)->c_str());
    }
    writer.WriteEndSection();

    writer.End();

    if (!writer.Save(path.c_str()))
    {
        ELOG("Error : JsonWriter::Save() Failed. path = %s", path.c_str());
        return false;
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      逆変換処理を行います.
//-----------------------------------------------------------------------------
bool MaterialConverter::ReverseConvert(const std::vector<uint8_t>& binary, Material& result)
{
    // 引数チェック.
    if (binary.empty())
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

    result.Shader           = bin->Shader()->c_str();
    result.AlphaMode        = uint8_t(bin->AlphaMode());
    result.CullMode         = uint8_t(bin->CullMode());
    result.Param.BaseColor  = param.BaseColor;
    result.Param.Alpha      = param.Alpha;
    result.Param.Occlusion  = param.Occlusion;
    result.Param.Roughness  = param.Roughness;
    result.Param.Metalness  = param.Metalness;
    result.Param.Ior        = param.Ior;
    result.Param.Emissive   = param.Emissive;

    result.Textures.resize(bin->Textures()->size());
    for(auto i=0u; i<bin->Textures()->size(); ++i)
    {
        result.Textures[i] = bin->Textures()->Get(i)->c_str();
    }

    return true;
}
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

} // namespace

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
        ELOG("Error : Invalid Argument.");
        return false;
    }

    std::vector<uint8_t> binary;
    if (!Convert(desc.InputPath.c_str(), binary))
    {
        ELOG("Error : MaterialConverter::Convert() Failed. inputPath = %s", desc.InputPath.c_str());
        return false;
    }

    // バイナリファイルに書き込み.
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

    return true;
}

//-----------------------------------------------------------------------------
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool MaterialConverter::Convert(const char* inputPath, std::vector<uint8_t>& binary)
{
    edit::Material material;
    if (!edit::LoadFromJson(inputPath, material))
    {
        ELOG("Error : edit::LoadFromJson() Failed. path = %s", inputPath);
        return false;
    }

    return Convert(material, binary);
}

//-----------------------------------------------------------------------------
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool MaterialConverter::Convert(const edit::Material& material, std::vector<uint8_t>& binary)
{
    flatbuffers::FlatBufferBuilder builder(1024);

    std::vector<flatbuffers::Offset<asdx::res::MaterialTexture>> textures;
    std::vector<flatbuffers::Offset<asdx::res::MaterialParameter>> params;

    for(const auto& tex : material.Textures)
    {
        auto dstTex = asdx::res::CreateMaterialTextureDirect(
            builder,
            tex.first.c_str(),
            tex.second.c_str());

        textures.emplace_back(dstTex);
    }

    for(const auto& param : material.Params)
    {
        auto dstParam = asdx::res::CreateMaterialParameterDirect(
            builder,
            param.first.c_str(),
            param.second);

        params.emplace_back(dstParam);
    }
    auto bin = asdx::res::CreateMaterialBinaryDirect(
        builder,
        CURRENT_VERSION,
        material.Kind,
        asdx::res::MaterialBlendState(material.BlendState),
        asdx::res::MaterialDepthState(material.DepthState),
        asdx::res::MaterialRasterizerState(material.RasterizerState),
        &params,
        &textures);

    builder.Finish(bin);

    binary.resize(builder.GetSize());
    memcpy(binary.data(), builder.GetBufferPointer(), builder.GetSize());

    return true;
}

//-----------------------------------------------------------------------------
//      逆変換処理を行います.
//-----------------------------------------------------------------------------
bool MaterialConverter::ReverseConvert(const std::vector<uint8_t>& binary, edit::Material& material)
{
    if (binary.empty())
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    auto matBin = asdx::res::GetMaterialBinary(binary.data());

    material.Kind            = matBin->Kind();
    material.BlendState      = edit::BlendState(matBin->BlendState());
    material.DepthState      = edit::DepthState(matBin->DepthState());
    material.RasterizerState = edit::RasterizerState(matBin->RasterizerState());

    auto params = matBin->Params();
    if (params != nullptr)
    {
        auto count = params->size();
        for(auto i=0u; i<count; ++i)
        {
            auto param = params->Get(i);
            material.Params[param->Name()->c_str()] = param->Value();
        }
    }

    auto textures = matBin->Textures();
    if (textures != nullptr)
    {
        auto count = textures->size();
        for(auto i=0u; i<count; ++i)
        {
            auto texture = textures->Get(i);
            material.Textures[texture->Name()->c_str()] = texture->Path()->c_str();
        }
    }

    return true;
}

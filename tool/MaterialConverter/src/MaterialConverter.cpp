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
    auto state = asdx::res::RenderState(
        asdx::res::BlendState(material.State.Blend),
        asdx::res::DepthState(material.State.Depth),
        asdx::res::RasterizerState(material.State.Rasterizer),
        material.State.UserFlag);

    std::vector<flatbuffers::Offset<asdx::res::MaterialBuffer>>  buffers;
    std::vector<flatbuffers::Offset<asdx::res::MaterialTexture>> textures;

    flatbuffers::FlatBufferBuilder builder(1024);

    for(const auto& buf : material.Buffers)
    {
        auto dstBuf = asdx::res::CreateMaterialBufferDirect(
            builder,
            buf.Name.c_str(),
            &buf.Buffer);

        buffers.emplace_back(dstBuf);
    }

    for(const auto& tex : material.Textures)
    {
        auto dstTex = asdx::res::CreateMaterialTextureDirect(
            builder,
            tex.Name.c_str(),
            tex.Path.c_str());

        textures.emplace_back(dstTex);
    }

    const char* shader = material.PixelShader.empty() ? nullptr : material.PixelShader.c_str();

    auto bin = asdx::res::CreateMaterialBinaryDirect(
        builder,
        CURRENT_VERSION,
        shader,
        &state,
        &buffers,
        &textures);

    builder.Finish(bin);

    binary.resize(builder.GetSize());
    memcpy(binary.data(), builder.GetBufferPointer(), builder.GetSize());

    return true;
}

} // namespace asdx

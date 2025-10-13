//-----------------------------------------------------------------------------
// File : FontConverter.cpp
// Desc : Font Converter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

#define NOMINMAX

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdio>
#include <Windows.h>
#include <FontConverter.h>
#include <simdjson.h>
#include <DirectXTex.h>
#include <FontBinary_generated.h>


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t CURRENT_VERSION = 1u;     //!< 現在のバイナリバージョン.


//-----------------------------------------------------------------------------
//      ワイド文字列に変換します.
//-----------------------------------------------------------------------------
std::wstring ToStringW( const std::string& value )
{
    auto length = MultiByteToWideChar(CP_ACP, 0, value.c_str(), int(value.size() + 1), nullptr, 0 );
    auto buffer = new wchar_t[length];

    MultiByteToWideChar(CP_ACP, 0, value.c_str(), int(value.size() + 1),  buffer, length );

    std::wstring result( buffer );
    delete[] buffer;

    return result;
}

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// FontConvert class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool FontConverter::Convert(const Desc& desc)
{
    {
        simdjson::ondemand::parser parser;
        auto json = simdjson::padded_string::load(desc.JsonPath.c_str());
        auto doc  = parser.iterate(json);

        auto glyphs = doc["glyphs"].get_array();
        assert(glyphs.error() == simdjson::SUCCESS);

        std::vector<asdx::res::Glyph> srcGlyph;
        srcGlyph.reserve(glyphs.count_elements().value());

        for(auto g : glyphs)
        {
            auto unicode = uint32_t(g["unicode"].get_uint64().value());
            auto advance = float(g["advance"].get_double().value());

            auto pl = 0.0f;
            auto pr = 0.0f;
            auto pt = 0.0f;
            auto pb = 0.0f;

            auto al = 0.0f;
            auto ar = 0.0f;
            auto at = 0.0f;
            auto ab = 0.0f;

            auto planeBounds = g["planeBounds"];
            if (planeBounds.error() == simdjson::SUCCESS)
            {
                pl = float(planeBounds["left"]  .get_double().value());
                pr = float(planeBounds["right"] .get_double().value());
                pt = float(planeBounds["top"]   .get_double().value());
                pb = float(planeBounds["bottom"].get_double().value());
            }

            auto atlasBounds = g["atlasBounds"];
            if (atlasBounds.error() == simdjson::SUCCESS)
            {
                al = float(atlasBounds["left"]  .get_double().value());
                ar = float(atlasBounds["right"] .get_double().value());
                at = float(atlasBounds["top"]   .get_double().value());
                ab = float(atlasBounds["bottom"].get_double().value());
            }

            asdx::res::Bounds pBound(pl, pr, pt, pb);
            asdx::res::Bounds aBound(al, ar, at, ab);

            srcGlyph.push_back(asdx::res::Glyph(unicode, advance, aBound, pBound));
        }

        auto atlas = doc["atlas"];
        assert(atlas.error() == simdjson::SUCCESS);

        auto distanceRange  = float(atlas["distanceRange"].get_double().value());
        auto fontSize       = uint32_t(atlas["size"]  .get_uint64().value());
        auto texWidth       = uint32_t(atlas["width"] .get_uint64().value());
        auto texHeight      = uint32_t(atlas["height"].get_uint64().value());
        auto isFlipY = atlas["yOrigin"].get_string().value() == std::string_view("bottom");
        assert(!isFlipY);
        if (isFlipY)
        {
            fprintf_s(stderr, "Error : yOrigin is not \"top\".");
            return false;
        }

        auto metrics = doc["metrics"];
        assert(metrics.error() == simdjson::SUCCESS);

        auto lineHeight     = float(metrics["lineHeight"].get_double().value());
        auto ascender       = float(metrics["ascender"]  .get_double().value());
        auto descender      = float(metrics["descender"] .get_double().value());

        DXGI_FORMAT format     = DXGI_FORMAT_UNKNOWN;
        uint32_t    rowPitch   = 0;
        uint32_t    slicePitch = 0;
        std::vector<uint8_t> texels;
        {
            DirectX::TexMetadata  texMetaData  = {};
            DirectX::ScratchImage scratchImage = {};

            std::wstring inputPath = ToStringW(desc.DdsPath);
            auto hr = DirectX::LoadFromDDSFile(
                inputPath.c_str(),
                DirectX::DDS_FLAGS_NONE,
                &texMetaData,
                scratchImage);
            if (FAILED(hr))
            {
                fprintf_s(stderr, "Error : DirectX::LoadFromDDSFile() Failed. errcode = 0x%x\n", hr);
                return false;
            }

            if (texMetaData.format == DXGI_FORMAT_UNKNOWN)
            {
                fprintf_s(stderr, "Error : Unsupported Resource Format.");
                return false;
            }

            if (texMetaData.width != texWidth)
            {
                fprintf_s(stderr, "Error : Width not matched. expected width = %u, actual width = %zu\n", texWidth, texMetaData.width);
                return false;
            }

            if (texMetaData.height != texHeight)
            {
                fprintf_s(stderr, "Error : Height not matched. expected height = %u, actual height = %zu\n", texHeight, texMetaData.height);
                return false;
            }

            if (texMetaData.dimension != DirectX::TEX_DIMENSION_TEXTURE2D)
            {
                fprintf_s(stderr, "Error : Invalid Deimension.");
                return false;
            }

            if (texMetaData.depth != 1 || texMetaData.arraySize != 1)
            {
                fprintf_s(stderr, "Error : Invalid DepthOrArraySize. depth = %zu, arraySize = %zu\n", texMetaData.depth, texMetaData.arraySize);
                return false;
            }

            auto images = scratchImage.GetImages();
            format      = texMetaData.format;
            rowPitch    = uint32_t(images[0].rowPitch);
            slicePitch  = uint32_t(images[0].slicePitch);

            texels.resize(scratchImage.GetPixelsSize());
            memcpy(texels.data(), scratchImage.GetPixels(), texels.size());

            scratchImage.Release();
        }

        flatbuffers::FlatBufferBuilder builder(1024);

        auto bin = asdx::res::CreateFontBinaryDirect(
            builder,
            CURRENT_VERSION,
            distanceRange,
            fontSize,
            texWidth,
            texHeight,
            rowPitch,
            slicePitch,
            (uint32_t)format,
            lineHeight,
            ascender,
            descender,
            &srcGlyph,
            &texels);

        builder.Finish(bin);

        auto buf     = builder.GetBufferPointer();
        auto bufSize = builder.GetSize();

        FILE* fp = nullptr;
        auto err = fopen_s(&fp, desc.OutputPath.c_str(), "wb");
        if (err != 0)
        {
            fprintf_s(stderr, "Output File Open Failed. path = %s\n", desc.OutputPath.c_str());
            return false;
        }

        fwrite(buf, bufSize, 1, fp);
        fclose(fp);
    }

    return true;
}

} // namespace asdx

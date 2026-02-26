//-----------------------------------------------------------------------------
// File : TextureConverter.cpp
// Desc : Texture Converter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#define NOMINMAX

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdio>
#include <vector>
#include <TextureConverter.h>
#include <TextureBinary_generated.h>
#include <DirectXTex.h>
#include <filesystem>


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t CURRENT_VERSION = 1u; //!< 現在のバイナリバージョン.


///////////////////////////////////////////////////////////////////////////////
// TEXTURE_DIMENSION enum
///////////////////////////////////////////////////////////////////////////////
enum TEXTURE_DIMENSION
{
    TEXTURE_DIMENSION_UNKNOWN,
    TEXTURE_DIMENSION_1D,
    TEXTURE_DIMENSION_2D,
    TEXTURE_DIMENSION_3D,
    TEXTURE_DIMENSION_CUBE
};

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

//-----------------------------------------------------------------------------
//      奥行または配列数を取得します.
//-----------------------------------------------------------------------------
uint16_t GetDepthOrArraySize(const DirectX::TexMetadata& metaData)
{
    if (metaData.dimension == DirectX::TEX_DIMENSION_TEXTURE3D)
        return uint16_t(metaData.depth);

    return uint16_t(metaData.arraySize);
}

//-----------------------------------------------------------------------------
//      テクスチャの次元を取得します.
//-----------------------------------------------------------------------------
uint32_t GetDimension(const DirectX::TexMetadata& metaData)
{
    switch(metaData.dimension)
    {
    case DirectX::TEX_DIMENSION_TEXTURE1D:
        return TEXTURE_DIMENSION_1D;

    case DirectX::TEX_DIMENSION_TEXTURE2D:
        if (metaData.IsCubemap())
        {
            return TEXTURE_DIMENSION_CUBE;
        }
        return TEXTURE_DIMENSION_2D;

    case DirectX::TEX_DIMENSION_TEXTURE3D:
        return TEXTURE_DIMENSION_3D;

    default:
        break;
    }

    return TEXTURE_DIMENSION_UNKNOWN;
}

} // namespace


///////////////////////////////////////////////////////////////////////////////
// TextureConverter class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool TextureConverter::Convert(const Desc& desc)
{
    if (desc.InputPath.empty() || desc.OutputPath.empty())
    {
        fprintf_s(stderr, "Invalid Arguments.");
        return false;
    }

    std::vector<uint8_t> blob;
    if (!Convert(desc.InputPath.c_str(), blob))
    {
        return false;
    }

    // ファイルに出力.
    {
        FILE* fp = nullptr;
        auto err = fopen_s(&fp, desc.OutputPath.c_str(), "wb");
        if (err != 0)
        {
            fprintf_s(stderr, "Output File Open Failed. path = %s\n", desc.OutputPath.c_str());
            return false;
        }

        // バイナリ書き込み.
        fwrite(blob.data(), blob.size(), 1, fp);
        fclose(fp);
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool TextureConverter::Convert(const char* path, std::vector<uint8_t>& output)
{
    if (path == nullptr)
    {
        fprintf_s(stderr, "Invalid Arguments.");
        return false;
    }

    DirectX::TexMetadata  texMetaData = {};
    DirectX::ScratchImage scratchImage;

    std::filesystem::path inputPath = path;
    HRESULT hr = S_OK;
    auto wpath = inputPath.wstring();

    auto ext = inputPath.extension();
    if (ext == ".dds")
    {
        hr = DirectX::LoadFromDDSFile(wpath.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, scratchImage);
    }
    else if (ext == ".tga")
    {
        hr = DirectX::LoadFromTGAFile(wpath.c_str(), DirectX::TGA_FLAGS_NONE, nullptr, scratchImage);
    }
    else if (ext == ".hdr")
    {
        hr = DirectX::LoadFromHDRFile(wpath.c_str(), nullptr, scratchImage);
    }
    else if (ext == L".bmp"  || ext == L".jpg" || ext == L".jpeg" || ext == L".png" || ext == L".tif"
          || ext == L".tiff" || ext == L".gif" || ext == L".hdp"  || ext == L".wdp" || ext == L".jxr"
          || ext == L".heif" || ext == L".heic")
    {
        hr = DirectX::LoadFromWICFile(wpath.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, scratchImage);
    }
    else
    {
        hr = E_FAIL;
        fprintf_s(stderr, "Unsupported File Extension (%s)", ext.string().c_str());
    }

    if (FAILED(hr))
    {
        fprintf_s(stderr, "TextureConvert::Convert() Failed. path = %s, errcode = 0x%x", path, hr);
        return false;
    }

    if (!Convert(scratchImage, output))
    {
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool TextureConverter::Convert(const DirectX::ScratchImage& scratchImage, std::vector<uint8_t>& output)
{
    const auto& texMetaData = scratchImage.GetMetadata();

    if (texMetaData.format == DXGI_FORMAT_UNKNOWN)
    {
        fprintf_s(stderr, "Converter Error: Unsupported Resource Format. format = 0x%x\n", texMetaData.format);
        return false;
    }

    if (texMetaData.mipLevels >= 15)
    {
        fprintf_s(stderr,"Converter Error: Invalid MipLevles. mipLevels = %zu\n", texMetaData.mipLevels);
        return false;
    }

    // 独自バイナリ形式に変換.
    {
        flatbuffers::FlatBufferBuilder builder(1024);

        // サブリソースを準備.
        std::vector<asdx::res::SubResource> subresources;
        subresources.resize(scratchImage.GetImageCount());

        // サブリソース構築.
        const auto* images = scratchImage.GetImages();
        uint64_t pixelOffset = 0;
        for(size_t i=0; i<scratchImage.GetImageCount(); ++i)
        {
            subresources[i] = asdx::res::SubResource(
                uint32_t(images[i].width),
                uint32_t(images[i].height),
                images[i].rowPitch,
                images[i].slicePitch,
                pixelOffset);

            pixelOffset += images[i].slicePitch;
        }

        // テクスチャバイナリを作成.
        auto resource = CreateTextureBinary(
            builder,
            CURRENT_VERSION,
            GetDimension(texMetaData),
            uint32_t(texMetaData.width),
            uint32_t(texMetaData.height),
            GetDepthOrArraySize(texMetaData),
            uint16_t(texMetaData.mipLevels),
            uint32_t(texMetaData.format),
            builder.CreateVectorOfStructs<asdx::res::SubResource>(subresources),
            builder.CreateVector<uint8_t>(scratchImage.GetPixels(), scratchImage.GetPixelsSize()));

        // ビルド終了.
        builder.Finish(resource);

        output.resize(builder.GetSize());
        memcpy(output.data(), builder.GetBufferPointer(), builder.GetSize());
    }

    return true;
}

//-----------------------------------------------------------------------------
//      逆変換処理を行います.
//-----------------------------------------------------------------------------
bool TextureConverter::ReverseConvert(const std::vector<uint8_t>& input, DirectX::ScratchImage& output)
{
    if (input.empty())
    {
        fprintf_s(stderr, "Converter Error : Invalid Argument.\n");
        return false;
    }

    auto bin = asdx::res::GetTextureBinary(input.data());

    std::vector<DirectX::Image> images;
    images.resize(bin->DepthOrArraySize());

    auto pixels = const_cast<uint8_t*>(bin->Texels()->data());
    uint64_t offset = 0;

    for(auto i=0u; i<bin->SubResources()->size(); ++i)
    {
        auto subres = bin->SubResources()->Get(i);
        images[i].width         = subres->Width();
        images[i].height        = subres->Height();
        images[i].format        = DXGI_FORMAT(bin->Format());
        images[i].rowPitch      = subres->RowPitch();
        images[i].slicePitch    = subres->SlicePitch();
        images[i].pixels        = pixels + offset;

        offset += subres->SlicePitch();
    }

    HRESULT hr = S_OK;

    switch(bin->Dimension())
    {
    case TEXTURE_DIMENSION_1D:
    case TEXTURE_DIMENSION_2D:
        {
            hr = output.InitializeArrayFromImages(images.data(), images.size());
        }
        break;

    case TEXTURE_DIMENSION_3D:
        {
            hr = output.Initialize3DFromImages(images.data(), images.size());
        }
        break;

    case TEXTURE_DIMENSION_CUBE:
        {
            hr = output.InitializeCubeFromImages(images.data(), images.size());
        }
        break;
    }

    if (FAILED(hr))
    {
        fprintf_s(stderr, "ReverseConvert Failed. errcode = 0x%x", hr);
        return false;
    }

    return true;
}

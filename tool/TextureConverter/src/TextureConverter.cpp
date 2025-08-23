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


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t CURRENT_VERSION = 1u; //!< 現在のバイナリバージョン.


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

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// TextureConverter class
///////////////////////////////////////////////////////////////////////////////
bool TextureConverter::Convert(const Desc& desc)
{
    if (desc.InputPath.empty() || desc.OutputPath.empty())
    {
        fprintf_s(stderr, "Invalid Arguments.");
        return false;
    }

    DirectX::TexMetadata  texMetaData = {};
    DirectX::ScratchImage scratchImage;

    // DDSしか受け付けない.
    // (フォーマット変換をバイナリコンバートの度に実行するのは時間がかかるため).
    {
        std::wstring inputPath = ToStringW(desc.InputPath);

        auto hr = DirectX::LoadFromDDSFile(
            inputPath.c_str(),
            DirectX::DDS_FLAGS_NONE,
            &texMetaData,
            scratchImage);
        if (FAILED(hr))
        {
            fprintf_s(stderr, "DirectX::LoadFromDDSFile() Failed. errcode = 0x%x\n", hr);
            return false;
        }
    }

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
        std::vector<asdx::res::SubresourceInfo> subresources;
        subresources.resize(scratchImage.GetImageCount());

        // サブリソース構築.
        const auto* images = scratchImage.GetImages();
        for(size_t i=0; i<scratchImage.GetImageCount(); ++i)
        {
            subresources[i] = asdx::res::SubresourceInfo(
                uint32_t(images[i].rowPitch),
                uint32_t(images[i].slicePitch));
        }

        // テクスチャバイナリを作成.
        auto resource = CreateTextureBinary(
            builder,
            CURRENT_VERSION,
            uint32_t(texMetaData.dimension),
            uint32_t(texMetaData.width),
            uint32_t(texMetaData.height),
            GetDepthOrArraySize(texMetaData),
            uint16_t(texMetaData.mipLevels),
            uint32_t(texMetaData.format),
            builder.CreateVectorOfStructs<asdx::res::SubresourceInfo>(subresources),
            builder.CreateVector<uint8_t>(scratchImage.GetPixels(), scratchImage.GetPixelsSize()));
        
        // ビルド終了.
        builder.Finish(resource);

        // バイナリ取得.
        auto binary     = builder.GetBufferPointer();
        auto binarySize = builder.GetSize();

        FILE* fp = nullptr;
        auto err = fopen_s(&fp, desc.OutputPath.c_str(), "wb");
        if (err != 0)
        {
            fprintf_s(stderr, "Output File Open Failed. path = %s\n", desc.OutputPath.c_str());
            return false;
        }

        // バイナリ書き込み.
        fwrite(binary, binarySize, 1, fp);
        fclose(fp);
    }

    // 正常終了.
    return true;
}

} // namespace asdx

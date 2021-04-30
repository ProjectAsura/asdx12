//-----------------------------------------------------------------------------
// File : asdxResMaterial.cpp
// Desc : Material Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <cstdio>
#include <res/asdxResMaterial.h>
#include <fnd/asdxLogger.h>


namespace {

///////////////////////////////////////////////////////////////////////////////
// MaterialFileHeader structure
///////////////////////////////////////////////////////////////////////////////
struct MaterialFileHeader
{
    uint8_t     Magic[4];
    uint32_t    Version;
};

///////////////////////////////////////////////////////////////////////////////
// MaterialHeader structure
///////////////////////////////////////////////////////////////////////////////
struct MaterialHeader
{
    uint32_t    NameSize;           //!< マテリアル名の長さです.
    uint32_t    PixelShaderSize;    //!< ピクセルシェーダ名の長さです.
    uint8_t     State;              //!< マテリアルステートです.
    uint8_t     DisplayFace;        //!< 表示面設定です.
    uint8_t     ShadowCast;         //!< シャドウキャスト.
    uint8_t     ShadowReceive;      //!< シャドウレシーブ.
    uint32_t    ParameterCount;     //!< パラメータ数です.
    uint32_t    TextureCount;       //!< テクスチャ数です.
    uint32_t    BufferSize;         //!< 定数バッファサイズです.
};

///////////////////////////////////////////////////////////////////////////////
// MaterialParameterHeader structure
///////////////////////////////////////////////////////////////////////////////
struct MaterialParameterHeader
{
    uint32_t    Type;       //!< データ型です.
    uint32_t    Offset;     //!< バッファ先頭からのオフセットです.
    uint16_t    NameSize;   //!< 名前の長さです.
};

///////////////////////////////////////////////////////////////////////////////
// MaterialTextureHeader
///////////////////////////////////////////////////////////////////////////////
struct MaterialTextureHeader
{
    uint8_t     Usage;      //!< テクスチャの使用用途.
    uint32_t    PathSize;   //!< パスの長さです.
};

} // namespace


namespace asdx {

//-----------------------------------------------------------------------------
//      マテリアルを保存します.
//-----------------------------------------------------------------------------
bool SaveMaterial(const char* path, const ResMaterial& material)
{
    FILE* pFile;
    auto err = fopen_s(&pFile, path, "wb");
    if (err != 0)
    {
        ELOGA("Error : File Open Failed. path = %s", path);
        return false;
    }

    MaterialFileHeader header;
    header.Magic[0] = 'M';
    header.Magic[1] = 'T';
    header.Magic[2] = 'L';
    header.Magic[3] = '\0';
    header.Version  = 0x1;

    fwrite(&header, sizeof(header), 1, pFile);

    {
        MaterialHeader materialHeader = {};
        materialHeader.NameSize         = uint32_t(material.Name.size());
        materialHeader.PixelShaderSize  = uint32_t(material.PixelShader.size());
        materialHeader.State            = material.State;
        materialHeader.DisplayFace      = material.DisplayFace;
        materialHeader.ShadowCast       = material.ShadowCast;
        materialHeader.ShadowReceive    = material.ShadowReceive;
        materialHeader.ParameterCount   = material.ParameterCount;
        materialHeader.TextureCount     = material.TextureCount;
        materialHeader.BufferSize       = material.BufferSize;
        assert(materialHeader.NameSize < 512);
        assert(materialHeader.PixelShaderSize < 512);

        fwrite(&materialHeader, sizeof(materialHeader), 1, pFile);
        fwrite(material.Name.data(), material.Name.size(), 1, pFile);
        fwrite(material.PixelShader.data(), material.PixelShader.size(), 1, pFile);

        for(size_t j=0; j<material.Parameters.size(); ++j)
        {
            auto& param = material.Parameters[j];

            MaterialParameterHeader paramHeader = {};
            paramHeader.Type       = uint32_t(param.Type);
            paramHeader.Offset     = param.Offset;
            paramHeader.NameSize   = uint32_t(param.Name.size());
            assert(paramHeader.NameSize < 512);

            fwrite(&paramHeader, sizeof(paramHeader), 1, pFile);
            fwrite(param.Name.data(), param.Name.size(), 1, pFile);
        }

        for(size_t j=0; j<material.Textures.size(); ++j)
        {
            auto& texture = material.Textures[j];

            MaterialTextureHeader textureHeader = {};
            textureHeader.Usage       = texture.Usage;
            textureHeader.PathSize    = uint32_t(texture.Path.size());
            assert(textureHeader.PathSize < 512);

            fwrite(&textureHeader, sizeof(textureHeader), 1, pFile);
            fwrite(texture.Path.data(), texture.Path.size(), 1, pFile);
        }

        fwrite(material.Buffer.data(), material.Buffer.size(), 1, pFile);
    }

    fclose(pFile);

    return true;
}

//-----------------------------------------------------------------------------
//      マテリアルを読み込みます.
//-----------------------------------------------------------------------------
bool LoadMaterial(const char* path, ResMaterial& material)
{
    FILE* pFile;
    auto err = fopen_s(&pFile, path, "rb");
    if (err != 0)
    {
        ELOGA("Error : File Open Failed. path = %s", path);
        return false;
    }

    MaterialFileHeader header;
    fread(&header, sizeof(header), 1, pFile);

    if (header.Magic[0] != 'M'
     || header.Magic[1] != 'T'
     || header.Magic[2] != 'L'
     || header.Magic[3] != '\0')
    {
        ELOGA("Error : Invalid File. path = %s", path);
        return false;
    }

    if (header.Version == 0x1)
    {
        MaterialHeader materialHeader = {};
        fread(&materialHeader, sizeof(materialHeader), 1, pFile);

        material.State          = materialHeader.State;
        material.DisplayFace    = materialHeader.DisplayFace;
        material.ShadowCast     = materialHeader.ShadowCast;
        material.ShadowReceive  = materialHeader.ShadowReceive;
        material.ParameterCount = materialHeader.ParameterCount;
        material.TextureCount   = materialHeader.TextureCount;
        material.BufferSize     = materialHeader.BufferSize;

        material.Parameters .resize(material.ParameterCount);
        material.Textures   .resize(material.TextureCount);
        material.Buffer     .resize(material.BufferSize);

        {
            char name[512] = {};
            assert(materialHeader.NameSize < 512);
            fread(name, materialHeader.NameSize, 1, pFile);
            material.Name = name;
        }

        for(auto i=0u; i<material.ParameterCount; ++i)
        {
            auto& p = material.Parameters[i];

            MaterialParameterHeader param = {};
            fread(&param, sizeof(param), 1, pFile);

            p.Type   = param.Type;
            p.Offset = param.Offset;

            char name[512] = {};
            assert(param.NameSize < 512);
            fread(name, param.NameSize, 1, pFile);
            p.Name = name;
        }

        for(auto i=0u; i<material.TextureCount; ++i)
        {
            MaterialTextureHeader texture = {};
            fread(&texture, sizeof(texture), 1, pFile);

            char buf[512] = {};
            assert(texture.PathSize < 512);
            fread(buf, texture.PathSize, 1, pFile);

            material.Textures[i].Usage = TEXTURE_USAGE(texture.Usage);
            material.Textures[i].Path  = buf;
        }

        fread(material.Buffer.data(), material.BufferSize, 1, pFile);
    }
    else
    {
        ELOGA("Error : Invalid Version. path = %s", path);
        fclose(pFile);
        return false;
    }

    fclose(pFile);
    return true;
}


} // namespace asdx
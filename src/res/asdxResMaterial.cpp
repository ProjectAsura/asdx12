//-----------------------------------------------------------------------------
// File : asdxResMaterial.cpp
// Desc : Material Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdio>
#include <res/asdxResMaterial.h>
#include <core/asdxLogger.h>


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
    uint32_t    Hash;               //!< マテリアル名を表すハッシュ値です.
    uint8_t     State;              //!< マテリアルステートです.
    uint8_t     DisplayFace;        //!< 表示面設定です.
    uint8_t     ShadowCast;         //!< シャドウキャスト.
    uint8_t     ShadowReceive;      //!< シャドウレシーブ.
    uint32_t    ParameterCount;     //!< パラメータ数です.
    uint32_t    TextureCount;       //!< テクスチャ数です.
    uint32_t    BufferSize;         //!< 定数バッファサイズです.
};

///////////////////////////////////////////////////////////////////////////////
// MaterialParameter structure
///////////////////////////////////////////////////////////////////////////////
struct MaterialParameter
{
    uint32_t    Type;       //!< データ型です.
    uint32_t    Hash;       //!< 名前を表すハッシュ値です.
    uint32_t    Count;      //!< 要素数です.
    uint32_t    Offset;     //!< バッファ先頭からのオフセットです.
};

///////////////////////////////////////////////////////////////////////////////
// MaterialTextureHeader
///////////////////////////////////////////////////////////////////////////////
struct MaterialTextureHeader
{
    uint8_t     Usage;      //!< テクスチャの使用用途.
    uint32_t    Size;       //!< 文字列サイズ.
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
        materialHeader.Hash             = material.Hash;
        materialHeader.State            = material.State;
        materialHeader.DisplayFace      = material.DisplayFace;
        materialHeader.ShadowCast       = material.ShadowCast;
        materialHeader.ShadowReceive    = material.ShadowReceive;
        materialHeader.ParameterCount   = material.ParameterCount;
        materialHeader.TextureCount     = material.TextureCount;
        materialHeader.BufferSize       = material.BufferSize;

        fwrite(&materialHeader, sizeof(materialHeader), 1, pFile);

        for(size_t j=0; j<material.Parameters.size(); ++j)
        {
            auto& param = material.Parameters[j];

            MaterialParameter paramHeader = {};
            paramHeader.Type    = uint32_t(param.Type);
            paramHeader.Hash    = param.Hash;
            paramHeader.Count   = param.Count;
            paramHeader.Offset  = param.Offset;

            fwrite(&paramHeader, sizeof(paramHeader), 1, pFile);
        }

        for(size_t j=0; j<material.Textures.size(); ++j)
        {
            auto& texture = material.Textures[j];

            MaterialTextureHeader textureHeader = {};
            textureHeader.Usage = texture.Usage;
            textureHeader.Size  = uint32_t(texture.Path.size());
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

        material.Hash           = materialHeader.Hash;
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

        for(auto i=0u; i<material.ParameterCount; ++i)
        {
            auto& p = material.Parameters[i];

            MaterialParameter param = {};
            fread(&param, sizeof(param), 1, pFile);

            p.Type   = param.Type;
            p.Hash   = param.Hash;
            p.Count  = param.Count;
            p.Offset = param.Offset;
        }

        for(auto i=0u; i<material.TextureCount; ++i)
        {
            MaterialTextureHeader texture = {};
            fread(&texture, sizeof(texture), 1, pFile);

            if (texture.Size > 512)
            { texture.Size = 512; }
            
            char buf[512] = {};
            fread(buf, texture.Size, 1, pFile);

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
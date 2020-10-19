//-----------------------------------------------------------------------------
// File : asdxResMaterial.cpp
// Desc : Material Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdio>
#include <asdxResMaterial.h>
#include <asdxLogger.h>


namespace {

struct MaterialHeader
{
    uint8_t     Magic[4];
    uint32_t    Version;
    uint32_t    MaterialCount;
};

struct MaterialHeaderV1
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

struct MaterialParameterV1
{
    uint32_t    Type;       //!< データ型です.
    uint32_t    Hash;       //!< 名前を表すハッシュ値です.
    uint32_t    Count;      //!< 要素数です.
    uint32_t    Offset;     //!< バッファ先頭からのオフセットです.
};

} // namespace

namespace asdx {

bool SaveMaterials(const char* path, const ResMaterials& materials)
{
    FILE* pFile;
    auto err = fopen_s(&pFile, path, "wb");
    if (err != 0)
    {
        ELOGA("Error : File Open Failed. path = %s", path);
        return false;
    }

    MaterialHeader header;
    header.Magic[0] = 'M';
    header.Magic[1] = 'T';
    header.Magic[2] = 'L';
    header.Magic[3] = '\0';
    header.Version  = 0x1;
    header.MaterialCount = uint32_t(materials.Materials.size());

    fwrite(&header, sizeof(header), 1, pFile);

    for(size_t i=0; i<materials.Materials.size(); ++i)
    {
        auto& material = materials.Materials[i];

        MaterialHeaderV1 materialHeader = {};
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

            MaterialParameterV1 paramHeader = {};
            paramHeader.Type    = uint32_t(param.Type);
            paramHeader.Hash    = param.Hash;
            paramHeader.Count   = param.Count;
            paramHeader.Offset  = param.Offset;

            fwrite(&paramHeader, sizeof(paramHeader), 1, pFile);
        }

        for(size_t j=0; j<material.Textures.size(); ++j)
        {
            char path[256] = {};
            strcpy_s(path, material.Textures[j].c_str());

            fwrite(path, sizeof(char), 256, pFile);
        }

        fwrite(material.pBuffer.data(), material.pBuffer.size(), 1, pFile);
    }

    fclose(pFile);

    return true;
}

bool LoadMaterials(const char* path, ResMaterials& materials)
{
    FILE* pFile;
    auto err = fopen_s(&pFile, path, "rb");
    if (err != 0)
    {
        ELOGA("Error : File Open Failed. path = %s", path);
        return false;
    }

    MaterialHeader header;
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
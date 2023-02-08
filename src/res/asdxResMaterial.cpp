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

constexpr uint32_t MAX_NAME_SIZE = 512;

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
    uint32_t                NameSize;           //!< マテリアル名の長さです.
    uint32_t                TextureCount;       //!< テクスチャ数です.
    uint32_t                BufferSize;         //!< 定数バッファサイズです.
    asdx::ResMaterialState  State;              //!< ステートです.
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
        materialHeader.State            = material.State;
        materialHeader.TextureCount     = material.TextureCount;
        materialHeader.BufferSize       = material.BufferSize;
        assert(materialHeader.NameSize < MAX_NAME_SIZE);

        fwrite(&materialHeader, sizeof(materialHeader), 1, pFile);
        fwrite(material.Name.data(), material.Name.size(), 1, pFile);

        for(size_t j=0; j<material.Textures.size(); ++j)
        {
            auto& texture = material.Textures[j];
            uint32_t texturePathSize = uint32_t(texture.size());
            assert(texturePathSize < MAX_NAME_SIZE);
            fwrite(&texturePathSize, sizeof(texturePathSize), 1, pFile);
            fwrite(texture.data(), texture.size(), 1, pFile);
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
        material.TextureCount   = materialHeader.TextureCount;
        material.BufferSize     = materialHeader.BufferSize;

        material.Textures   .resize(material.TextureCount);
        material.Buffer     .resize(material.BufferSize);

        {
            char name[MAX_NAME_SIZE] = {};
            fread(name, materialHeader.NameSize, 1, pFile);
            material.Name = name;
        }

        for(auto i=0u; i<material.TextureCount; ++i)
        {
            uint32_t pathSize = 0;
            fread(&pathSize, sizeof(pathSize), 1, pFile);
            assert(pathSize < 512);

            char buf[512] = {};
            fread(buf, pathSize, 1, pFile);

            material.Textures[i] = buf;
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

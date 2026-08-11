//-----------------------------------------------------------------------------
// File : SampleUtil.cpp
// Desc : Sample Utility
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <SampleUtil.h>
#include <fnd/asdxPath.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxFileIO.h>


///////////////////////////////////////////////////////////////////////////////
// Functions.
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      テクスチャをロードします.
//-----------------------------------------------------------------------------
bool LoadTexture(const char* path, asdx::TextureHolder& holder)
{
    asdx::fs::path input = path;
    asdx::fs::path findPath;
    if (!asdx::SearchFilePath(input, findPath))
    {
        ELOGA("Error : SearchFilePath() Failed. path = %s", path);
        return false;
    }

    holder = asdx::TextureManager::Instance().GetOrCreate(findPath.string().c_str());
    if (!holder.IsValid())
    {
        ELOGA("Error : Texture Load Faild. path = %s", findPath.string().c_str());
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      フォントをロードします.
//-----------------------------------------------------------------------------
bool LoadFont(const char* path, asdx::Font& font)
{
    asdx::fs::path input = path;
    asdx::fs::path findPath;
    if (!asdx::SearchFilePath(input, findPath))
    {
        ELOGA("Error : SearchFilePath() Failed. path = %s", path);
        return false;
    }

    std::vector<uint8_t> bin;
    if (!asdx::LoadA(findPath.string().c_str(), bin))
    {
        ELOGA("Error : File Load Failed. path = %s", findPath.string().c_str());
        return false;
    }

    if (!font.Init(std::move(bin)))
    {
        ELOGA("Error : Font::Init() Failed. path = %s", path);
        return false;
    }

    return true;
}

#if ASDX_ENABLE_SOUND
//-----------------------------------------------------------------------------
//      WAVファイルをロードします.
//-----------------------------------------------------------------------------
bool LoadWav(const char* path, asdx::SoundResource& resource, bool loop = false)
{
    asdx::fs::path input = path;
    asdx::fs::path findPath;
    if (!asdx::SearchFilePath(input, findPath))
    {
        ELOGA("Error : SearchFilePath() Failed. path = %s", path);
        return false;
    }

    asdx::SoundData data;
    if (!asdx::LoadFromWav(findPath.string().c_str(), data))
    {
        ELOGA("Error : File Load Failed. path = %s", findPath.string().c_str());
        return false;
    }

    data.Loop = loop;

    if (!resource.Init(data))
    {
        ELOGA("Error : SoundResource::Init() Failed.");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      Ogg Vorbis をロードします.
//-----------------------------------------------------------------------------
bool LoadOgg(const char* path, asdx::SoundResource& resource, bool loop = false)
{
    asdx::fs::path input = path;
    asdx::fs::path findPath;
    if (!asdx::SearchFilePath(input, findPath))
    {
        ELOGA("Error : SearchFilePath() Failed. path = %s", path);
        return false;
    }

    asdx::SoundData data;
    if (!asdx::LoadFromWav(findPath.string().c_str(), data))
    {
        ELOGA("Error : File Load Failed. path = %s", findPath.string().c_str());
        return false;
    }

    data.Loop = loop;

    if (!resource.Init(data))
    {
        ELOGA("Error : SoundResource::Init() Failed.");
        return false;
    }

    return true;
}

#endif//ASDX_ENABLE_SOUND

//-----------------------------------------------------------------------------
// File : asdxSoundData.h
// Desc : Sound Data.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <Windows.h>
#include <fnd/asdxLogger.h>
#include <snd/asdxSoundResource.h>
#include "../external/stb/stb_vorbis.h"


// Little-Endian.
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT  ' tmf'
#define fourccWAVE 'EVAW'
#define fourccDPDS 'sdpd'


namespace {

//-----------------------------------------------------------------------------
//      チャンクを検索します.
//-----------------------------------------------------------------------------
HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
{
    HRESULT hr = S_OK;
    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
        return HRESULT_FROM_WIN32(GetLastError());

    DWORD dwChunkType       = 0;
    DWORD dwChunkDataSize   = 0;
    DWORD dwRIFFDataSize    = 0;
    DWORD dwFileType        = 0;
    DWORD dwOffset          = 0;

    while(hr == S_OK)
    {
        DWORD dwRead;
        if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
            hr = HRESULT_FROM_WIN32(GetLastError());

        if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
            hr = HRESULT_FROM_WIN32(GetLastError());

        switch(dwChunkType)
        {
        case fourccRIFF:
            {
                dwRIFFDataSize  = dwChunkDataSize;
                dwChunkDataSize = 4;
                if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
                    hr = HRESULT_FROM_WIN32(GetLastError());
            }
            break;

        default:
            {
                if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
                    return HRESULT_FROM_WIN32(GetLastError());
            }
            break;
        }

        dwOffset += sizeof(DWORD) * 2;

        if (dwChunkType == fourcc)
        {
            dwChunkSize         = dwChunkDataSize;
            dwChunkDataPosition = dwOffset;
            return S_OK;
        }

        dwOffset += dwChunkDataSize;

        if(dwOffset >= dwRIFFDataSize)
            return S_FALSE;
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
//      チャンクを読み込みます.
//-----------------------------------------------------------------------------
HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD bufferSize, DWORD bufferOffset)
{
    HRESULT hr = S_OK;
    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferOffset, NULL, FILE_BEGIN))
        return HRESULT_FROM_WIN32(GetLastError());

    DWORD dwRead = 0;
    if (0 == ReadFile(hFile, buffer, bufferSize, &dwRead, NULL))
        hr = HRESULT_FROM_WIN32(GetLastError());

    return hr;
}

} // namespace

namespace asdx {

//-----------------------------------------------------------------------------
//      wavファイルからサウンドデータを読み込みます.
//-----------------------------------------------------------------------------
bool LoadFromWav(const char* path, SoundData& result)
{
    HANDLE hFile = CreateFileA(
        path,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        ELOGA("Error : CreateFileA() Failed. path = %s", path);
        return false;
    }

    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
    {
        ELOGA("Error : SetFilePointer() Failed.");
        return false;
    }

    DWORD dwChunkSize;
    DWORD dwChunkPosition;

    auto hr = FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
    if (FAILED(hr))
    {
        ELOGA("Error : FindChunk() Failed. errcode = 0x%x", hr);
        return false;
    }

    DWORD fileType;
    hr = ReadChunkData(hFile, &fileType, sizeof(DWORD), dwChunkPosition);
    if (FAILED(hr))
    {
        ELOGA("Error : ReadChunkData() Failed. errcode = 0x%x", hr);
        return false;
    }

    if (fileType != fourccWAVE)
    {
        ELOGA("Error : Invalid File Type. value = %u", fileType);
        return false;
    }

    hr = FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
    if (FAILED(hr))
    {
        ELOGA("Error : FindChunk() Failed. errcode = 0x%x", hr);
        return false;
    }

    result.FormatData.resize(dwChunkSize);
    hr = ReadChunkData(hFile, result.FormatData.data(), dwChunkSize, dwChunkPosition);
    if (FAILED(hr))
    {
        ELOGA("Error : ReadChunkData() Failed. errcode = 0x%x", hr);
        return false;
    }

    hr = FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
    if (FAILED(hr))
    {
        ELOGA("Error : FindChunk() Failed. errcode = 0x%x", hr);
        return false;
    }

    result.AudioData.resize(dwChunkSize);
    hr = ReadChunkData(hFile, result.AudioData.data(), dwChunkSize, dwChunkPosition);
    if (FAILED(hr))
    {
        ELOGA("Error : ReadChunkData() Failed. errcode = 0x%x", hr);
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      oggファイルからサウンドデータを読み込みます.
//-----------------------------------------------------------------------------
bool LoadFromOgg(const char* path, SoundData& data)
{
    int    channels  = 0;
    int    sampleRate = 0;
    short* output     = nullptr;

    auto samples = stb_vorbis_decode_filename(path, &channels, &sampleRate, &output);
    if (samples <= 0)
    {
        ELOGA("Error : File Load Failed. path = %s", path);
        return false;
    }

    auto size = samples * channels * sizeof(short);
    data.AudioData.resize(size);
    memcpy(data.AudioData.data(), output, size);

    WAVEFORMATEX wfx = {};
    wfx.wFormatTag      = WAVE_FORMAT_PCM;
    wfx.nChannels       = WORD(channels);
    wfx.nSamplesPerSec  = sampleRate;
    wfx.wBitsPerSample  = 16;
    wfx.nBlockAlign     = (wfx.nChannels * wfx.wBitsPerSample) / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize          = 0;

    data.FormatData.resize(sizeof(wfx));
    memcpy(data.FormatData.data(), &wfx, sizeof(wfx));

    return true;
}

} // namespace asdx

#undef fourccRIFF
#undef fourccDATA
#undef fourccFMT
#undef fourccWAVE
#undef fourccDPDS

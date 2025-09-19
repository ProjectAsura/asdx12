//-----------------------------------------------------------------------------
// File : asdxBlob.cpp
// Desc : Binary Large Object.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxBlob.h>
#include <fnd/asdxLogger.h>
#include <cstdio>
#include <cstdlib>
#include <cassert>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Blob structure
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Blob::Blob()
: m_pBuffer     (nullptr)
, m_BufferSize  (0)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Blob::~Blob()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Blob::Init(size_t size)
{
    assert(m_pBuffer == nullptr);
    m_pBuffer = malloc(size);
    if (m_pBuffer == nullptr)
    {
        ELOG("Error : Out of Memory.");
        return false;
    }

    m_BufferSize = size;
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Blob::Term()
{
    if (m_pBuffer)
    {
        free(m_pBuffer);
        m_pBuffer = nullptr;
    }
    m_BufferSize = 0;
}

//-----------------------------------------------------------------------------
//      バイナリをロードします.
//-----------------------------------------------------------------------------
bool Blob::LoadA(const char* path)
{
    // バイナリをロード.
    {
        if (path == nullptr)
        {
            ELOG("Error : Invalid Argument.");
            return false;
        }

        FILE* fp = nullptr;
        auto err = fopen_s(&fp, path, "rb");
        if (err != 0 || fp == nullptr)
        {
            ELOG("Error : File Open Failed. path = %s", path);
            return false;
        }

        auto begin = ftell(fp);
        fseek(fp, 0, SEEK_END);
        auto end = ftell(fp);
        size_t bufferSize = end - begin;
        fseek(fp, 0, SEEK_SET);

        if (!Init(bufferSize))
        {
            fclose(fp);
            ELOG("Error : Out of Memory.");
            return false;
        }

        fread(GetBuffer(), bufferSize, 1, fp);
        fclose(fp);
    }

    return true;
}

//-----------------------------------------------------------------------------
//      バイナリをロードします.
//-----------------------------------------------------------------------------
bool Blob::LoadW(const wchar_t* path)
{
    // バイナリをロード.
    {
        if (path == nullptr)
        {
            ELOG("Error : Invalid Argument.");
            return false;
        }

        FILE* fp = nullptr;
        auto err = _wfopen_s(&fp, path, L"rb");
        if (err != 0 || fp == nullptr)
        {
            ELOG("Error : File Open Failed. path = %ls", path);
            return false;
        }

        auto begin = ftell(fp);
        fseek(fp, 0, SEEK_END);
        auto end = ftell(fp);
        size_t bufferSize = end - begin;
        fseek(fp, 0, SEEK_SET);

        if (!Init(bufferSize))
        {
            fclose(fp);
            ELOG("Error : Out of Memory.");
            return false;
        }

        fread(GetBuffer(), bufferSize, 1, fp);
        fclose(fp);
    }

    return true;
}

//-----------------------------------------------------------------------------
//      バッファデータを取得します.
//-----------------------------------------------------------------------------
void* Blob::GetBuffer() const
{ return m_pBuffer; }

//-----------------------------------------------------------------------------
//      バッファサイズを返却します.
//-----------------------------------------------------------------------------
size_t Blob::GetBufferSize() const
{ return m_BufferSize; }

} // namespace asdx

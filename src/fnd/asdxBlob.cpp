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
#include <atomic>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Blob class
///////////////////////////////////////////////////////////////////////////////
class Blob : public asdx::IBlob
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    size_t m_Size = 0;
    void*  m_Buffer = nullptr;

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //      コンストラクタです.
    //-------------------------------------------------------------------------
    Blob()
    : m_Count(1)
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //      デストラクタです.
    //-------------------------------------------------------------------------
    ~Blob()
    {
        if (m_Buffer)
        {
            free(m_Buffer);
            m_Buffer = nullptr;
        }
        m_Size = 0;
    }

    //-------------------------------------------------------------------------
    //      メモリを確保します.
    //-------------------------------------------------------------------------
    bool Alloc(size_t size)
    {
        m_Buffer = malloc(size);
        if (m_Buffer == nullptr)
        { return false; }

        m_Size = size;
        return true;
    }

    //-------------------------------------------------------------------------
    //      参照カウントを上げます.
    //-------------------------------------------------------------------------
    void AddRef() override
    { m_Count++; }

    //-------------------------------------------------------------------------
    //      参照カウントを下げます.
    //-------------------------------------------------------------------------
    void Release() override
    {
        m_Count--;
        if (m_Count == 0)
        { delete this; }
    }

    //-------------------------------------------------------------------------
    //      参照カウントを取得します.
    //-------------------------------------------------------------------------
    uint32_t GetCount() const override
    { return m_Count; }

    //-------------------------------------------------------------------------
    //      バッファサイズを取得します.
    //-------------------------------------------------------------------------
    size_t GetBufferSize() override
    { return m_Size; }

    //-------------------------------------------------------------------------
    //      バッファデータを取得します.
    //-------------------------------------------------------------------------
    void* GetBuffer() override
    { return m_Buffer; }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    std::atomic<uint32_t> m_Count = {};

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

//-----------------------------------------------------------------------------
//      バイナリラージオブジェクトを生成します.
//-----------------------------------------------------------------------------
bool CreateBlob(size_t size, IBlob** ppResult)
{
    auto blob = new Blob();
    if (!blob->Alloc(size))
    {
        delete blob;
        return false;
    }

    *ppResult = blob;
    return true;
}

//-----------------------------------------------------------------------------
//      バイナリラージオブジェクトに読み込みます.
//-----------------------------------------------------------------------------
bool ReadFileToBlobA(const char* filename, IBlob** ppResult)
{
    FILE* pFile = nullptr;
    auto err = fopen_s(&pFile, filename, "rb");
    if (err != 0 || pFile == nullptr)
    {
        ELOGA("Error : ReadFileToBlobA() File. File open failed. path = %s, errcode = 0x%x", filename, err);
        return false;
    }

    auto prevpos = ftell(pFile);
    fseek(pFile, 0, SEEK_END);
    auto currpos = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    auto size = uint64_t(currpos) - uint64_t(prevpos);
    auto ptr = malloc(size);
    if (ptr == nullptr)
    {
        ELOG("Error : Out of memory.");
        fclose(pFile);
        return false;
    }

    fread(ptr, size, 1, pFile);
    fclose(pFile);

    auto blob = new Blob();
    blob->m_Buffer = ptr;
    blob->m_Size = size;

    *ppResult = blob;
    return true;
}

//-----------------------------------------------------------------------------
//      バイナリラージオブジェクトに読み込みます.
//-----------------------------------------------------------------------------
bool ReadFileToBlobW(const wchar_t* filename, IBlob** ppResult)
{
    FILE* pFile = nullptr;
    auto err = _wfopen_s(&pFile, filename, L"rb");
    if (err != 0 || pFile == nullptr)
    {
        ELOG("Error : ReadFileToBlobW() Failed. File open failed. path = %ls, errcode = 0x%x", filename, err);
        return false;
    }

    auto prevpos = ftell(pFile);
    fseek(pFile, 0, SEEK_END);
    auto currpos = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    auto size = uint64_t(currpos) - uint64_t(prevpos);
    auto ptr = malloc(size);
    if (ptr == nullptr)
    {
        fclose(pFile);
        return false;
    }

    fread(ptr, size, 1, pFile);
    fclose(pFile);

    auto blob = new Blob();
    blob->m_Buffer = ptr;
    blob->m_Size = size;

    *ppResult = blob;
    return true;
}


//-----------------------------------------------------------------------------
//      バイナリラージオブジェクトを書き出します.
//-----------------------------------------------------------------------------
bool WriteBlobToFileA(IBlob* pBlob, const char* filename)
{
    FILE* pFile = nullptr;
    auto err = fopen_s(&pFile, filename, "wb");
    if (err != 0 || pFile == nullptr)
    {
        ELOG("Error : WriteBlobToFileA() Failed. File open failed. path = %s, errcode = 0x%x", filename, err);
        return false;
    }

    fwrite(pBlob->GetBuffer(), pBlob->GetBufferSize(), 1, pFile);
    fclose(pFile);

    return true;
}

//-----------------------------------------------------------------------------
//      バイナリラージオブジェクトを書き出します.
//-----------------------------------------------------------------------------
bool WriteBlobToFileW(IBlob* pBlob, const wchar_t* filename)
{
    FILE* pFile = nullptr;
    auto err = _wfopen_s(&pFile, filename, L"wb");
    if (err != 0 || pFile == nullptr)
    {
        ELOG("Error : WriteBlobToFileW() Failed. File open failed. path = %ls, errcode = 0x%x", filename, err);
        return false;
    }

    fwrite(pBlob->GetBuffer(), pBlob->GetBufferSize(), 1, pFile);
    fclose(pFile);

    return true;
}

} // namespace asdx

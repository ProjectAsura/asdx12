//-----------------------------------------------------------------------------
// File : BinaryData.cpp
// Desc : Binary Data.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <BinaryData.h>


//-----------------------------------------------------------------------------
//      バイナリをファイルに保存します.
//-----------------------------------------------------------------------------
bool SaveBinary(const char* path, const Binary& binary)
{
    FILE* fp = nullptr;
    errno_t err = fopen_s(&fp, path, "wb");
    if (err != 0 || fp == nullptr)
    {
        fprintf_s(stderr, "Error : Output File Open Failed. path = %s, errcode = 0x%x\n", path, err);
        return false;
    }

    fwrite(binary.data(), binary.size(), 1, fp);
    fclose(fp);
    return true;
}

//-----------------------------------------------------------------------------
//      バイナリにコピーします.
//-----------------------------------------------------------------------------
void CopyToBinary(const void* pSrc, size_t size, Binary& binary)
{
    binary.resize(size);
    memcpy(binary.data(), pSrc, size);
}

//-----------------------------------------------------------------------------
// File : LevelConverter.cpp
// Desc : Level Converter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdio>
#include <LevelConverter.h>
#include <simdjson.h>
#include <LevelBinary_generated.h>


#ifndef ELOG
#define ELOG(x, ...) fprintf_s(stderr, "[File:%s, Line:%d] " x "\n", __FILE__, __LINE__, ##__VA_ARGS__ )
#endif//ELOG

namespace {

//-----------------------------------------------------------------------------
// Constant values.
//-----------------------------------------------------------------------------
constexpr uint32_t CURRENT_VERSION = 1u;    //!< 現在サポートされているバージョン.

} // namespace


///////////////////////////////////////////////////////////////////////////////
// LevelConverter class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      現在のバイナリバージョンを取得します.
//-----------------------------------------------------------------------------
uint32_t LevelConverter::GetCurrentVersion()
{ return CURRENT_VERSION; }

//-----------------------------------------------------------------------------
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool LevelConverter::Convert(const Desc& desc)
{
    if (desc.InputPath.empty() || desc.OutputPath.empty())
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    std::vector<uint8_t> binary;
    if (!Convert(desc.InputPath, binary))
    {
        ELOG("Error : Convert Failed.");
        return false;
    }

    // バイナリファイルに出力.
    {
        FILE* fp = nullptr;
        auto err = fopen_s(&fp, desc.OutputPath.c_str(), "wb");
        if (err != 0)
        {
            ELOG("Error : Output File Open Failed. path = %s", desc.OutputPath.c_str());
            return false;
        }

        fwrite(binary.data(), binary.size(), 1, fp);
        fclose(fp);
    }

    return true;
}

//-----------------------------------------------------------------------------
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool LevelConverter::Convert(const std::string& path, std::vector<uint8_t>& binary)
{
    EditLevel level;
    if (!Load(path, level))
    {
        ELOG("Error : Load() Failed. path = %s", path.c_str());
        return false;
    }

    if (!Convert(level, binary))
    {
        ELOG("Error : Binary Convert Failed.");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool LevelConverter::Convert(const EditLevel& level, std::vector<uint8_t>& binary)
{

    return true;
}

//-----------------------------------------------------------------------------
//      逆変換処理を行います.
//-----------------------------------------------------------------------------
bool LevelConverter::ReverseConvert(const std::vector<uint8_t>& binary, EditLevel& level)
{

    return true;
}

//-----------------------------------------------------------------------------
//      逆変換処理を行います.
//-----------------------------------------------------------------------------
bool LevelConverter::ReverseConvert(const std::vector<uint8_t>& binary, const std::string& path)
{
    EditLevel level;
    if (!ReverseConvert(binary, level))
    {
        ELOG("Error : Reverse Convert Failed.");
        return false;
    }

    return Save(path, level);
}

//-----------------------------------------------------------------------------
//      jsonファイルに保存します.
//-----------------------------------------------------------------------------
bool LevelConverter::Save(const std::string& path, const EditLevel& level)
{
    return true;
}

//-----------------------------------------------------------------------------
//      jsonファイルから読み込みします.
//-----------------------------------------------------------------------------
bool LevelConverter::Load(const std::string& path, EditLevel& level)
{
    return true;
}
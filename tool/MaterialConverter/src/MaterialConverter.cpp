//----------------------------------------------------------------------------
// File : MaterialConverter.cpp
// Desc : Material Binary (*.mtb) Converter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdio>
#include <simdjson.h>
#include <filesystem>
#include <MaterialConverter.h>
#include <MaterialBinary_generated.h>


#ifndef ELOG
#define ELOG(x, ...) fprintf_s(stderr, "[File:%s, Line:%d] " x "\n", __FILE__, __LINE__, ##__VA_ARGS__ )
#endif//ELOG

namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t CURRENT_VERSION = 1u;  //!< 現在サポートされているバージョン.

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// MaterialConverter class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool MaterialConverter::Convert(const Desc& desc)
{
    return false;
}

//-----------------------------------------------------------------------------
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool MaterialConverter::Convert(const char* inputPath, std::vector<uint8_t>& binary)
{
    return false;
}

} // namespace asdx

//-----------------------------------------------------------------------------
// File : MaterialConverter.cpp
// Desc : Material Converter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdio>
#include <MaterialConverter.h>
#include <MaterialBinary_generated.h>


#ifndef ELOG
#define ELOG(x, ...) fprintf_s(stderr, "[File:%s, Line:%d] " x "\n", __FILE__, __LINE__, ##__VA_ARGS__ )
#endif//ELOG

namespace {


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
    if (desc.InputPath.empty() || desc.OutputPath.empty())
    {
        ELOG("Error : Invalid Arguments.");
        return false;
    }

    // TODO : Implementation.

    return false;
}

} // namespace asdx
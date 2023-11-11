//-----------------------------------------------------------------------------
// File : asdxBitCast.h
// Desc : Bit Cast.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

#if __cplusplus >= 202002L
#include <bit>
#else
#include <cassert>
#include <cstring>

//-----------------------------------------------------------------------------
//      ビット列を崩さずに型キャストします.
//-----------------------------------------------------------------------------
template<typename To, typename From>
inline To bit_cast(const From& value)
{
    // データサイズが一致することを確認.
    static_assert(sizeof(To) == sizeof(From));

    To ret = {};
    memcpy(&ret, &value, sizeof(ret));
    return ret;
}
#endif

//-----------------------------------------------------------------------------
// File : asdxHalf.h
// Desc : Half Float Data Type.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdint>
#include <fnd/asdxBitCast.h>


namespace asdx {

using half = uint16_t;  //!< 16ビット浮動小数型です.

//-----------------------------------------------------------------------------
//      16bit 浮動小数に変換します.
//-----------------------------------------------------------------------------
inline half ToHalf(float value)
{
    // ビット列を崩さないままuint32_t型に変換.
    auto bit = bit_cast<uint32_t>(value); 

    // float表現の符号ビットを取り出し.
    auto sign = (bit & 0x80000000u) >> 16u;

    // 符号部を削ぎ落す.
    bit &= 0x7FFFFFFFu;

    // halfとして表現する際に値が大きすぎる場合は，無限大にクランプする.
    if (bit > 0x47FFEFFFu)
    {
        half result = 0x7FFFu | sign;
        return result;
    }

    // 正規化されたhalfとして表現するために小さすぎる値は正規化されていない値に変換.
    if (bit < 0x38800000u)
    {
        uint32_t shift = 113u - (bit >> 23u);
        bit = (0x800000u | ( bit & 0x7FFFFFu)) >> shift;
    }
    else
    {
        // 正規化されたhalfとして表現するために指数部にバイアスを×.
        bit += 0xC8000000u;
    }

    // half型表現にする.
    half result = ((bit + 0x0FFFu + ((bit >> 13u) & 1u)) >> 13u) & 0x7FFFu;

    // 符号部を付け足す.
    result |= sign;

    return result;
}

//-----------------------------------------------------------------------------
//      32bit 浮動小数に変換します.
//-----------------------------------------------------------------------------
inline float ToFloat(half value)
{
    uint32_t mantissa = uint32_t(value) & 0x03FF;
    uint32_t exponent = 0;
    uint32_t result   = 0;

    // 正規化済みの場合.
    if ((value & 0x7C00) != 0)
    {
        // 指数部を計算.
        exponent = uint32_t((value >> 10) & 0x1F);
    }
    // 正規化されていない場合.
    else if (mantissa != 0)
    {
        // 結果となるfloatで値を正規化する.
        exponent = 1;

        do {
            exponent--;
            mantissa <<= 1;
        } while ((mantissa & 0x0400u) == 0);

        mantissa &= 0x03FF;
    }
    // 値がゼロの場合.
    else
    {
        // 指数部を計算.
        exponent = (uint32_t)-112;
    }

    result = ((value & 0x8000u) << 16) | // 符号部.
             ((exponent + 112)  << 23) | // 指数部.
             (mantissa << 13);           // 仮数部.

    return bit_cast<float>(result);
}

}// namespace asdx

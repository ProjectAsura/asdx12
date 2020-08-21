//-----------------------------------------------------------------------------
// File : asdxHalf.cpp
// Desc : Half Float.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxHalf.h>


namespace {

///////////////////////////////////////////////////////////////////////////////
// FP32 
///////////////////////////////////////////////////////////////////////////////
union FP32
{
    uint32_t    u;
    float       f;
    struct
    {
        uint32_t Mantissa   : 23;
        uint32_t Exponent   : 8;
        uint32_t Sign       : 1;
    };
};

///////////////////////////////////////////////////////////////////////////////
// FP16
///////////////////////////////////////////////////////////////////////////////
union FP16
{
    uint16_t u;
    struct
    {
        uint32_t Mantissa   : 10;
        uint32_t Exponent   : 5;
        uint32_t Sign       : 1;
    };
};

static const FP32       kMagic      = { 113 << 23 };
static const uint32_t   kShiftedExp = 0x7c00 << 13;

} // namespace


namespace asdx {

//-----------------------------------------------------------------------------
//      半精度浮動小数に変換します.
//-----------------------------------------------------------------------------
Half ToHalf(float value)
{
    FP32 f;
    f.f = value;

    FP16 o = { 0 };

    if (f.Exponent == 0)
    { o.Exponent = 0; }

    else if (f.Exponent == 255)
    {
        o.Exponent = 31;
        o.Mantissa = f.Mantissa ? 0x200 : 0;
    }
    else
    {
        int newexp = f.Exponent - 127 + 15;
        
        if (newexp >= 31)
        { o.Exponent = 31; }
 
        else if (newexp <= 0)
        {
            if ((14 - newexp) <= 24)
            {
                uint16_t mant = f.Mantissa | 0x800000;
                o.Mantissa = mant >> (14 - newexp);
                if ((mant >> (13 - newexp)) & 1)
                { o.u++; }
            }
        }
        else
        {
            o.Exponent = newexp;
            o.Mantissa = f.Mantissa >> 13;
            if (f.Mantissa & 0x1000)
            { o.u++; }
        }
    }

    o.Sign = f.Sign;
    return o.u;
}

//-----------------------------------------------------------------------------
//      単精度浮動小数に変換します.
//-----------------------------------------------------------------------------
float ToFloat(Half v)
{
    FP16 h;
    h.u = v;

    FP32 o;

    o.u = (h.u & 0x7fff) << 13;
    uint32_t exp = kShiftedExp & o.u;
    o.u += (127 - 15) << 23;

    if (exp == kShiftedExp)
    { o.u += (128 - 16) << 23; }
    else if (exp == 0)
    {
        o.u += 1 << 23;
        o.f -= kMagic.f;
    }

    o.u |= (h.u & 0x8000) << 16;
    return o.f;
}

//-----------------------------------------------------------------------------
//      半精度浮動小数に変換します.
//-----------------------------------------------------------------------------
Half2 EncodeHalf2(const Vector2& value)
{
    Half2 packed;
    packed.x = ToHalf(value.x);
    packed.u = ToHalf(value.y);
    return packed;
}

//-----------------------------------------------------------------------------
//      単精度浮動小数に変換します.
//-----------------------------------------------------------------------------
Vector2 DecodeHalf2(const Half2& value)
{
    Vector2 unpacked;
    unpacked.x = ToFloat(value.x);
    unpacked.y = ToFloat(value.y);
    return unpacked;
}

//-----------------------------------------------------------------------------
//      半精度浮動小数に変換します.
//-----------------------------------------------------------------------------
Half3 EncodeHalf3(const Vector3& value)
{
    Half3 packed;
    packed.x = ToHalf(value.x);
    packed.y = ToHalf(value.y);
    packed.z = ToHalf(value.z);
    return packed;
}

//-----------------------------------------------------------------------------
//      単精度浮動小数に変換します.
//-----------------------------------------------------------------------------
Vector3 DecodeHalf3(const Half3& value)
{
    Vector3 unpacked;
    unpacked.x = ToFloat(value.x);
    unpacked.y = ToFloat(value.y);
    unpacked.z = ToFloat(value.z);
    return unpacked;
}

//-----------------------------------------------------------------------------
//      半精度浮動小数に変換します.
//-----------------------------------------------------------------------------
Half4 EncodeHalf4(const Vector4& value)
{
    Half4 packed;
    packed.x = ToHalf(value.x);
    packed.y = ToHalf(value.y);
    packed.z = ToHalf(value.z);
    packed.w = ToHalf(value.w);
    return packed;
}

//-----------------------------------------------------------------------------
//      単精度浮動小数に変換します.
//-----------------------------------------------------------------------------
Vector4 DecodeHalf4(const Half4& value)
{
    Vector4 unpacked;
    unpacked.x = ToFloat(value.x);
    unpacked.y = ToFloat(value.y);
    unpacked.z = ToFloat(value.z);
    unpacked.w = ToFloat(value.w);
    return unpacked;
}

} // namespace asdx

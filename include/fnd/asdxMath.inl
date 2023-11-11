﻿//-----------------------------------------------------------------------------
// File : asdxMath.inl
// Desc : Math Module.
// Copyright(c) Project Asura All right reserved.
//-----------------------------------------------------------------------------
#pragma once

namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Functions
///////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      ラジアンに変換します.
//-----------------------------------------------------------------------------
constexpr float ToRadian( float degree ) noexcept
{ return degree * ( F_PI / 180.0f ); }

//-----------------------------------------------------------------------------
//      ラジアンに変換します.
//-----------------------------------------------------------------------------
constexpr double ToRadian( double degree ) noexcept
{ return degree * ( D_PI / 180.0 ); }

//-----------------------------------------------------------------------------
//      度に変換します.
//-----------------------------------------------------------------------------
constexpr float ToDegree( float radian ) noexcept
{ return radian * ( 180.0f / F_PI ); }

//-----------------------------------------------------------------------------
//      度に変換します.
//-----------------------------------------------------------------------------
constexpr double ToDegree( double radian ) noexcept
{ return radian * ( 180.0 / D_PI ); }

//-----------------------------------------------------------------------------
//      ゼロかどうかチェックします.
//-----------------------------------------------------------------------------
inline
bool IsZero( float value ) noexcept
{ return fabs( value ) <= F_EPSILON; }

//-----------------------------------------------------------------------------
//      ゼロかどうかチェックします.
//-----------------------------------------------------------------------------
inline
bool IsZero( double value ) noexcept
{ return abs( value ) <= D_EPSILON; }

//-----------------------------------------------------------------------------
//      値が等しいかどうかチェックします.
//-----------------------------------------------------------------------------
inline
bool IsEqual( float value1, float value2 ) noexcept
{ return fabs( value1 - value2 ) <= F_EPSILON; }

//-----------------------------------------------------------------------------
//      値が等しいかどうかチェックします.
//-----------------------------------------------------------------------------
inline
bool IsEqual( double value1, double value2 ) noexcept
{ return abs( value1 - value2 ) <= D_EPSILON; }

//-----------------------------------------------------------------------------
//      非数かどうかチェックします.
//-----------------------------------------------------------------------------
constexpr bool IsNan( float value ) noexcept
{ return ( value != value ); }

//-----------------------------------------------------------------------------
//      非数かどうかチェックします.
//-----------------------------------------------------------------------------
constexpr bool IsNan( double value ) noexcept
{ return ( value != value ); }

//-----------------------------------------------------------------------------
//      無限大かどうかチェックします.
//-----------------------------------------------------------------------------
inline
bool IsInf( float value )
{
    // ビット列に変換して，指数部がすべて 1 かどうかチェック.
    uint32_t u;
    memcpy(&u, &value, sizeof(u));
    return ((u & 0x7f800000) == 0x7f800000) && (value == value);
}

//-----------------------------------------------------------------------------
//      無限大かどうかチェックします.
//-----------------------------------------------------------------------------
inline
bool IsInf( double value )
{
    uint64_t l;
    memcpy(&l, &value, sizeof(l));
    return ((l & 0x7ff0000000000000) == 0x7ff0000000000000) && (value == value);
}

//-----------------------------------------------------------------------------
//      平方和の平方根を求めます.
//-----------------------------------------------------------------------------
inline
float Hypot( float x, float y )
{ return sqrtf( x * x + y * y ); }

//-----------------------------------------------------------------------------
//      平方和の平方根を求めます.
//-----------------------------------------------------------------------------
inline
float Hypot( float x, float y, float z )
{ return sqrtf( x * x + y * y + z * z ); }

//-----------------------------------------------------------------------------
//      平方和の平方根を求めます.
//-----------------------------------------------------------------------------
inline
float Hypot( float x, float y, float z, float w )
{ return sqrtf( x * x + y * y + z * z + w * w ); }

//-----------------------------------------------------------------------------
//      平方和の平方根を求めます.
//-----------------------------------------------------------------------------
inline
double Hypot( double x, double y )
{ return sqrt( x * x + y * y ); }

//-----------------------------------------------------------------------------
//      平方和の平方根を求めます.
//-----------------------------------------------------------------------------
inline
double Hypot( double x, double y, double z )
{ return sqrt( x * x + y * y + z * z ); }

//-----------------------------------------------------------------------------
//      平方和の平方根を求めます.
//-----------------------------------------------------------------------------
inline
double Hypot( double x, double y, double z, double w )
{ return sqrt( x * x + y * y + z * z + w * w ); }

//-----------------------------------------------------------------------------
//      階乗計算します.
//-----------------------------------------------------------------------------
inline
uint32_t Fact( uint32_t number )
{
    uint32_t result = 1;
    for( uint32_t i=1; i<=number; ++i )
    { result *= i; }
    return result;
}

//-----------------------------------------------------------------------------
//      2重階乗を計算します.
//-----------------------------------------------------------------------------
inline
uint32_t DblFact( uint32_t number )
{
    uint32_t result = 1;
    uint32_t start = ( ( number % 2 ) == 0 ) ? 2 : 1;
    for( uint32_t i=start; i<=number; i+=2 )
    { result *= i; }
    return result;
}

//-----------------------------------------------------------------------------
//      順列を計算します.
//-----------------------------------------------------------------------------
inline
uint32_t Perm( uint32_t n, uint32_t r )
{
    assert( n >= r );
    return Fact( n ) / Fact( n - r );
}

//-----------------------------------------------------------------------------
//      組み合わせを計算します.
//-----------------------------------------------------------------------------
inline
uint32_t Comb( uint32_t n, uint32_t r )
{
    assert( n >= r );
    return Fact( n ) / ( Fact( n - r ) * Fact( r ) );
}

//-----------------------------------------------------------------------------
//      線形補間を行います.
//-----------------------------------------------------------------------------
inline
constexpr float Lerp( float a, float b, float amount ) noexcept
{ return a + amount * ( b - a ); }

//-----------------------------------------------------------------------------
//      線形補間を行います.
//-----------------------------------------------------------------------------
inline
constexpr double Lerp( double a, double b, double amount ) noexcept
{ return a + amount * ( b - a ); }


inline
int8_t Wrap(int8_t x, int8_t low, int8_t high)
{
    assert(low < high);
    const int8_t n = (x - low) % (high - low);
    return (n >= 0) ? (n + low) : (n + high);
}

inline
int16_t Wrap(int16_t x, int16_t low, int16_t high)
{
    assert(low < high);
    const int16_t n = (x - low) % (high - low);
    return (n >= 0) ? (n + low) : (n + high);
}

inline
int32_t Wrap(int32_t x, int32_t low, int32_t high)
{
    assert(low < high);
    const int32_t n = (x - low) % (high - low);
    return (n >= 0) ? (n + low) : (n + high);
}

inline
int64_t Wrap(int64_t x, int64_t low, int64_t high)
{
    assert(low < high);
    const int64_t n = (x - low) % (high - low);
    return (n >= 0) ? (n + low) : (n + high);
}

inline
float Wrap(float x, float low, float high)
{
    assert(low < high);
    const float n = std::fmod(x - low, high - low);
    return (n >= 0) ? (n + low) : (n + high);
}

inline
double Wrap(double x, double low, double high)
{
    assert(low < high);
    const double n = std::fmod(x - low, high - low);
    return (n >= 0) ? (n + low) : (n + high);
}

inline
float Remap(float value, float inputMin, float inputMax, float outputMin, float outputMax)
{ return (value - inputMin) * ((outputMax - outputMin) / (inputMax - inputMin)) + outputMin; }

inline
double Remap(double value, double inputMin, double inputMax, double outputMin, double outputMax)
{ return (value - inputMin) * ((outputMax - outputMin) / (inputMax - inputMin)) + outputMin; }


///////////////////////////////////////////////////////////////////////////////////////////////////
// Vector2 structure
///////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
inline
Vector2::Vector2()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      引数付きコンストラクタ.
//-----------------------------------------------------------------------------
inline
Vector2::Vector2( const float* pf )
{
    assert( pf != nullptr );
    x = pf[ 0 ];
    y = pf[ 1 ];
}

//-----------------------------------------------------------------------------
//      引数付きコンストラクタ.
//-----------------------------------------------------------------------------
inline
Vector2::Vector2( float nx, float ny )
: x( nx )
, y( ny )
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      float*へのキャストです.
//-----------------------------------------------------------------------------
inline
Vector2::operator float *()
{ return static_cast<float*>( &x ); }

//-----------------------------------------------------------------------------
//      const float*へのキャストです.
//-----------------------------------------------------------------------------
inline
Vector2::operator const float *() const
{ return static_cast<const float*>( &x ); }

//-----------------------------------------------------------------------------
//      加算代入演算子です.
//-----------------------------------------------------------------------------
inline
Vector2& Vector2::operator += ( const Vector2& v )
{
    x += v.x;
    y += v.y;
    return (*this);
}

//-----------------------------------------------------------------------------
//      減算代入演算子です.
//-----------------------------------------------------------------------------
inline
Vector2& Vector2::operator -= ( const Vector2& v )
{
    x -= v.x;
    y -= v.y;
    return (*this);
}

//-----------------------------------------------------------------------------
//      乗算代入演算子です.
//-----------------------------------------------------------------------------
inline
Vector2& Vector2::operator *= ( float f )
{
    x *= f;
    y *= f;
    return (*this);
}

//-----------------------------------------------------------------------------
//      除算代入演算子です.
//-----------------------------------------------------------------------------
inline
Vector2& Vector2::operator /= ( float f )
{
    assert( !IsZero( f ) );
    x /= f;
    y /= f;
    return (*this);
}

//-----------------------------------------------------------------------------
//      代入演算子です.
//-----------------------------------------------------------------------------
inline
Vector2& Vector2::operator = ( const Vector2& value )
{
    x = value.x;
    y = value.y;
    return (*this);
}

//-----------------------------------------------------------------------------
//      正符号演算子です.
//-----------------------------------------------------------------------------
inline
Vector2 Vector2::operator + () const
{ return (*this); }

//-----------------------------------------------------------------------------
//      負符号演算子です.
//-----------------------------------------------------------------------------
inline
Vector2 Vector2::operator - () const
{ return Vector2( -x, -y ); }

//-----------------------------------------------------------------------------
//      加算演算子です.
//-----------------------------------------------------------------------------
inline
Vector2 Vector2::operator + ( const Vector2& v ) const
{ return Vector2( x + v.x, y + v.y ); }

//-----------------------------------------------------------------------------
//      減算演算子です.
//-----------------------------------------------------------------------------
inline
Vector2 Vector2::operator - ( const Vector2& v ) const
{ return Vector2( x - v.x, y - v.y ); }

//-----------------------------------------------------------------------------
//      乗算演算子です.
//-----------------------------------------------------------------------------
inline
Vector2 Vector2::operator * ( float f ) const
{ return Vector2( x * f, y * f ); }

//-----------------------------------------------------------------------------
//      除算演算子です.
//-----------------------------------------------------------------------------
inline
Vector2 Vector2::operator / ( float f ) const
{
    assert( !IsZero( f ) );
    return Vector2( x / f, y / f );
}

//-----------------------------------------------------------------------------
//      乗算演算子です.
//-----------------------------------------------------------------------------
inline
Vector2 operator * ( float f, const Vector2& v )
{ return Vector2( f * v.x, f * v.y ); }

//-----------------------------------------------------------------------------
//      等価比較演算子です.
//-----------------------------------------------------------------------------
inline
bool Vector2::operator == ( const Vector2& v ) const
{ 
    return IsEqual( x, v.x )
        && IsEqual( y, v.y );
}

//-----------------------------------------------------------------------------
//      非等価比較演算子です.
//-----------------------------------------------------------------------------
inline
bool Vector2::operator != ( const Vector2& v ) const
{
    return !IsEqual( x, v.x )
        || !IsEqual( y, v.y );
}

//-----------------------------------------------------------------------------
//      長さを求めます.
//-----------------------------------------------------------------------------
inline
float Vector2::Length() const
{ return Hypot(x, y); }

//-----------------------------------------------------------------------------
//      長さの2乗を求めます.
//-----------------------------------------------------------------------------
inline
float Vector2::LengthSq() const
{ return ( x * x + y * y ); }

//-----------------------------------------------------------------------------
//      正規化を行います.
//-----------------------------------------------------------------------------
inline
Vector2& Vector2::Normalize()
{
    auto mag = Length();
    assert( mag > 0.0f );
    x /= mag;
    y /= mag;
    return (*this);
}

//-----------------------------------------------------------------------------
//      ゼロ除算を考慮して正規化を行います.
//-----------------------------------------------------------------------------
inline
Vector2& Vector2::SafeNormalize( const Vector2& set )
{
    auto mag = Length();
    if ( mag > 0.0f )
    {
        x /= mag;
        y /= mag;
        return (*this);
    }

    x = set.x;
    y = set.y;
    return (*this);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Vector2 Methods
///////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      各成分の絶対値を求めます.
//-----------------------------------------------------------------------------
inline
Vector2 Vector2::Abs( const Vector2& value )
{ 
    return Vector2(
        fabs( value.x ), 
        fabs( value.y ) 
    );
}

//-----------------------------------------------------------------------------
//      各成分の絶対値を求めます.
//-----------------------------------------------------------------------------
inline
void Vector2::Abs( const Vector2 &value, Vector2 &result )
{ 
    result.x = fabs( value.x );
    result.y = fabs( value.y );
}

//-----------------------------------------------------------------------------
//      各成分の値を制限します.
//-----------------------------------------------------------------------------
inline
Vector2 Vector2::Clamp( const Vector2& value, const Vector2& a, const Vector2& b )
{
    return Vector2(
        asdx::Clamp( value.x, a.x, b.x ),
        asdx::Clamp( value.y, a.y, b.y )
    );
}

//-----------------------------------------------------------------------------
//      各成分の値を制限します.
//-----------------------------------------------------------------------------
inline
void Vector2::Clamp( const Vector2& value, const Vector2& a, const Vector2& b, Vector2 &result )
{
    result.x = asdx::Clamp( value.x, a.x, b.x );
    result.y = asdx::Clamp( value.y, a.y, b.y );
}

//-----------------------------------------------------------------------------
//      各成分の値を0～1に収めます.
//-----------------------------------------------------------------------------
inline
Vector2 Vector2::Saturate( const Vector2& value )
{
    return Vector2(
        asdx::Saturate( value.x ),
        asdx::Saturate( value.y )
    );
}

//-----------------------------------------------------------------------------
//      各成分の値を0～1に収めます.
//-----------------------------------------------------------------------------
inline
void Vector2::Saturate( const Vector2& value, Vector2& result )
{
    result.x = asdx::Saturate( value.x );
    result.y = asdx::Saturate( value.y );
}

//-----------------------------------------------------------------------------
//      2点間距離を求めます.
//-----------------------------------------------------------------------------
inline
float Vector2::Distance( const Vector2& a, const Vector2& b )
{ return Hypot(b.x - a.x, b.y - a.y); }

//-----------------------------------------------------------------------------
//      2点間距離を求めます.
//-----------------------------------------------------------------------------
inline
void Vector2::Distance( const Vector2 &a, const Vector2 &b, float &result )
{ result = Hypot(b.x - a.x, b.y - a.y); }

//-----------------------------------------------------------------------------
//      2点間距離の2乗値を求めます.
//-----------------------------------------------------------------------------
inline
float Vector2::DistanceSq( const Vector2& a, const Vector2& b )
{
    auto X = b.x - a.x;
    auto Y = b.y - a.y;
    return X * X + Y * Y;
}

//-----------------------------------------------------------------------------
//      2点間距離の2乗値を求めます.
//-----------------------------------------------------------------------------
inline
void Vector2::DistanceSq( const Vector2 &a, const Vector2 &b, float &result )
{
    auto X = b.x - a.x;
    auto Y = b.y - a.y;
    result = X * X + Y * Y;
}

//-----------------------------------------------------------------------------
//      内積を求めます.
//-----------------------------------------------------------------------------
inline
float Vector2::Dot( const Vector2& a, const Vector2& b )
{ return ( a.x * b.x + a.y * b.y ); }

//-----------------------------------------------------------------------------
//      内積を求めます.
//-----------------------------------------------------------------------------
inline
void Vector2::Dot( const Vector2 &a, const Vector2 &b, float &result )
{ result = a.x * b.x + a.y * b.y; }

//-----------------------------------------------------------------------------
//      正規化します.
//-----------------------------------------------------------------------------
inline
Vector2 Vector2::Normalize( const Vector2& value )
{
    auto mag = value.Length();
    assert( mag > 0.0f );
    return Vector2(
        value.x / mag,
        value.y / mag 
    );
}

//-----------------------------------------------------------------------------
//      正規化します.
//-----------------------------------------------------------------------------
inline
void Vector2::Normalize( const Vector2 &value, Vector2 &result )
{
    auto mag = value.Length();
    assert( mag > 0.0f );
    result.x = value.x / mag;
    result.y = value.y / mag;
}

//-----------------------------------------------------------------------------
//      零除算を考慮して正規化します.
//-----------------------------------------------------------------------------
inline
Vector2 Vector2::SafeNormalize( const Vector2& value, const Vector2& set )
{
    auto mag = value.Length();
    if ( mag > 0.0f )
    {
        return Vector2(
            value.x / mag,
            value.y / mag 
        );
    }

    return set;
}

//-----------------------------------------------------------------------------
//      零除算を考慮して正規化します.
//-----------------------------------------------------------------------------
inline
void Vector2::SafeNormalize( const Vector2& value, const Vector2& set, Vector2& result )
{
    auto mag = value.Length();
    if ( mag > 0.0f )
    {
        result.x = value.x / mag;
        result.y = value.y / mag;
    }
    else
    {
        result.x = set.x;
        result.y = set.y;
    }
}

//-----------------------------------------------------------------------------
//      交差角を求めます.
//-----------------------------------------------------------------------------
inline
float Vector2::ComputeCrossingAngle( const Vector2& a, const Vector2& b )
{
    auto d = a.Length() * b.Length();
    if ( d <= 0.0f )
    { return 0.0f; }

    auto c = Vector2::Dot( a, b ) / d;
    if ( c >= 1.0f ) 
    { return 0.0f; }

    if ( c <= -1.0f )
    { return F_PI; }

    return acosf( c );
}

//-----------------------------------------------------------------------------
//      交差角を求めます.
//-----------------------------------------------------------------------------
inline
void Vector2::ComputeCrossingAngle( const Vector2 &a, const Vector2 &b, float &result )
{
    auto d = a.Length() * b.Length();
    if ( d <= 0.0f )
    {
        result = 0.0f;
        return;
    }

    auto c = Vector2::Dot( a, b ) / d;
    if ( c >= 1.0f ) 
    {
        result = 0.0f;
        return;
    }

    if ( c <= -1.0f )
    {
        result = F_PI;
        return;
    }

    result = acosf( c );
}

//-----------------------------------------------------------------------------
//      各成分の最小値を求めます.
//-----------------------------------------------------------------------------
inline
Vector2 Vector2::Min( const Vector2& a, const Vector2& b )
{ 
    return Vector2(
        asdx::Min( a.x, b.x ),
        asdx::Min( a.y, b.y )
    );
}

//-----------------------------------------------------------------------------
//      各成分の最小値を求めます.
//-----------------------------------------------------------------------------
inline
void Vector2::Min( const Vector2 &a, const Vector2 &b, Vector2 &result )
{
    result.x = asdx::Min( a.x, b.x );
    result.y = asdx::Min( a.y, b.y );
}

//-----------------------------------------------------------------------------
//      各成分の最大値を求めます.
//-----------------------------------------------------------------------------
inline
Vector2 Vector2::Max( const Vector2& a, const Vector2& b )
{
    return Vector2(
        asdx::Max( a.x, b.x ),
        asdx::Max( a.y, b.y )
    );
}

//-----------------------------------------------------------------------------
//      各成分の最大値を求めます.
//-----------------------------------------------------------------------------
inline
void Vector2::Max( const Vector2 &a, const Vector2 &b, Vector2 &result )
{
    result.x = asdx::Max( a.x, b.x );
    result.y = asdx::Max( a.y, b.y );
}

//-----------------------------------------------------------------------------
//      反射ベクトルを求めます.
//-----------------------------------------------------------------------------
inline
Vector2 Vector2::Reflect( const Vector2& i, const Vector2& n )
{
    auto dot = n.x * i.x + n.y * i.y;
    return Vector2(
        i.x - ( 2.0f * n.x ) * dot,
        i.y - ( 2.0f * n.y ) * dot 
    );
}

//-----------------------------------------------------------------------------
//      反射ベクトルを求めます.
//-----------------------------------------------------------------------------
inline
void Vector2::Reflect( const Vector2 &i, const Vector2 &n, Vector2 &result )
{
    auto dot = n.x * i.x + n.y * i.y;
    result.x = i.x - ( 2.0f * n.x ) * dot;
    result.y = i.y - ( 2.0f * n.y ) * dot;
}

//-----------------------------------------------------------------------------
//      屈折ベクトルを求めます.
//-----------------------------------------------------------------------------
inline
Vector2 Vector2::Refract( const Vector2& i, const Vector2& n, float eta )
{
    auto cosi   = ( -i.x * n.x ) + ( -i.y * n.y );
    auto cost2  = 1.0f - eta * eta * ( 1.0f - cosi * cosi );
    auto sign   = Sign( cost2 );
    auto sqrtC2 = sqrtf( fabs( cost2 ) );
    auto coeff  = eta * cosi - sqrtC2;

    return Vector2(
        sign * ( eta * i.x + coeff * n.x ),
        sign * ( eta * i.y + coeff * n.y )
    );
}

//-----------------------------------------------------------------------------
//      屈折ベクトルを求めます.
//-----------------------------------------------------------------------------
inline
void Vector2::Refract( const Vector2 &i, const Vector2 &n, float eta, Vector2 &result )
{
    auto cosi   =  ( -i.x * n.x ) + ( -i.y * n.y );
    auto cost2  = 1.0f - eta * eta * ( 1.0f - cosi * cosi );
    auto sign   = Sign( cost2 );
    auto sqrtC2 = sqrtf( fabs( cost2 ) );
    auto coeff  = eta * cosi - sqrtC2;

    result.x = sign * ( eta * i.x + coeff * n.x );
    result.y = sign * ( eta * i.y + coeff * n.y );
}

//-----------------------------------------------------------------------------
//      重心座標を求めます.
//-----------------------------------------------------------------------------
inline
Vector2 Vector2::Barycentric
(
    const Vector2& a,
    const Vector2& b,
    const Vector2& c,
    float            f,
    float            g 
)
{
    return Vector2(
        a.x + f * ( b.x - a.x ) + g * ( c.x - a.x ),
        a.y + f * ( b.y - a.y ) + g * ( c.y - a.y )
    );
}

//-----------------------------------------------------------------------------
//      重心座標を求めます.
//-----------------------------------------------------------------------------
inline
void Vector2::Barycentric
(
    const Vector2&  a,
    const Vector2&  b,
    const Vector2&  c,
    float             f,
    float             g,
    Vector2&        result
)
{
    result.x = a.x + f * ( b.x - a.x ) + g * ( c.x - a.x );
    result.y = a.y + f * ( b.y - a.y ) + g * ( c.y - a.y );
}

//-----------------------------------------------------------------------------
//      エルミートスプライン補間を行います.
//-----------------------------------------------------------------------------
inline
Vector2 Vector2::Hermite
(
    const Vector2&  a,
    const Vector2&  t1,
    const Vector2&  b,
    const Vector2&  t2,
    float             amount
)
{
    auto c2 = amount * amount;
    auto c3 = c2 * amount;

    Vector2 result;
    if ( amount <= 0.0f )
    {
        result.x = a.x;
        result.y = a.y;
    }
    else if ( amount >= 1.0f )
    {
        result.x = b.x;
        result.y = b.y;
    }
    else
    {
        result.x = ( 2.0f * a.x - 2.0f * b.x + t2.x + t1.x ) * c3 + ( 3.0f * b.x - 3.0f * a.x - 2.0f * t1.x - t2.x ) * c3 + t1.x * amount + a.x;
        result.y = ( 2.0f * a.y - 2.0f * b.y + t2.y + t1.y ) * c3 + ( 3.0f * b.y - 3.0f * a.y - 2.0f * t1.y - t2.y ) * c3 + t1.y * amount + a.y;
    }
    return result;
}

//-----------------------------------------------------------------------------
//      エルミートスプライン補間を行います.
//-----------------------------------------------------------------------------
inline
void Vector2::Hermite
(
    const Vector2&  a,
    const Vector2&  t1,
    const Vector2&  b,
    const Vector2&  t2,
    float             amount,
    Vector2&        result
)
{
    auto c2 = amount * amount;
    auto c3 = c2 * amount;

    if ( amount <= 0.0f )
    {
        result.x = a.x;
        result.y = a.y;
    }
    else if ( amount >= 1.0f )
    {
        result.x = b.x;
        result.y = b.y;
    }
    else
    {
        result.x = ( 2.0f * a.x - 2.0f * b.x + t2.x + t1.x ) * c3 + ( 3.0f * b.x - 3.0f * a.x - 2.0f * t1.x - t2.x ) * c3 + t1.x * amount + a.x;
        result.y = ( 2.0f * a.y - 2.0f * b.y + t2.y + t1.y ) * c3 + ( 3.0f * b.y - 3.0f * a.y - 2.0f * t1.y - t2.y ) * c3 + t1.y * amount + a.y;
    }
}

//-----------------------------------------------------------------------------
//      Catmull-Rom スプライン補間を行います.
//-----------------------------------------------------------------------------
inline
Vector2 Vector2::CatmullRom
(
    const Vector2&  a,
    const Vector2&  b,
    const Vector2&  c,
    const Vector2&  d,
    float             amount
)
{
    auto c2 = amount * amount;
    auto c3 = c2 * amount;

    return Vector2(
        ( 0.5f * ( 2.0f * b.x + ( c.x - a.x ) * amount + ( 2.0f * a.x - 5.0f * b.x + 4.0f * c.x - d.x ) * c2 + ( 3.0f * b.x - a.x - 3.0f * c.x + d.x ) * c3 ) ),
        ( 0.5f * ( 2.0f * b.y + ( c.y - a.y ) * amount + ( 2.0f * a.y - 5.0f * b.y + 4.0f * c.y - d.y ) * c2 + ( 3.0f * b.y - a.y - 3.0f * c.y + d.y ) * c3 ) )
    );
}

//-----------------------------------------------------------------------------
//      Catmull-Rom スプライン補間を行います.
//-----------------------------------------------------------------------------
inline
void Vector2::CatmullRom
(
    const Vector2& a,
    const Vector2& b,
    const Vector2& c,
    const Vector2& d,
    float            amount,
    Vector2&       result
)
{
    auto c2 = amount * amount;
    auto c3 = c2 * amount;

    result.x = ( 0.5f * ( 2.0f * b.x + ( c.x - a.x ) * amount + ( 2.0f * a.x - 5.0f * b.x + 4.0f * c.x - d.x ) * c2 + ( 3.0f * b.x - a.x - 3.0f * c.x + d.x ) * c3 ) );
    result.y = ( 0.5f * ( 2.0f * b.y + ( c.y - a.y ) * amount + ( 2.0f * a.y - 5.0f * b.y + 4.0f * c.y - d.y ) * c2 + ( 3.0f * b.y - a.y - 3.0f * c.y + d.y ) * c3 ) );
}

//-----------------------------------------------------------------------------
//      線形補間を行います.
//-----------------------------------------------------------------------------
inline
Vector2 Vector2::Lerp( const Vector2& a, const Vector2& b, float amount )
{
    return Vector2(
        a.x + amount * ( b.x - a.x ),
        a.y + amount * ( b.y - a.y )
    );
}

//-----------------------------------------------------------------------------
//      線形補間を行います.
//-----------------------------------------------------------------------------
inline
void Vector2::Lerp( const Vector2 &a, const Vector2 &b, float amount, Vector2 &result )
{
    result.x = a.x + amount * ( b.x - a.x );
    result.y = a.y + amount * ( b.y - a.y );
}

//-----------------------------------------------------------------------------
//      3次方程式を用いて，2つの値の間を補間します
//-----------------------------------------------------------------------------
inline
Vector2 Vector2::SmoothStep( const Vector2& a, const Vector2& b, float amount )
{
    auto s = asdx::Clamp( amount, 0.0f, 1.0f );
    auto u = ( s * s ) + ( 3.0f - ( 2.0f * s ) );
    return Vector2(
        a.x + u * ( b.x - a.x ),
        a.y + u * ( b.y - a.y )
    );
}

//-----------------------------------------------------------------------------
//      3次方程式を用いて，2つの値の間を補間します
//-----------------------------------------------------------------------------
inline
void Vector2::SmoothStep( const Vector2 &a, const Vector2 &b, float t, Vector2 &result )
{
    auto s = asdx::Clamp( t, 0.0f, 1.0f );
    auto u = ( s * s ) + ( 3.0f - ( 2.0f * s ) );
    result.x = a.x + u * ( b.x - a.x );
    result.y = a.y + u * ( b.y - a.y );
}

//-----------------------------------------------------------------------------
//      指定された行列を用いて，ベクトルを変換します.
//-----------------------------------------------------------------------------
inline
Vector2 Vector2::Transform( const Vector2& position, const Matrix& matrix )
{
    return Vector2(
        ((position.x * matrix._11) + (position.y * matrix._21)) + matrix._41,
        ((position.x * matrix._12) + (position.y * matrix._22)) + matrix._42 );
}

//-----------------------------------------------------------------------------
//      指定された行列を用いて，ベクトルを変換します.
//-----------------------------------------------------------------------------
inline
void Vector2::Transform( const Vector2 &position, const Matrix &matrix, Vector2 &result )
{
    result.x = ((position.x * matrix._11) + (position.y * matrix._21)) + matrix._41;
    result.y = ((position.x * matrix._12) + (position.y * matrix._22)) + matrix._42;
}

//-----------------------------------------------------------------------------
//      指定された行列を用いて，法線ベクトルを変換します.
//-----------------------------------------------------------------------------
inline
Vector2 Vector2::TransformNormal( const Vector2& normal, const Matrix& matrix )
{
    return Vector2(
        (normal.x * matrix._11) + (normal.y * matrix._21),
        (normal.x * matrix._12) + (normal.y * matrix._22) );
}

//-----------------------------------------------------------------------------
//      指定された行列を用いて，法線ベクトルを変換します.
//-----------------------------------------------------------------------------
inline
void Vector2::TransformNormal( const Vector2 &normal, const Matrix &matrix, Vector2 &result )
{
    result.x = (normal.x * matrix._11) + (normal.y * matrix._21);
    result.y = (normal.x * matrix._12) + (normal.y * matrix._22);
}

//-----------------------------------------------------------------------------
//      指定された行列を用いてベクトルを変換し，変換結果をw=1に射影します.
//-----------------------------------------------------------------------------
inline
Vector2 Vector2::TransformCoord( const Vector2& coords, const Matrix& matrix )
{
    auto X = ( ( ((coords.x * matrix._11) + (coords.y * matrix._21)) ) + matrix._41);
    auto Y = ( ( ((coords.x * matrix._12) + (coords.y * matrix._22)) ) + matrix._42);
    auto W = ( ( ((coords.x * matrix._14) + (coords.y * matrix._24)) ) + matrix._44);
    return Vector2(
        X / W,
        Y / W 
    );
}

//-----------------------------------------------------------------------------
//      指定された行列を用いてベクトルを変換し，変換結果をw=1に射影します.
//-----------------------------------------------------------------------------
inline
void Vector2::TransformCoord( const Vector2 &coords, const Matrix &matrix, Vector2 &result )
{
    auto X = ( ( ((coords.x * matrix._11) + (coords.y * matrix._21)) ) + matrix._41);
    auto Y = ( ( ((coords.x * matrix._12) + (coords.y * matrix._22)) ) + matrix._42);
    auto W = ( ( ((coords.x * matrix._14) + (coords.y * matrix._24)) ) + matrix._44);

    result.x = X / W;
    result.y = Y / W;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Vector3 structure
///////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
inline
Vector3::Vector3()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
inline
Vector3::Vector3( const float* pf )
{
    assert( pf != nullptr );
    x = pf[ 0 ];
    y = pf[ 1 ];
    z = pf[ 2 ];
}

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
inline
Vector3::Vector3( const Vector2& value, float nz )
: x( value.x )
, y( value.y )
, z( nz )
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
inline
Vector3::Vector3( float nx, float ny, float nz )
: x( nx )
, y( ny )
, z( nz )
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      float* 型へのキャストです.
//-----------------------------------------------------------------------------
inline
Vector3::operator float *()
{ return static_cast<float*>( &x ); }

//-----------------------------------------------------------------------------
//      const float* 型へのキャストです.
//-----------------------------------------------------------------------------
inline
Vector3::operator const float *() const
{ return static_cast<const float*>( &x ); }

//-----------------------------------------------------------------------------
//      加算代入演算子です.
//-----------------------------------------------------------------------------
inline
Vector3& Vector3::operator += ( const Vector3& v )
{
    x += v.x;
    y += v.y;
    z += v.z;
    return (*this);
}

//-----------------------------------------------------------------------------
//      減算代入演算子です.
//-----------------------------------------------------------------------------
inline
Vector3& Vector3::operator -= ( const Vector3& v )
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return (*this);
}

//-----------------------------------------------------------------------------
//      乗算代入演算子です.
//-----------------------------------------------------------------------------
inline
Vector3& Vector3::operator *= ( float f )
{
    x *= f;
    y *= f;
    z *= f;
    return (*this);
}

//-----------------------------------------------------------------------------
//      除算代入演算子です.
//-----------------------------------------------------------------------------
inline
Vector3& Vector3::operator /= ( float f )
{
    assert( !IsZero( f ) );
    x /= f;
    y /= f;
    z /= f;
    return (*this);
}

//-----------------------------------------------------------------------------
//      代入演算子です.
//-----------------------------------------------------------------------------
inline
Vector3& Vector3::operator = ( const Vector3& value )
{
    x = value.x;
    y = value.y;
    z = value.z;
    return (*this);
}

//-----------------------------------------------------------------------------
//      正符号演算子です.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::operator + () const
{ return (*this); }

//-----------------------------------------------------------------------------
//      負符号演算子です.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::operator - () const
{ return Vector3( -x, -y, -z ); }

//-----------------------------------------------------------------------------
//      加算演算子です.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::operator + ( const Vector3& v ) const
{ return Vector3( x + v.x, y + v.y, z + v.z ); }

//-----------------------------------------------------------------------------
//      減算演算子です.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::operator - ( const Vector3& v ) const
{ return Vector3( x - v.x, y - v.y, z - v.z ); }

//-----------------------------------------------------------------------------
//      乗算演算子です.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::operator * ( float f ) const
{ return Vector3( x * f, y * f, z * f ); }

//-----------------------------------------------------------------------------
//      除算演算子です.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::operator / ( float f ) const
{
    assert( !IsZero( f ) );
    return Vector3( x / f, y / f, z / f );
}

//-----------------------------------------------------------------------------
//      乗算演算子です.
//-----------------------------------------------------------------------------
inline
Vector3 operator * ( float f, const Vector3& v )
{ return Vector3( f * v.x, f * v.y, f * v.z ); }

//-----------------------------------------------------------------------------
//      等価比較演算子です.
//-----------------------------------------------------------------------------
inline
bool Vector3::operator == ( const Vector3& v ) const
{
    return IsEqual( x, v.x )
        && IsEqual( y, v.y )
        && IsEqual( z, v.z );
}

//-----------------------------------------------------------------------------
//      非等価比較演算子です.
//-----------------------------------------------------------------------------
inline
bool Vector3::operator != ( const Vector3& v ) const
{ 
    return !IsEqual( x, v.x )
        || !IsEqual( y, v.y )
        || !IsEqual( z, v.z );
}

//-----------------------------------------------------------------------------
//      ベクトルの大きさを求めます.
//-----------------------------------------------------------------------------
inline
float Vector3::Length() const
{ return Hypot( x, y, z ); }

//-----------------------------------------------------------------------------
//      ベクトルの大きさの2乗値を求めます.
//-----------------------------------------------------------------------------
inline
float Vector3::LengthSq() const
{ return ( x * x + y * y + z * z); }

//-----------------------------------------------------------------------------
//      正規化を行います.
//-----------------------------------------------------------------------------
inline
Vector3& Vector3::Normalize()
{
    auto mag = Length();
    assert( mag > 0.0f );
    x /= mag;
    y /= mag;
    z /= mag;
    return (*this);
}

//-----------------------------------------------------------------------------
//      零除算を考慮して正規化します.
//-----------------------------------------------------------------------------
inline
Vector3& Vector3::SafeNormalize( const Vector3& set )
{
    auto mag = Length();
    if ( mag > 0.0f )
    {
        x /= mag;
        y /= mag;
        z /= mag;
        return (*this);
    }

    x = set.x;
    y = set.y;
    z = set.z;
    return (*this);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Vector3 methods
///////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      各成分の絶対値を求めます.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::Abs( const Vector3& v )
{ 
    return Vector3(
        fabs( v.x ),
        fabs( v.y ),
        fabs( v.z ) 
     );
}

//-----------------------------------------------------------------------------
//      各成分の絶対値を求めます.
//-----------------------------------------------------------------------------
inline
void Vector3::Abs( const Vector3 &value, Vector3 &result )
{ 
    result.x = fabs( value.x );
    result.y = fabs( value.y );
    result.z = fabs( value.z );
}

//-----------------------------------------------------------------------------
//      各成分の値を制限します.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::Clamp( const Vector3& value, const Vector3& a, const Vector3& b )
{
    return Vector3( 
        asdx::Clamp( value.x, a.x, b.x ),
        asdx::Clamp( value.y, a.y, b.y ),
        asdx::Clamp( value.z, a.z, b.z )
    );
}

//-----------------------------------------------------------------------------
//      各成分の値を制限します.
//-----------------------------------------------------------------------------
inline
void Vector3::Clamp( const Vector3 &value, const Vector3 &a, const Vector3 &b, Vector3 &result )
{
    result.x = asdx::Clamp( value.x, a.x, b.x );
    result.y = asdx::Clamp( value.y, a.y, b.y );
    result.z = asdx::Clamp( value.z, a.z, b.z );
}

//-----------------------------------------------------------------------------
//      各成分の値を0～1に収めます.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::Saturate( const Vector3& value )
{
    return Vector3(
        asdx::Saturate( value.x ),
        asdx::Saturate( value.y ),
        asdx::Saturate( value.z )
    );
}

//-----------------------------------------------------------------------------
//      各成分の値を0～1に収めます.
//-----------------------------------------------------------------------------
inline
void Vector3::Saturate( const Vector3& value, Vector3& result )
{
    result.x = asdx::Saturate( value.x );
    result.y = asdx::Saturate( value.y );
    result.z = asdx::Saturate( value.z );
}

//-----------------------------------------------------------------------------
//      2点間距離を求めます.
//-----------------------------------------------------------------------------
inline
float Vector3::Distance( const Vector3& a, const Vector3& b )
{ return Hypot( b.x - a.x, b.y - a.y, b.z - a.z ); }

//-----------------------------------------------------------------------------
//      2点間距離を求めます.
//-----------------------------------------------------------------------------
inline
void Vector3::Distance( const Vector3 &a, const Vector3 &b, float &result )
{ result = Hypot( b.x - a.x, b.y - a.y, b.z - a.z ); }

//-----------------------------------------------------------------------------
//      2点間距離の2乗値を求めます.
//-----------------------------------------------------------------------------
inline
float Vector3::DistanceSq( const Vector3& a, const Vector3& b )
{
    auto X = b.x - a.x;
    auto Y = b.y - a.y;
    auto Z = b.z - a.z;
    return X * X + Y * Y + Z * Z;
}

//-----------------------------------------------------------------------------
//      2点間距離の2乗値を求めます.
//-----------------------------------------------------------------------------
inline
void Vector3::DistanceSq( const Vector3 &a, const Vector3 &b, float &result )
{
    auto X = b.x - a.x;
    auto Y = b.y - a.y;
    auto Z = b.z - a.z;
    result = X * X + Y * Y + Z * Z;
}

//-----------------------------------------------------------------------------
//      内積を求めます.
//-----------------------------------------------------------------------------
inline
float Vector3::Dot( const Vector3& a, const Vector3& b )
{ return ( a.x * b.x + a.y * b.y + a.z * b.z ); }

//-----------------------------------------------------------------------------
//      内積を求めます.
//-----------------------------------------------------------------------------
inline
void Vector3::Dot( const Vector3 &a, const Vector3 &b, float &result )
{ result = a.x * b.x + a.y * b.y + a.z * b.z; }

//-----------------------------------------------------------------------------
//      外積を求めます.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::Cross( const Vector3& a, const Vector3& b )
{
    return Vector3( 
        ( a.y * b.z ) - ( a.z * b.y ),
        ( a.z * b.x ) - ( a.x * b.z ),
        ( a.x * b.y ) - ( a.y * b.x )
    );
}

//-----------------------------------------------------------------------------
//      外積を求めます.
//-----------------------------------------------------------------------------
inline
void Vector3::Cross( const Vector3 &a, const Vector3 &b, Vector3 &result )
{
    result.x = ( a.y * b.z ) - ( a.z * b.y );
    result.y = ( a.z * b.x ) - ( a.x * b.z );
    result.z = ( a.x * b.y ) - ( a.y * b.x );
}

//-----------------------------------------------------------------------------
//      正規化を行います.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::Normalize( const Vector3& value )
{
    auto mag = value.Length();
    assert( mag > 0.0f );
    return Vector3(
        value.x / mag,
        value.y / mag,
        value.z / mag 
    );
}

//-----------------------------------------------------------------------------
//      正規化を行います.
//-----------------------------------------------------------------------------
inline
void Vector3::Normalize( const Vector3& value, Vector3 &result )
{
    auto mag = value.Length();
    assert( mag > 0.0f );
    result.x = value.x / mag;
    result.y = value.y / mag;
    result.z = value.z / mag;
}

//-----------------------------------------------------------------------------
//      零除算を考慮して正規化を行います.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::SafeNormalize( const Vector3& value, const Vector3& set )
{
    auto mag = value.Length();
    if ( mag > 0.0f )
    {
        return Vector3(
            value.x / mag,
            value.y / mag,
            value.z / mag
        );
    }

    return set;
}

//-----------------------------------------------------------------------------
//      零除算を考慮して正規化を行います.
//-----------------------------------------------------------------------------
inline
void Vector3::SafeNormalize( const Vector3& value, const Vector3& set, Vector3& result )
{
    auto mag = value.Length();
    if ( mag > 0.0f )
    {
        result.x = value.x / mag;
        result.y = value.y / mag;
        result.z = value.z / mag;
    }
    else
    {
        result.x = set.x;
        result.y = set.y;
        result.z = set.z;
    }
}

//-----------------------------------------------------------------------------
//      三角形の面法線を求めます.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::ComputeNormal( const Vector3& p1, const Vector3& p2, const Vector3& p3 )
{
    auto v1 = p2 - p1;
    auto v2 = p3 - p1;
    auto result = Vector3::Cross( v1, v2 );
    return result.Normalize();
}

//-----------------------------------------------------------------------------
//      三角形の面法線を求めます.
//-----------------------------------------------------------------------------
inline
void Vector3::ComputeNormal( const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, Vector3 &result )
{
    auto v1 = p2 - p1;
    auto v2 = p3 - p1;
    Vector3::Cross( v1, v2, result );
    result.Normalize();
}

//-----------------------------------------------------------------------------
//      四角形の面法線を求めます.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::ComputeQuadNormal
(
    const Vector3& p1,
    const Vector3& p2,
    const Vector3& p3,
    const Vector3& p4
)
{
    Vector3 result;
    auto n1a = Vector3::ComputeNormal( p1, p2, p3 );
    auto n1b = Vector3::ComputeNormal( p1, p3, p4 );
    auto n2a = Vector3::ComputeNormal( p2, p3, p4 );
    auto n2b = Vector3::ComputeNormal( p2, p4, p1 );
    if ( Vector3::Dot( n1a, n1b ) > Vector3::Dot( n2a, n2b ) )
    {
        result = n1a + n1b;
        result.Normalize();
    }
    else
    {
        result = n2a + n2b;
        result.Normalize();
    }
    return result;
}

//-----------------------------------------------------------------------------
//      四角形の面法線を求めます.
//-----------------------------------------------------------------------------
inline
void Vector3::ComputeQuadNormal
(
    const Vector3 &p1,
    const Vector3 &p2,
    const Vector3 &p3,
    const Vector3 &p4,
    Vector3 &result
)
{
    auto n1a = Vector3::ComputeNormal( p1, p2, p3 );
    auto n1b = Vector3::ComputeNormal( p1, p3, p4 );
    auto n2a = Vector3::ComputeNormal( p2, p3, p4 );
    auto n2b = Vector3::ComputeNormal( p2, p4, p1 );
    if ( Vector3::Dot( n1a, n1b ) > Vector3::Dot( n2a, n2b ) )
    {
        result = n1a + n1b;
        result.Normalize();
    }
    else
    {
        result = n2a + n2b;
        result.Normalize();
    }
}

//-----------------------------------------------------------------------------
//      交差角を求めます.
//-----------------------------------------------------------------------------
inline
float Vector3::ComputeCrossingAngle( const Vector3& a, const Vector3& b )
{
    auto d = a.Length() * b.Length();
    if ( d <= 0.0f ) 
    { return 0.0f; }

    auto c = Vector3::Dot( a, b ) / d;
    if ( c >= 1.0f )
    { return 0.0f; }

    if ( c <= -1.0f )
    { return F_PI; }

    return acosf( c );
}

//-----------------------------------------------------------------------------
//      交差角を求めます.
//-----------------------------------------------------------------------------
inline
void Vector3::ComputeCrossingAngle( const Vector3 &a, const Vector3 &b, float &result )
{
    auto d = a.Length() * b.Length();
    if ( d <= 0.0f )
    {
        result = 0.0f;
        return;
    }

    auto c = Vector3::Dot( a, b ) / d;
    if ( c >= 1.0f ) 
    {
        result = 0.0f;
        return;
    }

    if ( c <= -1.0f )
    {
        result = F_PI;
        return;
    }

    result = acosf( c );
}

//-----------------------------------------------------------------------------
//      各成分の最小値を求めます.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::Min( const Vector3& a, const Vector3& b )
{ 
    return Vector3( 
        asdx::Min( a.x, b.x ),
        asdx::Min( a.y, b.y ),
        asdx::Min( a.z, b.z )
    );
}

//-----------------------------------------------------------------------------
//      各成分の最小値を求めます.
//-----------------------------------------------------------------------------
inline
void Vector3::Min( const Vector3 &a, const Vector3 &b, Vector3 &result )
{
    result.x = asdx::Min( a.x, b.x );
    result.y = asdx::Min( a.y, b.y );
    result.z = asdx::Min( a.z, b.z );
}

//-----------------------------------------------------------------------------
//      各成分の最大値を求めます.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::Max( const Vector3& a, const Vector3& b )
{
    return Vector3(
        asdx::Max( a.x, b.x ),
        asdx::Max( a.y, b.y ),
        asdx::Max( a.z, b.z )
    );
}

//-----------------------------------------------------------------------------
//      各成分の最大値を求めます.
//-----------------------------------------------------------------------------
inline
void Vector3::Max( const Vector3 &a, const Vector3 &b, Vector3 &result )
{
    result.x = asdx::Max( a.x, b.x );
    result.y = asdx::Max( a.y, b.y );
    result.z = asdx::Max( a.z, b.z );
}

//-----------------------------------------------------------------------------
//      反射ベクトルを求めます.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::Reflect( const Vector3& i, const Vector3& n )
{
    auto dot = n.x * i.x + n.y * i.y + n.z * i.z;
    return Vector3(
        i.x - ( 2.0f * n.x ) * dot,
        i.y - ( 2.0f * n.y ) * dot,
        i.z - ( 2.0f * n.z ) * dot
    );
}

//-----------------------------------------------------------------------------
//      反射ベクトルを求めます.
//-----------------------------------------------------------------------------
inline
void Vector3::Reflect( const Vector3 &i, const Vector3 &n, Vector3 &result )
{
    auto dot = n.x * i.x + n.y * i.y + n.z * i.z;
    result.x = i.x - ( 2.0f * n.x ) * dot;
    result.y = i.y - ( 2.0f * n.y ) * dot;
    result.z = i.z - ( 2.0f * n.z ) * dot;
}

//-----------------------------------------------------------------------------
//      屈折ベクトルを求めます.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::Refract( const Vector3& i, const Vector3& n, float eta )
{
    auto cosi   = ( -i.x * n.x ) + ( -i.y * n.y ) + ( -i.z * n.z );
    auto cost2  = 1.0f - eta * eta * ( 1.0f - cosi * cosi );
    auto sign   = Sign( cost2 );
    auto sqrtC2 = sqrtf( fabs( cost2 ) );
    auto coeff  = eta * cosi - sqrtC2;

    return Vector3(
        sign * ( eta * i.x + coeff * n.x ),
        sign * ( eta * i.y + coeff * n.y ),
        sign * ( eta * i.z + coeff * n.z )
    );
}

//-----------------------------------------------------------------------------
//      屈折ベクトルを求めます.
//-----------------------------------------------------------------------------
inline
void Vector3::Refract( const Vector3 &i, const Vector3 &n, float eta, Vector3 &result )
{
    auto cosi   =  ( -i.x * n.x ) + ( -i.y * n.y ) + ( -i.z * n.z );
    auto cost2  = 1.0f - eta * eta * ( 1.0f - cosi * cosi );
    auto sign   = Sign( cost2 );
    auto sqrtC2 = sqrtf( fabs( cost2 ) );
    auto coeff  = eta * cosi - sqrtC2;

    result.x = sign * ( eta * i.x + coeff * n.x );
    result.y = sign * ( eta * i.y + coeff * n.y );
    result.z = sign * ( eta * i.z + coeff * n.z );
}

//-----------------------------------------------------------------------------
//      重心座標を求めます.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::Barycentric
(
    const Vector3&  a,
    const Vector3&  b,
    const Vector3&  c,
    float             f,
    float             g
)
{
    return Vector3(
        a.x + f * ( b.x - a.x ) + g * ( c.x - a.x ),
        a.y + f * ( b.y - a.y ) + g * ( c.y - a.y ),
        a.z + f * ( b.z - a.z ) + g * ( c.z - a.z )
    );
}

//-----------------------------------------------------------------------------
//      重心座標を求めます.
//-----------------------------------------------------------------------------
inline
void Vector3::Barycentric
(
    const Vector3&  a,
    const Vector3&  b,
    const Vector3&  c,
    float             f,
    float             g,
    Vector3&        result
)
{
    result.x = a.x + f * ( b.x - a.x ) + g * ( c.x - a.x );
    result.y = a.y + f * ( b.y - a.y ) + g * ( c.y - a.y );
    result.z = a.z + f * ( b.z - a.z ) + g * ( c.z - a.z );
}

//-----------------------------------------------------------------------------
//      エルミートスプライン補間を行います.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::Hermite
(
    const Vector3&  a,
    const Vector3&  t1,
    const Vector3&  b,
    const Vector3&  t2,
    float             amount
)
{
    auto c2 = amount * amount;
    auto c3 = c2 * amount;

    Vector3 result;
    if ( amount <= 0.0f )
    {
        result.x = a.x;
        result.y = a.y;
        result.z = a.z;
    }
    else if ( amount >= 1.0f )
    {
        result.x = b.x;
        result.y = b.y;
        result.z = b.z;
    }
    else
    {
        result.x = ( 2.0f * a.x - 2.0f * b.x + t2.x + t1.x ) * c3 + ( 3.0f * b.x - 3.0f * a.x - 2.0f * t1.x - t2.x ) * c3 + t1.x * amount + a.x;
        result.y = ( 2.0f * a.y - 2.0f * b.y + t2.y + t1.y ) * c3 + ( 3.0f * b.y - 3.0f * a.y - 2.0f * t1.y - t2.y ) * c3 + t1.y * amount + a.y;
        result.z = ( 2.0f * a.z - 2.0f * b.z + t2.z + t1.z ) * c3 + ( 3.0f * b.z - 3.0f * a.z - 2.0f * t1.z - t2.z ) * c3 + t1.y * amount + a.z;
    }
    return result;
}

//-----------------------------------------------------------------------------
//      エルミートスプライン補間を行います.
//-----------------------------------------------------------------------------
inline
void Vector3::Hermite
(
    const Vector3&  a,
    const Vector3&  t1,
    const Vector3&  b,
    const Vector3&  t2,
    float             amount,
    Vector3&        result
)
{
    auto c2 = amount * amount;
    auto c3 = c2 * amount;

    if ( amount <= 0.0f )
    {
        result.x = a.x;
        result.y = a.y;
        result.z = a.z;
    }
    else if ( amount >= 1.0f )
    {
        result.x = b.x;
        result.y = b.y;
        result.z = b.z;
    }
    else
    {
        result.x = ( 2.0f * a.x - 2.0f * b.x + t2.x + t1.x ) * c3 + ( 3.0f * b.x - 3.0f * a.x - 2.0f * t1.x - t2.x ) * c3 + t1.x * amount + a.x;
        result.y = ( 2.0f * a.y - 2.0f * b.y + t2.y + t1.y ) * c3 + ( 3.0f * b.y - 3.0f * a.y - 2.0f * t1.y - t2.y ) * c3 + t1.y * amount + a.y;
        result.z = ( 2.0f * a.z - 2.0f * b.z + t2.z + t1.z ) * c3 + ( 3.0f * b.z - 3.0f * a.z - 2.0f * t1.z - t2.z ) * c3 + t1.z * amount + a.z;
    }
}

//-----------------------------------------------------------------------------
//      Catmull-Rom スプライン補間を行います.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::CatmullRom
(
    const Vector3&  a,
    const Vector3&  b,
    const Vector3&  c,
    const Vector3&  d,
    float             amount
)
{
    auto c2 = amount * amount;
    auto c3 = c2 * amount;

    return Vector3(
        ( 0.5f * ( 2.0f * b.x + ( c.x - a.x ) * amount + ( 2.0f * a.x - 5.0f * b.x + 4.0f * c.x - d.x ) * c2 + ( 3.0f * b.x - a.x - 3.0f * c.x + d.x ) * c3 ) ),
        ( 0.5f * ( 2.0f * b.y + ( c.y - a.y ) * amount + ( 2.0f * a.y - 5.0f * b.y + 4.0f * c.y - d.y ) * c2 + ( 3.0f * b.y - a.y - 3.0f * c.y + d.y ) * c3 ) ),
        ( 0.5f * ( 2.0f * b.z + ( c.z - a.z ) * amount + ( 2.0f * a.z - 5.0f * b.z + 4.0f * c.z - d.z ) * c2 + ( 3.0f * b.z - a.z - 3.0f * c.z + d.z ) * c3 ) )
    );
}

//-----------------------------------------------------------------------------
//      Catmull-Rom スプライン補間を行います.
//-----------------------------------------------------------------------------
inline
void Vector3::CatmullRom
(
    const Vector3&  a,
    const Vector3&  b,
    const Vector3&  c,
    const Vector3&  d,
    float             amount,
    Vector3&        result
)
{
    auto c2 = amount * amount;
    auto c3 = c2 * amount;

    result.x = ( 0.5f * ( 2.0f * b.x + ( c.x - a.x ) * amount + ( 2.0f * a.x - 5.0f * b.x + 4.0f * c.x - d.x ) * c2 + ( 3.0f * b.x - a.x - 3.0f * c.x + d.x ) * c3 ) );
    result.y = ( 0.5f * ( 2.0f * b.y + ( c.y - a.y ) * amount + ( 2.0f * a.y - 5.0f * b.y + 4.0f * c.y - d.y ) * c2 + ( 3.0f * b.y - a.y - 3.0f * c.y + d.y ) * c3 ) );
    result.z = ( 0.5f * ( 2.0f * b.z + ( c.z - a.z ) * amount + ( 2.0f * a.z - 5.0f * b.z + 4.0f * c.z - d.z ) * c2 + ( 3.0f * b.z - a.z - 3.0f * c.z + d.z ) * c3 ) );
}

//-----------------------------------------------------------------------------
//      線形補間を行います.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::Lerp( const Vector3& a, const Vector3& b, float amount )
{
    return Vector3(
        a.x + amount * ( b.x - a.x ),
        a.y + amount * ( b.y - a.y ),
        a.z + amount * ( b.z - a.z ) 
    );
}

//-----------------------------------------------------------------------------
//      線形補間を行います.
//-----------------------------------------------------------------------------
inline
void Vector3::Lerp( const Vector3 &a, const Vector3 &b, float amount, Vector3 &result )
{
    result.x = a.x + amount * ( b.x - a.x );
    result.y = a.y + amount * ( b.y - a.y );
    result.z = a.z + amount * ( b.z - a.z );
}

//-----------------------------------------------------------------------------
//      3次方程式を用いて，２つの値を補間します.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::SmoothStep( const Vector3& a, const Vector3& b, float amount )
{
    auto s = asdx::Clamp( amount, 0.0f, 1.0f );
    auto u = ( s * s ) + ( 3.0f - ( 2.0f * s ) );
    return Vector3(
        a.x + u * ( b.x - a.x ),
        a.y + u * ( b.y - a.y ),
        a.z + u * ( b.z - a.z )
    );
}

//-----------------------------------------------------------------------------
//      3次方程式を用いて，２つの値を補間します.
//-----------------------------------------------------------------------------
inline
void Vector3::SmoothStep( const Vector3 &a, const Vector3 &b, float amount, Vector3 &result )
{ 
    auto s = asdx::Clamp( amount, 0.0f, 1.0f );
    auto u = ( s * s ) + ( 3.0f - ( 2.0f * s ) );
    result.x = a.x + u * ( b.x - a.x );
    result.y = a.y + u * ( b.y - a.y );
    result.z = a.z + u * ( b.z - a.z );
}

//-----------------------------------------------------------------------------
//      指定された行列を用いて，ベクトルを変換します.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::Transform( const Vector3& position, const Matrix& matrix )
{
    return Vector3(
        ( ((position.x * matrix._11) + (position.y * matrix._21)) + (position.z * matrix._31)) + matrix._41,
        ( ((position.x * matrix._12) + (position.y * matrix._22)) + (position.z * matrix._32)) + matrix._42,
        ( ((position.x * matrix._13) + (position.y * matrix._23)) + (position.z * matrix._33)) + matrix._43 );
}

//-----------------------------------------------------------------------------
//      指定された行列を用いて，ベクトルを変換します.
//-----------------------------------------------------------------------------
inline
void Vector3::Transform( const Vector3 &position, const Matrix &matrix, Vector3 &result )
{
    result.x = ( ((position.x * matrix._11) + (position.y * matrix._21)) + (position.z * matrix._31)) + matrix._41;
    result.y = ( ((position.x * matrix._12) + (position.y * matrix._22)) + (position.z * matrix._32)) + matrix._42;
    result.z = ( ((position.x * matrix._13) + (position.y * matrix._23)) + (position.z * matrix._33)) + matrix._43;
}

//-----------------------------------------------------------------------------
//      指定された行列を用いて，法線ベクトルを変換します.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::TransformNormal( const Vector3& normal, const Matrix& matrix )
{
    return Vector3(
        ((normal.x * matrix._11) + (normal.y * matrix._21)) + (normal.z * matrix._31),
        ((normal.x * matrix._12) + (normal.y * matrix._22)) + (normal.z * matrix._32),
        ((normal.x * matrix._13) + (normal.y * matrix._23)) + (normal.z * matrix._33) );
}

//-----------------------------------------------------------------------------
//      指定された行列を用いて，法線ベクトルを変換します.
//-----------------------------------------------------------------------------
inline
void Vector3::TransformNormal( const Vector3 &normal, const Matrix &matrix, Vector3 &result )
{
    result.x = ((normal.x * matrix._11) + (normal.y * matrix._21)) + (normal.z * matrix._31);
    result.y = ((normal.x * matrix._12) + (normal.y * matrix._22)) + (normal.z * matrix._32);
    result.z = ((normal.x * matrix._13) + (normal.y * matrix._23)) + (normal.z * matrix._33);
}

//-----------------------------------------------------------------------------
//      指定された行列を用いてベクトルを変換し，変換結果をw=1に射影します.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::TransformCoord( const Vector3& coords, const Matrix& matrix )
{
    auto X = ( ( ((coords.x * matrix._11) + (coords.y * matrix._21)) + (coords.z * matrix._31) ) + matrix._41);
    auto Y = ( ( ((coords.x * matrix._12) + (coords.y * matrix._22)) + (coords.z * matrix._32) ) + matrix._42);
    auto Z = ( ( ((coords.x * matrix._13) + (coords.y * matrix._23)) + (coords.z * matrix._33) ) + matrix._43);
    auto W = ( ( ((coords.x * matrix._14) + (coords.y * matrix._24)) + (coords.z * matrix._34) ) + matrix._44);
    return Vector3(
        X / W,
        Y / W,
        Z / W 
    );
}

//-----------------------------------------------------------------------------
//      指定された行列を用いてベクトルを変換し，変換結果をw=1に射影します.
//-----------------------------------------------------------------------------
inline
void Vector3::TransformCoord( const Vector3 &coords, const Matrix &matrix, Vector3 &result )
{
    auto X = ( ( ((coords.x * matrix._11) + (coords.y * matrix._21)) + (coords.z * matrix._31) ) + matrix._41);
    auto Y = ( ( ((coords.x * matrix._12) + (coords.y * matrix._22)) + (coords.z * matrix._32) ) + matrix._42);
    auto Z = ( ( ((coords.x * matrix._13) + (coords.y * matrix._23)) + (coords.z * matrix._33) ) + matrix._43);
    auto W = ( ( ((coords.x * matrix._14) + (coords.y * matrix._24)) + (coords.z * matrix._34) ) + matrix._44);

    result.x = X / W;
    result.y = Y / W;
    result.z = Z / W;
}

//-----------------------------------------------------------------------------
//      スカラー3重積を求めます.
//-----------------------------------------------------------------------------
inline
float Vector3::ScalarTriple( const Vector3& a, const Vector3& b, const Vector3& c )
{
    auto crossX = ( b.y * c.z ) - ( b.z * c.y );
    auto crossY = ( b.z * c.x ) - ( b.x * c.z );
    auto crossZ = ( b.x * c.y ) - ( b.y * c.x );

    return ( a.x * crossX ) + ( a.y * crossY ) + ( a.z * crossZ );
}

//-----------------------------------------------------------------------------
//      スカラー3重積を求めます.
//-----------------------------------------------------------------------------
inline
void Vector3::ScalarTriple( const Vector3& a, const Vector3& b, const Vector3& c, float& result )
{
    auto crossX = ( b.y * c.z ) - ( b.z * c.y );
    auto crossY = ( b.z * c.x ) - ( b.x * c.z );
    auto crossZ = ( b.x * c.y ) - ( b.y * c.x );

    result = ( a.x * crossX ) + ( a.y * crossY ) + ( a.z * crossZ );
}

//-----------------------------------------------------------------------------
//      ベクトル3重積を求めます.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::VectorTriple( const Vector3& a, const Vector3& b, const Vector3& c )
{
    auto crossX = ( b.y * c.z ) - ( b.z * c.y );
    auto crossY = ( b.z * c.x ) - ( b.x * c.z );
    auto crossZ = ( b.x * c.y ) - ( b.y * c.x );

    return Vector3(
        ( ( a.y * crossZ ) - ( a.z * crossY ) ),
        ( ( a.z * crossX ) - ( a.x * crossZ ) ),
        ( ( a.x * crossY ) - ( a.y * crossX ) )
    );
}

//-----------------------------------------------------------------------------
//      ベクトル3重積を求めます.
//-----------------------------------------------------------------------------
inline
void Vector3::VectorTriple( const Vector3& a, const Vector3& b, const Vector3& c, Vector3& result )
{
    auto crossX = ( b.y * c.z ) - ( b.z * c.y );
    auto crossY = ( b.z * c.x ) - ( b.x * c.z );
    auto crossZ = ( b.x * c.y ) - ( b.y * c.x );

    result.x = ( a.y * crossZ ) - ( a.z * crossY );
    result.y = ( a.z * crossX ) - ( a.x * crossZ );
    result.z = ( a.x * crossY ) - ( a.y * crossX );
}

//-----------------------------------------------------------------------------
//      四元数でベクトルを回転させます.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::Rotate( const Vector3& value, const Quaternion& rotation )
{
    auto a = Quaternion( value.x, value.y, value.z, 0.0f );
    auto q = Quaternion::Conjugate( rotation );
    auto r = Quaternion::Multiply( q, a );
    r = Quaternion::Multiply( r, rotation );
    return Vector3( r.x, r.y, r.z );
}

//-----------------------------------------------------------------------------
//      四元数でベクトルを回転させます.
//-----------------------------------------------------------------------------
inline
void Vector3::Rotate( const Vector3& value, const Quaternion& rotation, Vector3& result )
{
    auto a = Quaternion( value.x, value.y, value.z, 0.0f );
    auto q = Quaternion::Conjugate( rotation );
    auto r = Quaternion::Multiply( q, a );
    r = Quaternion::Multiply( r, rotation );
    result.x = r.x;
    result.y = r.y;
    result.z = r.z;
}

//-----------------------------------------------------------------------------
//      四元数でベクトルを逆回転させます.
//-----------------------------------------------------------------------------
inline
Vector3 Vector3::InverseRotate( const Vector3& value, const Quaternion& rotation )
{
    auto a = Quaternion( value.x, value.y, value.z, 0.0f );
    auto r = Quaternion::Multiply( rotation, a );
    auto q = Quaternion::Conjugate( rotation );
    r = Quaternion::Multiply( r, q );
    return Vector3( r.x, r.y, r.z );
}

//-----------------------------------------------------------------------------
//      四元数でベクトルを逆回転させます.
//-----------------------------------------------------------------------------
inline
void Vector3::InverseRotate( const Vector3& value, const Quaternion& rotation, Vector3& result )
{
    auto a = Quaternion( value.x, value.y, value.z, 0.0f );
    auto r = Quaternion::Multiply( rotation, a );
    auto q = Quaternion::Conjugate( rotation );
    r = Quaternion::Multiply( r, q );
    result.x = r.x;
    result.y = r.y;
    result.z = r.z;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Vector4 structure
///////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
inline
Vector4::Vector4()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
inline
Vector4::Vector4( const float* pf )
{
    assert( pf != nullptr );
    x = pf[ 0 ];
    y = pf[ 1 ];
    z = pf[ 2 ];
    w = pf[ 3 ];
}

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
inline
Vector4::Vector4( const Vector2& value, float nz, float nw )
: x( value.x )
, y( value.y )
, z( nz )
, w( nw )
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
inline
Vector4::Vector4( const Vector3& value, float nw )
: x( value.x )
, y( value.y )
, z( value.z )
, w( nw )
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
inline
Vector4::Vector4( float nx, float ny, float nz, float nw )
: x( nx )
, y( ny )
, z( nz )
, w( nw )
{ /* DO_NTOHING */ }

//-----------------------------------------------------------------------------
//      float* 型へのキャストです.
//-----------------------------------------------------------------------------
inline
Vector4::operator float *()
{ return static_cast<float*>( &x ); }

//-----------------------------------------------------------------------------
//      const float* 型へのキャストです.
//-----------------------------------------------------------------------------
inline
Vector4::operator const float *() const
{ return static_cast<const float*>( &x ); }

//-----------------------------------------------------------------------------
//      加算代入演算子です.
//-----------------------------------------------------------------------------
inline
Vector4& Vector4::operator += ( const Vector4& v )
{
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;
    return (*this);
}

//-----------------------------------------------------------------------------
//      減算代入演算子です.
//-----------------------------------------------------------------------------
inline
Vector4& Vector4::operator -= ( const Vector4& v )
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;
    return (*this);
}

//-----------------------------------------------------------------------------
//      乗算代入演算子です.
//-----------------------------------------------------------------------------
inline
Vector4& Vector4::operator *= ( float f )
{
    x *= f;
    y *= f;
    z *= f;
    w *= f;
    return (*this);
}

//-----------------------------------------------------------------------------
//      除算代入演算子です.
//-----------------------------------------------------------------------------
inline
Vector4& Vector4::operator /= ( float f )
{
    assert( !IsZero( f ) );
    x /= f;
    y /= f;
    z /= f;
    w /= f;
    return (*this);
}

//-----------------------------------------------------------------------------
//      代入演算子です.
//-----------------------------------------------------------------------------
inline
Vector4& Vector4::operator = ( const Vector4& value )
{
    x = value.x;
    y = value.y;
    z = value.z;
    w = value.w;
    return (*this);
}

//-----------------------------------------------------------------------------
//      正符号演算子です.
//-----------------------------------------------------------------------------
inline
Vector4 Vector4::operator + () const
{ return (*this); }

//-----------------------------------------------------------------------------
//      負符号演算子です.
//-----------------------------------------------------------------------------
inline
Vector4 Vector4::operator - () const
{ return Vector4( -x, -y, -z, -w ); }

//-----------------------------------------------------------------------------
//      加算演算子です.
//-----------------------------------------------------------------------------
inline
Vector4 Vector4::operator + ( const Vector4& v ) const
{ return Vector4( x + v.x, y + v.y, z + v.z, w + v.w ); }

//-----------------------------------------------------------------------------
//      減算演算子です.
//-----------------------------------------------------------------------------
inline
Vector4 Vector4::operator - ( const Vector4& v ) const
{ return Vector4( x - v.x, y - v.y, z - v.z, w - v.w ); }

//-----------------------------------------------------------------------------
//      乗算演算子です.
//-----------------------------------------------------------------------------
inline
Vector4 Vector4::operator * ( float f ) const
{ return Vector4( x * f, y * f, z * f, w * f ); }

//-----------------------------------------------------------------------------
//      除算演算子です.
//-----------------------------------------------------------------------------
inline
Vector4 Vector4::operator / ( float f ) const
{
    assert( !IsZero( f ) );
    return Vector4( x / f, y / f, z / f, w / f );
}

//-----------------------------------------------------------------------------
//      乗算演算子です.
//-----------------------------------------------------------------------------
inline
Vector4 operator * ( float f, const Vector4& v )
{ return Vector4( f * v.x, f * v.y, f * v.z, f * v.w ); }

//-----------------------------------------------------------------------------
//      等価比較演算子です.
//-----------------------------------------------------------------------------
inline
bool Vector4::operator == ( const Vector4& v ) const
{
    return IsEqual( x, v.x )
        && IsEqual( y, v.y )
        && IsEqual( z, v.z )
        && IsEqual( w, v.z );
}

//-----------------------------------------------------------------------------
//      非等価比較演算子です.
//-----------------------------------------------------------------------------
inline
bool Vector4::operator != ( const Vector4& v ) const
{ 
    return !IsEqual( x, v.x )
        || !IsEqual( y, v.y )
        || !IsEqual( z, v.z )
        || !IsEqual( w, v.w );
}

//-----------------------------------------------------------------------------
//      ベクトルの大きさを求めます.
//-----------------------------------------------------------------------------
inline
float Vector4::Length() const
{ return Hypot( x, y, z, w ); }

//-----------------------------------------------------------------------------
//      ベクトルの大きさの2乗値を求めます.
//-----------------------------------------------------------------------------
inline
float Vector4::LengthSq() const
{ return ( x * x + y * y + z * z + w * w ); }

//-----------------------------------------------------------------------------
//      正規化を行います.
//-----------------------------------------------------------------------------
inline
Vector4& Vector4::Normalize()
{
    auto mag = Length();
    assert( mag > 0.0f );
    x /= mag;
    y /= mag;
    z /= mag;
    w /= mag;
    return (*this);
}

//-----------------------------------------------------------------------------
//      零除算を考慮して正規化を行います.
//-----------------------------------------------------------------------------
inline
Vector4& Vector4::SafeNormalize( const Vector4& set )
{
    auto mag = Length();
    if ( mag > 0.0f )
    {
        x /= mag;
        y /= mag;
        z /= mag;
        w /= mag;
    }
    else
    {
        x = set.x;
        y = set.y;
        z = set.z;
        w = set.w;
    }

    return (*this);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Vector4  Methods
///////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      各成分の絶対値を求めます.
//-----------------------------------------------------------------------------
inline
Vector4 Vector4::Abs( const Vector4& value )
{ 
    return Vector4( 
        fabs( value.x ),
        fabs( value.y ),
        fabs( value.z ),
        fabs( value.w )
    );
}

//-----------------------------------------------------------------------------
//      各成分の絶対値を求めます.
//-----------------------------------------------------------------------------
inline
void Vector4::Abs( const Vector4 &value, Vector4 &result )
{ 
    result.x = fabs( value.x );
    result.y = fabs( value.y );
    result.z = fabs( value.z );
    result.w = fabs( value.w );
}

//-----------------------------------------------------------------------------
//      値を指定された範囲内に制限します.
//-----------------------------------------------------------------------------
inline
Vector4 Vector4::Clamp( const Vector4& value, const Vector4& a, const Vector4& b )
{
    return Vector4( 
        asdx::Clamp( value.x, a.x, b.x ),
        asdx::Clamp( value.y, a.y, b.y ),
        asdx::Clamp( value.z, a.z, b.z ),
        asdx::Clamp( value.w, a.w, b.w )
    );
}

//-----------------------------------------------------------------------------
//      値を指定された範囲内に制限します.
//-----------------------------------------------------------------------------
inline
void Vector4::Clamp( const Vector4 &value, const Vector4 &a, const Vector4 &b, Vector4 &result )
{
    result.x = asdx::Clamp( value.x, a.x, b.x );
    result.y = asdx::Clamp( value.y, a.y, b.y );
    result.z = asdx::Clamp( value.z, a.z, b.z );
    result.w = asdx::Clamp( value.w, a.w, b.w );
}

//-----------------------------------------------------------------------------
//      指定された値を0～1の範囲に制限します.
//-----------------------------------------------------------------------------
inline
Vector4 Vector4::Saturate( const Vector4& value )
{
    return Vector4(
        asdx::Saturate( value.x ),
        asdx::Saturate( value.y ),
        asdx::Saturate( value.z ),
        asdx::Saturate( value.w )
    );
}

//-----------------------------------------------------------------------------
//      指定された体を0～1の範囲に制限します.
//-----------------------------------------------------------------------------
inline
void Vector4::Saturate( const Vector4& value, Vector4& result )
{
    result.x = asdx::Saturate( value.x );
    result.y = asdx::Saturate( value.y );
    result.z = asdx::Saturate( value.z );
    result.w = asdx::Saturate( value.w );
}

//-----------------------------------------------------------------------------
//      2点間距離を求めます.
//-----------------------------------------------------------------------------
inline
float Vector4::Distance( const Vector4& a, const Vector4& b )
{ return Hypot( b.x - a.x, b.y - a.y, b.z - a.z, b.w - a.w ); }

//-----------------------------------------------------------------------------
//      2点間距離を求めます.
//-----------------------------------------------------------------------------
inline
void Vector4::Distance( const Vector4 &a, const Vector4 &b, float &result )
{ result = Hypot( b.x - a.x, b.y - a.y, b.z - a.z, b.w - a.w ); }

//-----------------------------------------------------------------------------
//      2点間距離の2乗値を求めます.
//-----------------------------------------------------------------------------
inline
float Vector4::DistanceSq( const Vector4& a, const Vector4& b )
{
    auto X = b.x - a.x;
    auto Y = b.y - a.y;
    auto Z = b.z - a.z;
    auto W = b.w - a.w;
    return X * X + Y * Y + Z * Z + W * W;
}

//-----------------------------------------------------------------------------
//      2点間距離の2乗値を求めます.
//-----------------------------------------------------------------------------
inline
void Vector4::DistanceSq( const Vector4 &a, const Vector4 &b, float &result )
{
    auto X = b.x - a.x;
    auto Y = b.y - a.y;
    auto Z = b.z - a.z;
    auto W = b.w - a.w;
    result = X * X + Y * Y + Z * Z + W * W;
}

//-----------------------------------------------------------------------------
//      内積を求めます.
//-----------------------------------------------------------------------------
inline
float Vector4::Dot( const Vector4& a, const Vector4& b )
{ return ( a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w ); }

//-----------------------------------------------------------------------------
//      内積を求めます.
//-----------------------------------------------------------------------------
inline
void Vector4::Dot( const Vector4 &a, const Vector4 &b, float &result )
{ result = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

//-----------------------------------------------------------------------------
//      正規化を行います.
//-----------------------------------------------------------------------------
inline
Vector4 Vector4::Normalize( const Vector4& value )
{
    auto mag = value.Length();
    assert( mag > 0.0f );
    return Vector4(
        value.x / mag,
        value.y / mag,
        value.z / mag,
        value.w / mag
    );
}

//-----------------------------------------------------------------------------
//      正規化を行います.
//-----------------------------------------------------------------------------
inline
void Vector4::Normalize( const Vector4 &value, Vector4 &result )
{
    auto mag = value.Length();
    assert( mag > 0.0f );
    result.x = value.x / mag;
    result.y = value.y / mag;
    result.z = value.z / mag;
    result.w = value.w / mag;
}

//-----------------------------------------------------------------------------
//      零除算を考慮して正規化を行います.
//-----------------------------------------------------------------------------
inline
Vector4 Vector4::SafeNormalize( const Vector4& value, const Vector4& set )
{
    auto mag = value.Length();
    if ( mag > 0.0f )
    {
        return Vector4(
            value.x / mag,
            value.y / mag,
            value.z / mag,
            value.w / mag
        );
    }

    return set;
}

//-----------------------------------------------------------------------------
//      零除算を考慮して正規化を行います.
//-----------------------------------------------------------------------------
inline
void Vector4::SafeNormalize( const Vector4& value, const Vector4& set, Vector4& result)
{
    auto mag = value.Length();
    if ( mag > 0.0f )
    {
        result.x = value.x / mag;
        result.y = value.y / mag;
        result.z = value.z / mag;
        result.w = value.w / mag;
    }
    else
    {
        result.x = set.x;
        result.y = set.y;
        result.z = set.z;
        result.w = set.w;
    }
}

//-----------------------------------------------------------------------------
//      交差角を求めます.
//-----------------------------------------------------------------------------
inline
float Vector4::ComputeCrossingAngle( const Vector4& a, const Vector4& b )
{
    auto d = a.Length() * b.Length();
    if ( d <= 0.0f )
    { return 0.0f; }

    auto c = Vector4::Dot( a, b ) / d;
    if ( c >= 1.0f )
    { return 0.0f; }

    if ( c <= -1.0f ) 
    { return F_PI; }

    return acosf( c );
}

//-----------------------------------------------------------------------------
//      交差角を求めます.
//-----------------------------------------------------------------------------
inline
void Vector4::ComputeCrossingAngle( const Vector4 &a, const Vector4 &b, float &result )
{
    auto d = a.Length() * b.Length();
    if ( d <= 0.0f )
    {
        result = 0.0f;
        return;
    }

    auto c = Vector4::Dot( a, b ) / d;
    if ( c >= 1.0f ) 
    {
        result = 0.0f;
        return;
    }

    if ( c <= -1.0f )
    {
        result = F_PI;
        return;
    }

    result = acosf( c );
}

//-----------------------------------------------------------------------------
//      各成分の最小値を求めます.
//-----------------------------------------------------------------------------
inline
Vector4 Vector4::Min( const Vector4& a, const Vector4& b )
{ 
    return Vector4( 
        asdx::Min( a.x, b.x ),
        asdx::Min( a.y, b.y ),
        asdx::Min( a.z, b.z ),
        asdx::Min( a.w, b.w )
    );
}

//-----------------------------------------------------------------------------
//      各成分の最小値を求めます.
//-----------------------------------------------------------------------------
inline
void Vector4::Min( const Vector4 &a, const Vector4 &b, Vector4 &result )
{
    result.x = asdx::Min( a.x, b.x );
    result.y = asdx::Min( a.y, b.y );
    result.z = asdx::Min( a.z, b.z );
    result.w = asdx::Min( a.w, b.w );
}

//-----------------------------------------------------------------------------
//      各成分の最大値を求めます.
//-----------------------------------------------------------------------------
inline
Vector4 Vector4::Max( const Vector4& a, const Vector4& b )
{
    return Vector4( 
        asdx::Max( a.x, b.x ),
        asdx::Max( a.y, b.y ),
        asdx::Max( a.z, b.z ),
        asdx::Max( a.w, b.w )
    );
}

//-----------------------------------------------------------------------------
//      各成分の最大値を求めます.
//-----------------------------------------------------------------------------
inline
void Vector4::Max( const Vector4 &a, const Vector4 &b, Vector4 &result )
{
    result.x = asdx::Max( a.x, b.x );
    result.y = asdx::Max( a.y, b.y );
    result.z = asdx::Max( a.z, b.z );
    result.w = asdx::Max( a.w, b.w );
}

//-----------------------------------------------------------------------------
//      重心座標を求めます.
//-----------------------------------------------------------------------------
inline
Vector4 Vector4::Barycentric
(
    const Vector4&  a,
    const Vector4&  b,
    const Vector4&  c,
    float             f,
    float             g
)
{
    return Vector4(
        a.x + f * ( b.x - a.x ) + g * ( c.x - a.x ),
        a.y + f * ( b.y - a.y ) + g * ( c.y - a.y ),
        a.z + f * ( b.z - a.z ) + g * ( c.z - a.z ),
        a.w + f * ( b.w - a.w ) + g * ( c.w - a.w )
    );
}

//-----------------------------------------------------------------------------
//      重心座標を求めます.
//-----------------------------------------------------------------------------
inline
void Vector4::Barycentric
(
    const Vector4&  a,
    const Vector4&  b,
    const Vector4&  c,
    float             f,
    float             g,
    Vector4&        result
)
{
    result.x = a.x + f * ( b.x - a.x ) + g * ( c.x - a.x );
    result.y = a.y + f * ( b.y - a.y ) + g * ( c.y - a.y );
    result.z = a.z + f * ( b.z - a.z ) + g * ( c.z - a.z );
    result.w = a.w + f * ( b.w - a.w ) + g * ( c.w - a.w );
}

//-----------------------------------------------------------------------------
//      エルミートスプライン補間を行います.
//-----------------------------------------------------------------------------
inline
Vector4 Vector4::Hermite
(
    const Vector4&  a,
    const Vector4&  t1,
    const Vector4&  b,
    const Vector4&  t2,
    float             amount
)
{
    auto c2 = amount * amount;
    auto c3 = c2 * amount;

    Vector4 result;
    if ( amount <= 0.0f )
    {
        result.x = a.x;
        result.y = a.y;
        result.z = a.z;
        result.w = a.w;
    }
    else if ( amount >= 1.0f )
    {
        result.x = b.x;
        result.y = b.y;
        result.z = b.z;
        result.w = b.w;
    }
    else
    {
        result.x = ( 2.0f * a.x - 2.0f * b.x + t2.x + t1.x ) * c3 + ( 3.0f * b.x - 3.0f * a.x - 2.0f * t1.x - t2.x ) * c3 + t1.x * amount + a.x;
        result.y = ( 2.0f * a.y - 2.0f * b.y + t2.y + t1.y ) * c3 + ( 3.0f * b.y - 3.0f * a.y - 2.0f * t1.y - t2.y ) * c3 + t1.y * amount + a.y;
        result.z = ( 2.0f * a.z - 2.0f * b.z + t2.z + t1.z ) * c3 + ( 3.0f * b.z - 3.0f * a.z - 2.0f * t1.z - t2.z ) * c3 + t1.y * amount + a.z;
        result.w = ( 2.0f * a.w - 2.0f * b.w + t2.w + t1.w ) * c3 + ( 3.0f * b.w - 3.0f * a.w - 2.0f * t1.w - t2.w ) * c3 + t1.w * amount + a.w;
    }
    return result;
}

//-----------------------------------------------------------------------------
//      エルミートスプライン補間を行います.
//-----------------------------------------------------------------------------
inline
void Vector4::Hermite
(
    const Vector4&  a,
    const Vector4&  t1,
    const Vector4&  b,
    const Vector4&  t2,
    float             amount,
    Vector4&        result
)
{
    auto c2 = amount * amount;
    auto c3 = c2 * amount;

    if ( amount <= 0.0f )
    {
        result.x = a.x;
        result.y = a.y;
        result.z = a.z;
        result.w = a.w;
    }
    else if ( amount >= 1.0f )
    {
        result.x = b.x;
        result.y = b.y;
        result.z = b.z;
        result.w = b.w;
    }
    else
    {
        result.x = ( 2.0f * a.x - 2.0f * b.x + t2.x + t1.x ) * c3 + ( 3.0f * b.x - 3.0f * a.x - 2.0f * t1.x - t2.x ) * c3 + t1.x * amount + a.x;
        result.y = ( 2.0f * a.y - 2.0f * b.y + t2.y + t1.y ) * c3 + ( 3.0f * b.y - 3.0f * a.y - 2.0f * t1.y - t2.y ) * c3 + t1.y * amount + a.y;
        result.z = ( 2.0f * a.z - 2.0f * b.z + t2.z + t1.z ) * c3 + ( 3.0f * b.z - 3.0f * a.z - 2.0f * t1.z - t2.z ) * c3 + t1.z * amount + a.z;
        result.w = ( 2.0f * a.w - 2.0f * b.w + t2.w + t1.w ) * c3 + ( 3.0f * b.w - 3.0f * a.w - 2.0f * t1.w - t2.w ) * c3 + t1.w * amount + a.w;
    }
}

//-----------------------------------------------------------------------------
//      Catmull-Rom スプライン補間を行います.
//-----------------------------------------------------------------------------
inline
Vector4 Vector4::CatmullRom
(
    const Vector4&  a,
    const Vector4&  b,
    const Vector4&  c,
    const Vector4&  d,
    float             amount
)
{
    auto c2 = amount * amount;
    auto c3 = c2 * amount;

    return Vector4(
        ( 0.5f * ( 2.0f * b.x + ( c.x - a.x ) * amount + ( 2.0f * a.x - 5.0f * b.x + 4.0f * c.x - d.x ) * c2 + ( 3.0f * b.x - a.x - 3.0f * c.x + d.x ) * c3 ) ),
        ( 0.5f * ( 2.0f * b.y + ( c.y - a.y ) * amount + ( 2.0f * a.y - 5.0f * b.y + 4.0f * c.y - d.y ) * c2 + ( 3.0f * b.y - a.y - 3.0f * c.y + d.y ) * c3 ) ),
        ( 0.5f * ( 2.0f * b.z + ( c.z - a.z ) * amount + ( 2.0f * a.z - 5.0f * b.z + 4.0f * c.z - d.z ) * c2 + ( 3.0f * b.z - a.z - 3.0f * c.z + d.z ) * c3 ) ),
        ( 0.5f * ( 2.0f * b.w + ( c.w - a.w ) * amount + ( 2.0f * a.w - 5.0f * b.w + 4.0f * c.w - d.w ) * c2 + ( 3.0f * b.w - a.w - 3.0f * c.w + d.w ) * c3 ) )
    );
}

//-----------------------------------------------------------------------------
//      Catmul-Rom スプライン補間を行います.
//-----------------------------------------------------------------------------
inline
void Vector4::CatmullRom
(
    const Vector4&  a,
    const Vector4&  b,
    const Vector4&  c,
    const Vector4&  d,
    float             amount,
    Vector4&        result
)
{
    auto c2 = amount * amount;
    auto c3 = c2 * amount;

    result.x = ( 0.5f * ( 2.0f * b.x + ( c.x - a.x ) * amount + ( 2.0f * a.x - 5.0f * b.x + 4.0f * c.x - d.x ) * c2 + ( 3.0f * b.x - a.x - 3.0f * c.x + d.x ) * c3 ) );
    result.y = ( 0.5f * ( 2.0f * b.y + ( c.y - a.y ) * amount + ( 2.0f * a.y - 5.0f * b.y + 4.0f * c.y - d.y ) * c2 + ( 3.0f * b.y - a.y - 3.0f * c.y + d.y ) * c3 ) );
    result.z = ( 0.5f * ( 2.0f * b.z + ( c.z - a.z ) * amount + ( 2.0f * a.z - 5.0f * b.z + 4.0f * c.z - d.z ) * c2 + ( 3.0f * b.z - a.z - 3.0f * c.z + d.z ) * c3 ) );
    result.w = ( 0.5f * ( 2.0f * b.w + ( c.w - a.w ) * amount + ( 2.0f * a.w - 5.0f * b.w + 4.0f * c.w - d.w ) * c2 + ( 3.0f * b.w - a.w - 3.0f * c.w + d.w ) * c3 ) );
}

//-----------------------------------------------------------------------------
//      線形補間を行います.
//-----------------------------------------------------------------------------
inline
Vector4 Vector4::Lerp( const Vector4& a, const Vector4& b, float amount )
{
    return Vector4(
        a.x + amount * ( b.x - a.x ),
        a.y + amount * ( b.y - a.y ),
        a.z + amount * ( b.z - a.z ),
        a.w + amount * ( b.w - a.w )
    );
}

//-----------------------------------------------------------------------------
//      線形補間を行います.
//-----------------------------------------------------------------------------
inline
void Vector4::Lerp( const Vector4 &a, const Vector4 &b, float amount, Vector4 &result )
{
    result.x = a.x + amount * ( b.x - a.x );
    result.y = a.y + amount * ( b.y - a.y );
    result.z = a.z + amount * ( b.z - a.z );
    result.w = a.w + amount * ( b.w - a.w );
}

//-----------------------------------------------------------------------------
//      3次方程式を用いて，2つの値の間を補間します.
//-----------------------------------------------------------------------------
inline
Vector4 Vector4::SmoothStep( const Vector4& a, const Vector4& b, float amount )
{
    auto s = asdx::Clamp( amount, 0.0f, 1.0f );
    auto u = ( s * s ) + ( 3.0f - ( 2.0f * s ) );
    return Vector4(
        a.x + u * ( b.x - a.x ),
        a.y + u * ( b.y - a.y ),
        a.z + u * ( b.z - a.z ),
        a.w + u * ( b.w - a.w )
    );
}

//-----------------------------------------------------------------------------
//      3次方程式を用いて，2つの値の間を補完します.
//-----------------------------------------------------------------------------
inline
void Vector4::SmoothStep( const Vector4 &a, const Vector4 &b, float amount, Vector4 &result )
{
    auto s = asdx::Clamp( amount, 0.0f, 1.0f );
    auto u = ( s * s ) + ( 3.0f - ( 2.0f * s ) );
    result.x = a.x + u * ( b.x - a.x );
    result.y = a.y + u * ( b.y - a.y );
    result.z = a.z + u * ( b.z - a.z );
    result.w = a.w + u * ( b.w - a.w );
}

//-----------------------------------------------------------------------------
//      指定された行列を用いて，ベクトルを変換します.
//-----------------------------------------------------------------------------
inline
Vector4 Vector4::Transform( const Vector4& position, const Matrix& matrix )
{
    return Vector4(
        ( ( ((position.x * matrix._11) + (position.y * matrix._21)) + (position.z * matrix._31) ) + (position.w * matrix._41)),
        ( ( ((position.x * matrix._12) + (position.y * matrix._22)) + (position.z * matrix._32) ) + (position.w * matrix._42)),
        ( ( ((position.x * matrix._13) + (position.y * matrix._23)) + (position.z * matrix._33) ) + (position.w * matrix._43)),
        ( ( ((position.x * matrix._14) + (position.y * matrix._24)) + (position.z * matrix._34) ) + (position.w * matrix._44)) );
}

//-----------------------------------------------------------------------------
//      指定された行列を用いて，ベクトルを変換します.
//-----------------------------------------------------------------------------
inline
void Vector4::Transform( const Vector4 &position, const Matrix &matrix, Vector4 &result )
{
    result.x = ( ( ((position.x * matrix._11) + (position.y * matrix._21)) + (position.z * matrix._31) ) + (position.w * matrix._41));
    result.y = ( ( ((position.x * matrix._12) + (position.y * matrix._22)) + (position.z * matrix._32) ) + (position.w * matrix._42));
    result.z = ( ( ((position.x * matrix._13) + (position.y * matrix._23)) + (position.z * matrix._33) ) + (position.w * matrix._43));
    result.w = ( ( ((position.x * matrix._14) + (position.y * matrix._24)) + (position.z * matrix._34) ) + (position.w * matrix._44));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Matrix structure (row-major)
///////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
inline
Matrix::Matrix()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
inline
Matrix::Matrix( const float* pf )
{
    assert( pf != nullptr );
    memcpy( &_11, pf, sizeof(Matrix) );
}

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
inline
Matrix::Matrix
(
    float _f11, float _f12, float _f13, float _f14,
    float _f21, float _f22, float _f23, float _f24,
    float _f31, float _f32, float _f33, float _f34,
    float _f41, float _f42, float _f43, float _f44 
)
{
    _11 = _f11; _12 = _f12; _13 = _f13; _14 = _f14;
    _21 = _f21; _22 = _f22; _23 = _f23; _24 = _f24;
    _31 = _f31; _32 = _f32; _33 = _f33; _34 = _f34;
    _41 = _f41; _42 = _f42; _43 = _f43; _44 = _f44;
}

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
inline
Matrix::Matrix( const Vector4& v1, const Vector4& v2, const Vector4& v3, const Vector4& v4 )
{
    _11 = v1.x; _12 = v1.y; _13 = v1.z; _14 = v1.z;
    _21 = v2.x; _22 = v2.y; _23 = v2.z; _24 = v2.z;
    _31 = v3.x; _32 = v3.y; _33 = v3.z; _34 = v3.z;
    _41 = v4.x; _42 = v4.y; _43 = v4.z; _44 = v4.z;
}

//-----------------------------------------------------------------------------
//      インデクサです.
//-----------------------------------------------------------------------------
inline 
float& Matrix::operator () ( uint32_t iRow, uint32_t iCol )
{ return m[iRow][iCol]; }

//-----------------------------------------------------------------------------
//      インデクサです(const版).
//-----------------------------------------------------------------------------
inline 
const float& Matrix::operator () ( uint32_t iRow, uint32_t iCol ) const
{ return m[iRow][iCol]; }

//-----------------------------------------------------------------------------
//      float* 型へのキャストです.
//-----------------------------------------------------------------------------
inline
Matrix::operator float* ()
{ return static_cast<float*>( &_11 ); }

//-----------------------------------------------------------------------------
//      const float* 型へのキャストです.
//-----------------------------------------------------------------------------
inline
Matrix::operator const float* () const 
{ return static_cast<const float*>( &_11 ); }

//-----------------------------------------------------------------------------
//      乗算代入演算子です.
//-----------------------------------------------------------------------------
inline 
Matrix& Matrix::operator *= ( const Matrix &value )
{
    auto m11 = ( _11 * value._11 ) + ( _12 * value._21 ) + ( _13 * value._31 ) + ( _14 * value._41 );
    auto m12 = ( _11 * value._12 ) + ( _12 * value._22 ) + ( _13 * value._32 ) + ( _14 * value._42 );
    auto m13 = ( _11 * value._13 ) + ( _12 * value._23 ) + ( _13 * value._33 ) + ( _14 * value._43 );
    auto m14 = ( _11 * value._14 ) + ( _12 * value._24 ) + ( _13 * value._34 ) + ( _14 * value._44 );

    auto m21 = ( _21 * value._11 ) + ( _22 * value._21 ) + ( _23 * value._31 ) + ( _24 * value._41 );
    auto m22 = ( _21 * value._12 ) + ( _22 * value._22 ) + ( _23 * value._32 ) + ( _24 * value._42 );
    auto m23 = ( _21 * value._13 ) + ( _22 * value._23 ) + ( _23 * value._33 ) + ( _24 * value._43 );
    auto m24 = ( _21 * value._14 ) + ( _22 * value._24 ) + ( _23 * value._34 ) + ( _24 * value._44 );

    auto m31 = ( _31 * value._11 ) + ( _32 * value._21 ) + ( _33 * value._31 ) + ( _34 * value._41 );
    auto m32 = ( _31 * value._12 ) + ( _32 * value._22 ) + ( _33 * value._32 ) + ( _34 * value._42 );
    auto m33 = ( _31 * value._13 ) + ( _32 * value._23 ) + ( _33 * value._33 ) + ( _34 * value._43 );
    auto m34 = ( _31 * value._14 ) + ( _32 * value._24 ) + ( _33 * value._34 ) + ( _34 * value._44 );

    auto m41 = ( _41 * value._11 ) + ( _42 * value._21 ) + ( _43 * value._31 ) + ( _44 * value._41 );
    auto m42 = ( _41 * value._12 ) + ( _42 * value._22 ) + ( _43 * value._32 ) + ( _44 * value._42 );
    auto m43 = ( _41 * value._13 ) + ( _42 * value._23 ) + ( _43 * value._33 ) + ( _44 * value._43 );
    auto m44 = ( _41 * value._14 ) + ( _42 * value._24 ) + ( _43 * value._34 ) + ( _44 * value._44 );

    _11 = m11;  _12 = m12;  _13 = m13;  _14 = m14;
    _21 = m21;  _22 = m22;  _23 = m23;  _24 = m24;
    _31 = m31;  _32 = m32;  _33 = m33;  _34 = m34;
    _41 = m41;  _42 = m42;  _43 = m43;  _44 = m44;

    return (*this);
}

//-----------------------------------------------------------------------------
//      加算代入演算子です.
//-----------------------------------------------------------------------------
inline 
Matrix& Matrix::operator += ( const Matrix& mat )
{
    _11 += mat._11; _12 += mat._12; _13 += mat._13; _14 += mat._14;
    _21 += mat._21; _22 += mat._22; _23 += mat._23; _24 += mat._24;
    _31 += mat._31; _32 += mat._32; _33 += mat._33; _34 += mat._34;
    _41 += mat._41; _42 += mat._42; _43 += mat._43; _44 += mat._44;
    return (*this);
}

//-----------------------------------------------------------------------------
//      減算代入演算子です.
//-----------------------------------------------------------------------------
inline 
Matrix& Matrix::operator -= ( const Matrix& mat )
{
    _11 -= mat._11; _12 -= mat._12; _13 -= mat._13; _14 -= mat._14;
    _21 -= mat._21; _22 -= mat._22; _23 -= mat._23; _24 -= mat._24;
    _31 -= mat._31; _32 -= mat._32; _33 -= mat._33; _34 -= mat._34;
    _41 -= mat._41; _42 -= mat._42; _43 -= mat._43; _44 -= mat._44;
    return (*this);
}

//-----------------------------------------------------------------------------
//      乗算代入演算子です.
//-----------------------------------------------------------------------------
inline
Matrix& Matrix::operator *= ( float f )
{
    _11 *= f; _12 *= f; _13 *= f; _14 *= f;
    _21 *= f; _22 *= f; _23 *= f; _24 *= f;
    _31 *= f; _32 *= f; _33 *= f; _34 *= f;
    _41 *= f; _42 *= f; _43 *= f; _44 *= f;
    return (*this);
}

//-----------------------------------------------------------------------------
//      除算代入演算子です.
//-----------------------------------------------------------------------------
inline 
Matrix& Matrix::operator /= ( float f )
{
    assert( !IsZero( f ) );
    _11 /= f; _12 /= f; _13 /= f; _14 /= f;
    _21 /= f; _22 /= f; _23 /= f; _24 /= f;
    _31 /= f; _32 /= f; _33 /= f; _34 /= f;
    _41 /= f; _42 /= f; _43 /= f; _44 /= f;
    return (*this);
}

//-----------------------------------------------------------------------------
//      代入演算子です.
//-----------------------------------------------------------------------------
inline
Matrix& Matrix::operator = ( const Matrix& value )
{
    memcpy( &_11, &value._11, sizeof(Matrix) );
    return (*this);
}

//-----------------------------------------------------------------------------
//      正符号演算子です.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::operator + () const
{ return (*this); }

//-----------------------------------------------------------------------------
//      負符号演算子です.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::operator - () const
{
    return Matrix(
        -_11, -_12, -_13, -_14,
        -_21, -_22, -_23, -_24,
        -_31, -_32, -_33, -_34,
        -_41, -_42, -_43, -_44 );
}

//-----------------------------------------------------------------------------
//      乗算演算子です.
//-----------------------------------------------------------------------------
inline 
Matrix Matrix::operator * ( const Matrix& value ) const
{
    return Matrix(
        ( _11 * value._11 ) + ( _12 * value._21 ) + ( _13 * value._31 ) + ( _14 * value._41 ),
        ( _11 * value._12 ) + ( _12 * value._22 ) + ( _13 * value._32 ) + ( _14 * value._42 ),
        ( _11 * value._13 ) + ( _12 * value._23 ) + ( _13 * value._33 ) + ( _14 * value._43 ),
        ( _11 * value._14 ) + ( _12 * value._24 ) + ( _13 * value._34 ) + ( _14 * value._44 ),

        ( _21 * value._11 ) + ( _22 * value._21 ) + ( _23 * value._31 ) + ( _24 * value._41 ),
        ( _21 * value._12 ) + ( _22 * value._22 ) + ( _23 * value._32 ) + ( _24 * value._42 ),
        ( _21 * value._13 ) + ( _22 * value._23 ) + ( _23 * value._33 ) + ( _24 * value._43 ),
        ( _21 * value._14 ) + ( _22 * value._24 ) + ( _23 * value._34 ) + ( _24 * value._44 ),

        ( _31 * value._11 ) + ( _32 * value._21 ) + ( _33 * value._31 ) + ( _34 * value._41 ),
        ( _31 * value._12 ) + ( _32 * value._22 ) + ( _33 * value._32 ) + ( _34 * value._42 ),
        ( _31 * value._13 ) + ( _32 * value._23 ) + ( _33 * value._33 ) + ( _34 * value._43 ),
        ( _31 * value._14 ) + ( _32 * value._24 ) + ( _33 * value._34 ) + ( _34 * value._44 ),

        ( _41 * value._11 ) + ( _42 * value._21 ) + ( _43 * value._31 ) + ( _44 * value._41 ),
        ( _41 * value._12 ) + ( _42 * value._22 ) + ( _43 * value._32 ) + ( _44 * value._42 ),
        ( _41 * value._13 ) + ( _42 * value._23 ) + ( _43 * value._33 ) + ( _44 * value._43 ),
        ( _41 * value._14 ) + ( _42 * value._24 ) + ( _43 * value._34 ) + ( _44 * value._44 )
    );
}

//-----------------------------------------------------------------------------
//      加算演算子です.
//-----------------------------------------------------------------------------
inline 
Matrix Matrix::operator + ( const Matrix& mat ) const
{
    return Matrix(
        _11 + mat._11, _12 + mat._12, _13 + mat._13, _14 + mat._14,
        _21 + mat._21, _22 + mat._22, _23 + mat._23, _24 + mat._24,
        _31 + mat._31, _32 + mat._32, _33 + mat._33, _34 + mat._34,
        _41 + mat._41, _42 + mat._42, _43 + mat._43, _44 + mat._44 );
}

//-----------------------------------------------------------------------------
//      減算演算子です.
//-----------------------------------------------------------------------------
inline 
Matrix Matrix::operator - ( const Matrix& mat ) const
{
    return Matrix(
        _11 - mat._11, _12 - mat._12, _13 - mat._13, _14 - mat._14,
        _21 - mat._21, _22 - mat._22, _23 - mat._23, _24 - mat._24,
        _31 - mat._31, _32 - mat._32, _33 - mat._33, _34 - mat._34,
        _41 - mat._41, _42 - mat._42, _43 - mat._43, _44 - mat._44 );
}

//-----------------------------------------------------------------------------
//      乗算演算子です.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::operator * ( float f ) const
{
    return Matrix( 
        _11 * f, _12 * f, _13 * f, _14 * f,
        _21 * f, _22 * f, _23 * f, _24 * f,
        _31 * f, _32 * f, _33 * f, _34 * f,
        _41 * f, _42 * f, _43 * f, _44 * f );
}

//-----------------------------------------------------------------------------
//      除算演算子です.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::operator / ( float f ) const
{
    assert( !IsZero( f ) );
    return Matrix(
        _11 / f, _12 / f, _13 / f, _14 / f,
        _21 / f, _22 / f, _23 / f, _24 / f,
        _31 / f, _32 / f, _33 / f, _34 / f,
        _41 / f, _42 / f, _43 / f, _44 / f);
}

//-----------------------------------------------------------------------------
//      乗算演算子です.
//-----------------------------------------------------------------------------
inline 
Matrix operator * ( float f, const Matrix& mat )
{
    return Matrix(
        f * mat._11, f * mat._12, f * mat._13, f * mat._14,
        f * mat._21, f * mat._22, f * mat._23, f * mat._24,
        f * mat._31, f * mat._32, f * mat._33, f * mat._34,
        f * mat._41, f * mat._42, f * mat._43, f * mat._44 );
}

//-----------------------------------------------------------------------------
//      等価比較演算子です.
//-----------------------------------------------------------------------------
inline 
bool Matrix::operator == ( const Matrix& mat ) const
{ return ( 0 == memcmp( this, &mat, sizeof( Matrix ) ) ); }

//-----------------------------------------------------------------------------
//      非等価比較演算子です.
//-----------------------------------------------------------------------------
inline 
bool Matrix::operator != ( const Matrix& mat ) const
{ return ( 0 != memcmp( this, &mat, sizeof( Matrix ) ) ); }

//-----------------------------------------------------------------------------
//      行列式を求めます.
//-----------------------------------------------------------------------------
inline 
float Matrix::Determinant() const
{
    return
        _11 * _22 * _33 * _44 + _11 * _23 * _34 * _42 +
        _11 * _24 * _32 * _43 + _12 * _21 * _34 * _43 +
        _12 * _23 * _31 * _44 + _12 * _24 * _33 * _41 +
        _13 * _21 * _32 * _44 + _13 * _22 * _34 * _41 +
        _13 * _24 * _31 * _42 + _14 * _21 * _33 * _42 +
        _14 * _22 * _31 * _43 + _14 * _23 * _32 * _41 -
        _11 * _22 * _34 * _43 - _11 * _23 * _32 * _44 -
        _11 * _24 * _33 * _42 - _12 * _21 * _33 * _44 -
        _12 * _23 * _34 * _41 - _12 * _24 * _31 * _43 -
        _13 * _21 * _34 * _42 - _13 * _22 * _31 * _44 -
        _13 * _24 * _32 * _41 - _14 * _21 * _32 * _43 -
        _14 * _22 * _33 * _41 - _14 * _23 * _31 * _42;
}

//-----------------------------------------------------------------------------
//      単位行列化します.
//-----------------------------------------------------------------------------
inline 
Matrix& Matrix::Identity()
{
    _11 = _22 = _33 = _44 = 1.0f;
    _12 = _13 = _14 =
    _21 = _23 = _24 =
    _31 = _32 = _34 =
    _41 = _42 = _43 = 0.0f;
    return (*this);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Matrix Methods (row-major)
///////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      単位行列を生成します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::CreateIdentity()
{
    return Matrix(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f );
}

//-----------------------------------------------------------------------------
//      単位行列かどうかチェックします.
//-----------------------------------------------------------------------------
inline
bool Matrix::IsIdentity( const Matrix &value )
{
    return (
        IsEqual( value.m[0][0], 1.0f ) && IsZero( value.m[0][1] ) && IsZero( value.m[0][2] ) && IsZero( value.m[0][3] ) &&
        IsZero( value.m[1][0] ) && IsEqual( value.m[1][1], 1.0f ) && IsZero( value.m[1][2] ) && IsZero( value.m[1][3] ) &&
        IsZero( value.m[2][0] ) && IsZero( value.m[2][1] ) && IsEqual( value.m[2][2] , 1.0f ) && IsZero( value.m[2][3] ) &&
        IsZero( value.m[3][0] ) && IsZero( value.m[3][1] ) && IsZero( value.m[3][2] ) && IsEqual( value.m[3][3], 1.0f ) );
}

//-----------------------------------------------------------------------------
//      行列を転置します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::Transpose( const Matrix& value )
{
    return Matrix(
        value._11, value._21, value._31, value._41,
        value._12, value._22, value._32, value._42,
        value._13, value._23, value._33, value._43,
        value._14, value._24, value._34, value._44 );
}

//-----------------------------------------------------------------------------
//      行列を転置します.
//-----------------------------------------------------------------------------
inline
void Matrix::Transpose( const Matrix &value, Matrix &result )
{
    result._11 = value._11;
    result._12 = value._21;
    result._13 = value._31;
    result._14 = value._41;

    result._21 = value._12;
    result._22 = value._22;
    result._23 = value._32;
    result._24 = value._42;

    result._31 = value._13;
    result._32 = value._23;
    result._33 = value._33;
    result._34 = value._34;

    result._41 = value._14;
    result._42 = value._24;
    result._43 = value._34;
    result._44 = value._44;
}

//-----------------------------------------------------------------------------
//      行列同士を乗算します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::Multiply( const Matrix& a, const Matrix& b )
{
    return Matrix(
        ( a._11 * b._11 ) + ( a._12 * b._21 ) + ( a._13 * b._31 ) + ( a._14 * b._41 ),
        ( a._11 * b._12 ) + ( a._12 * b._22 ) + ( a._13 * b._32 ) + ( a._14 * b._42 ),
        ( a._11 * b._13 ) + ( a._12 * b._23 ) + ( a._13 * b._33 ) + ( a._14 * b._43 ),
        ( a._11 * b._14 ) + ( a._12 * b._24 ) + ( a._13 * b._34 ) + ( a._14 * b._44 ),

        ( a._21 * b._11 ) + ( a._22 * b._21 ) + ( a._23 * b._31 ) + ( a._24 * b._41 ),
        ( a._21 * b._12 ) + ( a._22 * b._22 ) + ( a._23 * b._32 ) + ( a._24 * b._42 ),
        ( a._21 * b._13 ) + ( a._22 * b._23 ) + ( a._23 * b._33 ) + ( a._24 * b._43 ),
        ( a._21 * b._14 ) + ( a._22 * b._24 ) + ( a._23 * b._34 ) + ( a._24 * b._44 ),

        ( a._31 * b._11 ) + ( a._32 * b._21 ) + ( a._33 * b._31 ) + ( a._34 * b._41 ),
        ( a._31 * b._12 ) + ( a._32 * b._22 ) + ( a._33 * b._32 ) + ( a._34 * b._42 ),
        ( a._31 * b._13 ) + ( a._32 * b._23 ) + ( a._33 * b._33 ) + ( a._34 * b._43 ),
        ( a._31 * b._14 ) + ( a._32 * b._24 ) + ( a._33 * b._34 ) + ( a._34 * b._44 ),

        ( a._41 * b._11 ) + ( a._42 * b._21 ) + ( a._43 * b._31 ) + ( a._44 * b._41 ),
        ( a._41 * b._12 ) + ( a._42 * b._22 ) + ( a._43 * b._32 ) + ( a._44 * b._42 ),
        ( a._41 * b._13 ) + ( a._42 * b._23 ) + ( a._43 * b._33 ) + ( a._44 * b._43 ),
        ( a._41 * b._14 ) + ( a._42 * b._24 ) + ( a._43 * b._34 ) + ( a._44 * b._44 )
    );
}

//-----------------------------------------------------------------------------
//      行列同士を乗算します.
//-----------------------------------------------------------------------------
inline
void Matrix::Multiply( const Matrix &a, const Matrix &b, Matrix &result )
{
    result._11 = ( a._11 * b._11 ) + ( a._12 * b._21 ) + ( a._13 * b._31 ) + ( a._14 * b._41 );
    result._12 = ( a._11 * b._12 ) + ( a._12 * b._22 ) + ( a._13 * b._32 ) + ( a._14 * b._42 );
    result._13 = ( a._11 * b._13 ) + ( a._12 * b._23 ) + ( a._13 * b._33 ) + ( a._14 * b._43 );
    result._14 = ( a._11 * b._14 ) + ( a._12 * b._24 ) + ( a._13 * b._34 ) + ( a._14 * b._44 );

    result._21 = ( a._21 * b._11 ) + ( a._22 * b._21 ) + ( a._23 * b._31 ) + ( a._24 * b._41 );
    result._22 = ( a._21 * b._12 ) + ( a._22 * b._22 ) + ( a._23 * b._32 ) + ( a._24 * b._42 );
    result._23 = ( a._21 * b._13 ) + ( a._22 * b._23 ) + ( a._23 * b._33 ) + ( a._24 * b._43 );
    result._24 = ( a._21 * b._14 ) + ( a._22 * b._24 ) + ( a._23 * b._34 ) + ( a._24 * b._44 );

    result._31 = ( a._31 * b._11 ) + ( a._32 * b._21 ) + ( a._33 * b._31 ) + ( a._34 * b._41 );
    result._32 = ( a._31 * b._12 ) + ( a._32 * b._22 ) + ( a._33 * b._32 ) + ( a._34 * b._42 );
    result._33 = ( a._31 * b._13 ) + ( a._32 * b._23 ) + ( a._33 * b._33 ) + ( a._34 * b._43 );
    result._34 = ( a._31 * b._14 ) + ( a._32 * b._24 ) + ( a._33 * b._34 ) + ( a._34 * b._44 );

    result._41 = ( a._41 * b._11 ) + ( a._42 * b._21 ) + ( a._43 * b._31 ) + ( a._44 * b._41 );
    result._42 = ( a._41 * b._12 ) + ( a._42 * b._22 ) + ( a._43 * b._32 ) + ( a._44 * b._42 );
    result._43 = ( a._41 * b._13 ) + ( a._42 * b._23 ) + ( a._43 * b._33 ) + ( a._44 * b._43 );
    result._44 = ( a._41 * b._14 ) + ( a._42 * b._24 ) + ( a._43 * b._34 ) + ( a._44 * b._44 );
}

//-----------------------------------------------------------------------------
//      スカラー乗算します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::Multiply( const Matrix& value, float scaleFactor )
{
    return Matrix(
        value._11 * scaleFactor, value._12 * scaleFactor, value._13 * scaleFactor, value._14 * scaleFactor,
        value._21 * scaleFactor, value._22 * scaleFactor, value._23 * scaleFactor, value._24 * scaleFactor,
        value._31 * scaleFactor, value._32 * scaleFactor, value._33 * scaleFactor, value._34 * scaleFactor,
        value._41 * scaleFactor, value._42 * scaleFactor, value._43 * scaleFactor, value._44 * scaleFactor );
}

//-----------------------------------------------------------------------------
//      スカラー乗算します.
//-----------------------------------------------------------------------------
inline
void Matrix::Multiply( const Matrix &value, float scaleFactor, Matrix &result )
{
    result._11 = value._11 * scaleFactor;
    result._12 = value._12 * scaleFactor;
    result._13 = value._13 * scaleFactor;
    result._14 = value._14 * scaleFactor;

    result._21 = value._21 * scaleFactor;
    result._22 = value._22 * scaleFactor;
    result._23 = value._23 * scaleFactor;
    result._24 = value._24 * scaleFactor;

    result._31 = value._31 * scaleFactor;
    result._32 = value._32 * scaleFactor;
    result._33 = value._33 * scaleFactor;
    result._34 = value._34 * scaleFactor;

    result._41 = value._41 * scaleFactor;
    result._42 = value._42 * scaleFactor;
    result._43 = value._43 * scaleFactor;
    result._44 = value._44 * scaleFactor;
}

//-----------------------------------------------------------------------------
//      行列同士を乗算し，乗算結果を転置します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::MultiplyTranspose( const Matrix& a, const Matrix& b )
{
    return Matrix(
        ( a._11 * b._11 ) + ( a._12 * b._21 ) + ( a._13 * b._31 ) + ( a._14 * b._41 ),
        ( a._21 * b._11 ) + ( a._22 * b._21 ) + ( a._23 * b._31 ) + ( a._24 * b._41 ),
        ( a._31 * b._11 ) + ( a._32 * b._21 ) + ( a._33 * b._31 ) + ( a._34 * b._41 ),
        ( a._41 * b._11 ) + ( a._42 * b._21 ) + ( a._43 * b._31 ) + ( a._44 * b._41 ),

        ( a._11 * b._12 ) + ( a._12 * b._22 ) + ( a._13 * b._32 ) + ( a._14 * b._42 ),
        ( a._21 * b._12 ) + ( a._22 * b._22 ) + ( a._23 * b._32 ) + ( a._24 * b._42 ),
        ( a._31 * b._12 ) + ( a._32 * b._22 ) + ( a._33 * b._32 ) + ( a._34 * b._42 ),
        ( a._41 * b._12 ) + ( a._42 * b._22 ) + ( a._43 * b._32 ) + ( a._44 * b._42 ),

        ( a._11 * b._13 ) + ( a._12 * b._23 ) + ( a._13 * b._33 ) + ( a._14 * b._43 ),
        ( a._21 * b._13 ) + ( a._22 * b._23 ) + ( a._23 * b._33 ) + ( a._24 * b._43 ),
        ( a._31 * b._13 ) + ( a._32 * b._23 ) + ( a._33 * b._33 ) + ( a._34 * b._43 ),
        ( a._41 * b._13 ) + ( a._42 * b._23 ) + ( a._43 * b._33 ) + ( a._44 * b._43 ),

        ( a._11 * b._14 ) + ( a._12 * b._24 ) + ( a._13 * b._34 ) + ( a._14 * b._44 ),
        ( a._21 * b._14 ) + ( a._22 * b._24 ) + ( a._23 * b._34 ) + ( a._24 * b._44 ),
        ( a._31 * b._14 ) + ( a._32 * b._24 ) + ( a._33 * b._34 ) + ( a._34 * b._44 ),
        ( a._41 * b._14 ) + ( a._42 * b._24 ) + ( a._43 * b._34 ) + ( a._44 * b._44 )
    );
}

//-----------------------------------------------------------------------------
//      行列同士を乗算し，乗算結果を転置します.
//-----------------------------------------------------------------------------
inline
void Matrix::MultiplyTranspose( const Matrix &a, const Matrix &b, Matrix &result )
{
    result._11 = ( a._11 * b._11 ) + ( a._12 * b._21 ) + ( a._13 * b._31 ) + ( a._14 * b._41 );
    result._21 = ( a._11 * b._12 ) + ( a._12 * b._22 ) + ( a._13 * b._32 ) + ( a._14 * b._42 );
    result._31 = ( a._11 * b._13 ) + ( a._12 * b._23 ) + ( a._13 * b._33 ) + ( a._14 * b._43 );
    result._41 = ( a._11 * b._14 ) + ( a._12 * b._24 ) + ( a._13 * b._34 ) + ( a._14 * b._44 );

    result._12 = ( a._21 * b._11 ) + ( a._22 * b._21 ) + ( a._23 * b._31 ) + ( a._24 * b._41 );
    result._22 = ( a._21 * b._12 ) + ( a._22 * b._22 ) + ( a._23 * b._32 ) + ( a._24 * b._42 );
    result._32 = ( a._21 * b._13 ) + ( a._22 * b._23 ) + ( a._23 * b._33 ) + ( a._24 * b._43 );
    result._42 = ( a._21 * b._14 ) + ( a._22 * b._24 ) + ( a._23 * b._34 ) + ( a._24 * b._44 );

    result._13 = ( a._31 * b._11 ) + ( a._32 * b._21 ) + ( a._33 * b._31 ) + ( a._34 * b._41 );
    result._23 = ( a._31 * b._12 ) + ( a._32 * b._22 ) + ( a._33 * b._32 ) + ( a._34 * b._42 );
    result._33 = ( a._31 * b._13 ) + ( a._32 * b._23 ) + ( a._33 * b._33 ) + ( a._34 * b._43 );
    result._43 = ( a._31 * b._14 ) + ( a._32 * b._24 ) + ( a._33 * b._34 ) + ( a._34 * b._44 );

    result._14 = ( a._41 * b._11 ) + ( a._42 * b._21 ) + ( a._43 * b._31 ) + ( a._44 * b._41 );
    result._24 = ( a._41 * b._12 ) + ( a._42 * b._22 ) + ( a._43 * b._32 ) + ( a._44 * b._42 );
    result._34 = ( a._41 * b._13 ) + ( a._42 * b._23 ) + ( a._43 * b._33 ) + ( a._44 * b._43 );
    result._44 = ( a._41 * b._14 ) + ( a._42 * b._24 ) + ( a._43 * b._34 ) + ( a._44 * b._44 );
}

//-----------------------------------------------------------------------------
//      逆行列を求めます.
//-----------------------------------------------------------------------------
inline 
Matrix Matrix::Invert( const Matrix& value )
{
    auto det = value.Determinant();
    assert( !IsZero( det ) );

    auto m11 = value._22*value._33*value._44 + value._23*value._34*value._42 + value._24*value._32*value._43 - value._22*value._34*value._43 - value._23*value._32*value._44 - value._24*value._33*value._42;
    auto m12 = value._12*value._34*value._43 + value._13*value._32*value._44 + value._14*value._33*value._42 - value._12*value._33*value._44 - value._13*value._34*value._42 - value._14*value._32*value._43;
    auto m13 = value._12*value._23*value._44 + value._13*value._24*value._42 + value._14*value._22*value._43 - value._12*value._24*value._43 - value._13*value._22*value._44 - value._14*value._23*value._42;
    auto m14 = value._12*value._24*value._33 + value._13*value._22*value._34 + value._14*value._23*value._32 - value._12*value._23*value._34 - value._13*value._24*value._32 - value._14*value._22*value._33;

    auto m21 = value._21*value._34*value._43 + value._23*value._31*value._44 + value._24*value._33*value._41 - value._21*value._33*value._44 - value._23*value._34*value._41 - value._24*value._31*value._43;
    auto m22 = value._11*value._33*value._44 + value._13*value._34*value._41 + value._14*value._31*value._43 - value._11*value._34*value._43 - value._13*value._31*value._44 - value._14*value._33*value._41;
    auto m23 = value._11*value._24*value._43 + value._13*value._21*value._44 + value._14*value._23*value._41 - value._11*value._23*value._44 - value._13*value._24*value._41 - value._14*value._21*value._43;
    auto m24 = value._11*value._23*value._34 + value._13*value._24*value._31 + value._14*value._21*value._33 - value._11*value._24*value._33 - value._13*value._21*value._34 - value._14*value._23*value._31;

    auto m31 = value._21*value._32*value._44 + value._22*value._34*value._41 + value._24*value._31*value._42 - value._21*value._34*value._42 - value._22*value._31*value._44 - value._24*value._32*value._41;
    auto m32 = value._11*value._34*value._42 + value._12*value._31*value._44 + value._14*value._32*value._41 - value._11*value._32*value._44 - value._12*value._34*value._41 - value._14*value._31*value._42;
    auto m33 = value._11*value._22*value._44 + value._12*value._24*value._41 + value._14*value._21*value._42 - value._11*value._24*value._42 - value._12*value._21*value._44 - value._14*value._22*value._41;
    auto m34 = value._11*value._24*value._32 + value._12*value._21*value._34 + value._14*value._22*value._31 - value._11*value._22*value._34 - value._12*value._24*value._31 - value._14*value._21*value._32;

    auto m41 = value._21*value._33*value._42 + value._22*value._31*value._43 + value._23*value._32*value._41 - value._21*value._32*value._43 - value._22*value._33*value._41 - value._23*value._31*value._42;
    auto m42 = value._11*value._32*value._43 + value._12*value._33*value._41 + value._13*value._31*value._42 - value._11*value._33*value._42 - value._12*value._31*value._43 - value._13*value._32*value._41;
    auto m43 = value._11*value._23*value._42 + value._12*value._21*value._43 + value._13*value._22*value._41 - value._11*value._22*value._43 - value._12*value._23*value._41 - value._13*value._21*value._42;
    auto m44 = value._11*value._22*value._33 + value._12*value._23*value._31 + value._13*value._21*value._32 - value._11*value._23*value._32 - value._12*value._21*value._33 - value._13*value._22*value._31;

    return Matrix(
        m11 / det, m12 / det, m13 / det, m14 / det,
        m21 / det, m22 / det, m23 / det, m24 / det,
        m31 / det, m32 / det, m33 / det, m34 / det,
        m41 / det, m42 / det, m43 / det, m44 / det );
}

//-----------------------------------------------------------------------------
//      逆行列を求めます.
//-----------------------------------------------------------------------------
inline
void Matrix::Invert( const Matrix &value, Matrix &result )
{ 
    auto det = value.Determinant();
    assert( det != 0.0f );

    result._11 = value._22*value._33*value._44 + value._23*value._34*value._42 + value._24*value._32*value._43 - value._22*value._34*value._43 - value._23*value._32*value._44 - value._24*value._33*value._42;
    result._12 = value._12*value._34*value._43 + value._13*value._32*value._44 + value._14*value._33*value._42 - value._12*value._33*value._44 - value._13*value._34*value._42 - value._14*value._32*value._43;
    result._13 = value._12*value._23*value._44 + value._13*value._24*value._42 + value._14*value._22*value._43 - value._12*value._24*value._43 - value._13*value._22*value._44 - value._14*value._23*value._42;
    result._14 = value._12*value._24*value._33 + value._13*value._22*value._34 + value._14*value._23*value._32 - value._12*value._23*value._34 - value._13*value._24*value._32 - value._14*value._22*value._33;

    result._21 = value._21*value._34*value._43 + value._23*value._31*value._44 + value._24*value._33*value._41 - value._21*value._33*value._44 - value._23*value._34*value._41 - value._24*value._31*value._43;
    result._22 = value._11*value._33*value._44 + value._13*value._34*value._41 + value._14*value._31*value._43 - value._11*value._34*value._43 - value._13*value._31*value._44 - value._14*value._33*value._41;
    result._23 = value._11*value._24*value._43 + value._13*value._21*value._44 + value._14*value._23*value._41 - value._11*value._23*value._44 - value._13*value._24*value._41 - value._14*value._21*value._43;
    result._24 = value._11*value._23*value._34 + value._13*value._24*value._31 + value._14*value._21*value._33 - value._11*value._24*value._33 - value._13*value._21*value._34 - value._14*value._23*value._31;

    result._31 = value._21*value._32*value._44 + value._22*value._34*value._41 + value._24*value._31*value._42 - value._21*value._34*value._42 - value._22*value._31*value._44 - value._24*value._32*value._41;
    result._32 = value._11*value._34*value._42 + value._12*value._31*value._44 + value._14*value._32*value._41 - value._11*value._32*value._44 - value._12*value._34*value._41 - value._14*value._31*value._42;
    result._33 = value._11*value._22*value._44 + value._12*value._24*value._41 + value._14*value._21*value._42 - value._11*value._24*value._42 - value._12*value._21*value._44 - value._14*value._22*value._41;
    result._34 = value._11*value._24*value._32 + value._12*value._21*value._34 + value._14*value._22*value._31 - value._11*value._22*value._34 - value._12*value._24*value._31 - value._14*value._21*value._32;

    result._41 = value._21*value._33*value._42 + value._22*value._31*value._43 + value._23*value._32*value._41 - value._21*value._32*value._43 - value._22*value._33*value._41 - value._23*value._31*value._42;
    result._42 = value._11*value._32*value._43 + value._12*value._33*value._41 + value._13*value._31*value._42 - value._11*value._33*value._42 - value._12*value._31*value._43 - value._13*value._32*value._41;
    result._43 = value._11*value._23*value._42 + value._12*value._21*value._43 + value._13*value._22*value._41 - value._11*value._22*value._43 - value._12*value._23*value._41 - value._13*value._21*value._42;
    result._44 = value._11*value._22*value._33 + value._12*value._23*value._31 + value._13*value._21*value._32 - value._11*value._23*value._32 - value._12*value._21*value._33 - value._13*value._22*value._31;

    result._11 /= det;
    result._12 /= det;
    result._13 /= det;
    result._14 /= det;

    result._21 /= det;
    result._22 /= det;
    result._23 /= det;
    result._24 /= det;

    result._31 /= det;
    result._32 /= det;
    result._33 /= det;
    result._34 /= det;

    result._41 /= det;
    result._42 /= det;
    result._43 /= det;
    result._44 /= det;
}

//-----------------------------------------------------------------------------
//      拡大・縮小行列を生成します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::CreateScale( float scale )
{
    return Matrix(
        scale, 0.0f, 0.0f, 0.0f,
        0.0f, scale, 0.0f, 0.0f,
        0.0f, 0.0f, scale, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f );
}

//-----------------------------------------------------------------------------
//      拡大・縮小行列を生成します.
//-----------------------------------------------------------------------------
inline 
void Matrix::CreateScale( float scale, Matrix &result )
{
    result._11 = scale;    result._12 = 0.0f;    result._13 = 0.0f;     result._14 = 0.0f;
    result._21 = 0.0f;     result._22 = scale;   result._23 = 0.0f;     result._24 = 0.0f;
    result._31 = 0.0f;     result._32 = 0.0f;    result._33 = scale;    result._34 = 0.0f;
    result._41 = 0.0f;     result._42 = 0.0f;    result._43 = 0.0f;     result._44 = 1.0f;
}

//-----------------------------------------------------------------------------
//      拡大・縮小行列を生成します.
//-----------------------------------------------------------------------------
inline 
Matrix Matrix::CreateScale( float xScale, float yScale, float zScale )
{
    return Matrix(
        xScale, 0.0f,   0.0f,   0.0f,
        0.0f,   yScale, 0.0f,   0.0f,
        0.0f,   0.0f,   zScale, 0.0f,
        0.0f,   0.0f,   0.0f,   1.0f );
}

//-----------------------------------------------------------------------------
//      拡大・縮小行列を生成します.
//-----------------------------------------------------------------------------
inline
void Matrix::CreateScale( float xScale, float yScale, float zScale, Matrix &result )
{
    result._11 = xScale;    result._12 = 0.0f;      result._13 = 0.0f;      result._14 = 0.0f;
    result._21 = 0.0f;      result._22 = yScale;    result._23 = 0.0f;      result._24 = 0.0f;
    result._31 = 0.0f;      result._32 = 0.0f;      result._33 = zScale;    result._34 = 0.0f;
    result._41 = 0.0f;      result._42 = 0.0f;      result._43 = 0.0f;      result._44 = 1.0f;
}

//-----------------------------------------------------------------------------
//      拡大・縮小行列を生成します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::CreateScale( const Vector3& scales )
{
    return Matrix(
        scales.x,   0.0f,       0.0f,       0.0f,
        0.0f,       scales.y,   0.0f,       0.0f,
        0.0f,       0.0f,       scales.z,   0.0f,
        0.0f,       0.0f,       0.0f,       1.0f ); 
}

//-----------------------------------------------------------------------------
//      拡大・縮小行列を生成します.
//-----------------------------------------------------------------------------
inline
void Matrix::CreateScale( const Vector3 &scales, Matrix &result )
{
    result._11 = scales.x;  result._12 = 0.0f;      result._13 = 0.0f;      result._14 = 0.0f;
    result._21 = 0.0f;      result._22 = scales.y;  result._23 = 0.0f;      result._24 = 0.0f;
    result._31 = 0.0f;      result._32 = 0.0f;      result._33 = scales.z;  result._34 = 0.0f;
    result._41 = 0.0f;      result._42 = 0.0f;      result._43 = 0.0f;      result._44 = 1.0f;
}

//-----------------------------------------------------------------------------
//      平行移動行列を生成します.
//-----------------------------------------------------------------------------
inline 
Matrix Matrix::CreateTranslation( float xPos, float yPos, float zPos )
{
    return Matrix(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        xPos, yPos, zPos, 1.0f );
}

//-----------------------------------------------------------------------------
//      平行移動行列を生成します.
//-----------------------------------------------------------------------------
inline
void Matrix::CreateTranslation( float xPos, float yPos, float zPos, Matrix &result )
{
    result._11 = 1.0f;  result._12 = 0.0f;  result._13 = 0.0f;  result._14 = 0.0f;
    result._21 = 0.0f;  result._22 = 1.0f;  result._23 = 0.0f;  result._24 = 0.0f;
    result._31 = 0.0f;  result._32 = 0.0f;  result._33 = 1.0f;  result._34 = 0.0f;
    result._41 = xPos;  result._42 = yPos;  result._43 = zPos;  result._44 = 1.0f;
}

//-----------------------------------------------------------------------------
//      平行移動行列を生成します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::CreateTranslation( const Vector3& pos )
{
    return Matrix(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        pos.x, pos.y, pos.z, 1.0f );
}

//-----------------------------------------------------------------------------
//      平行移動行列を生成します.
//-----------------------------------------------------------------------------
inline
void Matrix::CreateTranslation( const Vector3 &pos, Matrix &result )
{
    result._11 = 1.0f;  result._12 = 0.0f;  result._13 = 0.0f;  result._14 = 0.0f;
    result._21 = 0.0f;  result._22 = 1.0f;  result._23 = 0.0f;  result._24 = 0.0f;
    result._31 = 0.0f;  result._32 = 0.0f;  result._33 = 1.0f;  result._34 = 0.0f;
    result._41 = pos.x; result._42 = pos.y; result._43 = pos.z; result._44 = 1.0f;
}

//-----------------------------------------------------------------------------
//      X軸周りの回転行列を生成します.
//-----------------------------------------------------------------------------
inline 
Matrix Matrix::CreateRotationX( const float radian )
{
    auto cosRad = cosf(radian);
    auto sinRad = sinf(radian);
    return Matrix(
        1.0f,   0.0f,   0.0f,   0.0f,
        0.0f,   cosRad, sinRad, 0.0f,
        0.0f,  -sinRad, cosRad, 0.0f,
        0.0f,   0.0f,   0.0f,   1.0f );
}

//-----------------------------------------------------------------------------
//      X軸周りの回転行列を生成します.
//-----------------------------------------------------------------------------
inline 
void Matrix::CreateRotationX( const float radian, Matrix &result )
{
    auto cosRad = cosf( radian );
    auto sinRad = sinf( radian );

    result._11 = 1.0f;      result._12 = 0.0f;      result._13 = 0.0f;      result._14 = 0.0f;
    result._21 = 0.0f;      result._22 = cosRad;    result._23 = sinRad;    result._24 = 0.0f;
    result._31 = 0.0f;      result._32 = -sinRad;   result._33 = cosRad;    result._34 = 0.0f;
    result._41 = 0.0f;      result._42 = 0.0f;      result._43 = 0.0f;      result._44 = 1.0f;
}

//-----------------------------------------------------------------------------
//      Y軸周りの回転行列を生成します.
//-----------------------------------------------------------------------------
inline 
Matrix Matrix::CreateRotationY( const float radian )
{
    auto cosRad = cosf( radian );
    auto sinRad = sinf( radian );

    return Matrix(
        cosRad, 0.0f,  -sinRad, 0.0f,
        0.0f,   1.0f,   0.0f,   0.0f,
        sinRad, 0.0f,   cosRad, 0.0f,
        0.0f,   0.0f,   0.0f,   1.0f );
}

//-----------------------------------------------------------------------------
//      Y軸周りの回転行列を生成します.
//-----------------------------------------------------------------------------
inline
void Matrix::CreateRotationY( const float radian, Matrix &result )
{
    auto cosRad = cosf( radian );
    auto sinRad = sinf( radian );

    result._11 = cosRad;    result._12 = 0.0f;  result._13 = -sinRad;   result._14 = 0.0f;
    result._21 = 0.0f;      result._22 = 1.0f;  result._23 = 0.0f;      result._24 = 0.0f;
    result._31 = sinRad;    result._32 = 0.0f;  result._33 = cosRad;    result._34 = 0.0f;
    result._41 = 0.0f;      result._42 = 0.0f;  result._43 = 0.0f;      result._44 = 1.0f;
}

//-----------------------------------------------------------------------------
//      Z軸周りの回転行列を生成します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::CreateRotationZ( const float radian )
{
    auto cosRad = cosf( radian );
    auto sinRad = sinf( radian );

    return Matrix( 
        cosRad, sinRad, 0.0f, 0.0f,
       -sinRad, cosRad, 0.0f, 0.0f,
        0.0f,   0.0f,   1.0f, 0.0f,
        0.0f,   0.0f,   0.0f, 1.0f );
}

//-----------------------------------------------------------------------------
//      Z軸周りの回転行列を生成します.
//-----------------------------------------------------------------------------
inline
void Matrix::CreateRotationZ( const float radian, Matrix &result )
{
    auto cosRad = cosf( radian );
    auto sinRad = sinf( radian );

    result._11 = cosRad;    result._12 = sinRad;    result._13 = 0.0f;    result._14 = 0.0f;
    result._21 = -sinRad;   result._22 = cosRad;    result._23 = 0.0f;    result._24 = 0.0f;
    result._31 = 0.0f;      result._32 = 0.0f;      result._33 = 1.0f;    result._34 = 0.0f;
    result._41 = 0.0f;      result._42 = 0.0f;      result._43 = 0.0f;    result._44 = 1.0f;
}

//-----------------------------------------------------------------------------
//      四元数から回転行列を生成します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::CreateFromQuaternion( const Quaternion& qua )
{
    auto xx = qua.x * qua.x; // num
    auto yy = qua.y * qua.y; // num2
    auto zz = qua.z * qua.z; // num3

    auto xy = qua.x * qua.y; // num4
    auto zw = qua.z * qua.w; // num5
    auto zx = qua.z * qua.x; // num6
    
    auto yw = qua.y * qua.w; // num7
    auto yz = qua.y * qua.z; // num8
    auto xw = qua.x * qua.w; // num9

    return Matrix(
        1.0f - 2.0f * (yy + zz),
        2.0f * (xy + zw),
        2.0f * (zx - yw),
        0.0f,

        2.0f * (xy - zw),
        1.0f - 2.0f * (zz + xx), 
        2.0f * (yz + xw),
        0.0f,

        2.0f * (zx + yw), 
        2.0f * (yz - xw),
        1.0f - 2.0f * (yy + xx),
        0.0f,

        0.0f,
        0.0f,
        0.0f,
        1.0f );
}

//-----------------------------------------------------------------------------
//      四元数から回転行列を生成します.
//-----------------------------------------------------------------------------
inline
void Matrix::CreateFromQuaternion( const Quaternion &qua, Matrix &result )
{
    auto xx = qua.x * qua.x; // num
    auto yy = qua.y * qua.y; // num2
    auto zz = qua.z * qua.z; // num3

    auto xy = qua.x * qua.y; // num4
    auto zw = qua.z * qua.w; // num5
    auto zx = qua.z * qua.x; // num6
    
    auto yw = qua.y * qua.w; // num7
    auto yz = qua.y * qua.z; // num8
    auto xw = qua.x * qua.w; // num9

    result._11 = 1.0f - 2.0f * (yy + zz);
    result._12 = 2.0f * (xy + zw);
    result._13 = 2.0f * (zx - yw);
    result._14 = 0.0f;

    result._21 = 2.0f * (xy - zw);
    result._22 = 1.0f - 2.0f * (zz + xx); 
    result._23 = 2.0f * (yz + xw);
    result._24 = 0.0f;

    result._31 = 2.0f * (zx + yw); 
    result._32 = 2.0f * (yz - xw);
    result._33 = 1.0f - 2.0f * (yy + xx);
    result._34 = 0.0f;

    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = 0.0f;
    result._44 = 1.0f;
}

//-----------------------------------------------------------------------------
//      指定された軸と角度から回転行列を求めます.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::CreateFromAxisAngle( const Vector3& axis, float radian )
{
    auto sinRad = sinf(radian);
    auto cosRad = cosf(radian);
    auto a = 1.0f -cosRad;
    
    auto ab = axis.x * axis.y * a;
    auto bc = axis.y * axis.z * a;
    auto ca = axis.z * axis.x * a;
    auto tx = axis.x * axis.x;
    auto ty = axis.y * axis.y;
    auto tz = axis.z * axis.z;

    return Matrix(
        tx + cosRad * (1.0f - tx),
        ab + axis.z * sinRad,
        ca - axis.y * sinRad,
        0.0f,

        ab - axis.z * sinRad,
        ty + cosRad * (1.0f - ty),
        bc + axis.x * sinRad,
        0.0f,

        ca + axis.y * sinRad,
        bc - axis.x * sinRad,
        tz + cosRad * (1.0f - tz),
        0.0f,

        0.0f,
        0.0f,
        0.0f,
        1.0f );
}

//-----------------------------------------------------------------------------
//      指定された軸と角度から回転行列を求めます.
//-----------------------------------------------------------------------------
inline
void Matrix::CreateFromAxisAngle( const Vector3 &axis, float radian, Matrix &result )
{
    auto sinRad = sinf(radian);
    auto cosRad = cosf(radian);
    auto a = 1.0f -cosRad;
    
    auto ab = axis.x * axis.y * a;
    auto bc = axis.y * axis.z * a;
    auto ca = axis.z * axis.x * a;
    auto tx = axis.x * axis.x;
    auto ty = axis.y * axis.y;
    auto tz = axis.z * axis.z;

    result._11 = tx + cosRad * (1.0f - tx);
    result._12 = ab + axis.z * sinRad;
    result._13 = ca - axis.y * sinRad;
    result._14 = 0.0f;

    result._21 = ab - axis.z * sinRad;
    result._22 = ty + cosRad * (1.0f - ty);
    result._23 = bc + axis.x * sinRad;
    result._24 = 0.0f;

    result._31 = ca + axis.y * sinRad;
    result._32 = bc - axis.x * sinRad;
    result._33 = tz + cosRad * (1.0f - tz);
    result._34 = 0.0f;

    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = 0.0f;
    result._44 = 1.0f;
}

//-----------------------------------------------------------------------------
//      ヨー・ピッチ・ロール角から回転行列を生成します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::CreateRotationFromYawPitchRoll( float yaw, float pitch, float roll )
{
    auto value = Quaternion::CreateFromYawPitchRoll( yaw, pitch, roll );
    return Matrix::CreateFromQuaternion( value );
}

//-----------------------------------------------------------------------------
//      ヨー・ピッチ・ロール角から回転行列を生成します.
//-----------------------------------------------------------------------------
inline
void Matrix::CreateRotationFromYawPitchRoll( float yaw, float pitch, float roll, Matrix& result )
{
    auto value = Quaternion::CreateFromYawPitchRoll( yaw, pitch, roll );
    Matrix::CreateFromQuaternion( value, result );
}

//-----------------------------------------------------------------------------
//      注視点を基にビュー行列を生成します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::CreateLookAt
(
    const Vector3& cameraPosition,
    const Vector3& cameraTarget,
    const Vector3& cameraUpVector
)
{
    auto zaxis = cameraPosition - cameraTarget;
    zaxis.Normalize();

    auto xaxis = Vector3::Cross(cameraUpVector, zaxis);
    xaxis.Normalize();

    auto yaxis = Vector3::Cross(zaxis, xaxis);
    yaxis.Normalize();

    return Matrix(
        xaxis.x, yaxis.x, zaxis.x, 0.0f,
        xaxis.y, yaxis.y, zaxis.y, 0.0f,
        xaxis.z, yaxis.z, zaxis.z, 0.0f,
        -Vector3::Dot(xaxis, cameraPosition),
        -Vector3::Dot(yaxis, cameraPosition),
        -Vector3::Dot(zaxis, cameraPosition),
        1.0f);
}

//-----------------------------------------------------------------------------
//      注視点を基にビュー行列を生成します.
//-----------------------------------------------------------------------------
inline
void Matrix::CreateLookAt
(
    const Vector3&  cameraPosition,
    const Vector3&  cameraTarget,
    const Vector3&  cameraUpVector,
    Matrix&         result
)
{
    auto zaxis = cameraPosition - cameraTarget;
    zaxis.Normalize();

    auto xaxis = Vector3::Cross(cameraUpVector, zaxis);
    xaxis.Normalize();

    auto yaxis = Vector3::Cross(zaxis, xaxis);
    yaxis.Normalize();

    result._11 = xaxis.x;
    result._12 = yaxis.x;
    result._13 = zaxis.x;
    result._14 = 0.0f;

    result._21 = xaxis.y;
    result._22 = yaxis.y;
    result._23 = zaxis.y;
    result._24 = 0.0f;

    result._31 = xaxis.z;
    result._32 = yaxis.z;
    result._33 = zaxis.z;
    result._34 = 0.0f;

    result._41 = -Vector3::Dot(xaxis, cameraPosition);
    result._42 = -Vector3::Dot(yaxis, cameraPosition);
    result._43 = -Vector3::Dot(zaxis, cameraPosition);
    result._44 = 1.0f;
}

//-----------------------------------------------------------------------------
//      視線ベクトルを基にビュー行列を生成します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::CreateLookTo
(
    const Vector3& cameraPosition,
    const Vector3& cameraDir,
    const Vector3& cameraUpVector
)
{
    auto zaxis = -cameraDir;
    zaxis.Normalize();

    auto xaxis = Vector3::Cross(cameraUpVector, zaxis);
    xaxis.Normalize();

    auto yaxis = Vector3::Cross(zaxis, xaxis);
    yaxis.Normalize();

    return Matrix(
        xaxis.x, yaxis.x, zaxis.x, 0.0f,
        xaxis.y, yaxis.y, zaxis.y, 0.0f,
        xaxis.z, yaxis.z, zaxis.z, 0.0f,
        -Vector3::Dot(xaxis, cameraPosition),
        -Vector3::Dot(yaxis, cameraPosition),
        -Vector3::Dot(zaxis, cameraPosition),
        1.0f);
}

//-----------------------------------------------------------------------------
//      視線ベクトルを基にビュー行列を生成します.
//-----------------------------------------------------------------------------
inline
void Matrix::CreateLookTo
(
    const Vector3&  cameraPosition,
    const Vector3&  cameraDir,
    const Vector3&  cameraUpVector,
    Matrix&         result
)
{
    auto zaxis = -cameraDir;
    zaxis.Normalize();

    auto xaxis = Vector3::Cross(cameraUpVector, zaxis);
    xaxis.Normalize();

    auto yaxis = Vector3::Cross(zaxis, xaxis);
    yaxis.Normalize();

    result._11 = xaxis.x;
    result._12 = yaxis.x;
    result._13 = zaxis.x;
    result._14 = 0.0f;

    result._21 = xaxis.y;
    result._22 = yaxis.y;
    result._23 = zaxis.y;
    result._24 = 0.0f;

    result._31 = xaxis.z;
    result._32 = yaxis.z;
    result._33 = zaxis.z;
    result._34 = 0.0f;

    result._41 = -Vector3::Dot(xaxis, cameraPosition);
    result._42 = -Vector3::Dot(yaxis, cameraPosition);
    result._43 = -Vector3::Dot(zaxis, cameraPosition);
    result._44 = 1.0f;
}

//-----------------------------------------------------------------------------
//      透視投影行列を生成します.
//-----------------------------------------------------------------------------
inline 
Matrix Matrix::CreatePerspective
(
    float width,
    float height,
    float nearClip,
    float farClip
)
{
    assert( !IsZero( width ) );
    assert( !IsZero( height ) );
    assert( !IsZero( nearClip - farClip ));
    auto range = farClip / (nearClip - farClip);

    return Matrix(
        2.0f * nearClip / width, 
        0.0f, 
        0.0f,
        0.0f,

        0.0f,
        2.0f * nearClip / height,
        0.0f,
        0.0f,

        0.0f,
        0.0f,
        range,
        -1.0f,

        0.0f,
        0.0f,
        range * nearClip,
        0.0f);
}

//-----------------------------------------------------------------------------
//      透視投影行列を生成します.
//-----------------------------------------------------------------------------
inline
void Matrix::CreatePerspective
(
    float     width,
    float     height,
    float     nearClip,
    float     farClip,
    Matrix& result
)
{
    assert( !IsZero( width ) );
    assert( !IsZero( height ) );
    assert( !IsZero( nearClip - farClip ) );
    auto range = farClip / (nearClip - farClip);

    result._11 = 2.0f * nearClip / width;
    result._12 = 0.0f;
    result._13 = 0.0f;
    result._14 = 0.0f;

    result._21 = 0.0f;
    result._22 = 2.0f * nearClip / height;
    result._23 = 0.0f;
    result._24 = 0.0f;

    result._31 = 0.0f;
    result._32 = 0.0f;
    result._33 = range;
    result._34 = -1.0f;
    
    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = range * nearClip;
    result._44 = 0.0f;
}

//-----------------------------------------------------------------------------
//      視野角に基づいて透視投影行列を生成します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::CreatePerspectiveFieldOfView
(
    float fieldOfView,
    float aspectRatio,
    float nearClip,
    float farClip
)
{
    assert( !IsZero( aspectRatio ) );
    assert( !IsZero( nearClip - farClip ) );
    auto sinFov = std::sin( 0.5f * fieldOfView );
    auto cosFov = std::cos( 0.5f * fieldOfView );
    auto height = cosFov / sinFov;
    auto width  = height / aspectRatio;
    auto range  = farClip / ( nearClip - farClip );

    return Matrix(
        width,
        0.0f,
        0.0f,
        0.0f,

        0.0f,
        height,
        0.0f,
        0.0f,

        0.0f,
        0.0f,
        range,
        -1.0f,

        0.0f,
        0.0f,
        range * nearClip,
        0.0f );
}

//-----------------------------------------------------------------------------
//      視野角に基づいて透視投影行列を生成します.
//-----------------------------------------------------------------------------
inline
void Matrix::CreatePerspectiveFieldOfView
( 
    float     fieldOfView,
    float     aspectRatio,
    float     nearClip,
    float     farClip,
    Matrix& result
)
{
    assert( !IsZero( aspectRatio ) );
    assert( !IsZero( nearClip - farClip ) );
    auto sinFov = std::sin( 0.5f * fieldOfView );
    auto cosFov = std::cos( 0.5f * fieldOfView );
    auto height = cosFov / sinFov;
    auto width  = height / aspectRatio;
    auto range  = farClip / ( nearClip - farClip );

    result._11 = width;
    result._12 = 0.0f;
    result._13 = 0.0f;
    result._14 = 0.0f;

    result._21 = 0.0f;
    result._22 = height;
    result._23 = 0.0f;
    result._24 = 0.0f;

    result._31 = 0.0f;
    result._32 = 0.0f;
    result._33 = range;
    result._34 = -1.0f;

    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = range * nearClip;
    result._44 = 0.0f;
}

//-----------------------------------------------------------------------------
//      視野角に基づいてReverse-Z透視投影行列を生成します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::CreatePerspectiveFieldOfViewReverseZ
(
    float fieldOfView,
    float aspectRatio,
    float nearClip
)
{
    assert( !IsZero( aspectRatio ) );
    auto sinFov = std::sin( 0.5f * fieldOfView );
    auto cosFov = std::cos( 0.5f * fieldOfView );
    auto height = cosFov / sinFov;
    auto width  = height / aspectRatio;

    return Matrix(
        width,
        0.0f,
        0.0f,
        0.0f,

        0.0f,
        height,
        0.0f,
        0.0f,

        0.0f,
        0.0f,
        0.0f,
        -1.0f,

        0.0f,
        0.0f,
        nearClip,
        0.0f );
}


//-----------------------------------------------------------------------------
//      視野角に基づいてReverse-Z透視投影行列を生成します.
//-----------------------------------------------------------------------------
inline
void Matrix::CreatePerspectiveFieldOfViewReverseZ
(
    float   fieldOfView,
    float   aspectRatio,
    float   nearClip,
    Matrix& result
)
{
    assert( !IsZero( aspectRatio ) );
    auto sinFov = std::sin( 0.5f * fieldOfView );
    auto cosFov = std::cos( 0.5f * fieldOfView );
    auto height = cosFov / sinFov;
    auto width  = height / aspectRatio;

    result._11 = width;
    result._12 = 0.0f;
    result._13 = 0.0f;
    result._14 = 0.0f;

    result._21 = 0.0f;
    result._22 = height;
    result._23 = 0.0f;
    result._24 = 0.0f;

    result._31 = 0.0f;
    result._32 = 0.0f;
    result._33 = 0.0f;
    result._34 = -1.0f;

    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = nearClip;
    result._44 = 0.0f;
}

//-----------------------------------------------------------------------------
//      カスタマイズした透視投影行列を生成します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::CreatePerspectiveOffCenter
(
    float left,
    float right,
    float bottom,
    float top,
    float nearClip,
    float farClip
)
{
    auto width  = right - left;
    auto height = top - bottom;
    auto depth  = nearClip - farClip;
    assert( !IsZero( width ) );
    assert( !IsZero( height ) );
    assert( !IsZero( depth ) );

    return Matrix(
        2.0f * nearClip / width,
        0.0f,
        0.0f,
        0.0f,

        0.0f,
        2.0f * nearClip / height,
        0.0f,
        0.0f,

        (left + right) / width,
        (top + bottom) / height,
        farClip / depth,
        -1.0f,

        0.0f,
        0.0f,
        nearClip * farClip/ depth,
        0.0f );
}

//-----------------------------------------------------------------------------
//      カスタマイズした透視投影行列を生成します.
//-----------------------------------------------------------------------------
inline
void Matrix::CreatePerspectiveOffCenter
(
    float     left,
    float     right,
    float     bottom,
    float     top,
    float     nearClip,
    float     farClip,
    Matrix& result
)
{
    auto width  = right - left;
    auto height = top - bottom;
    auto depth  = nearClip - farClip;
    assert( !IsZero( width ) );
    assert( !IsZero( height ) );
    assert( !IsZero( depth ) );

    result._11 = 2.0f * nearClip / width;
    result._12 = 0.0f;
    result._13 = 0.0f;
    result._14 = 0.0f;

    result._21 = 0.0f;
    result._22 = 2.0f * nearClip / height;
    result._23 = 0.0f;
    result._24 = 0.0f;

    result._31 = (left + right) / width;
    result._32 = (top + bottom) / height;
    result._33 = farClip / depth;
    result._34 = -1.0f;

    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = nearClip * farClip/ depth;
    result._44 = 0.0f;
}

//-----------------------------------------------------------------------------
//      正射影行列を生成します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::CreateOrthographic
(
    float width,
    float height,
    float nearClip,
    float farClip
)
{
    assert( !IsZero( width ) );
    assert( !IsZero( height ) );
    assert( !IsZero( nearClip - farClip ) );
    auto range = 1.0f / (nearClip - farClip);

    return Matrix(
        2.0f / width,
        0.0f,
        0.0f,
        0.0f,
    
        0.0f,
        2.0f / height,
        0.0f,
        0.0f,

        0.0f,
        0.0f,
        range,
        0.0f,

        0.0f,
        0.0f,
        range * nearClip,
        1.0f );
}

//-----------------------------------------------------------------------------
//      正射影行列を生成します.
//-----------------------------------------------------------------------------
inline
void Matrix::CreateOrthographic
(
    float     width,
    float     height,
    float     nearClip,
    float     farClip,
    Matrix& result
)
{
    assert( !IsZero( width ) );
    assert( !IsZero( height ) );
    assert( !IsZero( nearClip - farClip ) );
    auto range = 1.0f / (nearClip - farClip);

    result._11 = 2.0f / width;
    result._12 = 0.0f;
    result._13 = 0.0f;
    result._14 = 0.0f;
    
    result._21 = 0.0f;
    result._22 = 2.0f / height;
    result._23 = 0.0f;
    result._24 = 0.0f;

    result._31 = 0.0f;
    result._32 = 0.0f;
    result._33 = range;
    result._34 = 0.0f;

    result._41 = 0.0f;
    result._42 = 0.0f;
    result._43 = range * nearClip;
    result._44 = 1.0f;
}

//-----------------------------------------------------------------------------
//      カスタマイズした正射影行列を生成します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::CreateOrthographicOffCenter
(
    float left,
    float right,
    float bottom,
    float top,
    float nearClip,
    float farClip
)
{
    auto width  = right - left;
    auto height = bottom - top;
    auto depth  = nearClip - farClip;
    assert( !IsZero( width ) );
    assert( !IsZero( height ) );
    assert( !IsZero( depth ) );

    return Matrix(
        2.0f / width,
        0.0f,
        0.0f,
        0.0f,

        0.0f,
        2.0f / height,
        0.0f,
        0.0f,

        0.0f,
        0.0f,
        1.0f / depth,
        0.0f,

        (left + right) / width,
        (top + bottom) / height,
        nearClip / depth,
        1.0f
    );
}

//-----------------------------------------------------------------------------
//      カスタマイズした正射影行列を生成します.
//-----------------------------------------------------------------------------
inline
void Matrix::CreateOrthographicOffCenter
(
    float     left,
    float     right,
    float     bottom,
    float     top,
    float     nearClip,
    float     farClip,
    Matrix& result
)
{
    auto width  = right - left;
    auto height = bottom - top;
    auto depth  = nearClip - farClip;
    assert( !IsZero( width ) );
    assert( !IsZero( height ) );
    assert( !IsZero( depth ) );

    result._11 = 2.0f / width;
    result._12 = 0.0f;
    result._13 = 0.0f;
    result._14 = 0.0f;

    result._21 = 0.0f;
    result._22 = 2.0f / height;
    result._23 = 0.0f;
    result._24 = 0.0f;

    result._31 = 0.0f;
    result._32 = 0.0f;
    result._33 = 1.0f / depth;
    result._34 = 0.0f;

    result._41 = (left + right) / width;
    result._42 = (top + bottom) / height;
    result._43 = nearClip / depth;
    result._44 = 1.0f;
}


//-----------------------------------------------------------------------------
//      カスタマイズした正射影行列を生成します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::CreateOrthographicOffCenterReverseZ
(
    float left,
    float right,
    float bottom,
    float top,
    float nearClip,
    float farClip
)
{
    auto width  = right - left;
    auto height = bottom - top;
    auto depth  = nearClip - farClip;
    assert( !IsZero( width ) );
    assert( !IsZero( height ) );
    assert( !IsZero( depth ) );

    return Matrix(
        2.0f / width,
        0.0f,
        0.0f,
        0.0f,

        0.0f,
        2.0f / height,
        0.0f,
        0.0f,

        0.0f,
        0.0f,
        -1.0f / depth,
        0.0f,

        (left + right) / width,
        (top + bottom) / height,
        -nearClip / depth + 1.0f,
        1.0f
    );
}

//-----------------------------------------------------------------------------
//      カスタマイズした正射影行列を生成します.
//-----------------------------------------------------------------------------
inline
void Matrix::CreateOrthographicOffCenterReverseZ
(
    float     left,
    float     right,
    float     bottom,
    float     top,
    float     nearClip,
    float     farClip,
    Matrix&   result
)
{
    auto width  = right - left;
    auto height = bottom - top;
    auto depth  = nearClip - farClip;
    assert( !IsZero( width ) );
    assert( !IsZero( height ) );
    assert( !IsZero( depth ) );

    result._11 = 2.0f / width;
    result._12 = 0.0f;
    result._13 = 0.0f;
    result._14 = 0.0f;

    result._21 = 0.0f;
    result._22 = 2.0f / height;
    result._23 = 0.0f;
    result._24 = 0.0f;

    result._31 = 0.0f;
    result._32 = 0.0f;
    result._33 = -1.0f / depth;
    result._34 = 0.0f;

    result._41 = (left + right) / width;
    result._42 = (top + bottom) / height;
    result._43 = -nearClip / depth + 1.0f;
    result._44 = 1.0f;
}

//-----------------------------------------------------------------------------
//      2つの行列を線形補間します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::Lerp( const Matrix& a, const Matrix& b, float amount )
{
    return Matrix(
        a._11 + amount * ( b._11 - a._11 ),
        a._12 + amount * ( b._12 - a._12 ),
        a._13 + amount * ( b._13 - a._13 ),
        a._14 + amount * ( b._14 - a._14 ),

        a._21 + amount * ( b._21 - a._21 ),
        a._22 + amount * ( b._22 - a._22 ),
        a._23 + amount * ( b._23 - a._23 ),
        a._24 + amount * ( b._24 - a._24 ),

        a._31 + amount * ( b._31 - a._31 ),
        a._32 + amount * ( b._32 - a._32 ),
        a._33 + amount * ( b._33 - a._33 ),
        a._34 + amount * ( b._34 - a._34 ),

        a._41 + amount * ( b._41 - a._41 ),
        a._42 + amount * ( b._42 - a._42 ),
        a._43 + amount * ( b._43 - a._43 ),
        a._44 + amount * ( b._44 - a._44 )
    );
}

//-----------------------------------------------------------------------------
//      2つの行列を線形補間します.
//-----------------------------------------------------------------------------
inline
void Matrix::Lerp( const Matrix& a, const Matrix& b, float amount, Matrix &result )
{
    result._11 = a._11 + amount * ( b._11 - a._11 );
    result._12 = a._12 + amount * ( b._12 - a._12 );
    result._13 = a._13 + amount * ( b._13 - a._13 );
    result._14 = a._14 + amount * ( b._14 - a._14 );

    result._21 = a._21 + amount * ( b._21 - a._21 );
    result._22 = a._22 + amount * ( b._22 - a._22 );
    result._23 = a._23 + amount * ( b._23 - a._23 );
    result._24 = a._24 + amount * ( b._24 - a._24 );

    result._31 = a._31 + amount * ( b._31 - a._31 );
    result._32 = a._32 + amount * ( b._32 - a._32 );
    result._33 = a._33 + amount * ( b._33 - a._33 );
    result._34 = a._34 + amount * ( b._34 - a._34 );

    result._41 = a._41 + amount * ( b._41 - a._41 );
    result._42 = a._42 + amount * ( b._42 - a._42 );
    result._43 = a._43 + amount * ( b._43 - a._43 );
    result._44 = a._44 + amount * ( b._44 - a._44 );
}

//-----------------------------------------------------------------------------
//      ビルボード行列を生成します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::CreateBillboard( const Matrix& value )
{
    return Matrix(
        value._11, value._21, value._31, 0.0f,
        value._12, value._22, value._32, 0.0f,
        value._13, value._23, value._33, 0.0f,
        value._14, value._24, value._34, 1.0f);
}

//-----------------------------------------------------------------------------
//      ビルボード行列を生成します.
//-----------------------------------------------------------------------------
inline
void Matrix::CreateBillboard( const Matrix& value, Matrix& result )
{
    result._11 = value._11;     result._12 = value._21;    result._13 = value._31;      result._41 = 0.0f;
    result._21 = value._12;     result._22 = value._22;    result._23 = value._32;      result._24 = 0.0f;
    result._31 = value._13;     result._32 = value._23;    result._33 = value._33;      result._34 = 0.0f;
    result._41 = value._14;     result._42 = value._24;    result._43 = value._34;      result._44 = 1.0f;
}

//-----------------------------------------------------------------------------
//      Y軸ビルボード行列を生成します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::CreateBillboardAxisY( const Matrix& value )
{
    return Matrix(
        value._11,  0.0f,   value._31,  0.0f,
             0.0f,  1.0f,        0.0f,  0.0f,
        value._13,  0.0f,   value._33,  0.0f,
        value._14,  0.0f,   value._34,  1.0f);
}

//-----------------------------------------------------------------------------
//      Y軸ビルボード行列を生成します.
//-----------------------------------------------------------------------------
inline
void Matrix::CreateBillboardAxisY( const Matrix& value, Matrix& result )
{
    result._11 = value._11; result._12 = 0.0f;      result._13 = value._31; result._14 = 0.0f;
    result._21 = 0.0f;      result._22 = 1.0f;      result._23 = 0.0f;      result._24 = 0.0f;
    result._31 = value._13; result._32 = 0.0f;      result._33 = value._33; result._34 = 0.0f;
    result._41 = value._14; result._42 = value._24; result._43 = value._34; result._44 = 1.0f;
}

//-----------------------------------------------------------------------------
//      平行移動行列を左から掛けます.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::AppendTranslation(const Vector3& vec, Matrix& mat)
{
    auto m41 = ( vec.x * mat._11 ) + ( vec.y * mat._21 ) + ( vec.z * mat._31 ) + mat._41;
    auto m42 = ( vec.x * mat._12 ) + ( vec.y * mat._22 ) + ( vec.z * mat._32 ) + mat._42;
    auto m43 = ( vec.x * mat._13 ) + ( vec.y * mat._23 ) + ( vec.z * mat._33 ) + mat._43;
    auto m44 = ( vec.x * mat._14 ) + ( vec.y * mat._24 ) + ( vec.z * mat._34 ) + mat._44;
    mat._41 = m41;
    mat._42 = m42;
    mat._43 = m43;
    mat._44 = m44;
    return mat;
}

//-----------------------------------------------------------------------------
//      平行移動行列を右から掛けます.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::AppendTranslation(Matrix& mat, const Vector3& vec)
{
    mat._41 += vec.x;
    mat._42 += vec.y;
    mat._43 += vec.z;
    return mat;
}

//-----------------------------------------------------------------------------
//      スケール行列を左から掛けます.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::AppendScale(const Vector3& vec, Matrix& mat)
{
    mat._11 *= vec.x;
    mat._12 *= vec.x;
    mat._13 *= vec.x;
    mat._14 *= vec.x;

    mat._21 *= vec.y;
    mat._22 *= vec.y;
    mat._23 *= vec.y;
    mat._24 *= vec.y;

    mat._31 *= vec.z;
    mat._32 *= vec.z;
    mat._33 *= vec.z;
    mat._34 *= vec.z;

    return mat;
}

//-----------------------------------------------------------------------------
//      スケール行列を右から掛けます.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::AppendScale(Matrix& mat, const Vector3& vec)
{
    mat._11 *= vec.x;
    mat._12 *= vec.y;
    mat._13 *= vec.z;

    mat._21 *= vec.x;
    mat._22 *= vec.y;
    mat._23 *= vec.z;

    mat._31 *= vec.x;
    mat._32 *= vec.y;
    mat._33 *= vec.z;

    mat._41 *= vec.x;
    mat._42 *= vec.y;
    mat._43 *= vec.z;

    return mat;
}

//-----------------------------------------------------------------------------
//      明度を調整するカラー行列を生成します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::CreateBrightnessMatrix(float brightness)
{ return Matrix::CreateScale(brightness); }

//-----------------------------------------------------------------------------
//      彩度を調整するカラー行列を生成します.
//-----------------------------------------------------------------------------
inline 
Matrix Matrix::CreateSaturationMatrix(float r, float g, float b)
{
    // https://docs.microsoft.com/ja-jp/windows/win32/direct2d/saturation
    return Matrix(
        0.213f + 0.787f * r,
        0.213f - 0.213f * r,
        0.213f - 0.213f * r,
        0.0f,

        0.715f - 0.715f * g,
        0.715f + 0.285f * g,
        0.715f - 0.715f * g,
        0.0f,

        0.072f - 0.072f * b,
        0.072f - 0.072f * b,
        0.072f + 0.928f * b,
        0.0f,

        0.0f, 0.0f, 0.0f, 1.0f);
}

//-----------------------------------------------------------------------------
//      彩度を調整するカラー行列を生成します.
//-----------------------------------------------------------------------------
inline 
Matrix Matrix::CreateSaturationMatrix(float saturation)
{ return CreateSaturationMatrix(saturation, saturation, saturation); }

//-----------------------------------------------------------------------------
//      コントラストを調整するカラー行列を生成します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::CreateContrastMatrix(float contrast)
{
    const auto t = (1.0f - contrast) * 0.5f;
    return Matrix(
        contrast, 0.0f, 0.0f, 0.0f,
        0.0f, contrast, 0.0f, 0.0f,
        0.0f, 0.0f, contrast, 0.0f,
        t, t, t, 1.0f);
}

//-----------------------------------------------------------------------------
//      色相を調整するカラー行列を生成します.
//-----------------------------------------------------------------------------
inline
Matrix Matrix::CreateHueMatrix(float hue)
{
    // https://docs.microsoft.com/ja-jp/windows/win32/direct2d/hue-rotate
    auto rad = ToRadian(hue);
    auto u   = cosf(rad);
    auto w   = sinf(rad);

    return Matrix(
        0.213f + 0.787f * u - 0.213f * w,
        0.213f - 0.213f * u + 0.143f * w,
        0.213f - 0.213f * u - 0.787f * w,
        0.0f,

        0.715f - 0.715f * u - 0.715f * w,
        0.715f + 0.285f * u + 0.140f * w,
        0.715f - 0.715f * u - 0.283f * w,
        0.0f,

        0.072f - 0.072f * u + 0.928f * w,
        0.072f - 0.072f * u - 0.283f * w,
        0.072f + 0.928f * u + 0.072f * w,
        0.0f,

        0.0f,
        0.0f,
        0.0f,
        1.0f);
}

//-----------------------------------------------------------------------------
//      セピアカラーを調整するカラー行列を生成します.
//-----------------------------------------------------------------------------
inline 
Matrix Matrix::CreateSepiaMatrix(float tone)
{
    const Vector3 kWhite(0.298912f, 0.586611f, 0.114478f);
    const Vector3 kSepia(0.941f, 0.784f, 0.569f);

    return Matrix(
        tone * kWhite.x * kSepia.x + (1.0f - tone),
        tone * kWhite.x * kSepia.y,
        tone * kWhite.x * kSepia.z,
        0.0f,

        tone * kWhite.y * kSepia.x,
        tone * kWhite.y * kSepia.y + (1.0f - tone),
        tone * kWhite.y * kSepia.z,
        0.0f,

        tone * kWhite.z * kSepia.x,
        tone * kWhite.z * kSepia.y,
        tone * kWhite.z * kSepia.z + (1.0f - tone),
        0.0f,

        0.0f, 0.0f, 0.0f, 1.0f);
}

//-----------------------------------------------------------------------------
//      グレースケールカラーを調整するカラー行列を生成します.
//-----------------------------------------------------------------------------
inline 
Matrix Matrix::CreateGrayScaleMatrix(float tone)
{
    const Vector3 kGrayScale(0.22015f, 0.706655f, 0.071330f);
    return Matrix(
        tone * kGrayScale.x + (1.0f - tone),
        tone * kGrayScale.x,
        tone * kGrayScale.x,
        0.0f,

        tone * kGrayScale.y,
        tone * kGrayScale.y + (1.0f - tone),
        tone * kGrayScale.y,
        0.0f,

        tone * kGrayScale.z,
        tone * kGrayScale.z,
        tone * kGrayScale.z + (1.0f - tone),
        0.0f,

        0.0f, 0.0f, 0.0f, 1.0f);
}

//-----------------------------------------------------------------------------
//      ネガポジ反転のカラー行列を生成します.
//-----------------------------------------------------------------------------
inline 
Matrix Matrix::CreateReverseColorMatrix()
{
    return Matrix(
        -1.0f,  0.0f,  0.0f, 0.0f,
         0.0f, -1.0f,  0.0f, 0.0f,
         0.0f,  0.0f, -1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Quaternion
///////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
inline
Quaternion::Quaternion()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
inline
Quaternion::Quaternion( const float* pf )
{
    assert( pf != nullptr );
    x = pf[ 0 ];
    y = pf[ 1 ];
    z = pf[ 2 ];
    w = pf[ 3 ];
}

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
inline
Quaternion::Quaternion( float nx, float ny, float nz, float nw )
: x( nx )
, y( ny )
, z( nz )
, w( nw )
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      float* 型へのキャストです.
//-----------------------------------------------------------------------------
inline
Quaternion::operator float* ()
{ return static_cast<float*>( &x ); }

//-----------------------------------------------------------------------------
//      const float* 型へのキャストです.
//-----------------------------------------------------------------------------
inline
Quaternion::operator const float* () const
{ return static_cast<const float*>( &x ); }

//-----------------------------------------------------------------------------
//      加算代入演算子です.
//-----------------------------------------------------------------------------
inline 
Quaternion& Quaternion::operator += ( const Quaternion& q )
{
    x += q.x;
    y += q.y;
    z += q.z;
    w += q.w;
    return (*this);
}

//-----------------------------------------------------------------------------
//      減算代入演算子です.
//-----------------------------------------------------------------------------
inline 
Quaternion& Quaternion::operator -= ( const Quaternion& q )
{
    x -= q.x;
    y -= q.y;
    z -= q.z;
    w -= q.w;
    return (*this);
}

//-----------------------------------------------------------------------------
//      乗算代入演算子です.
//-----------------------------------------------------------------------------
inline
Quaternion& Quaternion::operator *= ( const Quaternion& q )
{
    auto X = ( q.x * w ) + ( x * q.w ) + ( q.y * z ) - ( q.z * y );
    auto Y = ( q.y * w ) + ( y * q.w ) + ( q.z * x ) - ( q.x * z );
    auto Z = ( q.z * w ) + ( z * q.w ) + ( q.x * y ) - ( q.y * x );
    auto W = ( q.w * w ) - ( q.x * x ) - ( q.y * y ) + ( q.z * z );
    x = X;
    y = Y;
    z = Z;
    w = W;
    return (*this);
}

//-----------------------------------------------------------------------------
//      乗算代入演算子です.
//-----------------------------------------------------------------------------
inline 
Quaternion& Quaternion::operator *= ( float f )
{
    x *= f;
    y *= f;
    z *= f;
    w *= f;
    return (*this);
}

//-----------------------------------------------------------------------------
//      除算代入演算子です.
//-----------------------------------------------------------------------------
inline 
Quaternion& Quaternion::operator /= ( float f )
{
    assert( !IsZero( f ) );
    x /= f;
    y /= f;
    z /= f;
    w /= f;
    return (*this);
}

//-----------------------------------------------------------------------------
//      正符号演算子です.
//-----------------------------------------------------------------------------
inline 
Quaternion Quaternion::operator + () const
{ return (*this); }

//-----------------------------------------------------------------------------
//      負符号演算子です.
//-----------------------------------------------------------------------------
inline 
Quaternion Quaternion::operator - () const
{ return Quaternion( -x, -y, -z, -w ); }

//-----------------------------------------------------------------------------
//      加算演算子です.
//-----------------------------------------------------------------------------
inline 
Quaternion Quaternion::operator + ( const Quaternion& q ) const
{ return Quaternion( x + q.x, y + q.y, z + q.z, w + q.z ); }

//-----------------------------------------------------------------------------
//      減算演算子です.
//-----------------------------------------------------------------------------
inline 
Quaternion Quaternion::operator - ( const Quaternion& q ) const
{ return Quaternion( x - q.x, y - q.y, z - q.z, w - q.z ); }

//-----------------------------------------------------------------------------
//      乗算演算子です.
//-----------------------------------------------------------------------------
inline 
Quaternion Quaternion::operator * ( const Quaternion& q ) const
{ 
    return Quaternion(
        ( q.x * w ) + ( x * q.w ) + ( q.y * z ) - ( q.z * y ),
        ( q.y * w ) + ( y * q.w ) + ( q.z * x ) - ( q.x * z ),
        ( q.z * w ) + ( z * q.w ) + ( q.x * y ) - ( q.y * x ),
        ( q.w * w ) - ( q.x * x ) - ( q.y * y ) + ( q.z * z )
   );
}

//-----------------------------------------------------------------------------
//      乗算演算子です.
//-----------------------------------------------------------------------------
inline 
Quaternion Quaternion::operator * ( float f ) const
{ return Quaternion( x * f, y * f, z * f, w *f ); }

//-----------------------------------------------------------------------------
//      除算演算子です.
//-----------------------------------------------------------------------------
inline 
Quaternion Quaternion::operator / ( float f ) const
{ 
    assert( !IsZero( f ) );
    return Quaternion( x / f, y / f, z / f, w / f ); 
}

//-----------------------------------------------------------------------------
//      乗算演算子です.
//-----------------------------------------------------------------------------
inline 
Quaternion operator * ( float f, const Quaternion& q )
{ return Quaternion( f * q.x, f * q.y, f * q.z, f * q.w ); }

//-----------------------------------------------------------------------------
//      等価比較演算子です.
//-----------------------------------------------------------------------------
inline 
bool Quaternion::operator == ( const Quaternion& q ) const
{ 
    return IsEqual( x, q.x )
        && IsEqual( y, q.y )
        && IsEqual( z, q.z )
        && IsEqual( w, q.w );
}

//-----------------------------------------------------------------------------
//      非等価比較演算子です.
//-----------------------------------------------------------------------------
inline
bool Quaternion::operator != ( const Quaternion& q ) const
{ 
    return !IsEqual( x, q.x )
        || !IsEqual( y, q.y )
        || !IsEqual( z, q.z )
        || !IsEqual( w, q.w );
}

//-----------------------------------------------------------------------------
//      四元数の大きさを求めます.
//-----------------------------------------------------------------------------
inline
float Quaternion::Length() const
{ return sqrtf( x * x + y * y + z * z + w * w ); }

//-----------------------------------------------------------------------------
//      四元数の大きさの2乗値を求めます.
//-----------------------------------------------------------------------------
inline
float Quaternion::LengthSq() const
{ return ( x * x + y * y + z * z + w * w ); }

//-----------------------------------------------------------------------------
//      正規化します.
//-----------------------------------------------------------------------------
inline 
Quaternion& Quaternion::Normalize()
{
    auto mag = Length();
    assert( mag > 0.0f );
    x /= mag;
    y /= mag;
    z /= mag;
    w /= mag;
    return (*this);
}

//-----------------------------------------------------------------------------
//      零除算を考慮して正規化します.
//-----------------------------------------------------------------------------
inline
Quaternion& Quaternion::SafeNormalize( const Quaternion& set )
{
    auto mag = Length();
    if ( mag > 0.0f )
    {
        x /= mag;
        y /= mag;
        z /= mag;
        w /= mag;
    }
    else
    {
        x = set.x;
        y = set.y;
        z = set.z;
        w = set.w;
    }
    return (*this);
}

//-----------------------------------------------------------------------------
//      単位四元数化します.
//-----------------------------------------------------------------------------
inline
Quaternion& Quaternion::Identity()
{
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    w = 1.0f;
    return (*this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Quaternion Methods
///////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      単位四元数を生成します.
//-----------------------------------------------------------------------------
inline
Quaternion Quaternion::CreateIdentity()
{ return Quaternion( 0.0f, 0.0f, 0.0f, 1.0f ); }

//-----------------------------------------------------------------------------
//      単位四元数かどうかチェックします.
//-----------------------------------------------------------------------------
inline
bool Quaternion::IsIdentity( const Quaternion& value )
{
    return IsEqual( value.x, 0.0f )
        && IsEqual( value.y, 0.0f )
        && IsEqual( value.z, 0.0f )
        && IsEqual( value.w, 1.0f );
}

//-----------------------------------------------------------------------------
//      正規化されているかどうかチェックします.
//-----------------------------------------------------------------------------
inline
bool Quaternion::IsNormalized( const Quaternion& value )
{
    return IsZero( 1.0f - value.Length() );
}

//-----------------------------------------------------------------------------
//      乗算を行います.
//-----------------------------------------------------------------------------
inline
Quaternion Quaternion::Multiply( const Quaternion& a, const Quaternion& b )
{
    return Quaternion(
        ( b.x * a.w ) + ( a.x * b.w ) + ( b.y * a.z ) - ( b.z * a.y ),
        ( b.y * a.w ) + ( a.y * b.w ) + ( b.z * a.x ) - ( b.x * a.z ),
        ( b.z * a.w ) + ( a.z * b.w ) + ( b.x * a.y ) - ( b.y * a.x ),
        ( b.w * a.w ) - ( b.x * a.x ) - ( b.y * a.y ) + ( b.z * a.z )
   );
}

//-----------------------------------------------------------------------------
//      四元数同士の乗算を行います.
//-----------------------------------------------------------------------------
inline
void Quaternion::Multiply( const Quaternion& a, const Quaternion& b, Quaternion& result )
{
    result.x = ( b.x * a.w ) + ( a.x * b.w ) + ( b.y * a.z ) - ( b.z * a.y );
    result.y = ( b.y * a.w ) + ( a.y * b.w ) + ( b.z * a.x ) - ( b.x * a.z );
    result.z = ( b.z * a.w ) + ( a.z * b.w ) + ( b.x * a.y ) - ( b.y * a.x );
    result.w = ( b.w * a.w ) - ( b.x * a.x ) - ( b.y * a.y ) + ( b.z * a.z );
}

//-----------------------------------------------------------------------------
//      内積を求めます.
//-----------------------------------------------------------------------------
inline
float Quaternion::Dot( const Quaternion& a, const Quaternion& b )
{ return ( a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w ); }

//-----------------------------------------------------------------------------
//      内積を求めます.
//-----------------------------------------------------------------------------
inline
void Quaternion::Dot( const Quaternion &a, const Quaternion &b, float &result )
{ result = ( a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w ); }

//-----------------------------------------------------------------------------
//      共役な四元数を求めます.
//-----------------------------------------------------------------------------
inline
Quaternion Quaternion::Conjugate( const Quaternion& value )
{ return Quaternion( -value.x, -value.y, -value.z, value.w ); }

//-----------------------------------------------------------------------------
//      共役な四元数を求めます.
//-----------------------------------------------------------------------------
inline
void Quaternion::Conjugate( const Quaternion &value, Quaternion &result )
{
    result.x = -value.x;
    result.y = -value.y;
    result.z = -value.z;
    result.w = value.w;
}

//-----------------------------------------------------------------------------
//      正規化を行います.
//-----------------------------------------------------------------------------
inline
Quaternion Quaternion::Normalize( const Quaternion& value )
{
    auto mag = value.Length();
    assert( mag > 0.0f );
    return Quaternion(
        value.x / mag,
        value.y / mag,
        value.z / mag,
        value.w / mag
    );
}

//-----------------------------------------------------------------------------
//      正規化を行います.
//-----------------------------------------------------------------------------
inline
void Quaternion::Normalize( const Quaternion& value, Quaternion &result )
{
    auto mag = value.Length();
    assert( mag > 0.0f );
    result.x /= mag;
    result.y /= mag;
    result.z /= mag;
    result.w /= mag;
}

//-----------------------------------------------------------------------------
//      零除算を考慮して正規化します.
//-----------------------------------------------------------------------------
inline
Quaternion  Quaternion::SafeNormalize( const Quaternion& value, const Quaternion& set )
{
    auto mag = value.Length();
    if ( mag > 0.0f )
    {
        return Quaternion(
            value.x / mag,
            value.y / mag,
            value.z / mag,
            value.w / mag
        );
    }

    return set;
}

//-----------------------------------------------------------------------------
//      零除算を考慮して正規化します.
//-----------------------------------------------------------------------------
inline
void Quaternion::SafeNormalize( const Quaternion& value, const Quaternion& set, Quaternion& result )
{
    auto mag = value.Length();
    if ( mag > 0.0f )
    {
        result.x = value.x / mag;
        result.y = value.y / mag;
        result.z = value.z / mag;
        result.w = value.w / mag;
    }
    else
    {
        result.x = set.x;
        result.y = set.y;
        result.z = set.z;
        result.w = set.w;
    }
}

//-----------------------------------------------------------------------------
//      ヨー・ピッチ・ロール角から四元数を生成します.
//-----------------------------------------------------------------------------
inline
Quaternion Quaternion::CreateFromYawPitchRoll( float yaw, float pitch, float roll )
{
    auto r = roll  * 0.5f;
    auto p = pitch * 0.5f;
    auto y = yaw   * 0.5f;
    auto sr = sinf( r );
    auto cr = cosf( r );
    auto sp = sinf( p );
    auto cp = cosf( p );
    auto sy = sinf( y );
    auto cy = cosf( y );

    return Quaternion(
        cy * sp * cr + sy * cp * sr,
        sy * cp * cr - cy * sp * sr,
        cy * cp * sr - sy * sp * cr,
        cy * cp * cr + sy * sp * sr);

}

//-----------------------------------------------------------------------------
//      ヨー・ピッチ・ロール角から四元数を生成します.
//-----------------------------------------------------------------------------
inline
void Quaternion::CreateFromYawPitchRoll( float yaw, float pitch, float roll, Quaternion& result )
{
    auto r = roll  * 0.5f;
    auto p = pitch * 0.5f;
    auto y = yaw   * 0.5f;
    auto sr = sinf(r);
    auto cr = cosf(r);
    auto sp = sinf(p);
    auto cp = cosf(p);
    auto sy = sinf(y);
    auto cy = cosf(y);

    result.x = cy * sp * cr + sy * cp * sr;
    result.y = sy * cp * cr - cy * sp * sr;
    result.z = cy * cp * sr - sy * sp * cr;
    result.w = cy * cp * cr + sy * sp * sr;
}

//-----------------------------------------------------------------------------
//      指定された軸と角度から四元数を生成します.
//-----------------------------------------------------------------------------
inline
Quaternion Quaternion::CreateFromAxisAngle( const Vector3& axis, float radian )
{
    auto halfRad = radian * 0.5f;
    auto sinX = sinf( halfRad );
    return Quaternion(
        axis.x * sinX,
        axis.y * sinX,
        axis.z * sinX,
        cosf( halfRad )
   );
}

//-----------------------------------------------------------------------------
//      指定された軸と角度から四元数を生成します.
//-----------------------------------------------------------------------------
inline
void Quaternion::CreateFromAxisAngle( const Vector3& axis, float radian, Quaternion& result )
{
    auto halfRad = radian * 0.5f;
    auto sinX = sinf( halfRad );
    result.x = axis.x * sinX;
    result.y = axis.y * sinX;
    result.z = axis.z * sinX;
    result.w = cosf( halfRad );
}

//-----------------------------------------------------------------------------
//      回転行列から四元数を生成します.
//-----------------------------------------------------------------------------
inline
Quaternion Quaternion::CreateFromRotationMatrix( const Matrix& value )
{
    auto tr = value._11 + value._22 + value._33;
    if ( tr > 0.0f )
    {
        auto s = sqrtf( tr + 1.0f );
        auto w = s * 0.5f;
        s = 0.5f / s;
        return Quaternion(
            ( value._23 - value._32 ) * s,
            ( value._31 - value._13 ) * s,
            ( value._12 - value._21 ) * s,
            w 
        );
    }
    if ( ( value._11 >= value._22 ) && ( value._11 >= value._33 ) )
    {
        auto s = sqrtf( 1.0f + value._11 - value._22 - value._33 );
        auto x = s * 0.5f;
        s = 0.5f / s;
        return Quaternion(
            x,
            ( value._12 + value._21 ) * s,
            ( value._13 + value._31 ) * s,
            ( value._23 - value._32 ) * s
        );
    }
    if ( value._22 > value._33 )
    {
        auto s = sqrtf( 1.0f + value._22 - value._11 - value._33 );
        auto y = s * 0.5f;
        s = 0.5f / s;
        return Quaternion(
            ( value._21 + value._12 ) * s,
            y,
            ( value._32 + value._23 ) * s,
            ( value._31 - value._13 ) * s
        );
    }
    auto s = sqrtf( 1.0f + value._33 - value._11 - value._22 );
    auto z = s * 0.5f;
    s = 0.5f / s;
    return Quaternion(
        ( value._31 + value._13 ) * s,
        ( value._32 + value._23 ) * s,
        z,
        ( value._12 - value._21 ) * s
    );
}

//-----------------------------------------------------------------------------
//      回転行列から四元数を生成します.
//-----------------------------------------------------------------------------
inline
void Quaternion::CreateFromRotationMatrix( const Matrix& value, Quaternion& result )
{
    auto tr = value._11 + value._22 + value._33;
    if ( tr > 0.0f )
    {
        auto s = sqrtf( tr + 1.0f );

        result.w = s * 0.5f;
        s = 0.5f / s;

        result.x = ( value._23 - value._32 ) * s;
        result.y = ( value._31 - value._13 ) * s;
        result.z = ( value._12 - value._21 ) * s;
        return;
    }
    if ( ( value._11 >= value._22 ) && ( value._11 >= value._33 ) )
    {
        auto s = sqrtf( 1.0f + value._11 - value._22 - value._33 );
        result.x = 0.5f * s;

        s = 0.5f / s;
        result.y = ( value._12 + value._21 ) * s;
        result.z = ( value._13 + value._31 ) * s;
        result.w = ( value._23 - value._32 ) * s;
        return;
    }
    if ( value._22 > value._33 )
    {
        auto s = sqrtf( 1.0f + value._22 - value._11 - value._33 );
        result.y = 0.5f * s;
        s = 0.5f / s;

        result.x = ( value._21 + value._12 ) * s;
        result.z = ( value._32 + value._23 ) * s;
        result.w = ( value._31 - value._13 ) * s;
        return;
    }
    auto s = sqrtf( 1.0f + value._33 - value._11 - value._22 );
    result.z = 0.5f * s;
    s = 0.5f / s;
    result.x = ( value._31 + value._13 ) * s;
    result.y = ( value._32 + value._23 ) * s;
    result.w = ( value._12 - value._21 ) * s;
}

//-----------------------------------------------------------------------------
//      回転角を取得します.
//-----------------------------------------------------------------------------
inline
Vector3 Quaternion::ToAxisAngle( const Quaternion& value )
{
    auto x2 = value.x * value.x;
    auto y2 = value.y * value.y;
    auto z2 = value.z * value.z;
    auto w2 = value.w * value.w;
    auto mag = x2 + y2 + z2 + w2;
    auto test = value.x * value.y + value.z * value.w;
    auto rad = F_PIDIV2;

    if (test > rad * mag)
    {
        return Vector3(
            0.0f,
            2.0f * std::atan2(value.x, value.w),
            F_PIDIV2);
    }
    if (test < -rad * mag)
    {
        return Vector3(
            0.0f,
            -2.0f * std::atan2(value.x, value.w),
            -F_PIDIV2);
    }

    return Vector3(
        std::atan2(2.0f * value.x * value.w - 2.0f * value.y * value.z, -x2 + y2 - z2 + w2),
        std::atan2(2.0f * value.y * value.w - 2.0f * value.x * value.z,  x2 - y2 - z2 + w2),
        std::asin (2.0f * test / mag));
}


//-----------------------------------------------------------------------------
//      回転角を取得します.
//-----------------------------------------------------------------------------
inline
void Quaternion::ToAxisAngle( const Quaternion& value, Vector3& result )
{
    auto x2 = value.x * value.x;
    auto y2 = value.y * value.y;
    auto z2 = value.z * value.z;
    auto w2 = value.w * value.w;
    auto mag = x2 + y2 + z2 + w2;
    auto test = value.x * value.y + value.z * value.w;
    auto rad = F_PIDIV2;

    if (test > rad * mag)
    {
        result.x = 0.0f;
        result.y = 2.0f * std::atan2(value.x, value.w);
        result.z = F_PIDIV2;
    }
    if (test < -rad * mag)
    {
        result.x = 0.0f;
        result.y = -2.0f * std::atan2(value.x, value.w);
        result.z = -F_PIDIV2;
    }
    else
    {
        result.x = std::atan2(2.0f * value.x * value.w - 2.0f * value.y * value.z, -x2 + y2 - z2 + w2);
        result.y = std::atan2(2.0f * value.y * value.w - 2.0f * value.x * value.z,  x2 - y2 - z2 + w2);
        result.z = std::asin (2.0f * test / mag);
    }
}

//-----------------------------------------------------------------------------
//      球面線形補間を行います.
//-----------------------------------------------------------------------------
inline
Quaternion Quaternion::Slerp
(
    const Quaternion&   a,
    const Quaternion&   b,
    const float           amount
)
{
    if ( amount <= 0.0f )
    { return a; }

    if ( amount >= 1.0f )
    { return b; }

    if ( a == b )
    { return b; }

    Quaternion temp;
    auto cosom = Quaternion::Dot( a, b );
    if ( cosom < 0.0f )
    { 
        temp = -b;
        cosom = -cosom;
    }
    else
    { temp = b; }

    auto scale0 = 0.0f;
    auto scale1 = 0.0f;
    if ( 1.0f - cosom > 1e-6f )
    {
        scale0 = 1.0f - cosom * cosom;
        auto sinom = 1.0f / sqrtf( scale0 );
        auto omega = atan2f( scale0 * sinom, cosom );
        scale0 = sinf( ( 1.0f - amount ) * omega ) * sinom;
        scale1 = sinf( amount * omega ) * sinom;
    }
    else
    {
        scale0 = 1.0f - amount;
        scale1 = amount;
    }

    return Quaternion(
        scale0 * a.x + scale1 * temp.x,
        scale0 * a.y + scale1 * temp.y,
        scale0 * a.z + scale1 * temp.z,
        scale0 * a.w + scale1 * temp.w);
}

//-----------------------------------------------------------------------------
//      球面線形補間を行います.
//-----------------------------------------------------------------------------
inline
void Quaternion::Slerp
(
    const Quaternion&   a,
    const Quaternion&   b,
    const float           amount,
    Quaternion&         result
)
{
    if ( amount <= 0.0f )
    {
        result = a;
        return;
    }

    if ( amount > 1.0f )
    {
        result = b;
        return;
    }

    if ( a == b )
    {
        result = b;
        return;
    }

   Quaternion temp;
    auto cosom = Quaternion::Dot( a, b );
    if ( cosom < 0.0f )
    { 
        temp = -b;
        cosom = -cosom;
    }
    else
    { temp = b; }

    auto scale0 = 0.0f;
    auto scale1 = 0.0f;
    if ( 1.0f - cosom > 1e-6f )
    {
        scale0 = 1.0f - cosom * cosom;
        auto sinom = 1.0f / sqrtf( scale0 );
        auto omega = atan2f( scale0 * sinom, cosom );
        scale0 = sinf( ( 1.0f - amount ) * omega ) * sinom;
        scale1 = sinf( amount * omega ) * sinom;
    }
    else
    {
        scale0 = 1.0f - amount;
        scale1 = amount;
    }

    result.x = scale0 * a.x + scale1 * temp.x;
    result.y = scale0 * a.y + scale1 * temp.y;
    result.z = scale0 * a.z + scale1 * temp.z;
    result.w = scale0 * a.w + scale1 * temp.w;
}

//-----------------------------------------------------------------------------
//      球面四角形補間を行います.
//-----------------------------------------------------------------------------
inline
Quaternion Quaternion::Squad
(
    const Quaternion&   q,
    const Quaternion&   a,
    const Quaternion&   b,
    const Quaternion&   c,
    float                 amount
)
{
    auto d = Quaternion::Slerp( q, c, amount );
    auto e = Quaternion::Slerp( a, b, amount );
    return Quaternion::Slerp( d, e, 2.0f * amount * ( 1.0f - amount ) );
}

//-----------------------------------------------------------------------------
//      球面四角形補間を行います.
//-----------------------------------------------------------------------------
inline
void Quaternion::Squad
(
    const Quaternion&   q,
    const Quaternion&   a,
    const Quaternion&   b,
    const Quaternion&   c,
    float               amount,
    Quaternion&         result
)
{
    Quaternion d, e;
    Quaternion::Slerp( q, c, amount, d );
    Quaternion::Slerp( a, b, amount, e );
    Quaternion::Slerp( d, e, 2.0f * amount * ( 1.0f - amount ), result );
}

//-----------------------------------------------------------------------------
//      正規直交基底を求めます.
//-----------------------------------------------------------------------------
inline
void CalcONB(const Vector3& N, Vector3& T, Vector3& B)
{
    float sig = (N.z >= 0.0) ? 1.0f : -1.0f;
    float a = -1.0f / (sig + N.z);
    float b = N.x * N.y * a;
    T = Vector3(1.0f + sig * N.x * N.x * a, sig * b, -sig * N.x);
    B = Vector3(b, sig + N.y * N.y * a, -N.y);
}

//-----------------------------------------------------------------------------
//      Hammersleyサンプルを行います.
//-----------------------------------------------------------------------------
inline
Vector2 Hammersley( uint32_t i, uint32_t numSamples )
{
    uint32_t bits = i;
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    float result = float(bits) * 2.3283064365386963e-10f; // / 0x100000000

    return Vector2( float(i)/float(numSamples), result );
}

//-----------------------------------------------------------------------------
//      平面式を正規化します.
//-----------------------------------------------------------------------------
inline
Vector4 NormalizePlane(const Vector4& value)
{
    auto mag = sqrt(value.x * value.x + value.y * value.y + value.z * value.z);
    return Vector4(value.x / mag, value.y / mag, value.z / mag, value.w / mag);
}

//-----------------------------------------------------------------------------
//      視錐台を構成する6平面を求めます.
//-----------------------------------------------------------------------------
inline
void CalcFrustumPlanes(const Matrix& view, const Matrix& proj, Vector4* planes)
{
    // Gil Gribb, Klaus Hartmann,
    // "Fast Extraction of Viewing Frustum Planes from the World-View-Projection Matrix"
    // https://www.gamedevs.org/
    auto vp = Matrix::MultiplyTranspose(view, proj);

    planes[PLANE_LEFT]   = NormalizePlane(vp.row[3] + vp.row[0]);
    planes[PLANE_RIGHT]  = NormalizePlane(vp.row[3] - vp.row[0]);
    planes[PLANE_BOTTOM] = NormalizePlane(vp.row[3] + vp.row[1]);
    planes[PLANE_TOP]    = NormalizePlane(vp.row[3] - vp.row[1]);
    planes[PLANE_NEAR]   = NormalizePlane(vp.row[2]);
    planes[PLANE_FAR]    = NormalizePlane(vp.row[3] - vp.row[2]);
}

//-----------------------------------------------------------------------------
//      交差線を求めます.
//-----------------------------------------------------------------------------
inline
void ComputeIntersectionLine(const Vector4& p1, const Vector4& p2, Vector3& orig, Vector3& dir)
{
    auto n1 = Vector3(p1.x, p1.y, p1.z);
    auto n2 = Vector3(p2.x, p2.y, p2.z);
    dir = Vector3::Cross(n1, n2);
    auto divider = Vector3::Dot(dir, dir);
    orig = Vector3::Cross(p1.w * n2 + p2.w * n1, dir) / divider;
}

//-----------------------------------------------------------------------------
//      平面とレイの交差点を求めます.
//-----------------------------------------------------------------------------
inline
Vector3 ComputeIntersection(const Vector4& plane, const Vector3& orig, const Vector3& dir)
{
    auto normal = Vector3(plane.x, plane.y, plane.z);
    auto t = (-plane.w - Vector3::Dot(normal, orig)) / Vector3::Dot(normal, dir);
    return orig + dir * t;
}

//-----------------------------------------------------------------------------
//      視錐台の8角を求めます.
//-----------------------------------------------------------------------------
inline
void GetCorners(const Vector4* planes, Vector3* corners)
{
    Vector3 orig, dir;
    ComputeIntersectionLine(planes[0], planes[2], orig, dir);
    corners[0] = ComputeIntersection(planes[4], orig, dir);
    corners[3] = ComputeIntersection(planes[5], orig, dir);
 
    ComputeIntersectionLine(planes[3], planes[0], orig, dir);
    corners[1] = ComputeIntersection(planes[4], orig, dir);
    corners[2] = ComputeIntersection(planes[5], orig, dir);

    ComputeIntersectionLine(planes[2], planes[1], orig, dir);
    corners[4] = ComputeIntersection(planes[4], orig, dir);
    corners[7] = ComputeIntersection(planes[5], orig, dir);

    ComputeIntersectionLine(planes[1], planes[3], orig, dir);
    corners[5] = ComputeIntersection(planes[4], orig, dir);
    corners[6] = ComputeIntersection(planes[5], orig, dir);
}

} // namespace asdx


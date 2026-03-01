//-----------------------------------------------------------------------------
// File : asdxGridSnap.h
// Desc : Grid Snap.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxMath.h>


namespace asdx {

struct HexCoord
{
    float q;
    float r;
};

struct CubeCoord
{
    float x;
    float y;
    float z;
};

inline 
CubeCoord AxialToCube(const HexCoord& h)
{ return { h.q, -h.q -h.r, h.r }; }

inline
CubeCoord RoundCube(const CubeCoord& c)
{
    auto rx = round(c.x);
    auto ry = round(c.y);
    auto rz = round(c.z);

    auto dx = abs(rx - c.x);
    auto dy = abs(ry - c.y);
    auto dz = abs(rz - c.z);

    if (dx > dy && dx > dz) rx = -ry - rz;
    else if (dy > dz)       ry = -rx - rz;
    else                    rz = -rx - ry;

    return { rx, ry, rz };
}

inline 
HexCoord CubeToAxial(const CubeCoord& c)
{ return { c.x, c.z }; }

inline
HexCoord WorldToHex(const Vector3& p, float gridSize)
{
    const auto kSqrt3 = 1.7320508075f;
    auto q = (kSqrt3 / 3.0f * p.x - 1.0f / 3.0f * p.z) / gridSize;
    auto r = (2.0f / 3.0f * p.z) / gridSize;
    return { q, r };
}

inline
Vector3 HexToWorld(const HexCoord& h, float gridSize)
{
    const auto kSqrt3 = 1.73205080757f;
    auto x = gridSize * kSqrt3 * (h.q + h.r * 0.5f);
    auto z = gridSize * 1.5f * h.r;
    return Vector3(x, 0.0f, z);
}

inline
Vector3 SnapHex(const Vector3& p, float gridSize)
{
    auto h  = WorldToHex(p, gridSize);
    auto c  = AxialToCube(h);
    auto rc = RoundCube(c);
    auto rh = CubeToAxial(rc);
    return HexToWorld(rh, gridSize);
}

inline
Vector3 SnapSquare(const Vector3& p, float gridSize)
{
    return Vector3(
        round(p.x / gridSize) * gridSize,
        round(p.y / gridSize) * gridSize,
        round(p.z / gridSize) * gridSize);
}

} // namespace asdx

//-----------------------------------------------------------------------------
// File : asdxResModel.h
// Desc : Model Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdint>
#include <vector>
#include <asdxMath.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ResBoneIndex structure
///////////////////////////////////////////////////////////////////////////////
struct ResBoneIndex
{
    uint8_t     Index0;    //!< ボーン番号0.
    uint8_t     Index1;    //!< ボーン番号1.
    uint8_t     Index2;    //!< ボーン番号2.
    uint8_t     Index3;    //!< ボーン番号3.
};

///////////////////////////////////////////////////////////////////////////////
// ResMeshlet structure
///////////////////////////////////////////////////////////////////////////////
struct ResMeshlet
{
    uint32_t    VertexOffset;       //!< 頂点番号オフセット.
    uint32_t    VertexCount;        //!< 頂点数.
    uint32_t    PrimitiveOffset;    //!< プリミティブ.
    uint32_t    PrimitiveCount;     //!< プリミティブ数.
};

///////////////////////////////////////////////////////////////////////////////
// ResCulling structure
///////////////////////////////////////////////////////////////////////////////
struct ResCulling
{
    Vector4     BoundingSphere;     //!< バウンディングスフィアです.
    uint32_t    NormalCone;         //!< 圧縮済み法錐です.
};

///////////////////////////////////////////////////////////////////////////////
// ResPrimitive structure
///////////////////////////////////////////////////////////////////////////////
struct ResPrimitive
{
    uint32_t    Index0   : 10;      //!< 出力頂点番号0
    uint32_t    Index1   : 10;      //!< 出力頂点番号1
    uint32_t    Index2   : 10;      //!< 出力頂点番号2
    uint32_t    Reserved : 2;       //!< 予約領域.
};

///////////////////////////////////////////////////////////////////////////////
// ResMesh structure
///////////////////////////////////////////////////////////////////////////////
struct ResMesh
{
    std::vector<Vector3>        Positions;      //!< 位置座標.
    std::vector<uint32_t>       TangentSpace;   //!< R10G10B10A2圧縮済み接線空間.
    std::vector<Vector2>        TexCoord0;      //!< テクスチャ座標0です.
    std::vector<Vector2>        TexCoord1;      //!< テクスチャ座標1です.
    std::vector<Vector2>        TexCoord2;      //!< テクスチャ座標2です.
    std::vector<Vector2>        TexCoord3;      //!< テクスチャ座標3です.
    std::vector<Vector4>        BoneWeights;    //!< ボーンの重みです.
    std::vector<ResBoneIndex>   BoneIndices;    //!< ボーン番号ですです.

    std::vector<ResMeshlet>     Meshlets;       //!< メッシュレットです.
    std::vector<uint32_t>       Indices;        //!< ユニーク頂点番号です.
    std::vector<ResPrimitive>   Primitives;     //!< 出力頂点番号です.

    uint32_t                    MaterialHash;   //!< マテリアルハッシュです.
};

///////////////////////////////////////////////////////////////////////////////
// ResModel structure
///////////////////////////////////////////////////////////////////////////////
struct ResModel
{
    std::vector<ResMesh>        Meshes;         //!< メッシュです.
};

} // namespace asdx
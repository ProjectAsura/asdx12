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
// ResCullingInfo structure
///////////////////////////////////////////////////////////////////////////////
struct ResCullingInfo
{
    Vector4     BoundingSphere;     //!< バウンディングスフィアです.
    Vector4     NormalCone;         //!< 圧縮済み法錐です.
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
    std::vector<Vector3>        Normals;        //!< 法線ベクトルです.
    std::vector<Vector3>        Tangents;       //!< 接線ベクトルです.
    std::vector<Vector2>        TexCoord0;      //!< テクスチャ座標0です.
    std::vector<Vector2>        TexCoord1;      //!< テクスチャ座標1です.
    std::vector<Vector2>        TexCoord2;      //!< テクスチャ座標2です.
    std::vector<Vector2>        TexCoord3;      //!< テクスチャ座標3です.
    std::vector<Vector4>        BoneWeights;    //!< ボーンの重みです.
    std::vector<ResBoneIndex>   BoneIndices;    //!< ボーン番号ですです.
    std::vector<Vector4>        Colors;         //!< カラーデータです.
    std::vector<uint32_t>       Indices;        //!< ユニーク頂点番号です.
    uint32_t                    MaterialHash;   //!< マテリアルハッシュです.
};

///////////////////////////////////////////////////////////////////////////////
// ResModel structure
///////////////////////////////////////////////////////////////////////////////
struct ResModel
{
    std::vector<ResMesh>        Meshes;         //!< メッシュです.
};

//-----------------------------------------------------------------------------
//      メッシュの破棄処理を行います.
//-----------------------------------------------------------------------------
inline void Dispose(asdx::ResMesh& resource)
{
    resource.Positions.clear();
    resource.Positions.shrink_to_fit();

    resource.TexCoord0.clear();
    resource.TexCoord0.shrink_to_fit();

    resource.TexCoord1.clear();
    resource.TexCoord1.shrink_to_fit();

    resource.TexCoord2.clear();
    resource.TexCoord2.shrink_to_fit();

    resource.TexCoord3.clear();
    resource.TexCoord3.shrink_to_fit();

    resource.BoneIndices.clear();
    resource.BoneIndices.shrink_to_fit();

    resource.BoneWeights.clear();
    resource.BoneWeights.shrink_to_fit();

    resource.Indices.clear();
    resource.Indices.shrink_to_fit();
}

//-----------------------------------------------------------------------------
//      モデルの破棄処理を行います.
//-----------------------------------------------------------------------------
inline void Dispose(asdx::ResModel& resource)
{
    for(size_t i=0; i<resource.Meshes.size(); ++i)
    { Dispose(resource.Meshes[i]); }

    resource.Meshes.clear();
    resource.Meshes.shrink_to_fit();
}


} // namespace asdx
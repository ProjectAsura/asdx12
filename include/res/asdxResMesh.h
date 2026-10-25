//-----------------------------------------------------------------------------
// File : asdxResMesh.h
// Desc : Mesh Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxMath.h>
#include <fnd/asdxStringView.h>
#include <fnd/asdxArrayView.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ResMesh structure
///////////////////////////////////////////////////////////////////////////////
struct ResMesh
{
    StringView          Name;           //!< メッシュ名.
    uint32_t            MaterialId;     //!< マテリアルID.
    ArrayView<Vector3>  Positions;      //!< 位置座標.
    ArrayView<Vector3>  Normals;        //!< 法線ベクトル.
    ArrayView<Vector4>  Tangents;       //!< 接線ベクトル.
    ArrayView<Unorm4>   Colors;         //!< 頂点カラー.
    ArrayView<Vector2>  TexCoords;      //!< テクスチャ座標.
    ArrayView<Vector4>  BoneWeights;    //!< ボーンの重み.
    ArrayView<Uint4>    BoneIndices;    //!< ボーンの番号.
    ArrayView<uint32_t> VertexIndices;  //!< 頂点番号.
};

} // namespace asdx

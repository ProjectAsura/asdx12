﻿//-----------------------------------------------------------------------------
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
#include <string>
#include <fnd/asdxMath.h>


namespace asdx {

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
// ResMeshletBounds structure
///////////////////////////////////////////////////////////////////////////////
struct ResMeshletBounds
{
    Vector4     Sphere;         //!< バウンディングスフィアです.
    uint32_t    NormalCone;     //!< 圧縮済み法錐です.
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
    std::string                         Name;               // メッシュ名.
    uint32_t                            MaterialId;         // マテリアルID.
    bool                                Visible;            // 可視フラグ.
    uint32_t                            BoneWeightStride;   // 1頂点あたりのボーンの重み数
    std::vector<asdx::Vector3>          Positions;          // 頂点位置.
    std::vector<asdx::Vector3>          Normals;            // 法線ベクトル.
    std::vector<asdx::Vector3>          Tangents;           // 接線ベクトル.
    std::vector<asdx::Vector4>          Colors;             // 頂点カラー.
    std::vector<asdx::Vector2>          TexCoords[4];       // テクスチャ座標.
    std::vector<uint16_t>               BoneIndices;        // ボーン番号.
    std::vector<float>                  BoneWeights;        // ボーン重み.
    std::vector<uint32_t>               Indices;            // 頂点インデックス.

    //-------------------------------------------------------------------------
    //! @brief      破棄処理を行います.
    //-------------------------------------------------------------------------
    void Dispose();
};

///////////////////////////////////////////////////////////////////////////////
// ResModel structure
///////////////////////////////////////////////////////////////////////////////
struct ResModel
{
    std::string                 Name;       // モデル名.
    bool                        Visible;    // 可視フラグ.
    std::vector<ResMesh>        Meshes;     // メッシュ.
    std::vector<std::string>    MaterialNames;

    //-------------------------------------------------------------------------
    //! @brief      破棄処理を行います.
    //-------------------------------------------------------------------------
    void Dispose();

    //-------------------------------------------------------------------------
    //! @brief      ファイルからモデルリソースを生成します.
    //! 
    //! @param[in]      filename        ファイル名です./
    //! @retval true    リソース生成に成功.
    //! @retval false   リソース生成に失敗.
    //-------------------------------------------------------------------------
    bool LoadFromFileA(const char* filename);

    //-------------------------------------------------------------------------
    //! @brief      ファイルからモデルリソースを生成します.
    //! 
    //! @param[in]      filename        ファイル名です.
    //! @retval true    リソース生成に成功.
    //! @retval false   リソース生成に失敗.
    //-------------------------------------------------------------------------
    bool LoadFromFileW(const wchar_t* filename);
};

//-----------------------------------------------------------------------------
//      八面体ラップ処理を行います.
//-----------------------------------------------------------------------------
Vector2 OctWrap(const Vector2& value);

//-----------------------------------------------------------------------------
//      法線ベクトルをパッキングします.
//-----------------------------------------------------------------------------
Vector2 PackNormal(const Vector3& value);

//-----------------------------------------------------------------------------
//      法線ベクトルをアンパッキングします.
//-----------------------------------------------------------------------------
Vector3 UnpackNormal(const Vector2& value);

//-----------------------------------------------------------------------------
//      接線空間を圧縮します.
//-----------------------------------------------------------------------------
uint32_t EncodeTBN(
    const Vector3&  normal,
    const Vector3&  tangent,
    uint8_t         binormalHandedeness);

//-----------------------------------------------------------------------------
//      圧縮された接線空間を展開します.
//-----------------------------------------------------------------------------
void DecodeTBN(
    uint32_t encoded,
    Vector3& tangent,
    Vector3& bitangent,
    Vector3& normal);

} // namespace asdx

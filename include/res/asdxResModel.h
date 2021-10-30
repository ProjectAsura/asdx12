//-----------------------------------------------------------------------------
// File : asdxResModel.h
// Desc : Model Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

#ifndef ASDX_ENABLE_ASSIMP
#define ASDX_ENABLE_ASSIMP  (0)
#endif//ASDX_ENABLE_ASSIMP

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdint>
#include <vector>
#include <string>
#include <fnd/asdxMath.h>

#if ASDX_ENABLE_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#endif


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
    Vector4     Sphere;             //!< バウンディングスフィアです.
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
// ResClusterMesh structure
///////////////////////////////////////////////////////////////////////////////
struct ResClusterMesh
{
    const ResMesh*              Geometry = nullptr;
    std::vector<ResMeshlet>     Meshlets;
    std::vector<ResPrimitive>   Primitives;
    std::vector<uint32_t>       UniqueVertexIndices;

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
    std::string                 Name;           // モデル名.
    bool                        Visible;        // 可視フラグ.
    std::vector<ResMesh>        Meshes;         // メッシュ.
    std::vector<std::string>    MaterialNames;  // マテリアル名.

    //-------------------------------------------------------------------------
    //! @brief      破棄処理を行います.
    //-------------------------------------------------------------------------
    void Dispose();
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

//-----------------------------------------------------------------------------
//      メッシュを最適化します.
//-----------------------------------------------------------------------------
void OptimizeMesh(ResMesh& mesh);

//-----------------------------------------------------------------------------
//      クラスターメッシュを生成します.
//-----------------------------------------------------------------------------
void CreateClusterMesh(const ResMesh& mesh, ResClusterMesh& clusterMesh);


#if ASDX_ENABLE_ASSIMP
///////////////////////////////////////////////////////////////////////////////
// MeshLoader class
///////////////////////////////////////////////////////////////////////////////
class MeshLoader
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================
    MeshLoader() = default;
    ~MeshLoader() = default;

    //-------------------------------------------------------------------------
    //! @brief      モデルをロードします.
    //-------------------------------------------------------------------------
    bool Load(const char* path, ResModel& model)
    {
        if (path == nullptr)
        { return false; }

        Assimp::Importer importer;
        unsigned int flag = 0;
        flag |= aiProcess_Triangulate;
        flag |= aiProcess_PreTransformVertices;
        flag |= aiProcess_CalcTangentSpace;
        flag |= aiProcess_GenSmoothNormals;
        flag |= aiProcess_GenUVCoords;
        flag |= aiProcess_RemoveRedundantMaterials;
        flag |= aiProcess_OptimizeMeshes;

        // ファイルを読み込み.
        m_pScene = importer.ReadFile(path, flag);

        // チェック.
        if (m_pScene == nullptr)
        { return false; }

        // メッシュのメモリを確保.
        auto meshCount = m_pScene->mNumTextures;
        model.Meshes.clear();
        model.Meshes.resize(meshCount);

        // メッシュデータを変換.
        for(size_t i=0; i<meshCount; ++i)
        {
            const auto pMesh = m_pScene->mMeshes[i];
            ParseMesh(model.Meshes[i], pMesh);
        }


        // マテリアル名取得.
        auto materialCount = m_pScene->mNumMaterials;
        model.MaterialNames.resize(materialCount);
        for(size_t i=0; i<materialCount; ++i)
        {
            aiString name;
            aiGetMaterialString(m_pScene->mMaterials[i], AI_MATKEY_NAME, &name);
            model.MaterialNames[i] = name.C_Str();
        }

        // 不要になったのでクリア.
        importer.FreeScene();
        m_pScene = nullptr;

        // 正常終了.
        return true;
    }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    const aiScene*  m_pScene = nullptr;

    //=========================================================================
    // private methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      メッシュを解析します.
    //-------------------------------------------------------------------------
    void ParseMesh(ResMesh& dstMesh, const aiMesh* srcMesh)
    {
        // マテリアル番号を設定.
        dstMesh.MaterialId = srcMesh->mMaterialIndex;

        aiVector3D zero3D(0.0f, 0.0f, 0.0f);

        // 頂点データのメモリを確保.
        auto vertexCount = srcMesh->mNumVertices;
        dstMesh.Positions.resize(vertexCount);

        if (srcMesh->HasNormals())
        { dstMesh.Normals.resize(vertexCount); }

        if (srcMesh->HasTangentsAndBitangents()) 
        { dstMesh.Tangents.resize(vertexCount); }

        if (srcMesh->HasVertexColors(0))
        { dstMesh.Colors.resize(vertexCount); }

        auto uvCount = srcMesh->GetNumUVChannels();
        uvCount = (uvCount > 4) ? 4 : uvCount;

        for(auto c=0u; c<uvCount; ++c) {
            assert(srcMesh->HasTextureCoords(c));
            dstMesh.TexCoords[c].resize(vertexCount);
        }

        for(auto i=0u; i<vertexCount; ++i)
        {
            auto& pos = srcMesh->mVertices[i];
            dstMesh.Positions[i] = asdx::Vector3(pos.x, pos.y, pos.z);

            if (srcMesh->HasNormals())
            {
                auto& normal = srcMesh->mNormals[i];
                dstMesh.Normals[i] = asdx::Vector3(normal.x, normal.y, normal.z);
            }

            if (srcMesh->HasTangentsAndBitangents())
            {
                auto& tangent = srcMesh->mTangents[i];
                dstMesh.Tangents[i] = asdx::Vector3(tangent.x, tangent.y, tangent.z);
            }
            if (srcMesh->HasVertexColors(0))
            {
                auto& color = srcMesh->mColors[0][i];
                dstMesh.Colors[i] = asdx::Vector4(color.r, color.g, color.b, color.a);
            }
            for(auto c=0u; c<uvCount; ++c)
            {
                auto& uv = srcMesh->mTextureCoords[c][i];
                dstMesh.TexCoords[c][i] = asdx::Vector2(uv.x, uv.y);
            }
        }

        // 頂点インデックスのメモリを確保.
        auto indexCount = srcMesh->mNumFaces * 3;
        dstMesh.Indices.resize(indexCount);

        for(size_t i=0; i<indexCount; ++i)
        {
            const auto& face = srcMesh->mFaces[i];
            assert(face.mNumIndices == 3);  // 三角形化しているので必ず3になっている.

            dstMesh.Indices[i * 3 + 0] = face.mIndices[0];
            dstMesh.Indices[i * 3 + 1] = face.mIndices[1];
            dstMesh.Indices[i * 3 + 2] = face.mIndices[2];
        }
    }
};
#endif//ASDX_ENABLE_ASSIMP



} // namespace asdx

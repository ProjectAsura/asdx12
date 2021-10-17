//-----------------------------------------------------------------------------
// File : asdxResModel.cpp
// Desc : Model Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <res/asdxResModel.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxMisc.h>
#include <meshoptimizer.h>
#if ASDX_ENABLE_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#endif

namespace {

///////////////////////////////////////////////////////////////////////////////
// TangentSpace
///////////////////////////////////////////////////////////////////////////////
union TangentSpace
{
    struct {
        uint32_t    NormalX             : 10;
        uint32_t    NormalY             : 10;
        uint32_t    CosAngle            : 8;
        uint32_t    CompIndex           : 2;
        uint32_t    TangentHandedness   : 1;
        uint32_t    BinormalHandedness  : 1;
    };
    uint32_t u;
};
static_assert(sizeof(TangentSpace) == sizeof(uint32_t), "TangentSpace Invalid Data Size");

///////////////////////////////////////////////////////////////////////////////
// TexCoord
///////////////////////////////////////////////////////////////////////////////
union TexCoord
{
    struct 
    {
        uint16_t x;
        uint16_t y;
    };
    uint32_t u;
};

///////////////////////////////////////////////////////////////////////////////
// Unorm88
///////////////////////////////////////////////////////////////////////////////
union Unorm8888
{
    struct
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
    uint32_t c;
};

//-----------------------------------------------------------------------------
//      最大成分を取得します.
//-----------------------------------------------------------------------------
inline float Max3(const asdx::Vector3& value)
{ return asdx::Max(value.x, asdx::Max(value.y, value.z)); }

//-----------------------------------------------------------------------------
//      [0, 1]の実数を符号なし整数32bitに変換します.
//-----------------------------------------------------------------------------
inline uint32_t ToUnorm8(const asdx::Vector4& value)
{
    Unorm8888 packed;
    packed.r = uint8_t(value.x * 255.0f);
    packed.g = uint8_t(value.y * 255.0f);
    packed.b = uint8_t(value.z * 255.0f);
    packed.a = uint8_t(value.w * 255.0f);
    return packed.c;
}

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
    bool Load(const char* path, asdx::ResModel& model)
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
    void ParseMesh(asdx::ResMesh& dstMesh, const aiMesh* srcMesh)
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

        // 最適化.
        {
            std::vector<uint32_t> remap(dstMesh.Indices.size());

            // 重複データを削除するための再マッピング用インデックスを生成.
            auto vertexCount = meshopt_generateVertexRemap(
                remap.data(),
                dstMesh.Indices.data(),
                dstMesh.Indices.size(),
                dstMesh.Positions.data(),
                dstMesh.Positions.size(),
                sizeof(asdx::Vector3));

            std::vector<asdx::Vector3> vertices(vertexCount);
            std::vector<uint32_t> indices(dstMesh.Indices.size());

            // 頂点インデックスを再マッピング.
            meshopt_remapIndexBuffer(
                indices.data(),
                dstMesh.Indices.data(),
                dstMesh.Indices.size(),
                remap.data());

            // 頂点データを再マッピング.
            meshopt_remapVertexBuffer(
                vertices.data(),
                dstMesh.Positions.data(),
                dstMesh.Positions.size(),
                sizeof(asdx::Vector3),
                remap.data());

            // 不要になったメモリを解放.
            remap.clear();
            remap.shrink_to_fit();

            // 最適化したサイズにメモリ量を減らす.
            dstMesh.Positions.resize(vertices.size());
            dstMesh.Indices .resize(indices .size());

            // 頂点キャッシュ最適化.
            meshopt_optimizeVertexCache(
                dstMesh.Indices.data(),
                indices.data(),
                indices.size(),
                vertexCount);

            // 不要になったメモリを解放.
            indices.clear();
            indices.shrink_to_fit();

            // 頂点フェッチ最適化.
            meshopt_optimizeVertexFetch(
                dstMesh.Positions.data(),
                dstMesh.Indices.data(),
                dstMesh.Indices.size(),
                vertices.data(),
                vertices.size(),
                sizeof(asdx::Vector3));

            // 不要になったメモリを解放.
            vertices.clear();
            vertices.shrink_to_fit();
        }

#if 0
        //// メッシュレット生成.
        //{
        //    const size_t kMaxVertices   = 64;
        //    const size_t kMaxPrimitives = 126;
        //    float coneWeight = 0.0f;

        //    auto maxMeshlets = meshopt_buildMeshletsBound(
        //            dstMesh.Indices.size(),
        //            kMaxVertices,
        //            kMaxPrimitives);

        //    std::vector<meshopt_Meshlet> meshlets(maxMeshlets);
        //    std::vector<uint32_t> meshletVertices(maxMeshlets * kMaxVertices);
        //    std::vector<uint8_t> meshletTriangles(maxMeshlets * kMaxPrimitives * 3);

        //    auto meshletCount = meshopt_buildMeshlets(
        //        meshlets.data(),
        //        meshletVertices.data(),
        //        meshletTriangles.data(),
        //        dstMesh.Indices.data(),
        //        dstMesh.Indices.size(),
        //        &dstMesh.Positions[0].x,
        //        dstMesh.Positions.size(),
        //        sizeof(asdx::Vector3),
        //        kMaxVertices,
        //        kMaxPrimitives,
        //        coneWeight);

        //    // 最大値でメモリを予約.
        //    dstMesh.UniqueVertexIndices.reserve(meshlets.size() * kMaxVertices);
        //    dstMesh.Primitives   .reserve(meshlets.size() * kMaxPrimitives);

        //    for(auto& meshlet : meshlets)
        //    {
        //        auto vertexOffset    = uint32_t(dstMesh.UniqueVertexIndices.size());
        //        auto primitiveOffset = uint32_t(dstMesh.Primitives         .size());

        //        for(auto i=0u; i<meshlet.vertex_count; ++i)
        //        { dstMesh.UniqueVertexIndices.push_back(meshletVertices[i]); }

        //        for(size_t i=0; i<meshlet.triangle_count; i+=3)
        //        {
        //            asdx::ResPrimitive tris = {};
        //            tris.Index1 = meshletTriangles[i + 0];
        //            tris.Index0 = meshletTriangles[i + 1];
        //            tris.Index2 = meshletTriangles[i + 2];
        //            dstMesh.Primitives.push_back(tris);
        //        }

        //        // メッシュレットデータ設定.
        //        asdx::ResMeshlet m = {};
        //        m.VertexCount       = meshlet.vertex_count;
        //        m.VertexOffset      = vertexOffset;
        //        m.PrimitiveCount    = meshlet.triangle_count;
        //        m.PrimitiveOffset   = primitiveOffset;

        //        dstMesh.Meshlets.push_back(m);

        //        // バウンディングを求める.
        //        auto bounds = meshopt_computeMeshletBounds(
        //            &meshletVertices[meshlet.vertex_offset],
        //            &meshletTriangles[meshlet.triangle_offset],
        //            meshlet.triangle_count,
        //            &dstMesh.Positions[0].x,
        //            dstMesh.Positions.size(),
        //            sizeof(asdx::Vector3));

        //        // カリングデータ設定.
        //        auto normalCone = asdx::Vector4(
        //            asdx::Saturate(bounds.cone_axis[0] * 0.5f + 0.5f),
        //            asdx::Saturate(bounds.cone_axis[1] * 0.5f + 0.5f),
        //            asdx::Saturate(bounds.cone_axis[2] * 0.5f + 0.5f),
        //            asdx::Saturate(bounds.cone_cutoff * 0.5f + 0.5f));

        //        asdx::ResMeshletBounds c = {};
        //        c.Sphere     = asdx::Vector4(bounds.center[0], bounds.center[1], bounds.center[2], bounds.radius);
        //        c.NormalCone = ToUnorm8(normalCone);

        //        dstMesh.Bounds.push_back(c);
        //    }

        //    // サイズ最適化.
        //    dstMesh.UniqueVertexIndices .shrink_to_fit();
        //    dstMesh.Primitives          .shrink_to_fit();
        //    dstMesh.Meshlets            .shrink_to_fit();
        //    dstMesh.Bounds              .shrink_to_fit();
        //}
#endif
    }
};
#endif//ASDX_ENABLE_ASSIMP

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ResMesh structure
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      メッシュの破棄処理を行います.
//-----------------------------------------------------------------------------
void ResMesh::Dispose()
{
    Name.clear();
    MaterialId = UINT32_MAX;
    Positions.clear();
    Positions.shrink_to_fit();

    Normals.clear();
    Normals.shrink_to_fit();

    Tangents.clear();
    Tangents.shrink_to_fit();

    Colors.clear();
    Colors.shrink_to_fit();

    for(auto i=0; i<4; ++i)
    {
        TexCoords[i].clear();
        TexCoords[i].shrink_to_fit();
    }

    BoneIndices.clear();
    BoneIndices.shrink_to_fit();

    BoneWeights.clear();
    BoneWeights.shrink_to_fit();

    Indices.clear();
    Indices.shrink_to_fit();

    BoneWeightStride = 0;
    Visible = false;
}

///////////////////////////////////////////////////////////////////////////////
// ResModel structure
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      モデルの破棄処理を行います.
//-----------------------------------------------------------------------------
void ResModel::Dispose()
{
    for(size_t i=0; i<Meshes.size(); ++i)
    { Meshes[i].Dispose(); }

    Meshes.clear();
    Meshes.shrink_to_fit();

    Visible = false;
    Name.clear();
}

#if ASDX_ENABLE_ASSIMP
//-----------------------------------------------------------------------------
//      ファイルからリソースを生成します.
//-----------------------------------------------------------------------------
bool ResModel::LoadFromFileA(const char* filename)
{
    MeshLoader loader;
    return loader.Load(filename, *this);
}

//-----------------------------------------------------------------------------
//      ファイルからリソースを生成します.
//-----------------------------------------------------------------------------
bool ResModel::LoadFromFileW(const wchar_t* filename)
{
    auto path = ToStringUTF8(filename);
    return LoadFromFileA(path.c_str());
}
#endif//ASDX_ENABLE_ASSIMP

//-----------------------------------------------------------------------------
//      八面体ラップ処理を行います.
//-----------------------------------------------------------------------------
Vector2 OctWrap(const Vector2& value)
{
    Vector2 result;
    result.x = (1.0f - abs(value.y)) * (value.x >= 0.0f ? 1.0f : -1.0f);
    result.y = (1.0f - abs(value.x)) * (value.y >= 0.0f ? 1.0f : -1.0f);
    return result;
}

//-----------------------------------------------------------------------------
//      法線ベクトルをパッキングします.
//-----------------------------------------------------------------------------
Vector2 PackNormal(const Vector3& value)
{
    auto n = value / (abs(value.x) + abs(value.y) + abs(value.z));
    Vector2 result(n.x, n.y);
    result = (n.z >= 0.0f) ? result : OctWrap(result);
    result = result * 0.5f + Vector2(0.5f, 0.5f);
    return result;
}

//-----------------------------------------------------------------------------
//      法線ベクトルをアンパッキングします.
//-----------------------------------------------------------------------------
Vector3 UnpackNormal(const Vector2& value)
{
    auto encoded = value * 2.0f - Vector2(1.0f, 1.0f);
    auto n = Vector3(encoded.x, encoded.y, 1.0f - abs(encoded.x) - abs(encoded.y));
    auto t = Saturate(-n.z);
    n.x += (n.x >= 0.0f) ? -t : t;
    n.y += (n.y >= 0.0f) ? -t : t;
    return Vector3::Normalize(n);
}

//-----------------------------------------------------------------------------
//      接線空間を圧縮します.
//-----------------------------------------------------------------------------
uint32_t EncodeTBN(const Vector3& normal, const Vector3& tangent, uint8_t binormalHandedness)
{
    auto packedNormal = PackNormal(normal);

    TangentSpace packed = {};
    packed.NormalX = uint32_t(packedNormal.x * 1023.0);
    packed.NormalY = uint32_t(packedNormal.y * 1023.0);

    auto tangentAbs = Vector3::Abs(tangent);
    auto maxComp = Max3(tangentAbs);

    Vector3 refVector;
    uint32_t compIndex = 0;
    if (maxComp == tangentAbs.x)
    {
        refVector = Vector3(1.0f, 0.0f, 0.0f);
        compIndex = 0;
    }
    else if (maxComp == tangentAbs.y)
    {
        refVector = Vector3(0.0f, 1.0f, 0.0f);
        compIndex = 1;
    }
    else if (maxComp == tangentAbs.z)
    {
        refVector = Vector3(0.0f, 0.0f, 1.0f);
        compIndex = 2;
    }

    auto orthoA = Vector3::Normalize(Vector3::Cross(normal, refVector));
    auto orthoB = Vector3::Cross(normal, orthoA);
    uint8_t cosAngle = uint8_t((Vector3::Dot(tangent, orthoA) * 0.5f + 0.5f) * 255.0f);
    uint8_t tangentHandedness = (Vector3::Dot(tangent, orthoB) > 0.0001f) ? 1 : 0;

    packed.CompIndex            = compIndex;
    packed.CosAngle             = cosAngle;
    packed.TangentHandedness    = tangentHandedness;
    packed.BinormalHandedness   = binormalHandedness;

    return packed.u;
}

//-----------------------------------------------------------------------------
//      圧縮された接線空間を展開します.
//-----------------------------------------------------------------------------
void DecodeTBN(uint32_t encoded, Vector3& tangent, Vector3& bitangent, Vector3& normal)
{
    TangentSpace packed;
    packed.u = encoded;

    normal = UnpackNormal(Vector2(packed.NormalX / 1023.0f, packed.NormalY / 1023.0f));

    Vector3 refVector;
    uint8_t compIndex = (packed.CompIndex);
    if (compIndex == 0)
    { refVector = Vector3(1.0f, 0.0f, 0.0f); }
    else if (compIndex == 1)
    { refVector = Vector3(0.0f, 1.0f, 0.0f); }
    else if (compIndex == 2)
    { refVector = Vector3(0.0f, 0.0f, 1.0f); }

    float cosAngle = (packed.CosAngle / 255.0f) * 2.0f - 1.0f;
    float sinAngle = sqrt(Saturate(1.0f - cosAngle * cosAngle));
    sinAngle = (packed.TangentHandedness == 0) ? -sinAngle : sinAngle;

    auto orthoA = Vector3::Normalize(Vector3::Cross(normal, refVector));
    auto orhotB = Vector3::Cross(normal, orthoA);
    tangent = Vector3::Normalize((cosAngle * orthoA) + (sinAngle * orhotB));

    bitangent = Vector3::Cross(normal, tangent);
    bitangent = (packed.BinormalHandedness == 0) ? bitangent : -bitangent;
}


} // namespace asdx

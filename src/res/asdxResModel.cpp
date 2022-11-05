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
#include <meshoptimizer.h>


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

///////////////////////////////////////////////////////////////////////////////
// ResModelHeader structure
///////////////////////////////////////////////////////////////////////////////
struct ResModelHeader
{
    uint8_t     Magic[4];           // 'MDL\0"
    uint32_t    NameLength;         // モデル名の長さ.
    uint32_t    MeshCount;          // メッシュ数.
    uint32_t    MaterialCount;      // マテリアル数.
};

///////////////////////////////////////////////////////////////////////////////
// ResMeshHeader structure
///////////////////////////////////////////////////////////////////////////////
struct ResMeshHeader
{
    uint32_t    NameLength;
    uint32_t    MaterialId;
    uint32_t    PositionCount;
    uint32_t    NormalCount;
    uint32_t    TangentCount;
    uint32_t    ColorCount;
    uint32_t    TexCoordCount[4];
    uint32_t    BoneIndexCount;
    uint32_t    BoneWeightCount;
    uint32_t    IndexCount;
};

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

    Name.clear();
}

//-----------------------------------------------------------------------------
//      ファイルからロードします.
//-----------------------------------------------------------------------------
bool ResModel::LoadFromFileA(const char* filename)
{
    FILE* pFile = nullptr;
    auto err = fopen_s(&pFile, filename, "rb");
    if (err != 0)
    {
        ELOGA("Error : File Open Failed. path = %s", filename);
        return false;
    }

    ResModelHeader header = {};
    fread(&header, sizeof(header), 1, pFile);

    if (header.Magic[0] != 'M' ||
        header.Magic[1] != 'D' ||
        header.Magic[2] != 'L' ||
        header.Magic[3] != '\0')
    {
        fclose(pFile);
        ELOGA("Error : Invalid File.");
        return false;
    }

    Name.resize(header.NameLength);
    fread(&Name[0], sizeof(char), header.NameLength, pFile);

    Meshes.resize(header.MeshCount);
    for(auto i=0u; i<header.MeshCount; ++i)
    {
        ResMeshHeader meshHeader = {};
        fread(&meshHeader, sizeof(meshHeader), 1, pFile);

        auto& dstMesh = Meshes[i];
        dstMesh.Name.resize(meshHeader.NameLength);
        fread(&dstMesh.Name[0], sizeof(char), meshHeader.NameLength, pFile);

        dstMesh.MaterialId = meshHeader.MaterialId;

        dstMesh.Positions.resize(meshHeader.PositionCount);
        fread(dstMesh.Positions.data(), sizeof(asdx::Vector3), meshHeader.PositionCount, pFile);

        if (meshHeader.NormalCount > 0)
        {
            dstMesh.Normals.resize(meshHeader.NormalCount);
            fread(dstMesh.Normals.data(), sizeof(asdx::Vector3), meshHeader.NormalCount, pFile);
        }

        if (meshHeader.TangentCount > 0)
        {
            dstMesh.Tangents.resize(meshHeader.TangentCount);
            fread(dstMesh.Tangents.data(), sizeof(asdx::Vector3), meshHeader.TangentCount, pFile);
        }

        if (meshHeader.ColorCount > 0)
        {
            dstMesh.Colors.resize(meshHeader.ColorCount);
            fread(dstMesh.Colors.data(), sizeof(asdx::Vector4), meshHeader.ColorCount, pFile);
        }

        for(auto j=0; j<4; ++j)
        {
            if (meshHeader.TexCoordCount[j] > 0)
            {
                dstMesh.TexCoords[j].resize(meshHeader.TexCoordCount[j]);
                fread(dstMesh.TexCoords[j].data(), sizeof(asdx::Vector2), meshHeader.TexCoordCount[j], pFile);
            }
        }

        if (meshHeader.BoneIndexCount > 0)
        {
            dstMesh.BoneIndices.resize(meshHeader.BoneIndexCount);
            fread(dstMesh.BoneIndices.data(), sizeof(ResBoneIndex), meshHeader.BoneIndexCount, pFile);
        }

        if (meshHeader.BoneWeightCount > 0)
        {
            dstMesh.BoneWeights.resize(meshHeader.BoneWeightCount);
            fread(dstMesh.BoneWeights.data(), sizeof(asdx::Vector4), meshHeader.BoneWeightCount, pFile);
        }

        if (meshHeader.IndexCount > 0)
        {
            dstMesh.Indices.resize(meshHeader.IndexCount);
            fread(dstMesh.Indices.data(), sizeof(uint32_t), meshHeader.IndexCount, pFile);
        }
    }

    MaterialTags.resize(header.MaterialCount);
    fread(&MaterialTags[0], sizeof(ResMaterialTag), header.MaterialCount, pFile);

    fclose(pFile);
    return true;
}

//-----------------------------------------------------------------------------
//      ファイルにセーブします.
//-----------------------------------------------------------------------------
bool ResModel::SaveToFileA(const char* filename)
{
    FILE* pFile = nullptr;
    auto err = fopen_s(&pFile, filename, "wb");
    if (err != 0)
    {
        ELOGA("Error : File Open Failed. path = %s", filename);
        return false;
    }

    ResModelHeader header = {};
    header.Magic[0] = 'M';
    header.Magic[1] = 'D';
    header.Magic[2] = 'L';
    header.Magic[3] = '\0';

    header.NameLength       = uint32_t(Name.length());
    header.MeshCount        = uint32_t(Meshes.size());
    header.MaterialCount    = uint32_t(MaterialTags.size());

    fwrite(&header, sizeof(header), 1, pFile);

    fwrite(Name.data(), sizeof(char), Name.length(), pFile);

    for(size_t i=0; i<Meshes.size(); ++i)
    {
        const auto& srcMesh = Meshes[i];

        ResMeshHeader meshHeader = {};
        meshHeader.NameLength       = uint32_t(srcMesh.Name.length());
        meshHeader.MaterialId       = srcMesh.MaterialId;
        meshHeader.PositionCount    = uint32_t(srcMesh.Positions.size());
        meshHeader.NormalCount      = uint32_t(srcMesh.Normals.size());
        meshHeader.TangentCount     = uint32_t(srcMesh.Tangents.size());
        meshHeader.ColorCount       = uint32_t(srcMesh.Colors.size());
        for(auto j=0; j<4; ++j)
        { meshHeader.TexCoordCount[j] = uint32_t(srcMesh.TexCoords[j].size()); }
        meshHeader.BoneIndexCount   = uint32_t(srcMesh.BoneIndices.size());
        meshHeader.BoneWeightCount  = uint32_t(srcMesh.BoneWeights.size());
        meshHeader.IndexCount       = uint32_t(srcMesh.Indices.size());

        fwrite(&meshHeader, sizeof(meshHeader), 1, pFile);

        assert(!srcMesh.Positions.empty());
        fwrite(srcMesh.Positions.data(), sizeof(asdx::Vector3), srcMesh.Positions.size(), pFile);

        if (!srcMesh.Normals.empty())
        { fwrite(srcMesh.Normals.data(), sizeof(asdx::Vector3), srcMesh.Normals.size(), pFile); }

        if (!srcMesh.Tangents.empty())
        { fwrite(srcMesh.Tangents.data(), sizeof(asdx::Vector3), srcMesh.Tangents.size(), pFile); }

        if (!srcMesh.Colors.empty())
        { fwrite(srcMesh.Colors.data(), sizeof(asdx::Vector4), srcMesh.Colors.size(), pFile); }

        for(auto j=0; j<4; ++j)
        {
            if (!srcMesh.TexCoords[j].empty())
            { fwrite(srcMesh.TexCoords[j].data(), sizeof(asdx::Vector2), srcMesh.TexCoords[j].size(), pFile); }
        }

        if (!srcMesh.BoneIndices.empty())
        { fwrite(srcMesh.BoneIndices.data(), sizeof(ResBoneIndex), srcMesh.BoneIndices.size(), pFile); }

        if (!srcMesh.BoneWeights.empty())
        { fwrite(srcMesh.BoneWeights.data(), sizeof(asdx::Vector4), srcMesh.BoneWeights.size(), pFile); }

        if (!srcMesh.Indices.empty())
        { fwrite(srcMesh.Indices.data(), sizeof(uint32_t), srcMesh.Indices.size(), pFile); }
    }

    if (!MaterialTags.empty())
    { fwrite(MaterialTags.data(), sizeof(ResMaterialTag), MaterialTags.size(), pFile); }

    fclose(pFile);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// ResClusterMesh structure
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      クラスターメッシュの破棄処理を行います.
//-----------------------------------------------------------------------------
void ResClusterMesh::Dispose()
{
    Meshlets.clear();
    Meshlets.shrink_to_fit();

    Primitives.clear();
    Primitives.shrink_to_fit();

    VertexIndices.clear();
    VertexIndices.shrink_to_fit();
}

//-----------------------------------------------------------------------------
//      クラスターメッシュを生成します.
//-----------------------------------------------------------------------------
void ResClusterMesh::Create(const ResMesh& mesh)
{
    const size_t kMaxVertices   = 64;
    const size_t kMaxPrimitives = 126;
    float coneWeight = 0.0f;

    auto maxMeshlets = meshopt_buildMeshletsBound(
            mesh.Indices.size(),
            kMaxVertices,
            kMaxPrimitives);

    std::vector<meshopt_Meshlet> meshlets(maxMeshlets);
    std::vector<uint32_t> meshletVertices(maxMeshlets * kMaxVertices);
    std::vector<uint8_t> meshletTriangles(maxMeshlets * kMaxPrimitives * 3);

    auto meshletCount = meshopt_buildMeshlets(
        meshlets.data(),
        meshletVertices.data(),
        meshletTriangles.data(),
        mesh.Indices.data(),
        mesh.Indices.size(),
        &mesh.Positions[0].x,
        mesh.Positions.size(),
        sizeof(asdx::Vector3),
        kMaxVertices,
        kMaxPrimitives,
        coneWeight);

    // 最大値でメモリを予約.
    VertexIndices.reserve(meshlets.size() * kMaxVertices);
    Primitives   .reserve(meshlets.size() * kMaxPrimitives);

    for(auto& meshlet : meshlets)
    {
        auto vertexOffset    = uint32_t(VertexIndices.size());
        auto primitiveOffset = uint32_t(Primitives   .size());

        for(auto i=0u; i<meshlet.vertex_count; ++i)
        { VertexIndices.push_back(meshletVertices[i]); }

        for(size_t i=0; i<meshlet.triangle_count; i+=3)
        {
            asdx::ResPrimitive tris = {};
            tris.Index1 = meshletTriangles[i + 0];
            tris.Index0 = meshletTriangles[i + 1];
            tris.Index2 = meshletTriangles[i + 2];
            Primitives.push_back(tris);
        }

        // メッシュレットデータ設定.
        asdx::ResMeshlet m = {};
        m.VertexCount       = meshlet.vertex_count;
        m.VertexOffset      = vertexOffset;
        m.PrimitiveCount    = meshlet.triangle_count;
        m.PrimitiveOffset   = primitiveOffset;

        // バウンディングを求める.
        auto bounds = meshopt_computeMeshletBounds(
            &meshletVertices[meshlet.vertex_offset],
            &meshletTriangles[meshlet.triangle_offset],
            meshlet.triangle_count,
            &mesh.Positions[0].x,
            mesh.Positions.size(),
            sizeof(asdx::Vector3));

        // カリングデータ設定.
        auto normalCone = asdx::Vector4(
            asdx::Saturate(bounds.cone_axis[0] * 0.5f + 0.5f),
            asdx::Saturate(bounds.cone_axis[1] * 0.5f + 0.5f),
            asdx::Saturate(bounds.cone_axis[2] * 0.5f + 0.5f),
            asdx::Saturate(bounds.cone_cutoff * 0.5f + 0.5f));

        m.Sphere     = asdx::Vector4(bounds.center[0], bounds.center[1], bounds.center[2], bounds.radius);
        m.NormalCone = ToUnorm8(normalCone);

        Meshlets.push_back(m);
    }

    // サイズ最適化.
    VertexIndices.shrink_to_fit();
    Primitives   .shrink_to_fit();
    Meshlets     .shrink_to_fit();
}


//-----------------------------------------------------------------------------
//      メッシュを最適化します.
//-----------------------------------------------------------------------------
void OptimizeMesh(ResMesh& mesh)
{
    std::vector<uint32_t> remap(mesh.Indices.size());

    // 重複データを削除するための再マッピング用インデックスを生成.
    auto vertexCount = meshopt_generateVertexRemap(
        remap.data(),
        mesh.Indices.data(),
        mesh.Indices.size(),
        mesh.Positions.data(),
        mesh.Positions.size(),
        sizeof(asdx::Vector3));

    std::vector<asdx::Vector3> vertices(vertexCount);
    std::vector<uint32_t> indices(mesh.Indices.size());

    // 頂点インデックスを再マッピング.
    meshopt_remapIndexBuffer(
        indices.data(),
        mesh.Indices.data(),
        mesh.Indices.size(),
        remap.data());

    // 頂点データを再マッピング.
    meshopt_remapVertexBuffer(
        vertices.data(),
        mesh.Positions.data(),
        mesh.Positions.size(),
        sizeof(asdx::Vector3),
        remap.data());

    // 不要になったメモリを解放.
    remap.clear();
    remap.shrink_to_fit();

    // 最適化したサイズにメモリ量を減らす.
    mesh.Positions.resize(vertices.size());
    mesh.Indices .resize(indices .size());

    // 頂点キャッシュ最適化.
    meshopt_optimizeVertexCache(
        mesh.Indices.data(),
        indices.data(),
        indices.size(),
        vertexCount);

    // 不要になったメモリを解放.
    indices.clear();
    indices.shrink_to_fit();

    // 頂点フェッチ最適化.
    meshopt_optimizeVertexFetch(
        mesh.Positions.data(),
        mesh.Indices.data(),
        mesh.Indices.size(),
        vertices.data(),
        vertices.size(),
        sizeof(asdx::Vector3));

    // 不要になったメモリを解放.
    vertices.clear();
    vertices.shrink_to_fit();
}

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

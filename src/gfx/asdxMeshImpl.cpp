//-----------------------------------------------------------------------------
// File : asdxMeshImpl.cpp
// Desc : Mesh Implementation.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include "asdxMeshImpl.h"
#include <res/asdxResModel.h>
#include <fnd/asdxLogger.h>


namespace {

//-----------------------------------------------------------------------------
//      頂点バッファを初期化します.
//-----------------------------------------------------------------------------
template<typename T>
bool InitVB(asdx::VertexBuffer& dst, const asdx::ArrayView<T>& src)
{
    // 存在しないものもいるので，空なら成功扱い.
    if (src.empty())
        return true;

    if (!dst.Init(sizeof(T) * src.size(), sizeof(T)))
        return false;

    auto ptr = dst.MapAs<T>();
    assert(ptr != nullptr);
    memcpy(ptr, src.data(), sizeof(T) * src.size());
    dst.Unmap();

    return true;
}

//-----------------------------------------------------------------------------
//      インデックスバッファを初期化します.
//-----------------------------------------------------------------------------
bool InitIB(asdx::IndexBuffer& dst, const asdx::ArrayView<uint32_t>& src)
{
    // 絶対に存在するはずなので，空なら失敗扱い.
    if (src.empty())
        return false;

    if (!dst.Init(sizeof(uint32_t) * src.size()))
        return false;

    auto ptr = dst.MapAs<uint32_t>();
    assert(ptr != nullptr);
    memcpy(ptr, src.data(), sizeof(uint32_t) * src.size());
    dst.Unmap();

    return true;
}

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Mesh class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Mesh::Mesh()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Mesh::~Mesh()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Mesh::Init(const ResMesh& mesh)
{
    // 名前を設定.
    m_Name = mesh.Name.c_str();

    // マテリアルIDを設定.
    m_MaterialId = mesh.MaterialId;

    // 位置座標バッファ初期化.
    if (!InitVB(m_Positions, mesh.Positions))
    {
        ELOG("Error : Positions Initialize Failed.");
        return false;
    }

    // 法線バッファ初期化.
    if (!InitVB(m_Normals, mesh.Normals))
    {
        ELOG("Error : Normal Initailize Failed.");
        return false;
    }

    // 接線バッファ初期化.
    if (!InitVB(m_Tangents, mesh.Tangents))
    {
        ELOG("Error : Tangents Initialize Failed.");
        return false;
    }

    // カラーバッファ初期化.
    if (!InitVB(m_Colors, mesh.Colors))
    {
        ELOG("Error : Colors Initialize Failed.");
        return false;
    }

    // テクスチャ座標バッファ初期化.
    if (!InitVB(m_TexCoords, mesh.TexCoords))
    {
        ELOG("Error : TexCoords Initialize Failed.");
        return false;
    }

    // ボーンインデックスバッファ初期化.
    if (!InitVB(m_BoneIndices, mesh.BoneIndices))
    {
        ELOG("Error : BoneIndices Initialize Failed.");
        return false;
    }

    // ボーンウェイトバッファ初期化.
    if (!InitVB(m_BoneWeights, mesh.BoneWeights))
    {
        ELOG("Error : BoneWeights Initailize Failed.");
        return false;
    }

    // 頂点インデックスバッファ初期化.
    if (!InitIB(m_VertexIndices, mesh.VertexIndices))
    {
        ELOG("Error : VertexIndices Initailize Failed.");
        return false;
    }

    // 頂点数を設定.
    m_VertexCount = uint32_t(mesh.Positions.size());

    // 頂点インデックス数を設定.
    m_IndexCount = uint32_t(mesh.VertexIndices.size());

    // バウンディングスフィアを設定.
    m_BoundingSphere = mesh.BoundingSphere;

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Mesh::Term()
{
    m_Name.clear();
    m_VertexCount   = 0;
    m_IndexCount    = 0;
    m_MaterialId    = UINT32_MAX;

    m_Positions     .Term();
    m_Normals       .Term();
    m_Tangents      .Term();
    m_Colors        .Term();
    m_TexCoords     .Term();
    m_BoneIndices   .Term();
    m_BoneWeights   .Term();
    m_VertexIndices .Term();
}

//-----------------------------------------------------------------------------
//      メッシュ名を取得します.
//-----------------------------------------------------------------------------
const std::string& Mesh::GetName() const
{ return m_Name; }

//-----------------------------------------------------------------------------
//      マテリアルIDを取得します.
//-----------------------------------------------------------------------------
uint32_t Mesh::GetMaterialId() const
{ return m_MaterialId; }

//-----------------------------------------------------------------------------
//      位置座標バッファを取得します.
//-----------------------------------------------------------------------------
const VertexBuffer& Mesh::GetPositions() const
{ return m_Positions; }

//-----------------------------------------------------------------------------
//      法線バッファを取得します.
//-----------------------------------------------------------------------------
const VertexBuffer& Mesh::GetNormals() const
{ return m_Normals; }

//-----------------------------------------------------------------------------
//      接線バッファを取得します.
//-----------------------------------------------------------------------------
const VertexBuffer& Mesh::GetTangents() const
{ return m_Tangents; }

//-----------------------------------------------------------------------------
//      カラーバッファを取得します.
//-----------------------------------------------------------------------------
const VertexBuffer& Mesh::GetColors() const
{ return m_Colors; }

//-----------------------------------------------------------------------------
//      テクスチャ座標バッファを取得します.
//-----------------------------------------------------------------------------
const VertexBuffer& Mesh::GetTexCoords() const
{ return m_TexCoords; }

//-----------------------------------------------------------------------------
//      ボーンインデックスバッファを取得します.
//-----------------------------------------------------------------------------
const VertexBuffer& Mesh::GetBoneIndices() const
{ return m_BoneIndices; }

//-----------------------------------------------------------------------------
//      ボーンウェイトバッファを取得します.
//-----------------------------------------------------------------------------
const VertexBuffer& Mesh::GetBoneWeights() const
{ return m_BoneWeights; }

//-----------------------------------------------------------------------------
//      頂点インデックスバッファを取得します.
//-----------------------------------------------------------------------------
const IndexBuffer& Mesh::GetVertexIndices() const
{ return m_VertexIndices; }

//-----------------------------------------------------------------------------
//      頂点数を取得します.
//-----------------------------------------------------------------------------
uint32_t Mesh::GetVertexCount() const
{ return m_VertexCount; }

//-----------------------------------------------------------------------------
//      頂点インデックス数を取得します.
//-----------------------------------------------------------------------------
uint32_t Mesh::GetIndexCount() const
{ return m_IndexCount; }

//-----------------------------------------------------------------------------
//      バウンディングスフィアを取得します.
//-----------------------------------------------------------------------------
const BoundingSphere3& Mesh::GetBoundingSphere() const
{ return m_BoundingSphere; }

//-----------------------------------------------------------------------------
//      レイトレーシングジオメトリトライアングル設定を取得します.
//-----------------------------------------------------------------------------
D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC Mesh::GetRayTracingGeometryTrianglesDesc() const
{
    D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC result = {};
    result.IndexFormat                  = DXGI_FORMAT_R32_UINT;
    result.VertexFormat                 = DXGI_FORMAT_R32G32B32_FLOAT;
    result.IndexCount                   = m_IndexCount;
    result.VertexCount                  = m_VertexCount;
    result.IndexBuffer                  = m_VertexIndices.GetResource()->GetGPUVirtualAddress();
    result.VertexBuffer.StartAddress    = m_Positions    .GetResource()->GetGPUVirtualAddress();
    result.VertexBuffer.StrideInBytes   = sizeof(Vector3);
    return result;
}

} // namespace asdx

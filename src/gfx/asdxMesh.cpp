//------------------M-----------------------------------------------------------
// File : asdxMesh.cpp
// Desc : Polygon Mesh.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include <gfx/asdxMesh.h>
#include <res/asdxResMesh.h>


namespace {

static const uint32_t kStaticMeshElementCount   = 5;
static const uint32_t kSkeletalMeshElementCount = 7;

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
bool Mesh::Init(const ResMesh& res)
{
    // 引数チェック.
    if (res.Positions.empty() || res.VertexIndices.empty())
    {
        ELOGA("Error : Invalid Argument.");
        return false;
    }


    // 位置座標初期化.
    if (!InitVB(m_Positions, res.Positions))
    {
        ELOGA("Error : Positions Initialize Failed.");
        return false;
    }

    // 法線ベクトル初期化.
    if (!InitVB(m_Normals, res.Normals))
    {
        ELOGA("Error : Normals Initialize Failed.");
        return false;
    }

    // 接線ベクトル初期化.
    if (!InitVB(m_Tangents, res.Tangents))
    {
        ELOGA("Error : Tangents Initialize Failed.");
        return false;
    }

    // 頂点カラー初期化.
    if (!InitVB(m_Colors, res.Colors))
    {
        ELOGA("Error : Colors Initialize Failed.");
        return false;
    }

    // テクスチャ座標初期化.
    if (!InitVB(m_TexCoords, res.TexCoords))
    {
        ELOGA("Error : TexCoord Initialize Failed.");
        return false;
    }

    // ボーン番号初期化.
    if (!InitVB(m_BoneIndices, res.BoneIndices))
    {
        ELOGA("Error : BoneIndices Initialize Failed.");
        return false;
    }

    // ボーン重み初期化.
    if (!InitVB(m_BoneWeights, res.BoneWeights))
    {
        ELOGA("Error : BoneWeights Initialize Failed.");
        return false;
    }

    //  頂点インデックス初期化.
    if (!InitIB(m_VertexIndices, res.VertexIndices))
    {
        ELOGA("Error : VertexIndices Initialize Failed.");
        return false;
    }

    // メッシュ名.
    m_Hash = res.Hash;

    // マテリアルID.
    m_MaterialId = res.MaterialId;

    // 頂点数とインデックス数を設定.
    m_VertexCount = uint32_t(res.Positions.size());
    m_IndexCount  = uint32_t(res.VertexIndices.size());

    auto mini = res.Positions[0];
    auto maxi = res.Positions[0];
    for(auto i=1; i<res.Positions.size(); ++i)
    {
        mini = Vector3::Min(mini, res.Positions[i]);
        maxi = Vector3::Max(maxi, res.Positions[i]);
    }
    m_Sphere = BoundingSphere((mini + maxi) * 0.5f, Vector3::Distance(mini, maxi));
    m_Box    = BoundingBox(mini, maxi);

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Mesh::Term()
{
    m_Hash = 0;

    m_Positions     .Term();
    m_Normals       .Term();
    m_Tangents      .Term();
    m_Colors        .Term();
    m_TexCoords     .Term();
    m_BoneIndices   .Term();
    m_BoneWeights   .Term();
    m_VertexIndices .Term();

    m_Sphere = {};
    m_Box    = {};

    m_VertexCount = 0;
    m_IndexCount  = 0;
    m_MaterialId  = UINT32_MAX;
}

//-----------------------------------------------------------------------------
//      メッシュ名のハッシュ値を取得します.
//-----------------------------------------------------------------------------
uint64_t Mesh::GetHash() const
{ return m_Hash; }

//-----------------------------------------------------------------------------
//      マテリアルIDを取得します.
//-----------------------------------------------------------------------------
uint32_t Mesh::GetMaterialId() const
{ return m_MaterialId; }

//-----------------------------------------------------------------------------
//      位置座標を取得します.
//-----------------------------------------------------------------------------
const VertexBuffer& Mesh::GetPositions() const
{ return m_Positions; }

//-----------------------------------------------------------------------------
//      法線ベクトルを取得します.
//-----------------------------------------------------------------------------
const VertexBuffer& Mesh::GetNormals() const
{ return m_Normals; }

//-----------------------------------------------------------------------------
//      接線ベクトルを取得します.
//-----------------------------------------------------------------------------
const VertexBuffer& Mesh::GetTangents() const
{ return m_Tangents; }

//-----------------------------------------------------------------------------
//      頂点カラーを取得します.
//-----------------------------------------------------------------------------
const VertexBuffer& Mesh::GetColors() const
{ return m_Colors; }

//-----------------------------------------------------------------------------
//      テクスチャ座標を取得します.
//-----------------------------------------------------------------------------
const VertexBuffer& Mesh::GetTexCoords() const
{ return m_TexCoords; }

//-----------------------------------------------------------------------------
//      ボーン番号を取得します.
//-----------------------------------------------------------------------------
const VertexBuffer& Mesh::GetBoneIndices() const
{ return m_BoneIndices; }

//-----------------------------------------------------------------------------
//      ボーンの重みを取得します.
//-----------------------------------------------------------------------------
const VertexBuffer& Mesh::GetBoneWeights() const
{ return m_BoneWeights; }

//-----------------------------------------------------------------------------
//      頂点インデックスを取得します.
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
const BoundingSphere& Mesh::GetSphere() const
{ return m_Sphere; }

//-----------------------------------------------------------------------------
//      バウンディングボックスを取得します.
//-----------------------------------------------------------------------------
const BoundingBox& Mesh::GetBox() const
{ return m_Box; }

//-----------------------------------------------------------------------------
//      レイトレーシングジオメトリトライアングル設定を取得します.
//-----------------------------------------------------------------------------
D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC Mesh::GetTrianglesDesc() const
{
    D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC result = {};
    result.IndexFormat                  = DXGI_FORMAT_R32_UINT;
    result.VertexFormat                 = DXGI_FORMAT_R32G32B32_FLOAT;
    result.IndexCount                   = m_IndexCount;
    result.VertexCount                  = m_VertexCount;
    result.IndexBuffer                  = m_VertexIndices.GetGpuAddress();
    result.VertexBuffer.StartAddress    = m_Positions    .GetGpuAddress();
    result.VertexBuffer.StrideInBytes   = sizeof(Vector3);
    return result;
}

//-----------------------------------------------------------------------------
//      メッシュジオメトリを描画します.
//-----------------------------------------------------------------------------
void DrawMesh
(
    ID3D12GraphicsCommandList*  pCmd,
    const Mesh*                 pMesh,
    uint32_t                    instanceCount,
    bool                        isSkeletal
)
{
    if (pCmd == nullptr || pMesh == nullptr || instanceCount == 0)
        return;

    D3D12_VERTEX_BUFFER_VIEW vbvs[] = {
        pMesh->GetPositions  ().GetVBV(),
        pMesh->GetNormals    ().GetVBV(),
        pMesh->GetTangents   ().GetVBV(),
        pMesh->GetTexCoords  ().GetVBV(),
        pMesh->GetColors     ().GetVBV(),
        pMesh->GetBoneIndices().GetVBV(),
        pMesh->GetBoneWeights().GetVBV(),
    };

    auto count = (isSkeletal) ? kSkeletalMeshElementCount : kStaticMeshElementCount;
    auto ibv = pMesh->GetVertexIndices().GetIBV();
    pCmd->IASetVertexBuffers(0, count, vbvs);
    pCmd->IASetIndexBuffer(&ibv);
    pCmd->DrawIndexedInstanced(pMesh->GetIndexCount(), instanceCount, 0, 0, 0);
}

} // namespace asdx

//-----------------------------------------------------------------------------
// File : asdxModel.cpp
// Desc : Model.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxModel.h>
#include <asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Mesh class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Mesh::Mesh()
: m_MaterialHash(0)
, m_MeshletCount(0)
, m_Box({asdx::Vector3(FLT_MAX, FLT_MAX, FLT_MAX), asdx::Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX)})
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Mesh::~Mesh()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Mesh::Init(const ResMesh& resource)
{
    {
        if (!m_Positions.Init(
            uint64_t(resource.Positions.size()),
            uint32_t(sizeof(resource.Positions[0])),
            resource.Positions.data()))
        {
            ELOG("Error : StructuredBuffer::Init() Failed.");
            return false;
        }
    }

    if (resource.TangentSpaces.size() > 0)
    {
        if (!m_TangentSpaces.Init(
            uint64_t(resource.TangentSpaces.size()),
            uint32_t(sizeof(resource.TangentSpaces[0])),
            resource.TangentSpaces.data()))
        {
            ELOG("Error : StructuredBuffer::Init() Failed.");
            return false;
        }
    }

    if (resource.Colors.size() > 0)
    {
        if (!m_Colors.Init(
            uint64_t(resource.Colors.size()),
            uint32_t(sizeof(resource.Colors[0])),
            resource.Colors.data()))
        {
            ELOG("Error : StructuredBuffer::Init() Failed.");
            return false;
        }
    }

    for(auto i=0; i<4; ++i)
    {
        if (resource.TexCoords[i].size() > 0)
        {
            if (!m_TexCoords[i].Init(
                uint64_t(resource.TexCoords[i].size()),
                uint32_t(sizeof(resource.TexCoords[i][0])),
                resource.TexCoords[i].data()))
            {
                ELOG("Error : StructuredBuffer:Init() Failed.");
                return false;
            }
        }
    }

    if (resource.BoneIndices.size() > 0)
    {
        if (!m_BoneIndices.Init(
            uint64_t(resource.BoneIndices.size()),
            uint32_t(sizeof(resource.BoneIndices[0])),
            resource.BoneIndices.data()))
        {
            ELOG("Error : StructuredBuffer::Init() Failed.");
            return false;
        }
    }

    if (resource.BoneWeights.size() > 0)
    {
        if (!m_BoneWeights.Init(
            uint64_t(resource.BoneWeights.size()),
            uint32_t(sizeof(resource.BoneWeights[0])),
            resource.BoneWeights.data()))
        {
            ELOG("Error : StructuredBuffer::Init() Failed.");
            return false;
        }
    }

    {
        if (!m_Indices.Init(
            uint64_t(resource.Indices.size()),
            uint32_t(sizeof(resource.Indices[0])),
            resource.Indices.data()))
        {
            ELOG("Error : StructuredBuffer::Init() Failed.");
            return false;
        }
    }

    {
        if (!m_Primitives.Init(
            uint64_t(resource.Primitives.size()),
            uint32_t(sizeof(resource.Primitives[0])),
            resource.Primitives.data()))
        {
            ELOG("Error : StructuredBuffer::Init() Failed.");
            return false;
        }
    }

    {
        if (!m_Meshlets.Init(
            uint64_t(resource.Meshlets.size()),
            uint32_t(sizeof(resource.Meshlets[0])),
            resource.Meshlets.data()))
        {
            ELOG("Error : StructuredBuffer::Init() Failed");
            return false;
        }
    }

    {
        if (!m_CullingInfos.Init(
            uint64_t(resource.CullingInfos.size()),
            uint32_t(sizeof(resource.CullingInfos[0])),
            resource.CullingInfos.data()))
        {
            ELOG("Error : StructuredBuffer::Init() Failed.");
            return false;
        }
    }

    m_Box.mini = m_Box.maxi = resource.Positions[0];
    for(auto i=1; i<resource.Positions.size(); ++i)
    {
        m_Box.mini = asdx::Vector3::Min(m_Box.mini, resource.Positions[i]);
        m_Box.maxi = asdx::Vector3::Max(m_Box.maxi, resource.Positions[i]);
    }

    m_MaterialHash = resource.MatrerialHash;
    m_MeshletCount = uint32_t(resource.Meshlets.size());

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Mesh::Term()
{
    m_Positions     .Term();
    m_TangentSpaces .Term();
    m_Colors        .Term();
    m_BoneIndices   .Term();
    m_BoneWeights   .Term();
    m_Indices       .Term();
    m_Primitives    .Term();
    m_Meshlets      .Term();
    m_CullingInfos  .Term();

    for(auto i=0; i<4; ++i)
    { m_TexCoords[i].Term(); }

    m_MeshletCount = 0;
    m_MaterialHash = 0;

    m_Box.mini = asdx::Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
    m_Box.maxi = asdx::Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
}

//-----------------------------------------------------------------------------
//      頂点データを取得します.
//-----------------------------------------------------------------------------
const StructuredBuffer& Mesh::GetPositions() const
{ return m_Positions; }

//-----------------------------------------------------------------------------
//      接線空間を取得します.
//-----------------------------------------------------------------------------
const StructuredBuffer& Mesh::GetTangentSpaces() const
{ return m_TangentSpaces; }

//-----------------------------------------------------------------------------
//      頂点カラーを取得します.
//-----------------------------------------------------------------------------
const StructuredBuffer& Mesh::GetColors() const
{ return m_Colors; }

//-----------------------------------------------------------------------------
//      テクスチャ座標を取得します.
//-----------------------------------------------------------------------------
const StructuredBuffer& Mesh::GetTexCoords(uint8_t index) const
{
    assert(index < 4);
    return m_TexCoords[index];
}

//-----------------------------------------------------------------------------
//      ボーン番号を取得します.
//-----------------------------------------------------------------------------
const StructuredBuffer& Mesh::GetBoneIndices() const
{ return m_BoneIndices; }

//-----------------------------------------------------------------------------
//      ボーンの重みを取得します.
//-----------------------------------------------------------------------------
const StructuredBuffer& Mesh::GetBoneWeights() const
{ return m_BoneWeights; }

//-----------------------------------------------------------------------------
//      インデックスデータを取得します.
//-----------------------------------------------------------------------------
const StructuredBuffer& Mesh::GetInindices() const
{ return m_Indices; }

//-----------------------------------------------------------------------------
//      プリミティブデータを取得します.
//-----------------------------------------------------------------------------
const StructuredBuffer& Mesh::GetPrimitives() const
{ return m_Primitives; }

//-----------------------------------------------------------------------------
//      メッシュレットデータのGPU仮想アドレスを取得します.
//-----------------------------------------------------------------------------
const StructuredBuffer& Mesh::GetMeshlets() const
{ return m_Meshlets; }

//-----------------------------------------------------------------------------
//      カリング情報を取得します.
//-----------------------------------------------------------------------------
const StructuredBuffer& Mesh::GetCullingInfos() const
{ return m_CullingInfos; }

//-----------------------------------------------------------------------------
//      メッシュハッシュを取得します.
//-----------------------------------------------------------------------------
uint32_t Mesh::GetMeshHash() const
{ return m_MeshHash; }

//-----------------------------------------------------------------------------
//      マテリアルハッシュを取得します.
//-----------------------------------------------------------------------------
uint32_t Mesh::GetMaterialHash() const
{ return m_MaterialHash; }

//-----------------------------------------------------------------------------
//      メッシュレット数を取得します.
//-----------------------------------------------------------------------------
uint32_t Mesh::GetMeshletCount() const
{ return m_MeshletCount; }

//-----------------------------------------------------------------------------
//      バウンディングボックスを取得します.
//-----------------------------------------------------------------------------
const BoundingBox& Mesh::GetBox() const
{ return m_Box; }

//-----------------------------------------------------------------------------
//      ボーンを持つかどうか?
//-----------------------------------------------------------------------------
bool Mesh::HasBone() const
{ return m_BoneWeights.GetResource() != nullptr; }

//-----------------------------------------------------------------------------
//      接線空間を持つかどうか?
//-----------------------------------------------------------------------------
bool Mesh::HasTangentSpace() const
{ return m_TangentSpaces.GetResource() != nullptr; }

//-----------------------------------------------------------------------------
//      頂点カラーを持つかどうか?
//-----------------------------------------------------------------------------
bool Mesh::HasColor() const
{ return m_Colors.GetResource() != nullptr; }

//-----------------------------------------------------------------------------
//      テクスチャ座標を持つかどうか?
//-----------------------------------------------------------------------------
bool Mesh::HasTexCoord(uint8_t index) const
{
    assert(index < 4);
    return m_TexCoords[index].GetResource() != nullptr;
}


///////////////////////////////////////////////////////////////////////////////
// Model class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Model::Model()
: m_Box({asdx::Vector3(FLT_MAX, FLT_MAX, FLT_MAX), asdx::Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX)})
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Model::~Model()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Model::Init(const ResModel& model)
{
    auto index = 0;
    m_Meshes.resize(model.Meshes.size());

    for(size_t i=0; i<model.Meshes.size(); ++i)
    {
        if (!m_Meshes[index].Init(model.Meshes[i]))
        { return false; }

        auto& box = m_Meshes[index].GetBox();

        if (index == 0)
        { m_Box = box; }
        else
        {
            m_Box.mini = asdx::Vector3::Min(m_Box.mini, box.mini);
            m_Box.maxi = asdx::Vector3::Max(m_Box.maxi, box.maxi);
        }
        index++;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Model::Term()
{
    for(size_t i=0; i<m_Meshes.size(); ++i)
    { m_Meshes[i].Term(); }

    m_Meshes.clear();
    m_Meshes.shrink_to_fit();
}

//-----------------------------------------------------------------------------
//      メッシュ数を取得します.
//-----------------------------------------------------------------------------
uint32_t Model::GetMeshCount() const
{ return uint32_t(m_Meshes.size()); }

//-----------------------------------------------------------------------------
//      メッシュを取得します.
//-----------------------------------------------------------------------------
const Mesh& Model::GetMesh(uint32_t index) const
{
    assert(index < GetMeshCount());
    return m_Meshes[index];
}

//-----------------------------------------------------------------------------
//      ボックスを取得します.
//-----------------------------------------------------------------------------
const BoundingBox& Model::GetBox() const
{ return m_Box; }

} // namespace asdx
//-----------------------------------------------------------------------------
// File : asdxModel.cpp
// Desc : Model.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxModel.h>
#include <gfx/asdxCommandList.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxHash.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Mesh class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Mesh::Mesh()
: m_MaterialId(UINT32_MAX)
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
    m_MeshHash = asdx::CalcHash(resource.Name.c_str());

    {
        auto stride = uint32_t(sizeof(resource.Positions[0]));
        auto size   = uint64_t(resource.Positions.size()) * stride;

        if (!m_Positions.Init(size, stride))
        {
            ELOG("Error : VertexBuffer::Init() Failed.");
            return false;
        }

        auto ptr = m_Positions.Map();
        memcpy(ptr, resource.Positions.data(), size);
        m_Positions.Unmap();
    }

    if (resource.Normals.size() > 0)
    {
        auto stride = uint32_t(sizeof(resource.Normals[0]));
        auto size   = uint64_t(resource.Normals.size()) * stride;

        if (!m_Normals.Init(size, stride))
        {
            ELOG("Error : VertexBuffer::Init() Failed.");
            return false;
        }

        auto ptr = m_Normals.Map();
        memcpy(ptr, resource.Normals.data(), size);
        m_Normals.Unmap();
    }

    if (resource.Tangents.size() > 0)
    {
        auto stride = uint32_t(sizeof(resource.Tangents[0]));
        auto size   = uint64_t(resource.Tangents.size()) * stride;

        if (!m_Tangents.Init(size, stride))
        {
            ELOG("Error : VertexBuffer::Init() Failed.");
            return false;
        }

        auto ptr = m_Tangents.Map();
        memcpy(ptr, resource.Tangents.data(), size);
        m_Tangents.Unmap();
    }

    if (resource.Colors.size() > 0)
    {
        auto stride = uint32_t(sizeof(resource.Colors[0]));
        auto size   = uint64_t(resource.Colors.size()) * stride;

        if (!m_Colors.Init(size, stride))
        {
            ELOG("Error : VertexBuffer::Init() Failed.");
            return false;
        }

        auto ptr = m_Colors.Map();
        memcpy(ptr, resource.Colors.data(), size);
        m_Colors.Unmap();
    }

    for(auto i=0; i<4; ++i)
    {
        if (resource.TexCoords[i].size() > 0)
        {
            auto stride = uint32_t(sizeof(resource.TexCoords[i][0]));
            auto size   = uint64_t(resource.TexCoords[i].size());
            if (!m_TexCoords[i].Init(size, stride))
            {
                ELOG("Error : VertexBuffer:Init() Failed.");
                return false;
            }

            auto ptr = m_TexCoords[i].Map();
            memcpy(ptr, resource.TexCoords[i].data(), size);
            m_TexCoords[i].Unmap();
        }
    }

    if (resource.BoneIndices.size() > 0)
    {
        auto stride = uint32_t(sizeof(resource.BoneIndices[0])) * resource.BoneWeightStride;
        auto size   = uint64_t(resource.BoneIndices.size()) * sizeof(resource.BoneIndices[0]);

        if (!m_BoneIndices.Init(size, stride))
        {
            ELOG("Error : VertexBuffer::Init() Failed.");
            return false;
        }

        auto ptr = m_BoneIndices.Map();
        memcpy(ptr, resource.BoneIndices.data(), size);
        m_BoneIndices.Unmap();
    }

    if (resource.BoneWeights.size() > 0)
    {
        auto stride = uint32_t(sizeof(resource.BoneWeights[0])) * resource.BoneWeightStride;
        auto size   = uint64_t(resource.BoneWeights.size()) * sizeof(resource.BoneWeights[0]);

        if (!m_BoneWeights.Init(size, stride))
        {
            ELOG("Error : VertexBuffer::Init() Failed.");
            return false;
        }
    }

    {
        auto stride     = sizeof(resource.Indices[0]);
        auto size       = resource.Indices.size() * stride;
        auto isShort    = (stride == 16);

        if (!m_Indices.Init(size, isShort))
        {
            ELOG("Error : IndexBuffer::Init() Failed.");
            return false;
        }

        auto ptr = m_Indices.Map();
        memcpy(ptr, resource.Indices.data(), size);
        m_Indices.Unmap();
    }


    m_Box.Mini = m_Box.Maxi = resource.Positions[0];
    for(auto i=1; i<resource.Positions.size(); ++i)
    {
        m_Box.Mini = asdx::Vector3::Min(m_Box.Mini, resource.Positions[i]);
        m_Box.Maxi = asdx::Vector3::Max(m_Box.Maxi, resource.Positions[i]);
    }

    m_MaterialId        = resource.MaterialId;
    m_BoneWeightStride  = resource.BoneWeightStride;

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Mesh::Term()
{
    m_Positions     .Term();
    m_Normals       .Term();
    m_Tangents      .Term();
    m_Colors        .Term();
    m_BoneIndices   .Term();
    m_BoneWeights   .Term();
    m_Indices       .Term();

    for(auto i=0; i<4; ++i)
    { m_TexCoords[i].Term(); }

    m_MaterialId   = 0;

    m_Box.Mini = asdx::Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
    m_Box.Maxi = asdx::Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
}

//-----------------------------------------------------------------------------
//      頂点データを取得します.
//-----------------------------------------------------------------------------
const VertexBuffer& Mesh::GetPositions() const
{ return m_Positions; }

//-----------------------------------------------------------------------------
//      接線空間を取得します.
//-----------------------------------------------------------------------------
const VertexBuffer& Mesh::GetNormals() const
{ return m_Normals; }

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
const VertexBuffer& Mesh::GetTexCoords(uint8_t index) const
{
    assert(index < 4);
    return m_TexCoords[index];
}

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
//      インデックスデータを取得します.
//-----------------------------------------------------------------------------
const IndexBuffer& Mesh::GetIndices() const
{ return m_Indices; }

//-----------------------------------------------------------------------------
//      メッシュハッシュを取得します.
//-----------------------------------------------------------------------------
uint32_t Mesh::GetMeshHash() const
{ return m_MeshHash; }

//-----------------------------------------------------------------------------
//      マテリアルハッシュを取得します.
//-----------------------------------------------------------------------------
uint32_t Mesh::GetMaterialId() const
{ return m_MaterialId; }

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
//      接線ベクトルを持つかどうか?
//-----------------------------------------------------------------------------
bool Mesh::HasTangent() const
{ return m_Tangents.GetResource() != nullptr; }

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
: m_ModelHash(0)
, m_Box({asdx::Vector3(FLT_MAX, FLT_MAX, FLT_MAX), asdx::Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX)})
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
    m_ModelHash = asdx::CalcHash(model.Name.c_str());

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
            m_Box.Mini = asdx::Vector3::Min(m_Box.Mini, box.Mini);
            m_Box.Maxi = asdx::Vector3::Max(m_Box.Maxi, box.Maxi);
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
//      モデルハッシュを取得します.
//-----------------------------------------------------------------------------
uint32_t Model::GetModelHash() const
{ return m_ModelHash; }

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
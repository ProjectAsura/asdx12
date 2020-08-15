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
bool Mesh::Init(GraphicsDevice& device, const ResMesh& resource, ResourceUploader& uploader)
{
    {
        asdx::IUploadResource* pUpload = nullptr;
        if (!m_Positions.Init(
            device,
            uint64_t(resource.Positions.size()),
            uint32_t(sizeof(resource.Positions[0])),
            resource.Positions.data(),
            &pUpload))
        {
            ELOG("Error : StructuredBuffer::Init() Failed.");
            return false;
        }
        uploader.Push(pUpload);
    }

    if (resource.TangentSpaces.size() > 0)
    {
        asdx::IUploadResource* pUpload = nullptr;
        if (!m_TangentSpaces.Init(
            device,
            uint64_t(resource.TangentSpaces.size()),
            uint32_t(sizeof(resource.TangentSpaces[0])),
            resource.TangentSpaces.data(),
            &pUpload))
        {
            ELOG("Error : StructuredBuffer::Init() Failed.");
            return false;
        }
        uploader.Push(pUpload);
    }

    if (resource.Colors.size() > 0)
    {
        asdx::IUploadResource* pUpload = nullptr;
        if (!m_Colors.Init(
            device,
            uint64_t(resource.Colors.size()),
            uint32_t(sizeof(resource.Colors[0])),
            resource.Colors.data(),
            &pUpload))
        {
            ELOG("Error : StructuredBuffer::Init() Failed.");
            return false;
        }
        uploader.Push(pUpload);
    }

    for(auto i=0; i<4; ++i)
    {
        if (resource.TexCoords[i].size() > 0)
        {
            asdx::IUploadResource* pUpload = nullptr;
            if (!m_TexCoord[i].Init(
                device,
                uint64_t(resource.TexCoords[i].size()),
                uint32_t(resource.TexCoords[i][0]),
                resource.TexCoords[i].data(),
                &pUpload))
            {
                ELOG("Error : StructuredBuffer:Init() Failed.");
                return false;
            }
            uploader.Push(pUpload);
        }
    }

    if (resource.BoneIndices.size() > 0)
    {
        asdx::IUploadResource* pUpload = nullptr;
        if (!m_BoneIndices.Init(
            device,
            uint64_t(resource.BoneIndices.size()),
            uint32_t(sizeof(resource.BoneIndices[0])),
            resource.BoneIndices.data(),
            &pUpload))
        {
            ELOG("Error : StructuredBuffer::Init() Failed.");
            return false;
        }
        uploader.Push(pUpload);
    }

    if (resource.BoneWeights.size() > 0)
    {
        asdx::IUploadResource* pUpload = nullptr;
        if (!m_BoneWeights.Init(
            device,
            uint64_t(resource.BoneWeights.size()),
            uint32_t(sizeof(resource.BoneWeights[0])),
            resource.BoneWeights.data(),
            &pUpload))
        {
            ELOG("Error : StructuredBuffer::Init() Failed.");
            return false;
        }
        uploader.Push(pUpload);
    }

    {
        asdx::IUploadResource* pUpload = nullptr;
        if (!m_Indices.Init(
            device,
            uint64_t(resource.Indices.size()),
            uint32_t(sizeof(resource.Indices[0])),
            resource.Indices.data(),
            &pUpload))
        {
            ELOG("Error : StructuredBuffer::Init() Failed.");
            return false;
        }
        uploader.Push(pUpload);
    }

    {
        asdx::IUploadResource* pUpload = nullptr;
        if (!m_Primitives.Init(device,
            uint64_t(resource.Primitives.size()),
            uint32_t(sizeof(resource.Primitives[0])),
            resource.Primitives.data(),
            &pUpload))
        {
            ELOG("Error : StructuredBuffer::Init() Failed.");
            return false;
        }
        uploader.Push(pUpload);
    }

    {
        asdx::IUploadResource* pUpload = nullptr;
        if (!m_Meshlets.Init(
            device,
            uint64_t(resource.Meshlets.size()),
            uint32_t(sizeof(resource.Meshlets[0])),
            resource.Meshlets.data(),
            &pUpload))
        {
            ELOG("Error : StructuredBuffer::Init() Failed");
            return false;
        }
        uploader.Push(pUpload);
    }

    {
        asdx::IUploadResource* pUpload = nullptr;
        if (!m_CullingInfos.Init(
            device,
            uint64_t(resource.CullingInfos.size()),
            uint32_t(sizeof(resource.CullingInfos[0])),
            resource.CullingInfos.data(),
            &pUpload))
        {
            ELOG("Error : StructuredBuffer::Init() Failed.");
            return false;
        }
        uploader.Push(pUpload);
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
    { m_TexCoord[i].Term(); }

    m_MeshletCount = 0;
    m_MaterialHash = 0;

    m_Box.mini = asdx::Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
    m_Box.maxi = asdx::Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
}

//-----------------------------------------------------------------------------
//      頂点データのGPU仮想アドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS Mesh::GetPositions() const
{
    if (m_Positions.GetResource() != nullptr)
    { return m_Positions.GetResource()->GetGPUVirtualAddress(); }

    return D3D12_GPU_VIRTUAL_ADDRESS();
}

//-----------------------------------------------------------------------------
//      接線空間のGPU仮想アドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS Mesh::GetTangentSpaces() const
{
    if (m_TangentSpaces.GetResource() != nullptr)
    { return m_TangentSpaces.GetResource()->GetGPUVirtualAddress(); }

    return D3D12_GPU_VIRTUAL_ADDRESS();
}

//-----------------------------------------------------------------------------
//      頂点カラーのGPU仮想アドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS Mesh::GetColors() const
{
    if (m_Colors.GetResource() != nullptr)
    { return m_Colors.GetResource()->GetGPUVirtualAddress(); }

    return D3D12_GPU_VIRTUAL_ADDRESS();
}

//-----------------------------------------------------------------------------
//      テクスチャ座標のGPU仮想アドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS Mesh::GetTexCoords(uint8_t index) const
{
    assert(index < 4);
    if (m_TexCoord[index].GetResource() != nullptr)
    { return m_TexCoord[index].GetResource()->GetGPUVirtualAddress(); }

    return D3D12_GPU_VIRTUAL_ADDRESS();
}

//-----------------------------------------------------------------------------
//      ボーン番号のGPU仮想アドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS Mesh::GetBoneIndices() const
{
    if (m_BoneIndices.GetResource() != nullptr)
    { return m_BoneIndices.GetResource()->GetGPUVirtualAddress(); }

    return D3D12_GPU_VIRTUAL_ADDRESS();
}

//-----------------------------------------------------------------------------
//      ボーンの重みのGPU仮想アドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS Mesh::GetBoneWeights() const
{
    if (m_BoneWeights.GetResource() != nullptr)
    { return m_BoneWeights.GetResource()->GetGPUVirtualAddress(); }

    return D3D12_GPU_VIRTUAL_ADDRESS();
}

//-----------------------------------------------------------------------------
//      インデックスデータのGPU仮想アドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS Mesh::GetInindices() const
{
    if (m_Indices.GetResource() != nullptr)
    { return m_Indices.GetResource()->GetGPUVirtualAddress(); }

    return D3D12_GPU_VIRTUAL_ADDRESS();
}

//-----------------------------------------------------------------------------
//      プリミティブデータのGPU仮想アドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS Mesh::GetPrimitives() const
{
    if (m_Primitives.GetResource() != nullptr)
    { return m_Primitives.GetResource()->GetGPUVirtualAddress(); }

    return D3D12_GPU_VIRTUAL_ADDRESS();
}

//-----------------------------------------------------------------------------
//      メッシュレットデータのGPU仮想アドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS Mesh::GetMeshlets() const
{
    if (m_Meshlets.GetResource() != nullptr)
    { return m_Meshlets.GetResource()->GetGPUVirtualAddress(); }

    return D3D12_GPU_VIRTUAL_ADDRESS();
}

//-----------------------------------------------------------------------------
//      カリング情報のGPU仮想アドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS Mesh::GetCullingInfos() const
{
    if (m_CullingInfos.GetResource() != nullptr)
    { return m_CullingInfos.GetResource()->GetGPUVirtualAddress(); }

    return D3D12_GPU_VIRTUAL_ADDRESS();
}

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
    return m_TexCoord[index].GetResource() != nullptr;
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
bool Model::Init(GraphicsDevice& device, const ResModel& model, ResourceUploader& uploader)
{
    auto index = 0;
    m_Meshes.resize(model.Meshes.size());

    for(size_t i=0; i<model.Meshes.size(); ++i)
    {
        if (!m_Meshes[index].Init(device, model.Meshes[i], uploader))
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
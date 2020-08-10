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
bool Mesh::Init(GraphicsDevice& device, const ResStaticMesh& resource, ResourceUploader& uploader)
{
    {
        asdx::IUploadResource* pUpload = nullptr;
        if (!m_Vertices.Init(
            device,
            uint64_t(resource.Vertices.size()),
            uint32_t(sizeof(resource.Vertices[0])),
            resource.Vertices.data(),
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

    m_Box.mini = m_Box.maxi = resource.Vertices[0].Position;
    for(auto i=1; i<resource.Vertices.size(); ++i)
    {
        m_Box.mini = asdx::Vector3::Min(m_Box.mini, resource.Vertices[i].Position);
        m_Box.maxi = asdx::Vector3::Max(m_Box.maxi, resource.Vertices[i].Position);
    }

    m_MaterialHash = resource.MatrerialHash;
    m_MeshletCount = uint32_t(resource.Meshlets.size());
    m_HasBone      = false;

    return true;
}

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Mesh::Init(GraphicsDevice& device, const ResSkinningMesh& resource, ResourceUploader& uploader)
{
    {
        asdx::IUploadResource* pUpload = nullptr;
        if (!m_Vertices.Init(
            device,
            uint64_t(resource.Vertices.size()),
            uint32_t(sizeof(resource.Vertices[0])),
            resource.Vertices.data(),
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

    m_Box.mini = m_Box.maxi = resource.Vertices[0].Position;
    for(auto i=1; i<resource.Vertices.size(); ++i)
    {
        m_Box.mini = asdx::Vector3::Min(m_Box.mini, resource.Vertices[i].Position);
        m_Box.maxi = asdx::Vector3::Max(m_Box.maxi, resource.Vertices[i].Position);
    }

    m_MaterialHash = resource.MatrerialHash;
    m_MeshletCount = uint32_t(resource.Meshlets.size());
    m_HasBone      = true;

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Mesh::Term()
{
    m_Vertices      .Term();
    m_Indices       .Term();
    m_Primitives    .Term();
    m_Meshlets      .Term();
    m_CullingInfos  .Term();

    m_MeshletCount = 0;
    m_MaterialHash = 0;

    m_Box.mini = asdx::Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
    m_Box.maxi = asdx::Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
}

//-----------------------------------------------------------------------------
//      頂点データのGPU仮想アドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS Mesh::GetVertices() const
{
    if (m_Vertices.GetResource() != nullptr)
    { return m_Vertices.GetResource()->GetGPUVirtualAddress(); }

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
{ return m_HasBone; }


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
bool Model::Init(GraphicsDevice& device, const ResModel& model, ResourceUploader& uploader)
{
    auto count = uint32_t(model.StaticMeshes.size() + model.SkinningMeshes.size());

    auto index = 0;
    m_Meshes.resize(count);

    for(size_t i=0; i<model.StaticMeshes.size(); ++i)
    {
        if (!m_Meshes[index].Init(device, model.StaticMeshes[i], uploader))
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

    for(size_t i=0; i<model.SkinningMeshes.size(); ++i)
    {
        if (!m_Meshes[index].Init(device, model.SkinningMeshes[i], uploader))
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

    m_ModelHash = model.ModelHash;

    return true;
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
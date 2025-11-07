//-----------------------------------------------------------------------------
// File : asdxModelInstanceImpl.cpp
// Desc : Model Instance Implementation.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include "asdxModelInstanceImpl.h"
#include "asdxModelManagerImpl.h"


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ModelInstance class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ModelInstance::ModelInstance()
    : m_RefCount(1)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ModelInstance::~ModelInstance()
{ Term(); }

//-----------------------------------------------------------------------------
//      参照カウンタを増やします.
//-----------------------------------------------------------------------------
void ModelInstance::AddRef()
{ m_RefCount++; }

//-----------------------------------------------------------------------------
//      解放処理を行います.
//-----------------------------------------------------------------------------
void ModelInstance::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    {
        // 管理対象から外す.
        ModelManager::Instance().RemoveModelInstance(this);

        // 破棄処理.
        delete this;
    }
}

//-----------------------------------------------------------------------------
//      参照カウンタを取得します.
//-----------------------------------------------------------------------------
uint32_t ModelInstance::GetRefCount() const
{ return m_RefCount; }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool ModelInstance::Init(Model* pMasterModel, const OffsetHandle& handle)
{
    if (pMasterModel == nullptr)
        return false;

    // マスターモデルの依存を張る.
    m_pMasterModel = pMasterModel;
    m_pMasterModel->AddRef();

    // オフセットハンドルを設定.
    m_OffsetHandle = handle;

    // 可視フラグを立てておく.
    m_Visible = true;

    // バウンディングスフィア
    m_BoundingSphere = m_pMasterModel->GetBoundingSphere();

    // ワールド行列を初期化.
    m_World = Matrix::CreateIdentity();

    // スキニング行列のメモリを確保.
    m_SkinningMatrices.resize(m_pMasterModel->GetBoneCount());
    for(size_t i=0; i<m_SkinningMatrices.size(); ++i)
    {
        m_SkinningMatrices[i] = Matrix::CreateIdentity();
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void ModelInstance::Term()
{
    if (m_pMasterModel)
    {
        m_pMasterModel->Release();
        m_pMasterModel = nullptr;
    }

    for(auto& itr : m_Materials)
    {
        auto material = itr.second;
        itr.second = nullptr;
        //if (material)
        //{
        //    material->Release();
        //    material = nullptr;
        //}
    }
    m_Materials.clear();

    m_MeshVisibilities.clear();
    m_DirtyWorld = false;
    m_DirtyVisibility = false;

    m_SkinningMatrices.clear();
}

//-----------------------------------------------------------------------------
//      マテリアル数を取得します.
//-----------------------------------------------------------------------------
size_t ModelInstance::GetMaterialCount() const
{
    assert(m_pMasterModel != nullptr);
    return m_pMasterModel->GetMaterialCount();
}

//-----------------------------------------------------------------------------
//      マテリアル名を取得します.
//-----------------------------------------------------------------------------
const std::string& ModelInstance::GetMaterialName(size_t index) const
{
    assert(m_pMasterModel != nullptr);
    assert(index < m_pMasterModel->GetMaterialCount());
    return m_pMasterModel->GetMaterialName(index);
}

//-----------------------------------------------------------------------------
//      マテリアルを取得します.
//-----------------------------------------------------------------------------
IMaterial* ModelInstance::GetMaterial(size_t index) const
{
    auto itr = m_Materials.find(index);
    if (itr != m_Materials.end())
    { return itr->second; }

    assert(m_pMasterModel != nullptr);
    return m_pMasterModel->GetMaterial(index);
}

//-----------------------------------------------------------------------------
//      マテリアルを設定します.
//-----------------------------------------------------------------------------
void ModelInstance::SetMaterial(size_t index, IMaterial* pMaterial)
{
    assert(index < GetMaterialCount());
    m_Materials[index] = pMaterial;
}

//-----------------------------------------------------------------------------
//      ボーン数を取得します.
//-----------------------------------------------------------------------------
size_t ModelInstance::GetBoneCount() const
{
    assert(m_pMasterModel != nullptr);
    return m_pMasterModel->GetBoneCount();
}

//-----------------------------------------------------------------------------
//      ボーンを持つかどうかチェックします.
//-----------------------------------------------------------------------------
bool ModelInstance::HasBone() const
{
    assert(m_pMasterModel != nullptr);
    return m_pMasterModel->HasBone();
}

//-----------------------------------------------------------------------------
//      ボーン名を取得します.
//-----------------------------------------------------------------------------
const IBone* ModelInstance::GetBone(size_t index) const
{
    assert(m_pMasterModel != nullptr);
    return m_pMasterModel->GetBone(index);
}

//-----------------------------------------------------------------------------
//      メッシュ数を取得します.
//-----------------------------------------------------------------------------
size_t ModelInstance::GetMeshCount() const
{
    assert(m_pMasterModel != nullptr);
    return m_pMasterModel->GetMeshCount();
}

//-----------------------------------------------------------------------------
//      メッシュを取得します.
//-----------------------------------------------------------------------------
const IMesh* ModelInstance::GetMesh(size_t index) const
{
    assert(m_pMasterModel != nullptr);
    return m_pMasterModel->GetMesh(index);
}

//-----------------------------------------------------------------------------
//      バウンディングスフィアを取得します.
//-----------------------------------------------------------------------------
const BoundingSphere3 & ModelInstance::GetBoundingSphere() const
{ return m_BoundingSphere; }

//-----------------------------------------------------------------------------
//      インスタンスIDを取得します.
//-----------------------------------------------------------------------------
uint32_t ModelInstance::GetInstanceId() const
{ return m_OffsetHandle.GetOffset(); }

//-----------------------------------------------------------------------------
//      ワールド行列を設定します.
//-----------------------------------------------------------------------------
void ModelInstance::SetWorldMatrix(const Matrix& value)
{
    if (m_World == value)
        return;

    m_World = value;
    m_BoundingSphere = BoundingSphere3::Transform(m_pMasterModel->GetBoundingSphere(), value);
}

//-----------------------------------------------------------------------------
//      ワールド行列を取得します.
//-----------------------------------------------------------------------------
const Matrix& ModelInstance::GetWorldMatrix() const
{ return m_World; }

//-----------------------------------------------------------------------------
//      スキニング行列を設定します.
//-----------------------------------------------------------------------------
void ModelInstance::SetSkinningMatrix(size_t index, const Matrix& value)
{
    assert(index < GetBoneCount());
    m_SkinningMatrices[index] = value;
}

//-----------------------------------------------------------------------------
//      スキニング行列を取得します.
//-----------------------------------------------------------------------------
const Matrix& ModelInstance::GetSkinningMatrix(size_t index) const
{
    assert(index < GetBoneCount());
    return m_SkinningMatrices[index];
}

//-----------------------------------------------------------------------------
//      可視フラグを取得します.
//-----------------------------------------------------------------------------
bool ModelInstance::IsVisible() const
{ return m_Visible; }

//-----------------------------------------------------------------------------
//      可視フラグを設定します.
//-----------------------------------------------------------------------------
void ModelInstance::SetVisibility(bool value)
{ m_Visible = value; }

//-----------------------------------------------------------------------------
//      メッシュの可視フラグを取得します.
//-----------------------------------------------------------------------------
bool ModelInstance::IsVisibleMesh(size_t index) const
{
    auto itr = m_MeshVisibilities.find(index);
    if (itr != m_MeshVisibilities.end())
        return itr->second;

    return m_pMasterModel->IsVisibleMesh(index);
}

//-----------------------------------------------------------------------------
//      メッシュの可視フラグを設定します.
//-----------------------------------------------------------------------------
void ModelInstance::SetMeshVisibility(size_t index, bool value)
{ m_MeshVisibilities[index] = value; }

//-----------------------------------------------------------------------------
//      ワールド行列のダーティフラグを取得します.
//-----------------------------------------------------------------------------
bool ModelInstance::IsDirtyWorld() const
{ return m_DirtyWorld; }

//-----------------------------------------------------------------------------
//      可視フラグのダーティフラグを取得します.
//-----------------------------------------------------------------------------
bool ModelInstance::IsDirtyVisibility() const
{ return m_DirtyVisibility; }

//-----------------------------------------------------------------------------
//      ダーティフラグをクリアします.
//-----------------------------------------------------------------------------
void ModelInstance::ClearDirtyFlags()
{
    m_DirtyWorld      = false;
    m_DirtyVisibility = false;
}

//-----------------------------------------------------------------------------
//      オフセットハンドルを取得します.
//-----------------------------------------------------------------------------
OffsetHandle& ModelInstance::GetOffsetHandle()
{ return m_OffsetHandle; }

//-----------------------------------------------------------------------------
//      生成処理を行います.
//-----------------------------------------------------------------------------
bool ModelInstance::Create(Model* pMasterModel, const OffsetHandle& handle, ModelInstance** ppModelInstance)
{
    auto instance = new(std::nothrow) ModelInstance();
    if (instance == nullptr)
    {
        ELOG("Error : Out of Memory.");
        return false;
    }

    if (!instance->Init(pMasterModel, handle))
    {
        ELOG("Error : ModelInstance::Init() Failed.");
        instance->Release();
        return false;
    }

    (*ppModelInstance) = instance;
    return true;
}

} // namespace asdx

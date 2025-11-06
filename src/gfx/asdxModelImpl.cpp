//-----------------------------------------------------------------------------
// File : asdxModelImpl.cpp
// Desc : Model Implementation.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <res/asdxResModel.h>
#include <fnd/asdxLogger.h>
#include "asdxModelImpl.h"
#include "asdxModelManagerImpl.h"


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Model class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Model::Model()
: m_RefCount        (1)
, m_Visibility      (true)
, m_DirtyVisibility (false)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Model::~Model()
{ Term(); }

//-----------------------------------------------------------------------------
//      参照カウンタを増やします.
//-----------------------------------------------------------------------------
void Model::AddRef()
{ m_RefCount++; }

//-----------------------------------------------------------------------------
//      解放処理を行います.
//-----------------------------------------------------------------------------
void Model::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    {
        // 管理対象から外す.
        ModelManager::Instance().RemoveModel(this);

        // 破棄.
        delete this;
    }
}

//-----------------------------------------------------------------------------
//      参照カウンタを取得します.
//-----------------------------------------------------------------------------
uint32_t Model::GetRefCount() const
{ return m_RefCount; }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Model::Init(const ModelBinary& binary)
{
    m_MaterialSlots.resize(binary.GetMaterialCount());
    for(auto i=0u; i<binary.GetMaterialCount(); ++i)
    {
        m_MaterialSlots[i].Name      = binary.GetMaterial(i);
        m_MaterialSlots[i].pMaterial = nullptr;
    }


    m_Meshes.resize(binary.GetMeshCount());
    for(auto i=0u; i<binary.GetMeshCount(); ++i)
    {
        auto res = binary.GetMesh(i);
        if (!m_Meshes[i].Init(res))
        {
            ELOG("Error : Mesh Initialize Failed. index = %zu", i);
            return false;
        }
    }

    m_BoneNames.resize(binary.GetBoneCount());
    m_BoneOffsetMatrices.resize(binary.GetBoneCount());
    for(auto i=0u; i<binary.GetBoneCount(); ++i)
    {
        auto res = binary.GetBone(i);
        m_BoneNames[i]          = res.Name.c_str();
        m_BoneOffsetMatrices[i] = res.OffsetMatrix;
    }

    m_BoundingSphere = binary.GetBoundingSphere();

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Model::Term()
{
    for(size_t i=0; i<m_Meshes.size(); ++i)
    {
        m_Meshes[i].Term();
    }

    for(size_t i=0; i<m_MaterialSlots.size(); ++i)
    {
        auto material = m_MaterialSlots[i].pMaterial;
        m_MaterialSlots[i].pMaterial = nullptr;
        m_MaterialSlots[i].Name.clear();

        //if (material)
        //{
        //    material->Release();
        //    material = nullptr;
        //}
    }

    m_Meshes            .clear();
    m_BoneNames         .clear();
    m_BoneOffsetMatrices.clear();
    m_MaterialSlots     .clear();
    m_MeshVisibilities  .clear();

    m_BoundingSphere    = BoundingSphere3();
    m_Visibility        = false;
    m_DirtyVisibility   = false;
}

//-----------------------------------------------------------------------------
//      マテリアル数を取得します.
//-----------------------------------------------------------------------------
size_t Model::GetMaterialCount() const
{ return m_MaterialSlots.size(); }

//-----------------------------------------------------------------------------
//      マテリアル名を取得します.
//-----------------------------------------------------------------------------
const std::string& Model::GetMaterialName(size_t index) const
{
    assert(index < m_MaterialSlots.size());
    return m_MaterialSlots[index].Name;
}

//-----------------------------------------------------------------------------
//      マテリアルを取得します.
//-----------------------------------------------------------------------------
IMaterial* Model::GetMaterial(size_t index) const
{
    assert(index < m_MaterialSlots.size());
    return m_MaterialSlots[index].pMaterial;
}

//-----------------------------------------------------------------------------
//      マテリアルを設定します.
//-----------------------------------------------------------------------------
void Model::SetMaterial(size_t index, IMaterial* pMaterial)
{
    assert(index < m_MaterialSlots.size());
    m_MaterialSlots[index].pMaterial = pMaterial;
}

//-----------------------------------------------------------------------------
//      ボーンを持つかどうかチェックします.
//-----------------------------------------------------------------------------
bool Model::HasBone() const
{ return !m_BoneNames.empty(); }

//-----------------------------------------------------------------------------
//      ボーン数を取得します.
//-----------------------------------------------------------------------------
size_t Model::GetBoneCount() const
{ return m_BoneNames.size(); }

//-----------------------------------------------------------------------------
//      ボーン名を取得します.
//-----------------------------------------------------------------------------
const std::string& Model::GetBoneName(size_t i) const
{ return m_BoneNames[i]; }

//-----------------------------------------------------------------------------
//      ボーンオフセット行列を取得します.
//-----------------------------------------------------------------------------
const Matrix& Model::GetBoneOffsetMatrix(size_t index) const
{
    assert(index < m_BoneOffsetMatrices.size());
    return m_BoneOffsetMatrices[index];
}

//-----------------------------------------------------------------------------
//      メッシュ数を取得します.
//-----------------------------------------------------------------------------
size_t Model::GetMeshCount() const
{ return m_Meshes.size(); }

//-----------------------------------------------------------------------------
//      メッシュを取得します.
//-----------------------------------------------------------------------------
const IMesh* Model::GetMesh(size_t index) const
{
    assert(index < m_Meshes.size());
    return &m_Meshes[index];
}

//-----------------------------------------------------------------------------
//      可視フラグを取得します.
//-----------------------------------------------------------------------------
bool Model::IsVisible() const
{ return m_Visibility; }

//-----------------------------------------------------------------------------
//      可視フラグを設定します.
//-----------------------------------------------------------------------------
void Model::SetVisibility(bool value)
{ m_Visibility = value; }

//-----------------------------------------------------------------------------
//      メッシュの可視フラグを取得します.
//-----------------------------------------------------------------------------
bool Model::IsVisibleMesh(size_t index) const
{
    assert(index < m_MeshVisibilities.size());
    return m_MeshVisibilities[index];
}

//-----------------------------------------------------------------------------
//      メッシュの可視フラグを設定します.
//-----------------------------------------------------------------------------
void Model::SetMeshVisibility(size_t index, bool value)
{
    assert(index < m_MeshVisibilities.size());
    m_MeshVisibilities[index] = value;
}

//-----------------------------------------------------------------------------
//      バウンディングスフィアを取得します.
//------------------------------------------------------------------------
const BoundingSphere3& Model::GetBoundingSphere() const
{ return m_BoundingSphere; }

//-----------------------------------------------------------------------------
//      可視フラグのダーティフラグを取得します.
//-----------------------------------------------------------------------------
bool Model::IsDirtyVisibility() const
{ return m_DirtyVisibility; }

//-----------------------------------------------------------------------------
//      ダーティフラグをクリアします.
//-----------------------------------------------------------------------------
void Model::ClearDirtyFlags()
{ m_DirtyVisibility = false; }

//-----------------------------------------------------------------------------
//      モデルを生成します.
//-----------------------------------------------------------------------------
bool Model::Create(const ModelBinary& binary, Model** ppModel)
{
    auto instance = new(std::nothrow) Model();
    if (instance == nullptr)
    {
        ELOG("Error : Out of Memory.");
        return false;
    }

    if (!instance->Init(binary))
    {
        ELOG("Error : Model::Init() Failed.");
        instance->Release();
        return false;
    }

    (*ppModel) = instance;
    return true;
}

} // namespace asdx

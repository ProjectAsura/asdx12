//-----------------------------------------------------------------------------
// File : asdxModelManager.cpp
// Desc : Model Manager.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxHash.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxFileIO.h>
#include <fnd/asdxPath.h>
#include <gfx/asdxModelManager.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ModelHolder class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
ModelHolder::ModelHolder(Model* pModel, uint64_t hash)
: m_pModel  (pModel)
, m_Hash    (hash)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      解放処理を行います.
//-----------------------------------------------------------------------------
void ModelHolder::Reset()
{ ModelManager::Instance().Remove(*this); }

//-----------------------------------------------------------------------------
//      有効かどうかチェックします.
//-----------------------------------------------------------------------------
bool ModelHolder::IsValid() const
{ return (m_pModel != nullptr) && (m_Hash != 0); }

//-----------------------------------------------------------------------------
//      マテリアル数を取得します.
//-----------------------------------------------------------------------------
uint32_t ModelHolder::GetMaterialCount() const
{
    assert(m_pModel != nullptr);
    return m_pModel->GetMaterialCount();
}

//-----------------------------------------------------------------------------
//      マテリアルを取得します.
//-----------------------------------------------------------------------------
const Material* ModelHolder::GetMaterial(uint32_t index) const
{
    assert(m_pModel != nullptr);
    return m_pModel->GetMaterial(index);
}

//-----------------------------------------------------------------------------
//      リソースマテリアルを取得します.
//-----------------------------------------------------------------------------
const res::Material& ModelHolder::GetResMaterial(uint32_t index) const
{
    assert(m_pModel != nullptr);
    return m_pModel->GetResMaterial(index);
}

//-----------------------------------------------------------------------------
//      ボーンを持つかどうかチェックします.
//-----------------------------------------------------------------------------
bool ModelHolder::HasBone() const
{
    assert(m_pModel != nullptr);
    return m_pModel->HasBone();
}

//-----------------------------------------------------------------------------
//      ボーン数を取得します.
//-----------------------------------------------------------------------------
uint32_t ModelHolder::GetBoneCount() const
{
    assert(m_pModel != nullptr);
    return m_pModel->GetBoneCount();
}

//-----------------------------------------------------------------------------
//      ボーンを取得します.
//-----------------------------------------------------------------------------
const res::Bone& ModelHolder::GetBone(uint32_t index) const
{
    assert(m_pModel != nullptr);
    return m_pModel->GetBone(index);
}

//-----------------------------------------------------------------------------
//      メッシュ数を取得します.
//-----------------------------------------------------------------------------
uint32_t ModelHolder::GetMeshCount() const
{
    assert(m_pModel != nullptr);
    return m_pModel->GetMeshCount();
}

//-----------------------------------------------------------------------------
//      メッシュを取得します(const版).
//-----------------------------------------------------------------------------
const Mesh* ModelHolder::GetMesh(uint32_t index) const
{
    assert(m_pModel != nullptr);
    return m_pModel->GetMesh(index);
}

//-----------------------------------------------------------------------------
//      メッシュを取得します.
//-----------------------------------------------------------------------------
Mesh* ModelHolder::GetMesh(uint32_t index)
{
    assert(m_pModel != nullptr);
    return m_pModel->GetMesh(index);
}

//-----------------------------------------------------------------------------
//      リソースメッシュを取得します.
//-----------------------------------------------------------------------------
const res::Mesh& ModelHolder::GetResMesh(uint32_t index) const
{
    assert(m_pModel != nullptr);
    return m_pModel->GetResMesh(index);
}

//-----------------------------------------------------------------------------
//      バッチ数を取得します.
//-----------------------------------------------------------------------------
uint32_t ModelHolder::GetBatchCount() const
{
    assert(m_pModel != nullptr);
    return m_pModel->GetBatchCount();
}

//-----------------------------------------------------------------------------
//      バッチを取得します.
//-----------------------------------------------------------------------------
const res::ModelBatch& ModelHolder::GetBatch(uint32_t index) const
{
    assert(m_pModel != nullptr);
    return m_pModel->GetBatch(index);
}

//-----------------------------------------------------------------------------
//      総インスタンス数を取得します.
//-----------------------------------------------------------------------------
uint64_t ModelHolder::GetTotalInstanceCount() const
{
    assert(m_pModel != nullptr);
    return m_pModel->GetTotalInstanceCount();
}

//-----------------------------------------------------------------------------
//      ローカル座標系のバウンディングスフィアを取得します.
//-----------------------------------------------------------------------------
const BoundingSphere3& ModelHolder::GetLocalSphere() const
{
    assert(m_pModel != nullptr);
    return m_pModel->GetLocalSphere();
}

//-----------------------------------------------------------------------------
//      ローカル座標系のバウンディングボックスを取得します.
//-----------------------------------------------------------------------------
const BoundingBox3& ModelHolder::GetLocalBox() const
{
    assert(m_pModel != nullptr);
    return m_pModel->GetLocalBox();
}

//-----------------------------------------------------------------------------
//      可視フラグを設定します.
//-----------------------------------------------------------------------------
void ModelHolder::SetVisibility(bool value)
{
    assert(m_pModel != nullptr);
    return m_pModel->SetVisibility(value);
}

//-----------------------------------------------------------------------------
//      可視フラグを取得します.
//-----------------------------------------------------------------------------
bool ModelHolder::IsVisible() const
{
    assert(m_pModel != nullptr);
    return m_pModel->IsVisible();
}

//-----------------------------------------------------------------------------
//      ユーザーデータを設定します.
//-----------------------------------------------------------------------------
void ModelHolder::SetUserData(void* value)
{
    assert(m_pModel != nullptr);
    return m_pModel->SetUserData(value);
}

//-----------------------------------------------------------------------------
//      ユーザーデータを取得します.
//-----------------------------------------------------------------------------
void* ModelHolder::GetUserData() const
{
    assert(m_pModel != nullptr);
    return m_pModel->GetUserData();
}

//-----------------------------------------------------------------------------
//      ボーンを検索します.
//-----------------------------------------------------------------------------
bool ModelHolder::FindBone(const char* name, uint32_t& index) const
{
    if (m_pModel == nullptr)
    {
        index = UINT32_MAX;
        return false;
    }

    return m_pModel->FindBone(name, index);
}

//-----------------------------------------------------------------------------
//      マテリアルを検索します.
//-----------------------------------------------------------------------------
bool ModelHolder::FindMaterial(const char* name, uint32_t& index) const
{
    if (m_pModel == nullptr)
    {
        index = UINT32_MAX;
        return false;
    }

    return m_pModel->FindMaterial(name, index);
}

//-----------------------------------------------------------------------------
//      メッシュを検索します.
//-----------------------------------------------------------------------------
bool ModelHolder::FindMesh(const char* name, uint32_t& index) const
{
    if (m_pModel == nullptr)
    {
        index = UINT32_MAX;
        return false;
    }

    return m_pModel->FindMesh(name, index);
}

//-----------------------------------------------------------------------------
//      インスタンスを検索します.
//-----------------------------------------------------------------------------
bool ModelHolder::FindInstance(const char* name, uint32_t& batchIndex, uint32_t& instanceIndex) const
{
    if (m_pModel == nullptr)
    {
        batchIndex    = UINT32_MAX;
        instanceIndex = UINT32_MAX;
        return false;
    }

    return m_pModel->FindInstance(name, batchIndex, instanceIndex);
}

///////////////////////////////////////////////////////////////////////////////
// ModelManager class
///////////////////////////////////////////////////////////////////////////////
ModelManager ModelManager::s_Instance = {};

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ModelManager::ModelManager()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ModelManager::~ModelManager()
{ Term(); }

//-----------------------------------------------------------------------------
//      シングルトンインスタンスを取得します.
//-----------------------------------------------------------------------------
ModelManager& ModelManager::Instance()
{ return s_Instance; }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool ModelManager::Init()
{
    // 初期化済み.
    if (m_Initialized)
        return true;

    ScopedLock<SpinLock> locker(m_SpinLock);

    // 初期化済みフラグを立てる.
    m_Initialized = true;

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void ModelManager::Term()
{
    // 解放済み.
    if (!m_Initialized)
        return;

    ScopedLock<SpinLock> locker(m_SpinLock);

    // モデルを解放.
    for(auto& itr : m_Models)
    {
        auto item = itr.second;
        itr.second = nullptr;
        SafeRelease(item);
    }
    m_Models.clear();

    // 初期化フラグを下す.
    m_Initialized = false;
}

//-----------------------------------------------------------------------------
//      生成または取得処理を行います.
//-----------------------------------------------------------------------------
ModelHolder ModelManager::GetOrCreate(const char* fullPath)
{
    // 引数チェック.
    if (fullPath == nullptr)
    {
        ELOG("Error : Invalid Argument.");
        return ModelHolder();
    }

    // ファイルハッシュを求める.
    auto hash = CalcHash(fullPath);

    ScopedLock<SpinLock> locker(m_SpinLock);
    auto itr = m_Models.find(hash);
    if (itr != m_Models.end())
    {
        auto pModel = itr->second; // 見つかった場合はポインタを返却.
        pModel->AddRef();   // 参照カウントを上げる.
        return ModelHolder(pModel, hash);
    }

    // モデル生成.
    Model* pModel = nullptr;
    if (!Model::Create(fullPath, &pModel))
    {
        ELOG("Error : Model Create Failed. path = %s", fullPath);
        return ModelHolder();
    }

    // モデルを登録.
    m_Models[hash] = pModel;

    // モデルホルダーを返却.
    return ModelHolder(pModel, hash);
}

//-----------------------------------------------------------------------------
//      削除処理を行います.
//-----------------------------------------------------------------------------
void ModelManager::Remove(ModelHolder& holder)
{
    // 無効なら即終了.
    if (!holder.IsValid())
        return;

    if (holder.m_pModel->GetRefCount() > 1)
    {
        // 参照カウントを減らす.
        holder.m_pModel->Release();
    }
    else
    {
        ScopedLock<SpinLock> locker(m_SpinLock);

        // 削除処理.
        auto itr = m_Models.find(holder.m_Hash);
        if (itr != m_Models.end())
        {
            auto item = itr->second;
            itr->second = nullptr;
            m_Models.erase(holder.m_Hash);
            SafeRelease(item);
        }
    }

    // クリア処理.
    holder.m_pModel = nullptr;
    holder.m_Hash   = 0;
}

} // namespace asdx

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
//      モデルを取得します.
//-----------------------------------------------------------------------------
const Model* ModelHolder::GetModel() const
{ return m_pModel; }

const Model* ModelHolder::operator->() const
{ return m_pModel; }

//-----------------------------------------------------------------------------
//      入れ替えます.
//-----------------------------------------------------------------------------
void ModelHolder::Swap(ModelHolder& value)
{
    auto pModel = m_pModel;
    auto hash   = m_Hash;

    m_pModel = value.m_pModel;
    m_Hash   = value.m_Hash;

    value.m_pModel = pModel;
    value.m_Hash   = hash;
}

//-----------------------------------------------------------------------------
//      入れ替えます.
//-----------------------------------------------------------------------------
void ModelHolder::Swap(ModelHolder&& value)
{
    auto pModel = m_pModel;
    auto hash   = m_Hash;

    m_pModel = value.m_pModel;
    m_Hash   = value.m_Hash;

    value.m_pModel = pModel;
    value.m_Hash   = hash;
}

//-----------------------------------------------------------------------------
//      等価比較演算子です.
//-----------------------------------------------------------------------------
bool ModelHolder::operator == (const ModelHolder& value) const
{
    return (m_pModel == value.m_pModel)
        && (m_Hash   == value.m_Hash);
}

//-----------------------------------------------------------------------------
//      非等価比較演算子です.
//-----------------------------------------------------------------------------
bool ModelHolder::operator != (const ModelHolder& value) const
{
    return (m_pModel != value.m_pModel)
        || (m_Hash   != value.m_Hash);
}

//-----------------------------------------------------------------------------
//      代入演算子です.
//-----------------------------------------------------------------------------
ModelHolder& ModelHolder::operator = (const ModelHolder& value)
{
    ModelHolder(value.m_pModel, value.m_Hash).Swap(*this);
    return *this;
}

//-----------------------------------------------------------------------------
//      ムーブ代入演算子です.
//-----------------------------------------------------------------------------
ModelHolder& ModelHolder::operator = (ModelHolder&& value)
{
    ModelHolder(value.m_pModel, value.m_Hash).Swap(*this);
    return *this;
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

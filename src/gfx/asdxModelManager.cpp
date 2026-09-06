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

    if (holder.m_pRes->GetRefCount() > 1)
    {
        // 参照カウントを減らす.
        holder.m_pRes->Release();
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
    holder.m_pRes = nullptr;
    holder.m_Hash = 0;
}

} // namespace asdx

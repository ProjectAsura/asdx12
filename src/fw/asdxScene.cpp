//-----------------------------------------------------------------------------
// File : asdxScene.cpp
// Desc : Game Scene.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fw/asdxScene.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// SceneManager class
///////////////////////////////////////////////////////////////////////////////
SceneManager SceneManager::s_Instance = {};

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
SceneManager::SceneManager()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
SceneManager::~SceneManager()
{
    // 下記のアサートに引っかかる場合は、解放漏れがあります.
    assert(m_pActiveScene == nullptr);
    assert(m_SceneList.empty());
}

//-----------------------------------------------------------------------------
//      シングルトンインスタンスを取得します.
//-----------------------------------------------------------------------------
SceneManager& SceneManager::Instance()
{ return s_Instance; }

//-----------------------------------------------------------------------------
//      初期化処理です.
//-----------------------------------------------------------------------------
bool SceneManager::Init()
{ return true; }

//-----------------------------------------------------------------------------
//      終了処理です.
//-----------------------------------------------------------------------------
void SceneManager::Term()
{
    ScopedLock<SpinLock> locker(m_SpinLock);

    // アクティブを外しておく.
    if (m_pActiveScene != nullptr)
    { m_pActiveScene = nullptr; }

    auto itr = m_SceneList.begin();
    while(itr != m_SceneList.end())
    {
        auto scene = &(*itr);

        // シーン終了コールバック呼び出し.
        itr->OnExit();

        // リストから削除.
        itr = m_SceneList.erase(itr);

        // シーンを削除.
        if (scene)
        {
            delete scene;
            scene = nullptr;
        }
    }

    m_SceneList.clear();
}

//-----------------------------------------------------------------------------
//      シーンを切り替えます.
//-----------------------------------------------------------------------------
bool SceneManager::ChangeScene(uint32_t id)
{
    ScopedLock<SpinLock> locker(m_SpinLock);
    for(auto& itr : m_SceneList)
    {
        if (itr.GetId() == id)
        {
            // シーン終了コールバック呼び出し.
            if (m_pActiveScene != nullptr)
            { m_pActiveScene->OnExit(); }

            // シーン間でのデータをやり取りするために、ここでブロードキャスト.
            MessageManager::Instance().Broadcast();

            // アクティブシーン切り替え.
            m_pActiveScene = &(itr);

            // シーン開始コールバックを呼び出し.
            if (m_pActiveScene != nullptr)
            { m_pActiveScene->OnEnter(); }

            // 正常終了.
            return true;
        }
    }

    // 異常終了.
    return false;
}

//-----------------------------------------------------------------------------
//      更新処理を行います
//-----------------------------------------------------------------------------
void SceneManager::Update(float deltaSec)
{
    if (!m_pActiveScene)
        return;

    m_pActiveScene->OnUpdate(deltaSec);
}

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void SceneManager::Draw(void* pArgs)
{
    if (!m_pActiveScene)
        return;

    m_pActiveScene->OnDraw(pArgs);
}

//-----------------------------------------------------------------------------
//      アクティブなシーンIDを取得します.
//-----------------------------------------------------------------------------
uint32_t SceneManager::GetActiveId() const
{
    if (!m_pActiveScene)
        return UINT32_MAX;

    return m_pActiveScene->GetId();
}

//-----------------------------------------------------------------------------
//      シーンを削除します.
//-----------------------------------------------------------------------------
void SceneManager::RemoveScene(uint32_t id)
{
    ScopedLock<SpinLock> locker(m_SpinLock);
    auto itr = m_SceneList.begin();
    while(itr != m_SceneList.end())
    {
        if (itr->GetId() == id)
        {
            auto scene = &(*itr);
            if (m_pActiveScene == scene)
            {
                m_pActiveScene->OnExit();
                m_pActiveScene = nullptr;
            }

            m_SceneList.erase(itr);

            if (scene)
            {
                delete scene;
                scene = nullptr;
            }

            return;
        }
        itr++;
    }
}

} // namespace asdx

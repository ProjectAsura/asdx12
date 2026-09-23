//-----------------------------------------------------------------------------
// File : asdxPhase.cpp
// Desc : Phase System.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <fnd/asdxLogger.h>
#include <fw/asdxPhase.h>


namespace asdx { 

///////////////////////////////////////////////////////////////////////////////
// PhaseManager class
///////////////////////////////////////////////////////////////////////////////
PhaseManager PhaseManager::s_Instance = {};

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
PhaseManager::PhaseManager()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
PhaseManager::~PhaseManager()
{ Term(); }

//-----------------------------------------------------------------------------
//      シングルトンインスタンスを取得します.
//-----------------------------------------------------------------------------
PhaseManager& PhaseManager::Instance()
{ return s_Instance; }

//-----------------------------------------------------------------------------
//      初期化処理です.
//-----------------------------------------------------------------------------
bool PhaseManager::Init(IPhaseFactory* pFactory, uint64_t firstPhaseId)
{
    // 引数チェック.
    if (!pFactory || firstPhaseId == UINT64_MAX)
    {
        ELOGA("Error : Invalid Argument.");
        return false;
    }

    // ファクトリーが設定されていれば初期化済みとみなす.
    if (m_pFactory)
    {
        ILOGA("Info : PhaseManager is already initialized.");
        return true;
    }

    // ファクトリー設定.
    m_pFactory = pFactory;

    // フェーズ生成.
    m_pPhase = m_pFactory->Create(firstPhaseId);
    if (!m_pPhase)
    {
        ELOGA("Error : Phase create failed. id = %llu", firstPhaseId);
        return false;
    }

    // フェーズ開始
    m_pPhase->OnStart();

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理です.
//-----------------------------------------------------------------------------
void PhaseManager::Term()
{
    // フェーズ破棄.
    if (m_pPhase != nullptr && m_pFactory != nullptr)
    {
        m_pPhase->OnEnd();
        m_pFactory->Dispose(m_pPhase);
    }

    // ポインタクリア.
    m_pFactory = nullptr;
    m_pPhase   = nullptr;

    // フェーズ番号クリア.
    m_PhaseId  = UINT64_MAX;

    // ブラックボードをクリア.
    m_Blackboard.Clear();
}

//-----------------------------------------------------------------------------
//      フェーズの変更処理を行います.
//-----------------------------------------------------------------------------
void PhaseManager::Change(uint64_t phaseId)
{
    // フェーズ番号が同じなら処理しない.
    if (m_PhaseId == phaseId)
        return;

    // ファクトリーが設定されていることが必須.
    assert(m_pFactory != nullptr);

    // フェーズが存在していれば破棄処理.
    if (m_pPhase)
    {
        // 終了処理呼び出し.
        m_pPhase->OnEnd();

        // フェーズ破棄.
        m_pFactory->Dispose(m_pPhase);

        // ポインタクリア.
        m_pPhase = nullptr;
    }

    // デバッグログ表示.
    DLOGA("Change Phase (0x%llx) ---> (0x%llx).", m_PhaseId, phaseId);

    // フェーズ生成.
    m_pPhase = m_pFactory->Create(phaseId);
    assert(m_pPhase != nullptr);

    // フェーズ番号を設定.
    m_PhaseId = phaseId;

    // フェーズ開始処理を呼び出し.
    m_pPhase->OnStart();
}

//-----------------------------------------------------------------------------
//      フェーズの更新処理を行います.
//-----------------------------------------------------------------------------
void PhaseManager::Update(float deltaSec)
{
    if (!m_pPhase)
        return;

    m_pPhase->OnUpdate(deltaSec);
}

//-----------------------------------------------------------------------------
//      フェーズの描画処理を行います.
//-----------------------------------------------------------------------------
void PhaseManager::Draw(void* pArgs)
{
    if (!m_pPhase)
        return;

    m_pPhase->OnDraw(pArgs);
}

//-----------------------------------------------------------------------------
//      現在のフェーズをリスタートします.
//-----------------------------------------------------------------------------
void PhaseManager::Restart()
{
    if (!m_pPhase)
        return;

    m_pPhase->OnEnd();
    m_pPhase->OnStart();
}

//-----------------------------------------------------------------------------
//      フェーズ番号を取得します.
//-----------------------------------------------------------------------------
uint64_t PhaseManager::GetPhaseId() const
{ return m_PhaseId; }

//-----------------------------------------------------------------------------
//      ブラックボードを取得します.
//-----------------------------------------------------------------------------
Blackboard& PhaseManager::GetBlackboard()
{ return m_Blackboard; }

//-----------------------------------------------------------------------------
//      ブラックボードを取得します(const版).
//-----------------------------------------------------------------------------
const Blackboard& PhaseManager::GetBlackboard() const
{ return m_Blackboard; }

} // namespace asdx

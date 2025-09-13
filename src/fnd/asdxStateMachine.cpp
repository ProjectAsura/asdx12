//-----------------------------------------------------------------------------
// File : asdxStateMachine.cpp
// Desc : State Machine.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <fnd/asdxStateMachine.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// StateMachine class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
StateMachine::StateMachine()
{
    m_Holder.State      = kInvalidState;
    m_Holder.pListener  = nullptr;
}

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
StateMachine::~StateMachine()
{
    auto itr = m_Listeners.begin();
    while(itr != m_Listeners.end())
    {
        if (itr->second)
        {
            auto listener = itr->second;
            itr->second = nullptr;
            if (listener != nullptr)
            {
                delete listener;
                listener = nullptr;
            }
        }
        itr = m_Listeners.erase(itr);
    }

    m_Listeners.clear();

    m_Holder.pListener = nullptr;
    m_Holder.State     = kInvalidState;
}

//-----------------------------------------------------------------------------
//      ステートを登録します.
//-----------------------------------------------------------------------------
bool StateMachine::RegisterState(uint32_t state, IStateListener* pListener)
{
    auto itr = m_Listeners.find(state);
    if (itr == m_Listeners.end())
    {
        m_Listeners[state] = pListener;
        return true;
    }

    assert(itr == m_Listeners.end());
    return false;
}

//-----------------------------------------------------------------------------
//      更新処理を行います.
//-----------------------------------------------------------------------------
void StateMachine::Update(float deltaSec)
{
    if (m_Holder.pListener == nullptr)
        return;

    m_Holder.pListener->OnUpdate(deltaSec);
}

//-----------------------------------------------------------------------------
//      ステートを変更します.
//-----------------------------------------------------------------------------
void StateMachine::ChangeState(uint32_t nextState)
{
    // 同じなら変更しない.
    if (m_Holder.State == nextState)
        return;

    // ステート終了通知.
    if (m_Holder.pListener != nullptr)
    {
        m_Holder.pListener->OnLeave();
        m_Holder.pListener = nullptr;
    }

    // 次のステートへ.
    m_Holder.State = nextState;

    // 対応するステートがあるかどうかチェック.
    auto itr = m_Listeners.find(nextState);
    if (itr != m_Listeners.end())
    {
        // リスナーを変更.
        m_Holder.pListener = itr->second;
        assert(m_Holder.pListener != nullptr);

        // ステート開始通知.
        m_Holder.pListener->OnEnter();
    }
}

//-----------------------------------------------------------------------------
//      現在のステートを取得します.
//-----------------------------------------------------------------------------
uint32_t StateMachine::GetState() const
{ return m_Holder.State; }

} // namespace asdx

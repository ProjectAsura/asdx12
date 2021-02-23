﻿//-----------------------------------------------------------------------------
// File : asdxMessage.cpp
// Desc : Message System.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxMessage.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// MessageMgr class
///////////////////////////////////////////////////////////////////////////////
MessageMgr MessageMgr::s_Instance;

//-----------------------------------------------------------------------------
//      シングルトンインスタンスを取得します.
//-----------------------------------------------------------------------------
MessageMgr& MessageMgr::Instance()
{ return s_Instance; }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool MessageMgr::Init(size_t size)
{ return m_Heap.Init(size); }

//-----------------------------------------------------------------------------
//      解放処理を行います.
//-----------------------------------------------------------------------------
void MessageMgr::Term()
{
    m_Listeners.clear();
    m_Queue.Clear();
    m_Heap.Term();
}

//-----------------------------------------------------------------------------
//      メッセージリスナーを追加します.
//-----------------------------------------------------------------------------
void MessageMgr::Add(IMessageListener* instance)
{ m_Listeners.push_back(instance); }

//-----------------------------------------------------------------------------
//      メッセージリスナーを削除します.
//-----------------------------------------------------------------------------
void MessageMgr::Remove(IMessageListener* instance)
{ m_Listeners.remove(instance); }

//-----------------------------------------------------------------------------
//      全メッセージリスナーを破棄します.
//-----------------------------------------------------------------------------
void MessageMgr::Clear()
{ m_Listeners.clear(); }

//-----------------------------------------------------------------------------
//      メッセージを追加します.
//-----------------------------------------------------------------------------
void MessageMgr::PushMessage(const Message& msg)
{
    auto buf = m_Heap.Alloc(sizeof(Message));
    assert(buf != nullptr);

    if (msg.GetSize() > 0)
    {
        auto data = m_Heap.Alloc(msg.GetSize());
        memcpy(data, msg.GetBuffer(), msg.GetSize());
        
        auto instance = new (buf) Message(msg.GetType(), data, msg.GetSize());
        m_Queue.Push(instance);
    }
    else
    {
        auto instance = new (buf) Message(msg.GetType());
        m_Queue.Push(instance);
    }
}

//-----------------------------------------------------------------------------
//      メッセージをブロードキャストします.
//-----------------------------------------------------------------------------
void MessageMgr::BroadCast()
{
    while(!m_Queue.IsEmpty())
    {
        auto msg = m_Queue.Pop();

        for(auto& itr : m_Listeners)
        { itr->OnMessage(*msg); }
    }

    m_Heap.Reset();
}

} // namespace asdx

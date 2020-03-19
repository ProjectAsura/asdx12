﻿//-----------------------------------------------------------------------------
// File : AppHistoryMgr.cpp
// Desc : History Manager For Application.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxAppHistoryMgr.h>
#include <asdxEditParam.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// AppHistoryMgr class
///////////////////////////////////////////////////////////////////////////////
AppHistoryMgr AppHistoryMgr::s_Instance;

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
AppHistoryMgr::AppHistoryMgr()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
AppHistoryMgr::~AppHistoryMgr()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      シングルトンインスタンスを取得します.
//-----------------------------------------------------------------------------
AppHistoryMgr& AppHistoryMgr::GetInstance()
{ return s_Instance; }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool AppHistoryMgr::Init(int capacity)
{ return m_Manager.Init(capacity); }

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void AppHistoryMgr::Term()
{ m_Manager.Term(); }

//-----------------------------------------------------------------------------
//      履歴を登録します.
//-----------------------------------------------------------------------------
void AppHistoryMgr::Add(asdx::IHistory* item, bool redo)
{ m_Manager.Add(item, redo); }

//-----------------------------------------------------------------------------
//      履歴をクリアします.
//-----------------------------------------------------------------------------
void AppHistoryMgr::Clear()
{ m_Manager.Clear(); }

//-----------------------------------------------------------------------------
//      やり直します.
//-----------------------------------------------------------------------------
void AppHistoryMgr::Redo()
{
    if (m_Manager.CanRedo())
    { m_Manager.Redo(); }
}

//-----------------------------------------------------------------------------
//      元に戻します.
//-----------------------------------------------------------------------------
void AppHistoryMgr::Undo()
{
    if (m_Manager.CanUndo())
    { m_Manager.Undo(); }
}

//-----------------------------------------------------------------------------
//      初期化済みかどうか?
//-----------------------------------------------------------------------------
bool AppHistoryMgr::IsInit() const
{ return m_Manager.IsInit(); }

} // namespace asdx

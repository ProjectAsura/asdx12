//-----------------------------------------------------------------------------
// File : asdxHistory.h
// Desc : History Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <list>
#include <vector>
#include <functional>
#include <fnd/asdxSpinLock.h>


namespace asdx {

//-----------------------------------------------------------------------------
// Type Definition
//-----------------------------------------------------------------------------
using HistoryAction = std::function<void(void)>;


///////////////////////////////////////////////////////////////////////////////
// IHistoryEventListener interface
///////////////////////////////////////////////////////////////////////////////
struct IHistoryEventListener
{
    virtual ~IHistoryEventListener() {}
    virtual void OnChanged() = 0;
};

///////////////////////////////////////////////////////////////////////////////
// IHistory interface
///////////////////////////////////////////////////////////////////////////////
struct IHistory
{
    virtual ~IHistory() {}
    virtual void Redo() = 0;
    virtual void Undo() = 0;
};

///////////////////////////////////////////////////////////////////////////////
// HistoryEventHandler class
///////////////////////////////////////////////////////////////////////////////
class HistoryEventHandler
{
public:
    HistoryEventHandler();
    ~HistoryEventHandler();

    void Invoke();
    HistoryEventHandler& operator += (IHistoryEventListener* listener);
    HistoryEventHandler& operator -= (IHistoryEventListener* listener);

private:
    std::list<IHistoryEventListener*>  m_Listeners;
};

///////////////////////////////////////////////////////////////////////////////
// History class
///////////////////////////////////////////////////////////////////////////////
class History : public IHistory
{
public:
    History(HistoryAction redo, HistoryAction undo);
    ~History();
    void Redo() override;
    void Undo() override;

private:
    HistoryAction  m_Redo;
    HistoryAction  m_Undo;
};

///////////////////////////////////////////////////////////////////////////////
// GroupHistory class
///////////////////////////////////////////////////////////////////////////////
class GroupHistory : public IHistory
{
public:
    HistoryEventHandler UndoExecuted;
    HistoryEventHandler RedoExecuted;

    GroupHistory();
    ~GroupHistory();

    void Add     (IHistory* item);
    void Clear   ();
    bool IsEmpty () const;
    int  GetCount() const;
    void Redo    () override;
    void Undo    () override;

private:
    std::list<IHistory*>    m_Histories;
};

///////////////////////////////////////////////////////////////////////////////
// HistoryMgr class
///////////////////////////////////////////////////////////////////////////////
class HistoryMgr
{
public:
    HistoryEventHandler UndoExecuted;
    HistoryEventHandler RedoExecuted;

    static HistoryMgr& Instance();

    bool Init           (int capacity);
    void Term           ();
    void Add            (IHistory* item, bool redo = true);
    void Clear          ();
    void Redo           ();
    void Undo           ();
    int  GetCurrent     () const;
    int  GetRedoCount   () const;
    int  GetUndoCount   () const;
    bool CanRedo        () const;
    bool CanUndo        () const;
    bool IsEmpty        () const;
    bool IsInit         () const;

private:
    static HistoryMgr       s_Instance;
    std::vector<IHistory*>  m_Histories = {};
    int                     m_Current   = 0;
    int                     m_Capacity  = 0;
    bool                    m_Init      = false;
    SpinLock                m_SpinLock  = {};

    HistoryMgr ();
    ~HistoryMgr();
};


} // namespace asdx

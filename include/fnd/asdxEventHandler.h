//-----------------------------------------------------------------------------
// File : asdxEventHandler.h
// Desc : Event Handler.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <list>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// IEventListener interface
///////////////////////////////////////////////////////////////////////////////
struct IEventListener
{
    virtual ~IEventListener() {}
    virtual void OnEvent() = 0;
};

///////////////////////////////////////////////////////////////////////////////
// EventHandler class
///////////////////////////////////////////////////////////////////////////////
class EventHandler
{
public:
    EventHandler();
    ~EventHandler();

    void Invoke();
    EventHandler& operator += (IEventListener* listener);
    EventHandler& operator -= (IEventListener* listener);

private:
    std::list<IEventListener*>  m_Listeners;
};

} // namespace asdx

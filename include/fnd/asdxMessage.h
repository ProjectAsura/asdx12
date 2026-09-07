//-----------------------------------------------------------------------------
// File : asdxMessage.h
// Desc : Message System.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdint>
#include <list>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Message class
///////////////////////////////////////////////////////////////////////////////
class Message
{
public:
    explicit Message(uint32_t type, const void* buffer = nullptr, uint64_t size = 0);
    uint32_t GetType() const;
    uint64_t GetSize() const;
    const void* GetBuffer() const;

    template<typename T>
    const T* GetAs() const
    { return reinterpret_cast<const T*>(m_pBuffer); }

protected:
    uint32_t    m_Type    = 0;
    uint64_t    m_Size    = 0;
    const void* m_pBuffer = nullptr;
};

///////////////////////////////////////////////////////////////////////////////
// IMessageListener interface
///////////////////////////////////////////////////////////////////////////////
struct IMessageListener
{
    virtual ~IMessageListener() = default;
    virtual void OnMessage(const Message& msg) = 0;
};

///////////////////////////////////////////////////////////////////////////////
// TypedMessage class
///////////////////////////////////////////////////////////////////////////////
template<typename T>
class TypedMessage : public Message
{
public:
    TypedMessage(uint32_t type, const T& data)
    : Message(type, &m_Data, sizeof(T))
    , m_Data(data)
    { /* DO_NOTHING */ }

private:
    T m_Data = {};
};

///////////////////////////////////////////////////////////////////////////////
// MessageHandler class
///////////////////////////////////////////////////////////////////////////////
class MessageHandler
{
public:
    MessageHandler() = default;
    ~MessageHandler();
    void Send(const Message& msg);
    MessageHandler& operator += (IMessageListener* listener);
    MessageHandler& operator -= (IMessageListener* listener);

private:
    std::list<IMessageListener*> m_Listeners;
};

} // namespace asdx


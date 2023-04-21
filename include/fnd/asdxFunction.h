//-----------------------------------------------------------------------------
// File : asdxFunction.h
// Desc : Fixed Size Function.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <utility>
#include <new>


namespace asdx {

template<typename Function, size_t MaxSize = 16, size_t Align = 8>
class function;

template<typename ReturnType, typename... Args, size_t MaxSize, size_t Align>
class function<ReturnType(Args...), MaxSize, Align>
{
public:
    using result_type = ReturnType;

    function()
    { /* DO_NOTHING */ }

    ~function()
    { reset(); }

    function(std::nullptr_t)
    { /* DO_NOTHING */ }

    function(function const& value)
    { copy(value); }

    function(function& value)
    { copy(value); }

    function(function&& value)
    { move(std::move(value)); }

    template<typename Functor>
    function(Functor&& f)
    { create(std::forward<Functor>(f)); }

    function& operator = (std::nullptr_t)
    {
        reset();
        return *this;
    }

    function& operator = (function const& value)
    {
        assign(value);
        return *this;
    }

    function& operator = (function& value)
    {
        assign(value);
        return *this;
    }

    function& operator = (function&& value)
    {
        assign(std::move(value));
        return *this;
    }

    template<typename Functor>
    function& operator = (Functor&& f)
    {
        assign(std::forward<Functor>(f));
        return *this;
    }

    void assign(function const& value)
    {
        reset();
        copy(value);
    }

    void assign(function& value)
    {
        reset();
        copy(value);
    }

    void assign(function&& value)
    {
        reset();
        move(std::move(value));
    }

    template<typename Functor>
    void assign(Functor&& f)
    {
        reset();
        create(std::forward<Functor>(f));
    }

    void reset()
    {
        auto dispose = m_VTable.Dispose;
        if (dispose)
        {
            m_VTable = VirtualMethodTable();
            dispose(m_Storage);
        }
    }

    explicit operator bool() const
    { return m_VTable.Invoke != nullptr; }

    ReturnType operator()(Args... args)
    {
        assert(m_VTable.Invoke != nullptr);
        return m_VTable.Invoke(m_Storage, std::forward<Args>(args)...);
    }

    void swap(function& other)
    {
        function temp = std::move(other);
        other = std::move(*this);
        *this = std::move(temp);
    }

    friend void swap(function& lhs, function& rhs)
    { lhs.swap(rhs); }

    friend bool operator == (std::nullptr_t, function const& action)
    { return !action; }

    friend bool operator == (function const& action, std::nullptr_t)
    { return !action; }

    friend bool operator != (std::nullptr_t, function const& action)
    { return action; }

    friend bool operator != (function const& action, std::nullptr_t)
    { return action; }

private:
    struct VirtualMethodTable
    {
        ReturnType (*Invoke)  (void*, Args&& ...)     = nullptr;
        void       (*Dispose) (void*)                 = nullptr;
        void       (*Copy)    (const void*, void*)    = nullptr;
        void       (*Move)    (void*, void*)          = nullptr;
    };
    VirtualMethodTable      m_VTable;
    alignas(Align) uint8_t  m_Storage[MaxSize];

    template<typename Functor>
    void create(Functor&& f)
    {
        using FunctorType = typename std::decay<Functor>::type;
        static_assert(sizeof(FunctorType) <= MaxSize);

        new (m_Storage) FunctorType(std::forward<Functor>(f));

        m_VTable.Invoke  = &Invoke<FunctorType>;
        m_VTable.Dispose = &Dispose<FunctorType>;
        m_VTable.Copy    = &Copy<FunctorType>;
        m_VTable.Move    = &Move<FunctorType>;
    }

    void copy(function const& value)
    {
        assert(m_VTable.Copy != nullptr);
        value.m_VTable.Copy(value.m_Storage, m_Storage);
        m_VTable = value.m_VTable;
    }

    void move(function&& value, std::true_type movable)
    {
        assert(m_VTable.Move != nullptr);
        value.m_VTable.Move(value.m_Storage, m_Storage);
        m_VTable = value.m_VTable;
        value.reset();
    }

    void move(function const& value, std::false_type movable)
    { copy(value); }

    template<typename Functor>
    static ReturnType Invoke(void* functor, Args&&... args)
    { return (*static_cast<Functor*>(functor))(std::forward<Args>(args)...); }

    template<typename Functor>
    static void Dispose(void* functor)
    { static_cast<Functor*>(functor)->~Functor(); }

    template<typename Functor>
    static void Copy(void const* functor, void* dest)
    { new (dest) Functor(*static_cast<Functor const*>(functor)); }

    template<typename Functor>
    static void Move(void* functor, void* dest)
    { new (dest) Functor(std::move(*static_cast<Functor*>(functor))); }

    // アクセス禁止.
    template<typename F, size_t S, size_t A> function             (function<F, S, A> const&)    = delete;
    template<typename F, size_t S, size_t A> function             (function<F, S, A>&)          = delete;
    template<typename F, size_t S, size_t A> function             (function<F, S, A>&&)         = delete;
    template<typename F, size_t S, size_t A> function& operator = (function<F, S, A> const&)    = delete;
    template<typename F, size_t S, size_t A> function& operator = (function<F, S, A>&)          = delete;
    template<typename F, size_t S, size_t A> function& operator = (function<F, S, A>&&)         = delete;
    template<typename F, size_t S, size_t A> void      assign     (function<F, S, A> const&)    = delete;
    template<typename F, size_t S, size_t A> void      assign     (function<F, S, A>&)          = delete;
    template<typename F, size_t S, size_t A> void      assign     (function<F, S, A>&&)         = delete;
};

template<typename... Args>
using Action = function<void(Args...)>;

} // namespace asdx

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

template<typename Function, size_t MaxSize=16, size_t Align=8>
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
        if (m_Base == nullptr)
        { return; }

        m_Base->~Base();
        m_Base = nullptr;
    }

    explicit operator bool() const
    { return m_Base != nullptr; }

    ReturnType operator()(Args... args)
    {
        assert(m_Base != nullptr);
        return m_Base->Invoke(std::forward<Args>(args)...);
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
    struct Base
    {
        virtual ~Base() {}
        virtual ReturnType Invoke(Args&& ...) = 0;
        virtual void       Copy  (void*)      = 0;
        virtual void       Move  (void*)      = 0;
    };

    template<typename Functor>
    struct Derived : Base
    {
        Functor func;

        Derived(Functor f)
        : func(std::move(f))
        { /* DO_NOTHING */ }

        ReturnType Invoke(Args&& ... args) override
        { return func(std::forward<Args>(args)...); }

        void Copy(void* dest) override
        { new (dest) Functor(func); }

        void Move(void* dest) override
        { new (dest) Functor(std::move(func)); }
    };

    alignas(Align) uint8_t  m_Storage[MaxSize] = {};
    Base*                   m_Base             = nullptr;

    template<typename Functor>
    void create(Functor&& f)
    {
        static_assert(sizeof(Functor) <= MaxSize);
        static_assert(Align % alignof(Functor) == 0u);
        m_Base = new(m_Storage) Derived<Functor>(std::forward<Functor>(f));
    }

    void copy(function const& value)
    {
        m_Base->Copy(value.m_Storage);
        m_Base = value.m_Base;
    }

    void move(function&& value)
    {
        m_Base->Move(value.m_Storage);
        m_Base = value.m_Base;
        value.reset();
    }

    // 以下, アクセス禁止.
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

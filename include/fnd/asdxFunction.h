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
#include <cstdint>
#include <cstddef>      // for nullptr_t, max_align_t
#include <type_traits>  // for decay enable_if, is_same
#include <utility>      // for forward, move
#include <new>


namespace asdx {

//=============================================================================
// Forward Declarations.
//=============================================================================
template<typename Func, size_t MaxSize=64, size_t Align=alignof(std::max_align_t)>
class Function;

///////////////////////////////////////////////////////////////////////////////
// Function class
///////////////////////////////////////////////////////////////////////////////
template<typename ReturnType, typename... Args, size_t MaxSize, size_t Align>
class Function<ReturnType(Args...), MaxSize, Align>
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // Type Definition.
    //=========================================================================
    using result_type = ReturnType;

    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================

    Function()
    { /* DO_NOTHING */ }

    ~Function()
    { reset(); }

    Function(std::nullptr_t)
    { /* DO_NOTHING */ }

    Function(Function const& value)
    { copy(value); }

    Function(Function&& value)
    { move(std::move(value)); }

    template<typename Functor,
        typename std::enable_if<!std::is_same<typename std::decay<Functor>::type, Function>::value, int>::type = 0>
    Function(Functor&& f)
    { create(std::forward<Functor>(f)); }

    Function& operator = (std::nullptr_t)
    {
        reset();
        return *this;
    }

    Function& operator = (Function const& value)
    {
        assign(value);
        return *this;
    }

    Function& operator = (Function&& value)
    {
        assign(std::move(value));
        return *this;
    }

    template<typename Functor,
        typename std::enable_if<!std::is_same<typename std::decay<Functor>::type, Function>::value, int>::type = 0>
    Function& operator = (Functor&& f)
    {
        assign(std::forward<Functor>(f));
        return *this;
    }

    void assign(Function const& value)
    {
        if (this == &value)
        { return; }

        reset();
        copy(value);
    }

    void assign(Function&& value)
    {
        if (this == &value)
        { return; }

        reset();
        move(std::move(value));
    }

    template<typename Functor,
        typename std::enable_if<!std::is_same<typename std::decay<Functor>::type, Function>::value, int>::type = 0>
    void assign(Functor&& f)
    {
        reset();
        create(std::forward<Functor>(f));
    }

    void reset()
    {
        if (m_Base == nullptr)
        { return; }

        m_Base->Dispose();
        m_Base = nullptr;
    }

    explicit operator bool() const
    { return m_Base != nullptr; }

    ReturnType operator()(Args... args) const
    {
        assert(m_Base != nullptr);
        return m_Base->Invoke(std::forward<Args>(args)...);
    }

    void swap(Function& other)
    {
        if (this == &other)
        { return; }

        auto temp = std::move(other);
        other = std::move(*this);
        *this = std::move(temp);
    }

    friend void swap(Function& lhs, Function& rhs)
    { lhs.swap(rhs); }

    friend bool operator == (std::nullptr_t, Function const& action)
    { return nullptr == action.m_Base; }

    friend bool operator == (Function const& action, std::nullptr_t)
    { return action.m_Base == nullptr; }

    friend bool operator != (std::nullptr_t, Function const& action)
    { return nullptr != action.m_Base; }

    friend bool operator != (Function const& action, std::nullptr_t)
    { return action.m_Base != nullptr; }

private:
    struct Base
    {
        virtual ~Base() {}
        virtual ReturnType Invoke (Args&& ...) const = 0;
        virtual Base*      Copy   (void*)      const = 0;
        virtual Base*      Move   (void*)            = 0;
        virtual void       Dispose()                 = 0;
    };

    template<typename Functor>
    struct Derived : public Base
    {
        mutable Functor func;

        Derived(Functor f)
        : func(std::move(f))
        { /* DO_NOTHING */ }

        ReturnType Invoke(Args&& ... args) const override
        { return func(std::forward<Args>(args)...); }

        Base* Copy(void* dest) const override
        { return new (dest) Derived(func); }

        Base* Move(void* dest) override
        { return new (dest) Derived(std::move(func)); }

        void Dispose() override
        { this->~Derived(); }
    };

    alignas(Align) uint8_t  m_Storage[MaxSize] = {};
    Base*                   m_Base             = nullptr;

    template<typename Functor>
    void create(Functor&& f)
    {
        using StoredFunctor = std::decay_t<Functor>;
        using StoredType    = Derived<StoredFunctor>;

        static_assert(sizeof(StoredType) <= MaxSize,
            "Function callable exceeds MaxSize");
        static_assert(alignof(StoredType) <= Align,
            "Function Align is too small for callable");

        m_Base = new(m_Storage) StoredType(std::forward<Functor>(f));
    }

    void copy(Function const& value)
    {
        if (value.m_Base == nullptr)
        { return; }

        m_Base = value.m_Base->Copy(m_Storage);
    }

    void move(Function&& value)
    {
        if (value.m_Base == nullptr)
        { return; }

        m_Base = value.m_Base->Move(m_Storage);
        value.reset();
    }
};

template<typename... Args>
using Action = Function<void(Args...)>;

} // namespace asdx

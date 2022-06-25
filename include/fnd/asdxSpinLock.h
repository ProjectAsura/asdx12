﻿//-----------------------------------------------------------------------------
// File : asdxSpinLock
// Desc : Spin Lock
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <atomic>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// SpinLock class
///////////////////////////////////////////////////////////////////////////////
class SpinLock
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      ロックします.
    //-------------------------------------------------------------------------
    void lock()
    {
        while (m_State.test_and_set(std::memory_order_acquire))
        { _mm_pause(); }
    }

    //-------------------------------------------------------------------------
    //! @brief      ロックを解除します.
    //-------------------------------------------------------------------------
    void unlock()
    { m_State.clear(std::memory_order_release); }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    std::atomic_flag    m_State = ATOMIC_FLAG_INIT;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};


///////////////////////////////////////////////////////////////////////////////
// ScopedLock class
///////////////////////////////////////////////////////////////////////////////
class ScopedLock
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    ScopedLock(SpinLock* value)
    : m_SpinLock(value)
    {
        if (m_SpinLock != nullptr)
        { m_SpinLock->lock(); }
    }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~ScopedLock()
    {
        if (m_SpinLock != nullptr)
        { m_SpinLock->unlock(); }
    }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    SpinLock*   m_SpinLock = nullptr;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

} // namespace asdx

﻿//-----------------------------------------------------------------------------
// File : asdxBitFlags.
// Desc : Bit Flags.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <cstdint>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// BitFlags8 class.
///////////////////////////////////////////////////////////////////////////////
class BitFlags8
{
    //=========================================================================
    // list of friend classes.
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
    BitFlags8() = default;

    //-------------------------------------------------------------------------
    //! @brief      引数付きコンストラクタです.
    //! 
    //! @param[in]      value       初期値.
    //-------------------------------------------------------------------------
    BitFlags8(uint8_t value)
    : m_Flags(value)
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~BitFlags8() = default;

    //-------------------------------------------------------------------------
    //! @brief      フラグを設定します.
    //! 
    //! @param[in]      index       インデックス.
    //! @param[in]      value       設定する値.
    //-------------------------------------------------------------------------
    void Set(size_t index, bool value)
    {
        assert(index < 8);
        uint8_t bit = (0x1 << index);
        if (value)
        { m_Flags |= bit; }
        else
        { m_Flags &= ~bit; }
    }

    //-------------------------------------------------------------------------
    //! @brief      フラグを取得します.
    //! 
    //! @param[in]      index       インデックス.
    //! @return     フラグを返却します.
    //-------------------------------------------------------------------------
    bool Get(size_t index) const
    {
        assert(index < 8);
        uint8_t bit = (0x1 << index);
        return !!(m_Flags & bit);
    }

    //-------------------------------------------------------------------------
    //! @brief      フラグをリセットします.
    //-------------------------------------------------------------------------
    void Reset()
    { m_Flags = 0; }

    //-------------------------------------------------------------------------
    //! @brief      任意のフラグが立っているかどうかチェックします.
    //! 
    //! @return     いずれかのフラグが立っていれば true を返却します.
    //-------------------------------------------------------------------------
    bool Any() const
    { return m_Flags != 0; }

    //-------------------------------------------------------------------------
    //! @brief      全てのフラグが立っているかどうかチェックします.
    //! 
    //! @return     全てのフラグが立っていれば true を返却します.
    //-------------------------------------------------------------------------
    bool All() const
    { return m_Flags == 0xFF; }

    //-------------------------------------------------------------------------
    //! @brief      全てのビットが0になっているか判定します.
    //! 
    //! @return     全てのビットが0であれば true を返却します.
    //-------------------------------------------------------------------------
    bool None() const
    { return m_Flags == 0; }

    //-------------------------------------------------------------------------
    //! @brief      キャスト演算子です.
    //-------------------------------------------------------------------------
    operator uint8_t () const
    { return m_Flags; }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    uint8_t m_Flags = 0;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

///////////////////////////////////////////////////////////////////////////////
// BitFlags16 class.
///////////////////////////////////////////////////////////////////////////////
class BitFlags16
{
    //=========================================================================
    // list of friend classes.
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
    BitFlags16 () = default;

    //-------------------------------------------------------------------------
    //! @brief      引数付きコンストラクタです.
    //! 
    //! @param[in]      value       初期値.
    //-------------------------------------------------------------------------
    BitFlags16(uint16_t value)
    : m_Flags(value)
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~BitFlags16() = default;

    //-------------------------------------------------------------------------
    //! @brief      フラグを設定します.
    //! 
    //! @param[in]      index       インデックス.
    //! @param[in]      value       設定する値.
    //-------------------------------------------------------------------------
    void Set(size_t index, bool value)
    {
        assert(index < 16);
        uint16_t bits = (0x1 << index);
        if (value)
        { m_Flags |= bits; }
        else
        { m_Flags &= ~bits; }
    }

    //-------------------------------------------------------------------------
    //! @brief      フラグを取得します.
    //! 
    //! @param[in]      index       インデックス.
    //! @return     フラグを返却します.
    //-------------------------------------------------------------------------
    bool Get(size_t index) const
    {
        assert(index < 16);
        uint16_t bits = (0x1 << index);
        return !!(m_Flags & bits);
    }

    //-------------------------------------------------------------------------
    //! @brief      フラグをリセットします.
    //-------------------------------------------------------------------------
    void Reset()
    { m_Flags = 0; }

    //-------------------------------------------------------------------------
    //! @brief      任意のフラグが立っているかどうかチェックします.
    //! 
    //! @return     いずれかのフラグが立っていれば true を返却します.
    //-------------------------------------------------------------------------
    bool Any() const
    { return m_Flags != 0; }

    //-------------------------------------------------------------------------
    //! @brief      全てのフラグが立っているかどうかチェックします.
    //! 
    //! @return     全てのフラグが立っていれば true を返却します.
    //-------------------------------------------------------------------------
    bool All() const
    { return m_Flags == 0xFFFF; }

    //-------------------------------------------------------------------------
    //! @brief      全てのビットが0になっているか判定します.
    //! 
    //! @return     全てのビットが0であれば true を返却します.
    //-------------------------------------------------------------------------
    bool None() const
    { return m_Flags == 0; }

    //-------------------------------------------------------------------------
    //! @brief      キャスト演算子です.
    //-------------------------------------------------------------------------
    operator uint16_t () const
    { return m_Flags; }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    uint16_t m_Flags = 0;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

///////////////////////////////////////////////////////////////////////////////
// BitFlags32 class.
///////////////////////////////////////////////////////////////////////////////
class BitFlags32
{
    //=========================================================================
    // list of friend classes.
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
    BitFlags32 () = default;

    //-------------------------------------------------------------------------
    //! @brief      引数付きコンストラクタです.
    //! 
    //! @param[in]      value       初期値.
    //-------------------------------------------------------------------------
    BitFlags32(uint32_t value)
    : m_Flags(value)
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~BitFlags32() = default;

    //-------------------------------------------------------------------------
    //! @brief      フラグを設定します.
    //! 
    //! @param[in]      index       インデックス.
    //! @param[in]      value       設定する値.
    //-------------------------------------------------------------------------
    void Set(size_t index, bool value)
    {
        assert(index < 32);
        uint32_t bits = (0x1u << index);
        if (value)
        { m_Flags |= bits; }
        else
        { m_Flags &= ~bits; }
    }

    //-------------------------------------------------------------------------
    //! @brief      フラグを取得します.
    //! 
    //! @param[in]      index       インデックス.
    //! @return     フラグを返却します.
    //-------------------------------------------------------------------------
    bool Get(size_t index) const
    {
        assert(index < 32);
        uint32_t bits = (0x1u << index);
        return !!(m_Flags & bits);
    }

    //-------------------------------------------------------------------------
    //! @brief      フラグをリセットします.
    //-------------------------------------------------------------------------
    void Reset()
    { m_Flags = 0; }

    //-------------------------------------------------------------------------
    //! @brief      任意のフラグが立っているかどうかチェックします.
    //! 
    //! @return     いずれかのフラグが立っていれば true を返却します.
    //-------------------------------------------------------------------------
    bool Any() const
    { return m_Flags != 0; }

    //-------------------------------------------------------------------------
    //! @brief      全てのフラグが立っているかどうかチェックします.
    //! 
    //! @return     全てのフラグが立っていれば true を返却します.
    //-------------------------------------------------------------------------
    bool All() const
    { return m_Flags == 0xFFFFFFFF; }

    //-------------------------------------------------------------------------
    //! @brief      全てのビットが0になっているか判定します.
    //! 
    //! @return     全てのビットが0であれば true を返却します.
    //-------------------------------------------------------------------------
    bool None() const
    { return m_Flags == 0; }

    //-------------------------------------------------------------------------
    //! @brief      キャスト演算子です.
    //-------------------------------------------------------------------------
    operator uint32_t () const
    { return m_Flags; }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    uint32_t m_Flags = 0;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

///////////////////////////////////////////////////////////////////////////////
// BitFlags64 class.
///////////////////////////////////////////////////////////////////////////////
class BitFlags64
{
    //=========================================================================
    // list of friend classes.
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
    BitFlags64 () = default;

    //-------------------------------------------------------------------------
    //! @brief      引数付きコンストラクタです.
    //! 
    //! @param[in]      value       初期値.
    //-------------------------------------------------------------------------
    BitFlags64(uint64_t value)
    : m_Flags(value)
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~BitFlags64() = default;

    //-------------------------------------------------------------------------
    //! @brief      フラグを設定します.
    //! 
    //! @param[in]      index       インデックス.
    //! @param[in]      value       設定する値.
    //-------------------------------------------------------------------------
    void Set(size_t index, bool value)
    {
        assert(index < 64);
        uint64_t bits = (0x1llu << index);
        if (value)
        { m_Flags |= bits; }
        else
        { m_Flags &= ~bits; }
    }

    //-------------------------------------------------------------------------
    //! @brief      フラグを取得します.
    //! 
    //! @param[in]      index       インデックス.
    //! @return     フラグを返却します.
    //-------------------------------------------------------------------------
    bool Get(size_t index) const
    {
        assert(index < 64);
        uint64_t bits = (0x1llu << index);
        return !!(m_Flags & bits);
    }

    //-------------------------------------------------------------------------
    //! @brief      フラグをリセットします.
    //-------------------------------------------------------------------------
    void Reset()
    { m_Flags = 0; }

    //-------------------------------------------------------------------------
    //! @brief      任意のフラグが立っているかどうかチェックします.
    //! 
    //! @return     いずれかのフラグが立っていれば true を返却します.
    //-------------------------------------------------------------------------
    bool Any() const
    { return m_Flags != 0; }

    //-------------------------------------------------------------------------
    //! @brief      全てのフラグが立っているかどうかチェックします.
    //! 
    //! @return     全てのフラグが立っていれば true を返却します.
    //-------------------------------------------------------------------------
    bool All() const
    { return m_Flags == 0xFFFFFFFFFFFFFFFF; }

    //-------------------------------------------------------------------------
    //! @brief      全てのビットが0になっているか判定します.
    //! 
    //! @return     全てのビットが0であれば true を返却します.
    //-------------------------------------------------------------------------
    bool None() const
    { return m_Flags == 0; }

    //-------------------------------------------------------------------------
    //! @brief      キャスト演算子です.
    //-------------------------------------------------------------------------
    operator uint64_t () const
    { return m_Flags; }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    uint64_t m_Flags = 0;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

static_assert(sizeof(BitFlags8)  == sizeof(uint8_t));
static_assert(sizeof(BitFlags16) == sizeof(uint16_t));
static_assert(sizeof(BitFlags32) == sizeof(uint32_t));
static_assert(sizeof(BitFlags64) == sizeof(uint64_t));

} // namespace asdx

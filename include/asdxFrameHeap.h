//-----------------------------------------------------------------------------
// File : FrameHeap.h
// Desc : Frame Heap
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdint>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// FrameHeap class
///////////////////////////////////////////////////////////////////////////////
class FrameHeap
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
    FrameHeap();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~FrameHeap();

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      size        メモリ確保サイズ.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(size_t size);

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      バッファ先頭にオフセットをリセットします.
    //-------------------------------------------------------------------------
    void Reset();

    //-------------------------------------------------------------------------
    //! @brief      メモリを確保します.
    //!
    //! @param[in]      size        確保するメモリサイズ.
    //! @return     確保したメモリへのポインタを返却します.
    //!             メモリ確保に失敗した場合は nullptr が返却されます.
    //-------------------------------------------------------------------------
    void* Alloc(size_t size);

    //-------------------------------------------------------------------------
    //! @brief      メモリサイズを取得します.
    //!
    //! @return     メモリサイズを返却します.
    //-------------------------------------------------------------------------
    size_t GetSize() const;

    //-------------------------------------------------------------------------
    //! @brief      利用可能なメモリサイズを取得します.
    //!
    //! @return     利用可能なメモリサイズを返却します.
    //-------------------------------------------------------------------------
    size_t GetRestSize() const;

    //-------------------------------------------------------------------------
    //! @brief      メモリを確保します.
    //!
    //! @param[in]      size        確保するメモリサイズ.
    //! @return     確保したメモリへのポインタを返却します.
    //!             メモリ確保に失敗した場合は nullptr が返却されます.
    //-------------------------------------------------------------------------
    template<typename T>
    T* Alloc()
    {
        auto buf = Alloc(sizeof(T));
        return new(buf) T();
    }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    size_t      m_Size;         //!< バッファサイズです.
    uint8_t*    m_pBuffer;      //!< バッファメモリです.
    size_t      m_Offset;       //!< バッファ先頭からのオフセットです.

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

} // namespace asdx
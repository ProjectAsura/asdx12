﻿//-----------------------------------------------------------------------------
// File : asdxIndexBuffer.h
// Desc : Index Buffer Wrapper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxGraphicsDevice.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// IndexBuffer class
///////////////////////////////////////////////////////////////////////////////
class IndexBuffer
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
    IndexBuffer();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~IndexBuffer();

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      device          グラフィックスデバイスです.
    //! @param[in]      size            バッファサイズです.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(GraphicsDevice& device, uint32_t size);

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      メモリマッピングを行います.
    //!
    //! @return     マッピング先のメモリアドレスを返却します.
    //!             マッピングに失敗した場合は nullptr が返却します.
    //-------------------------------------------------------------------------
    void* Map();

    //-------------------------------------------------------------------------
    //! @brief      メモリマッピングを解除します.
    //-------------------------------------------------------------------------
    void Unmap();

    //-------------------------------------------------------------------------
    //! @brief      インデックスバッファビューを取得します.
    //!
    //! @return     インデックスバッファビューを返却します.
    //-------------------------------------------------------------------------
    D3D12_INDEX_BUFFER_VIEW GetView() const;

    //-------------------------------------------------------------------------
    //! @brief      リソースを取得します.
    //!
    //! @return     リソースを返却します.
    //-------------------------------------------------------------------------
    ID3D12Resource* GetResource() const;

    //-------------------------------------------------------------------------
    //! @brief      メモリマッピングを行います.
    //!
    //! @return     マッピング先のメモリアドレスを返却します.
    //!             マッピングに失敗した場合は nullptr が返却します.
    //-------------------------------------------------------------------------
    template<typename T>
    T* Map()
    { return reinterpret_cast<T*>(Map()); }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    RefPtr<ID3D12Resource>      m_pResource;    //!< リソースです.
    D3D12_INDEX_BUFFER_VIEW     m_View;         //!< インデックスバッファビューです.

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

} // namespace asdx

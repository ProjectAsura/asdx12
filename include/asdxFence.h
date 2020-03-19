﻿//-----------------------------------------------------------------------------
// File : asdxFence.h
// Desc : Fence Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdint>
#include <d3d12.h>
#include <asdxRef.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Fence class
///////////////////////////////////////////////////////////////////////////////
class Fence final
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
    Fence();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~Fence();

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      pDevice         デバイスです.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init( ID3D12Device* pDevice );

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      コマンド実行完了を待機します.
    //!
    //! @param[in]      pQueue      コマンドキューです.
    //! @param[in]      mesc        タイムアウト時間(ミリ秒).
    //-------------------------------------------------------------------------
    void Wait( ID3D12CommandQueue* pQueue, uint32_t msec );

    //-------------------------------------------------------------------------
    //! @brief      フェンスを取得します.
    //!
    //! @return     フェンスを返却します.
    //-------------------------------------------------------------------------
    ID3D12Fence* GetPtr() const;

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    RefPtr<ID3D12Fence>     m_Fence;        //!< フェンスです.
    HANDLE                  m_Handle;       //!< イベントハンドルです.
    UINT64                  m_Counter;      //!< フェンスカウンタです.

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

} // namespace asdx



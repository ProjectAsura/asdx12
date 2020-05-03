﻿//-----------------------------------------------------------------------------
// File : asdxCommandList.h
// Desc : Command List Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxGraphicsDevice.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// CommandList class
///////////////////////////////////////////////////////////////////////////////
class CommandList
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
    CommandList();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~CommandList();

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      pDevice         デバイスです.
    //! @param[in]      type            コマンドリストタイプです.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(GraphicsDevice& device, D3D12_COMMAND_LIST_TYPE type);

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      コマンドリストをリセットします.
    //-------------------------------------------------------------------------
    ID3D12GraphicsCommandList6* Reset();

    //-------------------------------------------------------------------------
    //! @brief      アロケータを取得します.
    //!
    //! @param[in]      index       バッファ番号です.
    //! @return     コマンドアロケータを返却します.
    //-------------------------------------------------------------------------
    ID3D12CommandAllocator* GetAllocator(uint8_t index) const;

    //-------------------------------------------------------------------------
    //! @brief      グラフィックスコマンドリストを取得します.
    //!
    //! @return     グラフィックスコマンドリストを返却します.
    //-------------------------------------------------------------------------
    ID3D12GraphicsCommandList6* GetCommandList() const;

    //-------------------------------------------------------------------------
    //! @brief      コマンドリストにキャストします.
    //-------------------------------------------------------------------------
    ID3D12CommandList* Cast() const;

    //-------------------------------------------------------------------------
    //! @brief      現在のバッファ番号を返却します.
    //-------------------------------------------------------------------------
    uint8_t GetIndex() const;

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    RefPtr<ID3D12CommandAllocator>      m_Allocator[2]; //!< アロケータです.
    RefPtr<ID3D12GraphicsCommandList6>  m_CmdList;      //!< コマンドリストです.
    uint8_t                             m_Index;        //!< バッファ番号です.

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

} // namespace asdx
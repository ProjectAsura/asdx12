﻿//-----------------------------------------------------------------------------
// File : asdxByteAddressBuffer.h
// Desc : Byte Address Buffer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxView.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ByteAddressBuffer
///////////////////////////////////////////////////////////////////////////////
class ByteAddressBuffer
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
    ByteAddressBuffer();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~ByteAddressBuffer();

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //! 
    //! @param[in]      size        バッファサイズです
    //! @param[in]      state       リソースステートです
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(uint64_t size, D3D12_RESOURCE_STATES state);

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //! 
    //! @param[in]      size        バッファサイズです
    //! @param[in]      pInitData   初期化データです.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(uint64_t size, const void* pInitData);

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      リソースを取得します.
    //! 
    //! @return     リソースを返却します.
    //-------------------------------------------------------------------------
    ID3D12Resource* GetResource() const;

    //-------------------------------------------------------------------------
    //! @brief      シェーダリソースビューを取得します.
    //! 
    //! @return     シェーダリソースビューを返却します.
    //-------------------------------------------------------------------------
    IShaderResourceView* GetView() const;

private:
    //=========================================================================
    // private variables
    //=========================================================================
    RefPtr<ID3D12Resource>      m_Resource;
    RefPtr<IShaderResourceView> m_View;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

} // namespace asdx
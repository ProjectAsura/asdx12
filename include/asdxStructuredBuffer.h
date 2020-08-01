﻿//-----------------------------------------------------------------------------
// File : asdxStructuredBuffer.h
// Desc : Structured Buffer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxGraphicsDevice.h>
#include <asdxUploadResource.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// StructuredBuffer class
///////////////////////////////////////////////////////////////////////////////
class StructuredBuffer
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
    StructuredBuffer();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~StructuredBuffer();

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      device      グラフィックスデバイスです.
    //! @param[in]      count       配列数です.
    //! @param[in]      stride      構造体のサイズです.
    //! @param[in]      state       リソースステートです.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(GraphicsDevice& device, uint64_t count, uint32_t stride, D3D12_RESOURCE_STATES state);

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      device              グラフィックスデバイスです.
    //! @param[in]      count               配列数です.
    //! @param[in]      stride              構造体のサイズです.
    //! @param[in]      pInitData           初期化データです.
    //! @param[out]     ppUploadResource    アップロードリソースの格納先です.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(
        GraphicsDevice&     device,
        uint64_t            count,
        uint32_t            stride,
        void*               pInitData,
        IUploadResource**   ppUploadResource);

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
    //! @brief      ディスクリプタを取得します.
    //!
    //! @return     ディスクリプタを返却します.
    //-------------------------------------------------------------------------
    const Descriptor* GetDescriptor() const;

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    RefPtr<ID3D12Resource>  m_pResource;        //!< リソースです.
    RefPtr<Descriptor>      m_pDescriptor;      //!< ディスクリプタです.

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

} // namespace asdx
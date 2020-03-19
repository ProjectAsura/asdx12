﻿//-----------------------------------------------------------------------------
// File : asdxConstantBuffer.h
// Desc : Constant Buffer Wrapper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxGraphicsDevice.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ConstantBuffer class
///////////////////////////////////////////////////////////////////////////////
class ConstantBuffer
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
    ConstantBuffer();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~ConstantBuffer();

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      size        バッファサイズです.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(GraphicsDevice& device, uint64_t size);

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      更新処理を行います.
    //!
    //! @param[in]      pSrc        入力データへのポインタ.
    //! @param[in]      size        コピーサイズ.
    //! @param[in]      srcOffset   入力データのオフセット.
    //! @param[in]      dstOffset   書き込み先のオフセット.
    //-------------------------------------------------------------------------
    void Update(const void* pSrc, uint64_t size, uint64_t srcOffset = 0, uint64_t dstOffset = 0);

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

    //-------------------------------------------------------------------------
    //! @brief      バッファを入れ替えます.
    //-------------------------------------------------------------------------
    void SwapBuffer();

private:
    //=========================================================================
    // private varaibles.
    //=========================================================================
    RefPtr<ID3D12Resource>  m_Resource[2];
    uint8_t*                m_pDst[2];
    RefPtr<Descriptor>      m_pDescriptor[2];
    uint8_t                 m_Index;
    uint64_t                m_Size;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};


} // namespace asdx

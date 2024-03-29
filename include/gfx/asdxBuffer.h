﻿//-----------------------------------------------------------------------------
// File : asdxBuffer.h
// Desc : Buffer Wrapper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <d3d12.h>
#include <fnd/asdxRef.h>
#include <gfx/asdxView.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// VertexBuffer class
///////////////////////////////////////////////////////////////////////////////
class VertexBuffer
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
    VertexBuffer();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~VertexBuffer();

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      size        バッファサイズです.
    //! @param[in]      stride      1頂点当たりのサイズです.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(uint64_t size, uint32_t stride);

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
    //! @brief      頂点バッファビューを取得します.
    //!
    //! @return     頂点バッファビューを返却します.
    //-------------------------------------------------------------------------
    D3D12_VERTEX_BUFFER_VIEW GetView() const;

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
    { return static_cast<T*>(Map()); }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    RefPtr<ID3D12Resource>      m_pResource;        //!< リソースです.
    D3D12_VERTEX_BUFFER_VIEW    m_View;             //!< 頂点バッファビューです.

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};


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
    //! @param[in]      size            バッファサイズです.
    //! @param[in]      isShortFormat   16bitフォーマットを使用する場合は trueを，32bitフォーマットを使用する場合はfalseを指定します
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(uint64_t size, bool isShortFormat = false);

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
    { return static_cast<T*>(Map()); }

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
    bool Init(uint64_t size);

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
    //-------------------------------------------------------------------------
    ID3D12Resource* GetResource() const;

    //-------------------------------------------------------------------------
    //! @brief      リソースを取得します.
    //-------------------------------------------------------------------------
    ID3D12Resource* GetResource(uint32_t index) const;

    //-------------------------------------------------------------------------
    //! @brief      定数バッファビューを取得します.
    //-------------------------------------------------------------------------
    IConstantBufferView* GetView() const;

    //-------------------------------------------------------------------------
    //! @brief      定数バッファビューを取得します.
    //-------------------------------------------------------------------------
    IConstantBufferView* GetView(uint32_t index) const;

    //-------------------------------------------------------------------------
    //! @brief      バッファを入れ替えます.
    //-------------------------------------------------------------------------
    void SwapBuffer();

    //-------------------------------------------------------------------------
    //! @brief      メモリマッピングを行います
    //-------------------------------------------------------------------------
    void* Map(uint32_t index);

    //-------------------------------------------------------------------------
    //! @brief      メモリマッピングを解除します.
    //-------------------------------------------------------------------------
    void Unmap(uint32_t index);

    //-------------------------------------------------------------------------
    //! @brief      メモリマッピングを行います
    //-------------------------------------------------------------------------
    inline void* Map()
    { return Map(m_Index); }

    //-------------------------------------------------------------------------
    //! @brief      メモリマッピングを解除します.
    //-------------------------------------------------------------------------
    inline void Unmap()
    { Unmap(m_Index); }

    //-------------------------------------------------------------------------
    //! @brief      メモリマッピングを行います
    //-------------------------------------------------------------------------
    template<typename T>
    inline T* MapAs(uint32_t index)
    { return reinterpret_cast<T*>(Map(index)); }

    //-------------------------------------------------------------------------
    //! @brief      メモリマッピングを行います
    //-------------------------------------------------------------------------
    template<typename T>
    inline T* MapAs()
    { return reinterpret_cast<T*>(Map()); }

private:
    //=========================================================================
    // private varaibles.
    //=========================================================================
    RefPtr<ID3D12Resource>      m_Resource[2];
    RefPtr<IConstantBufferView> m_View[2];
    uint8_t*                    m_Dst[2];
    uint8_t                     m_Index;
    uint64_t                    m_Size;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};


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
    //! @param[in]      pCmdList    コマンドリストです.
    //! @param[in]      size        バッファサイズです
    //! @param[in]      pInitData   初期化データです.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(ID3D12GraphicsCommandList* pCmdList, uint64_t size, const void* pInitData);

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
    //! @param[in]      count       配列数です.
    //! @param[in]      stride      構造体のサイズです.
    //! @param[in]      state       リソースステートです.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(uint64_t count, uint32_t stride, D3D12_RESOURCE_STATES state);

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //!
    //! @param[in]      pCmdList            コマンドリストです.
    //! @param[in]      count               配列数です.
    //! @param[in]      stride              構造体のサイズです.
    //! @param[in]      pInitData           初期化データです.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(ID3D12GraphicsCommandList* pCmdList, uint64_t count, uint32_t stride, const void* pInitData);

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
    // private variables.
    //=========================================================================
    RefPtr<ID3D12Resource>          m_Resource;
    RefPtr<IShaderResourceView>     m_View;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

///////////////////////////////////////////////////////////////////////////////
// BufferUpdater class
///////////////////////////////////////////////////////////////////////////////
class BufferUpdater
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
    //! @brief      初期化処理を行います.
    //! 
    //! @param[in]      pDevice     デバイスです
    //! @param[in]      size        サイズです.
    //! @retval true    初期化に成功.
    //! @retval false   初期化に失敗.
    //-------------------------------------------------------------------------
    bool Init(ID3D12Device* pDevice, uint64_t size);

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      バッファを更新します.
    //! 
    //! @param[in]      pCommandList        コマンドリストです.
    //! @param[in]      pDstResource        更新対象バッファです.
    //! @param[in]      dstOffset           更新対象バッファの先頭からのオフセットです.
    //! @param[in]      pSrcResource        更新データです.
    //! @param[in]      size                更新データサイズです.
    //! @retval true    更新に成功.
    //! @retval false   更新に失敗.
    //-------------------------------------------------------------------------
    bool Update(
        ID3D12GraphicsCommandList*  pCommandList,
        ID3D12Resource*             pDstResource,
        uint64_t                    dstOffset,
        void*                       pSrcResource,
        uint64_t                    size);

    //-------------------------------------------------------------------------
    //! @brief      バッファを入れ替えます.
    //-------------------------------------------------------------------------
    void SwapBuffers();

    //-------------------------------------------------------------------------
    //! @brief      バッファ番号を取得します.
    //! 
    //! @return     バッファ番号を返却します.
    //-------------------------------------------------------------------------
    uint8_t GetIndex() const;

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    RefPtr<ID3D12Resource>  m_Resource[2];          //!< リソースです.
    uint8_t*                m_AddressCPU[2] = {};   //!< CPUアドレスです.
    uint64_t                m_Size          = 0;    //!< バッファサイズです.
    uint64_t                m_Offset        = 0;    //!< バッファオフセットです.
    uint8_t                 m_Index         = 0;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};


} // namespace asdx

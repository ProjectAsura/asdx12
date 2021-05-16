﻿//-----------------------------------------------------------------------------
// File : asdxCommandList.h
// Desc : Command List Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <d3d12.h>
#include <gfx/asdxView.h>


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
    bool Init(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type);

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
    //! @brief      現在のバッファ番号を返却します.
    //-------------------------------------------------------------------------
    uint8_t GetIndex() const;

    //-------------------------------------------------------------------------
    //! @brief      遷移バリアを設定します.
    //-------------------------------------------------------------------------
    void BarrierTransition(
        const IView*                    pView,
        uint32_t                        subresource,
        D3D12_RESOURCE_STATES           stateBefore,
        D3D12_RESOURCE_STATES           stateAfter,
        D3D12_RESOURCE_BARRIER_FLAGS    flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);

    //-------------------------------------------------------------------------
    //! @brief      遷移バリアを設定します.
    //-------------------------------------------------------------------------
    void BarrierTransition(
        ID3D12Resource*                 pResource,
        uint32_t                        subresource,
        D3D12_RESOURCE_STATES           stateBefore,
        D3D12_RESOURCE_STATES           stateAfter,
        D3D12_RESOURCE_BARRIER_FLAGS    flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);

    //-------------------------------------------------------------------------
    //! @brief      UAVバリアを設定します.
    //-------------------------------------------------------------------------
    void BarrierUAV(
        const IUnorderedAccessView*     pView,
        D3D12_RESOURCE_BARRIER_FLAGS    flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);

    //-------------------------------------------------------------------------
    //! @brief      UAVバリアを設定します.
    //-------------------------------------------------------------------------
    void BarrierUAV(
        ID3D12Resource*                 pResource,
        D3D12_RESOURCE_BARRIER_FLAGS    flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);

    //-------------------------------------------------------------------------
    //! @brief      レンダーターゲットビューをクリアします.
    //-------------------------------------------------------------------------
    void ClearRTV(const IRenderTargetView* pView, const float* pClearColor);

    //-------------------------------------------------------------------------
    //! @brief      深度ステンシルビューをクリアします.
    //-------------------------------------------------------------------------
    void ClearDSV(const IDepthStencilView* pView, float clearDepth);

    //-------------------------------------------------------------------------
    //! @brief      深度ステンシルビューをクリアします.
    //-------------------------------------------------------------------------
    void ClearDSV(
        const IDepthStencilView*    pView,
        D3D12_CLEAR_FLAGS           flag,
        float                       clearDepth,
        uint8_t                     clearStencil);

    //-------------------------------------------------------------------------
    //! @brief      アンオーダードアクセスビューをクリアします.
    //-------------------------------------------------------------------------
    void ClearUAV(const IUnorderedAccessView* pView, const uint32_t* pClearValues);

    //-------------------------------------------------------------------------
    //! @brief      アンオーダードアクセスビューをクリアします.
    //-------------------------------------------------------------------------
    void ClearUAV(const IUnorderedAccessView* pView, const float* pClearValues);

    //-------------------------------------------------------------------------
    //! @brief      ビューポートを設定します.
    //-------------------------------------------------------------------------
    void SetViewport(const IView* pView, bool setScissor = true);

    //-------------------------------------------------------------------------
    //! @brief      ビューポートを設定します.
    //-------------------------------------------------------------------------
    void SetViewport(ID3D12Resource* pResource, bool setScissor = true);

    //-------------------------------------------------------------------------
    //! @brief      レンダーターゲットを設定します.
    //-------------------------------------------------------------------------
    void SetTarget(const IRenderTargetView* pRTV, const IDepthStencilView* pDSV);

    //-------------------------------------------------------------------------
    //! @brief      レンダーターゲットを設定します.
    //-------------------------------------------------------------------------
    void SetTargets(uint32_t count, const IRenderTargetView** pRTVs, const IDepthStencilView* pDSV);

    //-------------------------------------------------------------------------
    //! @brief      ルート定数を設定します.
    //-------------------------------------------------------------------------
    void SetConstants(uint32_t index, uint32_t count, const void* data, uint32_t offset, bool compute = false);

    //-------------------------------------------------------------------------
    //! @brief      ディスクリプタテーブルを設定します.
    //-------------------------------------------------------------------------
    void SetTable(uint32_t index, const IView* pView, bool compute = false);

    //-------------------------------------------------------------------------
    //! @brief      定数バッファビューを設定します.
    //-------------------------------------------------------------------------
    void SetCBV(uint32_t index, const IConstantBufferView* pView, bool compute = false);

    //-------------------------------------------------------------------------
    //! @brief      定数バッファビューを設定します.
    //-------------------------------------------------------------------------
    void SetCBV(uint32_t index, ID3D12Resource* pResource, bool compute = false);

    //-------------------------------------------------------------------------
    //! @brief      シェーダリソースビューを設定します.
    //-------------------------------------------------------------------------
    void SetSRV(uint32_t index, const IShaderResourceView* pView, bool compute = false);

    //-------------------------------------------------------------------------
    //! @brief      シェーダリソースビューを設定します.
    //-------------------------------------------------------------------------
    void SetSRV(uint32_t index, ID3D12Resource* pResource, bool compute = false);

    //-------------------------------------------------------------------------
    //! @brief      アンオーダードアクセスビューを設定します.
    //-------------------------------------------------------------------------
    void SetUAV(uint32_t index, const IUnorderedAccessView* pView, bool compute = false);

    //-------------------------------------------------------------------------
    //! @brief      アンオーダードアクセスビューを設定します.
    //-------------------------------------------------------------------------
    void SetUAV(uint32_t index, ID3D12Resource* pResource, bool compute = false);

    //-------------------------------------------------------------------------
    //! @brief      イベント開始します.
    //-------------------------------------------------------------------------
    void BeginEvent(const char* text);

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

///////////////////////////////////////////////////////////////////////////////
// ScopedTransition class
///////////////////////////////////////////////////////////////////////////////
class ScopedTransition
{
public:
    ScopedTransition(
        ID3D12GraphicsCommandList*  pCmd,
        ID3D12Resource*             pResource,
        D3D12_RESOURCE_STATES       before,
        D3D12_RESOURCE_STATES       after,
        uint32_t                    subResource = 0
    );

    ScopedTransition(
        ID3D12GraphicsCommandList*  pCmd,
        const IView*                pView,
        D3D12_RESOURCE_STATES       before,
        D3D12_RESOURCE_STATES       after,
        uint32_t                    subResource = 0
    );

    ~ScopedTransition();

private:
    ID3D12GraphicsCommandList* m_pCmd;
    ID3D12Resource*            m_pResource;
    D3D12_RESOURCE_STATES      m_StateBefore;
    D3D12_RESOURCE_STATES      m_StateAfter;
    uint32_t                   m_SubResource;
};

///////////////////////////////////////////////////////////////////////////////
// ScopedMarker class
///////////////////////////////////////////////////////////////////////////////
class ScopedMarker
{
public:
    ScopedMarker(ID3D12GraphicsCommandList* pCmd, const char* text);
    ~ScopedMarker();

private:
    ID3D12GraphicsCommandList*  m_pCmd;
};

} // namespace asdx

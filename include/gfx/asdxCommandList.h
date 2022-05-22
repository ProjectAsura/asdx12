//-----------------------------------------------------------------------------
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

struct ResTexture;
class Sampler;

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
    void Reset();

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
    //! @brief      ディスクリプタテーブルを設定します.
    //-------------------------------------------------------------------------
    void SetTable(uint32_t index, const Sampler* sampler, bool compute = false);

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
    //! @brief      リソースを更新します.
    //! 
    //! @param[in]      pDstResource        アップロード先リソース.
    //! @param[in]      subResourceCount    アップロードするサブリソース数.
    //! @param[in]      subResourceOffset   開始サブリソース番号.
    //! @param[in]      subResources        アップロードするサブリソースの配列.
    //-------------------------------------------------------------------------
    void UpdateSubresources(
        ID3D12Resource*                 pDstResource,
        uint32_t                        subResourceCount,
        uint32_t                        subResourceOffset,
        const D3D12_SUBRESOURCE_DATA*   subResources);

    //-------------------------------------------------------------------------
    //! @brief      バッファを更新します.
    //-------------------------------------------------------------------------
    void UpdateBuffer(
        ID3D12Resource*     pDstResource,
        const void*         pSrcResource);

    //-------------------------------------------------------------------------
    //! @brief      テクスチャを更新します.
    //-------------------------------------------------------------------------
    void UpdateTexture(
        ID3D12Resource*     pDstResource,
        const ResTexture*   pSrcResource);

    //-------------------------------------------------------------------------
    //! @brief      イベントを開始します.
    //-------------------------------------------------------------------------
    void BeginEvent(const char* text);

    //-------------------------------------------------------------------------
    //! @brief      イベントを終了します.
    //-------------------------------------------------------------------------
    inline void EndEvent()
    { m_CmdList->EndEvent(); }

    //-------------------------------------------------------------------------
    //! @brief      クエリを開始します.
    //-------------------------------------------------------------------------
    inline void BeginQuery(ID3D12QueryHeap* pHeap, D3D12_QUERY_TYPE type, uint32_t index)
    { m_CmdList->BeginQuery(pHeap, type, index); }

    //-------------------------------------------------------------------------
    //! @brief      クエリを終了します.
    //-------------------------------------------------------------------------
    inline void EndQuery(ID3D12QueryHeap* pHeap, D3D12_QUERY_TYPE type, uint32_t index)
    { m_CmdList->EndQuery(pHeap, type, index); }

    //-------------------------------------------------------------------------
    //! @brief      タイムスタンプを設定します.
    //-------------------------------------------------------------------------
    inline void QueryTimeStamp(ID3D12QueryHeap* pHeap, uint32_t index)
    { m_CmdList->EndQuery(pHeap, D3D12_QUERY_TYPE_TIMESTAMP, index); }

    //-------------------------------------------------------------------------
    //! @brief      ルートシグニチャを設定します.
    //-------------------------------------------------------------------------
    void SetRootSignature(ID3D12RootSignature* pRootSignature, bool compute);

    //-------------------------------------------------------------------------
    //! @brief      パイプラインステートを設定します.
    //-------------------------------------------------------------------------
    inline void SetPipelineState(ID3D12PipelineState* value) 
    { m_CmdList->SetPipelineState(value); }

    //-------------------------------------------------------------------------
    //! @brief      ステートオブジェクトを設定します.
    //-------------------------------------------------------------------------
    inline void SetStateObject(ID3D12StateObject* value)
    { m_CmdList->SetPipelineState1(value); }

    //-------------------------------------------------------------------------
    //! @brief      プリミティブトポロジーを設定します.
    //-------------------------------------------------------------------------
    inline void SetPrimitiveToplogy(D3D12_PRIMITIVE_TOPOLOGY value)
    { m_CmdList->IASetPrimitiveTopology(value); }

    //-------------------------------------------------------------------------
    //! @brief      インデックスバッファを設定します.
    //-------------------------------------------------------------------------
    inline void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* pView)
    { m_CmdList->IASetIndexBuffer(pView); }

    //-------------------------------------------------------------------------
    //! @brief      頂点バッファを設定します.
    //-------------------------------------------------------------------------
    inline void SetVertexBuffers(uint32_t startSlot, uint32_t viewCount, const D3D12_VERTEX_BUFFER_VIEW* pViews)
    { m_CmdList->IASetVertexBuffers(startSlot, viewCount, pViews); }

    //-------------------------------------------------------------------------
    //! @brief      コンピュートシェーダを実行します.
    //-------------------------------------------------------------------------
    inline void Dispatch(uint32_t x, uint32_t y, uint32_t z)
    { m_CmdList->Dispatch(x, y, z); }

    //-------------------------------------------------------------------------
    //! @brief      メッシュシェーダを実行します.
    //-------------------------------------------------------------------------
    inline void DispatchMesh(uint32_t x)
    { m_CmdList->DispatchMesh(x, 1, 1); }

    //-------------------------------------------------------------------------
    //! @brief      レイトレーシングパイプラインを実行します.
    //-------------------------------------------------------------------------
    inline void DispatchRays(const D3D12_DISPATCH_RAYS_DESC* pDesc)
    { m_CmdList->DispatchRays(pDesc); }

    //-------------------------------------------------------------------------
    //! @brief      描画します.
    //-------------------------------------------------------------------------
    inline void DrawInstanced
    (
        uint32_t vertexCountPerInstance,
        uint32_t instanceCount,
        uint32_t startVertexLocation,
        uint32_t startInstanceLocation
    )
    {
        m_CmdList->DrawInstanced(
            vertexCountPerInstance,
            instanceCount,
            startVertexLocation,
            startInstanceLocation);
    }

    //-------------------------------------------------------------------------
    //! @brief      インデックス付きで描画します.
    //-------------------------------------------------------------------------
    inline void DrawIndexedInstanced
    (
        uint32_t indexCountPerInstance,
        uint32_t instanceCount,
        uint32_t startIndexLocation,
        int32_t  baseVertexLocation,
        uint32_t startInstanceLocation
    )
    {
        m_CmdList->DrawIndexedInstanced(
            indexCountPerInstance,
            instanceCount,
            startIndexLocation,
            baseVertexLocation,
            startInstanceLocation);
    }

    //-------------------------------------------------------------------------
    //! @brief      インダイレクトコマンドを実行します.
    //-------------------------------------------------------------------------
    inline void ExecuteIndirect
    (
        ID3D12CommandSignature* pCmdSignature,
        uint32_t                maxCmdCount,
        ID3D12Resource*         pArgBuffer,
        uint64_t                argBufferOffset,
        ID3D12Resource*         pCounterBuffer,
        uint64_t                counterBufferOffset
    )
    {
        m_CmdList->ExecuteIndirect(
            pCmdSignature,
            maxCmdCount,
            pArgBuffer,
            argBufferOffset,
            pCounterBuffer,
            counterBufferOffset);
    }

    //-------------------------------------------------------------------------
    //! @brief      領域を指定してバッファをコピーします.
    //-------------------------------------------------------------------------
    inline void CopyBufferRegion
    (
        ID3D12Resource* pDstBuffer,
        UINT64          dstOffset,
        ID3D12Resource* pSrcBuffer,
        UINT64          srcOffset,
        UINT64          numBytes
    )
    {
        m_CmdList->CopyBufferRegion(
            pDstBuffer,
            dstOffset,
            pSrcBuffer,
            srcOffset,
            numBytes);
    }

    //-------------------------------------------------------------------------
    //! @brief      領域を指定してテクスチャをコピーします.
    //-------------------------------------------------------------------------
    inline void CopyTextureRegion
    (
        const D3D12_TEXTURE_COPY_LOCATION*  pDst,
        UINT                                dstX,
        UINT                                dstY,
        UINT                                dstZ,
        const D3D12_TEXTURE_COPY_LOCATION*  pSrc,
        const D3D12_BOX*                    pSrcBox
    )
    {
        m_CmdList->CopyTextureRegion(
            pDst, dstX, dstY, dstZ, pSrc, pSrcBox);
    }

    //-------------------------------------------------------------------------
    //! @brief      リソースをコピーします.
    //-------------------------------------------------------------------------
    inline void CopyResource
    (
        ID3D12Resource* pDstResource,
        ID3D12Resource* pSrcResource
    )
    { m_CmdList->CopyResource(pDstResource, pSrcResource); }

    //-------------------------------------------------------------------------
    //! @brief      高速化機構をコピーします.
    //-------------------------------------------------------------------------
    inline void CopyRaytracingAS
    (
        D3D12_GPU_VIRTUAL_ADDRESS                           dstAS,
        D3D12_GPU_VIRTUAL_ADDRESS                           srcAS,
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE   mode
    )
    {
        m_CmdList->CopyRaytracingAccelerationStructure(
            dstAS, srcAS, mode);
    }

    //-------------------------------------------------------------------------
    //! @brief      サブリソースを解決します.
    //-------------------------------------------------------------------------
    inline void ResolveSubsource
    (
        ID3D12Resource* pDstResource,
        UINT            dstSubresource,
        ID3D12Resource* pSrcResource,
        UINT            srcSubresource,
        DXGI_FORMAT     format
    )
    {
        m_CmdList->ResolveSubresource(
            pDstResource,
            dstSubresource,
            pSrcResource,
            srcSubresource,
            format);
    }

    //-------------------------------------------------------------------------
    //! @brief      高速化機構をビルドします.
    //-------------------------------------------------------------------------
    inline void BuildRaytracingAS
    (
        const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC*           pDesc,
        UINT                                                                postBuildInfoDescCount,
        const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC*  pPostBuildInfoDescs
    )
    {
        m_CmdList->BuildRaytracingAccelerationStructure(
            pDesc, postBuildInfoDescCount, pPostBuildInfoDescs);
    }

    //-------------------------------------------------------------------------
    //! @brief      マーカーをプッシュします.
    //-------------------------------------------------------------------------
    inline void PushMarker(const char* name)
    {
        constexpr UINT PIX_EVENT_ANSI_VERSION = 1;
        auto size = static_cast<UINT>((strlen(name) + 1) * sizeof(name[0]));
        m_CmdList->BeginEvent(PIX_EVENT_ANSI_VERSION, name, size);
    }

    //-------------------------------------------------------------------------
    //! @brief      マーカーをポップします.
    //-------------------------------------------------------------------------
    inline void PopMarker()
    { m_CmdList->EndEvent(); }

    //-------------------------------------------------------------------------
    //! @brief      コマンドの記録を終了します.
    //-------------------------------------------------------------------------
    inline void Close()
    { m_CmdList->Close(); }

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
// ScopedMarker class
///////////////////////////////////////////////////////////////////////////////
class ScopedMarker
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
    ScopedMarker(ID3D12GraphicsCommandList* pCmd, const char* text);
    ~ScopedMarker();

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    ID3D12GraphicsCommandList*  m_pCmd = nullptr;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

///////////////////////////////////////////////////////////////////////////////
// ScopedBarrir class
///////////////////////////////////////////////////////////////////////////////
class ScopedBarrier
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
    ScopedBarrier
    (
        ID3D12GraphicsCommandList*  pCmd,
        ID3D12Resource*             pResource,
        D3D12_RESOURCE_STATES       beforeState,
        D3D12_RESOURCE_STATES       afterState
    )
    {
        m_pCmd          = pCmd;
        m_pResource     = pResource;
        m_BeforeState   = beforeState;
        m_AfterState    = afterState;

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags                   = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource    = m_pResource;
        barrier.Transition.Subresource  = 0;
        barrier.Transition.StateBefore  = m_BeforeState;
        barrier.Transition.StateAfter   = m_AfterState;

        m_pCmd->ResourceBarrier(1, &barrier);
    }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~ScopedBarrier()
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags                   = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource    = m_pResource;
        barrier.Transition.Subresource  = 0;
        barrier.Transition.StateBefore  = m_AfterState;
        barrier.Transition.StateAfter   = m_BeforeState;

        m_pCmd->ResourceBarrier(1, &barrier);
    }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    ID3D12GraphicsCommandList*  m_pCmd          = nullptr;
    ID3D12Resource*             m_pResource     = nullptr;
    D3D12_RESOURCE_STATES       m_BeforeState;
    D3D12_RESOURCE_STATES       m_AfterState;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};


///////////////////////////////////////////////////////////////////////////////
// BarrierHelper class
///////////////////////////////////////////////////////////////////////////////
class BarrierHelper
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
    //! @brief      初期設定を行います.
    //-------------------------------------------------------------------------
    void Setup(ID3D12Resource* pResource, D3D12_RESOURCE_STATES states)
    {
        m_pResource = nullptr;
        m_State     = states;
    }

    //-------------------------------------------------------------------------
    //! @brief      次のステートへの遷移バリアを取得します.
    //-------------------------------------------------------------------------
    D3D12_RESOURCE_BARRIER Next(D3D12_RESOURCE_STATES nextState, UINT subResource = 0)
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags                   = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource    = m_pResource;
        barrier.Transition.Subresource  = subResource;
        barrier.Transition.StateBefore  = m_State;
        barrier.Transition.StateAfter   = nextState;

        m_State = nextState;

        return barrier;
    }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    ID3D12Resource*         m_pResource = nullptr;
    D3D12_RESOURCE_STATES   m_State     = D3D12_RESOURCE_STATE_COMMON;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

} // namespace asdx

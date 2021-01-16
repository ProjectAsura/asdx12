﻿//-----------------------------------------------------------------------------
// File : asdxCmdHelper.cpp
// Desc : Command Helper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <asdxCmdHelper.h>


namespace asdx {

//-----------------------------------------------------------------------------
//      遷移バリアを発行します.
//-----------------------------------------------------------------------------
void BarrierTransition
(
    ID3D12GraphicsCommandList*      pCmd,
    IView*                          pView,
    uint32_t                        subresource,
    D3D12_RESOURCE_STATES           stateBefore,
    D3D12_RESOURCE_STATES           stateAfter,
    D3D12_RESOURCE_BARRIER_FLAGS    flags
)
{
    assert(pView != nullptr);
    BarrierTransition(
        pCmd,
        pView->GetResource(),
        subresource,
        stateBefore,
        stateAfter,
        flags);
}

//-----------------------------------------------------------------------------
//      遷移バリアを発行します.
//-----------------------------------------------------------------------------
void BarrierTransition
(
    ID3D12GraphicsCommandList*      pCmd,
    ID3D12Resource*                 pResource,
    uint32_t                        subresource,
    D3D12_RESOURCE_STATES           stateBefore,
    D3D12_RESOURCE_STATES           stateAfter,
    D3D12_RESOURCE_BARRIER_FLAGS    flags
)
{
    assert(pCmd != nullptr);
    assert(pResource != nullptr);

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                   = flags;
    barrier.Transition.pResource    = pResource;
    barrier.Transition.Subresource  = subresource;
    barrier.Transition.StateBefore  = stateBefore;
    barrier.Transition.StateAfter   = stateAfter;

    pCmd->ResourceBarrier(1, &barrier);
}

//-----------------------------------------------------------------------------
//      UAVバリアを発行します.
//-----------------------------------------------------------------------------
void BarrierUAV
(
    ID3D12GraphicsCommandList*      pCmd,
    IUnorderedAccessView*           pView,
    D3D12_RESOURCE_BARRIER_FLAGS    flags
)
{
    assert(pView != nullptr);
    BarrierUAV(pCmd, pView->GetResource(), flags);
}

//-----------------------------------------------------------------------------
//      UAVバリアを発行します.
//-----------------------------------------------------------------------------
void BarrierUAV
(
    ID3D12GraphicsCommandList*      pCmd,
    ID3D12Resource*                 pResource,
    D3D12_RESOURCE_BARRIER_FLAGS    flags
)
{
    assert(pCmd != nullptr);
    assert(pResource != nullptr);

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type            = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.Flags           = flags;
    barrier.UAV.pResource   = pResource;

    pCmd->ResourceBarrier(1, &barrier);
}

//-----------------------------------------------------------------------------
//      レンダーターゲットビューをクリアします.
//-----------------------------------------------------------------------------
void ClearRTV
(
    ID3D12GraphicsCommandList*  pCmd,
    const IRenderTargetView*    pView,
    const float*                pClearColor)
{
    assert(pCmd        != nullptr);
    assert(pView       != nullptr);
    assert(pClearColor != nullptr);

    auto handle = pView->GetHandleCPU();
    pCmd->ClearRenderTargetView(handle, pClearColor, 0, nullptr);
}

//-----------------------------------------------------------------------------
//      深度ステンシルビューをクリアします.
//-----------------------------------------------------------------------------
void ClearDSV
(
    ID3D12GraphicsCommandList*  pCmd,
    const IDepthStencilView*    pView,
    D3D12_CLEAR_FLAGS           flags,
    float                       clearDepth,
    uint8_t                     clearStencil
)
{
    assert(pCmd  != nullptr);
    assert(pView != nullptr);

    auto handle = pView->GetHandleCPU();
    pCmd->ClearDepthStencilView(handle, flags, clearDepth, clearStencil, 0, nullptr);
}

//-----------------------------------------------------------------------------
//      アンオーダードアクセスビューをクリアします.
//-----------------------------------------------------------------------------
void ClearUAV
(
    ID3D12GraphicsCommandList*  pCmd,
    const IUnorderedAccessView* pView,
    const uint32_t*             pClearValues
)
{
    assert(pCmd         != nullptr);
    assert(pView        != nullptr);
    assert(pClearValues != nullptr);

    pCmd->ClearUnorderedAccessViewUint(
        pView->GetHandleGPU(),
        pView->GetHandleCPU(),
        pView->GetResource(),
        pClearValues,
        0,
        nullptr);
}

//-----------------------------------------------------------------------------
//      アンオーダードアクセスビューをクリアします.
//-----------------------------------------------------------------------------
void ClearUAV
(
    ID3D12GraphicsCommandList*  pCmd,
    const IUnorderedAccessView* pView,
    const float*                pClearValues
)
{
    assert(pCmd         != nullptr);
    assert(pView        != nullptr);
    assert(pClearValues != nullptr);

    pCmd->ClearUnorderedAccessViewFloat(
        pView->GetHandleGPU(),
        pView->GetHandleCPU(),
        pView->GetResource(),
        pClearValues,
        0,
        nullptr);
}

//-----------------------------------------------------------------------------
//      ビューポートを設定します.
//-----------------------------------------------------------------------------
void SetViewport
(
    ID3D12GraphicsCommandList*  pCmd,
    const IView*                pView,
    bool                        setScissor
)
{
    SetViewport(pCmd, pView->GetResource(), setScissor);
}

//-----------------------------------------------------------------------------
//      ビューポートを設定します.
//-----------------------------------------------------------------------------
void SetViewport
(
    ID3D12GraphicsCommandList*  pCmd,
    ID3D12Resource*             pResource,
    bool                        setScissor
)
{
    auto desc = pResource->GetDesc();

    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width    = float(desc.Width);
    viewport.Height   = float(desc.Height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    pCmd->RSSetViewports(1, &viewport);

    if (setScissor == false)
    { return; }

    D3D12_RECT scissor = {};
    scissor.left    = 0;
    scissor.right   = LONG(desc.Width);
    scissor.top     = 0;
    scissor.bottom  = LONG(desc.Height);

    pCmd->RSSetScissorRects(1, &scissor);
}

//-----------------------------------------------------------------------------
//      レンダーターゲットを設定します.
//-----------------------------------------------------------------------------
void SetRenderTarget
(
    ID3D12GraphicsCommandList*  pCmd,
    const IRenderTargetView*    pRTV,
    const IDepthStencilView*    pDSV
)
{
    assert(pCmd != nullptr);
    assert(pRTV != nullptr);
    assert(pDSV != nullptr);

    auto handleRTV = pRTV->GetHandleCPU();
    auto handleDSV = pDSV->GetHandleCPU();
    pCmd->OMSetRenderTargets(1, &handleRTV, FALSE, &handleDSV);
}

//-----------------------------------------------------------------------------
//      レンダーターゲットを設定します.
//-----------------------------------------------------------------------------
void SetRenderTargets
(
    ID3D12GraphicsCommandList*  pCmd,
    uint32_t                    count,
    const IRenderTargetView**   pRTVs,
    const IDepthStencilView*    pDSV
)
{
    assert(pCmd  != nullptr);
    assert(pRTVs != nullptr);
    assert(pDSV  != nullptr);

    D3D12_CPU_DESCRIPTOR_HANDLE handleRTVs[8] = {};
    for(auto i=0u; i<count; ++i)
    {
        assert(pRTVs[i] != nullptr);
        handleRTVs[i] = pRTVs[i]->GetHandleCPU();
    }

    auto handleDSV = pDSV->GetHandleCPU();
    pCmd->OMSetRenderTargets(count, handleRTVs, FALSE, &handleDSV);
}

//-----------------------------------------------------------------------------
//      ディスクリプタテーブルを設定します.
//-----------------------------------------------------------------------------
void SetTable
(
    ID3D12GraphicsCommandList*  pCmd,
    uint32_t                    index,
    const IView*                pView,
    bool                        compute
)
{
    if (pView == nullptr || index == UINT32_MAX)
    { return; }

    if (compute)
    { pCmd->SetComputeRootDescriptorTable(index, pView->GetHandleGPU()); }
    else
    { pCmd->SetGraphicsRootDescriptorTable(index, pView->GetHandleGPU()); }
}

//-----------------------------------------------------------------------------
//      定数バッファビューを設定します.
//-----------------------------------------------------------------------------
void SetCBV
(
    ID3D12GraphicsCommandList*  pCmd,
    uint32_t                    index,
    IConstantBufferView*        view,
    bool                        compute
)
{
    if (view == nullptr)
    { return; }

    SetCBV(pCmd, index, view->GetResource(), compute);
}

//-----------------------------------------------------------------------------
//      定数バッファビューを設定します.
//-----------------------------------------------------------------------------
void SetCBV
(
    ID3D12GraphicsCommandList*  pCmd,
    uint32_t                    index,
    ID3D12Resource*             resource,
    bool                        compute
)
{
    if (resource == nullptr)
    { return; }

    auto addr = resource->GetGPUVirtualAddress();

    if (compute)
    { pCmd->SetComputeRootConstantBufferView(index, addr); }
    else
    { pCmd->SetGraphicsRootConstantBufferView(index, addr); }
}

//-----------------------------------------------------------------------------
//      シェーダリソースビューを設定します.
//-----------------------------------------------------------------------------
void SetSRV
(
    ID3D12GraphicsCommandList*  pCmd,
    uint32_t                    index,
    IShaderResourceView*        view,
    bool                        compute
)
{
    if (view == nullptr)
    { return; }

    SetSRV(pCmd, index, view->GetResource(), compute);
}

//-----------------------------------------------------------------------------
//      シェーダリソースビューを設定します.
//-----------------------------------------------------------------------------
void SetSRV
(
    ID3D12GraphicsCommandList*  pCmd,
    uint32_t                    index,
    ID3D12Resource*             resource,
    bool                        compute
)
{
    if (resource == nullptr || index == UINT32_MAX)
    { return; }

    auto addr = resource->GetGPUVirtualAddress();

    if (compute)
    { pCmd->SetComputeRootShaderResourceView(index, addr); }
    else
    { pCmd->SetGraphicsRootShaderResourceView(index, addr); }
}

//-----------------------------------------------------------------------------
//      アンオーダードアクセスビューを設定します.
//-----------------------------------------------------------------------------
void SetUAV
(
    ID3D12GraphicsCommandList*  pCmd,
    uint32_t                    index,
    IUnorderedAccessView*       view,
    bool                        compute
)
{
    if (view == nullptr)
    { return; }

    SetUAV(pCmd, index, view->GetResource(), compute);
}

//-----------------------------------------------------------------------------
//      アンオーダードアクセスビューを設定します.
//-----------------------------------------------------------------------------
void SetUAV
(
    ID3D12GraphicsCommandList*  pCmd,
    uint32_t                    index,
    ID3D12Resource*             resource,
    bool                        compute
)
{
    if (resource == 0 || index == UINT32_MAX)
    { return; }

    auto addr = resource->GetGPUVirtualAddress();

    if (compute)
    { pCmd->SetComputeRootUnorderedAccessView(index, addr); }
    else
    { pCmd->SetGraphicsRootUnorderedAccessView(index, addr); }
}

//-----------------------------------------------------------------------------
//      32bit定数を設定します.
//-----------------------------------------------------------------------------
void SetConstant
(
    ID3D12GraphicsCommandList*  pCmd,
    uint32_t                    index,
    uint32_t                    data,
    uint32_t                    offset,
    bool                        compute
)
{
    if (index == UINT32_MAX)
    { return; }

    if (compute)
    { pCmd->SetComputeRoot32BitConstant(index, data, offset); }
    else
    { pCmd->SetGraphicsRoot32BitConstant(index, data, offset); }
}

//-----------------------------------------------------------------------------
//      32bit定数を設定します.
//-----------------------------------------------------------------------------
void SetConstants
(
    ID3D12GraphicsCommandList*  pCmd,
    uint32_t                    index,
    uint32_t                    count,
    const void*                 data,
    uint32_t                    offset,
    bool                        compute
)
{
    if (index == UINT32_MAX || count == 0 || data == nullptr)
    { return; }

    if (compute)
    { pCmd->SetComputeRoot32BitConstants(index, count, data, offset); }
    else
    { pCmd->SetGraphicsRoot32BitConstants(index, count, data, offset); }
}

//-----------------------------------------------------------------------------
//      イベントを開始します.
//-----------------------------------------------------------------------------
void BeginEvent
(
    ID3D12GraphicsCommandList* pCmd,
    const char*                text
)
{
    assert(text != nullptr);
    static const UINT PIX_EVENT_ANSI_VERSION = 1;
    auto size = UINT((strlen(text) + 1) * sizeof(char));
    pCmd->BeginEvent(PIX_EVENT_ANSI_VERSION, text, size);
}

//-----------------------------------------------------------------------------
//      イベントを終了します.
//-----------------------------------------------------------------------------
void EndEvent(ID3D12GraphicsCommandList* pCmd)
{ pCmd->EndEvent(); }

} // namespace asdx

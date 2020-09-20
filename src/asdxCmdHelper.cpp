//-----------------------------------------------------------------------------
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
    const Descriptor*           pDescriptor,
    const float*                pClearColor)
{
    assert(pCmd != nullptr);
    assert(pDescriptor != nullptr);
    assert(pClearColor != nullptr);

    auto handle = pDescriptor->GetHandleCPU();
    pCmd->ClearRenderTargetView(handle, pClearColor, 0, nullptr);
}

//-----------------------------------------------------------------------------
//      深度ステンシルビューをクリアします.
//-----------------------------------------------------------------------------
void ClearDSV
(
    ID3D12GraphicsCommandList*  pCmd,
    const Descriptor*           pDescriptor,
    D3D12_CLEAR_FLAGS           flags,
    float                       clearDepth,
    uint8_t                     clearStencil
)
{
    assert(pCmd != nullptr);
    assert(pDescriptor != nullptr);

    auto handle = pDescriptor->GetHandleCPU();
    pCmd->ClearDepthStencilView(handle, flags, clearDepth, clearStencil, 0, nullptr);
}

//-----------------------------------------------------------------------------
//      アンオーダードアクセスビューをクリアします.
//-----------------------------------------------------------------------------
void ClearUAV
(
    ID3D12GraphicsCommandList*  pCmd,
    const Descriptor*           pDescriptor,
    ID3D12Resource*             pResource,
    const uint32_t*             pClearValues
)
{
    assert(pCmd         != nullptr);
    assert(pDescriptor  != nullptr);
    assert(pResource    != nullptr);
    assert(pClearValues != nullptr);

    pCmd->ClearUnorderedAccessViewUint(
        pDescriptor->GetHandleGPU(),
        pDescriptor->GetHandleCPU(),
        pResource,
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
    const Descriptor*           pDescriptor,
    ID3D12Resource*             pResource,
    const float*                pClearValues
)
{
    assert(pCmd         != nullptr);
    assert(pDescriptor  != nullptr);
    assert(pResource    != nullptr);
    assert(pClearValues != nullptr);

    pCmd->ClearUnorderedAccessViewFloat(
        pDescriptor->GetHandleGPU(),
        pDescriptor->GetHandleCPU(),
        pResource,
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
    ID3D12Resource*             pResource,
    bool                        setScissor
)
{
    auto desc = pResource->GetDesc();

    D3D12_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width    = float(desc.Width);
    viewport.Height   = float(desc.Height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    pCmd->RSSetViewports(1, &viewport);

    if (setScissor == false)
    { return; }

    D3D12_RECT scissor;
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
    const Descriptor*           pRTV,
    const Descriptor*           pDSV
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
//      ディスクリプタテーブルを設定します.
//-----------------------------------------------------------------------------
void SetDescriptorTable
(
    ID3D12GraphicsCommandList*  pCmd,
    bool                        compute,
    uint32_t                    index,
    const Descriptor*           pDescriptor
)
{
    if (pDescriptor == nullptr || index == UINT32_MAX)
    { return; }

    if (compute)
    { pCmd->SetComputeRootDescriptorTable(index, pDescriptor->GetHandleGPU()); }
    else
    { pCmd->SetGraphicsRootDescriptorTable(index, pDescriptor->GetHandleGPU()); }
}

//-----------------------------------------------------------------------------
//      定数バッファビューを設定します.
//-----------------------------------------------------------------------------
void SetCBV
(
    ID3D12GraphicsCommandList*  pCmd,
    bool                        compute,
    uint32_t                    index,
    D3D12_GPU_VIRTUAL_ADDRESS   addr
)
{
    if (addr == 0 || index == UINT32_MAX)
    { return; }

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
    bool                        compute,
    uint32_t                    index,
    D3D12_GPU_VIRTUAL_ADDRESS   addr
)
{
    if (addr == 0 || index == UINT32_MAX)
    { return; }

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
    bool                        compute,
    uint32_t                    index,
    D3D12_GPU_VIRTUAL_ADDRESS   addr
)
{
    if (addr == 0 || index == UINT32_MAX)
    { return; }

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
    bool                        compute,
    uint32_t                    index,
    uint32_t                    data,
    uint32_t                    offset
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
    bool                        compute,
    uint32_t                    index,
    uint32_t                    count,
    const void*                 data,
    uint32_t                    offset
)
{
    if (index == UINT32_MAX || count == 0 || data == nullptr)
    { return; }

    if (compute)
    { pCmd->SetComputeRoot32BitConstants(index, count, data, offset); }
    else
    { pCmd->SetGraphicsRoot32BitConstants(index, count, data, offset); }
}

} // namespace asdx
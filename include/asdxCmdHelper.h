//-----------------------------------------------------------------------------
// File : asdxCmdHelper.h
// Desc : Command Helper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxView.h>
#include <pix_win.h>


namespace asdx {

void BarrierTransition(
    ID3D12GraphicsCommandList*      pCmd,
    ID3D12Resource*                 pResource,
    uint32_t                        subresource,
    D3D12_RESOURCE_STATES           stateBefore,
    D3D12_RESOURCE_STATES           stateAfter,
    D3D12_RESOURCE_BARRIER_FLAGS    flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);

void BarrierUAV(
    ID3D12GraphicsCommandList*      pCmd,
    ID3D12Resource*                 pResource,
    D3D12_RESOURCE_BARRIER_FLAGS    flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);

void ClearRTV(
    ID3D12GraphicsCommandList*      pCmd,
    const IRenderTargetView*        pView,
    const float*                    pClearColor);

void ClearDSV(
    ID3D12GraphicsCommandList*      pCmd,
    const IDepthStencilView*        pView,
    D3D12_CLEAR_FLAGS               flag,
    float                           clearDepth,
    uint8_t                         clearStencil);

void ClearUAV(
    ID3D12GraphicsCommandList*      pCmd,
    const IUnorderedAccessView*     pView,
    ID3D12Resource*                 pResource,
    const uint32_t*                 pClearValues);

void ClearUAV(
    ID3D12GraphicsCommandList*      pCmd,
    const IUnorderedAccessView*     pView,
    ID3D12Resource*                 pResource,
    const float*                    pClearValues);

void SetViewport(
    ID3D12GraphicsCommandList*      pCmd,
    ID3D12Resource*                 pResource,
    bool                            setScissor = true);

void SetRenderTarget(
    ID3D12GraphicsCommandList*      pCmd,
    const IRenderTargetView*        pRTV,
    const IDepthStencilView*        pDSV);

void SetDescriptorTable(
    ID3D12GraphicsCommandList*      pCmd,
    bool                            compute,
    uint32_t                        index,
    const IView*                    pView);

void SetCBV(
    ID3D12GraphicsCommandList*      pCmd,
    bool                            compute,
    uint32_t                        index,
    D3D12_GPU_VIRTUAL_ADDRESS       addr);

void SetSRV(
    ID3D12GraphicsCommandList*      pCmd,
    bool                            compute,
    uint32_t                        index,
    D3D12_GPU_VIRTUAL_ADDRESS       addr);

void SetUAV(
    ID3D12GraphicsCommandList*      pCmd,
    bool                            compute,
    uint32_t                        index,
    D3D12_GPU_VIRTUAL_ADDRESS       addr);

void SetConstant(
    ID3D12GraphicsCommandList*      pCmd,
    bool                            compute,
    uint32_t                        index,
    uint32_t                        data,
    uint32_t                        offset);

void SetConstants(
    ID3D12GraphicsCommandList*      pCmd,
    bool                            compute,
    uint32_t                        index,
    uint32_t                        count,
    const void*                     data,
    uint32_t                        offset);

inline uint32_t DivRoundUp(uint32_t count, uint32_t div)
{ return (count + div - 1) / div; }

} // namespace asdx

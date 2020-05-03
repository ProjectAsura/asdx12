//-----------------------------------------------------------------------------
// File : asdxCmdHelper.h
// Desc : Command Helper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxDescriptor.h>
#include <pix3.h>


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
    const Descriptor*               pDescriptor,
    const float*                    pClearColor);

void ClearDSV(
    ID3D12GraphicsCommandList*      pCmd,
    const Descriptor*               pDescriptor,
    D3D12_CLEAR_FLAGS               flag,
    float                           clearDepth,
    uint8_t                         clearStencil);

void ClearUAV(
    ID3D12GraphicsCommandList*      pCmd,
    const Descriptor*               pDescriptor,
    ID3D12Resource*                 pResource,
    const uint32_t*                 pClearValues);

void ClearUAV(
    ID3D12GraphicsCommandList*      pCmd,
    const Descriptor*               pDescriptor,
    ID3D12Resource*                 pResource,
    const float*                    pClearValues);

void SetViewport(
    ID3D12GraphicsCommandList*      pCmd,
    ID3D12Resource*                 pResource,
    bool                            setScissor = true);

void SetRenderTarget(
    ID3D12GraphicsCommandList*      pCmd,
    const Descriptor*               pRTV,
    const Descriptor*               pDSV);


} // namespace asdx

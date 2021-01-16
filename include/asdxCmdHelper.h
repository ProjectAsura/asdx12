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


namespace asdx {

constexpr D3D12_RESOURCE_STATES STATE_COMMON                = D3D12_RESOURCE_STATE_COMMON;
constexpr D3D12_RESOURCE_STATES STATE_VBV_CBV               = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
constexpr D3D12_RESOURCE_STATES STATE_RTV                   = D3D12_RESOURCE_STATE_RENDER_TARGET;
constexpr D3D12_RESOURCE_STATES STATE_UAV                   = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
constexpr D3D12_RESOURCE_STATES STATE_DEPTH_WRITE           = D3D12_RESOURCE_STATE_DEPTH_WRITE;
constexpr D3D12_RESOURCE_STATES STATE_DEPTH_READ            = D3D12_RESOURCE_STATE_DEPTH_READ;
constexpr D3D12_RESOURCE_STATES STATE_SRV                   = (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
constexpr D3D12_RESOURCE_STATES STATE_INDIRECT_ARG          = D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
constexpr D3D12_RESOURCE_STATES STATE_COPY_DST              = D3D12_RESOURCE_STATE_COPY_DEST;
constexpr D3D12_RESOURCE_STATES STATE_COPY_SRC              = D3D12_RESOURCE_STATE_COPY_SOURCE;
constexpr D3D12_RESOURCE_STATES STATE_RESOLVE_DST           = D3D12_RESOURCE_STATE_RESOLVE_DEST;
constexpr D3D12_RESOURCE_STATES STATE_RESOLVE_SRC           = D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
constexpr D3D12_RESOURCE_STATES STATE_RTAS                  = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
constexpr D3D12_RESOURCE_STATES STATE_GENERIC_READ          = D3D12_RESOURCE_STATE_GENERIC_READ;
constexpr D3D12_RESOURCE_STATES STATE_PRESENT               = D3D12_RESOURCE_STATE_PRESENT;
constexpr D3D12_RESOURCE_STATES STATE_PREDICATION           = D3D12_RESOURCE_STATE_PREDICATION;
constexpr D3D12_RESOURCE_STATES STATE_VIDEO_DECODE_READ     = D3D12_RESOURCE_STATE_VIDEO_DECODE_READ;
constexpr D3D12_RESOURCE_STATES STATE_VIDEO_DECODE_WRITE    = D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE;
constexpr D3D12_RESOURCE_STATES STATE_VIDEO_PROCESS_READ    = D3D12_RESOURCE_STATE_VIDEO_PROCESS_READ;
constexpr D3D12_RESOURCE_STATES STATE_VIDEO_PROCESS_WRITE   = D3D12_RESOURCE_STATE_VIDEO_PROCESS_WRITE;
constexpr D3D12_RESOURCE_STATES STATE_VIDEO_ENCODE_READ     = D3D12_RESOURCE_STATE_VIDEO_ENCODE_READ;
constexpr D3D12_RESOURCE_STATES STATE_VIDEO_ENCODE_WRITE    = D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE;

void BarrierTransition(
    ID3D12GraphicsCommandList*      pCmd,
    const IView*                    pView,
    uint32_t                        subresource,
    D3D12_RESOURCE_STATES           stateBefore,
    D3D12_RESOURCE_STATES           stateAfter,
    D3D12_RESOURCE_BARRIER_FLAGS    flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);

void BarrierTransition(
    ID3D12GraphicsCommandList*      pCmd,
    ID3D12Resource*                 pResource,
    uint32_t                        subresource,
    D3D12_RESOURCE_STATES           stateBefore,
    D3D12_RESOURCE_STATES           stateAfter,
    D3D12_RESOURCE_BARRIER_FLAGS    flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);

void BarrierUAV(
    ID3D12GraphicsCommandList*      pCmd,
    const IUnorderedAccessView*     pView,
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
    const uint32_t*                 pClearValues);

void ClearUAV(
    ID3D12GraphicsCommandList*      pCmd,
    const IUnorderedAccessView*     pView,
    const float*                    pClearValues);

void SetViewport(
    ID3D12GraphicsCommandList*      pCmd,
    const IView*                    pView,
    bool                            setScissor = true);

void SetViewport(
    ID3D12GraphicsCommandList*      pCmd,
    ID3D12Resource*                 pResource,
    bool                            setScissor = true);

void SetRenderTarget(
    ID3D12GraphicsCommandList*      pCmd,
    const IRenderTargetView*        pRTV,
    const IDepthStencilView*        pDSV);

void SetRenderTargets(
    ID3D12GraphicsCommandList*      pCmd,
    uint32_t                        count,
    const IRenderTargetView**       pRTVs,
    const IDepthStencilView*        pDSV);

void SetTable(
    ID3D12GraphicsCommandList*      pCmd,
    uint32_t                        index,
    const IView*                    pView,
    bool                            compute = false);

void SetCBV(
    ID3D12GraphicsCommandList*      pCmd,
    uint32_t                        index,
    const IConstantBufferView*      view,
    bool                            compute = false);

void SetCBV(
    ID3D12GraphicsCommandList*      pCmd,
    uint32_t                        index,
    ID3D12Resource*                 resource,
    bool                            compute = false);

void SetSRV(
    ID3D12GraphicsCommandList*      pCmd,
    uint32_t                        index,
    const IShaderResourceView*      view,
    bool                            compute = false);

void SetSRV(
    ID3D12GraphicsCommandList*      pCmd,
    uint32_t                        index,
    ID3D12Resource*                 resource,
    bool                            compute = false);

void SetUAV(
    ID3D12GraphicsCommandList*      pCmd,
    uint32_t                        index,
    const IUnorderedAccessView*     view,
    bool                            compute = false);

void SetUAV(
    ID3D12GraphicsCommandList*      pCmd,
    uint32_t                        index,
    ID3D12Resource*                 resource,
    bool                            compute = false);

void SetConstant(
    ID3D12GraphicsCommandList*      pCmd,
    uint32_t                        index,
    uint32_t                        data,
    uint32_t                        offset,
    bool                            compute = false);

void SetConstant(
    ID3D12GraphicsCommandList*      pCmd,
    uint32_t                        index,
    float                           data,
    uint32_t                        offset,
    bool                            compute = false);

void SetConstant(
    ID3D12GraphicsCommandList*      pCmd,
    uint32_t                        index,
    int                             data,
    uint32_t                        offset,
    bool                            compute = false);

void SetConstants(
    ID3D12GraphicsCommandList*      pCmd,
    uint32_t                        index,
    uint32_t                        count,
    const void*                     data,
    uint32_t                        offset,
    bool                            compute = false);

void BeginEvent(
    ID3D12GraphicsCommandList*      pCmd,
    const char*                     text);

void EndEvent(
    ID3D12GraphicsCommandList*      pCmd);


class ScopedTransition
{
public:
    ScopedTransition(
        ID3D12GraphicsCommandList*  pCmd,
        ID3D12Resource*             pResource,
        D3D12_RESOURCE_STATES       before,
        D3D12_RESOURCE_STATES       after,
        uint32_t                    subResource = 0
    )
    : m_pCmd        (pCmd)
    , m_pResource   (pResource)
    , m_StateBefore (before)
    , m_StateAfter  (after)
    , m_SubResource (subResource)
    { BarrierTransition(m_pCmd, m_pResource, m_SubResource, m_StateBefore, m_StateAfter); }

    ScopedTransition
    (
        ID3D12GraphicsCommandList*  pCmd,
        const IView*                pView,
        D3D12_RESOURCE_STATES       before,
        D3D12_RESOURCE_STATES       after,
        uint32_t                    subResource = 0
    )
    : m_pCmd        (pCmd)
    , m_pResource   (pView->GetResource())
    , m_StateBefore (before)
    , m_StateAfter  (after)
    , m_SubResource (subResource)
    { BarrierTransition(m_pCmd, m_pResource, m_SubResource, m_StateBefore, m_StateAfter); }

    ~ScopedTransition()
    { BarrierTransition(m_pCmd, m_pResource, m_SubResource, m_StateAfter, m_StateBefore); }

private:
    ID3D12GraphicsCommandList* m_pCmd;
    ID3D12Resource*            m_pResource;
    D3D12_RESOURCE_STATES      m_StateBefore;
    D3D12_RESOURCE_STATES      m_StateAfter;
    uint32_t                   m_SubResource;
};

class ScopedMarker
{
public:
    ScopedMarker(ID3D12GraphicsCommandList* pCmd, const char* text)
    : m_pCmd(pCmd)
    { BeginEvent(m_pCmd, text); }

    ~ScopedMarker()
    { EndEvent(m_pCmd); }

private:
    ID3D12GraphicsCommandList*  m_pCmd;
};

inline uint32_t DivRoundUp(uint32_t count, uint32_t div)
{ return (count + div - 1) / div; }

} // namespace asdx

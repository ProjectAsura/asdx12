//-----------------------------------------------------------------------------
// File : asdxEnhancedBarrier.cpp
// Desc : Enhanced Barrier.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxEnhancedBarrier.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// EnhancedBarrier class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      リセット処理を行います.
//-----------------------------------------------------------------------------
void EnhancedBarrier::Reset()
{
    m_GlobalCount  = 0;
    m_TextureCount = 0;
    m_BufferCount  = 0;

    memset(m_Global,  0, sizeof(D3D12_GLOBAL_BARRIER)  * 16);
    memset(m_Texture, 0, sizeof(D3D12_TEXTURE_BARRIER) * 16);
    memset(m_Buffer,  0, sizeof(D3D12_BUFFER_BARRIER)  * 16);
}

//-----------------------------------------------------------------------------
//      グローバルバリアを設定します.
//-----------------------------------------------------------------------------
void EnhancedBarrier::Global
(
    D3D12_BARRIER_SYNC   syncBefore,
    D3D12_BARRIER_SYNC   syncAfter,
    D3D12_BARRIER_ACCESS accessBefore,
    D3D12_BARRIER_ACCESS accessAfter
)
{
    if (syncBefore == syncAfter && accessBefore == accessAfter)
        return;

    auto& barrier = m_Global[m_GlobalCount++];
    barrier.SyncBefore   = syncBefore;
    barrier.SyncAfter    = syncAfter;
    barrier.AccessBefore = accessBefore;
    barrier.AccessAfter  = accessAfter;
}

//-----------------------------------------------------------------------------
//      テクスチャバリアを設定します.
//-----------------------------------------------------------------------------
void EnhancedBarrier::Texture
(
    ID3D12Resource*             pResource,
    D3D12_BARRIER_SYNC          syncBefore,
    D3D12_BARRIER_SYNC          syncAfter,
    D3D12_BARRIER_ACCESS        accessBefore,
    D3D12_BARRIER_ACCESS        accessAfter,
    D3D12_BARRIER_LAYOUT        layoutBefore,
    D3D12_BARRIER_LAYOUT        layoutAfter,
    D3D12_TEXTURE_BARRIER_FLAGS flags,
    const D3D12_BARRIER_SUBRESOURCE_RANGE* pRange
)
{
    if (!pResource)
        return;

    if (syncBefore   == syncAfter   &&
        accessBefore == accessAfter &&
        layoutBefore == layoutAfter)
        return;

    auto& barrier = m_Texture[m_TextureCount++];
    barrier.pResource       = pResource;
    barrier.SyncBefore      = syncBefore;
    barrier.SyncAfter       = syncAfter;
    barrier.AccessBefore    = accessBefore;
    barrier.AccessAfter     = accessAfter;
    barrier.LayoutBefore    = layoutBefore;
    barrier.LayoutAfter     = layoutAfter;
    barrier.Flags           = flags;

    if (pRange == nullptr)
    {
        auto desc = pResource->GetDesc();

        barrier.Subresources.IndexOrFirstMipLevel   = 0;
        barrier.Subresources.NumMipLevels           = desc.MipLevels;
        barrier.Subresources.FirstArraySlice        = 0;
        barrier.Subresources.NumMipLevels           = desc.DepthOrArraySize;
        barrier.Subresources.FirstPlane             = 0;
        barrier.Subresources.NumPlanes              = 1;
    }
    else
    {
        barrier.Subresources = *pRange;
    }
}

//-----------------------------------------------------------------------------
//      バッファバリアを設定します.
//-----------------------------------------------------------------------------
void EnhancedBarrier::Buffer
(
    ID3D12Resource*      pResource,
    D3D12_BARRIER_SYNC   syncBefore,
    D3D12_BARRIER_SYNC   syncAfter,
    D3D12_BARRIER_ACCESS accessBefore,
    D3D12_BARRIER_ACCESS accessAfter,
    UINT64               offset,
    UINT64               size
)
{
    if (!pResource)
        return;

    if (syncBefore == syncAfter && accessBefore == accessAfter)
        return;

    auto& barrier = m_Buffer[m_BufferCount++];
    barrier.pResource = pResource;
    barrier.SyncBefore = syncBefore;
    barrier.SyncAfter = syncAfter;
    barrier.AccessBefore = accessBefore;
    barrier.AccessAfter = accessAfter;
    barrier.Offset = offset;

    if (size == 0)
    {
        auto desc = pResource->GetDesc();
        barrier.Size = desc.Width;
    }
    else
    {
        barrier.Size = size;
    }
}

//-----------------------------------------------------------------------------
//      バリアを適用します.
//-----------------------------------------------------------------------------
void EnhancedBarrier::Apply(ID3D12GraphicsCommandList7* pCmd)
{
    if (!pCmd)
        return;

    if ((m_GlobalCount + m_TextureCount + m_BufferCount) == 0)
        return;

    UINT count = 0;
    D3D12_BARRIER_GROUP group[3] = {};

    if (m_GlobalCount > 0)
    {
        auto& g = group[count++];
        g.Type              = D3D12_BARRIER_TYPE_GLOBAL;
        g.NumBarriers       = m_GlobalCount;
        g.pGlobalBarriers   = m_Global;
    }

    if (m_TextureCount > 0)
    {
        auto& g = group[count++];
        g.Type              = D3D12_BARRIER_TYPE_TEXTURE;
        g.NumBarriers       = m_TextureCount;
        g.pTextureBarriers  = m_Texture;
    }

    if (m_BufferCount > 0)
    {
        auto& g = group[count++];
        g.Type              = D3D12_BARRIER_TYPE_BUFFER;
        g.NumBarriers       = m_BufferCount;
        g.pBufferBarriers   = m_Buffer;
    }

    pCmd->Barrier(count, group);
    Reset();
}

} // namespace asdx

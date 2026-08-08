//-----------------------------------------------------------------------------
// File : asdxLegacyBarrier.h
// Desc : Legacy Resource Barrier.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <gfx/asdxLegacyBarrier.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// LegacyBarrier class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      リセット処理です.
//-----------------------------------------------------------------------------
void LegacyBarrier::Reset()
{
    memset(m_Barrier, 0, sizeof(D3D12_RESOURCE_BARRIER) * 16);
    m_Count = 0;
}

//-----------------------------------------------------------------------------
//      遷移バリアを設定します.
//-----------------------------------------------------------------------------
void LegacyBarrier::Transition
(
    ID3D12Resource*         pResource,
    D3D12_RESOURCE_STATES   before,
    D3D12_RESOURCE_STATES   after
)
{
    if (!pResource || before == after)
        return;

    auto& barrier = m_Barrier[m_Count++];
    assert(m_Count <= 16);

    barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                   = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource    = pResource;
    barrier.Transition.StateBefore  = before;
    barrier.Transition.StateAfter   = after;
    barrier.Transition.Subresource  = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
}

//-----------------------------------------------------------------------------
//      エイリアシングバリアを設定します.
//-----------------------------------------------------------------------------
void LegacyBarrier::Aliasing(ID3D12Resource* pBefore, ID3D12Resource* pAfter)
{
    if (!pBefore || !pAfter)
        return;

    auto& barrier = m_Barrier[m_Count++];
    assert(m_Count <= 16);

    barrier.Type                        = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
    barrier.Flags                       = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Aliasing.pResourceBefore    = pBefore;
    barrier.Aliasing.pResourceAfter     = pAfter;
}

//-----------------------------------------------------------------------------
//      UAVバリアを設定します.
//-----------------------------------------------------------------------------
void LegacyBarrier::UAV(ID3D12Resource* pResource)
{
    if (!pResource)
        return;

    auto& barrier = m_Barrier[m_Count++];
    assert(m_Count <= 16);

    barrier.Type            = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.Flags           = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.UAV.pResource   = pResource;
}

//-----------------------------------------------------------------------------
//      リソースバリアを適用します.
//-----------------------------------------------------------------------------
void LegacyBarrier::Apply(ID3D12GraphicsCommandList* pCmd)
{
    if (!pCmd || m_Count == 0)
        return;

    pCmd->ResourceBarrier(m_Count, m_Barrier);
    Reset();
}

} // namespace asdx

//-----------------------------------------------------------------------------
// File : asdxCommandList.cpp
// Desc : Command List Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <vector>
#include <gfx/asdxCommandList.h>
#include <gfx/asdxGraphicsSystem.h>
#include <gfx/asdxSampler.h>
#include <fnd/asdxLogger.h>

namespace {

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
//      必要な中間データサイズを取得します.
//-----------------------------------------------------------------------------
inline UINT64 GetRequiredIntermediateSize
(
    ID3D12Device*           pDevice,
    D3D12_RESOURCE_DESC*    pDesc,
    UINT                    firstSubresource,
    UINT                    subresourceCount
) noexcept
{
    UINT64 requiredSize = 0;
    pDevice->GetCopyableFootprints(
        pDesc,
        firstSubresource,
        subresourceCount,
        0,
        nullptr,
        nullptr,
        nullptr,
        &requiredSize);
    return requiredSize;
}

//-----------------------------------------------------------------------------
//      サブリソースのコピーを行います.
//-----------------------------------------------------------------------------
inline void CopySubresource
(
    const D3D12_MEMCPY_DEST*        pDst,
    const D3D12_SUBRESOURCE_DATA*   pSrc,
    SIZE_T                          rowSizeInBytes,
    UINT                            rowCount,
    UINT                            sliceCount
) noexcept
{
    for (auto z=0u; z<sliceCount; ++z)
    {
        auto pDstSlice = static_cast<BYTE*>(pDst->pData)       + pDst->SlicePitch * z;
        auto pSrcSlice = static_cast<const BYTE*>(pSrc->pData) + pSrc->SlicePitch * LONG_PTR(z);
        for (auto y=0u; y<rowCount; ++y)
        {
            memcpy(pDstSlice + pDst->RowPitch * y,
                   pSrcSlice + pSrc->RowPitch * LONG_PTR(y),
                   rowSizeInBytes);
        }
    }
}
}

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// GraphicsCommandList class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
CommandList::CommandList()
: m_Allocator()
, m_CmdList  ()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
CommandList::~CommandList()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool CommandList::Init(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type)
{
    // 引数チェック.
    if (pDevice == nullptr)
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    for(auto i=0; i<2; ++i)
    {
        // コマンドアロケータを生成.
        auto hr = pDevice->CreateCommandAllocator( type, IID_PPV_ARGS( m_Allocator[i].GetAddress() ) );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateCommandAllocator() Failed. errcode = 0x%x", hr );
            return false;
        }
    }

    // コマンドリストを生成.
    auto hr = pDevice->CreateCommandList(
        0,
        type,
        m_Allocator[0].GetPtr(),
        nullptr,
        IID_PPV_ARGS( m_CmdList.GetAddress() ) );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D12Device::CreateCommandList() Failed. errcode = 0x%x", hr );
        return false;
    }

    // 生成直後は開きっぱなしの扱いになっているので閉じておく.
    m_CmdList->Close();

    m_Index = 0;

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void CommandList::Term()
{
    m_CmdList.Reset();

    for(auto i=0; i<2; ++i)
    { m_Allocator[i].Reset(); }
}

//-----------------------------------------------------------------------------
//      コマンドリストをリセットします.
//-----------------------------------------------------------------------------
void CommandList::Reset()
{
    // ダブルバッファリング.
    m_Index = (m_Index + 1) & 0x1;

    // コマンドアロケータをリセット.
    m_Allocator[m_Index]->Reset();

    // コマンドリストをリセット.
    m_CmdList->Reset( m_Allocator[m_Index].GetPtr(), nullptr );

    // ディスクリプターヒープを設定しおく.
    SetDescriptorHeaps(m_CmdList.GetPtr());
}

//-----------------------------------------------------------------------------
//      コマンドリストアロケータを取得します.
//-----------------------------------------------------------------------------
ID3D12CommandAllocator* CommandList::GetAllocator(uint8_t index) const
{
    assert(index < 2);
    return m_Allocator[index].GetPtr();
}

//-----------------------------------------------------------------------------
//      グラフィックスコマンドリストを取得します.
//-----------------------------------------------------------------------------
ID3D12GraphicsCommandList6* CommandList::GetCommandList() const
{ return m_CmdList.GetPtr(); }

//-----------------------------------------------------------------------------
//      現在のバッファ番号を返却します.
//-----------------------------------------------------------------------------
uint8_t CommandList::GetIndex() const
{ return m_Index; }

//-----------------------------------------------------------------------------
//      遷移バリアを発行します.
//-----------------------------------------------------------------------------
void CommandList::BarrierTransition
(
    const IView*                    pView,
    uint32_t                        subresource,
    D3D12_RESOURCE_STATES           stateBefore,
    D3D12_RESOURCE_STATES           stateAfter,
    D3D12_RESOURCE_BARRIER_FLAGS    flags
)
{
    assert(pView != nullptr);
    BarrierTransition(
        pView->GetResource(),
        subresource,
        stateBefore,
        stateAfter,
        flags);
}

//-----------------------------------------------------------------------------
//      遷移バリアを発行します.
//-----------------------------------------------------------------------------
void CommandList::BarrierTransition
(
    ID3D12Resource*                 pResource,
    uint32_t                        subresource,
    D3D12_RESOURCE_STATES           stateBefore,
    D3D12_RESOURCE_STATES           stateAfter,
    D3D12_RESOURCE_BARRIER_FLAGS    flags
)
{
    assert(pResource != nullptr);

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                   = flags;
    barrier.Transition.pResource    = pResource;
    barrier.Transition.Subresource  = subresource;
    barrier.Transition.StateBefore  = stateBefore;
    barrier.Transition.StateAfter   = stateAfter;

    m_CmdList->ResourceBarrier(1, &barrier);
}

//-----------------------------------------------------------------------------
//      UAVバリアを発行します.
//-----------------------------------------------------------------------------
void CommandList::BarrierUAV
(
    const IUnorderedAccessView*     pView,
    D3D12_RESOURCE_BARRIER_FLAGS    flags
)
{
    assert(pView != nullptr);
    BarrierUAV(pView->GetResource(), flags);
}

//-----------------------------------------------------------------------------
//      UAVバリアを発行します.
//-----------------------------------------------------------------------------
void CommandList::BarrierUAV
(
    ID3D12Resource*                 pResource,
    D3D12_RESOURCE_BARRIER_FLAGS    flags
)
{
    assert(pResource != nullptr);

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type            = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.Flags           = flags;
    barrier.UAV.pResource   = pResource;

    m_CmdList->ResourceBarrier(1, &barrier);
}

//-----------------------------------------------------------------------------
//      レンダーターゲットビューをクリアします.
//-----------------------------------------------------------------------------
void CommandList::ClearRTV(const IRenderTargetView* pView, const float* pClearColor)
{
    assert(pView       != nullptr);
    assert(pClearColor != nullptr);

    auto handle = pView->GetHandleCPU();
    m_CmdList->ClearRenderTargetView(handle, pClearColor, 0, nullptr);
}

//-----------------------------------------------------------------------------
//      深度ステンシルビューをクリアします.
//-----------------------------------------------------------------------------
void CommandList::ClearDSV(const IDepthStencilView* pView, float clearDepth)
{
    ClearDSV(pView, D3D12_CLEAR_FLAG_DEPTH, clearDepth, 0);
}

//-----------------------------------------------------------------------------
//      深度ステンシルビューをクリアします.
//-----------------------------------------------------------------------------
void CommandList::ClearDSV
(
    const IDepthStencilView*    pView,
    D3D12_CLEAR_FLAGS           flags,
    float                       clearDepth,
    uint8_t                     clearStencil
)
{
    assert(pView != nullptr);
    auto handle = pView->GetHandleCPU();
    m_CmdList->ClearDepthStencilView(handle, flags, clearDepth, clearStencil, 0, nullptr);
}

//-----------------------------------------------------------------------------
//      アンオーダードアクセスビューをクリアします.
//-----------------------------------------------------------------------------
void CommandList::ClearUAV
(
    const IUnorderedAccessView* pView,
    const uint32_t*             pClearValues
)
{
    assert(pView        != nullptr);
    assert(pClearValues != nullptr);

    m_CmdList->ClearUnorderedAccessViewUint(
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
void CommandList::ClearUAV
(
    const IUnorderedAccessView* pView,
    const float*                pClearValues
)
{
    assert(pView        != nullptr);
    assert(pClearValues != nullptr);

    m_CmdList->ClearUnorderedAccessViewFloat(
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
void CommandList::SetViewport(const IView* pView, bool setScissor)
{
    assert(pView != nullptr);
    SetViewport(pView->GetResource(), setScissor);
}

//-----------------------------------------------------------------------------
//      ビューポートを設定します.
//-----------------------------------------------------------------------------
void CommandList::SetViewport(ID3D12Resource* pResource, bool setScissor)
{
    assert(pResource != nullptr);
    auto desc = pResource->GetDesc();

    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width    = float(desc.Width);
    viewport.Height   = float(desc.Height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    m_CmdList->RSSetViewports(1, &viewport);

    if (setScissor == false)
    { return; }

    D3D12_RECT scissor = {};
    scissor.left    = 0;
    scissor.right   = LONG(desc.Width);
    scissor.top     = 0;
    scissor.bottom  = LONG(desc.Height);

    m_CmdList->RSSetScissorRects(1, &scissor);
}

//-----------------------------------------------------------------------------
//      レンダーターゲットを設定します.
//-----------------------------------------------------------------------------
void CommandList::SetTarget(const IRenderTargetView* pRTV, const IDepthStencilView* pDSV)
{
    auto handleRTV = (pRTV != nullptr) ? pRTV->GetHandleCPU() : D3D12_CPU_DESCRIPTOR_HANDLE();
    auto handleDSV = (pDSV != nullptr) ? pDSV->GetHandleCPU() : D3D12_CPU_DESCRIPTOR_HANDLE();

    auto pHandleRTV = (pRTV != nullptr) ? &handleRTV : nullptr;
    auto pHandleDSV = (pDSV != nullptr) ? &handleDSV : nullptr;

    m_CmdList->OMSetRenderTargets(1, pHandleRTV, FALSE, pHandleDSV);
}

//-----------------------------------------------------------------------------
//      レンダーターゲットを設定します.
//-----------------------------------------------------------------------------
void CommandList::SetTargets
(
    uint32_t                    count,
    const IRenderTargetView**   pRTVs,
    const IDepthStencilView*    pDSV
)
{
    assert(pRTVs != nullptr);
    assert(pDSV  != nullptr);

    D3D12_CPU_DESCRIPTOR_HANDLE handleRTVs[8] = {};
    for(auto i=0u; i<count; ++i)
    {
        assert(pRTVs[i] != nullptr);
        handleRTVs[i] = pRTVs[i]->GetHandleCPU();
    }

    auto handleDSV = (pDSV != nullptr) ? pDSV->GetHandleCPU() : D3D12_CPU_DESCRIPTOR_HANDLE();
    auto pHandleDSV = (pDSV != nullptr) ? &handleDSV : nullptr;
    m_CmdList->OMSetRenderTargets(count, handleRTVs, FALSE, pHandleDSV);
}

//-----------------------------------------------------------------------------
//      32bit定数を設定します.
//-----------------------------------------------------------------------------
void CommandList::SetConstants
(
    uint32_t                    index,
    uint32_t                    count,
    const void*                 pData,
    uint32_t                    offset,
    bool                        compute
)
{
    if (index == UINT32_MAX || count == 0 || pData == nullptr)
    { return; }

    if (compute)
    { m_CmdList->SetComputeRoot32BitConstants(index, count, pData, offset); }
    else
    { m_CmdList->SetGraphicsRoot32BitConstants(index, count, pData, offset); }
}

//-----------------------------------------------------------------------------
//      ディスクリプタテーブルを設定します.
//-----------------------------------------------------------------------------
void CommandList::SetTable
(
    uint32_t                    index,
    const IView*                pView,
    bool                        compute
)
{
    if (pView == nullptr || index == UINT32_MAX)
    { return; }

    if (compute)
    { m_CmdList->SetComputeRootDescriptorTable(index, pView->GetHandleGPU()); }
    else
    { m_CmdList->SetGraphicsRootDescriptorTable(index, pView->GetHandleGPU()); }
}

//-----------------------------------------------------------------------------
//      ディスクリプタテーブルを設定します.
//-----------------------------------------------------------------------------
void CommandList::SetTable
(
    uint32_t                    index,
    const Sampler*              pSampler,
    bool                        compute
)
{
    if (pSampler == nullptr || index == UINT32_MAX)
    { return; }

    if (pSampler->GetDescriptor() == nullptr)
    { return; }

    if (compute)
    { m_CmdList->SetComputeRootDescriptorTable(index, pSampler->GetDescriptor()->GetHandleGPU()); }
    else
    { m_CmdList->SetGraphicsRootDescriptorTable(index, pSampler->GetDescriptor()->GetHandleGPU()); }
}

//-----------------------------------------------------------------------------
//      定数バッファビューを設定します.
//-----------------------------------------------------------------------------
void CommandList::SetCBV
(
    uint32_t                    index,
    const IConstantBufferView*  pView,
    bool                        compute
)
{
    if (pView == nullptr)
    { return; }

    SetCBV(index, pView->GetResource(), compute);
}

//-----------------------------------------------------------------------------
//      定数バッファビューを設定します.
//-----------------------------------------------------------------------------
void CommandList::SetCBV
(
    uint32_t                    index,
    ID3D12Resource*             pResource,
    bool                        compute
)
{
    if (pResource == nullptr)
    { return; }

    auto addr = pResource->GetGPUVirtualAddress();

    if (compute)
    { m_CmdList->SetComputeRootConstantBufferView(index, addr); }
    else
    { m_CmdList->SetGraphicsRootConstantBufferView(index, addr); }
}

//-----------------------------------------------------------------------------
//      シェーダリソースビューを設定します.
//-----------------------------------------------------------------------------
void CommandList::SetSRV
(
    uint32_t                    index,
    const IShaderResourceView*  pView,
    bool                        compute
)
{
    if (pView == nullptr)
    { return; }

    SetSRV(index, pView->GetResource(), compute);
}

//-----------------------------------------------------------------------------
//      シェーダリソースビューを設定します.
//-----------------------------------------------------------------------------
void CommandList::SetSRV
(
    uint32_t                    index,
    ID3D12Resource*             pResource,
    bool                        compute
)
{
    if (pResource == nullptr || index == UINT32_MAX)
    { return; }

    auto addr = pResource->GetGPUVirtualAddress();

    if (compute)
    { m_CmdList->SetComputeRootShaderResourceView(index, addr); }
    else
    { m_CmdList->SetGraphicsRootShaderResourceView(index, addr); }
}

//-----------------------------------------------------------------------------
//      アンオーダードアクセスビューを設定します.
//-----------------------------------------------------------------------------
void CommandList::SetUAV
(
    uint32_t                    index,
    const IUnorderedAccessView* pView,
    bool                        compute
)
{
    if (pView == nullptr)
    { return; }

    SetUAV(index, pView->GetResource(), compute);
}

//-----------------------------------------------------------------------------
//      アンオーダードアクセスビューを設定します.
//-----------------------------------------------------------------------------
void CommandList::SetUAV
(
    uint32_t                    index,
    ID3D12Resource*             pResource,
    bool                        compute
)
{
    if (pResource == 0 || index == UINT32_MAX)
    { return; }

    auto addr = pResource->GetGPUVirtualAddress();

    if (compute)
    { m_CmdList->SetComputeRootUnorderedAccessView(index, addr); }
    else
    { m_CmdList->SetGraphicsRootUnorderedAccessView(index, addr); }
}

//-----------------------------------------------------------------------------
//      ルートシグニチャを設定します.
//-----------------------------------------------------------------------------
void CommandList::SetRootSignature(ID3D12RootSignature* value, bool compute)
{
    if (value == nullptr)
    { return; }

    if (compute)
    { m_CmdList->SetComputeRootSignature(value); }
    else
    { m_CmdList->SetGraphicsRootSignature(value); }
}

//-----------------------------------------------------------------------------
//      イベントを開始します.
//-----------------------------------------------------------------------------
void CommandList::BeginEvent(const char* text)
{
    assert(text != nullptr);
    static const UINT PIX_EVENT_ANSI_VERSION = 1;
    auto size = UINT((strlen(text) + 1) * sizeof(char));
    m_CmdList->BeginEvent(PIX_EVENT_ANSI_VERSION, text, size);
}

//-----------------------------------------------------------------------------
//      リソースをアップロードします.
//-----------------------------------------------------------------------------
void CommandList::UpdateSubresources
(
    ID3D12Resource*                 pDstResource,
    uint32_t                        subResourceCount,
    uint32_t                        subResourceOffset,
    const D3D12_SUBRESOURCE_DATA*   subResources
)
{
    if (pDstResource == nullptr || subResources == nullptr || subResourceCount == 0)
    { return; }

    auto device  = GetD3D12Device();
    auto dstDesc = pDstResource->GetDesc();

    D3D12_RESOURCE_DESC uploadDesc = {
        D3D12_RESOURCE_DIMENSION_BUFFER,
        0,
        GetRequiredIntermediateSize(device, &dstDesc, subResourceOffset, subResourceCount),
        1,
        1,
        1,
        DXGI_FORMAT_UNKNOWN,
        { 1, 0 },
        D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
        D3D12_RESOURCE_FLAG_NONE
    };

    D3D12_HEAP_PROPERTIES props = {
        D3D12_HEAP_TYPE_UPLOAD,
        D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL_UNKNOWN,
        1,
        1
    };

    ID3D12Resource* pSrcResource = nullptr;
    auto hr = device->CreateCommittedResource(
        &props,
        D3D12_HEAP_FLAG_NONE,
        &uploadDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&pSrcResource));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device::CreateCommitedResource() Failed. errcode = 0x%x", hr);
        return;
    }

    // コマンドを生成.
    {
        auto count = subResourceCount;

        std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts;
        std::vector<UINT>                               rows;
        std::vector<UINT64>                             rowSizeInBytes;

        layouts       .resize(count);
        rows          .resize(count);
        rowSizeInBytes.resize(count);

        UINT64 requiredSize = 0;
        device->GetCopyableFootprints(
            &dstDesc, subResourceOffset, count, 0, layouts.data(), rows.data(), rowSizeInBytes.data(), &requiredSize);

        BYTE* pData = nullptr;
        hr = pSrcResource->Map(0, nullptr, reinterpret_cast<void**>(&pData));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Resource::Map() Failed. errcode = 0x%x", hr);
            pSrcResource->Release();
            return;
        }

        for(auto i=0u; i<count; ++i)
        {
            D3D12_MEMCPY_DEST dstData;
            dstData.pData       = pData + layouts[i].Offset;
            dstData.RowPitch    = layouts[i].Footprint.RowPitch;
            dstData.SlicePitch  = SIZE_T(layouts[i].Footprint.RowPitch) * SIZE_T(rows[i]);

            CopySubresource(
                &dstData,
                &subResources[i],
                SIZE_T(rowSizeInBytes[i]),
                rows[i],
                layouts[i].Footprint.Depth);
        }
        pSrcResource->Unmap(0, nullptr);

        if (dstDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
        {
            m_CmdList->CopyBufferRegion(
                pDstResource,
                0,
                pSrcResource,
                layouts[0].Offset,
                layouts[0].Footprint.Width);
        }
        else
        {
            for(auto i=0u; i<count; ++i)
            {
                D3D12_TEXTURE_COPY_LOCATION dstLoc = {};
                dstLoc.pResource        = pDstResource;
                dstLoc.Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                dstLoc.SubresourceIndex = i + subResourceOffset;

                D3D12_TEXTURE_COPY_LOCATION srcLoc = {};
                srcLoc.pResource        = pSrcResource;
                srcLoc.Type             = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                srcLoc.PlacedFootprint  = layouts[i];

                m_CmdList->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);
            }
        }
    }

    Dispose(pSrcResource);
}

//-----------------------------------------------------------------------------
//      バッファを更新します.
//-----------------------------------------------------------------------------
void CommandList::UpdateBuffer(ID3D12Resource* pDstResource, const void* pSrcResource)
{
    if (pSrcResource == nullptr || pDstResource == nullptr)
    { return; }

    auto desc = pDstResource->GetDesc();

    D3D12_SUBRESOURCE_DATA res = {};
    res.pData       = pSrcResource;
    res.RowPitch    = desc.Width;
    res.SlicePitch  = desc.Width;

    UpdateSubresources(pDstResource, 1, 0, &res);
}

//-----------------------------------------------------------------------
//      テクスチャを更新します.
//-----------------------------------------------------------------------
void CommandList::UpdateTexture(ID3D12Resource* pDstResource, const ResTexture* pResTexture)
{
    if (pDstResource == nullptr || pResTexture == nullptr)
    { return; }

    auto device = GetD3D12Device();
    auto dstDesc = pDstResource->GetDesc();

    auto count = pResTexture->MipMapCount * pResTexture->SurfaceCount;

    D3D12_RESOURCE_DESC uploadDesc = {
        D3D12_RESOURCE_DIMENSION_BUFFER,
        0,
        GetRequiredIntermediateSize(device, &dstDesc, 0, count),
        1,
        1,
        1,
        DXGI_FORMAT_UNKNOWN,
        { 1, 0 },
        D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
        D3D12_RESOURCE_FLAG_NONE
    };

    D3D12_HEAP_PROPERTIES props = {
        D3D12_HEAP_TYPE_UPLOAD,
        D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL_UNKNOWN,
        1,
        1
    };

    ID3D12Resource* pSrcResource = nullptr;
    auto hr = device->CreateCommittedResource(
        &props,
        D3D12_HEAP_FLAG_NONE,
        &uploadDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&pSrcResource));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device::CreateCommitedResource() Failed. errcode = 0x%x", hr);
        return;
    }

    // コマンドを生成.
    {
        std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts;
        std::vector<UINT>                               rows;
        std::vector<UINT64>                             rowSizeInBytes;

        layouts       .resize(count);
        rows          .resize(count);
        rowSizeInBytes.resize(count);

        UINT64 requiredSize = 0;
        device->GetCopyableFootprints(
            &dstDesc, 0, count, 0, layouts.data(), rows.data(), rowSizeInBytes.data(), &requiredSize);

        BYTE* pData = nullptr;
        hr = pSrcResource->Map(0, nullptr, reinterpret_cast<void**>(&pData));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Resource::Map() Failed. errcode = 0x%x", hr);
            pSrcResource->Release();
            return;
        }

        for(auto i=0u; i<count; ++i)
        {
            D3D12_SUBRESOURCE_DATA srcData = {};
            srcData.pData       = pResTexture->pResources[i].pPixels;
            srcData.RowPitch    = pResTexture->pResources[i].Pitch;
            srcData.SlicePitch  = pResTexture->pResources[i].SlicePitch;
            assert(layouts[i].Footprint.Width  == pResTexture->pResources[i].Width);
            assert(layouts[i].Footprint.Height == pResTexture->pResources[i].Height);

            D3D12_MEMCPY_DEST dstData = {};
            dstData.pData       = pData + layouts[i].Offset;
            dstData.RowPitch    = layouts[i].Footprint.RowPitch;
            dstData.SlicePitch  = SIZE_T(layouts[i].Footprint.RowPitch) * SIZE_T(rows[i]);

            CopySubresource(
                &dstData,
                &srcData,
                SIZE_T(rowSizeInBytes[i]),
                rows[i],
                layouts[i].Footprint.Depth);
        }
        pSrcResource->Unmap(0, nullptr);

        if (dstDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
        {
            m_CmdList->CopyBufferRegion(
                pDstResource,
                0,
                pSrcResource,
                layouts[0].Offset,
                layouts[0].Footprint.Width);
        }
        else
        {
            for(auto i=0u; i<count; ++i)
            {
                D3D12_TEXTURE_COPY_LOCATION dstLoc = {};
                dstLoc.pResource        = pDstResource;
                dstLoc.Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                dstLoc.SubresourceIndex = i;

                D3D12_TEXTURE_COPY_LOCATION srcLoc = {};
                srcLoc.pResource        = pSrcResource;
                srcLoc.Type             = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                srcLoc.PlacedFootprint  = layouts[i];

                m_CmdList->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);
            }
        }
    }

    Dispose(pSrcResource);
}

///////////////////////////////////////////////////////////////////////////////
// ScopedMarker class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ScopedMarker::ScopedMarker(ID3D12GraphicsCommandList* pCmd, const char* text)
: m_pCmd(pCmd)
{
    assert(text != nullptr);
    static const UINT PIX_EVENT_ANSI_VERSION = 1;
    auto size = UINT((strlen(text) + 1) * sizeof(char));
    m_pCmd->BeginEvent(PIX_EVENT_ANSI_VERSION, text, size);
}

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ScopedMarker::~ScopedMarker()
{ m_pCmd->EndEvent(); }


} // namespace asdx

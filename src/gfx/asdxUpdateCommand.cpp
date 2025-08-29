//-----------------------------------------------------------------------------
// File : asdxUpdateCommand.cpp
// Desc : Update Command.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <fnd/asdxLogger.h>
#include <gfx/asdxUpdateCommand.h>
#include <gfx/asdxDevice.h>
#include <res/asdxResTexture.h>


namespace asdx {

//-----------------------------------------------------------------------------
//      必要な中間データサイズを取得します.
//-----------------------------------------------------------------------------
uint64_t GetRequiredIntermediateSize
(
    ID3D12Device*           pDevice,
    D3D12_RESOURCE_DESC*    pDesc,
    uint32_t                firstSubresource,
    uint32_t                subresourceCount
) 
{
    uint64_t requiredSize = 0;
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
void CopySubresource
(
    const D3D12_MEMCPY_DEST*        pDst,
    const D3D12_SUBRESOURCE_DATA*   pSrc,
    size_t                          rowSizeInBytes,
    uint32_t                        rowCount,
    uint32_t                        sliceCount
)
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

//-----------------------------------------------------------------------------
//      サブリソースを更新します.
//-----------------------------------------------------------------------------
void UpdateSubResources
(
    ID3D12GraphicsCommandList*      pCmdList,
    ID3D12Resource*                 pDstResource,
    uint32_t                        subResourceCount,
    uint32_t                        subResourceOffset,
    const D3D12_SUBRESOURCE_DATA*   subResources
)
{
    if (pCmdList == nullptr || pDstResource == nullptr || subResources == nullptr || subResourceCount == 0)
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
            D3D12_MEMCPY_DEST dstData = {};
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
            pCmdList->CopyBufferRegion(
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

                pCmdList->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);
            }
        }
    }

    Dispose(pSrcResource);
}

//-----------------------------------------------------------------------------
//      バッファを更新します.
//-----------------------------------------------------------------------------
void UpdateBuffer
(
    ID3D12GraphicsCommandList*  pCmdList,
    ID3D12Resource*             pDstResource,
    const void*                 pSrcResource
)
{
    if (pCmdList == nullptr || pSrcResource == nullptr || pDstResource == nullptr)
    { return; }

    auto desc = pDstResource->GetDesc();

    D3D12_SUBRESOURCE_DATA res = {};
    res.pData       = pSrcResource;
    res.RowPitch    = desc.Width;
    res.SlicePitch  = desc.Width;

    UpdateSubResources(pCmdList, pDstResource, 1, 0, &res);
}

//-----------------------------------------------------------------------------
//      テクスチャを更新します.
//-----------------------------------------------------------------------------
void UpdateTexture
(
    ID3D12GraphicsCommandList*  pCmdList,
    ID3D12Resource*             pDstResource,
    const ResTexture*           pResTexture
)
{
    if (pCmdList == nullptr || pDstResource == nullptr || pResTexture == nullptr)
    { return; }

    auto device = GetD3D12Device();
    auto dstDesc = pDstResource->GetDesc();

    auto count = pResTexture->SubResourceCount;

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
            const auto& inRes = pResTexture->SubResources[i];

            D3D12_SUBRESOURCE_DATA srcData = {};
            srcData.pData       = inRes.pPixels;
            srcData.RowPitch    = inRes.RowPitch;
            srcData.SlicePitch  = inRes.SlicePitch;

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
            pCmdList->CopyBufferRegion(
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

                pCmdList->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);
            }
        }
    }

    Dispose(pSrcResource);
}

} // namespace asdx

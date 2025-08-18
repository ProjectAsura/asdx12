//-----------------------------------------------------------------------------
// File : asdxUpdateCommand.h
// Desc : Update Command.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdint>
#include <d3d12.h>


namespace asdx {

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
struct ResTexture;


//-----------------------------------------------------------------------------
//! @brief      必要な中間データサイズを取得します.
//-----------------------------------------------------------------------------
uint64_t GetRequiredIntermediateSize(
    ID3D12Device*                   pDevice,
    D3D12_RESOURCE_DESC*            pDesc,
    uint32_t                        subResourceCount,
    uint32_t                        subResourceOffset);

//-----------------------------------------------------------------------------
//! @brief      サブリソースのコピーを行います.
//-----------------------------------------------------------------------------
void CopySubresource(
    const D3D12_MEMCPY_DEST*        pDst,
    const D3D12_SUBRESOURCE_DATA*   pSrc,
    size_t                          rowSizeInBytes,
    uint32_t                        rowCount,
    uint32_t                        sliceCount);

//-----------------------------------------------------------------------------
//! @brief      サブリソースを更新します.
//-----------------------------------------------------------------------------
void UpdateSubResources(
    ID3D12GraphicsCommandList*      pCmdList,
    ID3D12Resource*                 pDstResource,
    uint32_t                        subResourceCount,
    uint32_t                        subResourceOffset,
    const D3D12_SUBRESOURCE_DATA*   pSubResources);

//-----------------------------------------------------------------------------
//! @brief      バッファを更新します.
//-----------------------------------------------------------------------------
void UpdateBuffer(
    ID3D12GraphicsCommandList*      pCmdList,
    ID3D12Resource*                 pDstResource,
    const void*                     pSrcResource);

//-----------------------------------------------------------------------------
//! @brief      テクスチャを更新します.
//-----------------------------------------------------------------------------
void UpdateTexture(
    ID3D12GraphicsCommandList*      pCmdList,
    ID3D12Resource*                 pDstResource,
    const ResTexture*               pSrcResource);

} // namespace asdx

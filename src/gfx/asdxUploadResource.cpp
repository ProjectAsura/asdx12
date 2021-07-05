//-----------------------------------------------------------------------------
// File : asdxUploadResource.cpp
// Desc : Upload Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <atomic>
#include <vector>
#include <gfx/asdxUploadResource.h>
#include <gfx/asdxGraphicsSystem.h>
#include <fnd/asdxLogger.h>


namespace {

///////////////////////////////////////////////////////////////////////////////
// CopyTextureRegionCommand structure
///////////////////////////////////////////////////////////////////////////////
struct CopyTextureRegionCommand
{
    D3D12_TEXTURE_COPY_LOCATION Dst;    //!< 出力.
    D3D12_TEXTURE_COPY_LOCATION Src;    //!< 入力.
};

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

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// TextureUploadResource class
///////////////////////////////////////////////////////////////////////////////
class TextureUploadResource : public IUploadTexture
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    TextureUploadResource()
    : m_RefCount(1)
    , m_pDstResource(nullptr)
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~TextureUploadResource()
    { Term(); }

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //-------------------------------------------------------------------------
    bool Init
    (
        ID3D12Device*       pDevice,
        ID3D12Resource*     pDstResource,
        const ResTexture&   resource
    )
    {
        if (pDevice == nullptr || pDstResource == nullptr)
        {
            ELOG("Error : Invalid Argument.");
            return false;
        }

        // 消されると困るので参照カウントを上げる.
        m_pDstResource = pDstResource;
        m_pDstResource->AddRef();

        auto count = resource.MipMapCount * resource.SurfaceCount;
        auto dstDesc = m_pDstResource->GetDesc();

        // アップロード用リソースを生成.
        {
            D3D12_RESOURCE_DESC uploadDesc = {
                D3D12_RESOURCE_DIMENSION_BUFFER,
                0,
                GetRequiredIntermediateSize( pDevice, &dstDesc, 0, count ),
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

            auto hr = pDevice->CreateCommittedResource(
                &props,
                D3D12_HEAP_FLAG_NONE,
                &uploadDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(m_pSrcResource.GetAddress()));
            if ( FAILED( hr ) )
            {
                ELOG("Error : ID3D12Device::CreateCommitedResoure() Failed. errcode = 0x%x", hr);
                return false;
            }
        }


        // コマンドを生成.
        {
            m_Layouts.resize(count);
            m_Rows.resize(count);
            m_RowSizeInBytes.resize(count);
            m_Commands.resize(count);

            UINT64 requiredSize = 0;
            pDevice->GetCopyableFootprints(
                &dstDesc, 0, count, 0, m_Layouts.data(), m_Rows.data(), m_RowSizeInBytes.data(), &requiredSize);

            Update(resource);
        }

        return true;
    }

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term()
    {
        if (m_pDstResource != nullptr)
        {
            m_pDstResource->Release();
            m_pDstResource = nullptr;
        }

        m_pSrcResource.Reset();
        m_Commands.clear();
        m_Commands.shrink_to_fit();
    }

    //-------------------------------------------------------------------------
    //! @brief      参照カウントを増やします.
    //-------------------------------------------------------------------------
    void AddRef() override
    { m_RefCount++; }

    //-------------------------------------------------------------------------
    //! @brief      参照カウントを減らします.
    //-------------------------------------------------------------------------
    void Release() override
    {
        m_RefCount--;
        if (m_RefCount == 0)
        { delete this; }
    }

    //-------------------------------------------------------------------------
    //! @brief      参照カウントを取得します.
    //!
    //! @return     参照カウントを返却します.
    //-------------------------------------------------------------------------
    uint32_t GetCount() const override
    { return m_RefCount; }

    //-------------------------------------------------------------------------
    //! @brief      アップロードコマンドを生成します.
    //-------------------------------------------------------------------------
    void Upload(ID3D12GraphicsCommandList* pCmdList) override
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource   = m_pDstResource;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
        barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_DEST;
        pCmdList->ResourceBarrier( 1, &barrier );

        for(size_t i=0; i<m_Commands.size(); ++i)
        {
            pCmdList->CopyTextureRegion(
                &m_Commands[i].Dst, 0, 0, 0,
                &m_Commands[i].Src, nullptr);
        }

        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_GENERIC_READ;
        pCmdList->ResourceBarrier( 1, &barrier );
    }

    //-------------------------------------------------------------------------
    //      リソースを更新します.
    //-------------------------------------------------------------------------
    void Update(const ResTexture& resource) override
    {
        BYTE* pData = nullptr;
        auto hr = m_pSrcResource->Map(0, nullptr, reinterpret_cast<void**>(&pData));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Resource::Map() Failed. errcode = 0x%x", hr);
            return;
        }

        auto idx = 0;
    #if ASDX_IS_SCARLETT
        for(auto m=int(resource.MipCount)-1; m>=0; m--)
    #else
        for(auto m=0u; m<resource.MipMapCount; ++m)
    #endif
        {
            for(auto s=0u; s<resource.SurfaceCount; ++s)
            {
                D3D12_SUBRESOURCE_DATA srcData = {};
                srcData.pData       = resource.pResources[idx].pPixels;
                srcData.RowPitch    = resource.pResources[idx].Pitch;
                srcData.SlicePitch  = resource.pResources[idx].SlicePitch;

                D3D12_MEMCPY_DEST dstData = {};
                dstData.pData       = pData + m_Layouts[idx].Offset;
                dstData.RowPitch    = m_Layouts[idx].Footprint.RowPitch;
                dstData.SlicePitch  = SIZE_T(m_Layouts[idx].Footprint.RowPitch) * SIZE_T(m_Rows[idx]);

                CopySubresource(
                    &dstData,
                    &srcData,
                    SIZE_T(m_RowSizeInBytes[idx]),
                    m_Rows[idx],
                    m_Layouts[idx].Footprint.Depth);

                m_Commands[idx].Dst.pResource           = m_pDstResource;
                m_Commands[idx].Dst.Type                = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                m_Commands[idx].Dst.PlacedFootprint     = {};
                m_Commands[idx].Dst.SubresourceIndex    = idx;

                m_Commands[idx].Src.pResource           = m_pSrcResource.GetPtr();
                m_Commands[idx].Src.Type                = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                m_Commands[idx].Src.PlacedFootprint     = m_Layouts[idx];

                idx++;
            }
        }

        m_pSrcResource->Unmap(0, nullptr);
    }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    std::atomic<uint32_t>                           m_RefCount;
    ID3D12Resource*                                 m_pDstResource;
    asdx::RefPtr<ID3D12Resource>                    m_pSrcResource;
    std::vector<CopyTextureRegionCommand>           m_Commands;
    std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> m_Layouts;
    std::vector<UINT>                               m_Rows;
    std::vector<UINT64>                             m_RowSizeInBytes;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};


///////////////////////////////////////////////////////////////////////////////
// BufferUploadResource class
///////////////////////////////////////////////////////////////////////////////
class BufferUploadResource : public IUploadBuffer
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    BufferUploadResource()
    : m_RefCount    (1)
    , m_pDstResource(nullptr)
    , m_pSrcResource()
    , m_SrcOffset   (0)
    , m_CopyBytes   (0)
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~BufferUploadResource()
    { Term(); }

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //-------------------------------------------------------------------------
    bool Init(ID3D12Device* pDevice, ID3D12Resource* pDstResource, const void* pInitData)
    {
        if (pDevice == nullptr || pDstResource == nullptr || pInitData == nullptr)
        {
            ELOG("Error : Invalid Argument.");
            return false;
        }

        m_pDstResource = pDstResource;
        m_pDstResource->AddRef();

        auto dstDesc = m_pDstResource->GetDesc();

        // アップロード用リソースを生成.
        {
            D3D12_RESOURCE_DESC uploadDesc = {
                D3D12_RESOURCE_DIMENSION_BUFFER,
                0,
                GetRequiredIntermediateSize( pDevice, &dstDesc, 0, 1 ),
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

            auto hr = pDevice->CreateCommittedResource(
                &props,
                D3D12_HEAP_FLAG_NONE,
                &uploadDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(m_pSrcResource.GetAddress()));
            if ( FAILED( hr ) )
            {
                ELOG("Error : ID3D12Device::CreateCommitedResoure() Failed. errcode = 0x%x", hr);
                return false;
            }
        }

        // コマンド生成.
        {
            pDevice->GetCopyableFootprints(
                &dstDesc, 0, 1, 0, &m_Layout, &m_RowCount, &m_RowSizeInBytes, &m_RequiredSize);

            m_SrcOffset = m_Layout.Offset;
            m_CopyBytes = m_Layout.Footprint.Width;

            Update(pInitData);
        }

        return true;
    }

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term()
    {
        if (m_pDstResource != nullptr)
        {
            m_pDstResource->Release();
            m_pDstResource = nullptr;
        }

        m_pSrcResource.Reset();
        m_SrcOffset = 0;
        m_CopyBytes = 0;
    }

    //-------------------------------------------------------------------------
    //! @brief      参照カウントを増やします.
    //-------------------------------------------------------------------------
    void AddRef() override
    { m_RefCount++; }

    //-------------------------------------------------------------------------
    //! @brief      参照カウントを減らします.
    //-------------------------------------------------------------------------
    void Release() override
    {
        m_RefCount--;
        if (m_RefCount == 0)
        { delete this; }
    }

    //-------------------------------------------------------------------------
    //! @brief      参照カウントを取得します.
    //-------------------------------------------------------------------------
    uint32_t GetCount() const override
    { return m_RefCount; }

    //-------------------------------------------------------------------------
    //! @brief      アップロードコマンドを生成します.
    //-------------------------------------------------------------------------
    void Upload(ID3D12GraphicsCommandList* pCmdList) override
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource   = m_pDstResource;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
        barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_DEST;
        pCmdList->ResourceBarrier( 1, &barrier );

        pCmdList->CopyBufferRegion(
            m_pDstResource,
            0,
            m_pSrcResource.GetPtr(),
            m_SrcOffset,
            m_CopyBytes);

        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_GENERIC_READ;
        pCmdList->ResourceBarrier( 1, &barrier );
    }

    //-------------------------------------------------------------------------
    //! @brief      リソースを更新します.
    //-------------------------------------------------------------------------
    void Update(const void* pResource) override
    {
        auto dstDesc = m_pDstResource->GetDesc();

        BYTE* pData = nullptr;
        auto hr = m_pSrcResource->Map(0, nullptr, reinterpret_cast<void**>(&pData));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Resource::Map() Failed. errcode = 0x%x", hr);
            return;
        }

        D3D12_SUBRESOURCE_DATA srcData = {};
        srcData.pData       = pResource;
        srcData.RowPitch    = dstDesc.Width;
        srcData.SlicePitch  = dstDesc.Width * dstDesc.Height;

        D3D12_MEMCPY_DEST dstData = {};
        dstData.pData       = pData + m_Layout.Offset;
        dstData.RowPitch    = m_Layout.Footprint.RowPitch;
        dstData.SlicePitch  = SIZE_T(m_Layout.Footprint.RowPitch) * SIZE_T(m_RowCount);

        CopySubresource(
            &dstData,
            &srcData,
            SIZE_T(m_RowSizeInBytes),
            m_RowCount,
            m_Layout.Footprint.Depth);

        m_pSrcResource->Unmap(0, nullptr);
    }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    std::atomic<uint32_t>               m_RefCount;
    ID3D12Resource*                     m_pDstResource;
    asdx::RefPtr<ID3D12Resource>        m_pSrcResource;
    UINT64                              m_SrcOffset;
    UINT64                              m_CopyBytes;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT  m_Layout = {};
    UINT64                              m_RowSizeInBytes  = 0;
    UINT                                m_RowCount        = 0;
    UINT64                              m_RequiredSize    = 0;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

//-----------------------------------------------------------------------------
//      アップロードテクスチャリソースを生成します.
//-----------------------------------------------------------------------------
bool CreateUploadTexture
(
    ID3D12Resource*     pDest,
    const ResTexture&   resource,
    IUploadTexture**    ppResource
)
{
    auto instance = new TextureUploadResource();
    if (!instance->Init(GetD3D12Device(), pDest, resource))
    {
        delete instance;
        return false;
    }

    *ppResource = instance;
    return true;
}

//-----------------------------------------------------------------------------
//      アップロードバッファリソースを生成します.
//-----------------------------------------------------------------------------
bool CreateUploadBuffer
(
    ID3D12Resource*     pDest,
    const void*         resource,
    IUploadBuffer**     ppResource
)
{
    auto instance = new BufferUploadResource();
    if (!instance->Init(GetD3D12Device(), pDest, resource))
    {
        delete instance;
        return false;
    }

    *ppResource = instance;
    return true;
}

} // namespace asdx
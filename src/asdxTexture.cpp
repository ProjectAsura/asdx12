//-----------------------------------------------------------------------------
// File : asdxTexture.cpp
// Desc : Texture.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <atomic>
#include <vector>
#include <asdxTexture.h>
#include <asdxLogger.h>


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
class TextureUploadResource : public IUploadResource
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
            std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts;
            layouts.resize(count);

            std::vector<UINT> rows;
            rows.resize(count);

            std::vector<UINT64> rowSizeInBytes;
            rowSizeInBytes.resize(count);

            m_Commands.resize(count);

            UINT64 requiredSize = 0;
            pDevice->GetCopyableFootprints(
                &dstDesc, 0, count, 0, layouts.data(), rows.data(), rowSizeInBytes.data(), &requiredSize);

            BYTE* pData = nullptr;
            auto hr = m_pSrcResource->Map(0, nullptr, reinterpret_cast<void**>(&pData));
            if (FAILED(hr))
            {
                ELOG("Error : ID3D12Resource::Map() Failed. errcode = 0x%x", hr);
                return false;
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
                    dstData.pData       = pData + layouts[idx].Offset;
                    dstData.RowPitch    = layouts[idx].Footprint.RowPitch;
                    dstData.SlicePitch  = SIZE_T(layouts[idx].Footprint.RowPitch) * SIZE_T(rows[idx]);

                    CopySubresource(
                        &dstData,
                        &srcData,
                        SIZE_T(rowSizeInBytes[idx]),
                        rows[idx],
                        layouts[idx].Footprint.Depth);

                    m_Commands[idx].Dst.pResource           = m_pDstResource;
                    m_Commands[idx].Dst.Type                = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                    m_Commands[idx].Dst.PlacedFootprint     = {};
                    m_Commands[idx].Dst.SubresourceIndex    = idx;

                    m_Commands[idx].Src.pResource           = m_pSrcResource.GetPtr();
                    m_Commands[idx].Src.Type                = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                    m_Commands[idx].Src.PlacedFootprint     = layouts[idx];

                    idx++;
                }
            }

            layouts.clear();
            layouts.shrink_to_fit();

            rows.clear();
            rows.shrink_to_fit();

            rowSizeInBytes.clear();
            rowSizeInBytes.shrink_to_fit();
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
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
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

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    std::atomic<uint32_t>                       m_RefCount;
    ID3D12Resource*                             m_pDstResource;
    asdx::RefPtr<ID3D12Resource>                m_pSrcResource;
    std::vector<CopyTextureRegionCommand>       m_Commands;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

///////////////////////////////////////////////////////////////////////////////
// Texture class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Texture::Texture()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Texture::~Texture()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Texture::Init
(
    GraphicsDevice&     device,
    const ResTexture&   resource,
    IUploadResource**   ppUploadResource
)
{
    if (ppUploadResource == nullptr)
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    auto pD3DDevice = device.GetDevice();

    auto dimension  = D3D12_RESOURCE_DIMENSION_UNKNOWN;
    auto isCube     = false;
    auto depth      = 1;
    auto format     = DXGI_FORMAT(resource.Format);

#if ASDX_IS_SCARLETT
    auto mostDetailedMip = resourc.MipMapCount - 1;
#else
    auto mostDetailedMip = 0u;
#endif

    D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
    viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    {
        D3D12_HEAP_PROPERTIES props = {
            D3D12_HEAP_TYPE_DEFAULT,
            D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            D3D12_MEMORY_POOL_UNKNOWN,
            1,
            1
        };

        switch(resource.Dimension)
        {
        case TEXTURE_DIMENSION_1D:
            {
                dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
                if (resource.SurfaceCount > 1)
                {
                    viewDesc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                    viewDesc.Format                             = format;
                    viewDesc.Texture1DArray.ArraySize           = resource.SurfaceCount;
                    viewDesc.Texture1DArray.FirstArraySlice     = 0;
                    viewDesc.Texture1DArray.MipLevels           = resource.MipMapCount;
                    viewDesc.Texture1DArray.MostDetailedMip     = mostDetailedMip;
                    viewDesc.Texture1DArray.ResourceMinLODClamp = 0;
                }
                else
                {
                    viewDesc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE1D;
                    viewDesc.Format                         = format;
                    viewDesc.Texture1D.MipLevels            = resource.MipMapCount;
                    viewDesc.Texture1D.MostDetailedMip      = mostDetailedMip;
                    viewDesc.Texture1D.ResourceMinLODClamp  = 0;
                }
            }
            break;

        case TEXTURE_DIMENSION_2D:
            {
                dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                if (resource.SurfaceCount > 1)
                {
                    viewDesc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                    viewDesc.Format                             = format;
                    viewDesc.Texture2DArray.ArraySize           = resource.SurfaceCount;
                    viewDesc.Texture2DArray.FirstArraySlice     = 0;
                    viewDesc.Texture2DArray.MipLevels           = resource.MipMapCount;
                    viewDesc.Texture2DArray.MostDetailedMip     = mostDetailedMip;
                    viewDesc.Texture2DArray.PlaneSlice          = 0;
                    viewDesc.Texture2DArray.ResourceMinLODClamp = 0;
                }
                else
                {
                    viewDesc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2D;
                    viewDesc.Format                             = format;
                    viewDesc.Texture2D.MipLevels                = resource.MipMapCount;
                    viewDesc.Texture2D.MostDetailedMip          = mostDetailedMip;
                    viewDesc.Texture2D.PlaneSlice               = 0;
                    viewDesc.Texture2D.ResourceMinLODClamp      = 0;
                }
            }
            break;

        case TEXTURE_DIMENSION_3D:
            {
                dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
                depth = resource.Depth;
            }
            break;

        case TEXTURE_DIMENSION_CUBE:
            {
                dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                depth = resource.SurfaceCount;
            }
            break;
        }

        D3D12_RESOURCE_DESC desc = {
            dimension,
            0,
            resource.Width,
            resource.Height,
            UINT16(depth),
            UINT16(resource.MipMapCount),
            format,
            { 1, 0 },
            D3D12_TEXTURE_LAYOUT_UNKNOWN,
            D3D12_RESOURCE_FLAG_NONE
        };

        auto hr = pD3DDevice->CreateCommittedResource(
            &props,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(m_pResource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateCommitedResource() Failed. errcode = 0x%x", hr);
            return false;
        }

        m_pResource->SetName(L"asdxTexture");
    }

    // シェーダリソースビューの生成.
    {
        auto ret = device.AllocHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_pDescriptor.GetAddress());
        if (!ret)
        {
            ELOG("Error : GraphicsDevice::AllocHandle() Failed.");
            return false;
        }

        pD3DDevice->CreateShaderResourceView(m_pResource.GetPtr(), &viewDesc, m_pDescriptor->GetHandleCPU());
    }

    // テクスチャアップロードリソースを生成します.
    auto uploadResource = new TextureUploadResource();
    if (!uploadResource->Init(device.GetDevice(), m_pResource.GetPtr(), resource))
    {
        uploadResource->Release();
        uploadResource = nullptr;
        return false;
    }

    *ppUploadResource = uploadResource;

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Texture::Term()
{
    m_pDescriptor.Reset();
    m_pResource  .Reset();
}

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* Texture::GetResource() const
{ return m_pResource.GetPtr(); }

//-----------------------------------------------------------------------------
//      ディスクリプタを取得します.
//-----------------------------------------------------------------------------
const Descriptor* Texture::GetDescriptor() const
{ return m_pDescriptor.GetPtr(); }

} // namespace asdx

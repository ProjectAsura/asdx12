//-----------------------------------------------------------------------------
// File : asdxStructuredBuffer.cpp
// Desc : Structured Buffer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <atomic>
#include <asdxStructuredBuffer.h>
#include <asdxLogger.h>


namespace {

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
// BufferUploadResource class
///////////////////////////////////////////////////////////////////////////////
class BufferUploadResource : public IUploadResource
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
            D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout = {};
            UINT64  rowSizeInBytes  = 0;
            UINT    rowCount        = 0;
            UINT64  requiredSize    = 0;

            pDevice->GetCopyableFootprints(
                &dstDesc, 0, 1, 0, &layout, &rowCount, &rowSizeInBytes, &requiredSize);

            BYTE* pData = nullptr;
            auto hr = m_pSrcResource->Map(0, nullptr, reinterpret_cast<void**>(&pData));
            if (FAILED(hr))
            {
                ELOG("Error : ID3D12Resource::Map() Failed. errcode = 0x%x", hr);
                return false;
            }

            D3D12_SUBRESOURCE_DATA srcData = {};
            srcData.pData       = pInitData;
            srcData.RowPitch    = dstDesc.Width;
            srcData.SlicePitch  = dstDesc.Width * dstDesc.Height;

            D3D12_MEMCPY_DEST dstData = {};
            dstData.pData       = pData + layout.Offset;
            dstData.RowPitch    = layout.Footprint.RowPitch;
            dstData.SlicePitch  = SIZE_T(layout.Footprint.RowPitch) * SIZE_T(rowCount);

            CopySubresource(
                &dstData,
                &srcData,
                SIZE_T(rowSizeInBytes),
                rowCount,
                layout.Footprint.Depth);

            m_pSrcResource->Unmap(0, nullptr);

            m_SrcOffset = layout.Offset;
            m_CopyBytes = layout.Footprint.Width;
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
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
        barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_DEST;
        //pCmdList->ResourceBarrier( 1, &barrier );

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

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    std::atomic<uint32_t>           m_RefCount;
    ID3D12Resource*                 m_pDstResource;
    asdx::RefPtr<ID3D12Resource>    m_pSrcResource;
    UINT64                          m_SrcOffset;
    UINT64                          m_CopyBytes;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};


///////////////////////////////////////////////////////////////////////////////
// StructuredBuffer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
StructuredBuffer::StructuredBuffer()
: m_pResource   ()
, m_pDescriptor ()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
StructuredBuffer::~StructuredBuffer()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool StructuredBuffer::Init(uint64_t count, uint32_t stride, D3D12_RESOURCE_STATES state)
{
    auto pDevice = GfxDevice().GetDevice();

    if (pDevice == nullptr || count == 0 || stride == 0)
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    uint64_t size = count * stride;

    D3D12_HEAP_PROPERTIES prop = {};
    prop.Type                   = D3D12_HEAP_TYPE_DEFAULT;
    prop.CPUPageProperty        = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    prop.MemoryPoolPreference   = D3D12_MEMORY_POOL_UNKNOWN;
    prop.VisibleNodeMask        = 1;
    prop.CreationNodeMask       = 1;

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width              = size;
    desc.Height             = 1;
    desc.DepthOrArraySize   = 1;
    desc.Format             = DXGI_FORMAT_UNKNOWN;
    desc.MipLevels          = 1;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

    auto flags = D3D12_HEAP_FLAG_NONE;

    auto hr = pDevice->CreateCommittedResource(
        &prop,
        flags,
        &desc,
        state,
        nullptr,
        IID_PPV_ARGS(m_pResource.GetAddress()));
    if ( FAILED(hr) )
    {
        ELOG("Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr);
        return false;
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
    viewDesc.Format                     = DXGI_FORMAT_UNKNOWN;
    viewDesc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
    viewDesc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    viewDesc.Buffer.FirstElement        = 0;
    viewDesc.Buffer.NumElements         = UINT(count);
    viewDesc.Buffer.StructureByteStride = stride;
    viewDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;

    if (!GfxDevice().AllocHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_pDescriptor.GetAddress()))
    {
        ELOG("Error : GraphicsDevice::AllocHandle() Failed.");
        return false;
    }

    pDevice->CreateShaderResourceView(m_pResource.GetPtr(), &viewDesc, m_pDescriptor->GetHandleCPU());

    return true;
}

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool StructuredBuffer::Init
(
    uint64_t            count,
    uint32_t            stride,
    const void*         pInitData
)
{
    if (!Init(count, stride, D3D12_RESOURCE_STATE_COPY_DEST))
    { return false;  }

    auto uploadResource = new BufferUploadResource();
    if (!uploadResource->Init(GfxDevice().GetDevice(), m_pResource.GetPtr(), pInitData))
    {
        uploadResource->Release();
        uploadResource = nullptr;
        return false;
    }

    GfxDevice().PushToResourceUploader(uploadResource);

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void StructuredBuffer::Term()
{
    auto descriptor = m_pDescriptor.GetPtr();
    if (descriptor != nullptr)
    {
        descriptor->AddRef();
        GfxDevice().PushToDescriptorDisposer(descriptor);
    }

    auto resource = m_pResource.GetPtr();
    if (resource != nullptr)
    {
        resource->AddRef();
        GfxDevice().PushToResourceDisposer(resource);
    }

    m_pResource  .Reset();
    m_pDescriptor.Reset();
}

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* StructuredBuffer::GetResource() const
{ return m_pResource.GetPtr(); }

//-----------------------------------------------------------------------------
//      ディスクリプタを取得します.
//-----------------------------------------------------------------------------
const Descriptor* StructuredBuffer::GetDescriptor() const
{ return m_pDescriptor.GetPtr(); }

} // namespace asdx

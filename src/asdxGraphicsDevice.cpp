//-----------------------------------------------------------------------------
// File : asdxGraphicsDevice.cpp
// Desc : Graphics Device.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxGraphicsDevice.h>
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
// GraphicsDevice class
///////////////////////////////////////////////////////////////////////////////
GraphicsDevice GraphicsDevice::s_Instance = {};

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
GraphicsDevice::GraphicsDevice()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
GraphicsDevice::~GraphicsDevice()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      唯一のインスタンスを取得します.
//-----------------------------------------------------------------------------
GraphicsDevice& GraphicsDevice::Instance()
{ return s_Instance; }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool GraphicsDevice::Init(const Desc* pDesc)
{
    if (pDesc->EnableDebug)
    {
        asdx::RefPtr<ID3D12Debug> debug;
        auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddress()));
        if (SUCCEEDED(hr))
        {
            hr = debug->QueryInterface(IID_PPV_ARGS(m_pDebug.GetAddress()));
            if (SUCCEEDED(hr))
            {
                m_pDebug->EnableDebugLayer();
            #if 0 // 呼び出すとメッシュシェーダが表示されなくなるので，封印.
                m_pDebug->SetEnableGPUBasedValidation(TRUE);
            #endif
            }
        }
    }

    if (pDesc->EnableDRED)
    {
        // DRED有効化.
        asdx::RefPtr<ID3D12DeviceRemovedExtendedDataSettings> dred;
        auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(dred.GetAddress()));
        if (SUCCEEDED(hr))
        {
            dred->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
            dred->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
        }
    }

    // DXGIファクトリを生成.
    {
        uint32_t flags = 0;
        if (pDesc->EnableDebug)
        { flags |= DXGI_CREATE_FACTORY_DEBUG; }

        RefPtr<IDXGIFactory2> factory;
        auto hr = CreateDXGIFactory2( flags, IID_PPV_ARGS(factory.GetAddress()) );
        if ( FAILED(hr) )
        {
            ELOG("Error : CreateDXGIFactory2() Failed. errcode = 0x%x", hr);
            return false;
        }

        hr = factory->QueryInterface(IID_PPV_ARGS(m_pFactory.GetAddress()));
        if ( FAILED(hr) )
        {
            ELOG("Error : QueryInterface() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    // デバイス生成.
    {
        asdx::RefPtr<ID3D12Device> device;
        auto hr = D3D12CreateDevice( nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(device.GetAddress()) );
        if (FAILED(hr))
        {
            ELOG("Error : D3D12CreateDevice() Failed. errcode = 0x%x", hr);
            return false;
        }

        // ID3D12Device8に変換.
        hr = device->QueryInterface(IID_PPV_ARGS(m_pDevice.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : QueryInterface() Failed. errcode = 0x%x", hr);
            return false;
        }

        m_pDevice->SetName(L"asdxDevice");

        // ID3D12InfoQueueに変換.
        if (pDesc->EnableDebug)
        {
            hr = m_pDevice->QueryInterface(IID_PPV_ARGS(m_pInfoQueue.GetAddress()));
            if (SUCCEEDED(hr))
            {
                // エラー発生時にブレークさせる.
                m_pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);

                // 警告発生時にブレークさせる.
                m_pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

                // クリア値が違うという警告は無効化しておく.
                m_pInfoQueue->SetBreakOnID(D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE, FALSE);
                m_pInfoQueue->SetBreakOnID(D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE, FALSE);
            }
        }
    }

    // 定数バッファ・シェーダリソース・アンオーダードアクセスビュー用ディスクリプタヒープ.
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = pDesc->MaxShaderResourceCount;
        desc.Type  = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if ( !m_DescriptorHeap[desc.Type].Init(m_pDevice.GetPtr(), &desc ) )
        {
            ELOG("Error : DescriptorHeap::Init() Failed.");
            return false;
        }
    }

    // サンプラー用ディスクリプタヒープ.
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = pDesc->MaxSamplerCount;
        desc.Type  = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if ( !m_DescriptorHeap[desc.Type].Init(m_pDevice.GetPtr(), &desc ) )
        {
            ELOG("Error : DescriptorHeap::Init() Failed");
            return false;
        }
    }

    // レンダーターゲットビュー用ディスクリプタヒープ.
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = pDesc->MaxColorTargetCount;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        if ( !m_DescriptorHeap[desc.Type].Init(m_pDevice.GetPtr(), &desc ) )
        {
            ELOG("Error : DescriptorHeap::Init() Failed.");
            return false;
        }
    }

    // 深度ステンシルビュー用ディスクリプタヒープ.
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = pDesc->MaxDepthTargetCount;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        if ( !m_DescriptorHeap[desc.Type].Init(m_pDevice.GetPtr(), &desc ) )
        {
            ELOG("Error : DescriptorHeap::Init() Failed");
            return false;
        }
    }

    // グラフィックスキューの生成.
    auto ret = CommandQueue::Create(
        m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_DIRECT, m_pGraphicsQueue.GetAddress());
    if (!ret)
    {
        ELOG("Error : Queue::Create() Failed.");
        return false;
    }

    // コンピュートキューの生成.
    ret = CommandQueue::Create(
        m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_COMPUTE, m_pComputeQueue.GetAddress());
    if (!ret)
    {
        ELOG("Error : Queue::Create() Failed.");
        return false;
    }

    // コピーキューの生成.
    ret = CommandQueue::Create(
        m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_COPY, m_pCopyQueue.GetAddress());
    if (!ret)
    {
        ELOG("Error : Queue::Create() Failed.");
        return false;
    }

    // ビデオデコードキューの生成.
    ret = CommandQueue::Create(
        m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE, m_pVideoDecodeQueue.GetAddress());
    if (!ret)
    {
        ELOG("Error : Queue::Create() Failed.");
        return false;
    }

    // ビデオプロセスキューの生成.
    ret = CommandQueue::Create(
        m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS, m_pVideoProcessQueue.GetAddress());
    if (!ret)
    {
        ELOG("Error : Queue::Create() Failed.");
        return false;
    }

    // ビデオエンコードキューの生成.
    ret = CommandQueue::Create(
        m_pDevice.GetPtr(), D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE, m_pVideoEncodeQueue.GetAddress());
    if (!ret)
    {
        ELOG("Error : Queue::Create() Failed.");
        return false;
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void GraphicsDevice::Term()
{
    m_ResourceUploader      .Clear();
    m_ResourceDisposer      .Clear();
    m_DescriptorDisposer    .Clear();
    m_PipelineStateDisposer .Clear();

    m_pGraphicsQueue    .Reset();
    m_pComputeQueue     .Reset();
    m_pCopyQueue        .Reset();
    m_pVideoDecodeQueue .Reset();
    m_pVideoProcessQueue.Reset();
    m_pVideoEncodeQueue .Reset();

    for(auto i=0; i<4; ++i)
    {
        m_DescriptorHeap[i].Term();
    }

    m_pDevice   .Reset();
    m_pInfoQueue.Reset();
    m_pDebug    .Reset();
    m_pFactory  .Reset();
}

//-----------------------------------------------------------------------------
//      ID3D12Device8を取得します.
//-----------------------------------------------------------------------------
ID3D12Device8* GraphicsDevice::GetDevice() const
{ return m_pDevice.GetPtr(); }

//-----------------------------------------------------------------------------
//      IDXGIFactory7を取得します.
//-----------------------------------------------------------------------------
IDXGIFactory7* GraphicsDevice::GetFactory() const
{ return m_pFactory.GetPtr(); }

//-----------------------------------------------------------------------------
//      グラフィックスキューを取得します.
//-----------------------------------------------------------------------------
CommandQueue* GraphicsDevice::GetGraphicsQueue() const
{ return m_pGraphicsQueue.GetPtr(); }

//-----------------------------------------------------------------------------
//      コンピュートキューを取得します.
//-----------------------------------------------------------------------------
CommandQueue* GraphicsDevice::GetComputeQueue() const
{ return m_pComputeQueue.GetPtr(); }

//-----------------------------------------------------------------------------
//      コピーキューを取得します.
//-----------------------------------------------------------------------------
CommandQueue* GraphicsDevice::GetCopyQueue() const
{ return m_pCopyQueue.GetPtr(); }

//-----------------------------------------------------------------------------
//      ビデオデコードキューを取得します.
//-----------------------------------------------------------------------------
CommandQueue* GraphicsDevice::GetVideoDecodeQueue() const
{ return m_pVideoDecodeQueue.GetPtr(); }

//-----------------------------------------------------------------------------
//      ビデオプロセスキューを取得します.
//-----------------------------------------------------------------------------
CommandQueue* GraphicsDevice::GetVideoProcessQueue() const
{ return m_pVideoProcessQueue.GetPtr(); }

//-----------------------------------------------------------------------------
//      ビデオエンコードキューを取得します.
//-----------------------------------------------------------------------------
CommandQueue* GraphicsDevice::GetVideoEncodeQueue() const
{ return m_pVideoEncodeQueue.GetPtr(); }

//-----------------------------------------------------------------------------
//      ディスクリプターを確保します.
//-----------------------------------------------------------------------------
bool GraphicsDevice::AllocHandle(int index, Descriptor** ppResult)
{
    std::lock_guard<std::mutex> guard(m_Mutex);

    if (index < 0 || index >= 4)
    { return false; }

    auto descriptor = m_DescriptorHeap[index].CreateDescriptor();
    if (descriptor == nullptr)
    { return false; }

    *ppResult = descriptor;

    return true;
}

//-----------------------------------------------------------------------------
//      アロー演算子です.
//-----------------------------------------------------------------------------
ID3D12Device8* GraphicsDevice::operator-> () const
{ return m_pDevice.GetPtr(); }

//-----------------------------------------------------------------------------
//      ディスクリプタヒープを設定します.
//-----------------------------------------------------------------------------
void GraphicsDevice::SetDescriptorHeaps(ID3D12GraphicsCommandList* pCmdList)
{
    if (pCmdList == nullptr)
    { return; }

    ID3D12DescriptorHeap* pHeaps[] = {
        m_DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].GetD3D12DescriptorHeap(),
        m_DescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].GetD3D12DescriptorHeap()
    };

    pCmdList->SetDescriptorHeaps(2, pHeaps);
}

//-----------------------------------------------------------------------------
//      コマンドキューの実行完了を待機します.
//-----------------------------------------------------------------------------
void GraphicsDevice::WaitIdle()
{
    if (m_pGraphicsQueue.GetPtr() != nullptr)
    {
        auto waitPoint = m_pGraphicsQueue->Signal();
        m_pGraphicsQueue->Sync(waitPoint);
    }

    if (m_pComputeQueue.GetPtr() != nullptr)
    {
        auto waitPoint = m_pComputeQueue->Signal();
        m_pComputeQueue->Sync(waitPoint);
    }

    if (m_pCopyQueue.GetPtr() != nullptr)
    {
        auto waitPoint = m_pCopyQueue->Signal();
        m_pCopyQueue->Sync(waitPoint);
    }

    if (m_pVideoDecodeQueue.GetPtr() != nullptr)
    {
        auto waitPoint = m_pVideoDecodeQueue->Signal();
        m_pVideoDecodeQueue->Sync(waitPoint);
    }

    if (m_pVideoEncodeQueue.GetPtr() != nullptr)
    {
        auto waitPoint = m_pVideoEncodeQueue->Signal();
        m_pVideoEncodeQueue->Sync(waitPoint);
    }

    if (m_pVideoProcessQueue.GetPtr() != nullptr)
    {
        auto waitPoint = m_pVideoProcessQueue->Signal();
        m_pVideoProcessQueue->Sync(waitPoint);
    }
}

//-----------------------------------------------------------------------------
//      リソースアップローダーに追加します.
//-----------------------------------------------------------------------------
void GraphicsDevice::PushToUploader(IUploadResource* pResource, uint8_t lifeTime)
{ m_ResourceUploader.Push(pResource, lifeTime); }

//-----------------------------------------------------------------------------
//      リソースディスポーザーに追加します.
//-----------------------------------------------------------------------------
void GraphicsDevice::PushToDisposer(ID3D12Resource*& pResource, uint8_t lifeTime)
{ m_ResourceDisposer.Push(pResource, lifeTime); }

//-----------------------------------------------------------------------------
//      ディスクリプタディスポーザーに追加します.
//-----------------------------------------------------------------------------
void GraphicsDevice::PushToDisposer(Descriptor*& pDescriptor, uint8_t lifeTime)
{ m_DescriptorDisposer.Push(pDescriptor, lifeTime); }

//-----------------------------------------------------------------------------
//      パイプラインステートディスポーザーに追加します.
//-----------------------------------------------------------------------------
void GraphicsDevice::PushToDisposer(ID3D12PipelineState*& pPipelineState, uint8_t lifeTime)
{ m_PipelineStateDisposer.Push(pPipelineState, lifeTime); }

//-----------------------------------------------------------------------------
//      アップロードコマンドを設定します.
//-----------------------------------------------------------------------------
void GraphicsDevice::SetUploadCommand(ID3D12GraphicsCommandList* pCmdList)
{ m_ResourceUploader.Upload(pCmdList); }

//-----------------------------------------------------------------------------
//      フレーム同期を取ります.
//-----------------------------------------------------------------------------
void GraphicsDevice::FrameSync()
{
    m_ResourceUploader      .FrameSync();
    m_ResourceDisposer      .FrameSync();
    m_DescriptorDisposer    .FrameSync();
    m_PipelineStateDisposer .FrameSync();
}

//-----------------------------------------------------------------------------
//      バッファ更新リソースを生成し登録します.
//-----------------------------------------------------------------------------
bool GraphicsDevice::UpdateBuffer(ID3D12Resource* pDstResource, const void* pInitData)
{
    if (pDstResource == nullptr || pInitData== nullptr)
    {
        ELOGA("Error : Invalid Argument.");
        return false;
    }

    auto instance = new BufferUploadResource();
    if (!instance->Init(m_pDevice.GetPtr(), pDstResource, pInitData))
    {
        instance->Release();
        instance = nullptr;
        ELOGA("Error : BufferUploadResource::Init() Failed.");
        return false;
    }

    PushToUploader(instance);
    return true;
}

//-----------------------------------------------------------------------------
//      テクスチャ更新リソースを生成し登録します.
//-----------------------------------------------------------------------------
bool GraphicsDevice::UpdateTexture(ID3D12Resource* pDstResource, const ResTexture& resource)
{
    if (pDstResource == nullptr || resource.pResources == nullptr)
    {
        ELOGA("Error : Invalid Argument");
        return false;
    }

    auto instance = new TextureUploadResource();
    if (!instance->Init(m_pDevice.GetPtr(), pDstResource, resource))
    {
        instance->Release();
        instance = nullptr;
        ELOGA("Error : TextureUploadResource::Init() Failed.");
        return false;
    }

    PushToUploader(instance);
    return true;
}

} // namespace asdx

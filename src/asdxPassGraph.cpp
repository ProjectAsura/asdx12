//-----------------------------------------------------------------------------
// File : asdxPassGraph.cpp
// Desc : Pass Graph System.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <atomic>
#include <asdxPassGraph.h>
#include <asdxFrameHeap.h>
#include <asdxCommandList.h>
#include <asdxDisposer.h>
#include <asdxHash.h>
#include <asdxLogger.h>
#include <asdxGraphicsDevice.h>
#include <asdxLfuCache.h>


namespace asdx {

//-----------------------------------------------------------------------------
//      ハッシュ値を生成します.
//-----------------------------------------------------------------------------
uint32_t CreateHashFromDesc(const PassResourceDesc& value)
{
    return Fnv1a(
        uint32_t(sizeof(value)),
        reinterpret_cast<const uint8_t*>(&value))
        .GetHash();
}

///////////////////////////////////////////////////////////////////////////////
// PassResource class
///////////////////////////////////////////////////////////////////////////////
class PassResource
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
    PassResource()
    : m_RefCount        (1)
    , m_DescriptorRTV   (nullptr)
    , m_DescriptorDSV   (nullptr)
    , m_DescriptorRes   (nullptr)
    , m_Resource        (nullptr)
    , m_Hash            (0)
    , m_Import          (false)
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~PassResource()
    {
        if (m_DescriptorRTV != nullptr)
        {
            m_DescriptorRTV->Release();
            m_DescriptorRTV = nullptr;
        }

        if (m_DescriptorDSV != nullptr)
        {
            m_DescriptorDSV->Release();
            m_DescriptorDSV = nullptr;
        }

        if (m_DescriptorRes != nullptr)
        {
            m_DescriptorRes->Release();
            m_DescriptorRes = nullptr;
        }

        if (!m_Import)
        {
            m_Resource->Release();
            m_Resource = nullptr;
        }
    }

    //-------------------------------------------------------------------------
    //! @brief      解放処理を行います.
    //-------------------------------------------------------------------------
    void Release()
    {
        m_RefCount--;
        if (m_RefCount == 0)
        { delete this; }
    }

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //-------------------------------------------------------------------------
    bool Init(const PassResourceDesc& value)
    {
        {
            D3D12_RESOURCE_DESC desc = {};
            switch(value.Dimension)
            {
            case PASS_RESOURCE_DIMENSION_BUFFER:
                { desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; }
                break;

            case PASS_RESOURCE_DIMENSION_1D:
                { desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D; }
                break;

            case PASS_RESOURCE_DIMENSION_2D:
                { desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; }
                break;

            case PASS_RESOURCE_DIMENSION_3D:
                { desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D; }
                break;
            }

            auto rtv = false;
            auto dsv = false;
            auto uav = false;

            D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
            if (value.Usage & PASS_RESOURCE_USAGE_RTV)
            {
                flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
                rtv = true;
            }
            if (value.Usage & PASS_RESOURCE_USAGE_DSV)
            {
                flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
                dsv = true;
            }
            if (value.Usage & PASS_RESOURCE_USAGE_UAV)
            {
                flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
                uav = true;
            }

            desc.Width              = value.Width;
            desc.Height             = value.Height;
            desc.DepthOrArraySize   = value.DepthOrArraySize;
            desc.MipLevels          = value.MipLevels;
            desc.Format             = value.Format;
            desc.Flags              = flags;
            desc.SampleDesc.Count   = 1;
            desc.SampleDesc.Quality = 0;
            desc.Layout             = (desc.Dimension == PASS_RESOURCE_DIMENSION_BUFFER) 
                                      ? D3D12_TEXTURE_LAYOUT_ROW_MAJOR 
                                      : D3D12_TEXTURE_LAYOUT_64KB_STANDARD_SWIZZLE;

            D3D12_CLEAR_VALUE clearValue;
            clearValue.Format = value.Format;
            if (value.Usage & PASS_RESOURCE_USAGE_DSV)
            {
                clearValue.DepthStencil.Depth   = value.ClearDepth;
                clearValue.DepthStencil.Stencil = value.ClearStencil;
            }
            else
            {
                clearValue.Color[0] = value.ClearColor.x;
                clearValue.Color[1] = value.ClearColor.y;
                clearValue.Color[2] = value.ClearColor.z;
                clearValue.Color[3] = value.ClearColor.w;
            }

            D3D12_HEAP_PROPERTIES props = {};
            props.Type                  = D3D12_HEAP_TYPE_DEFAULT;
            props.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            props.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
            props.CreationNodeMask      = 0;
            props.VisibleNodeMask       = 0;

            auto hr = GfxDevice()->CreateCommittedResource(
                &props,
                D3D12_HEAP_FLAG_NONE,
                &desc, 
                D3D12_RESOURCE_STATE_COMMON,
                &clearValue,
                IID_PPV_ARGS(&m_Resource));
            if (FAILED(hr))
            {
                ELOG("Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr);
                return false;
            }

            if (rtv)
            {
                if (!CreateRTV(value))
                { return false; }
            }
            else if (dsv)
            {
                if (!CreateDSV(value))
                { return false; }
            }

            if (uav)
            {
                if (!CreateUAV(value))
                { return false; }
            }

            if (!CreateSRV(value))
            { return false; }
        }

        m_Hash   = CreateHashFromDesc(value);
        m_Import = false;

        return true;
    }

    //-------------------------------------------------------------------------
    //! @brief      レンダーターゲットビューのディスクリプタハンドルを取得します.
    //-------------------------------------------------------------------------
    D3D12_CPU_DESCRIPTOR_HANDLE GetHandleRTV() const
    {
        if (m_DescriptorRTV != nullptr)
        { return m_DescriptorRTV->GetHandleCPU(); }

        return D3D12_CPU_DESCRIPTOR_HANDLE();
    }

    //-------------------------------------------------------------------------
    //! @brief      深度ステンシルビューのディスクリプタハンドルを取得します.
    //-------------------------------------------------------------------------
    D3D12_CPU_DESCRIPTOR_HANDLE GetHandleDSV() const
    {
        if (m_DescriptorDSV != nullptr)
        { return m_DescriptorDSV->GetHandleCPU(); }

        return D3D12_CPU_DESCRIPTOR_HANDLE();
    }

    //-------------------------------------------------------------------------
    //! @brief      UAV/SRVのディスクリプタハンドルを取得します.
    //-------------------------------------------------------------------------
    D3D12_GPU_DESCRIPTOR_HANDLE GetHandleRes() const
    {
        if (m_DescriptorRes != nullptr)
        { return m_DescriptorRes->GetHandleGPU(); }

        return D3D12_GPU_DESCRIPTOR_HANDLE();
    }

    //-------------------------------------------------------------------------
    //! @brief      GPU仮想アドレスを取得します.
    //-------------------------------------------------------------------------
    D3D12_GPU_VIRTUAL_ADDRESS GetVirtualAddress() const
    {
        if (m_Resource != nullptr)
        { return m_Resource->GetGPUVirtualAddress(); }

        return D3D12_GPU_VIRTUAL_ADDRESS();
    }

    //-------------------------------------------------------------------------
    //! @brief      構成設定を取得します.
    //-------------------------------------------------------------------------
    D3D12_RESOURCE_DESC GetDesc() const
    {
        if (m_Resource != nullptr)
        { return m_Resource->GetDesc(); }

        return D3D12_RESOURCE_DESC();
    }

    bool Import(ID3D12Resource* pResource, D3D12_RESOURCE_STATES state)
    {
        m_Import = pResource;
    }

    //-------------------------------------------------------------------------
    //! @brief      等価比較演算子です.
    //-------------------------------------------------------------------------
    bool operator == (const PassResource& value) const
    { return m_Hash == value.m_Hash; }

    //-------------------------------------------------------------------------
    //! @brief      非等価比較演算子です.
    //-------------------------------------------------------------------------
    bool operator != (const PassResource& value) const
    { return m_Hash != value.m_Hash; }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    std::atomic<int>    m_RefCount      = 1;
    Descriptor*         m_DescriptorRTV = nullptr;
    Descriptor*         m_DescriptorDSV = nullptr;
    Descriptor*         m_DescriptorRes = nullptr;
    ID3D12Resource*     m_Resource      = nullptr;
    PassResourceDesc    m_Desc          = {};
    uint32_t            m_Hash          = 0;
    bool                m_Import        = false;

    //=========================================================================
    // private methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      レンダーターゲットビューを生成します.
    //-------------------------------------------------------------------------
    bool CreateRTV(const PassResourceDesc& value)
    {
        D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
        switch(value.Dimension)
        {
        case PASS_RESOURCE_DIMENSION_BUFFER:
            { 
                viewDesc.ViewDimension          = D3D12_RTV_DIMENSION_BUFFER;
                viewDesc.Buffer.FirstElement    = 0;
                viewDesc.Buffer.NumElements     = uint32_t(value.Width / value.Stride);
            }
            break;

        case PASS_RESOURCE_DIMENSION_1D:
            {
                if (value.DepthOrArraySize > 1)
                {
                    viewDesc.ViewDimension                  = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                    viewDesc.Texture1DArray.ArraySize       = value.DepthOrArraySize;
                    viewDesc.Texture1DArray.FirstArraySlice = 0;
                    viewDesc.Texture1DArray.MipSlice        = 0;
                }
                else
                {
                    viewDesc.ViewDimension      = D3D12_RTV_DIMENSION_TEXTURE1D;
                    viewDesc.Texture1D.MipSlice = 0;
                }
            }
            break;

        case PASS_RESOURCE_DIMENSION_2D:
            {
                {
                    if (value.DepthOrArraySize > 1)
                    {
                        viewDesc.ViewDimension                  = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                        viewDesc.Texture2DArray.ArraySize       = value.DepthOrArraySize;
                        viewDesc.Texture2DArray.FirstArraySlice = 0;
                        viewDesc.Texture2DArray.MipSlice        = 0;
                        viewDesc.Texture2DArray.PlaneSlice      = 0;
                    }
                    else
                    {
                        viewDesc.ViewDimension          = D3D12_RTV_DIMENSION_TEXTURE2D;
                        viewDesc.Texture2D.MipSlice     = 0;
                        viewDesc.Texture2D.PlaneSlice   = 0;
                    }
                }
            }
            break;

        case PASS_RESOURCE_DIMENSION_3D:
            {
                viewDesc.ViewDimension          = D3D12_RTV_DIMENSION_TEXTURE3D;
                viewDesc.Texture3D.FirstWSlice  = 0;
                viewDesc.Texture3D.MipSlice     = 0;
                viewDesc.Texture3D.WSize        = value.DepthOrArraySize;
            }
            break;
        }

        if (!GfxDevice().AllocHandle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, &m_DescriptorRTV))
        {
            ELOG("Error : GraphicsDevice::AllocHandle() Failed.");
            return false;
        }
        GfxDevice()->CreateRenderTargetView(m_Resource, &viewDesc, m_DescriptorRTV->GetHandleCPU());
        return true;
    }

    //-------------------------------------------------------------------------
    //! @brief      深度ステンシルビューを生成します.
    //-------------------------------------------------------------------------
    bool CreateDSV(const PassResourceDesc& value)
    {
        D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {};
        switch(value.Dimension)
        {
        case PASS_RESOURCE_DIMENSION_BUFFER:
            ELOG("Error : Invalid Argument.");
            return false;

        case PASS_RESOURCE_DIMENSION_1D:
            {
                if (value.DepthOrArraySize > 1)
                {
                    viewDesc.ViewDimension                  = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
                    viewDesc.Texture1DArray.ArraySize       = value.DepthOrArraySize;
                    viewDesc.Texture1DArray.FirstArraySlice = 0;
                    viewDesc.Texture1DArray.MipSlice        = 0;
                }
                else
                {
                    viewDesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE1D;
                    viewDesc.Texture1D.MipSlice = 0;
                }
            }
            break;

        case PASS_RESOURCE_DIMENSION_2D:
            {
                if (value.DepthOrArraySize > 1)
                {
                    viewDesc.ViewDimension                  = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                    viewDesc.Texture2DArray.ArraySize       = value.DepthOrArraySize;
                    viewDesc.Texture2DArray.FirstArraySlice = 0;
                    viewDesc.Texture2DArray.MipSlice        = 0;
                }
                else
                {
                    viewDesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
                    viewDesc.Texture2D.MipSlice = 0;
                }
            }
            break;

        case PASS_RESOURCE_DIMENSION_3D:
            ELOG("Error : Invalid Argument.");
            return false;
        }

        if (!GfxDevice().AllocHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, &m_DescriptorDSV))
        {
            ELOG("Error : GfxDevice::AllocHandle() Failed.");
            return false;
        }

        GfxDevice()->CreateDepthStencilView(m_Resource, &viewDesc, m_DescriptorDSV->GetHandleCPU());
        return true;
    }

    //-------------------------------------------------------------------------
    //! @brief      アンオーダードアクセスビューを生成します.
    //-------------------------------------------------------------------------
    bool CreateUAV(const PassResourceDesc& value)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = {};
        switch(value.Dimension)
        {
        case PASS_RESOURCE_DIMENSION_BUFFER:
            {
                viewDesc.ViewDimension                  = D3D12_UAV_DIMENSION_BUFFER;
                viewDesc.Buffer.CounterOffsetInBytes    = 0;
                viewDesc.Buffer.FirstElement            = 0;
                viewDesc.Buffer.NumElements             = uint32_t(value.Width / value.Stride);
                viewDesc.Buffer.StructureByteStride     = value.Stride;
                viewDesc.Buffer.Flags                   = (value.Stride == 1) ? D3D12_BUFFER_UAV_FLAG_RAW : D3D12_BUFFER_UAV_FLAG_NONE;
            }
            break;

        case PASS_RESOURCE_DIMENSION_1D:
            {
                if (value.DepthOrArraySize > 1)
                {
                    viewDesc.ViewDimension                  = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
                    viewDesc.Texture1DArray.ArraySize       = value.DepthOrArraySize;
                    viewDesc.Texture1DArray.FirstArraySlice = 0;
                    viewDesc.Texture1DArray.MipSlice        = 0;
                }
                else
                {
                    viewDesc.ViewDimension      = D3D12_UAV_DIMENSION_TEXTURE1D;
                    viewDesc.Texture1D.MipSlice = 0;
                }
            }
            break;

        case PASS_RESOURCE_DIMENSION_2D:
            {
                if (value.DepthOrArraySize > 1)
                {
                    viewDesc.ViewDimension                  = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                    viewDesc.Texture2DArray.ArraySize       = value.DepthOrArraySize;
                    viewDesc.Texture2DArray.FirstArraySlice = 0;
                    viewDesc.Texture2DArray.MipSlice        = 0;
                    viewDesc.Texture2DArray.PlaneSlice      = 0;
                }
                else
                {
                    viewDesc.ViewDimension          = D3D12_UAV_DIMENSION_TEXTURE2D;
                    viewDesc.Texture2D.MipSlice     = 0;
                    viewDesc.Texture2D.PlaneSlice   = 0;
                }
            }
            break;

        case PASS_RESOURCE_DIMENSION_3D:
            {
                viewDesc.ViewDimension          = D3D12_UAV_DIMENSION_TEXTURE3D;
                viewDesc.Texture3D.FirstWSlice  = 0;
                viewDesc.Texture3D.MipSlice     = 0;
                viewDesc.Texture3D.WSize        = value.DepthOrArraySize;
            }
            break;
        }

        if (!GfxDevice().AllocHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, &m_DescriptorRes))
        {
            ELOG("Error : GraphicsDevice::AllocHandle() Failed.");
            return false;
        }

        GfxDevice()->CreateUnorderedAccessView(m_Resource, nullptr, &viewDesc, m_DescriptorRes->GetHandleCPU());
        return true;
    }

    //-------------------------------------------------------------------------
    //! @brief      シェーダリソースビューを生成します.
    //-------------------------------------------------------------------------
    bool CreateSRV(const PassResourceDesc& value)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
        switch(value.Dimension)
        {
        case PASS_RESOURCE_DIMENSION_BUFFER:
            {
                viewDesc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
                viewDesc.Buffer.FirstElement        = 0;
                viewDesc.Buffer.NumElements         = uint32_t(value.Width / value.Stride);
                viewDesc.Buffer.StructureByteStride = value.Stride;
                viewDesc.Buffer.Flags               = (value.Stride == 1) ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;
            }
            break;

        case PASS_RESOURCE_DIMENSION_1D:
            {
                if (value.DepthOrArraySize > 1)
                {
                    viewDesc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                    viewDesc.Texture1DArray.ArraySize           = value.DepthOrArraySize;
                    viewDesc.Texture1DArray.FirstArraySlice     = 0;
                    viewDesc.Texture1DArray.MipLevels           = value.MipLevels;
                    viewDesc.Texture1DArray.MostDetailedMip     = 0;
                    viewDesc.Texture1DArray.ResourceMinLODClamp = 0;
                }
                else
                {
                    viewDesc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE1D;
                    viewDesc.Texture1D.MipLevels            = value.MipLevels;
                    viewDesc.Texture1D.MostDetailedMip      = 0;
                    viewDesc.Texture1D.ResourceMinLODClamp  = 0;
                }
            }
            break;

        case PASS_RESOURCE_DIMENSION_2D:
            {
                if (value.DepthOrArraySize > 1)
                {
                    viewDesc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                    viewDesc.Texture2DArray.ArraySize           = value.DepthOrArraySize;
                    viewDesc.Texture2DArray.FirstArraySlice     = 0;
                    viewDesc.Texture2DArray.MipLevels           = value.MipLevels;
                    viewDesc.Texture2DArray.MostDetailedMip     = 0;
                    viewDesc.Texture2DArray.PlaneSlice          = 0;
                    viewDesc.Texture2DArray.ResourceMinLODClamp = 0;
                }
                else
                {
                    viewDesc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE2D;
                    viewDesc.Texture2D.MipLevels            = value.MipLevels;
                    viewDesc.Texture2D.MostDetailedMip      = 0;
                    viewDesc.Texture2D.PlaneSlice           = 0;
                    viewDesc.Texture2D.ResourceMinLODClamp  = 0;
                }
            }
            break;

        case PASS_RESOURCE_DIMENSION_3D:
            {
                viewDesc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE3D;
                viewDesc.Texture3D.MipLevels            = value.MipLevels;
                viewDesc.Texture3D.MostDetailedMip      = 0;
                viewDesc.Texture3D.ResourceMinLODClamp  = 0;
            }
            break;
        }

        if (m_DescriptorRes == nullptr)
        {
            if (!GfxDevice().AllocHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, &m_DescriptorRes))
            {
                ELOG("Error : GraphicsDevice::AllocHandle() Failed.");
                return false;
            }
        }

        GfxDevice()->CreateShaderResourceView(m_Resource, &viewDesc, m_DescriptorRes->GetHandleCPU());
        return true;
    }
};


///////////////////////////////////////////////////////////////////////////////
// PassResourceRegistry class
///////////////////////////////////////////////////////////////////////////////
class PassResourceRegistry
{
public:
    PassResourceRegistry()
    {
    }

    ResourceHandle GetOrCreate(const PassResourceDesc& value)
    {

        return 0;
    }

    PassResource* GetResource(ResourceHandle handle) const
    {
        //assert(m_Resources != nullptr);
        //assert(handle < m_ResourceCount);
        //return m_Resources[handle];

        return nullptr;
    }

private:
    Disposer<PassResource>  m_Dispoer;
    uint32_t                m_ResourceCount = 0;
};


///////////////////////////////////////////////////////////////////////////////
// RenderPass class
///////////////////////////////////////////////////////////////////////////////
class RenderPass
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    PassTag         m_Tag;
    PassSetup       m_Setup;
    PassExecute     m_Execute;
    bool            m_AsyncCompute;

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    RenderPass()
    : m_Setup       (nullptr)
    , m_Execute     (nullptr)
    , m_AsyncCompute(false)
    , m_RefCount    (1)
    , m_Next        (nullptr)
    , m_Prev        (nullptr)
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      参照カウントを増やします.
    //-------------------------------------------------------------------------
    void AddRef()
    { m_RefCount++; }

    //-------------------------------------------------------------------------
    //! @brief      参照カウントを減らします.
    //-------------------------------------------------------------------------
    void Release()
    { m_RefCount--; }

    //-------------------------------------------------------------------------
    //! @brief      参照カウントを取得します.
    //-------------------------------------------------------------------------
    int GetRefCount() const
    { return m_RefCount; }

    //-------------------------------------------------------------------------
    //! @brief      リスト末尾に追加します.
    //-------------------------------------------------------------------------
    void AddNext(RenderPass* node)
    {
        assert(node != nullptr);
        if (node == nullptr)
        { return; }

        auto next = m_Prev;
        m_Next = next;
        node->m_Prev = this;
        node->m_Next = next;
    }

    //-------------------------------------------------------------------------
    //! @brief      リストの接続解除します.
    //-------------------------------------------------------------------------
    void Unlink()
    {
        auto prev = m_Prev;
        auto next = m_Next;

        if (prev != nullptr)
        { prev->m_Next = next; }
        if (next != nullptr)
        { next->m_Prev = prev; }

        m_Prev = nullptr;
        m_Next = nullptr;
    }

    //-------------------------------------------------------------------------
    //! @brief      次のノードを取得します.
    //-------------------------------------------------------------------------
    RenderPass* GetNext() const
    { return m_Next; }

    //-------------------------------------------------------------------------
    //! @brief      前のノードを取得します.
    //-------------------------------------------------------------------------
    RenderPass* GetPrev() const
    { return m_Prev; }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    std::atomic<int>    m_RefCount;
    RenderPass*         m_Next;
    RenderPass*         m_Prev;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

///////////////////////////////////////////////////////////////////////////////
// PassGraph class
///////////////////////////////////////////////////////////////////////////////
class PassGraph : public IPassGraph
{
public:
    PassGraph()
    {}

    ~PassGraph()
    {
    }

    bool Init(uint32_t maxPassCount, uint32_t maxrResourceCount)
    {
    }

    void Release() override
    {
        delete this;
    }

    bool AddPass(PassTag& tag, PassSetup setup, PassExecute execute) override
    {
        assert(m_PassCount < m_MaxPassCount);
        if (m_PassCount >= m_MaxPassCount)
        { return false; }

        auto pass = m_FrameHeap.Alloc<RenderPass>();
        pass->m_Tag         = tag;
        pass->m_Setup       = setup;
        pass->m_Execute     = execute;

        if (m_Head == nullptr)
        {
            m_Head = pass;
            m_Tail = pass;
        }
        else
        {
            m_Tail->AddNext(pass);
        }
        m_PassCount++;

        return true;
    }

    void Compile() override
    {
        // リソース書き込みに対して Pass.m_RefCount++

        // リソース読み込みに対して PassResource.m_RefCount++

        // PassResource.m_RefCount == 0 をみつけてスタックに積む.

        // スタックが空でない場合
        
            // リソースをpop() 生成したproducerの参照カウントを下げる.

            // producerの参照カウントが0なら，読み取りするリソースの参照カウントを下げる.

            // 参照カウントがゼロのときにそれらのリソースをスタックに積む.

    }

    void Execute(ID3D12CommandQueue* pGraphics, ID3D12CommandQueue* pCompute) override
    {
        // Pass.m_RefCount != 0　に対してスレッド実行.

        
        // コマンドキューに積む.


        // パスをクリア.
        m_Head = nullptr;
        m_Tail = nullptr;

        // ヒープリセット.
        m_FrameHeap.Reset();
    }


    template<typename T>
    T* FrameAlloc()
    { return m_FrameHeap.Alloc<T>(); }

    ResourceHandle AllocResource(PassResourceDesc& desc)
    { return m_Registry.GetOrCreate(desc); }

    PassResource* GetResource(ResourceHandle handle) const
    { return m_Registry.GetResource(handle); }

private:
    FrameHeap               m_FrameHeap;
    RenderPass*             m_Head          = nullptr;
    RenderPass*             m_Tail          = nullptr;
    uint32_t                m_PassCount     = 0;
    CommandList*            m_pCommandLists = nullptr;
    uint32_t                m_MaxPassCount  = 0;
    PassResourceRegistry    m_Registry;
};


///////////////////////////////////////////////////////////////////////////////
// PassGraphBuilder class
///////////////////////////////////////////////////////////////////////////////
class PassGraphBuilder : public IPassGraphBuilder
{
public:
    PassGraphBuilder(PassGraph* graph, RenderPass* pass)
    : m_Graph(graph)
    , m_Pass (pass)
    { /* DO_NOTHING */ }

    ResourceHandle Read(ResourceHandle resource, uint32_t flag) override
    {
        // パスに情報を登録.

        return resource;
    }

    ResourceHandle Write(ResourceHandle resource, uint32_t flag) override
    {
        // パスに情報を登録.

        return resource;
    }

    ResourceHandle Create(PassResourceDesc& desc) override
    {
        // リソースを割り当て.
        auto resource = m_Graph->AllocResource(desc);

        // パスに情報を登録.

        return resource;
    }

    ResourceHandle Import(ID3D12Resource* resource, D3D12_RESOURCE_STATES state) override
    {
        // 

        return 0;
    }

    ResourceHandle Import(ID3D12Resource* resource, Descriptor* descriptor, D3D12_RESOURCE_STATES state) override
    {
        return 0;
    }

    void AsyncComputeEnable(bool value) override
    { m_Pass->m_AsyncCompute = value; }

private:
    PassGraph*  m_Graph = nullptr;
    RenderPass* m_Pass  = nullptr;
};

///////////////////////////////////////////////////////////////////////////////
// PassGraphContext class
///////////////////////////////////////////////////////////////////////////////
class PassGraphContext : public IPassGraphContext
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
    PassGraphContext(PassGraph* graph, RenderPass* pass, ID3D12GraphicsCommandList6* commandList)
    : m_Graph       (graph)
    , m_Pass        (pass)
    , m_CommandList (commandList)
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      RTV用ディスクリプタハンドルを取得します.
    //-------------------------------------------------------------------------
    D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(ResourceHandle resource) const override
    {
        assert(m_Graph != nullptr);
        return m_Graph->GetResource(resource).GetHandleRTV();
    }

    //-------------------------------------------------------------------------
    //! @brief      DSV用ディスクリプタハンドルを取得します.
    //-------------------------------------------------------------------------
    D3D12_CPU_DESCRIPTOR_HANDLE GetDSV(ResourceHandle resource) const override
    {
        assert(m_Graph != nullptr);
        return m_Graph->GetResource(resource)->GetHandleDSV();
    }

    //-------------------------------------------------------------------------
    //! @brief      UAV/SRV用ディスクリプタハンドルを取得します.
    //-------------------------------------------------------------------------
    D3D12_GPU_DESCRIPTOR_HANDLE GetRes(ResourceHandle resource) const override
    {
        assert(m_Graph != nullptr);
        return m_Graph->GetResource(resource)->GetHandleRes();
    }

    //-------------------------------------------------------------------------
    //! @brief      GPU仮想アドレスを取得します.
    //-------------------------------------------------------------------------
    D3D12_GPU_VIRTUAL_ADDRESS GetVirtualAddress(ResourceHandle resource) const override
    {
        assert(m_Graph != nullptr);
        return m_Graph->GetResource(resource)->GetVirtualAddress();
    }

    //-------------------------------------------------------------------------
    //! @brief      構成設定を取得します.
    //-------------------------------------------------------------------------
    D3D12_RESOURCE_DESC GetDesc(ResourceHandle resource) const override
    {
        assert(m_Graph != nullptr);
        return m_Graph->GetResource(resource)->GetDesc();
    }

    //-------------------------------------------------------------------------
    //! @brief      コマンドリストを取得します.
    //-------------------------------------------------------------------------
    ID3D12GraphicsCommandList6* GetCommandList() const
    { return m_CommandList; }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    PassGraph*                  m_Graph         = nullptr;
    RenderPass*                 m_Pass          = nullptr;
    ID3D12GraphicsCommandList6* m_CommandList   = nullptr;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

//-----------------------------------------------------------------------------
//      パスグラフを生成します.
//-----------------------------------------------------------------------------
bool CreatePassGraph(uint32_t maxPassCount)
{
    return false;
}

} // namespace asdx

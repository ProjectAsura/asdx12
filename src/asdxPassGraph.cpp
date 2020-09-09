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
#include <asdxList.h>
#include <asdxStack.h>


// パスで生成可能な最大リソース数.
#define MAX_PASS_RESOURCE_COUNT (16)


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// RESORUCE_INFO_FLAGS
///////////////////////////////////////////////////////////////////////////////
enum RESOURCE_INFO_FLAGS
{
    RESOURCE_INFO_FLAG_NONE     = 0,
    RESOURCE_INFO_FLAG_READ     = 0x1 << 0,
    RESOURCE_INFO_FLAG_WRITE    = 0x1 << 1,
    RESOURCE_INFO_FLAG_BARRIER  = 0x1 << 2,
};

//-----------------------------------------------------------------------------
// Forward Declarations.
//-----------------------------------------------------------------------------
class PassGraph;
class RenderPass;

///////////////////////////////////////////////////////////////////////////////
// PassResource class
///////////////////////////////////////////////////////////////////////////////
class PassResource 
: public List<PassResource>::Node
, public Stack<PassResource>::Node
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    D3D12_RESOURCE_STATES   ResourceState;

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    PassResource()
    : List<PassResource>::Node()
    , m_RefCount        (0)
    , m_DescriptorRTV   (nullptr)
    , m_DescriptorDSV   (nullptr)
    , m_DescriptorRes   (nullptr)
    , m_Resource        (nullptr)
    , m_Import          (false)
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~PassResource()
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //-------------------------------------------------------------------------
    bool Init(const PassResourceDesc& value, RenderPass* producer)
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

            D3D12_CLEAR_VALUE clearValue = {};
            clearValue.Format = value.Format;
            if (value.Usage & PASS_RESOURCE_USAGE_DSV)
            {
                clearValue.DepthStencil.Depth   = value.ClearValue.Depth;
                clearValue.DepthStencil.Stencil = value.ClearValue.Stencil;
            }
            else
            {
                clearValue.Color[0] = value.ClearValue.Color[0];
                clearValue.Color[1] = value.ClearValue.Color[1];
                clearValue.Color[2] = value.ClearValue.Color[2];
                clearValue.Color[3] = value.ClearValue.Color[3];
            }

            m_Stencil = false;
            if (value.Format == DXGI_FORMAT_D24_UNORM_S8_UINT)
            { m_Stencil = true; }
            else if (value.Format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
            { m_Stencil = true; }

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

            ResourceState = D3D12_RESOURCE_STATE_COMMON;

            if (rtv)
            {
                m_DescriptorRTV = new Descriptor* [value.DepthOrArraySize];

                for(auto i=0; i<value.DepthOrArraySize; ++i)
                {
                    if (!CreateRTV(value, i))
                    { return false; }
                }
            }
            else if (dsv)
            {
                m_DescriptorDSV = new Descriptor* [value.DepthOrArraySize];

                for(auto i=0; i<value.DepthOrArraySize; ++i)
                {
                    if (!CreateDSV(value, i))
                    { return false; }
                }
            }

            if (uav)
            {
                if (!CreateUAV(value))
                { return false; }
            }

            if (!CreateSRV(value))
            { return false; }
        }

        m_Import    = false;
        m_Desc      = value;
        m_Producer  = producer;

        return true;
    }

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term()
    {
        for(auto i=0; i<m_Desc.DepthOrArraySize; ++i)
        {
            if (m_DescriptorRTV[i] != nullptr)
            {
                m_DescriptorRTV[i]->Release();
                m_DescriptorRTV[i] = nullptr;
            }
                
            if (m_DescriptorDSV != nullptr)
            {
                m_DescriptorDSV[i]->Release();
                m_DescriptorDSV[i] = nullptr;
            }
        }

        if (m_DescriptorRTV != nullptr)
        {
            delete[] m_DescriptorRTV;
            m_DescriptorRTV = nullptr;
        }

        if (m_DescriptorDSV != nullptr)
        {
            delete[] m_DescriptorDSV;
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
        Term();
        delete this;
    }

    //-------------------------------------------------------------------------
    //! @brief      レンダーターゲットビューのディスクリプタハンドルを取得します.
    //-------------------------------------------------------------------------
    D3D12_CPU_DESCRIPTOR_HANDLE GetHandleRTV(uint16_t index) const
    {
        assert(index < m_Desc.DepthOrArraySize);
        if (m_DescriptorRTV[index] != nullptr)
        { return m_DescriptorRTV[index]->GetHandleCPU(); }

        return D3D12_CPU_DESCRIPTOR_HANDLE();
    }

    //-------------------------------------------------------------------------
    //! @brief      深度ステンシルビューのディスクリプタハンドルを取得します.
    //-------------------------------------------------------------------------
    D3D12_CPU_DESCRIPTOR_HANDLE GetHandleDSV(uint16_t index) const
    {
        assert(index < m_Desc.DepthOrArraySize);
        if (m_DescriptorDSV[index] != nullptr)
        { return m_DescriptorDSV[index]->GetHandleCPU(); }

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
    PassResourceDesc GetDesc() const
    { return m_Desc; }

    //-------------------------------------------------------------------------
    //! @brief      構成設定を取得します.
    //-------------------------------------------------------------------------
    D3D12_RESOURCE_DESC GetD3D12Desc() const
    {
        if (m_Resource != nullptr)
        { return m_Resource->GetDesc(); }
    
        return D3D12_RESOURCE_DESC();
    }

    //-------------------------------------------------------------------------
    //! @brief      ビューをクリアします.
    //-------------------------------------------------------------------------
    void ClearView(ID3D12GraphicsCommandList6* pCmd, ClearValue& value)
    {
        if (value.Type == CLEAR_TYPE_RTV)
        {
            for(auto i=0; i<m_Desc.DepthOrArraySize; ++i)
            {
                pCmd->ClearRenderTargetView(
                    m_DescriptorRTV[i]->GetHandleCPU(),
                    value.Color,
                    0,
                    nullptr);
            }
        }
        else if (value.Type == CLEAR_TYPE_DSV)
        {
            auto flag = D3D12_CLEAR_FLAG_DEPTH;
            if (m_Stencil)
            { flag |= D3D12_CLEAR_FLAG_STENCIL;}

            for(auto i=0; i<m_Desc.DepthOrArraySize; ++i)
            {
                pCmd->ClearDepthStencilView(
                    m_DescriptorDSV[i]->GetHandleCPU(),
                    flag,
                    value.Depth,
                    value.Stencil,
                    0,
                    nullptr);
            }
        }
        else if (value.Type == CLEAR_TYPE_UAV_FLOAT)
        {
            pCmd->ClearUnorderedAccessViewFloat(
                m_DescriptorRes->GetHandleGPU(),
                m_DescriptorRes->GetHandleCPU(),
                m_Resource,
                value.Float,
                0,
                nullptr);
        }
        else if (value.Type == CLEAR_TYPE_UAV_UINT)
        {
            pCmd->ClearUnorderedAccessViewUint(
                m_DescriptorRes->GetHandleGPU(),
                m_DescriptorRes->GetHandleCPU(),
                m_Resource,
                value.Uint,
                0,
                nullptr);
        }
    }

    //-------------------------------------------------------------------------
    //! @brief      リソースをインポートします.
    //-------------------------------------------------------------------------
    bool Import(ID3D12Resource* pResource, D3D12_RESOURCE_STATES state)
    {
        // TODO : Implement.
        assert(false);

        m_Import = true;
        m_Resource = pResource;

        // ビューを生成.

        return true;
    }

    //-------------------------------------------------------------------------
    //! @brief      構成設定が合致するかどうかチェックします.
    //-------------------------------------------------------------------------
    bool Match(const PassResourceDesc& value) const
    { 
        return (m_Desc.Dimension         == value.Dimension)
            && (m_Desc.Width             == value.Width)
            && (m_Desc.Height            == value.Height)
            && (m_Desc.DepthOrArraySize  == value.DepthOrArraySize)
            && (m_Desc.MipLevels         == value.MipLevels)
            && (m_Desc.Format            == value.Format);
    }

    //-------------------------------------------------------------------------
    //! @brief      参照カウントを増やします.
    //-------------------------------------------------------------------------
    void Increment()
    {
        if (!m_Import)
        { m_RefCount++; }
    }

    //-------------------------------------------------------------------------
    //! @brief      参照カウントを減らします.
    //-------------------------------------------------------------------------
    void Decrement()
    {
        if (!m_Import)
        { m_RefCount--; }
    }

    //-------------------------------------------------------------------------
    //! @brief      参照カウントを取得します.
    //-------------------------------------------------------------------------
    int GetRefCount() const
    { return m_RefCount; }

    //-------------------------------------------------------------------------
    //! @brief      インポートリソースかどうかチェックします.
    //-------------------------------------------------------------------------
    bool IsImport() const
    { return m_Import; }

    //-------------------------------------------------------------------------
    //! @brief      生成パスを取得します.
    //-------------------------------------------------------------------------
    RenderPass* GetProducer() const
    { return m_Producer; }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    std::atomic<int>    m_RefCount      = 1;
    Descriptor**        m_DescriptorRTV = nullptr;
    Descriptor**        m_DescriptorDSV = nullptr;
    Descriptor*         m_DescriptorRes = nullptr;
    ID3D12Resource*     m_Resource      = nullptr;
    PassResourceDesc    m_Desc          = {};
    bool                m_Import        = false;
    bool                m_Stencil       = false;
    RenderPass*         m_Producer      = nullptr;

    //=========================================================================
    // private methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      レンダーターゲットビューを生成します.
    //-------------------------------------------------------------------------
    bool CreateRTV(const PassResourceDesc& value, uint32_t arrayIndex)
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
                    viewDesc.Texture1DArray.FirstArraySlice = arrayIndex;
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
                        viewDesc.Texture2DArray.FirstArraySlice = arrayIndex;
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
                viewDesc.Texture3D.FirstWSlice  = arrayIndex;
                viewDesc.Texture3D.MipSlice     = 0;
                viewDesc.Texture3D.WSize        = value.DepthOrArraySize;
            }
            break;
        }

        Descriptor* descriptor = nullptr;
        if (!GfxDevice().AllocHandle(
            D3D12_DESCRIPTOR_HEAP_TYPE_RTV, &descriptor))
        {
            ELOG("Error : GraphicsDevice::AllocHandle() Failed.");
            return false;
        }

        GfxDevice()->CreateRenderTargetView(
            m_Resource, &viewDesc, descriptor->GetHandleCPU());
        m_DescriptorRTV[arrayIndex] = descriptor;

        return true;
    }

    //-------------------------------------------------------------------------
    //! @brief      深度ステンシルビューを生成します.
    //-------------------------------------------------------------------------
    bool CreateDSV(const PassResourceDesc& value, uint32_t arrayIndex)
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
                    viewDesc.Texture1DArray.FirstArraySlice = arrayIndex;
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
                    viewDesc.Texture2DArray.FirstArraySlice = arrayIndex;
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

        Descriptor* descriptor = nullptr;
        if (!GfxDevice().AllocHandle(
            D3D12_DESCRIPTOR_HEAP_TYPE_DSV, &descriptor))
        {
            ELOG("Error : GfxDevice::AllocHandle() Failed.");
            return false;
        }

        GfxDevice()->CreateDepthStencilView(
            m_Resource, &viewDesc, descriptor->GetHandleCPU());
        m_DescriptorDSV[arrayIndex] = descriptor;
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
                viewDesc.Buffer.Flags                   = (value.Stride == 1) 
                                                            ? D3D12_BUFFER_UAV_FLAG_RAW 
                                                            : D3D12_BUFFER_UAV_FLAG_NONE;
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

        if (!GfxDevice().AllocHandle(
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, &m_DescriptorRes))
        {
            ELOG("Error : GraphicsDevice::AllocHandle() Failed.");
            return false;
        }

        GfxDevice()->CreateUnorderedAccessView(
            m_Resource, nullptr, &viewDesc, m_DescriptorRes->GetHandleCPU());
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
                viewDesc.Buffer.Flags               = (value.Stride == 1) 
                                                        ? D3D12_BUFFER_SRV_FLAG_RAW
                                                        : D3D12_BUFFER_SRV_FLAG_NONE;
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
            if (!GfxDevice().AllocHandle(
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, &m_DescriptorRes))
            {
                ELOG("Error : GraphicsDevice::AllocHandle() Failed.");
                return false;
            }
        }

        GfxDevice()->CreateShaderResourceView(
            m_Resource, &viewDesc, m_DescriptorRes->GetHandleCPU());
        return true;
    }
};


///////////////////////////////////////////////////////////////////////////////
// PassResourceRegistry class
///////////////////////////////////////////////////////////////////////////////
class PassResourceRegistry
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
    PassResourceRegistry()
    : m_Capacity    (0)
    , m_Cache       ()
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~PassResourceRegistry()
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //-------------------------------------------------------------------------
    void Init(uint32_t capacity)
    {
        m_Capacity   = capacity;
        m_Cache.Clear();
    }

    //-------------------------------------------------------------------------
    //! @brief      取得または生成をおこないます.
    //-------------------------------------------------------------------------
    PassResource* GetOrCreate(const PassResourceDesc& value, RenderPass* producer)
    {
        // LRUキャッシュアルゴリズム.
        PassResource* node;
        if (Contains(value, &node))
        {
            Remove(node);
            PushBack(node);
        }
        else if(m_Cache.GetCount() < m_Capacity)
        {
            node = CreateResource(value, producer);
            PushBack(node);
        }
        else
        {
            auto head = PopFront();
            m_Dispoer.Push(head);

            node = CreateResource(value, producer);
            PushBack(node);
        }

        return node;
    }

    //-------------------------------------------------------------------------
    //! @brief      クリア処理を行います.
    //-------------------------------------------------------------------------
    void Clear()
    {
        // 全部を突っ込む.
        auto itr = m_Cache.GetHead();
        while(itr != nullptr)
        {
            auto node = itr;
            m_Dispoer.Push(node);

            if (!itr->HasNext())
            { break; }

            itr = itr->GetNext();
        }

        m_Cache.Clear();

        // 強制破棄.
        m_Dispoer.Clear();
    }

    //-------------------------------------------------------------------------
    //! @brief      フレーム同期を行い，遅延解放を行います.
    //-------------------------------------------------------------------------
    void FrameSync()
    { m_Dispoer.FrameSync(); }

    //-------------------------------------------------------------------------
    //! @brief      リスト先頭ポインタを取得します.
    //-------------------------------------------------------------------------
    PassResource* GetHead() const
    { return m_Cache.GetHead(); }

    //-------------------------------------------------------------------------
    //! @brief      リスト末尾ポインタを取得します.
    //-------------------------------------------------------------------------
    PassResource* GetTail() const
    { return m_Cache.GetTail();}

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    Disposer<PassResource>  m_Dispoer;
    uint32_t                m_Capacity = 0;
    List<PassResource>      m_Cache;

    //=========================================================================
    // private methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      構成設定が合致するリソースが含まれるかチェックします.
    //-------------------------------------------------------------------------
    bool Contains(const PassResourceDesc& value, PassResource** node)
    {
        auto itr = m_Cache.GetHead();
        while(itr != nullptr)
        {
            if (itr->Match(value))
            {
                *node = itr;
                return true;
            }

            if (!itr->HasNext())
            { break; }

            itr = itr->GetNext();
        }

        return false;
    }

    //-------------------------------------------------------------------------
    //! @brief      リストから削除します.
    //-------------------------------------------------------------------------
    void Remove(PassResource* node)
    {
        m_Cache.Remove(node);
    }

    //-------------------------------------------------------------------------
    //! @brief      リスト末尾に追加します.
    //-------------------------------------------------------------------------
    void PushBack(PassResource* node)
    {
        m_Cache.PushBack(node);
    }

    //-------------------------------------------------------------------------
    //! @brief      リスト先頭からポップします.
    //-------------------------------------------------------------------------
    PassResource* PopFront()
    {
        return m_Cache.PopFront();
    }

    //-------------------------------------------------------------------------
    //! @brief      リソースを生成します.
    //-------------------------------------------------------------------------
    PassResource* CreateResource(const PassResourceDesc& value, RenderPass* producer)
    {
        auto resource = new(std::nothrow) PassResource();
        assert(resource != nullptr);

        if (!resource->Init(value, producer))
        {
            ELOG("Error : PassResource::Init() Failed.");
            assert(false);
            resource->Term();
            return nullptr;
        }

        return resource;
    }
};


///////////////////////////////////////////////////////////////////////////////
// RenderPass class
///////////////////////////////////////////////////////////////////////////////
class RenderPass : public List<RenderPass>::Node
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    ///////////////////////////////////////////////////////////////////////////
    // ResourceInfo
    ///////////////////////////////////////////////////////////////////////////
    struct ResourceInfo
    {
        uint32_t                Count = 0;
        uint8_t                 Flags      [MAX_PASS_RESOURCE_COUNT] = {};
        PassResource*           Resources  [MAX_PASS_RESOURCE_COUNT] = {};
        D3D12_RESOURCE_BARRIER  Barriers   [MAX_PASS_RESOURCE_COUNT] = {};
    };

    ///////////////////////////////////////////////////////////////////////////
    // ClearInfo
    ///////////////////////////////////////////////////////////////////////////
    struct ClearInfo
    {
        uint32_t        Count = 0;
        ClearValue      Values   [MAX_PASS_RESOURCE_COUNT] = {};
        PassResource*   Resources[MAX_PASS_RESOURCE_COUNT] = {};
    };

    //=========================================================================
    // public variables.
    //=========================================================================
    PassTag         m_Tag;
    PassSetup       m_Setup         = nullptr;
    PassExecute     m_Execute       = nullptr;
    bool            m_AsyncCompute  = false;
    ResourceInfo    m_Infos         = {};
    ClearInfo       m_ClearInfos    = {};

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    RenderPass()
    : List<RenderPass>::Node()
    , m_Setup           (nullptr)
    , m_Execute         (nullptr)
    , m_AsyncCompute    (false)
    , m_RefCount        (1)
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      参照カウントを増やします.
    //-------------------------------------------------------------------------
    void Increment()
    { m_RefCount++; }

    //-------------------------------------------------------------------------
    //! @brief      参照カウントを減らします.
    //-------------------------------------------------------------------------
    void Decrement()
    { m_RefCount--; }

    //-------------------------------------------------------------------------
    //! @brief      参照カウントを取得します.
    //-------------------------------------------------------------------------
    int GetRefCount() const
    { return m_RefCount; }

    //-------------------------------------------------------------------------
    //! @brief      リソースバリアを設定します.
    //-------------------------------------------------------------------------
    void ResourceBarrier(ID3D12GraphicsCommandList6* pCmd)
    {
        D3D12_RESOURCE_BARRIER barriers[MAX_PASS_RESOURCE_COUNT];
        auto count = 0u;
        for(auto i=0u; i<m_Infos.Count; ++i)
        {
            if (m_Infos.Flags[i] & RESOURCE_INFO_FLAG_BARRIER)
            {
                barriers[count] = m_Infos.Barriers[i];
                count++;
            }
        }

        pCmd->ResourceBarrier(count, barriers);
    }

    //-------------------------------------------------------------------------
    //! @brief      ビューをクリアします.
    //-------------------------------------------------------------------------
    void ClearViews(ID3D12GraphicsCommandList6* pCmd)
    {
        for(auto i=0u; i<m_ClearInfos.Count; ++i)
        { m_ClearInfos.Resources[i]->ClearView(pCmd, m_ClearInfos.Values[i]); }
    }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    std::atomic<int>    m_RefCount;

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
    PassGraph();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~PassGraph();

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //-------------------------------------------------------------------------
    bool Init(uint32_t maxPassCount, uint32_t maxResourceCount);

    //-------------------------------------------------------------------------
    //! @brief      解放処理を行います.
    //-------------------------------------------------------------------------
    void Release() override;

    //-------------------------------------------------------------------------
    //! @brief      パスを追加します.
    //-------------------------------------------------------------------------
    bool AddPass(PassTag& tag, PassSetup setup, PassExecute execute) override;

    //-------------------------------------------------------------------------
    //! @brief      ビルドします.
    //-------------------------------------------------------------------------
    void Compile() override;

    //-------------------------------------------------------------------------
    //! @brief      レンダーパスを実行します。
    //-------------------------------------------------------------------------
    void Execute(ID3D12CommandQueue* pGraphics, ID3D12CommandQueue* pCompute) override;

    //-------------------------------------------------------------------------
    //! @brief      リソースを確保します.
    //-------------------------------------------------------------------------
    PassResource* AllocResource(const PassResourceDesc& desc, RenderPass* producer);

    //-------------------------------------------------------------------------
    //! @brief      フレームヒープからメモリを確保します.
    //-------------------------------------------------------------------------
    template<typename T>
    T* FrameAlloc()
    {
        auto ptr = m_FrameHeap[m_BufferIndex].Alloc<T>();
        assert(ptr != nullptr);
        return ptr;
    }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    FrameHeap               m_FrameHeap[2];
    uint8_t                 m_BufferIndex   = 0;
    List<RenderPass>        m_PassList;
    CommandList*            m_GraphicsCommandLists  = nullptr;
    CommandList*            m_ComputeCommandLists   = nullptr;
    uint32_t                m_MaxPassCount  = 0;
    PassResourceRegistry    m_Registry;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

///////////////////////////////////////////////////////////////////////////////
// PassGraphBuilder class
///////////////////////////////////////////////////////////////////////////////
class PassGraphBuilder : public IPassGraphBuilder
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
    PassGraphBuilder(PassGraph* graph, RenderPass* pass)
    : m_Graph(graph)
    , m_Pass (pass)
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      Readリソースを登録します.
    //-------------------------------------------------------------------------
    PassResource* Read(PassResource* resource) override
    {
        uint8_t flag = RESOURCE_INFO_FLAG_READ;
        auto index = m_Pass->m_Infos.Count;

        auto prevState = resource->ResourceState;
        if (prevState != D3D12_RESOURCE_STATE_GENERIC_READ)
        {
            // バリア設定.

            // ステート変更.
        }

        m_Pass->m_Infos.Resources[index] = resource;
        m_Pass->m_Infos.Flags    [index] = flag;
        m_Pass->m_Infos.Count++;

        resource->Increment();
        return resource;
    }

    //-------------------------------------------------------------------------
    //! @brief      Writeリソースを登録します.
    //-------------------------------------------------------------------------
    PassResource* Write(PassResource* resource) override
    {
        uint8_t flag = RESOURCE_INFO_FLAG_WRITE;

        auto index     = m_Pass->m_Infos.Count;
        auto prevState = resource->ResourceState;

        auto desc = resource->GetDesc();
        if (desc.Usage & PASS_RESOURCE_USAGE_RTV)
        {
            if (prevState != D3D12_RESOURCE_STATE_RENDER_TARGET)
            {
                // バリア設定.

                // ステート変更.
            }
        }
        else if (desc.Usage & PASS_RESOURCE_USAGE_DSV)
        {
            if (prevState != D3D12_RESOURCE_STATE_DEPTH_WRITE)
            {
                // バリア設定.

                // ステート変更.
            }
        }
        else if (desc.Usage & PASS_RESOURCE_USAGE_UAV)
        {
            if (prevState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
            {
                // バリア設定.

                // ステート変更.
            }
        }

        m_Pass->m_Infos.Resources[index] = resource;
        m_Pass->m_Infos.Flags    [index] = flag;
        m_Pass->m_Infos.Count++;

        m_Pass->Increment();
        return resource;
    }

    //-------------------------------------------------------------------------
    //! @brief      リソースを生成します.
    //-------------------------------------------------------------------------
    PassResource* Create(PassResourceDesc& desc) override
    {
        auto resource = m_Graph->AllocResource(desc, m_Pass);

        if (desc.InitState == PASS_RESOURCE_STATE_CLEAR)
        {
            auto index = m_Pass->m_ClearInfos.Count;
            m_Pass->m_ClearInfos.Resources[index] = resource;
            m_Pass->m_ClearInfos.Values   [index] = desc.ClearValue;
            m_Pass->m_ClearInfos.Count++;
        }

        return resource;
    }

    //-------------------------------------------------------------------------
    //! @brief      リソースをインポートします.
    //-------------------------------------------------------------------------
    PassResource* Import(ID3D12Resource* resource, D3D12_RESOURCE_STATES state) override
    {
        auto importResource = m_Graph->FrameAlloc<PassResource>();
        if (!importResource->Import(resource, state))
        { return nullptr; }

        return importResource;
    }

    //-------------------------------------------------------------------------
    //! @brief      非同期コンピュートを設定します.
    //-------------------------------------------------------------------------
    void AsyncComputeEnable(bool value) override
    { m_Pass->m_AsyncCompute = value; }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    PassGraph*  m_Graph = nullptr;
    RenderPass* m_Pass  = nullptr;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
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
    PassGraphContext
    (
        PassGraph*                  graph,
        RenderPass*                 pass,
        ID3D12GraphicsCommandList6* commandList
    )
    : m_Graph       (graph)
    , m_Pass        (pass)
    , m_CommandList (commandList)
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      RTV用ディスクリプタハンドルを取得します.
    //-------------------------------------------------------------------------
    D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(PassResource* resource, uint16_t index) const override
    {
        assert(resource != nullptr);
        return resource->GetHandleRTV(index);
    }

    //-------------------------------------------------------------------------
    //! @brief      DSV用ディスクリプタハンドルを取得します.
    //-------------------------------------------------------------------------
    D3D12_CPU_DESCRIPTOR_HANDLE GetDSV(PassResource* resource, uint16_t index) const override
    {
        assert(resource != nullptr);
        return resource->GetHandleDSV(index);
    }

    //-------------------------------------------------------------------------
    //! @brief      UAV/SRV用ディスクリプタハンドルを取得します.
    //-------------------------------------------------------------------------
    D3D12_GPU_DESCRIPTOR_HANDLE GetRes(PassResource* resource) const override
    {
        assert(resource != nullptr);
        return resource->GetHandleRes();
    }

    //-------------------------------------------------------------------------
    //! @brief      GPU仮想アドレスを取得します.
    //-------------------------------------------------------------------------
    D3D12_GPU_VIRTUAL_ADDRESS GetVirtualAddress(PassResource* resource) const override
    {
        assert(resource != nullptr);
        return resource->GetVirtualAddress();
    }

    //-------------------------------------------------------------------------
    //! @brief      構成設定を取得します.
    //-------------------------------------------------------------------------
    D3D12_RESOURCE_DESC GetDesc(PassResource* resource) const override
    {
        assert(resource != nullptr);
        return resource->GetD3D12Desc();
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
//      コンストラクタです.
//-----------------------------------------------------------------------------
PassGraph::PassGraph()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
PassGraph::~PassGraph()
{
    for(auto i=0; i<2; ++i)
    { m_FrameHeap[i].Term(); }

    if (m_GraphicsCommandLists != nullptr)
    {
        for(auto i=0u; i<m_MaxPassCount; ++i)
        { m_GraphicsCommandLists[i].Term(); }

        delete[] m_GraphicsCommandLists;
        m_GraphicsCommandLists = nullptr;
    }

    if (m_ComputeCommandLists != nullptr)
    {
        for(auto i=0u; i<m_MaxPassCount; ++i)
        { m_ComputeCommandLists[i].Term(); }

        delete[] m_ComputeCommandLists;
        m_ComputeCommandLists = nullptr;
    }

    m_Registry.Clear();
}

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool PassGraph::Init(uint32_t maxPassCount, uint32_t maxResourceCount)
{
    // パス数 + 最大リソース数で確保.
    auto frameHeapSize = sizeof(RenderPass) * maxPassCount
                       + sizeof(PassResource) * maxResourceCount;
    for(auto i=0; i<2; ++i)
    {
        if (!m_FrameHeap[i].Init(frameHeapSize))
        {
            ELOG("Error : FrameHeap::Init() Failed.");
            return false;
        }
    }

    m_Registry.Init(maxResourceCount);

    m_GraphicsCommandLists = new(std::nothrow) CommandList[maxPassCount];
    assert(m_GraphicsCommandLists != nullptr);
    if (m_GraphicsCommandLists == nullptr)
    {
        ELOG("Error : Out of Memory.");
        return false;
    }

    for(auto i=0u; i<maxPassCount; ++i)
    {
        if (!m_GraphicsCommandLists[i].Init(
            GfxDevice().GetDevice(),
            D3D12_COMMAND_LIST_TYPE_DIRECT))
        {
            ELOG("Error : CommandList::Init() Failed.");
            return false;
        }
    }

    m_ComputeCommandLists = new(std::nothrow) CommandList[maxPassCount];
    assert(m_ComputeCommandLists != nullptr);
    if (m_ComputeCommandLists == nullptr)
    {
        ELOG("Error : Out of Memory.");
        return false;
    }

    for(auto i=0u; i<maxPassCount; ++i)
    {
        if (!m_ComputeCommandLists[i].Init(
            GfxDevice().GetDevice(),
            D3D12_COMMAND_LIST_TYPE_COMPUTE))
        {
            ELOG("Error : CommandList::Init() Failed.");
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      解放処理を行います.
//-----------------------------------------------------------------------------
void PassGraph::Release()
{ delete this; }

//-----------------------------------------------------------------------------
//      パスを追加します.
//-----------------------------------------------------------------------------
bool PassGraph::AddPass(PassTag& tag, PassSetup setup, PassExecute execute)
{
    assert(m_PassList.GetCount() < m_MaxPassCount);
    if (m_PassList.GetCount() >= m_MaxPassCount)
    { return false; }

    auto pass = FrameAlloc<RenderPass>();
    pass->m_Tag         = tag;
    pass->m_Setup       = setup;
    pass->m_Execute     = execute;

    m_PassList.PushBack(pass);

    return true;
}

//-----------------------------------------------------------------------------
//      ビルドします.
//-----------------------------------------------------------------------------
void PassGraph::Compile()
{
    // 各パスについて処理.
    {
        auto itr = m_PassList.GetHead();
        while(itr != nullptr)
        {
            PassGraphBuilder builder(this, itr);
            itr->m_Setup(&builder);

            if (!itr->HasNext())
            { break; }

            itr = itr->GetNext();
        }
    }

    // 参照カウントゼロのリソースを格納するスタック. 
    Stack<PassResource> stack;

    // 参照カウントがゼロのリソースを見つける.
    {
        auto itr = m_Registry.GetHead();
        while(itr != nullptr)
        {
            if (itr->GetRefCount() == 0)
            {
                // スタックに積む.
                stack.Push(itr);
            }

            if (!itr->HasNext())
            { break; }

            itr = itr->GetNext();
        }
    }

    // スタックが空でない場合
    while(!stack.IsEmpty())
    {
        // リソースをpop() 生成したproducerの参照カウントを下げる.
        auto resource = stack.Pop();
        auto producer = resource->GetProducer();
        producer->Decrement();

        // producerの参照カウントが0の場合.
        if (producer->GetRefCount() == 0)
        {
            // 読み取りするリソースの参照カウントを下げる.


            // 参照カウントがゼロのときにそれらのリソースをスタックに積む.

        }
    }
}

//-----------------------------------------------------------------------------
//      レンダーパスを実行します。
//-----------------------------------------------------------------------------
void PassGraph::Execute(ID3D12CommandQueue* pGraphics, ID3D12CommandQueue* pCompute)
{
    // コマンドリストをリセット.
    for(auto i=0u; i<m_MaxPassCount; ++i)
    {
        m_GraphicsCommandLists[i].Reset();
        m_ComputeCommandLists[i].Reset();
    }

    // 有効なコマンドリストの数.
    auto graphisIndex = 0u;
    auto computeIndex = 0u;

    auto itr = m_PassList.GetHead();
    while(itr != nullptr)
    {
        if (itr->GetRefCount() != 0)
        {
            // コマンドリスト割り当て
            ID3D12GraphicsCommandList* pCmd = nullptr;
            if (itr->m_AsyncCompute)
            {
                pCmd = m_GraphicsCommandLists[graphisIndex].GetCommandList();
                graphisIndex++;
            }
            else
            {
                pCmd = m_GraphicsCommandLists[computeIndex].GetCommandList();
                computeIndex++;
            }

            // スレッド割り当て.
        }

        if (!itr->HasNext())
        { break; }

        itr = itr->GetNext();
    }

    // スレッド実行!!


    // コマンドキューに積む.


    // コマンドキュー実行.


    // パスをクリア.
    m_PassList.Clear();

    // ダブルバッファリング.
    m_BufferIndex = (m_BufferIndex + 1) & 0x1;

    // ヒープリセット.
    m_FrameHeap[m_BufferIndex].Reset();
}

//-----------------------------------------------------------------------------
//      リソースを確保します.
//-----------------------------------------------------------------------------
PassResource* PassGraph::AllocResource(const PassResourceDesc& desc, RenderPass* producer)
{ return m_Registry.GetOrCreate(desc, producer); }

//-----------------------------------------------------------------------------
//      パスグラフを生成します.
//-----------------------------------------------------------------------------
bool CreatePassGraph(uint32_t maxPassCount, uint32_t maxResourceCount)
{
    auto instance = new(std::nothrow) PassGraph;
    if (instance == nullptr)
    {
        ELOG("Error : Out of Memory.");
        return false;
    }

    if (!instance->Init(maxPassCount, maxResourceCount))
    {
        ELOG("Error : Init() Failed.");
        return false;
    }

    return false;
}

} // namespace asdx

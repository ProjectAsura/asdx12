//-----------------------------------------------------------------------------
// File : asdxTarget.cpp
// Desc : Target Wrapper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <gfx/asdxTarget.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxDescriptorHeap.h>
#include <gfx/asdxGfxMisc.h>
#include <fnd/asdxLogger.h>
#include <D3D12MemAlloc.h>


namespace {

//-----------------------------------------------------------------------------
//      リソースを初期化します.
//-----------------------------------------------------------------------------
bool InitResource
(
    const asdx::TargetDesc* pDesc,
    D3D12_RESOURCE_FLAGS    flags,
    ID3D12Resource**        ppResource,
    ID3D12Heap**            ppHeap,
    asdx::AllocationHolder& holder
)
{
    HRESULT hr = S_OK;

    // リソースはSRGB指定なし.
    auto format = asdx::GetNoSRGBFormat(pDesc->Format);

    D3D12_HEAP_PROPERTIES props = {
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL_UNKNOWN,
        1,
        1
    };

    auto layout = (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
        ? D3D12_TEXTURE_LAYOUT_ROW_MAJOR
        : D3D12_TEXTURE_LAYOUT_UNKNOWN;

    D3D12_RESOURCE_DESC desc = {
        pDesc->Dimension,
        pDesc->Alignment,
        pDesc->Width,
        pDesc->Height,
        pDesc->DepthOrArraySize,
        pDesc->MipLevels,
        format,
        pDesc->SampleDesc,
        layout,
        flags
    };

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = format;

    if (!!(flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET))
    {
        clearValue.Color[0] = pDesc->ClearColor[0];
        clearValue.Color[1] = pDesc->ClearColor[1];
        clearValue.Color[2] = pDesc->ClearColor[2];
        clearValue.Color[3] = pDesc->ClearColor[3];
    }
    else if (!!(flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
    {
        clearValue.DepthStencil.Depth   = pDesc->ClearDepth;
        clearValue.DepthStencil.Stencil = pDesc->ClearStencil;
    }

    D3D12_CLEAR_VALUE* pClearValue = &clearValue;

    // UAVの場合はクリア値を指定しない.
    if (!!(flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS))
        pClearValue = nullptr;

    auto allocator = asdx::GetD3D12MA();
    if (allocator != nullptr)
    {
        D3D12MA::ALLOCATION_DESC allocDesc = {};
        allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

        D3D12MA::Allocation* pAllocation;

        hr = allocator->CreateResource(
            &allocDesc,
            &desc,
            pDesc->InitState,
            pClearValue,
            &pAllocation,
            IID_PPV_ARGS(ppResource));
        if (FAILED(hr))
        {
            ELOGA("Error : D3D12MA::Allocator::CreateResource() Failed. errcode = 0x%x", hr);
            return false;
        }

        holder = asdx::AllocationHolder(pAllocation);
    }
    else if (pDesc->HeapSize > 0)
    {
        D3D12_HEAP_DESC heapDesc = {};
        heapDesc.SizeInBytes    = pDesc->HeapSize;
        heapDesc.Properties     = props;
        heapDesc.Alignment      = pDesc->Alignment;

        ID3D12Heap* pHeap = nullptr;

        hr = asdx::GetD3D12Device()->CreateHeap(
            &heapDesc,
            IID_PPV_ARGS(&pHeap));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreateHeap() Failed. errcode = 0x%x", hr);
            return false;
        }

        hr = asdx::GetD3D12Device()->CreatePlacedResource(
            pHeap,
            pDesc->HeapOffset,
            &desc,
            pDesc->InitState,
            pClearValue,
            IID_PPV_ARGS(ppResource));
        if (FAILED(hr))
        {
            asdx::SafeRelease(pHeap);
            ELOG( "Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr );
            return false;
        }

        (*ppHeap) = pHeap;
    }
    else
    {
        hr = asdx::GetD3D12Device()->CreateCommittedResource( 
            &props,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            pDesc->InitState,
            pClearValue,
            IID_PPV_ARGS(ppResource));
        if (FAILED(hr))
        {
            ELOGA( "Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr );
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      エイリアシングリソースを初期化します.
//-----------------------------------------------------------------------------
bool InitAliasingResource
(
    const asdx::TargetDesc*         pDesc,
    const asdx::AllocationHolder&   srcHolder,
    ID3D12Heap*                     pHeap,
    ID3D12Resource**                ppResource,
    asdx::AllocationHolder&         dstHolder
)
{
    HRESULT hr = S_OK;

    // リソースはSRGB指定なし.
    auto format = asdx::GetNoSRGBFormat(pDesc->Format);

    D3D12_HEAP_PROPERTIES props = {
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL_UNKNOWN,
        1,
        1
    };

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = format;

    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
    if (!!(pDesc->Flags & asdx::TARGET_FLAG_ALLOW_RTV))
    {
        flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        clearValue.Color[0] = pDesc->ClearColor[0];
        clearValue.Color[1] = pDesc->ClearColor[1];
        clearValue.Color[2] = pDesc->ClearColor[2];
        clearValue.Color[3] = pDesc->ClearColor[3];
    }
    if (!!(pDesc->Flags & asdx::TARGET_FLAG_ALLOW_DSV))
    {
        flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        clearValue.DepthStencil.Depth   = pDesc->ClearDepth;
        clearValue.DepthStencil.Stencil = pDesc->ClearStencil;
    }
    if (!!(pDesc->Flags & asdx::TARGET_FLAG_ALLOW_UAV))
    {
        flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        clearValue.Color[0] = pDesc->ClearColor[0];
        clearValue.Color[1] = pDesc->ClearColor[1];
        clearValue.Color[2] = pDesc->ClearColor[2];
        clearValue.Color[3] = pDesc->ClearColor[3];
    }

    auto layout = (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
        ? D3D12_TEXTURE_LAYOUT_ROW_MAJOR
        : D3D12_TEXTURE_LAYOUT_UNKNOWN;

    D3D12_RESOURCE_DESC desc = {
        pDesc->Dimension,
        pDesc->Alignment,
        pDesc->Width,
        pDesc->Height,
        pDesc->DepthOrArraySize,
        pDesc->MipLevels,
        format,
        pDesc->SampleDesc,
        layout,
        flags
    };

    D3D12_CLEAR_VALUE* pClearValue = &clearValue;

    // UAVの場合はクリア値を指定しない.
    if (!!(flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS))
        pClearValue = nullptr;

    auto allocator = asdx::GetD3D12MA();
    if (allocator != nullptr)
    {
        D3D12MA::ALLOCATION_DESC allocDesc = {};
        allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

        auto pAlloc = srcHolder.GetPtr();
        if (pAlloc == nullptr)
        {
            ELOGA("Error : Invalid D3D12MA::Allocation.");
            return false;
        }

        auto offset = pAlloc->GetOffset() + pDesc->HeapOffset;

        hr = allocator->CreateAliasingResource(
            pAlloc,
            offset,
            &desc,
            pDesc->InitState,
            pClearValue,
            IID_PPV_ARGS(ppResource));
        if (FAILED(hr))
        {
            ELOG("Error : D3D12MA::Allocator::CreateResource() Failed. errcode = 0x%x", hr);
            return false;
        }

        dstHolder = asdx::AllocationHolder(pAlloc);
    }
    else
    {
        hr = asdx::GetD3D12Device()->CreatePlacedResource(
            pHeap,
            pDesc->HeapOffset,
            &desc,
            pDesc->InitState,
            pClearValue,
            IID_PPV_ARGS(ppResource));
        if (FAILED(hr))
        {
            asdx::SafeRelease(pHeap);
            ELOG( "Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr );
            return false;
        }
    }

    return true;
}

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// BaseTarget class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
BaseTarget::~BaseTarget()
{
    auto resource = m_pResource.Detach();
    Dispose(resource);

    auto heap = m_pHeap.Detach();
    Dispose(heap);

    m_HolderAlloc.Reset();
}

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* BaseTarget::GetResource() const
{ return m_pResource.GetPtr(); }

//-----------------------------------------------------------------------------
//      構成設定を取得します.
//-----------------------------------------------------------------------------
TargetDesc BaseTarget::GetDesc() const
{ return m_Desc; }

//-----------------------------------------------------------------------------
//      デバッグ名を設定します.
//-----------------------------------------------------------------------------
void BaseTarget::SetName(LPCWSTR tag)
{
    if (m_pResource)
    { m_pResource->SetName(tag); }
}


///////////////////////////////////////////////////////////////////////////////
// ColorTarget class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ColorTarget::ColorTarget()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ColorTarget::~ColorTarget()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理です.
//-----------------------------------------------------------------------------
bool ColorTarget::Init(const TargetDesc* pDesc)
{
    if (pDesc == nullptr)
    {
        ELOGA("Error : Invalid Argument");
        return false;
    }

    if (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
    {
        ELOGA("Error : Invalid Resource Dimension.");
        return false;
    }

    // リソースを初期化.
    auto flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    if (!InitResource(pDesc, flags, m_pResource.GetAddress(), m_pHeap.GetAddress(), m_HolderAlloc))
    {
        ELOGA("Error : InitResource() Failed.");
        return false;
    }

    // 構成設定とリソースステートを設定.
    memcpy(&m_Desc, pDesc, sizeof(m_Desc));
    m_PrevState = m_Desc.InitState;

    // ビューを初期化.
    if (!InitView())
    {
        ELOGA("Error : InitView() Failed.");
        return false;
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      エイリアシングリソースとして初期化します.
//-----------------------------------------------------------------------------
bool ColorTarget::InitAliasing(const TargetDesc* pDesc, const BaseTarget& target)
{
    if (pDesc == nullptr)
    {
        ELOGA("Error : Invalid Argument");
        return false;
    }

    if (!(pDesc->Flags & TARGET_FLAG_ALIASING_RESOURCE))
    {
        ELOGA("Error : Invalid Flags.");
        return false;
    }

    // リソースを初期化.
    if (!InitAliasingResource(pDesc, target.m_HolderAlloc, target.m_pHeap.GetPtr(), m_pResource.GetAddress(), m_HolderAlloc))
    {
        ELOGA("Error : InitAliasingResource() Failed.");
        return false;
    }

    // 構成設定とリソースステートを設定.
    memcpy(&m_Desc, pDesc, sizeof(m_Desc));
    m_PrevState = m_Desc.InitState;

    // ビューを初期化.
    if (!InitView())
    {
        ELOGA("Error : InitView() Failed.");
        return false;
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      初期化処理です.
//-----------------------------------------------------------------------------
bool ColorTarget::Init
(
    IDXGISwapChain* pSwapChain,
    uint32_t        backBufferIndex,
    bool            sRGB
)
{
    HRESULT hr = S_OK;

    hr = pSwapChain->GetBuffer( backBufferIndex, IID_PPV_ARGS(m_pResource.GetAddress()));
    if ( FAILED( hr ) )
    {
        ELOG( "Error : IDXGISwapChain::GetBuffer() Failed. errcode = 0x%x", hr );
        return false;
    }

    auto desc = m_pResource->GetDesc();
    if ( desc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D )
    {
        ELOG( "Error : Invalid Resource Dimension. ");
        return false;
    }

    if (sRGB)
    { desc.Format = GetSRGBFormat(desc.Format); }

    // 構成設定
    m_Desc.Dimension        = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    m_Desc.Alignment        = desc.Alignment;
    m_Desc.Width            = desc.Width;
    m_Desc.Height           = desc.Height;
    m_Desc.DepthOrArraySize = desc.DepthOrArraySize;
    m_Desc.MipLevels        = desc.MipLevels;
    m_Desc.Format           = desc.Format;
    m_Desc.SampleDesc       = desc.SampleDesc;
    m_Desc.InitState        = D3D12_RESOURCE_STATE_COMMON;
    m_Desc.ClearColor[0]    = 0.0f;
    m_Desc.ClearColor[1]    = 0.0f;
    m_Desc.ClearColor[2]    = 0.0f;
    m_Desc.ClearColor[3]    = 0.0f;
    m_Desc.ClearDepth       = 1.0f;
    m_Desc.ClearStencil     = 0;
    m_Desc.Flags            = TARGET_FLAG_NONE;

    // リソースステートを設定.
    m_PrevState = D3D12_RESOURCE_STATE_COMMON;

    // ビューを初期化
    if (!InitView())
    {
        ELOGA("Error : InitView() Failed.");
        return false;
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      ビューを初期化します.
//-----------------------------------------------------------------------------
bool ColorTarget::InitView()
{
#if ASDX_IS_SCARLETT
    auto mostDetailedMip = (pDesc->MipLevels - 1);
#else
    auto mostDetailedMip = 0u;
#endif

    const auto desc = m_Desc;

    std::vector<D3D12_RENDER_TARGET_VIEW_DESC> rtv_descs = {};

    if (!!(desc.Flags & TARGET_FLAG_CREATE_MIP_RTV) && desc.MipLevels > 1)
        rtv_descs.resize(desc.MipLevels);
    else
        rtv_descs.resize(1);

    for(size_t i=0; i<rtv_descs.size(); ++i)
    {
        auto& rtv_desc = rtv_descs[i];
        rtv_desc.Format = desc.Format;

        if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
        {
            rtv_desc.ViewDimension          = D3D12_RTV_DIMENSION_TEXTURE3D;
            rtv_desc.Texture3D.FirstWSlice  = 0;
            rtv_desc.Texture3D.MipSlice     = UINT(i);
            rtv_desc.Texture3D.WSize        = desc.DepthOrArraySize;
        }
        else if(desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
        {
            if (desc.DepthOrArraySize > 1)
            {
                if (desc.SampleDesc.Count > 1)
                {
                    rtv_desc.ViewDimension                    = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
                    rtv_desc.Texture2DMSArray.ArraySize       = desc.DepthOrArraySize;
                    rtv_desc.Texture2DMSArray.FirstArraySlice = 0;
                }
                else
                {
                    rtv_desc.ViewDimension                   = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                    rtv_desc.Texture2DArray.ArraySize        = desc.DepthOrArraySize;
                    rtv_desc.Texture2DArray.FirstArraySlice  = 0;
                    rtv_desc.Texture2DArray.MipSlice         = UINT(i);
                    rtv_desc.Texture2DArray.PlaneSlice       = 0;
                }
            }
            else
            {
                if (desc.SampleDesc.Count > 1)
                {
                    rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    rtv_desc.ViewDimension      = D3D12_RTV_DIMENSION_TEXTURE2D;
                    rtv_desc.Texture2D.MipSlice = UINT(i);
                }
            }
        }
        else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
        {
            if (desc.DepthOrArraySize > 1)
            {
                rtv_desc.ViewDimension                  = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                rtv_desc.Texture1DArray.ArraySize       = desc.DepthOrArraySize;
                rtv_desc.Texture1DArray.FirstArraySlice = 0;
                rtv_desc.Texture1DArray.MipSlice        = UINT(i);
            }
        }
        else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
        {
            // Not Support Buffer.
            assert(false);
            ELOGA("Error : Not Support Buffer Type.");
            return false;
        }
    }

    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    m_HolderRTV.resize(rtv_descs.size());
    for(size_t i=0; i<m_HolderRTV.size(); ++i)
    {
        auto handleRTV = GetRtvDescriptorHeap()->Alloc(1);
        if (!handleRTV.IsValid())
        {
            ELOG("Error : DescriptorHeap::Alloc() Failed.");
            return false;
        }

        DescriptorHolder holder(DescriptorHolder::HEAP_RTV, handleRTV);
        m_HolderRTV[i].Swap(holder);
        pDevice->CreateRenderTargetView(m_pResource.GetPtr(), &rtv_descs[i], GetCpuHandleRTV());
    }

    if (!(desc.Flags & TARGET_FLAG_NO_SRV))
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};

        srv_desc.Format = desc.Format; 
        srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
        {
            srv_desc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE3D;
            srv_desc.Texture3D.MipLevels            = desc.MipLevels;
            srv_desc.Texture3D.MostDetailedMip      = mostDetailedMip;
            srv_desc.Texture3D.ResourceMinLODClamp  = 0;
        }
        else if(desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
        {
            if (desc.DepthOrArraySize > 1)
            {
                if (desc.SampleDesc.Count > 1)
                {
                    srv_desc.ViewDimension                    = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
                    srv_desc.Texture2DMSArray.ArraySize       = desc.DepthOrArraySize;
                    srv_desc.Texture2DMSArray.FirstArraySlice = 0;
                }
                else
                {
                    srv_desc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                    srv_desc.Texture2DArray.ArraySize           = desc.DepthOrArraySize;
                    srv_desc.Texture2DArray.FirstArraySlice     = 0;
                    srv_desc.Texture2DArray.MipLevels           = 0;
                    srv_desc.Texture2DArray.PlaneSlice          = 0;
                    srv_desc.Texture2DArray.ResourceMinLODClamp = 0.0f;
                    srv_desc.Texture2DArray.MostDetailedMip     = mostDetailedMip;
                }
            }
            else
            {
                if (desc.SampleDesc.Count > 1)
                {
                    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    srv_desc.Texture2D.MipLevels            = desc.MipLevels;
                    srv_desc.Texture2D.MostDetailedMip      = mostDetailedMip;
                    srv_desc.Texture2D.PlaneSlice           = 0;
                    srv_desc.Texture2D.ResourceMinLODClamp  = 0.0f;
                    srv_desc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE2D;
                }
            }
        }
        else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
        {
            if (desc.DepthOrArraySize > 1)
            {
                srv_desc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                srv_desc.Texture1DArray.ArraySize           = desc.DepthOrArraySize;
                srv_desc.Texture1DArray.FirstArraySlice     = 0;
                srv_desc.Texture1DArray.MipLevels           = desc.MipLevels;
                srv_desc.Texture1DArray.MostDetailedMip     = mostDetailedMip;
                srv_desc.Texture1DArray.ResourceMinLODClamp = 0;
            }
        }
        else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
        {
            // Not Support Buffer.
            assert(false);
            ELOGA("Error : Not Support Buffer Type.");
            return false;
        }

        auto handleSRV = GetResourceDescriptorHeap()->Alloc(1);
        if (!handleSRV.IsValid())
        {
            ELOG("Error : DescriptorHeap::Alloc() Failed.");
            return false;
        }

        DescriptorHolder holder(DescriptorHolder::HEAP_RES, handleSRV);
        m_HolderSRV.Swap(holder);
        pDevice->CreateShaderResourceView(m_pResource.GetPtr(), &srv_desc, GetCpuHandleSRV());
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void ColorTarget::Term()
{
    m_HolderAlloc.Reset();

    for(size_t i=0; i<m_HolderRTV.size(); ++i)
    {
        m_HolderRTV[i].Reset();
    }
    m_HolderRTV.clear();
    m_HolderRTV.shrink_to_fit();

    m_HolderSRV.Reset();

    auto resource = m_pResource.Detach();
    Dispose(resource);

    memset(&m_Desc, 0, sizeof(m_Desc));
}

//-----------------------------------------------------------------------------
//      リサイズ処理を行います.
//-----------------------------------------------------------------------------
bool ColorTarget::Resize(uint32_t width, uint32_t height)
{
    auto desc = m_Desc;
    Term();

    desc.Width  = width;
    desc.Height = height;
    return Init(&desc);
}

//-----------------------------------------------------------------------------
//      レンダーターゲットビュー用CPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE ColorTarget::GetCpuHandleRTV(uint32_t index) const
{
    assert(index < m_HolderRTV.size());
    return m_HolderRTV[index].GetHandleCPU();
}

//-----------------------------------------------------------------------------
//      シェーダリソースビュー用バインドレスインデックスを取得します.
//-----------------------------------------------------------------------------
uint32_t ColorTarget::GetBindlessIndexSRV() const
{ return m_HolderSRV.GetIndex(); }

//-----------------------------------------------------------------------------
//      シェーダリソースビュー用CPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE ColorTarget::GetCpuHandleSRV() const
{ return m_HolderSRV.GetHandleCPU(); }

//-----------------------------------------------------------------------------
//      シェーダリソースビュー用GPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE ColorTarget::GetGpuHandleSRV() const
{ return m_HolderSRV.GetHandleGPU(); }

//-----------------------------------------------------------------------------
//      sRGBフラグを取得します.
//-----------------------------------------------------------------------------
bool ColorTarget::IsSRGB() const
{ return IsSRGBFormat(m_Desc.Format); }


///////////////////////////////////////////////////////////////////////////////
// DepthTarget class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
DepthTarget::DepthTarget()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
DepthTarget::~DepthTarget()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool DepthTarget::Init(const TargetDesc* pDesc)
{
    if (pDesc == nullptr)
    {
        ELOGA("Error : Invalid Argument");
        return false;
    }

    if (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_BUFFER
     || pDesc->Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
    {
        ELOGA("Error : Invalid Resource Dimension");
        return false;
    }

    // リソースを初期化.
    auto flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    if (!InitResource(pDesc, flags, m_pResource.GetAddress(), m_pHeap.GetAddress(), m_HolderAlloc))
    {
        ELOGA("Error : InitResource() Failed.");
        return false;
    }

    // 構成設定とリソースステートを設定.
    memcpy(&m_Desc, pDesc, sizeof(m_Desc));
    m_PrevState = m_Desc.InitState;

    // ビューを初期化.
    if (!InitView())
    {
        ELOGA("Error : InitView() Failed.");
        return false;
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      エイリアシングリソースとして初期化します.
//-----------------------------------------------------------------------------
bool DepthTarget::InitAliasing(const TargetDesc* pDesc, const BaseTarget& target)
{
    if (pDesc == nullptr)
    {
        ELOGA("Error : Invalid Arguement.");
        return false;
    }

    if (!(pDesc->Flags & TARGET_FLAG_ALIASING_RESOURCE))
    {
        ELOGA("Error : Invalid Flags.");
        return false;
    }

    // リソースを初期化.
    if (!InitAliasingResource(pDesc, target.m_HolderAlloc, target.m_pHeap.GetPtr(), m_pResource.GetAddress(), m_HolderAlloc))
    {
        ELOGA("Error : InitAliasingResource() Failed.");
        return false;
    }

    // 構成設定とリソースステートを設定.
    memcpy(&m_Desc, pDesc, sizeof(m_Desc));
    m_PrevState = m_Desc.InitState;

    // ビューを初期化.
    if (!InitView())
    {
        ELOGA("Error : InitView() Failed.");
        return false;
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      ビューを初期化します.
//-----------------------------------------------------------------------------
bool DepthTarget::InitView()
{
    #if ASDX_IS_SCARLETT
        auto mostDetailedMip = (pDesc->MipLevels - 1);
    #else
        auto mostDetailedMip = 0u;
    #endif

    const auto desc = m_Desc;

    std::vector<D3D12_DEPTH_STENCIL_VIEW_DESC> dsv_descs = {};
    if (!!(desc.Flags & TARGET_FLAG_CREATE_MIP_DSV) && desc.MipLevels > 1)
        dsv_descs.resize(desc.MipLevels);
    else
        dsv_descs.resize(1);

    for(size_t i=0; i<dsv_descs.size(); ++i)
    {
        auto& dsv_desc = dsv_descs[i];
        dsv_desc.Format = desc.Format;

        if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
        {
            if (desc.DepthOrArraySize > 1)
            {
                if (desc.SampleDesc.Count > 1)
                {
                    dsv_desc.ViewDimension                      = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
                    dsv_desc.Texture2DMSArray.ArraySize         = desc.DepthOrArraySize;
                    dsv_desc.Texture2DMSArray.FirstArraySlice   = 0;
                }
                else
                {
                    dsv_desc.ViewDimension                  = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                    dsv_desc.Texture2DArray.ArraySize       = desc.DepthOrArraySize;
                    dsv_desc.Texture2DArray.FirstArraySlice = 0;
                    dsv_desc.Texture2DArray.MipSlice        = UINT(i);
                }
            }
            else
            {
                if (desc.SampleDesc.Count > 1)
                {
                    dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    dsv_desc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
                    dsv_desc.Texture2D.MipSlice = UINT(i);
                }
            }

        }
        else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
        {
            if (desc.DepthOrArraySize > 1)
            {
                dsv_desc.ViewDimension                  = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
                dsv_desc.Texture1DArray.ArraySize       = desc.DepthOrArraySize;
                dsv_desc.Texture1DArray.FirstArraySlice = 0;
                dsv_desc.Texture1DArray.MipSlice        = UINT(i);
            }
            else
            {
                dsv_desc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE1D;
                dsv_desc.Texture1D.MipSlice = UINT(i);
            }
        }
        else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
        {
            // Not Support Buffer.
            assert(false);
            ELOGA("Error : Not Support Buffer Type.");
            return false;
        }
    }

    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    m_HolderDSV.resize(dsv_descs.size());
    for(size_t i=0; i<m_HolderDSV.size(); ++i)
    {
        auto handleDSV = GetDsvDescriptorHeap()->Alloc(1);
        if (!handleDSV.IsValid())
        {
            ELOG("Error : DescriptorHeap::Alloc() Failed.");
            return false;
        }
        DescriptorHolder holder(DescriptorHolder::HEAP_DSV, handleDSV);
        m_HolderDSV[i].Swap(holder);
        pDevice->CreateDepthStencilView(m_pResource.GetPtr(), &dsv_descs[i], GetCpuHandleDSV());
    }

    if (!(desc.Flags & TARGET_FLAG_NO_SRV))
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
        srv_desc.Format = GetResourceFormat(desc.Format, false);
        srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
        {
            if (desc.DepthOrArraySize > 1)
            {
                if (desc.SampleDesc.Count > 1)
                {
                    srv_desc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
                    srv_desc.Texture2DMSArray.ArraySize         = desc.DepthOrArraySize;
                    srv_desc.Texture2DMSArray.FirstArraySlice   = 0;
                }
                else
                {
                    srv_desc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                    srv_desc.Texture2DArray.ArraySize           = desc.DepthOrArraySize;
                    srv_desc.Texture2DArray.FirstArraySlice     = 0;
                    srv_desc.Texture2DArray.MipLevels           = desc.MipLevels;
                    srv_desc.Texture2DArray.MostDetailedMip     = mostDetailedMip;
                    srv_desc.Texture2DArray.PlaneSlice          = 0;
                    srv_desc.Texture2DArray.ResourceMinLODClamp = 0;
                }
            }
            else
            {
                if (desc.SampleDesc.Count > 1)
                {
                    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    srv_desc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE2D;
                    srv_desc.Texture2D.MipLevels            = desc.MipLevels;
                    srv_desc.Texture2D.MostDetailedMip      = mostDetailedMip;
                    srv_desc.Texture2D.PlaneSlice           = 0;
                    srv_desc.Texture2D.ResourceMinLODClamp  = 0;
                }
            }

        }
        else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
        {
            if (desc.DepthOrArraySize > 1)
            {
                srv_desc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                srv_desc.Texture1DArray.ArraySize           = desc.DepthOrArraySize;
                srv_desc.Texture1DArray.FirstArraySlice     = 0;
                srv_desc.Texture1DArray.MipLevels           = desc.MipLevels;
                srv_desc.Texture1DArray.MostDetailedMip     = mostDetailedMip;
                srv_desc.Texture1DArray.ResourceMinLODClamp = 0;
            }
            else
            {
                srv_desc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE1D;
                srv_desc.Texture1D.MipLevels            = desc.MipLevels;
                srv_desc.Texture1D.MostDetailedMip      = mostDetailedMip;
                srv_desc.Texture1D.ResourceMinLODClamp  = 0;
            }
        }
        else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
        {
            // Not Support Buffer.
            assert(false);
            ELOGA("Error : Not Support Buffer Type.");
            return false;
        }

        auto handleSRV = GetResourceDescriptorHeap()->Alloc(1);
        if (!handleSRV.IsValid())
        {
            ELOG("Error : DescriptorHeap::Alloc() Failed.");
            return false;
        }
        {
            DescriptorHolder holder(DescriptorHolder::HEAP_RES, handleSRV);
            m_HolderSRV.Swap(holder);
        }
        pDevice->CreateShaderResourceView(m_pResource.GetPtr(), &srv_desc, GetCpuHandleSRV());
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void DepthTarget::Term()
{
    m_HolderAlloc.Reset();

    for(size_t i=0; i<m_HolderDSV.size(); ++i)
    {
        m_HolderDSV[i].Reset();
    }
    m_HolderDSV.clear();
    m_HolderDSV.shrink_to_fit();

    m_HolderSRV.Reset();

    auto resource = m_pResource.Detach();
    Dispose(resource);

    memset(&m_Desc, 0, sizeof(m_Desc));
}

//-----------------------------------------------------------------------------
//      リサイズ処理を行います.
//-----------------------------------------------------------------------------
bool DepthTarget::Resize(uint32_t width, uint32_t height)
{
    auto desc = m_Desc;
    Term();

    desc.Width  = width;
    desc.Height = height;
    return Init(&desc);
}

//-----------------------------------------------------------------------------
//      深度ステンシルビュー用CPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE DepthTarget::GetCpuHandleDSV(uint32_t index) const
{
    assert(index < m_HolderDSV.size());
    return m_HolderDSV[index].GetHandleCPU();
}

//-----------------------------------------------------------------------------
//      シェーダリソースビュー用バインドレスインデックスを取得します.
//-----------------------------------------------------------------------------
uint32_t DepthTarget::GetBindlessIndexSRV() const
{ return m_HolderSRV.GetIndex(); }

//-----------------------------------------------------------------------------
//      シェーダリソースビュー用CPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE DepthTarget::GetCpuHandleSRV() const
{ return m_HolderSRV.GetHandleCPU(); }

//-----------------------------------------------------------------------------
//      シェーダリソースビュー用CPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE DepthTarget::GetGpuHandleSRV() const
{ return m_HolderSRV.GetHandleGPU(); }


///////////////////////////////////////////////////////////////////////////////
// ComputeTarget class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ComputeTarget::ComputeTarget()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ComputeTarget::~ComputeTarget()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool ComputeTarget::Init(const TargetDesc* pDesc, uint32_t stride)
{
    if (pDesc == nullptr)
    {
        ELOGA("Error : Invalid Argument");
        return false;
    }

    if (pDesc->SampleDesc.Count > 1)
    {
        ELOGA("Error : Invalid Argument");
        return false;
    }

    // リソースを初期化.
    auto flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    if (!InitResource(pDesc, flags, m_pResource.GetAddress(), m_pHeap.GetAddress(), m_HolderAlloc))
    {
        ELOGA("Error : InitResource() Failed.");
        return false;
    }

    // 構成設定・リソースステート・構造体サイズを設定.
    memcpy(&m_Desc, pDesc, sizeof(m_Desc));
    m_PrevState = m_Desc.InitState;
    m_Stride    = stride;

    // ビューを初期化.
    if (!InitView())
    {
        ELOGA("Error : InitView() Failed.");
        return false;
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool ComputeTarget::Init(ColorTarget& target, uint32_t flags)
{
    // リソースを設定. 参照カウントが増えます.
    m_pResource = target.GetResource();

    // 構成設定・リソースステート・構造体サイズを設定.
    m_Desc      = target.GetDesc();
    m_PrevState = m_Desc.InitState;
    m_Stride    = 0;

    m_Desc.Flags = flags;

    // ビューを初期化.
    if (!InitView())
    {
        ELOGA("Error : InitView() Failed.");
        return false;
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      エイリアシングリソースとして初期化します.
//-----------------------------------------------------------------------------
bool ComputeTarget::InitAliasing(const TargetDesc* pDesc, uint32_t stride, const BaseTarget& target)
{
    if (pDesc == nullptr)
    {
        ELOGA("Error : Invalid Argument.");
        return false;
    }

    if (!(pDesc->Flags & TARGET_FLAG_ALIASING_RESOURCE))
    {
        ELOGA("Error : Invalid Flag.");
        return false;
    }

    // リソースを初期化.
    if (!InitAliasingResource(pDesc, target.m_HolderAlloc, target.m_pHeap.GetPtr(), m_pResource.GetAddress(), m_HolderAlloc))
    {
        ELOGA("Error : InitAliasingResource() Failed.");
        return false;
    }


    // 構成設定・リソースステート・構造体サイズを設定.
    memcpy(&m_Desc, pDesc, sizeof(m_Desc));
    m_PrevState = m_Desc.InitState;
    m_Stride    = stride;

    // ビューを初期化.
    if (!InitView())
    {
        ELOGA("Error : InitView() Failed.");
        return false;
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      ビューの初期化を行います.
//-----------------------------------------------------------------------------
bool ComputeTarget::InitView()
{
#if ASDX_IS_SCARLETT
    auto mostDetailedMip = (pDesc->MipLevels - 1);
#else
    auto mostDetailedMip = 0u;
#endif

    const auto desc = m_Desc;

    std::vector<D3D12_UNORDERED_ACCESS_VIEW_DESC> uav_descs = {};
    if (!!(desc.Flags & TARGET_FLAG_CREATE_MIP_UAV) && desc.MipLevels > 1)
        uav_descs.resize(desc.MipLevels);
    else
        uav_descs.resize(1);

    for(size_t i=0; i<uav_descs.size(); ++i)
    {
        auto& uav_desc = uav_descs[i];
        uav_desc.Format = desc.Format;

        if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
        {
            uav_desc.ViewDimension          = D3D12_UAV_DIMENSION_TEXTURE3D;
            uav_desc.Texture3D.FirstWSlice  = 0;
            uav_desc.Texture3D.MipSlice     = UINT(i);
            uav_desc.Texture3D.WSize        = desc.DepthOrArraySize;
        }
        else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
        {
            if (desc.DepthOrArraySize > 1)
            {
                uav_desc.ViewDimension                      = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                uav_desc.Texture2DArray.ArraySize           = desc.DepthOrArraySize;
                uav_desc.Texture2DArray.FirstArraySlice     = 0;
                uav_desc.Texture2DArray.MipSlice            = UINT(i);
                uav_desc.Texture2DArray.PlaneSlice          = 0;
            }

            else
            {
                uav_desc.ViewDimension          = D3D12_UAV_DIMENSION_TEXTURE2D;
                uav_desc.Texture2D.MipSlice     = UINT(i);
                uav_desc.Texture2D.PlaneSlice   = 0;
            }
        }
        else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
        {
            if (desc.DepthOrArraySize > 1)
            {
                uav_desc.ViewDimension                  = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
                uav_desc.Texture1DArray.ArraySize       = desc.DepthOrArraySize;
                uav_desc.Texture1DArray.FirstArraySlice = 0;
                uav_desc.Texture1DArray.MipSlice        = UINT(i);
            }
            else
            {
                uav_desc.ViewDimension      = D3D12_UAV_DIMENSION_TEXTURE1D;
                uav_desc.Texture1D.MipSlice = UINT(i);
            }
        }
        else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
        {
            auto elements = (m_Stride == 0) ? desc.Width : desc.Width / m_Stride;

            uav_desc.ViewDimension                  = D3D12_UAV_DIMENSION_BUFFER;
            uav_desc.Buffer.CounterOffsetInBytes    = 0;
            uav_desc.Buffer.FirstElement            = 0;
            uav_desc.Buffer.NumElements             = UINT(elements);
            uav_desc.Buffer.StructureByteStride     = m_Stride;
            uav_desc.Buffer.Flags                   = (m_Stride == 0) ? D3D12_BUFFER_UAV_FLAG_RAW : D3D12_BUFFER_UAV_FLAG_NONE;
        }
    }

    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    m_HolderUAV.resize(uav_descs.size());
    for(uint32_t i=0; i<m_HolderUAV.size(); ++i)
    {
        auto handleUAV = GetResourceDescriptorHeap()->Alloc(1);
        if (!handleUAV.IsValid())
        {
            ELOG("Error : DescriptorHeap::Alloc() Failed.");
            return false;
        }
        DescriptorHolder holder(DescriptorHolder::HEAP_RES, handleUAV);
        m_HolderUAV[i].Swap(holder);
        pDevice->CreateUnorderedAccessView(m_pResource.GetPtr(), nullptr, &uav_descs[i], GetCpuHandleUAV(i));
    }

    m_HolderClearUAV.resize(uav_descs.size());
    for(uint32_t i=0; i<m_HolderClearUAV.size(); ++i)
    {
        auto handleUAV = GetCpuResourceDescriptorHeap()->Alloc(1);
        if (!handleUAV.IsValid())
        {
            ELOG("Error : DescriptorHeap::Alloc() Failed.");
            return false;
        }
        DescriptorHolder holder(DescriptorHolder::HEAP_RES_CPU, handleUAV);
        m_HolderClearUAV[i].Swap(holder);
        pDevice->CreateUnorderedAccessView(m_pResource.GetPtr(), nullptr, &uav_descs[i], GetClearCpuHandleUAV(i));
    }

    if (!(desc.Flags & TARGET_FLAG_NO_SRV))
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
        srv_desc.Format = desc.Format; 
        srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
        {
            srv_desc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE3D;
            srv_desc.Texture3D.MipLevels            = desc.MipLevels;
            srv_desc.Texture3D.MostDetailedMip      = mostDetailedMip;
            srv_desc.Texture3D.ResourceMinLODClamp  = 0;
        }
        else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
        {
            if (desc.DepthOrArraySize > 1)
            {
                srv_desc.Texture2DArray.ArraySize           = desc.DepthOrArraySize;
                srv_desc.Texture2DArray.FirstArraySlice     = 0;
                srv_desc.Texture2DArray.MipLevels           = 0;
                srv_desc.Texture2DArray.PlaneSlice          = 0;
                srv_desc.Texture2DArray.ResourceMinLODClamp = 0.0f;
                srv_desc.Texture2DArray.MostDetailedMip     = mostDetailedMip;
                srv_desc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            }

            else
            {
                srv_desc.Texture2D.MipLevels            = desc.MipLevels;
                srv_desc.Texture2D.MostDetailedMip      = mostDetailedMip;
                srv_desc.Texture2D.PlaneSlice           = 0;
                srv_desc.Texture2D.ResourceMinLODClamp  = 0.0f;
                srv_desc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE2D;

            }
        }
        else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
        {
            if (desc.DepthOrArraySize > 1)
            {
                srv_desc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                srv_desc.Texture1DArray.ArraySize           = desc.DepthOrArraySize;
                srv_desc.Texture1DArray.FirstArraySlice     = 0;
                srv_desc.Texture1DArray.MipLevels           = desc.MipLevels;
                srv_desc.Texture1DArray.MostDetailedMip     = mostDetailedMip;
                srv_desc.Texture1DArray.ResourceMinLODClamp = 0;
            }
            else
            {
                srv_desc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE1D;
                srv_desc.Texture1D.MipLevels            = desc.MipLevels;
                srv_desc.Texture1D.MostDetailedMip      = mostDetailedMip;
                srv_desc.Texture1D.ResourceMinLODClamp  = 0;
            }
        }
        else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
        {
            auto elements = (m_Stride == 0) ? desc.Width : desc.Width / m_Stride;
            srv_desc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
            srv_desc.Buffer.FirstElement        = 0;
            srv_desc.Buffer.NumElements         = UINT(elements);
            srv_desc.Buffer.StructureByteStride = m_Stride;
            srv_desc.Buffer.Flags               = (m_Stride == 0) ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;
        }

        auto handleSRV = GetResourceDescriptorHeap()->Alloc(1);
        if (!handleSRV.IsValid())
        {
            ELOG("Error : DescriptorHeap::Alloc() Failed.");
            return false;
        }
        {
            DescriptorHolder holder(DescriptorHolder::HEAP_RES, handleSRV);
            m_HolderSRV.Swap(holder);
        }
        pDevice->CreateShaderResourceView(m_pResource.GetPtr(), &srv_desc, GetCpuHandleSRV());
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void ComputeTarget::Term()
{
    m_HolderAlloc.Reset();
    for(size_t i=0; i<m_HolderUAV.size(); ++i)
    {
        m_HolderUAV[i].Reset();
    }
    m_HolderUAV.clear();
    m_HolderUAV.shrink_to_fit();

    for(size_t i=0; i<m_HolderClearUAV.size(); ++i)
    {
        m_HolderClearUAV[i].Reset();
    }
    m_HolderClearUAV.clear();
    m_HolderClearUAV.shrink_to_fit();

    m_HolderSRV.Reset();

    auto resource = m_pResource.Detach();
    Dispose(resource);
 
    memset(&m_Desc, 0, sizeof(m_Desc));
    m_Stride = 0;
}

//-----------------------------------------------------------------------------
//      リサイズ処理を行います.
//-----------------------------------------------------------------------------
bool ComputeTarget::Resize(uint32_t width, uint32_t height, uint16_t depth)
{
    auto desc   = m_Desc;
    auto stride = m_Stride;
    Term();

    desc.Width            = width;
    desc.Height           = height;
    desc.DepthOrArraySize = depth;
    return Init(&desc, stride);
}

//-----------------------------------------------------------------------------
//      アンオーダードアクセスビュー用オフセットハンドルを取得します.
//-----------------------------------------------------------------------------
uint32_t ComputeTarget::GetBindlessIndexUAV(uint32_t index) const
{
    assert(index < m_HolderUAV.size());
    return m_HolderUAV[index].GetIndex();
}

//-----------------------------------------------------------------------------
//      アンオーダードアクセスビュー用CPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE ComputeTarget::GetCpuHandleUAV(uint32_t index) const
{
    assert(index < m_HolderUAV.size());
    return m_HolderUAV[index].GetHandleCPU();
}

//-----------------------------------------------------------------------------
//      アンオーダードアクセスビュー用GPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE ComputeTarget::GetGpuHandleUAV(uint32_t index) const
{
    assert(index < m_HolderUAV.size());
    return m_HolderUAV[index].GetHandleGPU();
}

//-----------------------------------------------------------------------------
//      アンオーダードアクセスビュー用CPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE ComputeTarget::GetClearCpuHandleUAV(uint32_t index) const
{
    assert(index < m_HolderClearUAV.size());
    return m_HolderClearUAV[index].GetHandleCPU();
}

//-----------------------------------------------------------------------------
//      シェーダリソースビュー用バインドレスインデックスを取得します.
//-----------------------------------------------------------------------------
uint32_t ComputeTarget::GetBindlessIndexSRV() const
{ return m_HolderSRV.GetIndex(); }

//-----------------------------------------------------------------------------
//      シェーダリソースビュー用CPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE ComputeTarget::GetCpuHandleSRV() const
{ return m_HolderSRV.GetHandleCPU(); }

//-----------------------------------------------------------------------------
//      シェーダリソースビュー用GPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE ComputeTarget::GetGpuHandleSRV() const
{ return m_HolderSRV.GetHandleGPU(); }

//-----------------------------------------------------------------------------
//      ストライドサイズを取得します.
//-----------------------------------------------------------------------------
uint32_t ComputeTarget::GetStride() const
{ return m_Stride; }

//-----------------------------------------------------------------------------
//      UAVバリアを設定します.
//-----------------------------------------------------------------------------
void ComputeTarget::UAVBarrier(ID3D12GraphicsCommandList* pCmdList)
{
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type            = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.UAV.pResource   = m_pResource.GetPtr();

    pCmdList->ResourceBarrier(1, &barrier);
}

} // namespace asdx

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


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// BaseTarget class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
BaseTarget::BaseTarget()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
BaseTarget::~BaseTarget()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理です.
//-----------------------------------------------------------------------------
bool BaseTarget::Init(const TargetDesc* pDesc)
{
    HRESULT hr = S_OK;

    D3D12_HEAP_PROPERTIES props = {
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL_UNKNOWN,
        1,
        1
    };

    D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;
    if (!!(pDesc->Flags & TARGET_FLAG_ALLOW_RTV))
        resFlags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    if (!!(pDesc->Flags & TARGET_FLAG_ALLOW_DSV))
        resFlags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    if (!!(pDesc->Flags & TARGET_FLAG_ALLOW_UAV))
        resFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    D3D12_RESOURCE_DESC desc = {
        pDesc->Dimension,
        pDesc->Alignment,
        pDesc->Width,
        pDesc->Height,
        pDesc->DepthOrArraySize,
        pDesc->MipLevels,
        pDesc->Format,
        pDesc->SampleDesc,
        D3D12_TEXTURE_LAYOUT_UNKNOWN,
        resFlags,
    };

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format   = pDesc->Format,
    clearValue.Color[0] = pDesc->ClearColor[0];
    clearValue.Color[1] = pDesc->ClearColor[1];
    clearValue.Color[2] = pDesc->ClearColor[2];
    clearValue.Color[3] = pDesc->ClearColor[3];

    auto allocator = GetD3D12MA();
    if (allocator != nullptr)
    {
        D3D12MA::ALLOCATION_DESC allocDesc = {};
        allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

        D3D12MA::Allocation* pAllocation;

        hr = allocator->CreateResource(
            &allocDesc,
            &desc,
            pDesc->InitState,
            &clearValue,
            &pAllocation,
            IID_PPV_ARGS(m_pResource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : D3D12MA::Allocator::CreateResource() Failed. errcode = 0x%x", hr);
            return false;
        }

        m_HolderAlloc = AllocationHolder(pAllocation);
    }
    else if (!!(pDesc->Flags & TARGET_FLAG_RESOURCE_ALIASING))
    {
        D3D12_HEAP_DESC heapDesc = {};
        heapDesc.SizeInBytes    = pDesc->HeapSize;
        heapDesc.Alignment      = pDesc->Alignment;
        heapDesc.Properties     = props;

        hr = GetD3D12Device()->CreateHeap(
            &heapDesc,
            IID_PPV_ARGS(m_pHeap.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateHeap() Failed. errcode = 0x%x", hr);
            return false;
        }

        hr = GetD3D12Device()->CreatePlacedResource(
            m_pHeap.GetPtr(),
            pDesc->HeapOffset,
            &desc,
            pDesc->InitState,
            &clearValue,
            IID_PPV_ARGS(m_pResource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreatePlacedResource() Failed. errcode = 0x%x", hr);
            return false;
        }
    }
    else
    {
        hr = GetD3D12Device()->CreateCommittedResource( 
            &props,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            pDesc->InitState,
            &clearValue,
            IID_PPV_ARGS(m_pResource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG( "Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr );
            return false;
        }
    }

    m_Desc      = (*pDesc);
    m_PrevState = pDesc->InitState;

    return true;
}

//-----------------------------------------------------------------------------
//      エイリアシングリソースとして初期化します.
//-----------------------------------------------------------------------------
bool BaseTarget::InitAsAliasing(const TargetDesc* pDesc, const BaseTarget& target)
{
    if (pDesc == nullptr)
    {
        ELOGA("Error : Invalid Arguments.");
        return false;
    }

    HRESULT hr = S_OK;

    D3D12_HEAP_PROPERTIES props = {
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL_UNKNOWN,
        1,
        1
    };

    D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;
    if (!!(pDesc->Flags & TARGET_FLAG_ALLOW_RTV))
        resFlags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    if (!!(pDesc->Flags & TARGET_FLAG_ALLOW_DSV))
        resFlags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    if (!!(pDesc->Flags & TARGET_FLAG_ALLOW_UAV))
        resFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    D3D12_RESOURCE_DESC desc = {
        pDesc->Dimension,
        pDesc->Alignment,
        pDesc->Width,
        pDesc->Height,
        pDesc->DepthOrArraySize,
        pDesc->MipLevels,
        pDesc->Format,
        pDesc->SampleDesc,
        D3D12_TEXTURE_LAYOUT_UNKNOWN,
        resFlags,
    };

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format   = pDesc->Format,
    clearValue.Color[0] = pDesc->ClearColor[0];
    clearValue.Color[1] = pDesc->ClearColor[1];
    clearValue.Color[2] = pDesc->ClearColor[2];
    clearValue.Color[3] = pDesc->ClearColor[3];

    ID3D12Heap* pHeap = target.m_pHeap.GetPtr();

    auto allocator = GetD3D12MA();
    if (allocator != nullptr)
    {
        D3D12MA::ALLOCATION_DESC allocDesc = {};
        allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

        D3D12MA::Allocation* pAllocation = target.m_HolderAlloc.GetPtr();

        auto offset = pAllocation->GetOffset();
        offset += pDesc->HeapOffset;

        hr = allocator->CreateAliasingResource(
            pAllocation,
            offset,
            &desc,
            pDesc->InitState,
            &clearValue,
            IID_PPV_ARGS(m_pResource.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : D3D12MA::Allocator::CreateResource() Failed. errcode = 0x%x", hr);
            return false;
        }

        m_HolderAlloc = AllocationHolder(pAllocation);
    }
    else if (pHeap != nullptr)
    {
        hr = GetD3D12Device()->CreatePlacedResource(
            pHeap,
            pDesc->HeapOffset,
            &desc,
            pDesc->InitState,
            &clearValue,
            IID_PPV_ARGS(m_pResource.GetAddress()));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Device::CreatePlacedResource() Failed. errcode = 0x%x", hr);
            return false;
        }

        m_pHeap.Attach(pHeap);
    }
    else
    {
        ELOGA("Error : BaseTarget::InitAsAliasing() Failed. Heap is nullptr.");
        return false;
    }

    m_Desc      = (*pDesc);
    m_PrevState = pDesc->InitState;

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void BaseTarget::Term()
{
    m_HolderAlloc.Reset();
    {
        auto resource = m_pResource.Detach();
        Dispose(resource);
    }
    {
        auto heap = m_pHeap.Detach();
        Dispose(heap);
    }
    m_Desc = {};
}

//-----------------------------------------------------------------------------
//      リサイズ処理を行います.
//-----------------------------------------------------------------------------
bool BaseTarget::Resize(uint32_t width, uint32_t height, uint16_t depthOrArraySize)
{
    auto desc = m_Desc;
    Term();

    desc.Width            = width;
    desc.Height           = height;
    desc.DepthOrArraySize = depthOrArraySize;
    return Init(&desc);
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

    auto desc = (*pDesc);
    desc.Format = GetNoSRGBFormat(pDesc->Format);
    desc.Flags |= TARGET_FLAG_ALLOW_RTV;
    if (!base::Init(&desc))
    {
        ELOGA("Error : BaseTarget::Init() Failed.");
        return false;
    }

    if (!InitView(pDesc))
    {
        ELOGA("Error : InitView() Failed.");
        return false;
    }

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

    hr = pSwapChain->GetBuffer(backBufferIndex, IID_PPV_ARGS(m_pResource.GetAddress()));
    if (FAILED(hr))
    {
        ELOG( "Error : IDXGISwapChain::GetBuffer() Failed. errcode = 0x%x", hr );
        return false;
    }

    auto desc = m_pResource->GetDesc();
    if (desc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D)
    {
        ELOGA( "Error : Invalid Resource Dimension. ");
        return false;
    }

    if (sRGB)
    { desc.Format = GetSRGBFormat(desc.Format); }

    TargetDesc targetDesc = {};
    targetDesc.Alignment            = desc.Alignment;
    targetDesc.Width                = desc.Width;
    targetDesc.Height               = desc.Height;
    targetDesc.DepthOrArraySize     = desc.DepthOrArraySize;
    targetDesc.MipLevels            = desc.MipLevels;
    targetDesc.Format               = desc.Format;
    targetDesc.SampleDesc.Count     = desc.SampleDesc.Count;
    targetDesc.SampleDesc.Quality   = desc.SampleDesc.Quality;
    targetDesc.InitState            = D3D12_RESOURCE_STATE_COMMON;
    targetDesc.ClearColor[0]        = 1.0f;
    targetDesc.ClearColor[1]        = 1.0f;
    targetDesc.ClearColor[2]        = 1.0f;
    targetDesc.ClearColor[3]        = 1.0f;
    targetDesc.Flags                = TARGET_FLAG_ALLOW_RTV;

    m_Desc      = targetDesc;
    m_PrevState = D3D12_RESOURCE_STATE_COMMON;

    if (!InitView(&targetDesc))
    {
        ELOGA("Error : InitView() Failed.");
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------
//      エイリアシングリソースとして初期化します
//----------------------------------------------------------------------------
bool ColorTarget::InitAsAliasing(const TargetDesc* pDesc, const BaseTarget& target)
{
    auto desc = (*pDesc);
    desc.Format = GetNoSRGBFormat(pDesc->Format);
    if (!base::InitAsAliasing(&desc, target))
    {
        ELOGA("Error : BaseTarget::InitAsAliasing() Failed.");
        return false;
    }

    if (!InitView(pDesc))
    {
        ELOGA("Error : InitView() Failed.");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      ビューを初期化します.
//-----------------------------------------------------------------------------
bool ColorTarget::InitView(const TargetDesc* pDesc)
{
#if ASDX_IS_SCARLETT
    auto mostDetailedMip = (pDesc->MipLevels - 1);
#else
    auto mostDetailedMip = 0u;
#endif

    std::vector<D3D12_RENDER_TARGET_VIEW_DESC> rtv_descs = {};

    if (!!(pDesc->Flags & TARGET_FLAG_CREATE_MIP_RTV) && pDesc->MipLevels > 1)
        rtv_descs.resize(pDesc->MipLevels);
    else
        rtv_descs.resize(1);

    for(size_t i=0; i<rtv_descs.size(); ++i)
    {
        auto& rtv_desc = rtv_descs[i];
        rtv_desc.Format = pDesc->Format;

        if (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
        {
            rtv_desc.ViewDimension          = D3D12_RTV_DIMENSION_TEXTURE3D;
            rtv_desc.Texture3D.FirstWSlice  = 0;
            rtv_desc.Texture3D.MipSlice     = UINT(i);
            rtv_desc.Texture3D.WSize        = pDesc->DepthOrArraySize;
        }
        else if(pDesc->Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
        {
            if ( pDesc->DepthOrArraySize > 1 )
            {
                if ( pDesc->SampleDesc.Count > 1 )
                {
                    rtv_desc.ViewDimension                    = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
                    rtv_desc.Texture2DMSArray.ArraySize       = pDesc->DepthOrArraySize;
                    rtv_desc.Texture2DMSArray.FirstArraySlice = 0;
                }
                else
                {
                    rtv_desc.ViewDimension                   = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                    rtv_desc.Texture2DArray.ArraySize        = pDesc->DepthOrArraySize;
                    rtv_desc.Texture2DArray.FirstArraySlice  = 0;
                    rtv_desc.Texture2DArray.MipSlice         = UINT(i);
                    rtv_desc.Texture2DArray.PlaneSlice       = 0;
                }
            }
            else
            {
                if ( pDesc->SampleDesc.Count > 1 )
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
        else if (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
        {
            if (pDesc->DepthOrArraySize > 1)
            {
                rtv_desc.ViewDimension                  = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                rtv_desc.Texture1DArray.ArraySize       = pDesc->DepthOrArraySize;
                rtv_desc.Texture1DArray.FirstArraySlice = 0;
                rtv_desc.Texture1DArray.MipSlice        = UINT(i);
            }
        }
        else if (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
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

    if (!(pDesc->Flags & TARGET_FLAG_NO_SRV))
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};

        srv_desc.Format = pDesc->Format; 
        srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        if (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
        {
            srv_desc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE3D;
            srv_desc.Texture3D.MipLevels            = pDesc->MipLevels;
            srv_desc.Texture3D.MostDetailedMip      = mostDetailedMip;
            srv_desc.Texture3D.ResourceMinLODClamp  = 0;
        }
        else if(pDesc->Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
        {
            if ( pDesc->DepthOrArraySize > 1 )
            {
                if ( pDesc->SampleDesc.Count > 1 )
                {
                    srv_desc.ViewDimension                    = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
                    srv_desc.Texture2DMSArray.ArraySize       = pDesc->DepthOrArraySize;
                    srv_desc.Texture2DMSArray.FirstArraySlice = 0;
                }
                else
                {
                    srv_desc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                    srv_desc.Texture2DArray.ArraySize           = pDesc->DepthOrArraySize;
                    srv_desc.Texture2DArray.FirstArraySlice     = 0;
                    srv_desc.Texture2DArray.MipLevels           = 0;
                    srv_desc.Texture2DArray.PlaneSlice          = 0;
                    srv_desc.Texture2DArray.ResourceMinLODClamp = 0.0f;
                    srv_desc.Texture2DArray.MostDetailedMip     = mostDetailedMip;
                }
            }
            else
            {
                if ( pDesc->SampleDesc.Count > 1 )
                {
                    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    srv_desc.Texture2D.MipLevels            = pDesc->MipLevels;
                    srv_desc.Texture2D.MostDetailedMip      = mostDetailedMip;
                    srv_desc.Texture2D.PlaneSlice           = 0;
                    srv_desc.Texture2D.ResourceMinLODClamp  = 0.0f;
                    srv_desc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE2D;
                }
            }
        }
        else if (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
        {
            if (pDesc->DepthOrArraySize > 1)
            {
                srv_desc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                srv_desc.Texture1DArray.ArraySize           = pDesc->DepthOrArraySize;
                srv_desc.Texture1DArray.FirstArraySlice     = 0;
                srv_desc.Texture1DArray.MipLevels           = pDesc->MipLevels;
                srv_desc.Texture1DArray.MostDetailedMip     = mostDetailedMip;
                srv_desc.Texture1DArray.ResourceMinLODClamp = 0;
            }
        }
        else if (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
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
    for(size_t i=0; i<m_HolderRTV.size(); ++i)
    {
        m_HolderRTV[i].Reset();
    }
    m_HolderRTV.clear();
    m_HolderRTV.shrink_to_fit();

    m_HolderSRV.Reset();

    base::Term();
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

    if (base::Init(pDesc))
    {
        ELOGA("BaseTarget::Init() Failed.");
        return false;
    }

    if (!InitView(pDesc))
    {
        ELOGA("Error : InitView() Failed.");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      エイリアシングリソースとして初期化します.
//-----------------------------------------------------------------------------
bool DepthTarget::InitAsAliasing(const TargetDesc* pDesc, const BaseTarget& target)
{
    if (!base::InitAsAliasing(pDesc, target))
    {
        ELOGA("Error : BaseTarget::InitAsAliasing() Failed.");
        return false;
    }

    if (!InitView(pDesc))
    {
        ELOGA("Error : InitView() Failed.");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      ビューを初期化します.
//-----------------------------------------------------------------------------
bool DepthTarget::InitView(const TargetDesc* pDesc)
{
    #if ASDX_IS_SCARLETT
        auto mostDetailedMip = (pDesc->MipLevels - 1);
    #else
        auto mostDetailedMip = 0u;
    #endif

    std::vector<D3D12_DEPTH_STENCIL_VIEW_DESC> dsv_descs = {};
    if (!!(pDesc->Flags & TARGET_FLAG_CREATE_MIP_DSV) && pDesc->MipLevels > 1)
        dsv_descs.resize(pDesc->MipLevels);
    else
        dsv_descs.resize(1);

    for(size_t i=0; i<dsv_descs.size(); ++i)
    {
        auto& dsv_desc = dsv_descs[i];
        dsv_desc.Format = pDesc->Format;

        if (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
        {
            if (pDesc->DepthOrArraySize > 1)
            {
                if (pDesc->SampleDesc.Count > 1)
                {
                    dsv_desc.ViewDimension                      = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
                    dsv_desc.Texture2DMSArray.ArraySize         = pDesc->DepthOrArraySize;
                    dsv_desc.Texture2DMSArray.FirstArraySlice   = 0;
                }
                else
                {
                    dsv_desc.ViewDimension                  = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                    dsv_desc.Texture2DArray.ArraySize       = pDesc->DepthOrArraySize;
                    dsv_desc.Texture2DArray.FirstArraySlice = 0;
                    dsv_desc.Texture2DArray.MipSlice        = UINT(i);
                }
            }
            else
            {
                if (pDesc->SampleDesc.Count > 1)
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
        else if (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
        {
            if (pDesc->DepthOrArraySize > 1)
            {
                dsv_desc.ViewDimension                  = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
                dsv_desc.Texture1DArray.ArraySize       = pDesc->DepthOrArraySize;
                dsv_desc.Texture1DArray.FirstArraySlice = 0;
                dsv_desc.Texture1DArray.MipSlice        = UINT(i);
            }
            else
            {
                dsv_desc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE1D;
                dsv_desc.Texture1D.MipSlice = UINT(i);
            }
        }
        else if (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
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
            ELOGA("Error : DescriptorHeap::Alloc() Failed.");
            return false;
        }
        DescriptorHolder holder(DescriptorHolder::HEAP_DSV, handleDSV);
        m_HolderDSV[i].Swap(holder);
        pDevice->CreateDepthStencilView(m_pResource.GetPtr(), &dsv_descs[i], GetCpuHandleDSV());
    }

    if (!(pDesc->Flags & TARGET_FLAG_NO_SRV))
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
        srv_desc.Format = GetResourceFormat(pDesc->Format, false);
        srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        if (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
        {
            if (pDesc->DepthOrArraySize > 1)
            {
                if (pDesc->SampleDesc.Count > 1)
                {
                    srv_desc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
                    srv_desc.Texture2DMSArray.ArraySize         = pDesc->DepthOrArraySize;
                    srv_desc.Texture2DMSArray.FirstArraySlice   = 0;
                }
                else
                {
                    srv_desc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                    srv_desc.Texture2DArray.ArraySize           = pDesc->DepthOrArraySize;
                    srv_desc.Texture2DArray.FirstArraySlice     = 0;
                    srv_desc.Texture2DArray.MipLevels           = pDesc->MipLevels;
                    srv_desc.Texture2DArray.MostDetailedMip     = mostDetailedMip;
                    srv_desc.Texture2DArray.PlaneSlice          = 0;
                    srv_desc.Texture2DArray.ResourceMinLODClamp = 0;
                }
            }
            else
            {
                if (pDesc->SampleDesc.Count > 1)
                {
                    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    srv_desc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE2D;
                    srv_desc.Texture2D.MipLevels            = pDesc->MipLevels;
                    srv_desc.Texture2D.MostDetailedMip      = mostDetailedMip;
                    srv_desc.Texture2D.PlaneSlice           = 0;
                    srv_desc.Texture2D.ResourceMinLODClamp  = 0;
                }
            }

        }
        else if (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
        {
            if (pDesc->DepthOrArraySize > 1)
            {
                srv_desc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                srv_desc.Texture1DArray.ArraySize           = pDesc->DepthOrArraySize;
                srv_desc.Texture1DArray.FirstArraySlice     = 0;
                srv_desc.Texture1DArray.MipLevels           = pDesc->MipLevels;
                srv_desc.Texture1DArray.MostDetailedMip     = mostDetailedMip;
                srv_desc.Texture1DArray.ResourceMinLODClamp = 0;
            }
            else
            {
                srv_desc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE1D;
                srv_desc.Texture1D.MipLevels            = pDesc->MipLevels;
                srv_desc.Texture1D.MostDetailedMip      = mostDetailedMip;
                srv_desc.Texture1D.ResourceMinLODClamp  = 0;
            }
        }
        else if (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
        {
            // Not Support Buffer.
            assert(false);
            ELOGA("Error : Not Support Buffer Type.");
            return false;
        }

        auto handleSRV = GetResourceDescriptorHeap()->Alloc(1);
        if (!handleSRV.IsValid())
        {
            ELOGA("Error : DescriptorHeap::Alloc() Failed.");
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
    for(size_t i=0; i<m_HolderDSV.size(); ++i)
    {
        m_HolderDSV[i].Reset();
    }
    m_HolderDSV.clear();
    m_HolderDSV.shrink_to_fit();

    m_HolderSRV.Reset();

    base::Term();
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

    auto desc = (*pDesc);
    desc.Flags |= TARGET_FLAG_ALLOW_UAV;
    if (base::Init(&desc))
    {
        ELOGA("Error : BaseTarget::Init() Failed.");
        return false;
    }

    if (!InitView(pDesc, stride))
    {
        ELOGA("Error : InitView() Failed.");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool ComputeTarget::Init(ColorTarget& target, uint32_t flags)
{
    m_pResource = target.GetResource();

    auto desc = target.GetDesc();

    std::vector<D3D12_UNORDERED_ACCESS_VIEW_DESC> uav_descs = {};
    if (!!(flags & TARGET_FLAG_CREATE_MIP_UAV) && desc.MipLevels > 1)
        uav_descs.resize(desc.MipLevels);
    else
        uav_descs.resize(1);

    for(size_t i=0; i<uav_descs.size(); ++i)
    {
        auto uav_desc = uav_descs[i];
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
            // Not Support Buffer.
            assert(false);
            ELOGA("Error : Not Support Buffer Type.");
            return false;
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

    if (!(flags & TARGET_FLAG_NO_SRV))
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
        srv_desc.Format = desc.Format;

        if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
        {
            srv_desc.ViewDimension                  = D3D12_SRV_DIMENSION_BUFFER;
            srv_desc.Texture3D.MostDetailedMip      = 0;
            srv_desc.Texture3D.MipLevels            = desc.MipLevels;
            srv_desc.Texture3D.ResourceMinLODClamp  = 0.0f;
        }
        else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
        {
            if (desc.DepthOrArraySize > 1)
            {
                srv_desc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                srv_desc.Texture2DArray.MostDetailedMip     = 0;
                srv_desc.Texture2DArray.MipLevels           = desc.MipLevels;
                srv_desc.Texture2DArray.FirstArraySlice     = 0;
                srv_desc.Texture2DArray.ArraySize           = desc.DepthOrArraySize;
                srv_desc.Texture2DArray.PlaneSlice          = 0;
                srv_desc.Texture2DArray.ResourceMinLODClamp = 0.0f;
            }
            else
            {
                srv_desc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE2D;
                srv_desc.Texture2D.MostDetailedMip      = 0;
                srv_desc.Texture2D.MipLevels            = desc.MipLevels;
                srv_desc.Texture2D.PlaneSlice           = 0;
                srv_desc.Texture2D.ResourceMinLODClamp  = 0.0f;
            }
        }
        else if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
        {
            if (desc.DepthOrArraySize > 1)
            {
                srv_desc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                srv_desc.Texture1DArray.MostDetailedMip     = 0;
                srv_desc.Texture1DArray.MipLevels           = desc.MipLevels;
                srv_desc.Texture1DArray.FirstArraySlice     = 0;
                srv_desc.Texture1DArray.ArraySize           = desc.DepthOrArraySize;
                srv_desc.Texture1DArray.ResourceMinLODClamp = 0.0f;
            }
            else
            {
                srv_desc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE1D;
                srv_desc.Texture1D.MostDetailedMip      = 0;
                srv_desc.Texture1D.MipLevels            = desc.MipLevels;
                srv_desc.Texture1D.ResourceMinLODClamp  = 0.0f;
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
//      エイリアシングリソースとして初期化します.
//-----------------------------------------------------------------------------
bool ComputeTarget::InitAsAliasing(const TargetDesc* pDesc, uint32_t stride, const BaseTarget& target)
{
    if (!base::InitAsAliasing(pDesc, target))
    {
        ELOGA("Error : BaseTarget::InitAsAliasing() Failed.");
        return false;
    }

    if (!InitView(pDesc, stride))
    {
        ELOGA("Error : InitView() Failed.");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      ビューを初期化します.
//-----------------------------------------------------------------------------
bool ComputeTarget::InitView(const TargetDesc* pDesc, uint32_t stride)
{
#if ASDX_IS_SCARLETT
    auto mostDetailedMip = (pDesc->MipLevels - 1);
#else
    auto mostDetailedMip = 0u;
#endif

    std::vector<D3D12_UNORDERED_ACCESS_VIEW_DESC> uav_descs = {};
    if (!!(pDesc->Flags & TARGET_FLAG_CREATE_MIP_UAV) && pDesc->MipLevels > 1)
        uav_descs.resize(pDesc->MipLevels);
    else
        uav_descs.resize(1);

    for(size_t i=0; i<uav_descs.size(); ++i)
    {
        auto& uav_desc = uav_descs[i];
        uav_desc.Format = pDesc->Format;

        if (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
        {
            uav_desc.ViewDimension          = D3D12_UAV_DIMENSION_TEXTURE3D;
            uav_desc.Texture3D.FirstWSlice  = 0;
            uav_desc.Texture3D.MipSlice     = UINT(i);
            uav_desc.Texture3D.WSize        = pDesc->DepthOrArraySize;
        }
        else if (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
        {
            if (pDesc->DepthOrArraySize > 1)
            {
                uav_desc.ViewDimension                      = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                uav_desc.Texture2DArray.ArraySize           = pDesc->DepthOrArraySize;
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
        else if (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
        {
            if (pDesc->DepthOrArraySize > 1)
            {
                uav_desc.ViewDimension                  = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
                uav_desc.Texture1DArray.ArraySize       = pDesc->DepthOrArraySize;
                uav_desc.Texture1DArray.FirstArraySlice = 0;
                uav_desc.Texture1DArray.MipSlice        = UINT(i);
            }
            else
            {
                uav_desc.ViewDimension      = D3D12_UAV_DIMENSION_TEXTURE1D;
                uav_desc.Texture1D.MipSlice = UINT(i);
            }
        }
        else if (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
        {
            auto elements = (stride == 0) ? pDesc->Width : pDesc->Width / stride;

            uav_desc.ViewDimension                  = D3D12_UAV_DIMENSION_BUFFER;
            uav_desc.Buffer.CounterOffsetInBytes    = 0;
            uav_desc.Buffer.FirstElement            = 0;
            uav_desc.Buffer.NumElements             = UINT(elements);
            uav_desc.Buffer.StructureByteStride     = stride;
            uav_desc.Buffer.Flags                   = (stride == 0) ? D3D12_BUFFER_UAV_FLAG_RAW : D3D12_BUFFER_UAV_FLAG_NONE;
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

    if (!(pDesc->Flags & TARGET_FLAG_NO_SRV))
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
        srv_desc.Format = pDesc->Format; 
        srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        if (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
        {
            srv_desc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE3D;
            srv_desc.Texture3D.MipLevels            = pDesc->MipLevels;
            srv_desc.Texture3D.MostDetailedMip      = mostDetailedMip;
            srv_desc.Texture3D.ResourceMinLODClamp  = 0;
        }
        else if (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
        {
            if (pDesc->DepthOrArraySize > 1)
            {
                srv_desc.Texture2DArray.ArraySize           = pDesc->DepthOrArraySize;
                srv_desc.Texture2DArray.FirstArraySlice     = 0;
                srv_desc.Texture2DArray.MipLevels           = 0;
                srv_desc.Texture2DArray.PlaneSlice          = 0;
                srv_desc.Texture2DArray.ResourceMinLODClamp = 0.0f;
                srv_desc.Texture2DArray.MostDetailedMip     = mostDetailedMip;
                srv_desc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            }

            else
            {
                srv_desc.Texture2D.MipLevels            = pDesc->MipLevels;
                srv_desc.Texture2D.MostDetailedMip      = mostDetailedMip;
                srv_desc.Texture2D.PlaneSlice           = 0;
                srv_desc.Texture2D.ResourceMinLODClamp  = 0.0f;
                srv_desc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE2D;

            }
        }
        else if (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
        {
            if (pDesc->DepthOrArraySize > 1)
            {
                srv_desc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                srv_desc.Texture1DArray.ArraySize           = pDesc->DepthOrArraySize;
                srv_desc.Texture1DArray.FirstArraySlice     = 0;
                srv_desc.Texture1DArray.MipLevels           = pDesc->MipLevels;
                srv_desc.Texture1DArray.MostDetailedMip     = mostDetailedMip;
                srv_desc.Texture1DArray.ResourceMinLODClamp = 0;
            }
            else
            {
                srv_desc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE1D;
                srv_desc.Texture1D.MipLevels            = pDesc->MipLevels;
                srv_desc.Texture1D.MostDetailedMip      = mostDetailedMip;
                srv_desc.Texture1D.ResourceMinLODClamp  = 0;
            }
        }
        else if (pDesc->Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
        {
            auto elements = (stride == 0) ? pDesc->Width : pDesc->Width / stride;
            srv_desc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
            srv_desc.Buffer.FirstElement        = 0;
            srv_desc.Buffer.NumElements         = UINT(elements);
            srv_desc.Buffer.StructureByteStride = stride;
            srv_desc.Buffer.Flags               = (stride == 0) ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;
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

    m_Stride = stride;

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void ComputeTarget::Term()
{
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

    m_Stride = 0;

    base::Term();
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

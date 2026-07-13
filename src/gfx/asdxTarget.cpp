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

    HRESULT hr = S_OK;

    {
        auto format = GetNoSRGBFormat(pDesc->Format);

        D3D12_HEAP_PROPERTIES props = {
            D3D12_HEAP_TYPE_DEFAULT,
            D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            D3D12_MEMORY_POOL_UNKNOWN,
            1,
            1
        };

        D3D12_RESOURCE_DESC desc = {
            pDesc->Dimension,
            pDesc->Alignment,
            pDesc->Width,
            pDesc->Height,
            pDesc->DepthOrArraySize,
            pDesc->MipLevels,
            format,
            pDesc->SampleDesc,
            D3D12_TEXTURE_LAYOUT_UNKNOWN,
            D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
        };

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format   = format,
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
    }

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

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    m_PrevState = m_Desc.InitState;

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

#if ASDX_IS_SCARLETT
    auto mostDetailedMip = (pDesc->MipLevels - 1);
#else
    auto mostDetailedMip = 0u;
#endif

    D3D12_RENDER_TARGET_VIEW_DESC   rtv_desc = {};
    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    rtv_desc.Format = desc.Format;
    srv_desc.Format = desc.Format;
    srv_desc.Shader4ComponentMapping =D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    if ( desc.DepthOrArraySize > 1 )
    {
        if ( desc.SampleDesc.Count >= 1 )
        {
            rtv_desc.Texture2DMSArray.ArraySize       = desc.DepthOrArraySize;
            rtv_desc.Texture2DMSArray.FirstArraySlice = 0;
            rtv_desc.ViewDimension                    = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;

            srv_desc.Texture2DMSArray.ArraySize       = desc.DepthOrArraySize;
            srv_desc.Texture2DMSArray.FirstArraySlice = 0;
            srv_desc.ViewDimension                    = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
        }
        else
        {
            rtv_desc.Texture2DArray.ArraySize        = desc.DepthOrArraySize;
            rtv_desc.Texture2DArray.FirstArraySlice  = 0;
            rtv_desc.Texture2DArray.MipSlice         = 0;
            rtv_desc.Texture2DArray.PlaneSlice       = 0;
            rtv_desc.ViewDimension                   = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;

            srv_desc.Texture2DArray.ArraySize           = desc.DepthOrArraySize;
            srv_desc.Texture2DArray.FirstArraySlice     = 0;
            srv_desc.Texture2DArray.MipLevels           = desc.MipLevels;
            srv_desc.Texture2DArray.MostDetailedMip     = mostDetailedMip;
            srv_desc.Texture2DArray.PlaneSlice          = 0;
            srv_desc.Texture2DArray.ResourceMinLODClamp = 0.0f;
        }
    }
    else
    {
        if ( desc.SampleDesc.Count <= 1 )
        {
            rtv_desc.Texture2D.MipSlice     = 0;
            rtv_desc.Texture2D.PlaneSlice   = 0;
            rtv_desc.ViewDimension          = D3D12_RTV_DIMENSION_TEXTURE2D;
                
            srv_desc.Texture2D.MipLevels            = desc.MipLevels;
            srv_desc.Texture2D.MostDetailedMip      = mostDetailedMip;
            srv_desc.Texture2D.PlaneSlice           = 0;
            srv_desc.Texture2D.ResourceMinLODClamp  = 0.0f;
            srv_desc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE2D;
        }
        else
        {
            rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;

            srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
        }
    }

    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    auto handleRTV = GetRtvDescriptorHeap()->Alloc(1);
    if (!handleRTV.IsValid())
    {
        ELOG("Error : DescriptorHeap::Alloc() Failed.");
        return false;
    }
    m_HolderRTV.resize(1);
    {
        DescriptorHolder holder(DescriptorHolder::HEAP_RTV, handleRTV);
        m_HolderRTV[0].Swap(holder);
    }
    pDevice->CreateRenderTargetView(m_pResource.GetPtr(), &rtv_desc, GetCpuHandleRTV());

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

    m_PrevState = D3D12_RESOURCE_STATE_COMMON;

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
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* ColorTarget::GetResource() const
{ return m_pResource.GetPtr(); }

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
//      構成設定を取得します.
//-----------------------------------------------------------------------------
TargetDesc ColorTarget::GetDesc() const
{ return m_Desc; }

//-----------------------------------------------------------------------------
//      sRGBフラグを取得します.
//-----------------------------------------------------------------------------
bool ColorTarget::IsSRGB() const
{ return IsSRGBFormat(m_Desc.Format); }

//-----------------------------------------------------------------------------
//      デバッグ名を設定します.
//-----------------------------------------------------------------------------
void ColorTarget::SetName(LPCWSTR tag)
{
    if (m_pResource)
    { m_pResource->SetName(tag); }
}


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

    HRESULT hr = S_OK;

    auto format = GetResourceFormat(pDesc->Format, false);
    {
        D3D12_HEAP_PROPERTIES props = {
            D3D12_HEAP_TYPE_DEFAULT,
            D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            D3D12_MEMORY_POOL_UNKNOWN,
            1,
            1
        };

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
            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
        };

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format               = pDesc->Format;
        clearValue.DepthStencil.Depth   = pDesc->ClearDepth;
        clearValue.DepthStencil.Stencil = pDesc->ClearStencil;

        auto allocator = GetD3D12MA();
        if (allocator != nullptr)
        {
            D3D12MA::ALLOCATION_DESC allocDesc = {};
            allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

            D3D12MA::Allocation* pAllocation = nullptr;

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
    }

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
            ELOG("Error : DescriptorHeap::Alloc() Failed.");
            return false;
        }
        DescriptorHolder holder(DescriptorHolder::HEAP_DSV, handleDSV);
        m_HolderDSV[i].Swap(holder);
        pDevice->CreateDepthStencilView(m_pResource.GetPtr(), &dsv_descs[i], GetCpuHandleDSV());
    }

    if (!(pDesc->Flags & TARGET_FLAG_NO_SRV))
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
        srv_desc.Format = format;
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
            ELOG("Error : DescriptorHeap::Alloc() Failed.");
            return false;
        }
        {
            DescriptorHolder holder(DescriptorHolder::HEAP_RES, handleSRV);
            m_HolderSRV.Swap(holder);
        }
        pDevice->CreateShaderResourceView(m_pResource.GetPtr(), &srv_desc, GetCpuHandleSRV());
    }

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    m_PrevState = m_Desc.InitState;

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
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* DepthTarget::GetResource() const
{ return m_pResource.GetPtr(); }

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

//-----------------------------------------------------------------------------
//      構成設定を取得します
//-----------------------------------------------------------------------------
TargetDesc DepthTarget::GetDesc() const
{ return m_Desc; }

//-----------------------------------------------------------------------------
//      デバッグ名を設定します.
//-----------------------------------------------------------------------------
void DepthTarget::SetName(LPCWSTR tag)
{
    if (m_pResource)
    { m_pResource->SetName(tag); }
}


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

    HRESULT hr = S_OK;

    {
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
            pDesc->Format,
            pDesc->SampleDesc,
            layout,
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
        };

        auto allocator = GetD3D12MA();
        if (allocator != nullptr)
        {
            D3D12MA::ALLOCATION_DESC allocDesc = {};
            allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

            D3D12MA::Allocation* pAllocation = nullptr;

            hr = allocator->CreateResource(
                &allocDesc,
                &desc,
                pDesc->InitState,
                nullptr,
                &pAllocation,
                IID_PPV_ARGS(m_pResource.GetAddress()));
            if (FAILED(hr))
            {
                ELOG("Error : D3D12MA::Allocator::CreateResource() Failed. errcode = 0x%x", hr);
                return false;
            }

            m_HolderAlloc = AllocationHolder(pAllocation);
        }
        else
        {
            hr = GetD3D12Device()->CreateCommittedResource( 
                &props,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                pDesc->InitState,
                nullptr,
                IID_PPV_ARGS(m_pResource.GetAddress()));
            if (FAILED(hr))
            {
                ELOG( "Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr );
                return false;
            }
        }
    }

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
    for(size_t i=0; i<m_HolderUAV.size(); ++i)
    {
        auto handleUAV = GetResourceDescriptorHeap()->Alloc(1);
        if (!handleUAV.IsValid())
        {
            ELOG("Error : DescriptorHeap::Alloc() Failed.");
            return false;
        }
        DescriptorHolder holder(DescriptorHolder::HEAP_RES, handleUAV);
        m_HolderUAV[i].Swap(holder);
        pDevice->CreateUnorderedAccessView(m_pResource.GetPtr(), nullptr, &uav_descs[i], GetCpuHandleUAV());
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

    memcpy(&m_Desc, pDesc, sizeof(m_Desc));

    m_PrevState = m_Desc.InitState;
    m_Stride    = stride;

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
    for(size_t i=0; i<m_HolderUAV.size(); ++i)
    {
        auto handleUAV = GetResourceDescriptorHeap()->Alloc(1);
        if (!handleUAV.IsValid())
        {
            ELOG("Error : DescriptorHeap::Alloc() Failed.");
            return false;
        }
        DescriptorHolder holder(DescriptorHolder::HEAP_RES, handleUAV);
        m_HolderUAV[i].Swap(holder);
        pDevice->CreateUnorderedAccessView(m_pResource.GetPtr(), nullptr, &uav_descs[i], GetCpuHandleUAV());
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

    memcpy(&m_Desc, &desc, sizeof(m_Desc));

    m_PrevState = m_Desc.InitState;

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
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* ComputeTarget::GetResource() const
{ return m_pResource.GetPtr(); }

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
//      構成設定を取得します.
//-----------------------------------------------------------------------------
TargetDesc ComputeTarget::GetDesc() const
{ return m_Desc; }

//-----------------------------------------------------------------------------
//      ストライドサイズを取得します.
//-----------------------------------------------------------------------------
uint32_t ComputeTarget::GetStride() const
{ return m_Stride; }

//-----------------------------------------------------------------------------
//      デバッグ名を設定します.
//-----------------------------------------------------------------------------
void ComputeTarget::SetName(LPCWSTR tag)
{
    if (m_pResource)
    { m_pResource->SetName(tag); }
}

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

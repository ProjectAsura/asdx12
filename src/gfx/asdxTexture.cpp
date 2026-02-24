//-----------------------------------------------------------------------------
// File : asdxTexture.cpp
// Desc : Texture.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <gfx/asdxTexture.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxUpdateCommand.h>
#include <gfx/asdxDescriptorHeap.h>
#include <res/asdxResTexture.h>
#include <fnd/asdxLogger.h>
#include <D3D12MemAlloc.h>


#if (D3D12_SDK_VERSION >= 613 || D3D12_PREVIEW_SDK_VERSION >= 710)
#define ASDX_ENABLE_GPU_UPLOAD_HEAPS    (1)
#else
#define ASDX_ENABLE_GPU_UPLOAD_HEAPS    (0)
#endif

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Texture class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Texture::Texture()
: m_RefCount(1)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Texture::~Texture()
{ Term(); }

//-----------------------------------------------------------------------------
//      参照カウントを増やします.
//-----------------------------------------------------------------------------
void Texture::AddRef()
{ m_RefCount++; }

//-----------------------------------------------------------------------------
//      参照カウントを減らします.
//-----------------------------------------------------------------------------
void Texture::Release()
{
    m_RefCount--;
    if (m_RefCount == 0)
    { delete this; }
}

//-----------------------------------------------------------------------------
//      参照カウントを取得します.
//-----------------------------------------------------------------------------
uint32_t Texture::GetRefCount() const
{ return m_RefCount; }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Texture::Init(ID3D12GraphicsCommandList* pCmdList, const ResTexture& resource)
{
    auto pDevice = GetD3D12Device();

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

    auto heapType  = D3D12_HEAP_TYPE_DEFAULT;
    auto initState = D3D12_RESOURCE_STATE_COPY_DEST;

    D3D12_HEAP_PROPERTIES props = {
        heapType,
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
            if (resource.DepthOrArraySize > 1)
            {
                viewDesc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                viewDesc.Format                             = format;
                viewDesc.Texture1DArray.ArraySize           = resource.DepthOrArraySize;
                viewDesc.Texture1DArray.FirstArraySlice     = 0;
                viewDesc.Texture1DArray.MipLevels           = resource.MipLevels;
                viewDesc.Texture1DArray.MostDetailedMip     = mostDetailedMip;
                viewDesc.Texture1DArray.ResourceMinLODClamp = 0;
            }
            else
            {
                viewDesc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE1D;
                viewDesc.Format                         = format;
                viewDesc.Texture1D.MipLevels            = resource.MipLevels;
                viewDesc.Texture1D.MostDetailedMip      = mostDetailedMip;
                viewDesc.Texture1D.ResourceMinLODClamp  = 0;
            }
        }
        break;

    case TEXTURE_DIMENSION_2D:
        {
            dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            if (resource.DepthOrArraySize> 1)
            {
                viewDesc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                viewDesc.Format                             = format;
                viewDesc.Texture2DArray.ArraySize           = resource.DepthOrArraySize;
                viewDesc.Texture2DArray.FirstArraySlice     = 0;
                viewDesc.Texture2DArray.MipLevels           = resource.MipLevels;
                viewDesc.Texture2DArray.MostDetailedMip     = mostDetailedMip;
                viewDesc.Texture2DArray.PlaneSlice          = 0;
                viewDesc.Texture2DArray.ResourceMinLODClamp = 0;
            }
            else
            {
                viewDesc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2D;
                viewDesc.Format                             = format;
                viewDesc.Texture2D.MipLevels                = resource.MipLevels;
                viewDesc.Texture2D.MostDetailedMip          = mostDetailedMip;
                viewDesc.Texture2D.PlaneSlice               = 0;
                viewDesc.Texture2D.ResourceMinLODClamp      = 0;
            }
        }
        break;

    case TEXTURE_DIMENSION_3D:
        {
            dimension   = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
            depth       = resource.DepthOrArraySize;

            viewDesc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE3D;
            viewDesc.Format                         = format;
            viewDesc.Texture3D.MipLevels            = resource.MipLevels;
            viewDesc.Texture3D.MostDetailedMip      = 0;
            viewDesc.Texture3D.ResourceMinLODClamp  = 0.0f;
        }
        break;

    case TEXTURE_DIMENSION_CUBE:
        {
            dimension   = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            depth       = resource.DepthOrArraySize;

            if (resource.DepthOrArraySize > 1)
            {
                viewDesc.ViewDimension                           = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                viewDesc.Format                                  = format;
                viewDesc.TextureCubeArray.First2DArrayFace       = 0;
                viewDesc.TextureCubeArray.MipLevels              = resource.MipLevels;
                viewDesc.TextureCubeArray.MostDetailedMip        = 0;
                viewDesc.TextureCubeArray.NumCubes               = resource.DepthOrArraySize;
                viewDesc.TextureCubeArray.ResourceMinLODClamp    = 0.0f;
            }
            else
            {
                viewDesc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURECUBE;
                viewDesc.Format                             = format;
                viewDesc.TextureCube.MipLevels              = resource.MipLevels;
                viewDesc.TextureCube.MostDetailedMip        = 0;
                viewDesc.TextureCube.ResourceMinLODClamp    = 0;
            }
        }
        break;
    }

    D3D12_RESOURCE_DESC desc = {
        dimension,
        0,
        resource.Width,
        resource.Height,
        resource.DepthOrArraySize,
        resource.MipLevels,
        format,
        { 1, 0 },
        D3D12_TEXTURE_LAYOUT_UNKNOWN,
        D3D12_RESOURCE_FLAG_NONE
    };

    auto allocator = GetD3D12MA();
    if (allocator != nullptr)
    {
        D3D12MA::ALLOCATION_DESC allocDesc = {};
        allocDesc.HeapType = heapType;

        D3D12MA::Allocation* pAllocation = nullptr;

        auto hr = allocator->CreateResource(
            &allocDesc,
            &desc,
            initState,
            nullptr,
            &pAllocation,
            IID_PPV_ARGS(m_Resource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : D3D12MA::Allocator::CreateResource() Failed. errcode = 0x%x", hr);
            return false;
        }

        m_Allocation = AllocationHolder(pAllocation);
    }
    else
    {
        auto hr = pDevice->CreateCommittedResource(
            &props,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            initState,
            nullptr,
            IID_PPV_ARGS(m_Resource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateCommitedResource() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    auto handleSRV = GetResourceDescriptorHeap()->Alloc(1);
    if (!handleSRV.IsValid())
    {
        ELOG("Error : DescriptorHeap::Alloc() Failed.");
        return false;
    }

    m_DescriptorSRV = DescriptorHolder(DescriptorHolder::HEAP_RES, handleSRV);
    pDevice->CreateShaderResourceView(m_Resource.GetPtr(), &viewDesc, GetHandleCPU());

    // コピーコマンドを使ってアップロード.
    UpdateTexture(pCmdList, m_Resource.GetPtr(), &resource);

    // ステート遷移.
    if (initState != D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE)
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags                   = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource    = m_Resource.GetPtr();
        barrier.Transition.Subresource  = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore  = initState;
        barrier.Transition.StateAfter   = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;

        pCmdList->ResourceBarrier(1, &barrier);
    }

    return true;
}

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Texture::Init(const ResTexture& resource)
{
    auto pDevice = GetD3D12Device();

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

    auto heapType  = D3D12_HEAP_TYPE_DEFAULT;
    auto initState = D3D12_RESOURCE_STATE_COPY_DEST;

    if (IsUMA())
    {
        initState  = D3D12_RESOURCE_STATE_COMMON;
    }

    if (IsSupportGpuUploadHeap())
    {
        heapType   = D3D12_HEAP_TYPE_GPU_UPLOAD;
        initState  = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
    }

    D3D12_HEAP_PROPERTIES props = {
        heapType,
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
            if (resource.DepthOrArraySize > 1)
            {
                viewDesc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                viewDesc.Format                             = format;
                viewDesc.Texture1DArray.ArraySize           = resource.DepthOrArraySize;
                viewDesc.Texture1DArray.FirstArraySlice     = 0;
                viewDesc.Texture1DArray.MipLevels           = resource.MipLevels;
                viewDesc.Texture1DArray.MostDetailedMip     = mostDetailedMip;
                viewDesc.Texture1DArray.ResourceMinLODClamp = 0;
            }
            else
            {
                viewDesc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE1D;
                viewDesc.Format                         = format;
                viewDesc.Texture1D.MipLevels            = resource.MipLevels;
                viewDesc.Texture1D.MostDetailedMip      = mostDetailedMip;
                viewDesc.Texture1D.ResourceMinLODClamp  = 0;
            }
        }
        break;

    case TEXTURE_DIMENSION_2D:
        {
            dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            if (resource.DepthOrArraySize> 1)
            {
                viewDesc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                viewDesc.Format                             = format;
                viewDesc.Texture2DArray.ArraySize           = resource.DepthOrArraySize;
                viewDesc.Texture2DArray.FirstArraySlice     = 0;
                viewDesc.Texture2DArray.MipLevels           = resource.MipLevels;
                viewDesc.Texture2DArray.MostDetailedMip     = mostDetailedMip;
                viewDesc.Texture2DArray.PlaneSlice          = 0;
                viewDesc.Texture2DArray.ResourceMinLODClamp = 0;
            }
            else
            {
                viewDesc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2D;
                viewDesc.Format                             = format;
                viewDesc.Texture2D.MipLevels                = resource.MipLevels;
                viewDesc.Texture2D.MostDetailedMip          = mostDetailedMip;
                viewDesc.Texture2D.PlaneSlice               = 0;
                viewDesc.Texture2D.ResourceMinLODClamp      = 0;
            }
        }
        break;

    case TEXTURE_DIMENSION_3D:
        {
            dimension   = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
            depth       = resource.DepthOrArraySize;

            viewDesc.ViewDimension                  = D3D12_SRV_DIMENSION_TEXTURE3D;
            viewDesc.Format                         = format;
            viewDesc.Texture3D.MipLevels            = resource.MipLevels;
            viewDesc.Texture3D.MostDetailedMip      = 0;
            viewDesc.Texture3D.ResourceMinLODClamp  = 0.0f;
        }
        break;

    case TEXTURE_DIMENSION_CUBE:
        {
            dimension   = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            depth       = resource.DepthOrArraySize;

            if (resource.DepthOrArraySize > 1)
            {
                viewDesc.ViewDimension                           = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                viewDesc.Format                                  = format;
                viewDesc.TextureCubeArray.First2DArrayFace       = 0;
                viewDesc.TextureCubeArray.MipLevels              = resource.MipLevels;
                viewDesc.TextureCubeArray.MostDetailedMip        = 0;
                viewDesc.TextureCubeArray.NumCubes               = resource.DepthOrArraySize;
                viewDesc.TextureCubeArray.ResourceMinLODClamp    = 0.0f;
            }
            else
            {
                viewDesc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURECUBE;
                viewDesc.Format                             = format;
                viewDesc.TextureCube.MipLevels              = resource.MipLevels;
                viewDesc.TextureCube.MostDetailedMip        = 0;
                viewDesc.TextureCube.ResourceMinLODClamp    = 0;
            }
        }
        break;
    }

    D3D12_RESOURCE_DESC desc = {
        dimension,
        0,
        resource.Width,
        resource.Height,
        resource.DepthOrArraySize,
        resource.MipLevels,
        format,
        { 1, 0 },
        D3D12_TEXTURE_LAYOUT_UNKNOWN,
        D3D12_RESOURCE_FLAG_NONE
    };

    auto allocator = GetD3D12MA();
    if (allocator != nullptr)
    {
        D3D12MA::ALLOCATION_DESC allocDesc = {};
        allocDesc.HeapType = heapType;

        D3D12MA::Allocation* pAllocation = nullptr;

        auto hr = allocator->CreateResource(
            &allocDesc,
            &desc,
            initState,
            nullptr,
            &pAllocation,
            IID_PPV_ARGS(m_Resource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : D3D12MA::Allocator::CreateResource() Failed. errcode = 0x%x", hr);
            return false;
        }

        m_Allocation = AllocationHolder(pAllocation);
    }
    else
    {
        auto hr = pDevice->CreateCommittedResource(
            &props,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            initState,
            nullptr,
            IID_PPV_ARGS(m_Resource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateCommitedResource() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    auto handleSRV = GetResourceDescriptorHeap()->Alloc(1);
    if (!handleSRV.IsValid())
    {
        ELOG("Error : DescriptorHeap::Alloc() Failed.");
        return false;
    }

    m_DescriptorSRV = DescriptorHolder(DescriptorHolder::HEAP_RES, handleSRV);
    pDevice->CreateShaderResourceView(m_Resource.GetPtr(), &viewDesc, GetHandleCPU());

    // 直接書き込む.
    {
        auto count = resource.SubResourceCount;
        for(auto i=0u; i<count; ++i)
        {
            const auto& inRes = resource.SubResources[i];
            auto srcPtr        = inRes.pPixels;
            auto srcRowPitch   = inRes.RowPitch;
            auto srcDepthPitch = inRes.SlicePitch;

            D3D12_BOX dstBox = {};
            dstBox.left     = 0;
            dstBox.right    = inRes.Width;
            dstBox.top      = 0;
            dstBox.bottom   = inRes.Height;
            dstBox.front    = 0;
            dstBox.back     = resource.DepthOrArraySize;

            m_Resource->WriteToSubresource(i, &dstBox, srcPtr, UINT(srcRowPitch), UINT(srcDepthPitch));
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Texture::Term()
{
    m_DescriptorSRV.Reset();
    m_Allocation   .Reset();

    auto resource = m_Resource.Detach();
    Dispose(resource);
}

//-----------------------------------------------------------------------------
//      リソース設定を取得します.
//-----------------------------------------------------------------------------
D3D12_RESOURCE_DESC Texture::GetDesc() const
{
    if (!m_Resource)
        return {};
    return m_Resource->GetDesc();
}

//-----------------------------------------------------------------------------
//      バインドレスインデックスを取得します.
//-----------------------------------------------------------------------------
uint32_t Texture::GetBindlessIndex() const
{ return m_DescriptorSRV.GetIndex(); }

//-----------------------------------------------------------------------------
//      CPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetHandleCPU() const
{ return m_DescriptorSRV.GetHandleCPU(); }

//-----------------------------------------------------------------------------
//      GPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE Texture::GetHandleGPU() const
{ return m_DescriptorSRV.GetHandleGPU(); }

//-----------------------------------------------------------------------------
//      デバッグ名を設定します.
//-----------------------------------------------------------------------------
void Texture::SetName(LPCWSTR tag)
{
    if (m_Resource.GetPtr() != nullptr)
    { m_Resource->SetName(tag); }
}

//-----------------------------------------------------------------------------
//      テクスチャを生成します.
//-----------------------------------------------------------------------------
bool Texture::Create(ID3D12GraphicsCommandList* pCmdList, const ResTexture& resource, Texture** ppTexture)
{
    auto instance = new (std::nothrow) Texture();
    if (instance == nullptr)
    {
        ELOG("Error : Out of Memory.");
        return false;
    }

    if (!instance->Init(pCmdList, resource))
    {
        instance->Release();
        return false;
    }

    (*ppTexture) = instance;
    return true;
}

//-----------------------------------------------------------------------------
//      テクスチャを生成します.
//-----------------------------------------------------------------------------
bool Texture::Create(const ResTexture& resource, Texture** ppTexture)
{
    if (!IsUMA() && !IsSupportGpuUploadHeap())
    {
        ELOG("Error : Not Support. UMA = %d, GpuUploadHeap = %d", IsUMA(), IsSupportGpuUploadHeap());
        return false;
    }

    auto instance = new (std::nothrow) Texture();
    if (instance == nullptr)
    {
        ELOG("Error : Out of Memory.");
        return false;
    }

    if (!instance->Init(resource))
    {
        instance->Release();
        return false;
    }

    (*ppTexture) = instance;
    return true;
}

} // namespace asdx

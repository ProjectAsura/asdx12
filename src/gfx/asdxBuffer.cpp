//-----------------------------------------------------------------------------
// File : asdxBuffer.cpp
// Desc : Buffer Wrapper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <gfx/asdxBuffer.h>
#include <gfx/asdxDevice.h>
#include <gfx/asdxDescriptorHeap.h>
#include <gfx/asdxUpdateCommand.h>
#include <fnd/asdxLogger.h>


namespace {

//-----------------------------------------------------------------------------
//      GPUアップロードがサポートされているかどうかチェックします.
//-----------------------------------------------------------------------------
bool IsSupportGpuUploadHeap(ID3D12Device* pDevice)
{
    D3D12_FEATURE_DATA_D3D12_OPTIONS16 options16 = {};
    bool gpuUploadHeapSupported = false;
    if (SUCCEEDED(pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS16, &options16, sizeof(options16))))
    {
        gpuUploadHeapSupported = options16.GPUUploadHeapSupported;
    }
    return gpuUploadHeapSupported;
}

} // namespace

namespace asdx {

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
VertexBuffer::VertexBuffer()
{ memset(&m_View, 0, sizeof(m_View)); }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
VertexBuffer::~VertexBuffer()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool VertexBuffer::Init(uint64_t size, uint32_t stride)
{
    auto pDevice = GetD3D12Device();

    if (pDevice == nullptr || size == 0 || stride == 0)
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    auto heapType = IsSupportGpuUploadHeap(pDevice)
        ? D3D12_HEAP_TYPE_GPU_UPLOAD
        : D3D12_HEAP_TYPE_UPLOAD;

    D3D12_HEAP_PROPERTIES prop = {};
    prop.Type                   = heapType;
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

    auto state = D3D12_RESOURCE_STATE_COMMON;
    auto flags = D3D12_HEAP_FLAG_NONE;

    auto pAllocator = GetD3D12MA();
    if (pAllocator != nullptr)
    {
        D3D12MA::ALLOCATION_DESC allocDesc = {};
        allocDesc.HeapType = heapType;

        auto hr = pAllocator->CreateResource(
            &allocDesc,
            &desc,
            state,
            nullptr,
            m_Allocation.GetAddress(),
            IID_PPV_ARGS(m_Resource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : D3D12MA::Allocator::CreateResource() Failed. errcode = 0x%x", hr);
            return false;
        }
    }
    else
    {
        auto hr = pDevice->CreateCommittedResource(
            &prop,
            flags,
            &desc,
            state,
            nullptr,
            IID_PPV_ARGS(m_Resource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    m_View.BufferLocation   = m_Resource->GetGPUVirtualAddress();
    m_View.SizeInBytes      = UINT(size);
    m_View.StrideInBytes    = stride;

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void VertexBuffer::Term()
{
    auto resource = m_Resource.Detach();
    Dispose(resource);
    memset(&m_View, 0, sizeof(m_View));
    m_Allocation.Reset();
}

//-----------------------------------------------------------------------------
//      メモリマッピングを行います.
//-----------------------------------------------------------------------------
void* VertexBuffer::Map()
{
    if (m_Resource.GetPtr() == nullptr)
    { return nullptr; }

    void* ptr = nullptr;
    auto hr = m_Resource->Map(0, nullptr, &ptr);
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Resource::Map() Failed. errcode = 0x%x", hr);
        return nullptr;
    }

    return ptr;
}

//-----------------------------------------------------------------------------
//      メモリマッピングを解除します.
//-----------------------------------------------------------------------------
void VertexBuffer::Unmap()
{
    if (m_Resource.GetPtr() == nullptr)
    { return; }

    m_Resource->Unmap(0, nullptr);
}

//-----------------------------------------------------------------------------
//      頂点バッファビューを取得します.
//-----------------------------------------------------------------------------
D3D12_VERTEX_BUFFER_VIEW VertexBuffer::GetVBV() const
{ return m_View; }

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* VertexBuffer::GetResource() const
{ return m_Resource.GetPtr(); }

//-----------------------------------------------------------------------------
//      デバッグ名を設定します.
//-----------------------------------------------------------------------------
void VertexBuffer::SetName(LPCWSTR tag)
{
    if (m_Resource)
    { m_Resource->SetName(tag); }
}


///////////////////////////////////////////////////////////////////////////////
// IndexBuffer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
IndexBuffer::IndexBuffer()
{ memset(&m_View, 0, sizeof(m_View)); }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
IndexBuffer::~IndexBuffer()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool IndexBuffer::Init(uint64_t size, bool isShortFormat)
{
    auto pDevice = GetD3D12Device();

    if (pDevice == nullptr || size == 0)
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    auto heapType = IsSupportGpuUploadHeap(pDevice)
        ? D3D12_HEAP_TYPE_GPU_UPLOAD
        : D3D12_HEAP_TYPE_UPLOAD;

    D3D12_HEAP_PROPERTIES prop = {};
    prop.Type                   = heapType;
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

    auto state = D3D12_RESOURCE_STATE_COMMON;
    auto flags = D3D12_HEAP_FLAG_NONE;

    auto pAllocator = GetD3D12MA();
    if (pAllocator != nullptr)
    {
        D3D12MA::ALLOCATION_DESC allocDesc = {};
        allocDesc.HeapType = heapType;

        auto hr = pAllocator->CreateResource(
            &allocDesc,
            &desc,
            state,
            nullptr,
            m_Allocation.GetAddress(),
            IID_PPV_ARGS(m_Resource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : D3D12MA::Allocator::CreateResource() Failed. errcode = 0x%x", hr);
            return false;
        }
    }
    else
    {
        auto hr = pDevice->CreateCommittedResource(
            &prop,
            flags,
            &desc,
            state,
            nullptr,
            IID_PPV_ARGS(m_Resource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateCommittedResource() Failed. errode = 0x%x", hr);
            return false;
        }
    }

    m_View.BufferLocation   = m_Resource->GetGPUVirtualAddress();
    m_View.SizeInBytes      = UINT(size);
    m_View.Format           = (isShortFormat) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void IndexBuffer::Term()
{
    auto resource = m_Resource.Detach();
    Dispose(resource);
    memset(&m_View, 0, sizeof(m_View));
    m_Allocation.Reset();
}

//-----------------------------------------------------------------------------
//      メモリマッピングを行います.
//-----------------------------------------------------------------------------
void* IndexBuffer::Map()
{
    if (m_Resource.GetPtr() == nullptr)
    { return nullptr; }

    void* ptr = nullptr;
    auto hr = m_Resource->Map(0, nullptr, &ptr);
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Resource::Map() Failed. errcode = 0x%x", hr);
        return nullptr;
    }

    return ptr;
}

//-----------------------------------------------------------------------------
//      メモリマッピングを解除します.
//-----------------------------------------------------------------------------
void IndexBuffer::Unmap()
{
    if (m_Resource.GetPtr() == nullptr)
    { return; }

    m_Resource->Unmap(0, nullptr);
}

//-----------------------------------------------------------------------------
//      インデックスバッファビューを取得します.
//-----------------------------------------------------------------------------
D3D12_INDEX_BUFFER_VIEW IndexBuffer::GetIBV() const
{ return m_View; }

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* IndexBuffer::GetResource() const
{ return m_Resource.GetPtr(); }

//-----------------------------------------------------------------------------
//      デバッグ名を設定します.
//-----------------------------------------------------------------------------
void IndexBuffer::SetName(LPCWSTR tag)
{
    if (m_Resource)
    { m_Resource->SetName(tag); }
}


///////////////////////////////////////////////////////////////////////////////
// ConstantBuffer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ConstantBuffer::ConstantBuffer()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ConstantBuffer::~ConstantBuffer()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool ConstantBuffer::Init(uint64_t size)
{
    if ( size == 0 )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    auto rest = size % 256;
    if ( rest != 0 )
    {
        ELOG( "Error : ConstantBuffer must be 256 byte alignment., (size %% 256) = %u", rest );
        return false;
    }

    auto pDevice = GetD3D12Device();

    auto heapType = IsSupportGpuUploadHeap(pDevice)
        ? D3D12_HEAP_TYPE_GPU_UPLOAD
        : D3D12_HEAP_TYPE_UPLOAD;

    D3D12_HEAP_PROPERTIES props = {
        heapType,
        D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL_UNKNOWN,
        1,
        1
    };

    D3D12_RESOURCE_DESC desc = {
        D3D12_RESOURCE_DIMENSION_BUFFER,
        0,
        size,
        1,
        1,
        1,
        DXGI_FORMAT_UNKNOWN,
        { 1, 0 },
        D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
        D3D12_RESOURCE_FLAG_NONE
    };

    auto allocator = GetD3D12MA();
    if (allocator != nullptr)
    {
        D3D12MA::ALLOCATION_DESC allocDesc = {};
        allocDesc.HeapType = heapType;

        auto hr = allocator->CreateResource(
            &allocDesc,
            &desc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            m_Allocation.GetAddress(),
            IID_PPV_ARGS(m_Resource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : D3D12MA::Allocator::CreateResource() Failed. errcode = 0x%x", hr);
            return false;
        }
    }
    else
    {
        auto hr = pDevice->CreateCommittedResource(
            &props,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(m_Resource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG( "Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr );
            return false;
        }
    }

    m_Size = size;

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void ConstantBuffer::Term()
{
    auto resource = m_Resource.Detach();
    Dispose(resource);
    m_Allocation.Reset();
    m_Size = 0;
}

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* ConstantBuffer::GetResource() const
{ return m_Resource.GetPtr(); }

//-----------------------------------------------------------------------------
//      GPUアドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetGpuAddress() const
{
    D3D12_GPU_VIRTUAL_ADDRESS result = {};
    if (m_Resource.GetPtr() != nullptr)
    { result = m_Resource->GetGPUVirtualAddress(); }
    return result;
}

//-----------------------------------------------------------------------------
//      サイズを取得します.
//-----------------------------------------------------------------------------
uint64_t ConstantBuffer::GetSize() const
{ return m_Size; }

//-----------------------------------------------------------------------------
//      メモリマッピングを行います.
//-----------------------------------------------------------------------------
void* ConstantBuffer::Map()
{
    if (m_Resource.GetPtr() == nullptr)
    { return nullptr; }

    void* pData = nullptr;
    auto hr = m_Resource->Map(0, nullptr, &pData);
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Resource::Map() Failed. errcode = 0x%x", hr);
        return nullptr;
    }

    return pData;
}

//-----------------------------------------------------------------------------
//      メモリマッピングを解除します.
//-----------------------------------------------------------------------------
void ConstantBuffer::Unmap()
{
    if (m_Resource.GetPtr() == nullptr)
    { return; }

    m_Resource->Unmap(0, nullptr);
}

//-----------------------------------------------------------------------------
//      デバッグ名を設定します.
//-----------------------------------------------------------------------------
void ConstantBuffer::SetName(LPCWSTR tag)
{
    if (m_Resource)
    { m_Resource->SetName(tag); }
}


///////////////////////////////////////////////////////////////////////////////
// ByteAddressBuffer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ByteAddressBuffer::ByteAddressBuffer()
: m_Resource()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ByteAddressBuffer::~ByteAddressBuffer()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool ByteAddressBuffer::Init(uint64_t size, D3D12_RESOURCE_STATES state)
{
    auto pDevice = GetD3D12Device();

    if (pDevice == nullptr || size == 0)
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    auto rest = size % 4;
    if (rest != 0)
    {
        size += rest;
    }

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

    auto allocator = GetD3D12MA();
    if (allocator != nullptr)
    {
        D3D12MA::ALLOCATION_DESC allocDesc = {};
        allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

        auto hr = allocator->CreateResource(
            &allocDesc,
            &desc,
            state,
            nullptr,
            m_Allocation.GetAddress(),
            IID_PPV_ARGS(m_Resource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : D3D12MA::Allocator::CreateResource() Failed. errcode = 0x%x", hr);
            return false;
        }
    }
    else
    {
        auto hr = pDevice->CreateCommittedResource(
            &prop,
            flags,
            &desc,
            state,
            nullptr,
            IID_PPV_ARGS(m_Resource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
    viewDesc.Format                     = DXGI_FORMAT_R32_TYPELESS;
    viewDesc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
    viewDesc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    viewDesc.Buffer.FirstElement        = 0;
    viewDesc.Buffer.NumElements         = UINT(size / 4);
    viewDesc.Buffer.StructureByteStride = 0;
    viewDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_RAW;

    m_HandleSRV = GetResourceDescriptorHeap()->Alloc(1);
    if (!m_HandleSRV.IsValid())
    {
        ELOG("Error : DescriptorHeap::Alloc() Failed.");
        return false;
    }

    pDevice->CreateShaderResourceView(m_Resource.GetPtr(), &viewDesc, GetCpuHandleSRV());
    m_State = state;

    return true;
}

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool ByteAddressBuffer::Init
(
    ID3D12GraphicsCommandList*  pCmdList,
    uint64_t                    size,
    const void*                 pInitData
)
{
    if (!Init(size, D3D12_RESOURCE_STATE_COMMON))
    { return false; }

    UpdateBuffer(pCmdList, m_Resource.GetPtr(), pInitData);

    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource    = m_Resource.GetPtr();
        barrier.Transition.StateBefore  = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter   = D3D12_RESOURCE_STATE_GENERIC_READ;
        barrier.Transition.Subresource  = 0;

        pCmdList->ResourceBarrier(1, &barrier);
    }
    m_State = D3D12_RESOURCE_STATE_GENERIC_READ;

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void ByteAddressBuffer::Term()
{
    if (m_HandleSRV.IsValid())
    { GetResourceDescriptorHeap()->Free(m_HandleSRV); }

    auto resource = m_Resource.Detach();
    Dispose(resource);
    m_Allocation.Reset();
}

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* ByteAddressBuffer::GetResource() const
{ return m_Resource.GetPtr(); }

//-----------------------------------------------------------------------------
//      オフセットハンドルを取得します.
//-----------------------------------------------------------------------------
const OffsetHandle& ByteAddressBuffer::GetOffsetHandleSRV() const
{ return m_HandleSRV; }

//-----------------------------------------------------------------------------
//      CPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE ByteAddressBuffer::GetCpuHandleSRV() const
{
    D3D12_CPU_DESCRIPTOR_HANDLE result = {};
    if (m_HandleSRV.IsValid())
    { result = GetResourceDescriptorHeap()->GetHandleCPU(m_HandleSRV); }
    return result;
}

//-----------------------------------------------------------------------------
//      GPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE ByteAddressBuffer::GetGpuHandleSRV() const
{
    D3D12_GPU_DESCRIPTOR_HANDLE result = {};
    if (m_HandleSRV.IsValid())
    { result = GetResourceDescriptorHeap()->GetHandleGPU(m_HandleSRV); }
    return result;
}

//-----------------------------------------------------------------------------
//      GPUアドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS ByteAddressBuffer::GetGpuAddress() const
{
    D3D12_GPU_VIRTUAL_ADDRESS result = {};
    if (m_Resource.GetPtr() != nullptr)
    { result = m_Resource->GetGPUVirtualAddress(); }
    return result;
}

//-----------------------------------------------------------------------------
//      UAVバリアを設定します.
//-----------------------------------------------------------------------------
void ByteAddressBuffer::UAVBarrier(ID3D12GraphicsCommandList* pCmdList)
{
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type            = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.Flags           = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.UAV.pResource   = m_Resource.GetPtr();

    pCmdList->ResourceBarrier(1, &barrier);
}


///////////////////////////////////////////////////////////////////////////////
// StructuredBuffer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
StructuredBuffer::StructuredBuffer()
: m_Resource   ()
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
    auto pDevice = GetD3D12Device();

    if (pDevice == nullptr || count == 0 || stride == 0)
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    uint64_t size = count * stride;
    auto rest = size % 4;
    if (rest != 0)
    {
        size += rest;
    }

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

    auto allocator = GetD3D12MA();
    if (allocator != nullptr)
    {
        D3D12MA::ALLOCATION_DESC allocDesc = {};
        allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

        auto hr = allocator->CreateResource(
            &allocDesc,
            &desc,
            state,
            nullptr,
            m_Allocation.GetAddress(),
            IID_PPV_ARGS(m_Resource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : D3D12MA::Allocator::CreateResource() Failed. errcode = 0x%x", hr);
            return false;
        }
    }
    else
    {
        auto hr = pDevice->CreateCommittedResource(
            &prop,
            flags,
            &desc,
            state,
            nullptr,
            IID_PPV_ARGS(m_Resource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
    viewDesc.Format                     = DXGI_FORMAT_UNKNOWN;
    viewDesc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
    viewDesc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    viewDesc.Buffer.FirstElement        = 0;
    viewDesc.Buffer.NumElements         = UINT(count);
    viewDesc.Buffer.StructureByteStride = stride;
    viewDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;

    m_HandleSRV = GetResourceDescriptorHeap()->Alloc(1);
    if (!m_HandleSRV.IsValid())
    {
        ELOG("Error : DescriptorHeap::Alloc() Failed.");
        return false;
    }

    pDevice->CreateShaderResourceView(m_Resource.GetPtr(), &viewDesc, GetCpuHandleSRV());
    m_State = state;

    return true;
}

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool StructuredBuffer::Init
(
    ID3D12GraphicsCommandList*  pCmdList,
    uint64_t                    count,
    uint32_t                    stride,
    const void*                 pInitData
)
{
    if (!Init(count, stride, D3D12_RESOURCE_STATE_COMMON))
    { return false;  }

    UpdateBuffer(pCmdList, m_Resource.GetPtr(), pInitData);

    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource    = m_Resource.GetPtr();
        barrier.Transition.StateBefore  = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter   = D3D12_RESOURCE_STATE_GENERIC_READ;
        barrier.Transition.Subresource  = 0;

        pCmdList->ResourceBarrier(1, &barrier);
    }
    m_State = D3D12_RESOURCE_STATE_GENERIC_READ;

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void StructuredBuffer::Term()
{
    if (m_HandleSRV.IsValid())
    { GetResourceDescriptorHeap()->Free(m_HandleSRV); }

    auto resource = m_Resource.Detach();
    Dispose(resource);
    m_Allocation.Reset();
}

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* StructuredBuffer::GetResource() const
{ return m_Resource.GetPtr(); }

//-----------------------------------------------------------------------------
//      オフセットハンドルを取得します.
//-----------------------------------------------------------------------------
const OffsetHandle& StructuredBuffer::GetOffsetHandleSRV() const
{ return m_HandleSRV; }

//-----------------------------------------------------------------------------
//      CPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE StructuredBuffer::GetCpuHandleSRV() const
{
    D3D12_CPU_DESCRIPTOR_HANDLE result = {};
    if (m_HandleSRV.IsValid())
    { result = GetResourceDescriptorHeap()->GetHandleCPU(m_HandleSRV); }
    return result;
}

//-----------------------------------------------------------------------------
//      GPUディスクリプタハンドルを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE StructuredBuffer::GetGpuHandleSRV() const
{
    D3D12_GPU_DESCRIPTOR_HANDLE result = {};
    if (m_HandleSRV.IsValid())
    { result = GetResourceDescriptorHeap()->GetHandleGPU(m_HandleSRV); }
    return result;
}

//-----------------------------------------------------------------------------
//      GPUアドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS StructuredBuffer::GetGpuAddress() const
{
    D3D12_GPU_VIRTUAL_ADDRESS result = {};
    if (m_Resource.GetPtr() != nullptr)
    { result = m_Resource->GetGPUVirtualAddress(); }
    return result;
}

//-----------------------------------------------------------------------------
//      UAVバリアを設定します.
//-----------------------------------------------------------------------------
void StructuredBuffer::UAVBarrier(ID3D12GraphicsCommandList* pCmdList)
{
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type            = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.Flags           = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.UAV.pResource   = m_Resource.GetPtr();

    pCmdList->ResourceBarrier(1, &barrier);
}


} // namespace asdx

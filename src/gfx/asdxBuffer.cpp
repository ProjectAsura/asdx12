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
#include <gfx/asdxUpdateCommand.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxMath.h>
#include <D3D12MemAlloc.h>


namespace asdx {

//-----------------------------------------------------------------------------
//      バッファUAVを生成します.
//-----------------------------------------------------------------------------
bool CreateBufferUAV
(
    UINT64                  bufferSize,
    D3D12_RESOURCE_STATES   initState,
    ID3D12Resource**        ppResource
)
{
    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    D3D12_HEAP_PROPERTIES props = {};
    props.Type                  = D3D12_HEAP_TYPE_DEFAULT;
    props.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
    props.CreationNodeMask      = 1;
    props.VisibleNodeMask       = 1;

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment          = 0;
    desc.Width              = bufferSize;
    desc.Height             = 1;
    desc.DepthOrArraySize   = 1;
    desc.MipLevels          = 1;
    desc.Format             = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    if (initState == D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE)
    { desc.Flags |= D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE; }

    auto hr = pDevice->CreateCommittedResource(
        &props,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        initState,
        nullptr,
        IID_PPV_ARGS(ppResource));
    if (FAILED(hr))
    {
        ELOGA("Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr);
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      アップロードバッファを生成します.
//-----------------------------------------------------------------------------
bool CreateUploadBuffer
(
    UINT64                  bufferSize,
    ID3D12Resource**        ppResource
)
{
    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    auto isGpuUpload = IsSupportGpuUploadHeap();

    D3D12_HEAP_PROPERTIES props = {};
    props.Type                  = (isGpuUpload) ? D3D12_HEAP_TYPE_GPU_UPLOAD : D3D12_HEAP_TYPE_UPLOAD;
    props.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
    props.CreationNodeMask      = 1;
    props.VisibleNodeMask       = 1;

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment          = 0;
    desc.Width              = bufferSize;
    desc.Height             = 1;
    desc.DepthOrArraySize   = 1;
    desc.MipLevels          = 1;
    desc.Format             = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

    auto hr = pDevice->CreateCommittedResource(
        &props,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(ppResource));
    if (FAILED(hr))
    {
        ELOGA("Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr);
        return false;
    }

    return true;
}

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

    auto heapType = IsSupportGpuUploadHeap()
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

        D3D12MA::Allocation* pAllocation = nullptr;

        auto hr = pAllocator->CreateResource(
            &allocDesc,
            &desc,
            state,
            nullptr,
            &pAllocation,
            IID_PPV_ARGS(m_Resource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : D3D12MA::Allocator::CreateResource() Failed. errcode = 0x%x", hr);
            return false;
        }

        m_Holder.Attach(pAllocation);
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
    m_Holder.Reset();
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
//      GPU仮想アドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS VertexBuffer::GetGpuAddress() const
{
    D3D12_GPU_VIRTUAL_ADDRESS result = {};
    if (m_Resource.GetPtr() != nullptr)
    { result = m_Resource->GetGPUVirtualAddress(); }
    return result;
}

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

    auto heapType = IsSupportGpuUploadHeap()
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

        D3D12MA::Allocation* pAllocation = nullptr;

        auto hr = pAllocator->CreateResource(
            &allocDesc,
            &desc,
            state,
            nullptr,
            &pAllocation,
            IID_PPV_ARGS(m_Resource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : D3D12MA::Allocator::CreateResource() Failed. errcode = 0x%x", hr);
            return false;
        }

        m_Holder.Attach(pAllocation);
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
    m_Holder.Reset();
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
//      GPU仮想アドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS IndexBuffer::GetGpuAddress() const
{
    D3D12_GPU_VIRTUAL_ADDRESS result = {};
    if (m_Resource.GetPtr() != nullptr)
    { result = m_Resource->GetGPUVirtualAddress(); }
    return result;
}

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

    auto heapType = IsSupportGpuUploadHeap()
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

        D3D12MA::Allocation* pAllocation = nullptr;

        auto hr = allocator->CreateResource(
            &allocDesc,
            &desc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            &pAllocation,
            IID_PPV_ARGS(m_Resource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : D3D12MA::Allocator::CreateResource() Failed. errcode = 0x%x", hr);
            return false;
        }

        m_Holder.Attach(pAllocation);
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
    m_Holder.Reset();
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

////////////////////////////////////////////////////////////////////////////////
// DoubledConstantBuffer
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
DoubledConstantBuffer::DoubledConstantBuffer()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
DoubledConstantBuffer::~DoubledConstantBuffer()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool DoubledConstantBuffer::Init(uint64_t size)
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

    for(auto i=0; i<2; ++i)
    {
        if (!m_Buffer[i].Init(size))
        { return false; }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void DoubledConstantBuffer::Term()
{
    for(auto i=0; i<2; ++i)
    { m_Buffer[i].Term(); }
}

//-----------------------------------------------------------------------------
//      メモリマッピングを行います.
//-----------------------------------------------------------------------------
void* DoubledConstantBuffer::Map()
{ return m_Buffer[m_Index].Map(); }

//-----------------------------------------------------------------------------
//      メモリマッピングを解除します.
//-----------------------------------------------------------------------------
void DoubledConstantBuffer::Unmap()
{ m_Buffer[m_Index].Unmap(); }

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
ID3D12Resource* DoubledConstantBuffer::GetResource() const
{ return m_Buffer[m_Index].GetResource(); }

//-----------------------------------------------------------------------------
//      GPUアドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS DoubledConstantBuffer::GetGpuAddress() const
{ return GetGpuAddress(m_Index); }

//-----------------------------------------------------------------------------
//      指定インデックスのGPUアドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS DoubledConstantBuffer::GetGpuAddress(uint8_t index) const
{ return m_Buffer[m_Index].GetGpuAddress(); }

//-----------------------------------------------------------------------------
//      サイズを取得します.
//-----------------------------------------------------------------------------
uint64_t DoubledConstantBuffer::GetSize() const
{ return m_Buffer[m_Index].GetSize(); }

//-----------------------------------------------------------------------------
//     データを更新します.
//-----------------------------------------------------------------------------
void DoubledConstantBuffer::Update(const void* pData, uint64_t size, uint64_t offset)
{ Update(m_Index, pData, size, offset); }

//-----------------------------------------------------------------------------
//     指定インデックスのデータを更新します.
//-----------------------------------------------------------------------------
void DoubledConstantBuffer::Update(uint8_t index, const void* pData, uint64_t size, uint64_t offset)
{
    assert(index < 2);
    auto dst = m_Buffer[index].MapAs<uint8_t*>();
    memcpy(dst + offset, pData, size);
    m_Buffer[index].Unmap();
}

//-----------------------------------------------------------------------------
//      バッファを入れ替えます.
//-----------------------------------------------------------------------------
void DoubledConstantBuffer::SwapBuffer()
{ m_Index = (m_Index + 1) & 0x1; }

//-----------------------------------------------------------------------------
//      デバッグ名を設定します.
//-----------------------------------------------------------------------------
void DoubledConstantBuffer::SetName(LPCWSTR tag)
{
    for(auto i=0; i<2; ++i)
    { m_Buffer[i].SetName(tag); }
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
    auto rest = size % 4;
    if ( rest != 0 )
    {
        ELOG( "Error : ByteAddressBuffer must be 4 byte alignment., (size %% 4) = %u", rest );
        return false;
    }

    auto pDevice = GetD3D12Device();

    if (pDevice == nullptr || size == 0)
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    // 4 byte アライメントにする.
    auto bufferSize = RoundUp<uint64_t>(size, 4llu);

    D3D12_HEAP_PROPERTIES prop = {};
    prop.Type                   = D3D12_HEAP_TYPE_DEFAULT;
    prop.CPUPageProperty        = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    prop.MemoryPoolPreference   = D3D12_MEMORY_POOL_UNKNOWN;
    prop.VisibleNodeMask        = 1;
    prop.CreationNodeMask       = 1;

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width              = bufferSize;
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

        D3D12MA::Allocation* pAllocation = nullptr;

        auto hr = allocator->CreateResource(
            &allocDesc,
            &desc,
            state,
            nullptr,
            &pAllocation,
            IID_PPV_ARGS(m_Resource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : D3D12MA::Allocator::CreateResource() Failed. errcode = 0x%x", hr);
            return false;
        }

        m_Holder.Attach(pAllocation);
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
    if (IsSupportGpuUploadHeap())
    {
        auto rest = size % 4;
        if ( rest != 0 )
        {
            ELOG( "Error : ByteAddressBuffer must be 4 byte alignment., (size %% 4) = %u", rest );
            return false;
        }

        auto pDevice = GetD3D12Device();

        if (pDevice == nullptr || size == 0)
        {
            ELOG("Error : Invalid Argument.");
            return false;
        }

        // 4 byte アライメントにする.
        auto bufferSize = RoundUp(size, 4llu);

        D3D12_HEAP_PROPERTIES prop = {};
        prop.Type                   = D3D12_HEAP_TYPE_GPU_UPLOAD;
        prop.CPUPageProperty        = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference   = D3D12_MEMORY_POOL_UNKNOWN;
        prop.VisibleNodeMask        = 1;
        prop.CreationNodeMask       = 1;

        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Width              = bufferSize;
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

            D3D12MA::Allocation* pAllocation = nullptr;

            auto hr = allocator->CreateResource(
                &allocDesc,
                &desc,
                D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                &pAllocation,
                IID_PPV_ARGS(m_Resource.GetAddress()));
            if (FAILED(hr))
            {
                ELOG("Error : D3D12MA::Allocator::CreateResource() Failed. errcode = 0x%x", hr);
                return false;
            }

            m_Holder.Attach(pAllocation);
        }
        else
        {
            auto hr = pDevice->CreateCommittedResource(
                &prop,
                flags,
                &desc,
                D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                IID_PPV_ARGS(m_Resource.GetAddress()));
            if (FAILED(hr))
            {
                ELOG("Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr);
                return false;
            }
        }

        {
            uint8_t* ptr = nullptr;
            auto hr = m_Resource->Map(0, nullptr, reinterpret_cast<void**>(&ptr));
            if (FAILED(hr))
            {
                ELOG("Error : ID3D12Resource::Map() Failed. errcode = 0x%x", hr);
                return false;
            }

            memcpy(ptr, pInitData, size);
            m_Resource->Unmap(0, nullptr);
        }

        m_State = D3D12_RESOURCE_STATE_COMMON;

        return true;
    }

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
    auto resource = m_Resource.Detach();
    Dispose(resource);
    m_Holder.Reset();
}

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* ByteAddressBuffer::GetResource() const
{ return m_Resource.GetPtr(); }

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

//-----------------------------------------------------------------------------
//      デバッグ名を設定します.
//-----------------------------------------------------------------------------
void ByteAddressBuffer::SetName(LPCWSTR tag)
{
    if (m_Resource)
    { m_Resource->SetName(tag); }
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
    auto size = count * stride;
    auto rest = size % 4;
    if ( rest != 0 )
    {
        ELOG( "Error : StructuredBuffer must be 4 byte alignment., (size %% 4) = %u", rest );
        return false;
    }

    auto pDevice = GetD3D12Device();

    if (pDevice == nullptr || count == 0 || stride == 0)
    {
        ELOG("Error : Invalid Argument.");
        return false;
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

        D3D12MA::Allocation* pAllocation = nullptr;

        auto hr = allocator->CreateResource(
            &allocDesc,
            &desc,
            state,
            nullptr,
            &pAllocation,
            IID_PPV_ARGS(m_Resource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : D3D12MA::Allocator::CreateResource() Failed. errcode = 0x%x", hr);
            return false;
        }

        m_Holder.Attach(pAllocation);
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
    if (IsSupportGpuUploadHeap())
    {
        auto size = count * stride;
        auto rest = size % 4;
        if ( rest != 0 )
        {
            ELOG( "Error : StructuredBuffer must be 4 byte alignment., (size %% 4) = %u", rest );
            return false;
        }

        auto pDevice = GetD3D12Device();

        if (pDevice == nullptr || count == 0 || stride == 0)
        {
            ELOG("Error : Invalid Argument.");
            return false;
        }

        D3D12_HEAP_PROPERTIES prop = {};
        prop.Type                   = D3D12_HEAP_TYPE_GPU_UPLOAD;
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

            D3D12MA::Allocation* pAllocation = nullptr;

            auto hr = allocator->CreateResource(
                &allocDesc,
                &desc,
                D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                &pAllocation,
                IID_PPV_ARGS(m_Resource.GetAddress()));
            if (FAILED(hr))
            {
                ELOG("Error : D3D12MA::Allocator::CreateResource() Failed. errcode = 0x%x", hr);
                return false;
            }

            m_Holder.Attach(pAllocation);
        }
        else
        {
            auto hr = pDevice->CreateCommittedResource(
                &prop,
                flags,
                &desc,
                D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                IID_PPV_ARGS(m_Resource.GetAddress()));
            if (FAILED(hr))
            {
                ELOG("Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr);
                return false;
            }
        }

        m_State = D3D12_RESOURCE_STATE_COMMON;
        return true;
    }

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
    auto resource = m_Resource.Detach();
    Dispose(resource);
    m_Holder.Reset();
}

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* StructuredBuffer::GetResource() const
{ return m_Resource.GetPtr(); }

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

//-----------------------------------------------------------------------------
//      デバッグ名を設定します.
//-----------------------------------------------------------------------------
void StructuredBuffer::SetName(LPCWSTR tag)
{
    if (m_Resource)
    { m_Resource->SetName(tag); }
}

///////////////////////////////////////////////////////////////////////////////
// ScratchBuffer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ScratchBuffer::ScratchBuffer()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ScratchBuffer::~ScratchBuffer()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool ScratchBuffer::Init(size_t size)
{
    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    D3D12_HEAP_PROPERTIES props = {};
    props.Type                  = D3D12_HEAP_TYPE_DEFAULT;
    props.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
    props.CreationNodeMask      = 1;
    props.VisibleNodeMask       = 1;

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment          = 0;
    desc.Width              = size;
    desc.Height             = 1;
    desc.DepthOrArraySize   = 1;
    desc.MipLevels          = 1;
    desc.Format             = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    auto allocator = GetD3D12MA();
    if (allocator != nullptr)
    {
        D3D12MA::ALLOCATION_DESC allocDesc = {};
        allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

        D3D12MA::Allocation* pAllocation = nullptr;

        auto hr = allocator->CreateResource(
            &allocDesc,
            &desc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            &pAllocation,
            IID_PPV_ARGS(m_Resource.GetAddress()));
        if (FAILED(hr))
        {
            ELOG("Error : D3D12MA::Allocator::CreateResource() Failed. errcode = 0x%x", hr);
            return false;
        }

        m_Holder.Attach(pAllocation);
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
            ELOGA("Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr);
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void ScratchBuffer::Term()
{
    m_Holder.Reset();
    auto resource = m_Resource.Detach();
    Dispose(resource);
}

//-----------------------------------------------------------------------------
//      GPU仮想アドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS ScratchBuffer::GetGpuAddress() const
{ 
    D3D12_GPU_VIRTUAL_ADDRESS result = {};
    if (m_Resource.GetPtr() != nullptr)
    { result = m_Resource->GetGPUVirtualAddress(); }
    return result;
}

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* ScratchBuffer::GetResource() const
{ return m_Resource.GetPtr(); }

//-----------------------------------------------------------------------------
//      デバッグ名を設定します.
//-----------------------------------------------------------------------------
void ScratchBuffer::SetName(LPCWSTR tag)
{
    if (m_Resource)
    { m_Resource->SetName(tag); }
}

///////////////////////////////////////////////////////////////////////////////
// AccelerationStructure class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
AccelerationStructure::AccelerationStructure()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
AccelerationStructure::~AccelerationStructure()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool AccelerationStructure::Init
(
    ID3D12GraphicsCommandList4*                                 pCmd,
    const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS* pInputs
)
{
    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    // ビルド前情報を取得.
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo = {};
    pDevice->GetRaytracingAccelerationStructurePrebuildInfo(pInputs, &prebuildInfo);
    if (prebuildInfo.ResultDataMaxSizeInBytes == 0)
    { return false; }

    auto scratchBufferSize = (prebuildInfo.ScratchDataSizeInBytes > prebuildInfo.UpdateScratchDataSizeInBytes)
        ? prebuildInfo.ScratchDataSizeInBytes
        : prebuildInfo.UpdateScratchDataSizeInBytes;

    // 高速化機構用バッファを生成.
    {
        D3D12_HEAP_PROPERTIES props = {};
        props.Type                  = D3D12_HEAP_TYPE_DEFAULT;
        props.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        props.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
        props.CreationNodeMask      = 1;
        props.VisibleNodeMask       = 1;

        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment          = 0;
        desc.Width              = prebuildInfo.ResultDataMaxSizeInBytes;
        desc.Height             = 1;
        desc.DepthOrArraySize   = 1;
        desc.MipLevels          = 1;
        desc.Format             = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
                                | D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE;

        auto allocator = GetD3D12MA();
        if (allocator != nullptr)
        {
            D3D12MA::ALLOCATION_DESC allocDesc = {};
            allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

            D3D12MA::Allocation* pAllocation = nullptr;

            auto hr = allocator->CreateResource(
                &allocDesc,
                &desc,
                D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
                nullptr,
                &pAllocation,
                IID_PPV_ARGS(m_Resource.GetAddress()));
            if (FAILED(hr))
            {
                ELOG("Error : D3D12MA::Allocator::CreateResource() Failed. errcode = 0x%x", hr);
                return false;
            }

            m_Holder = AllocationHolder(pAllocation);
        }
        else
        {
            auto hr = pDevice->CreateCommittedResource(
                &props,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
                nullptr,
                IID_PPV_ARGS(m_Resource.GetAddress()));
            if (FAILED(hr))
            {
                ELOGA("Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr);
                return false;
            }
        }
    }

    // スクラッチバッファ生成.
    ScratchBuffer scratchBuffer;
    if (!scratchBuffer.Init(scratchBufferSize))
    {
        ELOG("Error : ScratchBuffer::Init() Failed.");
        return false;
    }

    // 高速化機構をビルド.
    {
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc = {};
        desc.Inputs                           = (*pInputs);
        desc.DestAccelerationStructureData    = m_Resource->GetGPUVirtualAddress();
        desc.ScratchAccelerationStructureData = scratchBuffer.GetGpuAddress();

        pCmd->BuildRaytracingAccelerationStructure(&desc, 0, nullptr);

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type          = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrier.UAV.pResource = m_Resource.GetPtr();
        pCmd->ResourceBarrier(1, &barrier);
    }

    // スクラッチバッファを遅延解放.
    scratchBuffer.Term();

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void AccelerationStructure::Term()
{
    auto resource = m_Resource.Detach();
    Dispose(resource);

    m_Holder.Reset();
}

//-----------------------------------------------------------------------------
//      GPU仮想アドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS AccelerationStructure::GetGpuAddress() const 
{
    D3D12_GPU_VIRTUAL_ADDRESS result = {};
    if (m_Resource.GetPtr() != nullptr)
    { result = m_Resource->GetGPUVirtualAddress(); }
    return result;
}

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* AccelerationStructure::GetResource() const
{ return m_Resource.GetPtr(); }

//-----------------------------------------------------------------------------
//      デバッグ名を設定します.
//-----------------------------------------------------------------------------
void AccelerationStructure::SetName(LPCWSTR tag)
{
    if (m_Resource)
    { m_Resource->SetName(tag); }
}

} // namespace asdx

//-----------------------------------------------------------------------------
// File : asdxRayTracing.cpp
// Desc : DXR Helper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxRayTracing.h>
#include <asdxLogger.h>
#include <asdxGraphicsDevice.h>
#include <asdxMisc.h>


namespace asdx {

//-----------------------------------------------------------------------------
//      DXRがサポートされているかどうかチェックします.
//-----------------------------------------------------------------------------
bool IsSupportDXR(ID3D12Device6* pDevice)
{
    D3D12_FEATURE_DATA_D3D12_OPTIONS5 options = {};
    auto hr = pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options, sizeof(options));
    if (FAILED(hr))
    { return false; }

    return options.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
}

//-----------------------------------------------------------------------------
//      UAVバッファを生成します.
//-----------------------------------------------------------------------------
bool CreateUAVBuffer
(
    ID3D12Device*           pDevice,
    UINT64                  bufferSize,
    ID3D12Resource**        ppResource,
    D3D12_RESOURCE_STATES   initialResourceState
)
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
    desc.Width              = bufferSize;
    desc.Height             = 1;
    desc.DepthOrArraySize   = 1;
    desc.MipLevels          = 1;
    desc.Format             = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    auto hr = pDevice->CreateCommittedResource(
        &props,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        initialResourceState,
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
    ID3D12Device*           pDevice,
    UINT64                  bufferSize,
    ID3D12Resource**        ppResource
)
{
    D3D12_HEAP_PROPERTIES props = {};
    props.Type                  = D3D12_HEAP_TYPE_UPLOAD;
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
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(ppResource));
    if (FAILED(hr))
    {
        ELOGA("Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr);
        return false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// Blas class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Blas::~Blas()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Blas::Init
(
    ID3D12Device6*              pDevice,
    uint32_t                    count,
    const DXR_GEOMETRY_DESC*    pDescs,
    DXR_BUILD_FLAGS             flags
)
{
    if (pDevice == nullptr)
    { return false; }

    m_GeometryDesc.resize(count);
    if (pDescs != nullptr)
    {
        for(auto i=0u; i<count; ++i)
        { m_GeometryDesc[i] = pDescs[i]; }
    }

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
    inputs.DescsLayout      = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.Flags            = flags;
    inputs.NumDescs         = count;
    inputs.pGeometryDescs   = m_GeometryDesc.data();
    inputs.Type             = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo = {};
    pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuildInfo);
    if (prebuildInfo.ResultDataMaxSizeInBytes == 0)
    { return false; }

    if (!CreateUAVBuffer(
        pDevice,
        prebuildInfo.ScratchDataSizeInBytes,
        m_Scratch.GetAddress(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
    {
        ELOGA("Error : CreateUAVBuffer() Failed.");
        return false;
    }
    m_Scratch->SetName(L"asdxBlasScratch");

    if (!CreateUAVBuffer(
        pDevice,
        prebuildInfo.ResultDataMaxSizeInBytes,
        m_Structure.GetAddress(),
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE))
    {
        ELOGA("Error : CreateUAVBuffer() Failed.");
        return false;
    }
    m_Structure->SetName(L"asdxBlas");

    m_BuildDesc.Inputs                              = inputs;
    m_BuildDesc.ScratchAccelerationStructureData    = m_Scratch->GetGPUVirtualAddress();
    m_BuildDesc.DestAccelerationStructureData       = m_Structure->GetGPUVirtualAddress();

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Blas::Term()
{
    m_GeometryDesc.clear();
    m_Scratch   .Reset();
    m_Structure .Reset();
}

//-----------------------------------------------------------------------------
//      ジオメトリ数を取得します.
//-----------------------------------------------------------------------------
uint32_t Blas::GetGeometryCount() const
{ return uint32_t(m_GeometryDesc.size()); }

//-----------------------------------------------------------------------------
//      ジオメトリ構成を取得します.
//-----------------------------------------------------------------------------
const DXR_GEOMETRY_DESC& Blas::GetGeometry(uint32_t index) const
{
    assert(index < uint32_t(m_GeometryDesc.size()));
    return m_GeometryDesc[index];
}

//-----------------------------------------------------------------------------
//      ジオメトリ構成を設定します.
//-----------------------------------------------------------------------------
void Blas::SetGeometry(uint32_t index, const DXR_GEOMETRY_DESC& desc)
{
    assert(index < uint32_t(m_GeometryDesc.size()));
    m_GeometryDesc[index] = desc;
}

//-----------------------------------------------------------------------------
//      GPU仮想アドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS Blas::GetGPUVirtualAddress() const
{
    if (m_Structure.GetPtr() == nullptr)
    { return D3D12_GPU_VIRTUAL_ADDRESS(); }

    return m_Structure->GetGPUVirtualAddress();
}

//-----------------------------------------------------------------------------
//      ビルドします.
//-----------------------------------------------------------------------------
void Blas::Build(ID3D12GraphicsCommandList6* pCmd)
{
    pCmd->BuildRaytracingAccelerationStructure(&m_BuildDesc, 0, nullptr);

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type            = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.UAV.pResource   = m_Structure.GetPtr();
    pCmd->ResourceBarrier(1, &barrier);
}


///////////////////////////////////////////////////////////////////////////////
// Tlas class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Tlas::~Tlas()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Tlas::Init
(
    ID3D12Device6*              pDevice,
    uint32_t                    instanceDescCount,
    const DXR_INSTANCE_DESC*    pInstanceDescs,
    DXR_BUILD_FLAGS             flags
)
{
    if (!CreateUploadBuffer(
        pDevice, sizeof(DXR_INSTANCE_DESC) * instanceDescCount, 
        m_Instances.GetAddress()))
    {
        ELOGA("Error : CreateUploadBuffer() Failed.");
        return false;
    }

    // インスタンス設定をコピー.
    {
        DXR_INSTANCE_DESC* ptr = nullptr;
        auto hr = m_Instances->Map(0, nullptr, reinterpret_cast<void**>(&ptr));
        if (FAILED(hr))
        {
            ELOGA("Error : ID3D12Resource::Map() Failed. errcode = 0x%x", hr);
            return false;
        }

        memcpy(ptr, pInstanceDescs, sizeof(DXR_INSTANCE_DESC) * instanceDescCount);

        m_Instances->Unmap(0, nullptr);
    }

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
    inputs.DescsLayout      = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.Flags            = flags;
    inputs.NumDescs         = instanceDescCount;
    inputs.InstanceDescs    = m_Instances->GetGPUVirtualAddress();
    inputs.Type             = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo = {};
    pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuildInfo);
    if (prebuildInfo.ResultDataMaxSizeInBytes == 0)
    { return false; }

    if (!CreateUAVBuffer(
        pDevice,
        prebuildInfo.ScratchDataSizeInBytes,
        m_Scratch.GetAddress(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
    {
        ELOGA("Error : CreateUAVBuffer() Failed.");
        return false;
    }
    m_Scratch->SetName(L"asdxTlasScratch");

    if (!CreateUAVBuffer(
        pDevice,
        prebuildInfo.ResultDataMaxSizeInBytes,
        m_Structure.GetAddress(),
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE))
    {
        ELOGA("Error : CreateUAVBuffer() Failed.");
        return false;
    }
    m_Structure->SetName(L"asdxTlas");

    m_BuildDesc.Inputs                              = inputs;
    m_BuildDesc.ScratchAccelerationStructureData    = m_Scratch->GetGPUVirtualAddress();
    m_BuildDesc.DestAccelerationStructureData       = m_Structure->GetGPUVirtualAddress();

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Tlas::Term()
{
    m_Instances .Reset();
    m_Scratch   .Reset();
    m_Structure .Reset();
}

//-----------------------------------------------------------------------------
//      メモリマッピングを行います.
//-----------------------------------------------------------------------------
DXR_INSTANCE_DESC* Tlas::Map()
{
    DXR_INSTANCE_DESC* ptr = nullptr;
    auto hr = m_Instances->Map(0, nullptr, reinterpret_cast<void**>(&ptr));
    if (FAILED(hr))
    { return nullptr; }

    return ptr;
}

//-----------------------------------------------------------------------------
//      メモリマッピングを解除します.
//-----------------------------------------------------------------------------
void Tlas::Unmap()
{ m_Instances->Unmap(0, nullptr); }

//-----------------------------------------------------------------------------
//      GPU仮想アドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS Tlas::GetGPUVirtualAddress() const
{
    if (m_Structure.GetPtr() == nullptr)
    { return D3D12_GPU_VIRTUAL_ADDRESS(); }

    return m_Structure->GetGPUVirtualAddress();
}

//-----------------------------------------------------------------------------
//      ビルドします.
//-----------------------------------------------------------------------------
void Tlas::Build(ID3D12GraphicsCommandList6* pCmd)
{
    pCmd->BuildRaytracingAccelerationStructure(&m_BuildDesc, 0, nullptr);

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type            = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.UAV.pResource   = m_Structure.GetPtr();
    pCmd->ResourceBarrier(1, &barrier);
}


///////////////////////////////////////////////////////////////////////////////
// SubObjects
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
SubObjects::~SubObjects()
{ Clear(); }

//-----------------------------------------------------------------------------
//      サブオブジェクトを生成します.
//-----------------------------------------------------------------------------
void* SubObjects::Create(D3D12_STATE_SUBOBJECT_TYPE type, size_t size)
{
    auto ptr = malloc(size);
    if (ptr == nullptr)
    { return nullptr; }

    D3D12_STATE_SUBOBJECT item = {};
    item.Type  = type;
    item.pDesc = ptr;
    m_Objects.push_back(item);

    return ptr;
}

//-----------------------------------------------------------------------------
//      メモリを解放します.
//-----------------------------------------------------------------------------
void SubObjects::Clear()
{
    for(auto& itr : m_Objects)
    {
        if (itr.pDesc != nullptr)
        {
            // 別メモリ持って管理するの面倒だから..
            auto ptr = const_cast<void*>(itr.pDesc);

            free(ptr);
            itr.pDesc = nullptr;
        }
    }

    m_Objects.clear();
}

//-----------------------------------------------------------------------------
//      ステートオブジェクトの構成設定を作成します.
//-----------------------------------------------------------------------------
D3D12_STATE_OBJECT_DESC SubObjects::GetDesc() const
{
    D3D12_STATE_OBJECT_DESC result = {};
    result.Type          = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
    result.NumSubobjects = UINT(m_Objects.size());
    result.pSubobjects   = m_Objects.data();

    return result;
}

///////////////////////////////////////////////////////////////////////////////
// ShaderTable class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ShaderTable::ShaderTable()
: m_RecordSize(0)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ShaderTable::~ShaderTable()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理です.
//-----------------------------------------------------------------------------
bool ShaderTable::Init
(
    ID3D12Device*       pDevice,
    uint32_t            recordCount,
    const ShaderRecord* records,
    uint32_t            localRootArgumentSize
)
{
    // アライメントを揃える.
    m_RecordSize = RoundUp(
        D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + localRootArgumentSize,
        D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);

    auto bufferSize = m_RecordSize * recordCount;

    if (!CreateUploadBuffer(pDevice, bufferSize, m_Resource.GetAddress()))
    {
        ELOGA("Error : CreateUploadBuffer() Failed.");
        return false;
    }

    uint8_t* ptr = nullptr;
    auto hr = m_Resource->Map(0, nullptr, reinterpret_cast<void**>(&ptr));
    if (FAILED(hr))
    {
        ELOGA("Error : ID3D12Resource::Map() Failed. errcode = 0x%x", hr);
        return false;
    }

    // 大きく分岐して高速化.
    if (localRootArgumentSize == 0)
    {
        for(auto i=0u; i<recordCount; ++i)
        {
            auto record = records[i];
            memcpy(ptr, record.ShaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
            ptr += m_RecordSize;
        }
    }
    else
    {
        for(auto i=0u; i<recordCount; ++i)
        {
            auto record = records[i];
            memcpy(ptr, record.ShaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
            memcpy(ptr + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, record.LocalRootArguments, localRootArgumentSize);
            ptr += m_RecordSize;
        }
    }

    m_Resource->Unmap(0, nullptr);

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理です.
//-----------------------------------------------------------------------------
void ShaderTable::Term()
{ m_Resource.Reset(); }

//-----------------------------------------------------------------------------
//      GPU仮想アドレスと範囲を取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS_RANGE ShaderTable::GetRecordView() const
{
    D3D12_GPU_VIRTUAL_ADDRESS_RANGE result = {};
    result.StartAddress = m_Resource->GetGPUVirtualAddress();
    result.SizeInBytes  = m_Resource->GetDesc().Width;
    return result;
}

//-----------------------------------------------------------------------------
//      GPU仮想アドレスと範囲とストライドを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE ShaderTable::GetTableView() const
{
    D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE result = {};
    result.StartAddress     = m_Resource->GetGPUVirtualAddress();
    result.SizeInBytes      = m_Resource->GetDesc().Width;
    result.StrideInBytes    = m_RecordSize;
    return result;
}

} // namespace asdx

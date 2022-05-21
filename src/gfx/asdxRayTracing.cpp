//-----------------------------------------------------------------------------
// File : asdxRayTracing.cpp
// Desc : DXR Helper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include <fnd/asdxMisc.h>
#include <gfx/asdxRayTracing.h>
#include <gfx/asdxGraphicsSystem.h>


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
//      バッファUAVを生成します.
//-----------------------------------------------------------------------------
bool CreateBufferUAV
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
//      バッファSRVを生成します.
//-----------------------------------------------------------------------------
bool CreateBufferSRV
(
    ID3D12Device*           pDevice,
    ID3D12Resource*         pResource,
    UINT                    elementCount,
    UINT                    elementSize,
    IShaderResourceView**   ppView
)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
    desc.ViewDimension           = D3D12_SRV_DIMENSION_BUFFER;
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc.Buffer.NumElements      = elementCount;
    if (elementSize == 0)
    {
        desc.Format                     = DXGI_FORMAT_R32_TYPELESS;
        desc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_RAW;
        desc.Buffer.StructureByteStride = 0;
    }
    else
    {
        desc.Format                     = DXGI_FORMAT_UNKNOWN;
        desc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;
        desc.Buffer.StructureByteStride = elementSize;
    }

    return asdx::CreateShaderResourceView(pResource, &desc, ppView);
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

    // 設定をコピっておく.
    m_GeometryDesc.resize(count);
    if (pDescs != nullptr)
    {
        for(auto i=0u; i<count; ++i)
        { m_GeometryDesc[i] = pDescs[i]; }
    }

    // 高速化機構の入力設定.
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
    inputs.DescsLayout      = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.Flags            = flags;
    inputs.NumDescs         = count;
    inputs.pGeometryDescs   = m_GeometryDesc.data();
    inputs.Type             = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

    // ビルド前情報を取得.
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo = {};
    pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuildInfo);
    if (prebuildInfo.ResultDataMaxSizeInBytes == 0)
    { return false; }

    // スクラッチバッファ生成.
    if (!CreateBufferUAV(
        pDevice,
        prebuildInfo.ScratchDataSizeInBytes,
        m_Scratch.GetAddress(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
    {
        ELOGA("Error : CreateUAVBuffer() Failed.");
        return false;
    }
    m_Scratch->SetName(L"asdxBlasScratch");

    // 高速化機構用バッファ生成.
    if (!CreateBufferUAV(
        pDevice,
        prebuildInfo.ResultDataMaxSizeInBytes,
        m_Structure.GetAddress(),
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE))
    {
        ELOGA("Error : CreateUAVBuffer() Failed.");
        return false;
    }
    m_Structure->SetName(L"asdxBlas");

    // ビルド設定.
    memset(&m_BuildDesc, 0, sizeof(m_BuildDesc));
    m_BuildDesc.Inputs                              = inputs;
    m_BuildDesc.ScratchAccelerationStructureData    = m_Scratch->GetGPUVirtualAddress();
    m_BuildDesc.DestAccelerationStructureData       = m_Structure->GetGPUVirtualAddress();

    // 正常終了.
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
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* Blas::GetResource() const
{ return m_Structure.GetPtr(); }

//-----------------------------------------------------------------------------
//      ビルドします.
//-----------------------------------------------------------------------------
void Blas::Build(ID3D12GraphicsCommandList6* pCmd)
{
    // 高速化機構を構築.
    pCmd->BuildRaytracingAccelerationStructure(&m_BuildDesc, 0, nullptr);

    // バリアを張っておく.
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
    // アップロードバッファ生成.
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

    // 高速化機構の入力設定.
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
    inputs.DescsLayout      = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.Flags            = flags;
    inputs.NumDescs         = instanceDescCount;
    inputs.InstanceDescs    = m_Instances->GetGPUVirtualAddress();
    inputs.Type             = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

    // ビルド前情報を取得.
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo = {};
    pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuildInfo);
    if (prebuildInfo.ResultDataMaxSizeInBytes == 0)
    { return false; }

    // スクラッチバッファ生成.
    if (!CreateBufferUAV(
        pDevice,
        prebuildInfo.ScratchDataSizeInBytes,
        m_Scratch.GetAddress(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
    {
        ELOGA("Error : CreateUAVBuffer() Failed.");
        return false;
    }
    m_Scratch->SetName(L"asdxTlasScratch");

    // 高速化機構用バッファ生成.
    if (!CreateBufferUAV(
        pDevice,
        prebuildInfo.ResultDataMaxSizeInBytes,
        m_Structure.GetAddress(),
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE))
    {
        ELOGA("Error : CreateUAVBuffer() Failed.");
        return false;
    }
    m_Structure->SetName(L"asdxTlas");

    // ビルド設定.
    memset(&m_BuildDesc, 0, sizeof(m_BuildDesc));
    m_BuildDesc.Inputs                              = inputs;
    m_BuildDesc.ScratchAccelerationStructureData    = m_Scratch->GetGPUVirtualAddress();
    m_BuildDesc.DestAccelerationStructureData       = m_Structure->GetGPUVirtualAddress();

    // 正常終了.
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
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* Tlas::GetResource() const
{ return m_Structure.GetPtr(); }

//-----------------------------------------------------------------------------
//      ビルドします.
//-----------------------------------------------------------------------------
void Tlas::Build(ID3D12GraphicsCommandList6* pCmd)
{
    // 高速化機構を構築.
    pCmd->BuildRaytracingAccelerationStructure(&m_BuildDesc, 0, nullptr);

    // バリアを張っておく.
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type            = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.UAV.pResource   = m_Structure.GetPtr();
    pCmd->ResourceBarrier(1, &barrier);
}


///////////////////////////////////////////////////////////////////////////////
// RayTracingPipelineState class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
RayTracingPipelineState::RayTracingPipelineState()
: m_pObject (nullptr)
, m_pProps  (nullptr)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
RayTracingPipelineState::~RayTracingPipelineState()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool RayTracingPipelineState::Init(ID3D12Device5* pDevice, const RayTracingPipelineStateDesc& desc)
{
    uint32_t objCount = 6 + desc.HitGroupCount;

    std::vector<D3D12_STATE_SUBOBJECT> objDesc;
    objDesc.resize(objCount);

    auto index = 0;

    D3D12_GLOBAL_ROOT_SIGNATURE globalRootSignature = {};
    globalRootSignature.pGlobalRootSignature = desc.pGlobalRootSignature;

    objDesc[index].Type  = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
    objDesc[index].pDesc = &globalRootSignature;
    index++;

    D3D12_LOCAL_ROOT_SIGNATURE localRootSignature = {};
    if (desc.pLocalRootSignature != nullptr)
    {
        localRootSignature.pLocalRootSignature = desc.pLocalRootSignature;

        objDesc[index].Type  = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
        objDesc[index].pDesc = &localRootSignature;
        index++;
    }

    D3D12_DXIL_LIBRARY_DESC libDesc = {};
    libDesc.DXILLibrary = desc.DXILLibrary;
    libDesc.NumExports  = desc.ExportCount;
    libDesc.pExports    = desc.pExports;

    objDesc[index].Type     = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    objDesc[index].pDesc    = &libDesc;
    index++;

    D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
    shaderConfig.MaxAttributeSizeInBytes = desc.MaxAttributeSize;
    shaderConfig.MaxPayloadSizeInBytes   = desc.MaxPayloadSize;

    objDesc[index].Type     = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
    objDesc[index].pDesc    = &shaderConfig;
    index++;

    D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
    pipelineConfig.MaxTraceRecursionDepth = desc.MaxTraceRecursionDepth;

    objDesc[index].Type     = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
    objDesc[index].pDesc    = &pipelineConfig;
    index++;

    for(auto i=0u; i<desc.HitGroupCount; ++i)
    {
        objDesc[index].Type  = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
        objDesc[index].pDesc = &desc.pHitGroups[i];
        index++;
    }

    D3D12_STATE_OBJECT_DESC stateObjectDesc = {};
    stateObjectDesc.Type            = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
    stateObjectDesc.NumSubobjects   = index;
    stateObjectDesc.pSubobjects     = objDesc.data();

    auto hr = pDevice->CreateStateObject(&stateObjectDesc, IID_PPV_ARGS(m_pObject.GetAddress()));

    // メモリ解放.
    objDesc.clear();

    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device5::CreateStateObject() Failed. errcode = 0x%x", hr);
        return false;
    }

    hr = m_pObject->QueryInterface(IID_PPV_ARGS(m_pProps.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12StateObject::QueryInterface() Failed. errcode = 0x%x", hr);
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void RayTracingPipelineState::Term()
{
    m_pObject.Reset();
    m_pProps .Reset();
}

//-----------------------------------------------------------------------------
//      シェーダ識別子を取得します.
//-----------------------------------------------------------------------------
void* RayTracingPipelineState::GetShaderIdentifier(const wchar_t* exportName) const
{ return m_pProps->GetShaderIdentifier(exportName); }

//-----------------------------------------------------------------------------
//      シェーダスタックサイズを取得します.
//-----------------------------------------------------------------------------
UINT64 RayTracingPipelineState::GetShaderStackSize(const wchar_t* exportName) const
{ return m_pProps->GetShaderStackSize(exportName); }

//-----------------------------------------------------------------------------
//      ステートオブジェクトを取得します.
//-----------------------------------------------------------------------------
ID3D12StateObject* RayTracingPipelineState::GetStateObject() const
{ return m_pObject.GetPtr(); }


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
bool ShaderTable::Init(ID3D12Device* pDevice, const Desc* pDesc)
{
    // アライメントを揃える.
    m_RecordSize = RoundUp(
        D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + pDesc->LocalRootArgumentSize,
        D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);

    auto bufferSize = m_RecordSize * pDesc->RecordCount;

    // アップロードバッファ生成.
    if (!CreateUploadBuffer(pDevice, bufferSize, m_Resource.GetAddress()))
    {
        ELOGA("Error : CreateUploadBuffer() Failed.");
        return false;
    }

    // メモリマッピング.
    uint8_t* ptr = nullptr;
    auto hr = m_Resource->Map(0, nullptr, reinterpret_cast<void**>(&ptr));
    if (FAILED(hr))
    {
        ELOGA("Error : ID3D12Resource::Map() Failed. errcode = 0x%x", hr);
        return false;
    }

    // 大きく分岐して高速化.
    if (pDesc->LocalRootArgumentSize == 0)
    {
        for(auto i=0u; i<pDesc->RecordCount; ++i)
        {
            auto record = pDesc->pRecords[i];
            memcpy(ptr, record.ShaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
            ptr += m_RecordSize;
        }
    }
    else
    {
        for(auto i=0u; i<pDesc->RecordCount; ++i)
        {
            auto record = pDesc->pRecords[i];
            memcpy(ptr, record.ShaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
            memcpy(ptr + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, record.LocalRootArguments, pDesc->LocalRootArgumentSize);
            ptr += m_RecordSize;
        }
    }

    // メモリマッピング解除.
    m_Resource->Unmap(0, nullptr);

    // 正常終了.
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

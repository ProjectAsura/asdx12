//-----------------------------------------------------------------------------
// File : asdxDxr.cpp
// Desc : DirectX RayTracing Utility.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes.
//-----------------------------------------------------------------------------
#include <cassert>
#include <vector>
#include <fnd/asdxLogger.h>
#include <fnd/asdxMisc.h>
#include <gfx/asdxDXR.h>
#include <gfx/asdxDevice.h>


namespace {

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
//      シェーダテーブルを生成します.
//-----------------------------------------------------------------------------
bool CreateShaderTable
(
    ID3D12Device*           pDevice,
    std::vector<void*>      shaderIdentifiers,
    ID3D12Resource**        ppResource
)
{
    auto isGpuUploadHeap = false;

    D3D12_FEATURE_DATA_D3D12_OPTIONS16 options16 = {};
    if (SUCCEEDED(pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS16, &options16, sizeof(options16))))
    {
        if (options16.GPUUploadHeapSupported)
        { isGpuUploadHeap = true; }
    }

    auto recordSize = asdx::RoundUp(
        uint32_t(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES),
        uint32_t(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT));

    auto bufferSize = recordSize * shaderIdentifiers.size();

    D3D12_HEAP_PROPERTIES props = {};
    props.Type                  = (isGpuUploadHeap) ? D3D12_HEAP_TYPE_GPU_UPLOAD : D3D12_HEAP_TYPE_UPLOAD;
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

    uint8_t* ptr = nullptr;
    hr = (*ppResource)->Map(0, nullptr, reinterpret_cast<void**>(&ptr));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Resource::Map() Failed. errcode = 0x%x", hr);
        return false;
    }

    for(size_t i=0u; i<shaderIdentifiers.size(); ++i)
    {
        memcpy(ptr, shaderIdentifiers[i], D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
        ptr += recordSize;
    }
    (*ppResource)->Unmap(0, nullptr);

    return true;
}

} // namespace


namespace asdx {

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

    auto size = (prebuildInfo.ScratchDataSizeInBytes > prebuildInfo.UpdateScratchDataSizeInBytes)
        ? prebuildInfo.ScratchDataSizeInBytes
        : prebuildInfo.UpdateScratchDataSizeInBytes;

    // 高速化機構用バッファを生成.
    if (!CreateBufferUAV(
        pDevice,
        prebuildInfo.ResultDataMaxSizeInBytes,
        m_Resource.GetAddress(),
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE))
    {
        ELOG("Error : CreateBufferUAV() Failed.");
        return false;
    }

    // スクラッチバッファ生成.
    RefPtr<ID3D12Resource> scratchBuffer;
    if (!CreateBufferUAV(
        pDevice,
        size,
        scratchBuffer.GetAddress(),
        D3D12_RESOURCE_STATE_COMMON))
    {
        ELOG("Error : CreateBufferUAV() Failed.");
        return false;
    }

    // 高速化機構をビルド.
    {
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc = {};
        desc.Inputs                           = (*pInputs);
        desc.DestAccelerationStructureData    = m_Resource->GetGPUVirtualAddress();
        desc.ScratchAccelerationStructureData = scratchBuffer->GetGPUVirtualAddress();

        pCmd->BuildRaytracingAccelerationStructure(&desc, 0, nullptr);

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type          = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrier.UAV.pResource = m_Resource.GetPtr();
        pCmd->ResourceBarrier(1, &barrier);
    }

    // スクラッチバッファを遅延解放.
    {
        auto resource = scratchBuffer.Detach();
        Dispose(resource);
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void AccelerationStructure::Term()
{
    auto resource = m_Resource.Detach();
    Dispose(resource);
}

//-----------------------------------------------------------------------------
//      GPU仮想アドレスを取得します.
//-----------------------------------------------------------------------------
D3D12_GPU_VIRTUAL_ADDRESS AccelerationStructure::GetGpuAddress() const 
{ return m_Resource->GetGPUVirtualAddress(); }

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* AccelerationStructure::GetResource() const
{ return m_Resource.GetPtr(); }

//-----------------------------------------------------------------------------
//      デバッグ名を設定します.
//-----------------------------------------------------------------------------
void AccelerationStructure::SetName(LPCWSTR name)
{ m_Resource->SetName(name); }


///////////////////////////////////////////////////////////////////////////////
// DxrPipelineState class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
DxrPipelineState::DxrPipelineState()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
DxrPipelineState::~DxrPipelineState()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool DxrPipelineState::Init(const DxrPipelineStateDesc& desc)
{
    auto pDevice = GetD3D12Device();
    assert(pDevice != nullptr);

    auto objCount = 5u * desc.HitGroupCount;
    std::vector<D3D12_STATE_SUBOBJECT> objDescs;
    objDescs.resize(objCount);

    std::vector<D3D12_EXPORT_DESC> exports;
    {
        exports.push_back({desc.RayGeneration.c_str(), nullptr, D3D12_EXPORT_FLAG_NONE});
        for(auto i=0u; i<desc.HitGroupCount; ++i)
        {
            auto& hit = desc.pHitGroups[i];

            if (hit.AnyHitShaderImport)
            { exports.push_back({hit.AnyHitShaderImport, nullptr, D3D12_EXPORT_FLAG_NONE}); }

            if (hit.ClosestHitShaderImport)
            { exports.push_back({hit.ClosestHitShaderImport, nullptr, D3D12_EXPORT_FLAG_NONE}); }

            if (hit.IntersectionShaderImport)
            { exports.push_back({hit.IntersectionShaderImport, nullptr, D3D12_EXPORT_FLAG_NONE}); }
        }

        for(auto i=0u; i<desc.MissTableCount; ++i)
        {
            exports.push_back({desc.pMissTables[i].c_str(), nullptr, D3D12_EXPORT_FLAG_NONE});
        }
    }

    auto index = 0;

    D3D12_GLOBAL_ROOT_SIGNATURE globalRootSignature = {};
    globalRootSignature.pGlobalRootSignature = desc.pRootSignature;

    objDescs[index].Type  = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
    objDescs[index].pDesc = &globalRootSignature;
    index++;

    D3D12_DXIL_LIBRARY_DESC libDesc = {};
    libDesc.DXILLibrary = desc.Shader;
    libDesc.NumExports  = UINT(exports.size());
    libDesc.pExports    = exports.data();

    objDescs[index].Type     = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    objDescs[index].pDesc    = &libDesc;
    index++;

    D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
    shaderConfig.MaxAttributeSizeInBytes = desc.MaxAttributeSize;
    shaderConfig.MaxPayloadSizeInBytes   = desc.MaxPayloadSize;

    objDescs[index].Type     = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
    objDescs[index].pDesc    = &shaderConfig;
    index++;

    D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
    pipelineConfig.MaxTraceRecursionDepth = desc.MaxTraceDepth;

    objDescs[index].Type     = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
    objDescs[index].pDesc    = &pipelineConfig;
    index++;

    for(auto i=0u; i<desc.HitGroupCount; ++i)
    {
        objDescs[index].Type  = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
        objDescs[index].pDesc = &desc.pHitGroups[i];
        index++;
    }

    D3D12_STATE_OBJECT_DESC stateObjectDesc = {};
    stateObjectDesc.Type            = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
    stateObjectDesc.NumSubobjects   = index;
    stateObjectDesc.pSubobjects     = objDescs.data();

    auto hr = pDevice->CreateStateObject(&stateObjectDesc, IID_PPV_ARGS(m_Object.GetAddress()));

    // メモリ解放.
    objDescs.clear();

    if (FAILED(hr))
    {
        ELOG("Error : ID3D12Device5::CreateStateObject() Failed. errcode = 0x%x", hr);
        return false;
    }

    hr = m_Object->QueryInterface(IID_PPV_ARGS(m_Props.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12StateObject::QueryInterface() Failed. errcode = 0x%x", hr);
        return false;
    }

    // レイ生成テーブル.
    {
        std::vector<void*> shaderIdentifers;
        shaderIdentifers.resize(1);
        shaderIdentifers[0] = m_Props->GetShaderIdentifier(desc.RayGeneration.c_str());

        if (!CreateShaderTable(
            pDevice,
            shaderIdentifers,
            m_RayGenTable.GetAddress()))
        {
            ELOG("Error : RayGeneration Table Init Failed.");
            return false;
        }
    }

    // ミステーブル.
    {
        std::vector<void*> shaderIdentifers;
        shaderIdentifers.resize(desc.MissTableCount);
        for(size_t i=0; i<shaderIdentifers.size(); ++i)
        {
            shaderIdentifers[i] = m_Props->GetShaderIdentifier(desc.pMissTables[i].c_str());
        }

        if (!CreateShaderTable(
            pDevice,
            shaderIdentifers,
            m_MissTable.GetAddress()))
        {
            ELOG("Error : Miss Shader Table Init Failed.");
            return false;
        }
    }

    // ヒットグループ.
    {
        std::vector<void*> shaderIdentifers;
        shaderIdentifers.resize(desc.HitGroupCount);
        for(size_t i=0; i<shaderIdentifers.size(); ++i)
        {
            shaderIdentifers[i] = m_Props->GetShaderIdentifier(desc.pHitGroups[i].HitGroupExport);
        }

        if (!CreateShaderTable(
            pDevice,
            shaderIdentifers,
            m_HitGroupTable.GetAddress()))
        {
            ELOG("Error : HitGroup Table Init Failed.");
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void DxrPipelineState::Term()
{
    // ステートオブジェクトを遅延解放.
    {
        auto item = m_Object.Detach();
        Dispose(item);
    }

    // プロパティを遅延解放.
    {
        auto item = m_Props.Detach();
        Dispose(item);
    }

    // レイ生成テーブルを遅延解放.
    {
        auto item = m_RayGenTable.Detach();
        Dispose(item);
    }

    // ミステーブルを遅延解放.
    {
        auto item = m_MissTable.Detach();
        Dispose(item);
    }

    // ヒットグループを遅延解放.
    {
        auto item = m_HitGroupTable.Detach();
        Dispose(item);
    }
}

//-----------------------------------------------------------------------------
//      レイトレーシングパイプラインを起動します.
//-----------------------------------------------------------------------------
void DxrPipelineState::DispatchRays
(
    ID3D12GraphicsCommandList4* pCmd,
    uint32_t                    width,
    uint32_t                    height
)
{
    assert(pCmd != nullptr);
    assert(width  > 0);
    assert(height > 0);

    auto recordSize = asdx::RoundUp(
        uint32_t(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES),
        uint32_t(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT));

    D3D12_DISPATCH_RAYS_DESC desc = {};
    desc.RayGenerationShaderRecord.StartAddress = m_RayGenTable->GetGPUVirtualAddress();
    desc.RayGenerationShaderRecord.SizeInBytes  = m_RayGenTable->GetDesc().Width;

    desc.MissShaderTable.StartAddress   = m_MissTable->GetGPUVirtualAddress();
    desc.MissShaderTable.SizeInBytes    = m_MissTable->GetDesc().Width;
    desc.MissShaderTable.StrideInBytes  = recordSize;

    desc.HitGroupTable.StartAddress     = m_HitGroupTable->GetGPUVirtualAddress();
    desc.HitGroupTable.SizeInBytes      = m_HitGroupTable->GetDesc().Width;
    desc.HitGroupTable.StrideInBytes    = recordSize;

    desc.Width  = width;
    desc.Height = height;
    desc.Depth  = 1;

    pCmd->SetPipelineState1(m_Object.GetPtr());
    pCmd->DispatchRays(&desc);
}

} // namespace asdx

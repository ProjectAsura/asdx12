//-----------------------------------------------------------------------------
// File : asdxDxr.cpp
// Desc : DirectX RayTracing Utility.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes.
//-----------------------------------------------------------------------------
#include <cassert>
#include <fnd/asdxLogger.h>
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

} // namespace asdx

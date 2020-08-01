//-----------------------------------------------------------------------------
// File : asdxStructuredBuffer.cpp
// Desc : Structured Buffer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxStructuredBuffer.h>
#include <asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// StructuredBuffer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
StructuredBuffer::StructuredBuffer()
: m_pResource   ()
, m_pDescriptor ()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
StructuredBuffer::~StructuredBuffer()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool StructuredBuffer::Init(GraphicsDevice& device, uint64_t count, uint32_t stride)
{
    auto pDevice = device.GetDevice();

    if (pDevice == nullptr || count == 0 || stride == 0)
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    uint64_t size = count * stride;

    D3D12_HEAP_PROPERTIES prop = {};
    prop.Type                   = D3D12_HEAP_TYPE_UPLOAD;
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

    auto state = D3D12_RESOURCE_STATE_GENERIC_READ;
    auto flags = D3D12_HEAP_FLAG_NONE;

    auto hr = pDevice->CreateCommittedResource(
        &prop,
        flags,
        &desc,
        state,
        nullptr,
        IID_PPV_ARGS(m_pResource.GetAddress()));
    if ( FAILED(hr) )
    {
        ELOG("Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr);
        return false;
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
    viewDesc.Format                     = DXGI_FORMAT_UNKNOWN;
    viewDesc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
    viewDesc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    viewDesc.Buffer.FirstElement        = 0;
    viewDesc.Buffer.NumElements         = UINT(count);
    viewDesc.Buffer.StructureByteStride = stride;
    viewDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;

    if (!device.AllocHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_pDescriptor.GetAddress()))
    {
        ELOG("Error : GraphicsDevice::AllocHandle() Failed.");
        return false;
    }

    device->CreateShaderResourceView(m_pResource.GetPtr(), &viewDesc, m_pDescriptor->GetHandleCPU());

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void StructuredBuffer::Term()
{
    m_pResource  .Reset();
    m_pDescriptor.Reset();
}

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* StructuredBuffer::GetResource() const
{ return m_pResource.GetPtr(); }

//-----------------------------------------------------------------------------
//      ディスクリプタを取得します.
//-----------------------------------------------------------------------------
const Descriptor* StructuredBuffer::GetDescriptor() const
{ return m_pDescriptor.GetPtr(); }

} // namespace asdx

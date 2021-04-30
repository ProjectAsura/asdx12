//-----------------------------------------------------------------------------
// File : asdxByteAddressBuffer.cpp
// Desc : Byte Address Buffer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxByteAddressBuffer.h>
#include <gfx/asdxGraphicsSystem.h>
#include <core/asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ByteAddressBuffer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ByteAddressBuffer::ByteAddressBuffer()
: m_Resource()
, m_View    ()
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
    desc.Format             = DXGI_FORMAT_R32_TYPELESS;
    desc.MipLevels          = 1;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

    auto flags = D3D12_HEAP_FLAG_NONE;

    auto hr = pDevice->CreateCommittedResource(
        &prop,
        flags,
        &desc,
        state,
        nullptr,
        IID_PPV_ARGS(m_Resource.GetAddress()));
    if ( FAILED(hr) )
    {
        ELOG("Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr);
        return false;
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
    viewDesc.Format                     = DXGI_FORMAT_R32_TYPELESS;
    viewDesc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
    viewDesc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    viewDesc.Buffer.FirstElement        = 0;
    viewDesc.Buffer.NumElements         = UINT(size / 4);
    viewDesc.Buffer.StructureByteStride = 0;
    viewDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_RAW;

    if (!CreateShaderResourceView(m_Resource.GetPtr(), &viewDesc, m_View.GetAddress()))
    {
        ELOGA("Error : CreateShaderResourceView() Failed.");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool ByteAddressBuffer::Init(uint64_t size, const void* pInitData)
{
    if (!Init(size, D3D12_RESOURCE_STATE_GENERIC_READ))
    { return false; }

    if (!GfxSystem().UpdateBuffer(m_Resource.GetPtr(), pInitData))
    {
        ELOGA("Error : GraphicsDevice::UpdateBuffer() Failed.");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void ByteAddressBuffer::Term()
{
    auto resource = m_Resource.Detach();
    if (resource != nullptr)
    { GfxSystem().Dispose(resource); }
    m_View.Reset();
}

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* ByteAddressBuffer::GetResource() const
{ return m_Resource.GetPtr(); }

//-----------------------------------------------------------------------------
//      シェーダリソースビューを取得します.
//-----------------------------------------------------------------------------
IShaderResourceView* ByteAddressBuffer::GetView() const
{ return m_View.GetPtr(); }

} // namespace
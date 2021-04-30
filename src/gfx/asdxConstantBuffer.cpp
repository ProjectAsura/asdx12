//-----------------------------------------------------------------------------
// File : asdxConstantBuffer.cpp
// Desc : Constant Buffer Wrapper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxConstantBuffer.h>
#include <gfx/asdxGraphicsSystem.h>
#include <core/asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ConstantBuffer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ConstantBuffer::ConstantBuffer()
: m_Index(0)
{
    m_Dst[0] = nullptr;
    m_Dst[1] = nullptr;
    m_View[0] = nullptr;
    m_View[1] = nullptr;
    m_Resource[0] = nullptr;
    m_Resource[1] = nullptr;
}

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

    D3D12_HEAP_PROPERTIES props = {
        D3D12_HEAP_TYPE_UPLOAD,
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

    for(auto i=0; i<2; ++i)
    {
        auto hr = pDevice->CreateCommittedResource(
            &props,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(m_Resource[i].GetAddress()));
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateCommittedResource() Failed. errcode = 0x%x", hr );
            return false;
        }

        m_Resource[i]->SetName(L"asdxConstantBuffer");

        hr = m_Resource[i]->Map( 0, nullptr, reinterpret_cast<void**>( &m_Dst[i] ) );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Resource::Map() Failed. errcode = 0x%x", hr );
            return false;
        }

        D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc = {};
        viewDesc.SizeInBytes    = static_cast<uint32_t>(size);
        viewDesc.BufferLocation = m_Resource[i]->GetGPUVirtualAddress();

        if (!CreateConstantBufferView(m_Resource[i].GetPtr(), &viewDesc, m_View[i].GetAddress()))
        {
            ELOGA("Erorr : CreateConstantBufferView() Failed.");
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
    for(auto i=0; i<2; ++i)
    {
        m_View[i].Reset();
        m_Resource[i].Reset();
        m_Dst[i] = nullptr;
    }

    m_Size  = 0;
    m_Index = 0;
}

//-----------------------------------------------------------------------------
//      更新処理を行います.
//-----------------------------------------------------------------------------
void ConstantBuffer::Update(const void* pSrc, uint64_t size, uint64_t srcOffset, uint64_t dstOffset)
{
    auto dst = m_Dst[m_Index] + dstOffset;
    auto src = reinterpret_cast<const uint8_t*>(pSrc) + srcOffset;
    auto copy_size = (size > m_Size) ? m_Size : size;

    memcpy(dst, src, copy_size);
}

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* ConstantBuffer::GetResource() const
{ return m_Resource[m_Index].GetPtr(); }

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D12Resource* ConstantBuffer::GetResource(uint32_t index) const
{
    assert(index < 2);
    return m_Resource[index].GetPtr();
}

//-----------------------------------------------------------------------------
//      定数バッファビューを取得します.
//-----------------------------------------------------------------------------
IConstantBufferView* ConstantBuffer::GetView() const
{ return m_View[m_Index].GetPtr(); }

//-----------------------------------------------------------------------------
//      定数バッファビューを取得します.
//-----------------------------------------------------------------------------
IConstantBufferView* ConstantBuffer::GetView(uint32_t index) const
{
    assert(index < 2);
    return m_View[index].GetPtr();;
}

//-----------------------------------------------------------------------------
//      バッファを入れ替えます.
//-----------------------------------------------------------------------------
void ConstantBuffer::SwapBuffer()
{ m_Index = (m_Index + 1) & 0x1; }

//-----------------------------------------------------------------------------
//      メモリマッピングを行います.
//-----------------------------------------------------------------------------
void* ConstantBuffer::Map(uint32_t index)
{
    assert(index < 2);
    void* pData = nullptr;
    auto hr = m_Resource[index]->Map(0, nullptr, &pData);
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
void ConstantBuffer::Unmap(uint32_t index)
{
    assert(index < 2);
    m_Resource[index]->Unmap(0, nullptr);
}

} // namespace asdx

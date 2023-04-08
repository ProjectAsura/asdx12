//-----------------------------------------------------------------------------
// File : asdxCommandList.cpp
// Desc : Command List Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <vector>
#include <gfx/asdxCommandList.h>
#include <gfx/asdxDevice.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxMisc.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// GraphicsCommandList class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
CommandList::CommandList()
: m_Allocator()
, m_CmdList  ()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
CommandList::~CommandList()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool CommandList::Init(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type, uint32_t blockCount)
{
    // 引数チェック.
    if (pDevice == nullptr)
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    for(auto i=0; i<2; ++i)
    {
        // コマンドアロケータを生成.
        auto hr = pDevice->CreateCommandAllocator( type, IID_PPV_ARGS( m_Allocator[i].GetAddress() ) );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Device::CreateCommandAllocator() Failed. errcode = 0x%x", hr );
            return false;
        }
    }

    // コマンドリストを生成.
    auto hr = pDevice->CreateCommandList(
        0,
        type,
        m_Allocator[0].GetPtr(),
        nullptr,
        IID_PPV_ARGS( m_CmdList.GetAddress() ) );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D12Device::CreateCommandList() Failed. errcode = 0x%x", hr );
        return false;
    }

    // 生成直後は開きっぱなしの扱いになっているので閉じておく.
    m_CmdList->Close();

    m_Index = 0;

    // バッファリソース生成.
    if (blockCount > 0)
    {
        D3D12_HEAP_PROPERTIES props = {
            D3D12_HEAP_TYPE_UPLOAD,
            D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            D3D12_MEMORY_POOL_UNKNOWN,
            1,
            1
        };

        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Width              = UINT64(256 * blockCount);
        desc.Height             = 1;
        desc.DepthOrArraySize   = 1;
        desc.MipLevels          = 1;
        desc.Format             = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc         = { 1, 0 };
        desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

        for(auto i=0; i<2; ++i)
        {
            auto hr = pDevice->CreateCommittedResource(
                &props,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                IID_PPV_ARGS(m_Buffer[i].GetAddress()));
            if (FAILED(hr))
            {
                ELOG("Error : ID3D12Device::CreateCommittedReosurce() Failed. errcode = 0x%x", hr);
                return false;
            }

            hr = m_Buffer[i]->Map(0, nullptr, reinterpret_cast<void**>(&m_AddressCPU[i]));
            if (FAILED(hr))
            {
                ELOG("Error : ID3D12Resource::Map() Failed. errcode = 0x%x", hr);
                return false;
            }

            m_AddressGPU[i] = m_Buffer[i]->GetGPUVirtualAddress();
        }
    }

    m_BlockIndex    = 0;
    m_MaxBlockCount = blockCount;

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void CommandList::Term()
{
    m_CmdList.Reset();

    for(auto i=0; i<2; ++i)
    {
        m_Allocator[i].Reset();
    
        if (m_Buffer[i] != nullptr)
        {
            m_Buffer[i].Reset();

            m_AddressCPU[i] = nullptr;
            m_AddressGPU[i] = 0;
        }
    }

    m_BlockIndex    = 0;
    m_MaxBlockCount = 0;
}

//-----------------------------------------------------------------------------
//      コマンドリストをリセットします.
//-----------------------------------------------------------------------------
void CommandList::Reset()
{
    // ダブルバッファリング.
    m_Index = (m_Index + 1) & 0x1;

    // コマンドアロケータをリセット.
    m_Allocator[m_Index]->Reset();

    // コマンドリストをリセット.
    m_CmdList->Reset( m_Allocator[m_Index].GetPtr(), nullptr );

    // ディスクリプターヒープを設定しおく.
    SetDescriptorHeaps(m_CmdList.GetPtr());

    // ブロック先頭に戻す.
    m_BlockIndex = 0;
}

//-----------------------------------------------------------------------------
//      コマンドリストアロケータを取得します.
//-----------------------------------------------------------------------------
ID3D12CommandAllocator* CommandList::GetAllocator(uint8_t index) const
{
    assert(index < 2);
    return m_Allocator[index].GetPtr();
}

//-----------------------------------------------------------------------------
//      グラフィックスコマンドリストを取得します.
//-----------------------------------------------------------------------------
ID3D12GraphicsCommandList6* CommandList::GetCommandList() const
{ return m_CmdList.GetPtr(); }

//-----------------------------------------------------------------------------
//      現在のバッファ番号を返却します.
//-----------------------------------------------------------------------------
uint8_t CommandList::GetIndex() const
{ return m_Index; }

//-----------------------------------------------------------------------------
//      バッファを確保します.
//-----------------------------------------------------------------------------
BufferAddress CommandList::AllocBuffer(uint32_t size)
{
    BufferAddress result = {};

    uint32_t blockCount = RoundUp(size, 256) / 256;
    if (blockCount == 0 || (m_BlockIndex + blockCount) >= m_MaxBlockCount)
    { return result; }

    auto index = m_BlockIndex;
    m_BlockIndex += blockCount;
    result.AddressCPU = m_AddressCPU[m_Index] + 256u * index;
    result.AddressGPU = m_AddressGPU[m_Index] + 256llu * index;
    return result;
}


///////////////////////////////////////////////////////////////////////////////
// ScopedMarker class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ScopedMarker::ScopedMarker(ID3D12GraphicsCommandList* pCmd, const char* text)
: m_pCmd(pCmd)
{
    assert(text != nullptr);
    static const UINT PIX_EVENT_ANSI_VERSION = 1;
    auto size = UINT((strlen(text) + 1) * sizeof(char));
    m_pCmd->BeginEvent(PIX_EVENT_ANSI_VERSION, text, size);
}

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ScopedMarker::~ScopedMarker()
{ m_pCmd->EndEvent(); }


} // namespace asdx

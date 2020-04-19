//-----------------------------------------------------------------------------
// File : asdxCommandList.cpp
// Desc : Command List Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxCommandList.h>
#include <asdxLogger.h>


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
bool CommandList::Init(GraphicsDevice& device, D3D12_COMMAND_LIST_TYPE type)
{
    // 引数チェック.
    if ( device.GetDevice() == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    // コマンドアロケータを生成.
    auto hr = device->CreateCommandAllocator( type, IID_PPV_ARGS( m_Allocator.GetAddress() ) );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D12Device::CreateCommandAllocator() Failed." );
        return false;
    }

    // コマンドリストを生成.
    hr = device->CreateCommandList(
        0,
        type,
        m_Allocator.GetPtr(),
        nullptr,
        IID_PPV_ARGS( m_CmdList.GetAddress() ) );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D12Device::CreateCommandList() Failed." );
        return false;
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void CommandList::Term()
{
    m_CmdList  .Reset();
    m_Allocator.Reset();
}

//-----------------------------------------------------------------------------
//      コマンドリストをクリアします.
//-----------------------------------------------------------------------------
void CommandList::Clear()
{
    m_Allocator->Reset();
    m_CmdList->Reset( m_Allocator.GetPtr(), nullptr );

    GfxDevice().SetDescriptorHeaps(m_CmdList.GetPtr());
}

//-----------------------------------------------------------------------------
//      コマンドリストアロケータを取得します.
//-----------------------------------------------------------------------------
ID3D12CommandAllocator* CommandList::GetAllocator() const
{ return m_Allocator.GetPtr(); }

//-----------------------------------------------------------------------------
//      グラフィックスコマンドリストを取得します.
//-----------------------------------------------------------------------------
ID3D12GraphicsCommandList* CommandList::GetCommandList() const
{ return m_CmdList.GetPtr(); }

//-----------------------------------------------------------------------------
//      コマンドリストにキャストします.
//-----------------------------------------------------------------------------
ID3D12CommandList* CommandList::Cast() const
{ return static_cast<ID3D12CommandList*>(m_CmdList.GetPtr()); }

//-----------------------------------------------------------------------------
//      アロー演算子です.
//-----------------------------------------------------------------------------
ID3D12GraphicsCommandList* CommandList::operator-> () const
{ return m_CmdList.GetPtr(); }

} // namespace asdx

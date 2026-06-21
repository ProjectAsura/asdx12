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
, m_Index    (0)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
CommandList::~CommandList()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool CommandList::Init(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type)
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
    { m_Allocator[i].Reset(); }
}

//-----------------------------------------------------------------------------
//      コマンドリストをリセットします.
//-----------------------------------------------------------------------------
ID3D12GraphicsCommandList6* CommandList::Reset()
{
    // ダブルバッファリング.
    m_Index = (m_Index + 1) & 0x1;

    // コマンドアロケータをリセット.
    m_Allocator[m_Index]->Reset();

    // コマンドリストをリセット.
    m_CmdList->Reset( m_Allocator[m_Index].GetPtr(), nullptr );

    // ディスクリプターヒープを設定しおく.
    SetDescriptorHeaps(m_CmdList.GetPtr());

    return m_CmdList.GetPtr();
}

//-----------------------------------------------------------------------------
//      コマンドリストアロケータを取得します.
//-----------------------------------------------------------------------------
ID3D12CommandAllocator* CommandList::GetD3D12CommandAllocator(uint8_t index) const
{
    assert(index < 2);
    return m_Allocator[index].GetPtr();
}

//-----------------------------------------------------------------------------
//      グラフィックスコマンドリストを取得します.
//-----------------------------------------------------------------------------
ID3D12GraphicsCommandList6* CommandList::GetD3D12CommandList() const
{ return m_CmdList.GetPtr(); }

//-----------------------------------------------------------------------------
//      現在のバッファ番号を返却します.
//-----------------------------------------------------------------------------
uint8_t CommandList::GetIndex() const
{ return m_Index; }

//-----------------------------------------------------------------------------
//      デバッグ名を設定します.
//-----------------------------------------------------------------------------
void CommandList::SetName(LPCWSTR name)
{ m_CmdList->SetName(name); }


///////////////////////////////////////////////////////////////////////////////
// Functions.
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      UAVバリアを設定します.
//-----------------------------------------------------------------------------
void SetUAVBarrier(D3D12_RESOURCE_BARRIER& barrier, ID3D12Resource* pResource)
{
    assert(pResource != nullptr);
    barrier.Type          = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.Flags         = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.UAV.pResource = pResource;
}

//-----------------------------------------------------------------------------
//      UAVバリアを発行します.
//-----------------------------------------------------------------------------
void UAVBarrier(ID3D12GraphicsCommandList* pCmd, ID3D12Resource* pResource)
{
    assert(pCmd != nullptr);
    D3D12_RESOURCE_BARRIER barrier = {};
    SetUAVBarrier(barrier, pResource);
    pCmd->ResourceBarrier(1, &barrier);
}

//-----------------------------------------------------------------------------
//      遷移バリアを設定します.
//-----------------------------------------------------------------------------
void SetTransitionBarrier
(
    D3D12_RESOURCE_BARRIER& barrier,
    ID3D12Resource*         pResource,
    D3D12_RESOURCE_STATES   before,
    D3D12_RESOURCE_STATES   after
)
{
    assert(pResource != nullptr);
    if (before == after)
        return;

    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource   = pResource;
    barrier.Transition.StateBefore = before;
    barrier.Transition.StateAfter  = after;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
}

//-----------------------------------------------------------------------------
//      遷移バリアを発行します.
//-----------------------------------------------------------------------------
void TransitionBarrier
(
    ID3D12GraphicsCommandList*  pCmd,
    ID3D12Resource*             pResource,
    D3D12_RESOURCE_STATES       before,
    D3D12_RESOURCE_STATES       after
)
{
    assert(pCmd != nullptr);
    if (before == after)
        return;

    D3D12_RESOURCE_BARRIER barrier = {};
    SetTransitionBarrier(barrier, pResource, before, after);
    pCmd->ResourceBarrier(1, &barrier);
}

//-----------------------------------------------------------------------------
//      エイリアシングを設定します.
//-----------------------------------------------------------------------------
void SetAliasingBarrier
(
    D3D12_RESOURCE_BARRIER& barrier,
    ID3D12Resource*         pBefore,
    ID3D12Resource*         pAfter
)
{
    assert(pBefore != nullptr);
    assert(pAfter  != nullptr);
    barrier.Type                     = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
    barrier.Flags                    = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Aliasing.pResourceBefore = pBefore;
    barrier.Aliasing.pResourceAfter  = pAfter;
}

//-----------------------------------------------------------------------------
//      エイリアシングバリアを発行します.
//-----------------------------------------------------------------------------
void AliasingBarrier
(
    ID3D12GraphicsCommandList*  pCmd,
    ID3D12Resource*             pBefore,
    ID3D12Resource*             pAfter
)
{
    assert(pCmd != nullptr);
    D3D12_RESOURCE_BARRIER barrier = {};
    SetAliasingBarrier(barrier, pBefore, pAfter);
    pCmd->ResourceBarrier(1, &barrier);
}

} // namespace asdx

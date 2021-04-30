﻿//-----------------------------------------------------------------------------
// File : asdxCommandQueue.cpp
// Desc : Command Queue Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxCommandQueue.h>
#include <core/asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// CommandQueue class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
CommandQueue::CommandQueue()
: m_Fence       ()
, m_Queue       ()
, m_Counter     (1)
, m_FenceValue  (0)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
CommandQueue::~CommandQueue()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理です.
//-----------------------------------------------------------------------------
bool CommandQueue::Init(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type)
{
    if ( pDevice == nullptr )
    {
        ELOG( "Error : Invalid Arugment." );
        return false;
    }

    D3D12_COMMAND_QUEUE_DESC desc = {
        type,
        0,
        D3D12_COMMAND_QUEUE_FLAG_NONE
    };

    auto hr = pDevice->CreateCommandQueue( &desc, IID_PPV_ARGS(m_Queue.GetAddress()) );
    if ( FAILED(hr) )
    {
        ELOG( "Error : ID3D12Device::CreateCommandQueue() Failed. errcodes = 0x%x", hr );
        return false;
    }

    m_Queue->SetName(L"asdxQueue");

    if ( !m_Fence.Init(pDevice) )
    {
        ELOG( "Error : Fence::Init() Failed." );
        return false;
    }

    m_IsExecuted = false;
    m_FenceValue = 1;

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void CommandQueue::Term()
{
    m_Queue.Reset();
    m_Fence.Term();
    m_IsExecuted = false;
    m_FenceValue = 0;
}

//-----------------------------------------------------------------------------
//      参照カウントを増やします.
//-----------------------------------------------------------------------------
void CommandQueue::AddRef()
{ m_Counter++; }

//-----------------------------------------------------------------------------
//      解放処理を行います.
//-----------------------------------------------------------------------------
void CommandQueue::Release()
{
    m_Counter--;
    if (m_Counter == 0)
    { delete this; }
}

//-----------------------------------------------------------------------------
//      参照カウントを取得します.
//-----------------------------------------------------------------------------
uint32_t CommandQueue::GetCount() const
{ return m_Counter; }

//-----------------------------------------------------------------------------
//      コマンドを実行します.
//-----------------------------------------------------------------------------
void CommandQueue::Execute(uint32_t count, ID3D12CommandList** ppList)
{
    if(count == 0 || ppList == nullptr)
    { return; }

    m_Queue->ExecuteCommandLists(count, ppList);
    m_IsExecuted = true;
}

//-----------------------------------------------------------------------------
//      フェンスの値を更新します.
//-----------------------------------------------------------------------------
WaitPoint CommandQueue::Signal()
{
    WaitPoint result;

    const auto fence = m_FenceValue;
    auto hr = m_Queue->Signal(m_Fence.GetPtr(), fence);
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12CommandQueue::Signal() Failed.");
        return result;
    }
    m_FenceValue++;

    result.m_FenceValue = fence;
    result.m_pFence     = m_Fence.GetPtr();

    return result;
}

//-----------------------------------------------------------------------------
//      GPU上での待機点を設定します.
//-----------------------------------------------------------------------------
bool CommandQueue::Wait(const WaitPoint& value)
{
    auto hr = m_Queue->Wait(value.m_pFence, value.m_FenceValue);
    if (FAILED(hr))
    {
        ELOG("Error : ID3D12CommandQueue::Wait() Failed.");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      CPU上でコマンドの完了を待機します.
//-----------------------------------------------------------------------------
void CommandQueue::Sync(const WaitPoint& value, uint32_t msec)
{
    if (!m_IsExecuted)
    { return; }

    m_Fence.Wait(value.m_FenceValue, msec);
}

//-----------------------------------------------------------------------------
//      コマンドキューを取得します.
//-----------------------------------------------------------------------------
ID3D12CommandQueue* CommandQueue::GetQueue() const
{ return m_Queue.GetPtr(); }

//-----------------------------------------------------------------------------
//      生成処理を行います.
//-----------------------------------------------------------------------------
bool CommandQueue::Create
(
    ID3D12Device*           pDevice,
    D3D12_COMMAND_LIST_TYPE type,
    CommandQueue**          ppResult
)
{
    if ( pDevice == nullptr )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    auto queue = new (std::nothrow) CommandQueue();
    if (queue == nullptr)
    {
        ELOG( "Error : Ouf of Memory." );
        return false;
    }

    if (!queue->Init(pDevice, type))
    {
        queue->Release();
        ELOG( "Error : Queue::Init() Failed." );
        return false;
    }

    *ppResult = queue;

    return true;
}

} // namespace asdx

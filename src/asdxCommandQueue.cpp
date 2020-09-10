//-----------------------------------------------------------------------------
// File : asdxCommandQueue.cpp
// Desc : Command Queue Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxCommandQueue.h>
#include <asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// CommandQueue class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
CommandQueue::CommandQueue()
: m_Fence   ()
, m_Queue   ()
, m_Counter (1)
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
//      コマンドの完了を待機します.
//-----------------------------------------------------------------------------
void CommandQueue::Wait(uint32_t msec)
{
    if (m_IsExecuted)
    {
        m_Fence.SignalAndWait(m_Queue.GetPtr(), msec);
        m_IsExecuted = false;
    }
}

//-----------------------------------------------------------------------------
//      コマンドの完了を待機します.
//-----------------------------------------------------------------------------
void CommandQueue::WaitIdle()
{ Wait(kInfinite); }

//-----------------------------------------------------------------------------
//      コマンドキューを取得します.
//-----------------------------------------------------------------------------
ID3D12CommandQueue* CommandQueue::GetQueue() const
{ return m_Queue.GetPtr(); }

//-----------------------------------------------------------------------------
//      フェンスを取得します.
//-----------------------------------------------------------------------------
ID3D12Fence* CommandQueue::GetFence() const
{ return m_Fence.GetPtr(); }

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

//-----------------------------------------------------------------------------
// File : asdxFence.cpp
// Desc : Fence Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxFence.h>
#include <asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Fence class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Fence::Fence()
: m_Fence  ()
, m_Handle ( nullptr )
, m_Counter( 0 )
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Fence::~Fence()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Fence::Init( ID3D12Device* pDevice )
{
    // 引数チェック.
    if ( pDevice == nullptr )
    {
        ELOG( "Error : Invalid Error." );
        return false;
    }

    // イベントを生成.
    m_Handle = CreateEventEx( nullptr, FALSE, FALSE, EVENT_ALL_ACCESS );
    if ( m_Handle == nullptr )
    {
        ELOG( "Error : CreateEventW() Failed." );
        return false;
    }

    // フェンスを生成.
    auto hr = pDevice->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( m_Fence.GetAddress()) );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D12Device::CreateFence() Failed. errcode = 0x%x", hr );
        return false;
    }

    m_Fence->SetName(L"asdxFence");

    // カウンタ設定.
    m_Counter = 1;

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Fence::Term()
{
    // ハンドルを閉じる.
    if ( m_Handle != nullptr )
    {
        CloseHandle( m_Handle );
        m_Handle = nullptr;
    }

    // フェンスオブジェクトを破棄.
    m_Fence.Reset();

    // カウンターリセット.
    m_Counter = 1;
}

//-----------------------------------------------------------------------------
//      フェンスに値を設定します.
//-----------------------------------------------------------------------------
UINT64 Fence::Signal(ID3D12CommandQueue* pQueue)
{
    const auto fence = m_Counter;
    auto hr = pQueue->Signal( m_Fence.GetPtr(), fence );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D12CommandQueue::Signal() Failed." );
        return 0;
    }
    m_Counter++;
    return fence;
}

//-----------------------------------------------------------------------------
//      フェンスが指定された値に達するまで待機します.
//-----------------------------------------------------------------------------
void Fence::Wait(UINT64 fenceValue, uint32_t msec)
{
    if ( m_Fence->GetCompletedValue() < fenceValue )
    {
        auto hr = m_Fence->SetEventOnCompletion( fenceValue, m_Handle );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Fence::SetEventOnCompletation() Failed." );
            return;
        }

        WaitForSingleObject( m_Handle, msec );
    }
}

//-----------------------------------------------------------------------------
//      コマンドの完了を待機します.
//-----------------------------------------------------------------------------
void Fence::SignalAndWait( ID3D12CommandQueue* pQueue, uint32_t msec )
{
    auto fence = Signal(pQueue);
    Wait(fence, msec);
}

//-----------------------------------------------------------------------------
//      フェンスを取得します.
//-----------------------------------------------------------------------------
ID3D12Fence* Fence::GetPtr() const
{ return m_Fence.GetPtr(); }

} // namespace asdx

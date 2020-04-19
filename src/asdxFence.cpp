﻿//-----------------------------------------------------------------------------
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
        ELOG( "Error : ID3D12Device::CreateFence() Failed." );
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
    m_Counter = 0;
}

//-----------------------------------------------------------------------------
//      コマンドの完了を待機します.
//-----------------------------------------------------------------------------
void Fence::Wait( ID3D12CommandQueue* pQueue, uint32_t mesc )
{
    const auto fence = m_Counter;
    auto hr = pQueue->Signal( m_Fence.GetPtr(), fence );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID3D12CommandQueue::Signal() Failed." );
        return;
    }
    m_Counter++;

    if ( m_Fence->GetCompletedValue() < fence )
    {
        hr = m_Fence->SetEventOnCompletion( fence, m_Handle );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D12Fence::SetEventOnCompletation() Failed." );
            return;
        }

        WaitForSingleObject( m_Handle, mesc );
    }
}

//-----------------------------------------------------------------------------
//      フェンスを取得します.
//-----------------------------------------------------------------------------
ID3D12Fence* Fence::GetPtr() const
{ return m_Fence.GetPtr(); }

} // namespace asdx

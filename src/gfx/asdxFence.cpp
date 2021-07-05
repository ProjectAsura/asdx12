﻿//-----------------------------------------------------------------------------
// File : asdxFence.cpp
// Desc : Fence Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxFence.h>
#include <fnd/asdxLogger.h>


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
//      フェンスを取得します.
//-----------------------------------------------------------------------------
ID3D12Fence* Fence::GetPtr() const
{ return m_Fence.GetPtr(); }


///////////////////////////////////////////////////////////////////////////////
// WaitPoint class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
WaitPoint::WaitPoint()
: m_FenceValue  (0)
, m_pFence      (nullptr)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
WaitPoint::~WaitPoint()
{
    m_FenceValue    = 0;
    m_pFence        = nullptr;
}

//-----------------------------------------------------------------------------
//      代入演算子です.
//-----------------------------------------------------------------------------
WaitPoint& WaitPoint::operator=(const WaitPoint& value)
{
    m_FenceValue = value.m_FenceValue;
    m_pFence     = value.m_pFence;
    return *this;
}

//-----------------------------------------------------------------------------
//      有効かどうかチェックします.
//-----------------------------------------------------------------------------
bool WaitPoint::IsValid() const
{ return (m_FenceValue >= 1) && (m_pFence != nullptr); }

} // namespace asdx
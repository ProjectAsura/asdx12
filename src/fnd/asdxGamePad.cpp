//-----------------------------------------------------------------------------
// File : asdxGamePad.cpp
// Desc : Game Pad Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <Windows.h>
#include <Xinput.h>
#include <fnd/asdxHid.h>
#include <fnd/asdxMath.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// GamePad class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
GamePad::GamePad( )
: m_PlayerIndex( 0 )
{ ResetState(); }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
GamePad::~GamePad()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      ボタンの値をリセットします.
//-----------------------------------------------------------------------------
void GamePad::ResetState()
{
    m_Buttons  = 0;
    m_TriggerL = 0.0f;
    m_TriggerR = 0.0f;

    m_ThumbLX = 0.0f;
    m_ThumbLY = 0.0f;
    m_ThumbRX = 0.0f;
    m_ThumbRY = 0.0f;

    m_PressedButtons = 0;
    m_LastButtons    = 0;
}

//-----------------------------------------------------------------------------
//      コントローラーが接続されているかチェックします.
//-----------------------------------------------------------------------------
bool GamePad::IsConnected() const
{ return m_IsConnected; }

//-----------------------------------------------------------------------------
//      コントローラーの状態を更新します.
//-----------------------------------------------------------------------------
void GamePad::UpdateState()
{
    XINPUT_STATE state = {};

    const auto result = XInputGetState( m_PlayerIndex, &state );
    if ( result == ERROR_SUCCESS )
    {
        m_IsConnected = true;
    }
    else
    {
        m_IsConnected = false;
        ResetState();
        return;
    }

    // XINPUT_STATEのデータを設定.
    m_Buttons  = state.Gamepad.wButtons;
    m_TriggerL = float(state.Gamepad.bLeftTrigger)  / 255.0f;
    m_TriggerR = float(state.Gamepad.bRightTrigger) / 255.0f;
    auto thumbLX  = state.Gamepad.sThumbLX;
    auto thumbLY  = state.Gamepad.sThumbLY;
    auto thumbRX  = state.Gamepad.sThumbRX;
    auto thumbRY  = state.Gamepad.sThumbRY;


    if ( thumbLX <  XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
         thumbLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE )
    { thumbLX = 0; }

    if ( thumbLY <  XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
         thumbLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE )
    { thumbLY = 0; }

    if ( thumbRX <  XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
         thumbRX > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE )
    { thumbRX = 0; }

    if ( thumbRY <  XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
         thumbRY > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE )
    { thumbRY = 0; }

    // 正規化.
    m_ThumbLX = Max<float>( -1.0f, float(thumbLX) / 32767.0f );
    m_ThumbLY = Max<float>( -1.0f, float(thumbLY) / 32767.0f );
    m_ThumbRX = Max<float>( -1.0f, float(thumbRX) / 32767.0f );
    m_ThumbRY = Max<float>( -1.0f, float(thumbRY) / 32767.0f );

    if ( state.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD )
    { m_Buttons |= PAD_TRIGGER_L; }

    if ( state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD )
    { m_Buttons |= PAD_TRIGGER_R; }

    m_PressedButtons = ( m_LastButtons ^ m_Buttons ) & m_Buttons;
    m_LastButtons    = m_Buttons;
}

//-----------------------------------------------------------------------------
//      バイブレーションさせます.
//-----------------------------------------------------------------------------
void GamePad::Vibrate(float lhs, float rhs) const
{
    XINPUT_VIBRATION vibrate = {};

    vibrate.wLeftMotorSpeed  = static_cast<WORD>(lhs * 65535.0f);
    vibrate.wRightMotorSpeed = static_cast<WORD>(rhs * 65535.0f);

    XInputSetState( m_PlayerIndex, &vibrate );
}

//-----------------------------------------------------------------------------
//      プレイヤーインデックスを設定します.
//-----------------------------------------------------------------------------
void GamePad::SetPlayerIndex(uint32_t index)
{
    assert( index < 4 );
    m_PlayerIndex = index;
}

//-----------------------------------------------------------------------------
//      プレイヤーインデックスを取得します.
//-----------------------------------------------------------------------------
uint32_t GamePad::GetPlayerIndex() const
{ return m_PlayerIndex; }

//-----------------------------------------------------------------------------
//      ボタンが押されたかチェックします.
//-----------------------------------------------------------------------------
bool GamePad::IsDown(uint32_t buttonMask) const
{ return !!(m_PressedButtons & buttonMask); }

//-----------------------------------------------------------------------------
//      ボタンが押されているかチェックします.
//-----------------------------------------------------------------------------
bool GamePad::IsHold(uint32_t buttonMask) const
{ return !!(m_Buttons & buttonMask); }

//-----------------------------------------------------------------------------
//      正規化された左サムスティックのX成分を取得します.
//-----------------------------------------------------------------------------
float GamePad::GetThumbLX() const
{ return m_ThumbLX; }

//-----------------------------------------------------------------------------
//      正規化された左サムスティックのY成分を取得します.
//-----------------------------------------------------------------------------
float GamePad::GetThumbLY() const
{ return m_ThumbLY; }

//-----------------------------------------------------------------------------
//      正規化された右サムスティックのX成分を取得します.
//-----------------------------------------------------------------------------
float GamePad::GetThumbRX() const
{ return m_ThumbRX; }

//-----------------------------------------------------------------------------
//      正規化された右サムスティックのY成分を取得します.
//-----------------------------------------------------------------------------
float GamePad::GetThumbRY() const
{ return m_ThumbRY; }

//-----------------------------------------------------------------------------
//      左トリガーの値を取得します.
//-----------------------------------------------------------------------------
float GamePad::GetTriggerL() const
{ return m_TriggerL; }

//-----------------------------------------------------------------------------
//      右トリガーの値を取得します.
//-----------------------------------------------------------------------------
float GamePad::GetTriggerR() const
{ return m_TriggerR; }

} // namespace asdx

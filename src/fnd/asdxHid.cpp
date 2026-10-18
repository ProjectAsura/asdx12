//-----------------------------------------------------------------------------
// File : asdxHid.cpp
// Desc : Human Input Device.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <Windows.h>
#include <Xinput.h>
#include <GameInput.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxHid.h>
#include <fnd/asdxRef.h>
#include <fnd/asdxMath.h>
#include <fnd/asdxSpinLock.h>


#ifndef GAMEINPUT_API_VERSION
#define GAMEINPUT_API_VERSION 0
#endif

#if GAMEINPUT_API_VERSION == 1
using namespace GameInput::v1;
#elif GAMEINPUT_API_VERSION == 2
using namespace GameInput::v2;
#elif GAMEINPUT_API_VERSION == 3
using namespace GameInput::v3;
#elif GAMEINPUT_API_VERSION == 4
using namespace GameInput::v4;
#endif

namespace asdx {

namespace {
bool IsAssigned(IGameInputDevice* pDevice, uint32_t index);
} // namespace

///////////////////////////////////////////////////////////////////////////////
// Keyboard class
///////////////////////////////////////////////////////////////////////////////
class Keyboard final : public IKeyboard
{
    //=========================================================================
    // friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    static const uint32_t MAX_KEYS = 256;        //!< キーの最大数です.

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    Keyboard()
    : m_Index(0)
    { ResetState(); }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~Keyboard() = default;

    //-------------------------------------------------------------------------
    //! @brief      キーの状態を更新します.
    //-------------------------------------------------------------------------
    void UpdateState()
    {
        m_Index = 1 - m_Index;

        uint8_t keys[MAX_KEYS] = {};
        if (GetKeyboardState(keys) == TRUE)
        {
            for(auto i=0u; i<MAX_KEYS; ++i )
            { m_Keys[m_Index][i] = !!(keys[i] & 0x80); }
        }
        else
        {
            memset(m_Keys[m_Index], 0, sizeof(bool) * MAX_KEYS);
        }
    }

    //-------------------------------------------------------------------------
    //! @brief      キーが押されっぱなしかどうかチェックします.
    //!
    //! @param[in]      keyCode     チェックするキーコード.
    //! @return     キーが押された場合はtrueを返します.
    //! @memo       現在のフレームでキーが押されている場合にtrueを返します.
    //-------------------------------------------------------------------------
    bool IsHold(uint32_t keyCode) const override
    {
        assert(keyCode < MAX_KEYS);
        uint32_t idx = m_Index;
        uint32_t key = ConvertKey(keyCode);
        return m_Keys[idx][key];
    }

    //-------------------------------------------------------------------------
    //! @brief      キーが押されたかどうかチェックします.
    //!
    //! @param[in]      keyCode     チェックするキーコード.
    //! @return     キーが押されている場合はtrueを返します.
    //! @memo       前のフレームのキー状態と現在のフレームのキー状態が異なる場合にtrueを返します.
    //!             フラグの切り替え用途などに使用します.
    //-------------------------------------------------------------------------
    bool IsDown(uint32_t keyCode) const override
    {
        assert(keyCode < MAX_KEYS);
        uint32_t idx = m_Index;
        uint32_t key = ConvertKey( keyCode );
        return m_Keys[idx][key] && (!m_Keys[1 - idx][key]);
    }

    //-------------------------------------------------------------------------
    //! @brief      ボタンの値をリセットします.
    //-------------------------------------------------------------------------
    void ResetState()
    { memset(m_Keys, 0, sizeof(bool) * MAX_KEYS * 2); }

protected:
    //=========================================================================
    // protected variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // protected methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      キーコードを変換します.
    //!
    //! @return     キーコードを変換した結果を返却します.
    //-------------------------------------------------------------------------
    static uint32_t ConvertKey(uint32_t keyCode)
    {
        uint32_t key = keyCode;
        uint32_t result = 0;

        bool isAscii = (0x20 <= key && key <= 0x7e);
        if (isAscii)
        { return key; }

        switch(key)
        {
            case asdx::KEY_RETURN:  { result = VK_RETURN; }     break;
            case asdx::KEY_TAB:     { result = VK_TAB; }        break;
            case asdx::KEY_ESC:     { result = VK_ESCAPE; }     break;
            case asdx::KEY_BACK:    { result = VK_BACK; }       break;
            case asdx::KEY_SHIFT:   { result = VK_SHIFT; }      break;
            case asdx::KEY_CONTROL: { result = VK_CONTROL; }    break;
            case asdx::KEY_ALT:     { result = VK_MENU; }       break;
            case asdx::KEY_F1:      { result = VK_F1; }         break;
            case asdx::KEY_F2:      { result = VK_F2; }         break;
            case asdx::KEY_F3:      { result = VK_F3; }         break;
            case asdx::KEY_F4:      { result = VK_F4; }         break;
            case asdx::KEY_F5:      { result = VK_F5; }         break;
            case asdx::KEY_F6:      { result = VK_F6; }         break;
            case asdx::KEY_F7:      { result = VK_F7; }         break;
            case asdx::KEY_F8:      { result = VK_F8; }         break;
            case asdx::KEY_F9:      { result = VK_F9; }         break;
            case asdx::KEY_F10:     { result = VK_F10; }        break;
            case asdx::KEY_F11:     { result = VK_F11; }        break;
            case asdx::KEY_F12:     { result = VK_F12; }        break;
            case asdx::KEY_UP:      { result = VK_UP; }         break;
            case asdx::KEY_DOWN:    { result = VK_DOWN; }       break;
            case asdx::KEY_LEFT:    { result = VK_LEFT; }       break;
            case asdx::KEY_RIGHT:   { result = VK_RIGHT; }      break;
            case asdx::KEY_NUM0:    { result = VK_NUMPAD0; }    break;
            case asdx::KEY_NUM1:    { result = VK_NUMPAD1; }    break;
            case asdx::KEY_NUM2:    { result = VK_NUMPAD2; }    break;
            case asdx::KEY_NUM3:    { result = VK_NUMPAD3; }    break;
            case asdx::KEY_NUM4:    { result = VK_NUMPAD4; }    break;
            case asdx::KEY_NUM5:    { result = VK_NUMPAD5; }    break;
            case asdx::KEY_NUM6:    { result = VK_NUMPAD6; }    break;
            case asdx::KEY_NUM7:    { result = VK_NUMPAD7; }    break;
            case asdx::KEY_NUM8:    { result = VK_NUMPAD8; }    break;
            case asdx::KEY_NUM9:    { result = VK_NUMPAD9; }    break;
            case asdx::KEY_INSERT:  { result = VK_INSERT; }     break;
            case asdx::KEY_DELETE:  { result = VK_DELETE; }     break;
            case asdx::KEY_HOME:    { result = VK_HOME; }       break;
            case asdx::KEY_END:     { result = VK_END; }        break;
        }

        return result;
    }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    bool         m_Keys[ 2 ][ MAX_KEYS ];        //!< キーの押下状態です.
    uint32_t     m_Index;                        //!< バッファインデックスです.

    //=========================================================================
    // private methods.
    //=========================================================================
    Keyboard                (const Keyboard&) = delete;
    Keyboard& operator =    (const Keyboard&) = delete;
};


///////////////////////////////////////////////////////////////////////////////
// Mouse class
///////////////////////////////////////////////////////////////////////////////
class Mouse final : public IMouse
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    Mouse()
    : m_CursorX    (-1)
    , m_CursorY    (-1)
    , m_PrevCursorX(-1)
    , m_PrevCursorY(-1)
    , m_Index      (0)
    { memset(m_Button, false, sizeof(bool) * MAX_MOUSE_BUTTON_COUNT * 2); }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~Mouse() = default;

    //-------------------------------------------------------------------------
    //! @brief      マウスの状態を更新します.
    //-------------------------------------------------------------------------
    void UpdateState(void* pHandle)
    {
        m_Index       = 1 - m_Index;
        m_PrevCursorX = m_CursorX;
        m_PrevCursorY = m_CursorY;

        POINT pt;
        GetCursorPos( &pt );

        HWND hWnd = nullptr;
        if (pHandle != nullptr)
        {
            hWnd = (HWND)pHandle;
            ScreenToClient( hWnd, &pt );
        }

        m_CursorX = int(pt.x);
        m_CursorY = int(pt.y);

        // フォーカスがある場合のみ，ボタンステートを更新.
        if ( GetActiveWindow() == hWnd )
        {
            m_Button[m_Index][MOUSE_BUTTON_L ] = !!(GetAsyncKeyState( VK_LBUTTON )  & 0x8000);
            m_Button[m_Index][MOUSE_BUTTON_R ] = !!(GetAsyncKeyState( VK_RBUTTON )  & 0x8000);
            m_Button[m_Index][MOUSE_BUTTON_M ] = !!(GetAsyncKeyState( VK_MBUTTON )  & 0x8000);
            m_Button[m_Index][MOUSE_BUTTON_X1] = !!(GetAsyncKeyState( VK_XBUTTON1 ) & 0x8000);
            m_Button[m_Index][MOUSE_BUTTON_X2] = !!(GetAsyncKeyState( VK_XBUTTON2 ) & 0x8000);
        }
        else
        {
            m_Button[m_Index][MOUSE_BUTTON_L ] = false;
            m_Button[m_Index][MOUSE_BUTTON_R ] = false;
            m_Button[m_Index][MOUSE_BUTTON_M ] = false;
            m_Button[m_Index][MOUSE_BUTTON_X1] = false;
            m_Button[m_Index][MOUSE_BUTTON_X2] = false;
        }
    }

    //-------------------------------------------------------------------------
    //! @brief      マウスの状態をリセットします.
    //-------------------------------------------------------------------------
    void ResetState()
    {
        m_CursorX     = -1;
        m_CursorY     = -1;
        m_PrevCursorX = -1;
        m_PrevCursorY = -1;
        memset(m_Button, false, sizeof(bool) * MAX_MOUSE_BUTTON_COUNT * 2);
    }

    //-------------------------------------------------------------------------
    //! @brief      カーソルのX座標を取得します.
    //!
    //! @return     カーソルのX座標を返却します.
    //-------------------------------------------------------------------------
    int GetCursorX() const override
    { return m_CursorX; }

    //-------------------------------------------------------------------------
    //! @brief      カーソルのY座標を取得します.
    //!
    //! @return     カーソルのY座標を返却します.
    //-------------------------------------------------------------------------
    int GetCursorY() const override
    { return m_CursorY; }

    //-------------------------------------------------------------------------
    //! @brief      前のカーソルのX座標を取得します.
    //!
    //! @return     前のカーソルのX座標を返却します.
    //-------------------------------------------------------------------------
    int GetPrevCursorX() const override
    { return m_PrevCursorX; }

    //-------------------------------------------------------------------------
    //! @brief      前のカーソルのY座標を取得します.
    //!
    //! @return     前のカーソルのY座標を返却します.
    //-------------------------------------------------------------------------
    int GetPrevCursorY() const override
    { return m_PrevCursorY; }

    //-------------------------------------------------------------------------
    //! @brief      現在のカーソルと前のカーソルのX座標の差分を取得します.
    //!
    //! @return     現在のカーソルと前のカーソルのX座標の差分を返却します.
    //-------------------------------------------------------------------------
    int GetCursorDiffX() const override
    { return m_CursorX - m_PrevCursorX; }

    //-------------------------------------------------------------------------
    //! @brief      現在のカーソルと前のカーソルのY座標の差分を取得します.
    //!
    //! @return     現在のカーソルと前のカーソルのY座標の差分を返却します.
    //-------------------------------------------------------------------------
    int GetCursorDiffY() const override
    { return m_CursorY - m_PrevCursorY; }

    //-------------------------------------------------------------------------
    //! @brief      ボタンが押されっぱなしかどうかチェックします.
    //!
    //! @param[in]      button      ボタン.
    //! @return     ボタンが押された場合はtrueを返します.そうでない場合はfalseを返却します.
    //! @memo       現在のフレームでボタンが押されている場合にtrueを返します.
    //-------------------------------------------------------------------------
    bool IsHold(uint32_t button) const override
    {
        assert(button < MAX_MOUSE_BUTTON_COUNT);
        return m_Button[m_Index][button];
    }

    //-------------------------------------------------------------------------
    //! @brief      ボタンが押されたかどうかチェックします.
    //!
    //! @param[in]      button      ボタン.
    //! @return     ボタンが押されている場合はtrueを返します.
    //! @memo       前のフレームのボタン状態と現在のフレームのボタン状態が異なる場合にtrueを返します.
    //!             フラグの切り替え用途などに使用します.
    //-------------------------------------------------------------------------
    bool IsDown(uint32_t button) const override
    {
        assert(button < MAX_MOUSE_BUTTON_COUNT);
        return m_Button[m_Index][button] && (!m_Button[1 - m_Index][button]);
    }

    //-------------------------------------------------------------------------
    //! @brief      ドラッグされているかどうかチェックします.
    //!
    //! @param[in]      button      ボタン.
    //! @return     ドラッグ中の場合はtrueを返します.
    //-------------------------------------------------------------------------
    bool IsDrag(uint32_t button) const override
    {
        assert(button < MAX_MOUSE_BUTTON_COUNT);
        return m_Button[m_Index][button] && m_Button[1 - m_Index][button];
    }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    int         m_CursorX;                          //!< カーソルのX座標です.
    int         m_CursorY;                          //!< カーソルのY座標です.
    int         m_PrevCursorX;                      //!< 前のカーソルのX座標.
    int         m_PrevCursorY;                      //!< 前のカーソルのY座標.
    uint32_t    m_Index;                            //!< バッファのインデックスです.
    bool        m_Button[2][MAX_MOUSE_BUTTON_COUNT];//!< ボタンの状態です.

    //=========================================================================
    // private methods.
    //=========================================================================
    Mouse               (const Mouse&) = delete;
    Mouse& operator =   (const Mouse&) = delete;
};

///////////////////////////////////////////////////////////////////////////////
// GamePad class
///////////////////////////////////////////////////////////////////////////////
class GamePad final : public IGamePad
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    GamePad()
    : m_PlayerIndex(0)
    { ResetState(); }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~GamePad()
    { Term(); }

    void Term()
    { m_Device.Reset(); }

    //-------------------------------------------------------------------------
    //! @brief      プレイヤーインデックスを設定します.
    //!
    //! @param [in]     index       設定するプレイヤーインデックス.
    //-------------------------------------------------------------------------
    void SetPlayerIndex(uint32_t index)
    {
        assert(index < 4);
        m_PlayerIndex = index;
    }

    //-------------------------------------------------------------------------
    //! @brief      プレイヤーインデックスを取得します.
    //!
    //! @return     設定されているプレイヤーインデックスを返却します.
    //-------------------------------------------------------------------------
    uint32_t GetPlayerIndex() const override
    { return m_PlayerIndex; }

    //-------------------------------------------------------------------------
    //! @brief      コントローラーが接続されているかチェックします.
    //!
    //! @retval true    接続されています.
    //! @retval false   接続されていません.
    //-------------------------------------------------------------------------
    bool IsConnected() const override
    { return m_IsConnected; }

    //-------------------------------------------------------------------------
    //! @brief      コントローラーの状態を更新します.
    //! 
    //! @param[in]      pInput      ゲーム入力インタフェース.
    //-------------------------------------------------------------------------
    void UpdateState(IGameInput* pInput)
    {
        if (pInput != nullptr)
            UpdateByGameInput(pInput);
        else
            UpdateByXInput();
    }

    //-------------------------------------------------------------------------
    //! @brief      バイブレーションさせます.
    //!
    //! @param[in]      lhs     左のモーターの振動量(有効値:0.0 ～ 1.0).
    //! @param[in]      rhs     右のモーターの振動量(有効値:0.0 ～ 1.0).
    //-------------------------------------------------------------------------
    void Vibrate(float lhs, float rhs) const override
    {
        if (!m_Device)
        {
            XINPUT_VIBRATION vibrate = {};

            vibrate.wLeftMotorSpeed  = static_cast<WORD>(lhs * 65535.0f);
            vibrate.wRightMotorSpeed = static_cast<WORD>(rhs * 65535.0f);

            XInputSetState( m_PlayerIndex, &vibrate );
        }
        else
        {
            GameInputRumbleParams param = {};
            param.lowFrequency  = lhs;
            param.highFrequency = rhs;
            param.leftTrigger   = 0.0f;
            param.rightTrigger  = 0.0f;

            m_Device->SetRumbleState(&param);
        }
    }

    //-------------------------------------------------------------------------
    //! @brief      ボタンが押されたかチェックします.
    //!             フラグ切り替え向け用途として使用してください.
    //!
    //! @param[in]      buttonMask        チェックするボタン.
    //! @retval true    ボタンが押されました.
    //! @retval false   ボタンは押されません.
    //-------------------------------------------------------------------------
    bool IsDown(uint32_t buttonMask) const override
    { return !!(m_PressedButtons & buttonMask); } 

    //-------------------------------------------------------------------------
    //! @brief      ボタンが押されっぱなしかどうかチェックします.
    //!
    //! @param[in]      buttonMask        チェックするボタン.
    //! @return     ボタンが押されっぱなしの状態のときはtrueを返却.
    //-------------------------------------------------------------------------
    bool IsHold(uint32_t buttonMask) const override
    { return !!(m_Buttons & buttonMask); }

    //-------------------------------------------------------------------------
    //! @brief      正規化された左サムスティックのX成分を取得します.
    //!
    //! @return     正規化された左サムスティックのX成分を返却します.
    //-------------------------------------------------------------------------
    float GetThumbLX() const override
    { return m_ThumbLX; }

    //-------------------------------------------------------------------------
    //! @brief      正規化された左サムスティックのY成分を取得します.
    //!
    //! @return     正規化された左サムスティックのY成分を返却します.
    //-------------------------------------------------------------------------
    float GetThumbLY() const override
    { return m_ThumbLY; }

    //-------------------------------------------------------------------------
    //! @brief      正規化された右サムスティックのX成分を取得します.
    //!
    //! @return     正規化された右サムスティックのX成分を返却します.
    //-------------------------------------------------------------------------
    float GetThumbRX() const override
    { return m_ThumbRX; }

    //-------------------------------------------------------------------------
    //! @brief      正規化された右サムスティックのY成分を取得します.
    //!
    //! @return     正規化された右サムスティックのy成分を返却します.
    //-------------------------------------------------------------------------
    float GetThumbRY() const override
    { return m_ThumbRY; }

    //-------------------------------------------------------------------------
    //! @brief      左トリガーの値を取得します.
    //! 
    //! @return     左トリガーの値を返却します.
    //-------------------------------------------------------------------------
    float GetTriggerL() const override
    { return m_TriggerL; }

    //-------------------------------------------------------------------------
    //! @brief      右トリガーの値を取得します.
    //! 
    //! @return     右トリガーの値を返却します.
    //-------------------------------------------------------------------------
    float GetTriggerR() const override
    { return m_TriggerR; }

    //-------------------------------------------------------------------------
    //! @brief      ベンダーIDを取得します.
    //! 
    //! @return     ベンダーIDを返却します.
    //-------------------------------------------------------------------------
    uint16_t GetVendorId() const override
    {
        if (m_Device.GetPtr() != nullptr)
            return m_Device->GetDeviceInfo()->vendorId;

        return kPadVendorMicrosoft; // MS扱い.
    }

    //-------------------------------------------------------------------------
    //! @brief      製品IDを取得します.
    //! 
    //! @return     製品IDを返却します.
    //-------------------------------------------------------------------------
    uint16_t GetProductId() const override
    {
        if (m_Device.GetPtr() != nullptr)
            return m_Device->GetDeviceInfo()->productId;

        return 0x028E; // 有線Xbox360コントローラー扱い.
    }

    //-------------------------------------------------------------------------
    //! @brief      ボタンの値をリセットします.
    //-------------------------------------------------------------------------
    void ResetState()
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

    //-------------------------------------------------------------------------
    //! @brief      入力デバイスを取得します.
    //! 
    //! @return     入力デバイスを返却します.
    //-------------------------------------------------------------------------
    IGameInputDevice* GetDevice() const
    { return m_Device.GetPtr(); }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    int         m_PlayerIndex;          //!< プレーヤーインデックスです.
    bool        m_IsConnected;          //!< 接続中かどうか.
    float       m_TriggerL;             //!< 左トリガーの状態.
    float       m_TriggerR;             //!< 右トリガーの状態.
    float       m_ThumbLX;              //!< 正規化された左サムスティックX成分.
    float       m_ThumbLY;              //!< 正規化された左サムスティックY成分.
    float       m_ThumbRX;              //!< 正規化された右サムスティックX成分.
    float       m_ThumbRY;              //!< 正規化された右サムスティックY成分.
    uint32_t    m_Buttons;              //!< ボタンの状態.
    uint32_t    m_PressedButtons;       //!< 押されているボタン.
    uint32_t    m_LastButtons;          //!< 1つ前のボタンの状態.

    RefPtr<IGameInputDevice> m_Device;

    //=========================================================================
    // private methods.
    //=========================================================================
    GamePad             (const GamePad&) = delete;
    GamePad& operator = (const GamePad&) = delete;

    //-------------------------------------------------------------------------
    //! @brief      Microsoft.GameInputを用いて更新処理を行います.
    //-------------------------------------------------------------------------
    void UpdateByGameInput(IGameInput* pInput)
    {
        assert(pInput != nullptr);

        RefPtr<IGameInputReading> reading;
        if (!m_Device)
        {
            auto hr = pInput->GetCurrentReading(
                GameInputKindGamepad, nullptr, reading.GetAddress());
            if (FAILED(hr))
                return;

            IGameInputDevice* pDevice = nullptr;
            reading->GetDevice(&pDevice);

            // デバイス取得に失敗　または 別インデックスで登録済みなら失敗扱い.
            if (pDevice == nullptr || IsAssigned(pDevice, m_PlayerIndex))
                return;

            // デバイスを設定
            m_Device.Attach(pDevice);
        }
        else
        {
            auto hr= pInput->GetCurrentReading(
                GameInputKindGamepad, m_Device.GetPtr(), reading.GetAddress());
            if (FAILED(hr))
                return;
        }

        GameInputGamepadState state = {};
        reading->GetGamepadState(&state);
        reading.Reset();

        m_Buttons  = ConvertButtons(state.buttons);
        m_TriggerL = state.leftTrigger;
        m_TriggerR = state.rightTrigger;
        m_ThumbLX  = state.leftThumbstickX;
        m_ThumbLY  = state.leftThumbstickY;
        m_ThumbRX  = state.rightThumbstickX;
        m_ThumbRY  = state.rightThumbstickY;

        const auto kTriggerThreshold = float(XINPUT_GAMEPAD_TRIGGER_THRESHOLD) / 256.0f;
        if (state.leftTrigger > kTriggerThreshold)
            m_Buttons |= PAD_TRIGGER_L;
        if (state.rightTrigger > kTriggerThreshold)
            m_Buttons |= PAD_TRIGGER_R;

        m_PressedButtons = ( m_LastButtons ^ m_Buttons ) & m_Buttons;
        m_LastButtons    = m_Buttons;
    }

    //-------------------------------------------------------------------------
    //! @brief      XInputを用いて更新処理を行います.
    //-------------------------------------------------------------------------
    void UpdateByXInput()
    {
        XINPUT_STATE state = {};

        const auto result = XInputGetState(m_PlayerIndex, &state);
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

    //-------------------------------------------------------------------------
    //! @brief      Microsoft.GameInputのボタンマスクからラッパークラスのボタンマスクに変換します.
    //! 
    //! @return     ラッパークラスのボタンマスクに変換した結果を返却します.
    //-------------------------------------------------------------------------
    uint32_t ConvertButtons(uint32_t buttonMask)
    {
        uint32_t result = 0;

        if (!!(buttonMask & GameInputGamepadMenu))
            result |= asdx::PAD_START;
        if (!!(buttonMask & GameInputGamepadView))
            result |= asdx::PAD_BACK;
        if (!!(buttonMask & GameInputGamepadA))
            result |= asdx::PAD_A;
        if (!!(buttonMask & GameInputGamepadB))
            result |= asdx::PAD_B;
        if (!!(buttonMask & GameInputGamepadX))
            result |= asdx::PAD_X;
        if (!!(buttonMask & GameInputGamepadY))
            result |= asdx::PAD_Y;
        if (!!(buttonMask & GameInputGamepadDPadUp))
            result |= asdx::PAD_UP;
        if (!!(buttonMask & GameInputGamepadDPadDown))
            result |= asdx::PAD_DOWN;
        if (!!(buttonMask & GameInputGamepadDPadLeft))
            result |= asdx::PAD_LEFT;
        if (!!(buttonMask & GameInputGamepadDPadRight))
            result |= asdx::PAD_RIGHT;
        if (!!(buttonMask & GameInputGamepadLeftShoulder))
            result |= asdx::PAD_SHOULDER_L;
        if (!!(buttonMask & GameInputGamepadRightShoulder))
            result |= asdx::PAD_SHOULDER_R;
        if (!!(buttonMask & GameInputGamepadLeftThumbstick))
            result |= asdx::PAD_THUMB_L;
        if (!!(buttonMask & GameInputGamepadRightThumbstick))
            result |= asdx::PAD_THUMB_R;

        return result;
    }
};

namespace {

//-----------------------------------------------------------------------------
// Global Variables.
//-----------------------------------------------------------------------------
GamePad             g_Pad[4]     = {};
Keyboard            g_Keyboard   = {};
Mouse               g_Mouse      = {};
bool                g_Init       = false;
RefPtr<IGameInput>  g_GameInput  = {};
SpinLock            g_SpinLock   = {};
std::atomic<bool>   g_CtrlLock   = false;

//-----------------------------------------------------------------------------
//      アサイン済みかどうかチェックします.
//-----------------------------------------------------------------------------
bool IsAssigned(IGameInputDevice* pDevice, uint32_t index)
{
    assert(pDevice != nullptr);

    for(auto i=0; i<4; ++i)
    {
        // 自分自身は飛ばす.
        if (i == index)
            continue;

        // 同じデバイスなら登録済み.
        if (g_Pad[i].GetDevice() == pDevice)
            return true;

    }

    // 一致するものが見つからなかった.
    return false;
}

} // namespace

//-----------------------------------------------------------------------------
//      HIDの初期化処理を行います.
//-----------------------------------------------------------------------------
void InitHid()
{
    if (g_Init)
        return;

    ScopedLock<SpinLock> locker(g_SpinLock);

    auto hr = GameInputCreate(g_GameInput.GetAddress());
    if (FAILED(hr))
    {
        ELOGA("Error : GameInputCreate() Failed. errcode = 0x%x", hr);
        g_GameInput.Reset();
        ILOGA("Info : Switch to Microsoft.GameInput to XInput.");
    }

    for(auto i=0u; i<4; ++i)
        g_Pad[i].SetPlayerIndex(i);

    g_Init      = true;
    g_CtrlLock  = false;
}

//-----------------------------------------------------------------------------
//      HIDの終了処理を行います.
//-----------------------------------------------------------------------------
void TermHid()
{
    if (!g_Init)
        return;

    ScopedLock<SpinLock> locker(g_SpinLock);
    for(auto i=0; i<4; ++i)
    {
        g_Pad[i].Term();
        g_Pad[i].ResetState();
    }

    g_GameInput.Reset();
    g_Init     = false;
    g_CtrlLock = false;
}

//-----------------------------------------------------------------------------
//      HIDの更新処理を行います.
//-----------------------------------------------------------------------------
void UpdateHid(void* hWnd)
{
    if (!g_Init)
        return;

    // ウィンドウフォーカスを持つかどうかチェック.
    auto hasFocus = (GetFocus() == hWnd);

    ScopedLock<SpinLock> locker(g_SpinLock);
    // ロック中やウィンドウフォーカスがない場合はステートをリセット.
    if (g_CtrlLock || !hasFocus)
    {
        for(auto i=0; i<4; ++i)
            g_Pad[i].ResetState();

        g_Keyboard.ResetState();
        g_Mouse.ResetState();
    }
    // 非ロック中のみ更新.
    else
    {
        for(auto i=0; i<4; ++i)
            g_Pad[i].UpdateState(g_GameInput.GetPtr());

        g_Keyboard.UpdateState();
        g_Mouse.UpdateState(hWnd);
    }
}

//-----------------------------------------------------------------------------
//      HIDの状態をリセットします.
//-----------------------------------------------------------------------------
void ResetHid()
{
    ScopedLock<SpinLock> locker(g_SpinLock);
    for(auto i=0; i<4; ++i)
        g_Pad[i].ResetState();

    g_Keyboard.ResetState();
    g_Mouse.ResetState();
}

//-----------------------------------------------------------------------------
//      ゲームパッドを取得します.
//-----------------------------------------------------------------------------
const IGamePad& GetPad(uint32_t index)
{
    assert(index < 4);
    ScopedLock<SpinLock> locker(g_SpinLock);
    return g_Pad[index];
}

//-----------------------------------------------------------------------------
//      キーボードを取得します.
//-----------------------------------------------------------------------------
const IKeyboard& GetKeyboard()
{
    ScopedLock<SpinLock> locker(g_SpinLock);
    return g_Keyboard;
}

//-----------------------------------------------------------------------------
//      マウスを取得します.
//-----------------------------------------------------------------------------
const IMouse& GetMouse()
{
    ScopedLock<SpinLock> locker(g_SpinLock);
    return g_Mouse;
}

//-----------------------------------------------------------------------------
//      HID処理をロックします.
//-----------------------------------------------------------------------------
void LockHid()
{
    assert(g_Init);
    g_CtrlLock = true;
}

//-----------------------------------------------------------------------------
//      HID処理のロックを解除します.
//-----------------------------------------------------------------------------
void UnlockHid()
{
    assert(g_Init);
    g_CtrlLock = false;
}

//-----------------------------------------------------------------------------
//      HID処理がロック中かチェックします.
//-----------------------------------------------------------------------------
bool IsLockHid()
{ return g_CtrlLock; }

} // namespace asdx

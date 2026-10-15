//-----------------------------------------------------------------------------
// File : asdxHidManager.cpp
// Desc : Human Input Device Manager.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxHid.h>
#include <fnd/asdxRef.h>
#include <fnd/asdxLogger.h>
#include <cassert>
#include <GameInput.h>


//-----------------------------------------------------------------------------
// Using Namespace
//-----------------------------------------------------------------------------
using namespace GameInput::v4;


namespace {

///////////////////////////////////////////////////////////////////////////////
// GamePad structure
///////////////////////////////////////////////////////////////////////////////
struct GamePad
{
    asdx::RefPtr<IGameInputDevice> Device    = {};
    GameInputGamepadState          State     = {};
    bool                           Connected = false;
};

//-----------------------------------------------------------------------------
// Global Variables.
//-----------------------------------------------------------------------------
asdx::RefPtr<IGameInput>  g_GameInput  = {};
GamePad                   g_GamePad[4] = {};

//-----------------------------------------------------------------------------
//      アサイン済みかどうかチェックします.
//-----------------------------------------------------------------------------
bool IsAssigned(IGameInputDevice* pDevice)
{
    for(auto i=0; i<4; ++i)
    {
        if (g_GamePad[i].Device.GetPtr() == pDevice)
            return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
//      パッド状態を更新します.
//-----------------------------------------------------------------------------
void UpdateGamePad(uint32_t index)
{
    asdx::RefPtr<IGameInputReading> reading;

    auto& pad = g_GamePad[index];

    // ステートをリセット.
    pad.State     = {};
    pad.Connected = false;

    // 未割当.
    if (pad.Device.GetPtr() == nullptr)
    {
        auto hr = g_GameInput->GetCurrentReading(
            GameInputKindGamepad, nullptr, reading.GetAddress());

        if (FAILED(hr))
            return;

        asdx::RefPtr<IGameInputDevice> device;
        reading->GetDevice(device.GetAddress());

        if (device.GetPtr() != nullptr && IsAssigned(device.GetPtr()))
        {
            pad.Device    = device;
            pad.Connected = true;
        }
    }
    // 割り当て済み.
    else
    {
        auto hr = g_GameInput->GetCurrentReading(
            GameInputKindGamepad, pad.Device.GetPtr(), reading.GetAddress());

        if (FAILED(hr))
            return;

        pad.Connected = true;
    }

    if (!reading->GetGamepadState(&pad.State))
        return;
}

} // namespace


namespace asdx {

//-----------------------------------------------------------------------------
//      パッド状態を取得します.
//-----------------------------------------------------------------------------
void GetPadState(int index, GameInputGamepadState& state, bool& connected)
{
    assert(index < 4);
    state       = g_GamePad[index].State;
    connected   = g_GamePad[index].Connected;
}

//-----------------------------------------------------------------------------
//      パッドを振動させます.
//-----------------------------------------------------------------------------
void SetPadVibrate(int index, const GameInputRumbleParams* param)
{
    assert(index < 4);
    if (!g_GamePad[index].Device)
        return;

    g_GamePad[index].Device->SetRumbleState(param);
}

///////////////////////////////////////////////////////////////////////////////
// HidManager class
///////////////////////////////////////////////////////////////////////////////
HidManager HidManager::s_Instance = {};

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
HidManager::HidManager()
{
    for(auto i=0u; i<4; ++i)
    { m_Pad[i].SetPlayerIndex(i); }

    m_Mouse.ResetState();
}

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
HidManager::~HidManager()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      シングルトンインスタンスを取得します.
//-----------------------------------------------------------------------------
HidManager& HidManager::Instance()
{ return s_Instance; }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool HidManager::Init()
{
    if (g_GameInput.GetPtr() != nullptr)
        return true;

    auto hr = GameInputCreate(g_GameInput.GetAddress());
    if (FAILED(hr))
    {
        ELOGA("Error : GetInputCreate() failed. errcode = 0x%x", hr);
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void HidManager::Term()
{
    for(auto i=0; i<4; ++i)
    {
        g_GamePad[i].Device.Reset();
        g_GamePad[i].Connected  = false;
        g_GamePad[i].State      = {};
    }

    g_GameInput.Reset();
}

//-----------------------------------------------------------------------------
//      更新処理を行います.
//-----------------------------------------------------------------------------
void HidManager::Update(void* windowHandle)
{
    for(auto i=0u; i<4; ++i)
    {
        UpdateGamePad(i);
        m_Pad[i].UpdateState();
    }

    m_Mouse.UpdateState(windowHandle);
    m_Keyboard.UpdateState();
}

//-----------------------------------------------------------------------------
//      ゲームパッドを取得します.
//-----------------------------------------------------------------------------
const GamePad& HidManager::GetPad(uint32_t index) const
{
    assert(index < 4);
    return m_Pad[index];
}

//-----------------------------------------------------------------------------
//      マウスを取得します.
//-----------------------------------------------------------------------------
const Mouse& HidManager::GetMouse() const
{ return m_Mouse; }

//-----------------------------------------------------------------------------
//      キーボードを取得します.
//-----------------------------------------------------------------------------
const Keyboard& HidManager::GetKeyboard() const
{ return m_Keyboard; }

} // namespace asdx

//-----------------------------------------------------------------------------
// File : asdxSound.cpp
// Desc : Sound Manager.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fw/asdxSound.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxMisc.h>
#include <map>
#include <Windows.h>


namespace {

//-----------------------------------------------------------------------------
//      エラーを表示します.
//-----------------------------------------------------------------------------
void ShowError(uint32_t ret)
{
    if (ret == 0)
        return;

    char buf[512] = {};
    mciGetErrorStringA(ret, buf, sizeof(buf));
    ELOG("Error : %s", buf);
}

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// SoundManager class
///////////////////////////////////////////////////////////////////////////////
class SoundManager : public ISoundManager
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
    //! @brief      シングルトンインスタンスを取得します.
    //-------------------------------------------------------------------------
    static SoundManager& Instance()
    { return s_Instance; }

    //-------------------------------------------------------------------------
    //! @brief      初期化処理を行います.
    //-------------------------------------------------------------------------
    void Init(uintptr_t hWnd)
    { m_hWnd = hWnd; }

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term()
    {
        auto itr = std::begin(m_Status);
        while(itr != std::end(m_Status))
        {
            mciSendCommandA(itr->first, MCI_CLOSE, 0, 0);
            m_UserIds.erase(itr->second.DeviceId);
            itr = m_Status.erase(itr);
        }

        m_UserIds.clear();
        m_Status .clear();
    }

    //-------------------------------------------------------------------------
    //! @brief      オープンします
    //-------------------------------------------------------------------------
    bool Open(SoundId id, const char* path) override
    {
        if (path == nullptr)
        {
            ELOG("Error : Invalid Argument.");
            return false;
        }

        if (!IsExist(id))
        {
            ELOG("Error : Already Opend. id = %u", id);
            return false;
        }

        MCIERROR        ret   = {};
        MCI_OPEN_PARMSA param = {};
        param.lpstrElementName = path;

        auto ext = GetExtA( path );
        if (ext == "wave" || ext == "wav")
        {
            param.lpstrDeviceType = (LPCSTR)MCI_DEVTYPE_WAVEFORM_AUDIO;
            ret = mciSendCommandA(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_OPEN_ELEMENT, (DWORD_PTR)&param);
        }
        else if (ext == "midi" || ext == "mid")
        {
            param.lpstrDeviceType = (LPCSTR)MCI_DEVTYPE_SEQUENCER;
            ret = mciSendCommandA(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_OPEN_ELEMENT, (DWORD_PTR)&param);
        }
        else if (ext == "mp3")
        {
            param.lpstrDeviceType = "MPEGVideo";
            ret = mciSendCommandA(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD_PTR)&param);
        }
        else 
        {
            ELOG("Error : Unsupported Format. ext = %s", ext.c_str());
            return false;
        }

        if (ret != 0)
        {
            ShowError(ret);
            return false;
        }

        Status status = {};
        status.CurLoopCount = 0;
        status.MaxLoopCount = 0;
        status.State        = SoundState::Stop;
        status.DeviceId     = param.wDeviceID;

        m_Status[id] = status;
        m_UserIds[status.DeviceId] = id;

        return true;
    }

    //-------------------------------------------------------------------------
    //! @brief      クローズします.
    //-------------------------------------------------------------------------
    void Close(SoundId id) override
    {
        if (!IsExist(id))
        { return; }

        auto ret = mciSendCommandA(m_Status[id].DeviceId, MCI_CLOSE, 0, 0);
        m_UserIds.erase(m_Status[id].DeviceId);
        m_Status.erase(id);

        ShowError(ret);
    }

    //-------------------------------------------------------------------------
    //! @brief      再生します.
    //-------------------------------------------------------------------------
    void Play(SoundId id, int loopCount = 0) override
    {
        if (!IsExist(id))
        { return; }

        MCI_PLAY_PARMS param = {};
        param.dwCallback = m_hWnd;

        auto ret = mciSendCommandA(m_Status[id].DeviceId, MCI_PLAY, MCI_NOTIFY, (DWORD_PTR)&param);
        if ( ret == 0 )
        {
            m_Status[id].State = SoundState::Play;
            m_Status[id].MaxLoopCount = loopCount;
            m_Status[id].CurLoopCount = 0;
        }

        ShowError(ret);
    }

    //-------------------------------------------------------------------------
    //! @brief      停止します.
    //-------------------------------------------------------------------------
    void Stop(SoundId id) override
    {
        if (!IsExist(id))
        { return; }

        auto ret = mciSendCommandA(m_Status[id].DeviceId, MCI_STOP, 0, 0);
        if (ret == 0)
        { m_Status[id].State = SoundState::Stop; }

        if (ret != 0)
        {
            ShowError(ret);
            return;
        }

        ret = mciSendCommandA(m_Status[id].DeviceId, MCI_SEEK, MCI_SEEK_TO_START, 0);
        ShowError(ret);
    }

    //-------------------------------------------------------------------------
    //! @brief      一時停止します.
    //-------------------------------------------------------------------------
    void Pause(SoundId id) override
    {
        if (!IsExist(id))
        { return; }

        auto ret = mciSendCommandA(m_Status[id].DeviceId, MCI_PAUSE, 0, 0);
        if ( ret == 0 )
        { m_Status[id].State = SoundState::Pause; }

        ShowError(ret);
    }

    //-------------------------------------------------------------------------
    //! @brief      一時停止を解除します.
    //-------------------------------------------------------------------------
    void Resume(SoundId id) override
    {
        if (!IsExist(id))
        { return; }

        auto ret = mciSendCommandA(m_Status[id].DeviceId, MCI_RESUME, 0, 0);
        if ( ret == 0 )
        { m_Status[id].State = SoundState::Play; }

        ShowError( ret );
    }

    //-------------------------------------------------------------------------
    //! @brief      サウンドが登録されているかチェックします.
    //! 
    //! @param[in]      id      登録ID.
    //! @retval true    登録済み.
    //! @retval false   未登録.
    //-------------------------------------------------------------------------
    bool IsExist(SoundId id)
    { return m_Status.find(id) != m_Status.end(); }

    //-------------------------------------------------------------------------
    //! @brief      コールバック処理.
    //-------------------------------------------------------------------------
    void OnNotify(SoundId id, uint32_t param)
    {
        if (!IsExist(id))
        { return; }

        auto key = m_UserIds[id];
        if (param == MCI_NOTIFY_SUCCESSFUL)
        {
            mciSendCommandA(m_Status[key].DeviceId, MCI_SEEK, MCI_SEEK_TO_START, 0);
            m_Status[key].CurLoopCount++;

            if (m_Status[key].CurLoopCount < m_Status[key].MaxLoopCount || m_Status[key].MaxLoopCount == -1)
            {
                MCI_PLAY_PARMS param = {};
                param.dwCallback = m_hWnd;

                auto ret = mciSendCommandA(m_Status[key].DeviceId, MCI_PLAY, MCI_NOTIFY, (DWORD_PTR)&param);
                if (ret == 0)
                { m_Status[key].State = SoundState::Play; }
                ShowError(ret);
            }
            else
            {
                m_Status[key].State = SoundState::Done;
            }
        }
        else if (param == MCI_NOTIFY_FAILURE)
        {
            m_Status[key].State = SoundState::Error;
        }
    }

private:
    ///////////////////////////////////////////////////////////////////////////
    // Status structure
    ///////////////////////////////////////////////////////////////////////////
    struct Status
    {
        int         MaxLoopCount = 0;       //!< 最大ループ回数.
        int         CurLoopCount = 0;       //!< 現在のループ回数.
        int         State        = 0;       //!< 再生状態.
        uint32_t    DeviceId     = 0;       //!< デバイスID.
    };

    //=========================================================================
    // private variables.
    //=========================================================================
    static SoundManager         s_Instance;     //!< シングルトンインスタンス.
    std::map<SoundId, Status>   m_Status;       //!< 状態管理用.
    std::map<SoundId, uint32_t> m_UserIds;      //!< デバイスID ---> ユーザーID参照用.
    uintptr_t                   m_hWnd = 0;     //!< ウィンドウハンドル.

    //=========================================================================
    // private methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    SoundManager()
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~SoundManager()
    { Term(); }

    SoundManager           (const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;
};

//-----------------------------------------------------------------------------
//      サウンドマネージャを取得します.
//-----------------------------------------------------------------------------
ISoundManager& GetSoundManager()
{ return SoundManager::Instance(); }

//-----------------------------------------------------------------------------
//      サウンドマネージャを初期化します.
//-----------------------------------------------------------------------------
void InitSoundMgr(uintptr_t hWnd)
{ SoundManager::Instance().Init(hWnd); }

//-----------------------------------------------------------------------------
//      サウンドマネージャの終了処理です.
//-----------------------------------------------------------------------------
void TermSoundMgr()
{ SoundManager::Instance().Term(); }

//-----------------------------------------------------------------------------
//      サウンドマネージャのコールバック処理です.
//-----------------------------------------------------------------------------
void OnSoundMsg(SoundId id, uint32_t param)
{ SoundManager::Instance().OnNotify(id, param); }

} // namespace asdx

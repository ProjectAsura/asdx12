//-----------------------------------------------------------------------------
// File : asdxSoundResource.cpp
// Desc : Sound Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <fnd/asdxLogger.h>
#include <snd/asdxSoundResource.h>
#include <snd/asdxSoundEngine.h>
#include <xaudio2.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// SoundResource class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
SoundResource::SoundResource()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
SoundResource::~SoundResource()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool SoundResource::Init(const SoundData& data)
{
    if (data.FormatData.empty() || data.AudioData.empty())
    {
        ELOGA("Error : Invalid Argument.");
        return false;
    }

    auto pXAudio2 = GetXAudio2();
    if (pXAudio2 == nullptr)
    {
        ELOGA("Error : GetXAudio2() is nullptr");
        return false;
    }

    const auto pFormat = reinterpret_cast<const WAVEFORMATEX*>(data.FormatData.data());

    auto hr = pXAudio2->CreateSourceVoice(&m_pSourceVoice, pFormat);
    if (FAILED(hr))
    {
        ELOGA("Error : IXAudio2::CreateSourceVoice() Failed. errcode = 0x%x", hr);
        return false;
    }

    m_AudioData = data.AudioData;
    m_Loop      = data.Loop;

    ResetBuffer();

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void SoundResource::Term()
{
    m_AudioData.clear();
    m_AudioData.shrink_to_fit();

    if (m_pSourceVoice != nullptr)
    {
        m_pSourceVoice->DestroyVoice();
        m_pSourceVoice = nullptr;
    }

    m_Loop = false;
}

//-----------------------------------------------------------------------------
//      再生処理を行います.
//-----------------------------------------------------------------------------
void SoundResource::Play()
{
    assert(m_pSourceVoice != nullptr);
    if (IsFinished())
    { ResetBuffer(); }

    auto hr = m_pSourceVoice->Start();
    if (FAILED(hr))
        ELOGA("Error : IXAudio2SourceVoice::Start() Failed. errcode = 0x%x", hr);
}

//-----------------------------------------------------------------------------
//      停止処理を行います.
//-----------------------------------------------------------------------------
void SoundResource::Stop()
{
    assert(m_pSourceVoice != nullptr);
    auto hr = m_pSourceVoice->Stop();
    if (FAILED(hr))
        ELOGA("Error : IXAudio2SourceVoice::Stop() Failed. errcode = 0x%x", hr);
}

//-----------------------------------------------------------------------------
//      ループ再生フラグを取得します.
//-----------------------------------------------------------------------------
bool SoundResource::IsLoop() const
{ return m_Loop; }

//-----------------------------------------------------------------------------
//      ループ再生を終了します.
//-----------------------------------------------------------------------------
void SoundResource::ExitLoop()
{
    assert(m_pSourceVoice != nullptr);
    auto hr = m_pSourceVoice->ExitLoop();
    if (FAILED(hr))
        ELOGA("Error : IXAudio2SourceVoice::ExitLoop() Failed. errcode = 0x%x", hr);
    else
        m_Loop = false;
}

//-----------------------------------------------------------------------------
//      再生が完了しているかどうかチェックします.
//-----------------------------------------------------------------------------
bool SoundResource::IsFinished() const
{
    assert(m_pSourceVoice != nullptr);
    XAUDIO2_VOICE_STATE state = {};
    m_pSourceVoice->GetState(&state);
    return state.BuffersQueued == 0;
}

//-----------------------------------------------------------------------------
//      音量を設定します.
//-----------------------------------------------------------------------------
void SoundResource::SetVolume(float value)
{
    assert(m_pSourceVoice != nullptr);
    auto hr = m_pSourceVoice->SetVolume(value);
    if (FAILED(hr))
        ELOGA("Error : IXAudio2SourceVoice::SetVolume() Failed. errcode = 0x%x", hr);
}

//-----------------------------------------------------------------------------
//      音量を取得します.
//-----------------------------------------------------------------------------
float SoundResource::GetVolume() const
{
    assert(m_pSourceVoice != nullptr);
    float value = 0.0f;
    m_pSourceVoice->GetVolume(&value);
    return value;
}

//-----------------------------------------------------------------------------
//      ソースボイスを取得します.
//-----------------------------------------------------------------------------
IXAudio2SourceVoice* SoundResource::GetSourceVoice() const
{ return m_pSourceVoice; }

//-----------------------------------------------------------------------------
//      バッファを再設定します.
//-----------------------------------------------------------------------------
void SoundResource::ResetBuffer()
{
    XAUDIO2_BUFFER buf = {};
    buf.pAudioData = m_AudioData.data();
    buf.AudioBytes = UINT32(m_AudioData.size());
    buf.LoopCount  = (m_Loop) ? XAUDIO2_LOOP_INFINITE : 0;
    buf.Flags      = (m_Loop) ? 0 : XAUDIO2_END_OF_STREAM;

    m_pSourceVoice->SubmitSourceBuffer(&buf);
}

} // namespace asdx

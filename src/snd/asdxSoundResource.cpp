//-----------------------------------------------------------------------------
// File : asdxSoundResource.cpp
// Desc : Sound Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

#define XAUDIO2_HELPER_FUNCTIONS (1)

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <fnd/asdxLogger.h>
#include <fnd/asdxMath.h>
#include <snd/asdxSoundResource.h>
#include <snd/asdxSoundEngine.h>
#include <xaudio2.h>


namespace asdx {

static_assert(sizeof (WaveFormat) == sizeof (WAVEFORMATEX), "WaveFormat Structure Size Not Matched.");
static_assert(alignof(WaveFormat) == alignof(WAVEFORMATEX), "WaveFormat Structure Alignment Not Matched.");

//-----------------------------------------------------------------------------
//      デシベルからボリュームに変換します.
//-----------------------------------------------------------------------------
float DbToVolume(float db)
{ return XAudio2DecibelsToAmplitudeRatio(db); }

//-----------------------------------------------------------------------------
//      ボリュームからデシベルに変換します.
//-----------------------------------------------------------------------------
float VolumeToDb(float volume)
{ return XAudio2AmplitudeRatioToDecibels(volume); }


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
bool SoundResource::Init(const SoundData& data, const SoundBufferDesc& desc)
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

    m_SounData = data;
    m_Desc     = desc;

    ResetBuffer(desc);

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void SoundResource::Term()
{
    m_SounData = {};
    m_Desc     = {};
    m_Loop     = false;

    if (m_pSourceVoice != nullptr)
    {
        m_pSourceVoice->DestroyVoice();
        m_pSourceVoice = nullptr;
    }
}

//-----------------------------------------------------------------------------
//      再生処理を行います.
//-----------------------------------------------------------------------------
void SoundResource::Play()
{
    assert(m_pSourceVoice != nullptr);
    if (IsFinished())
    { ResetBuffer(m_Desc); }

    auto hr = m_pSourceVoice->Start();
    if (FAILED(hr))
        ELOGA("Error : IXAudio2SourceVoice::Start() Failed. errcode = 0x%x", hr);
}

//-----------------------------------------------------------------------------
//      一時停止処理を行います.
//-----------------------------------------------------------------------------
void SoundResource::Pause()
{
    assert(m_pSourceVoice != nullptr);
    auto hr = m_pSourceVoice->Stop(XAUDIO2_PLAY_TAILS);
    if (FAILED(hr))
        ELOGA("Error : IXAudio2SourceVoice::Stop() Failed. errcode = 0x%x", hr);
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

    hr = m_pSourceVoice->FlushSourceBuffers();
    if (FAILED(hr))
        ELOGA("Error : IXAudio2SourceVoice::FlushSourceBuffers() Failed. errcode = 0x%x", hr);
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
//      再生ピッチを設定します.
//-----------------------------------------------------------------------------
void SoundResource::SetPitch(float value)
{
    assert(m_pSourceVoice != nullptr);
    m_pSourceVoice->SetFrequencyRatio(value);
}

//----------------------------------------------------------------------------
//      再生ピッチを取得します.
//----------------------------------------------------------------------------
float SoundResource::GetPitch() const
{
    assert(m_pSourceVoice != nullptr);
    float value = 0.0f;
    m_pSourceVoice->GetFrequencyRatio(&value);
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
void SoundResource::ResetBuffer(const SoundBufferDesc& desc)
{
    XAUDIO2_BUFFER buf = {};
    buf.pAudioData = desc.pAudioData;
    buf.AudioBytes = desc.AudioBytes;
    buf.PlayBegin  = desc.PlayBegin;
    buf.PlayLength = desc.PlayLength;
    buf.LoopBegin  = desc.LoopBegin;
    buf.LoopLength = desc.LoopLength;
    buf.LoopCount  = desc.LoopCount;
    buf.Flags      = desc.Flags;

    m_Loop = (desc.LoopCount == XAUDIO2_LOOP_INFINITE);
    m_Desc = desc;

    m_pSourceVoice->SubmitSourceBuffer(&buf);
}

//-----------------------------------------------------------------------------
//      オーディオデータを取得します.
//-----------------------------------------------------------------------------
const std::vector<uint8_t>& SoundResource::GetAudioData() const
{ return m_SounData.AudioData; }

//-----------------------------------------------------------------------------
//      波形フォーマットを取得します.
//-----------------------------------------------------------------------------
const WaveFormat* SoundResource::GetWaveFormat() const
{ return reinterpret_cast<const WaveFormat*>(m_SounData.FormatData.data()); }

//-----------------------------------------------------------------------------
//      サウンドバッファ設定を取得します.
//-----------------------------------------------------------------------------
const SoundBufferDesc& SoundResource::GetDesc() const
{ return m_Desc; }

//-----------------------------------------------------------------------------
//      音量フェード処理を行います.
//-----------------------------------------------------------------------------
void SoundResource::Fade(float targetVolume, float targetSec)
{
    m_FadeTargetVolume = targetVolume;
    m_FadeStartVolume  = GetVolume();
    m_FadeSec          = targetSec;
    m_ElapsedSec       = 0.0f;
}

//-----------------------------------------------------------------------------
//      更新処理を行います.
//-----------------------------------------------------------------------------
void SoundResource::Update(float deltaSec)
{
    if (m_FadeSec <= 0.0f)
        return;

    m_ElapsedSec += deltaSec;
    auto complete = false;
    if (m_ElapsedSec > m_FadeSec)
    {
        m_ElapsedSec = m_FadeSec;
        complete     = true;
    }

    auto volume = Lerp(m_FadeStartVolume, m_FadeTargetVolume, Saturate(m_ElapsedSec / m_FadeSec));
    SetVolume(volume);

    if (complete)
    { m_FadeSec = 0.0f; }
}

} // namespace asdx

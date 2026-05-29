//-----------------------------------------------------------------------------
// File : asdxSpriteAnimation.cpp
// Desc : Sprite Animation.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxSpriteAnimation.h>
#include <gfx/asdxSprite.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// SpriteAnimationPlayer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
SpriteAnimationPlayer::SpriteAnimationPlayer()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
SpriteAnimationPlayer::~SpriteAnimationPlayer()
{ Reset(); }

//-----------------------------------------------------------------------------
//      リセット処理を行います.
//-----------------------------------------------------------------------------
void SpriteAnimationPlayer::Reset()
{
    m_TimeSec       = 0.0f;
    m_FrameIndex    = 0;
    m_Loop          = false;
    m_Pause         = false;
    m_PlaySpeed     = 1.0f;
    m_pTrack        = nullptr;
}

//-----------------------------------------------------------------------------
//      トラックを設定します.
//-----------------------------------------------------------------------------
void SpriteAnimationPlayer::SetTrack(const SpriteAnimationTrack* pTrack)
{ m_pTrack = pTrack; }

//-----------------------------------------------------------------------------
//      トラックを取得します.
//-----------------------------------------------------------------------------
const SpriteAnimationTrack* SpriteAnimationPlayer::GetTrack() const
{ return m_pTrack; }

//-----------------------------------------------------------------------------
//      ループフラグを設定します.
//-----------------------------------------------------------------------------
void SpriteAnimationPlayer::SetLoop(bool value)
{ m_Loop = value; }

//-----------------------------------------------------------------------------
//      ループフラグを取得します.
//-----------------------------------------------------------------------------
bool SpriteAnimationPlayer::IsLoop() const
{ return m_Loop; }

//-----------------------------------------------------------------------------
//      再生速度を設定します.
//-----------------------------------------------------------------------------
void SpriteAnimationPlayer::SetPlaySpeed(float value)
{ m_PlaySpeed = value; }

//-----------------------------------------------------------------------------
//      再生速度を取得します.
//-----------------------------------------------------------------------------
float SpriteAnimationPlayer::GetPlaySpeed() const
{ return m_PlaySpeed; }

//-----------------------------------------------------------------------------
//      一時停止フラグを設定します.
//-----------------------------------------------------------------------------
void SpriteAnimationPlayer::SetPause(bool value)
{ m_Pause = value; }

//-----------------------------------------------------------------------------
//      一時停止フラグを取得します.
//-----------------------------------------------------------------------------
bool SpriteAnimationPlayer::IsPause() const
{ return m_Pause; }

//-----------------------------------------------------------------------------
//      再生完了したかどうかチェックします.
//-----------------------------------------------------------------------------
bool SpriteAnimationPlayer::IsFinished() const
{
    if (!m_pTrack)
        return false;

    if (m_Loop)
        return false;

    return (m_TimeSec >= m_pTrack->DurationSec);
}

//-----------------------------------------------------------------------------
//      更新処理を行います.
//-----------------------------------------------------------------------------
void SpriteAnimationPlayer::Update(float deltaSec)
{
    if (!m_pTrack)
        return;

    if (m_Pause)
        return;

    // 時間を更新.
    m_TimeSec += deltaSec * m_PlaySpeed;

    // ループ設定の場合.
    if (m_Loop)
    {
        // 終了時刻を超えていたらフレーム番号をリセット.
        if (m_TimeSec >= m_pTrack->DurationSec)
        {
            m_FrameIndex = 0;
            m_TimeSec = Wrap(m_TimeSec, 0.0f, m_pTrack->DurationSec);
        }
    }

    // 最も近いフレームを検索し，フレーム番号を決定.
    m_FrameIndex = FindFrame(m_TimeSec);
}

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void SpriteAnimationPlayer::Draw(SpriteRenderer& renderer, int x, int y, int w, int h, uint8_t flag)
{
    if (m_pTrack == nullptr)
        return;

    auto uv0 = m_pTrack->Frames[m_FrameIndex].uv0;
    auto uv1 = m_pTrack->Frames[m_FrameIndex].uv1;

    // 水平方向に反転.
    if (!!(flag & Flag::FLIP_X))
    {
        auto u = uv0.x;
        uv0.x = uv1.x;
        uv1.x = u;
    }

    // 垂直方向に反転.
    if (!!(flag & Flag::FLIP_Y))
    {
        auto v = uv0.y;
        uv0.y = uv1.y;
        uv1.y = v;
    }

    renderer.Add(x, y, w, h, uv0, uv1);
}

//-----------------------------------------------------------------------------
//      フレーム番号を取得します.
//-----------------------------------------------------------------------------
uint32_t SpriteAnimationPlayer::GetFrameIndex() const
{ return m_FrameIndex; }

//-----------------------------------------------------------------------------
//      再生時間を取得します.
//-----------------------------------------------------------------------------
float SpriteAnimationPlayer::GetTimeSec() const
{ return m_TimeSec; }

//-----------------------------------------------------------------------------
//      フレーム先頭に戻します.
//-----------------------------------------------------------------------------
void SpriteAnimationPlayer::Cue()
{
    m_TimeSec    = 0.0f;
    m_FrameIndex = 0;
}

//-----------------------------------------------------------------------------
//      1フレーム進めます.
//-----------------------------------------------------------------------------
void SpriteAnimationPlayer::FrameAdvance()
{
    if (!m_pTrack)
        return;

    if (m_FrameIndex + 1 < m_pTrack->Frames.size())
        m_FrameIndex++;
    else if (m_Loop)
        m_FrameIndex = (m_FrameIndex + 1) % m_pTrack->Frames.size();
}

//-----------------------------------------------------------------------------
//      最も近いフレームを検索します.
//-----------------------------------------------------------------------------
uint32_t SpriteAnimationPlayer::FindFrame(float timeSec)
{
    assert(m_pTrack != nullptr);
    for(auto i=0u; i<m_pTrack->Frames.size(); ++i)
    {
        if (timeSec <= m_pTrack->Frames[i].time)
            return i;
    }

    return 0u;
}

} // namespace asdx

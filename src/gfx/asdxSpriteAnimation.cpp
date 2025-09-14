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
// SpriteAnimation class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
SpriteAnimation::SpriteAnimation()
: m_SpriteW   (0)
, m_SpriteH   (0)
, m_FrameIndex(0)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
SpriteAnimation::~SpriteAnimation()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
void SpriteAnimation::Init(int w, int h, const std::vector<Frame>& frames)
{
    m_SpriteW    = w;
    m_SpriteH    = h;
    m_FrameIndex = 0;

    m_Frames = frames;
    m_Frames.shrink_to_fit();
    assert(m_Frames.size() <= UINT32_MAX);
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void SpriteAnimation::Term()
{
    m_Frames.clear();
    m_Frames.shrink_to_fit();

    m_SpriteW    = 0;
    m_SpriteH    = 0;
    m_FrameIndex = 0;
}

//-----------------------------------------------------------------------------
//      次のフレームへ進めます.
//-----------------------------------------------------------------------------
void SpriteAnimation::NextFrame()
{
    auto size = uint32_t(m_Frames.size());
    m_FrameIndex = (m_FrameIndex + 1) % size;
}

//-----------------------------------------------------------------------------
//      時間に応じて次のフレームへ進めます.
//-----------------------------------------------------------------------------
void SpriteAnimation::NextFrameByTime(float changeSec, float deltaSec, float& elapedSec)
{
    auto time = elapedSec + deltaSec;
    if (time >= changeSec)
    {
        NextFrame();
        elapedSec = 0.0f;
    }
    else
    {
        elapedSec = time;
    }
}

//-----------------------------------------------------------------------------
//      スプライトを追加します.
//-----------------------------------------------------------------------------
void SpriteAnimation::Add(SpriteRenderer& renderer, int x, int y, int layer, uint8_t flag)
{
    assert(!m_Frames.empty());
    assert(m_FrameIndex <= m_Frames.size() - 1);
    auto uv0 = m_Frames[m_FrameIndex].uv0;
    auto uv1 = m_Frames[m_FrameIndex].uv1;

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

    renderer.Add(x, y, m_SpriteW, m_SpriteH, layer, uv0, uv1);
}

///////////////////////////////////////////////////////////////////////////////
// TimerSpriteAnimation class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
TimerSpriteAnimation::TimerSpriteAnimation()
: m_Animation   ()
, m_ChangeSec   (0.0f)
, m_ElapsedSec  (0.0f)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
TimerSpriteAnimation::~TimerSpriteAnimation()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
void TimerSpriteAnimation::Init(int w, int h, float changeSec, const std::vector<Frame>& frames)
{
    m_Animation.Init(w, h, frames);

    m_ChangeSec  = changeSec;
    m_ElapsedSec = 0.0f;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void TimerSpriteAnimation::Term()
{
    m_Animation.Term();

    m_ChangeSec  = 0.0f;
    m_ElapsedSec = 0.0f;
}

//-----------------------------------------------------------------------------
//      更新処理です.
//-----------------------------------------------------------------------------
void TimerSpriteAnimation::Update(float deltaSec)
{ m_Animation.NextFrameByTime(m_ChangeSec, deltaSec, m_ElapsedSec); }

//-----------------------------------------------------------------------------
//      スプライトを追加します.
//-----------------------------------------------------------------------------
void TimerSpriteAnimation::Add(SpriteRenderer& renderer, int x, int y, int layer, uint8_t flags)
{ m_Animation.Add(renderer, x, y, layer, flags); }

} // namespace asdx

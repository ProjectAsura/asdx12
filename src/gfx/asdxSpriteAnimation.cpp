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
void SpriteAnimation::Init(int w, int h, std::vector<Frame>&& frames)
{
    m_SpriteW    = w;
    m_SpriteH    = h;
    m_FrameIndex = 0;

    m_Frames = std::move(frames);
    m_Frames.shrink_to_fit();
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
void SpriteAnimation::Add(SpriteRenderer& renderer, int x, int y, int layer)
{
    assert(!m_Frames.empty());
    assert(m_FrameIndex <= m_Frames.size() - 1);
    const auto& frame = m_Frames[m_FrameIndex];
    renderer.Add(x, y, m_SpriteW, m_SpriteH, layer, frame.uv0, frame.uv1);
}

} // namespace asdx

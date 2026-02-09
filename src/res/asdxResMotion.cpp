//-----------------------------------------------------------------------------
// File : asdxResMotion.cpp
// Desc : Mootion Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxMacro.h>
#include <res/asdxResMotion.h>
#include "MotionBinary_generated.h"


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// MotionBinary class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
MotionBinary::MotionBinary()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
MotionBinary::~MotionBinary()
{ Term(); }

//-----------------------------------------------------------------------------
//      ロード処理を行います.
//-----------------------------------------------------------------------------
void MotionBinary::Load(std::vector<uint8_t>&& blob)
{
    m_Blob = std::move(blob);

#if ASDX_DEBUG
    // 整合性をチェック.
    {
        assert(!m_Blob.empty());
        flatbuffers::Verifier verifier(m_Blob.data(), m_Blob.size());
        assert(res::VerifySizePrefixedMotionBinaryBuffer(verifier));
        ASDX_UNUSED(verifier);
    }
#endif
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void MotionBinary::Term()
{
    m_Blob.clear();
    m_Blob.shrink_to_fit();
}

//-----------------------------------------------------------------------------
//      クリップ数を取得します.
//-----------------------------------------------------------------------------
uint32_t MotionBinary::GetClipCount() const
{
    assert(!m_Blob.empty());
    return res::GetMotionBinary(m_Blob.data())->Clips()->size();
}

//-----------------------------------------------------------------------------
//      クリップを取得します.
//-----------------------------------------------------------------------------
const res::MotionClip* MotionBinary::GetClip(uint32_t index) const
{
    assert(!m_Blob.empty());
    return res::GetMotionBinary(m_Blob.data())->Clips()->Get(index);
}

//-----------------------------------------------------------------------------
//      クリップを検索します.
//-----------------------------------------------------------------------------
const res::MotionClip* MotionBinary::FindClip(const char* name) const
{
    assert(!m_Blob.empty());
    return res::GetMotionBinary(m_Blob.data())->Clips()->LookupByKey(name);
}


///////////////////////////////////////////////////////////////////////////////
// MotionTrackProxy class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      トラック名を取得します.
//-----------------------------------------------------------------------------
const char* MotionTrackProxy::GetName(const res::MotionTrack* track)
{
    assert(track != nullptr);
    return track->Name()->c_str();
}

//-----------------------------------------------------------------------------
//      平行移動アニメーションキーフレームを検索します.
//-----------------------------------------------------------------------------
Vector3 MotionTrackProxy::FindTranslationKey(const res::MotionTrack* track, float timeSec)
{
    assert(track != nullptr);
    Vector3 result(0.0f, 0.0f, 0.0f);

    auto frame = track->Positions()->LookupByKey(timeSec);
    if (frame != nullptr)
    {
        result.x = frame->Value().X();
        result.y = frame->Value().Y();
        result.z = frame->Value().Z();
    }
    return result;
}

//-----------------------------------------------------------------------------
//      回転アニメーションキーフレームを検索します.
//-----------------------------------------------------------------------------
Quaternion MotionTrackProxy::FindRotationKey(const res::MotionTrack* track, float timeSec)
{
    assert(track != nullptr);
    Quaternion result(0.0f, 0.0f, 0.0f, 0.0f);

    auto frame = track->Rotations()->LookupByKey(timeSec);
    if (frame != nullptr)
    {
        result.x = frame->Value().X();
        result.y = frame->Value().Y();
        result.z = frame->Value().Z();
        result.w = frame->Value().W();
    }
    return result;
}

//-----------------------------------------------------------------------------
//      拡縮アニメーションキーフレームを検索します.
//-----------------------------------------------------------------------------
Vector3 MotionTrackProxy::FindScaleKey(const res::MotionTrack* track, float timeSec)
{
    assert(track != nullptr);
    Vector3 result(1.0f, 1.0f, 1.0f);

    auto frame = track->Scalings()->LookupByKey(timeSec);
    if (frame != nullptr)
    {
        result.x = frame->Value().X();
        result.y = frame->Value().Y();
        result.z = frame->Value().Z();
    }
    return result;
}

//-----------------------------------------------------------------------------
//      ローカル変換行列を求めます.
//-----------------------------------------------------------------------------
Matrix MotionTrackProxy::CalcLocalTransform(const res::MotionTrack* track, float timeSec)
{
    assert(track != nullptr);

    auto S = FindScaleKey      (track, timeSec);
    auto R = FindRotationKey   (track, timeSec);
    auto T = FindTranslationKey(track, timeSec);

    Matrix result;
    result  = Matrix::CreateScale(S);
    result *= Matrix::CreateFromQuaternion(R);
    result  = Matrix::AppendTranslation(result, T);
    return result;
}


///////////////////////////////////////////////////////////////////////////////
// MotionClipProxy class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      クリップ名を取得します.
//-----------------------------------------------------------------------------
const char* MotionClipProxy::GetName(const res::MotionClip* clip)
{
    assert(clip != nullptr);
    return clip->Name()->c_str();
}

//-----------------------------------------------------------------------------
//      アニメーション間隔を取得します.
//-----------------------------------------------------------------------------
float MotionClipProxy::GetDuration(const res::MotionClip* clip)
{
    assert(clip != nullptr);
    return clip->Duration();
}

//-----------------------------------------------------------------------------
//      1秒あたりの処理時間を取得します.
//-----------------------------------------------------------------------------
float MotionClipProxy::GetTicksPerSecond(const res::MotionClip* clip)
{
    assert(clip != nullptr);
    return clip->TicksPerSecond();
}

//-----------------------------------------------------------------------------
//      モーショントラック数を取得します.
//-----------------------------------------------------------------------------
uint32_t MotionClipProxy::GetTrackCount(const res::MotionClip* clip)
{
    assert(clip != nullptr);
    return clip->Tracks()->size();
}

//-----------------------------------------------------------------------------
//      モーショントラックを取得します.
//-----------------------------------------------------------------------------
const res::MotionTrack* MotionClipProxy::GetTrack(const res::MotionClip* clip, uint32_t index)
{
    assert(clip != nullptr);
    return clip->Tracks()->Get(index);
}

//-----------------------------------------------------------------------------
//      モーショントラックを検索します.
//-----------------------------------------------------------------------------
const res::MotionTrack* MotionClipProxy::FindTrack(const res::MotionClip* clip, const char* name)
{
    assert(clip != nullptr);
    return clip->Tracks()->LookupByKey(name);
}

} // namespace asdx

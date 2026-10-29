//-----------------------------------------------------------------------------
// File : asdxResMotion.cpp
// Desc : Mootion Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxMacro.h>
#include <fnd/asdxLogger.h>
#include <fnd/asdxHash.h>
#include <res/asdxResMotion.h>
#include "MotionBinary_generated.h"


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
const uint32_t CURRENT_VERSION = 2u;


//-----------------------------------------------------------------------------
//      Vector3型に変換します.
//-----------------------------------------------------------------------------
inline asdx::Vector3 ToVector3(const asdx::res::Float3& res)
{ return asdx::Vector3(res.X(), res.Y(), res.Z()); }

//-----------------------------------------------------------------------------
//      Quaternion型に変換します.
//-----------------------------------------------------------------------------
inline asdx::Quaternion ToQuaternion(const asdx::res::Quaternion& res)
{ return asdx::Quaternion(res.X(), res.Y(), res.Z(), res.W()); }

//-----------------------------------------------------------------------------
//      線形補間係数を計算します.
//-----------------------------------------------------------------------------
inline float CalcLerpFactor(float lhs, float rhs, float val)
{ return asdx::Saturate((val - lhs) / (rhs - lhs)); }

//-----------------------------------------------------------------------------
//      線形補間を行います.
//-----------------------------------------------------------------------------
inline asdx::Vector3 Lerp
(
    const asdx::res::KeyFloat3* lhs,
    const asdx::res::KeyFloat3* rhs,
    float time
)
{
    assert(lhs != nullptr);
    assert(rhs != nullptr);
    auto t = CalcLerpFactor(lhs->Time(), rhs->Time(), time);
    auto ret = asdx::Vector3::Lerp(
        ToVector3(lhs->Value()),
        ToVector3(rhs->Value()), t);
    return ret;
}

//-----------------------------------------------------------------------------
//      球面線形補間を行います.
//-----------------------------------------------------------------------------
inline asdx::Quaternion Lerp
(
    const asdx::res::KeyQuaternion* lhs,
    const asdx::res::KeyQuaternion* rhs,
    float time
)
{
    assert(lhs != nullptr);
    assert(rhs != nullptr);
    auto t = CalcLerpFactor(lhs->Time(), rhs->Time(), time);
    return asdx::Quaternion::Slerp(
        ToQuaternion(lhs->Value()),
        ToQuaternion(rhs->Value()), t);
}

//-----------------------------------------------------------------------------
//      指定された値より大きい値が現れる最初のインデックスを求めます.
//-----------------------------------------------------------------------------
template<typename T, typename U>
uint32_t LowerBound(T* items, uint32_t count, U key)
{
    auto lhs = 0u;
    auto rhs = count - 1;
    while(lhs < rhs)
    {
        auto mid = lhs + (rhs - lhs) / 2u;
        if (items->Get(mid)->Time() < key)
            lhs = mid + 1u;
        else
            rhs = mid;
    }

    return lhs;
}

//-----------------------------------------------------------------------------
//      指定された値より小さい値が現れる最初のインデックスを求めます.
//-----------------------------------------------------------------------------
template<typename T, typename U>
uint32_t UpperBound(T* items, uint32_t count, U key)
{
    auto lhs = 0u;
    auto rhs = count - 1;
    while(lhs < rhs)
    {
        auto mid = lhs + (rhs - lhs) / 2u;
        if (key < items->Get(mid)->Time())
            rhs = mid;
        else
            lhs = mid + 1;
    }

    return lhs;
}

//-----------------------------------------------------------------------------
//      フレームキーの値を計算します.
//-----------------------------------------------------------------------------
asdx::Vector3 CalcKeyValue(
    const flatbuffers::Vector<const asdx::res::KeyFloat3*>* items, float timeSec)
{
    assert(items != nullptr);

    // 値を一つしか持たないなら、それを返す.
    if (items->size() == 1)
        return ToVector3(items->Get(0)->Value());

    // 二分探索で最初に超える番号を求める.
    auto index = LowerBound(items, items->size(), timeSec);

    auto next = index;
    auto prev = (index == 0) ? (items->size() - 1) : index - 1;

    // 補間値を返す.
    return Lerp(items->Get(prev), items->Get(next), timeSec);
}

//-----------------------------------------------------------------------------
//      フレームキーの値を計算します.
//-----------------------------------------------------------------------------
asdx::Quaternion CalcKeyValue(
    const flatbuffers::Vector<const asdx::res::KeyQuaternion*>* items, float timeSec)
{
    assert(items != nullptr);

    // 値を一つしか持たないなら、それを返す.
    if (items->size() == 1)
        return ToQuaternion(items->Get(0)->Value());

    // 二分探索で最初に超える番号を求める.
    auto index = LowerBound(items, items->size(), timeSec);

    auto next = index;
    auto prev = (index == 0) ? (items->size() - 1) : index - 1;

    // 補間値を返す.
    return Lerp(items->Get(prev), items->Get(next), timeSec);
}

} // namespace

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
bool MotionBinary::Load(std::vector<uint8_t>&& blob)
{
    m_Blob = std::move(blob);

#if ASDX_DEBUG
    // 整合性をチェック.
    {
        assert(!m_Blob.empty());
        flatbuffers::Verifier verifier(m_Blob.data(), m_Blob.size());
        assert(res::VerifyMotionBinaryBuffer(verifier));
        ASDX_UNUSED(verifier);
    }
#endif

    // バージョンチェック.
    auto bin = res::GetMotionBinary(m_Blob.data());
    assert(bin->Version() == CURRENT_VERSION);
    if (bin->Version() != CURRENT_VERSION)
    {
        ELOGA("Error : Model Binary Version not match. expect = %u, loaded = %u", CURRENT_VERSION, bin->Version());
        return false;
    }

    return true;
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
    if (m_Blob.empty())
        return 0;

    return res::GetMotionBinary(m_Blob.data())->Clips()->size();
}

//-----------------------------------------------------------------------------
//      クリップを取得します.
//-----------------------------------------------------------------------------
const res::MotionClip* MotionBinary::GetClip(uint32_t index) const
{
    if (m_Blob.empty())
        return nullptr;

    return res::GetMotionBinary(m_Blob.data())->Clips()->Get(index);
}

//-----------------------------------------------------------------------------
//      ルート変換行列を取得します.
//-----------------------------------------------------------------------------
Matrix4x3 MotionBinary::GetRootTransform() const
{
    if (m_Blob.empty())
        return Matrix4x3::CreateIdentity();

    return *reinterpret_cast<const Matrix4x3*>(res::GetMotionBinary(m_Blob.data())->RootTransform());
}

//-----------------------------------------------------------------------------
//      クリップ名を検索します.
//-----------------------------------------------------------------------------
const res::MotionClip* MotionBinary::FindClip(const char* name) const
{
    auto hash = CalcHash(name);
    return FindClip(hash);
}

//-----------------------------------------------------------------------------
//      クリップを検索します.
//-----------------------------------------------------------------------------
const res::MotionClip* MotionBinary::FindClip(uint64_t hash) const
{
    if (m_Blob.empty())
        return nullptr;

    return res::GetMotionBinary(m_Blob.data())->Clips()->LookupByKey(hash);
}

//-----------------------------------------------------------------------------
//      名前を検索します.
//-----------------------------------------------------------------------------
StringView MotionBinary::FindName(uint64_t hash) const
{
    if (m_Blob.empty())
        return StringView();

    auto bin = res::GetMotionBinary(m_Blob.data());
    auto pair = bin->HashTable()->LookupByKey(hash);
    if (pair == nullptr)
        return StringView();

    return StringView(pair->Text()->c_str());
}


///////////////////////////////////////////////////////////////////////////////
// MotionTrackProxy class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      トラック名ハッシュを取得します.
//-----------------------------------------------------------------------------
uint64_t MotionTrackProxy::GetHash(const res::MotionTrack* track)
{
    assert(track != nullptr);
    return track->Hash();
}

//-----------------------------------------------------------------------------
//      平行移動アニメーションキーフレームを検索します.
//-----------------------------------------------------------------------------
Vector3 MotionTrackProxy::FindTranslationKey(const res::MotionTrack* track, float timeSec)
{
    assert(track != nullptr);
    Vector3 result(0.0f, 0.0f, 0.0f);
    auto positions = track->Positions();
    if (positions == nullptr)
        return result;

    // timeSec を超える最も近いフレームを検索.
    return CalcKeyValue(positions, timeSec);
}

//-----------------------------------------------------------------------------
//      回転アニメーションキーフレームを検索します.
//-----------------------------------------------------------------------------
Quaternion MotionTrackProxy::FindRotationKey(const res::MotionTrack* track, float timeSec)
{
    assert(track != nullptr);
    Quaternion result(0.0f, 0.0f, 0.0f, 0.0f);
    auto rots = track->Rotations();
    if (rots == nullptr)
        return result;

    // timeSec を超える最も近いフレームを検索.
    return CalcKeyValue(rots, timeSec);
}

//-----------------------------------------------------------------------------
//      拡縮アニメーションキーフレームを検索します.
//-----------------------------------------------------------------------------
Vector3 MotionTrackProxy::FindScaleKey(const res::MotionTrack* track, float timeSec)
{
    assert(track != nullptr);
    Vector3 result(1.0f, 1.0f, 1.0f);
    auto scales = track->Scalings();
    if (scales == nullptr)
        return result;

    // timeSec を超える最も近いフレームを検索.
    return CalcKeyValue(scales, timeSec);
}

//-----------------------------------------------------------------------------
//      ローカル変換行列を求めます.
//-----------------------------------------------------------------------------
Matrix4x3 MotionTrackProxy::FindLocalTransform(const res::MotionTrack* track, float timeSec)
{
    assert(track != nullptr);
    auto S = FindScaleKey(track, timeSec);
    auto R = FindRotationKey(track, timeSec);
    auto T = FindTranslationKey(track, timeSec);
    return CalcTransform(S, R, T);
}

//-----------------------------------------------------------------------------
//      スケールなしのローカル変換行列を求めます.
//-----------------------------------------------------------------------------
Matrix4x3 MotionTrackProxy::FindLocalTransformNoScale(const res::MotionTrack* track, float timeSec)
{
    assert(track != nullptr);
    auto R = FindRotationKey(track, timeSec);
    auto T = FindTranslationKey(track, timeSec);
    return CalcTransformNoScale(R, T);
}

//-----------------------------------------------------------------------------
//      変換行列を求めます.
//-----------------------------------------------------------------------------
Matrix4x3 MotionTrackProxy::CalcTransform(const Vector3& scale, const Quaternion& rotation, const Vector3& translation)
{
    Matrix4x3 result;
    result  = Matrix4x3::CreateScale(scale);
    result *= Matrix4x3::CreateFromQuaternion(rotation);
    result  = Matrix4x3::AppendTranslation(result, translation);
    return result;
}

//-----------------------------------------------------------------------------
//      変換行列をスケールなしで求めます.
//-----------------------------------------------------------------------------
Matrix4x3 MotionTrackProxy::CalcTransformNoScale(const Quaternion& rotation, const Vector3& translation)
{
    Matrix4x3 result;
    result = Matrix4x3::CreateFromQuaternion(rotation);
    result = Matrix4x3::AppendTranslation(result, translation);
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// MotionClipProxy class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      クリップ名を取得します.
//-----------------------------------------------------------------------------
uint64_t MotionClipProxy::GetHash(const res::MotionClip* clip)
{
    assert(clip != nullptr);
    return clip->Hash();
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
    auto hash = CalcHash(name);
    return FindTrack(clip, hash);
}

//-----------------------------------------------------------------------------
//      モーショントラックを検索します.
//-----------------------------------------------------------------------------
const res::MotionTrack* MotionClipProxy::FindTrack(const res::MotionClip* clip, uint64_t hash)
{
    assert(clip != nullptr);
    return clip->Tracks()->LookupByKey(hash);
}

} // namespace asdx

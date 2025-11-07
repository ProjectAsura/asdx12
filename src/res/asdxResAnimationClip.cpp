//-----------------------------------------------------------------------------
// File : asdxResAnimationClip.cpp
// Desc : Animation Clip Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxMacro.h>
#include <res/asdxResAnimationClip.h>
#include "AnimationClipBinary_generated.h"


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// AnimationClipBinary class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
AnimationClipBinary::AnimationClipBinary()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
AnimationClipBinary::~AnimationClipBinary()
{ Term(); }

//-----------------------------------------------------------------------------
//      ロード処理を行います.
//-----------------------------------------------------------------------------
void AnimationClipBinary::Load(std::vector<uint8_t>&& blob)
{
    m_Blob = std::move(blob);

#if ASDX_DEBUG
    // 整合性をチェック.
    {
        assert(!m_Blob.empty());
        flatbuffers::Verifier verifier(m_Blob.data(), m_Blob.size());
        assert(res::VerifySizePrefixedAnimationClipBinaryBuffer(verifier));
        ASDX_UNUSED(verifier);
    }
#endif
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void AnimationClipBinary::Term()
{
    m_Blob.clear();
    m_Blob.shrink_to_fit();
}

//-----------------------------------------------------------------------------
//      クリップ数を取得します.
//-----------------------------------------------------------------------------
uint32_t AnimationClipBinary::GetClipCount() const
{
    assert(!m_Blob.empty());
    return res::GetAnimationClipBinary(m_Blob.data())->Clips()->size();
}

//-----------------------------------------------------------------------------
//      クリップを取得します.
//-----------------------------------------------------------------------------
const res::AnimationClip* AnimationClipBinary::GetClip(uint32_t index) const
{
    assert(!m_Blob.empty());
    return res::GetAnimationClipBinary(m_Blob.data())->Clips()->Get(index);
}

//-----------------------------------------------------------------------------
//      クリップを検索します.
//-----------------------------------------------------------------------------
const res::AnimationClip* AnimationClipBinary::FindClip(const char* name) const
{
    assert(!m_Blob.empty());
    return res::GetAnimationClipBinary(m_Blob.data())->Clips()->LookupByKey(name);
}

//=============================================================================
// Functions.
//=============================================================================

//-----------------------------------------------------------------------------
//      ボーン名を取得します.
//-----------------------------------------------------------------------------
const char* GetBoneName(const res::BoneAnimation* boneAnim)
{
    assert(boneAnim != nullptr);
    return boneAnim->BoneName()->c_str();
}

//-----------------------------------------------------------------------------
//      平行移動アニメーションキーフレームを検索します.
//-----------------------------------------------------------------------------
Vector3 FindTranslationKey(const res::BoneAnimation* boneAnim, float timeSec)
{
    assert(boneAnim != nullptr);
    Vector3 result(0.0f, 0.0f, 0.0f);

    auto frame = boneAnim->Positions()->LookupByKey(timeSec);
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
Quaternion FindRotationKey(const res::BoneAnimation* boneAnim, float timeSec)
{
    assert(boneAnim != nullptr);
    Quaternion result(0.0f, 0.0f, 0.0f, 0.0f);

    auto frame = boneAnim->Rotations()->LookupByKey(timeSec);
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
Vector3 FindScaleKey(const res::BoneAnimation* boneAnim, float timeSec)
{
    assert(boneAnim != nullptr);
    Vector3 result(1.0f, 1.0f, 1.0f);

    auto frame = boneAnim->Scalings()->LookupByKey(timeSec);
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
Matrix ComputeLocalTransform(const res::BoneAnimation* boneAnim, float timeSec)
{
    assert(boneAnim != nullptr);

    auto S = FindScaleKey      (boneAnim, timeSec);
    auto R = FindRotationKey   (boneAnim, timeSec);
    auto T = FindTranslationKey(boneAnim, timeSec);

    Matrix result;
    result  = Matrix::CreateScale(S);
    result *= Matrix::CreateFromQuaternion(R);
    result  = Matrix::AppendTranslation(result, T);
    return result;
}

//-----------------------------------------------------------------------------
//      クリップ名を取得します.
//-----------------------------------------------------------------------------
const char* GetClipName(const res::AnimationClip* clip)
{
    assert(clip != nullptr);
    return clip->Name()->c_str();
}

//-----------------------------------------------------------------------------
//      アニメーション間隔を取得します.
//-----------------------------------------------------------------------------
float GetDuration(const res::AnimationClip* clip)
{
    assert(clip != nullptr);
    return clip->Duration();
}

//-----------------------------------------------------------------------------
//      1秒あたりの処理時間を取得します.
//-----------------------------------------------------------------------------
float GetTicksPerSecond(const res::AnimationClip* clip)
{
    assert(clip != nullptr);
    return clip->TicksPerSecond();
}

//-----------------------------------------------------------------------------
//      ボーンアニメーション数を取得します.
//-----------------------------------------------------------------------------
uint32_t GetBoneAnimationCount(const res::AnimationClip* clip)
{
    assert(clip != nullptr);
    return clip->Bones()->size();
}

//-----------------------------------------------------------------------------
//      ボーンアニメーションを取得します.
//-----------------------------------------------------------------------------
const res::BoneAnimation* GetBoneAnimation(const res::AnimationClip* clip, uint32_t index)
{
    assert(clip != nullptr);
    return clip->Bones()->Get(index);
}

//-----------------------------------------------------------------------------
//      ボーンアニメーションを検索します.
//-----------------------------------------------------------------------------
const res::BoneAnimation* FindBoneAnimation(const res::AnimationClip* clip, const char* name)
{
    assert(clip != nullptr);
    return clip->Bones()->LookupByKey(name);
}

} // namespace asdx

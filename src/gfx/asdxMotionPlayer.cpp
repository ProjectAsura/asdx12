//-----------------------------------------------------------------------------
// File: asdxMotionPlayer.cpp
// Desc : Motion Player.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include <fnd/asdxMisc.h>
#include <gfx/asdxMotionPlayer.h>
#include <gfx/asdxModel.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// MotionUpdater class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
MotionUpdater::MotionUpdater()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
MotionUpdater::~MotionUpdater()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool MotionUpdater::Init(const Model* pModel, const res::MotionClip* pClip)
{
    if (pModel == nullptr || pClip == nullptr)
        return false;

    Term();

    m_pClip = pClip;

    auto count = pModel->GetBoneCount();
    m_pTracks.resize(count);

    for(auto i=0u; i<count; ++i)
    {
        auto& bone = pModel->GetBone(i);
        auto  name = asdx::BoneProxy::GetName(bone);

        auto track = asdx::MotionClipProxy::FindTrack(m_pClip, name.c_str());
        m_pTracks[i] = track;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void MotionUpdater::Term()
{
    m_pClip = nullptr;
    m_pTracks.clear();
    m_pTracks.shrink_to_fit();
}

//-----------------------------------------------------------------------------
//      更新処理を行います.
//-----------------------------------------------------------------------------
bool MotionUpdater::Update(float deltaSec)
{
    if (m_pClip == nullptr || m_pTracks.empty())
        return false;

    // 一時停止中なら処理しない.
    if (m_Pause)
        return false;

    // 計測時間を取得.
    auto duration = asdx::MotionClipProxy::GetDuration(m_pClip);

    // 1秒あたりのtick
    auto tps = asdx::MotionClipProxy::GetTicksPerSecond(m_pClip);

    // 前フレームの時間を一時保存.
    auto prevTime = m_TimeInTicks;

    // 加算時間.
    auto addTime = deltaSec * tps * m_PlaySpeed;

    if (duration > 0.0f)
    {
        // 現在時間を更新.
        if (m_Loop)
            m_TimeInTicks = Wrap(m_TimeInTicks + addTime, 0.0f, duration);
        else
            m_TimeInTicks = Clamp(m_TimeInTicks + addTime, 0.0f, duration);
    }
    else
    {
        m_TimeInTicks = 0.0f;
    }

    return (m_TimeInTicks >= duration);
}

//-----------------------------------------------------------------------------
//      トラックがあるかチェックします.
//-----------------------------------------------------------------------------
bool MotionUpdater::HasTrack(uint32_t index) const
{
    if (m_pTracks.empty())
        return false;

    assert(index < m_pTracks.size());
    return m_pTracks[index] != nullptr;
}

//-----------------------------------------------------------------------------
//      スケールキーを取得します.
//-----------------------------------------------------------------------------
Vector3 MotionUpdater::GetScaleKey(uint32_t index) const
{
    assert(index < m_pTracks.size());
    assert(m_pTracks[index] != nullptr);
    return MotionTrackProxy::FindScaleKey(m_pTracks[index], m_TimeInTicks);
}

//-----------------------------------------------------------------------------
//      回転キーを取得します.
//-----------------------------------------------------------------------------
Quaternion MotionUpdater::GetRotationKey(uint32_t index) const
{
    assert(index < m_pTracks.size());
    assert(m_pTracks[index] != nullptr);
    return MotionTrackProxy::FindRotationKey(m_pTracks[index], m_TimeInTicks);
}

//-----------------------------------------------------------------------------
//      平行移動キーを取得します.
//-----------------------------------------------------------------------------
Vector3 MotionUpdater::GetTranslationKey(uint32_t index) const
{
    assert(index < m_pTracks.size());
    assert(m_pTracks[index] != nullptr);
    return MotionTrackProxy::FindTranslationKey(m_pTracks[index], m_TimeInTicks);
}

//-----------------------------------------------------------------------------
//      現在時間を設定します.
//-----------------------------------------------------------------------------
void MotionUpdater::SetTimeInTicks(float value)
{ m_TimeInTicks = value; }

//-----------------------------------------------------------------------------
//      再生速度を設定します.
//-----------------------------------------------------------------------------
void MotionUpdater::SetPlaySpeed(float value)
{ m_PlaySpeed = value; }

//-----------------------------------------------------------------------------
//      ループ再生フラグを設定します.
//-----------------------------------------------------------------------------
void MotionUpdater::SetLoop(bool value)
{ m_Loop = value; }

//-----------------------------------------------------------------------------
//      一時停止フラグを設定します.
//-----------------------------------------------------------------------------
void MotionUpdater::SetPause(bool value)
{ m_Pause = value; }

//-----------------------------------------------------------------------------
//      再生速度を設定します.
//-----------------------------------------------------------------------------
float MotionUpdater::GetPlaySpeed() const
{ return m_PlaySpeed; }

//-----------------------------------------------------------------------------
//      Tick あたりの時間を取得します.
//-----------------------------------------------------------------------------
float MotionUpdater::GetTimeInTicks() const
{ return m_TimeInTicks; }

//-----------------------------------------------------------------------------
//      1秒あたりの Tick を取得します.
//-----------------------------------------------------------------------------
float MotionUpdater::GetTickPerSecond() const
{
    if (m_pClip == nullptr)
        return 0.0f;

    return MotionClipProxy::GetTicksPerSecond(m_pClip); }

//-----------------------------------------------------------------------------
//      再生所要時間を取得します.
//-----------------------------------------------------------------------------
float MotionUpdater::GetDuration() const
{
    if (m_pClip == nullptr)
        return 0.0f;

    return MotionClipProxy::GetDuration(m_pClip);
}

//-----------------------------------------------------------------------------
//      ループ再生フラグを取得します.
//-----------------------------------------------------------------------------
bool MotionUpdater::IsLoop() const
{ return m_Loop; }

//-----------------------------------------------------------------------------
//      一時停止フラグを取得します.
//-----------------------------------------------------------------------------
bool MotionUpdater::IsPause() const
{ return m_Pause; }


///////////////////////////////////////////////////////////////////////////////
// MotionPlayer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
MotionPlayer::MotionPlayer()
: m_pModel(nullptr)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
MotionPlayer::~MotionPlayer()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
void MotionPlayer::Init(const Model* pModel)
{
    assert(pModel != nullptr);
    m_pModel = pModel;

    auto count = m_pModel->GetBoneCount();
    m_LocalTransforms.resize(count);
    m_WorldTransforms.resize(count);
    m_MatrixPalettes .resize(count);

    auto identity = Transform4x3::CreateIdentity();
    for(auto i=0u; i<count; ++i)
    {
        auto& bone     = m_pModel->GetBone(i);
        auto  bindPose = asdx::BoneProxy::GetBindPoseMatrix(bone);

        m_LocalTransforms[i] = bindPose;
        m_WorldTransforms[i] = identity;
        m_MatrixPalettes [i] = identity;
    }

    UpdateWorldTransform(identity);
    UpdateMatrixPalette();

    m_Finish = false;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void MotionPlayer::Term()
{
    m_pModel = nullptr;
    m_Finish = false;

    for(auto i=0; i<2; ++i)
    { m_Updater[i].Term(); }

    m_LocalTransforms.clear();
    m_WorldTransforms.clear();
    m_MatrixPalettes .clear();

    m_FinishListener.clear();
}

//-----------------------------------------------------------------------------
//      モーションクリップを設定します.
//-----------------------------------------------------------------------------
void MotionPlayer::SetClip(const res::MotionClip* pClip)
{
    if (m_pModel == nullptr)
        return;

    if (pClip == nullptr)
        m_Updater[m_CurrIndex].Term();
    else
        m_Updater[m_CurrIndex].Init(m_pModel, pClip);

    // 単位行列で初期化.
    auto count    = m_pModel->GetBoneCount();
    auto identity = Transform4x3::CreateIdentity();

    for(auto i=0u; i<count; ++i)
    {
        auto& bone     = m_pModel->GetBone(i);
        auto  bindPose = asdx::BoneProxy::GetBindPoseMatrix(bone);
        auto  name     = asdx::BoneProxy::GetName(bone);

        m_LocalTransforms[i] = bindPose;
        m_WorldTransforms[i] = identity;
        m_MatrixPalettes [i] = identity;
    }

    UpdateWorldTransform(identity);
    UpdateMatrixPalette();
}

//-----------------------------------------------------------------------------
//      次のクリップを設定します.
//-----------------------------------------------------------------------------
void MotionPlayer::NextClip
(
    const res::MotionClip*  pClip,
    bool                    loop,
    float                   transitionSec,
    bool                    timeInheritance
)
{
    if (m_pModel == nullptr)
        return;

    auto& next = m_Updater[m_NextIndex];
    next.Init(m_pModel, pClip);
    next.SetLoop(loop);
    next.SetPause(false);

    auto currDuration = GetDuration();
    if (timeInheritance && currDuration > 0.0f)
    {
        // 正規化した時間を求める.
        auto unitTime = GetTimeInTicks() / currDuration;

        // 次のクリップに合わせて，スケールし直す.
        auto timeInTicks = unitTime * next.GetDuration();

        // 変換した時間を設定.
        next.SetTimeInTicks(timeInTicks);
    }
    else
    {
        // 先頭から再生.
        next.SetTimeInTicks(0.0f);
    }

    m_BlendDuration = transitionSec;
    m_BlendDelta    = 0.0f;
}

//-----------------------------------------------------------------------------
//      更新処理を行います.
//-----------------------------------------------------------------------------
void MotionPlayer::Update(float deltaSec, const Transform4x3& rootTransform)
{
    if (m_pModel == nullptr)
        return;

    if (m_Updater[m_CurrIndex].IsPause())
        return;

    // モーションを更新.
    auto prevFinish = m_Finish;
    m_Finish = m_Updater[m_CurrIndex].Update(deltaSec);

    auto blend = (m_BlendDuration > 0.0f) && (m_BlendDelta <= m_BlendDuration);
    if (blend)
    {
        // ブレンド対象モーションを更新.
        m_Updater[m_NextIndex].Update(deltaSec);

        // ブレンド経過時間を加算.
        m_BlendDelta += deltaSec;
    }
    else if ((m_BlendDuration > 0.0f) && (m_BlendDelta > m_BlendDuration))
    {
        // 時間をリセット.
        m_BlendDelta    = 0.0f;
        m_BlendDuration = 0.0f;

        // アップデータ番号入れ替え.
        auto index = m_CurrIndex;
        m_CurrIndex = m_NextIndex;
        m_NextIndex = index;

        // 解放処理.
        m_Updater[m_NextIndex].Term();
    }

    // 骨を動かす.
    UpdateLocalTransform(blend);

    // ワールド行列を更新.
    UpdateWorldTransform(rootTransform);

    // 行列パレットを更新.
    UpdateMatrixPalette();

    if (m_Finish && !prevFinish)
    {
        for(auto& listner : m_FinishListener)
        { listner->OnExecute(*this); }
    }
}

//-----------------------------------------------------------------------------
//      ローカル変換行列を更新します.
//-----------------------------------------------------------------------------
void MotionPlayer::UpdateLocalTransform(bool blend)
{
    auto count = m_pModel->GetBoneCount();

    // アニメーションブレンド.
    if (blend)
    {
        const auto& curr = m_Updater[m_CurrIndex];
        const auto& next = m_Updater[m_NextIndex];

        auto blendWeight = Saturate(m_BlendDelta / m_BlendDuration);

        for(auto i=0u; i<count; ++i)
        {
            auto hasCurrTrack = curr.HasTrack(i);
            auto hasNextTrack = next.HasTrack(i);

            Vector3     S;
            Quaternion  R;
            Vector3     T;

            if (hasCurrTrack && hasNextTrack)
            {
                auto S0 = curr.GetScaleKey(i);
                auto R0 = curr.GetRotationKey(i);
                auto T0 = curr.GetTranslationKey(i);

                auto S1 = next.GetScaleKey(i);
                auto R1 = next.GetRotationKey(i);
                auto T1 = next.GetTranslationKey(i);

                S = Vector3::Lerp(S0, S1, blendWeight);
                R = Quaternion::Slerp(R0, R1, blendWeight);
                T = Vector3::Lerp(T0, T1, blendWeight);
            }
            else if (hasCurrTrack)
            {
                S = curr.GetScaleKey(i);
                R = curr.GetRotationKey(i);
                T = curr.GetTranslationKey(i);
            }
            else if (hasNextTrack)
            {
                S = next.GetScaleKey(i);
                R = next.GetRotationKey(i);
                T = next.GetTranslationKey(i);
            }
            else
            {
                continue;
            }

            // ローカル変換行列を計算.
            m_LocalTransforms[i] = asdx::MotionTrackProxy::CalcTransform(S, R, T);
        }
    }
    else
    {
        const auto& updater = m_Updater[m_CurrIndex];
        for(auto i=0u; i<count; ++i)
        {
            if (!updater.HasTrack(i))
                continue;

            auto S = updater.GetScaleKey(i);
            auto R = updater.GetRotationKey(i);
            auto T = updater.GetTranslationKey(i);

            // ローカル変換行列を計算.
            m_LocalTransforms[i] = asdx::MotionTrackProxy::CalcTransform(S, R, T);
        }
    }
}

//-----------------------------------------------------------------------------
//      ワールド行列を更新します.
//-----------------------------------------------------------------------------
void MotionPlayer::UpdateWorldTransform(const Transform4x3& rootTransform)
{
    auto count = m_pModel->GetBoneCount();
    for(auto i=0u; i<count; ++i)
    {
        const auto& bone = m_pModel->GetBone(i);
        const auto parentId = asdx::BoneProxy::GetParentId(bone);

        if (parentId >= 0)
        {
            // 親がいれば親を考慮.
            m_WorldTransforms[i] = m_LocalTransforms[i] * m_WorldTransforms[parentId];
        }
        else
        {
            // 親がいなければそのまま.
            m_WorldTransforms[i] = m_LocalTransforms[i] * rootTransform;
        }
    }
}

//-----------------------------------------------------------------------------
//      行列パレットを更新します.
//-----------------------------------------------------------------------------
void MotionPlayer::UpdateMatrixPalette()
{
    auto count = m_pModel->GetBoneCount();
    for(auto i=0u; i<count; ++i)
    {
        const auto& bone = m_pModel->GetBone(i);
        const auto invBindPose = asdx::BoneProxy::GetInverseBindPoseMatrix(bone);
        m_MatrixPalettes[i] = invBindPose * m_WorldTransforms[i];
    }
}

//-----------------------------------------------------------------------------
//      ローカル変換行列を取得します.
//-----------------------------------------------------------------------------
const std::vector<Transform4x3>& MotionPlayer::GetLocalTransforms() const
{ return m_LocalTransforms; }

//-----------------------------------------------------------------------------
//      ワールド変換行列を取得します.
//-----------------------------------------------------------------------------
const std::vector<Transform4x3>& MotionPlayer::GetWorldTransforms() const
{ return m_WorldTransforms; }

//-----------------------------------------------------------------------------
//      行列パレットを取得します.
//-----------------------------------------------------------------------------
const std::vector<Transform4x3>& MotionPlayer::GetMatrixPalettes() const
{ return m_MatrixPalettes; }

//-----------------------------------------------------------------------------
//      現在の再生時間を取得します.
//-----------------------------------------------------------------------------
float MotionPlayer::GetTimeInTicks() const
{ return m_Updater[m_CurrIndex].GetTimeInTicks(); }

//-----------------------------------------------------------------------------
//      再生所要時間を取得します.
//-----------------------------------------------------------------------------
float MotionPlayer::GetDuration() const
{ return m_Updater[m_CurrIndex].GetDuration(); }

//-----------------------------------------------------------------------------
//      1秒あたりのTick数を取得します.
//-----------------------------------------------------------------------------
float MotionPlayer::GetTicksPerSecond() const
{ return m_Updater[m_CurrIndex].GetTickPerSecond(); }

//-----------------------------------------------------------------------------
//      ループ再生フラグを取得します.
//-----------------------------------------------------------------------------
bool MotionPlayer::IsLoop() const
{ return m_Updater[m_CurrIndex].IsLoop(); }

//-----------------------------------------------------------------------------
//      ループ再生フラグを設定します.
//-----------------------------------------------------------------------------
void MotionPlayer::SetLoop(bool value)
{ m_Updater[m_CurrIndex].SetLoop(value); }

//-----------------------------------------------------------------------------
//      再生スピードを設定します.
//-----------------------------------------------------------------------------
void MotionPlayer::SetPlaySpeed(float value)
{ m_Updater[m_CurrIndex].SetPlaySpeed(value); }

//-----------------------------------------------------------------------------
//      再生スピードを取得します.
//-----------------------------------------------------------------------------
float MotionPlayer::GetPlaySpeed() const
{ return m_Updater[m_CurrIndex].GetPlaySpeed(); }

//-----------------------------------------------------------------------------
//      一時停止フラグを取得します.
//-----------------------------------------------------------------------------
bool MotionPlayer::IsPause() const
{ return m_Updater[m_CurrIndex].IsPause(); }

//-----------------------------------------------------------------------------
//      一時停止フラグを設定します.
//-----------------------------------------------------------------------------
void MotionPlayer::SetPause(bool value)
{ m_Updater[m_CurrIndex].SetPause(value); }

//-----------------------------------------------------------------------------
//      フレーム先頭に戻します.
//-----------------------------------------------------------------------------
void MotionPlayer::Cue()
{ m_Updater[m_CurrIndex].SetTimeInTicks(0.0f); }

//-----------------------------------------------------------------------------
//      1フレーム進めます.
//-----------------------------------------------------------------------------
void MotionPlayer::FrameAdvance(const Transform4x3& rootTransform)
{
    if (m_pModel == nullptr)
        return;

    // 60 FPS として計算.
    auto oneFrame = 1.0f / 60.0f;

    SetPause(false);
    Update(oneFrame, rootTransform);
    SetPause(true);
}

//-----------------------------------------------------------------------------
//      再生完了イベントリスナーを登録します.
//-----------------------------------------------------------------------------
void MotionPlayer::AddFinishListener(IMotionEventListener* listener)
{
    auto itr = std::find(m_FinishListener.begin(), m_FinishListener.end(), listener);
    if (itr == m_FinishListener.end())
    { m_FinishListener.push_back(listener); }
}

//-----------------------------------------------------------------------------
//      再生完了イベントリスナーの登録を解除します.
//-----------------------------------------------------------------------------
void MotionPlayer::RemoveFinishListener(IMotionEventListener* listener)
{
    auto itr = std::find(m_FinishListener.begin(), m_FinishListener.end(), listener);
    if (itr != m_FinishListener.end())
    { m_FinishListener.erase(itr); }
}

} // namespace asdx

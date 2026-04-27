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
// MotionPlayer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
MotionPlayer::MotionPlayer()
: m_pModel      (nullptr)
, m_TimeInTicks (0.0f)
, m_pMotionClip (nullptr)
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

    m_TimeInTicks = 0.0f;
    m_pMotionClip = nullptr;

    auto count = m_pModel->GetBoneCount();
    m_LocalTransforms.resize(count);
    m_WorldTransforms.resize(count);
    m_MatrixPalettes .resize(count);
    m_Tracks         .resize(count);

    auto identity = Matrix::CreateIdentity();
    for(auto i=0u; i<count; ++i)
    {
        auto& bone     = m_pModel->GetBone(i);
        auto  bindPose = asdx::BoneProxy::GetBindPoseMatrix(bone);

        m_LocalTransforms[i] = bindPose;
        m_WorldTransforms[i] = identity;
        m_MatrixPalettes [i] = identity;
        m_Tracks         [i] = nullptr;
    }

    UpdateWorldTransform(identity);
    UpdateMatrixPalette();
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void MotionPlayer::Term()
{
    m_pModel        = nullptr;
    m_pMotionClip   = nullptr;
    m_TimeInTicks   = 0.0f;
    m_LocalTransforms.clear();
    m_WorldTransforms.clear();
    m_MatrixPalettes .clear();
}

//-----------------------------------------------------------------------------
//      モーションクリップを設定します.
//-----------------------------------------------------------------------------
void MotionPlayer::SetClip(const res::MotionClip* pClip)
{
    if (m_pModel == nullptr)
        return;

    m_pMotionClip = pClip;
    m_TimeInTicks = 0.0f;

    // 単位行列で初期化.
    auto count    = m_pModel->GetBoneCount();
    auto identity = Matrix::CreateIdentity();

    for(auto i=0u; i<count; ++i)
    {
        auto& bone     = m_pModel->GetBone(i);
        auto  bindPose = asdx::BoneProxy::GetBindPoseMatrix(bone);
        auto  name     = asdx::BoneProxy::GetName(bone);

        if (m_pMotionClip)
        {
            auto track = asdx::MotionClipProxy::FindTrack(m_pMotionClip, name.c_str());
            m_Tracks[i] = track;
        }
        else
        {
            m_Tracks[i] = nullptr;
        }

        m_LocalTransforms[i] = bindPose;
        m_WorldTransforms[i] = identity;
        m_MatrixPalettes [i] = identity;
    }

    UpdateWorldTransform(identity);
    UpdateMatrixPalette();
}

//-----------------------------------------------------------------------------
//      更新処理を行います.
//-----------------------------------------------------------------------------
void MotionPlayer::Update(float deltaSec, const Matrix& rootTransform)
{
    if (m_pModel == nullptr || m_pMotionClip == nullptr)
        return;

    if (m_Pause)
        return;

    // 骨を動かす.
    UpdateLocalTransform(deltaSec);

    // ワールド行列を更新.
    UpdateWorldTransform(rootTransform);

    // 行列パレットを更新.
    UpdateMatrixPalette();
}

//-----------------------------------------------------------------------------
//      ローカル変換行列を更新します.
//-----------------------------------------------------------------------------
void MotionPlayer::UpdateLocalTransform(float deltaSec)
{
    // 計測時間を取得.
    auto duration = asdx::MotionClipProxy::GetDuration(m_pMotionClip);

    // 1秒あたりのtick
    auto tps = asdx::MotionClipProxy::GetTicksPerSecond(m_pMotionClip);

    // 前フレームの時間を一時保存.
    auto prevTime = m_TimeInTicks;

    // 加算時間.
    auto addTime = deltaSec * tps * m_PlaySpeed;

    // 現在時間を更新.
    if (m_Loop)
        m_TimeInTicks = Wrap(m_TimeInTicks + addTime, 0.0f, duration);
    else
        m_TimeInTicks = Clamp(m_TimeInTicks + addTime, 0.0f, duration);

    auto count = m_pModel->GetBoneCount();
    for(auto i=0u; i<count; ++i)
    {
        const auto& bone = m_pModel->GetBone(i);
        const auto track = m_Tracks[i];

        // アニメーションデータが無ければバインドポーズを適用.
        if (track == nullptr)
            continue;

        // ローカル変換行列を計算.
        m_LocalTransforms[i] = asdx::MotionTrackProxy::CalcLocalTransform(track, m_TimeInTicks);
    }
}

//-----------------------------------------------------------------------------
//      ワールド行列を更新します.
//-----------------------------------------------------------------------------
void MotionPlayer::UpdateWorldTransform(const Matrix& rootTransform)
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
const std::vector<Matrix>& MotionPlayer::GetLocalTransforms() const
{ return m_LocalTransforms; }

//-----------------------------------------------------------------------------
//      ワールド変換行列を取得します.
//-----------------------------------------------------------------------------
const std::vector<Matrix>& MotionPlayer::GetWorldTransforms() const
{ return m_WorldTransforms; }

//-----------------------------------------------------------------------------
//      行列パレットを取得します.
//-----------------------------------------------------------------------------
const std::vector<Matrix>& MotionPlayer::GetMatrixPalettes() const
{ return m_MatrixPalettes; }

//-----------------------------------------------------------------------------
//      現在の再生時間を取得します.
//-----------------------------------------------------------------------------
float MotionPlayer::GetTimeInTicks() const
{ return m_TimeInTicks; }

//-----------------------------------------------------------------------------
//      再生所要時間を取得します.
//-----------------------------------------------------------------------------
float MotionPlayer::GetDuration() const
{
    if (m_pMotionClip == nullptr)
        return 0.0f;
    return asdx::MotionClipProxy::GetDuration(m_pMotionClip);
}

//-----------------------------------------------------------------------------
//      1秒あたりのTick数を取得します.
//-----------------------------------------------------------------------------
float MotionPlayer::GetTicksPerSecond() const
{
    if (m_pMotionClip == nullptr)
        return 0.0f;
    return asdx::MotionClipProxy::GetTicksPerSecond(m_pMotionClip);
}

//-----------------------------------------------------------------------------
//      ループ再生フラグを取得します.
//-----------------------------------------------------------------------------
bool MotionPlayer::IsLoop() const
{ return m_Loop; }

//-----------------------------------------------------------------------------
//      ループ再生フラグを設定します.
//-----------------------------------------------------------------------------
void MotionPlayer::SetLoop(bool value)
{ m_Loop = value; }

//-----------------------------------------------------------------------------
//      再生スピードを設定します.
//-----------------------------------------------------------------------------
void MotionPlayer::SetPlaySpeed(float value)
{ m_PlaySpeed = value; }

//-----------------------------------------------------------------------------
//      再生スピードを取得します.
//-----------------------------------------------------------------------------
float MotionPlayer::GetPlaySpeed() const
{ return m_PlaySpeed; }

//-----------------------------------------------------------------------------
//      一時停止フラグを取得します.
//-----------------------------------------------------------------------------
bool MotionPlayer::IsPause() const
{ return m_Pause; }

//-----------------------------------------------------------------------------
//      一時停止フラグを設定します.
//-----------------------------------------------------------------------------
void MotionPlayer::SetPause(bool value)
{ m_Pause = value; }

//-----------------------------------------------------------------------------
//      フレーム先頭に戻します.
//-----------------------------------------------------------------------------
void MotionPlayer::Cue()
{ m_TimeInTicks = 0.0f; }

//-----------------------------------------------------------------------------
//      1フレーム進めます.
//-----------------------------------------------------------------------------
void MotionPlayer::FrameAdvance(const Matrix& rootTransform)
{
    if (m_pModel == nullptr || m_pMotionClip == nullptr)
        return;

    // 60 FPS として計算.
    auto oneFrame = 1.0f / 60.0f;

    m_Pause = false;
    Update(oneFrame, rootTransform);
    m_Pause = true;
}

} // namespace asdx

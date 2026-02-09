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
, m_CurrentTime (0.0f)
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

    m_CurrentTime = 0.0f;
    m_pMotionClip = nullptr;

    auto count = m_pModel->GetBoneCount();
    m_LocalTransforms.resize(count);
    m_WorldTransforms.resize(count);
    m_MatrixPalettes .resize(count);

    auto identity = Matrix::CreateIdentity();
    for(auto i=0u; i<count; ++i)
    {
        auto& bone     = m_pModel->GetBone(i);
        auto  bindPose = asdx::BoneProxy::GetBindPoseMatrix(bone);

        m_LocalTransforms[i] = bindPose;
        m_WorldTransforms[i] = identity;
        m_MatrixPalettes [i] = identity;
    }
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void MotionPlayer::Term()
{
    m_pModel        = nullptr;
    m_pMotionClip   = nullptr;
    m_CurrentTime   = 0.0f;
    m_LocalTransforms.clear();
    m_WorldTransforms.clear();
    m_MatrixPalettes .clear();
}

//-----------------------------------------------------------------------------
//      モーションクリップを設定します.
//-----------------------------------------------------------------------------
void MotionPlayer::SetClip(const res::MotionClip* pClip)
{
    if (m_pModel == nullptr || pClip == nullptr)
    {
        ELOG("Error : Invalid Arguments.");
        return;
    }

    m_pMotionClip = pClip;
    m_CurrentTime = 0.0f;

    // 単位行列で初期化.
    auto count    = m_pModel->GetBoneCount();
    auto identity = Matrix::CreateIdentity();
    for(auto i=0u; i<count; ++i)
    {
        auto& bone     = m_pModel->GetBone(i);
        auto  bindPose = asdx::BoneProxy::GetBindPoseMatrix(bone);

        m_LocalTransforms[i] = bindPose;
        m_WorldTransforms[i] = identity;
        m_MatrixPalettes [i] = identity;
    }

    auto trackCount = asdx::MotionClipProxy::GetTrackCount(m_pMotionClip);
    m_TrackIds.resize(trackCount);
    for(auto i=0u; i<trackCount; ++i)
    {
        auto track  = asdx::MotionClipProxy::GetTrack(m_pMotionClip, i);
        auto name   = asdx::MotionTrackProxy::GetName(track);
        auto boneId = 0u;

        if (m_pModel->FindBone(name, boneId))
            m_TrackIds[i] = boneId;
        else
            m_TrackIds[i] = UINT32_MAX;
    }
}

//-----------------------------------------------------------------------------
//      更新処理を行います.
//-----------------------------------------------------------------------------
void MotionPlayer::Update(float deltaSec, const Matrix& rootTransform)
{
    if (m_pModel == nullptr || m_pMotionClip == nullptr)
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

    // 前フレームの時間を一時保存.
    auto prevTime = m_CurrentTime;

    // 現在時間を更新.
    m_CurrentTime = Wrap(m_CurrentTime + deltaSec, 0.0f, duration);

    // ループ時の処理.
    if (m_CurrentTime < prevTime)
    {
        // ポーズを初期化.
        auto count = uint32_t(m_LocalTransforms.size());
        for(auto i=0u; i<count; ++i)
        {
            auto& bone     = m_pModel->GetBone(i);
            auto  bindPose = asdx::BoneProxy::GetBindPoseMatrix(bone);
            m_LocalTransforms[i] = bindPose;
        }
    }

    // アニメーション処理..
    {
        auto count = asdx::MotionClipProxy::GetTrackCount(m_pMotionClip);
        for(auto i=0u; i<count; ++i)
        {
            // ボーンIDを取得.
            auto boneId = m_TrackIds[i];
            if (boneId == UINT32_MAX)
                continue;

            // アニメーションデータを取得.
            auto anim = asdx::MotionClipProxy::GetTrack(m_pMotionClip, i);

            // ローカル変換行列を計算.
            m_LocalTransforms[boneId] = asdx::MotionTrackProxy::CalcLocalTransform(anim, m_CurrentTime);
        }
    }
}

//-----------------------------------------------------------------------------
//      ワールド行列を更新します.
//-----------------------------------------------------------------------------
void MotionPlayer::UpdateWorldTransform(const Matrix& rootTransform)
{
    // ルートボーンのワールド行列を計算.
    m_WorldTransforms[0] = m_LocalTransforms[0] * rootTransform;

    // 子ボーンのワールド行列を計算.
    auto count = uint32_t(m_WorldTransforms.size());
    for(auto i=1u; i<count; ++i)
    {
        const auto& bone = m_pModel->GetBone(i);
        auto parent = asdx::BoneProxy::GetParentId(bone);

        // 親がいれば親を考慮.
        if (parent >= 0)
            m_WorldTransforms[i] = m_LocalTransforms[i] * m_WorldTransforms[parent];
        // 親がいなければそのまま.
        else
            m_WorldTransforms[i] = m_LocalTransforms[i];
    }
}

//-----------------------------------------------------------------------------
//      行列パレットを更新します.
//-----------------------------------------------------------------------------
void MotionPlayer::UpdateMatrixPalette()
{
    auto count = uint32_t(m_MatrixPalettes.size());
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

} // namespace asdx

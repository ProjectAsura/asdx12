//-----------------------------------------------------------------------------
// File : MotionConverter.cpp
// Desc : Motion (*.mob) Converter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdio>
#include <MotionConverter.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <MotionBinary_generated.h>

#ifndef ELOG
#define ELOG(x, ...) fprintf_s(stderr, "[File:%s, Line:%d] " x "\n", __FILE__, __LINE__, ##__VA_ARGS__ )
#endif//ELOG


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t CURRENT_VERION = 1u;  //!< 現在サポートされているファイルバージョン.


//-----------------------------------------------------------------------------
//      Float3を持つキーフレームデータに変換します.
//-----------------------------------------------------------------------------
asdx::res::KeyFloat3 ToKeyFloat3(const aiVectorKey& value)
{
    return asdx::res::KeyFloat3(
        float(value.mTime),
        asdx::res::Float3(value.mValue.x, value.mValue.y, value.mValue.z));
}

//-----------------------------------------------------------------------------
//      Quaternionを持つキーフレームデータに変換します.
//-----------------------------------------------------------------------------
asdx::res::KeyQuaternion ToKeyQuaternion(const aiQuatKey& value)
{
    return asdx::res::KeyQuaternion(
        float(value.mTime),
        asdx::res::Quaternion(value.mValue.x, value.mValue.y, value.mValue.z, value.mValue.w));
}

//-----------------------------------------------------------------------------
//      Float4x4に変換します.
//-----------------------------------------------------------------------------
asdx::res::Float4x4 ToFloat4x4(const aiMatrix4x4& value)
{
    return asdx::res::Float4x4(
        value.a1, value.b1, value.c1, value.d1,
        value.a2, value.b2, value.c2, value.d2,
        value.a3, value.b3, value.c3, value.d3,
        value.a4, value.b4, value.c4, value.d4);
}

} // namespace


///////////////////////////////////////////////////////////////////////////////
// MotionConverter class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      現在のバイナリバージョンを取得します.
//-----------------------------------------------------------------------------
uint32_t MotionConverter::GetCurrentVersion()
{ return CURRENT_VERION; }

//-----------------------------------------------------------------------------
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool MotionConverter::Convert(const Desc& desc)
{
    // 引数チェック.
    if (desc.InputPath.empty() || desc.OutputPath.empty())
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    // 変換処理.
    std::vector<uint8_t> binary;
    if (!Convert(desc.InputPath, binary))
    {
        ELOG("Error : Convert Failed.");
        return false;
    }

    // バイナリファイルに出力.
    {
        FILE* fp = nullptr;
        auto err = fopen_s(&fp, desc.OutputPath.c_str(), "wb");
        if (err != 0)
        {
            ELOG("Error : File Open Failed. path = %s, errcode = 0x%x", desc.OutputPath.c_str(), err);
            return false;
        }

        fwrite(binary.data(), binary.size(), 1, fp);
        fclose(fp);
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool MotionConverter::Convert(const std::string& path, std::vector<uint8_t>& binary)
{
    if (path.empty())
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    int flag = 0;
    flag |= aiProcessPreset_TargetRealtime_MaxQuality;
    flag |= aiProcess_FlipUVs;
    flag |= aiProcess_FlipWindingOrder;

    // ファイル読み込み.
    Assimp::Importer importer;
    auto pScene = importer.ReadFile(path.c_str(), flag);

    if (pScene == nullptr)
    {
        ELOG("Error : Importer::ReadFile() Failed. path = %s", path.c_str());
        return false;
    }

    // アニメーションデータがあるかどうかチェック.
    if (!pScene->HasAnimations())
    {
        ELOG("Error : Not Found Animation Data. path = %s", path.c_str());
        return false;
    }

    flatbuffers::FlatBufferBuilder builder(1024);

    // データを変換.
    std::vector<flatbuffers::Offset<asdx::res::MotionClip>> clips;
    for(auto i=0u; i<pScene->mNumAnimations; ++i)
    {
        const auto srcAnim = pScene->mAnimations[i];

        std::string name = srcAnim->mName.C_Str();
        if (name == "" || name.empty())
        {
            name = "Clip" + std::to_string(i);
        }

        auto duration       = float(srcAnim->mDuration);
        auto ticksPerSecond = float(srcAnim->mTicksPerSecond);

        // ゼロの場合は gltf の値に合わせる.
        if (ticksPerSecond <= 0.0f)
        { ticksPerSecond = 1000.0f; }

        // ボーンアニメーションを変換.
        std::vector<flatbuffers::Offset<asdx::res::MotionTrack>> bones;
        for(auto j=0u; j<srcAnim->mNumChannels; ++j)
        {
            const auto srcCh = srcAnim->mChannels[j];

            std::vector<asdx::res::KeyFloat3>       positions;
            std::vector<asdx::res::KeyQuaternion>   rotations;
            std::vector<asdx::res::KeyFloat3>       scalings;

            positions.resize(srcCh->mNumPositionKeys);
            rotations.resize(srcCh->mNumRotationKeys);
            scalings .resize(srcCh->mNumScalingKeys);

            // 位置キーフレームデータを変換.
            for(auto k=0u; k<srcCh->mNumPositionKeys; ++k)
            {
                const auto& srcKey = srcCh->mPositionKeys[k];
                positions[k] = ToKeyFloat3(srcKey);
            }

            // 回転キーフレームデータを変換.
            for(auto k=0u; k<srcCh->mNumRotationKeys; ++k)
            {
                const auto& srcKey = srcCh->mRotationKeys[k];
                rotations[k] = ToKeyQuaternion(srcKey);
            }

            // 拡縮キーフレームデータを変換.
            for(auto k=0u; k<srcCh->mNumScalingKeys; ++k)
            {
                const auto& srcKey = srcCh->mScalingKeys[k];
                scalings[k] = ToKeyFloat3(srcKey);
            }

            // ボーンアニメーションを生成.
            auto bone = asdx::res::CreateMotionTrackDirect(
                builder,
                srcCh->mNodeName.C_Str(),
                &positions,
                &rotations,
                &scalings);

            // 追加.
            bones.emplace_back(bone);
        }

        // アニメーションクリップ生成.
        auto clip = asdx::res::CreateMotionClipDirect(
            builder,
            name.c_str(),
            duration,
            ticksPerSecond,
            &bones);

        // 追加.
        clips.emplace_back(clip);
    }

    asdx::res::Float4x4 rootTransform;
    if (pScene->mRootNode != nullptr)
    {
        rootTransform = ToFloat4x4(pScene->mRootNode->mTransformation);
    }
    else
    {
        rootTransform = asdx::res::Float4x4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);
    }

    // バイナリ生成.
    auto bin = asdx::res::CreateMotionBinaryDirect(
        builder,
        CURRENT_VERION,
        &rootTransform,
        &clips);

    // バイナリ作成完了.
    builder.Finish(bin);

    // コピー
    binary.resize(builder.GetSize());
    memcpy(binary.data(), builder.GetBufferPointer(), builder.GetSize());

    pScene = nullptr;

    // 正常終了.
    return true;
}

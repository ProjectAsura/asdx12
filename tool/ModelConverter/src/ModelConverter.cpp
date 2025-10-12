//-----------------------------------------------------------------------------
// File : ModelConverter.cpp
// Desc : Model Converter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdio>
#include <map>
#include <fnd/asdxMath.h>
#include <ModelConverter.h>
#include <mikktspace.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <ModelBinary_generated.h>


#ifndef ELOG
#define ELOG(x, ...) fprintf_s(stderr, "[File:%s, Line:%d] " x "\n", __FILE__, __LINE__, ##__VA_ARGS__ )
#endif//ELOG

namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t CURRENT_VERION = 1u;  //!< 現在サポートされているバージョン.

///////////////////////////////////////////////////////////////////////////////
// BoneInfo structure
///////////////////////////////////////////////////////////////////////////////
struct BoneInfo
{
    uint32_t        Index;          // Bone Index.
    std::string     Name;           // Name of Bone.
    asdx::Matrix    OffsetMatrix;   // Inverse Bind Pose Matrix.
};

//-----------------------------------------------------------------------------
//      Unorm4 に変換します.
//-----------------------------------------------------------------------------
asdx::res::Unorm4 ToUnorm4(const aiColor4D& color)
{
    auto r = uint8_t(color.r * 255.0f);
    auto g = uint8_t(color.g * 255.0f);
    auto b = uint8_t(color.b * 255.0f);
    auto a = uint8_t(color.a * 255.0f);
    return asdx::res::Unorm4(r, g, b, a);
}

//-----------------------------------------------------------------------------
//      Float4x4 に変換します.
//-----------------------------------------------------------------------------
asdx::res::Float4x4 ToFloat4x4(const asdx::Matrix& matrix)
{
    return asdx::res::Float4x4(
        matrix._11, matrix._12, matrix._13, matrix._14,
        matrix._21, matrix._22, matrix._23, matrix._24,
        matrix._31, matrix._32, matrix._33, matrix._34,
        matrix._41, matrix._42, matrix._43, matrix._44);
}

//-----------------------------------------------------------------------------
//      メッシュを解析します.
//-----------------------------------------------------------------------------
void ParseMesh
(
    flatbuffers::FlatBufferBuilder&         builder,
    std::map<std::string, BoneInfo>&        boneMap,
    const char*                             materialTag,
    flatbuffers::Offset<asdx::res::Mesh>&   dstMesh,
    const aiMesh*                           srcMesh
)
{
    std::vector<asdx::res::Float3> positions;
    std::vector<asdx::res::Float3> normals;
    std::vector<asdx::res::Float2> texcoords;
    std::vector<asdx::res::Unorm4> colors;
    std::vector<asdx::res::Uint4>  boneIndices;
    std::vector<asdx::res::Float4> boneWeights;

    positions.resize(srcMesh->mNumVertices);
    normals  .resize(srcMesh->mNumVertices);
    texcoords.resize(srcMesh->mNumVertices);
    colors   .resize(srcMesh->mNumVertices);

    const aiVector3D kZero(0.0f, 0.0f, 0.0f);
    const aiColor4D  kWhite(1.0f, 1.0f, 1.0f, 1.0f);

    for(auto i=0u; i<srcMesh->mNumVertices; ++i)
    {
        auto pos = srcMesh->mVertices[i];
        auto nrm = srcMesh->mNormals[i];
        auto tex = srcMesh->mTextureCoords[0][i];
        auto col = srcMesh->HasVertexColors(0) ? srcMesh->mColors[0][i] : kWhite;

        positions[i] = asdx::res::Float3(pos.x, pos.y, pos.z);
        normals  [i] = asdx::res::Float3(nrm.x, nrm.y, nrm.z);
        texcoords[i] = asdx::res::Float2(tex.x, tex.y);
        colors   [i] = ToUnorm4(col);
    }

    std::vector<uint32_t> vertexIndices;
    vertexIndices.resize(srcMesh->mNumFaces * 3);
    for(auto i=0u; i<srcMesh->mNumFaces; ++i)
    {
        const auto& face = srcMesh->mFaces[i];
        assert(face.mNumIndices == 3);

        vertexIndices[i * 3 + 0] = face.mIndices[0];
        vertexIndices[i * 3 + 1] = face.mIndices[1];
        vertexIndices[i * 3 + 2] = face.mIndices[2];
    }

    if (srcMesh->HasBones())
    {
        boneIndices.resize(srcMesh->mNumVertices);
        boneWeights.resize(srcMesh->mNumVertices);

        for(auto i=0u; i<srcMesh->mNumBones; ++i)
        {
            auto bone = srcMesh->mBones[i];
            auto boneName = std::string(bone->mName.C_Str());

            uint32_t boneId = 0;
            if (boneMap.find(boneName) == std::end(boneMap))
            {
                boneId = uint32_t(boneMap.size());
                BoneInfo info;
                info.Index = boneId;
                info.Name  = boneName;
                info.OffsetMatrix = asdx::Matrix(
                    bone->mOffsetMatrix.a1,
                    bone->mOffsetMatrix.a2,
                    bone->mOffsetMatrix.a3,
                    bone->mOffsetMatrix.a4,

                    bone->mOffsetMatrix.b1,
                    bone->mOffsetMatrix.b2,
                    bone->mOffsetMatrix.b3,
                    bone->mOffsetMatrix.b4,

                    bone->mOffsetMatrix.c1,
                    bone->mOffsetMatrix.c2,
                    bone->mOffsetMatrix.c3,
                    bone->mOffsetMatrix.c4,

                    bone->mOffsetMatrix.d1,
                    bone->mOffsetMatrix.d2,
                    bone->mOffsetMatrix.d3,
                    bone->mOffsetMatrix.d4);

                boneMap[boneName] = info;
            }
            else
            {
                boneId = boneMap[boneName].Index;
            }

            for(auto j=0u; j<bone->mNumWeights; ++j)
            {
                auto vertId = bone->mWeights[j].mVertexId;
                auto weight = bone->mWeights[j].mWeight;

                auto fx = boneWeights[i].X();
                auto fy = boneWeights[i].Y();
                auto fz = boneWeights[i].Z();
                auto fw = boneWeights[i].W();

                auto ix = boneIndices[i].X();
                auto iy = boneIndices[i].Y();
                auto iz = boneIndices[i].Z();
                auto iw = boneIndices[i].W();

                // 値が埋められていない箇所を調べる.
                auto processed = false;
                if (fx == 0.0f)
                {
                    fx = weight;
                    ix = boneId;
                    processed = true;
                }
                else if (fy == 0.0f)
                {
                    fy = weight;
                    iy = boneId;
                    processed = true;
                }
                else if (fz == 0.0f)
                {
                    fz = weight;
                    iz = boneId;
                    processed = true;
                }
                else if (fw == 0.0f)
                {
                    fw = weight;
                    iw = boneId;
                    processed = true;
                }

                // 値を埋められたら終了.
                if (processed)
                {
                    boneWeights[vertId] = asdx::res::Float4(fx, fy, fz, fw);
                    boneIndices[vertId] = asdx::res::Uint4(ix, iy, iz, iw);
                    continue;
                }

                // 最もウェイトが小さいものを求める.
                auto lowestIndex  = -1;
                auto lowestWeight = weight;
                if (fx < lowestWeight)
                {
                    lowestIndex  = 0;
                    lowestWeight = fx;
                }
                if (fy < lowestWeight)
                {
                    lowestIndex  = 1;
                    lowestWeight = fy;
                }
                if (fz < lowestWeight)
                {
                    lowestIndex  = 2;
                    lowestWeight = fz;
                }
                if (fw < lowestWeight)
                {
                    lowestIndex  = 3;
                    lowestWeight = fz;
                }

                // 一番小さいものが見つからなかった場合は終了.
                if (lowestIndex == -1)
                    continue;

                // 一番小さいものと入れ替える.
                switch(lowestIndex)
                {
                case 0:
                    {
                        fx = weight;
                        ix = boneId;
                    }
                    break;

                case 1:
                    {
                        fy = weight;
                        iy = boneId;
                    }
                    break;

                case 2:
                    {
                        fz = weight;
                        iz = boneId;
                    }
                    break;

                case 3:
                    {
                        fw = weight;
                        iw = boneId;
                    }
                    break;
                }

                boneWeights[vertId] = asdx::res::Float4(fx, fy, fz, fw);
                boneIndices[vertId] = asdx::res::Uint4(ix, iy, iz, iw);
            }
        }
    }

    std::vector<asdx::res::Float4> tangents;
    {
        tangents.resize(srcMesh->mNumVertices);

        struct CalcInfo
        {
            std::vector<uint32_t>&          VertexIndices;
            std::vector<asdx::res::Float3>& Positions;
            std::vector<asdx::res::Float4>& Tangents;
            std::vector<asdx::res::Float3>& Normals;
            std::vector<asdx::res::Float2>& TexCoords;
        };
        CalcInfo info = { vertexIndices, positions, tangents, normals, texcoords };

        auto GetFaceCount = [](const SMikkTSpaceContext* context) -> int
        {
            auto info = reinterpret_cast<CalcInfo*>(context->m_pUserData);
            return int(info->VertexIndices.size() / 3);
        };

        auto GetVertexCountOfFace = [](const SMikkTSpaceContext* context, int) -> int
        {
            return 3;
        };

        auto GetPosition = [](const SMikkTSpaceContext* context, float outPos[], const int faceIndex, const int vertexIndex)
        {
            auto info = reinterpret_cast<CalcInfo*>(context->m_pUserData);
            auto idx  = info->VertexIndices[faceIndex * 3 + vertexIndex];

            outPos[0] = info->Positions[idx].X();
            outPos[1] = info->Positions[idx].Y();
            outPos[2] = info->Positions[idx].Z();
        };

        auto GetNormal = [](const SMikkTSpaceContext* context, float outNormal[], const int faceIndex, const int vertexIndex)
        {
            auto info = reinterpret_cast<CalcInfo*>(context->m_pUserData);
            auto idx  = info->VertexIndices[faceIndex * 3 + vertexIndex];

            outNormal[0] = info->Normals[idx].X();
            outNormal[1] = info->Normals[idx].Y();
            outNormal[2] = info->Normals[idx].Z();
        };

        auto GetTexCoord = [](const SMikkTSpaceContext* context, float outUv[], const int faceIndex, const int vertexIndex)
        {
            auto info = reinterpret_cast<CalcInfo*>(context->m_pUserData);
            auto idx  = info->VertexIndices[faceIndex * 3 + vertexIndex];

            outUv[0] = info->TexCoords[idx].X();
            outUv[1] = info->TexCoords[idx].Y();
        };

        auto SetTSpaceBasic = [](const SMikkTSpaceContext* context, const float tangent[], const float binormalSign, const int faceIndex, const int vertexIndex)
        {
            auto info = reinterpret_cast<CalcInfo*>(context->m_pUserData);
            auto idx  = info->VertexIndices[faceIndex * 3 + vertexIndex];

            info->Tangents[idx] = asdx::res::Float4(tangent[0], tangent[1], tangent[2], binormalSign);
        };

        SMikkTSpaceInterface iface = {};
        iface.m_getNumFaces          = GetFaceCount;
        iface.m_getNumVerticesOfFace = GetVertexCountOfFace;
        iface.m_getPosition          = GetPosition;
        iface.m_getNormal            = GetNormal;
        iface.m_getTexCoord          = GetTexCoord;
        iface.m_setTSpaceBasic       = SetTSpaceBasic;

        SMikkTSpaceContext context = {};
        context.m_pInterface = &iface;
        context.m_pUserData  = &info;
        [[maybe_unused]] auto ret = genTangSpaceDefault(&context);
        assert(ret == 1);
    }

    dstMesh = asdx::res::CreateMeshDirect(
        builder,
        srcMesh->mName.C_Str(),
        materialTag,
        &positions,
        &normals,
        &tangents,
        &colors,
        &texcoords,
        &boneWeights,
        &boneIndices,
        &vertexIndices);
}

} // namespace

///////////////////////////////////////////////////////////////////////////////
// ModelConverter class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool ModelConverter::Convert(const Desc& desc)
{
    if (desc.InputPath.empty() || desc.OutputPath.empty())
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    int flag = 0;
    flag |= aiProcess_Triangulate;
    flag |= aiProcess_PreTransformVertices;
    flag |= aiProcess_GenSmoothNormals;
    flag |= aiProcess_GenUVCoords;
    flag |= aiProcess_RemoveRedundantMaterials;
    flag |= aiProcess_OptimizeMeshes;
    flag |= aiProcess_LimitBoneWeights;

    Assimp::Importer importer;
    auto pScene = importer.ReadFile(desc.InputPath.c_str(), flag);

    if (pScene == nullptr)
    {
        ELOG("Error : Importer::ReadFile() Failed. path = %s", desc.InputPath.c_str());
        return false;
    }

    flatbuffers::FlatBufferBuilder builder(1024);

    // メッシュデータを変換.
    std::vector<flatbuffers::Offset<asdx::res::Mesh>> meshes;
    std::map<std::string, BoneInfo> boneMap;
    meshes.resize(pScene->mNumMeshes);
    for(auto i=0u; i<pScene->mNumMeshes; ++i)
    {
        const auto srcMesh = pScene->mMeshes[i];
        auto& dstMesh = meshes[i];

        auto materialTag = pScene->mMaterials[srcMesh->mMaterialIndex]->GetName().C_Str();

        ParseMesh(builder, boneMap, materialTag, dstMesh, srcMesh);
    }

    // ボーンデータを変換.
    std::vector<flatbuffers::Offset<asdx::res::Bone>> bones;
    bones.reserve(boneMap.size());
    for(auto& itr : boneMap)
    {
        auto mtx = ToFloat4x4(itr.second.OffsetMatrix);
        auto bone = asdx::res::CreateBoneDirect(
            builder,
            itr.second.Name.c_str(),
            &mtx);
        bones.emplace_back(bone);
    }

    auto rootMtx = asdx::Matrix(
        pScene->mRootNode->mTransformation.a1,
        pScene->mRootNode->mTransformation.a2,
        pScene->mRootNode->mTransformation.a3,
        pScene->mRootNode->mTransformation.a4,
        pScene->mRootNode->mTransformation.b1,
        pScene->mRootNode->mTransformation.b2,
        pScene->mRootNode->mTransformation.b3,
        pScene->mRootNode->mTransformation.b4,
        pScene->mRootNode->mTransformation.c1,
        pScene->mRootNode->mTransformation.c2,
        pScene->mRootNode->mTransformation.c3,
        pScene->mRootNode->mTransformation.c4,
        pScene->mRootNode->mTransformation.d1,
        pScene->mRootNode->mTransformation.d2,
        pScene->mRootNode->mTransformation.d3,
        pScene->mRootNode->mTransformation.d4);
    auto invRootMtx = asdx::Matrix::Invert(rootMtx);

    auto resRootMtx    = ToFloat4x4(rootMtx);
    auto resInvRootMtx = ToFloat4x4(invRootMtx);

    auto bin = asdx::res::CreateModelBinaryDirect(
        builder,
        CURRENT_VERION,
        &meshes,
        &bones,
        &resRootMtx,
        &resInvRootMtx);

    builder.Finish(bin);

    // バイナリファイルに出力.
    {
        auto buffer     = builder.GetBufferPointer();
        auto bufferSize = builder.GetSize();

        FILE* fp = nullptr;
        auto err = fopen_s(&fp, desc.OutputPath.c_str(), "wb");
        if (err != 0)
        {
            ELOG("Error : Output File Open Failed. path = %s", desc.OutputPath.c_str());
            return false;
        }

        fwrite(buffer, bufferSize, 1, fp);
        fclose(fp);
    }

    pScene = nullptr;

    return true;
}

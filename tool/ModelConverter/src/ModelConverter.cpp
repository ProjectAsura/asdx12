//-----------------------------------------------------------------------------
// File : ModelConverter.cpp
// Desc : Model Binary (*.mdb) Converter.
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
static constexpr uint32_t CURRENT_VERSION = 1u;  //!< 現在サポートされているバージョン.


///////////////////////////////////////////////////////////////////////////////
// BoneInfo structure
///////////////////////////////////////////////////////////////////////////////
struct BoneInfo
{
    std::string     Name;                                               // Name of Bone.
    std::string     ParentName;                                         // Name of Parent Bone.
    uint32_t        Index           = 0;                                // Bone Index.
    asdx::Matrix    BindPose        = asdx::Matrix::CreateIdentity();   // Bind Pose Matrix.
    asdx::Matrix    InverseBindPose = asdx::Matrix::CreateIdentity();   // Inverse Bind Pose Matrix.
    std::vector<std::string> ChildrenName;
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
//      Matrix に変換します.
//-----------------------------------------------------------------------------
asdx::Matrix ToMatrix(const aiMatrix4x4& matrix)
{
    return asdx::Matrix(
        matrix.a1, matrix.b1, matrix.c1, matrix.d1,
        matrix.a2, matrix.b2, matrix.c2, matrix.d2,
        matrix.a3, matrix.b3, matrix.c3, matrix.d3,
        matrix.a4, matrix.b4, matrix.c4, matrix.d4);
}

//-----------------------------------------------------------------------------
//      親ボーンを検索します.
//-----------------------------------------------------------------------------
const aiBone* FindParentBone(const aiNode* node, aiBone** const pBones, uint32_t numBones)
{
    if (node == nullptr)
        return nullptr;

    // 親ノードが存在しなければ終了.
    if (node->mParent == nullptr)
        return nullptr;

    for(auto i=0u; i<numBones; ++i)
    {
        // 一致するものが見つかれば終了.
        if (node->mParent->mName == pBones[i]->mName)
            return pBones[i];
    }

    // 親を再帰的に辿っていく.
    return FindParentBone(node->mParent, pBones, numBones);
}

//-----------------------------------------------------------------------------
//      メッシュを解析します.
//-----------------------------------------------------------------------------
void ParseMesh
(
    flatbuffers::FlatBufferBuilder&         builder,
    std::map<std::string, BoneInfo>&        boneMap,
    const aiNode*                           rootNode,
    flatbuffers::Offset<asdx::res::Mesh>&   dstMesh,
    const aiMesh*                           srcMesh,
    asdx::BoundingSphere3&                  boundSphere
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
        auto tex = srcMesh->HasTextureCoords(0) ? srcMesh->mTextureCoords[0][i] : kZero;
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

            auto node   = rootNode->findBoneNode(bone);
            auto parent = FindParentBone(node, srcMesh->mBones, srcMesh->mNumBones);

            uint32_t boneId = 0;

            auto itr = boneMap.find(boneName);
            if (itr == std::end(boneMap))
            {
                boneId = int(boneMap.size());
                BoneInfo info;
                info.Index = boneId;
                info.Name  = boneName;

                if (parent != nullptr)
                {
                    info.ParentName = std::string(parent->mName.C_Str());
                }

                if (node && node->mNumChildren > 0)
                {
                    info.ChildrenName.resize(node->mNumChildren);
                    for(auto i=0u; i<node->mNumChildren; ++i)
                    {
                        info.ChildrenName[i] = std::string(node->mChildren[i]->mName.C_Str());
                    }
                }

                // バインドポーズ行列.
                auto bindPose = bone->mOffsetMatrix;    // いったんコピーしないと書き変わってしまうため.
                bindPose.Inverse();
                info.BindPose = ToMatrix(bindPose);

                // バインドポーズ逆行列.
                info.InverseBindPose = ToMatrix(bone->mOffsetMatrix);

                boneMap[boneName] = info;
            }
            else
            {
                boneId = itr->second.Index;
            }

            for(auto j=0u; j<bone->mNumWeights; ++j)
            {
                auto vertId = bone->mWeights[j].mVertexId;
                auto weight = bone->mWeights[j].mWeight;

                auto fx = boneWeights[vertId].X();
                auto fy = boneWeights[vertId].Y();
                auto fz = boneWeights[vertId].Z();
                auto fw = boneWeights[vertId].W();

                auto ix = boneIndices[vertId].X();
                auto iy = boneIndices[vertId].Y();
                auto iz = boneIndices[vertId].Z();
                auto iw = boneIndices[vertId].W();

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

                // 正規化
                auto mag = sqrtf((fx * fx) + (fy * fy) + (fz * fz) + (fw * fw));
                if (mag > 0.0f)
                {
                    fx /= mag;
                    fy /= mag;
                    fz /= mag;
                    fw /= mag;
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

    auto sphere = asdx::BoundingSphere3::Create(&srcMesh->mVertices[0].x, srcMesh->mNumVertices, sizeof(aiVector3D));
    boundSphere = asdx::BoundingSphere3::Merge(boundSphere, sphere);

    auto bounds = asdx::res::BoundingSphere(asdx::res::Float3(sphere.Center.x, sphere.Center.y, sphere.Center.z), sphere.Radius);

    dstMesh = asdx::res::CreateMeshDirect(
        builder,
        srcMesh->mName.C_Str(),
        srcMesh->mMaterialIndex,
        &positions,
        &normals,
        &tangents,
        &colors,
        &texcoords,
        &boneWeights,
        &boneIndices,
        &vertexIndices,
        &bounds);
}

} // namespace

namespace asdx {

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

    std::vector<uint8_t> binary;
    if (!Convert(desc.InputPath.c_str(), binary))
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
            ELOG("Error : Output File Open Failed. path = %s", desc.OutputPath.c_str());
            return false;
        }

        fwrite(binary.data(), binary.size(), 1, fp);
        fclose(fp);
    }

    return true;
}

//-----------------------------------------------------------------------------
//      変換処理を行います.
//-----------------------------------------------------------------------------
bool ModelConverter::Convert(const std::string& path, std::vector<uint8_t>& binary)
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

    Assimp::Importer importer;
    auto pScene = importer.ReadFile(path.c_str(), flag);

    if (pScene == nullptr)
    {
        ELOG("Error : Importer::ReadFile() Failed. path = %s", path.c_str());
        return false;
    }

    BoundingSphere3 bounds(Vector3(0.0f, 0.0f, 0.0f), 0.0f);

    flatbuffers::FlatBufferBuilder builder(1024);

    // メッシュデータを変換.
    std::vector<flatbuffers::Offset<asdx::res::Mesh>> meshes;
    std::map<std::string, BoneInfo> boneMap;
    meshes.resize(pScene->mNumMeshes);
    for(auto i=0u; i<pScene->mNumMeshes; ++i)
    {
        const auto srcMesh = pScene->mMeshes[i];
        auto& dstMesh = meshes[i];

        ParseMesh(builder, boneMap, pScene->mRootNode, dstMesh, srcMesh, bounds);
    }

    // ボーンデータを変換.
    std::vector<flatbuffers::Offset<asdx::res::Bone>> bones;
    bones.resize(boneMap.size());
    for(auto& srcBone : boneMap)
    {
        int parentId = -1;
        if (!srcBone.second.ParentName.empty())
        {
            auto itr = boneMap.find(srcBone.second.ParentName);
            if (itr != boneMap.end())
            { parentId = int(itr->second.Index); }
        }

        std::vector<int> children;
        if (!srcBone.second.ChildrenName.empty())
        {
            for(auto i=0u; i<srcBone.second.ChildrenName.size(); ++i)
            {
                auto itr = boneMap.find(srcBone.second.ChildrenName[i]);
                if (itr != boneMap.end())
                { children.push_back(int(itr->second.Index)); }
            }
        }

        auto bindPose    = ToFloat4x4(srcBone.second.BindPose);
        auto invBindPose = ToFloat4x4(srcBone.second.InverseBindPose);

        auto boneId = srcBone.second.Index;

        bones[boneId] = asdx::res::CreateBoneDirect(
            builder,
            srcBone.second.Name.c_str(),
            parentId,
            &bindPose,
            &invBindPose,
            &children);
    }

    // マテリアルデータを変換.
    std::vector<flatbuffers::Offset<flatbuffers::String>> materials;
    materials.reserve(pScene->mNumMaterials);
    for(auto i=0u; i<pScene->mNumMaterials; ++i)
    {
        const auto srcMat = pScene->mMaterials[i];
        std::string name = srcMat->GetName().C_Str();
        if (name.empty() || name == "")
        {
            name = "Material" + std::to_string(i);
        }
        materials.push_back(builder.CreateString(name.c_str()));
    }

    auto sphere = asdx::res::BoundingSphere(asdx::res::Float3(bounds.Center.x, bounds.Center.y, bounds.Center.z), bounds.Radius);

    auto bin = asdx::res::CreateModelBinaryDirect(
        builder,
        CURRENT_VERSION,
        &meshes,
        &materials,
        &bones,
        &sphere);

    builder.Finish(bin);

    binary.resize(builder.GetSize());
    memcpy(binary.data(), builder.GetBufferPointer(), builder.GetSize());

    pScene = nullptr;

    return true;
}

} // namespace asdx

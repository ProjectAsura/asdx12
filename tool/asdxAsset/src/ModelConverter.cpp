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
#include <fnd/asdxPath.h>
#include <ModelConverter.h>
#include <TextureConverter.h>
#include <mikktspace.h>
#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
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
static constexpr uint32_t CURRENT_VERSION = 2u;  //!< 現在サポートされているバージョン.


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
//      Float4x4 に変換します.
//-----------------------------------------------------------------------------
asdx::res::Float4x4 ToFloat4x4(const aiMatrix4x4& matrix)
{
    return asdx::res::Float4x4(
        matrix.a1, matrix.b1, matrix.c1, matrix.d1,
        matrix.a2, matrix.b2, matrix.c2, matrix.d2,
        matrix.a3, matrix.b3, matrix.c3, matrix.d3,
        matrix.a4, matrix.b4, matrix.c4, matrix.d4);
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
//      aiMatrix4x4 に変換します.
//-----------------------------------------------------------------------------
aiMatrix4x4 ToAiMatrix(const asdx::res::Float4x4* matrix)
{
    return aiMatrix4x4(
        matrix->M11(), matrix->M21(), matrix->M31(), matrix->M41(),
        matrix->M12(), matrix->M22(), matrix->M32(), matrix->M42(),
        matrix->M13(), matrix->M23(), matrix->M33(), matrix->M43(),
        matrix->M14(), matrix->M24(), matrix->M34(), matrix->M44());
}

//-----------------------------------------------------------------------------
//      指定名に合致するノードを検索します.
//-----------------------------------------------------------------------------
const aiNode* FindNode(aiNode* node, const char* name)
{
    if (node == nullptr || name == nullptr)
        return nullptr;

    if (strcmp(node->mName.C_Str(), name) == 0)
        return node;

    for(auto i=0u; i<node->mNumChildren; ++i)
    {
        auto child = node->mChildren[i];
        if (child == nullptr)
            continue;

        auto foundNode = FindNode(child, name);
        if (foundNode)
            return foundNode;
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
//      指定名に合致するボーンを検索します.
//-----------------------------------------------------------------------------
const aiBone* FindBone(const aiScene* scene, const char* name)
{
    if (scene == nullptr || name == nullptr)
        return nullptr;

    for(auto i=0u; i<scene->mNumMeshes; ++i)
    {
        auto mesh = scene->mMeshes[i];
        if (!mesh->HasBones())
            continue;

        for(auto j=0u; j<mesh->mNumBones; ++j)
        {
            auto bone = mesh->mBones[j];
            if (strcmp(name, bone->mName.C_Str()) == 0)
                return bone;
        }
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
//      再帰的にノードを辞書に登録します.
//-----------------------------------------------------------------------------
void RegisterDic(const aiNode* node, std::unordered_map<std::string, int>& dic)
{
    if (node == nullptr)
        return;

    if (node->mParent != nullptr)
    {
        auto parentName = std::string(node->mParent->mName.C_Str());
        auto itr = dic.find(parentName);
        if (itr == dic.end())
        {
            auto id = int(dic.size());
            dic[parentName] = id;
        }
    }

    auto name = std::string(node->mName.C_Str());
    auto itr = dic.find(name);
    if (itr == dic.end())
    {
        auto id = int(dic.size());
        dic[name] = id;
    }

    for(auto j=0u; j<node->mNumChildren; ++j)
    {
        RegisterDic(node->mChildren[j], dic);
    }
}

//-----------------------------------------------------------------------------
//      ボーンを解析します.
//-----------------------------------------------------------------------------
void ParseBone
(
    flatbuffers::FlatBufferBuilder&                     builder,
    const aiScene*                                      pScene,
    std::unordered_map<std::string, int>&               boneMap,
    std::vector<flatbuffers::Offset<asdx::res::Bone>>&  dstBones
)
{
    // 親 -> 子 の順番を守って登録する.
    // この順番を守らないと，スキニング行列が 親 -> 子 の順番にならず不具合を引き起こすため.
    for(auto i=0u; i<pScene->mNumMeshes; ++i)
    {
        const auto srcMesh = pScene->mMeshes[i];
        if (!srcMesh->HasBones())
            continue;

        for(auto j=0u; j<srcMesh->mNumBones; ++j)
        {
            auto bone = srcMesh->mBones[j];
            auto node = FindNode(pScene->mRootNode, bone->mName.C_Str());
            RegisterDic(node, boneMap);
        }
    }

    // ボーンデータを変換.
    dstBones.resize(boneMap.size());
    for(auto& item : boneMap)
    {
        auto& boneName = item.first;
        auto bone = FindBone(pScene, boneName.c_str());;
        auto node = FindNode(pScene->mRootNode, boneName.c_str());

        auto itr = boneMap.find(boneName);
        assert(itr != boneMap.end());
        auto boneId = itr->second;

        int parentId = -1;
        if (node != nullptr && node->mParent != nullptr)
        {
            auto itr = boneMap.find(node->mParent->mName.C_Str());
            if (itr != boneMap.end())
            { parentId = itr->second; }
        }

        std::vector<int> children;
        if (!!node && node->mNumChildren > 0)
        {
            children.reserve(node->mNumChildren);
            for(auto i=0u; i<node->mNumChildren; ++i)
            {
                auto itr = boneMap.find(node->mChildren[i]->mName.C_Str());
                if (itr != boneMap.end())
                { children.push_back(itr->second); }
            }
            children.shrink_to_fit();
        }

        asdx::res::Float4x4 bindPose(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);

        asdx::res::Float4x4 invBindPose(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);

        if (node != nullptr)
            bindPose = ToFloat4x4(node->mTransformation);

        if (bone != nullptr)
            invBindPose = ToFloat4x4(bone->mOffsetMatrix);

        dstBones[boneId] = asdx::res::CreateBoneDirect(
            builder,
            boneName.c_str(),
            parentId,
            &bindPose,
            &invBindPose,
            &children);
    }
}

//-----------------------------------------------------------------------------
//      メッシュを解析します.
//-----------------------------------------------------------------------------
void ParseMesh
(
    flatbuffers::FlatBufferBuilder&                 builder,
    const std::unordered_map<std::string, int>&     boneMap,
    const aiNode*                                   rootNode,
    flatbuffers::Offset<asdx::res::Mesh>&           dstMesh,
    const aiMesh*                                   srcMesh,
    asdx::BoundingSphere3&                          boundSphere,
    asdx::BoundingBox3&                             boundBox
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

    asdx::BoundingBox3 box;

    for(auto i=0u; i<srcMesh->mNumVertices; ++i)
    {
        auto pos = srcMesh->mVertices[i];
        auto nrm = srcMesh->mNormals[i];
        auto tex = srcMesh->HasTextureCoords(0) ? srcMesh->mTextureCoords[0][i] : kZero;
        auto col = srcMesh->HasVertexColors (0) ? srcMesh->mColors[0][i] : kWhite;

        positions[i] = asdx::res::Float3(pos.x, pos.y, pos.z);
        normals  [i] = asdx::res::Float3(nrm.x, nrm.y, nrm.z);
        texcoords[i] = asdx::res::Float2(tex.x, tex.y);
        colors   [i] = ToUnorm4(col);

        auto p   = asdx::Vector3(pos.x, pos.y, pos.z);
        box.Mini = asdx::Vector3::Min(box.Mini, p);
        box.Maxi = asdx::Vector3::Max(box.Maxi, p);
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
            auto itr = boneMap.find(bone->mName.C_Str());
            assert(itr != boneMap.end());
            auto boneId = itr->second;

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
    boundBox    = asdx::BoundingBox3::Merge(boundBox, box);

    auto bSphere = asdx::res::BoundingSphere(asdx::res::Float3(sphere.Center.x, sphere.Center.y, sphere.Center.z), sphere.Radius);
    auto bBox    = asdx::res::BoundingBox(
                    asdx::res::Float3(box.Mini.x, box.Mini.y, box.Mini.z),
                    asdx::res::Float3(box.Maxi.x, box.Maxi.y, box.Maxi.z));

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
        &bSphere,
        &bBox);
}

//-----------------------------------------------------------------------------
//      ファイルパスを連結します.
//-----------------------------------------------------------------------------
std::string PathCombine(const std::string& lhs, const std::string& rhs)
{
    if (rhs.empty())
        return std::string();
    if (lhs.empty())
        return rhs;

    return lhs + "\\" + rhs;
}

//-----------------------------------------------------------------------------
//      テクスチャコンバートを行います.
//-----------------------------------------------------------------------------
void ConvertTXB(const std::string& input, const std::string& output)
{
    TextureConverter::Desc desc = {};
    desc.InputPath  = input;
    desc.OutputPath = output;

    if (!TextureConverter::Convert(desc))
    { ELOG("Error : TextureConverter::Convert() Failed. path = %s", input.c_str()); }
}

//-----------------------------------------------------------------------------
//      マテリアルを解析します.
//-----------------------------------------------------------------------------
void ParseMaterial
(
    flatbuffers::FlatBufferBuilder& builder,
    const aiScene*                  pScene,
    const std::string&              inputDir,
    const std::string&              txbOutPath,
    bool                            txbConvert,
    std::vector<flatbuffers::Offset<asdx::res::Material>>& materials
)
{
    materials.reserve(pScene->mNumMaterials);
    for(auto i=0u; i<pScene->mNumMaterials; ++i)
    {
        const auto srcMat = pScene->mMaterials[i];
        std::string name = srcMat->GetName().C_Str();
        if (name.empty() || name == "")
        { name = "material__" + std::to_string(i); }

        std::string baseColorMap;
        std::string normalMap;
        std::string ormMap;
        std::string emissiveMap;

        asdx::Vector3 baseColorFactor = asdx::Vector3(1.0f, 1.0f, 1.0f);
        asdx::Vector3 emissiveFactor  = asdx::Vector3(0.0f, 0.0f, 0.0f);
        float alpha                   = 1.0f;
        float occulusionFactor        = 1.0f;
        float roughnessFactor         = 1.0f;
        float metalnessFactor         = 1.0f;
        float ior                     = 1.0f;
        bool  isOpaque                = true;
        float alphaCutOff             = 0.0f;

        auto alphaMode = asdx::res::AlphaType_Opaque;

        int shadingModel = 0;
        srcMat->Get(AI_MATKEY_SHADING_MODEL, shadingModel);

        // 法線マップ.
        {
            aiString mapPath;
            if (srcMat->GetTexture(aiTextureType_NORMALS, 0, &mapPath) == AI_SUCCESS)
            {
                asdx::fs::path p = mapPath.C_Str();
                normalMap = "textures\\" + p.filename().replace_extension(".txb").string();
                if (txbConvert)
                {
                    ConvertTXB(
                        PathCombine(inputDir, mapPath.C_Str()),
                        PathCombine(txbOutPath, normalMap));
                }
            }
        }

        // エミッシブマップ.
        {
            aiString mapPath;
            if (srcMat->GetTexture(aiTextureType_EMISSIVE, 0, &mapPath) == AI_SUCCESS)
            {
                asdx::fs::path p = mapPath.C_Str();
                emissiveMap = "textures\\" + p.filename().replace_extension(".txb").string();
                if (txbConvert)
                {
                    ConvertTXB(
                        PathCombine(inputDir, mapPath.C_Str()),
                        PathCombine(txbOutPath, emissiveMap));
                }
            }
        }

        // エミッシブカラー.
        {
            aiColor4D value;
            if (srcMat->Get(AI_MATKEY_COLOR_EMISSIVE, value) == AI_SUCCESS)
            {
                emissiveFactor.x = value.r;
                emissiveFactor.y = value.g;
                emissiveFactor.z = value.b;
            }
        }

        // 不透明度.
        {
            float value;
            if (srcMat->Get(AI_MATKEY_OPACITY, value) == AI_SUCCESS)
            {
                alpha = value;
                if (alpha < 1.0f)
                { isOpaque = false; }
            }
        }

        // 屈折率.
        {
            float value;
            if (srcMat->Get(AI_MATKEY_REFRACTI, value) == AI_SUCCESS)
            { ior = value; }
        }

        // PBRモデルの場合.
        if (shadingModel == aiShadingMode_PBR_BRDF)
        {
            // ベースカラーマップ.
            {
                aiString mapPath;
                if (srcMat->GetTexture(aiTextureType_BASE_COLOR, 0, &mapPath) == AI_SUCCESS)
                {
                    asdx::fs::path p = mapPath.C_Str();
                    baseColorMap = "textures\\" + p.filename().replace_extension(".txb").string();
                    if (txbConvert)
                    {
                        ConvertTXB(
                            PathCombine(inputDir, mapPath.C_Str()),
                            PathCombine(txbOutPath, baseColorMap));
                    }
                }
            }

            // Occlusion/Roughness/Metalnessマップ.
            {
                aiString mapPath;
                if (srcMat->GetTexture(aiTextureType_GLTF_METALLIC_ROUGHNESS, 0, &mapPath) == AI_SUCCESS)
                {
                    asdx::fs::path p = mapPath.C_Str();
                    ormMap = "textures\\" + p.filename().replace_extension(".txb").string();
                    if (txbConvert)
                    {
                        ConvertTXB(
                            PathCombine(inputDir, mapPath.C_Str()),
                            PathCombine(txbOutPath, ormMap));
                    }
                }
            }

            // ベースカラーファクター.
            {
                aiColor4D value;
                if (srcMat->Get(AI_MATKEY_BASE_COLOR, value) == AI_SUCCESS)
                {
                    baseColorFactor.x = value.r;
                    baseColorFactor.y = value.g;
                    baseColorFactor.z = value.b;
                }
            }

            // ラフネスファクター.
            {
                float value;
                if (srcMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, value) == AI_SUCCESS)
                {
                    roughnessFactor = value;
                }
            }

            // メタルネスファクター
            {
                float value;
                if (srcMat->Get(AI_MATKEY_METALLIC_FACTOR, value) == AI_SUCCESS)
                {
                    metalnessFactor = value;
                }
            }

            // アルファモード.
            {
                aiString alphaModeStr;
                if (srcMat->Get("$mat.gltf.alphaMode", 0, 0, alphaModeStr) == AI_SUCCESS)
                {
                    if (strcmp(alphaModeStr.C_Str(), "OPQAUE") == 0)
                    {
                        alphaMode = asdx::res::AlphaType_Opaque;
                    }
                    else if (strcmp(alphaModeStr.C_Str(), "MASK") == 0)
                    {
                        alphaMode   = asdx::res::AlphaType_Mask;
                        alphaCutOff = 0.5f;
                        srcMat->Get("$mat.gltf.alphaCutoff", 0, 0, alphaCutOff);
                    }
                    else if (strcmp(alphaModeStr.C_Str(), "BLEND") == 0)
                    {
                        alphaMode = asdx::res::AlphaType_Blend;
                    }
                }
            }
        }
        else
        {
            // ディフューズカラーマップ.
            {
                aiString mapPath;
                if (srcMat->GetTexture(aiTextureType_DIFFUSE, 0, &mapPath) == AI_SUCCESS)
                {
                    asdx::fs::path p = mapPath.C_Str();
                    baseColorMap = "textures\\" + p.filename().replace_extension(".txb").string();
                    if (txbConvert)
                    {
                        ConvertTXB(
                            PathCombine(inputDir, mapPath.C_Str()),
                            PathCombine(txbOutPath, baseColorMap));
                    }
                }
            }

            // ディフューズカラー.
            {
                aiColor4D value;
                if (srcMat->Get(AI_MATKEY_COLOR_DIFFUSE, value) == AI_SUCCESS)
                {
                    baseColorFactor.x = value.r;
                    baseColorFactor.y = value.g;
                    baseColorFactor.z = value.b;
                }
            }
        }

        asdx::res::Float3 bf(baseColorFactor.x, baseColorFactor.y, baseColorFactor.z);
        asdx::res::Float3 ef(emissiveFactor.x, emissiveFactor.y, emissiveFactor.z);

        auto dstMat = asdx::res::CreateMaterialDirect(
            builder,
            name.c_str(),
            &bf,
            alpha,
            occulusionFactor,
            roughnessFactor,
            metalnessFactor,
            ior,
            &ef,
            baseColorMap.c_str(),
            normalMap.c_str(),
            ormMap.c_str(),
            emissiveMap.c_str(),
            alphaMode,
            alphaCutOff);

        materials.emplace_back(dstMat);
    }
}

} // namespace


///////////////////////////////////////////////////////////////////////////////
// ModelConverter class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      現在のバイナリバージョンを取得します.
//-----------------------------------------------------------------------------
uint32_t ModelConverter::GetCurrentVersion()
{ return CURRENT_VERSION; }

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

    std::filesystem::path txbOutPath = desc.OutputPath;
    txbOutPath = txbOutPath.parent_path();

    std::vector<uint8_t> binary;
    if (!Convert(desc.InputPath.c_str(), desc.TextureConvert, txbOutPath.string(), binary))
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
bool ModelConverter::Convert
(
    const std::string&      path,
    bool                    txbConvert,
    const std::string&      txbOutPath,
    std::vector<uint8_t>&   binary
)
{
    if (path.empty())
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    std::string inputDir;
    if (txbConvert)
    {
        asdx::fs::path p = path;
        inputDir = p.parent_path().string();

        // 出力ディレクトリが存在しなければ作成する.
        auto texDir = txbOutPath + "\\textures";
        if (!asdx::fs::exists(texDir))
        {
            if (!asdx::fs::create_directories(texDir))
            { ELOG("Error : create_directories() Failed. path = %s", texDir.c_str()); }
        }
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

    asdx::BoundingSphere3 sphere(asdx::Vector3(0.0f, 0.0f, 0.0f), 0.0f);
    asdx::BoundingBox3    box;

    flatbuffers::FlatBufferBuilder builder(1024);

    // ボーンデータを変換.
    std::unordered_map<std::string, int> boneMap;
    std::vector<flatbuffers::Offset<asdx::res::Bone>> bones;
    ParseBone(builder, pScene, boneMap, bones);

    // メッシュデータを変換.
    std::vector<flatbuffers::Offset<asdx::res::Mesh>> meshes;
    meshes.resize(pScene->mNumMeshes);
    for(auto i=0u; i<pScene->mNumMeshes; ++i)
    {
        const auto srcMesh = pScene->mMeshes[i];
        auto& dstMesh = meshes[i];

        ParseMesh(builder, boneMap, pScene->mRootNode, dstMesh, srcMesh, sphere, box);
    }
    // 不要になったのでクリア.
    boneMap.clear();

    // マテリアルデータを変換.
    std::vector<flatbuffers::Offset<asdx::res::Material>> materials;
    ParseMaterial(builder, pScene, inputDir, txbOutPath, txbConvert, materials);

    auto bSphere = asdx::res::BoundingSphere(
                    asdx::res::Float3(sphere.Center.x, sphere.Center.y, sphere.Center.z),
                    sphere.Radius);
    auto bBox = asdx::res::BoundingBox(
                    asdx::res::Float3(box.Mini.x, box.Mini.y, box.Mini.z),
                    asdx::res::Float3(box.Maxi.x, box.Maxi.y, box.Maxi.z));

    auto rootTransform = ToFloat4x4(ToMatrix(pScene->mRootNode->mTransformation));

    auto bin = asdx::res::CreateModelBinaryDirect(
        builder,
        CURRENT_VERSION,
        &meshes,
        &materials,
        &bones,
        &bSphere,
        &bBox,
        &rootTransform);

    builder.Finish(bin);

    binary.resize(builder.GetSize());
    memcpy(binary.data(), builder.GetBufferPointer(), builder.GetSize());

    pScene = nullptr;

    return true;
}

//-----------------------------------------------------------------------------
//      逆変換処理を行います.
//-----------------------------------------------------------------------------
bool ModelConverter::ReverseConvert(const std::vector<uint8_t>& binary, const char* format, const std::string& path)
{
    if (binary.empty() || path.empty() || path == "" || format == nullptr)
    {
        ELOG("Error : Invalid Argument");
        return false;
    }

    auto modelBin = asdx::res::GetModelBinary(binary.data());

    aiScene scene = {};

    auto rootNode = new aiNode();
    rootNode->mName           = "RootNode";
    rootNode->mTransformation = ToAiMatrix(modelBin->RootTransform());
    rootNode->mParent         = nullptr;
    rootNode->mNumChildren    = 0;
    rootNode->mChildren       = nullptr;
    rootNode->mNumMeshes      = 0;
    rootNode->mMeshes         = nullptr;
    rootNode->mMetaData       = nullptr;

    scene.mRootNode = rootNode;

    auto materials = modelBin->Materials();
    {
        auto count = materials->size();
        scene.mNumMaterials = count;
        scene.mMaterials    = new aiMaterial* [count];

        for(auto i=0u; i<count; ++i)
        {
            auto srcMat = materials->Get(i);
            auto dstMat = new aiMaterial();

            // マテリアル名.
            {
                aiString name(srcMat->Name()->c_str());
                dstMat->AddProperty(&name, AI_MATKEY_NAME);
            }

            // シェーディングモデル.
            {
                int value = aiShadingMode_PBR_BRDF;
                dstMat->AddProperty(&value, 1, AI_MATKEY_SHADING_MODEL);
            }

            // ベースカラーファクター.
            {
                auto bc = srcMat->BaseColorFactor();
                aiColor4D value(bc->X(), bc->Y(), bc->Z(), 1.0f);
                dstMat->AddProperty(&value, 1, AI_MATKEY_BASE_COLOR);
            }

            // アルファ.
            {
                auto alpha = srcMat->Alpha();
                dstMat->AddProperty(&alpha, 1, AI_MATKEY_OPACITY);
            }

            // 屈折率.
            {
                auto ior = srcMat->Ior();
                dstMat->AddProperty(&ior, 1, AI_MATKEY_REFRACTI);
            }

            // ラフネスファクター.
            {
                auto roughness = srcMat->RoughnessFactor();
                dstMat->AddProperty(&roughness, 1, AI_MATKEY_ROUGHNESS_FACTOR);
            }

            // メタルネスファクター.
            {
                auto metalness = srcMat->MetalnessFactor();
                dstMat->AddProperty(&metalness, 1, AI_MATKEY_METALLIC_FACTOR);
            }

            // エミッシブファクター.
            {
                auto emissive = srcMat->EmissiveFactor();
                aiColor3D value(emissive->X(), emissive->Y(), emissive->Z());
                dstMat->AddProperty(&value, 1, AI_MATKEY_COLOR_EMISSIVE);
            }

            // ベースカラーマップ.
            {
                auto mapPath = srcMat->BaseColorMap();
                if (mapPath != nullptr)
                {
                    aiString value(mapPath->c_str());
                    dstMat->AddProperty(&value, AI_MATKEY_TEXTURE(aiTextureType_BASE_COLOR, 0));
                }
            }

            // 法線マップ.
            {
                auto mapPath = srcMat->NormalMap();
                if (mapPath != nullptr)
                {
                    aiString value(mapPath->c_str());
                    dstMat->AddProperty(&value, AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0));
                }
            }

            // Occlusion/Roughness/Metalnessマップ.
            {
                auto mapPath = srcMat->OrmMap();
                if (mapPath != nullptr)
                {
                    aiString value(mapPath->c_str());
                    dstMat->AddProperty(&value, AI_MATKEY_TEXTURE(aiTextureType_GLTF_METALLIC_ROUGHNESS, 0));
                }
            }

            // エミッシブマップ.
            {
                auto mapPath = srcMat->EmissiveMap();
                if (mapPath != nullptr)
                {
                    aiString value(mapPath->c_str());
                    dstMat->AddProperty(&value, AI_MATKEY_TEXTURE(aiTextureType_EMISSIVE, 0));
                }
            }

            scene.mMaterials[i] = dstMat;
        }
    }

    auto bones = modelBin->Bones();
    aiNode* dstNodes = nullptr;
    if (bones != nullptr)
    {
        auto count = bones->size();
        dstNodes = new aiNode[count];

        for(auto i=0u; i<count; ++i)
        {
            const auto srcBone = bones->Get(i);
            dstNodes[i].mName           = srcBone->Name()->c_str();
            dstNodes[i].mTransformation = ToAiMatrix(srcBone->BindPose());
            dstNodes[i].mNumMeshes      = 0;
            dstNodes[i].mMeshes         = nullptr;
            dstNodes[i].mMetaData       = nullptr;

            auto parentId = srcBone->Parent();
            if (parentId >= 0)
            {
                dstNodes[i].mParent = &dstNodes[parentId];
            }
            else
            {
                dstNodes[i].mParent = rootNode;
            }

            const auto srcChildren = srcBone->Children();
            if (srcChildren != nullptr)
            {
                dstNodes[i].mNumChildren = uint32_t(srcChildren->size());
                dstNodes[i].mChildren    = new aiNode* [srcChildren->size()];
                for(auto j=0u; j<srcChildren->size(); ++j)
                {
                    const auto srcChild = srcChildren->Get(j);
                    dstNodes[i].mChildren[j] = &dstNodes[srcChild];
                }
            }
            else
            {
                dstNodes[i].mNumChildren = 0;
                dstNodes[i].mChildren    = nullptr;
            }
        }
    }

    std::map<uint32_t, std::vector<uint32_t>> boneMeshMap;

    auto meshes = modelBin->Meshes();
    {
        auto count = meshes->size();
        scene.mNumMeshes = count;
        scene.mMeshes    = new aiMesh* [count];

        for(auto i=0u; i<count; ++i)
        {
            auto srcMesh = meshes->Get(i);
            auto dstMesh = new aiMesh();

            auto vertexCount = srcMesh->Positions()->size();
            auto faceCount   = srcMesh->VertexIndices()->size() / 3;

            dstMesh->mName           = srcMesh->Name()->c_str();
            dstMesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;
            dstMesh->mNumVertices    = vertexCount;
            dstMesh->mNumFaces       = faceCount;

            dstMesh->mVertices = new aiVector3D [vertexCount];
            for(auto j=0u; j<vertexCount; ++j)
            {
                auto srcPos = srcMesh->Positions()->Get(j);
                dstMesh->mVertices[j] = aiVector3D(srcPos->X(), srcPos->Y(), srcPos->Z());
            }

            dstMesh->mNormals = new aiVector3D [vertexCount];
            for(auto j=0u; j<vertexCount; ++j)
            {
                auto srcNormal = srcMesh->Normals()->Get(j);
                dstMesh->mNormals[j] = aiVector3D(srcNormal->X(), srcNormal->Y(), srcNormal->Z());
            }

            if (srcMesh->TexCoords() != nullptr)
            {
                dstMesh->mNumUVComponents[0] = 2;
                dstMesh->mTextureCoords[0] = new aiVector3D [vertexCount];
                for(auto j=0u; j<vertexCount; ++j)
                {
                    auto srcUv = srcMesh->TexCoords()->Get(j);
                    dstMesh->mTextureCoords[0][j] = aiVector3D(srcUv->X(), srcUv->Y(), 0.0f);
                }
            }

            if (srcMesh->Tangents() != nullptr)
            {
                dstMesh->mTangents   = new aiVector3D [vertexCount];
                dstMesh->mBitangents = new aiVector3D [vertexCount];
                for(auto j=0u; j<vertexCount; ++j)
                {
                    auto srcTangent = srcMesh->Tangents()->Get(j);
                    dstMesh->mTangents[j] = aiVector3D(srcTangent->X(), srcTangent->Y(), srcTangent->Z());

                    auto N = asdx::Vector3(dstMesh->mNormals [j].x, dstMesh->mNormals [j].y, dstMesh->mNormals [j].z);
                    auto T = asdx::Vector3(dstMesh->mTangents[j].x, dstMesh->mTangents[j].y, dstMesh->mTangents[j].z);
                    auto B = asdx::Vector3::Cross(T, N).Normalize() * srcTangent->W();

                    dstMesh->mBitangents[j] = aiVector3D(B.x, B.y, B.z);
                }
            }

            if (srcMesh->Colors() != nullptr)
            {
                dstMesh->mColors[0] = new aiColor4D [vertexCount];
                for(auto j=0u; j<vertexCount; ++j)
                {
                    auto srcColor = srcMesh->Colors()->Get(j);
                    auto r = float(srcColor->X()) / 255.0f;
                    auto g = float(srcColor->Y()) / 255.0f;
                    auto b = float(srcColor->Z()) / 255.0f;
                    auto a = float(srcColor->W()) / 255.0f;

                    dstMesh->mColors[0][j] = aiColor4D(r, g, b, a);
                }
            }

            if (srcMesh->BoneIndices() != nullptr && srcMesh->BoneWeights() != nullptr && bones != nullptr)
            {
                std::map<uint32_t, std::vector<aiVertexWeight>> tmpBones;

                for(auto j=0u; j<vertexCount; ++j)
                {
                    auto srcBoneIndex  = srcMesh->BoneIndices()->Get(j);
                    auto srcBoneWeight = srcMesh->BoneWeights()->Get(j);

                    auto w = srcBoneWeight->X();
                    if (w > 0.0f)
                    {
                        aiVertexWeight tmpWeight = {};
                        tmpWeight.mVertexId = j;
                        tmpWeight.mWeight   = w;

                        tmpBones[srcBoneIndex->X()].push_back(tmpWeight);
                    }

                    w = srcBoneWeight->Y();
                    if (w > 0.0f)
                    {
                        aiVertexWeight tmpWeight = {};
                        tmpWeight.mVertexId = j;
                        tmpWeight.mWeight   = w;

                        tmpBones[srcBoneIndex->Y()].push_back(tmpWeight);
                    }

                    w = srcBoneWeight->Z();
                    if (w > 0.0f)
                    {
                        aiVertexWeight tmpWeight = {};
                        tmpWeight.mVertexId = j;
                        tmpWeight.mWeight   = w;

                        tmpBones[srcBoneIndex->Z()].push_back(tmpWeight);
                    }

                    w = srcBoneWeight->W();
                    if (w > 0.0f)
                    {
                        aiVertexWeight tmpWeight = {};
                        tmpWeight.mVertexId = j;
                        tmpWeight.mWeight   = w;

                        tmpBones[srcBoneIndex->W()].push_back(tmpWeight);
                    }
                }

                dstMesh->mBones = new aiBone* [tmpBones.size()];

                for(auto& tmpBone : tmpBones)
                {
                    const auto srcBone = bones->Get(tmpBone.first);
                    const auto weightCount = uint32_t(tmpBone.second.size());

                    auto dstBone = new aiBone();

                    dstBone->mName          = srcBone->Name()->c_str();
                    dstBone->mNumWeights    = weightCount;
                    dstBone->mWeights       = new aiVertexWeight[weightCount];
                    dstBone->mOffsetMatrix  = ToAiMatrix(srcBone->InverseBindPose());
                    memcpy(dstBone->mWeights, tmpBone.second.data(), sizeof(aiVertexWeight) * weightCount);

                    dstMesh->mBones[tmpBone.first] = dstBone;

                    boneMeshMap[tmpBone.first].push_back(i);
                }
            }

            auto index = 0u;
            dstMesh->mFaces = new aiFace[faceCount];
            for(auto j=0u; j<faceCount; ++j)
            {
                dstMesh->mFaces[j].mNumIndices = 3;
                dstMesh->mFaces[j].mIndices    = new unsigned int[3];

                dstMesh->mFaces[j].mIndices[0] = srcMesh->VertexIndices()->Get(index++);
                dstMesh->mFaces[j].mIndices[1] = srcMesh->VertexIndices()->Get(index++);
                dstMesh->mFaces[j].mIndices[2] = srcMesh->VertexIndices()->Get(index++);
            }

            scene.mMeshes[i] = dstMesh;
        }
    }

    // 最後にメッシュを関連付ける.
    if (dstNodes != nullptr)
    {
        auto count = bones->size();
        for(auto i=0u; i<count; ++i)
        {
            auto itr = boneMeshMap.find(i);
            if (itr == boneMeshMap.end())
                continue;

            dstNodes[i].mNumMeshes = uint32_t(itr->second.size());
            dstNodes[i].mMeshes    = itr->second.data();
        }
    }

    Assimp::Exporter exporter;
    auto ret = exporter.Export(&scene, format, path.c_str());

    // Assimp側で面倒くさい解放の仕方をしてクラッシュ原因になる可能性があるので...
    // デストラクタが呼ばれる前に，先に自前で解放して，空にしておく.
    scene.mRootNode = nullptr;
    if (rootNode != nullptr)
    {
        delete rootNode;
        rootNode = nullptr;
    }

    if (dstNodes != nullptr)
    {
        for(auto i=0u; i<bones->size(); ++i)
        {
            if (dstNodes[i].mChildren != nullptr)
            {
                delete[] dstNodes[i].mChildren;
                dstNodes[i].mChildren = nullptr;
            }
            dstNodes[i].mNumChildren = 0;
            dstNodes[i].mNumMeshes   = 0;
            dstNodes[i].mMeshes      = nullptr;
        }
        delete[] dstNodes;
        dstNodes = nullptr;
    }

    if (ret != aiReturn_SUCCESS)
    {
        ELOG("Error : Assimp::Exporter::Export() Failed. path = %s", path.c_str());
        return false;
    }

    return true;
}

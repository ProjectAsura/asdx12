

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdint>
#include <string>
#include <fnd/asdxMath.h>
#include <generated/mesh_format.h>
#include <generated/material_format.h>
#include <tiny_gltf.h>


//-----------------------------------------------------------------------------
//      拡張子を取得.
//-----------------------------------------------------------------------------
void GetFilePathBaseAndExtension(const std::string &fileName, std::string& base, std::string& ext)
{
    auto index = fileName.find_last_of(".");
    if (index != std::string::npos)
    {
        base = fileName.substr(0, index);
        ext  = fileName.substr(index + 1);
        return;
    }

    base = fileName;
    ext  = "";
}

//-----------------------------------------------------------------------------
//      クオータニオンから回転角に変換します.
//-----------------------------------------------------------------------------
static void QuatToAngleAxis
(
    const std::vector<double>   quaternion,
    float&                      outAngleRadian,
    float*                      axis
)
{
    double qx = quaternion[0];
    double qy = quaternion[1];
    double qz = quaternion[2];
    double qw = quaternion[3];
  
    double angleRadians = 2 * acos(qw);
    if (angleRadians == 0.0)
    {
        outAngleRadian = 0.0f;
        axis[0] = 0.0f;
        axis[1] = 0.0f;
        axis[2] = 1.0f;
        return;
    }

    double denom = sqrt(1 - qw * qw);
    outAngleRadian = float(angleRadians);
    axis[0] = float(qx / denom);
    axis[1] = float(qy / denom);
    axis[2] = float(qz / denom);
}

//-----------------------------------------------------------------------------
//      GLTFを自前フォーマットに変換.
//-----------------------------------------------------------------------------
int Convert(const std::string& input, const std::string& output)
{
    tinygltf::Model    model;
    tinygltf::TinyGLTF context;

    std::string err;
    std::string warn;


    std::string base, ext;
    GetFilePathBaseAndExtension(input, base, ext);
    auto ret = false;

    // 拡張子からバイナリフォーマットかどうかを判定する.
    if (ext.compare("glb") == 0)
    { ret = context.LoadBinaryFromFile(&model, &err, &warn, input.c_str()); }
    else
    { ret = context.LoadASCIIFromFile(&model, &err, &warn, input.c_str()); }

    // 警告表示.
    if (!warn.empty())
    { printf_s("WARNING: %s\n", warn.c_str()); }

    // エラー表示.
    if (!err.empty())
    { printf_s("ERROR: %s\n", err.c_str()); }

    if (!ret)
    {
        printf_s("Error : Failed parse gltf\n");
        return -1;
    }

    // マテリアルファイル作成.
    {
        flatbuffers::FlatBufferBuilder builder(1024);

        std::vector<flatbuffers::Offset<asdx::res::Material>> dstMaterials;
        std::vector<flatbuffers::Offset<flatbuffers::String>> dstTextures;

        for(const auto& srcTexture : model.textures)
        {
            auto convTexture = builder.CreateString(srcTexture.name);
            dstTextures.push_back(convTexture);
        }

        for(const auto& srcMaterial : model.materials)
        {
            auto convMaterial = asdx::res::CreateMaterialDirect(
                builder,
                srcMaterial.name.c_str(),
                uint32_t(srcMaterial.pbrMetallicRoughness.baseColorTexture.index),
                uint32_t(srcMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index),
                uint32_t(srcMaterial.normalTexture.index),
                uint32_t(srcMaterial.emissiveTexture.index),
                0,
                0,
                0,
                srcMaterial.alphaCutoff < 1.0,
                srcMaterial.alphaMode.compare("BLEND") == 0);

            dstMaterials.push_back(convMaterial);
        }

        auto dstMaterialSet = asdx::res::CreateMaterialSetDirect(
            builder,
            base.c_str(),
            &dstMaterials,
            &dstTextures);

        builder.Finish(dstMaterialSet);

        auto buf  = builder.GetBufferPointer();
        auto size = builder.GetSize();

        FILE* pFile = nullptr;

        auto outName = output + ".mat";
        auto errcode = fopen_s(&pFile, outName.c_str(), "wb");
        if (errcode == 0)
        {
            fwrite(buf, size, 1, pFile);
            fclose(pFile);
            pFile = nullptr;

            printf_s("INFO : Output Material File. %s\n", outName.c_str());
        }
    }

    // モデルファイル作成.
    {
        flatbuffers::FlatBufferBuilder builder(1024);

        std::vector<flatbuffers::Offset<asdx::res::Mesh>>     dstMeshes;
        std::vector<flatbuffers::Offset<flatbuffers::String>> dstMaterials;

        for(const auto& srcMaterial : model.materials)
        {
            auto convMaterial = builder.CreateString(srcMaterial.name);
            dstMaterials.push_back(convMaterial);
        }

        std::vector<asdx::Matrix> transforms;
        transforms.resize(model.meshes.size());
        for(auto& mtx : transforms)
        {
            mtx.Identity();
        }

        for(size_t i=0; i<model.nodes.size(); ++i)
        {
            auto& node = model.nodes[i];

            if (node.mesh == -1)
            { continue; }

            if (node.matrix.size() == 16)
            {
                transforms[i].row[0] = asdx::Vector4(float(node.matrix[0]),  float(node.matrix[1]),  float(node.matrix[2]),  float(node.matrix[3]));
                transforms[i].row[1] = asdx::Vector4(float(node.matrix[4]),  float(node.matrix[5]),  float(node.matrix[6]),  float(node.matrix[7]));
                transforms[i].row[2] = asdx::Vector4(float(node.matrix[8]),  float(node.matrix[9]),  float(node.matrix[10]), float(node.matrix[11]));
                transforms[i].row[3] = asdx::Vector4(float(node.matrix[12]), float(node.matrix[13]), float(node.matrix[14]), float(node.matrix[15]));
            }
            else
            {
                auto matrix = asdx::Matrix::CreateIdentity();

                if (node.scale.size() == 3)
                {
                    matrix *= asdx::Matrix::CreateScale(
                        float(node.scale[0]),
                        float(node.scale[1]),
                        float(node.scale[2]));
                }
                
                if (node.rotation.size() == 4)
                {
                    float angleRad;
                    float axis[3];
                    QuatToAngleAxis(node.rotation, angleRad, axis);

                    matrix *= asdx::Matrix::CreateFromAxisAngle(
                        asdx::Vector3(axis[0], axis[1], axis[2]), angleRad);

                }

                if (node.translation.size() == 3)
                {
                    matrix *= asdx::Matrix::CreateTranslation(
                        float(node.translation[0]),
                        float(node.translation[1]),
                        float(node.translation[2]));
                }

                transforms[i] = matrix;
            }
        }

        for(size_t i=0; i<model.meshes.size(); ++i)
        {
            const auto& srcMesh = model.meshes[i];

            for(const auto& srcPrimitive : srcMesh.primitives)
            {
                std::vector<asdx::res::Float3>  position;
                std::vector<asdx::res::Float3>  normal;
                std::vector<asdx::res::Float2>  texcoord;
                std::vector<uint32_t>           boneIndex;
                std::vector<float>              boneWeight;
                std::vector<uint32_t>           vertexIndex;

                uint32_t materialId = uint32_t(srcPrimitive.material);
                uint32_t boneStride = 0;

                const auto notFound = srcPrimitive.attributes.end();
                auto attr = srcPrimitive.attributes.find("POSITION");
                if (attr != notFound)
                {
                    auto& accessor = model.accessors[attr->second];
                    auto& view     = model.bufferViews[accessor.bufferView];
                    auto& buffer   = model.buffers[view.buffer];
                    auto offset    = accessor.byteOffset + view.byteOffset;

                    assert(view.byteStride == sizeof(asdx::Vector3));
                    assert(view.byteStride == sizeof(asdx::res::Float3));

                    auto ptr = reinterpret_cast<asdx::Vector3*>(buffer.data.data() + offset);

                    position.resize(accessor.count);

                    for(size_t idx=0; idx<accessor.count; ++idx)
                    {
                        // 行列を適用し，格納する.
                        auto p = ptr[idx];
                        p = asdx::Vector3::TransformCoord(p, transforms[i]);

                        position[idx] = asdx::res::Float3(p.x, p.y, p.z);
                    }
                }

                attr = srcPrimitive.attributes.find("NORMAL");
                if (attr != notFound)
                {
                }

                attr = srcPrimitive.attributes.find("TANGENT");
                if (attr != notFound)
                {
                }

                attr = srcPrimitive.attributes.find("TEXCOORD_0");
                if (attr != notFound)
                {
                }

                attr = srcPrimitive.attributes.find("JOINTS_0");
                if (attr != notFound)
                {
                }

                attr = srcPrimitive.attributes.find("WEIGHTS_0");
                if (attr != notFound)
                {
                }


                auto pPosition    = position   .empty() ? nullptr : &position;
                auto pNormal      = normal     .empty() ? nullptr : &normal;
                auto pTexCoord    = texcoord   .empty() ? nullptr : &texcoord;
                auto pBoneIndex   = boneIndex  .empty() ? nullptr : &boneIndex;
                auto pBoneWeight  = boneWeight .empty() ? nullptr : &boneWeight;
                auto pVertexIndex = vertexIndex.empty() ? nullptr : &vertexIndex;

                auto convMesh = asdx::res::CreateMeshDirect(
                    builder,
                    pPosition,
                    pNormal,
                    pTexCoord,
                    pBoneIndex,
                    pBoneWeight,
                    pVertexIndex,
                    materialId,
                    boneStride);

                dstMeshes.push_back(convMesh);
            }
        }

        auto dstModel = asdx::res::CreateModelDirect(
            builder,
            base.c_str(),
            &dstMeshes,
            &dstMaterials);

        builder.Finish(dstModel);

        auto buf  = builder.GetBufferPointer();
        auto size = builder.GetSize();

        FILE* pFile = nullptr;

        auto outName = output + ".mdl";
        auto errcode = fopen_s(&pFile, outName.c_str(), "wb");
        if (errcode == 0)
        {
            fwrite(buf, size, 1, pFile);
            fclose(pFile);
            pFile = nullptr;

            printf_s("INFO : Output Model File. %s\n", outName.c_str());
        }

    }

    return 0;
}

//-----------------------------------------------------------------------------
//      ヘルプを表示します.
//-----------------------------------------------------------------------------
void ShowHelp()
{
    printf_s("modelconv.exe input.gltf output_base_name\n");
}

//-----------------------------------------------------------------------------
//      エントリーポイントです.
//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    if (argc < 2)
    {
        ShowHelp();
        return -1;
    }

    std::string input  = argv[1];
    std::string output = "output.mdl";

    return Convert(input, output);
}
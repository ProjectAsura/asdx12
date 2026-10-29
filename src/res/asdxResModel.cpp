//-----------------------------------------------------------------------------
// File : asdxResModel.cpp
// Desc : Model Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxMacro.h>
#include <fnd/asdxLogger.h>
#include <res/asdxResModel.h>
#include "ResTypes_generated.h"
#include <res/asdxResHelper.h>
#include "ModelBinary_generated.h"


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t CURRENT_VERSION = 3u; //!< 現在ランタイムでサポートされているバージョン.

static_assert(sizeof(asdx::ResMeshInstance) == sizeof(asdx::res::MeshInstance), "MeshInstance Size Not Matched.");

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ModelBinary class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ModelBinary::ModelBinary()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ModelBinary::~ModelBinary()
{ Term(); }

//-----------------------------------------------------------------------------
//      メモリからロードします.
//-----------------------------------------------------------------------------
void ModelBinary::Load(std::vector<uint8_t>&& blob)
{
    m_Blob = std::move(blob);

#if ASDX_DEBUG
    // データ整合性をチェック.
    {
        assert(!m_Blob.empty());
        flatbuffers::Verifier verifier(m_Blob.data(), m_Blob.size());
        assert(res::VerifyModelBinaryBuffer(verifier));
        ASDX_UNUSED(verifier);
    }
#endif
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void ModelBinary::Term()
{
    m_Blob.clear();
    m_Blob.shrink_to_fit();
}

//-----------------------------------------------------------------------------
//      メッシュ数を取得します.
//-----------------------------------------------------------------------------
uint32_t ModelBinary::GetMeshCount() const
{
    auto bin = res::GetModelBinary(m_Blob.data());
    assert(bin != nullptr);

    return bin->Meshes()->size();
}

//-----------------------------------------------------------------------------
//      メッシュリソースを取得します.
//-----------------------------------------------------------------------------
ResMesh ModelBinary::GetMesh(uint32_t index) const
{
    auto bin = res::GetModelBinary(m_Blob.data());
    assert(bin != nullptr);

    auto mesh = bin->Meshes()->Get(index);

    ResMesh result = {};
    result.Hash             = mesh->Hash();
    result.MaterialId       = mesh->MaterialId();
    result.Positions        = ToArrayView<Vector3>(mesh->Positions());
    result.Normals          = ToArrayView<Vector3>(mesh->Normals());
    result.Tangents         = ToArrayView<Vector4>(mesh->Tangents());
    result.Colors           = ToArrayView<Unorm4>(mesh->Colors());
    result.TexCoords        = ToArrayView<Vector2>(mesh->TexCoords());
    result.BoneWeights      = ToArrayView<Vector4>(mesh->BoneWeights());
    result.BoneIndices      = ToArrayView<Uint4>(mesh->BoneIndices());
    result.VertexIndices    = ToArrayView<uint32_t>(mesh->VertexIndices());

    return result;
}

//-----------------------------------------------------------------------------
//      マテリアル数を取得します.
//-----------------------------------------------------------------------------
uint32_t ModelBinary::GetMaterialCount() const
{
    auto bin = res::GetModelBinary(m_Blob.data());
    assert(bin != nullptr);

    return bin->Materials()->size();
}

//-----------------------------------------------------------------------------
//      マテリアル名ハッシュを取得します.
//-----------------------------------------------------------------------------
uint64_t ModelBinary::GetMaterialHash(uint32_t index) const
{
    auto bin = res::GetModelBinary(m_Blob.data());
    assert(bin != nullptr);

    return bin->Materials()->Get(index);
}

//-----------------------------------------------------------------------------
//      ボーン数を取得します.
//-----------------------------------------------------------------------------
uint32_t ModelBinary::GetBoneCount() const
{
    auto bin = res::GetModelBinary(m_Blob.data());
    assert(bin != nullptr);

    return bin->Bones()->size();
}

//-----------------------------------------------------------------------------
//      ボーンリソースを取得します.
//-----------------------------------------------------------------------------
ResBone ModelBinary::GetBone(uint32_t index) const
{
    auto bin = res::GetModelBinary(m_Blob.data());
    assert(bin != nullptr);

    auto bone = bin->Bones()->Get(index);

    ResBone result = {};
    result.Hash         = bone->Hash();
    result.Parent       = bone->Parent();
    result.BindPose     = FromFloat3x4(*bone->BindPose());
    result.InvBindPose  = FromFloat3x4(*bone->InverseBindPose());
    result.Children     = ToArrayView<BoneId>(bone->Children());

    return result;
}

//-----------------------------------------------------------------------------
//      メッシュバッチ数を取得します.
//-----------------------------------------------------------------------------
uint32_t ModelBinary::GetMeshBatchCount() const
{
    auto bin = res::GetModelBinary(m_Blob.data());
    assert(bin != nullptr);

    return bin->MeshBatches()->size();
}

//-----------------------------------------------------------------------------
//      メッシュバッチリソースを取得します.
//-----------------------------------------------------------------------------
ResMeshBatch ModelBinary::GetMeshBatch(uint32_t index) const
{
    auto bin = res::GetModelBinary(m_Blob.data());
    assert(bin != nullptr);

    auto batch = bin->MeshBatches()->Get(index);

    ResMeshBatch result = {};
    result.Meshes    = ToArrayView<uint32_t>(batch->Meshes());
    result.Instances = ToArrayView<ResMeshInstance>(batch->Instances());
    return result;
}

//-----------------------------------------------------------------------------
//      ハッシュテーブル数を取得します.
//-----------------------------------------------------------------------------
uint32_t ModelBinary::GetHashStringCount() const
{
    auto bin = res::GetModelBinary(m_Blob.data());
    assert(bin != nullptr);

    return bin->HashTable()->size();
}

//-----------------------------------------------------------------------------
//      ハッシュテーブルを取得します.
//-----------------------------------------------------------------------------
ResHashString ModelBinary::GetHashString(uint32_t index) const
{
    auto bin = res::GetModelBinary(m_Blob.data());
    assert(bin != nullptr);

    auto pair = bin->HashTable()->Get(index);

    ResHashString result = {};
    result.Hash = pair->Hash();
    result.View = StringView(pair->Text()->c_str());
    return result;
}

//-----------------------------------------------------------------------------
//      名前を検索します.
//-----------------------------------------------------------------------------
StringView ModelBinary::FindName(uint64_t hash) const
{
    auto bin = res::GetModelBinary(m_Blob.data());
    assert(bin != nullptr);

    auto ret = bin->HashTable()->LookupByKey(hash);
    if (ret == nullptr)
        return StringView();

    return StringView(ret->Text()->c_str());
}

} // namespace asdx

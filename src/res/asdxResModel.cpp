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
#include "ModelBinary_generated.h"


namespace {

static_assert((uint8_t)asdx::res::AlphaType_Opaque == (uint8_t)asdx::AlphaMode::Opaque);
static_assert((uint8_t)asdx::res::AlphaType_Mask   == (uint8_t)asdx::AlphaMode::Mask);
static_assert((uint8_t)asdx::res::AlphaType_Blend  == (uint8_t)asdx::AlphaMode::Blend);

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t CURRENT_VERSION = 3u; //!< 現在ランタイムでサポートされているバージョン.

//-----------------------------------------------------------------------------
//      配列ビューに変換します.
//-----------------------------------------------------------------------------
template<typename T, typename U>
asdx::ArrayView<T> ToArrayView(const flatbuffers::Vector<U>* value)
{
    if (value == nullptr)
        return asdx::ArrayView<T>();

    return asdx::ArrayView<T>(reinterpret_cast<const T*>(value->data()), value->size()); 
}

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
    assert(!m_Blob.empty());
    return res::GetModelBinary(m_Blob.data())->Meshes()->size();
}

//-----------------------------------------------------------------------------
//      ボーン数を取得します.
//-----------------------------------------------------------------------------
uint32_t ModelBinary::GetBoneCount() const
{
    assert(!m_Blob.empty());
    return res::GetModelBinary(m_Blob.data())->Bones()->size();
}

//-----------------------------------------------------------------------------
//      マテリアル数を取得します.
//-----------------------------------------------------------------------------
uint32_t ModelBinary::GetMaterialCount() const
{
    assert(!m_Blob.empty());
    return res::GetModelBinary(m_Blob.data())->Materials()->size();
}

//-----------------------------------------------------------------------------
//      バッチ数を取得します.
//-----------------------------------------------------------------------------
uint32_t ModelBinary::GetBatchCount() const
{
    assert(!m_Blob.empty());
    return res::GetModelBinary(m_Blob.data())->Batches()->size();
}

//-----------------------------------------------------------------------------
//      総インスタンス数を取得します.
//-----------------------------------------------------------------------------
uint64_t ModelBinary::GetTotalInstanceCount() const
{
    assert(!m_Blob.empty());
    return res::GetModelBinary(m_Blob.data())->TotalInstanceCount();
}

//-----------------------------------------------------------------------------
//      メッシュを取得します.
//-----------------------------------------------------------------------------
const res::Mesh& ModelBinary::GetMesh(uint32_t index) const
{
    assert(!m_Blob.empty());
    auto bin  = res::GetModelBinary(m_Blob.data());
    return *(bin->Meshes()->Get(index));
}

//-----------------------------------------------------------------------------
//      ボーンを取得します.
//-----------------------------------------------------------------------------
const res::Bone& ModelBinary::GetBone(uint32_t index) const
{
    assert(!m_Blob.empty());
    auto bin = res::GetModelBinary(m_Blob.data());
    return *(bin->Bones()->Get(index));
}

//-----------------------------------------------------------------------------
//      マテリアルを取得します.
//-----------------------------------------------------------------------------
const res::Material& ModelBinary::GetMaterial(uint32_t index) const
{
    assert(!m_Blob.empty());
    return *(res::GetModelBinary(m_Blob.data())->Materials()->Get(index));
}

//-----------------------------------------------------------------------------
//      バッチを取得します.
//-----------------------------------------------------------------------------
const res::ModelBatch& ModelBinary::GetBatch(uint32_t index) const
{
    assert(!m_Blob.empty());
    return *(res::GetModelBinary(m_Blob.data())->Batches()->Get(index));
}

//-----------------------------------------------------------------------------
//      バウンディングスフィアを取得します.
//-----------------------------------------------------------------------------
BoundingSphere3 ModelBinary::GetSphere() const
{
    assert(!m_Blob.empty());
    auto val = res::GetModelBinary(m_Blob.data())->BoundSphere();
    return BoundingSphere3(val->Center().X(), val->Center().Y(), val->Center().Z(), val->Radius());
}

//-----------------------------------------------------------------------------
//      バウンディングボックスを取得します.
//-----------------------------------------------------------------------------
BoundingBox3 ModelBinary::GetBox() const
{
    assert(!m_Blob.empty());
    auto val = res::GetModelBinary(m_Blob.data())->BoundBox();
    return BoundingBox3(
        asdx::Vector3(val->Min().X(), val->Min().Y(), val->Min().Z()),
        asdx::Vector3(val->Max().X(), val->Max().Y(), val->Max().Z()));
}

//-----------------------------------------------------------------------------
//      ボーンを検索します.
//-----------------------------------------------------------------------------
bool ModelBinary::FindBone(const char* name, uint32_t& index) const
{
    if (name == nullptr)
    {
        index = UINT32_MAX;
        return false;
    }

    assert(!m_Blob.empty());
    auto bones = res::GetModelBinary(m_Blob.data())->Bones();

    // 2分探索するために，名前順にソートしてしまうと，
    // ボーン番号が正しい値にならず不具合を引き起こすため，線形検索を行う.
    for(auto i=0u; i<bones->size(); ++i)
    {
        if (strcmp(bones->Get(i)->Name()->c_str(), name) == 0)
        {
            index = i;
            return true;
        }
    }

    index = UINT32_MAX;
    return false;
}

//-----------------------------------------------------------------------------
//      マテリアルを検索します.
//-----------------------------------------------------------------------------
bool ModelBinary::FindMaterial(const char* name, uint32_t& index) const
{
    if (name == nullptr)
    {
        index = UINT32_MAX;
        return false;
    }

    assert(!m_Blob.empty());
    auto mats = res::GetModelBinary(m_Blob.data())->Materials();

    // 2分探索するために，名前順にソートしてしまうと，
    // マテリアル番号が正しい値にならず不具合を引き起こすため，線形検索を行う.
    for(auto i=0u; i<mats->size(); ++i)
    {
        if (strcmp(mats->Get(i)->Name()->c_str(), name)  == 0)
        {
            index = i;
            return true;
        }
    }

    index = UINT32_MAX;
    return false;
}

//-----------------------------------------------------------------------------
//      メッシュを検索します.
//-----------------------------------------------------------------------------
bool ModelBinary::FindMesh(const char* name, uint32_t& index) const
{
    if (name == nullptr)
    {
        index = UINT32_MAX;
        return false;
    }

    assert(!m_Blob.empty());
    auto meshes = res::GetModelBinary(m_Blob.data())->Meshes();

    // 2分探索するために，名前順にソートしてしまうと，
    // メッシュ番号が正しい値にならず不具合を引き起こすため，線形検索を行う.
    for(auto i=0u; i<meshes->size(); ++i)
    {
        if (strcmp(meshes->Get(i)->Name()->c_str(), name)  == 0)
        {
            index = i;
            return true;
        }
    }

    index = UINT32_MAX;
    return false;
}

//-----------------------------------------------------------------------------
//      モデルインスタンスを検索します.
//-----------------------------------------------------------------------------
bool ModelBinary::FindInstance(const char* name, uint32_t& batchIndex, uint32_t& instanceIndex) const
{
    if (name == nullptr)
    {
        batchIndex    = UINT32_MAX;
        instanceIndex = UINT32_MAX;
        return false;
    }

    assert(!m_Blob.empty());
    auto batches = res::GetModelBinary(m_Blob.data())->Batches();

    for(auto i=0u; i<batches->size(); ++i)
    {
        auto names = batches->Get(i)->Names();
        for(auto j=0u; j<names->size(); ++j)
        {
            if (strcmp(names->Get(j)->c_str(), name) == 0)
            {
                batchIndex    = i;
                instanceIndex = j;
                return true;
            }
        }
    }

    batchIndex    = UINT32_MAX;
    instanceIndex = UINT32_MAX;
    return false;
}



///////////////////////////////////////////////////////////////////////////////
// MeshProxy class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      メッシュ名を取得します.
//-----------------------------------------------------------------------------
StringView MeshProxy::GetName(const res::Mesh& mesh)
{ return StringView(mesh.Name()->c_str()); }

//-----------------------------------------------------------------------------
//      マテリアルIDを取得します.
//-----------------------------------------------------------------------------
uint32_t MeshProxy::GetMaterialId(const res::Mesh& mesh)
{ return mesh.MaterialId(); }

//-----------------------------------------------------------------------------
//      位置配列を取得します.
//-----------------------------------------------------------------------------
ArrayView<Vector3> MeshProxy::GetPositions(const res::Mesh& mesh)
{ return ToArrayView<Vector3>(mesh.Positions()); }

//-----------------------------------------------------------------------------
//      法線配列を取得します.
//-----------------------------------------------------------------------------
ArrayView<Vector3> MeshProxy::GetNormals(const res::Mesh& mesh)
{ return ToArrayView<Vector3>(mesh.Normals()); }

//-----------------------------------------------------------------------------
//      接線配列を取得します.
//-----------------------------------------------------------------------------
ArrayView<Vector4> MeshProxy::GetTangents(const res::Mesh& mesh)
{ return ToArrayView<Vector4>(mesh.Tangents()); }

//-----------------------------------------------------------------------------
//      頂点カラー配列を取得します.
//-----------------------------------------------------------------------------
ArrayView<Unorm4> MeshProxy::GetColors(const res::Mesh& mesh)
{ return ToArrayView<Unorm4>(mesh.Colors()); }

//-----------------------------------------------------------------------------
//      テクスチャ座標配列を取得します.
//-----------------------------------------------------------------------------
ArrayView<Vector2> MeshProxy::GetTexCoords(const res::Mesh& mesh)
{ return ToArrayView<Vector2>(mesh.TexCoords()); }

//-----------------------------------------------------------------------------
//      ボーンインデックス配列を取得します.
//-----------------------------------------------------------------------------
ArrayView<Uint4> MeshProxy::GetBoneIndices(const res::Mesh& mesh)
{ return ToArrayView<Uint4>(mesh.BoneIndices()); }

//-----------------------------------------------------------------------------
//      ボーンウェイト配列を取得します.
//-----------------------------------------------------------------------------
ArrayView<Vector4> MeshProxy::GetBoneWeights(const res::Mesh& mesh)
{ return ToArrayView<Vector4>(mesh.BoneWeights()); }

//-----------------------------------------------------------------------------
//      頂点インデックス配列を取得します.
//-----------------------------------------------------------------------------
ArrayView<uint32_t> MeshProxy::GetVerexIndices(const res::Mesh& mesh)
{ return ToArrayView<uint32_t>(mesh.VertexIndices()); }

//-----------------------------------------------------------------------------
//      バウンディングスフィアを取得します.
//-----------------------------------------------------------------------------
BoundingSphere3 MeshProxy::GetSphere(const res::Mesh& mesh)
{
    auto val = mesh.BoundSphere();
    return BoundingSphere3(
        val->Center().X(),
        val->Center().Y(),
        val->Center().Z(),
        val->Radius());
}

//-----------------------------------------------------------------------------
//      バウンディングスフィアを取得します.
//-----------------------------------------------------------------------------
BoundingBox3 MeshProxy::GetBox(const res::Mesh& mesh)
{
    auto val = mesh.BoundBox();
    return BoundingBox3(
        asdx::Vector3(val->Min().X(), val->Min().Y(), val->Min().Z()),
        asdx::Vector3(val->Max().X(), val->Max().Y(), val->Max().Z()));
}

///////////////////////////////////////////////////////////////////////////////
// BoneProxy class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      ボーン名を取得します.
//-----------------------------------------------------------------------------
StringView BoneProxy::GetName(const res::Bone& bone)
{ return StringView(bone.Name()->c_str()); }

//-----------------------------------------------------------------------------
//      親ボーンIDを取得します.
//-----------------------------------------------------------------------------
int32_t BoneProxy::GetParentId(const res::Bone& bone)
{ return bone.Parent(); }

//-----------------------------------------------------------------------------
//      バインドポーズ行列を取得します.
//-----------------------------------------------------------------------------
Transform3x4 BoneProxy::GetBindPoseMatrix(const res::Bone& bone)
{ return *reinterpret_cast<const Transform3x4*>(bone.BindPose()); }

//-----------------------------------------------------------------------------
//      バインドポーズ逆行列を取得します.
//-----------------------------------------------------------------------------
Transform3x4 BoneProxy::GetInverseBindPoseMatrix(const res::Bone& bone)
{ return *reinterpret_cast<const Transform3x4*>(bone.InverseBindPose()); }

//-----------------------------------------------------------------------------
//      子ボーンID配列を取得します.
//-----------------------------------------------------------------------------
ArrayView<int32_t> BoneProxy::GetChildren(const res::Bone& bone)
{ return ToArrayView<int32_t>(bone.Children()); }


///////////////////////////////////////////////////////////////////////////////
// MaterialProxy class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      マテリアル名を取得します.
//-----------------------------------------------------------------------------
StringView MaterialProxy::GetName(const res::Material& material)
{ return StringView(material.Name()->c_str()); }

//-----------------------------------------------------------------------------
//      ベースカラーファクターを取得します.
//-----------------------------------------------------------------------------
Vector3 MaterialProxy::GetBaseColorFactor(const res::Material& material)
{ return *reinterpret_cast<const Vector3*>(material.BaseColorFactor()); }

//-----------------------------------------------------------------------------
//      アルファ値を取得します.
//-----------------------------------------------------------------------------
float MaterialProxy::GetAlpha(const res::Material& material)
{ return material.Alpha(); }

//-----------------------------------------------------------------------------
//      オクルージョンファクターを取得します.
//-----------------------------------------------------------------------------
float MaterialProxy::GetOcclusionFactor(const res::Material& material)
{ return material.OcclusionFactor(); }

//-----------------------------------------------------------------------------
//      ラフネスファクターを取得します.
//-----------------------------------------------------------------------------
float MaterialProxy::GetRoughnessFactor(const res::Material& material)
{ return material.RoughnessFactor(); }

//-----------------------------------------------------------------------------
//      メタルネスファクターを取得します.
//-----------------------------------------------------------------------------
float MaterialProxy::GetMetalnessFactor(const res::Material& material)
{ return material.MetalnessFactor(); }

//-----------------------------------------------------------------------------
//      エミッシブファクターを取得します.
//-----------------------------------------------------------------------------
Vector3 MaterialProxy::GetEmissiveFactor(const res::Material& material)
{ return *reinterpret_cast<const Vector3*>(material.EmissiveFactor()); }

//-----------------------------------------------------------------------------
//      屈折率を取得します.
//-----------------------------------------------------------------------------
float MaterialProxy::GetIor(const res::Material& material)
{ return material.Ior(); }

//-----------------------------------------------------------------------------
//      ベースカラーマップのファイルパスを取得します.
//-----------------------------------------------------------------------------
StringView MaterialProxy::GetBaseColorMap(const res::Material& material)
{ return StringView(material.BaseColorMap()->c_str()); }

//-----------------------------------------------------------------------------
//      法線マップのファイルパスを取得します.
//-----------------------------------------------------------------------------
StringView MaterialProxy::GetNormalMap(const res::Material& material)
{ return StringView(material.NormalMap()->c_str()); }

//-----------------------------------------------------------------------------
//      オクルージョン・ラフネス・メタルネスマップのファイルパスを取得します.
//-----------------------------------------------------------------------------
StringView MaterialProxy::GetOrmMap(const res::Material& material)
{ return StringView(material.OrmMap()->c_str()); }

//-----------------------------------------------------------------------------
//      エミッシブマップのファイルパスを取得します.
//-----------------------------------------------------------------------------
StringView MaterialProxy::GetEmissiveMap(const res::Material& material)
{ return StringView(material.EmissiveMap()->c_str()); }

//-----------------------------------------------------------------------------
//      アルファモードを取得します.
//-----------------------------------------------------------------------------
AlphaMode MaterialProxy::GetAlphaMode(const res::Material& material)
{ return AlphaMode(material.AlphaMode()); }

//-----------------------------------------------------------------------------
//      アルファテスト値を取得します.
//-----------------------------------------------------------------------------
float MaterialProxy::GetAlphaCutOff(const res::Material& material)
{ return material.AlphaCutOff(); }

//-----------------------------------------------------------------------------
//      両面描画フラグを取得します.
//-----------------------------------------------------------------------------
bool MaterialProxy::GetTwoSided(const res::Material& material)
{ return material.TwoSided(); }


///////////////////////////////////////////////////////////////////////////////
// ModelBatchProxy class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      インスタンス数を取得します.
//-----------------------------------------------------------------------------
uint32_t ModelBatchProxy::GetInstanceCount(const res::ModelBatch& batch)
{
    assert(batch.Transforms()->size() == batch.Names()->size());
    return batch.Transforms()->size();
}

//-----------------------------------------------------------------------------
//      インスタンス名を取得します.
//-----------------------------------------------------------------------------
StringView ModelBatchProxy::GetName(const res::ModelBatch& batch, uint32_t index)
{
    assert(index < batch.Names()->size());
    return StringView(batch.Names()->Get(index)->c_str());
}

//-----------------------------------------------------------------------------
//      変換行列を取得します.
//-----------------------------------------------------------------------------
ArrayView<Transform3x4> ModelBatchProxy::GetTransforms(const res::ModelBatch& batch)
{
    return asdx::ArrayView<Transform3x4>(
        reinterpret_cast<const Transform3x4*>(batch.Transforms()->data()),
        batch.Transforms()->size());
}

//-----------------------------------------------------------------------------
//      メッシュ番号の配列を取得します.
//-----------------------------------------------------------------------------
ArrayView<uint32_t> ModelBatchProxy::GetMeshIds(const res::ModelBatch& batch)
{ return ToArrayView<uint32_t>(batch.Meshes()); }

//-----------------------------------------------------------------------------
//      ローカル空間でのバウンディングスフィアを取得します.
//-----------------------------------------------------------------------------
BoundingSphere3 ModelBatchProxy::GetSphere(const res::ModelBatch& batch)
{
    return BoundingSphere3(
        batch.BoundSphere()->Center().X(),
        batch.BoundSphere()->Center().Y(),
        batch.BoundSphere()->Center().Z(),
        batch.BoundSphere()->Radius());
}

//-----------------------------------------------------------------------------
//      ローカル空間でのバウンディングボックスを取得します.
//-----------------------------------------------------------------------------
BoundingBox3 ModelBatchProxy::GetBox(const res::ModelBatch& batch)
{
    return BoundingBox3(
        Vector3(batch.BoundBox()->Min().X(),
                batch.BoundBox()->Min().Y(),
                batch.BoundBox()->Min().Z()),
        Vector3(batch.BoundBox()->Max().X(),
                batch.BoundBox()->Max().Y(),
                batch.BoundBox()->Max().Z()));
}


} // namespace asdx

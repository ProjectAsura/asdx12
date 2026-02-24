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

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t CURRENT_VERSION = 1u; //!< 現在ランタイムでサポートされているバージョン.

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
const res::Material& ModelBinary::GetMaterial(uint32_t materialIndex) const
{
    assert(!m_Blob.empty());
    return *(res::GetModelBinary(m_Blob.data())->Materials()->Get(materialIndex));
}

//-----------------------------------------------------------------------------
//      バウンディングスフィアを取得します.
//-----------------------------------------------------------------------------
BoundingSphere3 ModelBinary::GetBoundingSphere() const
{
    assert(!m_Blob.empty());
    auto val = res::GetModelBinary(m_Blob.data())->Bounds();
    return BoundingSphere3(val->Center().X(), val->Center().Y(), val->Center().Z(), val->Radius());
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
BoundingSphere3 MeshProxy::GetBounds(const res::Mesh& mesh)
{
    auto val = mesh.Bounds();
    return BoundingSphere3(
        val->Center().X(),
        val->Center().Y(),
        val->Center().Z(),
        val->Radius());
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
Matrix BoneProxy::GetBindPoseMatrix(const res::Bone& bone)
{ return *reinterpret_cast<const Matrix*>(bone.BindPose()); }

//-----------------------------------------------------------------------------
//      バインドポーズ逆行列を取得します.
//-----------------------------------------------------------------------------
Matrix BoneProxy::GetInverseBindPoseMatrix(const res::Bone& bone)
{ return *reinterpret_cast<const Matrix*>(bone.InverseBindPose()); }

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

} // namespace asdx

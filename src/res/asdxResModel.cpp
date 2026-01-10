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

//-----------------------------------------------------------------------------
//      名前から番号を求めます.
//-----------------------------------------------------------------------------
template<typename T>
bool FindIndex(T& items, const char* name, uint32_t& index)
{
    if (name == nullptr)
    {
        index = UINT32_MAX;
        return false;
    }

    uint32_t lhs = 0;
    uint32_t rhs = items->size();
 
    while(lhs < rhs)
    {
        uint32_t mid = lhs + (rhs - lhs) / 2;
        auto item = items->Get(mid);
        auto ret = item->KeyCompareWithValue(name);
        if (ret == 0)
        {
            index = mid;
            return true;
        }
        else if (ret < 0)
        {
            lhs = mid + 1;
        }
        else
        {
            rhs = mid;
        }
    }

    index = UINT32_MAX;
    return false;
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
const char* ModelBinary::GetMaterial(uint32_t materialIndex) const
{
    assert(!m_Blob.empty());
    return res::GetModelBinary(m_Blob.data())->Materials()->Get(materialIndex)->c_str();
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
    assert(!m_Blob.empty());
    auto bones = res::GetModelBinary(m_Blob.data())->Bones();
    return FindIndex(bones, name, index);
}

//-----------------------------------------------------------------------------
//      マテリアルを検索します.
//-----------------------------------------------------------------------------
bool ModelBinary::FindMaterial(const char* name, uint32_t& index) const
{
    assert(!m_Blob.empty());
    auto mats = res::GetModelBinary(m_Blob.data())->Materials();
    if (name == nullptr)
    {
        index = UINT32_MAX;
        return false;
    }

    uint32_t lhs = 0;
    uint32_t rhs = mats->size();
 
    while(lhs < rhs)
    {
        uint32_t mid = lhs + (rhs - lhs) / 2;
        auto mat = mats->Get(mid);
        auto ret = strcmp(mat->c_str(), name);
        if (ret == 0)
        {
            index = mid;
            return true;
        }
        else if (ret < 0)
        {
            lhs = mid + 1;
        }
        else
        {
            rhs = mid;
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
    assert(!m_Blob.empty());
    auto meshes = res::GetModelBinary(m_Blob.data())->Meshes();
    return FindIndex(meshes, name, index);
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
//      オフセット行列を取得します.
//-----------------------------------------------------------------------------
Matrix BoneProxy::GetOffsetMatrix(const res::Bone& bone)
{ return *reinterpret_cast<const Matrix*>(bone.OffsetMatrix()); }

//-----------------------------------------------------------------------------
//      子ボーンID配列を取得します.
//-----------------------------------------------------------------------------
ArrayView<int32_t> BoneProxy::GetChildren(const res::Bone& bone)
{ return ToArrayView<int32_t>(bone.Children()); }

} // namespace asdx

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
ResMesh ModelBinary::GetMesh(uint32_t meshIndex) const
{
    assert(!m_Blob.empty());
    auto bin  = res::GetModelBinary(m_Blob.data());
    auto mesh = bin->Meshes()->Get(meshIndex);

    ResMesh result;
    result.Name           = StringView(mesh->Name()->c_str());
    result.MaterialId     = mesh->MaterialId();
    result.Positions      = ToArrayView<Vector3> (mesh->Positions    ());
    result.Normals        = ToArrayView<Vector3> (mesh->Normals      ());
    result.Tangents       = ToArrayView<Vector4> (mesh->Tangents     ());
    result.Colors         = ToArrayView<Unorm4>  (mesh->Colors       ());
    result.TexCoords      = ToArrayView<Vector2> (mesh->TexCoords    ());
    result.BoneIndices    = ToArrayView<Uint4>   (mesh->BoneIndices  ());
    result.BoneWeights    = ToArrayView<Vector4> (mesh->BoneWeights  ());
    result.VertexIndices  = ToArrayView<uint32_t>(mesh->VertexIndices());
    result.BoundingSphere = BoundingSphere3(mesh->Bounds()->Center().X(), mesh->Bounds()->Center().Y(), mesh->Bounds()->Center().Z(), mesh->Bounds()->Radius());
    return result;
}

//-----------------------------------------------------------------------------
//      ボーンを取得します.
//-----------------------------------------------------------------------------
ResBone ModelBinary::GetBone(uint32_t meshIndex) const
{
    assert(!m_Blob.empty());
    auto bin = res::GetModelBinary(m_Blob.data());
    auto bone = bin->Bones()->Get(meshIndex);

    ResBone result;
    result.Name           = StringView(bone->Name()->c_str());
    result.ParentId       = bone->Parent();
    result.OffsetMatrix   = *reinterpret_cast<const Matrix*>(bone->OffsetMatrix());
    result.Children       = ToArrayView<int32_t>(bone->Children());

    return result;
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
bool ModelBinary::FindBone(const char* name, ResBone& result) const
{
    assert(!m_Blob.empty());
    auto bone = res::GetModelBinary(m_Blob.data())->Bones()->LookupByKey(name);
    if (bone == nullptr)
        return false;

    result.Name         = bone->Name()->c_str();
    result.OffsetMatrix = (*reinterpret_cast<const Matrix*>(bone->OffsetMatrix()));
    return true;
}

//-----------------------------------------------------------------------------
//      マテリアルを検索します.
//-----------------------------------------------------------------------------
bool ModelBinary::FindMaterial(const char* name, uint32_t& materialId) const
{
    assert(!m_Blob.empty());
    auto mats = res::GetModelBinary(m_Blob.data())->Materials();

    uint32_t lhs = 0;
    uint32_t rhs = mats->size();

    while(lhs < rhs)
    {
        uint32_t mid = lhs + (rhs - lhs) / 2;
        auto ret = strcmp(mats->Get(mid)->c_str(), name);
        if (ret == 0)
        {
            materialId = mid;
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

    materialId = UINT32_MAX;
    return false;
}

//-----------------------------------------------------------------------------
//      メッシュを検索します.
//-----------------------------------------------------------------------------
bool ModelBinary::FindMesh(const char* name, uint32_t& meshId) const
{
    assert(!m_Blob.empty());
    auto meshes = res::GetModelBinary(m_Blob.data())->Meshes();

    uint32_t lhs = 0;
    uint32_t rhs = meshes->size();
 
    while(lhs < rhs)
    {
        uint32_t mid = lhs + (rhs - lhs) / 2;
        auto ret = strcmp(meshes->Get(mid)->Name()->c_str(), name);
        if (ret == 0)
        {
            meshId = mid;
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

    meshId = UINT32_MAX;
    return false;
}

} // namespace asdx

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
        flatbuffers::Verifier::Options options;
        flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(m_Blob.data()), m_Blob.size(), options);
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
//      ルート変換行列を取得します.
//-----------------------------------------------------------------------------
const Matrix* ModelBinary::GetRootTransform() const
{
    assert(!m_Blob.empty());
    return reinterpret_cast<const Matrix*>(res::GetModelBinary(m_Blob.data())->RootTransform());
}

//-----------------------------------------------------------------------------
//      逆ルート変換行列を取得します.
//-----------------------------------------------------------------------------
const Matrix* ModelBinary::GetInverseRootTransform() const
{
    assert(!m_Blob.empty());
    return reinterpret_cast<const Matrix*>(res::GetModelBinary(m_Blob.data())->InvRootTransform());
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
//      メッシュ名を取得します.
//-----------------------------------------------------------------------------
const char* ModelBinary::GetMeshName(uint32_t meshIndex) const
{
    assert(!m_Blob.empty());
    auto mesh = res::GetModelBinary(m_Blob.data())->Meshes()->Get(meshIndex);
    return mesh->Name()->c_str();
}

//-----------------------------------------------------------------------------
//      マテリアル名を取得します.
//-----------------------------------------------------------------------------
const char* ModelBinary::GetMaterialName(uint32_t meshIndex) const
{
    assert(!m_Blob.empty());
    auto mesh = res::GetModelBinary(m_Blob.data())->Meshes()->Get(meshIndex);
    return mesh->MaterialTag()->c_str();
}

//-----------------------------------------------------------------------------
//      位置座標を取得します.
//-----------------------------------------------------------------------------
ArrayView<Vector3> ModelBinary::GetPositions(uint32_t meshIndex) const
{
    assert(!m_Blob.empty());
    auto mesh = res::GetModelBinary(m_Blob.data())->Meshes()->Get(meshIndex);
    return ArrayView<Vector3>(
        reinterpret_cast<const Vector3*>(mesh->Positions()->data()),
        mesh->Positions()->size());
}

//-----------------------------------------------------------------------------
//      法線ベクトルを取得します.
//-----------------------------------------------------------------------------
ArrayView<Vector3> ModelBinary::GetNormals(uint32_t meshIndex) const
{
    assert(!m_Blob.empty());
    auto mesh = res::GetModelBinary(m_Blob.data())->Meshes()->Get(meshIndex);
    return ArrayView<Vector3>(
        reinterpret_cast<const Vector3*>(mesh->Normals()->data()),
        mesh->Normals()->size());
}

//-----------------------------------------------------------------------------
//      接線ベクトルを取得します.
//-----------------------------------------------------------------------------
ArrayView<Vector4> ModelBinary::GetTangents(uint32_t meshIndex) const
{
    assert(!m_Blob.empty());
    auto mesh = res::GetModelBinary(m_Blob.data())->Meshes()->Get(meshIndex);
    return ArrayView<Vector4>(
        reinterpret_cast<const Vector4*>(mesh->Tangents()->data()),
        mesh->Tangents()->size());
}

//-----------------------------------------------------------------------------
//      テクスチャ座標を取得します.
//-----------------------------------------------------------------------------
ArrayView<Vector2> ModelBinary::GetTexCoords(uint32_t meshIndex) const
{
    assert(!m_Blob.empty());
    auto mesh = res::GetModelBinary(m_Blob.data())->Meshes()->Get(meshIndex);
    return ArrayView<Vector2>(
        reinterpret_cast<const Vector2*>(mesh->TexCoords()->data()),
        mesh->TexCoords()->size());
}

//-----------------------------------------------------------------------------
//      頂点カラーを取得します.
//-----------------------------------------------------------------------------
ArrayView<Unorm4> ModelBinary::GetColors(uint32_t meshIndex) const
{
    assert(!m_Blob.empty());
    auto mesh = res::GetModelBinary(m_Blob.data())->Meshes()->Get(meshIndex);
    return ArrayView<Unorm4>(
        reinterpret_cast<const Unorm4*>(mesh->Colors()->data()),
        mesh->Colors()->size());
}

//-----------------------------------------------------------------------------
//      ボーンウェイトを取得します.
//-----------------------------------------------------------------------------
ArrayView<Vector4> ModelBinary::GetBoneWeights(uint32_t meshIndex) const
{
    assert(!m_Blob.empty());
    auto mesh = res::GetModelBinary(m_Blob.data())->Meshes()->Get(meshIndex);
    return ArrayView<Vector4>(
        reinterpret_cast<const Vector4*>(mesh->BoneWeights()->data()),
        mesh->BoneWeights()->size());
}

//-----------------------------------------------------------------------------
//      ボーン番号を取得します.
//-----------------------------------------------------------------------------
ArrayView<Uint4> ModelBinary::GetBoneIndices(uint32_t meshIndex) const
{
    assert(!m_Blob.empty());
    auto mesh = res::GetModelBinary(m_Blob.data())->Meshes()->Get(meshIndex);
    return ArrayView<Uint4>(
        reinterpret_cast<const Uint4*>(mesh->BoneIndices()->data()),
        mesh->BoneIndices()->size());
}

//-----------------------------------------------------------------------------
//      頂点インデックスを取得します.
//-----------------------------------------------------------------------------
ArrayView<uint32_t> ModelBinary::GetVertexIndices(uint32_t meshIndex) const
{
    assert(!m_Blob.empty());
    auto mesh = res::GetModelBinary(m_Blob.data())->Meshes()->Get(meshIndex);
    return ArrayView<uint32_t>(
        reinterpret_cast<const uint32_t*>(mesh->VertexIndices()->data()),
        mesh->VertexIndices()->size());
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
//      ボーン名を取得します.
//-----------------------------------------------------------------------------
const char* ModelBinary::GetBoneName(uint32_t boneIndex) const
{
    assert(!m_Blob.empty());
    auto bone = res::GetModelBinary(m_Blob.data())->Bones()->Get(boneIndex);
    return bone->Name()->c_str();
}

//-----------------------------------------------------------------------------
//      ボーンオフセット行列を取得します.
//-----------------------------------------------------------------------------
const Matrix* ModelBinary::GetBoneOffsetMatrix(uint32_t boneIndex) const
{
    assert(!m_Blob.empty());
    auto bone = res::GetModelBinary(m_Blob.data())->Bones()->Get(boneIndex);
    return reinterpret_cast<const Matrix*>(bone->OffsetMatrix());
}

} // namespace asdx

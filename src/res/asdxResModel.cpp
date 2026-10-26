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

    auto bin = asdx::res::GetModelBinary(m_Blob.data());

    m_Meshes.resize(bin->Meshes()->size());
    for(uint32_t i=0; i<m_Meshes.size(); ++i)
    {
        const auto mesh = bin->Meshes()->Get(i);
        m_Meshes[i].Positions       = ToArrayView<Vector3>(mesh->Positions());
        m_Meshes[i].Normals         = ToArrayView<Vector3>(mesh->Normals());
        m_Meshes[i].Tangents        = ToArrayView<Vector4>(mesh->Tangents());
        m_Meshes[i].Colors          = ToArrayView<Unorm4>(mesh->Colors());
        m_Meshes[i].TexCoords       = ToArrayView<Vector2>(mesh->TexCoords());
        m_Meshes[i].BoneWeights     = ToArrayView<Vector4>(mesh->BoneWeights());
        m_Meshes[i].BoneIndices     = ToArrayView<Uint4>(mesh->BoneIndices());
        m_Meshes[i].VertexIndices   = ToArrayView<uint32_t>(mesh->VertexIndices());
    }

    m_Bones.resize(bin->Bones()->size());
    for(uint32_t i=0; i<m_Bones.size(); ++i)
    {
        auto bone = bin->Bones()->Get(i);

        m_Bones[i].Hash         = bone->Hash();
        m_Bones[i].Parent       = bone->Parent();
        m_Bones[i].BindPose     = FromFloat3x4(*bone->BindPose());
        m_Bones[i].InvBindPose  = FromFloat3x4(*bone->InverseBindPose());
        m_Bones[i].Children     = ToArrayView<int32_t>(bone->Children());
    }

    m_Instances.resize(bin->MeshInstances()->size());
    for(uint32_t i=0; i<m_Instances.size(); ++i)
    {
        auto instance = bin->MeshInstances()->Get(i);

        m_Instances[i].Hashes    = ToArrayView<uint64_t>(instance->Hashes());
        m_Instances[i].Meshes    = ToArrayView<uint32_t>(instance->Meshes());
        m_Instances[i].Transform = ToArrayView<Matrix4x3>(instance->Transforms());
    }

    m_HashTable.reserve(bin->HashTable()->size());
    for(uint32_t i=0u; i<m_HashTable.size(); ++i)
    {
        auto pair = bin->HashTable()->Get(i);
        m_HashTable[i].Hash = pair->Hash();
        m_HashTable[i].Name = StringView(pair->Name()->c_str());
    }
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void ModelBinary::Term()
{
    m_Meshes.clear();
    m_Meshes.shrink_to_fit();

    m_Bones.clear();
    m_Bones.shrink_to_fit();

    m_Instances.clear();
    m_Instances.shrink_to_fit();

    m_HashTable.clear();
    m_HashTable.shrink_to_fit();

    m_Bones.clear();
    m_Bones.shrink_to_fit();
}

//-----------------------------------------------------------------------------
//      モデルリソースを取得します.
//-----------------------------------------------------------------------------
ResModel ModelBinary::GetResource() const
{
    auto bin = asdx::res::GetModelBinary(m_Blob.data());

    ResModel result = {};
    result.Meshes        = ArrayView<ResMesh>(m_Meshes.data(), m_Meshes.size());
    result.Materials     = ArrayView<uint64_t>(bin->Materials()->data(), bin->Materials()->size());
    result.Bones         = ArrayView<ResBone>(m_Bones.data(), m_Bones.size());
    result.MeshInstances = ArrayView<ResMeshInstance>(m_Instances.data(), m_Instances.size());
    result.HashTable     = ArrayView<ResNamePair>(m_HashTable.data(), m_HashTable.size());

    return result;
}

} // namespace asdx

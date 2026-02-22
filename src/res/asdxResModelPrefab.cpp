//-----------------------------------------------------------------------------
// File : asdxResModelPrefab.cpp
// Desc : Model Prefab Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Inlcudes
//-----------------------------------------------------------------------------
#include <fnd/asdxMacro.h>
#include <fnd/asdxLogger.h>
#include <res/asdxResModelPrefab.h>
#include "ModelPrefabBinary_generated.h"


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t CURRENT_VERSION = 1u; //!< 現在ランタイムでサポートされているバージョン.

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ModelPrefabBinary class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ModelPrefabBinary::ModelPrefabBinary()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ModelPrefabBinary::~ModelPrefabBinary()
{ Term(); }

//-----------------------------------------------------------------------------
//      バイナリをロードします.
//-----------------------------------------------------------------------------
void ModelPrefabBinary::Load(std::vector<uint8_t>&& blob)
{
    m_Blob = std::move(blob);

#if ASDX_DEBUG
    // データ整合性をチェック.
    {
        assert(!m_Blob.empty());
        flatbuffers::Verifier verifier(m_Blob.data(), m_Blob.size());
        assert(res::VerifyModelPrefabBinaryBuffer(verifier));
        ASDX_UNUSED(verifier);
    }
#endif
}

//-----------------------------------------------------------------------------
//      終了処理です.
//-----------------------------------------------------------------------------
void ModelPrefabBinary::Term()
{
    m_Blob.clear();
    m_Blob.shrink_to_fit();
}

//-----------------------------------------------------------------------------
//      モデルのバイナリファイルパスを取得します.
//-----------------------------------------------------------------------------
StringView ModelPrefabBinary::GetModelPath() const
{
    assert(!m_Blob.empty());
    auto bin = asdx::res::GetModelPrefabBinary(m_Blob.data());
    return StringView(bin->ModelPath()->c_str());
}

//-----------------------------------------------------------------------------
//      マテリアル数を取得します.
//-----------------------------------------------------------------------------
uint32_t ModelPrefabBinary::GetMaterialCount() const
{
    assert(!m_Blob.empty());
    auto bin = asdx::res::GetModelPrefabBinary(m_Blob.data());
    return bin->Materials()->size();
}

//-----------------------------------------------------------------------------
//      マテリアルを取得します.
//-----------------------------------------------------------------------------
ResMeshMaterial ModelPrefabBinary::GetMaterial(uint32_t index) const
{
    assert(!m_Blob.empty());
    auto materials = asdx::res::GetModelPrefabBinary(m_Blob.data())->Materials();
    assert(index < materials->size());
    auto mat = materials->Get(index);

    ResMeshMaterial result = {};
    result.Name = StringView(mat->Name()->c_str());
    result.Path = StringView(mat->Path()->c_str());
    return result;
}

//-----------------------------------------------------------------------------
//      マテリアルを検索します.
//-----------------------------------------------------------------------------
bool ModelPrefabBinary::FindMaterial(const char* name, uint32_t& index) const
{
    assert(!m_Blob.empty());
    auto materials = asdx::res::GetModelPrefabBinary(m_Blob.data())->Materials();
    if (materials == nullptr)
    {
        index = UINT32_MAX;
        return false;
    }

    uint32_t lhs = 0;
    uint32_t rhs = materials->size();

    while(lhs < rhs)
    {
        uint32_t mid = lhs + (rhs - lhs) / 2;
        auto mat = materials->Get(mid);
        auto ret = strcmp(mat->Name()->c_str(), name);
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
//      マテリアルを検索します.
//-----------------------------------------------------------------------------
bool ModelPrefabBinary::FindMaterial(const char* name, ResMeshMaterial& result) const
{
    assert(!m_Blob.empty());
    auto materials = asdx::res::GetModelPrefabBinary(m_Blob.data())->Materials();
    auto mat = materials->LookupByKey(name);
    if (mat == nullptr)
        return false;

    result.Name = StringView(mat->Name()->c_str());
    result.Path = StringView(mat->Path()->c_str());
    return true;
}


} // namespace asdx
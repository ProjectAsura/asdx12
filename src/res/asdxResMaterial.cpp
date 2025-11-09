//-----------------------------------------------------------------------------
// File : asdxResMaterial.cpp
// Desc : Material Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxMacro.h>
#include <fnd/asdxLogger.h>
#include <res/asdxResMaterial.h>
#include "MaterialBinary_generated.h"
#include <res/asdxResHelper.h>

namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t CURRENT_VERSION = 1u;     //!< 現在ランタイムでサポートされているバージョン.

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// MaterialBinary class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
MaterialBinary::MaterialBinary()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
MaterialBinary::~MaterialBinary()
{ Term(); }

//-----------------------------------------------------------------------------
//      メモリからロードします.
//-----------------------------------------------------------------------------
void MaterialBinary::Load(std::vector<uint8_t>&& blob)
{
    m_Blob = std::move(blob);

#if ASDX_DEBUG
    // データ整合性をチェック.
    {
        assert(!m_Blob.empty());
        flatbuffers::Verifier verifier(m_Blob.data(), m_Blob.size());
        assert(res::VerifyMaterialBinaryBuffer(verifier));
        ASDX_UNUSED(verifier);
    }
#endif
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void MaterialBinary::Term()
{
    m_Blob.clear();
    m_Blob.shrink_to_fit();
}

//-----------------------------------------------------------------------------
//      マテリアル数を取得します.
//-----------------------------------------------------------------------------
uint32_t MaterialBinary::GetMaterialCount() const
{
    assert(!m_Blob.empty());
    return res::GetMaterialBinary(m_Blob.data())->Materials()->size();
}

//-----------------------------------------------------------------------------
//      マテリアルを取得します.
//-----------------------------------------------------------------------------
ResMaterial MaterialBinary::GetMaterial(uint32_t index) const
{
    assert(!m_Blob.empty());
    auto mat = res::GetMaterialBinary(m_Blob.data())->Materials()->Get(index);

    ResMaterial result = {};
    result.BindName         = StringView(mat->BindName    ()->c_str());
    result.BaseColorMap     = StringView(mat->BaseColorMap()->c_str());
    result.NormalMap        = StringView(mat->NormalMap   ()->c_str());
    result.OrmMap           = StringView(mat->OrmMap      ()->c_str());
    result.EmissiveMap      = StringView(mat->EmissiveMap ()->c_str());
    result.BaseColorFactor  = FromFloat3(*(mat->BaseColorFactor()));
    result.OcclusionFactor  = mat->OcclusionFactor();
    result.RoughnessFactor  = mat->RoughnessFactor();
    result.MetalnessFactor  = mat->MetalnessFactor();
    result.EmissiveFactor   = FromFloat3(*(mat->EmissiveFactor()));
    result.Ior              = mat->Ior();
    result.AlphaThreshold   = mat->AlphaThreshold();

    return result;
}

//-----------------------------------------------------------------------------
//      マテリアルを検索します.
//-----------------------------------------------------------------------------
bool MaterialBinary::FindMaterial(const char* name, ResMaterial& result) const
{
    assert(!m_Blob.empty());
    auto mat = res::GetMaterialBinary(m_Blob.data())->Materials();
    auto find = mat->LookupByKey(name);
    if (find == nullptr)
        return false;

    result.BindName         = StringView(find->BindName    ()->c_str());
    result.BaseColorMap     = StringView(find->BaseColorMap()->c_str());
    result.NormalMap        = StringView(find->NormalMap   ()->c_str());
    result.OrmMap           = StringView(find->OrmMap      ()->c_str());
    result.EmissiveMap      = StringView(find->EmissiveMap ()->c_str());
    result.BaseColorFactor  = FromFloat3(*(find->BaseColorFactor()));
    result.OcclusionFactor  = find->OcclusionFactor();
    result.RoughnessFactor  = find->RoughnessFactor();
    result.MetalnessFactor  = find->MetalnessFactor();
    result.EmissiveFactor   = FromFloat3(*(find->EmissiveFactor()));
    result.Ior              = find->Ior();
    result.AlphaThreshold   = find->AlphaThreshold();

    return true;
}

//-----------------------------------------------------------------------------
//      マテリアル番号を検索します.
//-----------------------------------------------------------------------------
bool MaterialBinary::FindMaterialId(const char* name, uint32_t& result) const
{
    assert(!m_Blob.empty());
    auto mat = res::GetMaterialBinary(m_Blob.data())->Materials();

    uint32_t lhs = 0;
    uint32_t rhs = mat->size();

    while(lhs < rhs)
    {
        uint32_t mid = lhs + (rhs - lhs) / 2;
        auto ret = strcmp(mat->Get(mid)->BindName()->c_str(), name);
        if (ret == 0)
        {
            result = mid;
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

    result = UINT32_MAX;
    return false;
}

} // namespace asdx

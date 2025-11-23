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


//-----------------------------------------------------------------------------
//      ResMaterialに変換します.
//-----------------------------------------------------------------------------
void ToResMaterial(asdx::ResMaterial& result, const asdx::res::Material* mat)
{
   result.BindName = asdx::StringView(mat->BindName()->c_str());

    for(auto tex : *(mat->Textures()))
    {
        auto path = tex->Path()->c_str();
        switch(tex->Kind())
        {
        case asdx::res::TextureKind_BaseColor:
            result.BaseColorMap = asdx::StringView(path);
            break;

        case asdx::res::TextureKind_Normal:
            result.NormalMap = asdx::StringView(path);
            break;

        case asdx::res::TextureKind_Orm:
            result.OrmMap = asdx::StringView(path);
            break;

        case asdx::res::TextureKind_Emissive:
            result.EmissiveMap = asdx::StringView(path);
            break;

        case asdx::res::TextureKind_Anisotropy:
            result.AnisotropyMap = asdx::StringView(path);
            break;

        case asdx::res::TextureKind_ClearCoat:
            result.ClearCoatMap = asdx::StringView(path);
            break;

        case asdx::res::TextureKind_ClearCoatNormal:
            result.ClearCoatNormalMap = asdx::StringView(path);
            break;

        case asdx::res::TextureKind_ClearCoatRoughness:
            result.ClearCoatRoughnessMap = asdx::StringView(path);
            break;

        case asdx::res::TextureKind_SheenColor:
            result.SheenColorMap = asdx::StringView(path);
            break;

        case asdx::res::TextureKind_SheenRoughness:
            result.SheenRoughnessMap = asdx::StringView(path);
            break;

        case asdx::res::TextureKind_Transmission:
            result.TransmissionMap = asdx::StringView(path);
            break;

        case asdx::res::TextureKind_Thickness:
            result.ThicknessMap = asdx::StringView(path);
            break;

        case asdx::res::TextureKind_Iridescence:
            result.IridescenceMap = asdx::StringView(path);
            break;

        case asdx::res::TextureKind_IridescenceThickness:
            result.IridescenceThicknessMap = asdx::StringView(path);
            break;

        default:
            break;
        }
    }

    result.BaseColorFactor              = asdx::FromFloat3(*(mat->BaseColorFactor()));
    result.OcclusionFactor              = mat->OcclusionFactor();
    result.RoughnessFactor              = mat->RoughnessFactor();
    result.MetalnessFactor              = mat->MetalnessFactor();
    result.EmissiveFactor               = asdx::FromFloat3(*(mat->EmissiveFactor()));
    result.AlphaThreshold               = mat->AlphaThreshold();
    result.AnisotropyStrength           = mat->AnisotropyStrength();
    result.AnisotropyRotation           = asdx::FromFloat2(*(mat->AnisotropyRotation()));
    result.ClearCoatFactor              = mat->ClearCoatFactor();
    result.ClearCoatRoughnessFactor     = mat->ClearCoatRoughnessFactor();
    result.SheenColorFactor             = asdx::FromFloat3(*(mat->SheenColorFactor()));
    result.SheenRoughnessFactor         = mat->SheenRoughnessFactor();
    result.Ior                          = mat->Ior();
    result.Dispersion                   = mat->Dispersion();
    result.TransimissionFactor          = mat->TransmissionFactor();
    result.IridescenceFactor            = mat->IridescenceFactor();
    result.IridescenceIor               = mat->IridescenceIor();
    result.IridescenceThicknessMinimum  = mat->IridescenceThicknessMinimum();
    result.IridescenceThicknessMaximum  = mat->iridescenceThicknessMaximum();
}

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
    ToResMaterial(result, mat);
    return result;
}

//-----------------------------------------------------------------------------
//      マテリアルを検索します.
//-----------------------------------------------------------------------------
bool MaterialBinary::FindMaterial(const char* name, ResMaterial& result) const
{
    assert(!m_Blob.empty());
    auto materials = res::GetMaterialBinary(m_Blob.data())->Materials();
    auto mat = materials->LookupByKey(name);
    if (mat == nullptr)
        return false;

    ToResMaterial(result, mat);
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

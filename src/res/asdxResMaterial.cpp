//-----------------------------------------------------------------------------
// File : asdxResMaterial.cpp
// Desc : Material Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxMacro.h>
#include <res/asdxResMaterial.h>
//#include "MaterialBinary_generated.h"


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr uint32_t CURRENT_VERSION = 1u; //!< 現在ランタイムでサポートされているバージョン.


//static_assert((uint8_t)asdx::res::AlphaType_Opaque == (uint8_t)asdx::AlphaMode::Opaque);
//static_assert((uint8_t)asdx::res::AlphaType_Mask   == (uint8_t)asdx::AlphaMode::Mask);
//static_assert((uint8_t)asdx::res::AlphaType_Blend  == (uint8_t)asdx::AlphaMode::Blend);

//static_assert((uint8_t)asdx::res::CullType_Back  == (uint8_t)asdx::CullMode::Back);
//static_assert((uint8_t)asdx::res::CullType_Front == (uint8_t)asdx::CullMode::Front);
//static_assert((uint8_t)asdx::res::CullType_None  == (uint8_t)asdx::CullMode::None);

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
        //assert(!m_Blob.empty());
        //flatbuffers::Verifier verifier(m_Blob.data(), m_Blob.size());
        //assert(res::VerifyMaterialBinaryBuffer(verifier));
        //ASDX_UNUSED(verifier);
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
//      マテリアルリソースを取得します.
//-----------------------------------------------------------------------------
ResMaterial MaterialBinary::GetResource() const
{
    ResMaterial result = {};

    return result;
}

} // namespace asdx

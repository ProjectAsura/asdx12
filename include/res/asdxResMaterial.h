//-----------------------------------------------------------------------------
// File : asdxResMaterial.h
// Desc : Material Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdint>
#include <vector>
#include <string>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// MATERIAL_PARAM_TYPE enum
///////////////////////////////////////////////////////////////////////////////
enum MATERIAL_PARAMETER_TYPE : uint32_t
{
    MATERIAL_PARAM_BOOL,        // bool型です.
    MATERIAL_PARAM_INT,         // int型です.
    MATERIAL_PARAM_UINT,        // uint型です.
    MATERIAL_PARAM_FLOAT,       // float型です.
    MATERIAL_PARAM_FLOAT2,      // float2型です.
    MATERIAL_PARAM_FLOAT3,      // float3型です.
    MATERIAL_PARAM_FLOAT4,      // float4型です.
};

///////////////////////////////////////////////////////////////////////////////
// MATERIAL_STATE enum
///////////////////////////////////////////////////////////////////////////////
enum MATERIAL_STATE : uint8_t
{
    MATERIAL_STATE_OPAQUE,          //!< 不透明.
    MATERIAL_STATE_ALPHABLEND,      //!< アルファブレンド.
    MATERIAL_STATE_ADDTIVE,         //!< 加算.
    MATERIAL_STATE_SUBTRACT,        //!< 減算.
    MATERIAL_STATE_PREMULTIPLIED,   //!< 事前乗算済みアルファ.
    MATERIAL_STATE_MULTIPLY,        //!< 乗算.
    MATERIAL_STATE_SCREEN,          //!< スクリーン合成.
    MATERIAL_STATE_DISTORTION,      //!< 歪み.
};

///////////////////////////////////////////////////////////////////////////////
// DISPLAY_FACE enum
///////////////////////////////////////////////////////////////////////////////
enum DISPLAY_FACE : uint8_t
{
    DISPLAY_FACE_BOTH,      //!< 両面表示.
    DISPLAY_FACE_FRONT,     //!< 表面表示.
    DISPLAY_FACE_BACK,      //!< 裏面表示.
};

///////////////////////////////////////////////////////////////////////////////
// TEXTURE_USAGE enum
///////////////////////////////////////////////////////////////////////////////
enum TEXTURE_USAGE : uint8_t
{
    TEXTURE_USAGE_BASE_COLOR,       //!< ベースカラー.
    TEXTURE_USAGE_MRO,              //!< メタルネス/ラフネス/オクルージョン.
    TEXTURE_USAGE_NORMAL,           //!< 法線マップ.
    TEXTURE_USAGE_TANGENT,          //!< 接線マップ.
    TEXTURE_USAGE_EMISSIVE,         //!< エミッシブ.
    TEXTURE_USAGE_VELOCITY,         //!< 速度マップ.
    TEXTURE_USAGE_FLOW,             //!< フローマップ.
    TEXTURE_USAGE_ALPHA,            //!< アルファマップ.
    TEXTURE_USAGE_HEIGHT,           //!< ハイトマップ.
    TEXTURE_USAGE_DIFFUSE,          //!< ディフューズカラー.
    TEXTURE_USAGE_SPECULAR,         //!< スペキュラーカラー.
    TEXTURE_USAGE_GLOSS,            //!< グロスマップ.
    TEXTURE_USAGE_SMOOTHNESS,       //!< スムースネスマップ.
    TEXTURE_USAGE_OCCLUSION,        //!< オクルージョン.
    TEXTURE_USAGE_ROUGHNESS,        //!< ラフネス.
    TEXTURE_USAGE_METALLNESS,       //!< メタルネス.
    TEXTURE_USAGE_COLOR,            //!< カラーマップ.
    TEXTURE_USAGE_VAT,              //!< 頂点アニメーションテクスチャ.,
    TEXTURE_USAGE_CUSTOM,           //!< カスタム.
};

///////////////////////////////////////////////////////////////////////////////
// ResMaterialParameter structure
///////////////////////////////////////////////////////////////////////////////
struct ResMaterialParameter
{
    uint32_t    Type;       //!< データ型です.
    uint32_t    Offset;     //!< バッファ先頭からのオフセットです.
    std::string Name;       //!< パラメータ名.
};

///////////////////////////////////////////////////////////////////////////////
// ResTexture structure
///////////////////////////////////////////////////////////////////////////////
struct ResMaterialTexture
{
    TEXTURE_USAGE   Usage;  //!< 使用用途.
    std::string     Path;   //!< ファイルパス.
};

///////////////////////////////////////////////////////////////////////////////
// ResMaterial structure
///////////////////////////////////////////////////////////////////////////////
struct ResMaterial
{
    std::string Name;               //!< マテリアル名です.
    std::string PixelShader;        //!< ピクセルシェーダ名.
    uint8_t     State;              //!< マテリアルステートです.
    uint8_t     DisplayFace;        //!< 表示面設定です.
    uint8_t     ShadowCast;         //!< シャドウキャスト.
    uint8_t     ShadowReceive;      //!< シャドウレシーブ.
    uint32_t    ParameterCount;     //!< パラメータ数です.
    uint32_t    TextureCount;       //!< テクスチャ数です.
    uint32_t    BufferSize;         //!< 定数バッファサイズです.

    std::vector<ResMaterialParameter>   Parameters;     //!< マテリアルパラメータ定義です.
    std::vector<ResMaterialTexture>     Textures;       //!< テクスチャです.
    std::vector<uint8_t>                Buffer;         //!< 定数バッファデータです.
};

//-----------------------------------------------------------------------------
//! @brief      マテリアルを保存します.
//!
//! @param[in]      path        出力パス.
//! @param[in]      materials   保存するマテリアルデータ.
//! @retval true    保存に成功.
//! @retval fasel   保存に失敗.
//-----------------------------------------------------------------------------
bool SaveMaterial(const char* path, const ResMaterial& materials);

//-----------------------------------------------------------------------------
//! @brief      マテリアルを読み込みます.
//!
//! @param[in]      path        入力パス.
//! @param[out]     materials   マテリアルデータの格納先.
//! @retval true    読み込みに成功.
//! @retval false   読み込みに失敗.
//-----------------------------------------------------------------------------
bool LoadMaterial(const char* path, ResMaterial& materials);

} // namespace asdx

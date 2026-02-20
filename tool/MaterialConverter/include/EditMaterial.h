//-----------------------------------------------------------------------------
// File : EditMaterial.h
// Desc : Material For Edit Data.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdint>
#include <string>
#include <vector>


//-----------------------------------------------------------------------------
// Linker
//-----------------------------------------------------------------------------
#pragma comment(lib, "dxcompiler.lib")


namespace asdx::edit {

///////////////////////////////////////////////////////////////////////////////
// BlendState enum
///////////////////////////////////////////////////////////////////////////////
enum BlendState : uint8_t
{
    Opaque,         //!< 不透明.
    AlphaBlend,     //!< アルファブレンド.
    Additive,       //!< 加算.
    Subtract,       //!< 減算.
    Premultiplied,  //!< 事前乗算済みアルファ.
    Multiply,       //!< 乗算.
    Screen,         //!< スクリーン.
};

///////////////////////////////////////////////////////////////////////////////
// DepthState enum
///////////////////////////////////////////////////////////////////////////////
enum DepthState : uint8_t
{
    ReadWrite,  //!< 深度テスト ON,  深度書き込み ON.
    ReadOnly,   //!< 深度テスト ON,  深度書き込み OFF.
    WriteOnly,  //!< 深度テスト OFF, 深度書き込み ON.
    None,       //!< 深度テスト OFF, 深度書き込み OFF.
};

///////////////////////////////////////////////////////////////////////////////
// RasterizerState enum
///////////////////////////////////////////////////////////////////////////////
enum RasterizerState : uint8_t
{
    CullNone,   //!< カリングしない.
    CullBack,   //!< 背面カリング.
    CullFront,  //!< 前面カリング.
    Wireframe,  //!< ワイヤーフレーム.
};

///////////////////////////////////////////////////////////////////////////////
// MaterialKind enum
///////////////////////////////////////////////////////////////////////////////
enum MaterialKind : uint32_t
{
    Lambert = 0,
    GGX,
    Anisotropy,
    ClearCoat,
    Sheen,
    Iridescence,
    Transmission,
};

///////////////////////////////////////////////////////////////////////////////
// MaterialTexture structure
///////////////////////////////////////////////////////////////////////////////
struct MaterialTexture
{
    std::string     Name;       //!< バインド名.
    std::string     Path;       //!< テクスチャファイルパス.
};

///////////////////////////////////////////////////////////////////////////////
// MaterialBuffer structure
///////////////////////////////////////////////////////////////////////////////
struct MaterialBuffer
{
    uint32_t                Size;   //!< バッファサイズ.
    std::vector<uint8_t>    Data;   //!< バッファデータ.
};

///////////////////////////////////////////////////////////////////////////////
// Material structure
///////////////////////////////////////////////////////////////////////////////
struct Material
{
    std::string                     Name;               //!< マテリアル名.
    uint32_t                        Kind;               //!< マテリアル種別.
    BlendState                      BlendState;         //!< ブレンドステート.,
    DepthState                      DepthState;         //!< 深度ステート.
    RasterizerState                 RasterizerState;    //!< ラスタライザーステート.
    std::vector<uint8_t>            Buffer;             //!< 定数バッファ.
    std::vector<MaterialTexture>    Textures;           //!< テクスチャリスト.
};

///////////////////////////////////////////////////////////////////////////////
// ParamLambert structure
///////////////////////////////////////////////////////////////////////////////
struct alignas(256) ParamLambert
{
    float   UvOffsetX     = 0.0f;
    float   UvOffsetY     = 0.0f;
    float   UvSizeX       = 1.0f;
    float   UvSizeY       = 1.0f;
    float   UvRotation    = 0.0f;
};

///////////////////////////////////////////////////////////////////////////////
// ParamGGX structure
///////////////////////////////////////////////////////////////////////////////
struct alignas(256) ParamGGX
{
    float   UvOffsetX     = 0.0f;
    float   UvOffsetY     = 0.0f;
    float   UvSizeX       = 1.0f;
    float   UvSizeY       = 1.0f;
    float   UvRotation    = 0.0f;
};

///////////////////////////////////////////////////////////////////////////////
// ParamAnisotropy structure
///////////////////////////////////////////////////////////////////////////////
struct alignas(256) ParamAnisotropy
{
    float   UvOffsetX           = 0.0f;
    float   UvOffsetY           = 0.0f;
    float   UvSizeX             = 1.0f;
    float   UvSizeY             = 1.0f;
    float   UvRotation          = 0.0f;
    float   AnisotropyStrength  = 0.0f;
    float   AnisotropyRotation  = 0.0f;
};

///////////////////////////////////////////////////////////////////////////////
// ParamClearCoat structure
///////////////////////////////////////////////////////////////////////////////
struct alignas(256) ParamClearCoat
{
    float   UvOffsetX                   = 0.0f;
    float   UvOffsetY                   = 0.0f;
    float   UvSizeX                     = 1.0f;
    float   UvSizeY                     = 1.0f;
    float   UvRotation                  = 0.0f;
    float   ClearCoatFactor             = 0.0f;
    float   ClearCoatRoughnessFactor    = 0.0f;
};

///////////////////////////////////////////////////////////////////////////////
// ParamSheen structure
///////////////////////////////////////////////////////////////////////////////
struct alignas(256) ParamSheen
{
    float   UvOffsetX               = 0.0f;
    float   UvOffsetY               = 0.0f;
    float   UvSizeX                 = 1.0f;
    float   UvSizeY                 = 1.0f;
    float   UvRotation              = 0.0f;
    float   SheenColorFactorR       = 0.0f;
    float   SheenColorFactorG       = 0.0f;
    float   SheenColorFactorB       = 0.0f;
    float   SheenRoughnessFactor    = 0.0f;
};

///////////////////////////////////////////////////////////////////////////////
// ParamIridescence structure
///////////////////////////////////////////////////////////////////////////////
struct alignas(256) ParamIridescence
{
    float   UvOffsetX               = 0.0f;
    float   UvOffsetY               = 0.0f;
    float   UvSizeX                 = 1.0f;
    float   UvSizeY                 = 1.0f;
    float   UvRotation              = 0.0f;
    float   IridescenceFactor       = 0.0f;
    float   IridescenceIor          = 1.3f;
    float   IridescenceThicknessMin = 100.0f;
    float   IridescenceThicknessMax = 400.0f;
};

///////////////////////////////////////////////////////////////////////////////
// ParamTransmission
///////////////////////////////////////////////////////////////////////////////
struct alignas(256) ParamTransmission
{
    float   UvOffsetX           = 0.0f;
    float   UvOffsetY           = 0.0f;
    float   UvSizeX             = 1.0f;
    float   UvSizeY             = 1.0f;
    float   UvRotation          = 0.0f;
    float   Ior                 = 1.5f;
    float   TransmissionFactor  = 0.0f;
    float   Dispersion          = 0.0f;
};

//-----------------------------------------------------------------------------
//! @brief      マテリアルを json ファイルに保存します.
//! 
//! @param[in]      path        jsonファイルパス.
//! @param[in]      material    保存するマテリアル.
//! @retval true    保存に成功.
//! @retval false   保存に失敗.
//-----------------------------------------------------------------------------
bool SaveToJson(const char* path, const Material& material);

//-----------------------------------------------------------------------------
//! @brief      json ファイルからマテリアルを読込します.
//! 
//! @param[in]      path        jsonファイルパス.
//! @param[out]     material    マテリアルの格納先.
//! @retval true    読込に成功.
//! @retval false   読込に失敗.
//-----------------------------------------------------------------------------
bool LoadFromJson(const char* path, Material& material);

//-----------------------------------------------------------------------------
//! @brief      マテリアル種別からマテリアルを初期化します.
//! 
//! @param[in]      kind        マテリアル種別.
//! @param[out]     material    マテリアルの格納先.
//! @retval true    初期化に成功.
//! @retval fasle   初期化に失敗.
//-----------------------------------------------------------------------------
bool InitMaterial(MaterialKind kind, Material& material);

} // namespace asdx::edit

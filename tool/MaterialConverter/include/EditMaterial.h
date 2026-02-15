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
#include <any>


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
// MaterialParamType enum.
///////////////////////////////////////////////////////////////////////////////
enum MaterialParamType : uint8_t
{
    Bool,       //!< bool型 (4ybteなので注意).
    Bool2,      //!< bool2型.
    Bool3,      //!< bool3型.
    Bool4,      //!< bool4型.
    Int,        //!< int型.
    Int2,       //!< int2型.
    Int3,       //!< int3型.
    Int4,       //!< int4型.
    Uint,       //!< uint型.
    Uint2,      //!< uint2型.
    Uint3,      //!< uint3型.
    Uint4,      //!< uint4型.
    Float,      //!< float型.
    Float2,     //!< float2型.
    Float3,     //!< float3型.
    Float4,     //!< float4型.
};

///////////////////////////////////////////////////////////////////////////////
// RenderState enum
///////////////////////////////////////////////////////////////////////////////
struct RenderState
{
    BlendState      Blend;      //!< ブレンドステート.
    DepthState      Depth;      //!< 深度ステート.
    RasterizerState Rasterizer; //!< ラスタライザーステート.
    uint8_t         UserFlag;   //!< ユーザーフラグ.
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
// MaterialParam structure
///////////////////////////////////////////////////////////////////////////////
struct MaterialParam
{
    std::string             Name;       //!< パラメータ名.
    MaterialParamType       Type;       //!< データ型.
    uint32_t                Offset;     //!< オフセット.
    uint32_t                ArraySize;  //!< 配列数.
};

///////////////////////////////////////////////////////////////////////////////
// MaterialBuffer structure
///////////////////////////////////////////////////////////////////////////////
struct MaterialBuffer
{
    std::string                 Name;       //!< バッファ名.
    std::vector<uint8_t>        Buffer;     //!< バッファデータ.
    std::vector<MaterialParam>  Params;     //!< リフレクションパラメータ.
};

///////////////////////////////////////////////////////////////////////////////
// Material structure
///////////////////////////////////////////////////////////////////////////////
struct Material
{
    std::string                     Name;           //!< マテリアル名.
    std::string                     PixelShader;    //!< ピクセルシェーダファイルパス.
    RenderState                     State;          //!< レンダーステート.
    std::vector<MaterialTexture>    Textures;       //!< テクスチャリスト.
    std::vector<MaterialBuffer>     Buffers;        //!< バッファリスト.
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
//! @brief      シェーダからマテリアルを初期化します.
//! 
//! @param[in]      path        コンパイル済みシェーダファイル名.
//! @param[out]     material    マテリアルの格納先.
//! @retval true    初期化に成功.
//! @retval fasle   初期化に失敗.
//-----------------------------------------------------------------------------
bool InitFromShader(const char* path, Material& material);

//-----------------------------------------------------------------------------
//! @brief      シェーダからマテリアルを初期化します.
//! 
//! @param[in]      buffer      バッファデータ.
//! @param[in]      bufferSize  バッファサイズ.
//! @param[out]     material    マテリアルの格納先.
//! @retval true    初期化に成功.
//! @retval fasle   初期化に失敗.
//-----------------------------------------------------------------------------
bool InitFromShader(const void* buffer, size_t bufferSize, Material& material);

} // namespace asdx::edit

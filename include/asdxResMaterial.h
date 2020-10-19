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
// MATERIAL_PARAMTERE_TYPE enum
///////////////////////////////////////////////////////////////////////////////
enum MATERIAL_PARAMETER_TYPE
{
    MATERIAL_PARAMETER_BOOL,        // bool型です.
    MATERIAL_PARAMETER_FLOAT,       // float型です.
    MATERIAL_PARAMETER_INT,         // int型です.
    MATERIAL_PARAMETER_UINT,        // uint型です.
};

///////////////////////////////////////////////////////////////////////////////
// MATERIAL_STATE enum
///////////////////////////////////////////////////////////////////////////////
enum MATERIAL_STATE
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
enum DISPLAY_FACE
{
    DISPLAY_FACE_BOTH,      //!< 両面表示.
    DISPLAY_FACE_FRONT,     //!< 表面表示.
    DISPLAY_FACE_BACK,      //!< 裏面表示.
};

///////////////////////////////////////////////////////////////////////////////
// ResMaterialParameter structure
///////////////////////////////////////////////////////////////////////////////
struct ResMaterialParameter
{
    MATERIAL_PARAMETER_TYPE Type;       //!< データ型です.
    uint32_t                Hash;       //!< 名前を表すハッシュ値です.
    uint32_t                Count;      //!< 要素数です.
    uint32_t                Offset;     //!< バッファ先頭からのオフセットです.
};

///////////////////////////////////////////////////////////////////////////////
// ResMaterial structure
///////////////////////////////////////////////////////////////////////////////
struct ResMaterial
{
    uint32_t    Hash;               //!< マテリアル名を表すハッシュ値です.
    uint8_t     State;              //!< マテリアルステートです.
    uint8_t     DisplayFace;        //!< 表示面設定です.
    uint8_t     ShadowCast;         //!< シャドウキャスト.
    uint8_t     ShadowReceive;      //!< シャドウレシーブ.
    uint32_t    ParameterCount;     //!< パラメータ数です.
    uint32_t    TextureCount;       //!< テクスチャ数です.
    uint32_t    BufferSize;         //!< 定数バッファサイズです.

    std::vector<ResMaterialParameter>   Parameters;     //!< マテリアルパラメータ定義です.
    std::vector<std::string>            Textures;       //!< テクスチャファイルパスです.
    std::vector<uint8_t>                pBuffer;        //!< 定数バッファデータです.
};

///////////////////////////////////////////////////////////////////////////////
// ResMaterials structure
///////////////////////////////////////////////////////////////////////////////
struct ResMaterials
{
    std::vector<ResMaterial>        Materials;  //!< マテリアルです.
};

//-----------------------------------------------------------------------------
//! @brief      マテリアルを保存します.
//!
//! @param[in]      path        出力パス.
//! @param[in]      materials   保存するマテリアルデータ.
//! @retval true    保存に成功.
//! @retval fasel   保存に失敗.
//-----------------------------------------------------------------------------
bool SaveMaterials(const char* path, const ResMaterials& materials);

//-----------------------------------------------------------------------------
//! @brief      マテリアルを読み込みます.
//!
//! @param[in]      path        入力パス.
//! @param[out]     materials   マテリアルデータの格納先.
//! @retval true    読み込みに成功.
//! @retval false   読み込みに失敗.
//-----------------------------------------------------------------------------
bool LoadMaterials(const char* path, ResMaterials& materials);

} // namespace asdx

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
// ResMaterialState structure
///////////////////////////////////////////////////////////////////////////////
struct ResMaterialState
{
    uint8_t     Visible       : 1;      // 可視フラグ.
    uint8_t     HasAlpha      : 1;      // アルファを持つか.
    uint8_t     DisplayFace   : 3;      // 表示面.
    uint8_t     ShadowCast    : 1;      // シャドウマップに描画するか?
    uint8_t     ShadowReceive : 1;      // シャドウが落ちるかどうか?
    uint8_t     Reserved      : 1;  
};

///////////////////////////////////////////////////////////////////////////////
// ResMaterial structure
///////////////////////////////////////////////////////////////////////////////
struct ResMaterial
{
    std::string      Name;               //!< マテリアル名です.
    ResMaterialState State;              //!< ステートです.
    uint8_t          TextureCount;       //!< テクスチャ数です.
    uint32_t         BufferSize;         //!< 定数バッファサイズです.

    std::vector<std::string>        Textures;       //!< テクスチャです.
    std::vector<uint8_t>            Buffer;         //!< 定数バッファデータです.
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

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
#include <map>


namespace edit {

///////////////////////////////////////////////////////////////////////////////
// Material structure
///////////////////////////////////////////////////////////////////////////////
struct Material
{
    std::string     Name;               //!< マテリアル名.
    uint32_t        Kind;               //!< マテリアル種別.

    std::map<std::string, float>        Params;
    std::map<std::string, std::string>  Textures;
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

} // namespace edit

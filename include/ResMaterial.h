//-----------------------------------------------------------------------------
// File : ResMaterial.h
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
    uint32_t    MaterialHash;       //!< マテリアル名を表すハッシュ値です.
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

} // namespace asdx

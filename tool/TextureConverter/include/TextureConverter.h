//-----------------------------------------------------------------------------
// File : TextureConverter.h
// Desc : Texture Converter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

// MEMO : 入力サポート形式は次の通りです.
// * Direct Draw Surface (*.dds)
// * Truevision Graphics Adapter (*.tga)
// * Radiance HDR (*.hdr)
// * Window Bitmap (*.bmp)
// * Join Photographic Experts Group (*.jpg, *.jpeg)
// * Portable Network Graphic (*.png)
// * Tagged Image File Format (*.tif, *.tiff)
// * Graphics Interchanged Format (*.gif)
// * HD Photo (*.hdp)
// * Window Media Photo (*.wdp)
// * JPEG XR (*.jxr)
// * High Efficiency Image File (*.heif, *.heic)

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <string>
#include <vector>


namespace DirectX {
class ScratchImage;
} // namespace DirectX

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// TextureConverter class
///////////////////////////////////////////////////////////////////////////////
class TextureConverter
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    ///////////////////////////////////////////////////////////////////////////
    // Desc structure
    ///////////////////////////////////////////////////////////////////////////
    struct Desc
    {
        std::string     InputPath;      //!< 入力ファイルパス.
        std::string     OutputPath;     //!< 出力ファイルパス.
    };

    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      変換処理を行います.
    //! 
    //! @param[in]      desc        変換設定です.
    //! @retval true    変換に成功.
    //! @retval false   変換に失敗.
    //-------------------------------------------------------------------------
    static bool Convert(const Desc& desc);

    //-------------------------------------------------------------------------
    //! @brief      変換処理を行います.
    //! 
    //! @param[in]      input       入力テクスチャ.
    //! @param[out]     output      出力バイナリ.
    //! @retval true    変換に成功.
    //! @retval false   変換に失敗.
    //-------------------------------------------------------------------------
    static bool Convert(const DirectX::ScratchImage& input, std::vector<uint8_t>& output);

    //-------------------------------------------------------------------------
    //! @brief      逆変換処理を行います.
    //! 
    //! @param[in]      input       入力バイナリ.
    //! @param[out]     output      出力テクスチャ.
    //! @retval true    逆変換に成功.
    //! @retval fasle   逆変換に失敗.
    //-------------------------------------------------------------------------
    static bool ReverseConvert(const std::vector<uint8_t>& input, DirectX::ScratchImage& output);

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

} // namespace asdx

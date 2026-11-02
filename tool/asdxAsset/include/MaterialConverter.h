//-----------------------------------------------------------------------------
// File : MaterialConverter.h
// Desc : Material Binary (*.mtb) Converter.
// Copyright(c) Project Asura. 
//-----------------------------------------------------------------------------
#pragma once


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <string>
#include <vector>
#include <string_view>
#include "../../../include/fnd/asdxMath.h"


///////////////////////////////////////////////////////////////////////////////
// MaterialConverter class
///////////////////////////////////////////////////////////////////////////////
class MaterialConverter
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
        std::string InputPath;  //!< 入力ファイルパス.
        std::string OutputPath; //!< 出力ファイルパス.
    };

    ///////////////////////////////////////////////////////////////////////////
    // AlphaMode enum
    ///////////////////////////////////////////////////////////////////////////
    enum AlphaMode : uint8_t
    {
        Opaque, //!< 不透明.
        Mask,   //!< マスク.
        Blend,  //!< アルファブレンド
    };

    ///////////////////////////////////////////////////////////////////////////
    // CullMode enum
    ///////////////////////////////////////////////////////////////////////////
    enum CullMode : uint8_t
    {
        Back,   //!< 背面カリング.
        Front,  //!< 全面カリング.
        None,   //!< カリングなし.(両面描画) 
    };

    ///////////////////////////////////////////////////////////////////////////
    // MaterialParam structre
    ///////////////////////////////////////////////////////////////////////////
    struct MaterialParam
    {
        asdx::Float3    BaseColor   = asdx::Float3(1.0f, 1.0f, 1.0f);   //!< ベースカラー乗算値.
        float           Alpha       = 1.0f;                             //!< アルファ値.
        float           Occlusion   = 1.0f;                             //!< オクルージョン乗算値.
        float           Roughness   = 1.0f;                             //!< ラフネス乗算値.
        float           Metalness   = 1.0f;                             //!< メタルネス乗算値.
        float           Ior         = 1.0f;                             //!< 屈折率.
        asdx::Float3    Emissive    = asdx::Float3(0.0f, 0.0f, 0.0f);   //!< エミッシブ乗算値.
        float           AlphaCutOff = 0.0f;                             //!< アルファテスト閾値.
    };

    ///////////////////////////////////////////////////////////////////////////
    // Material structure
    ///////////////////////////////////////////////////////////////////////////
    struct Material
    {
        std::string                 Shader;                         //!< マテリアルピクセルシェーダ名.
        uint8_t                     AlphaMode = AlphaMode::Opaque;  //!< アルファモード.
        uint8_t                     CullMode  = CullMode::Back;     //!< カリングモード.
        MaterialParam               Param     = {};                 //!< マテリアルパラメータ.
        std::vector<std::string>    Textures;                       //!< テクスチャパス.
    };

    //=========================================================================
    // public variables.
    //=========================================================================
    static constexpr uint32_t kBaseColorMapIndex  = 0;
    static constexpr uint32_t kNormalMapIndex     = 1;
    static constexpr uint32_t kOrmMapIndex        = 2;
    static constexpr uint32_t kEmissiveMapIndex   = 3;

    static constexpr std::string_view kDefaultBaseColorMap = "default.BaseColor";
    static constexpr std::string_view kDefaultNormalMap    = "default.Normal";
    static constexpr std::string_view kDefaultOrmMap       = "default.Orm";
    static constexpr std::string_view kDefaultEmissiveMap  = "default.TransparentBlack";

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      現在のバイナリバージョンを取得します.
    //! 
    //! @return     現在のバイナリバージョンを返却します.
    //-------------------------------------------------------------------------
    static uint32_t GetCurrentVersion();

    //-------------------------------------------------------------------------
    //! @brief      変換処理を行います.
    //! 
    //! @param[in]      desc        変換設定.
    //! @retval true    変換に成功.
    //! @retval false   変換に失敗.
    //-------------------------------------------------------------------------
    static bool Convert(const Desc& desc);

    //-------------------------------------------------------------------------
    //! @brief      変換処理を行います.
    //! 
    //! @param[in]      desc        変換設定.
    //! @retval true    変換に成功.
    //! @retval false   変換に失敗.
    //-------------------------------------------------------------------------
    static bool Convert(const Material& material, std::vector<uint8_t>& binary);

    //-------------------------------------------------------------------------
    //! @brief      逆変換処理を行います.
    //! 
    //! @param[in]      binary      入力バイナリ.
    //! @param[in]      path        JSON出力ファイルパス.
    //! @retval true    変換に成功.
    //! @retval false   変化に失敗.
    //-------------------------------------------------------------------------
    static bool ReverseConvert(const std::string& path, const std::vector<uint8_t>& binary);

    //-------------------------------------------------------------------------
    //! @brief      逆変換処理を行います.
    //! 
    //! @param[in]      binary      入力バイナリ.
    //! @param[in]      path        出力ファイルパス.
    //! @retval true    変換に成功.
    //! @retval false   変化に失敗.
    //-------------------------------------------------------------------------
    static bool ReverseConvert(const std::vector<uint8_t>& binary, Material& result);

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

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
#include <fnd/asdxMath.h>
#include <fnd/asdxStringView.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// MATERIAL_FEATURE
///////////////////////////////////////////////////////////////////////////////
enum MATERIAL_FEATURE
{
    MATERIAL_NONE           = 0x0,
    MATERIAL_ANISOTROPY     = 0x1 << 0,
    MATERIAL_CLEARCOAT      = 0x1 << 1,
    MATERIAL_SHEEN          = 0x1 << 2,
    MATERIAL_TRANSMISSION   = 0x1 << 3,
    MATERIAL_IOR            = 0x1 << 4,
    MATERIAL_DISPERSION     = 0x1 << 5,
    MATERIAL_VOLUME         = 0x1 << 6,
    MATERIAL_IRIDESCENCE    = 0x1 << 7,
};

///////////////////////////////////////////////////////////////////////////////
// ResMaterial structure
///////////////////////////////////////////////////////////////////////////////
struct ResMaterial
{
    uint32_t    FeatureMask;                //!< 機能マスク.
    StringView  BindName;                   //!< バインド名.
    StringView  BaseColorMap;               //!< ベースカラーマップ.
    StringView  NormalMap;                  //!< 法線マップ.
    StringView  OrmMap;                     //!< オクルージョン・ラフネス・メタルネスマップ.
    StringView  EmissiveMap;                //!< エミッシブマップ.
    StringView  AnisotropyMap;              //!< 異方性マップ.
    StringView  ClearCoatMap;               //!< クリアコートマップ.
    StringView  ClearCoatRoughnessMap;      //!< クリアコートラフネスマップ.
    StringView  ClearCoatNormalMap;         //!< クリアコート法線マップ.
    StringView  SheenColorMap;              //!< 光沢カラーマップ.
    StringView  SheenRoughnessMap;          //!< 光沢ラフネスマップ.
    StringView  TransmissionMap;            //!< 透過マップ.
    StringView  ThicknessMap;               //!< 厚みマップ.
    StringView  IridescenceMap;             //!< 玉虫色マップ.
    StringView  IridescenceThicknessMap;    //!< 玉虫厚みマップ.

    Vector3     BaseColorFactor;                //!< ベースカラー因子.
    float       OcclusionFactor;                //!< オクルージョン因子.
    float       RoughnessFactor;                //!< ラフネス因子.
    float       MetalnessFactor;                //!< メタルネス因子.
    Vector3     EmissiveFactor;                 //!< エミッシブ因子.
    float       AlphaThreshold;                 //!< アルファテスト閾値.
    float       AnisotropyStrength;             //!< 異方性強度.
    Vector2     AnisotropyRotation;             //!< 異方性回転.
    float       ClearCoatFactor;                //!< クリアコート因子.
    float       ClearCoatRoughnessFactor;       //!< クリアコートラフネス因子.
    Vector3     SheenColorFactor;               //!< 光沢カラー因子.
    float       SheenRoughnessFactor;           //!< 光沢ラフネス因子.
    float       Ior;                            //!< 屈折率.
    float       Dispersion;                     //!< 分散値.
    float       TransimissionFactor;            //!< 透過因子.
    float       IridescenceFactor;              //!< 玉虫色因子.
    float       IridescenceIor;                 //!< 玉虫色屈折率
    float       IridescenceThicknessMinimum;    //!< 玉虫色最小厚み.
    float       IridescenceThicknessMaximum;    //!< 玉虫色最大厚み.
};

///////////////////////////////////////////////////////////////////////////////
// MaterialBinary class
///////////////////////////////////////////////////////////////////////////////
class MaterialBinary
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    MaterialBinary();

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~MaterialBinary();

    //-------------------------------------------------------------------------
    //! @brief      メモリからロードします.
    //! 
    //! @param[in]      blob        バイナリラージオブジェクト.
    //-------------------------------------------------------------------------
    void Load(std::vector<uint8_t>&& blob);

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      マテリアル数を取得します.
    //! 
    //! @return     マテリアル数を返却します.
    //-------------------------------------------------------------------------
    uint32_t GetMaterialCount() const;

    //-------------------------------------------------------------------------
    //! @brief      マテリアルを取得します.
    //! 
    //! @param[in]      index       マテリアル番号.
    //! @return     マテリアルを返却します.
    //-------------------------------------------------------------------------
    ResMaterial GetMaterial(uint32_t index) const;

    //-------------------------------------------------------------------------
    //! @brief      マテリアルを検索します.
    //! 
    //! @param[in]      name        検索するマテリアル名.
    //! @param[out]     material    マテリアルの格納先.
    //! @retval true    検索にヒット.
    //! @retval false   見つかりませんでした.
    //-------------------------------------------------------------------------
    bool FindMaterial(const char* name, ResMaterial& material) const;

    //-------------------------------------------------------------------------
    //! @brief      マテリアル番号を検索します.
    //! 
    //! @param[in]      name            検索するマテリアル名.
    //! @param[out]     materialId      マテリアル番号の格納先.
    //! @retval true    検索にヒット.
    //! @retval fasle   見つかりませんでした.
    //-------------------------------------------------------------------------
    bool FindMaterialId(const char* name, uint32_t& materialId) const;

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    std::vector<uint8_t>    m_Blob;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

} // namespace asdx

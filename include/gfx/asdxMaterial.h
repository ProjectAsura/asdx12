//-----------------------------------------------------------------------------
// File : asdxMaterial.h
// Desc : Material.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxRef.h>
#include <gfx/asdxBuffer.h>
#include <res/asdxResMaterial.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// MaterialParam structure
///////////////////////////////////////////////////////////////////////////////
struct MaterialParam
{
    uint32_t    FeatureMask;                    //!< 機能マスク.
    uint32_t    BaseColorMap;
    uint32_t    NormalMap;
    uint32_t    OrmMap;

    uint32_t    EmissiveMap;
    uint32_t    AnisotropyMap;
    uint32_t    ClearCoatMap;
    uint32_t    ClearCoatRoughnessMap;

    uint32_t    ClearCoatNormalMap;
    uint32_t    SheenColorMap;
    uint32_t    SheenRoughnessMap;
    uint32_t    TransmissionMap;

    uint32_t    TicknessMap;
    uint32_t    IrradianceMap;
    uint32_t    IrradianceThicknessMap;

    Vector3     BaseColorFactor;                //!< ベースカラー因子.

    float       AlphaThreshold;                 //!< アルファテスト閾値.
    float       OcclusionFactor;                //!< オクルージョン因子.
    float       RoughnessFactor;                //!< ラフネス因子.
    float       MetalnessFactor;                //!< メタルネス因子.

    float       Ior;                            //!< 屈折率.
    Vector3     EmissiveFactor;                 //!< エミッシブ因子.

    float       AnisotropyStrength;             //!< 異方性強度.
    Vector2     AnisotropyRotation;             //!< 異方性回転.
    float       ClearCoatFactor;                //!< クリアコート因子.

    float       ClearCoatRoughnessFactor;       //!< クリアコートラフネス因子.
    Vector3     SheenColorFactor;               //!< 光沢カラー因子.

    float       SheenRoughnessFactor;           //!< 光沢ラフネス因子.
    float       Dispersion;                     //!< 分散値.
    float       TransimissionFactor;            //!< 透過因子.
    float       IridescenceFactor;              //!< 玉虫色因子.

    float       IridescenceIor;                 //!< 玉虫色屈折率
    float       IridescenceThicknessMinimum;    //!< 玉虫色最小厚み.
    float       IridescenceThicknessMaximum;    //!< 玉虫色最大厚み.
};

///////////////////////////////////////////////////////////////////////////////
// IMaterial interface
///////////////////////////////////////////////////////////////////////////////
struct IMaterial
{
    virtual ~IMaterial() {}
    virtual bool Init(const ResMaterial& value) = 0;
    virtual void Term();
    virtual ID3D12Resource* GetResource() const = 0;
    virtual D3D12_GPU_VIRTUAL_ADDRESS GetGpuAddress() const = 0;
    virtual void SetName(LPCWSTR tag) = 0;
};

///////////////////////////////////////////////////////////////////////////////
// StaticMaterial class
///////////////////////////////////////////////////////////////////////////////
class StaticMaterial : public IMaterial
{
public:
    bool Init(const ResMaterial& value) override;
    void Term() override;

    ID3D12Resource* GetResource() const override;
    D3D12_GPU_VIRTUAL_ADDRESS GetGpuAddress() const override;
    void SetName(LPCWSTR tag) override;

private:
    ConstantBuffer  m_ConstantBuffer;
};

///////////////////////////////////////////////////////////////////////////////
// DynamicMaterial class
///////////////////////////////////////////////////////////////////////////////
class DynamicMaterial : public IMaterial
{
public:
    bool Init(const ResMaterial& value) override;
    void Term() override;

    ID3D12Resource* GetResource() const override;
    D3D12_GPU_VIRTUAL_ADDRESS GetGpuAddress() const override;
    void SetName(LPCWSTR tag) override;

    MaterialParam* GetParam() const;
    void Update();

private:
    DoubledConstantBuffer   m_ConstantBuffer;
};


} // namespace asdx

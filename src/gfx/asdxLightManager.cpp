//-----------------------------------------------------------------------------
// File : asdxLightManager.cpp
// Desc : Light Manager.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxLogger.h>
#include <gfx/asdxLightManager.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Light class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
Light::Light(LightType type, PhotometricUnit unit, float intensity)
: m_Type(type)
, m_Unit(unit)
, m_Intensity(intensity)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      ライトタイプを取得します.
//-----------------------------------------------------------------------------
LightType Light::GetType() const
{ return m_Type; }

//-----------------------------------------------------------------------------
//      測光単位を取得します.
//-----------------------------------------------------------------------------
PhotometricUnit Light::GetUnit() const
{ return m_Unit; }

//-----------------------------------------------------------------------------
//      カラーを設定します.
//-----------------------------------------------------------------------------
void Light::SetColor(const Unorm3& value)
{ m_Color = value; }

//-----------------------------------------------------------------------------
//      カラーを取得します.
//-----------------------------------------------------------------------------
const Unorm3& Light::GetColor() const
{ return m_Color; }

//-----------------------------------------------------------------------------
//      強度を設定します.
//-----------------------------------------------------------------------------
void Light::SetIntensity(float value)
{ m_Intensity = value; }

//-----------------------------------------------------------------------------
//      強度を取得します.
//-----------------------------------------------------------------------------
float Light::GetIntensity() const
{ return m_Intensity; }

//-----------------------------------------------------------------------------
//      照度を取得します.
//-----------------------------------------------------------------------------
float Light::GetIlluminance(float distance) const
{
    switch(m_Type)
    {
    case LightType::Point:
        return CandelaToLux(m_Intensity, distance, 1.0f);

    case LightType::Spot:
        return CandelaToLux(m_Intensity, distance, 1.0f);

    case LightType::Directional:
        return m_Intensity;

    case LightType::ImageBased:
        return m_Intensity;

    default:
        return 0.0f;
    }
}

//-----------------------------------------------------------------------------
//      シャドウキャストフラグを設定します.
//-----------------------------------------------------------------------------
void Light::SetCastShadow(bool value)
{ m_CastShadow = value; }

//-----------------------------------------------------------------------------
//      シャドウキャストフラグを取得します.
//-----------------------------------------------------------------------------
bool Light::IsCastShadow() const
{ return m_CastShadow; }


///////////////////////////////////////////////////////////////////////////////
// PointLight class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
PointLight::PointLight(float cd)
: Light(LightType::Point, PhotometricUnit::Candela, cd)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      強度をカンデラ単位で取得します.
//-----------------------------------------------------------------------------
float PointLight::GetCandela() const
{ return GetIntensity(); }

//-----------------------------------------------------------------------------
//      強度をルーメン単位で取得します.
//-----------------------------------------------------------------------------
float PointLight::GetLumen() const
{ return PointCandelaToLumen(GetIntensity()); }

//-----------------------------------------------------------------------------
//      指定距離において強度をルクス単位で取得します.
//-----------------------------------------------------------------------------
float PointLight::GetLuxAtDistance(float distance) const
{ return CandelaToLux(GetIntensity(), distance, 1.0f); }

//-----------------------------------------------------------------------------
//      強度をカンデラ単位で設定します.
//-----------------------------------------------------------------------------
void PointLight::SetCandela(float value)
{ SetIntensity(value); }

//-----------------------------------------------------------------------------
//      強度をルーメン単位で設定します.
//-----------------------------------------------------------------------------
void PointLight::SetLumen(float value)
{ SetIntensity(PointLumenToCandela(value)); }


///////////////////////////////////////////////////////////////////////////////
// SpotLight class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
SpotLight::SpotLight(float cd, float innerAngle, float outerAngle)
: Light(LightType::Spot, PhotometricUnit::Candela, cd)
, m_InnerAngle(innerAngle)
, m_OuterAngle(outerAngle)
{ ValidateCone(); }

//-----------------------------------------------------------------------------
//      強度をカンデラ単位で取得します.
//-----------------------------------------------------------------------------
float SpotLight::GetCandela() const
{ return GetIntensity(); }

//-----------------------------------------------------------------------------
//      強度をルーメン単位で取得します.
//-----------------------------------------------------------------------------
float SpotLight::GetLumen() const
{ return SpotCandelaToLumen(GetIntensity(), m_OuterAngle); }

//-----------------------------------------------------------------------------
//      指定距離において強度をルクス単位で取得します.
//-----------------------------------------------------------------------------
float SpotLight::GetLuxAtDistance(float distance) const
{ return CandelaToLux(GetIntensity(), distance, 1.0f); }

//-----------------------------------------------------------------------------
//      内角を取得します.
//-----------------------------------------------------------------------------
float SpotLight::GetInnerAngle() const
{ return m_InnerAngle; }

//-----------------------------------------------------------------------------
//      外角を取得します.
//-----------------------------------------------------------------------------
float SpotLight::GetOuterAngle() const
{ return m_OuterAngle; }

//-----------------------------------------------------------------------------
//      内角を設定します.
//-----------------------------------------------------------------------------
void SpotLight::SetInnerAngle(float rad)
{ m_InnerAngle = rad; }

//-----------------------------------------------------------------------------
//      外角を設定します.
//-----------------------------------------------------------------------------
void SpotLight::SetOuterAngle(float rad)
{ m_OuterAngle = rad; }

//-----------------------------------------------------------------------------
//      強度をカンデラ単位で設定します.
//-----------------------------------------------------------------------------
void SpotLight::SetCandela(float cd)
{ SetIntensity(cd); }

//-----------------------------------------------------------------------------
//      強度をルーメン単位で設定します.
//-----------------------------------------------------------------------------
void SpotLight::SetLumen(float lm)
{ SetIntensity(SpotLumenToCandela(lm, m_OuterAngle)); }

//-----------------------------------------------------------------------------
//      円錐角を有効範囲に収めます.
//-----------------------------------------------------------------------------
void SpotLight::ValidateCone()
{
    m_InnerAngle = Clamp(m_InnerAngle, 0.0f, F_PI);
    m_OuterAngle = Clamp(m_OuterAngle, 0.0f, F_PI);
    if (m_InnerAngle > m_OuterAngle)
    { m_InnerAngle = m_OuterAngle; }
}

///////////////////////////////////////////////////////////////////////////////
// DirectionalLight class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
DirectionalLight::DirectionalLight(float lx)
: Light(LightType::Directional, PhotometricUnit::Lux, lx)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      強度をルクス単位で取得します.
//-----------------------------------------------------------------------------
float DirectionalLight::GetLux() const
{ return GetIntensity(); }

//-----------------------------------------------------------------------------
//      強度をルクス単位で設定します.
//-----------------------------------------------------------------------------
void DirectionalLight::SetLux(float lx)
{ SetIntensity(lx); }

///////////////////////////////////////////////////////////////////////////////
// ImageBasedLight class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      引数付きコンストラクタです.
//-----------------------------------------------------------------------------
ImageBasedLight::ImageBasedLight(float lx)
: Light(LightType::ImageBased, PhotometricUnit::Lux, lx)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      強度をルクス単位で取得します.
//-----------------------------------------------------------------------------
float ImageBasedLight::GetLux() const
{ return GetIntensity(); }

//-----------------------------------------------------------------------------
//      強度をルクス単位で設定します.
//-----------------------------------------------------------------------------
void ImageBasedLight::SetLux(float lx)
{ SetIntensity(lx); }


///////////////////////////////////////////////////////////////////////////////
// LightManager class
///////////////////////////////////////////////////////////////////////////////
LightManager LightManager::s_Instance = {};

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
LightManager::LightManager()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
LightManager::~LightManager()
{
    // 解放漏れがある場合，下記でアサートが発生します.
    assert(m_PointLights.empty());
    assert(m_SpotLights .empty());
    assert(m_DirectionalLights.empty());
    assert(m_ImageBasedLights .empty());
}

//-----------------------------------------------------------------------------
//      シングルトンインスタンスを取得します.
//-----------------------------------------------------------------------------
LightManager& LightManager::Instance()
{ return s_Instance; }

//-----------------------------------------------------------------------------
//      ポイントライトを生成します.
//-----------------------------------------------------------------------------
PointLight* LightManager::CreatePointLight(float cd)
{
    ScopedLock<SpinLock> locker(m_PointLightLock);

    auto light = new(std::nothrow) PointLight(cd);
    if (light == nullptr)
    {
        ELOGA("Error : Out of Memory.");
        return nullptr;
    }

    m_PointLights.push_back(light);
    return light;
}

//-----------------------------------------------------------------------------
//      スポットライトを生成します.
//-----------------------------------------------------------------------------
SpotLight* LightManager::CreateSpotLight(float cd, float innerAngle, float outerAngle)
{
    ScopedLock<SpinLock> locker(m_SpotLightLock);

    auto light = new(std::nothrow) SpotLight(cd, innerAngle, outerAngle);
    if (light == nullptr)
    {
        ELOGA("Error : Out of Memory.");
        return nullptr;
    }

    m_SpotLights.push_back(light);
    return light;
}

//-----------------------------------------------------------------------------
//      ディレクショナルライトを生成します.
//-----------------------------------------------------------------------------
DirectionalLight* LightManager::CreateDirectionalLight(float lx)
{
    ScopedLock<SpinLock> locker(m_DirectionalLightLock);

    auto light = new (std::nothrow) DirectionalLight(lx);
    if (light == nullptr)
    {
        ELOGA("Error : Out of Memory.");
        return nullptr;
    }
    m_DirectionalLights.push_back(light);
    return light;
}

//-----------------------------------------------------------------------------
//      イメージベースドライトを生成します.
//-----------------------------------------------------------------------------
ImageBasedLight* LightManager::CreateImageBasedLight(float lx)
{
    ScopedLock<SpinLock> locker(m_ImageBasedLightLock);

    auto light = new (std::nothrow) ImageBasedLight(lx);
    if (light == nullptr)
    {
        ELOGA("Error : Out of Memory.");
        return nullptr;
    }
    m_ImageBasedLights.push_back(light);
    return light;
}

//-----------------------------------------------------------------------------
//      ポイントライトを破棄します.
//-----------------------------------------------------------------------------
void LightManager::DisposePointLight(PointLight*& light)
{
    if (light == nullptr)
        return;

    ScopedLock<SpinLock> locker(m_PointLightLock);

    m_PointLights.erase(light);
    PointLight* item = light;
    delete item;
    light = nullptr;
}

//-----------------------------------------------------------------------------
//      スポットライトを破棄します.
//-----------------------------------------------------------------------------
void LightManager::DisposeSpotLight(SpotLight*& light)
{
    if (light == nullptr)
        return;

    ScopedLock<SpinLock> locker(m_SpotLightLock);

    m_SpotLights.erase(light);
    SpotLight* item = light;
    delete item;
    light = nullptr;
}

//-----------------------------------------------------------------------------
//      ディレクショナルライトを破棄します.
//-----------------------------------------------------------------------------
void LightManager::DisposeDirectionalLight(DirectionalLight*& light)
{
    if (light == nullptr)
        return;

    ScopedLock<SpinLock> locker(m_DirectionalLightLock);

    m_DirectionalLights.erase(light);
    DirectionalLight* item = light;
    delete item;
    light = nullptr;
}

//-----------------------------------------------------------------------------
//      イメージベースドライトを破棄します.
//-----------------------------------------------------------------------------
void LightManager::DisposeImageBasedLight(ImageBasedLight*& light)
{
    if (light == nullptr)
        return;

    ScopedLock<SpinLock> locker(m_ImageBasedLightLock);

    m_ImageBasedLights.erase(light);
    ImageBasedLight* item = light;
    delete item;
    light = nullptr;
}


} // namespace asdx

//-----------------------------------------------------------------------------
// File : BRDF.hlsli
// Desc : BRDF.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#ifndef ASDX_BRDF_HLSLI
#define ASDX_BRDF_HLSLI

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Math.hlsli"


//-----------------------------------------------------------------------------
//      ���t�l�X����X�y�L�����[�w�������߂܂�.
//-----------------------------------------------------------------------------
float ToSpecularPower(float roughness)
{ return 2.0f / min(0.99999f, max(0.0002f, roughness * roughness)) - 2.0f; }

//-----------------------------------------------------------------------------
//      �X�y�L�����[�w�����烉�t�l�X�l�����߂܂�.
//-----------------------------------------------------------------------------
float ToRoughness(float specularPower)
{
    // Dimiatr Lazarov, "Physically-based lighting in Call of Duty: Black Ops",
    // SIGGRAPH 2011 Cources: Advances in Real-Time Rendering in 3D Graphics.
    return sqrt(2.0f / (specularPower + 2.0f));
}

//-----------------------------------------------------------------------------
//      �f�B�t���[�Y���˗������߂܂�.
//-----------------------------------------------------------------------------
float3 ToKd(float3 baseColor, float metallic)
{ return (1.0f - metallic) * baseColor; }

//-----------------------------------------------------------------------------
//      �X�y�L�����[���˗������߂܂�.
//-----------------------------------------------------------------------------
float3 ToKs(float3 baseColor, float metallic)
{ return lerp(0.03f, baseColor, metallic); }

//-----------------------------------------------------------------------------
//      ���������.
//-----------------------------------------------------------------------------
float3 ToKsDielectics(float3 baseColor, float metallic, float reflectance)
{ return lerp(0.16f * reflectance * reflectance, baseColor, metallic); }

//-----------------------------------------------------------------------------
//      ��������.
//-----------------------------------------------------------------------------
float3 ToKsConductors(float3 baseColor, float metallic)
{ return baseColor * metallic; }

//-----------------------------------------------------------------------------
//      90�x���˂ɂ�����t���l�����˗������߂܂�.
//-----------------------------------------------------------------------------
float CalcF90(in float3 f0)
{ return saturate(50.0f * dot(f0, 0.33f)); }

//-----------------------------------------------------------------------------
//      Schlick�ɂ��t���l�����˂̋ߎ��l�����߂�.
//-----------------------------------------------------------------------------
float3 F_Schlick(in float3 f0, in float f90, in float u)
{ return f0 + (f90 - f0) * Pow5(1.0f - u); }

//-----------------------------------------------------------------------------
//      Schlick�ɂ��t���l�����˂̋ߎ��l�����߂�.
//-----------------------------------------------------------------------------
float F_Schlick(in float f0, in float f90, in float u)
{ return f0 + (f90 - f0) * Pow5(1.0f - u); }

//-----------------------------------------------------------------------------
//      �t���l�������v�Z���܂�.
//-----------------------------------------------------------------------------
float3 F_Schlick(const float3 f0, float VoH)
{
    float f = Pow5(1.0f - VoH);
    return f + f0 * (1.0f - f);
}

//-----------------------------------------------------------------------------
//      �t���l�������v�Z���܂�.
//-----------------------------------------------------------------------------
float F_Schlick(const float f0, float VoH)
{
    float f = Pow5(1.0f - VoH);
    return f + f0 * (1.0f - f);
}

//-----------------------------------------------------------------------------
//      �f�B�t���[�Y�̎x�z�I�ȕ��������߂܂�.
//-----------------------------------------------------------------------------
float3 GetDiffuseDominantDir(float3 N, float3 V, float NoV, float roughness)
{
    float a = 10.2341f * roughness - 1.51174f;
    float b = -0.511705f * roughness + 0.755868f;
    float lerpFactor = saturate((NoV * a + b) * roughness);
    return lerp(N, V, lerpFactor);
}

//-----------------------------------------------------------------------------
//      �X�y�L�����[�̎x�z�I�ȕ��������߂܂�.
//-----------------------------------------------------------------------------
float3 GetSpecularDomiantDir(float3 N, float3 R, float roughness)
{
    float smoothness = saturate(1.0f - roughness);
    float lerpFactor = smoothness * (sqrt(smoothness) + roughness);
    return lerp(N, R, lerpFactor);
}

//-----------------------------------------------------------------------------
//      �X�y�L�����[AO���v�Z���܂�.
//-----------------------------------------------------------------------------
float CalcSpecularAO(float NoV, float ao, float roughness)
{ return saturate(Pow(max(NoV + ao, 0.0f), exp2(-16.0f * roughness - 1.0f)) - 1.0f + ao); }

//-----------------------------------------------------------------------------
//      �����X�y�L�����[AO���v�Z���܂�.
//-----------------------------------------------------------------------------
float CalcHorizonAO(float RoN)
{
    // CalcSpecularAO���p�t�H�[�}���X�I�ɍ���ꍇ�ɁC�ȈՂȋߎ��Ƃ��Ďg�p����.
    float horizon = min(1.0f + RoN, 1.0f);
    return horizon * horizon;
}

//-----------------------------------------------------------------------------
//      �}�C�N���V���h�E���v�Z���܂�.
//-----------------------------------------------------------------------------
float ApplyMicroShadow(float ao, float NoL, float shadow)
{
    // See, "The Technical Art of Uncharted 4"
    float aperture = 2.0 * ao * ao;
    float microShadow = saturate(NoL + aperture - 1.0);
    return shadow * microShadow;
}

//-----------------------------------------------------------------------------
//      Lambert Diffuse�����߂܂�.
//-----------------------------------------------------------------------------
float LambertDiffuse(float NoL)
{ return NoL / F_PI; }

//-----------------------------------------------------------------------------
//      Half-Lambert Diffuse�����߂܂�.
//-----------------------------------------------------------------------------
float HalfLambertDiffuse(float NoL)
{
    float  v = NoL * 0.5f + 0.5f;
    return (v * v) * (3.0f / (4.0f * F_PI));
}

//-----------------------------------------------------------------------------
//      Disney Diffuse�����߂܂�.
//-----------------------------------------------------------------------------
float DisneyDiffuse(float NdotV, float NdotL, float LdotH, float roughness)
{
    float  energyBias   = lerp(0.0f, 0.5f, roughness);
    float  energyFactor = lerp(1.0f, 1.0f / 1.51f, roughness);
    float  fd90 = energyBias + 2.0f * LdotH * LdotH * roughness;
    float  lightScatter = F_Schlick(1.0f, fd90, NdotL).r;
    float  viewScatter  = F_Schlick(1.0f, fd90, NdotV).r;

    return lightScatter * viewScatter * energyFactor;
}

//-----------------------------------------------------------------------------
//      Phong Specular�����߂܂�.
//-----------------------------------------------------------------------------
float PhongSpecular(float3 N, float3 V, float3 L, float shininess)
{
    float3 R = -V + (2.0f * dot(N, V) * N);
    return Pow(max(dot(L, R), 0.0f), shininess) * ((shininess + 2.0f) / (2.0 * F_PI));
}

//-----------------------------------------------------------------------------
//      GGX��D�������߂܂�.
//-----------------------------------------------------------------------------
float D_GGX(float NdotH, float m)
{
    float a2 = m * m;
    float f = (NdotH * a2 - NdotH) * NdotH + 1;
    float d = a2 / (f * f);
    return SaturateHalf(d);
}

//-----------------------------------------------------------------------------
//      Height Correlated Smith�ɂ��G�������߂܂�.
//-----------------------------------------------------------------------------
float G_SmithGGX(float NdotL, float NdotV, float alphaG)
{
#if 0
    float a2 = alphaG * alphaG;
    float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0f - a2) + a2);
    float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0f - a2) + a2);
    return 0.5f / (Lambda_GGXV + Lambda_GGXL);
#else
    // sqrt()���Ȃ��œK���o�[�W����.
    float a = alphaG;
    float GGXV = NdotL * (NdotV * (1.0f - a) + a);
    float GGXL = NdotV * (NdotL * (1.0f - a) + a);
    return SaturateHalf(0.5f / (GGXV + GGXL));
#endif
}

//-----------------------------------------------------------------------------
//      D�����v�Z���܂�.
//-----------------------------------------------------------------------------
float D_Ashikhmin(float linearRoughness, float NoH)
{
    // Ashkhmin 2007, "Distribution-based BRDFs".
    float a2 = linearRoughness * linearRoughness;
    float cos2h = NoH * NoH;
    float sin2h = max(1.0f - cos2h, 0.0078125); // 2^(-14/2), so sin2h^2 0 in fp16
    float sin4h = sin2h * sin2h;
    float cot2 = -cos2h / (a2 * sin2h);
    return 1.0f / (F_PI * (4.0f * a2 + 1.0f) * sin4h) * (4.0f * exp(cot2) + sin4h);
}

//-----------------------------------------------------------------------------
//      D�����v�Z���܂�.
//-----------------------------------------------------------------------------
float D_Charlie(float linearRoughness, float NoH)
{
    // Estevez and Kulla 2017, "Production Friendly Microfacet Sheen BRDF".
    float invAlpha = 1.0f / linearRoughness;
    float cos2h = NoH * NoH;
    float sin2h = max(1.0f - cos2h, 0.0078125f); // 2^(-14/2), so sin2h^2 0 in fp16
    return (2.0f + invAlpha) * Pow(sin2h, invAlpha * 0.5f) / (2.0f * F_PI);
}

//-----------------------------------------------------------------------------
//      V�����v�Z���܂�.
//-----------------------------------------------------------------------------
float V_Neubelt(float NoV, float NoL)
{
    // Neubelt and Pettineo 2013, "Crafting a Next-gen Material Pipeline for THe Order: 1886".
    return SaturateHalf(1.0f / (4.0f * (NoL + NoV - NoL * NoV)));
}

//-----------------------------------------------------------------------------
//      �z�p�f�B�t���[�Y����]�����܂�.
//-----------------------------------------------------------------------------
float3 EvaluateClothDiffuse(float3 diffuseColor, float sheen, float3 subsurfaceColor, float NoL)
{ 
    float diffuse = 1.0f / F_PI;
    diffuse *= saturate((NoL + 0.5f) / 2.25f);
    float3 result = diffuseColor * diffuse;
    result *= saturate(subsurfaceColor + NoL);
    return result;
}

//-----------------------------------------------------------------------------
//      �z�p�X�y�L�����[����]�����܂�.
//-----------------------------------------------------------------------------
float3 EvaluateClothSpecular
(
    float   sheen,
    float   clothness,
    float   NoH,
    float   NoL,
    float   NoV
)
{
    float  D = D_Charlie(clothness, NoH);
    float  V = V_Neubelt(NoV, NoL);
    float3 F = float3(sheen, sheen, sheen);
    return (D * V * F) * NoL;
}

//-----------------------------------------------------------------------------
//      �w�A�̃X�y�L�����[�������߂܂�.
//-----------------------------------------------------------------------------
float ScheuermannSingleSpecularTerm(float3 T, float3 H, float exponent)
{
    // Thorsten Scheuermann, "Hair Rendering and Shading", ShaderX 3, p.244�@�Q��.
    float ToH   = dot(T, H);
    float sinTH = sqrt(1.0f - ToH * ToH);
    return Pow(sinTH, exponent);
}

//-----------------------------------------------------------------------------
//      �w�A�̃X�y�L�����[���������߂܂�.
//-----------------------------------------------------------------------------
float ScheuermannSpecularAttenuation(float NoL)
{
    // Thorsten Scheuermann, "Hair Rendering and Shading", ShaderX 3. p.246 �Q��.
    return saturate(1.75f * NoL + 0.25f);
}

//-----------------------------------------------------------------------------
//      Kajiya-Key�f�B�t���[�Y����]�����܂�.
//-----------------------------------------------------------------------------
float EvaluateScheuermannDiffuse(float NoL)
{
    // Thorsten Scheuermann, "Hair Rendering and Shading", ShaderX 3, p.243 �Q��.
    return saturate(0.75f * NoL + 0.25f);
}

//-----------------------------------------------------------------------------
//      Scheuermann�X�y�L�����[����]�����܂�.
//-----------------------------------------------------------------------------
float3 EvaluateScheuermannSpecular
(
    float3  T,              // �ڐ��x�N�g��.
    float3  N,              // �@���x�N�g��.
    float3  H,              // �����x�N�g���ƃ��C�g�x�N�g���̃n�[�t�x�N�g��.
    float   NoL,            // �@���ƃ��C�g�x�N�g���̓���.
    float4  specularColor0, // RGB : �X�y�L�����[�J���[, A : ���ˋ��x.
    float4  specularColor1, // RGB : �X�y�L�����[�J���[, A : ���ˋ��x.
    float   specularShift0, // �V�t�g��.
    float   specularShift1, // �V�t�g��.
    float   shiftValue,     // �V�t�g�e�N�X�`���̒l.
    float   noise           // �m�C�Y�e�N�X�`���̒l.
)
{
    float3 T0 = ShiftTangent(T, N, specularShift0 + shiftValue);
    float3 T1 = ShiftTangent(T, N, specularShift1 + shiftValue);

    float3 specular0 = specularColor0.rgb * ScheuermannSingleSpecularTerm(T0, H, specularColor0.a);
    float3 specular1 = specularColor1.rgb * ScheuermannSingleSpecularTerm(T1, H, specularColor1.a);

    specular1 *= noise;

    return (specular0 + specular1) * ScheuermannSpecularAttenuation(NoL);
}

//-----------------------------------------------------------------------------
//      Kajiya-Kay BRDF��]�����܂�.
//-----------------------------------------------------------------------------
float3 EvaluateKajiyaKay
(
    float3  T,              // �ڐ��x�N�g��.
    float3  N,              // �@���x�N�g��.
    float3  V,              // �����x�N�g��.
    float3  L,              // ���C�g�x�N�g��.
    float3  Kd,             // �f�B�t���[�Y�J���[.
    float3  Ks,             // �X�y�L�����[�J���[.
    float   noise,          // �m�C�Y�e�N�X�`���̒l.
    float   primaryScale,   // �v���C�}���[�n�C���C�g���x.
    float   secondaryWidth  // �Z�J���_���[�n�C���C�g��.
)
{
    // James T. Kajiya, Timothy L. Kay, "RENDERING FUR WITH THREE DIMENSIONAL TEXTURES",
    // Computer Graphics, Volume 23, Number 3, July 1989,
    // Diffuse  �� Equation (14) �Q��.
    // Specular �� Equation (16) �Q��.

    float SpecularPower0  = 80.0f * primaryScale;
    float SpecularPower1  = max(0.04f, SpecularPower0 / secondaryWidth * 4.0f);
    float Normalize0      = (SpecularPower0 + 2.0f) / (2.0f * F_PI);
    float Normalize1      = (SpecularPower1 + 2.0f) / (2.0f * F_PI);

    float cosTL = dot(T, L);
    float sinTL = ToSin(cosTL);

    float diffuse = max(sinTL, 0.0f);
    float alpha   = radians(noise * 10.0f); // �`���g�p(5 - 10 �x)

    float cosTRL = -cosTL;
    float sinTRL =  sinTL;
    float cosTV  = dot(T, V);
    float sinTV  = ToSin(cosTV);

    // �v���C�}���[�J���[�����߂�.
    float cosTRL0   = cosTRL * cos(2.0f * alpha) - sinTRL * sin(2.0f * alpha);
    float sinTRL0   = ToSin(cosTRL0);
    float specular0 = max(0, cosTRL0 * cosTV + sinTRL0 * sinTV);

    // �Z�J���_���[�J���[�����߂�.
    float cosTRL1   = cosTRL * cos(-3.0f * alpha) - sinTRL * sin(-3.0f * alpha);
    float sinTRL1   = ToSin(cosTRL1);
    float specular1 = max(0, cosTRL1 * cosTV + sinTRL1 * sinTV);

    // �X�y�L�����[�l.
    float power0 = Pow(specular0, SpecularPower0) * Normalize0;
    float power1 = Pow(specular1, SpecularPower1) * Normalize1;

    // �����_�����O�������̗]����.
    float NoL = saturate(dot(N, L));

    // BRDF��]��.
    float3 fd = Kd * diffuse / F_PI;
    float3 fs = SaturateHalf(Ks * (power0 + power1) * 0.5f);  // 2�������Ă���̂�2�Ŋ���(=0.5���|����).

    return (fd + fs) * NoL;
}

//-----------------------------------------------------------------------------
//      V�����v�Z���܂�.
//-----------------------------------------------------------------------------
float V_Kelemen(float LoH)
{ return 0.25f / (LoH * LoH); }

//-----------------------------------------------------------------------------
//      �N���A�R�[�g�̃��t�l�X�����߂܂�.
//-----------------------------------------------------------------------------
float GetClearCoatRoughness(float clearCoatRoughness)
{ return lerp(0.089f, 0.6f, clearCoatRoughness); }

//-----------------------------------------------------------------------------
//      �N���A�R�[�g�̃t���l���������߂܂�.
//-----------------------------------------------------------------------------
float GetClearCoatFresnel(float LoH, float clearCoat)
{ return F_Schlick(0.04f, 1.0f, LoH) * clearCoat; }

//-----------------------------------------------------------------------------
//      �ٕ���GGX��D�������߂܂�.
//-----------------------------------------------------------------------------
float D_GGXAnisotropic
(
    float  at,
    float  ab,
    float  NoH,
    float3 H,
    float3 T,
    float3 B
)
{
    //float  at  = max(linearRoughness * (1.0f + anisotropy), 0.001f);
    //float  ab  = max(linearRoughness * (1.0f - anisotropy), 0.001f);
    float  ToH = dot(T, H);
    float  BoH = dot(B, H);
    float  a2  = at * ab;
    float3 v   = float3(ab * ToH, at * BoH, a2 * NoH);
    float  v2  = dot(v, v);
    float  w2  = a2 / v2;

    return a2 * w2 * w2;
}

//-----------------------------------------------------------------------------
//      �ٕ���GGX��V�������߂܂�.
//-----------------------------------------------------------------------------
float V_SmithGGXHeightCorrelatedAnisotropic
(
    float at,
    float ab,
    float3 V,
    float3 L,
    float3 T,
    float3 B,
    float NoV,
    float NoL
)
{
    //float  at  = max(linearRoughness * (1.0f + anisotropy), 0.001f);
    //float  ab  = max(linearRoughness * (1.0f - anisotropy), 0.001f);
    float ToV = dot(T, V);
    float BoV = dot(B, V);
    float ToL = dot(T, L);
    float BoL = dot(B, L);

    float lambdaV = NoL * length(float3(at * ToV, ab * BoV, NoV));
    float lambdaL = NoV * length(float3(at * ToL, ab * BoL, NoL));
    float v = 0.5f / (lambdaV + lambdaL);
    return SaturateHalf(v);
}

//-----------------------------------------------------------------------------
//      �v�Z�ɂ��ዅ�p�̍��������߂܂�.
//-----------------------------------------------------------------------------
float ProcedualHeightForEye(float radius, float anteriorChamberDepth)
{
    // Jorge Jimenez, Javier von der Pahlen,
    // "Next-Generation Character Rendering", GDC 2013
    // Eye Rendering �Z�N�V�����Q��.
    //const float anteriorChamberDepth = 3.23f; // 3.23[nm] from [Lackner 2005]
    return anteriorChamberDepth * saturate(1.0f - 18.4f * radius * radius);
}

//-----------------------------------------------------------------------------
//      ���܃x�N�g�����v�Z���܂�.
//-----------------------------------------------------------------------------
float3 CalcRefraction(float n1, float n2, float3 N, float3 V)
{
    float NdotV = dot(N, V);
    float n = (NdotV > 0.0f) ? (n1 / n2) : (n2 / n1); 

    // "Real-time Rendering Third Edition", 9.5 Refraction, p.396
    // ��(9.31), ��(9.32)�Q��.
    // ���΋��ܗ� n = (�}��1�̋��ܗ� n1 / �}��2�̋��ܗ� n2).
    float w = n * NdotV;
    float k = sqrt(1.0f + (w - n) * (w + n));
    return (w - k) * N - n * V;
}

//-----------------------------------------------------------------------------
//      �����}�b�s���O�ɂ����܌�̃e�N�X�`�����W���v�Z���܂�.
//-----------------------------------------------------------------------------
float2 ParallaxRefraction
(
    float2      texcoord,               // �e�N�X�`�����W.
    float       height,                 // ����.
    float       parallaxScale,          // �����X�P�[��.
    float3      viewW,                  // ���[���h��Ԃ̎����x�N�g��.
    float3x3    world                   // ���[���h�s��.
)
{
    // Jorge Jimenez, Javier von der Pahlen,
    // "Next-Generation Character Rendering", GDC 2013
    // Eye Rendering �Z�N�V�����Q��.
    float2 viewL = mul(viewW, (float3x2)world);
    float2 offset = height * viewL;
    offset.y = -offset.y;
    return texcoord - parallaxScale * offset;
}

//-----------------------------------------------------------------------------
//      �����x�[�X�ɂ����܌�̃e�N�X�`�����W���v�Z���܂�.
//-----------------------------------------------------------------------------
float2 PhysicallyBasedRefraction
(
    float2      texcoord,           // �e�N�X�`�����W.
    float       height,             // ����.
    float       mask,               // �Ԗ����狭���ւ̕�Ԓl.
    float       n1,                 // �}��1�̋��ܗ�.
    float       n2,                 // �}��2�̋��ܗ�.
    float3      normalW,            // �@���x�N�g��.
    float3      viewW,              // ���[���h��Ԃł̎����x�N�g��.
    float3      frontNormalW,       // �ዅ�̎����x�N�g��
    float3x3    world               // ���[���h�s��
)
{
    // ���܃x�N�g�������߂�.
    float3 refractedW = CalcRefraction(n1, n2, normalW, viewW);

    // Jorge Jimenez, Javier von der Pahlen,
    // "Next-Generation Character Rendering", GDC 2013
    // Eye Rendering �Z�N�V�����Q��.
    float  cosAlpha = dot(frontNormalW, -refractedW);
    float  dist     = height / cosAlpha;
    float3 offsetW  = dist * refractedW;

    // ���[�J���ɕϊ�
    float2 offsetL = mul(offsetW, world).xy;

    return texcoord + float2(mask, -mask) * offsetL;
}

//-----------------------------------------------------------------------------
//      ���K���X��]�����܂�.
//-----------------------------------------------------------------------------
void EvaluateThinGlass
(
    in  float   eta,                // ���ܗ�.
    in  float   NoV,                // �@���Ǝ����x�N�g���̓���.
    in  float3  baseColor,          // �x�[�X�J���[.
    out float3  transmittance,      // �g�����X�~�b�^���X.
    out float3  reflectance,        // ���t���N�^���X.
    out float3  absorptionRatio     // �z����.
)
{
    float sinTheta2 = 1.0f - NoV * NoV;

    const float sinRefractedTheta2 = sinTheta2 / (eta * eta);
    const float cosRefractedTheta  = sqrt(1 - sinRefractedTheta2);

    const float q0 = mad(eta, cosRefractedTheta, -NoV);
    const float q1 = mad(eta, cosRefractedTheta,  NoV);
    const float q2 = mad(eta, NoV, -cosRefractedTheta);
    const float q3 = mad(eta, NoV,  cosRefractedTheta);

    const float r0 = q0 / q1;
    const float r1 = q2 / q3;

    // ���˖ʂɂ�����t���l�����t���N�^���X.
    const float R0 = 0.5 * saturate(r0 * r0 + r1 * r1);
    // ���˖ʂɂ�����t���l���g�����X�~�b�^���X.
    const float T0 = 1 - R0;

    const float3 R = float3(R0, R0, R0);
    const float3 T = float3(T0, T0, T0);
    const float3 C = float3(cosRefractedTheta, cosRefractedTheta, cosRefractedTheta);

    // �z�����l�����邽�߂̌W��.
    const float3 K = Pow(max(baseColor, 0.001), 1 / C);
    const float3 RK = R * K;

    transmittance   = saturate(T * T * K / (1 - RK * RK));
    reflectance     = saturate(RK  * transmittance + R);
    absorptionRatio = saturate(-(1 + RK) * transmittance + T);

    // reflectance���o�̓J���[�ɏ�Z.
    // transmittance�͍�������w�i�F�����L����p���ĕύX.
    // backGround = lerp(1.0, transmittance, alpha);
    // ������g���ăA���t�@�u�����f�B���O����.
}

//-----------------------------------------------------------------------------
//      �N���A�R�[�g�̒��ڌ���]�����܂�.
//-----------------------------------------------------------------------------
float3 EvaluateDirectLightClearCoat
(
    float3  N,
    float3  L,
    float3  V,
    float3  Kd,
    float3  Ks,
    float   roughness,
    float   clearCoatStrength,
    float   clearCoatRoughness
)
{
    float NoV = abs(dot(N, V));
    float3 H  = normalize(V + L);
    float LoH = saturate(dot(L, H));
    float NoL = saturate(dot(N, L));
    float NoH = saturate(dot(N, H));
    float HoV = saturate(dot(H, V));
    float a2  = max(roughness * roughness, 0.01f);
    float f90 = saturate(50.0f * dot(Ks, 0.33f));

    float3 Fd = (Kd / F_PI);
    float  D = D_GGX(NoH, a2);
    float  G = G_SmithGGX(NoL, NoV, a2);
    float3 F = F_Schlick(Ks, f90, LoH);
    float3 Fr = (D * G * F) / F_PI;

    float coatingPerceptualRoughness = GetClearCoatRoughness(clearCoatRoughness);
    float coatingRoughness = coatingPerceptualRoughness * coatingPerceptualRoughness;

    float  Dc  = D_GGX(NoH, coatingRoughness);
    float  Gc  = V_Kelemen(LoH);
    float  Fc  = GetClearCoatFresnel(LoH, clearCoatStrength);
    float  Frc = (Dc * Gc * Fc) / F_PI; 
    float  t   = max(1.0f - Fc, 0.0f);

    return ((Fd + Fr * (1.0f - Fc)) * (1.0f - Fc) + Frc) * NoL;
}

//-----------------------------------------------------------------------------
//      Lambert BRDF�̌`��ɂ��ƂÂ��T���v�����O���s���܂�.
//-----------------------------------------------------------------------------
float3 SampleLambert(float2 u)
{
    // PDF = NoL / F_PI
    return UniformSampleHemisphere(u);
}

//-----------------------------------------------------------------------------
//      Phong BRDF�̌`��ɂ��ƂÂ��T���v�����O���s���܂�.
//-----------------------------------------------------------------------------
float3 SamplePhong(float2 u, float shininess)
{
    float phi = 2.0f * F_PI * u.x;
    float cosTheta = pow(1.0f - u.y, 1.0f / (shininess + 1.0f));
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

    // PDF = normalizationTerm * pow(cosTheta, shininess)
    //     = ((1.0f + shininess) / (2.0f * F_PI)) * pow(cosTheta, shininess)
    return float3(
        sinTheta * cos(phi),
        sinTheta * sin(phi),
        cosTheta);
}

//-----------------------------------------------------------------------------
//      GGX BRDF�̌`��ɂ��ƂÂ��T���v�����O���s���܂�.
//-----------------------------------------------------------------------------
float3 SampleGGX(float2 u, float a)
{
    // a = linearRoughness * linearRoughness �Ƃ��܂�.
    float phi = 2.0 * F_PI * u.x;
    float cosTheta = sqrt( (1.0 - u.y) / (u.y * (a * a - 1.0) + 1.0) );
    float sinTheta = sqrt( 1.0 - cosTheta * cosTheta );

    // PDF = D * NdotH / (4.0f * LdotH);
    return float3(
        sinTheta * cos(phi),
        sinTheta * sin(phi),
        cosTheta);
}

//-----------------------------------------------------------------------------
//      ���@�����z���l������GGX BRDF�̌`��ɂ��ƂÂ��T���v�����O���s���܂�.
//-----------------------------------------------------------------------------
float3 SampleVndfGGX(float2 u, float2 a, float3 wi)
{
    // a = linearRoughness * linearRoughness �Ƃ��܂�.
    float3 V = normalize(wi.xy * a, wi.z);

    // [Dupuy 2023] Jonathan Dupuy, Anis Benyoub,
    // "Sampling Visible GGX Normals with Spherical Caps",
    // High-Performance Graphics 2023.
    float phi = 2.0f * F_PI * u.x;
    float z = mad((1.0f - u.y), (1.0f + V.z), -V.z);
    float sinTheta = sqrt(saturate(1.0f - z * z));
    float x = sinTheta * cos(phi);
    float y = sinTheta * cos(phi);
    float3 H = float3(x, y, z) + V;

    H = normalize(float3(H.xy * alpha, H.z));

    // PDF = D_vis * Jacobian
    //     = (G1(wi) * max(0, dot(wi, H)) * D(H) / wi.z) * (1.0f / (4.0f * LdotH));
    return H;
}

#endif//ADX_BRDF_HLSLI

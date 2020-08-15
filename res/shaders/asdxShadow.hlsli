//-----------------------------------------------------------------------------
// File : asdxShadow.hlsli
// Desc : Shadowing Utility.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#ifndef ASDX_SHADOW_HLSLI
#define ASDX_SHADOW_HLSLI


#define ASDX_SHADOW_EPSILON         (1.192092896e-07F)  // �C�v�V����.
#define ASDX_SHADOW_MANTISSA_BITS   (23)                // ������.

typedef SamplerComparisonState ShadowSampler;

#ifdef ASDX_COMPUTE_SHADER
//-----------------------------------------------------------------------------
//      �V���h�E�}�b�v���T���v�����܂�(�R���s���[�g�V�F�[�_�p).
//-----------------------------------------------------------------------------
float4 SampleShadow
(
    Texture2DArray  shadowMap,      // �J�X�P�[�h�V���h�E�}�b�v.
    ShadowSampler   shadowSmp,      // �V���h�E�}�b�v�T���v���[.
    float2          coord,          // �V���h�E�}�b�v���W.
    uint            casecadeIndex,  // �J�X�P�[�h�ԍ�.
    float           depth           // �V���h�E�}�b�v�Ɣ�r�������[�x�l.
)
{
    return shadowMap.GatherCmpRed(
        shadowSmp, float3(coord, cascadeIndex), depth);
}

#else
//-----------------------------------------------------------------------------
//      �V���h�E�}�b�v���T���v�����܂�(�s�N�Z���V�F�[�_�p).
//-----------------------------------------------------------------------------
float4 SampleShadow
(
    Texture2DArray  shadowMap,      // �J�X�P�[�h�V���h�E�}�b�v.
    ShadowSampler   shadowSmp,      // �V���h�E�}�b�v�T���v���[.
    float2          coord,          // �V���h�E�}�b�v���W.
    uint            cascadeIndex,   // �J�X�P�[�h�ԍ�.
    float           depth           // �V���h�E�}�b�v�Ɣ�r�������[�x�l.
)
{
    return shadowMap.SampleCmpLevelZero(
        shadowSmp, float3(coord, cascadeIndex), depth);
}

#endif//ASDX_COMPUTE_SHADER

//-----------------------------------------------------------------------------
//      UNORM�t�H�[�}�b�g�ł̒萔�o�C�A�X���v�Z���܂�.
//-----------------------------------------------------------------------------
float ConstantBiasUnorm(float bias)
{ return bias * ASDX_SHADOW_EPSILON; }

//-----------------------------------------------------------------------------
//      FLOAT�t�H�[�}�b�g�ł̒萔�o�C�A�X���v�Z���܂�.
//-----------------------------------------------------------------------------
float ConstantBiasFloat(float z, float bias)
{ return bias * pow(2.0f, (exp(z) - ASDX_SHADOW_MANTISSA_BITS)); }

//-----------------------------------------------------------------------------
//      �X�΃o�C�A�X���v�Z���܂�.
//------------------------------------------------------------------------------
float3 SlopeBias(float3 N, float3 L, float scale)
{ return L * saturate(1.0f - dot(N, L)) * scale; }

//-----------------------------------------------------------------------------
//      �@���I�t�Z�b�g�ɂ��[�x�o�C�A�X
//-----------------------------------------------------------------------------
float3 NormalOffsetBias(float3 pos, float3 N, float3 L, float offset_scale)
{
    float slope_scale = saturate(1.0 - dot(N, L));
    return pos + N * offset_scale * slope_scale;
}

//-----------------------------------------------------------------------------
//      PCF 1tap.
//-----------------------------------------------------------------------------
float SamplePCF1
(
    Texture2DArray  shadowMap,      // �J�X�P�[�h�V���h�E�}�b�v.
    ShadowSampler   shadowSmp,      // �V���h�E�T���v���[.
    float3          coord,          // (�V���h�E�}�b�v�s�� * �e�N�X�`���s��)�ŕϊ������ʒu���W.
    uint            cascadeIndex    // �J�X�P�[�h�ԍ�.
)
{ return SampleShadow(shadowMap, shadowSmp, coord.xy, cascadeIndex, saturate(coord.z)); }

//-----------------------------------------------------------------------------
//      PCF 4tap.
//-----------------------------------------------------------------------------
float SamplePCF4
(
    Texture2DArray  shadowMap,      // �J�X�P�[�h�V���h�E�}�b�v.
    ShadowSampler   shadowSmp,      // �V���h�E�T���v���[.
    float3          coord,          // (�V���h�E�}�b�v�s�� * �e�N�X�`���s��)�ŕϊ������ʒu���W.
    uint            cascadeIndex,   // �J�X�P�[�h�ԍ�.
    float2          shadowMapSize   // �V���h�E�}�b�v�T�C�Y.
)
{
    float2 invMapSize = 1.0f / shadowMapSize;
    float2 uv = coord.xy * shadowMapSize + float2(0.5f, 0.5f);
    float2 baseUV = floor(uv);
    float2 st = uv.xy - baseUV.xy;

    baseUV -= float2(0.5f, 0.5f);
    baseUV *= invMapSize;

    float2 w0 = 3.0f - 2.0f * st;
    float2 w1 = 1.0f + 2.0f * st;

    float2 uv0 = (2.0f - st) / w0 - 1.0f;
    float2 uv1 = st / w1 + 1.0f;
    uv0 *= invMapSize;
    uv1 *= invMapSize;

    float depth  = saturate(coord.z);
    float result = 0.0f;
    result += (w0.x * w0.y) * SampleShadow(shadowMap, shadowSmp, baseUV + float2(uv0.x, uv0.y), cascadeIndex, depth).x;
    result += (w1.x * w0.y) * SampleShadow(shadowMap, shadowSmp, baseUV + float2(uv1.x, uv0.y), cascadeIndex, depth).x;
    result += (w0.x * w1.y) * SampleShadow(shadowMap, shadowSmp, baseUV + float2(uv0.x, uv1.y), cascadeIndex, depth).x;
    result += (w1.x * w1.y) * SampleShadow(shadowMap, shadowSmp, baseUV + float2(uv1.x, uv1.y), cascadeIndex, depth).x;

    return saturate(result * 0.0625); // 0.0625 = 1/16.
}

//-----------------------------------------------------------------------------
//      �`�F�r�V�F�t�̕s����.
//-----------------------------------------------------------------------------
float Chebyshev(float2 moments, float mean, float minVariance)
{
    // ���U���v�Z���܂�.
    float variance = moments.y - (moments.x * moments.x);
    variance = max(variance, minVariance);

    // ����m�����v�Z���܂�.
    float d = mean - moments.x;
    float prob = variance / (variance + (d * d));

    // �Б��`�F�r�V�F�t�s����.
    return (mean <= moments.x) ? 1.0f : prob;
}

//-----------------------------------------------------------------------------
//      �v���V�[�W�����O���b�h��`�悵�܂�.
//-----------------------------------------------------------------------------
float3 DrawGrid
(
    float3 baseColor,         // ��{�F.
    float2 shadowCoord,       // �V���h�E�}�b�v�e�N�X�`�����W.
    float  gridLineWidth,     // �O���b�h���̑���(�s�N�Z���P��).
    float3 gridColor,         // �O���b�h���̐F.
    float  spacing            // �O���b�h�Ԋu(�s�N�Z���P��).
)
{
    float2 st = shadowCoord / spacing;
    float2 dx = float2(ddx_fine(st.x), ddy_fine(st.x));
    float2 dy = float2(ddx_fine(st.y), ddy_fine(st.y));
    float2 m  = frac(st);

    if ( m.x < gridLineWidth * length(dx) 
      || m.y < gridLineWidth * length(dy))
    { return gridColor; }
    else
    { return baseColor; }
}

//-----------------------------------------------------------------------------
//      �e�N�X�`�����g���Č덷���������܂�.
//-----------------------------------------------------------------------------
float3 VisualizeError
(
    Texture2D       errorMap,   // �덷��\���F���i�[���ꂽ�e�N�X�`��.
    SamplerState    errorSmp,   // �N�����v�T���v���[.
    float2          texcoord,   // �V���h�E�}�b�v�e�N�X�`�����W.
    float2          mapSize     // �V���h�E�}�b�v�T�C�Y.
)
{
    float2 ds = mapSize.x * ddx_fine(texcoord);
    float2 dt = mapSize.y * ddy_fine(texcoord);
    float s = 0.1f; // [0, 10.0]�̒l��[0, 1]�Ƀ}�b�s���O����̂�0.1�{.
    float error = max(length(ds + dt), length(ds - dt)) * s;
    return errorMap.Sample(errorSmp, error).rgb;
}

#endif // ASDX_SHADOW_HLSLI

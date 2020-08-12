//-----------------------------------------------------------------------------
// File : Math.hlsli
// Desc : Math Utility.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#ifndef ASDX_SHADOW_HLSLI
#define ASDX_SHADOW_HLSLI


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
    Texture2D       errorMap,      // �덷��\���F���i�[���ꂽ�e�N�X�`��.
    SamplerState    errorSmp,      // �N�����v�T���v���[.
    float2          texcoord,      // �V���h�E�}�b�v�e�N�X�`�����W.
    float2          map_size       // �V���h�E�}�b�v�T�C�Y.
)
{
    float2 ds = map_size.x * ddx_fine(texcoord);
    float2 dt = map_size.y * ddy_fine(texcoord);
    float s = 0.1f; // [0, 10.0]�̒l��[0, 1]�Ƀ}�b�s���O����̂�0.1�{.
    float error = max(length(ds + dt), length(ds - dt)) * s;
    return errorMap.Sample(errorSmp, error).rgb;
}


#endif

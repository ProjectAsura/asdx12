//-----------------------------------------------------------------------------
// File : SkyBoxPS.hlsl
// Desc : Pixel Shader For SkyBox.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float3 TexCoord : TEXCOORD;
};

//-----------------------------------------------------------------------------
// Textures and Samplers.
//-----------------------------------------------------------------------------
TextureCube  CubeMap : register(t0);
SamplerState CubeSmp : register(s0);

//-----------------------------------------------------------------------------
//      ���C���G���g���[�|�C���g�ł�.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET
{
    float3 color = CubeMap.SampleLevel(CubeSmp, input.TexCoord, 0).rgb;
    return float4(color, 0.0f);
}
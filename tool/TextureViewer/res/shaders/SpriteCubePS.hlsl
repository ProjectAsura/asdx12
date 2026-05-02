//-----------------------------------------------------------------------------
// File : SpriteCubePS.hlsl
// Desc : Pixel Shader For Sprite Draw.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color    : COLOR0;
};

///////////////////////////////////////////////////////////////////////////////
// CbParam structure
///////////////////////////////////////////////////////////////////////////////
cbuffer CbParam : register(b1)
{
    float MipLevel;
    float ArrayIndex;
    int   FaceIndex;
    int   Reserved;
};

//-----------------------------------------------------------------------------
// Textures and Samplers.
//-----------------------------------------------------------------------------
TextureCube  ColorMap : register(t0);
SamplerState ColorSmp : register(s0);

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET
{
    return ColorMap.SampleLevel(ColorSmp, float3(input.TexCoord, FaceIndex), MipLevel);
}

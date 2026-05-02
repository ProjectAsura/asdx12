//-----------------------------------------------------------------------------
// File : Sprite2dPS.hlsl
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
    int3  Reserved;
};

//-----------------------------------------------------------------------------
// Textures and Samplers.
//-----------------------------------------------------------------------------
Texture2D    ColorMap : register(t0);
SamplerState ColorSmp : register(s0);

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET
{
    return ColorMap.SampleLevel(ColorSmp, input.TexCoord, MipLevel);
}

//-----------------------------------------------------------------------------
// File : TestPS.hlsl
// Desc : Pixel Shader For Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

//-----------------------------------------------------------------------------
// Textures and Samplers.
//-----------------------------------------------------------------------------
Texture2D       ColorMap    : register(t0);
SamplerState    LinearClamp : register(s0);

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET0
{
    //return ColorMap.Sample(LinearClamp, input.TexCoord);
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}

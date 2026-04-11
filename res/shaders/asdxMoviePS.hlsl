//-----------------------------------------------------------------------------
// File : asdxMoviePS.hlsl
// Desc : Pixel Shader For Movie Draw.
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

//-----------------------------------------------------------------------------
// Textures and Samplers.
//-----------------------------------------------------------------------------
Texture2D    ColorMapY  : register(t0);
Texture2D    ColorMapUV : register(t1);
SamplerState ColorSmp   : register(s0);

//-----------------------------------------------------------------------------
//      SRGBからリニアへの変換.
//-----------------------------------------------------------------------------
float3 SRGB_To_Linear(float3 color)
{
    float3 result;
    result.x = (color.x < 0.0405f) ? color.x / 12.92f : pow((abs(color.x) + 0.055) / 1.055f, 2.4f);
    result.y = (color.y < 0.0405f) ? color.y / 12.92f : pow((abs(color.y) + 0.055) / 1.055f, 2.4f);
    result.z = (color.z < 0.0405f) ? color.z / 12.92f : pow((abs(color.z) + 0.055) / 1.055f, 2.4f);

    return result;
}

//-----------------------------------------------------------------------------
//      YPbPr から RGB に変換します.
//-----------------------------------------------------------------------------
float3 YPbPrToRGB(float3 color)
{
    // 正規化.
    float Y  = (color.x * 255.0f - 16.0f) / 219.0f;
    float Pb = (color.y * 255.0f - 128.0f) / 224.0f;
    float Pr = (color.z * 255.0f - 128.0f) / 224.0f;

    // BT.709
    return float3(
        Y + 1.5748f * Pr,
        Y - 0.1873f * Pb - 0.4681f * Pr,
        Y + 1.8556f * Pb);
}

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput iput) : SV_TARGET
{
    float  y  = ColorMapY .Sample(ColorSmp, iput.TexCoord).r;
    float2 uv = ColorMapUV.Sample(ColorSmp, iput.TexCoord).rg;
 
    float4 result = 0.0f.xxxx;
    result.rgb = YPbPrToRGB(float3(y, uv));
#if 0
    result.rgb = SRGB_To_Linear(result.rgb);
#endif
    result.a = 1.0f;

    return result;
}

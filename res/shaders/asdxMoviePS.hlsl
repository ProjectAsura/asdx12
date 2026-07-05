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
//      YCbCr から RGB に変換します.
//-----------------------------------------------------------------------------
float3 YCbCrToRGB(float3 color)
{
    // 正規化.
    float Y  = (color.x * 255.0f -  16.0f) / 219.0f;
    float Cb = (color.y * 255.0f - 128.0f) / 224.0f;
    float Cr = (color.z * 255.0f - 128.0f) / 224.0f;

    // BT.709
    return float3(
        Y + 1.5748f * Cr,
        Y - 0.1873f * Cb - 0.4681f * Cr,
        Y + 1.8556f * Cb);
}

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput iput) : SV_TARGET
{
    // YUV 形式から RGB 形式に変換して出力.
    float  y  = ColorMapY .SampleLevel(ColorSmp, iput.TexCoord, 0.0f).r;
    float2 uv = ColorMapUV.SampleLevel(ColorSmp, iput.TexCoord, 0.0f).rg;
    return float4(YCbCrToRGB(float3(y, uv)), 1.0f);
}

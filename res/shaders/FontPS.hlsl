//-----------------------------------------------------------------------------
// File : FontPS.hlsl
// Desc : Signed Distance Field Font Drawer.
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
// Resources.
//-----------------------------------------------------------------------------
Texture2D    SdfFontTexture : register(t0);
SamplerState LinearClamp    : register(s0);

//-----------------------------------------------------------------------------
//      中間値を取得します.
//-----------------------------------------------------------------------------
float Median(float a, float b, float c)
{ return max(min(a, b), min(max(a, b), c)); }

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET
{
    float4 outsideColor = 0.0f.xxxx;
    float4 insideColor  = input.Color;

    float3 s = SdfFontTexture.Sample(LinearClamp, input.TexCoord).rgb;
    float  d = Median(s.r, s.g, s.b) - 0.5f;
    float  w = saturate(d / fwidth(d) + 0.5f);
    return lerp(outsideColor, insideColor, w);
}

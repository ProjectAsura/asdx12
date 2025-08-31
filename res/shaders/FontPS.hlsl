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

///////////////////////////////////////////////////////////////////////////////
// CbParam constant buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbParam : register(b1)
{
    float   PixelRange;
    float2  InvTextureSize;
    uint    Reserved;
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
//      スクリーン空間でのピクセル範囲を求めます.
//-----------------------------------------------------------------------------
float ScreenPixelRange(float2 texcoord)
{
    float2 unitRange = PixelRange.xx * InvTextureSize;
    float2 screenTexSize = 1.0f.xx / fwidth(texcoord);
    return max(0.5f * dot(unitRange, screenTexSize), 1.0f);
}

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET
{
    // MSDFを取得.
    float3 msd = SdfFontTexture.Sample(LinearClamp, input.TexCoord).rgb;

    // Signed Distance を求める.
    float sd = Median(msd.r, msd.g, msd.b);

    // スクリーン上での距離を求める.
    float screenPixelDistance = ScreenPixelRange(input.TexCoord) * (sd - 0.5f);
 
    // 不透明度を求める.
    float opacity = saturate(screenPixelDistance + 0.5f);
 
    // 色を出力.
    return float4(input.Color.rgb, input.Color.a * opacity);
}

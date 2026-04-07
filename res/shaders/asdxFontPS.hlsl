//-----------------------------------------------------------------------------
// File : asdxFontPS.hlsl
// Desc : Signed Distance Field Font Drawer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

#ifndef SINGLE_CHANNEL
#define SINGLE_CHANNEL  (1)
#endif//SINGLE_CHANNEL

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

///////////////////////////////////////////////////////////////////////////////
// CbParam constant buffers.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbParam : register(b1)
{
    uint    Flags;
    float2  Offset;
    uint    OuterColor;
};

//-----------------------------------------------------------------------------
//      カラー値を取得します.
//-----------------------------------------------------------------------------
float4 ToColor(uint param)
{
    float r = ((param >> 24) & 0xff) / 255.0f;
    float g = ((param >> 16) & 0xff) / 255.0f;
    float b = ((param >>  8) & 0xff) / 255.0f;
    float a = (param & 0xff) / 255.0f;
    return float4(r, g, b, a);
}

//-----------------------------------------------------------------------------
//      アウター描画が有効かチェックします.
//-----------------------------------------------------------------------------
bool EnableOuter()
{ return !!(Flags & 0x1); }

//-----------------------------------------------------------------------------
//      アウターオフセットが有効かチェックします.
//-----------------------------------------------------------------------------
bool EnableOffset()
{ return !!((Flags >> 1) & 0x1); }

//-----------------------------------------------------------------------------
//      中間値を取得します.
//-----------------------------------------------------------------------------
float Median(float3 value)
{
    return max(
        min(value.r, value.g),
        min(value.b, max(value.r, value.g)));
}

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET
{
    const float kThreshold       = 0.5f;     // 変更禁止.
    const float kBlurRadius      = 0.1f;     // 適宜調整.
    const float kOuterThreshold  = 0.4f;
    const float kOuterBlurRadius = 0.1f;

    // 符号付き距離を取得.
#if SINGLE_CHANNEL
    float dist = SdfFontTexture.Sample(LinearClamp, input.TexCoord).r;
#else
    float dist = Median(SdfFontTexture.Sample(LinearClamp, input.TexCoord).rgb);
#endif

    float4 color = input.Color;
    float  alpha = saturate(smoothstep(
        kThreshold - kBlurRadius,
        kThreshold + kBlurRadius,
        dist));

    color *= alpha;

    // アウター描画.
    if (EnableOuter())
    {
        float dist2 = dist;

        // オフセットが有効な場合は，ずらした位置でテクスチャサンプル.
        if(EnableOffset())
        {
            float2 invSize;
            SdfFontTexture.GetDimensions(invSize.x, invSize.y);
            invSize.x = 1.0f / invSize.x;
            invSize.y = 1.0f / invSize.y;
        #if SINGLE_CHANNEL
            dist2 = Median(SdfFontTexture.Sample(LinearClamp, input.TexCoord + Offset * invSize).rgb);
        #else
            dist2 = SdfFontTexture.Sample(LinearClamp, input.TexCoord + Offset * invSize).r;
        #endif
        }

        float4 outer = ToColor(OuterColor);
        outer *= saturate(smoothstep(
            kOuterThreshold - kOuterBlurRadius,
            kOuterThreshold + kOuterBlurRadius,
            dist2));

        color = lerp(outer, color, alpha);
    }

    // カラーを出力.
    return color;
}

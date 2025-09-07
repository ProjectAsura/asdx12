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
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET
{
    const float kThreshold  = 0.5f;     // 変更禁止.
    const float kBlurRadius = 0.1f;     // 適宜調整.

    // 符号付き距離を取得.
    float dist = SdfFontTexture.Sample(LinearClamp, input.TexCoord).r;

    // アンチエリアシング.
    float alpha = smoothstep(
        kThreshold - kBlurRadius,
        kThreshold + kBlurRadius,
        dist);

    // カラーを出力.
    return input.Color * saturate(alpha);
}

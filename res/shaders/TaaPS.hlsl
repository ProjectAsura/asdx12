//-----------------------------------------------------------------------------
// File : TemporalAA_PS.hlsl
// Desc : Temporal Anti-Alias.
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

///////////////////////////////////////////////////////////////////////////////
// PSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct PSOutput
{
    float4 Color   : SV_TARGET0;
};

///////////////////////////////////////////////////////////////////////////////
// CbTemporalAA constant buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbTemporalAA : register(b0)
{
    float   Gamma;          // 1.0でうまく動くとのこと. [0.75, 1.25]で設定するのがGoodとのこと.
    float   BlendFactor;
    float2  HistoryMapSize;
};

//-----------------------------------------------------------------------------
// Textures and Samplers.
//-----------------------------------------------------------------------------
Texture2D       ColorMap    : register(t0);
Texture2D       HistoryMap  : register(t1);
Texture2D       VelocityMap : register(t2);
SamplerState    ColorSmp    : register(s0);
SamplerState    HistorySmp  : register(s1);
SamplerState    VelocitySmp : register(s2);

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static const int2 Offsets[8] = {
    int2(-1,- 1), int2(-1,  1),
    int2( 1, -1), int2( 1,  1),
    int2( 1,  0), int2( 0, -1),
    int2( 0,  1), int2(-1,  0)
};


//-----------------------------------------------------------------------------
//      RGBからYCoCgに変換します.
//-----------------------------------------------------------------------------
float3 RGBToYCoCg( float3 RGB )
{
    float Y  = dot(RGB, float3(  1, 2,  1 )) * 0.25;
    float Co = dot(RGB, float3(  2, 0, -2 )) * 0.25 + ( 0.5 * 256.0/255.0 );
    float Cg = dot(RGB, float3( -1, 2, -1 )) * 0.25 + ( 0.5 * 256.0/255.0 );
    return float3(Y, Co, Cg);
}

//-----------------------------------------------------------------------------
//      YCoCgからRGBに変換します.
//-----------------------------------------------------------------------------
float3 YCoCgToRGB( float3 YCoCg )
{
    float Y  = YCoCg.x;
    float Co = YCoCg.y - ( 0.5 * 256.0 / 255.0 );
    float Cg = YCoCg.z - ( 0.5 * 256.0 / 255.0 );
    float R  = Y + Co-Cg;
    float G  = Y + Cg;
    float B  = Y - Co-Cg;
    return float3(R,G,B);
}

//-----------------------------------------------------------------------------
//      Catmull-Rom フィルタリング.
//-----------------------------------------------------------------------------
float3 BicubicSampleCatmullRom(Texture2D map, SamplerState smp, float2 uv, float2 mapSize)
{
    float2 samplePos = uv * mapSize;
    float2 tc = floor(samplePos - 0.5f) + 0.5f;
    float2 f  = samplePos - tc;
    float2 f2 = f * f;
    float2 f3 = f * f2;

    float2 w0 = f2 - 0.5f * (f3 + f);
    float2 w1 = 1.5f * f3 - 2.5f * f2 + 1;
    float2 w3 = 0.5f * (f3 - f2);
    float2 w2 = 1 - w0 - w1 - w3;

    float2 w12 = w1 + w2;

    float2 invMapSize = 1.0f / mapSize;
    float2 tc0  = (tc - 1) * invMapSize;
    float2 tc12 = (tc + w2 / w12) * invMapSize;
    float2 tc3  = (tc + 2) * invMapSize;

    float3 result = float3(0.0f, 0.0f, 0.0f);

    result += map.SampleLevel(smp, float2(tc0.x,  tc0.y),  0).rgb * (w0.x  * w0.y);
    result += map.SampleLevel(smp, float2(tc0.x,  tc12.y), 0).rgb * (w0.x  * w12.y);
    result += map.SampleLevel(smp, float2(tc0.x,  tc3.y),  0).rgb * (w0.x  * w3.y);

    result += map.SampleLevel(smp, float2(tc12.x, tc0.y),  0).rgb * (w12.x * w0.y);
    result += map.SampleLevel(smp, float2(tc12.x, tc12.y), 0).rgb * (w12.x * w12.y);
    result += map.SampleLevel(smp, float2(tc12.x, tc3.y),  0).rgb * (w12.x * w3.y);

    result += map.SampleLevel(smp, float2(tc3.x,  tc0.y),  0).rgb * (w3.x * w0.y);
    result += map.SampleLevel(smp, float2(tc3.x,  tc12.y), 0).rgb * (w3.x * w12.y);
    result += map.SampleLevel(smp, float2(tc3.x,  tc3.y),  0).rgb * (w3.x * w3.y);

    return result;
}

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
PSOutput main(const VSOutput input)
{
    PSOutput output = (PSOutput)0;
    
    // Marco Salvi's Variance clipping algorithm.
    // https://www.dropbox.com/sh/dmye840y307lbpx/AAAQpC0MxMbuOsjm6XmTPgFJa?dl=0

    float2 currentUV = input.TexCoord;
    float3 current = RGBToYCoCg(ColorMap  .SampleLevel(ColorSmp,   currentUV, 0).rgb);

    float3 colorAve = current;
    float3 colorVar = current * current;

    int i;

    [unroll]
    for(i=0; i<8; ++i)
    {
        float3 fetch = RGBToYCoCg(ColorMap.SampleLevel(ColorSmp, currentUV, 0.0f, Offsets[i]).rgb);
        colorAve += fetch;
        colorVar += fetch * fetch;
    }
    const float oneOverNine = 1.0f / 9.0f;
    colorAve *= oneOverNine;
    colorVar *= oneOverNine;

    float3 sigma = sqrt(max(0, colorVar - colorAve * colorAve));
    float3 colorMin = colorAve - Gamma * sigma;
    float3 colorMax = colorAve + Gamma * sigma;

    // 最も長い速度ベクトルを見つける.
    float2 velocity = VelocityMap.SampleLevel(VelocitySmp, input.TexCoord, 0).xy;
    [unroll]
    for(i=0; i<8; ++i)
    {
        const float2 v = VelocityMap.SampleLevel(VelocitySmp, input.TexCoord, 0, Offsets[i]).xy;
        velocity = (dot(v, v) > dot(velocity, velocity)) ? v : velocity;
    }

    float2 historyUV = saturate(input.TexCoord + velocity);
    float3 history = RGBToYCoCg(BicubicSampleCatmullRom(HistoryMap, HistorySmp, historyUV, HistoryMapSize));

    // Box Clipping.
    history = clamp(history, colorMin, colorMax);

    // ブレンド率.
    // TODO : 深度やマスクに応じて棄却処理を実装する.
    float alpha = BlendFactor;

    output.Color = float4(YCoCgToRGB(lerp(current, history, alpha)), 1.0f);

    // TAA自体の実行速度を上げるため，ここではHistoryMapの更新は行いません.
    // HistoryMapの更新はこのシェーダを実行した後に，CopyResource()等で行ってください.
    return output;
}
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
    float4 History : SV_TARGET1;
};

///////////////////////////////////////////////////////////////////////////////
// CbTemporalAA constant buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbTemporalAA : register(b0)
{
    float   Gamma;
    float   BlendFactor;
};

//-----------------------------------------------------------------------------
// Textures and Samplers.
//-----------------------------------------------------------------------------
Texture2D       ColorMap   : register(t0);
Texture2D       HistoryMap : register(t1);
SamplerState    ColorSmp   : register(s0);
SamplerState    HistorySmp : register(s1);

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static const float2 Offsets[8] = {
    float2(-1,- 1), float2(-1,  1), 
    float2( 1, -1), float2( 1,  1), 
    float2( 1,  0), float2( 0, -1), 
    float2( 0,  1), float2(-1,  0) 
};


//-----------------------------------------------------------------------------
//      RGBからYCoCgに変換します.
//-----------------------------------------------------------------------------
float3 RGBToYCoCg( float3 RGB )
{
    float Y = dot(RGB, float3(  1, 2,  1 )) * 0.25;
    float Co= dot(RGB, float3(  2, 0, -2 )) * 0.25 + ( 0.5 * 256.0/255.0 );
    float Cg= dot(RGB, float3( -1, 2, -1 )) * 0.25 + ( 0.5 * 256.0/255.0 );
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
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
PSOutput main(const VSOutput input)
{
    PSOutput output = (PSOutput)0;
    
    // Simplest version of Marco Salvi's Variance clipping algorithm.
    // https://www.dropbox.com/sh/dmye840y307lbpx/AAAQpC0MxMbuOsjm6XmTPgFJa?dl=0
    
    float3 curr    = RGBToYCoCg(ColorMap  .SampleLevel(ColorSmp,   input.TexCoord, 0).rgb);
    float3 history = RGBToYCoCg(HistoryMap.SampleLevel(HistorySmp, input.TexCoord, 0).rgb);

    float3 colorAve = curr;
    float3 colorVar = curr * curr;

    float2 invSize;
    ColorMap.GetDimensions(invSize.x, invSize.y);
    invSize.x = 1.0f / invSize.x;
    invSize.y = 1.0f / invSize.y;

    [unroll]
    for(int i=0; i<8; ++i)
    {
        float3 fetch = RGBToYCoCg(ColorMap.SampleLevel(ColorSmp, input.TexCoord + Offsets[i] * invSize, 0.0f).rgb);
        colorAve += fetch;
        colorVar += fetch * fetch;
    }
    colorAve /= 9.0f;
    colorVar /= 9.0f;

    float3 sigma = sqrt(max(0, colorVar - colorAve * colorAve));
    float3 colorMin = colorAve - Gamma * sigma;
    float3 colorMax = colorAve + Gamma * sigma;

    history = clamp(history, colorMin, colorMax);
    output.Color   = float4(YCoCgToRGB(lerp(curr, history, BlendFactor)), 1.0f);
    output.History = output.Color;

    return output;
}
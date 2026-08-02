//-----------------------------------------------------------------------------
// File : asdxFxaaPS.hlsl
// Desc : Fast Approximate Anti-Aliasing Pixel Shader.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

// FXAAの設定.
#define FXAA_PC                 (1)     // PC版アルゴリズムを使用.
#define FXAA_HLSL_5             (1)     // 使用するHLSLのバージョン.
#define FXAA_QUALITY__PRESET    (13)    // 品質設定.

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "../../external/FXAA/FXAA3_11.hlsli"
#include "asdxSamplers.hlsli"


///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
};

///////////////////////////////////////////////////////////////////////////////
// Param constant buffers.
///////////////////////////////////////////////////////////////////////////////
cbuffer Param : register(b0)
{
    float2 RcpFrame;
    float2 Reserved;
};

//-----------------------------------------------------------------------------
// Resources
//-----------------------------------------------------------------------------
Texture2D ColorMap : register(t0);


//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET
{
    FxaaTex srcTex = { LinearClamp, ColorMap };

    return FxaaPixelShader(
        input.TexCoord,     // pos.
        0.0f.xxxx,          // fxaaConsolePosPos (PCなので未使用).
        srcTex,             // tex.
        srcTex,             // fxaaConsole360TexExpBiasNegOne (PCなので未使用).
        srcTex,             // fxaaConsole360TexExpBiasNegTwo (PCなので未使用).
        RcpFrame,           // fxaaQualityRcpFrame.
        0.0f.xxxx,          // fxaaConsolercpFrameOpt       (PCなので未使用).
        0.0f.xxxx,          // fxaaConsoleRcpFrameOpt2      (PCなので未使用).
        0.0f.xxxx,          // fxaaConsole360RcpFrameOpt2   (PCなので未使用).
        0.75f,              // fxaaQualitySubpix.
        0.125f,             // fxaaQualityEdgeThreshold.
        0.0625f,            // fxaaQualityEdgeThresholdMin.
        0.0f,               // fxaaConsoleEdgeSharpness.    (PCなので未使用).
        0.0f,               // fxaaConsoleEdgeThreshold.    (PCなので未使用).
        0.0f,               // fxaaConsoleEdgeThresholdMin. (PCなので未使用).
        0.0f.xxxx           // fxaaConsole360ConstDir.      (PCなので未使用).
    );
}

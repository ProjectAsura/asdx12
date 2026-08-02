//-----------------------------------------------------------------------------
// File : asdxFxaaCS.hlsl
// Desc : Fast Approximate Anti-Aliasing.
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
#include "asdxComputeUtil.hlsli"
#include "asdxSamplers.hlsli"

//-----------------------------------------------------------------------------
// Resources
//-----------------------------------------------------------------------------
Texture2D<float4>   Input  : register(t0);
RWTexture2D<float4> Output : register(u0);


///////////////////////////////////////////////////////////////////////////////
// Param constant buffers.
///////////////////////////////////////////////////////////////////////////////
cbuffer Param : register(b0)
{
    float2 RcpFrame;
    float2 TargetSize;
};

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void main
(
    uint3 disptachId : SV_DispatchThreadID,
    uint  groupIndex : SV_GroupIndex
)
{
    uint2 remappedId = RemapLane8x8(disptachId.xy, groupIndex);
    if (any(remappedId >= TargetSize))
    { return; }

    FxaaTex srcTex = { LinearClamp, Input };

    float2 uv = (remappedId + 0.5f.xx) * RcpFrame;
    float4 result = FxaaPixelShader(
        uv,                 // pos.
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

    Output[remappedId] = result;
}

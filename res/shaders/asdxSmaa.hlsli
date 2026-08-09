//-----------------------------------------------------------------------------
// File : asdxSmaa.hlsli
// Desc : Common Header For SMAA.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#ifndef ASDX_SMAA_HLSLI
#define ASDX_SMAA_HLSLI

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxComputeUtil.hlsli"
#include "asdxSamplers.hlsli"

cbuffer Param : register(b0)
{
    float4  Size;   // (1/w, 1/h, w, h)
};

// 設定関係の定義.
#define SMAA_RT_METRICS     Size
#define SMAA_PRESET_MEDIUM  (1)
#define SMAA_CUSTOM_SL      (1)

// 命令系の定義.
#define SMAATexture2D(tex)                              Texture2D tex
#define SMAATexturePass2D(tex)                          tex
#define SMAASampleLevelZero(tex, coord)                 tex.SampleLevel(LinearClamp, coord, 0)
#define SMAASampleLevelZeroPoint(tex, coord)            tex.SampleLevel(PointClamp , coord, 0)
#define SMAASampleLevelZeroOffset(tex, coord, offset)   tex.SampleLevel(LinearClamp, coord, 0, offset)
#define SMAASample(tex, coord)                          tex.SampleLevel(LinearClamp, coord, 0)
#define SMAASamplePoint(tex, coord)                     tex.SampleLevel(PointClamp , coord, 0)
#define SMAASampleOffset(tex, coord, offset)            tex.SampleLevel(LinearClamp, coord, 0, offset)
#define SMAA_FLATTEN                                    [flatten]
#define SMAA_BRANCH                                     [branch]
#define SMAATexture2DMS2(tex)                           Texture2DMS<float4, 2> tex
#define SMAALoad(tex, pos, sample)                      tex.Load(pos, sample)
#define SMAAGather(tex, coord)                          tex.Gather(LinearClamp, coord, 0)

// 実装をインクルード.
#include "../../external/SMAA/SMAA.hlsl"

#endif//ASDX_SMAA_HLSLI

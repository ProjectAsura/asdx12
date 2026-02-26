//-----------------------------------------------------------------------------
// File : asdxSkyBoxPS.hlsl
// Desc : Pixel Shader For Sky Box.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxMath.hlsli"
#include "asdxComputeUtil.hlsli"
#include "asdxSamplers.hlsli"
#include "asdxTextureUtil.hlsli"


///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;  //!< 位置座標.
    float2 TexCoord : TEXCOORD0;    //!< テクスチャ座標.
};

///////////////////////////////////////////////////////////////////////////////
// PSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct PSOutput
{
    float4 Color : SV_TARGET0;  //!< RTV0.
};

///////////////////////////////////////////////////////////////////////////////
// Param structure
///////////////////////////////////////////////////////////////////////////////
cbuffer Param : register(b0)
{
    float4x4 View;  //!< ビュー行列.
    float4x4 Proj;  //!< 射影行列.
};

//-----------------------------------------------------------------------------
// Resources.
//-----------------------------------------------------------------------------
TextureCube CubeMap : register(t0);


//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
PSOutput main(const VSOutput input)
{
    PSOutput output = (PSOutput)0;

    // カメラからスクリーンへのレイを求める.
    float2 st = input.TexCoord;
    st.y = 1.0f - st.y;
    float3 rayDir = CalcRayDir(st, View, Proj);

    // キューブマップをフェッチ.
    float4 color = CubeMap.SampleLevel(LinearClamp, rayDir, 0.0f);

    // 結果を書き込む.
    output.Color = color;

    return output;
}

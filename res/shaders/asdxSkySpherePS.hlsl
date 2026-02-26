//-----------------------------------------------------------------------------
// File : asdxSkySpherePS.hlsl
// Desc : Pixel Shader for Sky Sphere.
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
Texture2D SphereMap : register(t0);


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

    // スフィアマップのテクスチャ座標に変換.
    float2 uv = ToSphereMapCoord(rayDir);
 
    // スフィアマップをフェッチ.
    float4 color = SphereMap.SampleLevel(LinearClamp, uv, 0.0f);

    // 結果を書き込む.
    output.Color = color;

    return output;
}

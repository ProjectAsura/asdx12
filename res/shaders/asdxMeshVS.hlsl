//-----------------------------------------------------------------------------
// File : asdxMeshVS.hlsl
// Desc : Vertex Shader For Static Model Drawing.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxTransform.hlsli"


///////////////////////////////////////////////////////////////////////////////
// VSInput structure
///////////////////////////////////////////////////////////////////////////////
struct VSInput
{
    float3  Position    : POSITION;
    float3  Normal      : NORMAL;
    float4  Tangent     : TANGENT;
    float2  TexCoord    : TEXCOORD0;
    float4  Color       : COLOR0;
};

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4  Position     : SV_POSITION;
    float3  Normal       : NORMAL;
    float4  Tangent      : TANGENT;
    float2  TexCoord     : TEXCOORD0;
    float4  Color        : COLOR0;
    float4  CurrWorldPos : CURR_WORLD_POS;
    float4  PrevWorldPos : PREV_WORLD_POS;
};

///////////////////////////////////////////////////////////////////////////////
// CameraData structure
///////////////////////////////////////////////////////////////////////////////
struct CameraData
{
    float4x4 View;
    float4x4 Proj;
    float3   CameraPos;
    float    FieldOfView;
    float    NearClip;
    float    FarClip;
    uint2    JitterPos;
};

///////////////////////////////////////////////////////////////////////////////
// SceneParam constant buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer SceneParam : register(b0)
{
    CameraData Current;
    CameraData Previous;
};

//-----------------------------------------------------------------------------
// Resources.
//-----------------------------------------------------------------------------
StructuredBuffer<float4x3> CurrWorldMatrices : register(t0);
StructuredBuffer<float4x3> PrevWorldMatrices : register(t1);


//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
VSOutput main(const VSInput input, uint instanceId : SV_InstanceID)
{
    VSOutput output = (VSOutput) 0;

    float4x3 currWorld = CurrWorldMatrices[instanceId];

    float4 worldPos = float4(Transform(currWorld, input.Position), 1.0f);
    float4 viewPos  = Transform(Current.View, worldPos);
    float4 projPos  = Transform(Current.Proj, viewPos);
    
    float3 worldNormal  = normalize(TransformNormal(currWorld, input.Normal));
    float3 worldTangent = normalize(TransformNormal(currWorld, input.Tangent.xyz));
 
    // 従接線をチェック.
    float sign = input.Tangent.w;
    float3 B = cross(worldNormal, worldTangent) * input.Tangent.w;

    // 長さがゼロになる場合は、フォールバック.
    if (dot(B, B) < 1e-6f)
    {
        float3 axis = abs(worldNormal.z) < 0.999f ? float3(0, 0, 1) : float3(0, 1, 0);
        worldTangent = normalize(cross(axis, worldNormal));
        sign = 1.0f;
    }
 
    // 前フレームのワールド位置.
    float4x3 prevWorld    = PrevWorldMatrices[instanceId];
    float4   prevWorldPos = float4(Transform(prevWorld, input.Position), 1.0f);

    // 出力値設定.
    output.Position     = projPos;
    output.Normal       = worldNormal;
    output.Tangent      = float4(worldTangent, sign);
    output.TexCoord     = input.TexCoord;
    output.Color        = input.Color;
    output.CurrWorldPos = worldPos;
    output.PrevWorldPos = prevWorldPos;

    return output;
}

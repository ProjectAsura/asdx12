//-----------------------------------------------------------------------------
// File : asdxModelVS.hlsl
// Desc : Vertex Shader For Skinning Model Drawing.
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
    uint4   BoneIndices : BONEINDEX;
    float4  BoneWeights : BONEWEIGHT;
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
StructuredBuffer<float4x3> CurrMatrixPallets : register(t2);
StructuredBuffer<float4x3> PrevMatrixPallets : register(t3);


//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
VSOutput main(const VSInput input, uint instanceId : SV_InstanceID)
{
    VSOutput output = (VSOutput) 0;

    float3 skinnedPos     = (float3)0;
    float3 skinnedNormal  = (float3)0;
    float3 skinnedTangent = (float3)0;

    // スキニング処理.
    {
        for (int i=0; i<4; ++i)
        {
            skinnedPos     += Transform      (CurrMatrixPallets[input.BoneIndices[i]], input.Position) * input.BoneWeights[i];
            skinnedNormal  += TransformNormal(CurrMatrixPallets[input.BoneIndices[i]], input.Normal  ) * input.BoneWeights[i];
            skinnedTangent += TransformNormal(CurrMatrixPallets[input.BoneIndices[i]], input.Tangent ) * input.BoneWeights[i];
        }
    }

    float4x3 world = CurrWorldMatrices[instanceId];

    float4 worldPos = float4(Transform(world, skinnedPos), 1.0f);
    float4 viewPos  = Transform(Current.View, worldPos);
    float4 projPos  = Transform(Current.Proj, viewPos);

    float3 worldNormal  = normalize(TransformNormal(world, skinnedNormal));
    float3 worldTangent = normalize(TransformNormal(world, skinnedTangent));

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
 
    // スキニング処理.
    float3 prevSkinnedPos = (float3)0;
    {
        for (int i=0; i<4; ++i)
        {
            prevSkinnedPos += Transform(PrevMatrixPallets[input.BoneIndices[i]], input.Position) * input.BoneWeights[i];
        }
    }
    float4x3 prevWorld    = PrevWorldMatrices[instanceId];
    float4   prevWorldPos = float4(Transform(prevWorld, prevSkinnedPos), 1.0f);
 
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

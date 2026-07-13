//-----------------------------------------------------------------------------
// File : MeshVS.hlsl
// Desc : Vertex Shader For Model Drawing.
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
    float4  Position    : SV_POSITION;
    float3  Normal      : NORMAL;
    float4  Tangent     : TANGENT;
    float2  TexCoord    : TEXCOORD0;
    float4  Color       : COLOR0;
    float4  WorldPos    : WORLD_POS;
};

///////////////////////////////////////////////////////////////////////////////
// SceneParam constant buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer SceneParam : register(b0)
{
    float4x4    View;
    float4x4    Proj;
    float3      CameraPos;
    float       FieldOfView;
    float       NearClip;
    float       FarClip;
    float       TargetWidth;
    float       TargetHeight;
};

///////////////////////////////////////////////////////////////////////////////
// ModelParam constant buffers.
///////////////////////////////////////////////////////////////////////////////
cbuffer ModelParam : register(b1)
{
    uint    MatrixId;
    uint    Mode;
    uint2   Reserved;
};

StructuredBuffer<Transform4x3>  WorldMatrices : register(t0);

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
VSOutput main(const VSInput input, uint instanceId : SV_InstanceID)
{
    VSOutput output = (VSOutput)0;

    Transform4x3 world = WorldMatrices[MatrixId + instanceId];

    float4 worldPos = float4(Transform(world, input.Position), 1.0f);
    float4 viewPos  = Transform(View, worldPos);
    float4 projPos  = Transform(Proj, viewPos);

    float3 worldNormal  = normalize(TransformNormal(world, input.Normal));
    float3 worldTangent = normalize(TransformNormal(world, input.Tangent.xyz));
 
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

    output.Position     = projPos;
    output.Normal       = worldNormal;
    output.Tangent      = float4(worldTangent, sign);
    output.TexCoord     = input.TexCoord;
    output.Color        = input.Color;
    output.WorldPos     = worldPos;

    return output;
}
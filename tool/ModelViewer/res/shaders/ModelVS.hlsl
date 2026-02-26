//-----------------------------------------------------------------------------
// File : ModelVS.hlsl
// Desc : Vertex Shader For Model Drawing.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

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
    float4  Position    : SV_POSITION;
    float3  Normal      : NORMAL;
    float4  Tangent     : TANGENT;
    float2  TexCoord    : TEXCOORD0;
    float4  Color       : COLOR0;
    uint4   BoneIndices : BONEINDEX;
    float4  BoneWeights : BONEWEIGHT;
    float4  WorldPos    : WORLD_POS;
};

///////////////////////////////////////////////////////////////////////////////
// SceneParam constant buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer SceneParam : register(b0)
{
    float4x4    World;
    float4x4    View;
    float4x4    Proj;
    float3      CameraPos;
    float       FieldOfView;
    float       NearClip;
    float       FarClip;
    float       TargetWidth;
    float       TargetHeight;
};

StructuredBuffer<float4x4> MatrixPallets : register(t0);

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
VSOutput main(const VSInput input)
{
    VSOutput output = (VSOutput)0;
    float4 localPos     = float4(input.Position,    1.0f);
    float4 localNormal  = float4(input.Normal,      0.0f);
    float4 localTangent = float4(input.Tangent.xyz, 0.0f);

    float4 skinnedPos     = (float4)0;
    float4 skinnedNormal  = (float4)0;
    float4 skinnedTangent = (float4)0;

    for (int i=0; i<4; ++i)
    {
        skinnedPos     += mul(MatrixPallets[input.BoneIndices[i]], localPos    ) * input.BoneWeights[i];
        skinnedNormal  += mul(MatrixPallets[input.BoneIndices[i]], localNormal ) * input.BoneWeights[i];
        skinnedTangent += mul(MatrixPallets[input.BoneIndices[i]], localTangent) * input.BoneWeights[i];
    }
 
    float4 worldPos = mul(World, skinnedPos);
    float4 viewPos  = mul(View,  worldPos);
    float4 projPos  = mul(Proj,  viewPos);

    float3 worldNormal  = normalize(mul((float3x3)World, skinnedNormal.xyz));
    float3 worldTangent = normalize(mul((float3x3)World, skinnedTangent.xyz));
 
    output.Position     = projPos;
    output.Normal       = worldNormal;
    output.Tangent      = float4(worldTangent, input.Tangent.w);
    output.TexCoord     = input.TexCoord;
    output.Color        = input.Color;
    output.BoneIndices  = input.BoneIndices;
    output.BoneWeights  = input.BoneWeights;
    output.WorldPos     = worldPos;

    return output;
}
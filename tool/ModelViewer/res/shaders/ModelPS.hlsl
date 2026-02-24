//-----------------------------------------------------------------------------
// File : ModelPS.hlsl
// Desc : Pixel Shader For Model Drawing.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxBRDF.hlsli"
#include "asdxTangentSpace.hlsli"
#include "asdxSamplers.hlsli"


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

///////////////////////////////////////////////////////////////////////////////
// PixelParam constant buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer PixelParam : register(b1)
{
    uint    Mode;
    uint3   Reserved;
};

///////////////////////////////////////////////////////////////////////////////
// MaterialParam constant buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer MaterialParam : register(b2)
{
    float3  BaseColor;
    float   Alpha;
    float   Occlusion;
    float   Roughness;
    float   Metalness;
    float   Ior;
    float3  Emissive;
    float   Sheen;
}

//-----------------------------------------------------------------------------
// Resources
//-----------------------------------------------------------------------------
Texture2D BaseColorMap  : register(t1);
Texture2D NormalMap     : register(t2);
Texture2D OrmMap        : register(t3);
Texture2D EmissiveMap   : register(t4);

#define MODE_LIGHTING       (0)
#define MODE_POSITION       (1)
#define MODE_NORMAL         (2)
#define MODE_TANGENT        (3)
#define MODE_BITANGENT      (4)
#define MODE_TEXCOORD       (5)
#define MODE_COLOR          (6)
#define MODE_COLOR_R_ONLY   (7)
#define MODE_COLOR_G_ONLY   (8)
#define MODE_COLOR_B_ONLY   (9)
#define MODE_COLOR_A_ONLY   (10)
#define MODE_BLENDINDEX     (11)
#define MODE_BLENDWEIGHT    (12)
#define MODE_BASE_COLOR     (13)
#define MODE_OCCLUSION      (14)
#define MODE_ROUGHNESS      (15)
#define MODE_METALNESS      (16)
#define MODE_ALPHA          (17)
#define MODE_IOR            (18)
#define MODE_EMISSIVE       (19)

//-----------------------------------------------------------------------------
//      色相からRGB値を求めます.
//-----------------------------------------------------------------------------
float3 HueToRGB(float hue)
{
    // https://www.ronja-tutorials.com/post/041-hsv-colorspace/
    hue = frac(hue); //only use fractional part of hue, making it loop
    float r = -1.0f + abs(hue * 6.0f - 3.0f); //red
    float g =  2.0f - abs(hue * 6.0f - 2.0f); //green
    float b =  2.0f - abs(hue * 6.0f - 4.0f); //blue
    return saturate(float3(r, g, b)); //clamp between 0 and 1
}

//-----------------------------------------------------------------------------
//      リニアからSRGBへの変換.
//-----------------------------------------------------------------------------
float3 ToSRGB(float3 color)
{
    float3 result;
    result.x = (color.x < 0.0031308f) ? 12.92f * color.x : 1.055f * pow(abs(color.x), 1.0f / 2.4f) - 0.05f;
    result.y = (color.y < 0.0031308f) ? 12.92f * color.y : 1.055f * pow(abs(color.y), 1.0f / 2.4f) - 0.05f;
    result.z = (color.z < 0.0031308f) ? 12.92f * color.z : 1.055f * pow(abs(color.z), 1.0f / 2.4f) - 0.05f;

    return result;
}

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET0
{
    float3 gN = normalize(input.Normal);
    float3 gT = normalize(input.Tangent.xyz);
    float3 gB = normalize(cross(gN, gT) * input.Tangent.w);
 
    float3 tN = normalize(NormalMap.Sample(LinearClamp, input.TexCoord).xyz * 2.0f - 1.0f);

    float3 N = FromTangentSpaceToWorld(tN, gT, gB, gN);
    float3 T = RecalcTangent(N, gN);
    float3 B = cross(T, N);

    float4 output = 1.0f.xxxx;

    switch (Mode)
    {
    case MODE_LIGHTING:
    default:
        {
            float3 L = normalize(View._31_32_33);
            output.xyz = saturate(dot(N, L)).xxx * 0.5f.xxx;    // 陰影を見やすくするために白ではなく0.5にした.
        }
        break;

    case MODE_POSITION:
        {
            output.r = input.Position.x / TargetWidth;
            output.g = input.Position.y / TargetHeight;
            output.b = input.Position.z;
        }
        break;

    case MODE_NORMAL:
        { output.rgb = N * 0.5f + 0.5f; }
        break;

    case MODE_TANGENT:
        { output.rgb = T * 0.5f + 0.5f; }
        break;

    case MODE_BITANGENT:
        { output.rgb = B * 0.5f + 0.5f; }
        break;

    case MODE_TEXCOORD:
        { output.xy = input.TexCoord; }
        break;

    case MODE_COLOR:
        { output = input.Color; }
        break;
 
    case MODE_COLOR_R_ONLY:
        { output.rgb = input.Color.rrr; }
        break;
 
    case MODE_COLOR_G_ONLY:
        { output.rgb = input.Color.ggg; }
        break;
 
    case MODE_COLOR_B_ONLY:
        { output.rgb = input.Color.bbb; }
        break;
 
    case MODE_COLOR_A_ONLY:
        { output.rgb = input.Color.aaa; }
        break;

    case MODE_BLENDINDEX:
        {
            output.r = dot(float3(1.0f, 0.0f, 0.0f), ToSRGB(HueToRGB(input.BoneIndices.x * 1.1f)));
            output.g = dot(float3(0.0f, 1.0f, 0.0f), ToSRGB(HueToRGB(input.BoneIndices.y * 1.1f)));
            output.b = dot(float3(0.0f, 0.0f, 1.0f), ToSRGB(HueToRGB(input.BoneIndices.z * 1.1f)));
            output.a = dot(float3(1.0f, 1.0f, 1.0f), ToSRGB(HueToRGB(input.BoneIndices.w * 1.1f)));
        }
        break;

    case MODE_BLENDWEIGHT:
        { output = input.BoneWeights; }
        break;

    case MODE_BASE_COLOR:
        { output.rgb = BaseColorMap.Sample(LinearClamp, input.TexCoord).rgb * BaseColor; }
        break;
        
    case MODE_OCCLUSION:
        { output.rgb = OrmMap.Sample(LinearClamp, input.TexCoord).rrr * Occlusion; }
        break;

    case MODE_ROUGHNESS:
        { output.rgb = OrmMap.Sample(LinearClamp, input.TexCoord).ggg * Roughness; }
        break;
 
    case MODE_METALNESS:
        { output.rgb = OrmMap.Sample(LinearClamp, input.TexCoord).bbb * Metalness; }
        break;
 
    case MODE_ALPHA:
        { output.rgb = BaseColorMap.Sample(LinearClamp, input.TexCoord).aaa * Alpha; }
        break;
 
    case MODE_IOR:
        { output.rgb = Ior.xxx; }
        break;
 
    case MODE_EMISSIVE:
        { output.rgb = EmissiveMap.Sample(LinearClamp, input.TexCoord).rgb * Emissive; }
        break;
    }

    return output;
}
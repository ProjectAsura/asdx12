//-----------------------------------------------------------------------------
// File : MeshPS.hlsl
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
    float   UnusedParam0;
}

//-----------------------------------------------------------------------------
// Resources
//-----------------------------------------------------------------------------
Texture2D BaseColorMap  : register(t1);
Texture2D NormalMap     : register(t2);
Texture2D OrmMap        : register(t3);
Texture2D EmissiveMap   : register(t4);

Texture2D   DFGMap      : register(t5);     //!< DFGマップ.
TextureCube DiffuseLD   : register(t6);     //!< Diffuse LD.
TextureCube SpecularLD  : register(t7);     //!< Specular LD.

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
//      ディフューズIBLを評価します.
//-----------------------------------------------------------------------------
float3 EvaluateIBLDiffuse(float3 N)
{
    // Lambert BRDFはDFG項は積分すると1.0となるので，LD項のみを返却すれば良い
    return DiffuseLD.Sample(LinearWrap, N).rgb;
}

//-----------------------------------------------------------------------------
//      線形ラフネスからミップレベルを求めます.
//-----------------------------------------------------------------------------
float RoughnessToMipLevel(float linearRoughness, float mipCount)
{
    return (mipCount - 1) * linearRoughness;
}

//-----------------------------------------------------------------------------
//      スペキュラーIBLを評価します.
//-----------------------------------------------------------------------------
float3 EvaluateIBLSpecular
(
    float           NdotV,          // 法線ベクトルと視線ベクトルの内積.
    float3          N,              // 法線ベクトル.
    float3          R,              // 反射ベクトル.
    float3          f0,             // フレネル項
    float           roughness       // 線形ラフネス.
)
{
    float  a = roughness * roughness;
    float3 dominantR = GetSpecularDominantDir(N, R, a);

    float2 mapSize;
    float  mipLevels;
    SpecularLD.GetDimensions(0, mapSize.x, mapSize.y, mipLevels);
    float textureSize = max(mapSize.x, mapSize.y);

    // 関数を再構築.
    // L * D * (f0 * Gvis * (1 - Fc) + Gvis * Fc) * cosTheta / (4 * NdotL * NdotV).
    NdotV = max(NdotV, 0.5f / textureSize); // ゼロ除算が発生しないようにする.
    float  mipLevel = RoughnessToMipLevel(roughness, mipLevels); 
    float3 preLD    = SpecularLD.SampleLevel(LinearWrap, dominantR, mipLevel).xyz;

    // 事前積分したDFGをサンプルする.
    // Fc = ( 1 - HdotL )^5
    // PreIntegratedDFG.r = Gvis * (1 - Fc)
    // PreIntegratedDFG.g = Gvis * Fc
    float2 preDFG   = DFGMap.SampleLevel(LinearClamp, float2(NdotV, 1.0f - roughness), 0).xy;

    // LD * (f0 * Gvis * (1 - Fc) + Gvis * Fc)
    return preLD * (f0 * preDFG.x + preDFG.y);
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
            float3 V = normalize(input.WorldPos.xyz - GetPosition(View));
            float3 R = normalize(reflect(V, N));
 
            float NoV = saturate(dot(N, V));

            float4 bc  = BaseColorMap.Sample(LinearWrap, input.TexCoord);
            bc.rgb *= BaseColor;
            bc.a   *= Alpha;
            
            float3 orm = OrmMap.Sample(LinearWrap, input.TexCoord).rgb;
            orm.x *= Occlusion;
            orm.y *= Roughness;
            orm.z *= Metalness;

            float3 Kd = ToKd(bc.rgb, orm.z);
            float3 Ks = ToKs(bc.rgb, orm.z);
 
            float3 lit = 0;
            lit += EvaluateIBLDiffuse(N) * Kd * orm.x;
            lit += EvaluateIBLSpecular(NoV, N, R, Ks, orm.y) * orm.x;
            lit += EmissiveMap.Sample(LinearWrap, input.TexCoord).xyz * Emissive;
            output.rgb = lit;
            output.a   = bc.a;
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
        { output.rgb = input.Color.rgb; }
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
    case MODE_BLENDWEIGHT:
        { output.rgb = 0.0f.xxx; }
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
        { output.rgb = BaseColorMap.Sample(LinearClamp, input.TexCoord).a * Alpha.xxx; }
        break;
 
    case MODE_IOR:
        { output.rgb = max(Ior - 1.0f, 0.0f).xxx; }
        break;

    case MODE_EMISSIVE:
        { output.rgb = EmissiveMap.Sample(LinearClamp, input.TexCoord).rgb * Emissive; }
        break;
    }

    return output;
}
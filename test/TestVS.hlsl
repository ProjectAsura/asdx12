//-----------------------------------------------------------------------------
// File : TestVS.hlsl
// Desc : Vertex Shader For Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// Vertex structure
///////////////////////////////////////////////////////////////////////////////
struct Vertex
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD;
};

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

///////////////////////////////////////////////////////////////////////////////
// CbMesh constant buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbMesh : register(b0)
{
    float4x4 gWorld;
};

///////////////////////////////////////////////////////////////////////////////
// CbScene constant buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbScene : register(b1)
{
    float4x4 gView;
    float4x4 gProj;
};

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
VSOutput main(const Vertex input)
{
    VSOutput output = (VSOutput) 0;

    float4 localPos = float4(input.Position, 1.0f);
    //float4 worldPos = mul(localPos, gWorld);
    //float4 viewPos  = mul(worldPos, gView);
    //float4 projPos  = mul(viewPos,  gProj);

    output.Position = localPos;//projPos;
    output.TexCoord = input.TexCoord;
    
    return output;
}
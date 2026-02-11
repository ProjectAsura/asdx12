//-----------------------------------------------------------------------------
// File: asdxLineVS.hlsl
// Desc: Vertex Shader For Debug Line.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// VSInput structure
///////////////////////////////////////////////////////////////////////////////
struct VSInput
{
    float3 Position : POSITION;
    float4 Color    : COLOR0;
};

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR0;
};

///////////////////////////////////////////////////////////////////////////////
// CbParam constant buffers.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbParam : register(b0)
{
    float4x4 View;
    float4x4 Proj;
};

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
VSOutput main(const VSInput input)
{
    VSOutput output = (VSOutput)0;
    
    float4 worldPos = float4(input.Position, 1.0f);
    float4 viewPos = mul(View, worldPos);
    float4 projPos = mul(Proj, viewPos);

    output.Position = projPos;
    output.Color    = input.Color;

    return output;
}

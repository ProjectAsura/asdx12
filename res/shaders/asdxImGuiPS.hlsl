//-----------------------------------------------------------------------------
// File : asdxImGuiPS.hlsl
// Desc : Pixel Shader For ImGui.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// PSInput structure
///////////////////////////////////////////////////////////////////////////////
struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color    : COLOR0;   
};

//-----------------------------------------------------------------------------
// Texture and Samplers.
//-----------------------------------------------------------------------------
Texture2D       Texture0 : register(t0);
SamplerState    Sampler0 : register(s0);

//-----------------------------------------------------------------------------
//      ���C���G���g���[�|�C���g�ł�.
//-----------------------------------------------------------------------------
float4 main(PSInput input) : SV_TARGET
{
    float4 texel = Texture0.Sample(Sampler0, input.TexCoord);

    float4 result;
    result.rgb = pow(input.Color.rgb, 2.2) * texel.rgb;
    result.a   = input.Color.a * texel.a;

    return result;
}
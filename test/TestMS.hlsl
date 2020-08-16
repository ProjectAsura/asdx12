//-----------------------------------------------------------------------------
// File : TestMS.hlsl
// Desc : Mesh Shader Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// MSInput structure
///////////////////////////////////////////////////////////////////////////////
struct MSInput
{
    float3 Position;    // ���_���W.
    float2 TexCoord;    // �e�N�X�`�����W.
};

///////////////////////////////////////////////////////////////////////////////
// VertexOutput structure
///////////////////////////////////////////////////////////////////////////////
struct MSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};


//-----------------------------------------------------------------------------
// Resources and Samplers.
//-----------------------------------------------------------------------------
StructuredBuffer<MSInput>   Vertices    : register(t0);
StructuredBuffer<uint3>     Indices     : register(t1);

//-----------------------------------------------------------------------------
//      ���C���G���g���[�|�C���g�ł�.
//-----------------------------------------------------------------------------
[numthreads(32, 1, 1)]
[outputtopology("triangle")]
void main
(
    uint groupIndex : SV_GroupIndex,
    out vertices MSOutput verts[3],
    out indices  uint3    polys[1]
)
{
    // �X���b�h�O���[�v�̒��_�ƃv���~�e�B�u�̐���ݒ�.
    SetMeshOutputCounts(3, 1);

    if (groupIndex < 1)
    {
        polys[groupIndex] = Indices[groupIndex];         // ���_�C���f�b�N�X��ݒ�.
    }

    if (groupIndex < 3)
    {
        MSOutput vout;
        vout.Position   = float4(Vertices[groupIndex].Position, 1.0f);
        vout.TexCoord   = Vertices[groupIndex].TexCoord;

        verts[groupIndex] = vout;   // ���_���o��.
    }
}

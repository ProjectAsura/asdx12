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
    float3 Position;    // 頂点座標.
    float2 TexCoord;    // テクスチャ座標.
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
//      メインエントリーポイントです.
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
    // スレッドグループの頂点とプリミティブの数を設定.
    SetMeshOutputCounts(3, 1);

    if (groupIndex < 1)
    {
        polys[groupIndex] = Indices[groupIndex];         // 頂点インデックスを設定.
    }

    if (groupIndex < 3)
    {
        MSOutput vout;
        vout.Position   = float4(Vertices[groupIndex].Position, 1.0f);
        vout.TexCoord   = Vertices[groupIndex].TexCoord;

        verts[groupIndex] = vout;   // 頂点を出力.
    }
}

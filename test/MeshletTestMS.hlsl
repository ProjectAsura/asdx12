//-----------------------------------------------------------------------------
// File : MeshletTestMS.hlsl
// Desc : Test For Meshlet Draw.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxMath.hlsli"


///////////////////////////////////////////////////////////////////////////////
// MSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct MSOutput
{
    float4  Position     : SV_POSITION;
    uint    TexCoord     : TEXCOORD;
    uint    TangentSpace : TANGENT_SPACE;
    uint    MeshletId    : MESHLET_ID;
};

///////////////////////////////////////////////////////////////////////////////
// Meshlet structure
///////////////////////////////////////////////////////////////////////////////
struct Meshlet
{
    uint VertexOffset;      // 頂点番号オフセット.
    uint VertexCount;       // 出力頂点数.
    uint PrimitiveOffset;   // プリミティブ番号オフセット.
    uint PrimitiveCount;    // 出力プリミティブ数.
};

///////////////////////////////////////////////////////////////////////////////
// MeshParam structure
///////////////////////////////////////////////////////////////////////////////
struct MeshParam
{
    float4x4 World;
};

///////////////////////////////////////////////////////////////////////////////
// SceneParam structure
///////////////////////////////////////////////////////////////////////////////
struct SceneParam
{
    float4x4 View;
    float4x4 Proj;
};

//-----------------------------------------------------------------------------
// Resources
//-----------------------------------------------------------------------------
StructuredBuffer<float3>    Positions       : register(t0);
StructuredBuffer<uint>      TangentSpaces   : register(t1);
StructuredBuffer<uint>      TexCoords       : register(t2);
StructuredBuffer<uint>      Indices         : register(t3);
StructuredBuffer<uint>      Primitives      : register(t4);
StructuredBuffer<Meshlet>   Meshlets        : register(t5);
ConstantBuffer<MeshParam>   CbMesh          : register(b0);
ConstantBuffer<SceneParam>  CbScene         : register(b1);


//-----------------------------------------------------------------------------
//      メインエントリーポイント.
//-----------------------------------------------------------------------------
[numthreads(128, 1, 1)]
[outputtopology("triangle")]
void main
(
    uint groupThreadId  : SV_GroupThreadID,
    uint groupId        : SV_GroupId,
    out  vertices MSOutput verts[64],
    out  indices  uint3    polys[126]
)
{
    Meshlet m = Meshlets[groupId];
    SetMeshOutputCounts(m.VertexCount, m.PrimitiveCount);
    
    if (groupThreadId < m.PrimitiveCount)
    {
        uint packedIndex = Primitives[m.PrimitiveOffset + groupThreadId];
        polys[groupThreadId] = UnpackPrimitiveIndex(packedIndex);
    }
    
    if (groupThreadId < m.VertexCount)
    {
        uint index = Indices[m.VertexOffset + groupThreadId];
        float4 localPos = float4(Positions[index], 1.0f);
        float4 worldPos = mul(CbMesh.World, localPos);
        float4 viewPos  = mul(CbScene.View, worldPos);
        float4 projPos  = mul(CbScene.Proj, viewPos);

        float3 T, N;
        uint encodedTBN = TangentSpaces[index];
        UnpackTN(encodedTBN, T, N);

        float3 worldT = mul((float3x3)CbMesh.World, T);
        float3 worldN = mul((float3x3)CbMesh.World, N);
        uint worldEncodedTBN = PackTBN(N, T, 0);

        MSOutput output = (MSOutput)0;
        output.Position     = projPos;
        output.TexCoord     = TexCoords[index];
        output.TangentSpace = worldEncodedTBN;
        output.MeshletId    = groupId;

        verts[groupThreadId] = output;
    }
}

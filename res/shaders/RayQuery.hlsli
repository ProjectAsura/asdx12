//-----------------------------------------------------------------------------
// File : RayQuery.hlsli
// Desc : Ray Query Wrapper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#ifndef RAY_QUERY_HLSLI
#define RAY_QUERY_HLSLI

// 無効なインデックス.
#define INVALID_INDEX 0xFFFFFFFF

// TLASの定義.
typedef RaytracingAccelerationStructure Tlas;

///////////////////////////////////////////////////////////////////////////////
// HitRecord structure
///////////////////////////////////////////////////////////////////////////////
struct HitRecord
{
    uint    InstanceIndex;      //!< インスタンス番号.
    uint    PrimitiveIndex;     //!< プリミティブ番号.
    float2  BaryCentrics;       //!< 重心座標.
    float   Distance;           //!< ヒット距離.

    bool IsHit()
    { return PrimitiveIndex != INVALID_INDEX; }

    float3 GetBaryCentrics()
    { return float3(BaryCentrics, saturate(1.0f - BaryCentrics.x - BaryCentrics.y)); }
};

//-----------------------------------------------------------------------------
//      交差判定を行います.
//-----------------------------------------------------------------------------
HitRecord Intersect
(
    Tlas    tlas,                   //!< 高速化機構.
    uint    rayFlags,               //!< レイフラグ.
    uint    instanceInclusionMask,  //!< インスタンスマスク.
    float3  rayOrigin,              //!< レイの原点.
    float3  rayDirection,           //!< レイの方向ベクトル.
    float   tMin,                   //!< 交差許容最小距離.
    float   tMax                    //!< 交差許容最大距離.
)
{
    // レイの設定.
    RayDesc rayDesc;
    rayDesc.Origin    = rayOrigin;
    rayDesc.Direction = rayDirection;
    rayDesc.TMin      = tMin;
    rayDesc.TMax      = tMax;

    RayQuery<RAY_FLAG_NONE> rayQuery;

    // レイトレ実行.
    rayQuery.TraceRayInline(
        tlas,
        rayFlags,
        instanceInclusionMask,
        rayDesc);
 
    // 交差確定するまでループ.
    while (rayQuery.Proceed())
    { /* DO_NOTHING */ }

    const bool isHit = rayQuery.CommittedStatus() != COMMITTED_NOTHING;

    HitRecord result;
    result.PrimitiveIndex = (isHit) ? rayQuery.CommittedPrimitiveIndex() : INVALID_INDEX;
    result.InstanceIndex  = (isHit) ? rayQuery.CommittedInstanceIndex () : INVALID_INDEX;
    result.BaryCentrics   = (isHit) ? rayQuery.CommittedTriangleBarycentrics() : 0.0f.xx;
    result.Distance       = (isHit) ? rayQuery.CommittedRayT() : -1.0f;

    return result;
}

#endif//RAY_QUERY_HLSLI

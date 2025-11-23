//-----------------------------------------------------------------------------
// File : RtCommon.hlsli
// Desc : Ray Tracing Common Header.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#ifndef RT_COMMON_HLSLI
#define RT_COMMON_HLSLI


#ifndef ASDX_ENABLE_GEOMETRY_ID
#define ASDX_ENABLE_GEOMETRY_ID    (1)  // DXR Tier 1.1
#endif//ASDX_ENABLE_GEOMETRY_ID


//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static const uint INVALID_ID            = 0xffffffff;   //!< 無効なID.
static const uint RAY_INDEX_DEFAULT     = 0;            //!< 標準レイ番号.
static const uint RAY_INDEX_SHADOW      = 1;            //!< シャドウレイ番号.

//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------
typedef BuiltInTriangleIntersectionAttributes   HitArgs;
typedef RaytracingAccelerationStructure         Tlas;


///////////////////////////////////////////////////////////////////////////////
// RtPayload structure
///////////////////////////////////////////////////////////////////////////////
struct RtPayload
{
    uint    InstanceId;         //!< インスタンス番号.
    uint    PrimitiveId;        //!< プリミティブ番号.
    float2  Barycentrics;       //!< 重心座標.
#if ASDX_ENABLE_GEOMETRY_ID
    uint    GeometryId;         //!< ジオメトリ番号(=メッシュ番号).
#endif

    //-------------------------------------------------------------------------
    //      クリアします.
    //-------------------------------------------------------------------------
    void Clear()
    {
        InstanceId   = INVALID_ID;
        PrimitiveId  = INVALID_ID;
        Barycentrics = 0.0f.xx;
    #if ASDX_ENABLE_GEOMETRY_ID
        GeometryId   = INVALID_ID;
    #endif
    }

    //-------------------------------------------------------------------------
    //      ヒット判定を行います.
    //-------------------------------------------------------------------------
    bool HasHit(uint instanceId, uint primitiveId)
    {
        // 無効なIDなら未ヒット.
        if (InstanceId == INVALID_ID)
            return false;
        // 自分自身なら未ヒット.
        if (InstanceId == instanceId && PrimitiveId == primitiveId)
            return false;
        // ヒット.
        return true;
    }
 
#if ASDX_ENABLE_GEOMETRY_ID
    //-------------------------------------------------------------------------
    //      ヒット判定を行います.
    //-------------------------------------------------------------------------
    bool HasHit(uint instanceId, uint primitiveId, uint geometryId)
    {
        // 無効なIDなら未ヒット.
        if (InstanceId == INVALID_ID)
            return false;
        // 自分自身なら未ヒット.
        if (InstanceId == instanceId && PrimitiveId == primitiveId && GeometryId == geometryId)
            return false;
        // ヒット.
        return true;
    }
#endif
};

//-----------------------------------------------------------------------------
//      ヒット時の処理です.
//-----------------------------------------------------------------------------
[shader("closesthit")]
void OnClosestHit(inout RtPayload payload, in HitArgs args)
{
    payload.InstanceId   = InstanceID();
    payload.PrimitiveId  = PrimitiveIndex();
    payload.Barycentrics = args.barycentrics;
#if ASDX_ENABLE_GEOMETRY_ID
    payload.GeometryId  = GeometryIndex();  // DXR Tier 1.1
#endif
}

//-----------------------------------------------------------------------------
//      ミス時の処理です.
//-----------------------------------------------------------------------------
[shader("miss")]
void OnMiss(inout RtPayload payload)
{ payload.Clear(); }

//-----------------------------------------------------------------------------
//      シャドウレイヒット時の処理です.
//-----------------------------------------------------------------------------
[shader("anyhit")]
void OnShadowAnyHit(inout RtPayload payload, in HitArgs args)
{
    payload.InstanceId   = InstanceID();
    payload.PrimitiveId  = PrimitiveIndex();
    payload.Barycentrics = args.barycentrics;
#if ASDX_ENABLE_GEOMETRY_ID
    payload.GeometryId   = GeometryIndex(); // DXR Tier 1.1
#endif
    AcceptHitAndEndSearch();
}

//-----------------------------------------------------------------------------
//      シャドウレイミス時の処理です.
//-----------------------------------------------------------------------------
[shader("miss")]
void OnShadowMiss(inout RtPayload payload)
{ payload.Clear(); }

//-----------------------------------------------------------------------------
//      レイを求めます.
//-----------------------------------------------------------------------------
RayDesc CalcRay(float2 pixel, float4x4 view, float4x4 proj, float tmin, float tmax)
{
    // pixel : [-1, 1]の正規化されているピクセル座標とします(y方向は反転補正済みの前提).
    // view : ビュー行列.
    // proj : 透視投影行列.

    // カメラ位置.
    float4 pos = -view._11_12_13 * view._14
                 -view._21_22_23 * view._24
                 -view._31_32_33 * view._34;

    float aspect  = proj._22 / proj._11;
    float tanFovY = 1.0f / proj._22;

    // レイの方向ベクトル.
    float dir = normalize(
        (view._11_12_13 * pixel.x * tanFovY * aspect)
      + (view._21_22_23 * pixel.y * tanFovY)
      - (view._31_32_33)); // 右手系なので，レイの進む向きは-Z方向.

    // レイを設定.
    RayDesc result;
    result.Origin       = pos.xyz;
    result.Direction    = dir;
    result.TMin         = tmin;
    result.TMax         = tmax;

    return result;
}

//-----------------------------------------------------------------------------
//      重心座標を取得します.
//-----------------------------------------------------------------------------
float3 CalcBarycentrics(float2 value)
{ return float3(1.0f - value.x - value.y, value.x, value.y); }

//-----------------------------------------------------------------------------
//      レイのオフセット値を取得します.
//-----------------------------------------------------------------------------
float3 OffsetRay(const float3 p, const float3 n)
{
    // Ray Tracing Gems, Chapter 6.
    static const float origin       = 1.0f / 32.0f;
    static const float float_scale  = 1.0f / 65536.0f;
    static const float int_scale    = 256.0f;

    int3 of_i = int3(int_scale * n.x, int_scale * n.y, int_scale * n.z);

    float3 p_i = float3(
        asfloat(asint(p.x) + ((p.x < 0) ? -of_i.x : of_i.x)),
        asfloat(asint(p.y) + ((p.y < 0) ? -of_i.y : of_i.y)),
        asfloat(asint(p.z) + ((p.z < 0) ? -of_i.z : of_i.z)));

    return float3(
        abs(p.x) < origin ? p.x + float_scale * n.x : p_i.x,
        abs(p.y) < origin ? p.y + float_scale * n.y : p_i.y,
        abs(p.z) < origin ? p.z + float_scale * n.z : p_i.z);
}

//-----------------------------------------------------------------------------
//      シャドウレイ用のフラグを取得します.
//-----------------------------------------------------------------------------
uint GetShadowRayFlag()
{
    return RAY_FLAG_SKIP_CLOSEST_HIT_SHADER 
         | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH
         | RAY_FLAG_CULL_FRONT_FACING_TRIANGLES;
}

#endif//RT_COMMON_HLSLI

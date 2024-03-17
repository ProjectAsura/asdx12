//-----------------------------------------------------------------------------
// File : RayQuery.hlsli
// Desc : Ray Query Wrapper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#ifndef RAY_QUERY_HLSLI
#define RAY_QUERY_HLSLI

// �����ȃC���f�b�N�X.
#define INVALID_INDEX 0xFFFFFFFF

// TLAS�̒�`.
typedef RaytracingAccelerationStructure Tlas;

///////////////////////////////////////////////////////////////////////////////
// HitRecord structure
///////////////////////////////////////////////////////////////////////////////
struct HitRecord
{
    uint    InstanceIndex;      //!< �C���X�^���X�ԍ�.
    uint    PrimitiveIndex;     //!< �v���~�e�B�u�ԍ�.
    float2  BaryCentrics;       //!< �d�S���W.
    float   Distance;           //!< �q�b�g����.

    bool IsHit()
    { return PrimitiveIndex != INVALID_INDEX; }

    float3 GetBaryCentrics()
    { return float3(BaryCentrics, saturate(1.0f - BaryCentrics.x - BaryCentrics.y)); }
};

//-----------------------------------------------------------------------------
//      ����������s���܂�.
//-----------------------------------------------------------------------------
HitRecord Intersect
(
    Tlas    tlas,                   //!< �������@�\.
    uint    rayFlags,               //!< ���C�t���O.
    uint    instanceInclusionMask,  //!< �C���X�^���X�}�X�N.
    float3  rayOrigin,              //!< ���C�̌��_.
    float3  rayDirection,           //!< ���C�̕����x�N�g��.
    float   tMin,                   //!< �������e�ŏ�����.
    float   tMax                    //!< �������e�ő勗��.
)
{
    // ���C�̐ݒ�.
    RayDesc rayDesc;
    rayDesc.Origin    = rayOrigin;
    rayDesc.Direction = rayDirection;
    rayDesc.TMin      = tMin;
    rayDesc.TMax      = tMax;

    RayQuery<RAY_FLAG_NONE> rayQuery;

    // ���C�g�����s.
    rayQuery.TraceRayInline(
        tlas,
        rayFlags,
        instanceInclusionMask,
        rayDesc);
 
    // �����m�肷��܂Ń��[�v.
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

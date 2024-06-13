//----------------------------------------------------------------------------
// File : SpericalHarmonics.hlsli
// Desc : Sperical Harmonics Utility.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#ifndef ASDX_SPHERICAL_HARMONICS_HLSLI
#define ASDX_SPHERICAL_HARMONCIS_HLSLI


//-----------------------------------------------------------------------------
//      方向ベクトルから球面調和関数の係数ベクトルを求めます.
//-----------------------------------------------------------------------------
void EvaluateSH2(float3 dir, out float3 sh[4])
{
    sh[0] = 0.2820947917738781f;
    sh[1] = 0.4886025119029199f * dir.y;
    sh[2] = 0.4886025119029199f * dir.z;
    sh[3] = 0.4886025119029199f * dir.x;
}

//-----------------------------------------------------------------------------
//      方向ベクトルから球面調和関数の係数ベクトルを求めます.
//-----------------------------------------------------------------------------
void EvaluateSH3(float3 dir, out float3 sh[9])
{
    // Peter-Pike Sloan, "Efficient Spherical Harmonic Evaluation",
    // Journal of Computer Graphics Techniques, Vol.2, No.2, 2013.
    sh[0] = 0.2820947917738781f;
    sh[1] = 0.4886025119029199f * dir.y;
    sh[2] = 0.4886025119029199f * dir.z;
    sh[3] = 0.4886025119029199f * dir.x;

    float tmp = 1.092548430592079f * dir.z;
    sh[4] = 0.5462742152960395f * (dir.x * dir.y + dir.y * dir.x);
    sh[5] = tmp * dir.y;
    sh[6] = 0.9461746957575601f * (dir.z * dir.z) - 0.3153915652525201f;
    sh[7] = tmp * dir.x;
    sh[8] = 0.5462742152960395f * (dir.x * dir.x - dir.y * dir.y);
}

//-----------------------------------------------------------------------------
//      球面調和関数の係数ベクトルから放射照度を求めます.
//-----------------------------------------------------------------------------
float3 IrraidanceSH2(float3 n, float3 sh[4])
{
    const float c2 = 0.51166335397324424423977581244463;
    const float c4 = 0.88622692545275801364908374167057;

    float3 result = c4 * sh[0]
           + 2.0f * c2 * (sh[3] * n.x + sh[1] * n.y + sh[2] * n.z);
    return max(0.0f, result);
}

//-----------------------------------------------------------------------------
//      球面調和関数の係数ベクトルから放射照度を求めます.
//-----------------------------------------------------------------------------
float3 IrradianceSH3(float3 n, float3 sh[9])
{
    // Patapom, "Spherical Harmonics", 
    // http://www.patapom.com/blog/SHPortal/
    const float c1 = 0.42904276540489171563379376569857;
    const float c2 = 0.51166335397324424423977581244463;
    const float c3 = 0.24770795610037568833406429782001;
    const float c4 = 0.88622692545275801364908374167057;

    float3 result = (c1 * (n.x * n.x - n.y * n.y)) * sh[8]
        + (c3 * (3.0f * n.z * n.z - 1)) * sh[6]
        + c4 * sh[0]
        + 2.0f * c1 * (sh[4] * n.x * n.y + sh[7] * n.x * n.z + sh[5] * n.y * n.z)
        + 2.0f * c2 * (sh[3] * n.x + sh[1] * n.y + sh[2] * n.z);
    return max(0.0f, result);
}

//-----------------------------------------------------------------------------
//      Geomerics方式で球面調和関数を評価します.
//-----------------------------------------------------------------------------
float IrradianceSH_NonlinearL1(float3 normal, float4 coeff)
{
    // William Joseph, "球面調和関数データからの拡散反射光の再現", CEDEC 2015,
    // https://cedil.cesa.or.jp/cedil_sessions/view/1329
    float  L0 = coeff.x;
    float3 L1 = coeff.yzw;
    float  modL1 = length(L1);
    if (modL1 == 0.0f)
    { return 0.0f; }

    float q = saturate(0.5f + 0.5f * dot(normal, normalize(L1)));
    float r = modL1 / L0;
    float p = 1.0f + 2.0f * r;
    float a = (1.0f - r) / (1.0f + r);

    return L0 * lerp((1.0f + p) * pow(q, p), 1.0f, a);
}


#endif//ASDX_SPHERICAL_HARMONICS_HLSLI
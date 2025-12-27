//-----------------------------------------------------------------------------
// File : asdReservior.hlsli
// Desc : asdxReservior.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#ifndef ASDX_RESERVIOR_HLSLI
#define ASDX_RESERVIOR_HLSLI


#if __HLSL_VERSION >= 2021
///////////////////////////////////////////////////////////////////////////////
// Reservior structure
///////////////////////////////////////////////////////////////////////////////
template<typename T>
struct Reservior
{
    T       Samples;        //!< サンプル.
    uint    Count;          //!< サンプル数.
    float   Weight;         //!< 重み.
    float   TargetPDF;      //!< ターゲットPDF.

    //-------------------------------------------------------------------------
    //      リセットします.
    //-------------------------------------------------------------------------
    void Reset()
    {
        Samples     = (T)0;
        Count       = 0;
        Weight      = 1.0f;
        TargetPDF   = 1.0f;
    }

    //-------------------------------------------------------------------------
    //      更新処理を行います.
    //-------------------------------------------------------------------------
    void Update(float random, Reservior<T> input)
    {
        Weight += input.Weight;
        if ((random < (input.Weight / Weight) || Count == 0)
        {
            Samples   = input.Samples;
            TargetPDF = input.TargetPDF;
        }
        Count++;
    }
};

//-----------------------------------------------------------------------------
//      マージ処理を行います.
//-----------------------------------------------------------------------------
Reservior<T> MergeReservior(Reservior<T> lhs, Reservior<T> rhs, float randomLhs, float randomRhs)
{
    Reservior<T> result;
    result.Reset();

    result.Update(lhs, randomLhs);
    result.Update(rhs, randomRhs);
    result.Count = lhs.Count + rhs.Count;
    return result;
}
#endif

#endif//RESERVIOR_HLSLI

//-----------------------------------------------------------------------------
// File : asdxTimer.h
// Desc : Timer Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <Windows.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Timer class
///////////////////////////////////////////////////////////////////////////////
class Timer
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

private:
    //=========================================================================
    // private variables
    //=========================================================================
    bool    m_IsStop;               //!< 停止状態かどうか.
    int64_t m_TicksPerSec;          //!< 1秒あたりのタイマー刻み数.
    int64_t m_StopTime;             //!< 停止時間.
    int64_t m_ElapsedTime;          //!< 最後の処理から経過時間です.
    int64_t m_BaseTime;             //!< タイマーの開始時間です.
    double  m_InvTicksPerSec;       //!< 1タイマー刻み数当たりの秒数.

    //=========================================================================
    // private methods
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      調整された現在時間を取得します.
    //-------------------------------------------------------------------------
    inline int64_t GetAdjustedCurrentTime( void )
    {
        LARGE_INTEGER qwTime;

        // 停止状態であれば，停止時間を返却.
        if ( m_StopTime != 0 )
        { qwTime.QuadPart = m_StopTime; }
        // 非停止状態ならば，現在のカウンタを取得.
        else
        { QueryPerformanceCounter( &qwTime ); }

        return qwTime.QuadPart;
    }

public:
    //=========================================================================
    // private variables
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // private methods
    //=========================================================================

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    Timer()
    : m_IsStop     ( true )
    , m_StopTime   ( 0 )
    , m_ElapsedTime( 0 )
    , m_BaseTime   ( 0 )
    {
        LARGE_INTEGER qwTicksPerSec = { 0 };

        // 周波数を取得します.
        QueryPerformanceFrequency( &qwTicksPerSec );

        m_TicksPerSec = qwTicksPerSec.QuadPart;
        m_InvTicksPerSec = 1.0 / static_cast<double>( m_TicksPerSec );
    }

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~Timer()
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      タイマーをリセットします.
    //-------------------------------------------------------------------------
    inline void Reset()
    {
        // 調整された現在時間を取得.
        int64_t qwTime = GetAdjustedCurrentTime();

        m_BaseTime    = qwTime;
        m_ElapsedTime = qwTime;
        m_StopTime    = 0;
        m_IsStop      = false;
    }

    //-------------------------------------------------------------------------
    //! @brief      タイマーを開始します.
    //-------------------------------------------------------------------------
    inline void Start()
    {
        LARGE_INTEGER qwTime = { 0 };

        // 現在のカウンタを取得.
        QueryPerformanceCounter( &qwTime );

        // 停止中ならベース時間を加算.
        if ( m_IsStop )
        { m_BaseTime += qwTime.QuadPart - m_StopTime; }

        m_StopTime    = 0;
        m_ElapsedTime = qwTime.QuadPart;
        m_IsStop      = false;
    }

    //-------------------------------------------------------------------------
    //! @brief      タイマーを停止します.
    //-------------------------------------------------------------------------
    inline void Stop()
    {
        if ( !m_IsStop )
        {
            LARGE_INTEGER qwTime = { 0 };

            // 現在のカウンタを取得.
            QueryPerformanceCounter( &qwTime );

            m_StopTime    = qwTime.QuadPart;
            m_ElapsedTime = qwTime.QuadPart;
            m_IsStop      = true;
        }
    }

    //-------------------------------------------------------------------------
    //! @brief      0.1秒タイマーを進めます.
    //-------------------------------------------------------------------------
    inline void Advance()
    { m_StopTime += m_TicksPerSec / 10; }

    //-------------------------------------------------------------------------
    //! @brief      停止状態かどうか判定します.
    //!
    //! @retval true    停止状態.
    //! @retval false   非停止状態.
    //-------------------------------------------------------------------------
    inline bool IsStop() const
    { return m_IsStop; }

    //-------------------------------------------------------------------------
    //! @brief      システム時間を取得します.
    //!
    //! @return     システム時間を返却します.
    //-------------------------------------------------------------------------
    inline double GetAbsoluteTime()
    {
        LARGE_INTEGER qwTime = { 0 };

        // 現在のカウンタを取得.
        QueryPerformanceCounter( &qwTime );

        // システム時間を算出して，返却する.
        return qwTime.QuadPart * m_InvTicksPerSec;
    }

    //-------------------------------------------------------------------------
    //! @brief      相対時間を取得します.
    //!
    //! @return     相対時間を返却します.
    //-------------------------------------------------------------------------
    inline double GetTime()
    {
        // 調整された現在時間を取得.
        int64_t qwTime = GetAdjustedCurrentTime();

        // 時間を算出.
        return ( qwTime - m_BaseTime ) * m_InvTicksPerSec;
    }

    //-------------------------------------------------------------------------
    //! @brief      経過時間を取得します.
    //!
    //! @return     経過時間を返却します.
    //-------------------------------------------------------------------------
    inline double GetElapsedTime()
    {
        // 調整された現在時間を取得.
        int64_t qwTime = GetAdjustedCurrentTime();

        // 経過時間を算出.
        double elapsedTime = ( qwTime - m_ElapsedTime ) * m_InvTicksPerSec;

        // 経過時間を更新.
        m_ElapsedTime = qwTime;

        // 0以下であればランプ.
        if ( elapsedTime < 0 )
        { elapsedTime = 0.0; }

        return elapsedTime;
    }

    //-------------------------------------------------------------------------
    //! @brief      時間の値を取得します.
    //!
    //! @param [out]    time           相対時間を格納する変数.
    //! @param [out]    absoluteTime   システム時間を格納する変数.
    //! @param [out]    elapsedTime    経過時間を格納する変数.
    //-------------------------------------------------------------------------
    inline void Update( double& time, double& absoluteTime, double& elapsedTime )
    {
        // 調整された現在時間を取得.
        int64_t qwTime = GetAdjustedCurrentTime();

        // 経過時間を取得.
        double diffTime = ( qwTime - m_ElapsedTime ) * m_InvTicksPerSec;

        // 経過時間を更新.
        m_ElapsedTime = qwTime;

        // 0以下であればクランプ.
        if ( diffTime < 0 )
        { diffTime = 0.0; }

        // システム時間.
        absoluteTime = qwTime * m_InvTicksPerSec;

        // 相対時間.
        time = ( qwTime - m_BaseTime ) * m_InvTicksPerSec;

        // 経過時間.
        elapsedTime = diffTime;
    }
};


} // namespace asdx

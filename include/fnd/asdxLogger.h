//-------------------------------------------------------------------------------------------------
// File : asdxLogger.h
// Desc : Logger Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <cstdint>


namespace asdx {

#ifndef __ASDX_WIDE
#define __ASDX_WIDE( _string )      L ## _string
#endif//__ASDX_WIDE


#ifndef ASDX_WIDE
#define ASDX_WIDE( _string )        __ASDX_WIDE( _string )
#endif//ASDX_WIDE

///////////////////////////////////////////////////////////////////////////////////////////////////
// LogLevel enum
///////////////////////////////////////////////////////////////////////////////////////////////////
enum class LogLevel : uint32_t
{
    Verbose = 0,          //!< VERBOSEレベル (白).
    Info,                 //!< INFOレベル    (緑).
    Debug,                //!< DEBUGレベル   (青).
    Warning,              //!< WRARNINGレベル(黄).
    Error,                //!< ERRORレベル   (赤).
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// ILogger interface
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ILogger
{
    //---------------------------------------------------------------------------------------------
    //! @brief      ログを出力します.
    //!
    //! @param[in]      level       ログレベルです.
    //! @param[in]      format      フォーマットです.
    //---------------------------------------------------------------------------------------------
    virtual void WriteA(LogLevel level, const char* format, ... ) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      ログを出力します.
    //!
    //! @param[in]      level       ログレベルです.
    //! @param[in]      format      フォーマットです.
    //---------------------------------------------------------------------------------------------
    virtual void WriteW(LogLevel level, const wchar_t* format, ... ) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      フィルタを設定します.
    //!
    //! @param[in]      filter      設定するフィルタ.
    //---------------------------------------------------------------------------------------------
    virtual void SetFilter(LogLevel filter ) = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      設定されているフィルタを取得します.
    //!
    //! @return     設定されているフィルタを取得します.
    //---------------------------------------------------------------------------------------------
    virtual LogLevel GetFilter() = 0;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// SystemLogger class
///////////////////////////////////////////////////////////////////////////////////////////////////
class SystemLogger : public ILogger
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      唯一のインスタンスを取得します.
    //!
    //! @return     シングルトンインスタンスを返却します.
    //---------------------------------------------------------------------------------------------
    static SystemLogger& Instance();

    //---------------------------------------------------------------------------------------------
    //! @brief      ログを出力します.
    //!
    //! @param[in]      level       ログレベルです.
    //! @param[in]      format      フォーマットです.
    //---------------------------------------------------------------------------------------------
    void WriteA(LogLevel level, const char* format, ... ) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      ログを出力します.
    //!
    //! @param[in]      level       ログレベルです.
    //! @param[in]      format      フォーマットです.
    //---------------------------------------------------------------------------------------------
    void WriteW(LogLevel level, const wchar_t* format, ... ) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      フィルタを設定します.
    //!             フィルタを設定したもののみが，ログに出力されるようになります.
    //!
    //! @param[in]      filter      設定するフィルタ.
    //---------------------------------------------------------------------------------------------
    void SetFilter(LogLevel filter ) override;

    //---------------------------------------------------------------------------------------------
    //! @brief      設定されているフィルタを取得します.
    //!
    //! @return     設定されているフィルタを取得します.
    //---------------------------------------------------------------------------------------------
    LogLevel GetFilter() override;

protected:
    //=============================================================================================
    // protected variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // protected methods.
    //=============================================================================================
    /* NOTHING */

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    static SystemLogger     s_Instance;     //!< シングルトンインスタンスです.
    LogLevel                m_Filter;       //!< フィルターです.

    //=============================================================================================
    // private methods.
    //=============================================================================================
    SystemLogger();
    SystemLogger             (const SystemLogger&) = delete;
    SystemLogger& operator = (const SystemLogger&) = delete;
};

} // namespace asdx


//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
#ifndef DLOGA
  #if defined(DEBUG) || defined(_DEBUG)
    #define DLOGA( fmt, ... )      asdx::SystemLogger::Instance().WriteA( asdx::LogLevel::Debug, "[File: %s, Line: %d] "fmt"\n", __FILE__, __LINE__, ##__VA_ARGS__ )
  #else
    #define DLOGA( fmt, ... )      ((void)0)
  #endif//defined(DEBUG) || defined(_DEBUG)
#endif//DLOGA

#ifndef DLOGW
  #if defined(DEBUG) || defined(_DEBUG)
    #define DLOGW( fmt, ... )      asdx::SystemLogger::Instance().WriteW( asdx::LogLevel::Debug, ASDX_WIDE("[File: %s, Line: %d] ") ASDX_WIDE(fmt) ASDX_WIDE("\n"), ASDX_WIDE(__FILE__), __LINE__, ##__VA_ARGS__ )
  #else
    #define DLOGW( fmt, ... )      ((void)0)
  #endif//defined(DEBUG) || defined(_DEBUG)
#endif//DLOGW


#ifndef VLOGA
#define VLOGA( fmt, ... )   asdx::SystemLogger::Instance().WriteA( asdx::LogLevel::Verbose, fmt "\n", ##__VA_ARGS__ )
#endif//VLOGA

#ifndef VLOGW
#define VLOGW( fmt, ... )   asdx::SystemLogger::Instance().WriteW( asdx::LogLevel::Verbose, ASDX_WIDE(fmt) ASDX_WIDE("\n"), ##__VA_ARGS__ )
#endif//VLOGW

#ifndef ILOGA
#define ILOGA( fmt, ... )   asdx::SystemLogger::Instance().WriteA( asdx::LogLevel::Info, fmt "\n", ##__VA_ARGS__ )
#endif//ILOGA

#ifndef ILOGW
#define ILOGW( fmt, ... )   asdx::SystemLogger::Instance().WriteW( asdx::LogLevel::Info, ASDX_WIDE(fmt) ASDX_WIDE("\n"), ##__VA_ARGS__ );
#endif//ILOGW

#ifndef WLOGA
#define WLOGA( fmt, ... )   asdx::SystemLogger::Instance().WriteA( asdx::LogLevel::Warning, fmt "\n", ##__VA_ARGS__ )
#endif//WLOGA

#ifndef WLOGW
#define WLOGW( fmt, ... )   asdx::SystemLogger::Instance().WriteW( asdx::LogLevel::Warning, ASDX_WIDE(fmt) ASDX_WIDE("\n"), ##__VA_ARGS__ )
#endif//WLOGW

#ifndef ELOGA
#define ELOGA( fmt, ... )   asdx::SystemLogger::Instance().WriteA( asdx::LogLevel::Error, "[File: %s, Line: %d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__ )
#endif//ELOGA

#ifndef ELOGW
#define ELOGW( fmt, ... )   asdx::SystemLogger::Instance().WriteW( asdx::LogLevel::Error, ASDX_WIDE("[File: %s, Line: %d] ") ASDX_WIDE(fmt) ASDX_WIDE("\n"), ASDX_WIDE(__FILE__), __LINE__, ##__VA_ARGS__ )
#endif//ELOGW

#if defined(UNICODE) || defined(_UNICODE)
    #define VLOG        VLOGW
    #define DLOG        DLOGW
    #define ILOG        ILOGW
    #define WLOG        WLOGW
    #define ELOG        ELOGW
#else
    #define VLOG        VLOGA 
    #define DLOG        DLOGA
    #define ILOG        ILOGA
    #define WLOG        WLOGA 
    #define ELOG        ELOGA
#endif


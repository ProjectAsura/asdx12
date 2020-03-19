﻿//-------------------------------------------------------------------------------------------------
// File : asdxLogger.h
// Desc : Logger Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <cstdio>
#include <cstdarg>
#include <Windows.h>
#include <asdxLogger.h>


namespace /* anonymous */ {

///////////////////////////////////////////////////////////////////////////////////////////////////
// ConsoleColor class
///////////////////////////////////////////////////////////////////////////////////////////////////
class ConsoleColor
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
    //! @brief      カラーを設定します.
    //!
    //! @param[in]      level       ログレベルです.
    //---------------------------------------------------------------------------------------------
    void Bind(asdx::LogLevel level)
    {
        auto handle = GetStdHandle( STD_OUTPUT_HANDLE );
        GetConsoleScreenBufferInfo( handle, &m_Info );

        auto attribute = m_Info.wAttributes;
        switch( level )
        {
        case asdx::LogLevel::Verbose:
            attribute = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
            break;

        case asdx::LogLevel::Info:
            attribute = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            break;

        case asdx::LogLevel::Debug:
            attribute = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            break;

        case asdx::LogLevel::Warning:
            attribute = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
            break;

        case asdx::LogLevel::Error:
            attribute = FOREGROUND_RED | FOREGROUND_INTENSITY;
            break;
        }

        SetConsoleTextAttribute( handle, attribute );
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      カラー設定を解除します.
    //---------------------------------------------------------------------------------------------
    void Unbind()
    {
        auto handle = GetStdHandle( STD_OUTPUT_HANDLE );
        SetConsoleTextAttribute( handle, m_Info.wAttributes );
    }

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    CONSOLE_SCREEN_BUFFER_INFO  m_Info;

    //=============================================================================================
    // private methods.
    //=============================================================================================
    /* NOTHING */
};

}// namespace /* anonymous */


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// SystemLogger class
///////////////////////////////////////////////////////////////////////////////////////////////////
SystemLogger SystemLogger::s_Instance;

//-------------------------------------------------------------------------------------------------
//      コンストラクタです
//-------------------------------------------------------------------------------------------------
SystemLogger::SystemLogger()
: m_Filter( LogLevel::Verbose )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      インスタンスを取得します.
//-------------------------------------------------------------------------------------------------
SystemLogger& SystemLogger::GetInstance()
{ return s_Instance; }

//-------------------------------------------------------------------------------------------------
//      ログを出力します.
//-------------------------------------------------------------------------------------------------
void SystemLogger::LogA( const LogLevel level, const char* format, ... )
{
    if ( level >= m_Filter )
    {
        ConsoleColor color;

        // カラーを設定.
        color.Bind( level );

        // ログ出力.
        {
            char msg[ 2048 ] = "\0";
            va_list arg;

            va_start( arg, format );
            vsprintf_s( msg, format, arg );
            va_end( arg );

            printf_s( "%s", msg );

            OutputDebugStringA( msg );
        }

        // カラー設定解除.
        color.Unbind();
    }
}


//-------------------------------------------------------------------------------------------------
//      ログを出力します.
//-------------------------------------------------------------------------------------------------
void SystemLogger::LogW( const LogLevel level, const wchar_t* format, ... )
{
    if ( level >= m_Filter )
    {
        ConsoleColor color;

        // カラーを設定.
        color.Bind( level );

        // ログ出力.
        {
            wchar_t msg[ 2048 ] = L"\0";
            va_list arg;

            va_start( arg, format );
            vswprintf_s( msg, format, arg );
            va_end( arg );

            wprintf_s( L"%s", msg );

            OutputDebugStringW( msg );
        }

        // カラー設定解除.
        color.Unbind();
    }
}

//-------------------------------------------------------------------------------------------------
//      フィルタを設定します.
//-------------------------------------------------------------------------------------------------
void SystemLogger::SetFilter( const LogLevel filter )
{ m_Filter = filter; }

//-------------------------------------------------------------------------------------------------
//      設定されているフィルタを取得します.
//-------------------------------------------------------------------------------------------------
LogLevel SystemLogger::GetFilter()
{ return m_Filter; }

} // namespace asdx

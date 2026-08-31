//-----------------------------------------------------------------------------
// File : asdxMisc.cpp
// Desc : Utility Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <algorithm>
#include <shlwapi.h>
#include <locale>
#include <sstream>
#include <fnd/asdxMisc.h>


namespace asdx {

//-----------------------------------------------------------------------------
//      ワイド文字列に変換します.
//-----------------------------------------------------------------------------
std::wstring ToStringW(const std::string& value)
{
    auto length = MultiByteToWideChar(CP_ACP, 0, value.c_str(), int(value.size() + 1), nullptr, 0 );
    auto buffer = new wchar_t[length];

    MultiByteToWideChar(CP_ACP, 0, value.c_str(), int(value.size() + 1),  buffer, length );

    std::wstring result(buffer);
    delete[] buffer;

    return result;
}

//-----------------------------------------------------------------------------
//      マルチバイト文字列に変換します.
//-----------------------------------------------------------------------------
std::string ToStringA(const std::wstring& value)
{
    auto length = WideCharToMultiByte(CP_ACP, 0, value.c_str(), int(value.size() + 1), nullptr, 0, nullptr, nullptr); 
    auto buffer = new char[length];
 
    WideCharToMultiByte(CP_ACP, 0, value.c_str(), int(value.size() + 1), buffer, length, nullptr, nullptr);

    std::string result(buffer);
    delete[] buffer;

    return result;
}

#if _HAS_CXX20
//-----------------------------------------------------------------------------
//      UTF-8文字列に変換します.
//-----------------------------------------------------------------------------
std::u8string ToStringUTF8(const std::wstring& src)
{
    auto const dest_size = ::WideCharToMultiByte(CP_UTF8, 0U, src.data(), -1, nullptr, 0, nullptr, nullptr);
    std::vector<char> dest(dest_size, '\0');
    if (::WideCharToMultiByte(CP_UTF8, 0U, src.data(), -1, dest.data(), int(dest.size()), nullptr, nullptr) == 0) {
        throw std::system_error{static_cast<int>(::GetLastError()), std::system_category()};
    }
    return std::u8string(dest.begin(), dest.end());
}

//-----------------------------------------------------------------------------
//      UTF-8文字列に変換します.
//-----------------------------------------------------------------------------
std::u8string ToStringUTF8(const std::string& value)
{
    auto wide = ToStringW(value);
    return ToStringUTF8(wide);
}

#else
//-----------------------------------------------------------------------------
//      UTF-8文字列に変換します.
//-----------------------------------------------------------------------------
std::string ToStringUTF8(const std::wstring& src)
{
    auto const dest_size = ::WideCharToMultiByte(CP_UTF8, 0U, src.data(), -1, nullptr, 0, nullptr, nullptr);
    std::vector<char> dest(dest_size, '\0');
    if (::WideCharToMultiByte(CP_UTF8, 0U, src.data(), -1, dest.data(), int(dest.size()), nullptr, nullptr) == 0) {
        throw std::system_error{static_cast<int>(::GetLastError()), std::system_category()};
    }
    return std::string(dest.begin(), dest.end());
}

//-----------------------------------------------------------------------------
//      UTF-8文字列に変換します.
//-----------------------------------------------------------------------------
std::string ToStringUTF8(const std::string& value)
{
    auto wide = ToStringW(value);
    return ToStringUTF8(wide);
}
#endif

//-----------------------------------------------------------------------------
//      指定文字で文字列を分割します.
//-----------------------------------------------------------------------------
std::vector<std::string> Split(const std::string& input, char delimiter)
{
    std::istringstream stream(input);

    std::string field;
    std::vector<std::string> result;
    while (std::getline(stream, field, delimiter))
    { result.push_back(field); }
    return result;
}

//-----------------------------------------------------------------------------
//      指定文字で文字列を分割します.
//-----------------------------------------------------------------------------
std::vector<std::wstring> Split(const std::wstring& input, wchar_t delimiter)
{
    std::wistringstream stream(input);

    std::wstring field;
    std::vector<std::wstring> result;
    while (std::getline(stream, field, delimiter))
    { result.push_back(field); }
    return result;
}

#if _HAS_CXX20
//-----------------------------------------------------------------------------
//      指定文字で文字列を分割します.
//-----------------------------------------------------------------------------
std::vector<std::u8string> Split(const std::u8string& input, char8_t delimiter)
{
    using u8istringstream = std::basic_istringstream<char8_t, std::char_traits<char8_t>, std::allocator<char8_t>>;
    u8istringstream stream(input);

    std::u8string field;
    std::vector<std::u8string> result;
    while (std::getline(stream, field, delimiter))
    { result.push_back(field); }
    return result;
}
#endif

//-----------------------------------------------------------------------------
//      外部プロセスを実行します.
//-----------------------------------------------------------------------------
bool RunProcess(const char* cmd, bool wait, int* retcode)
{
    STARTUPINFOA        startup_info = {};
    PROCESS_INFORMATION process_info = {};

    DWORD flag = NORMAL_PRIORITY_CLASS;
    startup_info.cb = sizeof(STARTUPINFOA);

    // 成功すると0以外, 失敗すると0が返る.
    auto ret = CreateProcessA(
        nullptr,
        const_cast<char*>(cmd), // 実害はないはず...
        nullptr,
        nullptr,
        FALSE,
        flag,
        nullptr,
        nullptr,
        &startup_info,
        &process_info);

    if (ret == 0)
    {
        //ELOGA("Error : プロセス起動に失敗. コマンド = %s", cmd);
        CloseHandle(process_info.hProcess);
        CloseHandle(process_info.hThread);
        return false;
    }

    if (wait)
    { WaitForSingleObject(process_info.hProcess, INFINITE); }

    DWORD exitCode;
    ret = GetExitCodeProcess(process_info.hProcess, &exitCode);

    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);

    if (ret == 0)
    { return false; }

    if (retcode != nullptr)
    { *retcode = int(exitCode); }

    return true;
}

//-----------------------------------------------------------------------------
//      情報ダイアログです.
//-----------------------------------------------------------------------------
void InfoDlg(const char* title, const char* msg)
{ MessageBoxA(nullptr, msg, title, MB_ICONINFORMATION | MB_OK); }

//-----------------------------------------------------------------------------
//      エラーダイアログです.
//-----------------------------------------------------------------------------
void ErrorDlg(const char* title, const char* msg )
{ MessageBoxA(nullptr, msg, title, MB_ICONERROR | MB_OK); }

//-----------------------------------------------------------------------------
//      バックスラッシュをスラッシュに変更します.
//-----------------------------------------------------------------------------
std::string ToSlash(const std::string& path)
{
    std::string ret = path;
    auto pos = ret.find('\\');
    while (pos != std::string::npos)
    {
        ret[pos] = '/';
        pos = ret.find('\\');
    }
    return ret;
}

#if _HAS_CXX20
//-----------------------------------------------------------------------------
//      バックスラッシュをスラッシュに変更します.
//-----------------------------------------------------------------------------
std::u8string ToSlash(const std::u8string& path)
{
    std::u8string ret = path;
    auto pos = ret.find(u8'\\');
    while (pos != std::u8string::npos)
    {
        ret[pos] = u8'/';
        pos = ret.find(u8'\\');
    }
    return ret;
}
#endif

//-----------------------------------------------------------------------------
//      文字列を置換します.
//-----------------------------------------------------------------------------
std::string Replace
(
    const std::string&  input,
    std::string         pattern,
    std::string         replace
)
{
    if (pattern.empty())
        return input;

    std::string result = input;
    auto pos = result.find( pattern );

    while( pos != std::string::npos )
    {
        result.replace( pos, pattern.length(), replace );
        pos = result.find( pattern, pos + replace.length() );
    }

    return result;
}

#if _HAS_CXX20
//-----------------------------------------------------------------------------
//      文字列を置換します.
//-----------------------------------------------------------------------------
std::u8string Replace
(
    const std::u8string&  input,
    std::u8string         pattern,
    std::u8string         replace
)
{
    if (pattern.empty())
        return input;

    std::u8string result = input;
    auto pos = result.find( pattern );

    while( pos != std::u8string::npos )
    {
        result.replace( pos, pattern.length(), replace );
        pos = result.find( pattern, pos + replace.length() );
    }

    return result;
}
#endif

//-----------------------------------------------------------------------------
//      小文字に変換します.
//-----------------------------------------------------------------------------
std::string ToLowerA(const std::string& value)
{
    std::string result = value;
    std::transform(result.begin(), result.end(), result.begin(), tolower);
    return result;
}

//-----------------------------------------------------------------------------
//      小文字に変換します.
//-----------------------------------------------------------------------------
std::wstring ToLowerW(const std::wstring& value)
{
    std::wstring result = value;
    std::transform(result.begin(), result.end(), result.begin(), tolower);
    return result;
}

#if _HAS_CXX20
//-----------------------------------------------------------------------------
//      小文字に変換します.
//-----------------------------------------------------------------------------
std::u8string ToLowerUTF8(const std::u8string& value)
{
    std::u8string result = value;
    std::transform(result.begin(), result.end(), result.begin(), tolower);
    return result;
}
#endif

//-----------------------------------------------------------------------------
//      環境変数を取得します.
//-----------------------------------------------------------------------------
std::string GetEnv(const char* name)
{
    size_t size;
    if (getenv_s(&size, nullptr, 0, name))
    { return ""; }

    if (size == 0)
    { return ""; }

    std::string result;
    result.resize(size + 1);
    getenv_s(&size, &result[0], size, name);
    result.resize(std::strlen(result.c_str()));
    result.shrink_to_fit();

    return result;
}

} // namespace asdx

//-----------------------------------------------------------------------------
// File : asdxMisc.h
// Desc : Utility Moudle.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdint>
#include <string>
#include <vector>


#ifdef ASDX_AUTO_LINK
//-----------------------------------------------------------------------------
// Linker
//-----------------------------------------------------------------------------
#pragma comment( lib, "shlwapi.lib" )
#endif//ASDX_AUTO_LINK


//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
#ifndef ASDX_U8
#define ASDX_U8(str)    reinterpret_cast<const char*>(u8##str)
#endif//ASDX_U8


namespace asdx {

//-----------------------------------------------------------------------------
//! @brief      static_castの省略型です.
//-----------------------------------------------------------------------------
template<typename T, typename U>
inline constexpr T scast(U&& value) noexcept
{ return static_cast<T>(std::forward<U>(value)); }

//-----------------------------------------------------------------------------
//! @brief      reinterpet_castの省略型です.
//-----------------------------------------------------------------------------
template<typename T, typename U>
inline constexpr T rcast(U&& value) noexcept
{ return reinterpret_cast<T>(std::forward<U>(value)); }

#if _HAS_CXX20
//-----------------------------------------------------------------------------
//! @brief      const char8_t型文字列をconst char型文字列にキャストします.
//-----------------------------------------------------------------------------
inline const char* ToChar(const char8_t* value) noexcept
{ return reinterpret_cast<const char*>(value); }
#endif

//-----------------------------------------------------------------------------
//! @brief      ワイド文字列に変換します.
//!
//! @param[in]      value       変換するマルチバイト文字列.
//! @return     指定された文字列をワイド文字列に変換した結果を返却します.
//-----------------------------------------------------------------------------
std::wstring ToStringW(const std::string& value);

//-----------------------------------------------------------------------------
//! @brief      マルチバイト文字列に変換します.
//!
//! @param[in]      value       変換するワイド文字列.
//! @return     指定された文字列をマルチバイト文字列に変換した結果を返却します.
//-----------------------------------------------------------------------------
std::string  ToStringA(const std::wstring& value);

#if _HAS_CXX20
//-----------------------------------------------------------------------------
//! @brief      UTF-8文字列に変換します.
//!
//! @param[in]      value       変換するワイド文字列.
//! @return     指定された文字列をUTF-8に変換した結果を返却します.
//-----------------------------------------------------------------------------
std::u8string ToStringUTF8(const std::wstring& value);

//-----------------------------------------------------------------------------
//! @brief      UTF-8文字列に変換します.
//!
//! @param[in]      value       変換するASCII文字列.
//! @return     指定された文字列をUTF-8に変換した結果を返却します.
//-----------------------------------------------------------------------------
std::u8string ToStringUTF8(const std::string& value);

#else
//-----------------------------------------------------------------------------
//! @brief      UTF-8文字列に変換します.
//!
//! @param[in]      value       変換するワイド文字列.
//! @return     指定された文字列をUTF-8に変換した結果を返却します.
//-----------------------------------------------------------------------------
std::string ToStringUTF8(const std::wstring& value);

//-----------------------------------------------------------------------------
//! @brief      UTF-8文字列に変換します.
//!
//! @param[in]      value       変換するASCII文字列.
//! @return     指定された文字列をUTF-8に変換した結果を返却します.
//-----------------------------------------------------------------------------
std::string ToStringUTF8(const std::string& value);
#endif

//-----------------------------------------------------------------------------
//! @brief      指定文字で文字列を分割します.
//!
//! @param[in]      value       入力文字列.
//! @param[in]      delimiter   分割文字.
//! @return     指定文字で分割された文字列の配列を返却します.
//-----------------------------------------------------------------------------
std::vector<std::string> Split(const std::string& value, char delimiter);

//-----------------------------------------------------------------------------
//! @brief      指定文字で文字列を分割します.
//!
//! @param[in]      value       入力文字列.
//! @param[in]      delimiter   分割文字.
//! @return     指定文字で分割された文字列の配列を返却します.
//-----------------------------------------------------------------------------
std::vector<std::wstring> Split(const std::wstring& value, wchar_t delimiter);

#if _HAS_CXX20
//-----------------------------------------------------------------------------
//! @brief      指定文字で文字列を分割します.
//!
//! @param[in]      value       入力文字列.
//! @param[in]      delimiter   分割文字.
//! @return     指定文字で分割された文字列の配列を返却します.
//-----------------------------------------------------------------------------
std::vector<std::u8string> Split(const std::u8string& value, char8_t delimiter);
#endif

//-----------------------------------------------------------------------------
//! @brief      外部プロセスを実行します.
//!
//! @param[in]      cmd         コマンドライン.
//! @param[in]      wait        待機する場合は true.
//! @param[out]     retcode     リターンコード.
//-----------------------------------------------------------------------------
bool RunProcess(const char* cmd, bool wait = true, int* retcode = nullptr);

//-----------------------------------------------------------------------------
//! @brief      情報ダイアログを出します.
//! 
//! @param[in]      title       ウィンドウタイトル名.
//! @param[in]      msg         表示メッセージ.
//-----------------------------------------------------------------------------
void InfoDlg(const char* title, const char* msg);

//-----------------------------------------------------------------------------
//! @brief      エラーダイアログを出します.
//! 
//! @param[in]      title       ウィンドウタイトル名.
//! @param[in]      msg         表示メッセージ.
//-----------------------------------------------------------------------------
void ErrorDlg(const char* title, const char* msg);

//-----------------------------------------------------------------------------
//! @brief      バックスラッシュからスラッシュに変換します.
//!
//! @param[in]      path        変換するファイルパス.
//-----------------------------------------------------------------------------
std::string ToSlash(const std::string& path);

#if _HAS_CXX20
//-----------------------------------------------------------------------------
//! @brief      バックスラッシュからスラッシュに変換します.
//!
//! @param[in]      path        変換するファイルパス.
//-----------------------------------------------------------------------------
std::u8string ToSlash(const std::u8string& path);
#endif

//-----------------------------------------------------------------------------
//! @brief      文字列を置換します.
//!
//! @param[in]      input       入力文字列.
//! @param[in]      pattern     検索パターン.
//! @param[in]      replace     置換文字列.
//-----------------------------------------------------------------------------
std::string Replace(const std::string& input, std::string pattern, std::string replace);

#if _HAS_CXX20
//-----------------------------------------------------------------------------
//! @brief      文字列を置換します.
//!
//! @param[in]      input       入力文字列.
//! @param[in]      pattern     検索パターン.
//! @param[in]      replace     置換文字列.
//-----------------------------------------------------------------------------
std::u8string Replace(const std::u8string& input, std::u8string pattern, std::u8string replace);
#endif

//-----------------------------------------------------------------------------
//! @breif      小文字に変換します.
//!
//! @param[in]      value       変換する文字列.
//-----------------------------------------------------------------------------
std::string ToLowerA(const std::string& value);

//-----------------------------------------------------------------------------
//! @breif      小文字に変換します.
//!
//! @param[in]      value       変換する文字列.
//-----------------------------------------------------------------------------
std::wstring ToLowerW(const std::wstring& value);

#if _HAS_CXX20
//-----------------------------------------------------------------------------
//! @breif      小文字に変換します.
//!
//! @param[in]      value       変換する文字列.
//-----------------------------------------------------------------------------
std::u8string ToLowerUTF8(const std::u8string& value);
#endif

//-----------------------------------------------------------------------------
//! @brief      環境変数を取得します.
//!
//! @param[in]      name        環境変数名.
//-----------------------------------------------------------------------------
std::string GetEnv(const char* name);

//-----------------------------------------------------------------------------
//! @brief      二分検索を行います.
//! 
//! @param[in]      items       検索対象の配列.
//! @param[in]      count       配列の数.
//! @param[in]      key         検索キー.
//! @param[in]      comp        比較関数です.
//! @return     検索にヒットした配列番号を返却します.
//!             検索にヒットしない場合はSIZE_MAXを返却します.
//! @note       検索データは事前にソートされている必要があります.
//-----------------------------------------------------------------------------
template<typename T, typename Compare, typename index_t = size_t>
inline index_t BinarySearch(const T items[], index_t count, const T& key, Compare comp)
{
    index_t lhs = 0;
    index_t rhs = count;

    while(lhs < rhs)
    {
        index_t mid = lhs + (rhs - lhs) / 2;
        int ret = comp(items[mid], key);
        if (ret == 0)
            return mid;
        else if (ret < key)
            lhs = mid + 1;
        else
            rhs = mid;
    }

    return SIZE_MAX;
}

//-----------------------------------------------------------------------------
//! @brief      二分検索を行います.
//! 
//! @param[in]      items       検索対象の配列.
//! @param[in]      count       配列の数.
//! @param[in]      key         検索キー.
//! @return     検索にヒットした配列番号を返却します.
//!             検索にヒットしない場合はSIZE_MAXを返却します.
//! @note       検索データは事前にソートされている必要があります.
//-----------------------------------------------------------------------------
template<typename T, typename index_t = size_t>
inline index_t BinarySearch(const T items[], index_t count, const T& key)
{
    return BinarySearch(items, count, key,
        [](const T& lhs, const T& rhs)
        { return lhs == rhs; });
}

//-----------------------------------------------------------------------------
//! @brief      指定された値より大きい値が現れる最初のインデックスを求めます.
//! 
//! @param[in]      items       検索対象の配列.
//! @param[in]      count       配列の数.
//! @param[in]      key         検索キー.
//! @return     検索にヒットした配列番号を返却します.
//! @note       検索データは事前にソートされている必要があります.
//-----------------------------------------------------------------------------
template<typename T, typename U, typename Compare, typename index_t = size_t>
inline index_t LowerBound(const T items[], index_t count, const U& key, Compare comp)
{
    index_t lhs = 0;
    index_t rhs = count;

    while(lhs < rhs)
    {
        index_t mid = lhs + (rhs - lhs) / 2;
        if (comp(items[mid], key))
            lhs = mid + 1;
        else
            rhs = mid;
    }

    return lhs;
}

//-----------------------------------------------------------------------------
//! @brief      指定された値より小さい値が現れる最初のインデックスを求めます.
//! 
//! @param[in]      items       検索対象の配列.
//! @param[in]      count       配列の数.
//! @param[in]      key         検索キー.
//! @return     検索にヒットした配列番号を返却します.
//! @note       検索データは事前にソートされている必要があります.
//-----------------------------------------------------------------------------
template<typename T, typename U, typename Compare, typename index_t = size_t>
inline index_t UpperBound(const T items[], index_t count, const U& key, Compare comp)
{
    index_t lhs = 0;
    index_t rhs = count;

    while(lhs < rhs)
    {
        index_t mid = lhs + (rhs - lhs) / 2;
        if (comp(items[mid], key))
            rhs = mid;
        else
            lhs = mid + 1;
    }

    return lhs;
}

} // namespacec asdx

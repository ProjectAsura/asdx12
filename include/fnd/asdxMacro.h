//-----------------------------------------------------------------------------
// File : asdxMacro.h
// Desc : Macro
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

#if defined(DEBUG) || defined(_DEBUG)
    #define ASDX_DEBUG      (1)
#else
    #define ASDX_RELEASE    (1)
#endif

#ifndef ASDX_UNUSED
#define ASDX_UNUSED(x)  (void)x
#endif//ASDX_UNUSED

#ifndef ASDX_DEBUG_CODE
    #ifdef ASDX_DEBUG
        #define ASDX_DEBUG_CODE(x)  x
    #else
        #define ASDX_DEBUG_CODE(x)
    #endif
#endif

#ifndef ASDX_COUNT_OF
#define ASDX_COUNT_OF(arr) (sizeof(arr) / sizeof(arr[0]))
#endif//ASDX_COUNT_OF

#ifndef ASDX_DEV_VAR
    #ifndef ASDX_RELEASE
        #define ASDX_DEV_VAR(develop, release) develop
    #else
        #define ASDX_DEV_VAR(develop, release) release
    #endif
#endif

#ifndef ASDX_LIKELY
#define ASDX_LIKELY(x) (!!(x)) [[likely]]
#endif//ASDX_LIKELY

#ifndef ASDX_UNLIKELY
#define ASDX_UNLIKELY(x) (!!(x)) [[unlikely]]
#endif//ASDX_UNLIKELY

#ifndef ASDX_CONCAT_
#define ASDX_CONCAT_(a, b) a##b
#endif//ASDX_CONCAT_

#ifndef ASDX_CONCAT
#define ASDX_CONCAT(a, b) ASDX_CONCAT_(a, b)
#endif//ASDX_CONCAT

#ifdef __COUNTER__
#define ASDX_UNIQUE_NAME(base) ASDX_CONCAT(base, __COUNTER__)
#else
#define ASDX_UNIQUE_NAME(base) ASDX_CONCAT(base, __LINE__)
#endif

namespace asdx {
template<size_t Size>
struct Padding
{
private:
    static_assert(Size > 0, "Padding size must be > 0");
    unsigned char unused_[Size] = {};
};

template <>
struct Padding<0> {};

} // namespace asdx

#ifndef ASDX_PADDING
#define ASDX_PADDING(size)     asdx::Padding<size> ASDX_UNIQUE_NAME(padding_)
#endif//ASDX_PADDING

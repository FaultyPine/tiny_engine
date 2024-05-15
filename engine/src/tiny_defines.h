#ifndef TINY_DEFINES_H
#define TINY_DEFINES_H

#include "basics.type.h"

#define TINY_DEBUG

#define _CRT_SECURE_NO_WARNINGS

// assertions only in debug mode
#ifdef TINY_DEBUG
#define TINY_ASSERTIONS_ENABLED
#endif

#if defined(__clang__) || defined(__GNUC__)
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT static_assert
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) ( sizeof((arr))/sizeof((arr)[0]) )
#endif

#define SET_NTH_BIT(bitfield, n_bit, onoff) \
    (bitfield = (bitfield & ~((u32)1 << n_bit)) | ((u32)onoff << n_bit) )

#define TOGGLE_NTH_BIT(bitfield, n_bit) \
    ( (bitfield) ^ (1 << (n_bit)) )

#define CHECK_NTH_BIT(bitfield, n_bit) ((( bitfield >> n_bit ) & 1U) == 1)

// concat tokens without expanding macro definitions
#define TMACRO_CONCAT(a,b) a##b
// concat tokens after macro expanding them
#define TMACRO_CONCAT_EX(a,b) TMACRO_CONCAT(a,b)

// stringize token without macro expanding A
#define TMACRO_STRINGIZE(A) #A
// stringize token after macro expanding A
#define TMACRO_STRINGIZE_EX(A) TMACRO_STRINGIZE(A)

#define U32_INVALID_ID 999999999U

// exports
#ifdef TEXPORT
#ifdef _MSC_VER
#define TAPI __declspec(dllexport)
#else
#define TAPI __attribute__((visibility("default")))
#endif
// imports
#else 
#ifdef _MSC_VER
#define TAPI __declspec(dllimport)
#else
#define TAPI
#endif
#endif

#ifdef _MSC_VER
#define TALIGN(n) __declspec(align(n))
#else
#define TALIGN(n) __attribute__((aligned(n)))
#endif

#ifdef _MSC_VER
#define TINLINE __forceinline
#define TNOINLINE __declspec(noinline)
#else
#define TINLINE static inline
#define TNOINLINE
#endif

#endif
#ifndef TINY_DEFINES_H
#define TINY_DEFINES_H

#include "basics.type.h"

#define TINY_DEBUG

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
    ( (onoff) ? ( (bitfield) |= ((onoff) << (n_bit)) ) : ( (bitfield) &= ~((onoff) << (n_bit)) ) )

#define TOGGLE_NTH_BIT(bitfield, n_bit) \
    ( (bitfield) ^ (1 << (n_bit)) )


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



#endif
#ifndef TINY_DEFINES_H
#define TINY_DEFINES_H

#include "types/generated/basics.type.h"

#define TINY_DEBUG

// assertions only in debug mode
#ifdef TINY_DEBUG
#define TINY_ASSERTIONS_ENABLED
#endif

#define ARRAY_SIZE(arr) ( sizeof((arr))/sizeof((arr)[0]) )

#define SET_NTH_BIT(bitfield, n_bit, onoff) \
    ( (onoff) ? ( (bitfield) |= ((onoff) << (n_bit)) ) : ( (bitfield) &= ~((onoff) << (n_bit)) ) )

#define TOGGLE_NTH_BIT(bitfield, n_bit) \
    ( (bitfield) ^ (1 << (n_bit)) )


#define TAPI __declspec(dllexport)





#endif
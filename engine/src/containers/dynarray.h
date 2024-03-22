#ifndef TINY_DYNARRAY_H
#define TINY_DYNARRAY_H

#include "tiny_defines.h"
#include "tiny_log.h"

// "stretchy buffer" implementation
// dynamic array that resizes itself when capacity is reached
// stores capacity/size in a header section stored *before* the actual array pointer

typedef void* DynArray;
constexpr static u32 INITIAL_CAPACITY = 5;

typedef void* (*DynArrayAllocFunc)(size_t size);
typedef void (*DynArrayFreeFunc)(void* data);

// Create an array with an optional initial capacity (number of elements)
DynArray __DynArrayCreate(u32 stride, u32 initialCapacity);
template<typename T>
T* DynArrayCreate(u32 initialCapacity = INITIAL_CAPACITY, DynArrayAllocFunc allocFunc = nullptr, DynArrayFreeFunc freeFunc = nullptr)
{
    return (T*)__DynArrayCreate(sizeof(T), initialCapacity, allocFunc, freeFunc);
}
// Frees backing memory
void DynArrayDestroy(DynArray& array);

// Retrives the size from the DynArray header
u32 DynArrayGetSize(DynArray array);
// Retrives the capacity from the DynArray header
u32 DynArrayGetCapacity(DynArray array);
// Retrives the stride from the DynArray header
u32 DynArrayGetStride(DynArray array);

template <typename T>
inline T& DynArrayGet(DynArray array, u32 index)
{
    return ((T*)array)[index];
}

void* __DynArrayPushAt(DynArray array, void* obj, u32 index);
// Copies an object to a specified index (and moves all other elements over)
// passing reference as this could potentially reallocate if backing mem is full
// pushing to an index outside the range [0,length] returns nullptr, logs an error, and does nothing
template <typename T>
void DynArrayPushAt(T*& array, T obj, u32 index)
{
    array = (T*)__DynArrayPushAt(array, &obj, index);
}
inline void* __DynArrayPush(DynArray array, void* obj);
// Copies an object to the end of the array
template <typename T>
void DynArrayPush(T*& array, T obj)
{
    array = (T*)__DynArrayPush((DynArray*)array, (void*)&obj);
}

void __DynArrayPopAt(DynArray array, u32 index, void* out = 0);
// remove (and optionally return element) at specified index
// popping at an index outside the range [0,length-1] does nothing and logs an error
template <typename T>
inline void DynArrayPopAt(DynArray array, u32 index, T& out)
{
    __DynArrayPopAt(array, index, &out);
}

// remove (and optionally return) the last element
inline void __DynArrayPop(DynArray array, void* out = 0);
template <typename T>
inline void DynArrayPop(DynArray array, T& out)
{
    __DynArrayPop(array, &out);
}

// sets array size to 0, does not free backing memory
void DynArrayClear(DynArray array);


#define DynArrayForEach(arr, idx) \
    int idx = 0; idx < DynArrayGetSize(arr); i++

#endif


void DynArrayTests();
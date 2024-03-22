#include "dynarray.h"


#include "tiny_log.h"
#include "mem/tiny_arena.h"

#define ARRAY_CHECKS 1


struct DynArrayHeader
{
    // number of elements currently in the array
    u32 size;
    // number of *elements* we can hold in our backing memory
    u32 capacity;
    // size in bytes of each element
    u32 stride;
    // allocation function
    DynArrayAllocFunc allocFunc;
    DynArrayFreeFunc freeFunc;
};

inline DynArrayHeader* GetHeaderPointer(DynArray array)
{
    u32 headerSize = sizeof(DynArrayHeader);
    // our header will always be 'behind' our array pointer.
    DynArrayHeader* headerPtr = (DynArrayHeader*)(((u8*)array) - headerSize);
    return headerPtr;
}

// ===== Create & Destroy =====

void* DynArrayInternalAlloc(DynArrayAllocFunc allocFunc, size_t size)
{
    if (allocFunc == nullptr)
    {
        return TSYSALLOC(size);
    }
    else
    {
        return allocFunc(size);
    }
}

void DynArrayInternalFree(DynArrayFreeFunc freeFunc, void* data)
{
    if (freeFunc == nullptr)
    {
        TSYSFREE(data);
    }
    else
    {
        freeFunc(data);
    }
}

DynArray __DynArrayCreate(u32 stride, u32 initialCapacity, DynArrayAllocFunc allocFunc, DynArrayFreeFunc freeFunc)
{
    u32 headerSize = sizeof(DynArrayHeader);
    u32 arraySize = initialCapacity * stride;
    u32 allocSize = headerSize + arraySize;
    // TODO: allow custom allocator
    u8* arrayBackingMem = (u8*)DynArrayInternalAlloc(allocFunc, allocSize);
    TMEMSET(arrayBackingMem, 0, allocSize);
    // populate header
    DynArrayHeader* headerPointer = (DynArrayHeader*)arrayBackingMem;
    headerPointer->size = 0;
    headerPointer->capacity = initialCapacity;
    headerPointer->stride = stride;
    headerPointer->allocFunc = allocFunc;
    headerPointer->freeFunc = freeFunc;
    // our DynArray is a pointer to our array elements, and metadata about the array
    // is stored just before that pointer
    DynArray result = arrayBackingMem + headerSize; 
    return result;
}

void DynArrayDestroy(DynArray& array)
{
    // since header info is stored before the array pointer, move back to the beginning of the allocation to free it
    DynArrayHeader* baseArrayPtr = GetHeaderPointer(array);
    DynArrayInternalFree(baseArrayPtr->freeFunc, baseArrayPtr);
    array = (void*)0;
}

constexpr static u32 GROWTH_FACTOR = 2;
DynArray DynArrayResize(DynArray array)
{
    DynArrayHeader* header = GetHeaderPointer(array);
#if ARRAY_CHECKS
    TINY_ASSERT(header->capacity != 0 && "resize called on array with 0 capacity");
#endif
    u32 newCapacity = header->size * GROWTH_FACTOR;
    DynArray newArray = __DynArrayCreate(header->stride, newCapacity, header->allocFunc, header->freeFunc);
    DynArrayHeader* newArrayBasePtr = GetHeaderPointer(newArray);
    u32 totalOldArraySize = (header->size * header->stride) + sizeof(DynArrayHeader);
    TMEMCPY(newArrayBasePtr, header, totalOldArraySize);
    newArrayBasePtr->capacity = newCapacity;
    DynArrayDestroy(array);
    return newArray;
}

// ===== Modify array ======

void* __DynArrayPushAt(DynArray array, void* obj, u32 index)
{
    DynArrayHeader* header = GetHeaderPointer(array);
#if ARRAY_CHECKS
    if (index > header->size)
    {
        LOG_FATAL("Tried to push element outside the bounds of a dynarray. index = %u  arr size = %u", index, header->size);
        return nullptr;
    }
#endif
    if (header->size >= header->capacity)
    {
        array = DynArrayResize(array);
        header = GetHeaderPointer(array);
    }
    u32 arrSize =header->size;
    u32 stride = header->stride;
    u8* arrayMem = (u8*)array;
    // if not on last element, copy all elements 1 to the right
    if (index < arrSize-1)
    {
        u32 moveSize = (arrSize - index) * stride;
        u8* moveTo   = arrayMem + ((index+1) * stride);
        u8* moveFrom = arrayMem + ((index+0) * stride);
        TMEMMOVE(moveTo, moveFrom, moveSize);
    }
    // copy object to the index
    TMEMCPY(arrayMem + (index * stride), obj, stride);
    header->size++;
    return array;
}



void* __DynArrayPush(DynArray array, void* obj)
{
    return __DynArrayPushAt(array, obj, DynArrayGetSize(array));
}

void __DynArrayPopAt(DynArray array, u32 index, void* out)
{
    DynArrayHeader* header = GetHeaderPointer(array);
    u32 arrSize = header->size;
    u32 capacity = header->capacity;
    u32 stride = header->stride;
#if ARRAY_CHECKS
    if (index >= arrSize)
    {
        LOG_FATAL("Tried to pop at invalid dynarray index. index = %u  arr size = %u", index, arrSize);
        return;
    }
#endif
    u8* arrayMem = (u8*)array;
    if (out)
    {
        TMEMCPY(out, arrayMem + (index * stride), stride);
    }
    // if not last element, copy everything to the right of it 1 spot to the left
    if (index != arrSize-1)
    {
        u32 moveSize = (arrSize - index) * stride;
        u8* moveTo   = arrayMem + ((index+0) * stride);
        u8* moveFrom = arrayMem + ((index+1) * stride);
        TMEMMOVE(moveTo, moveFrom, moveSize);
    }
    header->size--;
}

void __DynArrayPop(DynArray array, void* out)
{
    return __DynArrayPopAt(array, DynArrayGetSize(array)-1, out);
}

void DynArrayClear(DynArray array)
{
    DynArrayHeader* header = GetHeaderPointer(array);
    header->size = 0;
}

// ===== Get header info ======

u32 DynArrayGetSize(DynArray array)
{
    DynArrayHeader* headerPtr = GetHeaderPointer(array);
    return headerPtr->size;
}

u32 DynArrayGetCapacity(DynArray array)
{
    DynArrayHeader* headerPtr = GetHeaderPointer(array);
    return headerPtr->capacity;
}

u32 DynArrayGetStride(DynArray array)
{
    DynArrayHeader* headerPtr = GetHeaderPointer(array);
    return headerPtr->stride;
}


void DynArrayTests()
{
    LOG_INFO("Testing DynArray...");
    s32* arr = DynArrayCreate<s32>();
    s32 x = 1;
    DynArrayPush(arr, x);
    TINY_ASSERT(DynArrayGetSize(arr) == 1);
    TINY_ASSERT(arr[0] == 1);
    DynArrayPush(arr, -1);
    DynArrayPush(arr, 2);
    TINY_ASSERT(DynArrayGetSize(arr) == 3);
        
    s32 expected[3] = {1,-1,2};
    for (int i = 0; i < 3; i++)
    {
        TINY_ASSERT(expected[i] == arr[i]);
    }
    DynArrayPushAt(arr, 90, 1);
    s32 expected_2[4] = {1,90,-1,2};
    for (int i = 0; i < 4; i++)
    {
        TINY_ASSERT(expected_2[i] == arr[i]);
        LOG_INFO("%i",arr[i]);
    }
    TINY_ASSERT(DynArrayGetCapacity(arr) == 5);
    TINY_ASSERT(DynArrayGetStride(arr) == sizeof(s32));

    for (int i = 0; i < 20; i++)
    {
        DynArrayPush(arr, i);
    }

    TINY_ASSERT(DynArrayGetSize(arr) == 24);
    TINY_ASSERT(DynArrayGetCapacity(arr) == 40);

    s32 lastElem;
    DynArrayPop(arr, lastElem);
    TINY_ASSERT(DynArrayGetSize(arr) == 23);
    TINY_ASSERT(lastElem == 19);
    s32 shouldBeZero;
    DynArrayPopAt(arr, 4, shouldBeZero);
    TINY_ASSERT(shouldBeZero == 0);
    TINY_ASSERT(DynArrayGetSize(arr) == 22);

    DynArrayClear(arr);
    TINY_ASSERT(DynArrayGetSize(arr) == 0);

    s32 shouldntChange = 0xDEADBEEF;
    LOG_INFO("Expecting two fatal errors here:");
    DynArrayPop(arr, shouldntChange);
    DynArrayPopAt(arr, 0, shouldntChange);
    TINY_ASSERT(shouldntChange == 0xDEADBEEF);

    LOG_INFO("DynArray Tests complete");
}


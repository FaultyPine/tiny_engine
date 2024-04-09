//#include "fixed_growable_array.h"

#include "tiny_log.h"
#include "mem/tiny_mem.h"

constexpr u32 GROWTH_FACTOR = 2;
template <typename T, u32 fixedSize>
static void CheckArrayResize(FixedGrowableArray<T, fixedSize>& array)
{
    TINY_ASSERT(array.size <= array.capacity);
    if (array.size >= array.capacity)
    {
        array.capacity = array.size * GROWTH_FACTOR;
        // moving old buffer (could be our fixed mem or a dyn alloc) to a new allocation
        T* prevAlloc = array.elements == &array.fixedMem[0] ? nullptr : array.elements;
        T* prevElements = array.elements;
        array.elements = (T*)TSYSALLOC(sizeof(T) * array.capacity);
        TMEMMOVE(array.elements, prevElements, sizeof(T) * array.size);
        if (prevElements != &array.fixedMem[0])
        {
            TSYSFREE(prevElements);
        }
        // move elements from prev buffer to new one
        // notably... this will invalidate pointers to these elements. This is fine, use indices instead
    }
}

template <typename T, u32 fixedSize>
FixedGrowableArray<T, fixedSize>::FixedGrowableArray()
{
    elements = &fixedMem[0];
    capacity = fixedSize;
    size = 0;
}

template <typename T, u32 fixedSize>
void FixedGrowableArray<T, fixedSize>::push_back(const T& element)
{
    CheckArrayResize(*this);
    DynArrayGet<T>(elements, size) = element;
    size++;
}

template <typename T, u32 fixedSize>
void FixedGrowableArray<T, fixedSize>::insert(const T& element, u32 index)
{
    CheckArrayResize(*this);
    if (index >= size)
    {
        LOG_ERROR("Attempted to insert into FixedGrowableArray at invalid index");
        return;
    }
    if (index == size)
    {
        DynArrayGet<T>(elements, size) = element;
    }
    else
    {
        u32 elementsToMove = size - index;
        TMEMMOVE(&DynArrayGet<T>(elements, index + 1), &DynArrayGet<T>(elements, index), sizeof(T) * elementsToMove);
        DynArrayGet<T>(elements, index) = element;
    }
    size++;
}

template <typename T, u32 fixedSize>
T FixedGrowableArray<T, fixedSize>::erase_and_fill(u32 index)
{
    if (index >= size)
    {
        LOG_ERROR("attempted to erase invalid index");
        return {};
    }
    T tmp = DynArrayGet<T>(elements, index); // copy
    // swap element to erase with last element
    DynArrayGet<T>(elements, index) = DynArrayGet<T>(elements, size-1);
    size--;
    return tmp;
}

template <typename T, u32 fixedSize>
T FixedGrowableArray<T, fixedSize>::erase(u32 index)
{
    if (index >= size)
    {
        LOG_ERROR("attempted to erase invalid index");
        return {};
    }
    T tmp = DynArrayGet<T>(elements, index); // copy
    // move everything from the right of this index to the left
    size_t moveSize = sizeof(T) * (size-index);
    TMEMMOVE(&DynArrayGet<T>(elements, index), &DynArrayGet<T>(elements, index+1), moveSize); 
    size--;
    return tmp;
}

template <typename T, u32 fixedSize>
T& FixedGrowableArray<T, fixedSize>::at(u32 index)
{
    TINY_ASSERT(index < size);
    return DynArrayGet<T>(elements, index);
}
template <typename T, u32 fixedSize>
const T& FixedGrowableArray<T, fixedSize>::at(u32 index) const
{
    TINY_ASSERT(index < size);
    return DynArrayGet<T>(elements, index);
}

template <typename T, u32 fixedSize>
void FixedGrowableArray<T, fixedSize>::clear()
{
    size = 0;
}


void FixedGrowableArrayTests()
{
    LOG_INFO("Running FixedGrowableArray tests...");
    constexpr u32 testFixedSize = 10;
    FixedGrowableArray<u32, testFixedSize> arr = {};
    TINY_ASSERT(arr.size == 0);
    arr.push_back(0);
    arr.push_back(1);
    arr.push_back(2);
    arr.push_back(3);
    TINY_ASSERT(arr.size == 4);
    TINY_ASSERT(arr.elements == &arr.fixedMem[0]);
    while (arr.size < testFixedSize)
    {
        arr.push_back(arr.size);
    }
    for(u32 i = 0; i < arr.size; i++)
    {
        LOG_INFO("arr[%i] = %i", i, arr.at(i));
    }
    LOG_INFO("Filled array");
    TINY_ASSERT(arr.elements == &arr.fixedMem[0]);
    arr.push_back(testFixedSize); // this should trigger a reallocation of the entire memory
    TINY_ASSERT(arr.elements != &arr.fixedMem[0]);
    TINY_ASSERT(arr.erase(0) == 0);
    LOG_INFO("Removed first element. New first element: %i", arr.at(0));
    TINY_ASSERT(arr.at(0) == 1);
    TINY_ASSERT(arr.size == testFixedSize);
    TINY_ASSERT(arr.at(arr.size-1) == testFixedSize);
    TINY_ASSERT(arr.erase_and_fill(0) == 1); // removes "1" and puts the last element "testFixedSize" in its place
    TINY_ASSERT(arr.at(0) == testFixedSize); // first element should now be what was the last element
    arr.insert(99, 1);
    TINY_ASSERT(arr.at(0) == testFixedSize); // should still be the case...
    TINY_ASSERT(arr.at(1) == 99);
    TINY_ASSERT(arr.at(2) == 2);
    TINY_ASSERT(arr.at(arr.size-1) == 9);
    arr.clear();
    TINY_ASSERT(arr.size == 0);
    //arr.insert(999, 3); // should report an error
    LOG_INFO("FixedGrowableArray tests successful!");
}
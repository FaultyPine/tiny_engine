#pragma once

#include "tiny_defines.h"
#include "dynarray.h"

// array that stores a fixed-size number of elements in place
// if we go over the fixed size of this array, we dynamically allocate
// more space for extra elements in addition to the fixed size buffer
// extra elements will always be allocated with the system allocator right now. 
// The extra elements are meant as a fallback/backup allocation strat - for that reason
// I won't be allowing those to be allocated with an Arena or things like that

// TODO: when we dip below the fixedSize limit, switch back to using fixed mem
// as of rn, as soon as we transition to dyn mem, we never switch back

template <typename T, u32 fixedSize>
struct FixedGrowableArray
{
    TAPI FixedGrowableArray();
    TAPI FixedGrowableArray(const FixedGrowableArray&& arr)
    {
        size = arr.size;
        capacity = arr.capacity;
        elements = arr.elements;
        if (arr.elements == &arr.fixedMem[0])
        { // don't need to copy fixedmem over if we've switched to dynamic mem
            TMEMCPY(fixedMem, arr.fixedMem, sizeof(T) * size);
        }
    }
    TAPI FixedGrowableArray& operator=(const FixedGrowableArray& arr)
    {
        size = arr.size;
        capacity = arr.capacity;
        elements = arr.elements;
        if (arr.elements == &arr.fixedMem[0])
        { // don't need to copy fixedmem over if we've switched to dynamic mem
            TMEMCPY(fixedMem, arr.fixedMem, sizeof(T) * size);
        }
        return *this;
    }
    TAPI ~FixedGrowableArray()
    {
        if (elements != &fixedMem[0])
        {
            TSYSFREE(elements);
        }
    }

    // adds element to end of array
    TAPI void push_back(const T& element);
    // inserts element at specified index - pushes elements to the right. Clamps index to within bounds
    TAPI void insert(const T& element, u32 index);
    // zeroes out specified element (*DOES NOT CALL DTOR/DELETE*) and places the rightmost element in it's place
    TAPI T erase_and_fill(u32 index);
    // zeroes out specified element (*DOES NOT CALL DTOR/DELETE*) and moves elements to the right of it to fill the gap
    TAPI T erase(u32 index);

    // returns element at index
    TAPI T& at(u32 index);
    // sets size to 0 - does not zero out internal memory or do any deallocation
    TAPI void clear();
 
    TAPI inline T* get_elements() { return elements; }

    // this points to the current array of elements.
    // when size < fixedSize, elements points to fixedMem.
    // when size >= fixedSize elements points to a heap-allocated array
    T* elements = nullptr;
    T fixedMem[fixedSize] = {};
    // both in terms of number of elements
    u32 size = 0;
    u32 capacity = 0;
};

TAPI void FixedGrowableArrayTests();

#include "fixed_growable_array.tcc"
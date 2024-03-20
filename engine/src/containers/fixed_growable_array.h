#pragma once

#include "tiny_defines.h"
#include "dynarray.h"

// array that stores a fixed-size number of elements in place
// if we go over the fixed size of this array, we dynamically allocate
// more space for extra elements in addition to the fixed size buffer
// extra elements will always be allocated with the system allocator right now. 
// The extra elements are meant as a fallback/backup allocation strat - for that reason
// I won't be allowing those to be allocated with an Arena or things like that

template <typename T, u32 fixedSize>
struct FixedGrowableArray
{
    FixedGrowableArray();

    // adds element to end of array
    void push_back(const T& element);
    // inserts element at specified index - pushes elements to the right. Clamps index to within bounds
    void insert(const T& element, u32 index);
    // zeroes out specified element (*DOES NOT CALL DTOR/DELETE*) and places the rightmost element in it's place
    T erase_and_fill(u32 index);
    // zeroes out specified element (*DOES NOT CALL DTOR/DELETE*) and moves elements to the right of it to fill the gap
    T erase(u32 index);

    // returns element at index
    T& at(u32 index);
    // sets size to 0 - does not zero out internal memory or do any deallocation
    void clear();

    // this points to the current array of elements.
    // when size < fixedSize, elements points to fixedMem.
    // when size >= fixedSize elements points to a heap-allocated array
    DynArray elements = nullptr;
    T fixedMem[fixedSize] = {};
    u32 size = 0;
};

TAPI void FixedGrowableArrayTests();
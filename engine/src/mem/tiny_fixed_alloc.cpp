#include "tiny_fixed_alloc.h"

#include "tiny_log.h"

// uses a backing memory buffer and allocates fixed-size chunks from it at a time
// freeing blocks adds them to a linked list of free blocks which can be reclaimed on 
// subsequent allocations

FixedBlockAllocator InitializeFixedBlockAllocator(u8* backingMem, size_t memSize, size_t blockSize)
{
    TINY_ASSERT(blockSize >= sizeof(size_t)); // we use size_t bytes to store pointers to the next free blocks
    FixedBlockAllocator alloc;
    alloc.mem = backingMem;
    alloc.memSize = memSize;
    alloc.offset = 0;
    alloc.blockSize = blockSize;
    alloc.freeListStart = 0;
    alloc.freeListEnd = 0;
    return alloc;
}

void* fixedblock_alloc(FixedBlockAllocator* allocator)
{
    size_t& offset = allocator->offset;
    bool full = offset + allocator->blockSize > allocator->memSize;
    if (full && allocator->freeListStart != 0)
    {
        // grab freelist block from the start of the chain
        u8* freeBlock = allocator->freeListStart;
        u8* nextFreeBlock = (u8*) *(size_t*)allocator->freeListStart;
        if (nextFreeBlock != 0)
        {
            // each free list block holds a pointer to the next free block in the first size_t bytes
            allocator->freeListStart = nextFreeBlock;
            return freeBlock;
        }
        else
        {
            TINY_ASSERT(false && "Out of memory in fixed block allocator! (attempted to grab freelist block but failed)");
        }
    }
    else
    {
        TINY_ASSERT(false && "Out of memory in fixed block allocator!");
    }
    void* newAlloc = allocator->mem + offset;
    offset += allocator->blockSize;
    return newAlloc;
}

void fixedblock_free_block(FixedBlockAllocator* allocator, void* block)
{
    // block should be within our allocator mem
    TINY_ASSERT(block >= allocator->mem && block <= allocator->mem + allocator->memSize);
    // make sure block is aligned on blockSize
    TINY_ASSERT( ((size_t)block - (size_t)allocator->mem) % allocator->blockSize == 0 );
    if (allocator->freeListStart == 0)
    {
        allocator->freeListStart = (u8*)block;
        allocator->freeListEnd = (u8*)block;
        // storing pointer to next freelist block - in the form of an offset from the mem base address
        // in first few bytes of the freed block
        *(size_t*)block = 0;
    }
    else
    {
        // write the pointer to the next free block to the previous free block in the chain
        *(size_t*)allocator->freeListEnd = (size_t)block;
        // advance freelist end pointer to (this) recently freed block
        u8* nextFreeBlock = (u8*)block;
        allocator->freeListEnd = nextFreeBlock;
    }
}

void fixedblock_clear(FixedBlockAllocator* allocator)
{
    allocator->freeListStart = 0;
    allocator->offset = 0;
}
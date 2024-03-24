#ifndef TINY_MEM_H
#define TINY_MEM_H

#include <string.h>

#define TSYSALLOC(size) malloc(size)
#define TSYSREALLOC(ptr, size) realloc(ptr, size)
#define TSYSFREE(ptr) { free(ptr); (ptr)=0; }
#define TMEMSET(ptr, val, size) memset(ptr, val, size)
#define TMEMCPY(dst, src, size) memcpy(dst, src, size)
#define TMEMMOVE(dst, src, size) memmove(dst, src, size)

#define KILOBYTES_BYTES(kb) (kb*1024)
#define MEGABYTES_BYTES(mb) (mb*KILOBYTES_BYTES(1024))
#define GIGABYTES_BYTES(gb) (gb*MEGABYTES_BYTES(1024))

// non-owning
template <typename T>
struct BufferView
{
    T* data = nullptr;
    size_t size = 0;
};


#endif
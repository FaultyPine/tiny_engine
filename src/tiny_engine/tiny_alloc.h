#pragma once

//#include "pch.h"
#include <string.h>

#define TALLOC(size) malloc(size)
#define TFREE(ptr) { free(ptr); (ptr)=0; }
#define TMEMSET(ptr, val, size) memset(ptr, val, size)
#define TMEMCPY(dst, src, size) memcpy(dst, src, size)
#define TMEMMOVE(dst, src, size) memmove(dst, src, size)

#define KILOBYTES_BYTES(kb) (kb*1024)
#define MEGABYTES_BYTES(mb) (mb*KILOBYTES_BYTES(1024))
#define GIGABYTES_BYTES(gb) (gb*MEGABYTES_BYTES(1024))

typedef void*(*AllocFunction)(size_t size);

// ARENAS

struct Arena {
    unsigned char* backing_mem = 0;
    size_t backing_mem_size = 0;
    size_t offset = 0;
    size_t prev_offset = 0;
};


Arena arena_init(void* backing_buffer, size_t arena_size);
Arena arena_init(void* backing_buffer, size_t arena_size, const char* name);
void* arena_alloc(Arena* arena, size_t alloc_size);
void* arena_resize(Arena* arena, void* old_mem, size_t old_size, size_t new_size);
void arena_free_all(Arena* arena);
const char* arena_get_name(Arena* arena);


struct ArenaTemp {
    Arena* arena;
    size_t prev_offset;
    size_t offset;
};

inline void* arena_alloc(ArenaTemp* arena, size_t alloc_size) {
    // when using temp arenas, use the underlying arena
    return arena_alloc(arena->arena, alloc_size);
}
inline void* arena_resize(ArenaTemp* arena, void* old_mem, size_t old_size, size_t new_size) {
    return arena_resize(arena->arena, old_mem, old_size, new_size);
}

ArenaTemp arena_temp_init(Arena* arena);
void arena_temp_end(ArenaTemp tmp_arena);
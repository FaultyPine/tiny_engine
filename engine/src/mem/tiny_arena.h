#pragma once

//#include "pch.h"
#include "tiny_defines.h"
#include "tiny_mem.h"

// ARENAS

struct Arena {
    unsigned char* backing_mem = 0;
    size_t backing_mem_size = 0;
    size_t offset = 0;
    size_t prev_offset = 0;
};

#define arena_alloc_type(arena, type, num) ((type*)arena_alloc(arena, sizeof(type) * num))

template <typename T>
T* arena_alloc_and_init(Arena* arena, u32 numElements = 1)
{
    T* alloc = arena_alloc_type(arena, T, numElements);
    new(alloc) T(); // because some c++ types need their ctors called
    return alloc;
}

TAPI Arena arena_init(void* backing_buffer, size_t arena_size);
TAPI Arena arena_init(void* backing_buffer, size_t arena_size, const char* name);
TAPI void* arena_alloc(Arena* arena, size_t alloc_size);
TAPI void* arena_resize(Arena* arena, void* old_mem, size_t old_size, size_t new_size);
TAPI void arena_clear(Arena* arena);
TAPI void arena_free_all(Arena* arena);
TAPI const char* arena_get_name(Arena* arena);

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

TAPI ArenaTemp arena_temp_init(Arena* arena);
TAPI void arena_temp_end(ArenaTemp tmp_arena);


//#include "pch.h"
#include "mem/tiny_arena.h"
#include "tiny_log.h"

#define MAX_ARENA_NAME_LEN 30

Arena arena_init(void* backing_buffer, size_t arena_size) {
    Arena a;
    a.backing_mem = (unsigned char*)backing_buffer;
    a.backing_mem_size = arena_size;
    a.offset = 0;
    a.prev_offset = 0;
    //TMEMSET(backing_buffer, 0, arena_size);
    return a;
}

Arena arena_init(void* backing_buffer, size_t arena_size, const char* name) {
    Arena a = arena_init(backing_buffer, arena_size);
    char* name_mem = (char*)arena_alloc(&a, strnlen(name, MAX_ARENA_NAME_LEN)); 
    strcpy(name_mem, (char*)name);
    return a;
}

const char* arena_get_name(Arena* arena) {
    const char* possible_string = (const char*)arena->backing_mem;
    for (int i = 0; i < MAX_ARENA_NAME_LEN; i++) {
        if (possible_string[i] == '\0') {
            return possible_string;
        }
    }
    return "UNNAMED_ARENA";
}

void* arena_alloc(Arena* arena, size_t alloc_size) {
    size_t& offset = arena->offset;
    bool is_out_of_mem = offset + alloc_size > arena->backing_mem_size;
    if (is_out_of_mem) 
    {
        LOG_FATAL("Out of memory in arena %s\n", arena_get_name(arena));
        // maybe we automatically resize here?
        return nullptr;
    }
    // TODO: enforce alignment    
    void* new_alloc = arena->backing_mem + offset;
    arena->prev_offset = offset;
    offset += alloc_size;
    return new_alloc;
}

void* arena_resize(Arena* arena, void* old_mem, size_t old_size, size_t new_size) {
    // resize memory block if it's the most recent alloc.
    // otherwise, resizing just means reallocating and copying old mem to new spot
    uintptr_t old_mem_addr = (uintptr_t)old_mem;
    uintptr_t backing_mem_addr = (uintptr_t)arena->backing_mem;
    bool is_old_mem_in_range = old_mem_addr >= backing_mem_addr && old_mem_addr < backing_mem_addr + arena->offset;
    if (is_old_mem_in_range) {
        bool is_most_recent_alloc = old_mem_addr == backing_mem_addr + arena->prev_offset;
        if (is_most_recent_alloc) {
            arena->offset = arena->prev_offset + new_size;
            return old_mem;
        }
        else {
            void* new_mem = arena_alloc(arena, new_size);
            size_t copy_size = old_size < new_size ? old_size : new_size; // smaller of the two
            memmove(new_mem, old_mem, copy_size);
            return new_mem;
        }
    }
    else {
        TINY_ASSERT(false && "Out of bounds resize in arena");
        return nullptr;
    }
}

void arena_pop_latest(Arena* arena)
{
    if (arena->offset == arena->prev_offset)
    {
        LOG_WARN("Attempted to pop on arena without a valid most recent allocation");
        return;
    }
    arena->offset = arena->prev_offset;
}

void arena_clear(Arena* arena) {
    arena->offset = 0;
    arena->prev_offset = 0;
}

void arena_free_all(Arena* arena)
{
    arena_clear(arena);
    arena->backing_mem_size = 0;
    TSYSFREE(arena->backing_mem);
}



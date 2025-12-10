#include "arena.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct arena_block_t arena_block_t;

struct arena_block_t {
    arena_block_t* next;
    uint8_t memory[];
};

struct arena_t {
    arena_block_t* last_block;
    size_t last_block_used;
    size_t block_size;
    arena_block_t first_block;
};

const size_t DEFAULT_BLOCK_SIZE = 1 << 16;

// Not sure yet if want to make the api public
static arena_t* arena_alloc_with_block_size(size_t block_size) {
    // Arena and the first block allocated in place
    arena_t* arena = malloc(sizeof(arena_t) + block_size);
    arena->first_block.next = 0;
    arena->last_block = &arena->first_block;
    arena->block_size = block_size;
    arena->last_block_used = 0;
    return arena;
}

static void arena_push_block(arena_t* arena, size_t min_size) {
    size_t allocation_size =
        min_size > arena->block_size ? min_size : arena->block_size;
    arena_block_t* block = malloc(sizeof(arena_block_t) + allocation_size);
    arena->last_block->next = block;
    arena->last_block = block;
    arena->last_block_used = 0;
}

static void arena_pop_block(arena_t* arena) {
    arena_block_t* before_last = &arena->first_block;
    arena_block_t* last = &arena->first_block;

    while (last != arena->last_block) {
        before_last = last;
        last = last->next;
    }

    if (last != before_last) {
        free(last);
        arena->last_block = before_last;
        before_last->next = 0;
    }
}

static size_t arena_block_count(const arena_t* arena) {
    size_t result = 1;
    const arena_block_t* block = &arena->first_block;
    while (block->next != 0) {
        block = block->next;
        result += 1;
    }

    return result;
}

arena_t* arena_alloc(void) {
    return arena_alloc_with_block_size(DEFAULT_BLOCK_SIZE);
}

void arena_release(arena_t* arena) {
    arena_block_t* block = &arena->first_block;
    while (block->next != 0) {
        // The first block is in the same allocation as arena, so skip it here
        block = block->next;
        free(block);
    }
    free(arena);
}

size_t arena_align(arena_t* arena, size_t align) {
    size_t offset = arena->last_block_used % align;
    if (offset != 0) {
        size_t size_to_push = align - offset;
        arena_push(arena, size_to_push);
        return size_to_push;
    }
    return 0;
}

void* arena_push(arena_t* arena, size_t size) {
    if (arena->last_block_used + size > arena->block_size) {
        arena_push_block(arena, size);
    }

    void* result = arena->last_block->memory + arena->last_block_used;
    arena->last_block_used += size;
    return result;
}

void* arena_push_zero(arena_t* arena, size_t size) {
    void* result = arena_push(arena, size);
    memset(result, 0, size);
    return result;
}

void* arena_push_copy(arena_t* arena, void* src, size_t size) {
    void* result = arena_push(arena, size);
    memcpy(result, src, size);
    return result;
}

// pop some bytes off the 'stack' - the way to free
void arena_pop(arena_t* arena, size_t size) {
    if (size < arena->last_block_used) {
        arena->last_block_used -= size;
    } else {
        arena_pop_block(arena);
    }
}

// get the # of bytes currently allocated.
size_t arena_get_pos(arena_t* arena) {
    size_t count = arena_block_count(arena);
    return count - 1 + arena->last_block_used;
}

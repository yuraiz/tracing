#ifndef TRC_ARENA
#define TRC_ARENA

#include <stddef.h>
#include <stdint.h>

// Arena Allocator.
//
// Convenient to allocate multiple objects and free them together

typedef struct arena_t arena_t;

// Create `arena`.
//
// Must be freed with arena_release.
arena_t* arena_alloc(void);

// Release `arena`
void arena_release(arena_t* arena);

// Allocate and discard part of the buffer to align
// the pointer for the next allocation.
size_t arena_align(arena_t* arena, size_t align);

// Raw arena allocation methods
void* arena_push(arena_t* arena, size_t size);
// Push + memset 0
void* arena_push_zero(arena_t* arena, size_t size);
// Push + memcpy
void* arena_push_copy(arena_t* arena, void* src, size_t size);

// Compute the allocation size based on the type
#define arena_push_array(arena, type, count) \
    (type*)arena_push((arena), sizeof(type) * (count))
// Push array + memset 0
#define arena_push_array_zero(arena, type, count) \
    (type*)arena_push_zero((arena), sizeof(type) * (count))
// Compute the allocation size based on the type
#define arena_push_struct(arena, type) push_array((arena), (type), 1)
// Push struct + memset 0
#define arena_push_struct_zero(arena, type) push_array_zero((arena), (type), 1)

// Free last `size` bytes
void arena_pop(arena_t* arena, size_t size);

// Get how much bytes is allocated
size_t arena_get_pos(arena_t* arena);

#endif  // TRC_ARENA

#ifndef TRC_ARENA
#define TRC_ARENA

#include <stddef.h>
#include <stdint.h>

typedef struct arena_t arena_t;

// create or destroy a 'stack' - an "arena"
arena_t* arena_alloc(void);
void arena_release(arena_t* arena);

size_t arena_align(arena_t* arena, size_t align);

// push some bytes onto the 'stack' - the way to allocate
void* arena_push(arena_t* arena, size_t size);
void* arena_push_zero(arena_t* arena, size_t size);
void* arena_push_copy(arena_t* arena, void* src, size_t size);

#define arena_push_array(arena, type, count) \
    (type*)arena_push((arena), sizeof(type) * (count))
#define arena_push_array_zero(arena, type, count) \
    (type*)arena_push_zero((arena), sizeof(type) * (count))
#define arena_push_struct(arena, type) push_array((arena), (type), 1)
#define arena_push_struct_zero(arena, type) push_array_zero((arena), (type), 1)

// pop some bytes off the 'stack' - the way to free
void arena_pop(arena_t* arena, size_t size);

// get the # of bytes currently allocated.
size_t arena_get_pos(arena_t* arena);

#endif  // TRC_ARENA

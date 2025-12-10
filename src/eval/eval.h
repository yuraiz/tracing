#ifndef TRC_EVAL
#define TRC_EVAL

#include <mach/mach_types.h>

#include "../alloc/arena.h"
#include "../app_state.h"
#include "../string/string.h"

void eval(
    arena_t* arena, string_t expr, app_state_t* app_state, thread_t thread
);

void eval_str(
    arena_t* arena,
    string_t expr,
    app_state_t* __unused app_state,
    thread_t thread
);

#endif  // TRC_EVAL

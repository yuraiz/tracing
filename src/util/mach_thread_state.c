#include "mach_thread_state.h"

#include <inttypes.h>
#include <mach/mach.h>
#include <stdio.h>

#include "error.h"

#define GET_THREAD_STATE_BODY(_type, _flavor)                       \
    _type state;                                                    \
    mach_msg_type_number_t state_count = _flavor##_COUNT;           \
    expect_ok(                                                      \
        thread_get_state(                                           \
            thread, _flavor, (thread_state_t) & state, &state_count \
        ),                                                          \
        "thread_get_state_failed"                                   \
    );                                                              \
    return state;

arm_thread_state64_t trc_thread_get_arm_thread_state64(thread_act_t thread) {
    GET_THREAD_STATE_BODY(arm_thread_state64_t, ARM_THREAD_STATE64);
}

arm_debug_state64_t trc_thread_get_arm_debug_state64(thread_act_t thread) {
    GET_THREAD_STATE_BODY(arm_debug_state64_t, ARM_DEBUG_STATE64);
}

#define SET_THREAD_STATE_BODY(_flavor)                              \
    expect_ok(                                                      \
        thread_set_state(                                           \
            thread, _flavor, (thread_state_t)state, _flavor##_COUNT \
        ),                                                          \
        "thread_get_state_failed"                                   \
    );

void trc_thread_set_arm_thread_state64(
    thread_act_t thread, arm_thread_state64_t* state
) {
    SET_THREAD_STATE_BODY(ARM_THREAD_STATE64);
}

void trc_thread_set_arm_debug_state64(
    thread_act_t thread, arm_debug_state64_t* state
) {
    SET_THREAD_STATE_BODY(ARM_DEBUG_STATE64);
}

void trc_dump_arm_thread_state64(arm_thread_state64_t thread_state) {
    for (int i = 0; i < 29; i++) {
        printf("x%i: %" PRIu64 "\n", i, thread_state.__x[i]);
    }

    printf("x29: %" PRIu64 " (Frame pointer)\n", thread_state.__fp);
    printf("x29: %" PRIu64 " (Link register)\n", thread_state.__lr);
    printf("x29: %" PRIu64 " (Stack pointer)\n", thread_state.__sp);
    printf("x29: %" PRIu64 " (Program counter)\n", thread_state.__pc);
    printf(
        "x29: %" PRIu32 " (Current program status register)\n",
        thread_state.__cpsr
    );
}

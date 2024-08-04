#ifndef TRC_M_THREAD_STATE_UTIL_H
#define TRC_M_THREAD_STATE_UTIL_H

#include <inttypes.h>
#include <mach/mach.h>
#include <mach/mach_types.h>
#include <stdio.h>

#include "error_util.h"
#include "mach_thread_state_util.h"

// Get

arm_thread_state64_t trc_thread_get_arm_thread_state64(thread_act_t thread);

arm_debug_state64_t trc_thread_get_arm_debug_state64(thread_act_t thread);

// Set

void trc_thread_set_arm_thread_state64(
    thread_act_t thread, arm_thread_state64_t* state
);

void trc_thread_set_arm_debug_state64(
    thread_act_t thread, arm_debug_state64_t* state
);

// Dump

void trc_dump_arm_thread_state64(arm_thread_state64_t thread_state);

#endif  // TRC_M_THREAD_STATE_UTIL_H
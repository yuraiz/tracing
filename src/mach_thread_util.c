#include "mach_thread_util.h"

#include <inttypes.h>
#include <mach/exc.h>
#include <mach/exception.h>
#include <mach/exception_types.h>
#include <mach/kern_return.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <mach/mach_traps.h>
#include <mach/mach_types.h>
#include <mach/mach_vm.h>
#include <mach/message.h>
#include <mach/task.h>
#include <mach/thread_status.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "error_util.h"
#include "mach_thread_state_util.h"

void trc_thread_enable_single_step(thread_act_t thread) {
    arm_debug_state64_t debug_state = trc_thread_get_arm_debug_state64(thread);

    // Set SS (Single Stepping) bit
    debug_state.__mdscr_el1 |= 0x1;

    trc_thread_set_arm_debug_state64(thread, &debug_state);
}

void trc_thread_disable_single_step(thread_act_t thread) {
    arm_debug_state64_t debug_state = trc_thread_get_arm_debug_state64(thread);

    // Clear SS (Single Stepping) bit
    debug_state.__mdscr_el1 |= ~(0x1);

    trc_thread_set_arm_debug_state64(thread, &debug_state);
}

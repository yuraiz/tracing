#include <mach/arm/vm_types.h>
#include <mach/mach.h>
#include <mach/mach_types.h>
#include <mach/mach_vm.h>
#include <mach/message.h>
#include <stdio.h>

#include "app_state.h"
#include "breakpoint/breakpoint_controller.h"
#include "repl.h"
#include "string/string.h"
#include "symbolication/symbolicator.h"
#include "util/mach_thread_state.h"

extern kern_return_t catch_mach_exception_raise(
    mach_port_t __unused exception_port,
    mach_port_t thread,
    mach_port_t __unused task,
    exception_type_t exception,
    exception_data_t __unused code,
    mach_msg_type_number_t __unused code_count
) {
    if (exception == EXC_BREAKPOINT) {
        app_state_t* app_state = get_app_state();

        const arm_thread_state64_t thread_state =
            trc_thread_get_arm_thread_state64(thread);

        string_t symbol = trc_symbolicator_symbol_at_address(
            app_state->symbolicator, thread_state.__pc, 0
        );

        printf("hit breakpoint at %s\n", symbol.ptr);

        trc_breakpoint_controller_on_hit(
            &app_state->breakpoint_controller, thread_state.__pc
        );

        start_repl_bp(thread);

        return KERN_SUCCESS;
    } else {
        printf("unknown exception\n");
    }

    return KERN_FAILURE;
}

// Unused handles

extern kern_return_t catch_mach_exception_raise_state(
    mach_port_t __unused exception_port,
    exception_type_t __unused exception,
    exception_data_t __unused code,
    mach_msg_type_number_t __unused code_count,
    int* __unused flavor,
    thread_state_t __unused in_state,
    mach_msg_type_number_t __unused in_state_count,
    thread_state_t __unused out_state,
    mach_msg_type_number_t* __unused out_state_count
) {
    fprintf(stderr, "this handler should not be called");
    return MACH_RCV_INVALID_TYPE;
}

extern kern_return_t catch_mach_exception_raise_state_identity(
    mach_port_t __unused exception_port,
    mach_port_t __unused thread,
    mach_port_t __unused task,
    exception_type_t __unused exception,
    exception_data_t __unused code,
    mach_msg_type_number_t __unused code_count,
    int* __unused flavor,
    thread_state_t __unused in_state,
    mach_msg_type_number_t __unused in_state_count,
    thread_state_t __unused out_state,
    mach_msg_type_number_t* __unused out_state_count
) {
    fprintf(stderr, "this handler should not be called");
    return MACH_RCV_INVALID_TYPE;
}

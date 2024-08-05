#include <mach/exc.h>
#include <mach/exception_types.h>
#include <mach/mach.h>

#include "error.h"

// Function to set up an exception handler
void trc_setup_exception_handler(task_t task, mach_port_t* exc_port) {
    expect_ok(
        mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, exc_port),
        "failed to allocate port"
    );
    expect_ok(
        mach_port_insert_right(
            mach_task_self(), *exc_port, *exc_port, MACH_MSG_TYPE_MAKE_SEND
        ),
        "failed call to port insert right"
    );
    expect_ok(
        task_set_exception_ports(
            task,
            EXC_MASK_BREAKPOINT,
            *exc_port,
            EXCEPTION_DEFAULT | MACH_EXCEPTION_CODES,
            MACHINE_THREAD_STATE
        ),
        "failed to set exception ports"
    );
}

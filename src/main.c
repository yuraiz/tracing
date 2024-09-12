#include <mach/arm/kern_return.h>
#include <mach/exc.h>
#include <mach/mach.h>
#include <mach/message.h>
#include <mach/task.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/_types/_null.h>

#include "breakpoint/breakpoint_controller.h"
#include "globals.h"
#include "symbolication/symbolicator.h"
#include "util/error.h"
#include "util/mach_task.h"
#include "util/mach_thread.h"

const int MAC_OS_MAX_PID = 99998;

extern boolean_t mach_exc_server(
    mach_msg_header_t* InHeadP, mach_msg_header_t* OutHeadP
);

int main(int argc, char** argv) {
    kern_return_t status_code;

    if (argc != 2) {
        error("Usage [progam] [pid]");
    }

    const int decimal_radix = 10;
    char* end = NULL;
    unsigned long pid_in = strtoul(argv[1], &end, decimal_radix);

    if (*end || end == argv[1] || pid_in > MAC_OS_MAX_PID) {
        error("passed [pid] is not a number");
    }

    pid_t pid = (pid_t)pid_in;

    task_t task = 0;
    status_code = task_for_pid(mach_task_self(), pid, &task);
    expect_ok(status_code, "failed to get port");

    symbolicator_t symbolicator = trc_symbolicator_new_with_task(task);

    // const char* symbol_name =
    //     "_$LT$T$u20$as$u20$wgpu..context..DynContext$GT$::queue_submit::"
    //     "h495960884c2aca72";

    // trc_symbolicator_find_symbols_with_name(symbolicator, "", "", false,
    // size_t *symbol_count)

    trc_address_opt_t address_opt = trc_symbolicator_find_symbol(
        symbolicator, "my_println", "simple_app", true, 0, true
    );

    expect_true(address_opt.is_present, "failed to get address");

    printf(
        "symbol: %s\n",
        trc_symbolicator_symbol_at_address(
            symbolicator, address_opt.address, NULL
        )
    );

    BREAKPOINT_CONTROLLER = trc_breakpoint_controller_with_task_alloc(task);

    trc_breakpoint_controller_set_breakpoint(
        BREAKPOINT_CONTROLLER, address_opt.address
    );

    mach_port_t exc_port = 0;
    trc_setup_exception_handler(task, &exc_port);

    status_code = task_suspend(task);
    expect_ok(status_code, "failed to suspend");

    thread_act_array_t threads = NULL;
    mach_msg_type_number_t threads_len = 0;
    task_threads(task, &threads, &threads_len);

    expect_true(threads_len > 0, "child process has no threads");

    trc_thread_enable_watch_point(threads[0]);

    // trc_thread_enable_single_step(threads[0]);

    status_code = task_resume(task);
    expect_ok(status_code, "failed to resume");

    printf("starting loop\n");

    const mach_msg_size_t msg_size = 2048;

    status_code = mach_msg_server_once(
        mach_exc_server, msg_size, exc_port, MACH_MSG_TIMEOUT_NONE
    );
    // status_code = mach_msg_server(
    //     mach_exc_server, msg_size, exc_port, MACH_MSG_TIMEOUT_NONE
    // );
    expect_ok(status_code, "mach_msg_server() returned on error!");

    // trc_thread_disable_single_step(threads[0]);

    // printf("Hello, world, pid = %i\n", pid);

    printf("finished execution");
}

#include <mach/arm/kern_return.h>
#include <mach/exc.h>
#include <mach/mach.h>
#include <mach/message.h>
#include <mach/task.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/_types/_mach_port_t.h>
#include <sys/_types/_null.h>
#include <sys/_types/_pid_t.h>

#include "app_state.h"
#include "breakpoint/breakpoint_controller.h"
#include "repl.h"
#include "symbolication/symbolicator.h"
#include "util/error.h"
#include "util/mach_task.h"
#include "util/mach_thread.h"

const int MAC_OS_MAX_PID = 99998;

extern boolean_t mach_exc_server(
    mach_msg_header_t* InHeadP, mach_msg_header_t* OutHeadP
);

typedef struct {
    pid_t pid;
    char* error_message;
} parsed_args_t;

parsed_args_t parse_args(int argc, char** argv) {
    parsed_args_t result = {
        .error_message = 0,
    };

    if (argc != 2) {
        result.error_message = "Usage [progam] [pid]";
        return result;
    }

    const int decimal_radix = 10;
    char* end = NULL;
    unsigned long pid_in = strtoul(argv[1], &end, decimal_radix);

    if (*end || end == argv[1] || pid_in > MAC_OS_MAX_PID) {
        result.error_message = "passed [pid] is not a number";
        return result;
    }

    result.pid = (pid_t)pid_in;
    return result;
}

int main(int argc, char** argv) {
    parsed_args_t args = parse_args(argc, argv);

    if (args.error_message != 0) {
        error(args.error_message);
    }

    kern_return_t status_code = 0;

    task_t task = 0;
    status_code = task_for_pid(mach_task_self(), args.pid, &task);
    expect_ok(status_code, "failed to get port");

    symbolicator_t symbolicator = trc_symbolicator_new_with_task(task);

    breakpoint_controller_t breakpoint_controller =
        trc_breakpoint_controller_new_with_task(task);

    // Init exception handler

    mach_port_t exc_port = 0;
    trc_setup_exception_handler(task, &exc_port);

    status_code = task_suspend(task);
    expect_ok(status_code, "failed to suspend");

    thread_act_array_t threads = NULL;
    mach_msg_type_number_t threads_len = 0;
    task_threads(task, &threads, &threads_len);

    expect_true(threads_len > 0, "child process has no threads");

    trc_thread_enable_watch_point(threads[0]);

    // Init app state

    const app_state_t app_state = {
        .task = task,
        .symbolicator = symbolicator,
        .breakpoint_controller = breakpoint_controller,
        .exc_port = exc_port,
    };

    init_app_state(app_state);

    start_repl();
}

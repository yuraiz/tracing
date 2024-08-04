#include <mach/exc.h>
#include <mach/mach.h>
#include <stdio.h>
#include <stdlib.h>

#include "util/error.h"
#include "util/mach_task.h"
#include "util/mach_thread.h"

const int MAC_OS_MAX_PID = 99998;

extern boolean_t mach_exc_server(
    mach_msg_header_t* InHeadP, mach_msg_header_t* OutHeadP
);

int main(int argc, char** argv) {
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
    expect_ok(task_for_pid(mach_task_self(), pid, &task), "failed to get port");

    mach_port_t exc_port = 0;
    trc_setup_exception_handler(task, &exc_port);

    expect_ok(task_suspend(task), "failed to suspend");

    thread_act_array_t threads = NULL;
    mach_msg_type_number_t threads_len = 0;
    task_threads(task, &threads, &threads_len);

    expect_true(threads_len > 0, "child process has no threads");

    trc_thread_enable_single_step(threads[0]);

    expect_ok(task_resume(task), "failed to resume");

    printf("starting loop\n");

    const mach_msg_size_t msg_size = 2048;
    expect_ok(
        mach_msg_server_once(
            mach_exc_server, msg_size, exc_port, MACH_MSG_TIMEOUT_NONE
        ),
        "mach_msg_server() returned on error!"
    );

    trc_thread_disable_single_step(threads[0]);

    printf("Hello, world, pid = %i\n", pid);
}

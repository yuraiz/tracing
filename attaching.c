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

void error(char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(1);
}

void expect_ok(kern_return_t status_code, char* failure_message) {
    if (status_code) {
        fprintf(stderr, "Status code: %s\n", mach_error_string(status_code));
        error(failure_message);
    }
}

void expect_true(int boolean, char* failure_message) {
    if (!boolean) {
        fprintf(stderr, "Status code: %i\n", boolean);
        error(failure_message);
    }
}

void dump_arm_thread_state(arm_thread_state64_t thread_state) {
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

// Function to set up an exception handler
void setup_exception_handler(task_t task, mach_port_t* exc_port) {
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

    // exception_mask_t mask = EXC_MASK_BREAKPOINT;
    exception_mask_t mask = EXC_MASK_ALL;
    kern_return_t kr = task_set_exception_ports(
        task,
        mask,
        *exc_port,
        // EXCEPTION_DEFAULT | MACH_EXCEPTION_CODES,
        EXCEPTION_DEFAULT | EXCEPTION_STATE | EXCEPTION_STATE_IDENTITY,
        ARM_THREAD_STATE64
    );
    expect_ok(kr, "failed to set exception ports");
}

#define MACH_EXCEPTION_RAISE 2401
#define MACH_EXCEPTION_RAISE_STATE 2402
#define MACH_EXCEPTION_RAISE_STATE_IDENTITY 2403

// Exception handling function
void handle_exception(mach_port_t exc_port) {
    mach_msg_header_t* msg = (mach_msg_header_t*)malloc(4096);
    while (1) {
        // Define the exception message structure
        kern_return_t kr;

        // Receive the exception message
        kr = mach_msg(
            msg,
            MACH_RCV_MSG,
            0,
            4096,
            exc_port,
            MACH_MSG_TIMEOUT_NONE,
            MACH_PORT_NULL
        );
        if (kr != KERN_SUCCESS) {
            fprintf(stderr, "mach_msg failed: %s\n", mach_error_string(kr));
            continue;
        }

        printf("msg id: %u\n", msg->msgh_id);
        printf("msg size: %u\n", msg->msgh_size);

        // Determine the type of exception message received and handle
        // accordingly
        switch (msg->msgh_id) {
            case MACH_EXCEPTION_RAISE:
                // Handle MACH_EXCEPTION_RAISE message
                printf("Received MACH_EXCEPTION_RAISE\n");
                // Extract details and handle the exception
                break;
            case MACH_EXCEPTION_RAISE_STATE:
                // Handle MACH_EXCEPTION_RAISE_STATE message
                printf("Received MACH_EXCEPTION_RAISE_STATE\n");
                // Extract details and handle the exception
                break;
            case MACH_EXCEPTION_RAISE_STATE_IDENTITY:
                // Handle MACH_EXCEPTION_RAISE_STATE_IDENTITY message
                printf("Received MACH_EXCEPTION_RAISE_STATE_IDENTITY\n");

                __Request__exception_raise_state_identity_t* exc_msg =
                    (__Request__exception_raise_state_identity_t*)&msg;

                // Example handling logic
                // printf("Exception received: %d\n", exc_msg->exception);
                // printf("Thread: %u, Task: %u\n", exc_msg.t, task);

                // task_resume(exc_msg->task.disposition);

                // // Print exception codes
                // for (int i = 0; i < code_count; i++) {
                //     printf("Exception code[%d]: %u\n", i, code[i]);
                // }

                // expect_ok(
                //     thread_resume(exc_msg->thread.name),
                //     "failed to resume thread"
                // );

                arm_thread_state64_t thread_state;
                memcpy(&thread_state, exc_msg->old_state, sizeof(thread_state));

                dump_arm_thread_state(thread_state);

                // expect_ok(
                //     task_resume(exc_msg->task.name), "failed to resume task"
                // );

                // Extract details and handle the exception
                break;
            default:
                printf("Unknown message received with ID: %d\n", msg->msgh_id);
                break;
        }

        // Cast the received message to an exception message type
        // mach_exception_raise_state_t* exc_msg =
        //     (mach_exception_raise_state_t*)&msg;

        // // Extract the thread and task ports
        // thread_t thread = exc_msg->thread.name;
        // task_t task = exc_msg->task.name;

        // // Handle the exception (retrieve and potentially modify the thread
        // // state)
        // handle_thread_state(thread, task);

        // Acknowledge the exception
        // Depending on your implementation, you might need to send a reply
        // message or update thread state
    }
}

void enable_single_step(thread_act_t thread) {
    kern_return_t kr;
    arm_debug_state64_t debug_state;
    mach_msg_type_number_t state_count = ARM_DEBUG_STATE64_COUNT;

    // Get the debug state
    kr = thread_get_state(
        thread, ARM_DEBUG_STATE64, (thread_state_t)&debug_state, &state_count
    );
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "thread_get_state failed: %s\n", mach_error_string(kr));
        exit(1);
    }

    // Enable single-stepping
    debug_state.__mdscr_el1 |= 0x1;  // Set the SS bit in MDSCR_EL1

    // Set the modified debug state
    kr = thread_set_state(
        thread,
        ARM_DEBUG_STATE64,
        (thread_state_t)&debug_state,
        ARM_DEBUG_STATE64_COUNT
    );
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "thread_set_state failed: %s\n", mach_error_string(kr));
        exit(1);
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        error("Usage [progam] [pid]");
    }

    pid_t pid = atoi(argv[1]);
    task_t task = 0;
    expect_ok(task_for_pid(mach_task_self(), pid, &task), "failed to get port");

    mach_port_t exc_port;
    setup_exception_handler(task, &exc_port);

    // // Start a thread to handle exceptions
    // pthread_t exception_thread;
    // pthread_create(
    //     &exception_thread,
    //     NULL,
    //     (void* (*)(void*))handle_exception,
    //     (void*)(size_t)exc_port
    // );

    expect_ok(task_suspend(task), "failed to suspend");

    thread_act_array_t threads = NULL;
    mach_msg_type_number_t threads_len = 0;
    task_threads(task, &threads, &threads_len);

    if (threads_len > 0) {
        thread_act_t thread_act = threads[0];

        enable_single_step(thread_act);

        // arm_thread_state64_t thread_state;
        // mach_msg_type_number_t state_count = ARM_THREAD_STATE64_COUNT;

        // expect_ok(
        //     thread_get_state(
        //         thread_act,
        //         ARM_THREAD_STATE64,
        //         (thread_state_t)&thread_state,
        //         &state_count
        //     ),
        //     "failed to get state"
        // );

        // dump_arm_thread_state(thread_state);

        // int32_t data[32];
        // mach_msg_type_number_t data_len = sizeof(data);

        // thread_state.__x[0] = 1;

        // expect_ok(
        //     thread_set_state(
        //         thread_act,
        //         ARM_THREAD_STATE64,
        //         (thread_state_t)&thread_state,
        //         ARM_THREAD_STATE64_COUNT
        //     ),
        //     "failed to set state"
        // );
    }

    expect_ok(task_resume(task), "failed to resume");

    // sleep(10);

    handle_exception(exc_port);

    printf("Hello, world, pid = %i\n", pid);
}

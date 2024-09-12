#ifndef TRC_BREAKPOINT_CONTROLLER
#define TRC_BREAKPOINT_CONTROLLER

#include <mach/arm/vm_types.h>
#include <mach/mach_types.h>
#include <stdlib.h>

#include "breakpoint_table.h"

typedef struct {
    breakpoint_table_t table;
    task_t task;
} breakpoint_controller_t;

static inline breakpoint_controller_t trc_breakpoint_controller_new_with_task(
    task_t task
) {
    breakpoint_controller_t controller = {
        .table = trc_breakpoint_table_new(), .task = task
    };
    return controller;
}

static inline breakpoint_controller_t*
trc_breakpoint_controller_with_task_alloc(task_t task) {
    breakpoint_controller_t* controller =
        (breakpoint_controller_t*)calloc(1, sizeof(breakpoint_controller_t));
    controller->task = task;
    return controller;
}

void trc_breakpoint_controller_set_breakpoint(
    breakpoint_controller_t* controller, mach_vm_address_t address
);

void trc_breakpoint_controller_disable_breakpoint(
    breakpoint_controller_t* controller, mach_vm_address_t address
);

void trc_breakpoint_controller_remove_breakpoint(
    breakpoint_controller_t* controller, mach_vm_address_t address
);

#endif  // TRC_BREAKPOINT_CONTROLLER

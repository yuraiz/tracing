#include "breakpoint_controller.h"

#include <mach/arm/vm_types.h>
#include <mach/mach_vm.h>
#include <mach/vm_types.h>
#include <stddef.h>

#include "../util/error.h"
#include "../util/mach_write.h"
#include "breakpoint_table.h"

const breakpoint_table_value_t BREAKPOINT_OPCODE = {
    .data = {0x00, 0x00, 0x20, 0xD4}
};

static breakpoint_table_value_t read_value(task_t task, vm_address_t address) {
    breakpoint_table_value_t value;

    const mach_vm_size_t requested_size = sizeof(value.data);
    mach_vm_size_t out_size = 0;

    expect_ok(
        mach_vm_read_overwrite(
            task,
            address,
            requested_size,
            (mach_vm_address_t)&value.data,
            &out_size

        ),
        "failed to read instruction"
    );

    return value;
}

static void write_value(
    task_t task, vm_address_t address, breakpoint_table_value_t value
) {
    trc_mach_write_to_protected(
        task, (mach_vm_address_t)address, value.data, sizeof(value.data), true
    );
}

void trc_breakpoint_controller_set_breakpoint(
    breakpoint_controller_t* controller, vm_address_t address
) {
    breakpoint_table_value_t value = read_value(controller->task, address);
    trc_breakpoint_table_set(&controller->table, address, value);
    write_value(controller->task, address, BREAKPOINT_OPCODE);
}

void trc_breakpoint_controller_disable_breakpoint(
    breakpoint_controller_t* controller, vm_address_t address
) {
    breakpoint_table_value_t value =
        trc_breakpoint_table_get(&controller->table, address);
    write_value(controller->task, address, value);
}

void trc_breakpoint_controller_remove_breakpoint(
    breakpoint_controller_t* controller, vm_address_t address
) {
    breakpoint_table_value_t value =
        trc_breakpoint_table_remove(&controller->table, address);
    write_value(controller->task, address, value);
}

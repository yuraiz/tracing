#include "breakpoint_controller.h"

#include <mach/arm/vm_types.h>
#include <mach/mach_vm.h>
#include <mach/vm_types.h>
#include <stddef.h>
#include <stdio.h>

#include "../util/debug_helpers.h"
#include "../util/error.h"
#include "../util/mach_write.h"
#include "breakpoint_table.h"

const breakpoint_table_value_t BREAKPOINT_OPCODE = {
    .data = {0x00, 0x00, 0x20, 0xD4}
};

static void print_as_hex(uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
}

static breakpoint_table_value_t read_value(
    task_t task, mach_vm_address_t address
) {
    breakpoint_table_value_t value;

    const mach_vm_size_t requested_size = sizeof(value.data);

    mach_vm_size_t outsize = 0;

    mach_vm_read_overwrite(
        task, address, requested_size, (mach_vm_offset_t)value.data, &outsize
    );

    printf("DEBUG btable read:\n");
    debug_task_mem(task, address, 4 * 16);

    // expect_ok(
    //     mach_vm_read_overwrite(
    //         task,
    //         address,
    //         requested_size,
    //         (mach_vm_address_t)value.data,
    //         &out_size

    //     ),
    //     "failed to read instruction"
    // );

    return value;
}

static void write_value(
    task_t task, mach_vm_address_t address, breakpoint_table_value_t value
) {
    printf("passed address: %p\n", (void*)address);
    trc_mach_write_to_protected(
        task, address, value.data, sizeof(value.data), true
    );
}

void trc_breakpoint_controller_set_breakpoint(
    breakpoint_controller_t* controller, mach_vm_address_t address
) {
    printf("wanted address: %p\n", (void*)address);

    breakpoint_table_value_t value = read_value(controller->task, address);
    printf(
        "btable read: %02x%02x%02x%02x\n",
        value.data[0],
        value.data[1],
        value.data[2],
        value.data[3]
    );

    trc_breakpoint_table_set(&controller->table, address, value);
    write_value(controller->task, address, BREAKPOINT_OPCODE);

    breakpoint_table_value_t value2 = read_value(controller->task, address);
    printf(
        "btable read2: %02x%02x%02x%02x\n",
        value2.data[0],
        value2.data[1],
        value2.data[2],
        value2.data[3]
    );

    breakpoint_table_value_t bp_opcode = BREAKPOINT_OPCODE;
    printf(
        "btable bp_opcode: %02x%02x%02x%02x\n",
        bp_opcode.data[0],
        bp_opcode.data[1],
        bp_opcode.data[2],
        bp_opcode.data[3]
    );
}

void trc_breakpoint_controller_disable_breakpoint(
    breakpoint_controller_t* controller, mach_vm_address_t address
) {
    breakpoint_table_value_t value =
        trc_breakpoint_table_get(&controller->table, address);
    write_value(controller->task, address, value);
}

void trc_breakpoint_controller_remove_breakpoint(
    breakpoint_controller_t* controller, mach_vm_address_t address
) {
    breakpoint_table_value_t value =
        trc_breakpoint_table_remove(&controller->table, address);
    write_value(controller->task, address, value);
}

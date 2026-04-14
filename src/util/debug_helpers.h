#ifndef TRC_DEBUG_HELPERS_H
#define TRC_DEBUG_HELPERS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "mach/mach_types.h"
#include "mach/mach_vm.h"

static inline void debug_task_mem(
    task_t task, mach_vm_address_t address, mach_vm_size_t size
) {
    printf("address: %p\n", (void*)address);

    uint8_t* buf = (uint8_t*)calloc(1, size);
    mach_vm_size_t outsize = 0;

    mach_vm_read_overwrite(
        task, address, size, (mach_vm_offset_t)buf, &outsize
    );

    for (size_t i = 0; i < size; i++) {
        printf("%02x", buf[i]);
    }
    printf("\n");
}

static void print_as_hex(uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
}

#endif  // TRC_DEBUG_HELPERS_H

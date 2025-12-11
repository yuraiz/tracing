#ifndef TRC_BREAKPOINT_TABLE
#define TRC_BREAKPOINT_TABLE

#include <mach/vm_types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/_types/_null.h>

// A value for `breakpoint_table_t`.
//
// Stores the original instruction bytes, which is replaced with `brk` on breakpoint set.
typedef struct {
    // TODO(yuraiz): 4 bytes on ARM64, 1 byte on x84_64
    uint8_t data[4];
} breakpoint_table_value_t;

// A "table" with addresses and the saved values.
//
// Used in the breakpoint controller, not intended to be used directly.
//
// Current implementation: "Structure of arrays" with addresses and values.
// Value for the address is stored at the same index as the address.
typedef struct {
    mach_vm_address_t* addresses;
    breakpoint_table_value_t* values;
    size_t len;
    size_t capacity;
} breakpoint_table_t;

static inline breakpoint_table_t trc_breakpoint_table_new(void) {
    breakpoint_table_t result = {
        .addresses = NULL,
        .values = NULL,
        .len = 0,
        .capacity = 0,
    };
    return result;
}

static inline breakpoint_table_t* trc_breakpoint_table_new_alloc(void) {
    return (breakpoint_table_t*)calloc(1, sizeof(breakpoint_table_t));
}

bool trc_breakpoint_table_contains(
    breakpoint_table_t* table, mach_vm_address_t address
);

void trc_breakpoint_table_set(
    breakpoint_table_t* table,
    mach_vm_address_t address,
    breakpoint_table_value_t value
);

breakpoint_table_value_t trc_breakpoint_table_get(
    breakpoint_table_t* table, mach_vm_address_t address
);

breakpoint_table_value_t trc_breakpoint_table_remove(
    breakpoint_table_t* table, mach_vm_address_t address
);

void trc_breakpoint_table_dump(breakpoint_table_t* table);

#endif  // TRC_BREAKPOINT_TABLE

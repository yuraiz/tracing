#ifndef TRC_BREAKPOINT_TABLE
#define TRC_BREAKPOINT_TABLE

#include <mach/vm_types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/_types/_null.h>

typedef struct {
    // TODO(yuraiz): 4 bytes on ARM64, 1 byte on x84_64
    int8_t data[4];
} breakpoint_table_value_t;

typedef struct {
    vm_address_t* addresss;
    breakpoint_table_value_t* values;
    size_t len;
    size_t capacity;
} breakpoint_table_t;

static inline breakpoint_table_t trc_breakpoint_table_new(void) {
    breakpoint_table_t result = {
        .addresss = NULL,
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
    breakpoint_table_t* table, vm_address_t address
);

void trc_breakpoint_table_set(
    breakpoint_table_t* table,
    vm_address_t address,
    breakpoint_table_value_t value
);

breakpoint_table_value_t trc_breakpoint_table_get(
    breakpoint_table_t* table, vm_address_t address
);

breakpoint_table_value_t trc_breakpoint_table_remove(
    breakpoint_table_t* table, vm_address_t address
);

void trc_breakpoint_table_dump(breakpoint_table_t* table);

#endif  // TRC_BREAKPOINT_TABLE

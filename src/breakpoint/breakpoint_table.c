#include "breakpoint_table.h"

#include <inttypes.h>
#include <mach/vm_types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_null.h>

// #include <mach/vm_types.h>
// #include <stdint.h>

// typedef struct {
//     // TODO: 4 bytes on ARM64, 1 byte on x84_64
//     int8_t data[4];
// } breakpoint_table_value_t;

// struct breakpoint_table {
//     mach_vm_address_t* addresss;
//     breakpoint_table_value_t* values;
// };

typedef struct {
    bool exists;
    size_t position;
} pos_result_t;

static pos_result_t breakpoint_table_address_position(
    breakpoint_table_t* table, mach_vm_address_t address
) {
    pos_result_t result = {
        .exists = false,
        .position = 0,
    };

    // TODO(yuraiz): Use binary search
    for (; result.position < table->len; result.position++) {
        mach_vm_address_t value = table->addresss[result.position];
        if (value == address) {
            result.exists = true;
            break;
        } else if (value > address) {
            result.exists = false;
            break;
        }
    }

    return result;
}

static void breakpoint_table_reserve(
    breakpoint_table_t* table, size_t new_capacity
) {
    if (table->capacity > new_capacity) {
        return;
    }

    table->capacity = new_capacity;

    table->addresss =
        realloc(table->addresss, sizeof(mach_vm_address_t) * new_capacity);

    table->values =
        realloc(table->values, sizeof(breakpoint_table_value_t) * new_capacity);
}

static void breakpoint_table_ensure_capacity(
    breakpoint_table_t* table, size_t min
) {
    if (table->capacity < min) {
        const size_t new_capacity =
            table->capacity == 0 ? 16 : table->capacity * 2;
        breakpoint_table_reserve(table, new_capacity);
    }
}

static void breakpoint_table_add_new_position(
    breakpoint_table_t* table, size_t pos
) {
    if (pos == table->len) {
        // Add to the end
        table->len++;
        breakpoint_table_ensure_capacity(table, table->len);
    } else {
        // Required to move elements
        breakpoint_table_ensure_capacity(table, table->len + 1);

        const size_t element_count = table->len - pos;

        mach_vm_address_t* start_adr = table->addresss + pos;
        mach_vm_address_t* dst_adr = start_adr + 1;
        size_t size_adr = sizeof(mach_vm_address_t) * element_count;
        memmove(dst_adr, start_adr, size_adr);

        breakpoint_table_value_t* start_val = table->values + pos;
        breakpoint_table_value_t* dst_val = start_val + 1;
        size_t size_val = sizeof(breakpoint_table_value_t) * element_count;
        memmove(dst_val, start_val, size_val);

        table->len++;
    }
}

static void breakpoint_table_remove_position(
    breakpoint_table_t* table, size_t pos
) {
    if (pos == table->len - 1) {
        // Remove from the end
        table->len--;
    } else {
        table->len--;
        // Required to move elements
        const size_t element_count = table->len - pos;

        mach_vm_address_t* start_adr = table->addresss + pos + 1;
        mach_vm_address_t* dst_adr = start_adr - 1;
        size_t size_adr = sizeof(mach_vm_address_t) * element_count;
        memmove(dst_adr, start_adr, size_adr);

        breakpoint_table_value_t* start_val = table->values + pos + 1;
        breakpoint_table_value_t* dst_val = start_val - 1;
        size_t size_val = sizeof(breakpoint_table_value_t) * element_count;
        memmove(dst_val, start_val, size_val);
    }
}

bool trc_breakpoint_table_contains(
    breakpoint_table_t* table, mach_vm_address_t address
) {
    return breakpoint_table_address_position(table, address).exists;
}

void trc_breakpoint_table_set(
    breakpoint_table_t* table,
    mach_vm_address_t address,
    breakpoint_table_value_t value
) {
    pos_result_t res = breakpoint_table_address_position(table, address);

    if (!res.exists) {
        breakpoint_table_add_new_position(table, res.position);
        table->addresss[res.position] = address;
    }

    table->values[res.position] = value;
}

breakpoint_table_value_t trc_breakpoint_table_get(
    breakpoint_table_t* table, mach_vm_address_t address
) {
    pos_result_t res = breakpoint_table_address_position(table, address);
    if (res.exists) {
        return table->values[res.position];
    } else {
        breakpoint_table_value_t zeroed = {0};
        return zeroed;
    }
}

breakpoint_table_value_t trc_breakpoint_table_remove(
    breakpoint_table_t* table, mach_vm_address_t address
) {
    pos_result_t res = breakpoint_table_address_position(table, address);

    if (res.exists) {
        breakpoint_table_value_t value = table->values[res.position];
        breakpoint_table_remove_position(table, res.position);
        return value;
    } else {
        breakpoint_table_value_t zeroed = {0};
        return zeroed;
    }
}

void trc_breakpoint_table_dump(breakpoint_table_t* table) {
    printf("breakpoint table 0x%" PRIXPTR "\n", (uintptr_t)table);
    for (size_t i = 0; i < table->len; i++) {
        mach_vm_address_t address = table->addresss[i];
        breakpoint_table_value_t value = table->values[i];
        printf("    address: %p\n", (void*)address);
        printf("    value: ");
        const size_t byte_count = sizeof(value);
        for (size_t j = 0; j < byte_count; j++) {
            printf("%02X", value.data[j]);
        }
        printf("\n");
    }
}

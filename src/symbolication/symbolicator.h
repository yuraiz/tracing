// Api based on "Bit-Slicer/Bit Slicer/ZGSymbolicator.h"
// Should be enough for basic handling

#ifndef TRC_SYMBOLICATOR_H
#define TRC_SYMBOLICATOR_H

#include <mach/mach_types.h>
#include <stdbool.h>
#include <stddef.h>

#include "../deps/CSTypeRef.h"
#include "../string/string.h"

typedef struct sCSTypeRef symbolicator_t;

typedef struct {
    mach_vm_address_t start;
    vm_size_t len;
} symbol_range_t;

typedef struct {
    bool is_present;
    mach_vm_address_t address;
} trc_address_opt_t;

symbolicator_t trc_symbolicator_new_with_task(task_t);

string_t trc_symbolicator_symbol_at_address(
    symbolicator_t symbolicator,
    mach_vm_address_t address,
    mach_vm_address_t* relative_offset
);

symbol_range_t* trc_symbolicator_find_symbols_with_name(
    symbolicator_t symbolicator,
    const string_t symbol_name,
    const string_t partial_symbol_owner_name,
    bool exact_match,
    size_t* symbol_count
);

trc_address_opt_t trc_symbolicator_find_symbol(
    symbolicator_t symbolicator,
    const string_t symbol_name,
    const string_t partial_symbol_owner_name,
    bool exact_match,
    mach_vm_address_t past_address,
    bool allow_wrapping_to_beginning
);

#endif  //TRC_SYMBOLICATOR_H

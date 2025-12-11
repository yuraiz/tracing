// Api based on "Bit-Slicer/Bit Slicer/ZGSymbolicator.h"
// Should be enough for basic handling

#ifndef TRC_SYMBOLICATOR_H
#define TRC_SYMBOLICATOR_H

#include <mach/mach_types.h>
#include <stdbool.h>
#include <stddef.h>

#include "../deps/CSTypeRef.h"
#include "../string/string.h"

// A wrapper around CoreSymbolication api, because I don't need the whole flexibility.
//
// The original api requires usage the Objective-C blocks.

typedef struct sCSTypeRef symbolicator_t;

// Range of multiple symbols.
typedef struct {
    mach_vm_address_t start;
    vm_size_t len;
} symbol_range_t;

// Result of the symbol search.
typedef struct {
    bool is_present;
    mach_vm_address_t address;
} trc_address_opt_t;

// Create a symbolicator for the task.
symbolicator_t trc_symbolicator_new_with_task(task_t);

string_t trc_symbolicator_symbol_at_address(
    symbolicator_t symbolicator,
    mach_vm_address_t address,
    mach_vm_address_t* relative_offset
);

// Search for multiple symbols matching the name.
symbol_range_t* trc_symbolicator_find_symbols_with_name(
    symbolicator_t symbolicator,
    const string_t symbol_name,
    const string_t partial_symbol_owner_name,
    bool exact_match,
    size_t* symbol_count
);

// Search for the symbol matching name.
trc_address_opt_t trc_symbolicator_find_symbol(
    symbolicator_t symbolicator,
    const string_t symbol_name,
    const string_t partial_symbol_owner_name,
    bool exact_match,
    mach_vm_address_t past_address,
    bool allow_wrapping_to_beginning
);

#endif  //TRC_SYMBOLICATOR_H

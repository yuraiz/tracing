#include "symbolicator.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/_types/_null.h>

#include "../deps/CoreSymbolication.h"

symbolicator_t trc_symbolicator_new_with_task(task_t task) {
    return CSSymbolicatorCreateWithTask(task);
}

const char* trc_symbolicator_symbol_at_address(
    symbolicator_t symbolicator,
    vm_address_t address,
    vm_address_t* relative_offset
) {
    const char* result = NULL;
    if (!CSIsNull(symbolicator)) {
        CSSymbolRef symbol = CSSymbolicatorGetSymbolWithAddressAtTime(
            symbolicator, address, kCSNow
        );
        if (!CSIsNull(symbol)) {
            result = CSSymbolGetName(symbol);

            if (relative_offset != NULL) {
                CSRange symbol_range = CSSymbolGetRange(symbol);
                *relative_offset = address - symbol_range.location;
            }
        }
    }
    return result;
}

static bool str_eq(const char* left, const char* right) {
    if (left == NULL || right == NULL) {
        return false;
    }

    while (*left) {
        if ((*left++ != *right++)) {
            return false;
        }
    }

    return *left == *right;
}

static bool str_has_suffix(const char* left, const char* right) {
    if (left == NULL || right == NULL) {
        return false;
    }
    // skip to the suffix
    while (*left && *left != *right) {
        left++;
    }

    return str_eq(left, right);
}

static int symbol_range_cmp(const symbol_range_t* l, const symbol_range_t* r) {
    return (l->start > r->start) - (l->start < r->start);
}

symbol_range_t* trc_symbolicator_find_symbols_with_name(
    symbolicator_t symbolicator,
    const char* symbol_name,
    const char* partial_symbol_owner_name,
    bool exact_match,
    size_t* symbol_count
) {
    printf("trc_symbolicator_find_symbols_with_name\n");

    __block symbol_range_t* result_buffer = NULL;
    __block size_t capacity = 0;
    __block size_t index = 0;
    // CSSymbolicatorForeachSymbolOwnerAtTime(CSSymbolicatorRef symbolicator,
    // uint64_t time, ^(CSSymbolOwnerRef owner)it)
    CSSymbolicatorForeachSymbolOwnerAtTime(
        symbolicator,
        kCSNow,
        ^void(CSSymbolOwnerRef owner) {
            // this really returns a suffix
            const char* symbol_owner_name = CSSymbolOwnerGetName(owner);

            // printf("symbol owner name: %s\n", symbol_owner_name);

            if (partial_symbol_owner_name == NULL ||
                str_has_suffix(partial_symbol_owner_name, symbol_owner_name)) {
                CSSymbolOwnerForeachSymbol(owner, ^(CSSymbolRef symbol) {
                    const char* current_symbol = CSSymbolGetName(symbol);

                    if (current_symbol != NULL &&
                        ((exact_match && str_eq(symbol_name, current_symbol)) ||
                         (!exact_match &&
                          strstr(current_symbol, symbol_name) != NULL))) {
                        CSRange cs_symbol_range = CSSymbolGetRange(symbol);

                        symbol_range_t symbol_range = {
                            .start = cs_symbol_range.location,
                            .len = cs_symbol_range.length
                        };

                        if (index >= capacity) {
                            capacity = capacity == 0 ? 16 : capacity * 2;
                            result_buffer = realloc(
                                result_buffer, capacity * sizeof(symbol_range_t)
                            );
                        }

                        // printf("symbol name: %s\n", current_symbol);
                        // printf("symbol owner name: %s\n", symbol_owner_name);
                        // printf(
                        //     "symbol range: %lx %lx\n",
                        //     symbol_range.start,
                        //     symbol_range.len
                        // );

                        result_buffer[index] = symbol_range;
                        index += 1;
                    }
                });
            }
        }
    );

    // TODO(yuraiz): Replace with a sort
    qsort(
        result_buffer,
        index,
        sizeof(symbol_range_t),
        (int (*)(const void*, const void*))symbol_range_cmp
    );

    *symbol_count = index;
    return result_buffer;
}

trc_address_opt_t trc_symbolicator_find_symbol(
    symbolicator_t symbolicator,
    const char* symbol_name,
    const char* partial_symbol_owner_name,
    bool exact_match,
    vm_address_t past_address,
    bool allow_wrappint_to_beginning
) {
    trc_address_opt_t result = {.is_present = false, .address = 0};

    size_t symbol_count = 0;
    symbol_range_t* symbols = trc_symbolicator_find_symbols_with_name(
        symbolicator,
        symbol_name,
        partial_symbol_owner_name,
        exact_match,
        &symbol_count
    );

    for (size_t i = 0; i < symbol_count; i++) {
        symbol_range_t symbol_range = symbols[i];
        if (symbol_range.start > past_address) {
            result.is_present = true;
            result.address = symbol_range.start;
            break;
        }
    }

    if (allow_wrappint_to_beginning && symbol_count > 0) {
        result.is_present = true;
        result.address = symbols[0].start;
    }

    free(symbols);
    return result;
}

#include <mach/vm_region.h>
#include <stdbool.h>
#include <stddef.h>

void trc_mach_write_to_protected(
    task_t task,
    mach_vm_address_t address,
    const void* data,
    size_t size,
    bool revert_back
);

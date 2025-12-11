#include <mach/vm_region.h>
#include <stdbool.h>
#include <stddef.h>

// Write to the protected memory, restoring the protection after.
//
// Modifying instructions is required to set breakpoints, see `breakpoint_controller.c` for more info.
void trc_mach_write_to_protected(
    task_t task,
    mach_vm_address_t address,
    const void* data,
    size_t size,
    bool revert_back
);

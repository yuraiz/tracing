#include "mach_write.h"

#include <alloca.h>
#include <mach/arm/vm_types.h>
#include <mach/mach_types.h>
#include <mach/mach_vm.h>
#include <mach/message.h>
#include <mach/port.h>
#include <mach/task.h>
#include <mach/vm_inherit.h>
#include <mach/vm_map.h>
#include <mach/vm_prot.h>
#include <mach/vm_region.h>
#include <mach/vm_types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/_types/_null.h>

#include "debug_helpers.h"
#include "error.h"

const char* const BOOL_TO_STR[] = {[false] = "false", [true] = "true"};

static void print_vm_prot(vm_prot_t protection) {
    char buf[4] = {0};
    size_t index = 0;

    if (protection & VM_PROT_READ) {
        buf[index] = 'R';
        index += 1;
    }
    if (protection & VM_PROT_WRITE) {
        buf[index] = 'W';
        index += 1;
    }
    if (protection & VM_PROT_EXECUTE) {
        buf[index] = 'X';
        index += 1;
    }

    if (buf[0] != 0) {
        puts(buf);
    } else {
        puts("None");
    }
}

static void print_vm_inherit(vm_inherit_t inherit) {
    const char* res = 0;
    if (inherit == VM_INHERIT_SHARE) {
        res = "SHARE";
    } else if (inherit == VM_INHERIT_COPY) {
        res = "COPY";
    } else if (inherit == VM_INHERIT_NONE) {
        res = "NONE";
    } else if (inherit == VM_INHERIT_DONATE_COPY) {
        res = "DONATE_COPY";
    } else {
        res = "UNKNOWN";
        printf("Unknown inherit: %u", inherit);
    }
    puts(res);
}

static void print_region_info(vm_region_basic_info_64_t region_info) {
    printf("Region Info:\n");
    printf("protection: ");
    print_vm_prot(region_info->protection);
    printf("max_protection: ");
    print_vm_prot(region_info->max_protection);
    printf("inheritance: ");
    print_vm_inherit(region_info->inheritance);
    printf("shared: %s\n", BOOL_TO_STR[region_info->shared]);
    printf("reserved: %s\n", BOOL_TO_STR[region_info->reserved]);
    printf("offset: %s\n", BOOL_TO_STR[region_info->offset]);
    printf("behavior: %u?\n", region_info->behavior);
    printf("user_wired_count: %u\n", region_info->user_wired_count);
    printf("\n");
    fflush(stdout);
}

static void debug_task_protection(
    task_t task, mach_vm_address_t address, mach_vm_size_t vm_size
) {
    mach_port_t object_name = MACH_PORT_NULL;
    mach_msg_type_number_t region_info_size = VM_REGION_BASIC_INFO_COUNT_64;
    vm_region_basic_info_64_t region_info = alloca(sizeof(*region_info));
    expect_ok(
        mach_vm_region(
            task,
            &address,
            &vm_size,
            VM_REGION_BASIC_INFO_64,
            (vm_region_info_t)region_info,
            &region_info_size,
            &object_name
        ),
        "failed to get region info for debug"
    );
    printf("New protection:");
    print_vm_prot(region_info->protection);
}

static void print_as_hex(uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
}

void trc_mach_write_to_protected(
    const task_t task,
    const mach_vm_address_t address,
    const void* data,
    const size_t size,
    const bool revert_back
) {
    mach_port_t object_name = MACH_PORT_NULL;
    mach_msg_type_number_t region_info_size = VM_REGION_BASIC_INFO_COUNT_64;

    vm_region_basic_info_64_t region_info = alloca(sizeof(*region_info));

    mach_vm_address_t region_address = address;
    mach_vm_size_t region_size = (mach_vm_size_t)size;

    expect_ok(
        mach_vm_region(
            task,
            &region_address,
            &region_size,
            VM_REGION_BASIC_INFO_64,
            (vm_region_info_t)region_info,
            &region_info_size,
            &object_name
        ),
        "failed to get region info"
    );

    const vm_prot_t old_protection = region_info->protection;

    bool needs_to_change_protection =
        ((old_protection & VM_PROT_WRITE) == 0 ||
         (old_protection & VM_PROT_EXECUTE) != 0);

    bool executable_protection_modified = false;
    if (needs_to_change_protection) {
        vm_prot_t new_protection = 0;

        if ((old_protection & VM_PROT_EXECUTE) != 0) {
            new_protection =
                (old_protection & ~VM_PROT_EXECUTE) | VM_PROT_WRITE;
            executable_protection_modified = true;

            task_suspend(task);
        } else {
            new_protection = (old_protection | VM_PROT_WRITE);
        }

        expect_ok(
            mach_vm_protect(
                task,
                region_address,
                region_size,
                false,
                new_protection | VM_PROT_COPY
            ),
            "failed to disable write protection"
        );
    }

    expect_ok(
        mach_vm_write(
            task, address, (vm_offset_t)data, (mach_msg_type_number_t)size
        ),
        "failed to write data"
    );

    mach_vm_size_t out_size = 0;
    mach_vm_read_overwrite(
        task,
        address,
        (mach_msg_type_number_t)size,
        (vm_offset_t)data,
        &out_size
    );

    // Re-protect the region back to the way it was
    if ((revert_back || executable_protection_modified) &&
        needs_to_change_protection) {
        expect_ok(
            mach_vm_protect(
                task, region_address, region_size, false, old_protection
            ),
            "failed to enable protection back"
        );

        if (executable_protection_modified) {
            task_resume(task);
        }
    }
}

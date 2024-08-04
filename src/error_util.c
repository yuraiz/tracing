#include "error_util.h"

#include <mach/mach_error.h>
#include <stdio.h>
#include <stdlib.h>

void error(char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(1);
}

void expect_ok(kern_return_t status_code, char* failure_message) {
    if (status_code) {
        fprintf(stderr, "Status code: %s\n", mach_error_string(status_code));
        error(failure_message);
    }
}

void expect_true(int boolean, char* failure_message) {
    if (!boolean) {
        fprintf(stderr, "Status code: %i\n", boolean);
        error(failure_message);
    }
}

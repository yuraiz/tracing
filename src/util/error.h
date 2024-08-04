#ifndef TRC_ERROR_UTIL_H
#define TRC_ERROR_UTIL_H

#include <mach/mach_error.h>

void error(char* msg);

void expect_ok(kern_return_t status_code, char* failure_message);

void expect_true(int boolean, char* failure_message);

#endif  // TRC_ERROR_UTIL_H

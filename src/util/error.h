#ifndef TRC_ERROR_UTIL_H
#define TRC_ERROR_UTIL_H

#include <mach/mach_error.h>

// Simple error handlers that terminate the process.
//
// Use only if you're sure that the error never happens, or
// if it's okay to crash on the error

// Print the error message and exit.
void error(char* msg);

// If status code is not `0` print the error message and exit.
void expect_ok(kern_return_t status_code, char* failure_message);

// If the value is not `true` print the error message and exit.
void expect_true(int boolean, char* failure_message);

#endif  // TRC_ERROR_UTIL_H

#ifndef TRC_M_TASK_UTIL_H
#define TRC_M_TASK_UTIL_H

#include <mach/mach_types.h>

// Function to set up an exception handler
void trc_setup_exception_handler(task_t task, mach_port_t* exc_port);

#endif  // TRC_M_TASK_UTIL_H

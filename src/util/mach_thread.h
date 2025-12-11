#ifndef TRC_M_THREAD_UTIL_H
#define TRC_M_THREAD_UTIL_H

#include <mach/mach_types.h>

// A set of extension methods to control the execution mode.

// Enable the watch point registers, for the thread.
void trc_thread_enable_watch_point(thread_act_t thread);

void trc_thread_disable_watch_point(thread_act_t thread);

// Single step is enabled by setting a special flag.
//
// Steps by instructions, not by the code lines, as the processor
// doesn't understand what the code lines are.
// To step line by line you must get an instruction address
// from the line number and set a breakpoint to it.
void trc_thread_enable_single_step(thread_act_t thread);

void trc_thread_disable_single_step(thread_act_t thread);

#endif  // TRC_M_THREAD_UTIL_H

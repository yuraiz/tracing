#ifndef TRC_M_THREAD_UTIL_H
#define TRC_M_THREAD_UTIL_H

#include <mach/mach_types.h>

void trc_thread_enable_single_step(thread_act_t thread);

void trc_thread_disable_single_step(thread_act_t thread);

#endif  // TRC_M_THREAD_UTIL_H

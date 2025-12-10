#include <sys/_types/_mach_port_t.h>

#include "breakpoint/breakpoint_controller.h"
#include "mach/task.h"
#include "symbolication/symbolicator.h"

typedef struct {
    task_t task;
    symbolicator_t symbolicator;
    breakpoint_controller_t breakpoint_controller;
    mach_port_t exc_port;
} app_state_t;

bool init_app_state(app_state_t app_state);

app_state_t* get_app_state(void);

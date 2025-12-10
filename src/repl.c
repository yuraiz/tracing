#include <histedit.h>
#include <mach/arm/kern_return.h>
#include <mach/exc.h>
#include <mach/mach.h>
#include <mach/message.h>
#include <mach/port.h>
#include <mach/task.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/_types/_mach_port_t.h>
#include <sys/_types/_null.h>

#include "alloc/arena.h"
#include "app_state.h"
#include "breakpoint/breakpoint_controller.h"
#include "eval/eval.h"
#include "string/string.h"
#include "symbolication/symbolicator.h"
#include "util/error.h"

extern boolean_t mach_exc_server(
    mach_msg_header_t* InHeadP, mach_msg_header_t* OutHeadP
);

typedef enum {
    CMD_TY_BREAKPOINT,
    CMD_TY_DISABLE,
    CMD_TY_START,
    CMD_TY_EVAL_EXPR,
    CMD_TY_STRING,
    CMD_TY_INVALID,
} cmd_tag_t;

typedef struct {
    cmd_tag_t tag;
    union {
        string_t breakpoint;
        string_t expr;
        char* invalid;
    } value;
} cmd_t;

typedef struct {
    cmd_tag_t tag;
    string_t rest;
} parse_cmd_tag_res_t;

string_t check_prefixes(string_t str, const char** prefixes, const size_t n) {
    for (size_t i = 0; i < n; i++) {
        const char* prefix = prefixes[i];
        string_t value = string_strip_cprefix(str, prefix);
        if (value.ptr != str.ptr) {
            return value;
        }
    }
    return str;
}

parse_cmd_tag_res_t parse_cmd_tag(string_t string) {
    parse_cmd_tag_res_t result;

    string_t value;

    const char* breakpoint_pref[] = {
        "b ",
        "bp ",
        "breakpoint ",
    };

    value = check_prefixes(
        string,
        breakpoint_pref,
        sizeof(breakpoint_pref) / sizeof(*breakpoint_pref)
    );

    if (value.ptr != string.ptr) {
        result.tag = CMD_TY_BREAKPOINT;
        result.rest = value;
        return result;
    }

    const char* disable_pref[] = {
        "d ",
        "disable ",
    };

    value = check_prefixes(
        string, disable_pref, sizeof(disable_pref) / sizeof(*disable_pref)
    );

    if (value.ptr != string.ptr) {
        result.tag = CMD_TY_DISABLE;
        result.rest = value;
        return result;
    }

    const char* string_pref[] = {
        "str ",
        "string ",
    };

    value = check_prefixes(
        string, string_pref, sizeof(string_pref) / sizeof(*string_pref)
    );
    if (value.ptr != string.ptr) {
        result.tag = CMD_TY_STRING;
        result.rest = value;
        return result;
    }

    const char* start_pref[] = {
        "s",
        "start",
    };

    value = check_prefixes(
        string, start_pref, sizeof(start_pref) / sizeof(*start_pref)
    );

    if (value.ptr != string.ptr) {
        result.tag = CMD_TY_START;
        result.rest = value;
        return result;
    }

    const char* eval_pref[] = {
        "e ",
        "eval ",
    };

    value = check_prefixes(
        string, eval_pref, sizeof(eval_pref) / sizeof(*eval_pref)
    );
    if (value.ptr != string.ptr) {
        result.tag = CMD_TY_EVAL_EXPR;
        result.rest = value;
        return result;
    }

    result.tag = CMD_TY_INVALID;
    result.rest = string;
    return result;
}

cmd_t parse_command(__unused arena_t* arena, string_t string) {
    cmd_t cmd = {.tag = CMD_TY_INVALID, .value.invalid = "Parsing error"};

    parse_cmd_tag_res_t parse_cmd_tag_res = parse_cmd_tag(string);

    cmd.tag = parse_cmd_tag_res.tag;
    string = parse_cmd_tag_res.rest;

    if (cmd.tag == CMD_TY_START) {
        cmd.value.invalid = NULL;
    } else if (cmd.tag == CMD_TY_BREAKPOINT || cmd.tag == CMD_TY_DISABLE) {
        cmd.value.breakpoint = string;
    } else if (cmd.tag == CMD_TY_EVAL_EXPR || cmd.tag == CMD_TY_STRING) {
        cmd.value.expr = string;
    }

    return cmd;
}

static void cmd_resume(app_state_t* app_state) {
    kern_return_t status_code = 0;

    task_t task = app_state->task;
    mach_port_t exc_port = app_state->exc_port;

    status_code = task_resume(task);
    expect_ok(status_code, "failed to resume");

    printf("resumed the task\n");

    const mach_msg_size_t msg_size = 2048;

    status_code = mach_msg_server_once(
        mach_exc_server, msg_size, exc_port, MACH_MSG_TIMEOUT_NONE
    );
    expect_ok(status_code, "mach_msg_server() returned on error!");
    status_code = task_suspend(task);

    expect_ok(status_code, "failed to suspend");

    trc_breakpoint_controller_after_hit(&app_state->breakpoint_controller);
}

void exec_cmd(
    arena_t* arena, cmd_t cmd, app_state_t* app_state, thread_t thread
) {
    if (cmd.tag == CMD_TY_BREAKPOINT || cmd.tag == CMD_TY_DISABLE) {
        const trc_address_opt_t address_opt = trc_symbolicator_find_symbol(
            app_state->symbolicator,
            cmd.value.breakpoint,
            EMPTY_STRING,
            false,
            0,
            true
        );

        if (address_opt.is_present) {
            if (cmd.tag == CMD_TY_BREAKPOINT) {
                trc_breakpoint_controller_set_breakpoint(
                    &app_state->breakpoint_controller, address_opt.address
                );
                printf(
                    "breakpoint %s was set\n",
                    string_to_cstr(arena, cmd.value.breakpoint)
                );
            } else {
                trc_breakpoint_controller_disable_breakpoint(
                    &app_state->breakpoint_controller, address_opt.address
                );
                printf(
                    "breakpoint %s was disabled\n",
                    string_to_cstr(arena, cmd.value.breakpoint)
                );
            }
        } else {
            printf(
                "breakpoint %s wasn't found\n",
                string_to_cstr(arena, cmd.value.breakpoint)
            );
        }
    } else if (cmd.tag == CMD_TY_EVAL_EXPR) {
        eval(arena, cmd.value.expr, app_state, thread);
    } else if (cmd.tag == CMD_TY_STRING) {
        eval_str(arena, cmd.value.expr, app_state, thread);
    } else if (cmd.tag == CMD_TY_START) {
        cmd_resume(app_state);
    }
}

void start_repl_bp(thread_t thread) {
    using_history();

    while (1) {
        arena_t* arena = arena_alloc();

        char* input = readline("trc bp> ");

        cmd_t cmd = parse_command(arena, cstr_to_string(input));

        if (cmd.tag == CMD_TY_START) {
            add_history(input);
            return;
        } else if (cmd.tag == CMD_TY_INVALID) {
            printf("invalid tag\n");
        } else {
            add_history(input);

            exec_cmd(arena, cmd, get_app_state(), thread);
        }

        // Free buffer that was allocated by readline
        free(input);
        arena_release(arena);
    }
}

void start_repl(thread_t thread_id) {
    start_repl_bp(thread_id);
    while (1) {
        cmd_resume(get_app_state());
    }
}

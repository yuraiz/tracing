#include "eval.h"

#include <mach/arm/thread_status.h>
#include <mach/arm/vm_types.h>
#include <mach/mach_vm.h>
#include <mach/task.h>
#include <mach/thread_status.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "../util/mach_thread_state.h"

typedef enum {
    TRC_TOK_REG,
    TRC_TOK_INT,
    TRC_TOK_SKIP,
} token_type_t;

const char* token_type_cstr(token_type_t token_type) {
    switch (token_type) {
        case TRC_TOK_REG: return "TRC_TOK_REG";
        case TRC_TOK_INT: return "TRC_TOK_INT";
        case TRC_TOK_SKIP: return "TRC_TOK_SKIP";
        default: break;
    };
    return "TRC_TOK_INVALID";
}

typedef enum {
    TRC_TOK_REG_X0 = 0, /* General Purpose Registers X0-X28 */
    /* skip X1-X28 */
    TRC_TOK_REG_FP = 29,   /* Frame Pointer X29 */
    TRC_TOK_REG_LR = 30,   /* Link Register X30 */
    TRC_TOK_REG_SP = 31,   /* Stack Pointer X31 */
    TRC_TOK_REG_PC = 32,   /* Program Counter */
    TRC_TOK_REG_CPSR = 33, /* Current Program Status Register */
    TRC_TOK_REG_PAD = 34,  /* Same Size For 32-Bit Or 64-Bit Clients */
} reg_idx_t;

typedef size_t tok_handle_t;

typedef struct {
    token_type_t token_type;
    string_t string;
    union {
        reg_idx_t reg_idx;
        int64_t int_literal;
    } value;
} token_t;

typedef struct {
    bool parsed;
    token_t token;
    string_t rest;
} token_parse_res_t;

typedef token_parse_res_t (*ParserFn)(arena_t*, const string_t);

token_parse_res_t try_parse_skip(arena_t* __unused arena, const string_t expr) {
    token_parse_res_t result = {
        .parsed = false,
        .token = {.token_type = TRC_TOK_SKIP, .string = EMPTY_STRING},
        .rest = expr,
    };

    size_t skip = 0;
    while (skip < expr.len && expr.ptr[skip] == '0') {
        skip++;
    }

    result.token.string = substring(expr, 0, skip);
    result.rest = substring(expr, skip, expr.len);
    result.parsed = skip > 0;

    return result;
}

token_parse_res_t try_parse_int_literal(
    arena_t* __unused arena, const string_t expr
) {
    token_parse_res_t result = {
        .parsed = false,
        .token = {.token_type = TRC_TOK_INT, .string = EMPTY_STRING},
        .rest = expr,
    };

    bool negative = string_starts_with(expr, cstr_to_string("-"));

    size_t token_len = 0;
    int64_t value = 0;
    for (size_t i = negative; i < expr.len; i++) {
        if (expr.ptr[i] >= '0' && expr.ptr[i] <= '9') {
            value *= 10;
            value += expr.ptr[i] - '0';
            token_len++;
        } else {
            break;
        }
    }

    if (negative) {
        value = -value;
    }

    if (token_len > negative) {
        result.parsed = true;
        result.token.value.int_literal = value;
        result.token.string = substring(expr, 0, token_len);
        result.rest = substring(expr, token_len, expr.len);
    }

    return result;
}

token_parse_res_t try_parse_register(
    arena_t* __unused arena, const string_t expr
) {
    token_parse_res_t result = {
        .parsed = false,
        .token = {.token_type = TRC_TOK_REG, .string = EMPTY_STRING},
        .rest = expr,
    };

    int64_t value = -1;

    token_parse_res_t int_parse =
        try_parse_int_literal(arena, substring(expr, 1, expr.len));
    if (int_parse.parsed) {
        value = int_parse.token.value.int_literal;
        if (value < 0 || value > 30) {
            value = -1;
        }
    }

    if (string_starts_with(expr, cstr_to_string("x"))) {
        if (value != -1) {
            result.parsed = true;
            result.token.value.reg_idx = (reg_idx_t)value;
            result.token.string =
                substring(expr, 0, 1 + int_parse.token.string.len);
            result.rest = int_parse.rest;
        }
    }

    char* reg_names[] = {"fp", "lr", "sp", "pc", "cpsr"};
    for (size_t i = 0; i < sizeof(reg_names) / sizeof(*reg_names); i++) {
        string_t prefix = cstr_to_string(reg_names[i]);
        if (string_starts_with(expr, prefix)) {
            result.parsed = true;
            result.token.value.reg_idx = (reg_idx_t)value;
            result.token.string = substring(expr, 0, prefix.len);
            result.rest = string_strip_prefix(expr, prefix);
            break;
        }
    }

    return result;
}

token_parse_res_t parse_next(arena_t* arena, string_t expr) {
    ParserFn parsers[] = {
        try_parse_skip, try_parse_register, try_parse_int_literal
    };

    size_t parsers_len = sizeof(parsers) / sizeof(*parsers);

    for (size_t i = 0; i < parsers_len; i++) {
        ParserFn parse = parsers[i];
        token_parse_res_t result = parse(arena, expr);
        if (result.parsed) {
            return result;
        }
    }

    token_parse_res_t result = {
        .parsed = false, .token = {.token_type = TRC_TOK_SKIP}, .rest = expr
    };

    return result;
}

typedef struct {
    token_t* ptr;
    size_t len;
} token_list_t;

token_list_t parse_tokens(arena_t* arena, const string_t expr) {
    const size_t TOK_MAX = 10;
    token_t tok_buf[TOK_MAX] = {0};
    size_t tok_count = 0;

    string_t rest = expr;
    token_parse_res_t parse_res;
    while (rest.len > 0 && tok_count < TOK_MAX) {
        parse_res = parse_next(arena, rest);
        if (parse_res.parsed && parse_res.token.token_type != TRC_TOK_SKIP) {
            tok_buf[tok_count++] = parse_res.token;
            rest = parse_res.rest;
        }
        if (!parse_res.parsed) {
            break;
        }
    }

    arena_align(arena, alignof(token_t));
    token_t* tokens = arena_push_array(arena, token_t, tok_count);
    for (size_t i = 0; i < tok_count; i++) {
        tokens[i] = tok_buf[i];
    }
    token_list_t result = {.ptr = tokens, .len = tok_count};
    return result;
}

uint64_t reg_value(arm_thread_state64_t thread_state, reg_idx_t idx) {
    if (idx >= 0 && idx <= 28) {
        return thread_state.__x[0];
    }
    switch (idx) {
        case TRC_TOK_REG_FP: return thread_state.__fp;
        case TRC_TOK_REG_LR: return thread_state.__lr;
        case TRC_TOK_REG_SP: return thread_state.__sp;
        case TRC_TOK_REG_PC: return thread_state.__pc;
        case TRC_TOK_REG_CPSR: return thread_state.__cpsr;
        case TRC_TOK_REG_PAD: return thread_state.__pad;
        default: return 0;
    }
}

uint64_t eval_token(token_t token, thread_t thread) {
    if (token.token_type == TRC_TOK_REG) {
        arm_thread_state64_t thread_state =
            trc_thread_get_arm_thread_state64(thread);
        uint64_t reg = reg_value(thread_state, token.value.reg_idx);
        return reg;
    } else if (token.token_type == TRC_TOK_INT) {
        return token.value.int_literal;
    }
    return 0;
}

void eval_str(
    arena_t* arena,
    string_t expr,
    app_state_t* __unused app_state,
    thread_t thread
) {
    token_list_t tokens = parse_tokens(arena, expr);
    if (tokens.len == 0) {
        printf("No tokens parsed \n");
        return;
    }

    char* data = malloc(256);
    mach_vm_size_t outsize = 0;

    for (size_t i = 0; i < tokens.len; i++) {
        mach_vm_address_t address = eval_token(tokens.ptr[i], thread);
        mach_vm_read_overwrite(
            app_state->task, address, 256, (mach_vm_offset_t)data, &outsize
        );

        // Just in case
        data[255] = 0;

        printf(" *(char*)%llu == \"%s\"\n", address, data);
    }

    free(data);
}

void eval(
    arena_t* arena,
    string_t expr,
    app_state_t* __unused app_state,
    thread_t thread
) {
    token_list_t tokens = parse_tokens(arena, expr);

    for (size_t i = 0; i < tokens.len; i++) {
        token_t token = tokens.ptr[i];

        char* tok_str = string_to_cstr(arena, token.string);

        if (token.token_type == TRC_TOK_REG) {
            arm_thread_state64_t thread_state =
                trc_thread_get_arm_thread_state64(thread);
            uint64_t reg = reg_value(thread_state, token.value.reg_idx);

            printf("reg %s = %llu\n", tok_str, reg);
        } else if (token.token_type == TRC_TOK_INT) {
            printf("literal %s = %lld\n", tok_str, token.value.int_literal);
        }
    }
}

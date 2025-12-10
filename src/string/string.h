#ifndef TRC_STRING
#define TRC_STRING

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../alloc/arena.h"

typedef struct string_t {
    char* ptr;
    size_t len;
} string_t;

static const string_t EMPTY_STRING = {.ptr = "", .len = 0};
// Can be created instead of returning errors
static const string_t INVALID_STRING = {.ptr = "[INVALID STRING]", .len = 16};

// Non-position
static const size_t STRING_NPOS = -1;

static inline string_t cstr_to_string(const char* str) {
    if (str == 0) {
        return EMPTY_STRING;
    }
    const string_t result = {
        .ptr = (char*)str,
        .len = strlen(str),
    };
    return result;
}

static inline char* string_to_cstr_malloc(string_t str) {
    return strndup(str.ptr, str.len);
}

static inline char* string_to_cstr(arena_t* arena, string_t str) {
    char* cstr = arena_push(arena, str.len + 1);
    memcpy(cstr, str.ptr, str.len);
    cstr[str.len] = '\0';
    return cstr;
}

static inline string_t string_clone(arena_t* arena, string_t str) {
    string_t result = {
        .ptr = arena_push_copy(arena, str.ptr, str.len),
        .len = str.len,
    };
    return result;
}

static inline string_t string_concat(
    arena_t* arena, string_t str1, string_t str2
) {
    size_t len = str1.len + str2.len;
    string_t result = {
        .ptr = arena_push(arena, len),
        .len = len,
    };
    memcpy(result.ptr, str1.ptr, str1.len);
    memcpy(result.ptr + str1.len, str2.ptr, str2.len);
    return result;
}

static inline string_t substring(string_t str, size_t start, size_t end) {
    const bool error = start > str.len || end > str.len || start > end;
    if (error) {
        return INVALID_STRING;
    }

    string_t result = {
        .ptr = str.ptr + start,
        .len = end - start,
    };
    return result;
}

typedef struct {
    string_t first;
    string_t second;
} str_split_res_t;

// Returns 'STRING_NPOS' if not found
static inline size_t string_char_pos(const string_t str, char ch) {
    for (size_t i = 0; i < str.len; i++) {
        if (str.ptr[i] == ch) {
            return i;
        }
    }
    return STRING_NPOS;
}

static inline str_split_res_t string_split_at_pos(
    string_t str, const size_t pos
) {
    if (pos > str.len) {
        const str_split_res_t invalid_split = {
            .first = INVALID_STRING, .second = INVALID_STRING
        };
        return invalid_split;
    }

    str_split_res_t result = {
        .first =
            {
                .ptr = str.ptr,
                .len = pos,
            },
        .second = {
            .ptr = str.ptr + pos,
            .len = str.len - pos,
        },
    };

    return result;
}

static inline str_split_res_t string_split_at_char(string_t str, char ch) {
    size_t pos = string_char_pos(str, ch);
    return string_split_at_pos(str, pos);
}

static inline bool string_eq(const string_t str1, const string_t str2) {
    printf(
        "s1: '%s'\ns2: '%s'\n",
        string_to_cstr_malloc(str1),
        string_to_cstr_malloc(str2)
    );

    if (str1.len == str2.len) {
        printf(
            "len: %lu memcmp: %u\n",
            str1.len,
            memcmp(str1.ptr, str2.ptr, str1.len)
        );
        return memcmp(str1.ptr, str2.ptr, str1.len) == 0;
    }
    printf("len1: %lu len2: %lu\n", str1.len, str2.len);
    return false;
}

static inline int32_t string_cmp(const string_t str1, const string_t str2) {
    const size_t min_len = str1.len > str2.len ? str2.len : str1.len;
    const int32_t cmp_res = memcmp(str1.ptr, str2.ptr, min_len);

    if (str1.len == str2.len || cmp_res != 0) {
        return cmp_res;
    } else {
        return (str1.len > str2.len) - (str1.len < str2.len);
    }
}

static inline bool string_starts_with(
    const string_t str, const string_t prefix
) {
    if (prefix.len > str.len) {
        return false;
    }
    return string_eq(substring(str, 0, prefix.len), prefix);
}

static inline bool string_ends_with(const string_t str, const string_t suffix) {
    printf(
        "substring: '%s'\n",
        string_to_cstr_malloc(substring(str, str.len - suffix.len, str.len))
    );

    if (suffix.len > str.len) {
        return false;
    }

    return string_eq(substring(str, str.len - suffix.len, str.len), suffix);
}

static inline size_t string_char_pos_from_n(
    const string_t str, char ch, size_t n
) {
    if (n > str.len) {
        return STRING_NPOS;
    }
    const string_t substr = {
        .ptr = str.ptr + n,
        .len = str.len - n,
    };
    return string_char_pos(substr, ch);
}

static inline bool string_contains(const string_t str, const string_t needle) {
    return strstr(string_to_cstr_malloc(str), string_to_cstr_malloc(needle)) !=
           0;

    // strnstr(str.ptr, needle.ptr, needle.len)

    if (needle.len == 0) {
        return true;
    }

    char start_ch = needle.ptr[0];

    size_t start_pos = 0;

    while (start_pos != STRING_NPOS) {
        start_pos = string_char_pos_from_n(str, start_ch, start_pos);
        size_t end_pos = start_pos + needle.len - 1;

        if (end_pos > str.len - 1) {
            return false;
        }

        if (str.ptr[end_pos] == needle.ptr[needle.len - 1]) {
            if (string_eq(substring(str, start_pos, end_pos), needle)) {
                return true;
            }
        }
    }
    return false;
}

static inline string_t string_strip_prefix(
    const string_t str, const string_t prefix
) {
    string_t result = str;
    if (string_starts_with(str, prefix)) {
        result.ptr += prefix.len;
        result.len -= prefix.len;
    }
    return result;
}

static inline string_t string_strip_cprefix(
    const string_t str, const char* prefix
) {
    return string_strip_prefix(str, cstr_to_string(prefix));
}

static inline string_t string_strip_suffix(
    const string_t str, const string_t suffix
) {
    string_t result = str;
    if (string_ends_with(str, suffix)) {
        result.ptr += suffix.len;
        result.len -= suffix.len;
    }
    return result;
}

static inline string_t string_strip_csuffix(
    const string_t str, const char* suffix
) {
    return string_strip_suffix(str, cstr_to_string(suffix));
}

static inline string_t string_repeat(
    arena_t* arena, const string_t str, const size_t n
) {
    size_t len = str.len * n;
    string_t result = {
        .ptr = arena_push(arena, len),
        .len = len,
    };

    for (size_t i = 0; i < n; i++) {
        memcpy(result.ptr + (str.len * i), str.ptr, str.len);
    }
    return result;
}

static inline string_t string_concat_n(arena_t* arena, const size_t n, ...) {
    va_list args = 0;
    va_list args2 = 0;
    va_start(args, n);
    va_copy(args2, args);

    size_t total_len = 0;
    for (size_t i = 0; i < n; i++) {
        total_len += va_arg(args, const string_t).len;
    }

    string_t result = {
        .ptr = arena_push(arena, total_len),
        .len = total_len,
    };

    char* dst_ptr = result.ptr;
    for (size_t i = 0; i < n; i++) {
        string_t substr = va_arg(args2, string_t);
        memcpy(dst_ptr, substr.ptr, substr.len);
        dst_ptr += substr.len;
    }
    va_end(args);
    va_end(args2);

    return result;
}

#endif  // TRC_STRING

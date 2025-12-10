#include "app_state.h"

#include <stdatomic.h>

static atomic_bool APP_STATE_IS_SET = 0;

static app_state_t APP_STATE = {0};

bool init_app_state(app_state_t app_state) {
    bool expected = false;
    const bool was_set = atomic_compare_exchange_strong_explicit(
        &APP_STATE_IS_SET,
        &expected,
        true,
        memory_order_acquire,
        memory_order_relaxed
    );

    if (was_set) {
        APP_STATE = app_state;
    }

    return was_set;
}

app_state_t* get_app_state(void) {
    bool is_set = atomic_load_explicit(&APP_STATE_IS_SET, memory_order_acquire);
    if (is_set) {
        return &APP_STATE;
    } else {
        return NULL;
    }
}

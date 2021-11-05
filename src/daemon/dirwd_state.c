/**
 * @file dirwd_state.h
 * @date 3 Nov 2021
 * @brief Directory watchdog Linux daemon state structure and methods
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../util/fentry.h"
#include "dirwd_state.h"

dirwd_status_t dirwd_state_set(struct dirwd_state_t* state, const char* target_dir, uint32_t timeout) {
    assert(state != NULL);

    if ((target_dir == NULL) || (strlen(target_dir) == 0)) {
        return DIRWD_INVALID_CONFIG_TARGET_DIR;
    }

    state->target_dir = (char*) malloc((strlen(target_dir) + 1) * sizeof(char));
    strcpy(state->target_dir, target_dir);
    state->entries = fentry_set_new();
    state->timeout_sec = timeout;

    return DIRWD_SUCCESS;
}

dirwd_status_t dirwd_state_clean(struct dirwd_state_t* state) {
    assert(state != NULL);

    free(state->target_dir);
    fentry_set_drop(&state->entries);

    return DIRWD_SUCCESS;
}
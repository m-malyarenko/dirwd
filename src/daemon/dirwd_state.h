/**
 * @file dirwd_state.h
 * @date 3 Nov 2021
 * @brief Directory watchdog Linux daemon state structure and methods
 */

#ifndef __DAEMON_DIRWD_STATE_H__
#define __DAEMON_DIRWD_STATE_H__

#include <stdint.h>

#include <sys/stat.h>

#include "dirwd_status.h"
#include "../util/fentry.h"

/* Define -------------------------------------------------------------------*/

/* Constants ----------------------------------------------------------------*/

/* Structures ---------------------------------------------------------------*/

struct dirwd_state_t {
    char* target_dir;
    struct fentry_set_t* entries;
    uint16_t timeout_sec;
};

/* Function definitions -----------------------------------------------------*/

dirwd_status_t dirwd_state_set(struct dirwd_state_t* state, const char* target_dir, uint32_t timeout);

dirwd_status_t dirwd_state_clean(struct dirwd_state_t* state);

#endif /* __DAEMON_DIRWD_STATE_H__ */
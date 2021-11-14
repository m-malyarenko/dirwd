/**
 * @file dirwd.h
 * @date 1 Nov 2021
 * @brief Directory watchdog Linux daemon
 */

#ifndef __DAEMON_DIRWD_H__
#define __DAEMON_DIRWD_H__

#include <stdint.h>

#include "dirwd_status.h"
#include "dirwd_config.h"
#include "dirwd_state.h"

/* Define -------------------------------------------------------------------*/

/* Constants ----------------------------------------------------------------*/

/* Structures ---------------------------------------------------------------*/

/* Function definitions -----------------------------------------------------*/

dirwd_status_t dirwd_exec();

dirwd_status_t dirwd_init(const char* config_path, struct dirwd_state_t* cur_state);

void dirwd_inspect(struct dirwd_state_t* cur_state);

void dirwd_scan_dir(struct fentry_set_t* entries, const char* path);

void dirwd_log_error(const dirwd_status_t err);

void dirwd_log_diff(
    struct fentry_set_t* new_entries,
    struct fentry_set_t* deleted_entries,
    struct fentry_set_t* modified_entries
);

void dirwd_sigterm_handler(int sig);

void dirwd_sighup_handler(int sig);

#endif /* __DAEMON_DIRWD_H__ */
/**
 * @file dirwd_config.h
 * @date 14 Nov 2021
 * @brief Directory watchdog Linux daemon configuration module
 */

#ifndef __DIRWD_CONFIG_H__
#define __DIRWD_CONFIG_H__

#include <stdint.h>

#include "dirwd_status.h"
#include "dirwd_state.h"

/* Define -------------------------------------------------------------------*/

#define MAX_DIR_NAME_LEN    ((size_t) 128)
#define STRING_BUFFER_SIZE  ((size_t) 256)

#define MAX_TIMEOUT ((uint64_t) 3600)
#define MIN_TIMEOUT ((uint64_t) 10)

#define QUOTE_SYMBOL '"'
#define SHIELD_SYMBOL '\\'

/* Constants ----------------------------------------------------------------*/

/* Structures ---------------------------------------------------------------*/

struct dirwd_config_t {
    char* target_dir;
    size_t timeout_sec;
};

/* Function definitions -----------------------------------------------------*/

dirwd_status_t dirwd_config_read(const char* path, struct dirwd_config_t* config_buf);

dirwd_status_t dirwd_config_assert(const struct dirwd_config_t* config);

dirwd_status_t dirwd_config_setup(const struct dirwd_config_t* config, struct dirwd_state_t* cur_state);

dirwd_status_t dirwd_config_tokenize(const char* config_string, char* target_dir_buf, size_t* timeout_buf);

#endif /* __DIRWD_CONFIG_H__ */
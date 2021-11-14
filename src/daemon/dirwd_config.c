/**
 * @file dirwd_config.c
 * @date 14 Nov 2021
 * @brief Directory watchdog Linux daemon configuration module
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <sys/unistd.h>
#include <sys/stat.h>

#include "dirwd_config.h"

dirwd_status_t dirwd_config_read(const char* path, struct dirwd_config_t* config_buf) {
    config_buf->target_dir = NULL;
    config_buf->timeout_sec = 0;
    
    /* Assert parametrs */
    assert(path != NULL);
    assert(config_buf != NULL);

    FILE* const fin = fopen(path, "r");

    if (fin == NULL) {
        return DIRWD_FAILED_TO_OPEN_CONFIG;
    }

    /* Raed configuration from file */
    char file_string_buffer[STRING_BUFFER_SIZE] = { 0 };
    char config_string_buffer[STRING_BUFFER_SIZE] = { 0 };
    bool config_string_found = false;
    bool config_redefinition = false;

    while (fgets(file_string_buffer, STRING_BUFFER_SIZE, fin) != NULL) {
        /* Replace newline character if any */
        char* newline_char_ptr = strchr(file_string_buffer, '\n');
        if (newline_char_ptr != NULL) {
            *newline_char_ptr = '\0';
        }

        if (strlen(file_string_buffer) == 0) {
            continue;
        } else if (!config_string_found) {
            config_string_found = true;
            strcpy(config_string_buffer, file_string_buffer);
        } else if (config_string_found) {
            config_redefinition = true;
            break;
        }
    }

    if (ferror(fin) != 0) {
        fclose(fin);
        return DIRWD_FAILED_TO_READ_CONFIG;
    } else if (!config_string_found || config_redefinition) {
        fclose(fin);
        return DIRWD_INVALID_CONFIG_FORMAT;
    } else if (feof(fin) == 0) {
        fclose(fin);
        return DIRWD_FAILED_TO_READ_CONFIG;
    }

    char* token = NULL;
    char target_dir_string[STRING_BUFFER_SIZE] = { 0 };
    char timeout_string[STRING_BUFFER_SIZE] = { 0 };

    /* Parse target dir */
    if ((token = strtok(config_string_buffer, " ")) != NULL) {
        strcpy(target_dir_string, token);
    }

    /* Parse timeout string representation */
    if ((token = strtok(NULL, " ")) != NULL) {
        strcpy(timeout_string, token);
    }

    if (((token = strtok(NULL, " ")) != NULL)
        || (strlen(target_dir_string) == 0)
        || (strlen(timeout_string) == 0))
    {
        fclose(fin);
        return DIRWD_INVALID_CONFIG_FORMAT;
    }

    const int64_t timeout_parsed = strtol(timeout_string, NULL, 10);
    if ((timeout_parsed <= 0) || (errno == ERANGE)) {
        fclose(fin);
        return DIRWD_INVALID_CONFIG_TIMEOUT;
    }

    uint16_t timeout_sec = (uint16_t) timeout_parsed;
    char* target_dir = (char*) malloc((strlen(target_dir_string) + 1) *sizeof(char));
    strcpy(target_dir,target_dir_string);

    /* Copy configuration parameters to the buffer structure */
    config_buf->target_dir = target_dir;
    config_buf->timeout_sec = timeout_sec;

    fclose(fin);
    return DIRWD_SUCCESS;
}

dirwd_status_t dirwd_config_assert(const struct dirwd_config_t* config) {
    assert(config != NULL);

    /* Assert taget directory */
    struct stat target_dir_stat;
    if (stat(config->target_dir, &target_dir_stat) != 0) {
        return DIRWD_INVALID_CONFIG_TARGET_DIR;
    } else if (!S_ISDIR(target_dir_stat.st_mode)) {
        return DIRWD_TARGET_NOT_DIR;
    }

    /* Assert target dir permissions */
    if ((target_dir_stat.st_mode & X_OK) == 0) {
        return DIRWD_FAILED_TO_OPEN_TARGET_DIR;
    } else if ((target_dir_stat.st_mode & R_OK) == 0) {
        return DIRWD_FAILED_TO_READ_TARGET_DIR;
    }

    /* Assert timeout */
    if ((config->timeout_sec < MIN_TIMEOUT) || (config->timeout_sec > MAX_TIMEOUT)) {
        return DIRWD_INVALID_CONFIG_TIMEOUT;
    }

    return DIRWD_SUCCESS;
}

dirwd_status_t dirwd_config_setup(const struct dirwd_config_t* config, struct dirwd_state_t* cur_state) {
    assert(config != NULL);
    assert(cur_state != NULL);

    dirwd_status_t status = 0;

    status = dirwd_state_clean(cur_state);
    if (status != DIRWD_SUCCESS) {
        return status;
    }

    status = dirwd_state_set(cur_state, config->target_dir, config->timeout_sec);
    if (status != DIRWD_SUCCESS) {
        return status;
    }

    return DIRWD_SUCCESS;
}
/**
 * @file dirwd.c
 * @date 1 Nov 2021
 * @brief Directory watchdog Linux daemon
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <sys/unistd.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <dirent.h>

#include "../dirwd_config.h"
#include "dirwd_status.h"
#include "dirwd_state.h"
#include "dirwd.h"

static struct dirwd_state_t state;

dirwd_status_t dirwd_exec() {
    /* Initialize daemon with current configuration */
    const dirwd_status_t status = dirwd_init(DIRWD_CONFIG_PATH, &state);

    if (status == DIRWD_SUCCESS) {
        syslog(LOG_INFO, "Daemon initialized successfully");
    } else {
        syslog(LOG_ERR, "Failed to initialize daemon");
        return DIRWD_FAILURE;
    }

    /* Main loop */
    while (true) {
        dirwd_inspect(&state);
        sleep(state.timeout_sec);
    }

    dirwd_state_clean(&state);
    return DIRWD_SUCCESS;
}

dirwd_status_t dirwd_init(const char* config_path, struct dirwd_state_t* cur_state) {
    assert(config_path != NULL);
    dirwd_status_t status = 0;

    /* Raed daemon configuration */
    struct dirwd_config_t config;
    status = dirwd_config_read(config_path, &config);

    if (status == DIRWD_SUCCESS) {
        syslog(LOG_DEBUG, "Configuration read");
    } else {
        dirwd_log_error(status);
        return DIRWD_FAILURE;
    }

    /* Assert configureation parameters */
    status = dirwd_config_assert(&config);

    if (status == DIRWD_SUCCESS) {
        syslog(LOG_DEBUG, "Configuration format asserted");
    } else {
        dirwd_log_error(status);
        free(config.target_dir);
        return DIRWD_FAILURE;
    }

    /* Setup daemon with configuration */
    status = dirwd_config_setup(&config, cur_state);

    if (status == DIRWD_SUCCESS) {
        syslog(LOG_DEBUG, "Configuration set up");
    } else {
        dirwd_log_error(status);
        free(config.target_dir);
        return DIRWD_FAILURE;
    }

    syslog(LOG_INFO,
        "Current configuration: target directory '%s' timeout: %u seconds",
        config.target_dir,
        config.timeout_sec
    );

    free(config.target_dir);
    return DIRWD_SUCCESS;
}

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

void dirwd_inspect(struct dirwd_state_t* cur_state) {
    assert(cur_state != NULL);

    struct fentry_set_t* old_state_entries = cur_state->entries;
    struct fentry_set_t* new_state_entries = fentry_set_new();
    dirwd_scan_dir(new_state_entries, cur_state->target_dir);

    struct fentry_set_t* new_entries = fentry_set_diff(new_state_entries, old_state_entries);
    struct fentry_set_t* deleted_entries = fentry_set_diff(old_state_entries, new_state_entries);
    struct fentry_set_t* modified_entries = fentry_set_new();

    /* Find modiefies entries */
    const size_t old_entries_num = fentry_set_len(old_state_entries);

    for (size_t i = 0; i < old_entries_num; i++) {
        struct fentry_t* old_file = fentry_set_pop(old_state_entries);
        
        if (fentry_set_contains(new_state_entries, old_file->file_name)) {
            const struct fentry_t* new_file = fentry_set_get(new_state_entries, old_file->file_name);
            if (!fentry_equals(old_file, new_file)) {
                fentry_set_insert(modified_entries, fentry_clone(old_file));
            }
        }

        fentry_drop(&old_file);
    }

    dirwd_log_diff(new_entries, deleted_entries, modified_entries);

    fentry_set_drop(&new_entries);
    fentry_set_drop(&deleted_entries);
    fentry_set_drop(&modified_entries);

    fentry_set_drop(&cur_state->entries);
    cur_state->entries = new_state_entries;
}

void dirwd_scan_dir(struct fentry_set_t* entries, const char* path) {
    if ((entries == NULL) || (path == NULL)) {
        return;
    }

    char file_path_buffer[STRING_BUFFER_SIZE] = { 0 };
    const size_t path_len = strlen(path);
    strcpy(file_path_buffer, path);

    DIR* const dir = opendir(path);

    if (dir == NULL) {
        syslog(LOG_ERR, "Failed to open directory '%s': %s", path, strerror(errno));
        return;
    }

    struct dirent* dir_entry = NULL;
    struct stat file_stat = { 0 };

    while ((dir_entry = readdir(dir)) != NULL) {
        /* Check if entry is not . or .. directory */
        const bool is_entry_current_dir = strcmp(dir_entry->d_name, ".") == 0;
        const bool is_entry_parent_dir = strcmp(dir_entry->d_name, "..") == 0;
        if (is_entry_current_dir || is_entry_parent_dir) {
            continue;
        }

        /* Get file full path */
        file_path_buffer[path_len] = '/';
        file_path_buffer[path_len + 1] = '\0';
        strcat(file_path_buffer, dir_entry->d_name);

        /* Get file metadata */
        if (stat(file_path_buffer, &file_stat) != 0) {
            syslog(LOG_ERR,
                "Failed to read metadata of file '%s': %s",
                file_path_buffer,
                strerror(errno)
            );
            continue;
        }

        if (S_ISDIR(file_stat.st_mode)) {
            /* If file isdirectory - scan directory recursively */
            dirwd_scan_dir(entries, file_path_buffer);
        } else {
            /* If file is not directory - insert file entry to the set */
            fentry_set_insert(entries, fentry_new(file_path_buffer, &file_stat));
        }
    }

    closedir(dir);
}

void dirwd_log_error(const dirwd_status_t err) {
    switch (err) {
    case DIRWD_FAILED_TO_OPEN_CONFIG:
        syslog(LOG_ERR, "Failed to open configuration file: %s.", strerror(errno));
        break;
    case DIRWD_FAILED_TO_READ_CONFIG:
        syslog(LOG_ERR, "Failed to read configuration file: %s.", strerror(errno));
        break;
    case DIRWD_INVALID_CONFIG_FORMAT:
        syslog(LOG_ERR, "Invalid configuration file format.");
        break;
    case DIRWD_INVALID_CONFIG_PATH:
        syslog(LOG_ERR, "Invalid configuration file path.");
        break;
    case DIRWD_INVALID_CONFIG_TIMEOUT:
        syslog(LOG_ERR, "Invalid timeout parameter.");
        break;
    case DIRWD_INVALID_CONFIG_TARGET_DIR:
        syslog(LOG_ERR, "Invalid target directory parameter.");
        break;
    case DIRWD_TARGET_NOT_DIR:
        syslog(LOG_ERR, "Target is not a directory.");
        break;
    case DIRWD_FAILED_TO_OPEN_TARGET_DIR:
        syslog(LOG_ERR, "Failed to open target dir: %s.", strerror(errno));
        break;
    case DIRWD_FAILED_TO_READ_TARGET_DIR:
        syslog(LOG_ERR, "Failed to read target dir: %s.", strerror(errno));
        break;
    default:
        syslog(LOG_DEBUG, "Unhandled dirwd error status.");
        break;
    }
}

void dirwd_log_diff(
    struct fentry_set_t* new_entries,
    struct fentry_set_t* deleted_entries,
    struct fentry_set_t* modified_entries
)
{
    const size_t new_entries_mum = fentry_set_len(new_entries);
    for (size_t i = 0; i < new_entries_mum; i++) {
        struct fentry_t* entry = fentry_set_pop(new_entries);
        syslog(LOG_INFO,
            "NEW: '%s'",
            entry->file_name
        );
        fentry_drop(&entry);
    }

    const size_t deleted_entries_mum = fentry_set_len(deleted_entries);
    for (size_t i = 0; i < deleted_entries_mum; i++) {
        struct fentry_t* entry = fentry_set_pop(deleted_entries);
        syslog(LOG_INFO,
            "DELETED: '%s'",
            entry->file_name
        );
        fentry_drop(&entry);
    }

    const size_t modified_entries_mum = fentry_set_len(modified_entries);
    for (size_t i = 0; i < modified_entries_mum; i++) {
        struct fentry_t* entry = fentry_set_pop(modified_entries);
        syslog(LOG_INFO,
            "MODIFIED: '%s'",
            entry->file_name
        );
        fentry_drop(&entry);
    }
}

void dirwd_sigterm_handler(int sig) {
    if (sig == SIGTERM) {
        syslog(LOG_INFO, "SIGTERM signal is received. Daemon is shutting down...");
        syslog(LOG_INFO, "Deamon finished successfully");
        dirwd_state_clean(&state);
        exit(EXIT_SUCCESS);
    }
}

void dirwd_sighup_handler(int sig) {
    if (sig == SIGHUP) {
        syslog(LOG_INFO, "SIGHUP signal is received. Refreshing configuration...");
        const dirwd_status_t status = dirwd_init(DIRWD_CONFIG_PATH, &state);

        if (status == DIRWD_SUCCESS) {
            syslog(LOG_INFO, "Daemon initialized successfully.");
        } else {
            syslog(LOG_ERR, "Failed to initialize daemon. Previous configuration kept.");
        }
    }

    signal(SIGHUP, dirwd_sighup_handler);
}

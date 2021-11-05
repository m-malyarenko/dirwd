/**
 * @file dirwd_status.h
 * @date 1 Nov 2021
 * @brief Directory watchdog Linux daemon status
 */

#ifndef __DAEMON_DIRWD_STATUS_H__
#define __DAEMON_DIRWD_STATUS_H__

#include <stdint.h>

typedef uint8_t dirwd_status_t;

#define DIRWD_SUCCESS                   ((dirwd_status_t) 0)
#define DIRWD_FAILURE                   ((dirwd_status_t) 1)

#define DIRWD_FAILED_TO_OPEN_CONFIG     ((dirwd_status_t) 10)
#define DIRWD_FAILED_TO_READ_CONFIG     ((dirwd_status_t) 11)
#define DIRWD_INVALID_CONFIG_FORMAT     ((dirwd_status_t) 12)
#define DIRWD_INVALID_CONFIG_PATH       ((dirwd_status_t) 13)
#define DIRWD_INVALID_CONFIG_TIMEOUT    ((dirwd_status_t) 14)
#define DIRWD_INVALID_CONFIG_TARGET_DIR ((dirwd_status_t) 15)
#define DIRWD_TARGET_NOT_DIR            ((dirwd_status_t) 16)

#define DIRWD_FAILED_TO_OPEN_TARGET_DIR ((dirwd_status_t) 20)
#define DIRWD_FAILED_TO_READ_TARGET_DIR ((dirwd_status_t) 21)

#endif /* __DIRWD_STATUS_H__ */
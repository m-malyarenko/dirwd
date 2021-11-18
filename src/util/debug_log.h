/**
 * @file debug_log.h
 * @date 18 Nov 2021
 * @brief Debug syslog utility
 */

#ifndef __DEBUG_LOG_H__
#define __DEBUG_LOG_H__

#ifndef NDEBUG
/* DEBUG build type */
#include <sys/syslog.h>
#define DEBUG_LOG(message) syslog(LOG_DEBUG, message)
#else
/* RELEASE build type */
#define DEBUG_LOG(message)
#endif

#endif /* __DEBUG_LOG_H__ */

/**
 * @file dirwd_config.h
 * @date 2 Nov 2021
 * @brief dirwd daemon configuration file
 */

#ifndef __DIRWD_CONFIG_H__
#define __DIRWD_CONFIG_H__

#include <stdint.h>

#define DIRWD_SYSLOG_IDENT "dirwdd"
#define DIRWD_WORKING_DIR "/"
#define DIRWD_CONFIG_PATH "/home/michael/dirwd/dirwdd.config"
#define DIRWD_UMASK ((uint32_t) 0x0002)

#endif /* __DIRWD_CONFIG_H__ */
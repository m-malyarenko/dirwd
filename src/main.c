/**
 * @file main.c
 * @date 1 Nov 2021
 * @brief Test Linux daemon
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <sys/unistd.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/syslog.h>

#include "dirwd_config.h"
#include "daemon/dirwd.h"

int main() {
	/* Open syslog */
	openlog(DIRWD_SYSLOG_IDENT, LOG_PID, LOG_USER);

	/* Fork daemon process */
	const pid_t pid = fork();

	if (pid < 0) {
		syslog(LOG_ERR, "Failed to fork daemon from parent process");
		exit(EXIT_FAILURE);
	} else if (pid > 0) {
		syslog(LOG_DEBUG, "Daemon process forked from parent process");
		exit(EXIT_SUCCESS);
	}

	/* Set file permissions */
	umask(DIRWD_UMASK);
	syslog(LOG_DEBUG, "Daemon enviroment UMASK set");

	/* Set new session ID for daemon */
	const pid_t sid = setsid();
	if (sid < 0) {
		syslog(LOG_ERR, "failed to set daemon's SID");
		exit(EXIT_FAILURE);
	} else {
		syslog(LOG_DEBUG, "Daemon process SID set");
	}

	/* Change PWD to daemon folder */
	if (chdir(DIRWD_WORKING_DIR) < 0) {
		syslog(LOG_ERR, "failed to daemon's working directory");
		exit(EXIT_FAILURE);
	}
	syslog(LOG_DEBUG, "Daemon process PWD set");

	/* Close stdin, stdout, stderr */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	syslog(LOG_DEBUG, "Daemon process stdin, stdout, stderr closed");

	/* Set signal handlers */
	signal(SIGTERM, dirwd_sigterm_handler);
	signal(SIGHUP, dirwd_sighup_handler);
	syslog(LOG_DEBUG, "Daemon signal handlers are set");

	syslog(LOG_INFO, "Deamon started successfully");

	/* Execute daemon */
	const dirwd_status_t daemon_status = dirwd_exec();
	int exit_status = 0;

	if (daemon_status == DIRWD_SUCCESS) {
		syslog(LOG_INFO, "Deamon finished successfully");
		exit_status = EXIT_SUCCESS;
	} else {
		syslog(LOG_ERR, "Daemon finished with error");
		exit_status = EXIT_FAILURE;
	}
	closelog();

	exit(exit_status);
}

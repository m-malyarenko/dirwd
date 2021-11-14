# Directory Watchdog Linux daemon

This program is a Linux daemon that periodically recursively inspects target directory for modification
and writes inspection results to the syslog.

Daemon detects following events:
- `NEW` - new file was added to the target dir or all its subdirectories
- `DELETED` - files was deleted from the target dir or all its subdirectories
- `MODIFIED` - file in the target dir or all its subdirectories was altered

Daemon do not use any specific system library functions or any specific system library modes.

## Build configuration

Following parameters must be configured in the [project config header](src/config.h):

- `DIRWD_SYSLOG_IDENT` - Daemon syslog identificator name
- `DIRWD_WORKING_DIR` - Daemon's working directory
- `DIRWD_CONFIG_PATH` - Path to the daemon configuration file
- `DIRWD_UMASK` - Daemon's process umask

## Build

Daemon can be build with GNU Make.

### Variables

- `BUILD_TYPE` - may be `DEBUG` (default) or `RELEASE`

### Commands

- `make all` - build executable
- `make clean` - delete project temporary files and build files
- `make run` - build executable and run program

## Daemon configurtion

### Syntax

Daemon reads its configuration from specified file. Configuration file must contain single* line with following whitespace separated fields:

```
[target dir absolute path] [inspection timeout in sec]
```
**Empty lines are acceptable, if configuration has more than one non-empty line it will be discarded*

Target directory path may contain double quotes '"' and shielding symbol '\' to implement verbatim reading.

### Example

```
/home/user/Documents/Do\ not\ touch 60
```
```
/home/user/Documents/"Do not touch" 60
```

Configuration means: inspect "/home/user/Documents/Do not touch" directory and all its subdirectories once a minute (60 seconds).

## Usage

1. **Start daemon** by running daemon executable
2. **Update daemon** configuration by sending SIGHUP signal to the daemon process. New configuration will be read from specified configuration file.
3. **Shutdown daemon** by sending SIGKILL signal to the daemon process
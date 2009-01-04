/**
 * SMSD PID file handling
 */
#ifndef __smsd_pidfile_h__
#define __smsd_pidfile_h__

/**
 * Check existing PID file if it points to existing application.
 */
extern void check_pid(const char *pid_file);

/**
 * Write a pid file.
 */
extern void write_pid(const char *pid_file);

#endif

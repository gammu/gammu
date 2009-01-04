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

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

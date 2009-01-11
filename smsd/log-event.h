/**
 * Windows event log logging backend.
 */

#ifndef __smsd_common_h__
#define __smsd_common_h__

/**
 * Initalizes logging to Windows event log.
 */
void *eventlog_init(void);

/**
 * Logs a message to Windows event log.
 */
void eventlog_log(void *handle, int level, const char *message);

/**
 * Closes logging to Windows event log.
 */
void eventlog_close(void *handle);

#endif

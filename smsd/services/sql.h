/* (c) 2010 by  Miloslav Semler */

#ifndef __sql_h_
#define __sql_h_

#include "../core.h"

extern GSM_SMSDService SMSDSQL;

/**
 * Formats a database-specific expression adding seconds to current time.
 */
const char *SMSDSQL_NowPlus(GSM_SMSDConfig * Config, int seconds);

/**
 * Returns phone status expiry interval including the heartbeat grace period.
 */
int SMSDSQL_PhoneStatusTimeout(unsigned int status_frequency);

/**
 * Parses date string into time_t.
 *
 * \return Negative value on failure, -2 for special date "0000-00-00 00:00:00"
 */
time_t SMSDSQL_ParseDate(GSM_SMSDConfig * Config, const char *date);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

/* (c) 2010 by  Miloslav Semler */

#ifndef __sql_h_
#define __sql_h_

#include "../core.h"

extern GSM_SMSDService SMSDSQL;

time_t SMSDSQL_ParseDate(GSM_SMSDConfig * Config, const char *date);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

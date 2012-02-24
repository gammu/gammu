/**
 * Windows event log logging backend.
 */

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include "smsd-event.h"
#include "core.h"

void *eventlog_init(void)
{
	HANDLE handle;
        handle = RegisterEventSource(NULL, "gammu-smsd");
        if (handle == NULL) {
            fprintf(stderr, "Error opening event log!\n");
        }
	return (void *)handle;
}

void eventlog_log(void *handle, int level, const char *message)
{
        LPCTSTR lpstrings[1];
        WORD evtype = EVENTLOG_ERROR_TYPE;
	DWORD eventid = 0;

	switch (level) {
		case DEBUG_ERROR:
			evtype = EVENTLOG_ERROR_TYPE;
			eventid = EVENT_MSG_ERROR;
			break;
		case DEBUG_INFO:
			evtype = EVENTLOG_SUCCESS;
			eventid = EVENT_MSG_INFO;
			break;
		case DEBUG_NOTICE:
			eventid = EVENT_MSG_NOTICE;
			evtype = EVENTLOG_INFORMATION_TYPE;
			break;
		case DEBUG_SQL:
			eventid = EVENT_MSG_SQL;
			evtype = EVENTLOG_INFORMATION_TYPE;
			break;
		case DEBUG_GAMMU:
			eventid = EVENT_MSG_GAMMU;
			evtype = EVENTLOG_INFORMATION_TYPE;
			break;
		default:
			eventid = EVENT_MSG_OTHER;
			evtype = EVENTLOG_INFORMATION_TYPE;
			break;
	}
	lpstrings[0] = message;
	/*
	 * @todo: 1024 is probably wrong, we should use mc to get proper
	 * event identifiers.
	 */
	ReportEvent(handle, evtype, EVENT_CAT_SMSD, eventid, NULL, 1, 0,
		lpstrings, NULL);
}

void eventlog_close(void *handle)
{
	DeregisterEventSource(handle);
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

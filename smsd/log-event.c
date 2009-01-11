/**
 * Windows event log logging backend.
 */

#include <windows.h>
#include <stdio.h>

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
        WORD evtype= EVENTLOG_ERROR_TYPE;

	switch (level) {
		case -1:
			evtype = EVENTLOG_ERROR_TYPE;
			break;
		case 0:
		case 1:
		default:
			evtype = EVENTLOG_INFORMATION_TYPE;
			break;
	}
	lpstrings[0] = message;
	/*
	 * @todo: 1024 is probably wrong, we should use mc to get proper
	 * event identifiers.
	 */
	ReportEvent(handle, evtype, 0, 1024, NULL, 1, 0,
		lpstrings, NULL);
}

void eventlog_close(void *handle)
{
	DeregisterEventSource(handle);
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

/**
 * Windows event log logging backend.
 */

#include <winsock2.h>
#include <windows.h>
#include <winreg.h>
#include <stdio.h>
#include "smsd-event.h"
#include "core.h"

gboolean eventlog_deregister(void)
{
	LONG ret;

	ret = RegDeleteKey(
		HKEY_LOCAL_MACHINE,
		"System\\CurrentControlSet\\Services\\EventLog\\Application\\GammuSMSD"
		);

	return (ret == ERROR_SUCCESS);
}

gboolean eventlog_register(void)
{
	LONG ret;
	HKEY hKey;
	DWORD data;

	SECURITY_DESCRIPTOR SD;
	SECURITY_ATTRIBUTES SA;

	char program_name[MAX_PATH];

	if (GetModuleFileName(NULL, program_name, sizeof(program_name)) == 0)
		return FALSE;

	if (!InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION)) {
		return FALSE;
	}

	if(!SetSecurityDescriptorDacl(&SD, TRUE, 0, FALSE)) {
		return FALSE;
	}

	SA.nLength = sizeof(SA);
	SA.lpSecurityDescriptor = &SD;
	SA.bInheritHandle = FALSE;

	ret = RegCreateKeyEx(
		HKEY_LOCAL_MACHINE,
		"System\\CurrentControlSet\\Services\\EventLog\\Application\\GammuSMSD",
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		&SA,
		&hKey,
		NULL);

	if (ret != ERROR_SUCCESS) {
		fprintf(stderr, "Failed to create registry key!\n");
		return FALSE;
	}

	data = 3;
	ret = RegSetValueEx(
		hKey,
		"CategoryCount",
		0,
		REG_DWORD,
		(BYTE *)&data,
		sizeof(DWORD));

	if (ret != ERROR_SUCCESS) {
		fprintf(stderr, "Failed to write CategoryCount to registry!\n");
		return FALSE;
	}

	ret = RegSetValueEx(
		hKey,
		"CategoryMessageFile",
		0,
		REG_SZ,
		(BYTE *)program_name,
		(DWORD)(strlen(program_name) + 1));

	if (ret != ERROR_SUCCESS) {
		fprintf(stderr, "Failed to write CategoryMessageFile to registry!\n");
		return FALSE;
	}

	ret = RegSetValueEx(
		hKey,
		"EventMessageFile",
		0,
		REG_SZ,
		(BYTE *)program_name,
		(DWORD)strlen(program_name) + 1);

	if (ret != ERROR_SUCCESS) {
		fprintf(stderr, "Failed to write EventMessageFile to registry!\n");
		return FALSE;
	}

	ret = RegSetValueEx(
		hKey,
		"ParameterMessageFile",
		0,
		REG_SZ,
		(BYTE *)program_name,
		(DWORD)strlen(program_name) + 1);

	if (ret != ERROR_SUCCESS) {
		fprintf(stderr, "Failed to write ParameterMessageFile to registry!\n");
		return FALSE;
	}

	data = EVENTLOG_ERROR_TYPE | EVENTLOG_INFORMATION_TYPE | EVENTLOG_WARNING_TYPE;
	ret = RegSetValueEx(
		hKey,
		"TypesSupported",
		0,
		REG_DWORD,
		(BYTE *)&data,
		sizeof(DWORD));

	if (ret != ERROR_SUCCESS) {
		fprintf(stderr, "Failed to write TypesSupported to registry!\n");
		return FALSE;
	}

	RegCloseKey(hKey);

	return TRUE;
}

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
	WORD eventcat = 0;
	DWORD eventid = 0;

	switch (level) {
		case DEBUG_ERROR:
			evtype = EVENTLOG_ERROR_TYPE;
			eventid = EVENT_MSG_ERROR;
			eventcat = EVENT_CAT_SMSD;
			break;
		case DEBUG_INFO:
			evtype = EVENTLOG_SUCCESS;
			eventid = EVENT_MSG_INFO;
			eventcat = EVENT_CAT_SMSD;
			break;
		case DEBUG_NOTICE:
			eventid = EVENT_MSG_NOTICE;
			evtype = EVENTLOG_INFORMATION_TYPE;
			eventcat = EVENT_CAT_SMSD;
			break;
		case DEBUG_SQL:
			eventid = EVENT_MSG_SQL;
			evtype = EVENTLOG_INFORMATION_TYPE;
			eventcat = EVENT_CAT_SQL;
			break;
		case DEBUG_GAMMU:
			eventid = EVENT_MSG_GAMMU;
			evtype = EVENTLOG_INFORMATION_TYPE;
			eventcat = EVENT_CAT_GAMMU;
			break;
		default:
			eventid = EVENT_MSG_OTHER;
			evtype = EVENTLOG_INFORMATION_TYPE;
			eventcat = EVENT_CAT_SMSD;
			break;
	}
	lpstrings[0] = message;
	/*
	 * @todo: 1024 is probably wrong, we should use mc to get proper
	 * event identifiers.
	 */
	ReportEvent(handle, evtype, eventcat, eventid, NULL, 1, 0,
		lpstrings, NULL);
}

void eventlog_close(void *handle)
{
	DeregisterEventSource(handle);
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

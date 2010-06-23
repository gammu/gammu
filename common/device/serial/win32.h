
#ifdef WIN32
#ifndef winserial_h
#define winserial_h

#include <windows.h>

typedef struct {
	HANDLE 		hPhone;
	OVERLAPPED 	osWrite, osRead;
	DCB 		backup_dcb;
} GSM_Device_SerialData;

#endif
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

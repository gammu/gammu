
#ifdef WIN32
#ifndef winserial_h
#define winserial_h

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef struct {
	HANDLE 		hPhone;
	DCB 		old_settings;
	OVERLAPPED 	osWrite,osRead;
} GSM_Device_SerialData;

#endif
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

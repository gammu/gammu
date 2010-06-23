
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

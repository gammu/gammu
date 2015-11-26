#ifndef proxy_h
#define proxy_h

#include <signal.h>

typedef struct {
	int hRead;
	int hWrite;
	pid_t hProcess;
} GSM_Device_ProxyData;

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

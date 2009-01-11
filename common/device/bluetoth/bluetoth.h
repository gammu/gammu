#ifndef bluetooth_h
#define bluetooth_h

#include "../../misc/misc.h"

typedef struct {
	socket_type hPhone;
	void *Data;
} GSM_Device_BlueToothData;

/* These are actually implemented in backend services */
GSM_Error bluetooth_connect(GSM_StateMachine *s, int port, char *device);
GSM_Error bluetooth_findchannel(GSM_StateMachine *s);
int bluetooth_read(GSM_StateMachine *s, void *buf, size_t nbytes);
int bluetooth_write(GSM_StateMachine *s, const void *buf, size_t nbytes);
GSM_Error bluetooth_close(GSM_StateMachine *s);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

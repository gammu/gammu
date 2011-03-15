
#ifndef irda_device_functions_h
#define irda_device_functions_h
#include "DeviceDef.h"
#include "commFun.h"



int socket_read(GSM_DeviceData *s, void *buf, size_t nbytes, int hPhone);

int socket_write(GSM_DeviceData *s, unsigned char *buf, size_t nbytes, int hPhone,Debug_Info *debugInfo);

GSM_Error socket_close(GSM_DeviceData *s, int hPhone);


#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

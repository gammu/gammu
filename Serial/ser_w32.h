#ifndef winserial_h
#define winserial_h
#include "DeviceDef.h"
#include "mbglobals.h"
#include "commFun.h"

GSM_Error serial_close(GSM_DeviceData *s,Debug_Info *debugInfo);
GSM_Error serial_open (GSM_DeviceData *s,Debug_Info *debugInfo);
GSM_Error serial_setparity (GSM_DeviceData *s, bool parity,Debug_Info *debugInfo);
GSM_Error serial_setdtrrts(GSM_DeviceData *s, bool dtr, bool rts,Debug_Info *debugInfo);
GSM_Error serial_setspeed(GSM_DeviceData *s, int speed,Debug_Info *debugInfo);
int serial_read(GSM_DeviceData *s, void *buf, size_t nbytes,Debug_Info *debugInfo);
int serial_write(GSM_DeviceData *s, void *buf, size_t nbytes,Debug_Info *debugInfo);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

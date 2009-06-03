
#ifndef device_functions_h
#define device_functions_h

#include "../gsmstate.h"

#ifdef GSM_ENABLE_BLUETOOTHDEVICE
#ifdef BLUETOOTH_RF_SEARCHING

int bluetooth_checkservicename(GSM_StateMachine *s, const char *name);

#endif
#endif

#if defined (GSM_ENABLE_BLUETOOTHDEVICE) || defined (GSM_ENABLE_IRDADEVICE)

int socket_read(GSM_StateMachine *s, void *buf, size_t nbytes, socket_type hPhone);

int socket_write(GSM_StateMachine *s, unsigned const char *buf, size_t nbytes, socket_type hPhone);

GSM_Error socket_close(GSM_StateMachine *s, socket_type hPhone);

#endif

GSM_Error 	lock_device	(GSM_StateMachine *s, const char* port, char **lock_device);
gboolean 		unlock_device	(GSM_StateMachine *s, char **lock_file);

int FindSerialSpeed(const char *buffer);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

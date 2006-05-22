/* (c) 2006 by Marcin Wiacek */

#ifndef gnapplet_h
#define gnapplet_h

#include "../protocol.h"

typedef struct {
	int			MsgRXState;
	GSM_Protocol_Message 	Msg;
} GSM_Protocol_GNAPBUSData;

#if defined(GSM_ENABLE_BLUEGNAPBUS)
#  ifndef GSM_USED_BLUETOOTHDEVICE
#    define GSM_USED_BLUETOOTHDEVICE
#  endif
#endif
#if defined(GSM_ENABLE_IRDAGNAPBUS)
#  ifndef GSM_USED_IRDADEVICE
#    define GSM_USED_IRDADEVICE
#  endif
#endif

#define GNAPBUS_FRAME_ID		0x5A

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

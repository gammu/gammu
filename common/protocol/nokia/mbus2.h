/* (c) 2001-2003 by Marcin Wiacek */
/* based on some work from MyGnokii (www.mwiacek.com) */

#ifndef mbus2_h
#define mbus2_h

#include "../protocol.h"

#define MBUS2_FRAME_ID       0x1f
#define MBUS2_DEVICE_PHONE   0x00 /* Nokia mobile phone */
#define MBUS2_DEVICE_PC      0x10 /* Our PC (MBUS) */
#define MBUS2_ACK_BYTE	     0x7f /* Acknowledge of the received frame */

typedef struct {
	int			MsgSequenceNumber;
	int			MsgRXState;
	GSM_Protocol_Message	Msg;
} GSM_Protocol_MBUS2Data;

#ifndef GSM_USED_SERIALDEVICE
#  define GSM_USED_SERIALDEVICE
#endif

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

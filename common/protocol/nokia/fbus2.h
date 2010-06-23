
#ifndef fbus2_h
#define fbus2_h

#include "../protocol.h"

#define FBUS2_FRAME_ID       0x1e
#define FBUS2_IR_FRAME_ID    0x1c
#define FBUS2_DEVICE_PHONE   0x00 /* Nokia mobile phone */
#define FBUS2_DEVICE_PC      0x0c /* Our PC */
#define FBUS2_ACK_BYTE	     0x7f /* Acknowledge of the received frame */

#define FBUS2_MAX_TRANSMIT_LENGTH 120

typedef struct {
	int			MsgSequenceNumber;
	int			MsgRXState;
	int			FramesToGo;
	GSM_Protocol_Message	MultiMsg;
	GSM_Protocol_Message	Msg;
} GSM_Protocol_FBUS2Data;

#ifndef GSM_USED_SERIALDEVICE
#  define GSM_USED_SERIALDEVICE
#endif
#if defined(GSM_ENABLE_ATBLUETOOTH) || defined(GSM_ENABLE_DLR3BLUETOOTH)
#  ifndef GSM_USED_BLUETOOTHDEVICE
#    define GSM_USED_BLUETOOTHDEVICE
#  endif
#endif

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

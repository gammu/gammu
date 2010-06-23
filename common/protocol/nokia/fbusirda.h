
#ifndef fbus2irda_h
#define fbus2irda_h

#include "../protocol.h"

#define FBUS2IRDA_FRAME_ID       0x14
#define FBUS2IRDA_DEVICE_PHONE   0x00 /* Nokia mobile phone */
#define FBUS2IRDA_DEVICE_PC      0x0c /* Our PC */

#define FBUS2IRDA_MAX_TRANSMIT_LENGTH 1000

typedef struct {
	int			MsgRXState;
	GSM_Protocol_Message	Msg;
} GSM_Protocol_FBUS2IRDAData;

#ifndef GSM_USED_IRDADEVICE
#  define GSM_USED_IRDADEVICE
#endif

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

/* (c) 2003 by Marcin Wiacek */

#ifndef obex_h
#define obex_h

#include "../protocol.h"

typedef struct {
	int			MsgRXState;
	GSM_Protocol_Message 	Msg;
} GSM_Protocol_OBEXData;

#ifndef GSM_USED_SERIALDEVICE
#  define GSM_USED_SERIALDEVICE
#endif
#if defined(GSM_ENABLE_BLUEOBEX)
#  ifndef GSM_USED_BLUETOOTHDEVICE
#    define GSM_USED_BLUETOOTHDEVICE
#  endif
#endif
#if defined(GSM_ENABLE_IRDAOBEX)
#  ifndef GSM_USED_IRDADEVICE
#    define GSM_USED_IRDADEVICE
#  endif
#endif

void OBEXAddBlock(char *Buffer, int *Pos, unsigned char ID, const char *AddData, int AddLength);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

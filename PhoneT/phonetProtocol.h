
#ifndef PHONETPROTOCOL_h
#define PHONETPROTOCOL_h
#include "commfun.h"

#define PHONET_FRAME_ID      	0x14
#define PHONET_BLUE_FRAME_ID	0x19
#define PHONET_DEVICE_PHONE   	0x00 /* Nokia mobile phone */
#define PHONET_DEVICE_PC      	0x0c /* Our PC */
#define PHONET_BLUE_DEVICE_PC   0x10 /* Our PC */

typedef struct {
	int			MsgRXState;
	GSM_Protocol_Message	Msg;
} GSM_Protocol_PHONETData;

GSM_Error PHONET_Terminate();
GSM_Error PHONET_Initialise();
GSM_Error PHONET_StateMachine(unsigned char rx_char);
GSM_Error PHONET_WriteMessage (unsigned char *MsgBuffer,int MsgLength,unsigned char	MsgType);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

/* (c) 2003 by Marcin Wiacek */
#ifndef obex_h
#define obex_h
#include "commfun.h"


typedef struct {
	int			MsgRXState;
	GSM_Protocol_Message 	Msg;
} GSM_Protocol_OBEXData;

GSM_Error OBEX_Terminate();
GSM_Error OBEX_Initialise();
GSM_Error OBEX_StateMachine(unsigned char rx_char);
GSM_Error OBEX_WriteMessage (unsigned char *buffer,int length, unsigned char type);

void OBEXAddBlock(unsigned char *Buffer, int *Pos, unsigned char ID,unsigned  char *AddBuffer, int AddLength);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

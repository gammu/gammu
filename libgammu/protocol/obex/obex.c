/* (c) 2003 by Marcin Wiacek */
/* www.irda.org OBEX specs 1.3 */

#include "../../gsmstate.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(GSM_ENABLE_BLUEOBEX) || defined(GSM_ENABLE_IRDAOBEX) || defined(GSM_ENABLE_ATOBEX)

#include "../../gsmcomon.h"
#include "../../misc/coding/coding.h"
#include "obex.h"

static GSM_Error OBEX_WriteMessage (GSM_StateMachine *s, unsigned const char *MsgBuffer,
				    int MsgLength, unsigned char type)
{
	unsigned char	*buffer=NULL;
	int 		length=0,sent=0;

	buffer = (unsigned char *)malloc(MsgLength + 3);

	OBEXAddBlock(buffer, &length, type, MsgBuffer, MsgLength);

	GSM_DumpMessageLevel2(s, buffer+3, MsgLength, type);
	GSM_DumpMessageLevel3(s, buffer+3, MsgLength, type);

	/* Send it out... */
	sent = s->Device.Functions->WriteDevice(s,buffer,length);

	free(buffer);
	buffer=NULL;

	if (sent!=length) {
		return ERR_DEVICEWRITEERROR;
	}
	usleep(length*1000);
	return ERR_NONE;
}

static GSM_Error OBEX_StateMachine(GSM_StateMachine *s, unsigned char rx_char)
{
	GSM_Phone_Functions 	*Phone	= s->Phone.Functions;
	GSM_Protocol_OBEXData 	*d	= &s->Protocol.Data.OBEX;

	switch (d->MsgRXState) {
	case RX_Sync:
		d->Msg.Type   = rx_char;
		d->MsgRXState = RX_GetLength1;
		break;
	case RX_GetLength1:
		d->Msg.Length = rx_char * 256;
		d->MsgRXState = RX_GetLength2;
		break;
	case RX_GetLength2:
		d->Msg.Length = d->Msg.Length + rx_char - 3;
		d->Msg.Count  = 0;
		if (d->Msg.Count == d->Msg.Length) {
			s->Phone.Data.RequestMsg	= &d->Msg;
			s->Phone.Data.DispatchError	= Phone->DispatchMessage(s);
			d->MsgRXState = RX_Sync;
		} else {
			if (d->Msg.BufferUsed < d->Msg.Length) {
				d->Msg.BufferUsed 	= d->Msg.Length;
				d->Msg.Buffer 		= (unsigned char *)realloc(d->Msg.Buffer,d->Msg.BufferUsed);
			}
			d->MsgRXState 	= RX_GetMessage;
		}
		break;
	case RX_GetMessage:
		d->Msg.Buffer[d->Msg.Count] = rx_char;
		d->Msg.Count++;
		if (d->Msg.Count == d->Msg.Length) {
			s->Phone.Data.RequestMsg	= &d->Msg;
			s->Phone.Data.DispatchError	= Phone->DispatchMessage(s);
			d->MsgRXState = RX_Sync;
		}
		break;
	}

	return ERR_NONE;
}

static GSM_Error OBEX_Initialise(GSM_StateMachine *s)
{
	GSM_Protocol_OBEXData *d = &s->Protocol.Data.OBEX;

	d->Msg.BufferUsed	= 0;
	d->Msg.Buffer 		= NULL;
	d->Msg.Length		= 0;

	d->MsgRXState 		= RX_Sync;

	return ERR_NONE;
}

static GSM_Error OBEX_Terminate(GSM_StateMachine *s)
{
	free(s->Protocol.Data.OBEX.Msg.Buffer);
	s->Protocol.Data.OBEX.Msg.Buffer=NULL;
	return ERR_NONE;
}

GSM_Protocol_Functions OBEXProtocol = {
	OBEX_WriteMessage,
	OBEX_StateMachine,
	OBEX_Initialise,
	OBEX_Terminate
};

void OBEXAddBlock(char *Buffer, int *Pos, unsigned char ID, const char *AddData, int AddLength)
{
	Buffer[(*Pos)++] = ID;
	Buffer[(*Pos)++] = (AddLength+3)/256;
	Buffer[(*Pos)++] = (AddLength+3)%256;
	if (AddData != NULL) {
		memcpy(Buffer+(*Pos),AddData,AddLength);
		(*Pos) += AddLength;
	}
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

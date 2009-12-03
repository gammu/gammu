/* (c) 2006 by Marcin Wiacek */

#include "../../gsmstate.h"

#if defined(GSM_ENABLE_BLUEGNAPBUS) || defined(GSM_ENABLE_IRDAGNAPBUS)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../gsmcomon.h"
#include "gnapbus.h"

static GSM_Error GNAPBUS_WriteMessage (GSM_StateMachine *s, unsigned const char *MsgBuffer,
				    int MsgLength, unsigned char MsgType)
{
	unsigned char	*buffer=NULL;
	int		sent=0,length=0,i=0;
	unsigned char	checksum=0;

	GSM_DumpMessageLevel3(s, MsgBuffer, MsgLength, MsgType);

	buffer = (unsigned char *)malloc(MsgLength + 10);

	buffer[0] = GNAPBUS_FRAME_ID,
	buffer[1] = 0x00;
	buffer[2] = MsgLength / 256;
	buffer[3] = MsgLength % 256;
	buffer[4] = MsgType;
	buffer[5] = 0x00;
	memcpy(buffer + 6, MsgBuffer, MsgLength);
	length = MsgLength+6;

	if (MsgLength & 1) buffer[length++] = 0x00;
	/* Odd messages require additional 0x00 byte */
/* 	if (MsgLength % 2) buffer[length++] = 0x00; */

	checksum 	= 0;
	for (i = 0; i < length; i+=2) checksum ^= buffer[i];
	buffer[length++] 	= checksum;

	checksum 	= 0;
	for (i = 1; i < length; i+=2) checksum ^= buffer[i];
	buffer[length++] 	= checksum;

/* 	GSM_DumpMessageLevel2(s, buffer, length, MsgType); */
	GSM_DumpMessageLevel2(s, MsgBuffer, MsgLength, MsgType);

	/* Sending to phone */
	sent = s->Device.Functions->WriteDevice(s,buffer,length);
	free(buffer);
	buffer=NULL;

	if (sent!=length) {
		return ERR_DEVICEWRITEERROR;
	}
	usleep(length*1000);
	return ERR_NONE;
}

static GSM_Error GNAPBUS_StateMachine(GSM_StateMachine *s, unsigned char rx_char)
{
	GSM_Protocol_GNAPBUSData *d = &s->Protocol.Data.GNAPBUS;

/* printf("%02x ",rx_char); */
/* fflush(stdout); */
	/* XOR the byte with the earlier checksum */
	d->Msg.CheckSum[d->Msg.Count & 1] ^= rx_char;

	switch (d->MsgRXState) {
	case RX_Sync:
		if (rx_char == GNAPBUS_FRAME_ID) {
			d->MsgRXState = RX_GetDestination;
			d->Msg.Count = 0;
			d->Msg.Length = 0;
			d->Msg.CheckSum[0] = rx_char;
			d->Msg.CheckSum[1] = 0;
		} else smprintf(s,"Sync error: %02x\n",rx_char);
		break;
	case RX_GetDestination:
		d->MsgRXState = RX_GetLength1;
		break;
	case RX_GetLength1:
		d->MsgRXState = RX_GetLength2;
		d->Msg.Length = rx_char << 8;
		break;
	case RX_GetLength2:
		d->MsgRXState = RX_GetType;
		d->Msg.Length += rx_char;
		d->Msg.Buffer 	= (unsigned char *)malloc(d->Msg.Length+3);
		break;
	case RX_GetType:
		d->MsgRXState = RX_GetSource;
		d->Msg.Type = rx_char;
		break;
	case RX_GetSource:
		d->MsgRXState = RX_GetMessage;
		break;
	case RX_GetMessage:
		d->Msg.Buffer[d->Msg.Count] = rx_char;
		d->Msg.Count++;

		if (d->Msg.Count != ((d->Msg.Length+3)& ~1)) return ERR_NONE;

/* 	GSM_DumpMessageLevel2(s, d->Msg.Buffer, d->Msg.Count, d->Msg.Type); */
/* printf("\n%02x %02x\n",d->Msg.CheckSum[0],d->Msg.CheckSum[1]); */
		/* Checksum is incorrect */
		if (d->Msg.CheckSum[0] != d->Msg.CheckSum[1]) {
			if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR ||
			    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl==DL_TEXTERRORDATE) {
				smprintf(s,"[ERROR: checksum]\n");
			}
			free(d->Msg.Buffer);
			d->Msg.Buffer = NULL;
			d->Msg.Length = 0;
			d->MsgRXState = RX_Sync;
			return ERR_NONE;
		}
		s->Phone.Data.RequestMsg = &d->Msg;
		s->Phone.Data.DispatchError = s->Phone.Functions->DispatchMessage(s);
		free(d->Msg.Buffer);
		d->Msg.Buffer = NULL;
		d->Msg.Length = 0;
		d->MsgRXState = RX_Sync;
		return ERR_NONE;
	}
	return ERR_NONE;
}

static GSM_Error GNAPBUS_Initialise(GSM_StateMachine *s)
{
	GSM_Protocol_GNAPBUSData *d = &s->Protocol.Data.GNAPBUS;

	d->Msg.BufferUsed	= 0;
	d->Msg.Buffer 		= NULL;
	d->Msg.Length		= 0;

	d->MsgRXState 		= RX_Sync;

	return ERR_NONE;
}

static GSM_Error GNAPBUS_Terminate(GSM_StateMachine *s)
{
	free(s->Protocol.Data.GNAPBUS.Msg.Buffer);
	s->Protocol.Data.GNAPBUS.Msg.Buffer=NULL;
	return ERR_NONE;
}

GSM_Protocol_Functions GNAPBUSProtocol = {
	GNAPBUS_WriteMessage,
	GNAPBUS_StateMachine,
	GNAPBUS_Initialise,
	GNAPBUS_Terminate
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

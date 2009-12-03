/* (c) 2001-2003 by Marcin Wiacek */
/* based on some work from MyGnokii (www.mwiacek.com) */

#include "../../gsmstate.h"

#ifdef GSM_ENABLE_MBUS2

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../gsmcomon.h"
#include "mbus2.h"

static GSM_Error MBUS2_WriteMessage (GSM_StateMachine 	*s,
				     unsigned const char 	*MsgBuffer,
				     int 		MsgLength,
				     unsigned char 	MsgType)
{
	unsigned char 			*buffer=NULL, checksum = 0;
	GSM_Protocol_MBUS2Data 		*d = &s->Protocol.Data.MBUS2;
	int 				i=0, sent=0, length=0;

	GSM_DumpMessageLevel3(s, MsgBuffer, MsgLength, MsgType);

	buffer = (unsigned char *)malloc(MsgLength + 8);

	buffer[0] = MBUS2_FRAME_ID;
	buffer[1] = MBUS2_DEVICE_PHONE;		/*  destination */
	buffer[2] = MBUS2_DEVICE_PC;    		/*  source */
	buffer[3] = MsgType;
	buffer[4] = MsgLength / 256;
	buffer[5] = MsgLength % 256;

	memcpy(buffer + 6, MsgBuffer, MsgLength);
	length = 6 + MsgLength;

	/* According to http://www.flosys.com/tdma/n5160.html some phones
         * can have problems with checksum equal 0x1F. Phones can recognize
         * received frame, but won't send ACK for it. When checksum is 0x1F,
         * we increment the sequence number
         */
	do {
		d->MsgSequenceNumber++;

		buffer[length] = d->MsgSequenceNumber;

		/* Calculating checksum */
		checksum = 0;

		for (i = 0; i < length + 1; i++) {
			checksum ^= buffer[i];
		}
	} while (checksum == 0x1f);

	buffer[length++] = d->MsgSequenceNumber;
	buffer[length++] = checksum;

	GSM_DumpMessageLevel2(s, buffer+6, MsgLength, MsgType);

	/* Sending to phone */
	sent=s->Device.Functions->WriteDevice(s,buffer,length);
	free(buffer);
	buffer=NULL;

	if (sent!=length) {
		return ERR_DEVICEWRITEERROR;
	}
	usleep(length*1000);
	return ERR_NONE;
}

static GSM_Error MBUS2_SendAck(GSM_StateMachine 	*s,
			       unsigned char 		type,
			       unsigned char 		sequence)
{
	GSM_Device_Functions 	*Device		= s->Device.Functions;
	unsigned char 		buffer[6]={0};
	int 			i=0,length=0,write_data=0;

	buffer[0] = MBUS2_FRAME_ID;
	buffer[1] = MBUS2_DEVICE_PHONE;	/* destination */
	buffer[2] = MBUS2_DEVICE_PC;		/* source */
	buffer[3] = MBUS2_ACK_BYTE;
	buffer[4] = sequence;
	buffer[5] = '\0';
	length=strlen(buffer);

	/* Calculating checksum */
	for (i = 0; i < length; i++) {
		buffer[5] ^= buffer[i];
	}
	smprintf_level(s, D_TEXT, "[Sending Ack of type %02x, seq: %x]\n",type,sequence);

	/* Sending to phone */
	write_data=Device->WriteDevice(s,buffer,length);

	if (write_data!=length) {
		return ERR_DEVICEWRITEERROR;
	}
	usleep(length*1000);
	return ERR_NONE;
}

static GSM_Error MBUS2_StateMachine(GSM_StateMachine *s, unsigned char rx_char)
{
	GSM_Phone_Functions 	*Phone	= s->Phone.Functions;
	GSM_Protocol_MBUS2Data 	*d	= &s->Protocol.Data.MBUS2;

	d->Msg.CheckSum[0] = d->Msg.CheckSum[1];
	d->Msg.CheckSum[1] ^= rx_char;

	if (d->MsgRXState == RX_GetMessage) {
		d->Msg.Buffer[d->Msg.Count] = rx_char;
		d->Msg.Count++;

		/* This is not last byte in frame */
		if (d->Msg.Count != d->Msg.Length+2) return ERR_NONE;

		/* Checksum is incorrect */
		if (d->Msg.CheckSum[0] != rx_char) {
			smprintf_level(s, D_ERROR, "[ERROR: checksum]\n");

			d->MsgRXState = RX_Sync;
			return ERR_NONE;
		}

		if (d->Msg.Destination != MBUS2_DEVICE_PHONE) {
			MBUS2_SendAck(s, d->Msg.Type, d->Msg.Buffer[d->Msg.Count-2]);
			s->Phone.Data.RequestMsg	= &d->Msg;
			s->Phone.Data.DispatchError	= Phone->DispatchMessage(s);
		}

		d->MsgRXState = RX_Sync;
		return ERR_NONE;
	}
	if (d->MsgRXState == RX_GetLength2) {
		if (d->Msg.Type == MBUS2_ACK_BYTE) {
			smprintf_level(s, D_TEXT, "[Received Ack]\n");

			d->MsgRXState = RX_Sync;
			return ERR_NONE;
		}

		d->Msg.Length = d->Msg.Length + rx_char;
		if (d->Msg.BufferUsed < d->Msg.Length+2) {
			d->Msg.BufferUsed 	= d->Msg.Length+2;
			d->Msg.Buffer 		= (unsigned char *)realloc(d->Msg.Buffer,d->Msg.BufferUsed);
		}

		d->MsgRXState = RX_GetMessage;
		return ERR_NONE;
	}
	if (d->MsgRXState == RX_GetLength1) {
		d->Msg.Length = rx_char * 256;

		d->MsgRXState = RX_GetLength2;
		return ERR_NONE;
	}
	if (d->MsgRXState == RX_GetType) {
		d->Msg.Type   = rx_char;

		d->MsgRXState = RX_GetLength1;
		return ERR_NONE;
	}
	if (d->MsgRXState == RX_GetSource) {
		if (rx_char != MBUS2_DEVICE_PHONE && rx_char != MBUS2_DEVICE_PC) {
			smprintf_level(s, D_ERROR, "[ERROR: incorrect char - %02x, not %02x and %02x]\n",
					rx_char, MBUS2_DEVICE_PHONE, MBUS2_DEVICE_PC);
			d->MsgRXState = RX_Sync;
			return ERR_NONE;
		}
		d->Msg.Source = rx_char;

		d->MsgRXState = RX_GetType;
		return ERR_NONE;
	}
	if (d->MsgRXState == RX_GetDestination) {
		if (rx_char != MBUS2_DEVICE_PC && rx_char != MBUS2_DEVICE_PHONE) {
			smprintf_level(s, D_ERROR, "[ERROR: incorrect char - %02x, not %02x and %02x]\n",
					rx_char, MBUS2_DEVICE_PHONE, MBUS2_DEVICE_PC);
			d->MsgRXState = RX_Sync;
			return ERR_NONE;
		}
		d->Msg.Destination 	= rx_char;

		d->MsgRXState 		= RX_GetSource;
		return ERR_NONE;
	}
	if (d->MsgRXState == RX_Sync) {
		if (rx_char != MBUS2_FRAME_ID) {
			smprintf_level(s, D_ERROR, "[ERROR: incorrect char - %02x, not %02x]\n",
					rx_char, MBUS2_FRAME_ID);
			return ERR_NONE;
		}
		d->Msg.CheckSum[1] 	= MBUS2_FRAME_ID;
		d->Msg.Count 		= 0;

		d->MsgRXState 		= RX_GetDestination;
		return ERR_NONE;
	}
	return ERR_NONE;
}

static GSM_Error MBUS2_Initialise(GSM_StateMachine *s)
{
	GSM_Device_Functions 	*Device	= s->Device.Functions;
	GSM_Protocol_MBUS2Data 	*d	= &s->Protocol.Data.MBUS2;
	GSM_Error 		error;

	d->Msg.Length		= 0;
	d->Msg.BufferUsed	= 0;
	d->Msg.Buffer		= NULL;

	d->MsgSequenceNumber	= 0;
	d->MsgRXState		= RX_Sync;

	error=Device->DeviceSetSpeed(s,9600);
	if (error!=ERR_NONE) return error;

	error=Device->DeviceSetParity(s,TRUE);
	if (error!=ERR_NONE) return error;

	error=Device->DeviceSetDtrRts(s,FALSE,TRUE); /*DTR low,RTS high*/
	if (error!=ERR_NONE) return error;
        usleep(200000);

	return ERR_NONE;
}

static GSM_Error MBUS2_Terminate(GSM_StateMachine *s)
{
	free(s->Protocol.Data.MBUS2.Msg.Buffer);
	s->Protocol.Data.MBUS2.Msg.Buffer=NULL;
	return ERR_NONE;
}

GSM_Protocol_Functions MBUS2Protocol = {
	MBUS2_WriteMessage,
	MBUS2_StateMachine,
	MBUS2_Initialise,
	MBUS2_Terminate
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

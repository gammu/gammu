/* (c) 2002-2003 by Marcin Wiacek */
/* based on some work from MyGnokii (www.mwiacek.com) */
/* Based on some work from Gnokii (www.gnokii.org)
 * (C) 1999-2000 Hugh Blemings & Pavel Janik ml. (C) 2001-2004 Pawel Kot
 * GNU GPL version 2 or later
 */
/* Due to a problem in the source code management, the names of some of
 * the authors have unfortunately been lost. We do not mean to belittle
 * their efforts and hope they will contact us to see their names
 * properly added to the Copyright notice above.
 * Having published their contributions under the terms of the GNU
 * General Public License (GPL) [version 2], the Copyright of these
 * authors will remain respected by adhering to the license they chose
 * to publish their code under.
 */

#include "../../gsmstate.h"

#if defined(GSM_ENABLE_FBUS2) || defined(GSM_ENABLE_FBUS2IRDA) || defined(GSM_ENABLE_FBUS2DLR3) || defined(GSM_ENABLE_FBUS2BLUE) || defined(GSM_ENABLE_BLUEFBUS2) || defined(GSM_ENABLE_DKU5FBUS2) || defined(GSM_ENABLE_FBUS2PL2303)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../gsmcomon.h"
#include "fbus2.h"

static GSM_Error FBUS2_Initialise(GSM_StateMachine *s);

static GSM_Error FBUS2_WriteFrame(GSM_StateMachine 	*s,
				  unsigned const char 	*MsgBuffer,
				  int 			MsgLength,
				  unsigned char 	MsgType)
{
	unsigned char 		  buffer2[FBUS2_MAX_TRANSMIT_LENGTH + 10];
	unsigned char 		  checksum=0;
	int 			  i, len, sent;

	buffer2[0] 	= FBUS2_FRAME_ID;
	if (s->ConnectionType==GCT_FBUS2IRDA) buffer2[0] = FBUS2_IRDA_FRAME_ID;

	buffer2[1] 	= FBUS2_DEVICE_PHONE;		/* destination */
	buffer2[2]	= FBUS2_DEVICE_PC;		/* source */
	buffer2[3]	= MsgType;
	buffer2[4]	= MsgLength / 256;
	buffer2[5]	= MsgLength % 256;

	memcpy(buffer2 + 6, MsgBuffer, MsgLength);
	len = MsgLength + 6;

	/* Odd messages require additional 0x00 byte */
	if (MsgLength % 2) buffer2[len++] = 0x00;

	checksum 	= 0;
	for (i = 0; i < len; i+=2) checksum ^= buffer2[i];
	buffer2[len++] 	= checksum;

	checksum 	= 0;
	for (i = 1; i < len; i+=2) checksum ^= buffer2[i];
	buffer2[len++] 	= checksum;

	/* Sending to phone */
	sent=s->Device.Functions->WriteDevice(s,buffer2,len);
	if (sent!=len) return ERR_DEVICEWRITEERROR;

	return ERR_NONE;
}

static GSM_Error FBUS2_WriteMessage (GSM_StateMachine 	*s,
				     unsigned const char 	*MsgBuffer,
				     int 		MsgLength,
				     unsigned char 	MsgType)
{
	int 			i, nom, togo, thislength; /* number of messages, ... */
	unsigned char 		buffer2[FBUS2_MAX_TRANSMIT_LENGTH + 2], seqnum;
	GSM_Protocol_FBUS2Data	*d = &s->Protocol.Data.FBUS2;
	GSM_Error 		error;

	GSM_DumpMessageLevel3(s, MsgBuffer, MsgLength, MsgType);

	nom  = (MsgLength + FBUS2_MAX_TRANSMIT_LENGTH - 1) / FBUS2_MAX_TRANSMIT_LENGTH;
	togo = MsgLength;

	for (i = 0; i < nom; i++) {
		seqnum 			= d->MsgSequenceNumber;
		if (i==0) seqnum 	= seqnum + 0x40;
		d->MsgSequenceNumber 	= (d->MsgSequenceNumber + 1) & 0x07;

		thislength = togo;
		if (togo > FBUS2_MAX_TRANSMIT_LENGTH) thislength = FBUS2_MAX_TRANSMIT_LENGTH;
		memcpy(buffer2, MsgBuffer + (MsgLength - togo), thislength);
		buffer2[thislength]	= nom - i;
		buffer2[thislength + 1]	= seqnum;
		togo 			= togo - thislength;

		GSM_DumpMessageLevel2(s, buffer2, thislength, MsgType);

		error = FBUS2_WriteFrame(s, buffer2, thislength + 2, MsgType);
		if (error != ERR_NONE) return error;
	}

	return ERR_NONE;
}

static GSM_Error FBUS2_SendAck(GSM_StateMachine 	*s,
			       unsigned char 		MsgType,
			       unsigned char 		MsgSequence)
{
	unsigned char buffer2[2];

	buffer2[0] = MsgType;
	buffer2[1] = MsgSequence;

	smprintf_level(s, D_TEXT, "[Sending Ack of type %02x, seq %x]\n",
			buffer2[0],
			buffer2[1]);

	/* Sending to phone */
	return FBUS2_WriteFrame(s, buffer2, 2, FBUS2_ACK_BYTE);
}

static GSM_Error FBUS2_StateMachine(GSM_StateMachine *s, unsigned char rx_char)
{
	GSM_Protocol_FBUS2Data 	*d = &s->Protocol.Data.FBUS2;
	unsigned char 		frm_num, seq_num;
	gboolean			correct = FALSE;

	/* XOR the byte with the earlier checksum */
	d->Msg.CheckSum[d->Msg.Count & 1] ^= rx_char;

	if (d->MsgRXState == RX_GetMessage) {
		d->Msg.Buffer[d->Msg.Count] = rx_char;
		d->Msg.Count++;

		/* This is not last byte in frame */
		if (d->Msg.Count != d->Msg.Length+(d->Msg.Length%2)+2) return ERR_NONE;

		/* Checksum is incorrect */
		if (d->Msg.CheckSum[0] != d->Msg.CheckSum[1]) {
			smprintf_level(s, D_ERROR, "[ERROR: checksum]\n");

			free(d->Msg.Buffer);
			d->Msg.Length 		= 0;
			d->Msg.Buffer 		= NULL;

			d->MsgRXState 		= RX_Sync;
			return ERR_NONE;
		}

		seq_num = d->Msg.Buffer[d->Msg.Length-1];

		if (d->Msg.Type == FBUS2_ACK_BYTE) {
			smprintf_level(s, D_TEXT, "[Received Ack of type %02x, seq %02x]\n",
					d->Msg.Buffer[0], seq_num);

			free(d->Msg.Buffer);
			d->Msg.Buffer 	= NULL;
			d->Msg.Length 	= 0;

			d->MsgRXState = RX_Sync;
			return ERR_NONE;
		}

		frm_num = d->Msg.Buffer[d->Msg.Length-2];

		if ((seq_num & 0x40) == 0x40) {
			d->FramesToGo		= frm_num;
			d->MultiMsg.Length	= 0;
			d->MultiMsg.Type	= d->Msg.Type;
			d->MultiMsg.Destination	= d->Msg.Destination;
			d->MultiMsg.Source	= d->Msg.Source;
		}

		if ((seq_num & 0x40) != 0x40 && d->FramesToGo != frm_num) {
			smprintf_level(s, D_ERROR, "[ERROR: Missed part of multiframe msg]\n");

			free(d->Msg.Buffer);
			d->Msg.Length 		= 0;
			d->Msg.Buffer 		= NULL;

			d->MsgRXState 		= RX_Sync;
			return ERR_NONE;
		}

		if ((seq_num & 0x40) != 0x40 && d->Msg.Type != d->MultiMsg.Type) {
			smprintf_level(s, D_ERROR, "[ERROR: Multiframe msg in multiframe msg]\n");

			free(d->Msg.Buffer);
			d->Msg.Length 		= 0;
			d->Msg.Buffer 		= NULL;

			d->MsgRXState 		= RX_Sync;
			return ERR_NONE;
		}

		if (d->MultiMsg.BufferUsed < d->MultiMsg.Length+d->Msg.Length-2) {
			d->MultiMsg.BufferUsed 	= d->MultiMsg.Length+d->Msg.Length-2;
			d->MultiMsg.Buffer 	= (unsigned char *)realloc(d->MultiMsg.Buffer,d->MultiMsg.BufferUsed);
		}
		memcpy(d->MultiMsg.Buffer+d->MultiMsg.Length,d->Msg.Buffer,d->Msg.Length-2);
		d->MultiMsg.Length = d->MultiMsg.Length+d->Msg.Length-2;

		free(d->Msg.Buffer);
		d->Msg.Length 	= 0;
		d->Msg.Buffer 	= NULL;

		d->FramesToGo--;

		/* do not ack debug trace, as this could generate a
		 * (feedback loop) flood of which even Noah would be scared.
		 */
		if (d->Msg.Type != 0) {
			FBUS2_SendAck(s,d->Msg.Type,((unsigned char)(seq_num & 0x0f)));
		}

		if (d->FramesToGo == 0) {
			s->Phone.Data.RequestMsg	= &d->MultiMsg;
			s->Phone.Data.DispatchError	= s->Phone.Functions->DispatchMessage(s);
		}
		d->MsgRXState = RX_Sync;
		return ERR_NONE;
	}
	if (d->MsgRXState == RX_GetLength2) {
		d->Msg.Length 	= d->Msg.Length + rx_char;
		d->Msg.Buffer 	= (unsigned char *)malloc(d->Msg.Length+3);
		d->MsgRXState 	= RX_GetMessage;
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
		if (rx_char != FBUS2_DEVICE_PHONE) {
			smprintf_level(s, D_ERROR, "[ERROR: incorrect char - %02x, not %02x]\n",
					rx_char, FBUS2_DEVICE_PHONE);

			d->MsgRXState = RX_Sync;
			return ERR_NONE;
		}
		d->Msg.Source = rx_char;

		d->MsgRXState = RX_GetType;
		return ERR_NONE;
	}
	if (d->MsgRXState == RX_GetDestination) {
		if (rx_char != FBUS2_DEVICE_PC) {
			smprintf_level(s, D_ERROR, "[ERROR: incorrect char - %02x, not %02x]\n",
					rx_char, FBUS2_DEVICE_PC);

			d->MsgRXState = RX_Sync;
			return ERR_NONE;
		}
		d->Msg.Destination = rx_char;

		d->MsgRXState 	   = RX_GetSource;
		return ERR_NONE;
	}
	if (d->MsgRXState == RX_Sync) {
		switch (s->ConnectionType) {
			case GCT_FBUS2:
			case GCT_FBUS2DLR3:
			case GCT_DKU5FBUS2:
			case GCT_FBUS2PL2303:
			case GCT_FBUS2BLUE:
			case GCT_BLUEFBUS2:
				if (rx_char == FBUS2_FRAME_ID) correct = TRUE;
				break;
			case GCT_FBUS2IRDA:
				if (rx_char == FBUS2_IRDA_FRAME_ID) correct = TRUE;
				break;
			default:
				break;
		}
		if (!correct) {
			smprintf_level(s, D_ERROR, "[ERROR: incorrect char - %02x, not %02x]\n",
					rx_char,
					(s->ConnectionType == GCT_FBUS2IRDA) ? FBUS2_IRDA_FRAME_ID : FBUS2_FRAME_ID);
			if (rx_char == 0x20) {
				smprintf(s, "0x20 reply detected...\n");
				smprintf(s, "Trying to reinit connection...\n");
				FBUS2_Initialise(s);
			}
			return ERR_NONE;
		}

		d->Msg.CheckSum[0] = rx_char;
		d->Msg.CheckSum[1] = 0;
		d->Msg.Count	   = 0;

		d->MsgRXState	   = RX_GetDestination;
		return ERR_NONE;
	}
	return ERR_NONE;
}

#if defined(GSM_ENABLE_FBUS2DLR3) || defined(GSM_ENABLE_DKU5FBUS2) || defined(GSM_ENABLE_FBUS2BLUE) || defined(GSM_ENABLE_BLUEFBUS2) || defined(GSM_ENABLE_FBUS2PL2303)
/**
 * Writes (AT) command to device and reads reply.
 *
 * \todo This makes no reply parsing or error detection.
 */
static void FBUS2_WriteDLR3(GSM_StateMachine *s, const char *command, int length, int timeout)
{
	unsigned char		buff[300];
	int			w = 0;
	gboolean			wassomething = FALSE;
	int recvlen;


	GSM_DumpMessageLevel2(s, command, length, 0xff);
	s->Device.Functions->WriteDevice(s, command, length);

	for (w = 0; w < timeout; w++) {
		recvlen = s->Device.Functions->ReadDevice(s, buff, sizeof(buff) - 1);
		if (wassomething && recvlen == 0) {
			return;
		} else if (recvlen > 0) {
			GSM_DumpMessageLevel2Recv(s, buff, recvlen, 0xff);
			wassomething = TRUE;
		}
		usleep(50000);
	}
}

/**
 * Performs switch to FBUS2 protocol using AT commands.
 *
 * \todo We should check return codes here.
 */
static GSM_Error FBUS2_ATSwitch(GSM_StateMachine *s)
{
	static const char init_1[] = "AT\r\n";
	static const char init_2[] = "AT&F\r\n";
	static const char init_3[] = "AT*NOKIAFBUS\r\n";

	smprintf(s, "Switching to FBUS using AT commands\n");

	FBUS2_WriteDLR3(s, init_1, strlen(init_1), 100);
	FBUS2_WriteDLR3(s, init_2, strlen(init_2), 100);
	FBUS2_WriteDLR3(s, init_3, strlen(init_3), 100);

	return ERR_NONE;
}
#endif

/**
 * Performs initial synchronisation of FBUS2.
 */
static GSM_Error FBUS2_InitSequence(GSM_StateMachine *s, const int repeats, const int delays, const gboolean terminate)
{
	int count;
	static const unsigned char init_char = 0x55;
	static const unsigned char end_init_char = 0xc1;

	for (count = 0; count < repeats; count ++) {
		if (s->Device.Functions->WriteDevice(s, &init_char, 1) != 1) {
			return ERR_DEVICEWRITEERROR;
		}
		if (delays > 0) {
			usleep(delays);
		}
	}

	if (terminate) {
		if (s->Device.Functions->WriteDevice(s, &end_init_char, 1) != 1) {
			return ERR_DEVICEWRITEERROR;
		}
	}

	sleep(1);

	return ERR_NONE;
}

static GSM_Error FBUS2_Initialise(GSM_StateMachine *s)
{
	GSM_Protocol_FBUS2Data	*d		= &s->Protocol.Data.FBUS2;
	GSM_Device_Functions	*Device 	= s->Device.Functions;
	GSM_Error		error;
	unsigned char		buff[300];

	d->Msg.Length		= 0;
	d->Msg.Buffer		= NULL;
	d->MultiMsg.BufferUsed	= 0;
	d->MultiMsg.Length	= 0;
	d->MultiMsg.Buffer	= NULL;

	d->MsgSequenceNumber	= 0;
	d->FramesToGo		= 0;
	d->MsgRXState		= RX_Sync;

	error = Device->DeviceSetParity(s, FALSE);
	if (error != ERR_NONE) return error;

	switch (s->ConnectionType) {
#if defined(GSM_ENABLE_BLUEFBUS2) || defined(GSM_ENABLE_FBUS2BLUE)
	case GCT_FBUS2BLUE:
	case GCT_BLUEFBUS2:
		error = FBUS2_ATSwitch(s);
		if (error != ERR_NONE) return error;
		break;
#endif
#if defined(GSM_ENABLE_FBUS2DLR3) || defined(GSM_ENABLE_DKU5FBUS2) || defined(GSM_ENABLE_FBUS2PL2303)
	case GCT_DKU5FBUS2:
	case GCT_FBUS2PL2303:
	case GCT_FBUS2DLR3:
		error = Device->DeviceSetDtrRts(s,FALSE,FALSE);
		if (error != ERR_NONE) return error;
		sleep(1);

		if (! s->NoPowerCable) {
			error = Device->DeviceSetDtrRts(s,TRUE,TRUE);
			if (error != ERR_NONE) return error;
			sleep(1);
		}

		error = Device->DeviceSetSpeed(s,19200);
		if (error != ERR_NONE) return error;

		error = FBUS2_ATSwitch(s);
		if (error != ERR_NONE) return error;

		error = Device->DeviceSetSpeed(s,115200);
		if (error != ERR_NONE) return error;

		error = FBUS2_InitSequence(s, 32, 0, TRUE);
		if (error != ERR_NONE) return error;

		break;
#endif
	case GCT_FBUS2:
		error = Device->DeviceSetSpeed(s,115200);
		if (error != ERR_NONE) return error;

		/* Set DTR as power supply if needed, RTS is always low */
		error = Device->DeviceSetDtrRts(s, !(s->NoPowerCable), FALSE);
		if (error != ERR_NONE) return error;

		error = FBUS2_InitSequence(s, 32, 0, TRUE);
		if (error != ERR_NONE) return error;

		break;
#ifdef GSM_ENABLE_FBUS2IRDA
	case GCT_FBUS2IRDA:
		error = Device->DeviceSetSpeed(s,9600);
		if (error != ERR_NONE) return error;

		error = FBUS2_InitSequence(s, 32, 0, TRUE);
		if (error != ERR_NONE) return error;

		error = Device->DeviceSetSpeed(s,115200);
		if (error != ERR_NONE) return error;

		break;
#endif
	default:
		break;
	}

	/* A bit more of synchronisation could be needed here */
	if (s->ConnectionType != GCT_FBUS2BLUE && s->ConnectionType != GCT_BLUEFBUS2) {
		error = FBUS2_InitSequence(s, 250, 100, FALSE);
		if (error != ERR_NONE) return error;
	}

	/* Read any possible junk on the line */
	while (s->Device.Functions->ReadDevice(s, buff, sizeof(buff)) > 0) {
		usleep(1000);
	}

	return ERR_NONE;
}

static GSM_Error FBUS2_Terminate(GSM_StateMachine *s)
{
	free(s->Protocol.Data.FBUS2.Msg.Buffer);
	free(s->Protocol.Data.FBUS2.MultiMsg.Buffer);
	s->Protocol.Data.FBUS2.Msg.Buffer 	= NULL;
	s->Protocol.Data.FBUS2.MultiMsg.Buffer 	= NULL;

	sleep(2);
	return ERR_NONE;
}

GSM_Protocol_Functions FBUS2Protocol = {
	FBUS2_WriteMessage,
	FBUS2_StateMachine,
	FBUS2_Initialise,
	FBUS2_Terminate
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

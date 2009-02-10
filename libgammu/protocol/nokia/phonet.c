/* (c) 2002-2003 by Marcin Wiacek */
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

#if defined(GSM_ENABLE_IRDA) || defined(GSM_ENABLE_PHONETBLUE) || defined(GSM_ENABLE_BLUEPHONET) || defined(GSM_ENABLE_DKU2PHONET)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../gsmcomon.h"
#include "phonet.h"

static GSM_Error PHONET_WriteMessage (GSM_StateMachine 	*s,
				      unsigned const char 	*MsgBuffer,
				      int 		MsgLength,
				      unsigned char 	MsgType)
{
	unsigned char		*buffer2;
	int			sent;

	GSM_DumpMessageLevel3(s, MsgBuffer, MsgLength, MsgType);

	buffer2 = (unsigned char *)malloc(MsgLength + 6);
	if (buffer2 == NULL) return ERR_MOREMEMORY;

	switch (s->ConnectionType) {
		case GCT_PHONETBLUE:
		case GCT_BLUEPHONET:
			buffer2[0] = PHONET_BLUE_FRAME_ID;
			buffer2[1] = PHONET_DEVICE_PHONE;	/* destination */
			buffer2[2] = PHONET_BLUE_DEVICE_PC;	/* source */
			break;
		case GCT_DKU2PHONET:
		case GCT_FBUS2USB:
			buffer2[0] = PHONET_DKU2_FRAME_ID;
			buffer2[1] = PHONET_DEVICE_PHONE; 		/* destination */
			buffer2[2] = PHONET_DEVICE_PC;    		/* source */
			break;
		default:
			buffer2[0] = PHONET_FRAME_ID,
			buffer2[1] = PHONET_DEVICE_PHONE; 		/* destination */
			buffer2[2] = PHONET_DEVICE_PC;    		/* source */
			break;
	}

	buffer2[3] = MsgType;
	buffer2[4] = MsgLength / 256;
	buffer2[5] = MsgLength % 256;

	memcpy(buffer2 + 6, MsgBuffer, MsgLength);

	GSM_DumpMessageLevel2(s, buffer2+6, MsgLength, MsgType);

	/* Sending to phone */
	sent = s->Device.Functions->WriteDevice(s,buffer2,MsgLength+6);

	free(buffer2);

	if (sent!=MsgLength+6) return ERR_DEVICEWRITEERROR;
	return ERR_NONE;
}

static GSM_Error PHONET_StateMachine(GSM_StateMachine *s, unsigned char rx_char)
{
	GSM_Protocol_PHONETData 	*d = &s->Protocol.Data.PHONET;
	bool				correct = false;

	if (d->MsgRXState==RX_GetMessage) {
		d->Msg.Buffer[d->Msg.Count] = rx_char;
		d->Msg.Count++;

		/* This is not last byte in frame */
		if (d->Msg.Count != d->Msg.Length) return ERR_NONE;

		s->Phone.Data.RequestMsg	= &d->Msg;
		s->Phone.Data.DispatchError	= s->Phone.Functions->DispatchMessage(s);

		free(d->Msg.Buffer);
		d->Msg.Length 			= 0;
		d->Msg.Buffer 			= NULL;

		d->MsgRXState 			= RX_Sync;
		return ERR_NONE;
	}
	if (d->MsgRXState==RX_GetLength2) {
		d->Msg.Length 	= d->Msg.Length + rx_char;
		d->Msg.Buffer 	= (unsigned char *)malloc(d->Msg.Length);

		d->MsgRXState 	= RX_GetMessage;
		return ERR_NONE;
	}
	if (d->MsgRXState==RX_GetLength1) {
		d->Msg.Length = rx_char * 256;

		d->MsgRXState = RX_GetLength2;
		return ERR_NONE;
	}
	if (d->MsgRXState==RX_GetType) {
		d->Msg.Type   = rx_char;

		d->MsgRXState = RX_GetLength1;
		return ERR_NONE;
	}
	if (d->MsgRXState==RX_GetSource) {
		if (rx_char != PHONET_DEVICE_PHONE) {
			smprintf_level(s, D_ERROR, "[ERROR: incorrect char - %02x, not %02x]\n",
					rx_char, PHONET_DEVICE_PHONE);
			d->MsgRXState = RX_Sync;
			return ERR_NONE;
		}
		d->Msg.Source = rx_char;

		d->MsgRXState = RX_GetType;
		return ERR_NONE;
	}
	if (d->MsgRXState==RX_GetDestination) {
		switch (s->ConnectionType) {
		case GCT_DKU2PHONET:
		case GCT_IRDAPHONET:
		case GCT_FBUS2USB:
			if (rx_char == PHONET_DEVICE_PC) correct = true;
			break;
		case GCT_PHONETBLUE:
		case GCT_BLUEPHONET:
			if (rx_char == PHONET_BLUE_DEVICE_PC) correct = true;
			break;
		default:
			break;
		}
		if (!correct) {
			smprintf_level(s, D_ERROR, "[ERROR: incorrect char - %02x, not %02x]\n", rx_char, PHONET_DEVICE_PC);
			d->MsgRXState = RX_Sync;
			return ERR_NONE;
		}
		d->Msg.Destination 	= rx_char;

		d->MsgRXState 		= RX_GetSource;
		return ERR_NONE;
	}
	if (d->MsgRXState==RX_Sync) {
		switch (s->ConnectionType) {
		case GCT_IRDAPHONET:
			if (rx_char == PHONET_FRAME_ID) correct = true;
			break;
		case GCT_PHONETBLUE:
		case GCT_BLUEPHONET:
			if (rx_char == PHONET_BLUE_FRAME_ID) correct = true;
			break;
		case GCT_DKU2PHONET:
		case GCT_FBUS2USB:
			if (rx_char == PHONET_DKU2_FRAME_ID) correct = true;
			break;
		default:
			break;
		}
		if (!correct) {
			smprintf_level(s, D_ERROR, "[ERROR: incorrect char - %02x, not %02x]\n", rx_char, PHONET_FRAME_ID);
			return ERR_NONE;
		}
		d->Msg.Count  = 0;

		d->MsgRXState = RX_GetDestination;
		return ERR_NONE;
	}
	return ERR_NONE;
}

static GSM_Error PHONET_Initialise(GSM_StateMachine *s)
{
	int 				total = 0, i, n;
	GSM_Protocol_PHONETData 	*d = &s->Protocol.Data.PHONET;
	unsigned char			req[65536];

	d->Msg.Length	= 0;
	d->Msg.Buffer	= NULL;
	d->MsgRXState	= RX_Sync;

	if (s->ConnectionType == GCT_PHONETBLUE || s->ConnectionType == GCT_BLUEPHONET) {
		/* Send frame in PHONET style */
		req[0] = PHONET_BLUE_FRAME_ID;  req[1] = PHONET_DEVICE_PHONE;
		req[2] = PHONET_BLUE_DEVICE_PC; req[3] = 0xD0;
		req[4] = 0x00;			req[5] = 0x01;
		req[6] = 0x04;
		if (s->Device.Functions->WriteDevice(s,req,7) != 7) return ERR_DEVICEWRITEERROR;

		while (total < 7) {
			n = s->Device.Functions->ReadDevice(s, req + total, sizeof(req) - total);
			total += n;
		}

		/* Answer frame in PHONET style */
		req[10] = PHONET_BLUE_FRAME_ID; req[11] = PHONET_BLUE_DEVICE_PC;
		req[12] = PHONET_DEVICE_PHONE;	req[13] = 0xD0;
		req[14] = 0x00;			req[15] = 0x01;
		req[16] = 0x05;

		for (i = 0; i < 7; i++) {
			if (req[i] != req[10+i]) {
				smprintf(s,"Incorrect byte in the answer\n");
				return ERR_UNKNOWN;
			}
		}
	}

	return ERR_NONE;
}

static GSM_Error PHONET_Terminate(GSM_StateMachine *s)
{
	free(s->Protocol.Data.PHONET.Msg.Buffer);
	return ERR_NONE;
}

GSM_Protocol_Functions PHONETProtocol = {
	PHONET_WriteMessage,
	PHONET_StateMachine,
	PHONET_Initialise,
	PHONET_Terminate
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

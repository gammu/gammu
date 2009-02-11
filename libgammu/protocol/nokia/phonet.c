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
	unsigned char		*buffer;
	int			sent;
	GSM_Protocol_PHONETData 	*d = &s->Protocol.Data.PHONET;

	GSM_DumpMessageLevel3(s, MsgBuffer, MsgLength, MsgType);

	buffer = (unsigned char *)malloc(MsgLength + 6);
	if (buffer == NULL) return ERR_MOREMEMORY;

	buffer[0] = d->frame_id;
	buffer[1] = d->device_phone;
	buffer[2] = d->device_pc;
	buffer[3] = MsgType;
	buffer[4] = MsgLength / 256;
	buffer[5] = MsgLength % 256;

	memcpy(buffer + 6, MsgBuffer, MsgLength);

	GSM_DumpMessageLevel2(s, buffer + 6, MsgLength, MsgType);

	/* Sending to phone */
	sent = s->Device.Functions->WriteDevice(s, buffer, MsgLength + 6);

	free(buffer);

	if (sent != MsgLength + 6)
		return ERR_DEVICEWRITEERROR;
	return ERR_NONE;
}

static GSM_Error PHONET_StateMachine(GSM_StateMachine *s, unsigned char rx_char)
{
	GSM_Protocol_PHONETData 	*d = &s->Protocol.Data.PHONET;

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
		if (rx_char != d->device_phone) {
			smprintf_level(s, D_ERROR, "[ERROR: incorrect source ID - 0x%02x, not 0x%02x]\n", rx_char, d->device_phone);
			d->MsgRXState = RX_Sync;
			return ERR_NONE;
		}
		d->Msg.Source = rx_char;

		d->MsgRXState = RX_GetType;
		return ERR_NONE;
	}
	if (d->MsgRXState==RX_GetDestination) {
		if (rx_char != d->device_pc) {
			smprintf_level(s, D_ERROR, "[ERROR: incorrect destination ID - 0x%02x, not 0x%02x]\n", rx_char, d->device_pc);
			d->MsgRXState = RX_Sync;
			return ERR_NONE;
		}
		d->Msg.Destination 	= rx_char;

		d->MsgRXState 		= RX_GetSource;
		return ERR_NONE;
	}
	if (d->MsgRXState==RX_Sync) {
		if (rx_char != d->frame_id) {
			smprintf_level(s, D_ERROR, "[ERROR: incorrect frame ID - 0x%02x, not 0x%02x]\n", rx_char, d->frame_id);
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
	int 				total = 0;
	GSM_Protocol_PHONETData 	*d = &s->Protocol.Data.PHONET;
	unsigned char			req[10];

	d->Msg.Length	= 0;
	d->Msg.Buffer	= NULL;
	d->MsgRXState	= RX_Sync;

	/* Connection type specific ids */
	switch (s->ConnectionType) {
		case GCT_PHONETBLUE:
		case GCT_BLUEPHONET:
			d->frame_id = PHONET_BLUE_FRAME_ID;
			d->device_phone =  PHONET_DEVICE_PHONE;
			d->device_pc = PHONET_BLUE_DEVICE_PC;
			break;
		case GCT_DKU2PHONET:
		case GCT_FBUS2USB:
			d->frame_id = PHONET_DKU2_FRAME_ID;
			d->device_phone =  PHONET_DEVICE_PHONE;
			d->device_pc = PHONET_DEVICE_PC;
			break;
		case GCT_IRDAPHONET:
		default:
			d->frame_id = PHONET_FRAME_ID;
			d->device_phone =  PHONET_DEVICE_PHONE;
			d->device_pc = PHONET_DEVICE_PC;
			break;
	}

	if (s->ConnectionType == GCT_PHONETBLUE || s->ConnectionType == GCT_BLUEPHONET) {
		/* Send frame in PHONET style */
		req[0] = d->frame_id;
		req[1] = d->device_phone;
		req[2] = d->device_pc;
		req[3] = 0xD0;
		req[4] = 0x00;
		req[5] = 0x01;
		req[6] = 0x04;
		if (s->Device.Functions->WriteDevice(s,req,7) != 7) return ERR_DEVICEWRITEERROR;

		while (total < 7) {
			total += s->Device.Functions->ReadDevice(s, req + total, sizeof(req) - total);
		}
		if (req[0] != d->frame_id) {
			smprintf_level(s, D_ERROR, "Phonet_init: invalid frame id 0x%02x!\n", req[0]);
			return ERR_UNKNOWN;
		}
		if (req[1] != d->device_pc) {
			smprintf_level(s, D_ERROR, "Phonet_init: invalid destination id 0x%02x!\n", req[1]);
			return ERR_UNKNOWN;
		}
		if (req[2] != d->device_phone) {
			smprintf_level(s, D_ERROR, "Phonet_init: invalid source id 0x%02x!\n", req[2]);
			return ERR_UNKNOWN;
		}
		if (req[3] != 0xD0) {
			smprintf_level(s, D_ERROR, "Phonet_init: invalid char id 0x%02x!\n", req[3]);
			return ERR_UNKNOWN;
		}
		if (req[4] != 0x00) {
			smprintf_level(s, D_ERROR, "Phonet_init: invalid char id 0x%02x!\n", req[4]);
			return ERR_UNKNOWN;
		}
		if (req[5] != 0x01) {
			smprintf_level(s, D_ERROR, "Phonet_init: invalid char id 0x%02x!\n", req[5]);
			return ERR_UNKNOWN;
		}
		if (req[6] != 0x05) {
			smprintf_level(s, D_ERROR, "Phonet_init: invalid char id 0x%02x!\n", req[6]);
			return ERR_UNKNOWN;
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

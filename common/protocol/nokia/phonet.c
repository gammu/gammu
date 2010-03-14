
#include "../../gsmstate.h"

#if defined(GSM_ENABLE_IRDA) || defined(GSM_ENABLE_PHONETBLUE) || defined(GSM_ENABLE_BLUEPHONET)

#include <stdio.h>
#include <string.h>

#include "../../gsmcomon.h"
#include "phonet.h"

static GSM_Error PHONET_WriteMessage (GSM_StateMachine *s, unsigned char *buffer,
				     int length, unsigned char type)
{
	unsigned char	*out_buffer; 
	int		current=0, sent;

	GSM_DumpMessageLevel3(s, buffer, length, type);

	out_buffer = (unsigned char *)malloc(length + 6);
	
	/* Now construct the message header. */
	if (s->ConnectionType==GCT_PHONETBLUE || s->ConnectionType==GCT_BLUEPHONET) {
		out_buffer[current++] = PHONET_BLUE_FRAME_ID;	/* Start of the frame indicator */
		out_buffer[current++] = PHONET_DEVICE_PHONE; 	/* Destination */
		out_buffer[current++] = PHONET_BLUE_DEVICE_PC;  /* Source */
	} else {
		out_buffer[current++] = PHONET_FRAME_ID;	/* Start of the frame indicator */
		out_buffer[current++] = PHONET_DEVICE_PHONE; 	/* Destination */
		out_buffer[current++] = PHONET_DEVICE_PC;    	/* Source */
	}
	out_buffer[current++] = type;               /* Type */
	out_buffer[current++] = length / 256;       /* Length */
	out_buffer[current++] = length % 256;       /* Length */

	/* Copy in data if any. */		
	if (length != 0) {
		memcpy(out_buffer + current, buffer, length);
		current+=length;
	}

	GSM_DumpMessageLevel2(s, out_buffer+6, length, type);

	/* Send it out... */
	sent = s->Device.Functions->WriteDevice(s,out_buffer,current);

	free(out_buffer);

	if (sent!=current) return GE_DEVICEWRITEERROR;
	return GE_NONE;
}

static GSM_Error PHONET_StateMachine(GSM_StateMachine *s, unsigned char rx_byte)
{
	GSM_Protocol_PHONETData 	*d = &s->Protocol.Data.PHONET;
	bool				correct = false;

	switch (d->MsgRXState) {

	case RX_Sync:

	switch (s->ConnectionType) {
	case GCT_IRDAPHONET:
		if (rx_byte == PHONET_FRAME_ID) correct = true;
		break;
	case GCT_PHONETBLUE:
	case GCT_BLUEPHONET:
		if (rx_byte == PHONET_BLUE_FRAME_ID) correct = true;
		break;
	default:
		break;
	}
	if (correct) {
		d->Msg.Count  = 0;
		d->MsgRXState = RX_GetDestination;	
	} else {
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR ||
		    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl==DL_TEXTERRORDATE) {
			smprintf(s,"[ERROR: incorrect char - %02x, not %02x]\n", rx_byte, PHONET_FRAME_ID);
		}		
	}
	break;

	case RX_GetDestination:

	switch (s->ConnectionType) {
	case GCT_IRDAPHONET:
		if (rx_byte == PHONET_DEVICE_PC) correct = true;
		break;
	case GCT_PHONETBLUE:
	case GCT_BLUEPHONET:
		if (rx_byte == PHONET_BLUE_DEVICE_PC) correct = true;
		break;
	default:
		break;
	}
	if (!correct) {
		d->MsgRXState = RX_Sync;
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR ||
		    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl==DL_TEXTERRORDATE) {
		    smprintf(s,"[ERROR: incorrect char - %02x, not %02x]\n", rx_byte, PHONET_DEVICE_PC);
		}
	} else {
		d->Msg.Destination 	= rx_byte;
		d->MsgRXState 		= RX_GetSource;
	}
	break;

	case RX_GetSource:

	if (rx_byte != PHONET_DEVICE_PHONE) {
		d->MsgRXState = RX_Sync;
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR ||
		    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl==DL_TEXTERRORDATE) {
		    smprintf(s,"[ERROR: incorrect char - %02x, not %02x]\n", rx_byte, PHONET_DEVICE_PHONE);
		}
	} else {
		d->Msg.Source = rx_byte;
		d->MsgRXState = RX_GetType;
	}
	break;

	case RX_GetType:

	d->Msg.Type   = rx_byte;
	d->MsgRXState = RX_GetLength1;
	break;

	case RX_GetLength1:

	d->Msg.Length = rx_byte * 256;
	d->MsgRXState = RX_GetLength2;
	break;
    
	case RX_GetLength2:

	d->Msg.Length 	= d->Msg.Length + rx_byte;
	d->Msg.Buffer 	= (unsigned char *)malloc(d->Msg.Length);
	d->MsgRXState 	= RX_GetMessage;

	break;
    
	case RX_GetMessage:

	d->Msg.Buffer[d->Msg.Count] = rx_byte;
	d->Msg.Count++;

	if (d->Msg.Count == d->Msg.Length) {
		s->Phone.Data.RequestMsg	= &d->Msg;
		s->Phone.Data.DispatchError	= s->Phone.Functions->DispatchMessage(s);

		free(d->Msg.Buffer);
		d->Msg.Length 			= 0;
		d->Msg.Buffer 			= NULL;

		d->MsgRXState 			= RX_Sync;
	}
	break;

	}

	return GE_NONE;
}

static GSM_Error PHONET_Initialise(GSM_StateMachine *s)
{
	int 				total = 0, i, n;
	GSM_Protocol_PHONETData 	*d = &s->Protocol.Data.PHONET;
	unsigned char			req[50];

	d->Msg.Length	= 0;
	d->Msg.Buffer	= NULL;
	d->MsgRXState	= RX_Sync;

	if (s->ConnectionType == GCT_PHONETBLUE || s->ConnectionType == GCT_BLUEPHONET) {
		/* Send frame in PHONET style */
		req[0] = PHONET_BLUE_FRAME_ID;  req[1] = PHONET_DEVICE_PHONE;
		req[2] = PHONET_BLUE_DEVICE_PC; req[3] = 0xD0;
		req[4] = 0x00;			req[5] = 0x01;
		req[6] = 0x04;
		if (s->Device.Functions->WriteDevice(s,req,7) != 7) return GE_DEVICEWRITEERROR;

		while (total < 7) {
			n = s->Device.Functions->ReadDevice(s, req + total, 50 - total);
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
				return GE_UNKNOWN;
			}
		}
	}

	return GE_NONE;
}

static GSM_Error PHONET_Terminate(GSM_StateMachine *s)
{
	free(s->Protocol.Data.PHONET.Msg.Buffer);
	return GE_NONE;
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

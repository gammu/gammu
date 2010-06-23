
#include "../../gsmstate.h"

#ifdef GSM_ENABLE_IRDA

#include <stdio.h>
#include <string.h>

#include "../../gsmcomon.h"
#include "fbusirda.h"

static GSM_Error FBUS2IRDA_WriteMessage (GSM_StateMachine *s, unsigned char *buffer,
				     int length, unsigned char type)
{
	unsigned char	out_buffer[FBUS2IRDA_MAX_TRANSMIT_LENGTH + 6];  
	int		current=0;
	int		sent;

	GSM_DumpMessageLevel3(s, buffer, length, type);

	/* Now construct the message header. */
	out_buffer[current++] = FBUS2IRDA_FRAME_ID;	/* Start of the frame indicator */
	out_buffer[current++] = FBUS2IRDA_DEVICE_PHONE; /* Destination */
	out_buffer[current++] = FBUS2IRDA_DEVICE_PC;    /* Source */
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
	sent=s->Device.Functions->WriteDevice(s,out_buffer,current);
	if (sent!=current) return GE_DEVICEWRITEERROR;

	return GE_NONE;
}

static GSM_Error FBUS2IRDA_StateMachine(GSM_StateMachine *s, unsigned char rx_byte)
{
	GSM_Protocol_FBUS2IRDAData 	*d = &s->Protocol.Data.FBUS2IRDA;

	switch (d->MsgRXState) {

	case RX_Sync:

	if (rx_byte == FBUS2IRDA_FRAME_ID) {
		d->Msg.Count = 0;
		d->MsgRXState = RX_GetDestination;	
	} else {
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR ||
		    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl==DL_TEXTERRORDATE) {
			smprintf(s,"[ERROR: incorrect char - %02x, not %02x]\n", rx_byte, FBUS2IRDA_FRAME_ID);
		}		
	}
	break;

	case RX_GetDestination:

	if (rx_byte != FBUS2IRDA_DEVICE_PC) {
		d->MsgRXState = RX_Sync;
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR ||
		    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl==DL_TEXTERRORDATE) {
		    smprintf(s,"[ERROR: incorrect char - %02x, not %02x]\n", rx_byte, FBUS2IRDA_DEVICE_PC);
		}
	} else {
		d->Msg.Destination = rx_byte;
		d->MsgRXState = RX_GetSource;
	}
	break;

	case RX_GetSource:

	if (rx_byte != FBUS2IRDA_DEVICE_PHONE) {
		d->MsgRXState = RX_Sync;
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR ||
		    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl==DL_TEXTERRORDATE) {
		    smprintf(s,"[ERROR: incorrect char - %02x, not %02x]\n", rx_byte, FBUS2IRDA_DEVICE_PHONE);
		}
	} else {
		d->Msg.Source = rx_byte;
		d->MsgRXState = RX_GetType;
	}
	break;

	case RX_GetType:

	d->Msg.Type = rx_byte;
	d->MsgRXState = RX_GetLength1;
	break;

	case RX_GetLength1:

	d->Msg.Length = rx_byte * 256;
	d->MsgRXState = RX_GetLength2;
	break;
    
	case RX_GetLength2:

	d->Msg.Length = d->Msg.Length + rx_byte;
	d->MsgRXState = RX_GetMessage;
	break;
    
	case RX_GetMessage:

	d->Msg.Buffer[d->Msg.Count] = rx_byte;
	d->Msg.Count++;

	if (d->Msg.Count == d->Msg.Length) {
		s->Phone.Data.RequestMsg=&d->Msg;
		s->Phone.Data.DispatchError=s->Phone.Functions->DispatchMessage(s);
		d->MsgRXState = RX_Sync;
	}
	break;

	}

	return GE_NONE;
}

static GSM_Error FBUS2IRDA_Initialise(GSM_StateMachine *s)
{
	GSM_Protocol_FBUS2IRDAData *d = &s->Protocol.Data.FBUS2IRDA;

	d->MsgRXState=RX_Sync;

	return GE_NONE;
}

GSM_Protocol_Functions FBUS2IRDAProtocol = {
	FBUS2IRDA_WriteMessage,
	FBUS2IRDA_StateMachine,
	FBUS2IRDA_Initialise,
	NONEFUNCTION
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

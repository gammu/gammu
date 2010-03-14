
#include "../../gsmstate.h"

#ifdef GSM_ENABLE_MBUS2

#include <stdio.h>
#include <string.h>

#include "../../gsmcomon.h"
#include "mbus2.h"

static GSM_Error MBUS2_WriteMessage (GSM_StateMachine *s, unsigned char *buffer,
				     int length, unsigned char type)
{
	unsigned char 		out_buffer[MBUS2_MAX_TRANSMIT_LENGTH];
	GSM_Protocol_MBUS2Data 	*d = &s->Protocol.Data.MBUS2;
	unsigned char 		checksum = 0;
	int 			count, current=0;
	int 			sent;

	GSM_DumpMessageLevel3(s, buffer, length, type);

	/* Now construct the message header. */
	out_buffer[current++] = MBUS2_FRAME_ID;     /* Start of the frame indicator */
	out_buffer[current++] = MBUS2_DEVICE_PHONE; /* Destination */
	out_buffer[current++] = MBUS2_DEVICE_PC;    /* Source */
	out_buffer[current++] = type;               /* Type */
	out_buffer[current++] = length/256;	    /* Length */
	out_buffer[current++] = length%256;	    /* Length */

	/* Copy in data if any. */		
	if (length != 0) {
		memcpy(out_buffer + current, buffer, length);
		current+=length;
	}

	/* Checksum problem:
	 * It seems that some phones have problems with a checksum of 1F.
	 * The frame will be recognized but it will not respond with a ACK
	 * frame.
	 * Workaround:
	 * If the checksum will be 1F, increment the sequence number so that
	 * the checksum will be different., recalculate the checksum then and
	 * send.
	 * Source: http://www.flosys.com/tdma/n5160.html
	 */
	do {
		d->MsgSequenceNumber++;

		out_buffer[current] = d->MsgSequenceNumber;  
	
		/* Now calculate checksum over entire message */
		checksum = 0;
		for (count = 0; count < current + 1; count++) checksum ^= out_buffer[count];
	} while (checksum == 0x1f);

	/* Now append sequence and checksum to message. */
	out_buffer[current++] = d->MsgSequenceNumber;  
	out_buffer[current++] = checksum;

	GSM_DumpMessageLevel2(s, out_buffer+6, length, type);

	/* Send it out... */
	my_sleep(10);
	sent=s->Device.Functions->WriteDevice(s,out_buffer,current);
	if (sent!=current) return GE_DEVICEWRITEERROR;

	return GE_NONE;
}

static GSM_Error MBUS2_SendAck(GSM_StateMachine *s, unsigned char type, unsigned char sequence)
{
	GSM_Device_Functions 	*Device		= s->Device.Functions;
	unsigned char 		out_buffer[6];
	int 			count;

	out_buffer[0] = MBUS2_FRAME_ID;
	out_buffer[1] = MBUS2_DEVICE_PHONE;
	out_buffer[2] = MBUS2_DEVICE_PC;
	out_buffer[3] = MBUS2_ACK_BYTE;
	out_buffer[4] = sequence;
	out_buffer[5] = 0;

	/* Checksum */
	for (count = 0; count < 5; count++) out_buffer[5] ^= out_buffer[count];

	if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL ||
	    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE) {
	    smprintf(s,"[Sending Ack of type %02x, seq: %x]\n",type,sequence);
	}

	/* Send it out... */
	my_sleep(10);
	if (Device->WriteDevice(s,out_buffer,6)!=6) return GE_DEVICEWRITEERROR;

	return GE_NONE;
}

static GSM_Error MBUS2_StateMachine(GSM_StateMachine *s, unsigned char rx_byte)
{
	GSM_Phone_Functions 	*Phone	= s->Phone.Functions;
	GSM_Protocol_MBUS2Data 	*d	= &s->Protocol.Data.MBUS2;

	d->Msg.CheckSum[0] = d->Msg.CheckSum[1];
	d->Msg.CheckSum[1] ^= rx_byte;

	switch (d->MsgRXState) {

	case RX_Sync:

	if (rx_byte == MBUS2_FRAME_ID) {
		d->Msg.CheckSum[1] 	= MBUS2_FRAME_ID;
		d->Msg.Count 		= 0;
		d->MsgRXState 		= RX_GetDestination;	
	} else {
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR ||
		    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl==DL_TEXTERRORDATE) {
		    smprintf(s,"[ERROR: incorrect char - %02x, not %02x]\n", rx_byte, MBUS2_FRAME_ID);
		}
	}
	break;

	case RX_GetDestination:

	if (rx_byte != MBUS2_DEVICE_PC && rx_byte != MBUS2_DEVICE_PHONE) {
		d->MsgRXState = RX_Sync;
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR ||
		    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl==DL_TEXTERRORDATE) {
		    smprintf(s,"[ERROR: incorrect char - %02x, not %02x and %02x]\n", rx_byte, MBUS2_DEVICE_PHONE, MBUS2_DEVICE_PC);
		}
	} else {
		d->Msg.Destination 	= rx_byte;
		d->MsgRXState 		= RX_GetSource;
	}
	break;

	case RX_GetSource:

	if (rx_byte != MBUS2_DEVICE_PHONE && rx_byte != MBUS2_DEVICE_PC) {
		d->MsgRXState = RX_Sync;
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR ||
		    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl==DL_TEXTERRORDATE) {
		    smprintf(s,"[ERROR: incorrect char - %02x, not %02x and %02x]\n", rx_byte, MBUS2_DEVICE_PHONE, MBUS2_DEVICE_PC);
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

	if (d->Msg.Type == MBUS2_ACK_BYTE)
	{
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL ||
		    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE) {
		    smprintf(s,"[Received Ack]\n");
		}
		d->MsgRXState = RX_Sync;
	} else {
		d->Msg.Length = d->Msg.Length + rx_byte;
		d->MsgRXState = RX_GetMessage;
	}    
	break;
    
	case RX_GetMessage:

	d->Msg.Buffer[d->Msg.Count] = rx_byte;
	d->Msg.Count++;

	if (d->Msg.Count == d->Msg.Length+2) {
		/* If this is the last byte, it's the checksum. */
		/* Is the checksum correct? */        
		if (d->Msg.CheckSum[0] == rx_byte) {
			if (d->Msg.Destination != MBUS2_DEVICE_PHONE) {
				MBUS2_SendAck(s, d->Msg.Type, d->Msg.Buffer[d->Msg.Count-2]);
				s->Phone.Data.RequestMsg=&d->Msg;
				s->Phone.Data.DispatchError=Phone->DispatchMessage(s);
			}
		} else {
			if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR ||
			    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl==DL_TEXTERRORDATE) {
			    smprintf(s,"[ERROR: checksum]\n");
			}
		}
		d->MsgRXState = RX_Sync;
	}
	break;

	}

	return GE_NONE;
}

static GSM_Error MBUS2_Initialise(GSM_StateMachine *s)
{
	GSM_Device_Functions 	*Device	= s->Device.Functions;
	GSM_Protocol_MBUS2Data 	*d	= &s->Protocol.Data.MBUS2;
	GSM_Error 		error;

	d->MsgSequenceNumber	= 0;
	d->MsgRXState		= RX_Sync;

	error=Device->DeviceSetSpeed(s,9600);
	if (error!=GE_NONE) return error;

	error=Device->DeviceSetParity(s,true);
	if (error!=GE_NONE) return error;

	error=Device->DeviceSetDtrRts(s,false,true); /*DTR low,RTS high*/
	if (error!=GE_NONE) return error; 
        my_sleep(200);

	return GE_NONE;
}

GSM_Protocol_Functions MBUS2Protocol = {
	MBUS2_WriteMessage,
	MBUS2_StateMachine,
	MBUS2_Initialise,
	NONEFUNCTION
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */


#include "../../gsmstate.h"

#ifdef GSM_ENABLE_MBUS2

#include <stdio.h>
#include <string.h>

#include "../../gsmcomon.h"
#include "mbus2.h"

static GSM_Error MBUS2_WriteMessage (GSM_StateMachine *s, unsigned char *buffer,
				     int length, unsigned char type)
{
	unsigned char 		*out_buffer, checksum = 0;
	GSM_Protocol_MBUS2Data 	*d = &s->Protocol.Data.MBUS2;
	int 			count, current=0, sent;

	GSM_DumpMessageLevel3(s, buffer, length, type);

	out_buffer = (unsigned char *)malloc(length + 8);

	/* Preparing header of message */
	out_buffer[current++] = MBUS2_FRAME_ID;
	out_buffer[current++] = MBUS2_DEVICE_PHONE; /* Frame destination */
	out_buffer[current++] = MBUS2_DEVICE_PC;    /* Frame source */
	out_buffer[current++] = type;               /* Frame type */
	out_buffer[current++] = length/256;	    /* Frame length */
	out_buffer[current++] = length%256;	    /* Frame length */

	/* Copying frame */
	if (length != 0) {
		memcpy(out_buffer + current, buffer, length);
		current+=length;
	}

	/* According to http://www.flosys.com/tdma/n5160.html some phones
         * can have problems with checksum equal 0x1F. Phones can recognize
         * received frame, but won't send ACK for it. When checksum is 0x1F,
         * we increment the sequence number
         */
	do {
		d->MsgSequenceNumber++;

		out_buffer[current] = d->MsgSequenceNumber;  
	
		/* Calculating checksum */
		checksum = 0;
		for (count = 0; count < current + 1; count++) checksum ^= out_buffer[count];
	} while (checksum == 0x1f);

	/* Adding sequence and checksum */
	out_buffer[current++] = d->MsgSequenceNumber;  
	out_buffer[current++] = checksum;

	/* Writing debug output */
	GSM_DumpMessageLevel2(s, out_buffer+6, length, type);

	/* Sending to phone */
	my_sleep(10);
	sent=s->Device.Functions->WriteDevice(s,out_buffer,current);

	free(out_buffer);

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

	/* Calculating checksum */
	for (count = 0; count < 5; count++) out_buffer[5] ^= out_buffer[count];

	if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL ||
	    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE) {
	    smprintf(s,"[Sending Ack of type %02x, seq: %x]\n",type,sequence);
	}

	/* Sending to phone */
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
		if (d->Msg.BufferUsed < d->Msg.Length+2) {
			d->Msg.BufferUsed 	= d->Msg.Length+2;
			d->Msg.Buffer 		= (unsigned char *)realloc(d->Msg.Buffer,d->Msg.BufferUsed);
		}
		d->MsgRXState 	= RX_GetMessage;
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
				s->Phone.Data.RequestMsg	= &d->Msg;
				s->Phone.Data.DispatchError	= Phone->DispatchMessage(s);
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

	d->Msg.Length		= 0;
	d->Msg.BufferUsed	= 0;
	d->Msg.Buffer		= NULL;

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

static GSM_Error MBUS2_Terminate(GSM_StateMachine *s)
{
	free(s->Protocol.Data.MBUS2.Msg.Buffer);
	return GE_NONE;
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

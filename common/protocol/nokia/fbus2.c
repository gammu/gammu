
#include "../../gsmstate.h"

#if defined(GSM_ENABLE_FBUS2) || defined(GSM_ENABLE_IRDA) || defined(GSM_ENABLE_DLR3AT) || defined(GSM_ENABLE_DLR3BLUETOOTH)

#include <stdio.h>
#include <string.h>

#include "../../gsmcomon.h"
#include "fbus2.h"

static GSM_Error FBUS2_WriteFrame(GSM_StateMachine *s, unsigned char *buffer,
				  int length, unsigned char type)
{
	unsigned char 	out_buffer[FBUS2_MAX_TRANSMIT_LENGTH + 10];  
	unsigned char 	checksum=0;
	int 		count, current=0;
	int 		sent;

	/* Now construct the message header. */
	if (s->connectiontype==GCT_INFRARED) {
    	    out_buffer[current++] = FBUS2_IR_FRAME_ID;	/* Start of the frame indicator */
	} else {
    	    out_buffer[current++] = FBUS2_FRAME_ID;	/* Start of the frame indicator */
	}
	out_buffer[current++] = FBUS2_DEVICE_PHONE; /* Destination */
	out_buffer[current++] = FBUS2_DEVICE_PC;    /* Source */
	out_buffer[current++] = type;               /* Type */
	out_buffer[current++] = length / 256;       /* Length */
	out_buffer[current++] = length % 256;       /* Length */

	/* Copy in data if any. */		
	if (length != 0) {
		memcpy(out_buffer + current, buffer, length);
		current+=length;
	}

	/* If the message length is odd we should add pad byte 0x00 */
	if (length % 2) out_buffer[current++]=0x00;

	/* Now calculate checksums over entire message and append to message. */

	/* Odd bytes */
	checksum = 0;
	for (count = 0; count < current; count+=2) checksum ^= out_buffer[count];
	out_buffer[current++] = checksum;

	/* Even bytes */
	checksum = 0;
	for (count = 1; count < current; count+=2) checksum ^= out_buffer[count];
	out_buffer[current++] = checksum;

	/* Send it out... */
	sent=s->Device.Functions->WriteDevice(s,out_buffer,current);
	if (sent!=current) return GE_DEVICEWRITEERROR;

	return GE_NONE;
}

static GSM_Error FBUS2_WriteMessage (GSM_StateMachine *s, unsigned char *buffer,
				     int length, unsigned char type)
{
	unsigned char 		out_buffer[FBUS2_MAX_TRANSMIT_LENGTH + 2];
	unsigned char 		seqnum;
	GSM_Protocol_FBUS2Data	*d = &s->Protocol.Data.FBUS2;
	GSM_Error 		error;
	int 			nom, togo, thislength; /* number of messages, ... */
	int 			i;

	GSM_DumpMessageLevel3(s, buffer, length, type);

	nom = (length + FBUS2_MAX_TRANSMIT_LENGTH - 1) / FBUS2_MAX_TRANSMIT_LENGTH;
	togo = length;

	for (i = 0; i < nom; i++) {
		seqnum = d->MsgSequenceNumber;
		if (i==0) seqnum = seqnum + 0x40;
		d->MsgSequenceNumber = (d->MsgSequenceNumber + 1) & 0x07;

		thislength = togo;
		if (togo > FBUS2_MAX_TRANSMIT_LENGTH) thislength = FBUS2_MAX_TRANSMIT_LENGTH;
		memcpy(out_buffer, buffer + (length - togo), thislength);
		out_buffer[thislength]		= nom - i;
		out_buffer[thislength + 1]	= seqnum;
		togo = togo - thislength;

		GSM_DumpMessageLevel2(s, out_buffer, thislength, type);

		error=FBUS2_WriteFrame(s, out_buffer, thislength + 2, type);
		if (error!=GE_NONE) return error;
	}

	return GE_NONE;
}

static GSM_Error FBUS2_SendAck(GSM_StateMachine *s, unsigned char type, unsigned char sequence)
{
	unsigned char out_buffer[2];

	out_buffer[0] = type;
	out_buffer[1] = sequence;

	if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL) {
	    smprintf(s,"[Sending Ack of type %02x, seq: %x]\n",type,sequence);
	}

	/* Send it out... */
	return FBUS2_WriteFrame(s, out_buffer, 2, FBUS2_ACK_BYTE);
}

static GSM_Error FBUS2_StateMachine(GSM_StateMachine *s, unsigned char rx_byte)
{
	GSM_Protocol_FBUS2Data 	*d = &s->Protocol.Data.FBUS2;
	unsigned char 		frm_num, seq_num;
	bool			correct;

	/* XOR the byte with the current checksum */
	d->Msg.CheckSum[d->Msg.Count & 1] ^= rx_byte;

	switch (d->MsgRXState) {

	case RX_Sync:

	correct = false;
	switch (s->connectiontype) {
		case GCT_FBUS2:
		case GCT_DLR3AT:
		case GCT_DLR3BLUE:
			if (rx_byte == FBUS2_FRAME_ID) correct = true;
			break;
		case GCT_INFRARED:
			if (rx_byte == FBUS2_IR_FRAME_ID) correct = true;
			break;
		default:
			break;
	}
	if (correct) {
		/* Initialize checksums. */
		d->Msg.CheckSum[0] = rx_byte;
		d->Msg.CheckSum[1] = 0;

		d->Msg.Count	   = 0;
		d->MsgRXState	   = RX_GetDestination;	
	} else {
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR) {
		    if (s->connectiontype==GCT_INFRARED) {
			smprintf(s,"[ERROR: incorrect char - %02x, not %02x]\n", rx_byte, FBUS2_IR_FRAME_ID);
		    } else {
			smprintf(s,"[ERROR: incorrect char - %02x, not %02x]\n", rx_byte, FBUS2_FRAME_ID);
		    }
		}		
	}
	break;

	case RX_GetDestination:

	if (rx_byte != FBUS2_DEVICE_PC) {
		d->MsgRXState = RX_Sync;
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR) {
		    smprintf(s,"[ERROR: incorrect char - %02x, not %02x]\n", rx_byte, FBUS2_DEVICE_PC);
		}
	} else {
		d->Msg.Destination = rx_byte;
		d->MsgRXState = RX_GetSource;
	}
	break;

	case RX_GetSource:

	if (rx_byte != FBUS2_DEVICE_PHONE) {
		d->MsgRXState = RX_Sync;
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR) {
		    smprintf(s,"[ERROR: incorrect char - %02x, not %02x]\n", rx_byte, FBUS2_DEVICE_PHONE);
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

	if (d->Msg.Count == d->Msg.Length+(d->Msg.Length%2)+2) {
		/* If this is the last byte, it's the checksum. */
		/* Is the checksum correct? */        
		if (d->Msg.CheckSum[0] == d->Msg.CheckSum[1]) {
			seq_num = d->Msg.Buffer[d->Msg.Length-1];

			if (d->Msg.Type == FBUS2_ACK_BYTE) {
				if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL) {
				    smprintf(s, "[Received Ack of type %02x, seq: %02x]\n",d->Msg.Buffer[0],seq_num);
				}
			} else {
				frm_num = d->Msg.Buffer[d->Msg.Length-2];

				if ((seq_num & 0x40) == 0x40) {
					d->FramesToGo		= frm_num;
					d->MultiMsg.Length	= 0;
					d->MultiMsg.Type	= d->Msg.Type;
					d->MultiMsg.Destination	= d->Msg.Destination;
					d->MultiMsg.Source	= d->Msg.Source;
				} else {
					if (d->FramesToGo != frm_num) {
						if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR) {
						    smprintf(s, "[ERROR: Missed part of multiframe msg]\n");
						}
						d->MsgRXState = RX_Sync;
					}
					if (d->Msg.Type != d->MultiMsg.Type) {
						if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR) {
						    smprintf(s, "[ERROR: Multiframe msg in multiframe msg]\n");
						}
						d->MsgRXState = RX_Sync;
					}
				}

				/* No errors */
				if (d->MsgRXState != RX_Sync) {
					memcpy(d->MultiMsg.Buffer+d->MultiMsg.Length,d->Msg.Buffer,d->Msg.Length-2);
					d->MultiMsg.Length=d->MultiMsg.Length+d->Msg.Length-2;

					d->FramesToGo--;

					FBUS2_SendAck(s, d->Msg.Type, ((unsigned char)(seq_num & 0x0f)));
					
					/* Finally dispatch if ready */
					if (d->FramesToGo == 0) {
						s->Phone.Data.RequestMsg	= &d->MultiMsg;
						s->Phone.Data.DispatchError	= s->Phone.Functions->DispatchMessage(s);
					}
				}
			}
		} else {
			if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR) {
			    smprintf(s,"[ERROR: checksum]\n");
			}
		}
		d->MsgRXState = RX_Sync;
	}
	break;

	}

	return GE_NONE;
}

#if defined(GSM_ENABLE_DLR3AT) || defined(GSM_ENABLE_DLR3BLUETOOTH)
static void FBUS2_WriteDLR3(GSM_StateMachine *s, char *command, int length, int timeout)
{
	unsigned char		buff[300];
	int			w = 0;
	bool			wassomething = false;

	s->Device.Functions->WriteDevice(s,command,length);

	for (w=0;w<timeout;w++) {    
		if (wassomething) {
			if (s->Device.Functions->ReadDevice(s, buff, 255)==0) return;
		} else {
			if (s->Device.Functions->ReadDevice(s, buff, 255)>0) wassomething = true;
		}
		mili_sleep(50);
	}
}
#endif

static GSM_Error FBUS2_Initialise(GSM_StateMachine *s)
{
	unsigned char		init_char	= 0x55;
#ifdef GSM_ENABLE_INFRARED
	unsigned char		end_init_char	= 0xc1;
#endif

	GSM_Protocol_FBUS2Data	*d		= &s->Protocol.Data.FBUS2;
	GSM_Device_Functions	*Device 	= s->Device.Functions;
	GSM_Error		error;
	int			count;

	d->MsgSequenceNumber	= 0;
	d->FramesToGo		= 0;
	d->MsgRXState		= RX_Sync;

	error=Device->DeviceSetParity(s,false);
	if (error!=GE_NONE) return error;

	switch (s->connectiontype) {
#ifdef GSM_ENABLE_DLR3BLUETOOTH
	case GCT_DLR3BLUE:
		FBUS2_WriteDLR3(s,"AT\r\n",		 4,10);
		FBUS2_WriteDLR3(s,"AT&F\r\n",		 6,10);
		FBUS2_WriteDLR3(s,"AT*NOKIAFBUS\r\n",	14,10);
		break;
#endif
#ifdef GSM_ENABLE_DLR3AT
	case GCT_DLR3AT:
		error=Device->DeviceSetDtrRts(s,false,false);
	    	if (error!=GE_NONE) return error; 
		mili_sleep(1000);

		error=Device->DeviceSetDtrRts(s,true,true);
	    	if (error!=GE_NONE) return error; 
		error=Device->DeviceSetSpeed(s,19200);
		if (error!=GE_NONE) return error;

		FBUS2_WriteDLR3(s,"AT\r\n",		 4,10);
		FBUS2_WriteDLR3(s,"AT&F\r\n",		 6,10);
		FBUS2_WriteDLR3(s,"AT*NOKIAFBUS\r\n",	14,10);

		error=Device->CloseDevice(s);
		if (error!=GE_NONE) return error;
		mili_sleep(1000);

		error=Device->OpenDevice(s);
		if (error!=GE_NONE) return error;
		error=Device->DeviceSetParity(s,false);
	    	if (error!=GE_NONE) return error;
		error=Device->DeviceSetSpeed(s,115200);
	    	if (error!=GE_NONE) return error; 
		error=Device->DeviceSetDtrRts(s,false,false);
		if (error!=GE_NONE) return error; 
			
		for (count = 0; count < 55; count ++) {
			if (Device->WriteDevice(s,&init_char,1)!=1) return GE_DEVICEWRITEERROR;
		}
		break;
#endif
	case GCT_FBUS2:
		error=Device->DeviceSetSpeed(s,115200);
		if (error!=GE_NONE) return error; 

		error=Device->DeviceSetDtrRts(s,true,false); /*DTR high,RTS low*/
		if (error!=GE_NONE) return error; 

		for (count = 0; count < 55; count ++) {
			if (Device->WriteDevice(s,&init_char,1)!=1) return GE_DEVICEWRITEERROR;
			mili_sleep(10);
		}
		break;
#ifdef GSM_ENABLE_INFRARED
	case GCT_INFRARED:
		error=Device->DeviceSetSpeed(s,9600);
		if (error!=GE_NONE) return error;

		for (count = 0; count < 55; count ++) {
			if (Device->WriteDevice(s,&init_char,1)!=1) return GE_DEVICEWRITEERROR;
			mili_sleep(10);
		}

		if (Device->WriteDevice(s,&end_init_char,1)!=1) return GE_DEVICEWRITEERROR;
		mili_sleep(20);

		error=Device->DeviceSetSpeed(s,115200);
		if (error!=GE_NONE) return error;

		break;			
#endif
	default:
		break;
	}

	return GE_NONE;
}

static GSM_Error FBUS2_Terminate(GSM_StateMachine *s)
{
	mili_sleep(200);
	return GE_NONE;
}

GSM_Protocol_Functions FBUS2Protocol = {
	FBUS2_WriteMessage,
	FBUS2_StateMachine,
	FBUS2_Initialise,
	FBUS2_Terminate
};

#endif

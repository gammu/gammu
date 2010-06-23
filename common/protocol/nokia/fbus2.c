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

#if defined(GSM_ENABLE_FBUS2) || defined(GSM_ENABLE_FBUS2IRDA) || defined(GSM_ENABLE_FBUS2DLR3) || defined(GSM_ENABLE_FBUS2BLUE) || defined(GSM_ENABLE_BLUEFBUS2) || defined(GSM_ENABLE_FBUS2DKU5) || defined(GSM_ENABLE_FBUS2PL2303)

#include <stdio.h>
#include <string.h>

#include "../../gsmcomon.h"
#include "fbus2.h"

static GSM_Error FBUS2_WriteFrame(GSM_StateMachine 	*s,
				  unsigned char 	*MsgBuffer,
				  int 			MsgLength,
				  unsigned char 	MsgType)
{
	unsigned char 		  buffer2[FBUS2_MAX_TRANSMIT_LENGTH + 10];  
	unsigned char 		  checksum=0;
	int 			  i, len, sent;

	buffer2[0] 	= FBUS2_FRAME_ID;
	if (s->ConnectionType==GCT_FBUS2IRDA) buffer2[0] = FBUS2_IRDA_FRAME_ID;

	buffer2[1] 	= FBUS2_DEVICE_PHONE;		//destination
	buffer2[2]	= FBUS2_DEVICE_PC;		//source
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
				     unsigned char 	*MsgBuffer,
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

		error=FBUS2_WriteFrame(s, buffer2, thislength + 2, MsgType);
		if (error!=ERR_NONE) return error;
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

	if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL ||
	    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE) {
		smprintf(s,"[Sending Ack of type %02x, seq %x]\n",buffer2[0],buffer2[1]);
	}

	/* Sending to phone */
	return FBUS2_WriteFrame(s, buffer2, 2, FBUS2_ACK_BYTE);
}

static GSM_Error FBUS2_StateMachine(GSM_StateMachine *s, unsigned char rx_char)
{
	GSM_Protocol_FBUS2Data 	*d = &s->Protocol.Data.FBUS2;
	unsigned char 		frm_num, seq_num;
	bool			correct = false;

	/* XOR the byte with the earlier checksum */
	d->Msg.CheckSum[d->Msg.Count & 1] ^= rx_char;

	if (d->MsgRXState == RX_GetMessage) {
		d->Msg.Buffer[d->Msg.Count] = rx_char;
		d->Msg.Count++;

		/* This is not last byte in frame */
		if (d->Msg.Count != d->Msg.Length+(d->Msg.Length%2)+2) return ERR_NONE;

		/* Checksum is incorrect */
		if (d->Msg.CheckSum[0] != d->Msg.CheckSum[1]) {
			if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR ||
			    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl==DL_TEXTERRORDATE) {
				smprintf(s,"[ERROR: checksum]\n");	
			}
			free(d->Msg.Buffer);
			d->Msg.Length 		= 0;
			d->Msg.Buffer 		= NULL;

			d->MsgRXState 		= RX_Sync;
			return ERR_NONE;
		}

		seq_num = d->Msg.Buffer[d->Msg.Length-1];

		if (d->Msg.Type == FBUS2_ACK_BYTE) {
			if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL ||
			    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE) {
				smprintf(s, "[Received Ack of type %02x, seq %02x]\n",d->Msg.Buffer[0],seq_num);
			}
			free(d->Msg.Buffer);

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
			if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR ||
			    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl==DL_TEXTERRORDATE) {
				smprintf(s, "[ERROR: Missed part of multiframe msg]\n");
			}

			free(d->Msg.Buffer);
			d->Msg.Length 		= 0;
			d->Msg.Buffer 		= NULL;

			d->MsgRXState 		= RX_Sync;
			return ERR_NONE;
		}

		if ((seq_num & 0x40) != 0x40 && d->Msg.Type != d->MultiMsg.Type) {
			if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR ||
			    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl==DL_TEXTERRORDATE) {
				smprintf(s, "[ERROR: Multiframe msg in multiframe msg]\n");
			}

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
			if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR ||
			    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl==DL_TEXTERRORDATE) {
				smprintf(s,"[ERROR: incorrect char - %02x, not %02x]\n", rx_char, FBUS2_DEVICE_PHONE);
			}

			d->MsgRXState = RX_Sync;
			return ERR_NONE;
		}
		d->Msg.Source = rx_char;

		d->MsgRXState = RX_GetType;
		return ERR_NONE;
	}
	if (d->MsgRXState == RX_GetDestination) {
		if (rx_char != FBUS2_DEVICE_PC) {
			if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR ||
			    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl==DL_TEXTERRORDATE) {
				    smprintf(s,"[ERROR: incorrect char - %02x, not %02x]\n", rx_char, FBUS2_DEVICE_PC);
			}

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
			case GCT_FBUS2DKU5:
			case GCT_FBUS2PL2303:
			case GCT_FBUS2BLUE:
			case GCT_BLUEFBUS2:
				if (rx_char == FBUS2_FRAME_ID) correct = true;
				break;
			case GCT_FBUS2IRDA:
				if (rx_char == FBUS2_IRDA_FRAME_ID) correct = true;
				break;
			default:
				break;
		}
		if (!correct) {
			if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR ||
			    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl==DL_TEXTERRORDATE) {
				if (s->ConnectionType==GCT_FBUS2IRDA) {
					smprintf(s,"[ERROR: incorrect char - %02x, not %02x]\n", rx_char, FBUS2_IRDA_FRAME_ID);
			    	} else {
					smprintf(s,"[ERROR: incorrect char - %02x, not %02x]\n", rx_char, FBUS2_FRAME_ID);
			    	}
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

#if defined(GSM_ENABLE_FBUS2DLR3) || defined(GSM_ENABLE_FBUS2DKU5) || defined(GSM_ENABLE_FBUS2BLUE) || defined(GSM_ENABLE_BLUEFBUS2) || defined(GSM_ENABLE_FBUS2PL2303) 
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
		my_sleep(50);
	}
}
#endif

static GSM_Error FBUS2_Initialise(GSM_StateMachine *s)
{
	unsigned char		init_char	= 0x55;
#ifdef GSM_ENABLE_FBUS2IRDA
	unsigned char		end_init_char	= 0xc1;
#endif

	GSM_Protocol_FBUS2Data	*d		= &s->Protocol.Data.FBUS2;
	GSM_Device_Functions	*Device 	= s->Device.Functions;
	GSM_Error		error;
	int			count;

	d->Msg.Length		= 0;
	d->Msg.Buffer		= NULL;
	d->MultiMsg.BufferUsed	= 0;
	d->MultiMsg.Length	= 0;
	d->MultiMsg.Buffer	= NULL;

	d->MsgSequenceNumber	= 0;
	d->FramesToGo		= 0;
	d->MsgRXState		= RX_Sync;

	error=Device->DeviceSetParity(s,false);
	if (error!=ERR_NONE) return error;

	switch (s->ConnectionType) {
#if defined(GSM_ENABLE_BLUEFBUS2) || defined(GSM_ENABLE_FBUS2BLUE)
	case GCT_FBUS2BLUE:
	case GCT_BLUEFBUS2:
		FBUS2_WriteDLR3(s,"AT\r\n",		 4,10);
		FBUS2_WriteDLR3(s,"AT&F\r\n",		 6,10);
		FBUS2_WriteDLR3(s,"AT*NOKIAFBUS\r\n",	14,10);
		break;
#endif
#if defined(GSM_ENABLE_FBUS2DLR3) || defined(GSM_ENABLE_FBUS2DKU5) || defined(GSM_ENABLE_FBUS2PL2303)
	case GCT_FBUS2DKU5:
	case GCT_FBUS2PL2303:
	case GCT_FBUS2DLR3:
		error=Device->DeviceSetDtrRts(s,false,false);
	    	if (error!=ERR_NONE) return error; 
		my_sleep(1000);

		error=Device->DeviceSetDtrRts(s,true,true);
	    	if (error!=ERR_NONE) return error; 
		error=Device->DeviceSetSpeed(s,19200);
		if (error!=ERR_NONE) return error;

		FBUS2_WriteDLR3(s,"AT\r\n",		 4,10);
		FBUS2_WriteDLR3(s,"AT&F\r\n",		 6,10);
		FBUS2_WriteDLR3(s,"AT*NOKIAFBUS\r\n",	14,10);

		error=Device->CloseDevice(s);
		if (error!=ERR_NONE) return error;
		my_sleep(1000);

		error=Device->OpenDevice(s);
		if (error!=ERR_NONE) return error;
		error=Device->DeviceSetParity(s,false);
	    	if (error!=ERR_NONE) return error;
		error=Device->DeviceSetSpeed(s,115200);
	    	if (error!=ERR_NONE) return error; 
		error=Device->DeviceSetDtrRts(s,false,false);
		if (error!=ERR_NONE) return error; 
			
		for (count = 0; count < 55; count ++) {
			if (Device->WriteDevice(s,&init_char,1)!=1) return ERR_DEVICEWRITEERROR;
		}
		break;
#endif
	case GCT_FBUS2:
		error=Device->DeviceSetSpeed(s,115200);
		if (error!=ERR_NONE) return error; 

		error=Device->DeviceSetDtrRts(s,true,false); /*DTR high,RTS low*/
		if (error!=ERR_NONE) return error; 

		for (count = 0; count < 55; count ++) {
			if (Device->WriteDevice(s,&init_char,1)!=1) return ERR_DEVICEWRITEERROR;
			my_sleep(10);
		}
		break;
#ifdef GSM_ENABLE_FBUS2IRDA
	case GCT_FBUS2IRDA:
		error=Device->DeviceSetSpeed(s,9600);
		if (error!=ERR_NONE) return error;

		for (count = 0; count < 55; count ++) {
			if (Device->WriteDevice(s,&init_char,1)!=1) return ERR_DEVICEWRITEERROR;
			my_sleep(10);
		}

		if (Device->WriteDevice(s,&end_init_char,1)!=1) return ERR_DEVICEWRITEERROR;
		my_sleep(20);

		error=Device->DeviceSetSpeed(s,115200);
		if (error!=ERR_NONE) return error;

		break;			
#endif
	default:
		break;
	}

	return ERR_NONE;
}

static GSM_Error FBUS2_Terminate(GSM_StateMachine *s)
{
	free(s->Protocol.Data.FBUS2.Msg.Buffer);
	free(s->Protocol.Data.FBUS2.MultiMsg.Buffer);

	my_sleep(200);
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

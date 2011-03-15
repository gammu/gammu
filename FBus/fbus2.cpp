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


#include "stdafx.h"

#include <stdio.h>
#include <string.h>
#include "DeviceAPI.h"
#include "fbus.h"

#include "fbus2.h"
extern CFBusApp theApp;


unsigned char g_AckSeqNo = 0x00 ;
BOOL g_GetAck = FALSE ;

int FBUS2_ReadDevice (bool wait)
{
	unsigned char	buff[300];
	int		res = 0, count;
	long  waitseconds1=1;
	clock_t waitTime1=0;

	CDeviceAPI	*Device 	= &theApp.m_DeviceAPI;
	if(Device->m_bDeviceOpened ==FALSE)
		return 0;

	if(wait)
			waitTime1 = clock () + waitseconds1 * CLK_TCK ;

	do 
	{
		res =  Device->DeviceAPI_ReadDevice(buff, 255);
		if (!wait) break;
		if (res > 0) break;
		Sleep(5);
	}
	while (clock() < waitTime1 && Device->m_bDeviceOpened);

	for (count = 0; count < res; count++)
	{
		if(Device->m_bDeviceOpened)
			FBUS2_StateMachine(buff[count]);
	}

	return res;

}
int FBUS2_ReadAck (bool wait)
{
	CDeviceAPI	*Device 	= &theApp.m_DeviceAPI;
	if(Device->m_bDeviceOpened ==FALSE)
		return 0;

	int				i =0;
	while (i<1)
	{
		if (FBUS2_ReadDevice(wait)!=0) i=-1;
		if(g_GetAck ) break;
		i++;
	} 
	
	return true;
}




GSM_Error FBUS2_WriteFrame(unsigned char 	*MsgBuffer,
				  int 			MsgLength,
				  unsigned char 	MsgType)
{
	unsigned char 		  buffer2[FBUS2_MAX_TRANSMIT_LENGTH + 10];  
	unsigned char 		  checksum=0;
	int 			  i, len, sent;

	CDeviceAPI	*Device 	= &theApp.m_DeviceAPI;

	buffer2[0] 	= FBUS2_FRAME_ID;
	if (Device->m_DeviceData.ConnectionType==GCT_FBUS2IRDA) buffer2[0] = FBUS2_IRDA_FRAME_ID;

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
	sent=Device->DeviceAPI_WriteDevice(buffer2,len);
	if (sent!=len) return ERR_DEVICEWRITEERROR;

	/* Sending to phone */
	return ERR_NONE;
}

GSM_Error FBUS2_WriteMessage (unsigned char 	*MsgBuffer,
				     int 		MsgLength,
				     unsigned char 	MsgType)
{
	int 			i, nom, togo, thislength; /* number of messages, ... */
	unsigned char 		buffer2[FBUS2_MAX_TRANSMIT_LENGTH + 2], seqnum;
	GSM_Protocol_FBUS2Data	*d		= &theApp.m_FBUS2Data;
	GSM_Error 		error;

//	GSM_DumpMessageLevel3(s, MsgBuffer, MsgLength, MsgType);

	nom  = (MsgLength + FBUS2_MAX_TRANSMIT_LENGTH - 1) / FBUS2_MAX_TRANSMIT_LENGTH;
	togo = MsgLength;

	for (i = 0; i < nom; i++) {
		seqnum 			= d->MsgSequenceNumber;
		if (i==0) seqnum 	= seqnum + 0x40;

		if(d->MsgSequenceNumber==0x32) seqnum = seqnum + 0x40;
		d->MsgSequenceNumber 	= (d->MsgSequenceNumber + 1) & 0x07;

		thislength = togo;
		if (togo > FBUS2_MAX_TRANSMIT_LENGTH) thislength = FBUS2_MAX_TRANSMIT_LENGTH;
		memcpy(buffer2, MsgBuffer + (MsgLength - togo), thislength);
		buffer2[thislength]	= nom - i;
		buffer2[thislength + 1]	= seqnum;
		togo 			= togo - thislength;

	//	GSM_DumpMessageLevel2(s, buffer2, thislength, MsgType);
g_GetAck=FALSE;
		error=FBUS2_WriteFrame(buffer2, thislength + 2, MsgType);
		if (error!=ERR_NONE) return error;
		if(nom > 1 && i!=nom-1)
		{
			for(int j = 0 ;j<2 ;j++)
			{
				FBUS2_ReadAck(TRUE);
				if(g_GetAck)
				{ 
					break;
				}
			}
			if(g_GetAck == false)
			{
				error=FBUS2_WriteFrame(buffer2, thislength + 2, MsgType);
				if (error!=ERR_NONE) return error;
				for(int j = 0 ;j<2 ;j++)
				{
					FBUS2_ReadAck(TRUE);
					if(g_GetAck)
						break;
				}
			}
		}
	}

	return ERR_NONE;
}

GSM_Error FBUS2_SendAck( unsigned char 		MsgType,
			       unsigned char 		MsgSequence)
{
	unsigned char buffer2[2];
	Debug_Info	*pDebugInfo= theApp.m_pDebugInfo;

	buffer2[0] = MsgType;
	buffer2[1] = MsgSequence;

	if(pDebugInfo)
	{
		if (pDebugInfo->dl==DL_TEXT || pDebugInfo->dl==DL_TEXTALL ||
			pDebugInfo->dl==DL_TEXTDATE || pDebugInfo->dl==DL_TEXTALLDATE) {
			smprintf(pDebugInfo,"[Sending Ack of type %02x, seq %x]\n",buffer2[0],buffer2[1]);
		}
	}

	/* Sending to phone */
	return FBUS2_WriteFrame(buffer2, 2, FBUS2_ACK_BYTE);
}

GSM_Error FBUS2_StateMachine(unsigned char rx_char)
{
	GSM_Protocol_FBUS2Data	*d		= &theApp.m_FBUS2Data;
	CDeviceAPI	*Device 	= &theApp.m_DeviceAPI;
	Debug_Info	*pDebugInfo= theApp.m_pDebugInfo;
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
		if (d->Msg.CheckSum[0] != d->Msg.CheckSum[1])
		{
			if(pDebugInfo)
			{
				if (pDebugInfo->dl==DL_TEXT || pDebugInfo->dl==DL_TEXTALL || pDebugInfo->dl==DL_TEXTERROR ||
					pDebugInfo->dl==DL_TEXTDATE || pDebugInfo->dl==DL_TEXTALLDATE || pDebugInfo->dl==DL_TEXTERRORDATE) {
					smprintf(pDebugInfo,"[ERROR: checksum]\n");	
				}
			}
			free(d->Msg.Buffer);
			d->Msg.Length 		= 0;
			d->Msg.Buffer 		= NULL;

			d->MsgRXState 		= RX_Sync;
			return ERR_NONE;
		}

		seq_num = d->Msg.Buffer[d->Msg.Length-1];

		if (d->Msg.Type == FBUS2_ACK_BYTE)
		{
			if(pDebugInfo)
			{
				if (pDebugInfo->dl==DL_TEXT || pDebugInfo->dl==DL_TEXTALL ||
					pDebugInfo->dl==DL_TEXTDATE || pDebugInfo->dl==DL_TEXTALLDATE)
				{
					smprintf(pDebugInfo, "[Received Ack of type %02x, seq %02x]\n",d->Msg.Buffer[0],seq_num);
				}
			}

			free(d->Msg.Buffer);
			d->Msg.Length 		= 0;
			d->Msg.Buffer 		= NULL;

		
			g_GetAck = TRUE;

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

		if ((seq_num & 0x40) != 0x40 && d->FramesToGo != frm_num)
		{
			if(pDebugInfo)
			{
				if (pDebugInfo->dl==DL_TEXT || pDebugInfo->dl==DL_TEXTALL || pDebugInfo->dl==DL_TEXTERROR ||
					pDebugInfo->dl==DL_TEXTDATE || pDebugInfo->dl==DL_TEXTALLDATE || pDebugInfo->dl==DL_TEXTERRORDATE) {
					smprintf(pDebugInfo, "[ERROR: Missed part of multiframe msg]\n");
				}
			}

			free(d->Msg.Buffer);
			d->Msg.Length 		= 0;
			d->Msg.Buffer 		= NULL;

			d->MsgRXState 		= RX_Sync;
			return ERR_NONE;
		}

		if ((seq_num & 0x40) != 0x40 && d->Msg.Type != d->MultiMsg.Type) 
		{
			if(pDebugInfo)
			{
				if (pDebugInfo->dl==DL_TEXT || pDebugInfo->dl==DL_TEXTALL || pDebugInfo->dl==DL_TEXTERROR ||
					pDebugInfo->dl==DL_TEXTDATE || pDebugInfo->dl==DL_TEXTALLDATE || pDebugInfo->dl==DL_TEXTERRORDATE) {
					smprintf(pDebugInfo, "[ERROR: Multiframe msg in multiframe msg]\n");
				}
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
			FBUS2_SendAck(d->Msg.Type,((unsigned char)(seq_num & 0x0f)));
		}
					
		if (d->FramesToGo == 0) {
	//		s->Phone.Data.RequestMsg	= &d->MultiMsg;
	//		s->Phone.Data.DispatchError	= s->Phone.Functions->DispatchMessage(s);
			if(theApp.m_bFinishRequest == false)
				theApp.m_DispatchError = theApp.DispatchMessage(&d->MultiMsg);
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
		if (rx_char != FBUS2_DEVICE_PHONE) 
		{
			if(pDebugInfo)
			{
				if (pDebugInfo->dl==DL_TEXT || pDebugInfo->dl==DL_TEXTALL || pDebugInfo->dl==DL_TEXTERROR ||
					pDebugInfo->dl==DL_TEXTDATE || pDebugInfo->dl==DL_TEXTALLDATE || pDebugInfo->dl==DL_TEXTERRORDATE) {
					smprintf(pDebugInfo,"[ERROR: incorrect char - %02x, not %02x]\n", rx_char, FBUS2_DEVICE_PHONE);
				}
			}
		}
		d->Msg.Source = rx_char;

		d->MsgRXState = RX_GetType;
		return ERR_NONE;
	}
	if (d->MsgRXState == RX_GetDestination) {
		if (rx_char != FBUS2_DEVICE_PC) 
		{
			if(pDebugInfo)
			{
				if (pDebugInfo->dl==DL_TEXT || pDebugInfo->dl==DL_TEXTALL || pDebugInfo->dl==DL_TEXTERROR ||
					pDebugInfo->dl==DL_TEXTDATE || pDebugInfo->dl==DL_TEXTALLDATE || pDebugInfo->dl==DL_TEXTERRORDATE) {
						smprintf(pDebugInfo,"[ERROR: incorrect char - %02x, not %02x]\n", rx_char, FBUS2_DEVICE_PC);
				}
			}

			free(d->Msg.Buffer);
			d->Msg.Length 		= 0;
			d->Msg.Buffer 		= NULL;

			d->MsgRXState = RX_Sync;
			return ERR_NONE;
		}
		d->Msg.Destination = rx_char;

		d->MsgRXState 	   = RX_GetSource;
		return ERR_NONE;
	}
	if (d->MsgRXState == RX_Sync) {
		switch (Device->m_DeviceData.ConnectionType) {
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
		if (!correct) 
		{
			if(pDebugInfo)
			{
				if (pDebugInfo->dl==DL_TEXT || pDebugInfo->dl==DL_TEXTALL || pDebugInfo->dl==DL_TEXTERROR ||
					pDebugInfo->dl==DL_TEXTDATE || pDebugInfo->dl==DL_TEXTALLDATE || pDebugInfo->dl==DL_TEXTERRORDATE) {
					if (Device->m_DeviceData.ConnectionType==GCT_FBUS2IRDA) {
						smprintf(pDebugInfo,"[ERROR: incorrect char - %02x, not %02x]\n", rx_char, FBUS2_IRDA_FRAME_ID);
			    		} else {
						smprintf(pDebugInfo,"[ERROR: incorrect char - %02x, not %02x]\n", rx_char, FBUS2_FRAME_ID);
			    		}
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

int FBUS2_WriteDLR3(char *command, int length, int timeout)
{

	GSM_Protocol_FBUS2Data	*d		= &theApp.m_FBUS2Data;
	CDeviceAPI	*Device 	= &theApp.m_DeviceAPI;
	unsigned char		buff[300];
	int			w = 0;
	bool			wassomething = false;

	Device->DeviceAPI_WriteDevice(command,length);

	for (w=0;w<timeout;w++) {   
		Sleep(10);		
		if (wassomething) {
			if (Device->DeviceAPI_ReadDevice( buff, 255)==0) return 1;
		} else {
			if (Device->DeviceAPI_ReadDevice( buff, 255)>0) wassomething = true;
		}
	}
	return 0;

}

GSM_Error FBUS2_Initialise()
{
	unsigned char		init_char	= 0x55;
	unsigned char		end_init_char	= 0xc1;

	GSM_Protocol_FBUS2Data	*d		= &theApp.m_FBUS2Data;
	CDeviceAPI	*Device 	= &theApp.m_DeviceAPI;
	GSM_Error		error;
	int			count;

	d->Msg.Length		= 0;
	d->Msg.Buffer		= NULL;
	d->MultiMsg.BufferUsed	= 0;
	d->MultiMsg.Length	= 0;
	d->MultiMsg.Buffer	= NULL;

	d->MsgSequenceNumber	= 0x20;
	d->FramesToGo		= 0;
	d->MsgRXState		= RX_Sync;

	error=Device->DeviceAPI_DeviceSetParity(false);
	if (error!=ERR_NONE) return error;

	switch (Device->m_DeviceData.ConnectionType)
	{
	case GCT_FBUS2BLUE:
	case GCT_BLUEFBUS2:
		FBUS2_WriteDLR3("AT\r",		 3,10);
		FBUS2_WriteDLR3("AT&F\r",		 5,10);
		FBUS2_WriteDLR3("AT*NOKIAFBUS\r",	13,10);
		break;
	case GCT_FBUS2DKU5:
	case GCT_FBUS2PL2303:
	case GCT_FBUS2DLR3:
        error=Device->DeviceAPI_DeviceSetDtrRts(false,false);
	    if (error!=ERR_NONE) return error; 
		error=Device->DeviceAPI_DeviceSetSpeed(115200);
		if (error!=ERR_NONE) return error;


		FBUS2_WriteDLR3("AT\r",		 3,50);
        FBUS2_WriteDLR3("AT&F\r\n",		 6,50);

		if(FBUS2_WriteDLR3("AT*NOKIAFBUS\r\n",	14,50) == 0)
		{
			error=Device->DeviceAPI_CloseDevice();
			if (error!=ERR_NONE) return error;
		    my_sleep(1000);
		   //Sleep( 100 );

			error=Device->DeviceAPI_OpenDevice();
			if (error!=ERR_NONE) return error;

			error=Device->DeviceAPI_DeviceSetParity(false);
	    	if (error!=ERR_NONE) return error;
			error=Device->DeviceAPI_DeviceSetSpeed(115200);
	    	if (error!=ERR_NONE) return error; 
	//		error=Device->DeviceSetDtrRts(s,true,false);
	        error=Device->DeviceAPI_DeviceSetDtrRts(false,false);
			if (error!=ERR_NONE) return error; 
        }

		unsigned char uinit[128];
		FillMemory(uinit,128,0x55);
		if (Device->DeviceAPI_WriteDevice(&uinit,128)!=128) 
			return ERR_DEVICEWRITEERROR;
		break;

	case GCT_FBUS2:
		error=Device->DeviceAPI_DeviceSetSpeed(115200);
		if (error!=ERR_NONE) return error; 

		error=Device->DeviceAPI_DeviceSetDtrRts(true,false); /*DTR high,RTS low*/
		if (error!=ERR_NONE) return error; 

		for (count = 0; count < 55; count ++) {
			if (Device->DeviceAPI_WriteDevice(&init_char,1)!=1) return ERR_DEVICEWRITEERROR;
			my_sleep(10);
		}
		break;
	case GCT_FBUS2IRDA:
		error=Device->DeviceAPI_DeviceSetSpeed(9600);
		if (error!=ERR_NONE) return error;

		for (count = 0; count < 55; count ++) {
			if (Device->DeviceAPI_WriteDevice(&init_char,1)!=1) return ERR_DEVICEWRITEERROR;
			my_sleep(10);
		}

		if (Device->DeviceAPI_WriteDevice(&end_init_char,1)!=1) return ERR_DEVICEWRITEERROR;
		my_sleep(20);

		error=Device->DeviceAPI_DeviceSetSpeed(115200);
		if (error!=ERR_NONE) return error;

		break;			
	default:
		break;
	}

	return ERR_NONE;
}

GSM_Error FBUS2_Terminate()
{
	GSM_Protocol_FBUS2Data	*d		= &theApp.m_FBUS2Data;
	if(d->Msg.Buffer !=NULL)
		free(d->Msg.Buffer);
	if(d->MultiMsg.Buffer !=NULL)
		free(d->MultiMsg.Buffer);

	return ERR_NONE;
}



/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

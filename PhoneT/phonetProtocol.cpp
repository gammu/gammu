#include "stdafx.h"

#include <stdio.h>
#include <string.h>
#include "DeviceAPI.h"
#include "PhoneT.h"


extern CPhoneTApp theApp;


#include "phonetProtocol.h"

GSM_Error PHONET_WriteMessage (unsigned char 	*MsgBuffer,
				      int 		MsgLength,
				      unsigned char 	MsgType)
{
	CDeviceAPI	*Device 	= &theApp.m_DeviceAPI;
	Debug_Info	*pDebugInfo= theApp.m_pDebugInfo;

	unsigned char		*buffer2; 
	int			sent;

//	GSM_DumpMessageLevel3(s, MsgBuffer, MsgLength, MsgType);

	buffer2 = (unsigned char *)malloc(MsgLength + 6);

	buffer2[0] = PHONET_FRAME_ID,
	buffer2[1] = PHONET_DEVICE_PHONE; 		//destination
	buffer2[2] = PHONET_DEVICE_PC;    		//source
	
	if (Device->m_DeviceData.ConnectionType==GCT_PHONETBLUE || Device->m_DeviceData.ConnectionType==GCT_BLUEPHONET) {
		buffer2[0] = PHONET_BLUE_FRAME_ID;
		buffer2[1] = PHONET_DEVICE_PHONE;	//destination
		buffer2[2] = PHONET_BLUE_DEVICE_PC;	//source
	}

	buffer2[3] = MsgType;
	buffer2[4] = MsgLength / 256;
	buffer2[5] = MsgLength % 256;

	memcpy(buffer2 + 6, MsgBuffer, MsgLength);

//	GSM_DumpMessageLevel2(s, buffer2+6, MsgLength, MsgType);

	/* Sending to phone */

	sent = 0;
	int i =0;
	int nOutBuffersize = MsgLength+6;
	while (sent != nOutBuffersize) 
	{
		if ((i = Device->DeviceAPI_WriteDevice(buffer2 + sent, nOutBuffersize - sent)) == 0)
			return ERR_DEVICEWRITEERROR;
		sent += i;
	}




//	sent = Device->DeviceAPI_WriteDevice(buffer2,MsgLength+6);

	free(buffer2);

	if (sent!=MsgLength+6) return ERR_DEVICEWRITEERROR;
	return ERR_NONE;
}

GSM_Error PHONET_StateMachine(unsigned char rx_char)
{
	GSM_Protocol_PHONETData 	*d = &theApp.m_PhonetData;
	CDeviceAPI	*Device 	= &theApp.m_DeviceAPI;
	Debug_Info	*pDebugInfo= theApp.m_pDebugInfo;

	bool				correct = false;

	if (d->MsgRXState==RX_GetMessage) {
		d->Msg.Buffer[d->Msg.Count] = rx_char;
		d->Msg.Count++;
		/* This is not last byte in frame */
		if (d->Msg.Count != d->Msg.Length) return ERR_NONE;

//		s->Phone.Data.RequestMsg	= &d->Msg;
//		s->Phone.Data.DispatchError	= s->Phone.Functions->DispatchMessage(s);
		if(theApp.m_bFinishRequest == false)
			theApp.m_DispatchError = theApp.DispatchMessage(&d->Msg);

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
		if (rx_char != PHONET_DEVICE_PHONE)
		{
			if(pDebugInfo)
			{
				if (pDebugInfo->dl==DL_TEXT || pDebugInfo->dl==DL_TEXTALL ||pDebugInfo->dl==DL_TEXTERROR ||
					pDebugInfo->dl==DL_TEXTDATE || pDebugInfo->dl==DL_TEXTALLDATE || pDebugInfo->dl==DL_TEXTERRORDATE) 
				{
					smprintf(pDebugInfo,"[ERROR: incorrect char - %02x, not %02x]\n", rx_char, PHONET_DEVICE_PHONE);
				}
			}
			d->MsgRXState = RX_Sync;
			return ERR_NONE;
		}
		d->Msg.Source = rx_char;

		d->MsgRXState = RX_GetType;
		return ERR_NONE;
	}
	if (d->MsgRXState==RX_GetDestination) {
		switch (Device->m_DeviceData.ConnectionType) {
		case GCT_IRDAPHONET:
			if (rx_char == PHONET_DEVICE_PC) correct = true;
			break;
		case GCT_PHONETBLUE:
		case GCT_BLUEPHONET:
			if (rx_char == PHONET_BLUE_DEVICE_PC) correct = true;
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
					smprintf(pDebugInfo,"[ERROR: incorrect char - %02x, not %02x]\n", rx_char, PHONET_DEVICE_PC);
				}
			}
			d->MsgRXState = RX_Sync;
			return ERR_NONE;
		}
		d->Msg.Destination 	= rx_char;

		d->MsgRXState 		= RX_GetSource;
		return ERR_NONE;
	}
	if (d->MsgRXState==RX_Sync) {
		switch (Device->m_DeviceData.ConnectionType) {
		case GCT_IRDAPHONET:
			if (rx_char == PHONET_FRAME_ID) correct = true;
			break;
		case GCT_PHONETBLUE:
		case GCT_BLUEPHONET:
			if (rx_char == PHONET_BLUE_FRAME_ID) correct = true;
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
					smprintf(pDebugInfo,"[ERROR: incorrect char - %02x, not %02x]\n", rx_char, PHONET_FRAME_ID);
				}
			}
			return ERR_NONE;
		}
		d->Msg.Count  = 0;

		d->MsgRXState = RX_GetDestination;	
		return ERR_NONE;
	}
	return ERR_NONE;
}

GSM_Error PHONET_Initialise()
{
	int 				total = 0, i, n;
	GSM_Protocol_PHONETData 	*d = &theApp.m_PhonetData;
	CDeviceAPI	*Device 	= &theApp.m_DeviceAPI;
	Debug_Info	*pDebugInfo= theApp.m_pDebugInfo;
	unsigned char			req[50];
	clock_t _waitTime;

	d->Msg.Length	= 0;
	d->Msg.Buffer	= NULL;
	d->MsgRXState	= RX_Sync;

	if (Device->m_DeviceData.ConnectionType == GCT_PHONETBLUE || Device->m_DeviceData.ConnectionType == GCT_BLUEPHONET)
	{
		/* Send frame in PHONET style */
		req[0] = PHONET_BLUE_FRAME_ID;  req[1] = PHONET_DEVICE_PHONE;
		req[2] = PHONET_BLUE_DEVICE_PC; req[3] = 0xD0;
		req[4] = 0x00;			req[5] = 0x01;
		req[6] = 0x04;
		if (Device->DeviceAPI_WriteDevice(req,7) != 7) return ERR_DEVICEWRITEERROR;

	/*	while (total < 7) {
			n = s->Device.Functions->ReadDevice(s, req + total, 50 - total);
			total += n;
		}*/
		_waitTime = clock () + 5 * CLK_TCK ;

		while (total < 7 && clock() < _waitTime)
		{
			n = Device->DeviceAPI_ReadDevice( req + total, 50 - total);
			total += n;
		}
		if(total < 7) return ERR_DEVICEREADERROR;

		/* Answer frame in PHONET style */
		req[10] = PHONET_BLUE_FRAME_ID; req[11] = PHONET_BLUE_DEVICE_PC;
		req[12] = PHONET_DEVICE_PHONE;	req[13] = 0xD0;
		req[14] = 0x00;			req[15] = 0x01;
		req[16] = 0x05;

		for (i = 0; i < 7; i++) 
		{
			if (req[i] != req[10+i]) 
			{
				smprintf(pDebugInfo,"Incorrect byte in the answer\n");
				return ERR_UNKNOWN;
			}
		}
	}

	return ERR_NONE;
}

GSM_Error PHONET_Terminate()
{
	if(theApp.m_PhonetData.Msg.Buffer)
		free(theApp.m_PhonetData.Msg.Buffer);
	return ERR_NONE;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

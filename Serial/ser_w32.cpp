/* (c) 2002-2004 by Marcin Wiacek */
/* based on some work from MSDN and others */
/* based on some work from Gnokii (www.gnokii.org)
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
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <io.h>
#include <memory.h>

#include "ser_w32.h"
#include "commfun.h"

DWORD MAXData = 255;

GSM_Error serial_close(GSM_DeviceData *s,Debug_Info *debugInfo)
{
	COMSTAT			ComStat;
	DWORD			ErrorFlags;


	ClearCommError(s->hPhone, &ErrorFlags, &ComStat);

	// Disables all monitored events for device 
	SetCommMask(s->hPhone, 0);
	smprintf(debugInfo,"[SetCommMask]\n");

	my_sleep(10);


	/* Discards all characters from input/output buffer and terminates
	 * pending read/write operations
	 */


	PurgeComm(s->hPhone, PURGE_TXABORT | PURGE_RXABORT |
			     PURGE_TXCLEAR | PURGE_RXCLEAR);

	smprintf(debugInfo,"[PurgeComm]\n");



	my_sleep(10);
// Restores old settings 
	if (SetCommState(s->hPhone, &s->old_settings)==0) {
		GSM_OSErrorInfo(debugInfo, "SetCommState in serial_close");
	}
	smprintf(debugInfo,"[SetCommState]\n");


	my_sleep(10);
	// Clears the DTR (data-terminal-ready) signal 
	EscapeCommFunction(s->hPhone, CLRDTR);
//	my_sleep(10);
//	EscapeCommFunction(d->hPhone, CLRRTS);

	my_sleep(10);

	/* Closes device */
	if (CloseHandle(s->hPhone)==0) {
		GSM_OSErrorInfo(debugInfo, "CloseHandle in serial_close");
		smprintf(debugInfo,"CloseHandle in serial_close\n");
	}
	smprintf(debugInfo,"[CloseHandle]\n");

	// Compare ; by mingfa
	if(s->osRead.hEvent	) CloseHandle(s->osRead.hEvent); 
	return ERR_NONE;
}

GSM_Error serial_open (GSM_DeviceData *s,Debug_Info *debugInfo)
{
	DCB 			dcb;
	char 		DeviceName[80],DeviceName2[80];
	int			i;
#ifdef GSM_ENABLE_FBUS2DKU5
	HKEY 			hKey;
	DWORD 			DeviceNameLen, KeyNameLen;
	unsigned char		KeyName[100];
#endif
	COMMTIMEOUTS TimeOut;
	TimeOut.ReadIntervalTimeout = MAXDWORD;//0xFFFFFFFF;
	TimeOut.ReadTotalTimeoutMultiplier = 0;
	TimeOut.ReadTotalTimeoutConstant = 1000; 
	TimeOut.WriteTotalTimeoutMultiplier = 0;
	TimeOut.WriteTotalTimeoutConstant = 1000;


	strcpy(DeviceName2,s->CurrentConfig.DeviceCOMport);

#ifdef GSM_ENABLE_FBUS2DKU5
	if (s->ConnectionType == GCT_FBUS2DKU5) {
		smprintf(debugInfo,"Reading DKU5 device\n");
		DeviceName2[0] = 0;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) {
			smprintf(debugInfo,"Error opening key\n");
			return ERR_DEVICENOTWORK;
		}
		i = 0;
		while(1) {
			DeviceNameLen = 80;
			KeyNameLen = 100;
			if (RegEnumValue(hKey,i,KeyName,&KeyNameLen,NULL,NULL,DeviceName2,&DeviceNameLen) != ERROR_SUCCESS) {
				smprintf(debugInfo,"Error reading key value\n");
				return ERR_DEVICENOTWORK;
			}
//			smprintf(s,"Key name is %s, value is %s\n",KeyName,DeviceName2);
			if (!strncmp(KeyName,"\\Device\\AtmelVirtualPort",24)) break;
			i++;
		}
		RegCloseKey(hKey);
		if (strlen(DeviceName2) == 0) return ERR_DEVICENOTWORK;
		smprintf(debugInfo,"DKU5 device is \"%s\"\n",DeviceName2);
		//nodriver
	}
#endif

	if ((s->ConnectionType == GCT_FBUS2DKU5) ||	 
	    (!strncmp(DeviceName2,"com",3) && strlen(DeviceName2)>3)) {
		sprintf(DeviceName,"\\\\.\\COM%i",atoi(DeviceName2+3));
	} 
	//else if (s->ConnectionType == GCT_FBUS2DLR3)
	else 
	{
		//strcpy(DeviceName,DeviceName2);
		sprintf(DeviceName,"\\\\.\\%s",DeviceName2);
	}

	smprintf(debugInfo,"Device is %s\n",DeviceName);

	/* Allows for reading/writing, no device sharing */
	s->hPhone = CreateFile(DeviceName,
			       GENERIC_READ | GENERIC_WRITE,
			       0,
			       0,
			       OPEN_EXISTING,
			       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
			       NULL);

	if (s->hPhone == INVALID_HANDLE_VALUE) {
		i = GetLastError();
		GSM_OSErrorInfo(debugInfo, "CreateFile in serial_open");
		if (i == 2)   return ERR_DEVICENOTWORK; //can't find specified file
		if (i == 5)   return ERR_DEVICEBUSY;    //access denied
		if (i == 31)  return ERR_DEVICENOTWORK; //attached device not working
		if (i == 123) return ERR_DEVICENOTEXIST;
		return ERR_DEVICEOPENERROR;
	}

	s->old_settings.DCBlength = sizeof(DCB);
	if (GetCommState(s->hPhone, &s->old_settings)==0) {
		GSM_OSErrorInfo(debugInfo, "ReadDevice in serial_open");
		return ERR_DEVICEREADERROR;
	}

//	my_sleep(10);
//	EscapeCommFunction(d->hPhone, SETDTR );
//	my_sleep(10);
//	EscapeCommFunction(d->hPhone, SETRTS );

	/* When char will be received, we will receive notifications */
	//SetCommMask(d->hPhone, EV_RXCHAR|EV_ERR);
	SetCommMask(s->hPhone, EV_RXCHAR |EV_BREAK |EV_ERR);

	/* Sets size for input/output buffer */
	SetupComm(s->hPhone, 4096, 4096);
	//SetupComm(d->hPhone, 8192, 4096);

	/* Discards all characters from input/output buffer and terminates
	 * pending read/write operations
	 */
	PurgeComm(s->hPhone, PURGE_TXABORT | PURGE_RXABORT |
			     PURGE_TXCLEAR | PURGE_RXCLEAR);

 	memcpy(&dcb, &s->old_settings, sizeof(DCB));

	dcb.ByteSize 	 = 8;
	dcb.Parity 	 = NOPARITY; 
	dcb.StopBits 	 = ONESTOPBIT;

	/* No Xon/Xof flow control */
//	dcb.fOutX 	 = false;		
//	dcb.fInX 	 = false;
	
	/* Hardware flow control */
//	dcb.fOutxDsrFlow = true;
//	dcb.fOutxCtsFlow = true;
//	dcb.fDtrControl  = DTR_CONTROL_HANDSHAKE;
//	dcb.fRtsControl  = RTS_CONTROL_HANDSHAKE;

    	/* Initialise the port settings */
	if (SetCommState(s->hPhone, &dcb)==0) {
		GSM_OSErrorInfo(debugInfo, "WriteDevice in serial_open");
		return ERR_DEVICEOPENERROR;
	}       	

	if (SetCommTimeouts(s->hPhone,&TimeOut))
		dbgprintf("SetCommTimeouts success");
	else
		dbgprintf("SetCommTimeouts failed"); 

	s->osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	return ERR_NONE;
}

GSM_Error serial_setparity (GSM_DeviceData *s, bool parity,Debug_Info *debugInfo)
{
	DCB 			dcb;

	dcb.DCBlength = sizeof(DCB);
	if (GetCommState(s->hPhone, &dcb)==0) {
		GSM_OSErrorInfo(debugInfo, "ReadDevice in serial_setparity");
		return ERR_DEVICEREADERROR;
	}

	if (parity) {
		dcb.Parity = ODDPARITY;
	} else {
		dcb.Parity = NOPARITY; 
	}

	if (SetCommState(s->hPhone, &dcb)==0) {
		GSM_OSErrorInfo(debugInfo, "WriteDevice in serial_setparity");
		return ERR_DEVICEPARITYERROR;
	}

	return ERR_NONE;
}

GSM_Error serial_setdtrrts(GSM_DeviceData *s, bool dtr, bool rts,Debug_Info *debugInfo)
{
	DCB        		dcb;

	dcb.DCBlength = sizeof(DCB);
	if (GetCommState(s->hPhone, &dcb)==0) {
		GSM_OSErrorInfo(debugInfo, "ReadDevice in serial_setdtrrts");
		return ERR_DEVICEREADERROR;
	}

	dcb.fOutxDsrFlow = 0;
	dcb.fDtrControl  = DTR_CONTROL_DISABLE;
	if (dtr) dcb.fDtrControl = DTR_CONTROL_ENABLE;

	dcb.fOutxCtsFlow = 0;
	dcb.fRtsControl  = RTS_CONTROL_DISABLE;
	if (rts) dcb.fRtsControl = RTS_CONTROL_ENABLE;

	/* no software (Xon/Xof) flow control */
	dcb.fInX = dcb.fOutX = 0;

	if (SetCommState(s->hPhone, &dcb)==0) {
		GSM_OSErrorInfo(debugInfo, "WriteDevice in serial_setdtrrts");
		return ERR_DEVICEDTRRTSERROR;
	}

	/* the rest of function checks, if setting was really done */

	dcb.DCBlength = sizeof(DCB);
	GetCommState(s->hPhone, &dcb);

	dbgprintf("Serial device:");
	dbgprintf(" DTR is ");
	switch (dcb.fDtrControl) {
		case DTR_CONTROL_ENABLE    : dbgprintf("up");        break;
		case DTR_CONTROL_DISABLE   : dbgprintf("down");      break;
		case DTR_CONTROL_HANDSHAKE : dbgprintf("handshake"); break;
	}
	dbgprintf(", RTS is ");
	switch (dcb.fRtsControl) {
		case RTS_CONTROL_ENABLE    : dbgprintf("up");        break;
		case RTS_CONTROL_DISABLE   : dbgprintf("down");      break;
		case RTS_CONTROL_HANDSHAKE : dbgprintf("handshake"); break;
		case RTS_CONTROL_TOGGLE    : dbgprintf("toggle");    break;
	}
	dbgprintf("\n");
	if ( dtr && dcb.fDtrControl != DTR_CONTROL_ENABLE ) return ERR_DEVICEDTRRTSERROR;
	if (!dtr && dcb.fDtrControl != DTR_CONTROL_DISABLE) return ERR_DEVICEDTRRTSERROR;
	if ( rts && dcb.fRtsControl != RTS_CONTROL_ENABLE ) return ERR_DEVICEDTRRTSERROR;
	if (!rts && dcb.fRtsControl != RTS_CONTROL_DISABLE) return ERR_DEVICEDTRRTSERROR;

	return ERR_NONE;
}

GSM_Error serial_setspeed(GSM_DeviceData *s, int speed,Debug_Info *debugInfo)
{
	DCB			dcb;

	dcb.DCBlength = sizeof(DCB);
	if (GetCommState(s->hPhone, &dcb)==0) {
		GSM_OSErrorInfo(debugInfo, "ReadDevice in serial_setspeed");
		return ERR_DEVICEREADERROR;
	}

	dcb.BaudRate = speed;

	if (SetCommState(s->hPhone, &dcb)==0) {
		GSM_OSErrorInfo(debugInfo, "WriteDevice in serial_setspeed");
		return ERR_DEVICECHANGESPEEDERROR;
	}
	PurgeComm(s->hPhone, PURGE_TXCLEAR | PURGE_RXCLEAR);

	return ERR_NONE;	
}

int serial_read(GSM_DeviceData *s, void *buf, size_t nbytes,Debug_Info *debugInfo)
{
	COMSTAT			ComStat;
	DWORD			ErrorFlags, Length;
//	DWORD			dwEvent;


	/* Gets information about a communications error and
	 * current status of device
	 */
	if(s->hPhone == INVALID_HANDLE_VALUE)
	   return 0;

//	SetCommMask(d->hPhone, EV_RXCHAR |EV_BREAK |EV_ERR);

//	GetCommMask(d->hPhone,&dwEvent);
   //if(!WaitCommEvent(d->hPhone,&dwEvent,0))
	   //return 0;
 
	ClearCommError(s->hPhone, &ErrorFlags, &ComStat);

	// Compare by mingfa
    Length =   min(MAXData ,ComStat.cbInQue);
   //  Length = ComStat.cbInQue;
	 
	// Nothing to read  
	if (Length <= 0) 
	{
		//Length=nbytes;
		return Length;
	}
	//Length=nbytes;
	/* Read without problems */
	if (ReadFile(s->hPhone, buf, Length, &Length, &s->osRead)) return Length;

	if (GetLastError() != ERROR_IO_PENDING) {
		GSM_OSErrorInfo(debugInfo, "serial_read1");
		Length = 0;
		ClearCommError(s->hPhone, &ErrorFlags, &ComStat);
		return Length;
	}
			
	while(1) {
		if (GetOverlappedResult(s->hPhone,&s->osRead, &Length, TRUE)) break;
		if (GetLastError() != ERROR_IO_INCOMPLETE) {
			GSM_OSErrorInfo(debugInfo, "serial_read2");
			/* an error occurred, try to recover */
			ClearCommError(s->hPhone, &ErrorFlags, &ComStat);
			break;
		}
	}
	return Length;
}

int serial_write(GSM_DeviceData *s, void *buf, size_t nbytes,Debug_Info *debugInfo)
{
	DWORD			BytesWritten,ErrorFlags,BytesSent=0;
	COMSTAT			ComStat;
//	GSM_Device_SerialData 	*d = &s->Device.Data.Serial;
//	PurgeComm(d->hPhone, PURGE_TXCLEAR );
	//EscapeCommFunction(d->hPhone, CLRDTR);
//	EscapeCommFunction( d->hPhone, RESETDEV | CLRBREAK ) ;


	if (WriteFile(s->hPhone, buf, nbytes, &BytesSent, &s->osWrite)) return BytesSent;

	if (GetLastError() != ERROR_IO_PENDING) {
	    	GSM_OSErrorInfo(debugInfo, "serial_write1");
	    	ClearCommError(s->hPhone, &ErrorFlags, &ComStat);
	    	return BytesSent;
	}

	while (1) {
		if (GetOverlappedResult(s->hPhone, &s->osWrite, &BytesWritten, TRUE)) break;
		if (GetLastError() != ERROR_IO_INCOMPLETE) {
			GSM_OSErrorInfo(debugInfo, "serial_write2");
			ClearCommError(s->hPhone, &ErrorFlags, &ComStat);
			break;
		}
		BytesSent += BytesWritten;
	}
	BytesSent += BytesWritten;

	return BytesSent;
}


/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

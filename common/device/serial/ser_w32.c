
#include "../../gsmstate.h"

#ifdef GSM_ENABLE_SERIALDEVICE
#ifdef WIN32

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <io.h>
#include <memory.h>

#include "../../gsmcomon.h"
#include "ser_w32.h"

/* Close the serial port and restore old settings. */
static GSM_Error serial_close(GSM_StateMachine *s)
{
	GSM_Device_SerialData *d = &s->Device.Data.Serial;

	/* disable event notification and wait for thread
	   to halt */
	SetCommMask(d->hPhone, 0);

	/* purge any outstanding reads/writes and close device handle */
	PurgeComm(d->hPhone, PURGE_TXABORT | PURGE_RXABORT |
			     PURGE_TXCLEAR | PURGE_RXCLEAR);

	/* drop DTR */
	EscapeCommFunction(d->hPhone, CLRDTR);

	if (SetCommState(d->hPhone, &d->backup_dcb)==0) {
		GSM_OSErrorInfo(s, "SetCommState in serial_close");
	}

	if (CloseHandle(d->hPhone)==0) {
		GSM_OSErrorInfo(s, "CloseHandle in serial_close");
	}

	return GE_NONE;
}

static GSM_Error serial_open (GSM_StateMachine *s)
{
	GSM_Device_SerialData 	*d = &s->Device.Data.Serial;
	DCB 			dcb;
	unsigned char 		DeviceName[80];
	int			i;
#ifdef GSM_ENABLE_FBUS2DKU5
	HKEY 			hKey;
	DWORD 			DeviceNameLen;
#endif

	strcpy(DeviceName,s->CurrentConfig->Device);

#ifdef GSM_ENABLE_FBUS2DKU5
	if (s->ConnectionType == GCT_FBUS2DKU5) {
		DeviceName[0] = 0;
		RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_QUERY_VALUE, &hKey);
		RegQueryValueEx(hKey, "\\Device\\AtmelVirtualPort000", NULL, NULL, (LPBYTE) DeviceName, &DeviceNameLen);
		RegCloseKey(hKey);
		if (strlen(DeviceName) == 0) return GE_DEVICENOTWORK;
		smprintf(s,"DKU5 device is \"%s\"\n",DeviceName);
		//nodriver
	}
#endif

	d->hPhone = CreateFile(	DeviceName,
				GENERIC_READ | GENERIC_WRITE,
				0,                    /* exclusive access */
				NULL,                 /* no security attrs */
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL |
				FILE_FLAG_OVERLAPPED, /* overlapped I/O */
			NULL);

	if (d->hPhone == INVALID_HANDLE_VALUE) {
		i = GetLastError();
		GSM_OSErrorInfo(s, "CreateFile in serial_open");
		if (i == 2)   return GE_DEVICENOTWORK; //can't find specified file
		if (i == 5)   return GE_DEVICEBUSY;    //access denied
		if (i == 31)  return GE_DEVICENOTWORK; //attached device not working
		if (i == 123) return GE_DEVICENOTEXIST;
		return GE_DEVICEOPENERROR;
	}

	d->backup_dcb.DCBlength = sizeof(DCB);
	if (GetCommState(d->hPhone, &d->backup_dcb)==0) {
		GSM_OSErrorInfo(s, "ReadDevice in serial_open");
		return GE_DEVICEREADERROR;
	}

	/* get any early notifications */
	SetCommMask(d->hPhone, EV_RXCHAR);

	/* setup device buffers */
	SetupComm(d->hPhone, 4096, 4096);

	/* purge any information in the buffer */
	PurgeComm(d->hPhone, PURGE_TXABORT | PURGE_RXABORT |
			     PURGE_TXCLEAR | PURGE_RXCLEAR);

    	/* Initialise the port settings */
 	memcpy(&dcb, &d->backup_dcb, sizeof(DCB));

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

	if (SetCommState(d->hPhone, &dcb)==0) {
		GSM_OSErrorInfo(s, "WriteDevice in serial_open");
		return GE_DEVICEOPENERROR;
	}       	

	return GE_NONE;
}

static GSM_Error serial_setparity (GSM_StateMachine *s, bool parity)
{
	DCB 			dcb;
	GSM_Device_SerialData 	*d = &s->Device.Data.Serial;

	dcb.DCBlength = sizeof(DCB);
	if (GetCommState(d->hPhone, &dcb)==0) {
		GSM_OSErrorInfo(s, "ReadDevice in serial_setparity");
		return GE_DEVICEREADERROR;
	}

	if (parity) dcb.Parity = ODDPARITY;
	       else dcb.Parity = NOPARITY; 

	if (SetCommState(d->hPhone, &dcb)==0) {
		GSM_OSErrorInfo(s, "WriteDevice in serial_setparity");
		return GE_DEVICEPARITYERROR;
	}

	return GE_NONE;
}

/* Set the DTR and RTS bit of the serial device. */
static GSM_Error serial_setdtrrts(GSM_StateMachine *s, bool dtr, bool rts)
{
	DCB        		dcb;
	GSM_Device_SerialData 	*d = &s->Device.Data.Serial;

	dcb.DCBlength = sizeof(DCB);
	if (GetCommState(d->hPhone, &dcb)==0) {
		GSM_OSErrorInfo(s, "ReadDevice in serial_setdtrrts");
		return GE_DEVICEREADERROR;
	}

	dcb.fOutxDsrFlow = 0;
	if (dtr) dcb.fDtrControl = DTR_CONTROL_ENABLE;
	    else dcb.fDtrControl = DTR_CONTROL_DISABLE;

	dcb.fOutxCtsFlow = 0;
	if (rts) dcb.fRtsControl = RTS_CONTROL_ENABLE;
	    else dcb.fRtsControl = RTS_CONTROL_DISABLE;

	/* no software (Xon/Xof) flow control */
	dcb.fInX = dcb.fOutX = 0;

	if (SetCommState(d->hPhone, &dcb)==0) {
		GSM_OSErrorInfo(s, "WriteDevice in serial_setdtrrts");
		return GE_DEVICEDTRRTSERROR;
	}

	dcb.DCBlength = sizeof(DCB);
	GetCommState(d->hPhone, &dcb);

	dprintf("Serial device:");
	dprintf(" DTR is ");
	switch (dcb.fDtrControl) {
		case DTR_CONTROL_ENABLE    : dprintf("up");        break;
		case DTR_CONTROL_DISABLE   : dprintf("down");      break;
		case DTR_CONTROL_HANDSHAKE : dprintf("handshake"); break;
	}
	dprintf(", RTS is ");
	switch (dcb.fRtsControl) {
		case RTS_CONTROL_ENABLE    : dprintf("up");        break;
		case RTS_CONTROL_DISABLE   : dprintf("down");      break;
		case RTS_CONTROL_HANDSHAKE : dprintf("handshake"); break;
		case RTS_CONTROL_TOGGLE    : dprintf("toggle");    break;
	}
	dprintf("\n");
	if ( dtr && dcb.fDtrControl != DTR_CONTROL_ENABLE ) return GE_DEVICEDTRRTSERROR;
	if (!dtr && dcb.fDtrControl != DTR_CONTROL_DISABLE) return GE_DEVICEDTRRTSERROR;
	if ( rts && dcb.fRtsControl != RTS_CONTROL_ENABLE ) return GE_DEVICEDTRRTSERROR;
	if (!rts && dcb.fRtsControl != RTS_CONTROL_DISABLE) return GE_DEVICEDTRRTSERROR;

	return GE_NONE;
}

/* Change the speed of the serial device.
 * RETURNS: Success
 */
static GSM_Error serial_setspeed(GSM_StateMachine *s, int speed)
{
	DCB			dcb;
	GSM_Device_SerialData 	*d = &s->Device.Data.Serial;

	dcb.DCBlength = sizeof(DCB);
	if (GetCommState(d->hPhone, &dcb)==0) {
		GSM_OSErrorInfo(s, "ReadDevice in serial_setspeed");
		return GE_DEVICEREADERROR;
	}

	dcb.BaudRate = speed;

	if (SetCommState(d->hPhone, &dcb)==0) {
		GSM_OSErrorInfo(s, "WriteDevice in serial_setspeed");
		return GE_DEVICECHANGESPEEDERROR;
	}

	return GE_NONE;	
}

/* Read from serial device. */
static int serial_read(GSM_StateMachine *s, void *buf, size_t nbytes)
{
	BOOL			fReadStat;
	COMSTAT			ComStat;
	DWORD			dwErrorFlags;
	DWORD			dwLength=0;
	DWORD			dwError;
	GSM_Device_SerialData 	*d = &s->Device.Data.Serial;

	/* only try to read number of bytes in queue */
	ClearCommError(d->hPhone, &dwErrorFlags, &ComStat);
	dwLength = min((DWORD) nbytes, ComStat.cbInQue);
	if (dwLength > 0) {
		fReadStat = ReadFile(d->hPhone, buf, dwLength, &dwLength, &d->osRead);
		if (!fReadStat) {
			if (GetLastError() == ERROR_IO_PENDING) {
//				dprintf("IO Pending\n");
				/* We have to wait for read to complete.
				 * This function will timeout according to the
				 * CommTimeOuts.ReadTotalTimeoutConstant variable
				 * Every time it times out, check for port errors
				 */
				while(!GetOverlappedResult(d->hPhone,
						   &d->osRead, &dwLength, TRUE)) {
					dwError = GetLastError();
					if (dwError == ERROR_IO_INCOMPLETE)
						/* normal result if not finished */
						continue;
					else {
						GSM_OSErrorInfo(s, "serial_read");
						/* an error occurred, try to recover */
						ClearCommError(d->hPhone, &dwErrorFlags, &ComStat);
						break;
					}
				}
			} else {
				GSM_OSErrorInfo(s, "serial_read");
				/* some other error occurred */
				dwLength = 0;
				ClearCommError(d->hPhone, &dwErrorFlags, &ComStat);
			}
		}
	}

	return (dwLength);
}

/* Write to serial device. */
static int serial_write(GSM_StateMachine *s, void *buf, size_t nbytes)
{
	BOOL			fWriteStat;
	DWORD			dwBytesWritten;
	DWORD			dwErrorFlags;
	DWORD   		dwError;
	DWORD			dwBytesSent = 0;
	COMSTAT			ComStat;
	GSM_Device_SerialData 	*d = &s->Device.Data.Serial;

	fWriteStat = WriteFile(d->hPhone, buf, nbytes, &dwBytesSent, &d->osWrite);

	/* Note that normally the code will not execute the following
	 * because the driver caches write operations. Small I/O requests
	 * (up to several thousand bytes) will normally be accepted
	 * immediately and WriteFile will return true even though an
	 * overlapped operation was specified
	 */
	if (!fWriteStat) {
		if(GetLastError() == ERROR_IO_PENDING) {
		    /* We should wait for the completion of the write operation
		     * so we know if it worked or not
		     *
		     * This is only one way to do this. It might be beneficial to
		     * place the write operation in a separate thread
		     * so that blocking on completion will not negatively
		     * affect the responsiveness of the UI
		     *          	
		     * If the write takes too long to complete, this
		     * function will timeout according to the
		     * CommTimeOuts.WriteTotalTimeoutMultiplier variable.
		     * This code logs the timeout but does not retry
		     * the write.
		     */
			while(!GetOverlappedResult(d->hPhone, &d->osWrite, &dwBytesWritten, TRUE)) {
				dwError = GetLastError();
				if(dwError == ERROR_IO_INCOMPLETE)
				{
					/* normal result if not finished */
					dwBytesSent += dwBytesWritten;
					continue;
				} else {
					GSM_OSErrorInfo(s, "serial_write");
			    		/* an error occurred, try to recover */
					ClearCommError(d->hPhone, &dwErrorFlags, &ComStat);
					break;
				}
			}
			dwBytesSent += dwBytesWritten;
		} else {
		    GSM_OSErrorInfo(s, "serial_write");
		    /* some other error occurred */
		    ClearCommError(d->hPhone, &dwErrorFlags, &ComStat);
		    return dwBytesSent;
		}
	}

	return dwBytesSent;
}

GSM_Device_Functions SerialDevice = {
	serial_open,
	serial_close,
	serial_setparity,
	serial_setdtrrts,
	serial_setspeed,
	serial_read,
	serial_write
};

#endif
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

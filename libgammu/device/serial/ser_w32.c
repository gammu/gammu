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

/* To define GUID and not only declare */
#define INITGUID

#include "../../gsmstate.h"

#ifdef GSM_ENABLE_SERIALDEVICE
#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <memory.h>

#if defined(GSM_ENABLE_DKU2PHONET) || defined(GSM_ENABLE_DKU2AT)
#  include <initguid.h>
#  include <setupapi.h>
#  ifndef __GNUC__
#    pragma comment(lib, "setupapi.lib")
#    pragma comment(lib, "advapi32.lib")
#  endif
#endif

#include "../../gsmcomon.h"
#include "ser_w32.h"

static GSM_Error serial_close(GSM_StateMachine *s)
{
	GSM_Device_SerialData *d = &s->Device.Data.Serial;

	if (s->ConnectionType != GCT_DKU2PHONET) {
		/* Disables all monitored events for device */
		SetCommMask(d->hPhone, 0);

		/* Discards all characters from input/output buffer and terminates
		 * pending read/write operations
		 */
		PurgeComm(d->hPhone, PURGE_TXABORT | PURGE_RXABORT |
				     PURGE_TXCLEAR | PURGE_RXCLEAR);

		/* Clears the DTR (data-terminal-ready) signal */
		EscapeCommFunction(d->hPhone, CLRDTR);

		/* Restores old settings */
		if (SetCommState(d->hPhone, &d->old_settings)==0) {
			GSM_OSErrorInfo(s, "SetCommState in serial_close");
		}
	}

	/* Closes device */
	if (CloseHandle(d->hPhone)==0) {
		GSM_OSErrorInfo(s, "CloseHandle in serial_close");
	}

	return ERR_NONE;
}

#if defined(GSM_ENABLE_DKU2PHONET) || defined(GSM_ENABLE_DKU2AT)
  DEFINE_GUID(DKU2AT,    0x4F919104, 0x4adf, 0x11d5, 0x88, 0x2d, 0x0, 0xb0, 0xd0, 0x2f, 0xe3, 0x81); /* "4F919104-4ADF-11D5-882D-00B0D02FE381" */
  DEFINE_GUID(DKU2FBUS2, 0x4F919102, 0x4adf, 0x11d5, 0x88, 0x2d, 0x0, 0xb0, 0xd0, 0x2f, 0xe3, 0x81); /* "4F919102-4ADF-11D5-882D-00B0D02FE381" */
  DEFINE_GUID(DKU2OBEX,  0x4F919100, 0x4adf, 0x11d5, 0x88, 0x2d, 0x0, 0xb0, 0xd0, 0x2f, 0xe3, 0x81); /* "4F919100-4ADF-11D5-882D-00B0D02FE381" */
#endif

static GSM_Error serial_open (GSM_StateMachine *s)
{
	GSM_Device_SerialData 		 *d = &s->Device.Data.Serial;
	DCB 				 dcb;
	unsigned char 			 DeviceName[256],DeviceName2[256];
	int				 i;
	DWORD				 err;
#if defined(GSM_ENABLE_DKU2PHONET) || defined(GSM_ENABLE_DKU2AT)
	UCHAR				 bu[sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) +(sizeof(TCHAR)*1000)];
	DWORD				 reqsize;
	SP_DEVINFO_DATA		 	 DeviceInfoData;
	SP_DEVICE_INTERFACE_DATA         DeviceInterfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)bu;
	HDEVINFO			 ListHandle;
	unsigned char			 KeyName2[256];
#endif
#ifdef GSM_ENABLE_DKU5FBUS2
	HKEY 				 hKey;
	DWORD 				 DeviceNameLen, KeyNameLen;
	unsigned char			 KeyName[256];
#endif

	strcpy(DeviceName2,s->CurrentConfig->Device);

#ifdef GSM_ENABLE_DKU2PHONET
	if (s->ConnectionType == GCT_DKU2PHONET) {
	        ListHandle = SetupDiGetClassDevs(&DKU2FBUS2, NULL, NULL,  DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
		if (ListHandle == INVALID_HANDLE_VALUE) {
		        SetupDiDestroyDeviceInfoList(ListHandle);
			return ERR_UNKNOWN;
		}
		DeviceInfoData.cbSize = sizeof(DeviceInfoData);
		DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);
            	if (SetupDiEnumDeviceInfo(ListHandle, 0, &DeviceInfoData)) {
	                SetupDiGetDeviceRegistryProperty(ListHandle, &DeviceInfoData, SPDRP_DEVICEDESC, NULL, KeyName2, 200, &reqsize);
                	if (SetupDiEnumDeviceInterfaces(ListHandle, &DeviceInfoData, &DKU2FBUS2, 0, &DeviceInterfaceData)) {
				DeviceInterfaceDetailData->cbSize = 5;
                    		if (SetupDiGetDeviceInterfaceDetail(ListHandle, &DeviceInterfaceData, DeviceInterfaceDetailData, 200, &reqsize, &DeviceInfoData)) {
					strcpy(DeviceName2,DeviceInterfaceDetailData->DevicePath);
				}
			}
		}
	        SetupDiDestroyDeviceInfoList(ListHandle);
	}
#endif
#ifdef GSM_ENABLE_DKU2AT
	if (s->ConnectionType == GCT_DKU2AT) {
	        ListHandle = SetupDiGetClassDevs(&DKU2AT, NULL, NULL,  DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
		if (ListHandle == INVALID_HANDLE_VALUE) {
		        SetupDiDestroyDeviceInfoList(ListHandle);
			return ERR_UNKNOWN;
		}
		DeviceInfoData.cbSize = sizeof(DeviceInfoData);
		DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);
            	if (SetupDiEnumDeviceInfo(ListHandle, 0, &DeviceInfoData)) {
	                SetupDiGetDeviceRegistryProperty(ListHandle, &DeviceInfoData, SPDRP_DEVICEDESC, NULL, KeyName2, 200, &reqsize);
                	if (SetupDiEnumDeviceInterfaces(ListHandle, &DeviceInfoData, &DKU2AT, 0, &DeviceInterfaceData)) {
				DeviceInterfaceDetailData->cbSize = 5;
                    		if (SetupDiGetDeviceInterfaceDetail(ListHandle, &DeviceInterfaceData, DeviceInterfaceDetailData, 200, &reqsize, &DeviceInfoData)) {
					strcpy(DeviceName2,DeviceInterfaceDetailData->DevicePath);
				}
			}
		}
	        SetupDiDestroyDeviceInfoList(ListHandle);
	}
#endif
#ifdef GSM_ENABLE_DKU5FBUS2
	if (s->ConnectionType == GCT_DKU5FBUS2) {
		smprintf(s,"Reading DKU5 device\n");
		/* win xp */
		DeviceName2[0] = 0;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) {
			smprintf(s,"Error opening key\n");
			return ERR_DEVICENOTWORK;
		}
		i = 0;
		while(1) {
			DeviceNameLen = 80;
			KeyNameLen = 100;
			if (RegEnumValue(hKey,i,KeyName,&KeyNameLen,NULL,NULL,DeviceName2,&DeviceNameLen) != ERROR_SUCCESS) break;
			if (!strncmp(KeyName,"\\Device\\AtmelVirtualPort",24)) break;
			DeviceName2[0] = 0;
			i++;
		}
		RegCloseKey(hKey);
		if (strlen(DeviceName2) == 0) {
			/*  win 98 */
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Enum\\USBVSP\\ATMELPORT\\0000", 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) {
				smprintf(s,"Error opening key\n");
				return ERR_DEVICENOTWORK;
			}
			i = 0;
			while(1) {
				DeviceNameLen = 80;
				KeyNameLen = 100;
				if (RegEnumValue(hKey,i,KeyName,&KeyNameLen,NULL,NULL,DeviceName2,&DeviceNameLen) != ERROR_SUCCESS) break;
				if (!strncmp(KeyName,"PORTNAME",8)) break;
				DeviceName2[0] = 0;
				i++;
			}
			RegCloseKey(hKey);
			if (strlen(DeviceName2) == 0) return ERR_DEVICENOTWORK;
		}
		smprintf(s,"DKU5 device is \"%s\"\n",DeviceName2);
	}
#endif

	if ((s->ConnectionType == GCT_DKU5FBUS2) ||
	    (!strncmp(DeviceName2,"com",3) && strlen(DeviceName2)>3)) {
		sprintf(DeviceName,"\\\\.\\COM%i",atoi(DeviceName2+3));
	} else {
		strcpy(DeviceName,DeviceName2);
	}

	smprintf(s,"Device is %s\n",DeviceName);

	/* Allows for reading/writing, no device sharing */
	d->hPhone = CreateFile(DeviceName,
			       GENERIC_READ | GENERIC_WRITE,
			       0,
			       0,
			       OPEN_EXISTING,
			       FILE_ATTRIBUTE_NORMAL,
			       NULL);

	if (d->hPhone == INVALID_HANDLE_VALUE) {
		err = GetLastError();
		GSM_OSErrorInfo(s, "CreateFile in serial_open");
		if (err == ERROR_FILE_NOT_FOUND)   return ERR_DEVICENOTEXIST; /* can't find specified file */
		if (err == ERROR_ACCESS_DENIED)   return ERR_DEVICEBUSY;    /* access denied */
		if (err == ERROR_GEN_FAILURE)  return ERR_DEVICENOTWORK; /* attached device not working */
		if (err == ERROR_INVALID_NAME) return ERR_DEVICENOTEXIST;
		if (err == ERROR_PATH_NOT_FOUND)   return ERR_DEVICENOTEXIST; /* can't find specified file */
		return ERR_DEVICEOPENERROR;
	}

	if (s->ConnectionType != GCT_DKU2PHONET && s->ConnectionType != GCT_DKU2AT) {
		d->old_settings.DCBlength = sizeof(DCB);
		if (GetCommState(d->hPhone, &d->old_settings)==0) {
			GSM_OSErrorInfo(s, "ReadDevice in serial_open");
			return ERR_DEVICEREADERROR;
		}

		/* When char will be received, we will receive notifications */
		SetCommMask(d->hPhone, EV_RXCHAR);

		/* Sets size for input/output buffer */
		SetupComm(d->hPhone, 4096, 4096);

		/* Discards all characters from input/output buffer and terminates
		 * pending read/write operations
		 */
		PurgeComm(d->hPhone, PURGE_TXABORT | PURGE_RXABORT |
				     PURGE_TXCLEAR | PURGE_RXCLEAR);

	 	memcpy(&dcb, &d->old_settings, sizeof(DCB));

		dcb.ByteSize 	 = 8;
		dcb.Parity 	 = NOPARITY;
		dcb.StopBits 	 = ONESTOPBIT;

		/* No Xon/Xof flow control */
	/* 	dcb.fOutX 	 = FALSE; */
	/* 	dcb.fInX 	 = FALSE; */

		/* Hardware flow control */
	/* 	dcb.fOutxDsrFlow = TRUE; */
	/* 	dcb.fOutxCtsFlow = TRUE; */
	/* 	dcb.fDtrControl  = DTR_CONTROL_HANDSHAKE; */
	/* 	dcb.fRtsControl  = RTS_CONTROL_HANDSHAKE; */

	    	/* Initialise the port settings */
		if (SetCommState(d->hPhone, &dcb)==0) {
			GSM_OSErrorInfo(s, "WriteDevice in serial_open");
			return ERR_DEVICEOPENERROR;
		}
	}

	return ERR_NONE;
}

static GSM_Error serial_setparity (GSM_StateMachine *s, gboolean parity)
{
	DCB 			dcb;
	GSM_Device_SerialData 	*d = &s->Device.Data.Serial;

	dcb.DCBlength = sizeof(DCB);
	if (GetCommState(d->hPhone, &dcb)==0) {
		GSM_OSErrorInfo(s, "ReadDevice in serial_setparity");
		return ERR_DEVICEREADERROR;
	}

	if (parity) {
		dcb.Parity = ODDPARITY;
	} else {
		dcb.Parity = NOPARITY;
	}

	if (SetCommState(d->hPhone, &dcb)==0) {
		GSM_OSErrorInfo(s, "WriteDevice in serial_setparity");
		return ERR_DEVICEPARITYERROR;
	}

	return ERR_NONE;
}

static GSM_Error serial_setdtrrts(GSM_StateMachine *s, gboolean dtr, gboolean rts)
{
	DCB        		dcb;
	GSM_Device_SerialData 	*d = &s->Device.Data.Serial;

	if (s->SkipDtrRts) return ERR_NONE;

	dcb.DCBlength = sizeof(DCB);
	if (GetCommState(d->hPhone, &dcb)==0) {
		GSM_OSErrorInfo(s, "ReadDevice in serial_setdtrrts");
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

	if (SetCommState(d->hPhone, &dcb)==0) {
		GSM_OSErrorInfo(s, "WriteDevice in serial_setdtrrts");
		return ERR_DEVICEDTRRTSERROR;
	}

	/* the rest of function checks, if setting was really done */

	dcb.DCBlength = sizeof(DCB);
	GetCommState(d->hPhone, &dcb);

	smprintf(s, "Serial device:");
	smprintf(s, " DTR is ");
	switch (dcb.fDtrControl) {
		case DTR_CONTROL_ENABLE    : smprintf(s, "up");        break;
		case DTR_CONTROL_DISABLE   : smprintf(s, "down");      break;
		case DTR_CONTROL_HANDSHAKE : smprintf(s, "handshake"); break;
	}
	smprintf(s, ", RTS is ");
	switch (dcb.fRtsControl) {
		case RTS_CONTROL_ENABLE    : smprintf(s, "up");        break;
		case RTS_CONTROL_DISABLE   : smprintf(s, "down");      break;
		case RTS_CONTROL_HANDSHAKE : smprintf(s, "handshake"); break;
		case RTS_CONTROL_TOGGLE    : smprintf(s, "toggle");    break;
	}
	smprintf(s, "\n");
	if ( dtr && dcb.fDtrControl != DTR_CONTROL_ENABLE ) return ERR_DEVICEDTRRTSERROR;
	if (!dtr && dcb.fDtrControl != DTR_CONTROL_DISABLE) return ERR_DEVICEDTRRTSERROR;
	if ( rts && dcb.fRtsControl != RTS_CONTROL_ENABLE ) return ERR_DEVICEDTRRTSERROR;
	if (!rts && dcb.fRtsControl != RTS_CONTROL_DISABLE) return ERR_DEVICEDTRRTSERROR;

	return ERR_NONE;
}

static GSM_Error serial_setspeed(GSM_StateMachine *s, int speed)
{
	DCB			dcb;
	GSM_Device_SerialData 	*d = &s->Device.Data.Serial;

	if (s->SkipDtrRts) return ERR_NONE;

	dcb.DCBlength = sizeof(DCB);
	if (GetCommState(d->hPhone, &dcb)==0) {
		GSM_OSErrorInfo(s, "ReadDevice in serial_setspeed");
		return ERR_DEVICEREADERROR;
	}

	dcb.BaudRate = speed;

	if (SetCommState(d->hPhone, &dcb)==0) {
		GSM_OSErrorInfo(s, "WriteDevice in serial_setspeed");
		return ERR_DEVICECHANGESPEEDERROR;
	}

    	smprintf(s, "Setting speed to %d\n", speed);

	return ERR_NONE;
}

static int serial_read(GSM_StateMachine *s, void *buf, size_t nbytes)
{
	COMSTAT			ComStat;
	DWORD			ErrorFlags, Length, Error;
	GSM_Device_SerialData 	*d = &s->Device.Data.Serial;
	BOOL			ReadStatus;

	if (s->ConnectionType != GCT_DKU2PHONET && s->ConnectionType != GCT_DKU2AT) {
		/* Gets information about a communications error and
		 * current status of device
		 */
		ClearCommError(d->hPhone, &ErrorFlags, &ComStat);

		/* How much we can read? */
		Length = min(nbytes, ComStat.cbInQue);
	} else {
		Length = 5;
	}

	/* Nothing to read */
	if (Length <= 0) goto end;

	/* Isn't that to much? */
	if (Length > nbytes) Length = nbytes;

	/* Read without problems */
	ReadStatus = ReadFile(d->hPhone, buf, Length, &Length, &d->osRead);

	if (!ReadStatus) {
		if (GetLastError() != ERROR_IO_PENDING) {
			Length = 0;
			ClearCommError(d->hPhone, &ErrorFlags, &ComStat);
			goto end;
		}

		/* Operation did not complete */
		GSM_OSErrorInfo(s, "win_serial_read-pending");

		/* Wait for completing */
		while (!GetOverlappedResult(d->hPhone, &d->osRead, &Length, TRUE)) {
			Error = GetLastError();
			if (Error == ERROR_IO_INCOMPLETE) {
				/* Just go on */
				continue;
			} else {
				/* Something went wrong, bail out */
				GSM_OSErrorInfo(s, "win_serial_read-overlapped");
				ClearCommError(d->hPhone, &ErrorFlags, &ComStat);
				break;
			}
		}
	}
end:
	return Length;
}

static int serial_write(GSM_StateMachine *s, const void *buf, size_t nbytes)
{
	DWORD			BytesWritten,ErrorFlags,BytesSent=0;
	COMSTAT			ComStat;
	GSM_Device_SerialData 	*d = &s->Device.Data.Serial;

	if (WriteFile(d->hPhone, buf, nbytes, &BytesSent, &d->osWrite)) return BytesSent;

	if (GetLastError() != ERROR_IO_PENDING) {
	    	GSM_OSErrorInfo(s, "serial_write1");
	    	ClearCommError(d->hPhone, &ErrorFlags, &ComStat);
	    	return BytesSent;
	}

	while (1) {
		if (GetOverlappedResult(d->hPhone, &d->osWrite, &BytesWritten, TRUE)) break;
		if (GetLastError() != ERROR_IO_INCOMPLETE) {
			GSM_OSErrorInfo(s, "serial_write2");
			ClearCommError(d->hPhone, &ErrorFlags, &ComStat);
			break;
		}
		BytesSent += BytesWritten;
	}
	BytesSent += BytesWritten;

	return BytesSent;
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

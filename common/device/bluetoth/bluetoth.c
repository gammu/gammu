/* (c) 2003-2004 by Marcin Wiacek and Intra */
/* Linux part based on work by Marcel Holtmann */

#include "../../gsmstate.h"

#ifdef GSM_ENABLE_BLUETOOTHDEVICE
#ifndef DJGPP

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#ifndef WIN32
#  include <sys/socket.h>
#  include <sys/time.h>
#  include <unistd.h>
#  include <bluetooth/bluetooth.h>
#  include <bluetooth/rfcomm.h>
#else
#  include <windows.h>
#  include <io.h>
#endif

#include "../../gsmcomon.h"
#include "../devfunc.h"
#include "bluetoth.h"

#ifdef WIN32

static GSM_Error bluetooth_connect(GSM_StateMachine *s, int port, char *device)
{
	GSM_Device_BlueToothData 	*d = &s->Device.Data.BlueTooth;
    	WSADATA				wsaData;
	SOCKADDR_BTH 			sab;
	int				i;

	dbgprintf("Connecting to RF channel %i\n",port);

    	WSAStartup(MAKEWORD(1,1), &wsaData);

    	d->hPhone = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
	if (d->hPhone == INVALID_SOCKET) {
		i = GetLastError();
		GSM_OSErrorInfo(s, "Socket in bluetooth_open");
		if (i == 10041) return ERR_DEVICENODRIVER;//unknown socket type
		return ERR_UNKNOWN;
	}

	memset (&sab, 0, sizeof(sab));
	sab.port 		= port;
	sab.addressFamily 	= AF_BTH;
	sab.btAddr 		= 0;
	for (i=0;i<(int)strlen(device);i++) {
		if (device[i] >='0' && device[i] <='9') {
			sab.btAddr = sab.btAddr * 16;
			sab.btAddr = sab.btAddr + (device[i]-'0');
		}
		if (device[i] >='a' && device[i] <='f') {
			sab.btAddr = sab.btAddr * 16;
			sab.btAddr = sab.btAddr + (device[i]-'a'+10);
		}
		if (device[i] >='A' && device[i] <='F') {
			sab.btAddr = sab.btAddr * 16;
			sab.btAddr = sab.btAddr + (device[i]-'A'+10);
		}
	}
	dbgprintf("Remote Bluetooth device is %04x%08x\n",
	  		GET_NAP(sab.btAddr), GET_SAP(sab.btAddr));

	if (connect (d->hPhone, (struct sockaddr *)&sab, sizeof(sab)) != 0) {
		i = GetLastError();
		GSM_OSErrorInfo(s, "Connect in bluetooth_open");
		if (i == 10060) return ERR_TIMEOUT;	 //remote device failed to respond
		if (i == 10050) return ERR_DEVICENOTWORK; //socket operation connected with dead network
		//noauth
		close(d->hPhone);
		return ERR_UNKNOWN;
	}

	return ERR_NONE;
}

#ifdef MS_VC_BLUETOOTH_IRPROPS_LIB

#pragma comment(lib, "irprops.lib")
#pragma comment(lib, "ws2_32.lib")

static GSM_Error bluetooth_checkdevice(GSM_StateMachine *s, char *address, WSAPROTOCOL_INFO *protocolInfo)
{
	GSM_Device_BlueToothData 	*d = &s->Device.Data.BlueTooth;
	WSAQUERYSET 			querySet;
	DWORD				flags;
	GUID				protocol;
	int				i, result;
	BYTE 				buffer[2000];
	char 				addressAsString[1000];
	DWORD 				bufferLength, addressSize;
	WSAQUERYSET 			*pResults = (WSAQUERYSET*)&buffer;
	HANDLE				handle;

	memset(&querySet, 0, sizeof(querySet));
	querySet.dwSize 	  = sizeof(querySet);
	protocol 		  = L2CAP_PROTOCOL_UUID;
	querySet.lpServiceClassId = &protocol;
	querySet.dwNameSpace 	  = NS_BTH;
	querySet.lpszContext 	  = address;

	flags = LUP_FLUSHCACHE |LUP_RETURN_NAME |
		LUP_RETURN_TYPE | LUP_RETURN_ADDR |
		LUP_RETURN_BLOB | LUP_RETURN_COMMENT;

        result = WSALookupServiceBegin(&querySet, flags, &handle);
	if (result != 0) return ERR_UNKNOWN;

	bufferLength = sizeof(buffer);
	while (1) {
                result = WSALookupServiceNext(handle, flags, &bufferLength, pResults);
		if (result != 0) break;
 		addressSize 		= sizeof(addressAsString);
		addressAsString[0] 	= 0;
		if (WSAAddressToString(pResults->lpcsaBuffer->RemoteAddr.lpSockaddr,
			pResults->lpcsaBuffer->RemoteAddr.iSockaddrLength, protocolInfo, 
			addressAsString,&addressSize)==0) {
                	dbgprintf("%s - ", addressAsString);
		}
		dbgprintf("\"%s\"\n", pResults->lpszServiceInstanceName);
		if (addressAsString[0] != 0) {
			for (i=strlen(addressAsString)-1;i>0;i--) {
				if (addressAsString[i] == ':') break;
			}
			if (s->ConnectionType == GCT_BLUEPHONET && strstr(_strupr(pResults->lpszServiceInstanceName),"NOKIA PC SUITE")!=NULL) {
				return bluetooth_connect(s,atoi(addressAsString+i+1),address+1);
			}
			if (s->ConnectionType == GCT_BLUEOBEX && strstr(_strupr(pResults->lpszServiceInstanceName),"OBEX")!=NULL) {
				return bluetooth_connect(s,atoi(addressAsString+i+1),address+1);
			}
			if (s->ConnectionType == GCT_BLUEAT && strstr(_strupr(pResults->lpszServiceInstanceName),"COM")!=NULL) {
				return bluetooth_connect(s,atoi(addressAsString+i+1),address+1);
			}
		}
	}
	result = WSALookupServiceEnd(handle);
	return ERR_NOTSUPPORTED;
}

static GSM_Error bluetooth_findchannel(GSM_StateMachine *s)
{
	GSM_Device_BlueToothData 	*d = &s->Device.Data.BlueTooth;
    	WSADATA				wsaData;
	int				i, protocolInfoSize, result;
	WSAPROTOCOL_INFO 		protocolInfo;
	HANDLE 				handle;
	DWORD				flags;
	WSAQUERYSET 			querySet;
	BYTE 				buffer[2000];
	char 				addressAsString[1000];
	DWORD 				bufferLength, addressSize;
	WSAQUERYSET 			*pResults = (WSAQUERYSET*)&buffer;
	GSM_Error			error;

    	if (WSAStartup(MAKEWORD(2,2), &wsaData)!=0x00) return ERR_DEVICENODRIVER;

    	d->hPhone = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
	if (d->hPhone == INVALID_SOCKET) {
		i = GetLastError();
		GSM_OSErrorInfo(s, "Socket in bluetooth_open");
		if (i == 10041) return ERR_DEVICENODRIVER;//unknown socket type
		return ERR_UNKNOWN;
	}

	protocolInfoSize = sizeof(protocolInfo);
      	if (getsockopt(d->hPhone, SOL_SOCKET, SO_PROTOCOL_INFO,
		(char*)&protocolInfo, &protocolInfoSize) != 0)
	{
		return ERR_UNKNOWN;		
	}

	if (!strcmp(s->CurrentConfig->Device,"com2:")) {
		bufferLength = sizeof(buffer);

      		flags = LUP_RETURN_NAME | LUP_CONTAINERS | 
			LUP_RETURN_ADDR | LUP_FLUSHCACHE | 
			LUP_RETURN_TYPE | LUP_RETURN_BLOB | LUP_RES_SERVICE;

      		memset(&querySet, 0, sizeof(querySet));
      		querySet.dwSize 	= sizeof(querySet);
      		querySet.dwNameSpace 	= NS_BTH;

	        result = WSALookupServiceBegin(&querySet, flags, &handle);
		if (result != 0) return ERR_UNKNOWN;

		while (1) {
			result = WSALookupServiceNext(handle, flags, &bufferLength, pResults);
			if (result != 0) break;

 	                printf("%s", pResults->lpszServiceInstanceName);

	 		addressSize 		= sizeof(addressAsString);
			addressAsString[0] 	= 0;
			if (WSAAddressToString(pResults->lpcsaBuffer->RemoteAddr.lpSockaddr,
				pResults->lpcsaBuffer->RemoteAddr.iSockaddrLength, &protocolInfo, 
				addressAsString,&addressSize)==0) {
	                	dbgprintf("- %s\n", addressAsString);
				error = bluetooth_checkdevice(s, addressAsString,&protocolInfo);
				if (error == ERR_NONE) return error;
			} else dbgprintf("\n");
		}

		result = WSALookupServiceEnd(handle);
		return ERR_NOTSUPPORTED;
	} else {
		return bluetooth_checkdevice(s, s->CurrentConfig->Device,&protocolInfo);
	}
}
#endif

#else

/* Linux */

static GSM_Error bluetooth_connect(GSM_StateMachine *s, int port, char *device)
{
	GSM_Device_BlueToothData 	*d = &s->Device.Data.BlueTooth;
	struct sockaddr_rc 		laddr, raddr;
	bdaddr_t			bdaddr;
	int 				fd;

	fd = socket(PF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	if (fd < 0) {
		dbgprintf("Can't create socket\n");
		return ERR_DEVICENODRIVER;
	}

	bacpy(&laddr.rc_bdaddr, BDADDR_ANY);
	laddr.rc_family 	= AF_BLUETOOTH;
	laddr.rc_channel 	= 0;

	if (bind(fd, (struct sockaddr *)&laddr, sizeof(laddr)) < 0) {
		dbgprintf("Can't bind socket\n");
		close(fd);
		return ERR_DEVICEOPENERROR;
	}

	str2ba(device, &bdaddr);
	bacpy(&raddr.rc_bdaddr, &bdaddr);
	raddr.rc_family 	= AF_BLUETOOTH;
	raddr.rc_channel 	= port;

	if (connect(fd, (struct sockaddr *)&raddr, sizeof(raddr)) < 0) {
		dbgprintf("Can't connect\n");
		close(fd);
		return ERR_DEVICEOPENERROR;
	}

	d->hPhone = fd;
    	return ERR_NONE;
}
#endif

static GSM_Error bluetooth_open (GSM_StateMachine *s)
{
#ifdef MS_VC_BLUETOOTH_IRPROPS_LIB
	return bluetooth_findchannel(s);
#endif

	switch (s->ConnectionType) {
	case GCT_BLUEAT:
		return bluetooth_connect(s,1,s->CurrentConfig->Device);
	case GCT_BLUEOBEX:
		return bluetooth_connect(s,9,s->CurrentConfig->Device);
	case GCT_BLUEPHONET:
//		return bluetooth_connect(s,14,s->CurrentConfig->Device);
		return bluetooth_connect(s,15,s->CurrentConfig->Device);
	default:
		return ERR_UNKNOWN;
	}
}

static int bluetooth_read(GSM_StateMachine *s, void *buf, size_t nbytes)
{
	return socket_read(s, buf, nbytes, s->Device.Data.BlueTooth.hPhone);
}

#ifdef WIN32
static int bluetooth_write(GSM_StateMachine *s, unsigned char *buf, size_t nbytes)
#else
static int bluetooth_write(GSM_StateMachine *s, void *buf, size_t nbytes)
#endif
{
	return socket_write(s, buf, nbytes, s->Device.Data.BlueTooth.hPhone);
}

static GSM_Error bluetooth_close(GSM_StateMachine *s)
{
	return socket_close(s, s->Device.Data.BlueTooth.hPhone);
}

GSM_Device_Functions BlueToothDevice = {
	bluetooth_open,
	bluetooth_close,
	NONEFUNCTION,
	NONEFUNCTION,
	NONEFUNCTION,
	bluetooth_read,
	bluetooth_write
};

#endif
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

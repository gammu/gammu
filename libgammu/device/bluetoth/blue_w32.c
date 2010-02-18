/* (c) 2003-2004 by Marcin Wiacek and Intra */

/* To define GUID and not only declare */
#define INITGUID
#include "../../gsmstate.h"

#ifdef GSM_ENABLE_BLUETOOTHDEVICE
#ifdef WIN32

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ole2.h>
#include <io.h>

#include "../../misc/coding/coding.h"
#include "../../gsmcomon.h"
#include "../devfunc.h"
#include "bluetoth.h"
#include "blue_w32.h"

GSM_Error bluetooth_connect(GSM_StateMachine *s, int port, char *device)
{
	GSM_Device_BlueToothData 	*d = &s->Device.Data.BlueTooth;
    	WSADATA				wsaData;
	SOCKADDR_BTH 			sab;
	DWORD				err;
	const char *ch;

	smprintf(s, "Connecting to RF channel %i\n",port);

	/* BCC comes with broken MAKEWORD, which emmits warnings */
#ifdef __BORLANDC__
#    pragma warn -8084
#endif
    	WSAStartup(MAKEWORD(1,1), &wsaData);
#ifdef __BORLANDC__
#    pragma warn +8084
#endif

    	d->hPhone = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
	if (d->hPhone == INVALID_SOCKET) {
		err = GetLastError();
		GSM_OSErrorInfo(s, "Socket in bluetooth_open");
		if (err == WSAEPROTOTYPE)
			return ERR_DEVICENODRIVER;/* unknown socket type */
		return ERR_UNKNOWN;
	}

	memset (&sab, 0, sizeof(sab));
	sab.port 		= port;
	sab.addressFamily 	= AF_BTH;
	sab.btAddr 		= 0;
	for (ch = device; *ch != '\0'; ch++){
		if (*ch >='0' && *ch <='9') {
			sab.btAddr = sab.btAddr * 16;
			sab.btAddr = sab.btAddr + (*ch-'0');
		} else if (*ch >='a' && *ch <='f') {
			sab.btAddr = sab.btAddr * 16;
			sab.btAddr = sab.btAddr + (*ch-'a'+10);
		} else if (*ch >='A' && *ch <='F') {
			sab.btAddr = sab.btAddr * 16;
			sab.btAddr = sab.btAddr + (*ch-'A'+10);
		}
	}

	smprintf(s, "Remote Bluetooth device is %02x:%02x:%02x:%02x:%02x:%02x\n",
			GET_BYTE(sab.btAddr, 5),
			GET_BYTE(sab.btAddr, 4),
			GET_BYTE(sab.btAddr, 3),
			GET_BYTE(sab.btAddr, 2),
			GET_BYTE(sab.btAddr, 1),
			GET_BYTE(sab.btAddr, 0));

	if (connect (d->hPhone, (struct sockaddr *)&sab, sizeof(sab)) != 0) {
		err = GetLastError();
		GSM_OSErrorInfo(s, "Connect in bluetooth_open");

		/* Close the handle */
		close(d->hPhone);

		switch (err) {
			case WSAETIMEDOUT:
				smprintf(s, "The I/O timed out at the Bluetooth radio level (PAGE_TIMEOUT).\n");
				return ERR_TIMEOUT;
			case WSAEDISCON:
				smprintf(s, "The RFCOMM channel disconnected by remote peer.\n");
				return ERR_DEVICEOPENERROR;
			case WSAEHOSTDOWN:
				smprintf(s, "The RFCOMM received DM response.\n");
				return ERR_DEVICEOPENERROR;
			case WSAENETDOWN:
				smprintf(s, "Unexpected network error.\n");
				return ERR_DEVICENOTWORK;
			case WSAESHUTDOWN:
				smprintf(s, "The L2CAP channel disconnected by remote peer.\n");
				return ERR_DEVICEOPENERROR;
			case WSAENETUNREACH:
				smprintf(s, "Error other than time-out at L2CAP or Bluetooth radio level.\n");
				return ERR_DEVICENOTWORK;
		}
		return ERR_UNKNOWN;
	}

	return ERR_NONE;
}

#ifdef BLUETOOTH_RF_SEARCHING

DEFINE_GUID(L2CAP_PROTOCOL_UUID,  0x00000100, 0x0000, 0x1000, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB);

#ifndef __GNUC__
#pragma comment(lib, "ws2_32.lib")
#endif

static GSM_Error bluetooth_checkdevice(GSM_StateMachine *s, char *address, WSAPROTOCOL_INFO *protocolInfo)
{
	int				found = -1;
	int				score, bestscore = 0;
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

	flags = LUP_FLUSHCACHE  | LUP_RETURN_NAME |
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
                	smprintf(s, "%s - ", addressAsString);
		}
		score = bluetooth_checkservicename(s, pResults->lpszServiceInstanceName);
		smprintf(s, "\"%s\" (score=%d)\n", pResults->lpszServiceInstanceName, score);
		if (addressAsString[0] != 0) {
			for (i=strlen(addressAsString)-1;i>0;i--) {
				if (addressAsString[i] == ':') break;
			}
			if (score > bestscore) {
				found = atoi(addressAsString+i+1);
				bestscore = score;
			}
		}
	}
	result = WSALookupServiceEnd(handle);
	if (found != -1) {
		return bluetooth_connect(s,found,address+1);
	}
	return ERR_NOTSUPPORTED;
}

GSM_Error bluetooth_findchannel(GSM_StateMachine *s)
{
	GSM_Device_BlueToothData 	*d = &s->Device.Data.BlueTooth;
    	WSADATA				wsaData;
	int				protocolInfoSize, result;
	WSAPROTOCOL_INFO 		protocolInfo;
	HANDLE 				handle;
	DWORD				flags;
	WSAQUERYSET 			querySet;
	BYTE 				buffer[2000];
	char 				addressAsString[1000];
	DWORD 				bufferLength, addressSize;
	WSAQUERYSET 			*pResults = (WSAQUERYSET*)&buffer;
	GSM_Error			error;
	DWORD				err;

	/* BCC comes with broken MAKEWORD, which emmits warnings */
#ifdef __BORLANDC__
#    pragma warn -8084
#endif
    	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0x00) return ERR_DEVICENODRIVER;
#ifdef __BORLANDC__
#    pragma warn +8084
#endif

    	d->hPhone = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
	if (d->hPhone == INVALID_SOCKET) {
		err = GetLastError();
		GSM_OSErrorInfo(s, "Socket in bluetooth_open");
		if (err == WSAEPROTOTYPE)
			return ERR_DEVICENODRIVER;/* unknown socket type */
		smprintf(s, "Failed to socket in bluetooth_open\n");
		return ERR_UNKNOWN;
	}

	protocolInfoSize = sizeof(protocolInfo);
      	if (getsockopt(d->hPhone, SOL_SOCKET, SO_PROTOCOL_INFO,
		(char*)&protocolInfo, &protocolInfoSize) != 0)
	{
		GSM_OSErrorInfo(s, "getsockopt in bluetooth_open");
		close(d->hPhone);
		smprintf(s, "Failed to getsockopt in bluetooth_open\n");
		return ERR_UNKNOWN;
	}
	close(d->hPhone);

	if (!strcmp(s->CurrentConfig->Device,"com2:")) {
		bufferLength = sizeof(buffer);

      		flags = LUP_RETURN_NAME | LUP_CONTAINERS |
			LUP_RETURN_ADDR | LUP_FLUSHCACHE |
			LUP_RETURN_TYPE | LUP_RETURN_BLOB | LUP_RES_SERVICE;

      		memset(&querySet, 0, sizeof(querySet));
      		querySet.dwSize 	= sizeof(querySet);
      		querySet.dwNameSpace 	= NS_BTH;

	        result = WSALookupServiceBegin(&querySet, flags, &handle);
		GSM_OSErrorInfo(s, "WSALookupServiceBegin in bluetooth_open");
		if (result != 0) {
			smprintf(s, "Failed to WSALookupServiceBegin in bluetooth_open\n");
			return ERR_UNKNOWN;
		}

		while (1) {
			result = WSALookupServiceNext(handle, flags, &bufferLength, pResults);
			if (result != 0) break;

 	                smprintf(s, "\"%s\"", pResults->lpszServiceInstanceName);

	 		addressSize 		= sizeof(addressAsString);
			addressAsString[0] 	= 0;
			if (WSAAddressToString(pResults->lpcsaBuffer->RemoteAddr.lpSockaddr,
				pResults->lpcsaBuffer->RemoteAddr.iSockaddrLength, &protocolInfo,
				addressAsString,&addressSize)==0) {
	                	smprintf(s, " - %s\n", addressAsString);
				error = bluetooth_checkdevice(s, addressAsString,&protocolInfo);
				if (error == ERR_NONE) {
					free(s->CurrentConfig->Device);
					s->CurrentConfig->Device = strdup(addressAsString);
					if (s->CurrentConfig->Device == NULL) {
						return ERR_MOREMEMORY;
					}
					result = WSALookupServiceEnd(handle);
					return error;
				}
			} else smprintf(s, "\n");
		}

		result = WSALookupServiceEnd(handle);
		return ERR_NOTSUPPORTED;
	} else {
		return bluetooth_checkdevice(s, s->CurrentConfig->Device,&protocolInfo);
	}
}

#endif
#endif
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

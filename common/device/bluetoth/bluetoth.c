/* (c) 2003-2004 by Marcin Wiacek and Intra */
/* Linux part based on work by Marcel Holtmann and other authors of Bluez */

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
#  include <bluetooth/sdp.h>
#  include <bluetooth/sdp_lib.h>
#else
#  include <windows.h>
#  include <io.h>
#endif

#include "../../gsmcomon.h"
#include "../devfunc.h"
#include "bluetoth.h"

static GSM_Error bluetooth_checkservicename(GSM_StateMachine *s, char *name)
{
        if (s->ConnectionType == GCT_BLUEPHONET && strstr(name,"Nokia PC Suite")!=NULL) return ERR_NONE;
	if (s->ConnectionType == GCT_BLUEOBEX 	&& strstr(name,"OBEX")		!=NULL) return ERR_NONE;
        if (s->ConnectionType == GCT_BLUEAT 	&& strstr(name,"COM 1")		!=NULL) return ERR_NONE;
        return ERR_UNKNOWN;
}

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
			if bluetooth_checkservicename(s, pResults->lpszServiceInstanceName) == ERR_NONE) {
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

 	                dbgprintf("\"%s\"", pResults->lpszServiceInstanceName);

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

#else

static GSM_Error bluetooth_findchannel(GSM_StateMachine *s)
{
	switch (s->ConnectionType) {
	case GCT_BLUEAT:
		return bluetooth_connect(s,1,s->CurrentConfig->Device);
	case GCT_BLUEOBEX:
		return bluetooth_connect(s,9,s->CurrentConfig->Device);
	case GCT_BLUEPHONET:
//		return bluetooth_connect(s,14,s->CurrentConfig->Device); //older Series 40 - 8910, 6310
		return bluetooth_connect(s,15,s->CurrentConfig->Device); //new Series 40 - 6310i, 6230
	default:
		return ERR_UNKNOWN;
	}
}

#endif

#else   /* Linux */

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

struct search_context {
	char				*svc;
	uuid_t				group;
	int				tree;
	uint32_t			handle;
};

static void print_service_desc(void *value, void *user) 
{ 
	sdp_data_t 	*p = (sdp_data_t *)value; 
	int 		i = 0, proto = 0, *channel = (int *)user; 

	(*channel) = -1;
	
	for (; p; p = p->next, i++) { 
		switch (p->dtd) { 
		case SDP_UUID16:
		case SDP_UUID32:
		case SDP_UUID128:
			proto = sdp_uuid_to_proto(&p->val.uuid);
			break;
		case SDP_UINT8: 
			if (proto == RFCOMM_UUID) {
				dbgprintf("Channel %05d", p->val.uint8); 
				(*channel) = p->val.uint8;
				return;
			}
			break; 
		} 
	} 
}

void print_access_protos(value, user)
{
	sdp_list_t 	*protDescSeq = (sdp_list_t *)value;
	int		*channel = (int *)user;
	
	sdp_list_foreach(protDescSeq,print_service_desc,channel);
}

static GSM_Error bluetooth_checkdevice(GSM_StateMachine *s, bdaddr_t *bdaddr, struct search_context *context)
{
	sdp_session_t			*sess;
	sdp_list_t			*attrid, *search, *seq, *next, *proto = 0;
	uint32_t			range = 0x0000ffff;
	char				str[20];
	sdp_record_t 			*rec;
	sdp_data_t			*d;
	bdaddr_t			interface;
	struct search_context 		subcontext;
	int				channel,channel2;

	bacpy(&interface,BDADDR_ANY);

	ba2str(bdaddr, str);
	dbgprintf("%s\n", str);

	sess = sdp_connect(&interface, bdaddr, SDP_RETRY_IF_BUSY);
	if (!sess) {
		dbgprintf("Failed to connect to SDP server on %s: %s\n", str, strerror(errno));
		return ERR_UNKNOWN;
	}

	attrid = sdp_list_append(0, &range);
	search = sdp_list_append(0, &context->group);
	if (sdp_service_search_attr_req(sess, search, SDP_ATTR_REQ_RANGE, attrid, &seq)) {
		dbgprintf("Service Search failed: %s\n", strerror(errno));
		sdp_close(sess);
		return ERR_UNKNOWN;
	}
	sdp_list_free(attrid, 0);
	sdp_list_free(search, 0);

	channel2 = -1;
	for (; seq; seq = next) {
		rec 	= (sdp_record_t *) seq->data;
		
		if (channel2 == -1) {		
			if (!context->tree) {
				d = sdp_data_get(rec,SDP_ATTR_SVCNAME_PRIMARY);
			
				if (sdp_get_access_protos(rec,&proto) == 0) {
					sdp_list_foreach(proto,print_access_protos,&channel);
					sdp_list_free(proto,(sdp_free_func_t)sdp_data_free);
				}
				if (d) dbgprintf(" - \"%s\"\n",d->val.str);
				if (channel2 == -1 && bluetooth_checkservicename(s, d->val.str) == ERR_NONE) {
					channel2 = channel;			
				}			
			}
    			if (sdp_get_group_id(rec,&subcontext.group) != -1) {
				memcpy(&subcontext, context, sizeof(struct search_context));
				if (subcontext.group.value.uuid16 != context->group.value.uuid16) bluetooth_checkdevice(s,bdaddr,&subcontext);
			}
		}

		next = seq->next;
		free(seq);
		sdp_record_free(rec);
	}
	sdp_close(sess);
	
	if (channel2 != -1) return bluetooth_connect(s, channel2, str);
	
	return ERR_UNKNOWN;
}

static GSM_Error bluetooth_findchannel(GSM_StateMachine *s)
{
	inquiry_info			ii[20];
	uint8_t				count = 0;
	int				i;
	struct search_context 		context;
	GSM_Error			error = ERR_NOTSUPPORTED;

	memset(&context, '\0', sizeof(struct search_context));
	sdp_uuid16_create(&(context.group),PUBLIC_BROWSE_GROUP);

	if (!strcmp(s->CurrentConfig->Device,"/dev/ttyS1")) {
		dbgprintf("Searching for devices\n");
		if (sdp_general_inquiry(ii, 20, 8, &count) < 0) {
			return ERR_UNKNOWN;
		}
	} else {
		count = 1;
		str2ba(s->CurrentConfig->Device,&ii[0].bdaddr);
	}
	for (i=0;i<count;i++) {
		error = bluetooth_checkdevice(s,&ii[i].bdaddr,&context);
		if (error == ERR_NONE) return error;
	}
	return error;
}

#endif

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
	bluetooth_findchannel,
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

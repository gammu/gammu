
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
#  include "win32/ws2bth.h"
#endif

#include "../../gsmcomon.h"
#include "../devfunc.h"
#include "bluetoth.h"

#ifdef WIN32
static GSM_Error bluetooth_connect(GSM_StateMachine *s, int port)
{
	GSM_Device_BlueToothData 	*d = &s->Device.Data.BlueTooth;
    	WSADATA				wsaData;
	SOCKADDR_BTH 			sab;
	int				i;
    
    	WSAStartup(MAKEWORD(1,1), &wsaData);

    	d->hPhone = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
	if (d->hPhone == INVALID_SOCKET) {
		GSM_OSErrorInfo(s, "Socket in bluetooth_open");
		return GE_UNKNOWN;
	}

	memset (&sab, 0, sizeof(sab));
	sab.port 		= port;
	sab.addressFamily 	= AF_BTH;
	sab.btAddr 		= 0;
	for (i=0;i<(int)strlen(s->Config.Device);i++) {
		if (s->Config.Device[i] >='0' && s->Config.Device[i] <='9') {
			sab.btAddr = sab.btAddr * 16;
			sab.btAddr = sab.btAddr + (s->Config.Device[i]-'0');
		}
		if (s->Config.Device[i] >='a' && s->Config.Device[i] <='f') {
			sab.btAddr = sab.btAddr * 16;
			sab.btAddr = sab.btAddr + (s->Config.Device[i]-'a'+10);
		}
		if (s->Config.Device[i] >='A' && s->Config.Device[i] <='F') {
			sab.btAddr = sab.btAddr * 16;
			sab.btAddr = sab.btAddr + (s->Config.Device[i]-'A'+10);
		}
	}
	dprintf("Remote Bluetooth device is %04x%08x\n",
	  		GET_NAP(sab.btAddr), GET_SAP(sab.btAddr));

	if (connect (d->hPhone, (struct sockaddr *)&sab, sizeof(sab)) != 0) {
		i = GetLastError();
		GSM_OSErrorInfo(s, "Connect in bluetooth_open");
		if (i == 10060) return GE_TIMEOUT;
		if (i == 10050) return GE_DEVICEOPENERROR;
		close(d->hPhone);
		return GE_UNKNOWN;
	}

	return GE_NONE;
}
#else
static GSM_Error bluetooth_connect(GSM_StateMachine *s, int port)
{
	GSM_Device_BlueToothData 	*d = &s->Device.Data.BlueTooth;
	struct sockaddr_rc 		laddr, raddr;
	bdaddr_t			bdaddr;
	int 				fd;

	if ((fd = socket(PF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM)) < 0) {
		dprintf("Can't create socket\n");
		return GE_DEVICEOPENERROR;;
	}

	bacpy(&laddr.rc_bdaddr, BDADDR_ANY);
	laddr.rc_family 	= AF_BLUETOOTH;
	laddr.rc_channel 	= 0;

	if (bind(fd, (struct sockaddr *)&laddr, sizeof(laddr)) < 0) {
		dprintf("Can't bind socket\n");
		close(fd);
		return GE_DEVICEOPENERROR;
	}

	str2ba(s->Config.Device, &bdaddr);
	bacpy(&raddr.rc_bdaddr, &bdaddr);
	raddr.rc_family 	= AF_BLUETOOTH;
	raddr.rc_channel 	= port;

	if (connect(fd, (struct sockaddr *)&raddr, sizeof(raddr)) < 0) {
		dprintf("Can't connect\n");
		close(fd);
		return GE_DEVICEOPENERROR;
	}

	d->hPhone = fd;
    	return GE_NONE;
}
#endif

static GSM_Error bluetooth_open (GSM_StateMachine *s)
{
	if (s->ConnectionType == GCT_BLUEPHONET) {
		return bluetooth_connect(s,14);
	} else {
		return bluetooth_connect(s,1);
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

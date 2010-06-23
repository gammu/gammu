
#include "../../gsmstate.h"

#ifdef GSM_ENABLE_BLUETOOTHDEVICE
#ifndef DJGPP
#ifndef WIN32

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#include "../../gsmcomon.h"
#include "unixblue.h"

static GSM_Error bluetooth_close(GSM_StateMachine *s)
{
	GSM_Device_BlueToothData *d = &s->Device.Data.BlueTooth;

	close(d->hPhone); /*FIXME: error checking */

	return GE_NONE;
}

static GSM_Error bluetooth_open (GSM_StateMachine *s)
{
	GSM_Device_BlueToothData 	*d = &s->Device.Data.BlueTooth;
	struct sockaddr_rc 		laddr, raddr;
	bdaddr_t			bdaddr;
	int 				fd;

	if ((fd = socket(PF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM)) < 0) {
		dprintf("Can't create socket\n");
		return GE_DEVICEOPENERROR;;
	}

	laddr.rc_family = AF_BLUETOOTH;
	bacpy(&laddr.rc_bdaddr, BDADDR_ANY);
	laddr.rc_channel = 0;

	if (bind(fd, (struct sockaddr *)&laddr, sizeof(laddr)) < 0) {
		dprintf("Can't bind socket\n");
		close(fd);
		return GE_DEVICEOPENERROR;
	}

	str2ba(s->CFGDevice, &bdaddr);
	raddr.rc_family = AF_BLUETOOTH;
	bacpy(&raddr.rc_bdaddr, &bdaddr);
	raddr.rc_channel = htobs(1);

	if (connect(fd, (struct sockaddr *)&raddr, sizeof(raddr)) < 0) {
		dprintf("Can't connect\n");
		close(fd);
		return GE_DEVICEOPENERROR;
	}

	d->hPhone = fd;

    	return GE_NONE;
}

static int bluetooth_read(GSM_StateMachine *s, void *buf, size_t nbytes)
{
    GSM_Device_BlueToothData 	*d = &s->Device.Data.BlueTooth;
    struct timeval 	  	timeout2;
    fd_set		 	readfds;

    FD_ZERO(&readfds);
    FD_SET(d->hPhone, &readfds);

    timeout2.tv_sec	= 0;
    timeout2.tv_usec	= 0;

    if (select(d->hPhone+1, &readfds, NULL, NULL, &timeout2)) {
	return (read(d->hPhone, buf, nbytes));
    } else {
	return 0;
    }
}

static int bluetooth_write(GSM_StateMachine *s, void *buf, size_t nbytes)
{
    GSM_Device_BlueToothData 	*d = &s->Device.Data.BlueTooth;

    return (write(d->hPhone, buf, nbytes));
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
#endif

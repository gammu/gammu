
/* Have to include wsock32.lib library to MS VC6 project to compile it */

#include "../../gsmstate.h"

#ifdef GSM_ENABLE_IRDADEVICE
#ifndef DJGPP

#ifndef WIN32
#  include <stdlib.h>
#  include <unistd.h>
#  include <stdio.h>
#  include <fcntl.h>
#  include <errno.h>
#  include <string.h>
#  include <sys/time.h>
#  include <sys/poll.h>
#  include <sys/socket.h>
#  include <sys/ioctl.h>
#else
#  include <windows.h>
#  include <io.h>
#endif

#include "../../gsmcomon.h"
#include "../devfunc.h"
#include "irda.h"

static bool irda_discover_device(GSM_StateMachine *state)
{
    GSM_Device_IrdaData 	*d = &state->Device.Data.Irda;
    struct irda_device_list	*list;
    unsigned char		*buf;
    unsigned int		sec;

    int				s, z, len, fd, i;
    GSM_DateTime		Date;
    bool			founddevice = false;

#ifdef WIN32
    WSADATA			wsaData;
    int 			index;
    
    WSAStartup(MAKEWORD(1,1), &wsaData);
#endif

    fd = socket(AF_IRDA, SOCK_STREAM, 0);

    /* 10 = max devices in discover */
    len  = sizeof(struct irda_device_list) + sizeof(struct irda_device_info) * 10;
    buf  = malloc(len);
    list = (struct irda_device_list *)buf;

    /* Trying to find device during 2 seconds */
    for (z=0;z<2;z++) {
	    GSM_GetCurrentDateTime (&Date);
	    sec = Date.Second;
	    while (sec==Date.Second)
	    {
		s = len;
		memset(buf, 0, s);

		if (getsockopt(fd, SOL_IRLMP, IRLMP_ENUMDEVICES, buf, &s) == 0) {
		    for (i = 0; i < (int)list->numDevice; i++) {
			dprintf("Irda: found device \"%s\" (address %x) - ",list->Device[i].irdaDeviceName,list->Device[i].irdaDeviceID);
			if (strcmp(GetModelData(NULL,NULL,list->Device[i].irdaDeviceName)->number,"") != 0) {
				founddevice = true;
				/* Model AUTO */
				if (state->Config.Model[0]==0) strcpy(state->Phone.Data.Model,GetModelData(NULL,NULL,list->Device[i].irdaDeviceName)->number);
				state->Phone.Data.ModelInfo = GetModelData(NULL,state->Phone.Data.Model,NULL);
			}
			if (founddevice) {				
			    dprintf("correct\n");
#ifdef WIN32
			    for(index=0; index <= 3; index++)
				d->peer.irdaDeviceID[index] = list->Device[i].irdaDeviceID[index];
#else
		    	    d->peer.irdaDeviceID = list->Device[i].irdaDeviceID;
#endif
			    break;
			}
			dprintf("\n");
		    }
		}
		if (founddevice) break;
		my_sleep(10);
		GSM_GetCurrentDateTime(&Date);
	    }
            if (founddevice) break;
    }
    free(buf);
    close(fd);

    return founddevice;
}

static GSM_Error irda_open (GSM_StateMachine *s)
{
    GSM_Device_IrdaData *d = &s->Device.Data.Irda;
    int			fd = -1;

    /* discover the devices */
    if (irda_discover_device(s)==false) return GE_TIMEOUT;

    /* Create socket */
    fd = socket(AF_IRDA, SOCK_STREAM, 0);

    d->peer.irdaAddressFamily 	= AF_IRDA;
#ifndef WIN32
    d->peer.sir_lsap_sel 	= LSAP_ANY;
#endif
    strcpy(d->peer.irdaServiceName, "Nokia:PhoNet");

    /* Connect to service "Nokia:PhoNet" */
    if (connect(fd, (struct sockaddr *)&d->peer, sizeof(d->peer))) {
	close(fd);
	return GE_DEVICEOPENERROR;
    }

    d->hPhone=fd;

    return GE_NONE;
}

static int irda_read(GSM_StateMachine *s, void *buf, size_t nbytes)
{
	return socket_read(s, buf, nbytes, s->Device.Data.Irda.hPhone);
}

#ifdef WIN32
static int irda_write(GSM_StateMachine *s, unsigned char *buf, size_t nbytes)
#else
static int irda_write(GSM_StateMachine *s, void *buf, size_t nbytes)
#endif
{
	return socket_write(s, buf, nbytes, s->Device.Data.Irda.hPhone);
}

static GSM_Error irda_close(GSM_StateMachine *s)
{
	return socket_close(s, s->Device.Data.Irda.hPhone);
}

GSM_Device_Functions IrdaDevice = {
	irda_open,
	irda_close,
	NONEFUNCTION,
	NONEFUNCTION,
	NONEFUNCTION,
	irda_read,
	irda_write
};

#endif
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

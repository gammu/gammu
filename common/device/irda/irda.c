/* (c) 2001-2004 by Marcin Wiacek */
/* based on some work from Ralf Thelen and MyGnokii (www.mwiacek.com) */
/* based on some work from MSDN */
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

/* You have to include wsock32.lib library to MS VC project to compile it */

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
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  include <winsock2.h>
#  include <io.h>
#endif

#include "../../gsmcomon.h"
#include "../../gsmphones.h"
#include "../devfunc.h"
#include "irda.h"

static bool irda_discover_device(GSM_StateMachine *state, int *fd)
{
	GSM_Device_IrdaData 	*d = &state->Device.Data.Irda;
	struct irda_device_list	*list;
    	unsigned char		*buf;
    	unsigned int		sec;
    	int			s, z, len, i;
    	GSM_DateTime		Date;
    	bool			founddevice = false;
	int deviceid;
#ifdef WIN32
	int			index;
#endif

   	(*fd) = socket(AF_IRDA, SOCK_STREAM, 0);

    	/* can handle maximally 10 devices during discovering */
    	len  = sizeof(struct irda_device_list) + sizeof(struct irda_device_info) * 10;
    	buf  = malloc(len);
    	list = (struct irda_device_list *)buf;

    	/* Trying to find device during 2 seconds */
    	for (z=0;z<2;z++) {
		GSM_GetCurrentDateTime (&Date);
	    	sec = Date.Second;
	    	while (sec==Date.Second) {
			s = len;
			memset(buf, 0, s);

			if (getsockopt(*fd, SOL_IRLMP, IRLMP_ENUMDEVICES, buf, &s) == 0) {
		    		for (i = 0; i < (int)list->numDevice; i++) {

#ifdef WIN32
					deviceid = (list->Device[i].irdaDeviceID[0] << 24) |
							(list->Device[i].irdaDeviceID[1] << 16) |
							(list->Device[i].irdaDeviceID[2] << 8) |
							(list->Device[i].irdaDeviceID[3]);
#else
					deviceid = list->Device[i].irdaDeviceID;
#endif
					smprintf(state, "Irda: found device \"%s\" (address %d) - ",
							list->Device[i].irdaDeviceName,
							deviceid
							);
					if (strcmp(GetModelData(state, NULL,NULL,list->Device[i].irdaDeviceName)->number,"") != 0) {
						founddevice = true;
						/* Model AUTO */
						if (state->CurrentConfig->Model[0]==0)
							strcpy(state->Phone.Data.Model,GetModelData(state, NULL,NULL,list->Device[i].irdaDeviceName)->number);
						state->Phone.Data.ModelInfo = GetModelData(state, NULL,state->Phone.Data.Model,NULL);
					}
					if (founddevice) {
			    			smprintf(state, "correct\n");
#ifdef WIN32
			    			for(index=0; index <= 3; index++)
							d->peer.irdaDeviceID[index] = list->Device[i].irdaDeviceID[index];
#else
		    	    			d->peer.irdaDeviceID = list->Device[i].irdaDeviceID;
#endif
			   			break;
					}
					smprintf(state, "\n");
		    		}
			}
			if (founddevice) break;
			usleep(10000);
			GSM_GetCurrentDateTime(&Date);
	    	}
		if (founddevice) break;
    	}
    	free(buf);
	if (!founddevice) {
#ifdef WIN32
	    	closesocket(*fd);
#else
	    	close(*fd);
#endif
	}
    	return founddevice;
}

static GSM_Error irda_open (GSM_StateMachine *s)
{
    	GSM_Device_IrdaData 	*d = &s->Device.Data.Irda;
    	int			fd = -1;
	bool			failed;
#ifdef WIN32
    	int 			Enable9WireMode = 1;
    	WSADATA			wsaData;
	/* BCC comes with broken MAKEWORD, which emmits warnings */
#ifdef __BORLANDC__
#    pragma warn -8084
#endif
    	WSAStartup(MAKEWORD(1,1), &wsaData);
#ifdef __BORLANDC__
#    pragma warn +8084
#endif
#else
    	if (s->ConnectionType == GCT_IRDAAT) return ERR_SOURCENOTAVAILABLE;
#endif

    	/* discovering devices */
    	if (irda_discover_device(s,&fd)==false) return ERR_TIMEOUT;

    	d->peer.irdaAddressFamily 	= AF_IRDA;
#ifndef WIN32
    	d->peer.sir_lsap_sel 		= LSAP_ANY;
#endif
    	switch (s->ConnectionType) {
    	case GCT_IRDAGNAPBUS:
    	case GCT_IRDAAT:
    		strcpy(d->peer.irdaServiceName, "IrDA:IrCOMM");
#ifdef WIN32
		if (setsockopt(fd, SOL_IRLMP, IRLMP_9WIRE_MODE, (const char *) &Enable9WireMode,
               		sizeof(int))==SOCKET_ERROR) return ERR_UNKNOWN;
#endif
		break;
    	case GCT_IRDAPHONET:
    		strcpy(d->peer.irdaServiceName, "Nokia:PhoNet");
		break;
    	case GCT_IRDAOBEX:
    		strcpy(d->peer.irdaServiceName, "OBEX");
		break;
    	default:
		return ERR_UNKNOWN;
    	}

    	/* Connecting to service */
    	if (connect(fd, (struct sockaddr *)&d->peer, sizeof(d->peer))) {
		smprintf(s, "Can't connect to service %s\n",d->peer.irdaServiceName);
		failed = true;
		/* Try alternatives if we failed */
		if (s->ConnectionType == GCT_IRDAOBEX) {
			smprintf(s, "Trying alternate config: IrDA:OBEX\n");
			strcpy(d->peer.irdaServiceName, "IrDA:OBEX");
			if (connect(fd, (struct sockaddr *)&d->peer, sizeof(d->peer))) {
				smprintf(s, "Can't connect to service %s\n",d->peer.irdaServiceName);
				smprintf(s, "Trying alternate config: OBEX:IrXfer\n");
				strcpy(d->peer.irdaServiceName, "OBEX:IrXfer");
				if (!connect(fd, (struct sockaddr *)&d->peer, sizeof(d->peer))) {
					failed = false;
				}
			} else {
				failed = false;
			}
		}
		if (failed) {
			close(fd);
			return ERR_NOTSUPPORTED;
		}
    	}

    	d->hPhone=fd;

    	return ERR_NONE;
}

static int irda_read(GSM_StateMachine *s, void *buf, size_t nbytes)
{
	return socket_read(s, buf, nbytes, s->Device.Data.Irda.hPhone);
}

static int irda_write(GSM_StateMachine *s, const void *buf, size_t nbytes)
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

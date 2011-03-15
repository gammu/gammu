/* Some source from Gnokii
 * (C) 1999-2000 Hugh Blemings & Pavel Janik ml. (C) 2001-2004 Pawel Kot 
 * GNU GPL version 2 or later
 */
/* Some source from Minicom 
 * (C) 1991,1992,1993,1994,1995,1996 by Miquel van Smoorenburg
 * GNU GPL version 2
 */
#include <stdafx.h>

#include <string.h>
#ifdef WIN32
#  include <io.h>
#else
#  include <errno.h>
#  include <signal.h>
#endif
#include "devicedef.h"
#include <Winsock2.h>
#include "commfun.h"

#ifdef BLUETOOTH_RF_SEARCHING

GSM_Error bluetooth_checkservicename(GSM_DeviceData *s, char *name)
{
        if (s->ConnectionType == GCT_BLUEPHONET && strstr(name,"Nokia PC Suite")!=NULL) return ERR_NONE;
	if (s->ConnectionType == GCT_BLUEOBEX 	&& strstr(name,"OBEX")		!=NULL) return ERR_NONE;
        if (s->ConnectionType == GCT_BLUEAT 	&& strstr(name,"COM 1")		!=NULL) return ERR_NONE;
        return ERR_UNKNOWN;
}

#endif


int socket_read(GSM_DeviceData *s, void *buf, size_t nbytes, int hPhone)
{
	fd_set 		readfds;
	struct timeval 	timer;

	FD_ZERO(&readfds);
	FD_SET(hPhone, &readfds);
	memset(&timer,0,sizeof(timer));
	if (select(0, &readfds, NULL, NULL, &timer) != 0) {
		return(recv(hPhone, (char *)buf, nbytes, 0));
	}
	return 0;
}

int socket_write(GSM_DeviceData *s, unsigned char *buf, size_t nbytes, int hPhone,Debug_Info *debugInfo)
{
	int		ret;
	size_t		actual = 0;

	do {
		ret = send(hPhone, (char *)buf, nbytes - actual, 0);
        	if (ret < 0) {
            		if (actual != nbytes) GSM_OSErrorInfo(debugInfo,"socket_write");
            		return actual;
        	}
		actual 	+= ret;
		buf 	+= ret;
	} while (actual < nbytes);

	return actual;
}

GSM_Error socket_close(GSM_DeviceData *s, int hPhone)
{
	shutdown(hPhone, 0);
#ifdef WIN32
	closesocket(hPhone); /*FIXME: error checking */
#else
	close(hPhone); /*FIXME: error checking */
#endif
	return ERR_NONE;
}


/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

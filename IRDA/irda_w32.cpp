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

#include "stdafx.h"

#include <windows.h>
#include <io.h>
#include "devfunc.h"
#include "irda_w32.h"

bool irda_discover_device(GSM_DeviceData *state)
{
	struct irda_device_list	*list;
    	char		*buf;
    	unsigned int		sec;
    	int			s, z, len,  i;
    	GSM_DateTime		Date;
    	bool			founddevice = false;
		SOCKET fd;
#ifdef WIN32
	int			index;
#endif

/* v1.0.3.5
Notes for IrDA Sockets
The Af_irda.h header file must be explicitly included. 
Only SOCK_STREAM is supported; the SOCK_DGRAM type is not supported by IrDA. 
The protocol parameter is always set to 0 for IrDA. 
*/
   	fd = socket(AF_IRDA, SOCK_STREAM, 0);

	/* added by mingfa , v1.0.3.5
      if (fd ==INVALID_SOCKET)
      {
        return founddevice;
      }
	*/
    	/* can handle maximally 10 devices during discovering */
    	len  = sizeof(struct irda_device_list) + sizeof(struct irda_device_info) * 10;
    	buf  =(char *) malloc(len);
    	list = (struct irda_device_list *)buf;

    	/* Trying to find device during 2 seconds */
    	for (z=0;z<2;z++) {
		GSM_GetCurrentDateTime (&Date);
	    	sec = Date.Second;
	    	while (sec==Date.Second) {
			s = len;
			memset(buf, 0, s);

			if (getsockopt(fd, SOL_IRLMP, IRLMP_ENUMDEVICES, buf, &s) == 0) {
		    		for (i = 0; i < (int)list->numDevice; i++) {
					dbgprintf("Irda: found device \"%s\" (address %x) - ",list->Device[i].irdaDeviceName,list->Device[i].irdaDeviceID);
			/*		if (strcmp(GetModelData(NULL,NULL,list->Device[i].irdaDeviceName)->number,"") != 0) {
						founddevice = true;
						/* Model AUTO */
					/**	if (state->CurrentConfig->Model[0]==0) strcpy(state->Phone.Data.Model,GetModelData(NULL,NULL,list->Device[i].irdaDeviceName)->number);
						state->Phone.Data.ModelInfo = GetModelData(NULL,state->Phone.Data.Model,NULL);
					}*/
					if(strcmp(state->irdamodel,list->Device[i].irdaDeviceName)== 0)
					{
						founddevice = true;
					}
					if (founddevice) {				
			    			dbgprintf("correct\n");
#ifdef WIN32
			    			for(index=0; index <= 3; index++)
							state->peer.irdaDeviceID[index] = list->Device[i].irdaDeviceID[index];
#else
		    	    			state->peer.irdaDeviceID = list->Device[i].irdaDeviceID;
#endif
			   			break;
					}
					dbgprintf("\n");
		    		}
			}
			if (founddevice) break;
			my_sleep(10);
			GSM_GetCurrentDateTime(&Date);
	    	}
		if (founddevice) break;
    	}
    	free(buf);


    //	close(fd); // v1.0.3.5 , 0307 , marked by mingfa
	// v1.0.3.5 , 0307, added by mingfa
	shutdown(fd, 0); // v1.0.3.5 , 0307 , change 0 to SD_BOTH by mingfa
#ifdef WIN32
	closesocket(fd); /*FIXME: error checking */
#else
	close(fd); /*FIXME: error checking */
#endif
    // mingfa--
// WSACleanup();

   	return founddevice;
}

GSM_Error irda_open (GSM_DeviceData *s,Debug_Info *debugInfo)
{
    	SOCKET			fd = -1;
		// v1.0.3.5 , 0307 , added by mingfa for irda version
		int err;
#ifdef WIN32
    	int 			Enable9WireMode = 1;
    	WSADATA			wsaData;
    
		// v1.0.3.5 , 0307 , modified by mingfa
		//The WSAStartup function must be the first Windows Sockets function called by an application or DLL
    	err = WSAStartup(MAKEWORD(1,1), &wsaData);
		if ( err != 0 )
		  return ERR_DEVICEOPENERROR; 
		// mingfa--
#else
    	if (s->ConnectionType == GCT_IRDAAT) return ERR_SOURCENOTAVAILABLE;
#endif

    	/* discovering devices */
   	if (irda_discover_device(s)==false) return ERR_TIMEOUT;

	//return ERR_TIMEOUT;// for test

    	/* Creating socket */
    //	err = WSAStartup(MAKEWORD(1,1), &wsaData); // for test

    	fd = socket(AF_IRDA, SOCK_STREAM, 0);

	/* added by mingfa , v1.0.3.5
      if (fd ==INVALID_SOCKET)
      {
        return ERR_DEVICEOPENERROR;
      }
	*/

    	s->peer.irdaAddressFamily 	= AF_IRDA;
#ifndef WIN32
    	s->peer.sir_lsap_sel 		= LSAP_ANY;
#endif
    	switch (s->ConnectionType) {
    	case GCT_IRDAAT:
    		strcpy(s->peer.irdaServiceName, "IrDA:IrCOMM");

#ifdef WIN32
		if (setsockopt(fd, SOL_IRLMP, IRLMP_9WIRE_MODE, (const char *) &Enable9WireMode,
               		sizeof(int))==SOCKET_ERROR) return ERR_UNKNOWN;
#endif
		break;
    	case GCT_IRDAPHONET:
    		strcpy(s->peer.irdaServiceName, "Nokia:PhoNet");
		break;
    	case GCT_IRDAOBEX:
		/* IrDA:OBEX not supported by N3650 */
//    		strcpy(d->peer.irdaServiceName, "IrDA:OBEX");

    		strcpy(s->peer.irdaServiceName, "OBEX");

		/* Alternative server is "OBEX:IrXfer" */
		break;
    	default:
		return ERR_UNKNOWN;
    	}

    	/* Connecting to service */
    	if (connect(fd, (struct sockaddr *)&s->peer, sizeof(s->peer))) {
		dbgprintf("Can't connect to service %s\n",s->peer.irdaServiceName);
    //	close(fd); // v1.0.3.5 , 0307 , marked by mingfa
	    // v1.0.3.5 , 0307, added by mingfa
	    shutdown(fd, 0); // v1.0.3.5 , 0307 , change 0 to SD_BOTH by mingfa
#ifdef WIN32
	    closesocket(fd); /*FIXME: error checking */
#else
	    close(fd); /*FIXME: error checking */
#endif
    // mingfa--

		return ERR_NOTSUPPORTED;
    	}

    	s->hSocketPhone=fd;

    	return ERR_NONE;
}



GSM_Error irda_close(GSM_DeviceData *s,Debug_Info *debugInfo)
{

//	return socket_close(s, s->Device.Data.Irda.hPhone);
    socket_close(s, s->hSocketPhone);

#ifdef WIN32
	// v1.0.3.5 , 0307 , modified by mingfa
	//There must be a call to WSACleanup for every successful call to WSAStartup made by a task
    WSACleanup();
#endif

	return ERR_NONE;
}


/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

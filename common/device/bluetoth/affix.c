/* (c) 2004 by Marcin Wiacek */

#include "../../gsmstate.h"

#ifdef GSM_ENABLE_BLUETOOTHDEVICE
#ifdef GSM_ENABLE_AFFIX

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "../../gsmcomon.h"
#include "../devfunc.h"
#include "bluetoth.h"

GSM_Error bluetooth_connect(GSM_StateMachine *s, int port, char *device)
{
	d->hPhone = fd;
    	return ERR_NONE;
}

#ifdef BLUETOOTH_RF_SEARCHING

static GSM_Error bluetooth_checkdevice(GSM_StateMachine *s, bdaddr_t *bdaddr, struct search_context *context)
{
}

GSM_Error bluetooth_findchannel(GSM_StateMachine *s)
{
	return error;
}

#endif
#endif
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

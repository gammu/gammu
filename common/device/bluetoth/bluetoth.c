/* (c) 2003-2004 by Marcin Wiacek and Marcel Holtmann and others */

#include "../../gsmstate.h"

#ifdef GSM_ENABLE_BLUETOOTHDEVICE
#ifndef DJGPP

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "../../misc/coding/coding.h"
#include "../../gsmcomon.h"
#include "../devfunc.h"
#include "bluetoth.h"

#ifdef GSM_ENABLE_BLUEZ
#  include "bluez.h"
#endif
#ifdef GSM_ENABLE_AFFIX
#  include "affix.h"
#endif
#ifdef WIN32
#  include "blue_w32.h"
#endif

GSM_Error bluetooth_findrfchannel(GSM_StateMachine *s)
{
	GSM_Error error;

#ifdef BLUETOOTH_RF_SEARCHING
	if (!mystrncasecmp(s->CurrentConfig->Connection, "bluerf", 6)) return bluetooth_findchannel(s);
#endif
	
	switch (s->ConnectionType) {
	case GCT_BLUEAT:
		return bluetooth_connect(s,1,s->CurrentConfig->Device);
	case GCT_BLUEOBEX:
		return bluetooth_connect(s,9,s->CurrentConfig->Device);
	case GCT_BLUEPHONET:
		error = bluetooth_connect(s,14,s->CurrentConfig->Device); //older Series 40 - 8910, 6310
		if (error == ERR_NONE) return error;
		return bluetooth_connect(s,15,s->CurrentConfig->Device); //new Series 40 - 6310i, 6230
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
	bluetooth_findrfchannel,
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

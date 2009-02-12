/* (c) 2003-2004 by Marcin Wiacek and Marcel Holtmann and others */

#include "../../gsmstate.h"

#ifdef GSM_ENABLE_BLUETOOTHDEVICE
#ifndef DJGPP

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

//#include "../../misc/coding/coding.h"
#include "../../gsmcomon.h"
#include "../devfunc.h"
#include "bluetoth.h"

#include "../helper/string.h"

#ifdef BLUEZ_FOUND
#  include "bluez.h"
#endif
#ifdef FBSD_BLUE_FOUND
#  include "blue_fbsd.h"
#endif
#ifdef OSX_BLUE_FOUND
#  include "blue_osx.h"
#endif
#ifdef WIN32
#  include "blue_w32.h"
#endif

GSM_Error bluetooth_findrfchannel(GSM_StateMachine *s)
{
	GSM_Error error;
	char *device;
	char *channel;
	int  channel_id = 0;

	/* Temporary string */
	device = strdup(s->CurrentConfig->Device);
	if (device == NULL) {
		return ERR_MOREMEMORY;
	}

	/* Does the string contain channel information? */
	channel = strchr(device, '/');
	if (channel == NULL &&
			strncasecmp(s->CurrentConfig->Connection, "bluerf", 6) != 0) {
		free(device);
#ifdef BLUETOOTH_RF_SEARCHING
		return bluetooth_findchannel(s);
#else
		return ERR_SOURCENOTAVAILABLE;
#endif
	}

	/* Default channel settings */
	switch (s->ConnectionType) {
		case GCT_BLUEAT:
			channel_id = 1;
			break;
		case GCT_BLUEOBEX:
			channel_id = 9;
			break;
		case GCT_BLUEGNAPBUS:
			channel_id = 14;
			break;
		case GCT_BLUEFBUS2: /* fixme */
		case GCT_BLUEPHONET:
			channel_id = 15;
			break;
		default:
			channel_id = 0;
			break;
	}

	/* Parse channel from configuration */
	if (channel != NULL) {
		/* Zero terminate our string */
		*channel = 0;
		/* Grab channel number */
		channel++;
		channel_id = atoi(channel);
	} else {
		/* Notify user about hard wired default */
		smprintf(s, "Using hard coded bluetooth channel %d.\n",
				channel_id);
	}

	/* Check for zero */
	if (channel_id == 0) {
		smprintf(s, "Please configure bluetooth channel!\n");
		error = ERR_SPECIFYCHANNEL;
		goto done;
	}


	/* Connect to phone */
	error = bluetooth_connect(s, channel_id, device);
	if (error == ERR_NONE) goto done;

	/* Hack for Nokia phones, they moved channel from 14 to 15, so
	 * we want to try both.
	 *
	 * Older Series 40 (eg. 8910 and 6310) use channel 14
	 * Newer Series 40 (eg. 6230 and 6310i) use channel 15
	 * */
	if (((s->ConnectionType == GCT_BLUEPHONET) ||
			(s->ConnectionType == GCT_BLUEFBUS2)) &&
			(channel_id == 15)) {
		channel_id = 14;
		error = bluetooth_connect(s, channel_id, device);
	}

done:
	free(device);
	return error;
}

#ifndef OSX_BLUE_FOUND
int bluetooth_read(GSM_StateMachine *s, void *buf, size_t nbytes)
{
	return socket_read(s, buf, nbytes, s->Device.Data.BlueTooth.hPhone);
}

int bluetooth_write(GSM_StateMachine *s, const void *buf, size_t nbytes)
{
	return socket_write(s, buf, nbytes, s->Device.Data.BlueTooth.hPhone);
}

GSM_Error bluetooth_close(GSM_StateMachine *s)
{
	return socket_close(s, s->Device.Data.BlueTooth.hPhone);
}
#endif

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

/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Copyright (c) 2011 Michal Cihar <michal@cihar.com>
 */


#include "../../gsmstate.h"

#if defined(GSM_ENABLE_S60)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../gsmcomon.h"
#include "s60.h"
#include "s60-ids.h"

static GSM_Error S60_WriteMessage (GSM_StateMachine *s, unsigned const char *MsgBuffer,
				    int MsgLength, unsigned char MsgType)
{
	unsigned char	*buffer=NULL;
	int pos, sent, length;

	/* Debugging */
	GSM_DumpMessageLevel3(s, MsgBuffer, MsgLength, MsgType);
	GSM_DumpMessageLevel2(s, MsgBuffer, MsgLength, MsgType);

	/* Allocate buffer for composing message */
	buffer = (unsigned char *)malloc(MIN(MAX_LENGTH, MsgLength) + 10);
	if (buffer == NULL) {
		return ERR_MOREMEMORY;
	}

	/* Send message parts */
	for (pos = 0; MsgLength - pos > MAX_LENGTH; pos += MAX_LENGTH) {
		buffer[0] = NUM_PARTIAL_MESSAGE;
		memcpy(buffer + 1, MsgBuffer + pos, MAX_LENGTH);
		length = MAX_LENGTH + 1;
		sent = s->Device.Functions->WriteDevice(s, buffer, length);
		if (sent != length) {
			return ERR_DEVICEWRITEERROR;
		}
	}

	/* Send final message */
	buffer[0] = MsgType;
	length = MsgLength - pos;
	memcpy(buffer + 1, MsgBuffer + pos, length);
	sent = s->Device.Functions->WriteDevice(s, buffer, length);
	if (sent != length) {
		return ERR_DEVICEWRITEERROR;
	}

	return ERR_NONE;
}

static GSM_Error S60_Receive(GSM_StateMachine *s, unsigned char *data, size_t length)
{
	return ERR_NONE;
}

static GSM_Error S60_Initialise(GSM_StateMachine *s)
{
	GSM_Protocol_S60Data *d = &s->Protocol.Data.S60;

	d->Msg.BufferUsed	= 0;
	d->Msg.Buffer 		= NULL;
	d->Msg.Length		= 0;

	return ERR_NONE;
}

static GSM_Error S60_Terminate(GSM_StateMachine *s)
{
	free(s->Protocol.Data.S60.Msg.Buffer);
	s->Protocol.Data.S60.Msg.Buffer = NULL;

	return ERR_NONE;
}

GSM_Protocol_Functions S60Protocol = {
	S60_WriteMessage,
	NULL,
	S60_Receive,
	S60_Initialise,
	S60_Terminate
};

#endif


/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

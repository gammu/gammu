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
	int pos, sent, length, buflen, bufpos;
	GSM_Error error;

	/* No type */
	if (MsgType == 0) {
		return ERR_NONE;
	}

	/* Debugging */
	GSM_DumpMessageLevel3(s, MsgBuffer, MsgLength, MsgType);
	GSM_DumpMessageLevel2(s, MsgBuffer, MsgLength, MsgType);

	/* Allocate buffer for composing message */
	buflen = MIN(MAX_LENGTH, MsgLength) + 10;
	buffer = (unsigned char *)malloc(buflen);
	if (buffer == NULL) {
		return ERR_MOREMEMORY;
	}

	/* Send message parts */
	for (pos = 0; MsgLength - pos > MAX_LENGTH; pos += MAX_LENGTH) {
		error = S60_WriteMessage(s, MsgBuffer + pos, MAX_LENGTH, NUM_PARTIAL_MESSAGE);
		if (error != ERR_NONE) {
			return ERR_DEVICEWRITEERROR;
		}
	}

	/* Send final message */
	buffer[0] = MsgType;
	length = MsgLength - pos;

	bufpos = snprintf(buffer, buflen, "%d%c", MsgType, NUM_END_HEADER);
	memcpy(buffer + bufpos, MsgBuffer + pos, length);
	buffer[bufpos + length] = '\n';
	length += bufpos + 1;
	sent = s->Device.Functions->WriteDevice(s, buffer, length);
	if (sent != length) {
		return ERR_DEVICEWRITEERROR;
	}

	return ERR_NONE;
}

static GSM_Error S60_Receive(GSM_StateMachine *s, unsigned char *data, size_t length)
{
	GSM_Protocol_S60Data *d = &s->Protocol.Data.S60;
	unsigned char *pos;
	size_t payload;

	if (length == 0) {
		return ERR_NONE;
	}

	/* Allocate buffer */
	if (d->Msg.BufferUsed < d->Msg.Length + length) {
		d->Msg.BufferUsed = d->Msg.Length + length;
		d->Msg.Buffer = (unsigned char *)realloc(d->Msg.Buffer, d->Msg.BufferUsed);
		if (d->Msg.Buffer == NULL) {
			return ERR_MOREMEMORY;
		}
	}

	/* Parse message type */
	d->Msg.Type = atoi(data);
	pos = strchr(data, NUM_END_HEADER);
	if (pos == NULL) {
		smprintf(s, "Can not find payload in packet!\n");
		return ERR_BUG;
	}
	if (data[length - 1] != NUM_END_TEXT) {
		smprintf(s, "Can not find end of payload in packet!\n");
		return ERR_BUG;
	}
	/* Skip header end */
	pos++;
	payload = length - (pos - data) - 1;

	/* Store data */
	memcpy(d->Msg.Buffer + d->Msg.Length, pos, payload);
	d->Msg.Length += payload;

	/* If it was message part, wait for other parts */
	if (d->Msg.Type == NUM_PARTIAL_MESSAGE) {
		return ERR_NONE;
	}

	/* We've got data to process */
	s->Phone.Data.RequestMsg = &d->Msg;
	s->Phone.Data.DispatchError = s->Phone.Functions->DispatchMessage(s);

	/* Reset message length */
	d->Msg.Length = 0;

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

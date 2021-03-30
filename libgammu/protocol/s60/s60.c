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

static GSM_Error S60_WriteMessage (GSM_StateMachine *s, const unsigned char *MsgBuffer,
				    size_t MsgLength, int MsgType)
{
	unsigned char	*buffer=NULL;
	int pos, sent, length, buflen, bufpos;
	GSM_Error error;

	/* No type */
	if (MsgType == 0) {
		return ERR_NONE;
	}

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
			free(buffer);
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

	/* Debugging */
	GSM_DumpMessageBinary(s, buffer, length, MsgType);
	GSM_DumpMessageText(s, buffer, length, MsgType);

	sent = s->Device.Functions->WriteDevice(s, buffer, length);
	free(buffer);
	if (sent != length) {
		return ERR_DEVICEWRITEERROR;
	}

	return ERR_NONE;
}

static GSM_Error S60_StateMachine(GSM_StateMachine *s, unsigned char rxchar)
{
	GSM_Protocol_S60Data *d = &s->Protocol.Data.S60;

	/* Did we complete part of packet? */
	switch (d->State) {
		case S60_Header:
			if (rxchar == NUM_END_HEADER) {
				d->Msg.Type = atoi(d->idbuffer);
				d->State = S60_Data;
				d->idpos = 0;
			} else {
				d->idbuffer[d->idpos++] = rxchar;
				d->idbuffer[d->idpos] = 0;
			}
			break;
		case S60_Data:
			if (rxchar == NUM_END_TEXT) {
				d->State = S60_Header;
				/* Should we wait for other parts? */
				if (d->Msg.Type == NUM_PARTIAL_MESSAGE) {
					return ERR_NONE;
				}

				/* We've got data to process */
				s->Phone.Data.RequestMsg = &d->Msg;
				s->Phone.Data.DispatchError = s->Phone.Functions->DispatchMessage(s);

				/* Reset message length */
				d->Msg.Length = 0;
			} else {
				/* Allocate buffer */
				if (d->Msg.BufferUsed < d->Msg.Length + 2) {
					d->Msg.BufferUsed = d->Msg.Length + 2;
					d->Msg.Buffer = (unsigned char *)realloc(d->Msg.Buffer, d->Msg.BufferUsed);
					if (d->Msg.Buffer == NULL) {
						return ERR_MOREMEMORY;
					}
				}

				/* Store received byte */
				d->Msg.Buffer[d->Msg.Length++] = rxchar;
				d->Msg.Buffer[d->Msg.Length] = 0;
			}
			break;
	}

	return ERR_NONE;
}

static GSM_Error S60_Initialise(GSM_StateMachine *s)
{
	GSM_Protocol_S60Data *d = &s->Protocol.Data.S60;

	d->Msg.BufferUsed	= 0;
	d->Msg.Buffer 		= NULL;
	d->Msg.Length		= 0;
	d->State = S60_Header;
	d->idpos = 0;

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
	S60_StateMachine,
	S60_Initialise,
	S60_Terminate
};

#endif


/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

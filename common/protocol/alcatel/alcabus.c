/* (c) 2002-2003 by Michal Cihar
 *
 * Low level functions for communication with Alcatel One Touch phones.
 *
 * This code implements the protocol used for synchronisation with PC.
 */
#include "../../gsmstate.h"

#if defined(GSM_ENABLE_ALCABUS)

#include <stdio.h>
#include <string.h>

#include "../../gsmcomon.h"
#include "alcabus.h"

static GSM_Error ALCABUS_WriteMessage (GSM_StateMachine *s, unsigned const char *data, int len, unsigned char type)
{
	GSM_Protocol_ALCABUSData 	*d = &s->Protocol.Data.ALCABUS;
	unsigned char	 		buffer[1024];
	int				size = 0;
	int				sent = 0;
	int 				i = 0, checksum = 0;

	if ((type == 0) && (len == 0)) return ERR_NONE;

	buffer[0] = ALCATEL_HEADER;
	buffer[1] = type;
	switch (type) {
		case ALCATEL_CONNECT:
			buffer[2] 	= 0x0A;
			buffer[3] 	= 0x04;
			buffer[4] 	= 0x00;
			size 		= 5;
			d->next_frame 	= ALCATEL_CONNECT_ACK;
			d->busy 	= true;
			break;
		case ALCATEL_DISCONNECT:
			size 		= 2;
			d->next_frame 	= ALCATEL_DISCONNECT_ACK;
			d->busy 	= true;
			break;
		case ALCATEL_DATA:
			buffer[2] = d->out_counter;

			/* Increase outgoing packet counter */
			if (d->out_counter == ALCATEL_MAX_COUNTER) d->out_counter = 0;
			else d->out_counter++;

			buffer[3] 	= '\0';
			buffer[4] 	= len;
			memcpy(buffer+5, data, len);
			size 		= 5 + len;
			d->next_frame 	= ALCATEL_ACK;
			d->busy 	= true;
			break;
		case ALCATEL_ACK:
			buffer[2] = d->in_counter;
			if (d->in_counter == 0) d->in_counter = 1;
			size 		= 3;
			d->next_frame 	= ALCATEL_DATA;
			break;
		default:
			/* In fact, other types probably can came just from mobile... */
			smprintf(s,"WARNING: Wanted to send some unknown packet (%02X)\n", type);
			return ERR_NOTIMPLEMENTED;
	}

	/* Calculate packet checksum */
	for (i=0; i<size; i++) checksum ^= buffer[i];

	buffer[size] = checksum;
	size ++;

	GSM_DumpMessageLevel2(s, buffer, size, type);
	GSM_DumpMessageLevel3(s, buffer, size, type);
	while (sent != size ) {
		if ((i = s->Device.Functions->WriteDevice(s,buffer + sent, size - sent)) == 0) {
			return ERR_DEVICEWRITEERROR;
		}
		sent += i;
	}

	if (type == ALCATEL_CONNECT || type == ALCATEL_DISCONNECT) {
		/* For connect and disconnect we need a bit larger delay */
		while (d->busy) {
			GSM_ReadDevice(s,true);
			usleep(1000);
			i++;
			if (i == 10) return ERR_TIMEOUT;
		}
	}
	return ERR_NONE;
}

static GSM_Error ALCABUS_StateMachine(GSM_StateMachine *s, unsigned char rx_char)
{
	GSM_Protocol_ALCABUSData 	*d = &s->Protocol.Data.ALCABUS;
	size_t				i;
	int				checksum = 0;

	if (d->Msg.BufferUsed < d->Msg.Length + 1) {
		d->Msg.BufferUsed	= d->Msg.Length + 1;
		d->Msg.Buffer 	= (unsigned char *)realloc(d->Msg.Buffer,d->Msg.BufferUsed);
	}

	/* Check for header */
	if ((d->Msg.Length == 0) && (rx_char != ALCATEL_HEADER)) {
		smprintf(s,"WARNING: Expecting alcatel header (%02X) but got (%02X)\n", ALCATEL_HEADER, rx_char);
		return ERR_UNKNOWNRESPONSE;
	/* Check for packet type */
	} else if (d->Msg.Length == 1){
		d->Msg.Type = rx_char;
		/* Was it unexpected packet? */
		if ((rx_char != d->next_frame) && (rx_char != ALCATEL_CONTROL)) {
				smprintf(s,"WARNING: Expecting alcatel packet type (%02X) but got (%02X)\n", d->next_frame, rx_char);
		}
		/* Determine packet size */
		switch (rx_char) {
			case ALCATEL_ACK:
				d->expected_size = 4;
				break;
			case ALCATEL_DATA:
			/* Packet length is in it's header */
				d->expected_size = -1;
				break;
			case ALCATEL_CONTROL:
				d->expected_size = 4;
				break;
			case ALCATEL_CONNECT_ACK:
				d->expected_size = 6;
				break;
			case ALCATEL_DISCONNECT_ACK:
				d->expected_size = 3;
				break;
			default:
				smprintf(s,"WARNING: Something went wrong, unknown packet received (%02X)\n", rx_char);
				return ERR_UNKNOWNRESPONSE;
		}
	/* Check counter, we can probably ignore error here ;-) */
	} else if ((d->Msg.Length == 2) && (d->Msg.Type == ALCATEL_DATA)) {
		if (rx_char != d->in_counter) {
			smprintf(s,"WARNING: Unexpected packet number, ignoring (expected %02X, received %02X)\n", d->in_counter, rx_char);
			d->in_counter = rx_char;
		}
		/* Increase incoming packet counter */
		if (d->in_counter == ALCATEL_MAX_COUNTER) d->in_counter = 0;
		else d->in_counter++;
	/* Read size for data packet */
	} else if ((d->Msg.Length == 4) && (d->Msg.Type == ALCATEL_DATA)) {
	/* Header till now + checksum */
		d->expected_size = (int)rx_char + 6;
	}

	/* Write received byte into buffer */
	d->Msg.Buffer[d->Msg.Length++] = rx_char;

	/* Did we received whole packet? */
	if (d->expected_size == d->Msg.Length) {
		/* Check checksum */
		for (i=0; i< (d->Msg.Length - 1); i++) checksum ^= d->Msg.Buffer[i];
		if (checksum != d->Msg.Buffer[d->Msg.Length - 1]) {
			/* We can only warn, as we don't know what should happend now... */
			smprintf(s,"WARNING: Ignoring incorrect packet checksum!\n");
		}

		/* Was it data? */
		if (d->Msg.Type == ALCATEL_DATA) {
			/* Dispatch message */
			s->Phone.Data.RequestMsg	= &d->Msg;
			s->Phone.Data.DispatchError	= s->Phone.Functions->DispatchMessage(s);
			/* Send ack */
			ALCABUS_WriteMessage (s, 0, 0, ALCATEL_ACK);
			/* Reset message length */
			d->Msg.Length = 0;
		/* Was it ack? */
		} else if ((d->Msg.Type == ALCATEL_ACK) ||
				(d->Msg.Type == ALCATEL_CONTROL) ||
				(d->Msg.Type == ALCATEL_CONNECT_ACK) ||
				(d->Msg.Type == ALCATEL_DISCONNECT_ACK)) {
			/* TODO: check counter of ack? */
			if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL ||
				s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE) {
				smprintf(s, "Received %s ack ",
						(d->Msg.Type == ALCATEL_ACK) ? "normal" :
						(d->Msg.Type == ALCATEL_CONTROL) ? "control" :
						(d->Msg.Type == ALCATEL_CONNECT_ACK) ? "connect" :
						(d->Msg.Type == ALCATEL_DISCONNECT_ACK) ? "disconnect" :
						"BUG");
				smprintf(s, "0x%02x / 0x" SIZE_T_HEX4_FORMAT, d->Msg.Type, d->Msg.Length);
				DumpMessage(&s->di, d->Msg.Buffer, d->Msg.Length);
			}
			if (s->di.dl==DL_BINARY) {
				smprintf(s,"%c",0x02);	/* Receiving */
				smprintf(s,"%c",d->Msg.Type);
				smprintf(s,"%c",(int)d->Msg.Length/256);
				smprintf(s,"%c",(int)d->Msg.Length%256);
				for (i=0;i<d->Msg.Length;i++) smprintf(s,"%c",d->Msg.Buffer[i]);
			}
			if (d->Msg.Type != ALCATEL_CONTROL) {
				d->next_frame 	= ALCATEL_DATA;
				d->busy 	= false;
			}
			/* Reset message length */
			d->Msg.Length = 0;
		}

		/* Was it unexpected type? */
		if ((d->Msg.Type != d->next_frame) && (d->Msg.Type != ALCATEL_CONTROL)) {
			return ERR_FRAMENOTREQUESTED;
		}
	} /* Last byte of packet */

	return ERR_NONE;
}

static GSM_Error ALCABUS_Initialise(GSM_StateMachine *s)
{
	GSM_Protocol_ALCABUSData *d = &s->Protocol.Data.ALCABUS;

	/* Initialise some variables */
	d->Msg.BufferUsed	= 0;
	d->Msg.Buffer		= NULL;
	d->Msg.Length		= 0;
	d->Msg.Type		= 0;
	d->in_counter		= 1;
	d->out_counter		= 0;
	d->busy			= false;

	/* Initialise protocol */
	smprintf(s, "Initializing binary mode\n");
	return ALCABUS_WriteMessage (s, 0, 0, ALCATEL_CONNECT);
}

static GSM_Error ALCABUS_Terminate(GSM_StateMachine *s)
{
	/* Terminate protocol */
	smprintf(s, "Closing binary mode\n");
	return ALCABUS_WriteMessage (s, 0, 0, ALCATEL_DISCONNECT);
}

GSM_Protocol_Functions ALCABUSProtocol = {
	ALCABUS_WriteMessage,
	ALCABUS_StateMachine,
	ALCABUS_Initialise,
	ALCABUS_Terminate
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

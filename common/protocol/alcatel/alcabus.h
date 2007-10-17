/* (c) 2002-2003 by Michal Cihar */
/*
 * Low level functions for communication with Alcatel One Touch phones.
 *
 * This code implements the protocol used for synchronisation with PC.
 */

#ifndef alcabus_h
#define alcabus_h

#include "../protocol.h"

#define ALCATEL_HEADER			0x7E

/* packet types: */
/* used for starting binary connection (must be preceeded by
 * AT+CPROT=16,"V1.0",16 and phone should response to it by CONNECT_ACK)
 */
#define ALCATEL_CONNECT			0x0A
/* received when connect suceeded */
#define ALCATEL_CONNECT_ACK		0x0C
/* used for stopping binary connection */
#define ALCATEL_DISCONNECT		0x0D
/* received when binnary connection ends */
#define ALCATEL_DISCONNECT_ACK		0x0E
/* some control ack, I really don't know what should it do, so currently it
 * is just ignored. It comes time to time, and communication continues OK also
 * if no reply was made. */
#define ALCATEL_CONTROL			0x0F
/* sending/recieving data */
#define ALCATEL_DATA			0x02
/* acknowledge to data */
#define ALCATEL_ACK			0x06

/* Maximal value for packet counter */
#define ALCATEL_MAX_COUNTER		0x3D

typedef struct {
	GSM_Protocol_Message 	Msg;
	/* Incoming packets ID counter */
	int			in_counter;
	/* Outgoing packets ID counter */
	int			out_counter;
	/* Expected size of incoming packet */
	size_t			expected_size;
	/* What is type of frame we expect next */
	unsigned char		next_frame;
	/* State of mobile, if we expect something (generally some ack) we set
	 * this to true and no other action can be performed until it is false. */
	bool			busy;
} GSM_Protocol_ALCABUSData;

#ifndef GSM_USED_SERIALDEVICE
# define GSM_USED_SERIALDEVICE
#endif

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

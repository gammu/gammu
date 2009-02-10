/* (c) 2002-2003 by Marcin Wiacek */

#ifndef protocol_common_h
#define protocol_common_h

typedef enum {
	RX_Sync,
	RX_GetDestination,
	RX_GetSource,
	RX_GetType,
	RX_GetLength1,
	RX_GetLength2,
	RX_GetMessage
} GSM_Protocol_RXState;

typedef struct {
	size_t 		Length;
	size_t 		Count;
	unsigned char 	Type;
	unsigned char 	Source;
	unsigned char 	Destination;
	unsigned char	*Buffer;
	size_t		BufferUsed;
	unsigned char 	CheckSum[2];
} GSM_Protocol_Message;

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

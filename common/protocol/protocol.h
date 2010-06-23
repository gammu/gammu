#ifndef protocol_common_h
#define protocol_common_h

#define PROTOCOL_MAX_RECEIVE_LENGTH	2000

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
	int 		Length;
	int 		Count;
	unsigned char 	Type;
	unsigned char 	Source;
	unsigned char 	Destination;
	unsigned char 	Buffer		[PROTOCOL_MAX_RECEIVE_LENGTH];
	unsigned char 	CheckSum	[2];
} GSM_Protocol_Message;

#endif

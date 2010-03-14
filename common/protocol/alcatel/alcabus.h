
#ifndef alcabus_h
#define alcabus_h

#include "../protocol.h"

typedef struct {
	GSM_Protocol_Message 	Msg;
	bool 			wascrlf;
	char 			*linestart;
} GSM_Protocol_ALCABUSData;

#ifndef GSM_USED_SERIALDEVICE
#  define GSM_USED_SERIALDEVICE
#endif

#endif

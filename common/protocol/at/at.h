
#ifndef at_h
#define at_h

#include "../protocol.h"

typedef struct {
	GSM_Protocol_Message 	Msg;
	bool 			wascrlf;
	char 			*linestart;
} GSM_Protocol_ATData;

#ifndef GSM_USED_SERIALDEVICE
#  define GSM_USED_SERIALDEVICE
#endif
#if defined(GSM_ENABLE_ATBLUETOOTH) || defined(GSM_ENABLE_DLR3BLUETOOTH)
#  ifndef GSM_USED_BLUETOOTHDEVICE
#    define GSM_USED_BLUETOOTHDEVICE
#  endif
#endif

#endif

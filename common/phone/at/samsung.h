#ifndef samsung_h
#define samsung_h

#include "../../gsmstate.h"

GSM_Error SAMSUNG_ReplyGetRingtone	(GSM_Protocol_Message, GSM_StateMachine *);
GSM_Error SAMSUNG_ReplySetRingtone	(GSM_Protocol_Message, GSM_StateMachine *);
GSM_Error SAMSUNG_ReplyGetBitmap	(GSM_Protocol_Message, GSM_StateMachine *);
GSM_Error SAMSUNG_ReplySetBitmap	(GSM_Protocol_Message, GSM_StateMachine *);
GSM_Error SAMSUNG_GetRingtone		(GSM_StateMachine *, GSM_Ringtone *, bool);
GSM_Error SAMSUNG_SetRingtone		(GSM_StateMachine *, GSM_Ringtone *, int *);
GSM_Error SAMSUNG_GetBitmap		(GSM_StateMachine *, GSM_Bitmap *);
GSM_Error SAMSUNG_SetBitmap		(GSM_StateMachine *, GSM_Bitmap *);
GSM_Error SAMSUNG_GetCallLogs		(GSM_StateMachine *, GSM_MemoryEntry *, int);
GSM_Error SAMSUNG_ReplyGetMemoryInfo(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error SAMSUNG_ReplyGetMemory(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error SAMSUNG_SetMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry);

#endif

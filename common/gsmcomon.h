
#ifndef gsm_common_h
#define gsm_common_h

#include <stdio.h>
#ifndef WIN32
#  include <unistd.h>
#endif

#include "../cfg/config.h"
#include "misc/misc.h"
#include "misc/cfg.h"

#define PHONE_MAXSMSINFOLDER		200

#define GSM_MAX_NUMBER_LENGTH		50

/* GSM_Error is used to notify about errors */
typedef enum {
/* 1*/	GE_NONE=1,
	GE_DEVICEOPENERROR,		/* Error during opening device */
	GE_DEVICEDTRRTSERROR,		/* Error during setting DTR/RTS in device */
	GE_DEVICECHANGESPEEDERROR,	/* Error during changing speed in device */
	GE_DEVICEWRITEERROR,		/* Error during writing device */
	GE_DEVICEREADERROR,		/* Error during reading device */
	GE_DEVICEPARITYERROR,		/* Can't set parity on device */
	GE_TIMEOUT,			/* Command timed out */
	GE_FRAMENOTREQUESTED,		/* Frame handled, but not requested in this moment */
/*10*/	GE_UNKNOWNRESPONSE,		/* Response not handled by gammu */
	GE_UNKNOWNFRAME,		/* Frame not handled by gammu */
	GE_UNKNOWNCONNECTIONTYPESTRING,	/* Unknown connection type given by user */
	GE_UNKNOWNMODELSTRING,		/* Unknown model given by user */
	GE_SOURCENOTAVAILABLE,		/* Some functions not compiled in your OS */
	GE_NOTSUPPORTED,		/* Not supported by phone */
	GE_EMPTY,			/* Empty phonebook entry, ... */
	GE_SECURITYERROR,		/* Not allowed */
	GE_INVALIDLOCATION,		/* Too high or too low location... */
	GE_NOTIMPLEMENTED,		/* Function not implemented */
/*20*/	GE_FULL,			/* Memory is full */
	GE_UNKNOWN,
	GE_CANTOPENFILE, 		/* Error during opening file */
	GE_MOREMEMORY,			/* More memory required */
	GE_PERMISSION,			/* No permission */
/*25*/	GE_EMPTYSMSC,			/* SMSC number is empty */
	GE_INSIDEPHONEMENU,		/* Inside phone menu - can't make something */
	GE_NOTCONNECTED,		/* Phone NOT connected - can't make something */
	GE_WORKINPROGRESS		/* Work in progress */
} GSM_Error;

/* Structure used for passing dates/times to date/time functions such as
   GSM_GetTime and GSM_GetAlarm etc. */
typedef struct {
	int	Year;		/* The complete year specification - e.g.1999.Y2K :-) */
	int 	Month;		/* January = 1 */
	int 	Day;
	int	Hour;
	int 	Minute;
	int	Second;
	int	Timezone;	/* The difference between local time and GMT */
} GSM_DateTime;

extern GSM_Error NoneFunction		(void);
extern GSM_Error NotImplementedFunction	(void);
extern GSM_Error NotSupportedFunction	(void);

#define NONEFUNCTION 	(void *) NoneFunction
#define NOTIMPLEMENTED 	(void *) NotImplementedFunction
#define NOTSUPPORTED 	(void *) NotSupportedFunction

char *GetMsg 		(CFG_Header *cfg, int number, char *default_string);
char *print_error	(GSM_Error e, FILE *df, CFG_Header *cfg);

void GSM_GetCurrentDateTime (GSM_DateTime *Date);

GSM_Error 	lock_device	(const char* port, char **lock_device);
bool 		unlock_device	(char *lock_file);

char *GetGammuVersion();

GSM_Error GSM_SetDebugFile(char *info, Debug_Info *di);

#endif

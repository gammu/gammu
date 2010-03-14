
#ifndef gsm_common_h
#define gsm_common_h

#include <stdio.h>
#ifndef WIN32
#  include <unistd.h>
#endif

#include "../cfg/config.h"
#include "misc/misc.h"
#include "misc/cfg.h"

#define MAX_MANUFACTURER_LENGTH		50
#define MAX_MODEL_LENGTH		50
#define MAX_VERSION_LENGTH		50
#define MAX_VERSION_DATE_LENGTH		50
#define	MAX_IMEI_LENGTH			20
#define PHONE_MAXSMSINFOLDER		200
#define GSM_MAX_NUMBER_LENGTH		50
#define GSM_MAXCALENDARTODONOTES	300

/* GSM_Error is used to notify about errors */
typedef enum {
/* 1*/	GE_NONE=1,
	GE_DEVICEOPENERROR,		/* Error during opening device */
	GE_DEVICELOCKED,		/* Device locked */
	GE_DEVICEDTRRTSERROR,		/* Error during setting DTR/RTS in device */
	GE_DEVICECHANGESPEEDERROR,	/* Error during changing speed in device */
	GE_DEVICEWRITEERROR,		/* Error during writing device */
	GE_DEVICEREADERROR,		/* Error during reading device */
	GE_DEVICEPARITYERROR,		/* Can't set parity on device */
	GE_TIMEOUT,			/* Command timed out */
/*10*/	GE_FRAMENOTREQUESTED,		/* Frame handled, but not requested in this moment */
	GE_UNKNOWNRESPONSE,		/* Response not handled by gammu */
	GE_UNKNOWNFRAME,		/* Frame not handled by gammu */
	GE_UNKNOWNCONNECTIONTYPESTRING,	/* Unknown connection type given by user */
	GE_UNKNOWNMODELSTRING,		/* Unknown model given by user */
	GE_SOURCENOTAVAILABLE,		/* Some functions not compiled in your OS */
	GE_NOTSUPPORTED,		/* Not supported by phone */
	GE_EMPTY,			/* Empty phonebook entry, ... */
	GE_SECURITYERROR,		/* Not allowed */
	GE_INVALIDLOCATION,		/* Too high or too low location... */
/*20*/	GE_NOTIMPLEMENTED,		/* Function not implemented */
	GE_FULL,			/* Memory is full */
	GE_UNKNOWN,
	GE_CANTOPENFILE, 		/* Error during opening file */
	GE_MOREMEMORY,			/* More memory required */
	GE_PERMISSION,			/* No permission */
	GE_EMPTYSMSC,			/* SMSC number is empty */
	GE_INSIDEPHONEMENU,		/* Inside phone menu - can't make something */
	GE_NOTCONNECTED,		/* Phone NOT connected - can't make something */
	GE_WORKINPROGRESS,		/* Work in progress */
/*30*/	GE_PHONEOFF,			/* Phone is disabled and connected to charger */
	GE_FILENOTSUPPORTED,		/* File format not supported by Gammu */
	GE_BUG,                  	/* Found bug in implementation or phone */
    	GE_CANCELED,                 	/* Action was canceled by user */
	GE_NEEDANOTHERANSWER   		/* Inside Gammu: phone module need to send another answer frame */
} GSM_Error;

extern GSM_Error NoneFunction		(void);
extern GSM_Error NotImplementedFunction	(void);
extern GSM_Error NotSupportedFunction	(void);

#define NONEFUNCTION 	(void *) NoneFunction
#define NOTIMPLEMENTED 	(void *) NotImplementedFunction
#define NOTSUPPORTED 	(void *) NotSupportedFunction

unsigned char *GetMsg 		(CFG_Header *cfg, unsigned char *default_string);
unsigned char *print_error	(GSM_Error e, FILE *df, CFG_Header *cfg);

GSM_Error GSM_SetDebugFile(char *info, Debug_Info *privdi);

char *GetGammuVersion();

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

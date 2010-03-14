/* (c) 2002-2004 by Marcin Wiacek & Michal Cihar */

#ifndef __gsm_comon_h
#define __gsm_comon_h

#include <stdio.h>
#ifndef WIN32
#  include <unistd.h>
#endif

#include "config.h"
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
/* 1*/	ERR_NONE=1,
	ERR_DEVICEOPENERROR,		/* Error during opening device */
	ERR_DEVICELOCKED,		/* Device locked */
	ERR_DEVICENOTEXIST,
	ERR_DEVICEBUSY,
	ERR_DEVICENOPERMISSION,
	ERR_DEVICENODRIVER,
	ERR_DEVICENOTWORK,
	ERR_DEVICEDTRRTSERROR,		/* Error during setting DTR/RTS in device */
/*10*/	ERR_DEVICECHANGESPEEDERROR,	/* Error during changing speed in device */
	ERR_DEVICEWRITEERROR,		/* Error during writing device */
	ERR_DEVICEREADERROR,		/* Error during reading device */
	ERR_DEVICEPARITYERROR,		/* Can't set parity on device */
	ERR_TIMEOUT,			/* Command timed out */
	ERR_FRAMENOTREQUESTED,		/* Frame handled, but not requested in this moment */
	ERR_UNKNOWNRESPONSE,		/* Response not handled by gammu */
	ERR_UNKNOWNFRAME,		/* Frame not handled by gammu */
	ERR_UNKNOWNCONNECTIONTYPESTRING,/* Unknown connection type given by user */
	ERR_UNKNOWNMODELSTRING,		/* Unknown model given by user */
/*20*/	ERR_SOURCENOTAVAILABLE,		/* Some functions not compiled in your OS */
	ERR_NOTSUPPORTED,		/* Not supported by phone */
	ERR_EMPTY,			/* Empty phonebook entry, ... */
	ERR_SECURITYERROR,		/* Not allowed */
	ERR_INVALIDLOCATION,		/* Too high or too low location... */
	ERR_NOTIMPLEMENTED,		/* Function not implemented */
	ERR_FULL,			/* Memory is full */
	ERR_UNKNOWN,
	ERR_CANTOPENFILE, 		/* Error during opening file */
	ERR_MOREMEMORY,			/* More memory required */
/*30*/	ERR_PERMISSION,			/* No permission */
	ERR_EMPTYSMSC,			/* SMSC number is empty */
	ERR_INSIDEPHONEMENU,		/* Inside phone menu - can't make something */
	ERR_NOTCONNECTED,		/* Phone NOT connected - can't make something */
	ERR_WORKINPROGRESS,		/* Work in progress */
	ERR_PHONEOFF,			/* Phone is disabled and connected to charger */
	ERR_FILENOTSUPPORTED,		/* File format not supported by Gammu */
	ERR_BUG,                  	/* Found bug in implementation or phone */
    	ERR_CANCELED,                 	/* Action was canceled by user */
	ERR_NEEDANOTHERANSWER,   	/* Inside Gammu: phone module need to send another answer frame */
/*40*/	ERR_OTHERCONNECTIONREQUIRED,
	ERR_WRONGCRC,
	ERR_INVALIDDATETIME,		/* Invalid date/time */
	ERR_MEMORY,			/* Phone memory error, maybe it is read only */
	ERR_INVALIDDATA,		/* Invalid data */
	ERR_FILEALREADYEXIST		/* File with specified name already exist */
} GSM_Error;

extern GSM_Error NoneFunction		(void);
extern GSM_Error NotImplementedFunction	(void);
extern GSM_Error NotSupportedFunction	(void);

#define NONEFUNCTION 	(void *) NoneFunction
#define NOTIMPLEMENTED 	(void *) NotImplementedFunction
#define NOTSUPPORTED 	(void *) NotSupportedFunction

unsigned char *GetMsg 		(INI_Section *cfg, unsigned char *default_string);
unsigned char *print_error	(GSM_Error e, FILE *df, INI_Section *cfg);

GSM_Error GSM_SetDebugFile(char *info, Debug_Info *privdi);

const char *GetGammuLocalePath(void);
const char *GetGammuVersion(void);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

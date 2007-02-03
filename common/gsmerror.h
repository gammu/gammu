#ifndef __gsm_error_h
#define __gsm_error_h

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
	ERR_INVALIDDATA,		/* Invalid data given to phone */
	ERR_FILEALREADYEXIST,		/* File with specified name already exist */
	ERR_FILENOTEXIST,		/* File with specified name doesn't exist */
	ERR_SHOULDBEFOLDER,		/* You have to give folder (not file) name */
	ERR_SHOULDBEFILE,		/* You have to give file (not folder) name */
	ERR_NOSIM,			/* Can not access SIM card */
	ERR_GNAPPLETWRONG,
	ERR_FOLDERPART,
	ERR_FOLDERNOTEMPTY,
	ERR_DATACONVERTED
} GSM_Error;
#endif

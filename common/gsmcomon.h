/* (c) 2002-2004 by Marcin Wiacek & Michal Cihar */

#ifndef __gsm_comon_h
#define __gsm_comon_h

#include <stdio.h>
#ifndef WIN32
#  include <unistd.h>
#endif

#include "config.h"
#include "gsmerror.h"
#include "misc/misc.h"
#include "misc/cfg.h"

#define MAX_MANUFACTURER_LENGTH		50
#define MAX_MODEL_LENGTH		50
#define MAX_VERSION_LENGTH		100
#define MAX_VERSION_DATE_LENGTH		50
#define	MAX_IMEI_LENGTH			20
#define PHONE_MAXSMSINFOLDER		600
#define GSM_MAX_NUMBER_LENGTH		50
#define GSM_MAXCALENDARTODONOTES	550

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

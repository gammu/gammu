/* (c) 2002-2004 by Marcin Wiacek & Michal Cihar */

#ifndef __gsm_comon_h
#define __gsm_comon_h

#include <stdio.h>
#ifndef WIN32
#  include <unistd.h>
#endif

#include <gammu.h>
#include <gammu-limits.h>
#include "config.h"
#include "misc/misc.h"

extern GSM_Error NoneFunction		(void);
extern GSM_Error NotImplementedFunction	(void);
extern GSM_Error NotSupportedFunction	(void);

#define NONEFUNCTION 	(void *) NoneFunction
#define NOTIMPLEMENTED 	(void *) NotImplementedFunction
#define NOTSUPPORTED 	(void *) NotSupportedFunction

unsigned char *print_error	(GSM_Error e, FILE *df);

GSM_Error GSM_SetDebugFile(char *info, Debug_Info *privdi);

const char *GetGammuLocalePath(void);
const char *GetGammuVersion(void);

/**
 * Initializes locales.
 */
extern void InitLocales(const char *path);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

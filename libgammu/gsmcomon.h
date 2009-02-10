/* (c) 2002-2004 by Marcin Wiacek & Michal Cihar */

#ifndef __gsm_comon_h
#define __gsm_comon_h

#include <gammu-error.h>

extern GSM_Error NoneFunction		(void);
extern GSM_Error NotImplementedFunction	(void);
extern GSM_Error NotSupportedFunction	(void);

#define NONEFUNCTION 	(void *) NoneFunction
#define NOTIMPLEMENTED 	(void *) NotImplementedFunction
#define NOTSUPPORTED 	(void *) NotSupportedFunction

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

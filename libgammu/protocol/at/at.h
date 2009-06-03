/* (c) 2002-2003 by Marcin Wiacek and Michal Cihar */

#ifndef at_h
#define at_h

#include "../protocol.h"

typedef struct {
	GSM_Protocol_Message 	Msg;
	gboolean 			wascrlf;
	int			LineStart,LineEnd;
	int			SpecialAnswerLines,SpecialAnswerStart;

	gboolean			EditMode;	/* wait for modem answer or not */
	gboolean			FastWrite;
} GSM_Protocol_ATData;

#ifndef GSM_USED_SERIALDEVICE
#  define GSM_USED_SERIALDEVICE
#endif
#if defined(GSM_ENABLE_BLUEAT)
#  ifndef GSM_USED_BLUETOOTHDEVICE
#    define GSM_USED_BLUETOOTHDEVICE
#  endif
#endif
#if defined(GSM_ENABLE_IRDAAT)
#  ifndef GSM_USED_IRDADEVICE
#    define GSM_USED_IRDADEVICE
#  endif
#endif

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

/* (c) 2003 by Marcin Wiacek */

#ifndef obexgen_h
#define obexgen_h

#include "../../gsmcomon.h"
#include "../../gsmstate.h"
#include "../../service/gsmmisc.h"
#include "../../service/sms/gsmsms.h"

#ifndef GSM_USED_IRDAOBEX
#  define GSM_USED_IRDAOBEX
#endif
#ifndef GSM_USED_BLUEOBEX
#  define GSM_USED_BLUEOBEX
#endif

typedef enum {
	OBEX_None = 1,
	OBEX_BrowsingFolders
} OBEX_Service;

typedef struct {
	int				FileLev;
	int				FilesLocationsUsed;
	int				FilesLocationsCurrent;
	GSM_File			Files[500];
	bool				FileLastPart;

	int				FrameSize;
	OBEX_Service			Service;
} GSM_Phone_OBEXGENData;

GSM_Error OBEXGEN_GetFilePart	(GSM_StateMachine *s, GSM_File *File);
GSM_Error OBEXGEN_AddFilePart	(GSM_StateMachine *s, GSM_File *File, int *Pos);
GSM_Error OBEXGEN_Disconnect	(GSM_StateMachine *s);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

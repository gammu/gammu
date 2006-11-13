/* (c) 2002-2006 by Marcin Wiacek and Michal Cihar */

#ifndef obexgen_functions_h
#define obexgen_functions_h

#include "../../gsmcomon.h"
#include "../../gsmstate.h"

extern GSM_Reply_Function	OBEXGENReplyFunctions[];
extern GSM_Error OBEXGEN_InitialiseVars(GSM_StateMachine *s);
extern GSM_Error OBEXGEN_GetFilePart	(GSM_StateMachine *s, GSM_File *File, int *Handle, int *Size);
extern GSM_Error OBEXGEN_AddFilePart	(GSM_StateMachine *s, GSM_File *File, int *Pos, int *Handle);
extern GSM_Error OBEXGEN_GetNextFileFolder(GSM_StateMachine *s, GSM_File *File, bool start);
extern GSM_Error OBEXGEN_Disconnect	(GSM_StateMachine *s);
extern GSM_Error OBEXGEN_Connect	(GSM_StateMachine *s, OBEX_Service service);
extern GSM_Error OBEXGEN_DeleteFile(GSM_StateMachine *s, unsigned char *ID);
extern GSM_Error OBEXGEN_AddFolder(GSM_StateMachine *s, GSM_File *File);
extern GSM_Error OBEXGEN_GetMemoryStatus(GSM_StateMachine *s, GSM_MemoryStatus *Status);
extern GSM_Error OBEXGEN_GetNextMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry, bool start);
extern GSM_Error OBEXGEN_GetMemory(GSM_StateMachine *s, GSM_MemoryEntry *Entry);
extern GSM_Error OBEXGEN_AddMemory(GSM_StateMachine *s, GSM_MemoryEntry *Entry);
extern GSM_Error OBEXGEN_SetMemory(GSM_StateMachine *s, GSM_MemoryEntry *Entry);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

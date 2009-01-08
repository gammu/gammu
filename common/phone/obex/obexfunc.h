/* (c) 2002-2006 by Marcin Wiacek and Michal Cihar */

#ifndef obexgen_functions_h
#define obexgen_functions_h

#include "../../gsmstate.h"
#include "obexgen.h"

extern GSM_Reply_Function	OBEXGENReplyFunctions[];
extern GSM_Error OBEXGEN_InitialiseVars(GSM_StateMachine *s);
extern void OBEXGEN_FreeVars(GSM_StateMachine *s);
extern GSM_Error OBEXGEN_GetFilePart	(GSM_StateMachine *s, GSM_File *File, int *Handle, int *Size);
extern GSM_Error OBEXGEN_AddFilePart	(GSM_StateMachine *s, GSM_File *File, int *Pos, int *Handle);
extern GSM_Error OBEXGEN_SendFilePart	(GSM_StateMachine *s, GSM_File *File, int *Pos, int *Handle);
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
extern GSM_Error OBEXGEN_DeleteMemory(GSM_StateMachine *s, GSM_MemoryEntry *Entry);
extern GSM_Error OBEXGEN_DeleteAllMemory(GSM_StateMachine *, GSM_MemoryType MemoryTypes);
extern GSM_Error OBEXGEN_GetCalendarStatus(GSM_StateMachine *s, GSM_CalendarStatus *Status);
extern GSM_Error OBEXGEN_GetCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Entry);
extern GSM_Error OBEXGEN_GetNextCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Entry, bool start);
extern GSM_Error OBEXGEN_AddCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Entry);
extern GSM_Error OBEXGEN_SetCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Entry);
extern GSM_Error OBEXGEN_DeleteCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Entry);
extern GSM_Error OBEXGEN_DeleteAllCalendar(GSM_StateMachine *s);
extern GSM_Error OBEXGEN_GetTodoStatus(GSM_StateMachine *s, GSM_ToDoStatus *Status);
extern GSM_Error OBEXGEN_GetTodo(GSM_StateMachine *s, GSM_ToDoEntry *Entry);
extern GSM_Error OBEXGEN_GetNextTodo(GSM_StateMachine *s, GSM_ToDoEntry *Entry, bool start);
extern GSM_Error OBEXGEN_AddTodo(GSM_StateMachine *s, GSM_ToDoEntry *Entry);
extern GSM_Error OBEXGEN_SetTodo(GSM_StateMachine *s, GSM_ToDoEntry *Entry);
extern GSM_Error OBEXGEN_DeleteTodo(GSM_StateMachine *s, GSM_ToDoEntry *Entry);
extern GSM_Error OBEXGEN_DeleteAllTodo(GSM_StateMachine *s);
extern GSM_Error OBEXGEN_GetNoteStatus(GSM_StateMachine *s, GSM_ToDoStatus *Status);
extern GSM_Error OBEXGEN_GetNote(GSM_StateMachine *s, GSM_NoteEntry *Entry);
extern GSM_Error OBEXGEN_GetNextNote(GSM_StateMachine *s, GSM_NoteEntry *Entry, bool start);
extern GSM_Error OBEXGEN_AddNote(GSM_StateMachine *s, GSM_NoteEntry *Entry);
extern GSM_Error OBEXGEN_SetNote(GSM_StateMachine *s, GSM_NoteEntry *Entry);
extern GSM_Error OBEXGEN_DeleteNote(GSM_StateMachine *s, GSM_NoteEntry *Entry);
extern GSM_Error OBEXGEN_DeleteAllNotes(GSM_StateMachine *s);
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

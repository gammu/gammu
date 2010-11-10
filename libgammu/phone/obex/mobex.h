/* (c) 2010 by Michal Cihar */

/**
 * \file obexgen.c
 * @author Michal Čihař
 */
/**
 * @addtogroup Phone
 * @{
 */
/**
 * \addtogroup MOBEXPhone
 *
 * @{
 */

#ifndef mobex_h
#define mobex_h

/**
 * Returns status of given memory.
 */
GSM_Error MOBEX_GetStatus(GSM_StateMachine *s, const char *path, int *free_records, int *used);

/**
 * Creates an entry.
 */
GSM_Error MOBEX_CreateEntry(GSM_StateMachine *s, const char *path, int *location, const char *data);

/**
 * Updates an entry.
 */
GSM_Error MOBEX_UpdateEntry(GSM_StateMachine *s, const char *path, const int location, const char *data);

/**
 * Reads memory entry using m-obex.
 */
GSM_Error MOBEX_GetMemory(GSM_StateMachine *s, GSM_MemoryEntry *Entry);

/**
 * Reads calendar entry using m-obex.
 */
GSM_Error MOBEX_GetCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Entry);
#endif
/*@}*/
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

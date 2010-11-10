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

#endif
/*@}*/
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

/**
 * \file gammu.h
 * \author Michal Čihař
 * 
 * Top level include for applications.
 */
#ifndef __gammu_gammu_h
#define __gammu_gammu_h

struct _GSM_StateMachine;

/**
 * \defgroup StateMachine State machine
 * Generic state machine layer.
 */

/**
 * Private structure holding information about phone connection.
 * \ingroup StateMachine
 */
typedef struct _GSM_StateMachine 	GSM_StateMachine;

#include <error.h>

/**
 * Initiates connection.
 *
 * \ingroup StateMachine
 *
 * \param s State machine data
 * \param ReplyNum Number of replies to await (usually 3).
 * \return Error code
 */
GSM_Error GSM_InitConnection		(GSM_StateMachine *s, int ReplyNum);

/**
 * Terminates connection.
 *
 * \ingroup StateMachine
 *
 * \param s State machine data
 * \return Error code
 */
GSM_Error GSM_TerminateConnection	(GSM_StateMachine *s);

#endif
/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

/**
 * \file gammu.h
 * \author Michal Čihař
 * 
 * Top level include for applications.
 */
#ifndef __gammu_gammu_h
#define __gammu_gammu_h

#include <gammu-types.h>

/**
 * \defgroup StateMachine State machine
 * Generic state machine layer.
 */

/**
 * Private structure holding information about phone connection.
 * \ingroup StateMachine
 */
typedef struct _GSM_StateMachine 	GSM_StateMachine;

#include <gammu-error.h>
#include <gammu-inifile.h>

/**
 * Initiates connection.
 *
 * \ingroup StateMachine
 *
 * \param s State machine data
 * \param ReplyNum Number of replies to await (usually 3).
 * \return Error code
 */
GSM_Error GSM_InitConnection(GSM_StateMachine *s, int ReplyNum);

/**
 * Terminates connection.
 *
 * \ingroup StateMachine
 *
 * \param s State machine data
 * \return Error code
 */
GSM_Error GSM_TerminateConnection(GSM_StateMachine *s);

/**
 * Attempts to read data from phone. This can be used for getting 
 * status of incoming events, which would not be found out without 
 * polling device.
 *
 * \ingroup StateMachine
 *
 * \param s State machine data
 * \param wait Whether to wait for some event
 * \return Number of read bytes
 */
int GSM_ReadDevice(GSM_StateMachine *s, bool wait);
#endif
/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

/* (c) 2007 Michal Cihar */

/**
 * @file motorola.h
 * @author Michal Čihař
 */
/**
 * @ingroup Phone
 * @{
 */
/**
 * @addtogroup ATPhone
 * @{
 */

#include <gammu-config.h>

#ifdef GSM_ENABLE_ATGEN

/**
 * Switches to correct mode to execute command.
 *
 * \param s State machine data.
 * \param command Command which should be checked.
 *
 * \return Error code.
 */
GSM_Error MOTOROLA_SetMode(GSM_StateMachine *s, const char *command);

/**
 * Wrapper around \ref GSM_WaitFor, which automatically sets
 * correct Motorola mode. It accepts same parameters as
 * \ref GSM_WaitFor.
 */
#define ATGEN_WaitFor(s, cmd, len, type, time, request) \
        error = MOTOROLA_SetMode(s, cmd); \
        if (error != ERR_NONE) return error; \
        error = GSM_WaitFor(s, cmd, len, type, time, request);


#endif

/*@}*/
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

/**
 * \file gammu-debug.h
 * \author Michal Čihař
 *
 * Debuging handling.
 */
#ifndef __gammu_debug_h
#define __gammu_debug_h

/**
 * \defgroup Debug Debug
 * Debuging handling.
 */

#include <stdio.h>

#include <gammu-error.h>
#include <gammu-misc.h>

/**
 * Debugging configuration.
 * \ingroup Debug
 */
typedef struct _GSM_Debug_Info GSM_Debug_Info;

/**
 * Sets logging function.
 *
 * \param info Function to call.
 * \param data User data to pass as a second parameter to callback.
 * \param privdi Pointer to debug information data.
 * \return Error code.
 *
 * \ingroup Debug
 */
GSM_Error GSM_SetDebugFunction(GSM_Log_Function info, void *data,
			       GSM_Debug_Info * privdi);

/**
 * Sets debug file.
 *
 * \param info File path.
 * \param privdi Pointer to debug information data.
 * \return Error code.
 *
 * \ingroup Debug
 */
GSM_Error GSM_SetDebugFile(const char *info, GSM_Debug_Info * privdi);

/**
 * Sets debug file.
 *
 * \param fd File descriptor.
 * \param privdi Pointer to debug information data.
 * \param closable Whether Gammu can close the file when it is no longer
 * needed for debug output. Please note that stderr or stdout are never
 * closed.
 * \return Error code.
 *
 * \ingroup Debug
 */
GSM_Error GSM_SetDebugFileDescriptor(FILE * fd, gboolean closable,
				     GSM_Debug_Info * privdi);

/**
 * Returns global debug settings.
 *
 * \return Pointer to global settings.
 *
 * \ingroup Debug
 */
GSM_Debug_Info *GSM_GetGlobalDebug(void);

/**
 * Gets debug information for state machine.
 *
 * \param s State machine data
 * \return Debug information.
 *
 * \ingroup StateMachine
 */
GSM_Debug_Info *GSM_GetDebug(GSM_StateMachine * s);

/**
 * Returns debug information active for state machine. Please note that
 * it can be either global debug or state machine debug structure,
 * depending on use_global flag. For configuring usite GSM_GetDebug.
 *
 * \param s State machine data
 * \return Debug information.
 *
 * \ingroup StateMachine
 */
GSM_Debug_Info *GSM_GetDI(GSM_StateMachine * s);

/**
 * Sets debug level.
 *
 * \param info Level as text.
 * \param privdi Pointer to debug information data.
 * \return True on success.
 *
 * \ingroup Debug
 */
gboolean GSM_SetDebugLevel(const char *info, GSM_Debug_Info * privdi);

/**
 * Sets debug encoding.
 *
 * \param info Encoding to set.
 * \param privdi Pointer to debug information data.
 * \return True on success.
 *
 * \ingroup Debug
 */
gboolean GSM_SetDebugCoding(const char *info, GSM_Debug_Info * privdi);

/**
 * Enables using of global debugging configuration. Makes no effect on
 * global debug configuration.
 *
 * \param info Enable global debug usage..
 * \param privdi Pointer to debug information data.
 * \return True on success.
 *
 * \ingroup Debug
 */
gboolean GSM_SetDebugGlobal(gboolean info, GSM_Debug_Info * privdi);

/**
 * Logs error to debug log with additional message.
 *
 * \param s State machine structure pointer.
 * \param message String to be show in message.
 * \param err Error code.
 *
 * \ingroup Debug
 */
void GSM_LogError(GSM_StateMachine * s, const char *message,
		  const GSM_Error err);

/**
 * Prints string to defined debug log.
 *
 * \param s State machine, where to print.
 * \param format Format string as for printf.
 * \return Upon successful return, these functions return the number of characters printed (as printf).
 *
 * \ingroup Debug
 */
PRINTF_STYLE(2, 3)
int smprintf(GSM_StateMachine * s, const char *format, ...);

#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

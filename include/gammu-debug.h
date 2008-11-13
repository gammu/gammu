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

#include <gammu-statemachine.h>
#include <gammu-error.h>
#include <gammu-misc.h>

/**
 * Debugging configuration.
 * \ingroup Debug
 */
typedef struct _DebugInfo GSM_Debug_Info;

/**
 * Prints string to global debug log.
 *
 * \param format Format string as for printf.
 * \return Upon successful return, these functions return the number of characters printed (as printf).
 *
 * \ingroup Debug
 */
#ifdef DEBUG
PRINTF_STYLE(1, 2)
int dbgprintf(const char *format, ...);
#else
#  ifdef __GNUC__
#    define dbgprintf(a...) do { } while (0)
#  else
#    define dbgprintf
#  endif
#endif

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

/**
 * Severity of printed message.
 */
typedef enum {
	/**
	 * Message will not be printed.
	 */
	D_NONE,
	/**
	 * Message is printed in all text log levels.
	 */
	D_TEXT,
	/**
	 * Message is printed on all text error log levels.
	 */
	D_ERROR,
} GSM_DebugSeverity;

/**
 * Prints string to defined debug log.
 *
 * \param s State machine, where to print.
 * \param severity Severity of printed message.
 * \param format Format string as for printf.
 * \return Upon successful return, these functions return the number of characters printed (as printf).
 *
 * \ingroup Debug
 */
PRINTF_STYLE(3, 4)
int smprintf_level(GSM_StateMachine * s, GSM_DebugSeverity severity, const char *format, ...);

/**
 * Sets debug file.
 *
 * \param info File path.
 * \param privdi Pointert to debug information data.
 * \return Error code.
 *
 * \ingroup Debug
 */
GSM_Error GSM_SetDebugFile(char *info, GSM_Debug_Info * privdi);

/**
 * Sets debug file.
 *
 * \param fd File descriptor.
 * \param privdi Pointert to debug information data.
 * \param closable Whether Gammu can close the file when it is no longer
 * needed for debug output. Please note that stderr or stdout are never
 * closed.
 * \return Error code.
 *
 * \ingroup Debug
 */
GSM_Error GSM_SetDebugFileDescriptor(FILE * fd, bool closable, GSM_Debug_Info * privdi);

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
 * Sets debug level.
 *
 * \param info Level as text.
 * \param privdi Pointer to debug information data.
 * \return True on success.
 *
 * \ingroup Debug
 */
bool GSM_SetDebugLevel(const char *info, GSM_Debug_Info * privdi);

/**
 * Sets debug encoding.
 *
 * \param info Encoding to set.
 * \param privdi Pointer to debug information data.
 * \return True on success.
 *
 * \ingroup Debug
 */
bool GSM_SetDebugCoding(const char *info, GSM_Debug_Info * privdi);

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
bool GSM_SetDebugGlobal(bool info, GSM_Debug_Info * privdi);

/**
 * Logs error to debug log with additional message.
 *
 * \param s State machine structure pointer.
 * \param message String to be show in message.
 * \param err Error code.
 *
 * \ingroup Debug
 */
void GSM_LogError(GSM_StateMachine * s, const char * message, const GSM_Error err);

#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

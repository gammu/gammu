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
#if defined(__GNUC__) && !defined(printf)
__attribute__((format(printf, 1, 2)))
#endif
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
#if defined(__GNUC__) && !defined(printf)
__attribute__((format(printf, 2, 3)))
#endif
int smprintf(GSM_StateMachine *s, const char *format, ...);


/**
 * Sets debug file.
 *
 * \param info File path.
 * \param privdi Pointert to debug information data.
 * \return Error code.
 *
 * \ingroup Debug
 */
GSM_Error GSM_SetDebugFile(char *info, GSM_Debug_Info *privdi);

/**
 * Sets debug file.
 *
 * \param fd File descriptor.
 * \param privdi Pointert to debug information data.
 * \return Error code.
 *
 * \ingroup Debug
 */
GSM_Error GSM_SetDebugFileDescriptor(FILE *fd, GSM_Debug_Info *privdi);

/**
 * Returns global debug settings.
 *
 * \return Pointer to global settings.
 *
 * \ingroup Debug
 */
GSM_Debug_Info *GSM_GetGlobalDebug(void);

/**
 * Sets debug level.
 *
 * \param info Level as text.
 * \param privdi Pointer to debug information data.
 * \return True on success.
 *
 * \ingroup Debug.
 */
bool GSM_SetDebugLevel(const char *info, GSM_Debug_Info *privdi);

/**
 * Sets debug encoding.
 *
 * \param info Encoding to set.
 * \param privdi Pointer to debug information data.
 * \return True on success.
 *
 * \ingroup Debug.
 */
bool GSM_SetDebugCoding(const char *info, GSM_Debug_Info *privdi);

#endif

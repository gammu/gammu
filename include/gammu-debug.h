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
 * Debugging level.
 * \ingroup Debug
 */
typedef enum {
	DL_NONE = 0,		/**< No debug messages		*/
	DL_BINARY = 1,		/**< Binary transmission dump 	*/
	DL_TEXT,		/**< Text transmission dump	*/
	DL_TEXTALL,		/**< Everything			*/
	DL_TEXTERROR,		/**< Only errors			*/
	DL_TEXTDATE,		/**< Text transmission dump	*/
	DL_TEXTALLDATE,		/**< Everything			*/
	DL_TEXTERRORDATE	/**< Only errors			*/
} Debug_Level;

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
 * \param d Debug information structure, where to print.
 * \param format Format string as for printf.
 * \return Upon successful return, these functions return the number of characters printed (as printf).
 *
 * \ingroup Debug
 */
#if defined(__GNUC__) && !defined(printf)
__attribute__((format(printf, 2, 3)))
#endif
int smfprintf(GSM_Debug_Info *d, const char *format, ...);


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
 * Returns global debug settings.
 *
 * \return Pointer to global settings.
 *
 * \ingroup Debug
 */
GSM_Debug_Info *GSM_GetGlobalDebug();

/**
 * Sets debug level.
 *
 * \param info File path.
 * \param privdi Pointer to debug information data.
 * \return True on success.
 *
 * \ingroup Debug.
 */
bool GSM_SetDebugLevel(char *info, GSM_Debug_Info *privdi);

#endif

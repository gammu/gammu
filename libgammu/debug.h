#ifndef __gammu_common_debug_h
#define __gammu_common_debug_h

#include <gammu-debug.h>
#include <stdarg.h>

/* ------------------------------------------------------------------------- */


extern GSM_Debug_Info	GSM_global_debug;
extern GSM_Debug_Info	GSM_none_debug;

void DumpMessage(GSM_Debug_Info *d, const unsigned char *message, const int messagesize);
void DumpMessageText(GSM_Debug_Info *d, const unsigned char *message, const int messagesize);


/* ------------------------------------------------------------------------- */


/**
 * Debugging level.
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

struct _GSM_Debug_Info {
	Debug_Level	dl; /**< Level of messages to display */
	FILE		*df; /**< File used for debug messages output */
	gboolean        	use_global; /**< Whether to use global debug structure instead of this one. */
	const char	*coding; /**< Encoding used in console */
	gboolean		was_lf; /**< Has there already been new line */
	gboolean		closable; /**< Whether Gammu can close the file when it is no longer needed for debug output. */
    /**
     * Function which performs logging, in case it is set, no file logging happens.
     */
    GSM_Log_Function    log_function;
    /**
     * User data to be passed to callback.
     */
    void * user_data;
};


PRINTF_STYLE(2, 3)
int smfprintf(GSM_Debug_Info *d, const char *format, ...);

PRINTF_STYLE(2, 0)
int dbg_vprintf(GSM_Debug_Info *d, const char *format, va_list argp);

/**
 * Prints string to global debug log.
 *
 * \param format Format string as for printf.
 * \return Upon successful return, these functions return the number of characters printed (as printf).
 *
 * \ingroup Debug
 */
#ifdef DEBUG
#define dbgprintf smfprintf
#else
#  ifdef __GNUC__
#    define dbgprintf(...) do { } while (0)
#  else
#    define dbgprintf
#  endif
#endif

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
	D_ERROR
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

#endif

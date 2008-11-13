/* (c) 2002-2004 by Marcin Wiacek */

#ifndef __misc_h
#define __misc_h

#if defined(_MSC_VER) && defined(__cplusplus)
    extern "C" {
#endif

#define GAMMU_URL "<http://www.gammu.org>"
#define BACKUP_MAIN_HEADER "; This file format was designed for Gammu and is compatible with Gammu+"
#define BACKUP_INFO_HEADER "; See " GAMMU_URL " for more info"

#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#endif

#include <gammu-config.h>
#include <gammu-types.h>
#include <gammu-datetime.h>
#include <gammu-misc.h>
#include <gammu-debug.h>

/* ------------------------------------------------------------------------- */

/**
 * Listing of cut points for text lines.
 */
typedef struct {
	/**
	 * Cut points.
	 */
	int *numbers;
	/**
	 * Number of currently allocated entries.
	 */
	int allocated;
} GSM_CutLines;

/**
 * Calculates string cut points to split it to lines.
 */
void SplitLines(const char *message, int messagesize, GSM_CutLines *lines, unsigned char *whitespaces, int spaceslen, bool eot);

/**
 * Returns pointer to static buffer containing line.
 *
 * @param message Parsed message.
 * @param lines Parsed lines information.
 * @param start Which line we want.
 */
char *GetLineString(const char *message, const GSM_CutLines *lines, int start);

/**
 * Returns line length.
 * @param message Parsed message.
 * @param lines Parsed lines information.
 * @param start Which line we want.
 */
int GetLineLength(const char *message, const GSM_CutLines * lines, int start);

/**
 * Initializes line cut structure.
 */
void InitLines(GSM_CutLines *lines);

/**
 * Frees line cut structure.
 */
void FreeLines(GSM_CutLines *lines);

/**
 * Copies line to variable.
 */
void CopyLineString(char *dest, const char *src, const GSM_CutLines * lines, int start);

void GetTimeDifference(unsigned long diff, GSM_DateTime * DT, bool Plus,
		       int multi);

GSM_DateTime GSM_AddTime(GSM_DateTime DT, GSM_DeltaTime delta);

/**
 *
 * \ingroup DateTime
 */
int GetDayOfYear(unsigned int year, unsigned int month, unsigned int day);

/**
 *
 * \ingroup DateTime
 */
int GetWeekOfMonth(unsigned int year, unsigned int month, unsigned int day);

/**
 *
 * \ingroup DateTime
 */
int GetDayOfWeek(unsigned int year, unsigned int month, unsigned int day);


/* ------------------------------------------------------------------------- */


extern GSM_Debug_Info	di;

void DumpMessage(GSM_Debug_Info *d, const unsigned char *message, const int messagesize);


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

struct _DebugInfo {
	Debug_Level	dl; /**< Level of messages to display */
	FILE		*df; /**< File used for debug messages output */
	bool        	use_global; /**< Whether to use global debug structure instead of this one. */
	const char	*coding; /**< Encoding used in console */
	bool		was_lf; /**< Has there already been new line */
	bool		closable; /**< Whether Gammu can close the file when it is no longer needed for debug output. */
};


PRINTF_STYLE(2, 3)
int smfprintf(GSM_Debug_Info *d, const char *format, ...);

PRINTF_STYLE(2, 0)
int dbg_vprintf(GSM_Debug_Info *d, const char *format, va_list argp);

#ifdef WIN32
typedef SOCKET socket_type;
#define socket_invalid (INVALID_SOCKET)
#else
typedef int socket_type;
#define socket_invalid (-1)
#endif

/**
 * Strips spaces from string.
 *
 * \param[in,out] buff String where to strip buffers.
 */
void StripSpaces(char *buff);

#if defined(_MSC_VER) && defined(__cplusplus)

    }
#endif

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

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
#ifdef WIN32
#  include <windows.h>
#endif

#include "../config.h"

#include <gammu-types.h>
#include <gammu-datetime.h>
#include <gammu-misc.h>

/* ------------------------------------------------------------------------- */

#define MAX_LINES 50

typedef struct {
	int numbers[MAX_LINES*2];
} GSM_Lines;

void SplitLines(unsigned char *message, int messagesize, GSM_Lines *lines, unsigned char *whitespaces, int spaceslen, bool eot);
/**
 * Returns pointer to static buffer containing line.
 * @param message Parsed message.
 * @param lines Parsed lines information.
 * @param start Which line we want.
 */
char *GetLineString(unsigned char *message, GSM_Lines lines, int start);
/**
 * Returns line length.
 * @param message Parsed message.
 * @param lines Parsed lines information.
 * @param start Which line we want.
 */
int GetLineLength(unsigned char *message, GSM_Lines lines, int start);
void CopyLineString(unsigned char *dest, unsigned char *src, GSM_Lines lines, int start);

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

/**
 * Debugging configuration.
 */
typedef struct {
	Debug_Level	dl; /**< Level of messages to display */
	FILE		*df; /**< File used for debug messages output */
	bool        	use_global; /**< Whether to use global debug structure instead of this one. */
	char		*coding; /**< Encoding used in console */
	bool		was_lf; /**< Has there already been new line */
} Debug_Info;

extern Debug_Info	di;

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

#if defined(__GNUC__) && !defined(printf)
__attribute__((format(printf, 2, 3)))
#endif
int smfprintf(Debug_Info *d, const char *format, ...);

void DumpMessage(Debug_Info *d, const unsigned char *message, int messagesize);

bool GSM_SetDebugLevel(char *info, Debug_Info *di);

/* ------------------------------------------------------------------------- */



#if defined(_MSC_VER) && defined(__cplusplus)
    }
#endif

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

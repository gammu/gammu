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
void SplitLines(const char *message, const int messagesize, GSM_CutLines *lines, const char *whitespaces, const int spaceslen, const char *quotes, const int quoteslen, const gboolean eot);

/**
 * Returns pointer to static buffer containing line.
 *
 * @param message Parsed message.
 * @param lines Parsed lines information.
 * @param start Which line we want.
 */
const char *GetLineString(const char *message, const GSM_CutLines *lines, int start);

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

void GetTimeDifference(unsigned long diff, GSM_DateTime * DT, gboolean Plus,
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

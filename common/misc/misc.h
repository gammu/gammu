/* (c) 2002-2004 by Marcin Wiacek */

#ifndef __misc_h
#define __misc_h

#if defined(_MSC_VER) && defined(__cplusplus)
    extern "C" {
#endif

#include <stdio.h>
#include <time.h>
#ifdef WIN32
#  include <windows.h>
#endif

#include "../config.h"

#ifndef __cplusplus
#ifndef false
#  define false 0
#endif
#ifndef true
#  define true 	!0
#endif
#ifndef bool    
#  define bool	char
#endif
#endif /* __cplusplus */

#ifdef WIN32
#  define my_sleep(x) ((x)<1000 ? Sleep(1) : Sleep((x)/1000))
#else
#  define my_sleep(x) usleep(x)
#endif

#undef MAX
#define MAX(a,b) ((a)>(b) ? (a) : (b))
#undef MIN
#define MIN(a,b) ((a)<(b) ? (a) : (b))

/* ------------------------------------------------------------------------- */

#define MAX_LINES 50

int GetLine(FILE *File, char *Line, int count);

typedef struct {
	int numbers[MAX_LINES*2];
} GSM_Lines;

void SplitLines(unsigned char *message, int messagesize, GSM_Lines *lines, unsigned char *whitespaces, int spaceslen, bool eot);
char *GetLineString(unsigned char *message, GSM_Lines lines, int start);
void CopyLineString(unsigned char *dest, unsigned char *src, GSM_Lines lines, int start);

/* ------------------------------------------------------------------------- */

typedef enum {
	DL_BINARY = 1,		/* Binary transmission dump 	*/
	DL_TEXT,		/* Text transmission dump	*/
	DL_TEXTALL,		/* Everything			*/
	DL_TEXTERROR,		/* Only errors			*/
	DL_TEXTDATE,		/* Text transmission dump	*/
	DL_TEXTALLDATE,		/* Everything			*/
	DL_TEXTERRORDATE	/* Only errors			*/
} Debug_Level;

typedef struct {
	Debug_Level	dl;
	FILE		*df;
	bool        	use_global;
	char		*coding;
} Debug_Info;

extern Debug_Info	di;

#ifdef DEBUG
#ifdef __GNUC__
__attribute__((format(printf, 1, 2))) 
#endif
int dbgprintf(const char *format, ...);
#else
#  ifndef WIN32
#    define dbgprintf(a...) do { } while (0)
#  else
#    define dbgprintf
#  endif
#endif

#ifdef __GNUC__
__attribute__((format(printf, 3, 4)))
#endif
int smfprintf(FILE *f, Debug_Level dl, const char *format, ...);

void DumpMessage(FILE *df, Debug_Level dl, const unsigned char *message, int messagesize);

bool GSM_SetDebugLevel(char *info, Debug_Info *di);

/* ------------------------------------------------------------------------- */

/**
 * Structure used for saving date and time 
 */
typedef struct {
	/**
	 * The difference between local time and GMT in hours
	 */
	int			Timezone;

	unsigned int		Second;
	unsigned int 		Minute;
	unsigned int		Hour;

	unsigned int 		Day;
	/**
	 * January = 1, February = 2, etc.
	 */
	unsigned int 		Month;
	/**
	 * Complete year number. Not 03, but 2003
	 */
	unsigned int		Year;
} GSM_DateTime;

void GSM_GetCurrentDateTime 	(GSM_DateTime *Date);
char *OSDateTime 		(GSM_DateTime dt, bool TimeZone);
char *OSDate 			(GSM_DateTime dt);
char *DayOfWeek 		(int year, int month, int day);
time_t Fill_Time_T		(GSM_DateTime DT, int TZ);
void GetTimeDifference		(unsigned long diff, GSM_DateTime *DT, bool Plus, int multi);
void Fill_GSM_DateTime		(GSM_DateTime *Date, time_t timet);
bool CheckDate			(GSM_DateTime *date);
bool CheckTime			(GSM_DateTime *date);

char 				*GetCompiler(void);
char 				*GetOS(void);

#if defined(_MSC_VER) && defined(__cplusplus)
    }
#endif

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

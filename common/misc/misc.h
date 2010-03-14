
#ifndef __misc_h
#define __misc_h

#include <stdio.h>
#include <time.h>
#ifdef WIN32
#  include <windows.h>
#endif

#include "../config.h"

#ifndef false
#  define false (0)
#endif

#ifndef true
#  define true (!false)
#endif

#ifndef bool    
#  define bool int
#endif

#ifdef WIN32
#  define my_sleep(x) Sleep(((x) < 1000) ? 1 : ((x) / 1000))
#else
#  define my_sleep(x) usleep(x)
#endif

#undef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#undef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))

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
} Debug_Info;

extern Debug_Info	di;

#ifdef DEBUG
int dprintf(const char *format, ...);
#else
#  ifndef WIN32
#    define dprintf(a...) do { } while (0)
#  else
#    define dprintf
#  endif
#endif

int smfprintf(FILE *f, const char *format, ...);

void DumpMessage(FILE *df, const unsigned char *message, int messagesize);

bool GSM_SetDebugLevel(char *info, Debug_Info *di);

/* ------------------------------------------------------------------------- */

/* Structure used for passing dates/times to date/time functions such as
   GSM_GetTime and GSM_GetAlarm etc. */
typedef struct {
	unsigned int	Year;		/* The complete year specification - e.g.1999.Y2K :-) */
	unsigned int 	Month;		/* January = 1 */
	unsigned int 	Day;
	unsigned int	Hour;
	unsigned int 	Minute;
	unsigned int	Second;
	int		Timezone;	/* The difference between local time and GMT */
} GSM_DateTime;

void GSM_GetCurrentDateTime 	(GSM_DateTime *Date);
char *OSDateTime 		(GSM_DateTime dt, bool TimeZone);
char *OSDate 			(GSM_DateTime dt);
char *DayOfWeek 		(int year, int month, int day);
time_t Fill_Time_T		(GSM_DateTime DT, int TZ);
void GetTimeDifference		(unsigned long diff, GSM_DateTime *DT, bool Plus, int multi);
void Fill_GSM_DateTime		(GSM_DateTime *Date, time_t timet);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

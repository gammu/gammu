/* (c) 2002-2005 by Marcin Wiacek and Michal Cihar */
/* Checking used compiler (c) 2002 by Michal Cihar */

#include <gammu-config.h>
#include <gammu-misc.h>

#include "misc.h"

#include "coding/coding.h"
#include "../debug.h"

#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  include <locale.h>
#  define gmtime_r(x, y) gmtime_s(y, x)
#  define localtime_r(x, y) localtime_s(y, x)
#endif
#ifdef HAVE_SYS_UTSNAME_H
#  include <sys/utsname.h>
#endif
#ifdef __CYGWIN__
#include <cygwin/version.h>
#endif

#ifdef WIN32
int setenv(const char *name, const char *value, int overwrite)
{
  char env_var[_MAX_ENV];
  char* has_value = getenv(name);
  if(!has_value || overwrite) {
    snprintf(env_var, _MAX_ENV, "%s=%s", name, value);
    return _putenv(env_var);
  }
  return -1;
}

void unsetenv(const char *name)
{
  _putenv(name);
}
#endif // WIN32

/**
 * Recalculates struct tm content. We can not use mktime directly, as it
 * works only for dates > 1970. Day of week calculation is nased on article
 * in Polish PC-Kurier 8/1998 page 104.
 *
 * @see http://www.pckurier.pl
 */
int RecalcDateTime(struct tm *st, const int year, const int month, const int day, const int hour, const int minute, const int second)
{
	const int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
	int i, p, q, r;
	GSM_DateTime Date;

	Date.Year	= year;
	Date.Month	= month;
	Date.Day	= day;
	Date.Hour	= hour;
	Date.Minute	= minute;
	Date.Second	= second;
	Date.Timezone	= 0;

	if (!CheckDate(&Date) || !CheckTime(&Date)) return 0;

	memset(st, 0, sizeof(*st));

	/* Calculate day of year */
	st->tm_yday = day;
	for (i = 0; i < month - 1; i++)
		st->tm_yday += days[i];

	/* Calculate day of week */
	p = (14 - month) / 12;
	q = month + 12 * p - 2;
	r = year - p;
	st->tm_wday = (day + (31 * q) / 12 + r + r / 4 - r / 100 + r / 400) % 7;


	st->tm_hour = hour;
	st->tm_min = minute;
	st->tm_sec = second;
	st->tm_year = year - 1900;
	st->tm_mon = month - 1;
	st->tm_mday = day;

	st->tm_isdst = -1; /* FIXME */

	return 1;
}


/**
 * Recalculates struct tm content.
 */
int RecalcDate(struct tm *st, const int year, const int month, const int day)
{
	return RecalcDateTime(st, year, month, day, 0, 0, 0);
}


/**
 * Return day of year index.
 */
int GetDayOfYear(unsigned int year, unsigned int month, unsigned int day)
{
	struct tm st;

	RecalcDate(&st, year, month, day);

	return st.tm_yday;
}

/**
 * Return day of week index.
 */
int GetWeekOfMonth(unsigned int year, unsigned int month, unsigned int day)
{
	struct tm st;

	RecalcDate(&st, year, month, day);

	return 1 + (day - st.tm_wday) / 7;
}

/**
 * Return day of week index.
 */
int GetDayOfWeek(unsigned int year, unsigned int month, unsigned int day)
{
	struct tm st;

	RecalcDate(&st, year, month, day);

	return st.tm_wday;
}

/**
 * Return textual representation of day of week;
 */
char *DayOfWeek (unsigned int year, unsigned int month, unsigned int day)
{
	static char 	DayOfWeekChar[10];

	strcpy(DayOfWeekChar,"");
	switch (GetDayOfWeek(year, month, day)) {
		case 0: strcpy(DayOfWeekChar,"Sun"); break;
		case 1: strcpy(DayOfWeekChar,"Mon"); break;
		case 2: strcpy(DayOfWeekChar,"Tue"); break;
		case 3: strcpy(DayOfWeekChar,"Wed"); break;
		case 4: strcpy(DayOfWeekChar,"Thu"); break;
		case 5: strcpy(DayOfWeekChar,"Fri"); break;
		case 6: strcpy(DayOfWeekChar,"Sat"); break;
	}
	return DayOfWeekChar;
}

int get_local_timezone_offset(time_t posix_time)
{
  struct tm *tm = gmtime(&posix_time);
  tm->tm_isdst = -1;
  return posix_time - mktime(tm);
}

int GSM_GetLocalTimezoneOffset()
{
  return get_local_timezone_offset(time(NULL));
}

void GSM_DateTimeToTimestamp(GSM_DateTime *Date, char *str)
{
	time_t timet;
	timet = Fill_Time_T(*Date);
	sprintf(str, "%ld", (long)timet);
}

void GSM_DateTimeFromTimestamp(GSM_DateTime *Date, const char *str)
{
	time_t timet;

	timet = atof(str);
	Fill_GSM_DateTime(Date, timet);
}

void Fill_GSM_DateTime(GSM_DateTime *Date, time_t timet)
{
	struct tm *now;

	now  		= localtime(&timet);
	Date->Year	= now->tm_year + 1900;
	Date->Month	= now->tm_mon + 1;
	Date->Day	= now->tm_mday;
	Date->Hour	= now->tm_hour;
	Date->Minute	= now->tm_min;
	Date->Second	= now->tm_sec;
	Date->Timezone	= GSM_GetLocalTimezoneOffset();
}

void GSM_GetCurrentDateTime (GSM_DateTime *Date)
{
	Fill_GSM_DateTime(Date, time(NULL));
}

/*
 * Convert GSM_DateTime to POSIX time.
 *
 * A GSM timestamp consists of a date and time given in UTC and
 * an originating local time offset. If the time and date part
 * of the GSM_DateTime argument is not in UTC the returned
 * calculated POSIX calendar time will not be valid.
 */
time_t Fill_Time_T(GSM_DateTime DT)
{
  char *tz;
	struct tm tm;
	time_t posix_time;

	dbgprintf(NULL, "StartTime: %s\n", OSDate(DT));

	memset(&tm, 0, sizeof(tm));
	tm.tm_year 	= DT.Year - 1900;
	tm.tm_mon  	= DT.Month - 1;
	tm.tm_mday 	= DT.Day;
	tm.tm_hour 	= DT.Hour;
	tm.tm_min  	= DT.Minute;
	tm.tm_sec  	= DT.Second;
	tm.tm_isdst = 0;

	tz = getenv("TZ");
	if(tz) {
    tz = strdup(tz);
    if(!tz)
      return -1;
  }

	putenv((char*)"TZ=GMT+00");
	tzset();

	posix_time = mktime(&tm);
	if(posix_time != -1)
    posix_time += DT.Timezone > 0 ? -DT.Timezone : abs(DT.Timezone);

	if(tz) {
    setenv("TZ", tz, 1);
    free(tz);
  }
	else {
	  unsetenv("TZ");
	}
	tzset();

	return posix_time;
}

GSM_DateTime GSM_AddTime (GSM_DateTime DT , GSM_DeltaTime delta)
{
	struct tm tm_time;
	time_t t_time;
	GSM_DateTime Date;

	memset(&tm_time, 0, sizeof(tm_time));
	tm_time.tm_year 	= DT.Year - 1900;
	tm_time.tm_mon  	= DT.Month - 1;
	tm_time.tm_mday 	= DT.Day;
	tm_time.tm_hour 	= DT.Hour;
	tm_time.tm_min  	= DT.Minute;
	tm_time.tm_sec  	= DT.Second;
	tm_time.tm_isdst	= -1;

	/* TODO: This works only for dates after 1970. But birthday dates may be before, so a more general
	   method than mktime /localtime should be used. */
	t_time = mktime (&tm_time);
	t_time = t_time + delta.Second + 60* (delta.Minute + 60* (delta.Hour + 24*delta.Day));

	Fill_GSM_DateTime ( &Date, t_time);
	return Date;
}

void GetTimeDifference(unsigned long diff, GSM_DateTime *DT, gboolean Plus, int multi)
{
	time_t t_time;

	t_time = Fill_Time_T(*DT);

	if (Plus) {
		t_time 		+= diff*multi;
	} else {
		t_time 		-= diff*multi;
	}

	Fill_GSM_DateTime(DT, t_time);
	dbgprintf(NULL, "EndTime: %02i-%02i-%04i %02i:%02i:%02i\n",
		DT->Day,DT->Month,DT->Year,DT->Hour,DT->Minute,DT->Second);
}

char *OSDateTime (GSM_DateTime dt, gboolean TimeZone)
{
	struct tm 	timeptr;
	static char 	retval[200],retval2[200];

	if (!RecalcDateTime(&timeptr, dt.Year, dt.Month, dt.Day,
				dt.Hour, dt.Minute, dt.Second)) {
		retval2[0] = '\0';
		return retval2;
	}

#ifdef WIN32
	setlocale(LC_ALL, ".OCP");
#endif

	/* This is not Y2K safe */
	strftime(retval2, 200, "%c", &timeptr);
	if (TimeZone) {
		snprintf(retval, sizeof(retval) - 1, " %+03i%02i",
			dt.Timezone / 3600, abs((dt.Timezone % 3600) / 60));
		strcat(retval2,retval);
	}
	/* If don't have weekday name, include it */
	strftime(retval, 200, "%A", &timeptr);
	if (strstr(retval2,retval)==NULL) {
		/* Check for abbreviated weekday */
		strftime(retval, 200, "%a", &timeptr);
		if (strstr(retval2,retval)==NULL) {
			strcat(retval2," (");
			strcat(retval2,retval);
			strcat(retval2,")");
		}
	}

#ifdef WIN32
	setlocale(LC_ALL, ".ACP");
#endif

	return retval2;
}

char *OSDate (GSM_DateTime dt)
{
	struct tm 	timeptr;
	static char 	retval[200],retval2[200];

#ifdef WIN32
	setlocale(LC_ALL, ".OCP");
#endif

	timeptr.tm_yday 	= 0; 			/* FIXME */
	timeptr.tm_isdst 	= -1; 			/* FIXME */
	timeptr.tm_year 	= dt.Year - 1900;
	timeptr.tm_mon  	= dt.Month - 1;
	timeptr.tm_mday 	= dt.Day;
	timeptr.tm_hour 	= dt.Hour;
	timeptr.tm_min  	= dt.Minute;
	timeptr.tm_sec  	= dt.Second;
	timeptr.tm_wday 	= GetDayOfWeek(dt.Year, dt.Month, dt.Day);
#ifdef HAVE_STRUCT_TM_TM_ZONE
	timeptr.tm_zone		= NULL;
#endif

	/* This is not Y2K safe */
	strftime(retval2, 200, "%x", &timeptr);

	/* If don't have weekday name, include it */
	strftime(retval, 200, "%A", &timeptr);
	if (strstr(retval2,retval)==NULL) {
		/* Check also for short name */
		strftime(retval, 200, "%a", &timeptr);
		if (strstr(retval2,retval)==NULL) {
            		strcat(retval2," (");
            		strcat(retval2,retval);
            		strcat(retval2,")");
            	}
	}

#ifdef WIN32
	setlocale(LC_ALL, ".ACP");
#endif

	return retval2;
}

gboolean CheckDate(GSM_DateTime *date)
{
	const int days[]={31,28,31,30,31,30,31,31,30,31,30,31};

	if (date->Year != 0 &&
	    ((date->Year % 4 == 0 && date->Year % 100 != 0) || date->Year % 400 == 0) &&
	    date->Month == 2) {
		return (date->Day <= 29);
	}
	return date->Year != 0 &&
	       date->Month >= 1 && date->Month <= 12 &&
	       date->Day >= 1 && date->Day <= days[date->Month-1];
}

gboolean CheckTime(GSM_DateTime *date)
{
	return date->Hour <= 23 &&
		date->Minute <= 59 &&
		date->Second <= 59;
}

size_t GetLine(FILE *File, char *Line, int count)
{
	int num;

	if (fgets(Line, count, File) != NULL) {
		num = strlen(Line) - 1;
		while (num > 0) {
			if (Line[num] != '\n' && Line[num] != '\r') break;
			Line[num--] = '\0';
		}
		return strlen(Line);
	}
	return -1;
}

void InitLines(GSM_CutLines *lines)
{
	lines->numbers = NULL;
	lines->allocated = 0;
	lines->retval = NULL;
}

void FreeLines(GSM_CutLines *lines)
{
	free(lines->numbers);
	lines->numbers = NULL;
	lines->allocated = 0;
	free(lines->retval);
	lines->retval = NULL;
}

void SplitLines(const char *message, const size_t messagesize, GSM_CutLines *lines,
	const char *whitespaces, const size_t spaceslen,
	const char *quotes, const size_t quoteslen,
	const gboolean eot)
{
	size_t 	 i=0,number=0,j=0, lastquote = 0;
	gboolean whitespace = TRUE, nowwhite = FALSE, insidequotes = FALSE;

	/* Clean current lines */
	for (i = 0; i < lines->allocated; i++) {
		lines->numbers[i] = 0;
	}

	/* Go through message */
	for (i = 0; i < messagesize; i++) {
		/* Reallocate buffer if needed */
		if (number + 2 >= lines->allocated) {
			lines->allocated += 20;
			lines->numbers = (size_t *)realloc(lines->numbers, lines->allocated * sizeof(size_t));
			if (lines->numbers == NULL) {
				return;
			}
			for (j = lines->allocated - 20; j < lines->allocated; j++) {
				lines->numbers[j] = 0;
			}
		}

		nowwhite = FALSE;

		/* Check for quotes */
		for (j = 0; j < quoteslen; j++) {
			if (quotes[j] == message[i]) {
				insidequotes = !(insidequotes);
				lastquote = i;
				break;
			}
		}

		/* Find matching quote */
		if (insidequotes) {
			continue;
		}

rollback_quote:
		/* Check for whitespace */
		for (j = 0; j < spaceslen; j++) {
			if (whitespaces[j] == message[i]) {
				nowwhite = TRUE;
				break;
			}
		}

		/* Split line if there is change from whitespace to non whitespace */
		if (whitespace) {
			if (!nowwhite) {
				lines->numbers[number] = i;
				number++;
				whitespace = FALSE;
			}
		} else {
			if (nowwhite) {
				lines->numbers[number] = i;
				number++;
				whitespace = TRUE;
			}

		}
	}

	/* Check for unterminated quotes and roll back to ignore them */
	if (number % 2 == 1 && insidequotes) {
		insidequotes = FALSE;
		i = lastquote;
		goto rollback_quote;
	}

	/* Store possible end if there was not just whitespace */
    	if (eot && !whitespace) {
		lines->numbers[number] = messagesize;
	}
}

const char *GetLineStringPos(const char *message, const GSM_CutLines *lines, int start)
{
	if (message == NULL) {
		return NULL;
	}

	return message + lines->numbers[start * 2 - 2];
}

const char *GetLineString(const char *message, GSM_CutLines *lines, int start)
{
	int len=0;
	const char *pos;

	pos = GetLineStringPos(message, lines, start);
	if (pos == NULL) {
		return NULL;
	}

	len = GetLineLength(message, lines, start);

	lines->retval = (char *)realloc(lines->retval, len + 1);
	if (lines->retval == NULL) {
		dbgprintf(NULL, "Allocation failed!\n");
		return NULL;
	}

	memcpy(lines->retval, pos, len);

	lines->retval[len] = '\0';

	return lines->retval;
}

int GetLineLength(const char *message UNUSED, const GSM_CutLines *lines, int start)
{
	return lines->numbers[start*2-2+1] - lines->numbers[start*2-2];
}

void CopyLineString(char *dest, const char *src, const GSM_CutLines *lines, int start)
{
	int len;
	const char *pos;

	len = GetLineLength(src, lines, start);
	pos = GetLineStringPos(src, lines, start);
	if (pos == NULL) {
		dest[0] = '\0';
		return;
	}
	memcpy(dest, pos, len);
	dest[len] = '\0';
}

const char *GetOS(void)
{
#ifdef WIN32
	OSVERSIONINFOEX Ver;
	gboolean		Extended = TRUE;
#else
#  ifdef HAVE_SYS_UTSNAME_H
	struct utsname	Ver;
#  endif
#endif
	static char 	Buffer[240] = {0x00};

	/* Value was already calculated */
	if (Buffer[0] != 0) return Buffer;

#ifdef WIN32
	memset(&Ver,0,sizeof(OSVERSIONINFOEX));
	Ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

   	if (!GetVersionEx((OSVERSIONINFO *)&Ver)) {
		Extended 		= FALSE;
	      	Ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	        if (!GetVersionEx((OSVERSIONINFO *)&Ver)) {
			snprintf(Buffer, sizeof(Buffer) - 1, "Windows");
			return Buffer;
		}
	}

	/* ----------------- 9x family ------------------ */

	/* no info about Win95 SP1, Win95 OSR2.1, Win95 OSR2.5.... */
	if (Ver.dwMajorVersion == 4 && Ver.dwMinorVersion == 0 && Ver.dwBuildNumber == 950) {
		snprintf(Buffer, sizeof(Buffer) - 1, "Windows 95");
	} else if (Ver.dwMajorVersion == 4 && Ver.dwMinorVersion == 0 && Ver.dwBuildNumber == 1111) {
		snprintf(Buffer, sizeof(Buffer) - 1, "Windows 95 OSR2.x");

	/* no info about Win98 SP1.... */
	} else if (Ver.dwMajorVersion == 4 && Ver.dwMinorVersion == 10 && Ver.dwBuildNumber == 1998) {
		snprintf(Buffer, sizeof(Buffer) - 1, "Windows 98");
	} else if (Ver.dwMajorVersion == 4 && Ver.dwMinorVersion == 10 && Ver.dwBuildNumber == 2222) {
		snprintf(Buffer, sizeof(Buffer) - 1, "Windows 98 SE");

	} else if (Ver.dwMajorVersion == 4 && Ver.dwMinorVersion == 90 && Ver.dwBuildNumber == 3000) {
		snprintf(Buffer, sizeof(Buffer) - 1, "Windows ME");

	/* ---------------- NT family ------------------- */

	} else if (Ver.dwMajorVersion == 4 && Ver.dwMinorVersion == 0 && Ver.dwBuildNumber == 1381) {
		snprintf(Buffer, sizeof(Buffer) - 1, "Windows NT 4.0");

	} else if (Ver.dwMajorVersion == 5 && Ver.dwMinorVersion == 0 && Ver.dwBuildNumber == 2195) {
		snprintf(Buffer, sizeof(Buffer) - 1, "Windows 2000");

	} else if (Ver.dwMajorVersion == 5 && Ver.dwMinorVersion == 1 && Ver.dwBuildNumber == 2600) {
		snprintf(Buffer, sizeof(Buffer) - 1, "Windows XP");
#if _MSC_VER > 1200 /* 6.0 has it undeclared */
		if (Extended) {
			if (Ver.wSuiteMask & VER_SUITE_PERSONAL) {
				snprintf(Buffer+strlen(Buffer), sizeof(Buffer) - 1 - strlen(Buffer)," Home");
			} else {
				snprintf(Buffer+strlen(Buffer), sizeof(Buffer) - 1 - strlen(Buffer)," Pro");
			}
		}
#endif

	} else if (Ver.dwMajorVersion == 5 && Ver.dwMinorVersion == 2) {
		snprintf(Buffer, sizeof(Buffer) - 1, "Windows 2003");

	} else if (Ver.dwMajorVersion == 6 && Ver.dwMinorVersion == 0) {
		snprintf(Buffer, sizeof(Buffer) - 1, "Windows Vista");

	} else if (Ver.dwMajorVersion == 6 && Ver.dwMinorVersion > 0) {
		snprintf(Buffer, sizeof(Buffer) - 1, "Windows Server 2007");

	} else {
		snprintf(Buffer, sizeof(Buffer) - 1, "Windows %i.%i.%i",(int)Ver.dwMajorVersion,(int)Ver.dwMinorVersion,(int)Ver.dwBuildNumber);
	}

	if (Extended && Ver.wServicePackMajor != 0) {
		snprintf(Buffer+strlen(Buffer), sizeof(Buffer) - 1 - strlen(Buffer)," SP%i",Ver.wServicePackMajor);
	}
#elif defined(HAVE_SYS_UTSNAME_H)
	uname(&Ver);
	snprintf(Buffer, sizeof(Buffer) - 1, "%s, kernel %s (%s)", Ver.sysname, Ver.release, Ver.version);
#elif defined(__FreeBSD__)
	snprintf(Buffer, sizeof(Buffer) - 1, "FreeBSD");
#elif defined(__NetBSD__)
	snprintf(Buffer, sizeof(Buffer) - 1, "NetBSD");
#elif defined(__OpenBSD__)
	snprintf(Buffer, sizeof(Buffer) - 1, "OpenBSD");
#elif defined(__GNU__)
	snprintf(Buffer, sizeof(Buffer) - 1, "GNU/Hurd");
#elif defined(sun) || defined(__sun) || defined(__sun__)
#  ifdef __SVR4
	snprintf(Buffer, sizeof(Buffer) - 1, "Sun Solaris");
#  else
	snprintf(Buffer, sizeof(Buffer) - 1, "SunOS");
#  endif
#elif defined(hpux) || defined(__hpux) || defined(__hpux__)
	snprintf(Buffer, sizeof(Buffer) - 1, "HP-UX");
#elif defined(ultrix) || defined(__ultrix) || defined(__ultrix__)
	snprintf(Buffer, sizeof(Buffer) - 1, "DEC Ultrix");
#elif defined(sgi) || defined(__sgi)
	snprintf(Buffer, sizeof(Buffer) - 1, "SGI Irix");
#elif defined(__osf__)
	snprintf(Buffer, sizeof(Buffer) - 1, "OSF Unix");
#elif defined(bsdi) || defined(__bsdi__)
	snprintf(Buffer, sizeof(Buffer) - 1, "BSDI Unix");
#elif defined(_AIX)
	snprintf(Buffer, sizeof(Buffer) - 1, "AIX Unix");
#elif defined(_UNIXWARE)
	snprintf(Buffer, sizeof(Buffer) - 1, "SCO Unixware");
#elif defined(DGUX)
	snprintf(Buffer, sizeof(Buffer) - 1, "DG Unix");
#elif defined(__QNX__)
	snprintf(Buffer, sizeof(Buffer) - 1, "QNX");
#else
	snprintf(Buffer, sizeof(Buffer) - 1, "unknown OS");
#endif
	return Buffer;
}

const char *GetCompiler(void)
{
	static char Buffer[100] = {0x00};

	/* Value was already calculated */
	if (Buffer[0] != 0) return Buffer;

#ifdef _MSC_VER
	if (_MSC_VER == 1200) { /* ? */
		snprintf(Buffer, sizeof(Buffer) - 1, "MS VC 6.0");
	} else if (_MSC_VER == 1300) {
		snprintf(Buffer, sizeof(Buffer) - 1, "MS VC .NET 2002");
	} else if (_MSC_VER == 1310) {
		snprintf(Buffer, sizeof(Buffer) - 1, "MS VC .NET 2003");
	} else if (_MSC_VER == 1400) {
		snprintf(Buffer, sizeof(Buffer) - 1, "MS VC .NET 2005");
	} else {
		snprintf(Buffer, sizeof(Buffer) - 1, "MS VC %i",_MSC_VER);
	}
#elif defined(__BORLANDC__)
	snprintf(Buffer, sizeof(Buffer) - 1, "Borland C++ %i",__BORLANDC__);
#elif defined(__MINGW32__)
	snprintf(Buffer, sizeof(Buffer) - 1, "GCC %i.%i, MinGW %i.%i", __GNUC__, __GNUC_MINOR__, __MINGW32_MAJOR_VERSION, __MINGW32_MINOR_VERSION);
#elif defined(__CYGWIN__)
	snprintf(Buffer, sizeof(Buffer) - 1, "GCC %i.%i, Cygwin %i.%i", __GNUC__, __GNUC_MINOR__, CYGWIN_VERSION_DLL_MAJOR, CYGWIN_VERSION_DLL_MINOR);
#elif defined(__GNUC__)
	snprintf(Buffer, sizeof(Buffer) - 1, "GCC %i.%i", __GNUC__, __GNUC_MINOR__);
#elif defined(DJGPP)
	snprintf(Buffer, sizeof(Buffer) - 1, "djgpp %d.%d", __DJGPP, __DJGPP_MINOR);
#elif defined(__SUNPRO_CC)
	snprintf(Buffer, sizeof(Buffer) - 1, "Sun C++ %x", __SUNPRO_CC);
#elif defined(__INTEL_COMPILER)
	snprintf(Buffer, sizeof(Buffer) - 1, "Intel Compiler %ld", __INTEL_COMPILER);
#else
	snprintf(Buffer, sizeof(Buffer) - 1, "unknown compiler");
#endif

	return Buffer;
}

gboolean GSM_IsNewerVersion(const char *latest_version, const char *current_version)
{
	size_t i;
	size_t len = strlen(latest_version);

	for (i = 0; i < len ; i++) {
		if (latest_version[i] > current_version[i]) {
			return TRUE;
		}
	}

	return FALSE;
}

void StripSpaces(char *buff)
{
	ssize_t i = 0;

	while(isspace(buff[i])) {
		i++;
	}
	if (i > 0) {
		memmove(buff, buff + i, strlen(buff + i));
	}
	i = strlen(buff) - 1;
	while(isspace(buff[i]) && i >= 0) {
		buff[i] = 0;
		i--;
	}

}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

/* (c) 2002-2003 by Marcin Wiacek and Michal Cihar */

#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <locale.h>
#include <sys/timeb.h>
#ifdef WIN32
#  include "windows.h"
#endif

#include "../gsmstate.h"
#include "misc.h"

/* Based on article in Polish PC-Kurier 8/1998 page 104
 * Archive on http://www.pckurier.pl
 */
char *DayOfWeek (int year, int month, int day)
{
	int 		p,q,r,w;
	static char 	DayOfWeekChar[10];

	p=(14-month) / 12;
	q=month+12*p-2;
	r=year-p;
	w=(day+(31*q) / 12 + r + r / 4 - r / 100 + r / 400) % 7;
	strcpy(DayOfWeekChar,"");
	switch (w) {
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

void Fill_GSM_DateTime(GSM_DateTime *Date, time_t timet)
{
	struct tm *now;

	now  		= localtime(&timet);
	Date->Year	= now->tm_year;
	Date->Month	= now->tm_mon+1;
	Date->Day	= now->tm_mday;
	Date->Hour	= now->tm_hour;
	Date->Minute	= now->tm_min;
	Date->Second	= now->tm_sec;	
}

void GSM_GetCurrentDateTime (GSM_DateTime *Date)
{
	Fill_GSM_DateTime(Date, time(NULL));
	if (Date->Year<1900) {
		if (Date->Year>90) Date->Year = Date->Year+1900;
			      else Date->Year = Date->Year+2000;
	}
}

time_t Fill_Time_T(GSM_DateTime DT, int TZ)
{
	struct tm 	tm_starttime;
	unsigned char 	buffer[30];

	dbgprintf("  StartTime  : %02i-%02i-%04i %02i:%02i:%02i\n",
		DT.Day,DT.Month,DT.Year,DT.Hour,DT.Minute,DT.Second);

	if (TZ != 0) {
#if defined(WIN32) || defined(__SVR4)
	    sprintf(buffer,"TZ=PST+%i",TZ);
	    putenv(buffer);
#else
	    sprintf(buffer,"PST+%i",TZ);
	    setenv("TZ",buffer,1);
#endif
	}
	tzset();

	memset(&tm_starttime, 0, sizeof(tm_starttime));
	tm_starttime.tm_year 	= DT.Year - 1900;
	tm_starttime.tm_mon  	= DT.Month - 1;
	tm_starttime.tm_mday 	= DT.Day;
	tm_starttime.tm_hour 	= DT.Hour;
	tm_starttime.tm_min  	= DT.Minute;
	tm_starttime.tm_sec  	= DT.Second;
	tm_starttime.tm_isdst	= 0;
	
	return mktime(&tm_starttime);
}

void GetTimeDifference(unsigned long diff, GSM_DateTime *DT, bool Plus, int multi)
{
	time_t t_time;

	t_time = Fill_Time_T(*DT,8);

	if (Plus) {
		t_time 		+= diff*multi;
	} else {
		t_time 		-= diff*multi;
	}

	Fill_GSM_DateTime(DT, t_time);
	DT->Year = DT->Year + 1900;
	dbgprintf("  EndTime    : %02i-%02i-%04i %02i:%02i:%02i\n",
		DT->Day,DT->Month,DT->Year,DT->Hour,DT->Minute,DT->Second);
}

char *OSDateTime (GSM_DateTime dt, bool TimeZone)
{
	struct tm 	timeptr;
	static char 	retval[200],retval2[200];
	int 		p,q,r,w;

#ifdef WIN32
	setlocale(LC_ALL, ".OCP");
#endif

	/* Based on article in Polish PC-Kurier 8/1998 page 104
	 * Archive on http://www.pckurier.pl
	 */
	p=(14-dt.Month) / 12;
	q=dt.Month+12*p-2;
	r=dt.Year-p;
	w=(dt.Day+(31*q) / 12 + r + r / 4 - r / 100 + r / 400) % 7;

	timeptr.tm_yday 	= 0; 			/* FIXME */
	timeptr.tm_isdst 	= -1; 			/* FIXME */
	timeptr.tm_year 	= dt.Year - 1900;
	timeptr.tm_mon  	= dt.Month - 1;
	timeptr.tm_mday 	= dt.Day;
	timeptr.tm_hour 	= dt.Hour;
	timeptr.tm_min  	= dt.Minute;
	timeptr.tm_sec  	= dt.Second;
	timeptr.tm_wday 	= w;
#ifdef _BSD_SOURCE
	timeptr.tm_zone		= NULL;
#endif

#ifdef WIN32
	strftime(retval2, 200, "%#c", &timeptr);
#else
	strftime(retval2, 200, "%c", &timeptr);
#endif
	if (TimeZone) {
		if (dt.Timezone >= 0) {
			sprintf(retval," +%02i00",dt.Timezone);
		} else {
			sprintf(retval," -%02i00",dt.Timezone);
		}
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
	int 		p,q,r,w;

#ifdef WIN32
	setlocale(LC_ALL, ".OCP");
#endif

	/* Based on article in Polish PC-Kurier 8/1998 page 104
	 * Archive on http://www.pckurier.pl
	 */
	p=(14-dt.Month) / 12;
	q=dt.Month+12*p-2;
	r=dt.Year-p;
	w=(dt.Day+(31*q) / 12 + r + r / 4 - r / 100 + r / 400) % 7;

	timeptr.tm_yday 	= 0; 			/* FIXME */
	timeptr.tm_isdst 	= -1; 			/* FIXME */
	timeptr.tm_year 	= dt.Year - 1900;
	timeptr.tm_mon  	= dt.Month - 1;
	timeptr.tm_mday 	= dt.Day;
	timeptr.tm_hour 	= dt.Hour;
	timeptr.tm_min  	= dt.Minute;
	timeptr.tm_sec  	= dt.Second;
	timeptr.tm_wday 	= w;
#ifdef _BSD_SOURCE
	timeptr.tm_zone		= NULL;
#endif

#ifdef WIN32
	strftime(retval2, 200, "%#x", &timeptr);
#else
	strftime(retval2, 200, "%x", &timeptr);
#endif
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

bool CheckDate(GSM_DateTime *date)
{
	/* FIXME: This could also check if day is correct for selected month */
	return date->Year != 0 &&
		date->Month >= 1 && date->Month <= 12 &&
		date->Day >= 1 && date->Day <= 31;
}

bool CheckTime(GSM_DateTime *date)
{
	return date->Hour <= 23 && date->Hour >= 0 &&
		date->Minute <= 59 && date->Minute >= 0 &&
		date->Second <= 59 && date->Second >= 0;
}

int GetLine(FILE *File, char *Line, int count)
{
	int num;

	if (fgets(Line, count, File) != NULL) {
		num = strlen(Line) - 1;
		while(1) {
			if (Line[num] != '\n' && Line[num] != '\r') break;
			if (num == 0) break;
			Line[num--] = 0;
		}
		return strlen(Line);
	}
	return -1;
}

void SplitLines(unsigned char *message, int messagesize, GSM_Lines *lines, unsigned char *whitespaces, int spaceslen, bool eot)
{
	int 	i,number=0,j;
	bool 	whitespace=true, nowwhite;

	for (i=0;i<MAX_LINES*2;i++) lines->numbers[i]=0;

	for (i=0;i<messagesize;i++) {
		nowwhite = false;
		for (j=0;j<spaceslen;j++) {
			if (whitespaces[j] == message[i]) {
				nowwhite = true;
				break;
			}
		}
		if (whitespace) {			
			if (!nowwhite) {
				lines->numbers[number]=i;
				number++;
				whitespace=false;
			}
		} else {
			if (nowwhite) {
				lines->numbers[number]=i;
				number++;
				whitespace=true;
			}

		}
	}
    	if (eot && !whitespace) lines->numbers[number]=messagesize;
}

char *GetLineString(unsigned char *message, GSM_Lines lines, int start)
{
	static char retval[800];

	memcpy(retval,message + lines.numbers[start*2-2],lines.numbers[start*2-2+1]-lines.numbers[start*2-2]);
	retval[lines.numbers[start*2-2+1]-lines.numbers[start*2-2]]=0;

	return retval;
}

void CopyLineString(unsigned char *dest, unsigned char *src, GSM_Lines lines, int start)
{
	memcpy(dest,GetLineString(src, lines, start),strlen(GetLineString(src, lines, start)));
	dest[strlen(GetLineString(src, lines, start))] = 0;
}

Debug_Info di = {0,NULL,false,""};

#ifdef DEBUG
#ifdef __GNUC__
__attribute__((format(printf, 1, 2)))
#endif
int dbgprintf(const char *format, ...)
{
	va_list			argp;
	int 			result;
	static unsigned char 	nextline[2000]="";
	unsigned char		buffer[2000];
	GSM_DateTime 		date_time;

	if (di.df != NULL && (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE)) {
		va_start(argp, format);
		result = vsprintf(buffer, format, argp);
		strcat(nextline, buffer);
		if (strstr(buffer, "\n")) {
			if (di.dl == DL_TEXTALLDATE) {
				GSM_GetCurrentDateTime(&date_time);
				fprintf(di.df,"%s %4d/%02d/%02d %02d:%02d:%02d: %s",
		                	DayOfWeek(date_time.Year, date_time.Month, date_time.Day),
		                	date_time.Year, date_time.Month, date_time.Day,
		                	date_time.Hour, date_time.Minute, date_time.Second,nextline);
			} else {
				fprintf(di.df,"%s",nextline);
			}
			strcpy(nextline, "");
		}
		fflush(di.df);
		va_end(argp);
		return result;
	}
	return 0;
}
#endif

/* assumption: if \n is present it is always the last char,
 * string never of the form "......\n..."
 */
#ifdef __GNUC__
__attribute__((format(printf, 3, 4)))
#endif
int smfprintf(FILE *f, Debug_Level dl, const char *format, ...)
{
        va_list 		argp;
	int 			result=0;
	static unsigned char 	prevline[2000] = "", nextline[2000]="";
	static unsigned int 	linecount=0;
	unsigned char		buffer[2000];
	GSM_DateTime 		date_time;

	if (f == NULL) return 0;
	va_start(argp, format);
	result = vsprintf(buffer, format, argp);
	strcat(nextline, buffer);
	if (strstr(buffer, "\n")) {
		if (ftell(f) < 5000000) {
			GSM_GetCurrentDateTime(&date_time);
			if (linecount > 0) {
				if (dl == DL_TEXTALLDATE || dl == DL_TEXTERRORDATE || dl == DL_TEXTDATE) {
			                fprintf(f,"%s %4d/%02d/%02d %02d:%02d:%02d: <%i> %s",
			                        DayOfWeek(date_time.Year, date_time.Month, date_time.Day),
			                        date_time.Year, date_time.Month, date_time.Day,
			                        date_time.Hour, date_time.Minute, date_time.Second,linecount,prevline);
				} else {
			                fprintf(f,"%s",prevline);
				}
			}
			linecount=0;
			if (dl == DL_TEXTALLDATE || dl == DL_TEXTERRORDATE || dl == DL_TEXTDATE) {
		                fprintf(f,"%s %4d/%02d/%02d %02d:%02d:%02d: %s",
		                        DayOfWeek(date_time.Year, date_time.Month, date_time.Day),
		                        date_time.Year, date_time.Month, date_time.Day,
		                        date_time.Hour, date_time.Minute, date_time.Second,nextline);
			} else {
		                fprintf(f,"%s",nextline);
			}
			strcpy(prevline, nextline);
		}
		strcpy(nextline, "");
		fflush(f);
	}
	va_end(argp);
	return result;
}

bool GSM_SetDebugLevel(char *info, Debug_Info *di)
{
	if (!strcmp(info,"nothing")) 		{di->dl = 0;		 	return true;}
	if (!strcmp(info,"text")) 		{di->dl = DL_TEXT;	 	return true;}
	if (!strcmp(info,"textall")) 		{di->dl = DL_TEXTALL;    	return true;}
	if (!strcmp(info,"binary"))  		{di->dl = DL_BINARY;     	return true;}
	if (!strcmp(info,"errors"))  		{di->dl = DL_TEXTERROR;  	return true;}
	if (!strcmp(info,"textdate")) 		{di->dl = DL_TEXTDATE;	 	return true;}
	if (!strcmp(info,"textalldate")) 	{di->dl = DL_TEXTALLDATE;    	return true;}
	if (!strcmp(info,"errorsdate"))  	{di->dl = DL_TEXTERRORDATE;  	return true;}
	return false;
}

/* Dumps a message */
void DumpMessage(FILE *df, Debug_Level dl, const unsigned char *message, int messagesize)
{
	int 		i,j=0,len=16;
	unsigned char	buffer[200];

	if (df==NULL || messagesize == 0) return;

	smfprintf(df, dl, "\n");

	memset(buffer,0x20,sizeof(buffer));
	buffer[len*5-1]=0;

	for (i = 0; i < messagesize; i++) {
		sprintf(buffer+j*4,"%02x",message[i]);
		buffer[j*4+2] = 0x20;
		if (isprint(message[i]) && message[i]!=0x09) {
			if (j != len-1) buffer[j*4+2] 	= message[i];
			buffer[(len-1)*4+j+3]		= message[i];
		} else {
			buffer[(len-1)*4+j+3]		= '.';
		}
		if (j != len-1 && i != messagesize-1) buffer[j*4+3] = '|';
		if (j == len-1) {
			smfprintf(df, dl, "%s\n", buffer);
			memset(buffer,0x20,sizeof(buffer));
			buffer[len*5-1]=0;
			j = 0;
		} else {
			j++;
		}
	}
	if (j != 0) smfprintf(df, dl, "%s\n", buffer);
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

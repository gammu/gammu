
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/timeb.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef WIN32
#  include "windows.h"
#endif

#include "../../cfg/config.h"
#include "misc.h"

/* Based on article in Polish PC-Kurier 8/1998 page 104
 * Archive on http://www.pckurier.pl
 */
char *DayOfWeek (int year, int month, int day)
{
	int 		p,q,r,w;
	static char 	DayOfWeekChar;

	p=(14-month) / 12;
	q=month+12*p-2;
	r=year-p;
	w=(day+(31*q) / 12 + r + r / 4 - r / 100 + r / 400) % 7;
	strcpy(&DayOfWeekChar,"");
	switch (w) {
		case 0: strcpy(&DayOfWeekChar,"Sun"); break;
		case 1: strcpy(&DayOfWeekChar,"Mon"); break;
		case 2: strcpy(&DayOfWeekChar,"Tue"); break;
		case 3: strcpy(&DayOfWeekChar,"Wed"); break;
		case 4: strcpy(&DayOfWeekChar,"Thu"); break;
		case 5: strcpy(&DayOfWeekChar,"Fri"); break;
		case 6: strcpy(&DayOfWeekChar,"Sat"); break;
	}
	return &DayOfWeekChar;
}

/* Dumps a message */
void DumpMessage(FILE *df, unsigned char *message, int messagesize)
{
	int 	i;
	char 	buf[17];

	if (df==NULL) return;

	buf[16] = 0;

	for (i = 0; i < messagesize; i++) {
		if (i % 16 == 0) {
			if (i != 0) fprintf(df, " %s", buf);
			fprintf(df, "\n");
			memset(buf, ' ', 16);
		} else {
			fprintf(df, "|");
		}
		fprintf(df, "%02x", message[i]);
		if (isprint(message[i]) && message[i]!=0x09) {
			if ((i+1) % 16 != 0) fprintf(df, "%c", message[i]);
			buf[i % 16] = message[i];
		} else {
			if ((i+1) % 16 != 0) fprintf(df, " ");
			buf[i % 16] = '.';
		}
	}
	if (i % 16 == 0) {
		fprintf(df, " %s", buf);
	} else {
		fprintf(df, "%*s %s", 4 * (16 - i % 16) - 1, "", buf);
	}
	fprintf(df, "\n");
}

int GetLine(FILE *File, char *Line, int count)
{
	char *ptr;

	if (fgets(Line, count, File)) {
		ptr=Line+strlen(Line)-1;

		while ( (*ptr == '\n' || *ptr == '\r') && ptr>=Line) *ptr--='\0';

		return strlen(Line);
	} else {
		return -1;
	}
}

void SplitLines(unsigned char *message, int messagesize, GSM_Lines *lines, unsigned char *whitespaces, int spaceslen)
{
	int 	i,number=0,j;
	bool 	whitespace=true, nowwhite;

	for (i=0;i<40;i++) lines->numbers[i]=0;

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

Debug_Info di = {0,NULL};

#ifdef DEBUG
int dprintf(const char *format, ...)
{
	va_list	argp;
	int 	result;

	if (di.dl == DL_TEXTALL) {
		va_start(argp, format);
		result = vfprintf(di.df, format, argp);
		va_end(argp);
		return result;
	}
	return 0;
}
#endif

bool GSM_SetDebugLevel(char *info, Debug_Info *di)
{
//	printf("Setting debug level to \"%s\"\n",info);
	if (!strcmp(info,"nothing")) 	{di->dl = 0;		 return true;}
	if (!strcmp(info,"text")) 	{di->dl = DL_TEXT;	 return true;}
	if (!strcmp(info,"textall")) 	{di->dl = DL_TEXTALL;    return true;}
	if (!strcmp(info,"binary"))  	{di->dl = DL_BINARY;     return true;}
	if (!strcmp(info,"errors"))  	{di->dl = DL_TEXTERROR;  return true;}
	return false;
}

#include "debug.h"
#include "gsmstate.h"

#include <string.h>
#include <ctype.h>

GSM_Debug_Info GSM_none_debug = {
	0,
	NULL,
	false,
	"",
	false,
	false
	};

GSM_Debug_Info GSM_global_debug = {
	0,
	NULL,
	false,
	"",
	false,
	false
	};

PRINTF_STYLE(2, 0)
int dbg_vprintf(GSM_Debug_Info *d, const char *format, va_list argp)
{
	int 			result=0;
	char			buffer[3000];
	char			*pos, *end;
	char			save = 0;
	GSM_DateTime 		date_time;
	FILE			*f;
	Debug_Level		l;

	f = d->df;
	l = d->dl;

	if (l == DL_NONE || f == NULL) return 0;

	result = vsnprintf(buffer, sizeof(buffer) - 1, format, argp);
	pos = buffer;

	while (*pos != 0) {

		/* Find new line in string */
		end = strstr(pos, "\n");

		/* Are we at start of line? */
		if (d->was_lf) {
			/* Show date? */
			if (l == DL_TEXTALLDATE || l == DL_TEXTERRORDATE || l == DL_TEXTDATE) {
				GSM_GetCurrentDateTime(&date_time);
		                fprintf(f,"%s %4d/%02d/%02d %02d:%02d:%02d: ",
		                        DayOfWeek(date_time.Year, date_time.Month, date_time.Day),
		                        date_time.Year, date_time.Month, date_time.Day,
		                        date_time.Hour, date_time.Minute, date_time.Second);
			}
			d->was_lf = false;
		}

		/* Remember end char */
		if (end != NULL) {
			save = *end;
			*end = 0;
		}

		/* Output */
		fprintf(f, "%s", pos);

		if (end != NULL) {
			/* We had new line */
			fprintf(f, "\n");
			d->was_lf = true;

			/* Restore saved char */
			*end = save;

			/* Advance to next line */
			pos = end + strlen("\n");
		} else {
			/* We hit end of string */
			break;
		}
	}

	/* Flush buffers, this might be configurable, but it could cause drop of last log messages */
	fflush(f);

	return result;
}

bool GSM_SetDebugLevel(const char *info, GSM_Debug_Info *privdi)
{
	if (info == NULL)			{privdi->dl = DL_NONE;	 	return true;}
	if (!strcmp(info,"nothing")) 		{privdi->dl = DL_NONE;	 	return true;}
	if (!strcmp(info,"text")) 		{privdi->dl = DL_TEXT;	 	return true;}
	if (!strcmp(info,"textall")) 		{privdi->dl = DL_TEXTALL;    	return true;}
	if (!strcmp(info,"binary"))  		{privdi->dl = DL_BINARY;     	return true;}
	if (!strcmp(info,"errors"))  		{privdi->dl = DL_TEXTERROR;  	return true;}
	if (!strcmp(info,"textdate")) 		{privdi->dl = DL_TEXTDATE;	 	return true;}
	if (!strcmp(info,"textalldate")) 	{privdi->dl = DL_TEXTALLDATE;    	return true;}
	if (!strcmp(info,"errorsdate"))  	{privdi->dl = DL_TEXTERRORDATE;  	return true;}
	return false;
}

bool GSM_SetDebugCoding(const char *info, GSM_Debug_Info *privdi)
{
	privdi->coding = info;
	return true;
}

bool GSM_SetDebugGlobal(bool info, GSM_Debug_Info *privdi)
{
	privdi->use_global = info;
	return true;
}

PRINTF_STYLE(2, 3)
int smfprintf(GSM_Debug_Info *d, const char *format, ...)
{
        va_list 		argp;
	int 			result;
	GSM_Debug_Info		*tmpdi;

	if (d == NULL || d->use_global) {
		tmpdi = &GSM_global_debug;
	} else {
		tmpdi = d;
	}

	va_start(argp, format);
	result = dbg_vprintf(tmpdi, format, argp);
	va_end(argp);

	return result;
}


PRINTF_STYLE(2, 3)
int smprintf(GSM_StateMachine *s, const char *format, ...)
{
	va_list		argp;
	int 		result=0;
	GSM_Debug_Info *curdi;

	curdi = &GSM_global_debug;
	if (s != NULL && s->di.use_global == false) {
		curdi = &(s->di);
	}

	va_start(argp, format);

	result = dbg_vprintf(curdi, format, argp);

	va_end(argp);
	return result;
}

PRINTF_STYLE(3, 4)
int smprintf_level(GSM_StateMachine * s, GSM_DebugSeverity severity, const char *format, ...)
{
	va_list		argp;
	int 		result=0;
	GSM_Debug_Info *curdi;

	curdi = GSM_GetDI(s);

	if (severity == D_TEXT) {
		if (curdi->dl != DL_TEXT &&
				curdi->dl != DL_TEXTALL &&
				curdi->dl != DL_TEXTDATE &&
				curdi->dl != DL_TEXTALLDATE) {
			return 0;
		}
	} else if (severity == D_ERROR) {
		if (curdi->dl != DL_TEXT &&
				curdi->dl != DL_TEXTALL &&
				curdi->dl != DL_TEXTDATE &&
				curdi->dl != DL_TEXTALLDATE &&
				curdi->dl != DL_TEXTERROR &&
				curdi->dl != DL_TEXTERRORDATE) {
			return 0;
		}
	}
	va_start(argp, format);

	result = dbg_vprintf(curdi, format, argp);

	va_end(argp);
	return result;
}

#define CHARS_PER_LINE (16)

/* Dumps a message */
void DumpMessage(GSM_Debug_Info *d, const unsigned char *message, const int messagesize)
{
	int i, j = 0;
	char buffer[(CHARS_PER_LINE * 5) + 1];

	if (d->df == NULL || messagesize == 0) return;

	smfprintf(d, "\n");

	memset(buffer, ' ', CHARS_PER_LINE * 5);
	buffer[CHARS_PER_LINE * 5] = 0;

	for (i = 0; i < messagesize; i++) {
		/* Write hex number */
		snprintf(buffer + (j * 4), 3, "%02X", message[i]);
		buffer[(j * 4) + 2] = ' '; /* wipe snprintf's \0 */

		/* Write char if possible */
		if (isprint(message[i])
				/* 0x09 = tab */
				&& message[i] != 0x09
				/* 0x01 = beep in windows xp */
				&& message[i] != 0x01
				/* these are empty in windows xp */
				&& message[i] != 0x85
				&& message[i] != 0x95
				&& message[i] != 0xA6
				&& message[i] != 0xB7) {
			buffer[(j * 4) + 2] = message[i];
			buffer[(CHARS_PER_LINE - 1) * 4 + j + 4] = message[i];
		} else {
			buffer[(CHARS_PER_LINE - 1) * 4 + j + 4] = '.';
		}

		/* Write char separator */
		if (j != CHARS_PER_LINE - 1 && i != messagesize - 1) {
			buffer[j * 4 + 3] = '|';
		}

		/* Print out buffer */
		if (j == CHARS_PER_LINE - 1) {
			smfprintf(d, "%s\n", buffer);
			memset(buffer, ' ', CHARS_PER_LINE * 5);
			j = 0;
		} else {
			j++;
		}
	}

	/* Anything remains to be printed? */
	if (j != 0) {
		smfprintf(d, "%s\n", buffer);
	}
}

#undef CHARS_PER_LINE

void DumpMessageText(GSM_Debug_Info *d, const unsigned char *message, const int messagesize)
{
	if (d->dl != DL_TEXTALL && d->dl == DL_TEXTALLDATE) return;
	DumpMessage(d, message, messagesize);

}

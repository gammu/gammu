/* Gammu logging/debugging functions */
/* Copyright (c) 2008-2009 by Michal Cihar <michal@cihar.com> */
/* Licensed under GPL2+ */

#include "debug.h"
#include "gsmstate.h"

#include <string.h>
#include <ctype.h>

/* Commit flag for opening files is MS extension, some other
 * implementations (like BCC 5.5) don't like this flag at all */
#ifdef _MSC_VER
#  define COMMIT_FLAG "c"
#else
#  define COMMIT_FLAG ""
#endif

GSM_Debug_Info GSM_none_debug = {
	0,
	NULL,
	false,
	"",
	false,
	false,
    NULL,
    NULL
	};

GSM_Debug_Info GSM_global_debug = {
	0,
	NULL,
	false,
	"",
	false,
	false,
    NULL,
    NULL
	};

/**
 * Actually writes message to debuging file.
 */
void dbg_write(GSM_Debug_Info *d, const char *text)
{
    if (d->log_function != NULL) {
        d->log_function(text, d->user_data);
    } else if (d->df != NULL) {
        fprintf(d->df, "%s", text);
    }


}

PRINTF_STYLE(2, 0)
int dbg_vprintf(GSM_Debug_Info *d, const char *format, va_list argp)
{
	int 			result=0;
	char			buffer[3000], timestamp[60];
	char			*pos, *end;
	char			save = 0;
	GSM_DateTime 		date_time;
	Debug_Level		l;

	l = d->dl;

	if (l == DL_NONE) return 0;

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
                sprintf(timestamp, "%s %4d/%02d/%02d %02d:%02d:%02d: ",
		                        DayOfWeek(date_time.Year, date_time.Month, date_time.Day),
		                        date_time.Year, date_time.Month, date_time.Day,
		                        date_time.Hour, date_time.Minute, date_time.Second);
                dbg_write(d, timestamp);
			}
			d->was_lf = false;
		}

		/* Remember end char */
		if (end != NULL) {
			save = *end;
			*end = 0;
		}

		/* Output */
		dbg_write(d, pos);

		if (end != NULL) {
			/* We had new line */
			dbg_write(d, "\n");
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
    if (d->df != NULL) {
        fflush(d->df);
    }

	return result;
}

GSM_Error GSM_SetDebugFileDescriptor(FILE *fd, bool closable, GSM_Debug_Info *privdi)
{
	privdi->was_lf = true;

	if (privdi->df != NULL
			&& fileno(privdi->df) != fileno(stderr)
			&& fileno(privdi->df) != fileno(stdout)
			&& privdi->closable) {
		fclose(privdi->df);
	}

	privdi->df = fd;
	privdi->closable = closable;

	return ERR_NONE;
}

GSM_Error GSM_SetDebugFile(const char *info, GSM_Debug_Info *privdi)
{
	FILE *testfile;

	if (info == NULL || strlen(info) == 0) {
		return GSM_SetDebugFileDescriptor(NULL, false, privdi);
	}

	switch (privdi->dl) {
		case DL_BINARY:
			testfile = fopen(info,"wb" COMMIT_FLAG);
			break;
		case DL_TEXTERROR:
		case DL_TEXTERRORDATE:
			testfile = fopen(info,"a" COMMIT_FLAG);
			if (!testfile) {
				dbgprintf(privdi, "Can't open debug file\n");
				return ERR_CANTOPENFILE;
			}
			fseek(testfile, 0, SEEK_END);
			if (ftell(testfile) > 5000000) {
				fclose(testfile);
				testfile = fopen(info,"w" COMMIT_FLAG);
			}
			break;
		default:
			testfile = fopen(info,"w" COMMIT_FLAG);
	}

	if (testfile == NULL) {
		dbgprintf(privdi, "Can't open debug file\n");
		return ERR_CANTOPENFILE;
	} else {
		return GSM_SetDebugFileDescriptor(testfile, true, privdi);
	}
}

GSM_Error GSM_SetDebugFunction(GSM_Log_Function info, void *data, GSM_Debug_Info * privdi)
{
    privdi->log_function = info;
    privdi->user_data = data;
    return ERR_NONE;
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

	smfprintf(d, "\n");

	if (messagesize == 0) return;

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

/* (c) 2002-2004 by Marcin Wiacek & Michal Cihar */

#include <ctype.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#ifndef __OpenBSD__
#  include <wchar.h>
#endif
#ifdef WIN32
#  include <windows.h>
#else
#  include <stdlib.h>
#  include <errno.h>
#  include <fcntl.h>
#  include <signal.h>
#endif

#include "gsmcomon.h"
#include "misc/coding/coding.h"

GSM_Error NoneFunction(void)
{
	return ERR_NONE;
}

GSM_Error NotImplementedFunction(void)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error NotSupportedFunction(void)
{
	return ERR_NOTSUPPORTED;
}

unsigned char *GetMsg (INI_Section *cfg, unsigned char *default_string)
{
	unsigned char 		*retval, buffer[40], buff2[40], buff[2000];
	static unsigned char	def_str[2000];
	INI_Entry		*e;
	INI_Section 		*h;
	int			num;
	int			len;

	if (cfg==NULL) return default_string;

	EncodeUnicode (buff2, "common", 6);

	/* Set all 0x0a to \n */
	memset(def_str,0,sizeof(def_str));
	for (num=0;num<((int)strlen(default_string));num++) {
		if (default_string[num] == 0x0a) {
			def_str[strlen(def_str)] = '\\';
			def_str[strlen(def_str)] = 'n';
		} else def_str[strlen(def_str)] = default_string[num];
	}

	e = NULL;
	/* First find our section */
        for (h = cfg; h != NULL; h = h->Next) {
		if (mywstrncasecmp(buff2, h->SectionName, 0)) {
			e = h->SubEntries;
			break;
		}
	}
	while (e != NULL) {
		num = -1;
		DecodeUnicode(e->EntryName,buffer);
		if (strlen(buffer) == 5 && (buffer[0] == 'F' || buffer[0] == 'f')) {
			num = atoi(buffer+2);
		}
		if (num!=-1) {
			DecodeUnicode(e->EntryValue,buff);
			/* Remove quotes */
			if (buff[0] == '"') {
				len = strlen(buff);
				memmove(buff, buff + 1, len - 1);
				if (buff[len - 2] == '"') buff[len - 2] = 0;
			}
			if (strcmp(buff, def_str) == 0) {
				sprintf(buff,"T%04i",num);
				EncodeUnicode (buffer, buff, 5);
			        retval = INI_GetValue(cfg, buff2, buffer, true);
			        if (retval) {
					sprintf(buff,"%s",DecodeUnicodeConsole(retval+2));
					buff[strlen(buff)-1] = 0;
					/* Set all \n to 0x0a */
					memset(def_str,0,sizeof(def_str));
					num = 0;
					while (num != (int)strlen(buff)) {
						if (num < (int)strlen(buff) - 1) {
							if (buff[num] == '\\' && buff[num+1] == 'n') {
								def_str[strlen(def_str)] = 0x0a;
								num+=2;
							} else {
								def_str[strlen(def_str)] = buff[num++];
							}
						} else {
							def_str[strlen(def_str)] = buff[num++];
						}
					}
					retval = def_str;
				} else {
					retval = default_string;
				}
				return retval;
			}
		}
		e = e->Next;
	}
	return default_string;
}

typedef struct {
	GSM_Error		ErrorNum;
	unsigned char		*ErrorText;
} PrintErrorEntry;

static PrintErrorEntry PrintErrorEntries[] = {
	{ERR_NONE,			"No error."},
	{ERR_DEVICEOPENERROR,		"Error opening device. Unknown/busy or no permissions."},
	{ERR_DEVICELOCKED,		"Error opening device. Device locked."},
	{ERR_DEVICENOTEXIST,		"Error opening device. Doesn't exist."},
	{ERR_DEVICEBUSY,		"Error opening device. Already opened by other application."},
	{ERR_DEVICENOPERMISSION,	"Error opening device. No permissions."},
	{ERR_DEVICENODRIVER,		"Error opening device. No required driver in operating system."},
	{ERR_DEVICENOTWORK,		"Error opening device. Some hardware not connected/wrong configured."},
	{ERR_DEVICEDTRRTSERROR,		"Error setting device DTR or RTS."},
	{ERR_DEVICECHANGESPEEDERROR,	"Error setting device speed. Maybe speed not supported."},
	{ERR_DEVICEWRITEERROR,		"Error writing device."},
	{ERR_DEVICEREADERROR,		"Error during reading device."},
	{ERR_DEVICEPARITYERROR,		"Can't set parity on device."},
	{ERR_TIMEOUT,			"No response in specified timeout. Probably phone not connected."},
	/* Some missed */
	{ERR_UNKNOWNRESPONSE,		"Unknown response from phone. See readme.txt, how to report it."},
	/* Some missed */
	{ERR_UNKNOWNCONNECTIONTYPESTRING,"Unknown connection type string. Check config file."},
	{ERR_UNKNOWNMODELSTRING,	"Unknown model type string. Check config file."},
	{ERR_SOURCENOTAVAILABLE,	"Some required functions not compiled for your OS. Please contact."},
	{ERR_NOTSUPPORTED,		"Function not supported by phone."},
	{ERR_EMPTY,			"Entry is empty"},
	{ERR_SECURITYERROR,		"Security error. Maybe no PIN ?"},
	{ERR_INVALIDLOCATION,		"Invalid location. Maybe too high ?"},
	{ERR_NOTIMPLEMENTED,		"Function not implemented. Help required."},
	{ERR_FULL,			"Memory full."},
	{ERR_UNKNOWN,			"Unknown error."},
	/* Some missed */
	{ERR_CANTOPENFILE,		"Can't open specified file. Read only ?"},
	{ERR_MOREMEMORY,		"More memory required..."},
	{ERR_PERMISSION,		"Permission to file/device required..."},
	{ERR_EMPTYSMSC,			"Empty SMSC number. Set in phone or use -smscnumber."},
	{ERR_INSIDEPHONEMENU,		"You're inside phone menu (during editing ?). Leave it and try again."},
	{ERR_WORKINPROGRESS,		"Function is during writing. If want help, please contact with authors."},
	{ERR_PHONEOFF,			"Phone is disabled and connected to charger."},
	{ERR_FILENOTSUPPORTED,		"File format not supported by Gammu."},
	{ERR_BUG,			"Nobody is perfect, some bug appeared in protocol implementation. Please contact authors."},
	{ERR_CANCELED,			"Transfer was canceled by phone (you pressed cancel on phone?)"},
	/* Some missed */
	{ERR_OTHERCONNECTIONREQUIRED,	"Current connection type doesn't support called function."},
	{ERR_WRONGCRC,			"CRC error."},
	{ERR_INVALIDDATETIME,		"Invalid date or time specified."},
	{ERR_MEMORY,			"Phone memory error, maybe it is read only."},
	{ERR_INVALIDDATA,		"Invalid data."},
	{ERR_FILEALREADYEXIST,		"File with specified name already exist."},

	{0,				""}
};

unsigned char *print_error(GSM_Error e, FILE *df, INI_Section *cfg)
{
	unsigned char 	*def 	= NULL;
	int 		i	= 0;

	while (PrintErrorEntries[i].ErrorNum != 0) {
		if (PrintErrorEntries[i].ErrorNum == e) {
			def 	= PrintErrorEntries[i].ErrorText;
			break;
		}
		i++;
	}
	if (def == NULL) def = "Unknown error.";
	if (df!=NULL && di.dl!=0) fprintf(df,"[ERROR %i: %s]\n",e,def);

	return GetMsg(cfg,def);
}

const char *GetGammuLocalePath(void)
{
#ifdef LOCALE_PATH
	static const char Buffer[] = LOCALE_PATH;
	return Buffer;
#else
	return NULL;
#endif
}

const char *GetGammuVersion(void)
{
	static const char Buffer[] = VERSION;
	return Buffer;
}

GSM_Error GSM_SetDebugFile(char *info, Debug_Info *privdi)
{
	FILE *testfile;

	/* If we should use global file descriptor, use it */
	if (privdi->use_global) {
		/* Aren't we the changing the global di? */
		if (privdi != &di) {
			if (privdi->df != di.df && 
					privdi->dl!=0 && 
					fileno(privdi->df) != 1 && 
					fileno(privdi->df) != 2)
				fclose(privdi->df);
			privdi->df = di.df;
			return ERR_NONE;
        	}
    	} else {
        	/* If we should not use global file descriptor, don't even try use it */
        	if (privdi->df == di.df) privdi->df = stdout;
    	}

	if (info[0]!=0 && privdi->dl != 0) {
		switch (privdi->dl) {
		case DL_BINARY:
			testfile = fopen(info,"wcb");
			break;
		case DL_TEXTERROR:
		case DL_TEXTERRORDATE:
			testfile = fopen(info,"ac");
			if (!testfile) {
				dbgprintf("Can't open debug file\n");
				return ERR_CANTOPENFILE;
			}
			fseek(testfile, 0, SEEK_END);
			if (ftell(testfile) > 5000000) {
				fclose(testfile);
				testfile = fopen(info,"wc");
			}
			break;
		default:
			testfile = fopen(info,"wc");
		}
		if (!testfile) {
			dbgprintf("Can't open debug file\n");
			return ERR_CANTOPENFILE;
		} else {
			if (privdi->df && privdi->df != stdout) {
				fclose(privdi->df);
			}
			privdi->df = testfile;
		}
	}
	return ERR_NONE;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

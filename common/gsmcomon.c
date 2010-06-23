
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
#include "misc/coding.h"

GSM_Error NoneFunction(void)
{
	return GE_NONE;
}

GSM_Error NotImplementedFunction(void)
{
	return GE_NOTIMPLEMENTED;
}

GSM_Error NotSupportedFunction(void)
{
	return GE_NOTSUPPORTED;
}

unsigned char *GetMsg (CFG_Header *cfg, unsigned char *default_string)
{
	unsigned char 		*retval, buffer[40], buff2[40], buff[2000];
	static unsigned char	def_str[2000];
	CFG_Entry		*e;
	CFG_Header 		*h;
	int			num;

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
        for (h = cfg; h != NULL; h = h->next) {
		if (mywstrncasecmp(buff2, h->section, 0)) {
			e = h->entries;
			break;
		}
	}
	while (1) {
		if (e == NULL) break;
		num = -1;
		DecodeUnicode(e->key,buffer);
		if (strlen(buffer) == 5 && (buffer[0] == 'F' || buffer[0] == 'f')) {
			num = atoi(buffer+2);
		}
		if (num!=-1) {
			DecodeUnicode(e->value+2,buff);
			if (strncmp(buff,def_str,strlen(def_str))==0) {
				sprintf(buff,"T%04i",num);
				EncodeUnicode (buffer, buff, 5);
			        retval = CFG_Get(cfg, buff2, buffer, true);
			        if (retval) {
					DecodeUnicode(retval+2,buff);
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
		e = e->next;
	}
	return default_string;
}

typedef struct {
	GSM_Error		ErrorNum;
	unsigned char		*ErrorText;
} PrintErrorEntry;

static PrintErrorEntry PrintErrorEntries[] = {
	{GE_NONE,			"No error."},
	{GE_DEVICEOPENERROR,		"Error opening device. Unknown/busy or no permissions."},
	{GE_DEVICELOCKED,		"Error opening device. Device locked."},
	{GE_DEVICENOTEXIST,		"Error opening device. Not exist."},
	{GE_DEVICEBUSY,			"Error opening device. Already opened by other application."},
	{GE_DEVICENOPERMISSION,		"Error opening device. No permissions."},
	{GE_DEVICENODRIVER,		"Error opening device. No required driver in operating system."},
	{GE_DEVICENOTWORK,		"Error opening device. Some hardware not connected/wrong configured."},
	{GE_DEVICEDTRRTSERROR,		"Error setting device DTR or RTS."},
	{GE_DEVICECHANGESPEEDERROR,	"Error setting device speed. Maybe speed not supported."},
	{GE_DEVICEWRITEERROR,		"Error writing device."},
	{GE_DEVICEREADERROR,		"Error during reading device"},
	{GE_DEVICEPARITYERROR,		"Can't set parity on device"},
	{GE_TIMEOUT,			"No response in specified timeout. Probably phone not connected."},
	/* Some missed */
	{GE_UNKNOWNRESPONSE,		"Unknown response from phone. See /readme.txt, how to report it."},
	/* Some missed */
	{GE_UNKNOWNCONNECTIONTYPESTRING,"Unknown connection type string. Check config file."},
	{GE_UNKNOWNMODELSTRING,		"Unknown model type string. Check config file."},
	{GE_SOURCENOTAVAILABLE,		"Some required functions not compiled for your OS. Please contact."},
	{GE_NOTSUPPORTED,		"Function not supported by phone."},
	{GE_EMPTY,			"Entry is empty"},
	{GE_SECURITYERROR,		"Security error. Maybe no PIN ?"},
	{GE_INVALIDLOCATION,		"Invalid location. Maybe too high ?"},
	{GE_NOTIMPLEMENTED,		"Function not implemented. Help required."},
	{GE_FULL,			"Memory full."},
	{GE_UNKNOWN,			"Unknown error."},
	/* Some missed */
	{GE_CANTOPENFILE,		"Can't open specified file. Read only ?"},
	{GE_MOREMEMORY,			"More memory required..."},
	{GE_PERMISSION,			"Permission to file/device required..."},
	{GE_EMPTYSMSC,			"Empty SMSC number. Set in phone or use -smscnumber"},
	{GE_INSIDEPHONEMENU,		"You're inside phone menu (during editing ?). Leave it and try again."},
	{GE_WORKINPROGRESS,		"Function is during writing. If want help, please contact with authors."},
	{GE_PHONEOFF,			"Phone is disabled and connected to charger"},
	{GE_FILENOTSUPPORTED,		"File format not supported by Gammu"},
	{GE_BUG,			"Nobody is perfect, some bug appeared in protocol implementation. Please contact authors."},
	{GE_CANCELED,			"Transfer was canceled by phone (you pressed cancel on phone?)."},
	/* Some missed */
	{GE_OTHERCONNECTIONREQUIRED,	"Current connection type doesn't support called function."},

	{0,				""}
};

unsigned char *print_error(GSM_Error e, FILE *df, CFG_Header *cfg)
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
	if (df!=NULL) fprintf(df,"[ERROR %i: %s]\n",e,def);

	return GetMsg(cfg,def);
}

char *GetGammuVersion()
{
	static char Buffer[1000]="";

	sprintf(Buffer, "%s",VERSION);
	return Buffer;
}

GSM_Error GSM_SetDebugFile(char *info, Debug_Info *privdi)
{
	FILE *testfile;

	/* If we should use global file descriptor, use it */
	if (privdi->use_global) {
		/* Aren't we the changing the global di? */
		if (privdi != &di) {
			if (privdi->df == stdout) privdi->df = di.df;
			return GE_NONE;
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
				dprintf("Can't open debug file\n");
				return GE_CANTOPENFILE;
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
			dprintf("Can't open debug file\n");
			return GE_CANTOPENFILE;
		} else {
			if (privdi->df && privdi->df != stdout)
			{
				fclose(privdi->df);
			}
			privdi->df = testfile;
		}
	}
	return GE_NONE;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

/* (c) 2002-2005 by Marcin Wiacek & Michal Cihar */

#include <ctype.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#ifndef __OpenBSD__
#  include <wchar.h>
#endif
#ifdef WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  include <stdlib.h>
#  include <errno.h>
#  include <fcntl.h>
#  include <signal.h>
#endif

#include "misc/locales.h"
#include "misc/misc.h"

#include <gammu-debug.h>

/* Commit flag for opening files is MS extension, some other
 * implementations (like BCC 5.5) don't like this flag at all */
#ifdef _MSC_VER
#  define COMMIT_FLAG "c"
#else
#  define COMMIT_FLAG ""
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

/**
 * Initializes locales from given path or from system default.
 *
 * @param path Optional path to locales, can be NULL.
 */
void GSM_InitLocales(const char *path) {
	setlocale(LC_ALL, "");
#ifdef GETTEXTLIBS_FOUND
	if (path == NULL || strlen(path) == 0) {
#if defined(LOCALE_PATH)
		bindtextdomain("gammu", LOCALE_PATH);
#else
		bindtextdomain("gammu", ".");
#endif
	} else {
		bindtextdomain("gammu", path);
	}
	textdomain("gammu");
#endif
}

/**
 * Gammu errors descriptions.
 */
typedef struct {
	GSM_Error		ErrorNum; /**< Error code */
	unsigned char		*ErrorText; /**< Textual representation, not localised, use gettext to get localised string */
} PrintErrorEntry;

/**
 * Mapping of error numbers to descriptions.
 */
static PrintErrorEntry PrintErrorEntries[] = {
	{ERR_NONE,			N_("No error.")},
	{ERR_DEVICEOPENERROR,		N_("Error opening device. Unknown/busy or no permissions.")},
	{ERR_DEVICELOCKED,		N_("Error opening device, it is locked.")},
	{ERR_DEVICENOTEXIST,		N_("Error opening device, it doesn't exist.")},
	{ERR_DEVICEBUSY,		N_("Error opening device, it is already opened by other application.")},
	{ERR_DEVICENOPERMISSION,	N_("Error opening device, you don't have permissions.")},
	{ERR_DEVICENODRIVER,		N_("Error opening device. No required driver in operating system.")},
	{ERR_DEVICENOTWORK,		N_("Error opening device. Some hardware not connected/wrong configured.")},
	{ERR_DEVICEDTRRTSERROR,		N_("Error setting device DTR or RTS.")},
	{ERR_DEVICECHANGESPEEDERROR,	N_("Error setting device speed. Maybe speed not supported.")},
	{ERR_DEVICEWRITEERROR,		N_("Error writing device.")},
	{ERR_DEVICEREADERROR,		N_("Error during reading device.")},
	{ERR_DEVICEPARITYERROR,		N_("Can't set parity on device.")},
	{ERR_TIMEOUT,			N_("No response in specified timeout. Probably phone not connected.")},
	/* Some missed */
	{ERR_UNKNOWNRESPONSE,		N_("Unknown response from phone. See <http://cihar.com/gammu/report> for information how to report it.")},
	/* Some missed */
	{ERR_UNKNOWNCONNECTIONTYPESTRING,N_("Unknown connection type string. Check config file.")},
	{ERR_UNKNOWNMODELSTRING,	N_("Unknown model type string. Check config file.")},
	{ERR_SOURCENOTAVAILABLE,	N_("Some functions not available for your system (disabled in config or not written).")},
	{ERR_NOTSUPPORTED,		N_("Function not supported by phone.")},
	{ERR_EMPTY,			N_("Entry is empty")},
	{ERR_SECURITYERROR,		N_("Security error. Maybe no PIN?")},
	{ERR_INVALIDLOCATION,		N_("Invalid location. Maybe too high?")},
	{ERR_NOTIMPLEMENTED,		N_("Function not implemented. Help required.")},
	{ERR_FULL,			N_("Memory full.")},
	{ERR_UNKNOWN,			N_("Unknown error.")},
	/* Some missed */
	{ERR_CANTOPENFILE,		N_("Can't open specified file. Read only?")},
	{ERR_MOREMEMORY,		N_("More memory required...")},
	{ERR_PERMISSION,		N_("Permission to file/device required...")},
	{ERR_EMPTYSMSC,			N_("Empty SMSC number. Set in phone or use -smscnumber.")},
	{ERR_INSIDEPHONEMENU,		N_("You're inside phone menu (maybe editing?). Leave it and try again.")},
	{ERR_WORKINPROGRESS,		N_("Function is during writing. If want help, please contact with authors.")},
	{ERR_PHONEOFF,			N_("Phone is disabled and connected to charger.")},
	{ERR_FILENOTSUPPORTED,		N_("File format not supported by Gammu.")},
	{ERR_BUG,			N_("Nobody is perfect, some bug appeared in protocol implementation. Please contact authors.")},
	{ERR_CANCELED,			N_("Transfer was canceled by phone, maybe you pressed cancel on phone.")},
	/* Some missed */
	{ERR_OTHERCONNECTIONREQUIRED,	N_("Current connection type doesn't support called function.")},
	{ERR_WRONGCRC,			N_("CRC error.")},
	{ERR_INVALIDDATETIME,		N_("Invalid date or time specified.")},
	{ERR_MEMORY,			N_("Phone memory error, maybe it is read only.")},
	{ERR_INVALIDDATA,		N_("Invalid data given to phone.")},
	{ERR_FILEALREADYEXIST,		N_("File with specified name already exist.")},
	{ERR_FILENOTEXIST,		N_("File with specified name doesn't exist.")},
	{ERR_SHOULDBEFOLDER,		N_("You have to give folder name and not file name.")},
	{ERR_SHOULDBEFILE,		N_("You have to give file name and not folder name.")},
	{ERR_NOSIM,			N_("Can not access SIM card.")},
	{ERR_GNAPPLETWRONG,		N_("Wrong GNAPPLET version in phone. Use version from currently used Gammu.")},
	{ERR_FOLDERNOTEMPTY,		N_("Folder must be empty.")},
	{ERR_DATACONVERTED,		N_("Data were converted.")},
	{ERR_UNCONFIGURED,		N_("Gammu is not configured.")},

	{0,				""}
};

const char *GSM_ErrorString(GSM_Error e)
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
	if (def == NULL) def = N_("Unknown error.");

	return gettext(def);
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

GSM_Debug_Info *GSM_GetGlobalDebug()
{
	return &di;
}

GSM_Error GSM_SetDebugFileDescriptor(FILE *fd, GSM_Debug_Info *privdi)
{
	privdi->was_lf = true;

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

	if (privdi->df && privdi->df != stdout) {
		fclose(privdi->df);
	}
	privdi->df = fd;
	return ERR_NONE;
}

GSM_Error GSM_SetDebugFile(char *info, GSM_Debug_Info *privdi)
{
	FILE *testfile;

	if (info != NULL && info[0]!=0 && privdi->dl != 0) {
		switch (privdi->dl) {
		case DL_BINARY:
			testfile = fopen(info,"wb" COMMIT_FLAG);
			break;
		case DL_TEXTERROR:
		case DL_TEXTERRORDATE:
			testfile = fopen(info,"a" COMMIT_FLAG);
			if (!testfile) {
				dbgprintf("Can't open debug file\n");
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
		if (!testfile) {
			dbgprintf("Can't open debug file\n");
			return ERR_CANTOPENFILE;
		} else {
			GSM_SetDebugFileDescriptor(testfile, privdi);
		}
	}
	return ERR_NONE;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

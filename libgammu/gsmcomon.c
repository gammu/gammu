/* (c) 2002-2005 by Marcin Wiacek & Michal Cihar */

#include <ctype.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#ifdef HAVE_WCHAR_H
#  include <wchar.h>
#endif
#ifdef WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

#include "misc/locales.h"

#include "debug.h"

#include <gammu-debug.h>

#include "gsmcomon.h"

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
#ifdef GETTEXTLIBS_FOUND
void GSM_InitLocales(const char *path) {
	setlocale(LC_ALL, "");
	if (path == NULL || strlen(path) == 0) {
#if defined(LOCALE_PATH)
		bindtextdomain("libgammu", LOCALE_PATH);
#else
		bindtextdomain("libgammu", ".");
#endif
	} else {
		bindtextdomain("libgammu", path);
	}
}
#else
void GSM_InitLocales(const char UNUSED *path) {
	setlocale(LC_ALL, "");
}
#endif

/**
 * Gammu errors descriptions.
 */
typedef struct {
	/**
	 * Error code.
	 */
	GSM_Error ErrorNum;
	/**
	 * Name of error.
	 */
	const char *ErrorName;
	/**
	 * Textual representation, not localised, use gettext to get localised string.
	 */
	const char *ErrorText;
} PrintErrorEntry;

/**
 * Mapping of error numbers to descriptions.
 */
static PrintErrorEntry PrintErrorEntries[] = {
	{ERR_NONE, "NONE",			N_("No error.")},
	{ERR_DEVICEOPENERROR, "DEVICEOPENERROR",		N_("Error opening device. Unknown/busy or no permissions.")},
	{ERR_DEVICELOCKED, "DEVICELOCKED",		N_("Error opening device, it is locked.")},
	{ERR_DEVICENOTEXIST, "DEVICENOTEXIST",		N_("Error opening device, it doesn't exist.")},
	{ERR_DEVICEBUSY, "DEVICEBUSY",		N_("Error opening device, it is already opened by other application.")},
	{ERR_DEVICENOPERMISSION, "DEVICENOPERMISSION",	N_("Error opening device, you don't have permissions.")},
	{ERR_DEVICENODRIVER, "DEVICENODRIVER",		N_("Error opening device. No required driver in operating system.")},
	{ERR_DEVICENOTWORK, "DEVICENOTWORK",		N_("Error opening device. Some hardware not connected/wrong configured.")},
	{ERR_DEVICEDTRRTSERROR, "DEVICEDTRRTSERROR",		N_("Error setting device DTR or RTS.")},
	{ERR_DEVICECHANGESPEEDERROR, "DEVICECHANGESPEEDERROR",	N_("Error setting device speed. Maybe speed not supported.")},
	{ERR_DEVICEWRITEERROR, "DEVICEWRITEERROR",		N_("Error writing device.")},
	{ERR_DEVICEREADERROR, "DEVICEREADERROR",		N_("Error during reading device.")},
	{ERR_DEVICEPARITYERROR, "DEVICEPARITYERROR",		N_("Can't set parity on device.")},
	{ERR_TIMEOUT, "TIMEOUT",			N_("No response in specified timeout. Probably phone not connected.")},
	{ERR_FRAMENOTREQUESTED, "FRAMENOTREQUESTED",		N_("Frame not requested right now. See <http://cihar.com/gammu/report> for information how to report it.")},
	{ERR_UNKNOWNRESPONSE, "UNKNOWNRESPONSE",		N_("Unknown response from phone. See <http://cihar.com/gammu/report> for information how to report it.")},
	{ERR_UNKNOWNFRAME, "UNKNOWNFRAME",		N_("Unknown frame. See <http://cihar.com/gammu/report> for information how to report it.")},
	{ERR_UNKNOWNCONNECTIONTYPESTRING, "UNKNOWNCONNECTIONTYPESTRING",N_("Unknown connection type string. Check config file.")},
	{ERR_UNKNOWNMODELSTRING, "UNKNOWNMODELSTRING",	N_("Unknown model type string. Check config file.")},
	{ERR_SOURCENOTAVAILABLE, "SOURCENOTAVAILABLE",	N_("Some functions not available for your system (disabled in config or not written).")},
	{ERR_NOTSUPPORTED, "NOTSUPPORTED",		N_("Function not supported by phone.")},
	{ERR_EMPTY, "EMPTY",			N_("Entry is empty")},
	{ERR_SECURITYERROR, "SECURITYERROR",		N_("Security error. Maybe no PIN?")},
	{ERR_INVALIDLOCATION, "INVALIDLOCATION",		N_("Invalid location. Maybe too high?")},
	{ERR_NOTIMPLEMENTED, "NOTIMPLEMENTED",		N_("Function not implemented. Help required.")},
	{ERR_FULL, "FULL",			N_("Memory full.")},
	{ERR_UNKNOWN, "UNKNOWN",			N_("Unknown error.")},
	{ERR_CANTOPENFILE, "CANTOPENFILE",		N_("Can't open specified file. Read only?")},
	{ERR_MOREMEMORY, "MOREMEMORY",		N_("More memory required...")},
	{ERR_PERMISSION, "PERMISSION",		N_("Permission to file/device required...")},
	{ERR_EMPTYSMSC, "EMPTYSMSC",			N_("Empty SMSC number. Set in phone or use -smscnumber.")},
	{ERR_INSIDEPHONEMENU, "INSIDEPHONEMENU",		N_("You're inside phone menu (maybe editing?). Leave it and try again.")},
	{ERR_NOTCONNECTED, "NOTCONNECTED",		N_("Phone is not connected.")},
	{ERR_WORKINPROGRESS, "WORKINPROGRESS",		N_("Function is during writing. If want help, please contact with authors.")},
	{ERR_PHONEOFF, "PHONEOFF",			N_("Phone is disabled and connected to charger.")},
	{ERR_FILENOTSUPPORTED, "FILENOTSUPPORTED",		N_("File format not supported by Gammu.")},
	{ERR_BUG, "BUG",			N_("Nobody is perfect, some bug appeared in protocol implementation. Please contact authors.")},
	{ERR_CANCELED, "CANCELED",			N_("Transfer was canceled by phone, maybe you pressed cancel on phone.")},
	{ERR_NEEDANOTHERANSWER, "NEEDANOTHERANSWER",		N_("Phone module need to send another answer frame.")}, /* This should be only internal. */
	{ERR_OTHERCONNECTIONREQUIRED, "OTHERCONNECTIONREQUIRED",	N_("Current connection type doesn't support called function.")},
	{ERR_WRONGCRC, "WRONGCRC",			N_("CRC error.")},
	{ERR_INVALIDDATETIME, "INVALIDDATETIME",		N_("Invalid date or time specified.")},
	{ERR_MEMORY, "MEMORY",			N_("Phone memory error, maybe it is read only.")},
	{ERR_INVALIDDATA, "INVALIDDATA",		N_("Invalid data given to phone.")},
	{ERR_FILEALREADYEXIST, "FILEALREADYEXIST",		N_("File with specified name already exist.")},
	{ERR_FILENOTEXIST, "FILENOTEXIST",		N_("File with specified name doesn't exist.")},
	{ERR_SHOULDBEFOLDER, "SHOULDBEFOLDER",		N_("You have to give folder name and not file name.")},
	{ERR_SHOULDBEFILE, "SHOULDBEFILE",		N_("You have to give file name and not folder name.")},
	{ERR_NOSIM, "NOSIM",			N_("Can not access SIM card.")},
	{ERR_GNAPPLETWRONG, "GNAPPLETWRONG",		N_("Wrong GNAPPLET version in phone. Use version from currently used Gammu.")},
	{ERR_FOLDERPART, "FOLDERPART",		N_("Only part of folder has been listed.")},
	{ERR_FOLDERNOTEMPTY, "FOLDERNOTEMPTY",		N_("Folder must be empty.")},
	{ERR_DATACONVERTED, "DATACONVERTED",		N_("Data were converted.")},
	{ERR_UNCONFIGURED, "UNCONFIGURED",		N_("Gammu is not configured.")},
	{ERR_WRONGFOLDER, "WRONGFOLDER",		N_("Wrong folder used.")},
	{ERR_PHONE_INTERNAL, "PHONE_INTERNAL",		N_("Internal phone error.")},
	{ERR_WRITING_FILE, "WRITING_FILE",		N_("Error writing file to disk.")},
	{ERR_NONE_SECTION, "NONE_SECTION",		N_("No such section exists.")},
	{ERR_USING_DEFAULTS, "USING_DEFAULTS",		N_("Using default values.")},
	{ERR_CORRUPTED, "CORRUPTED",			N_("Corrupted data returned by phone.")},
	{ERR_BADFEATURE, "BADFEATURE",		N_("Bad feature string in configuration.")},
	{ERR_DISABLED, "DISABLED",		N_("Desired functionality has been disabled on compile time.")},
	{ERR_SPECIFYCHANNEL, "SPECIFYCHANNEL", N_("Bluetooth configuration requires channel option.")},
	{ERR_NOTRUNNING, "NOTRUNNING", N_("Service is not runnig.")},
	{ERR_NOSERVICE, "NOSERVICE", N_("Service configuration is missing.")},

	{0,	"",				""}
};

const char *GSM_ErrorName(GSM_Error e)
{
	const char	*def 	= NULL;
	int	i	= 0;

	while (PrintErrorEntries[i].ErrorNum != 0) {
		if (PrintErrorEntries[i].ErrorNum == e) {
			def 	= PrintErrorEntries[i].ErrorName;
			break;
		}
		i++;
	}

	return def;
}

const char *GSM_ErrorString(GSM_Error e)
{
	const char	*def 	= NULL;
	int	i	= 0;

	while (PrintErrorEntries[i].ErrorNum != 0) {
		if (PrintErrorEntries[i].ErrorNum == e) {
			def 	= PrintErrorEntries[i].ErrorText;
			break;
		}
		i++;
	}
	if (def == NULL) def = N_("Unknown error description.");

	return dgettext("libgammu", def);
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
	return &GSM_global_debug;
}

void GSM_LogError(GSM_StateMachine * s, const char * message, const GSM_Error err) {
	if (err != ERR_NONE) {
		smprintf(s, "%s failed with error %s[%d]: %s\n", message,
				GSM_ErrorName(err), err,
				GSM_ErrorString(err));
	}
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

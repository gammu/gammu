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
#ifdef LIBINTL_LIB_FOUND
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
	{ERR_DEVICEOPENERROR, "DEVICEOPENERROR",		N_("Error opening device. Unknown, busy or no permissions.")},
	{ERR_DEVICELOCKED, "DEVICELOCKED",		N_("Error opening device, it is locked.")},
	{ERR_DEVICENOTEXIST, "DEVICENOTEXIST",		N_("Error opening device, it doesn't exist.")},
	{ERR_DEVICEBUSY, "DEVICEBUSY",		N_("Error opening device, it is already opened by another app.")},
	{ERR_DEVICENOPERMISSION, "DEVICENOPERMISSION",	N_("Error opening device, you don't have the required permission.")},
	{ERR_DEVICENODRIVER, "DEVICENODRIVER",		N_("Error opening device. No required driver in operating system.")},
	{ERR_DEVICENOTWORK, "DEVICENOTWORK",		N_("Error opening device. Some hardware not connected/wrongly configured.")},
	{ERR_DEVICEDTRRTSERROR, "DEVICEDTRRTSERROR",		N_("Error setting device DTR or RTS.")},
	{ERR_DEVICECHANGESPEEDERROR, "DEVICECHANGESPEEDERROR",	N_("Error setting device speed. Maybe the speed is not supported?")},
	{ERR_DEVICEWRITEERROR, "DEVICEWRITEERROR",		N_("Error writing to the device.")},
	{ERR_DEVICEREADERROR, "DEVICEREADERROR",		N_("Error during reading from the device.")},
	{ERR_DEVICEPARITYERROR, "DEVICEPARITYERROR",		N_("Can't set parity on the device.")},
	{ERR_TIMEOUT, "TIMEOUT",			N_("No response in specified timeout. Probably the phone is not connected.")},
	{ERR_FRAMENOTREQUESTED, "FRAMENOTREQUESTED",		N_("Frame not requested right now. See <https://wammu.eu/support/bugs/> for information on how to report it.")},
	{ERR_UNKNOWNRESPONSE, "UNKNOWNRESPONSE",		N_("Unknown response from phone. See <https://wammu.eu/support/bugs/> for information on how to report it.")},
	{ERR_UNKNOWNFRAME, "UNKNOWNFRAME",		N_("Unknown frame. See <https://wammu.eu/support/bugs/> for information on how to report it.")},
	{ERR_UNKNOWNCONNECTIONTYPESTRING, "UNKNOWNCONNECTIONTYPESTRING",N_("Unknown connection type string. Check config file.")},
	{ERR_UNKNOWNMODELSTRING, "UNKNOWNMODELSTRING",	N_("Unknown model type string. Check config file.")},
	{ERR_SOURCENOTAVAILABLE, "SOURCENOTAVAILABLE",	N_("Some functions are not available for your system (disabled in config or not implemented).")},
	{ERR_NOTSUPPORTED, "NOTSUPPORTED",		N_("Function not supported by phone.")},
	{ERR_EMPTY, "EMPTY",			N_("Empty entry.")},
	{ERR_SECURITYERROR, "SECURITYERROR",		N_("Security error. Maybe no PIN?")},
	{ERR_INVALIDLOCATION, "INVALIDLOCATION",		N_("Invalid location. Maybe too high?")},
	{ERR_NOTIMPLEMENTED, "NOTIMPLEMENTED",		N_("Functionality not implemented. You are welcome to help.")},
	{ERR_FULL, "FULL",			N_("Memory full.")},
	{ERR_UNKNOWN, "UNKNOWN",			N_("Unknown error.")},
	{ERR_CANTOPENFILE, "CANTOPENFILE",		N_("Can not open specified file.")},
	{ERR_MOREMEMORY, "MOREMEMORY",		N_("More memory required…")},
	{ERR_PERMISSION, "PERMISSION",		N_("Operation not allowed by phone.")},
	{ERR_EMPTYSMSC, "EMPTYSMSC",			N_("No SMSC number given. Provide it manually or use the one configured on the phone.")},
	{ERR_INSIDEPHONEMENU, "INSIDEPHONEMENU",		N_("You're inside the phone menu (maybe editing?). Exit it and try again.")},
	{ERR_NOTCONNECTED, "NOTCONNECTED",		N_("Phone not connected.")},
	{ERR_WORKINPROGRESS, "WORKINPROGRESS",		N_("Function is currently being implemented. If you want to help, please contact authors.")},
	{ERR_PHONEOFF, "PHONEOFF",			N_("Phone disabled and connected to charger.")},
	{ERR_FILENOTSUPPORTED, "FILENOTSUPPORTED",		N_("File format not supported by Gammu.")},
	{ERR_BUG, "BUG",			N_("Nobody is perfect, some bug appeared in protocol implementation. Please contact authors.")},
	{ERR_CANCELED, "CANCELED",			N_("Transfer canceled by phone, maybe you pressed \"Cancel\" on the phone.")},
	{ERR_NEEDANOTHERANSWER, "NEEDANOTHERANSWER",		N_("Phone module need to send another answer frame.")}, /* This should be only internal. */
	{ERR_OTHERCONNECTIONREQUIRED, "OTHERCONNECTIONREQUIRED",	N_("Current connection type doesn't support called function.")},
	{ERR_WRONGCRC, "WRONGCRC",			N_("CRC error.")},
	{ERR_INVALIDDATETIME, "INVALIDDATETIME",		N_("Invalid date or time specified.")},
	{ERR_MEMORY, "MEMORY",			N_("Phone memory error, maybe it is read only?")},
	{ERR_INVALIDDATA, "INVALIDDATA",		N_("Invalid data given to phone.")},
	{ERR_FILEALREADYEXIST, "FILEALREADYEXIST",		N_("File with specified name already exists.")},
	{ERR_FILENOTEXIST, "FILENOTEXIST",		N_("File with specified name doesn't exist.")},
	{ERR_SHOULDBEFOLDER, "SHOULDBEFOLDER",		N_("You have to supply folder name and not filename.")},
	{ERR_SHOULDBEFILE, "SHOULDBEFILE",		N_("You have to supply filename and not folder name.")},
	{ERR_NOSIM, "NOSIM",			N_("Can not access SIM card.")},
	{ERR_GNAPPLETWRONG, "GNAPPLETWRONG",		N_("Wrong GNAPPLET version in use on phone. Use version from currently used Gammu.")},
	{ERR_FOLDERPART, "FOLDERPART",		N_("Only part of folder has been listed.")},
	{ERR_FOLDERNOTEMPTY, "FOLDERNOTEMPTY",		N_("Folder must be empty.")},
	{ERR_DATACONVERTED, "DATACONVERTED",		N_("Data converted.")},
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
	{ERR_NOTRUNNING, "NOTRUNNING", N_("Service is not running.")},
	{ERR_NOSERVICE, "NOSERVICE", N_("Service configuration is missing.")},
	{ERR_BUSY, "BUSY", N_("Command rejected because device was busy. Wait and restart.")},
	{ERR_COULDNT_CONNECT, "COULDNT_CONNECT", N_("Could not connect to the server.")},
	{ERR_COULDNT_RESOLVE, "COULDNT_RESOLVE", N_("Could not resolve the host name.")},
	{ERR_GETTING_SMSC, "GETTING_SMSC", N_("Failed to get SMSC number from phone.")},
	{ERR_ABORTED, "ABORTED", N_("Operation aborted.")},
	{ERR_INSTALL_NOT_FOUND, "INSTALL_NOT_FOUND", N_("Installation data not found, please consult debug log and/or documentation for more details.")},
	{ERR_READ_ONLY, "READ_ONLY", N_("Entry is read only.")},
	{ERR_NETWORK_ERROR, "NETWORK_ERROR", N_("Network error.")},
	{ERR_DB_VERSION, "DB_VERSION", N_("Invalid database version.")},
	{ERR_DB_DRIVER, "DB_DRIVER", N_("Failed to initialize DB driver.")},
	{ERR_DB_CONFIG, "DB_CONFIG", N_("Failed to configure DB driver.")},
	{ERR_DB_CONNECT, "DB_CONNECT", N_("Failed to connect to database.")},
	{ERR_DB_TIMEOUT, "DB_TIMEOUT", N_("Database connection timeout.")},
	{ERR_SQL, "SQL", N_("Error in executing SQL query.")},
	{ERR_MEMORY_NOT_AVAILABLE, "MEMORY_NOT_AVAILABLE", N_("The type of memory is not available or has been disabled.")},
	{ERR_INVALID_OPERATION, "INVALID_OPERATION", N_("The operation cannot be performed.")},

	{0,	"",				""}
};

const char *GSM_ErrorName(GSM_Error e)
{
	const char *def = NULL;
	int i = 0;

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
	const char *def	= NULL;
	int i = 0;

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
	static const char Buffer[] = GAMMU_VERSION;
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

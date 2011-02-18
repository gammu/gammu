/**
 * \file gammu-error.h
 * \author Michal Čihař
 *
 * Error codes definitions.
 */
#ifndef __gammu_error_h
#define __gammu_error_h

/**
 * \defgroup Error Error handling
 * Error handling and manipulation.
 */

/**
 * Error types.
 * \ingroup Error
 */
typedef enum {
	/**
	 * No error
	 */
	ERR_NONE = 1,
	/**
	 * Error during opening device
	 */
	ERR_DEVICEOPENERROR,
	/**
	 * Device locked
	 */
	ERR_DEVICELOCKED,
	/**
	 * Device does not exits
	 */
	ERR_DEVICENOTEXIST,
	/**
	 * Device is busy
	 */
	ERR_DEVICEBUSY,
	/**
	 * No permissions to open device
	 */
	ERR_DEVICENOPERMISSION,
	/**
	 * No driver installed for a device
	 */
	ERR_DEVICENODRIVER,
	/**
	 * Device doesn't seem to be working
	 */
	ERR_DEVICENOTWORK,
	/**
	 * Error during setting DTR/RTS in device
	 */
	ERR_DEVICEDTRRTSERROR,

	/** 10 **/
	/**
	 * Error during changing speed in device
	 */
	ERR_DEVICECHANGESPEEDERROR,
	/**
	 * Error during writing device
	 */
	ERR_DEVICEWRITEERROR,
	/**
	 * Error during reading device
	 */
	ERR_DEVICEREADERROR,
	/**
	 * Can't set parity on device
	 */
	ERR_DEVICEPARITYERROR,
	/**
	 * Command timed out
	 */
	ERR_TIMEOUT,
	/**
	 * Frame handled, but not requested in this moment
	 */
	ERR_FRAMENOTREQUESTED,
	/**
	 * Response not handled by gammu
	 */
	ERR_UNKNOWNRESPONSE,
	/**
	 * Frame not handled by gammu
	 */
	ERR_UNKNOWNFRAME,
	/**
	 * Unknown connection type given by user
	 */
	ERR_UNKNOWNCONNECTIONTYPESTRING,
	/**
	 * Unknown model given by user
	 */
	ERR_UNKNOWNMODELSTRING,

	/** 20 **/
	/**
	 * Some functions not compiled in your OS
	 */
	ERR_SOURCENOTAVAILABLE,
	/**
	 * Not supported by phone
	 */
	ERR_NOTSUPPORTED,
	/**
	 * Empty entry or transfer end.
	 */
	ERR_EMPTY,
	/**
	 * Not allowed
	 */
	ERR_SECURITYERROR,
	/**
	 * Too high or too low location...
	 */
	ERR_INVALIDLOCATION,
	/**
	 * Function not implemented
	 */
	ERR_NOTIMPLEMENTED,
	/**
	 * Memory is full
	 */
	ERR_FULL,
	/**
	 * Unknown response from phone
	 */
	ERR_UNKNOWN,
	/**
	 * Error during opening file
	 */
	ERR_CANTOPENFILE,
	/**
	 * More memory required
	 */
	ERR_MOREMEMORY,

	/** 30 **/
	/**
	 * No permission
	 */
	ERR_PERMISSION,
	/**
	 * SMSC number is empty
	 */
	ERR_EMPTYSMSC,
	/**
	 * Inside phone menu - can't make something
	 */
	ERR_INSIDEPHONEMENU,
	/**
	 * Phone NOT connected - can't make something
	 */
	ERR_NOTCONNECTED,
	/**
	 * Work in progress
	 */
	ERR_WORKINPROGRESS,
	/**
	 * Phone is disabled and connected to charger
	 */
	ERR_PHONEOFF,
	/**
	 * File format not supported by Gammu
	 */
	ERR_FILENOTSUPPORTED,
	/**
	 * Found bug in implementation or phone
	 */
	ERR_BUG,
	/**
	 * Action was canceled by user
	 */
	ERR_CANCELED,
	/**
	 * Inside Gammu: phone module need to send another answer frame
	 */
	ERR_NEEDANOTHERANSWER,

	/** 40 **/
	/**
	 * You need other connectin for this operation.
	 */
	ERR_OTHERCONNECTIONREQUIRED,
	/**
	 * Wrong CRC
	 */
	ERR_WRONGCRC,
	/**
	 * Invalid date/time
	 */
	ERR_INVALIDDATETIME,
	/**
	 * Phone memory error, maybe it is read only
	 */
	ERR_MEMORY,
	/**
	 * Invalid data given to phone
	 */
	ERR_INVALIDDATA,
	/**
	 * File with specified name already exist
	 */
	ERR_FILEALREADYEXIST,
	/**
	 * File with specified name doesn't exist
	 */
	ERR_FILENOTEXIST,
	/**
	 * You have to give folder (not file) name
	 */
	ERR_SHOULDBEFOLDER,
	/**
	 * You have to give file (not folder) name
	 */
	ERR_SHOULDBEFILE,
	/**
	 * Can not access SIM card
	 */
	ERR_NOSIM,

	/** 50 **/
	/**
	 * Invalid gnapplet version
	 */
	ERR_GNAPPLETWRONG,
	/**
	 * Only part of folders listed
	 */
	ERR_FOLDERPART,
	/**
	 * Folder is not empty
	 */
	ERR_FOLDERNOTEMPTY,
	/**
	 * Data were converted
	 */
	ERR_DATACONVERTED,
	/**
	 * Gammu is not configured.
	 */
	ERR_UNCONFIGURED,
	/**
	 * Wrong folder selected (eg. for SMS).
	 */
	ERR_WRONGFOLDER,
	/**
	 * Internal phone error (phone got crazy).
	 */
	ERR_PHONE_INTERNAL,
	/**
	 * Could not write to a file (on local filesystem).
	 */
	ERR_WRITING_FILE,
	/**
	 * No such section exists.
	 */
	ERR_NONE_SECTION,
	/**
	 * Using default values.
	 */
	ERR_USING_DEFAULTS,

	/** 60 **/
	/**
	 * Corrupted data returned by phone.
	 */
	ERR_CORRUPTED,
	/**
	 * Bad feature string.
	 */
	ERR_BADFEATURE,
	/**
	 * Some functions not compiled in your OS
	 */
	ERR_DISABLED,
	/**
	 * Bluetooth configuration requires channel option.
	 */
	ERR_SPECIFYCHANNEL,
	/**
	 * Service is not running.
	 */
	ERR_NOTRUNNING,
	/**
	 * Service setup is missing.
	 */
	ERR_NOSERVICE,
	/**
	 * Command failed. Try again.
	 */
	ERR_BUSY,
	/**
	 * Can not connect to server.
	 */
	ERR_COULDNT_CONNECT,
	/**
	 * Can not resolve host name.
	 */
	ERR_COULDNT_RESOLVE,
	/**
	 * Failed to get SMSC number from phone.
	 */
	ERR_GETTING_SMSC,

	/** 70 **/
	/**
	 * Operation aborted.
	 */
	ERR_ABORTED,
	/**
	 * Installation data not found.
	 */
	ERR_INSTALL_NOT_FOUND,
	/**
	 * Entry is read only.
	 */
	ERR_READ_ONLY,

	/**
	 * Just marker of highest error code, should not be used.
	 */
	ERR_LAST_VALUE
} GSM_Error;

/**
 * Returns text for error.
 *
 * \param e Error code.
 * \return Text (in current locales) describing error
 *
 * \ingroup Error
 */
const char *GSM_ErrorString(GSM_Error e);

/**
 * Returns name for error.
 *
 * \param e Error code.
 * \return Text with error name
 *
 * \ingroup Error
 */
const char *GSM_ErrorName(GSM_Error e);
#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

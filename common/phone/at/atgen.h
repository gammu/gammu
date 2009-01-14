/* (c) 2002-2007 by Marcin Wiacek and Michal Cihar */

/**
 * @file atgen.h
 * @author Michal Čihař
 * @author Marcin Wiacek
 */
/**
 * @addtogroup Phone
 * @{
 */
/**
 * @defgroup ATPhone AT phones communication
 *
 * This module implements standard AT commands.
 *
 * @see http://www.etsi.org
 * @see http://www.3gpp.org
 *
 * @{
 */

#ifndef atgen_h
#define atgen_h

#include <gammu-types.h>
#include <gammu-error.h>
#include <gammu-statemachine.h>

#include "../../misc/misc.h" /* For GSM_CutLines */

#include "motorola.h"

#ifndef GSM_USED_AT
#  define GSM_USED_AT
#endif
#ifndef GSM_USED_BLUEAT
#  define GSM_USED_BLUEAT
#endif
#ifndef GSM_USED_IRDAAT
#  define GSM_USED_IRDAAT
#endif

#define MAX_VCALENDAR_LOCATION 50

typedef enum {
	SMS_AT_PDU = 1,
	SMS_AT_TXT
} GSM_AT_SMS_Modes;

typedef enum {
	AT_Reply_OK = 1,
	AT_Reply_Connect,
	AT_Reply_Error,
	AT_Reply_Unknown,
	AT_Reply_CMSError,
	AT_Reply_CMEError,
	AT_Reply_SMSEdit
} GSM_AT_Reply_State;

/**
 * Manufacturer identification. This is usually used to enable some
 * hacks which are specific for all phones from one manufacturer.
 */
typedef enum {
	AT_Nokia = 1,
	AT_Alcatel,
	AT_Siemens,
	AT_HP,
	AT_Falcom,
	AT_Ericsson,
	AT_Sagem,
	AT_Samsung,
	AT_Mitsubishi,
	AT_Sharp,
	AT_Option,
	AT_Motorola,
	AT_Wavecom,
	AT_Unknown
} GSM_AT_Manufacturer;

/**
 * Character set definitions.
 */
typedef enum {
	/**
	 * Hex encoded GSM.
	 */
	AT_CHARSET_HEX = 1,
	/**
	 * GSM charset.
	 */
	AT_CHARSET_GSM,
	/**
	 * Hex encoded UCS2.
	 */
	AT_CHARSET_UCS2,
	/**
	 * Hex encoded UCS2.
	 */
	AT_CHARSET_UCS_2,
	/**
	 * UTF-8.
	 */
	AT_CHARSET_UTF8,
	/**
	 * UTF-8.
	 */
	AT_CHARSET_UTF_8,
	/**
	 * CP-437, not fully supported.
	 */
	AT_CHARSET_PCCP437,
	/**
	 * IRA, what is same as 7-bit ASCII.
	 */
	AT_CHARSET_IRA,
	/**
	 * 7-bit ASCII.
	 */
	AT_CHARSET_ASCII,
	/**
	 * ISO-8859-1, supported only with iconv.
	 */
	AT_CHARSET_ISO88591,
	/**
	 * ISO-8859-2, supported only with iconv.
	 */
	AT_CHARSET_ISO88592,
	/**
	 * ISO-8859-3, supported only with iconv.
	 */
	AT_CHARSET_ISO88593,
	/**
	 * ISO-8859-4, supported only with iconv.
	 */
	AT_CHARSET_ISO88594,
	/**
	 * ISO-8859-5, supported only with iconv.
	 */
	AT_CHARSET_ISO88595,
	/**
	 * ISO-8859-6, supported only with iconv.
	 */
	AT_CHARSET_ISO88596,
} GSM_AT_Charset;

/**
 * Character set preference indication.
 */
typedef enum {
	/**
	 * Prefer standard GSM character set.
	 */
	AT_PREF_CHARSET_NORMAL = 1,
	/**
	 * Prefer Unicode character set to get text without any possible
	 * garbling caused by conversion.
	 */
	AT_PREF_CHARSET_UNICODE,
	/**
	 * IRA is ASCII character set. This might be useful for some
	 * operations where we only want to transmit ASCII characters
	 * and we don't want to take care about encoding or encoding of
	 * some transmitted characters is not possible to GSM.
	 */
	AT_PREF_CHARSET_IRA,
	/**
	 * This option just sets again charset in phone. Useful after
	 * something what resets this information in phone.
	 */
	AT_PREF_CHARSET_RESET,
} GSM_AT_Charset_Preference;

typedef enum {
	AT_AVAILABLE = 1,
	AT_NOTAVAILABLE
} GSM_AT_Feature;

typedef enum {
	AT_Status,
	AT_NextEmpty,
	AT_Total,
	AT_First,
	AT_Sizes
} GSM_AT_NeededMemoryInfo;

/**
 * Maximal length of PDU cache line. It has to be at least
 * 160 * 2 (message can be 160 bytes and hex representation needs
 * twice more space).
 */
#define GSM_AT_MAXPDULEN 400

/**
 * Structure for SMS cache.
 */
typedef struct {
	/**
	 * Location of SMS (translated).
	 */
	int Location;
	/**
	 * State of message.
	 */
	int State;
	/**
	 * PDU data.
	 */
	char PDU[GSM_AT_MAXPDULEN];
} GSM_AT_SMS_Cache;

/**
 * Maximal length of phonebook memories list.
 */
#define AT_PBK_MAX_MEMORIES	200

typedef struct {
	/**
	 * Who is manufacturer
	 */
	GSM_AT_Manufacturer	Manufacturer;
	/**
	 * Allow to simply get each line in response
	 */
	GSM_CutLines		Lines;
	/**
	 * What response type - error, OK, etc.
	 */
	GSM_AT_Reply_State	ReplyState;
	/**
	 * Error codes from responses
	 */
	int			ErrorCode;
	/**
	 * Error description
	 */
    	char			*ErrorText;

	/**
	 * Last read PBK memory
	 */
	GSM_MemoryType		PBKMemory;
	/**
	 * Supported by phone PBK memories
	 */
	char			PBKMemories[AT_PBK_MAX_MEMORIES + 1];
	/**
	 * Next empty memory entry
	 */
	int			NextMemoryEntry;
	/**
	 * First memory entry to be read
	 */
	int			FirstMemoryEntry;
	/**
	 * Charset set in phone
	 */
	GSM_AT_Charset		Charset;
	/**
	 * True if phone encodes also commands and not only values
	 */
	bool			EncodedCommands;
	/**
	 * Charset to use for unicode mode
	 */
	GSM_AT_Charset		UnicodeCharset;
	/**
	 * Charset to use for non unicode mode
	 */
	GSM_AT_Charset		NormalCharset;
	/**
	 * Charset to use for IRA mode
	 */
	GSM_AT_Charset		IRACharset;
	GSM_AT_Feature		PBKSBNR;
	GSM_AT_Feature		PBK_SPBR;
	size_t			NumberLength;
	size_t			TextLength;
	int			MemorySize;
	int			MemoryUsed;

	GSM_SMSMemoryStatus	LastSMSStatus;
	int			LastSMSRead;
	int			FirstCalendarPos;
	int			FirstFreeCalendarPos;
	/**
	 * Can we write messages to SIM memory?
	 */
	GSM_AT_Feature		SIMSaveSMS;
	/**
	 * Can we write messages to phone memory?
	 */
	GSM_AT_Feature		PhoneSaveSMS;
	/**
	 * Is phone SMS memory available ?
	 */
	GSM_AT_Feature		PhoneSMSMemory;
	/**
	 * Whether to use Motorola like SMS folders.
	 */
	bool			MotorolaSMS;
	/**
	 * Is SIM SMS memory available ?
	 */
	GSM_AT_Feature		SIMSMSMemory;
	/**
	 * Last read SMS memory
	 */
	GSM_MemoryType		SMSMemory;
	/**
	 * PDU or TEXT mode for SMS ?
	 */
	GSM_AT_SMS_Modes	SMSMode;
	/**
	 * Does phone provide enhanced text mode readout of SMS flags
	 * in text mode?
	 */
	bool			SMSTextDetails;

	/**
	 * Mode used for incoming message notification.
	 */
	int			CNMIMode;
	/**
	 * Procedure used for incoming message notification.
	 */
	int			CNMIProcedure;
	/**
	 * Procedure used for incoming delivery report message notification.
	 */
	int			CNMIDeliverProcedure;
#ifdef GSM_ENABLE_CELLBROADCAST
	/**
	 * Mode used for incoming broadcast message notification.
	 */
	int			CNMIBroadcastProcedure;
#endif

	/**
	 * Whether phone support Motorola like mode switching.
	 */
	bool			Mode;
	/**
	 * Current Motorola mode.
	 */
	int			CurrentMode;
	GSM_File		file;
	/**
	 * Number of entries in SMSCache.
	 */
	int			SMSCount;
	/**
	 * Locations of non empty SMSes.
	 */
	GSM_AT_SMS_Cache	*SMSCache;
	/**
	 * Which folder do we read SMS from.
	 */
	int			SMSReadFolder;
	/**
	 * Mode of SQWE (Siemens phones and switching to OBEX).
	 */
	int			SQWEMode;
} GSM_Phone_ATGENData;

/**
 * Generates error code from current CMS error according to
 * specifications.
 *
 * \param s State machine structure.
 */
GSM_Error ATGEN_HandleCMSError	(GSM_StateMachine *s);

/**
 * Generates error code from current CME error according to
 * specifications.
 *
 * \param s State machine structure.
 */
GSM_Error ATGEN_HandleCMEError	(GSM_StateMachine *s);

GSM_Error ATGEN_DispatchMessage	(GSM_StateMachine *s);

/**
 * Wrapper around \ref GSM_WaitFor, which automatically sets
 * correct Motorola mode. It accepts same parameters as
 * \ref GSM_WaitFor.
 */
#define ATGEN_WaitFor(s, cmd, len, type, time, request) \
        error = MOTOROLA_SetMode(s, cmd); \
        if (error != ERR_NONE) return error; \
        error = GSM_WaitFor(s, cmd, len, type, time, request);

/**
 * Parses AT formatted reply. This is a bit like sprintf parser, but
 * specially focused on AT replies and can automatically convert text
 * encoding and decode some special fields.
 *
 * \par Parser tokens:
 * - \b \@i - Number, expects pointer to int.
 * - \b \@I - Optional number (string can be empty), expects pointer to int.
 * - \b \@l - Number, expects pointer to long int.
 * - \b \@s - String, will be converted from phone encoding, stripping
 *   quotes, expects pointer to unsigned char and size of storage.
 *   Encoding is somehow guessed.
 * - \b \@e - String, will be converted from phone encoding, stripping
 *   quotes, expects pointer to unsigned char and size of storage.
 *   No encoding guessing.
 * - \b \@S - String with Samsung specials (0x02 at beginning and 0x03
 *   at the end, encoded in utf-8), otherwise same as \@s.
 * - \b \@p - String with phone number hint for heuristics, otherwise
 *   same as \@s.
 * - \b \@r - Raw string, no conversion will be done, only stripped
 *   quotes, expects pointer to char and size of storage.
 * - \b \@d - Date, expects pointer to GSM_DateTime.
 * - \b \@\@ - \@ literal.
 * - \b \@0 - Ignore rest of input, same as .* regular expression.
 *
 * \par Special behaviour:
 * Any space is automatically treated as [[:space:]]* regular
 * expression. So use space whenever some weird implementation in phone
 * can insert it space.
 *
 * \param s State machine structure.
 * \param input Input string to parse.
 * \param format Format string for parser.
 * \param ... Pointers to various buffers as defined by format string.
 */
GSM_Error ATGEN_ParseReply(GSM_StateMachine *s, const unsigned char *input, const char *format, ...);

/**
 * Sets charset in phone according to preference.
 *
 * \param s State machine structure.
 * \param Prefer What charset setting is prefered.
 *
 * \return Error code.
 */
GSM_Error ATGEN_SetCharset(GSM_StateMachine *s, GSM_AT_Charset_Preference Prefer);

#endif
/*@}*/
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

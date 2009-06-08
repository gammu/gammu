/* (c) 2002-2008 by Marcin Wiacek and Michal Cihar */

/**
 * @file atgen.c
 * @author Michal Čihař
 * @author Marcin Wiacek
 */
/**
 * @ingroup Phone
 * @{
 */
/**
 * @addtogroup ATPhone
 * @{
 */

#define _GNU_SOURCE
#include <gammu-config.h>

#ifdef GSM_ENABLE_ATGEN

#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdarg.h>

#include "../../gsmcomon.h"
#include "../../gsmphones.h"
#include "../../misc/coding/coding.h"
#include "../../service/gsmpbk.h"
#include "../pfunc.h"

#include "atgen.h"
#include "atfunc.h"

#include "samsung.h"
#include "siemens.h"
#include "motorola.h"

#include "../../../helper/string.h"

#ifdef GSM_ENABLE_ALCATEL
GSM_Error ALCATEL_ProtocolVersionReply (GSM_Protocol_Message, GSM_StateMachine *);
#endif

#ifdef GSM_ENABLE_ATOBEX
#include "../atobex/atobexfunc.h"
#endif


typedef struct {
	const GSM_AT_Charset	charset;
	const char		*text;
	const gboolean		unicode;
	const gboolean		ira;
} GSM_AT_Charset_Info;

/**
 * List of charsets and text identifying them in phone responses, order
 * defines their preferences, so if first is found it is used.
 */
static GSM_AT_Charset_Info AT_Charsets[] = {
	{AT_CHARSET_HEX,	"HEX",		FALSE,	FALSE},
	{AT_CHARSET_GSM,	"GSM",		FALSE,	FALSE},
	{AT_CHARSET_PCCP437,	"PCCP437",	FALSE,	FALSE},
	{AT_CHARSET_UTF_8,	"UTF-8",	TRUE,	FALSE},
	{AT_CHARSET_UTF8,	"UTF8",		TRUE,	FALSE},
	{AT_CHARSET_UCS_2,	"UCS-2",	TRUE,	FALSE},
	{AT_CHARSET_UCS2,	"UCS2",		TRUE,	FALSE},
	{AT_CHARSET_IRA,	"IRA",		FALSE,	TRUE},
	{AT_CHARSET_ASCII,	"ASCII",	FALSE,	TRUE},
#ifdef ICONV_FOUND
	{AT_CHARSET_ISO88591,	"8859-1",	FALSE,	FALSE},
	{AT_CHARSET_ISO88592,	"8859-2",	FALSE,	FALSE},
	{AT_CHARSET_ISO88593,	"8859-3",	FALSE,	FALSE},
	{AT_CHARSET_ISO88594,	"8859-4",	FALSE,	FALSE},
	{AT_CHARSET_ISO88595,	"8859-5",	FALSE,	FALSE},
	{AT_CHARSET_ISO88596,	"8859-6",	FALSE,	FALSE},
#endif
	{0,			NULL,		FALSE,	FALSE}
};

typedef struct {
	int     Number;
	char    Text[60];
} ATErrorCode;

static ATErrorCode CMSErrorCodes[] = {
	/*
	 * Error codes not specified here were either undefined or reserved in my
	 * copy of specifications, if you have newer one, please fill in the gaps.
	 */
	/* 0...127 from GSM 04.11 Annex E-2 */
	{1,    "Unassigned (unallocated) number"},
	{8,    "Operator determined barring"},
	{10,   "Call barred"},
	{21,   "Short message transfer rejected"},
	{27,   "Destination out of service"},
	{28,   "Unidentified subscriber"},
	{29,   "Facility rejected"},
	{30,   "Unknown subscriber"},
	{38,   "Network out of order"},
	{41,   "Temporary failure"},
	{42,   "Congestion"},
	{47,   "Resources unavailable, unspecified"},
	{50,   "Requested facility not subscribed"},
	{69,   "Requested facility not implemented"},
	{81,   "Invalid short message transfer reference value"},
	{95,   "Invalid message, unspecified"},
	{96,   "Invalid mandatory information"},
	{97,   "Message type non-existent or not implemented"},
	{98,   "Message not compatible with short message protocol state"},
	{99,   "Information element non-existent or not implemented"},
	{111,  "Protocol error, unspecified"},
	{127,  "Interworking, unspecified"},
	/* 128...255 from GSM 03.40 subclause 9.2.3.22 */
	{0x80, "Telematic interworking not supported"},
	{0x81, "Short message Type 0 not supported"},
	{0x82, "Cannot replace short message"},
	{0x8F, "Unspecified TP-PID error"},
	{0x90, "Data coding scheme (alphabet) not supported"},
	{0x91, "Message class not supported"},
	{0x9F, "Unspecified TP-DCS error"},
	{0xA0, "Command cannot be actioned"},
	{0xA1, "Command unsupported"},
	{0xAF, "Unspecified TP-Command error"},
	{0xB0, "TPDU not supported"},
	{0xC0, "SC busy"},
	{0xC1, "No SC subscription"},
	{0xC2, "SC system failure"},
	{0xC3, "Invalid SME address"},
	{0xC4, "Destination SME barred"},
	{0xC5, "SM Rejected-Duplicate SM"},
	{0xC6, "TP-VPF not supported"},
	{0xC7, "TP-VP not supported"},
	{0xD0, "SIM SMS storage full"},
	{0xD1, "No SMS storage capability in SIM"},
	{0xD2, "Error in MS"},
	{0xD3, "Memory Capacity Exceede"},
	{0xD4, "SIM Application Toolkit Busy"},
	{0xFF, "Unspecified error cause"},
	/* From Siemens documentation, does not have to be valid for all vendors */
	{256, "Operation temporary not allowed"},
	{257, "call barred"},
	{258, "phone busy"},
	{259, "user abort"},
	{260, "invalid dial string"},
	{261, "ss not executed"},
	{262, "SIM blocked"},
	{263, "Invalid Block"},
	/* 300...511 from GSM 07.05 subclause 3.2.5 */
	{300,  "ME failure"},
	{301,  "SMS service of ME reserved"},
	{302,  "operation not allowed"},
	{303,  "operation not supported"},
	{304,  "invalid PDU mode parameter"},
	{305,  "invalid text mode parameter"},
	{310,  "SIM not inserted"},
	{311,  "SIM PIN required"},
	{312,  "PH-SIM PIN required"},
	{313,  "SIM failure"},
	{314,  "SIM busy"},
	{315,  "SIM wrong"},
	{316,  "SIM PUK required"},
	{317,  "SIM PIN2 required"},
	{318,  "SIM PUK2 required"},
	{320,  "memory failure"},
	{321,  "invalid memory index"},
	{322,  "memory full"},
	{330,  "SMSC address unknown"},
	{331,  "no network service"},
	{332,  "network timeout"},
	{340,  "no CNMA acknowledgement expected"},
	{500,  "unknown error"},
	/* > 512 are manufacturer specific according to GSM 07.05 subclause 3.2.5 */
	{516,  "Motorola - too high location?"},
	/* Siemens */
	{512, "User abort"},
	{513, "unable to store"},
	{514, "invalid status"},
	{515, "invalid character in address string"},
	{516, "invalid length"},
	{517, "invalid character in pdu"},
	{519, "invalid length or character"},
	{520, "invalid character in text"},
	{521, "timer expired"},
	{522, "Operation temporary not allowed"},
	{532, "SIM not ready"},
	{534, "Cell Broadcast error unknown"},
	{535, "PS busy"},
	{538, "invalid parameter"},
	{549, "incorrect PDU length"},
	{550, "invalid message type indication (MTI)"},
	{551, "invalid (non-hex) chars in address"},
	{553, "incorrect PDU length (UDL)"},
	{554, "incorrect SCA length"},
	{578, "GPRS - unspecified activation rejection"},
	{588, "GPRS - feature not supported"},
	{594, "GPRS - invalid address length"},
	{595, "GPRS - invalid character in address string"},
	{596, "GPRS - invalid cid value"},
	{607, "GPRS - missing or unknown APN"},
	{615, "network failure"},
	{616, "network is down"},
	{625, "GPRS - pdp type not supported"},
	{630, "GPRS - profile (cid) not defined"},
	{632, "GPRS - QOS not accepted"},
	{633, "GPRS - QOS validation fail"},
	{639, "service type not yet available"},
	{640, "operation of service temporary not allowed"},
	{643, "GPRS - unknown PDP address or type"},
	{644, "GPRS - unknown PDP context"},
	{646, "GPRS - QOS invalid parameter"},
	{764, "missing input value"},
	{765, "invalid input value"},
	{767, "operation failed"},
	{769, "unable to get control of required module"},
	{770, "SIM invalid - network reject"},
	{771, "call setup in progress"},
	{772, "SIM powered down"},
	{-1,   ""}
};

static ATErrorCode CMEErrorCodes[] = {
	/* CME Error codes from GSM 07.07 section 9.2 */
	{0,   "phone failure"},
	{1,   "no connection to phone"},
	{2,   "phone-adaptor link reserved"},
	{3,   "operation not allowed"},
	{4,   "operation not supported"},
	{5,   "PH-SIM PIN required"},
	{10,  "SIM not inserted"},
	{11,  "SIM PIN required"},
	{12,  "SIM PUK required"},
	{13,  "SIM failure"},
	{14,  "SIM busy"},
	{15,  "SIM wrong"},
	{16,  "incorrect password"},
	{17,  "SIM PIN2 required"},
	{18,  "SIM PUK2 required"},
	{20,  "memory full"},
	{21,  "invalid index"},
	{22,  "not found"},
	{23,  "memory failure"},
	{24,  "text string too long"},
	{25,  "invalid characters in text string"},
	{26,  "dial string too long"},
	{27,  "invalid characters in dial string"},
	{30,  "no network service"},
	{31,  "network timeout"},
	/* 3GPP TS 27.007 /2/ */
	{32,  "Network not allowed - emergency calls only."},
	{40,  "Network personalization PIN required."},
	{41,  "Network personalization PUK required."},
	{42,  "Network subset personalization PIN required."},
	{43,  "Network subset personalization PUK required."},
	{44,  "Service provider personalization PIN required."},
	{45,  "Service provider personalization PUK required."},
	{46,  "Corporate personalization PIN required."},
	{47,  "Corporate personalization PUK required."},
	{100, "unknown"},
	/* GPRS-related errors - (#X = GSM 04.08 cause codes) */
	{103,  "Illegal MS (#3)."},
	{106,  "Illegal ME (#6)."},
	{107,  "GPRS services not allowed (#7)."},
	{111,  "Public Land Mobile Network (PLMN) not allowed (#11)."},
	{112,  "Location area not allowed (#12)."},
	{113,  "Roaming not allowed in this location area (#13)."},
	/* Errors related to a failure in Activating a Context and
	   Other GPRS errors */
	{132,  "Service option not supported (#32)."},
	{133,  "Requested service option not subscribed (#33)."},
	{134,  "Service option temporarily out of order (#34)."},
	{148,  "Unspecified GPRS error."},
	{149,  "PDP authentication failure."},
	{150,  "Invalid mobile class."},
};

static char samsung_location_error[] = "[Samsung] Empty location";


GSM_Error ATGEN_HandleCMEError(GSM_StateMachine *s)
{
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;

	if (Priv->ErrorCode == 0) {
		smprintf(s, "CME Error occured, but it's type not detected\n");
	} else if (Priv->ErrorText == NULL) {
		smprintf(s, "CME Error %i, no description available\n", Priv->ErrorCode);
	} else {
		smprintf(s, "CME Error %i: \"%s\"\n", Priv->ErrorCode, Priv->ErrorText);
	}
	/* For error codes descriptions see table a bit above */
	switch (Priv->ErrorCode) {
		case -1:
			return ERR_EMPTY;
		case 3:
			return ERR_PERMISSION;
		case 4:
			return ERR_NOTSUPPORTED;
		case 5:
		case 11:
		case 12:
		case 16:
		case 17:
		case 18:
		case 40:
		case 41:
		case 42:
		case 43:
		case 44:
		case 45:
		case 46:
		case 47:
			return ERR_SECURITYERROR;
		case 10:
		case 13:
		case 14:
		case 15:
			return ERR_NOSIM;
		case 20:
			return ERR_FULL;
		case 21:
			return ERR_INVALIDLOCATION;
		case 22:
			return ERR_EMPTY;
		case 23:
			return ERR_MEMORY;
		case 24:
		case 25:
		case 26:
		case 27:
			return ERR_INVALIDDATA;
		default:
			return ERR_UNKNOWN;
	}
}

GSM_Error ATGEN_HandleCMSError(GSM_StateMachine *s)
{
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;

	if (Priv->ErrorCode == 0) {
		smprintf(s, "CMS Error occured, but it's type not detected\n");
	} else if (Priv->ErrorText == NULL) {
		smprintf(s, "CMS Error %i, no description available\n", Priv->ErrorCode);
	} else {
		smprintf(s, "CMS Error %i: \"%s\"\n", Priv->ErrorCode, Priv->ErrorText);
	}
	/* For error codes descriptions see table a bit above */
	switch (Priv->ErrorCode) {
		case 0xD3:
			return ERR_FULL;
		case 0:
		case 300:
			return ERR_PHONE_INTERNAL;
		case 304:
			return ERR_NOTSUPPORTED;
		case 305:
			return ERR_BUG;
		case 302:
		case 311:
		case 312:
		case 316:
		case 317:
		case 318:
			return ERR_SECURITYERROR;
		case 313:
		case 314:
		case 315:
			return ERR_NOSIM;
		case 322:
			return ERR_FULL;
		case 321:
		case 516:
			return ERR_INVALIDLOCATION;
		default:
			return ERR_UNKNOWN;
	}
}

/**
 * Checks whether string contains some non hex chars.
 *
 * \param text String to check.
 *
 * \return True when text does not contain non hex chars.
 */
INLINE gboolean ATGEN_HasOnlyHexChars(const char *text, const size_t length)
{
	size_t i;

	for (i = 0; i < length; i++) {
		if (!isxdigit((int)text[i])) return FALSE;
	}

	return TRUE;
}

/**
 * Detects whether given text can be UCS2.
 *
 * \param s State machine structure.
 * \param len Length of string.
 * \param text Text.
 * \return True when text can be UCS2.
 */
INLINE gboolean ATGEN_IsUCS2(const char *text, const size_t length)
{
	return (length > 8) &&
		(length % 4 == 0) &&
		ATGEN_HasOnlyHexChars(text, length);
}

/**
 * Detects whether given text can be HEX.
 *
 * \param s State machine structure.
 * \param len Length of string.
 * \param text Text.
 * \return True when text can be HEX.
 */
INLINE gboolean ATGEN_IsHex(const char *text, const size_t length)
{
	return (length > 4) &&
		(length % 2 == 0) &&
		ATGEN_HasOnlyHexChars(text, length);
}

/**
 * Encodes text to current phone charset.
 *
 * \param s State machine structure.
 * \param input Input string.
 * \param inlength Length of string to convert.
 * \param output Storage for converted text.
 * \param outlength Size of output storage.
 *
 * \return Error code.
 */
GSM_Error ATGEN_EncodeText(GSM_StateMachine *s,
		const unsigned char *input,
		const size_t inlength,
		unsigned char *output,
		const size_t outlength,
		size_t *resultlength
		)
{
	size_t len = inlength;
	unsigned char *uname;
	GSM_Phone_ATGENData 	*Priv 	= &s->Phone.Data.Priv.ATGEN;

	/* As input is unicode, we should not need that much memory, but it is safe */
	uname = (unsigned char *)malloc(2 * (inlength + 1));
	if (uname == NULL) return ERR_MOREMEMORY;

	switch (Priv->Charset) {
		case AT_CHARSET_HEX:
			EncodeDefault(uname, input, &len, TRUE, NULL);
			EncodeHexBin(output, uname, len);
			len = strlen(output);
			break;
		case AT_CHARSET_GSM:
			smprintf(s, "str: %s\n", DecodeUnicodeString(input));
			EncodeDefault(output, input, &len, TRUE, NULL);
			break;
		case AT_CHARSET_UCS2:
		case AT_CHARSET_UCS_2:
			EncodeHexUnicode(output, input, UnicodeLength(input));
			len = strlen(output);
			break;
  		case AT_CHARSET_IRA:
  		case AT_CHARSET_ASCII:
			return ERR_NOTSUPPORTED;
  		case AT_CHARSET_UTF8:
  		case AT_CHARSET_UTF_8:
			EncodeUTF8(output, input);
			len = strlen(output);
  			break;
#ifdef ICONV_FOUND
  		case AT_CHARSET_PCCP437:
			IconvEncode("CP437", input, 2 * len, output, outlength);
			len = strlen(output);
			break;
  		case AT_CHARSET_ISO88591:
			IconvEncode("ISO-8859-1", input, 2 * len, output, outlength);
			len = strlen(output);
			break;
  		case AT_CHARSET_ISO88592:
			IconvEncode("ISO-8859-2", input, 2 * len, output, outlength);
			len = strlen(output);
			break;
  		case AT_CHARSET_ISO88593:
			IconvEncode("ISO-8859-3", input, 2 * len, output, outlength);
			len = strlen(output);
			break;
  		case AT_CHARSET_ISO88594:
			IconvEncode("ISO-8859-4", input, 2 * len, output, outlength);
			len = strlen(output);
			break;
  		case AT_CHARSET_ISO88595:
			IconvEncode("ISO-8859-5", input, 2 * len, output, outlength);
			len = strlen(output);
			break;
  		case AT_CHARSET_ISO88596:
			IconvEncode("ISO-8859-6", input, 2 * len, output, outlength);
			len = strlen(output);
			break;
#else
		case AT_CHARSET_PCCP437:
			/* FIXME: correctly encode to PCCP437 */
			smprintf(s, "str: %s\n", DecodeUnicodeString(input));
			EncodeDefault(output, input, &len, TRUE, NULL);
			break;
#endif
		default:
			smprintf(s, "Unsupported charset! (%d)\n", Priv->Charset);
			free(uname);
			return ERR_SOURCENOTAVAILABLE;
		}
	*resultlength = len;
	free(uname);
	return ERR_NONE;
}


/**
 * Decodes text from phone encoding to internal representation.
 *
 * \param s State machine structure.
 * \param input Input string.
 * \param length Length of string to convert.
 * \param output Storage for converted text.
 * \param outlength Size of output storage.
 * \param guess Allow guessing whether input is really encoded.
 * \param phone Whether input is phone number, used only when guessing.
 *
 * \return Error code.
 */
GSM_Error ATGEN_DecodeText(GSM_StateMachine *s,
		const unsigned char *input,
		const size_t length,
		unsigned char *output,
		const size_t outlength,
		const gboolean guess,
		const gboolean phone)
{
	unsigned char *buffer;
	GSM_AT_Charset charset;
	GSM_Phone_ATGENData 	*Priv 	= &s->Phone.Data.Priv.ATGEN;

	/* Default to charset from state machine */
	charset = s->Phone.Data.Priv.ATGEN.Charset;

	/* Can we do guesses? */
	if (guess) {
		/* Are there HEX only chars? */
		if  (charset == AT_CHARSET_HEX
			&& ! ATGEN_IsHex(input, length)) {
			charset = AT_CHARSET_GSM;
		}
		/*
		 * Motorola sometimes replies in UCS2 while there is HEX chosen.
		 * If string starts with two zeroes, it is definitely not HEX.
		 */
		if  (charset == AT_CHARSET_HEX
			&& ATGEN_IsUCS2(input, length)
			&& input[0] == '0'
			&& input[1] == '0'
			&& input[4] == '0'
			&& input[5] == '0'
			) {
			charset = AT_CHARSET_UCS2;
		}
		/*
		 * Motorola sometimes criples HEX reply while UCS-2 is chosen.
		 * It seems to be identified by trailing zero.
		 */
		if  (charset == AT_CHARSET_UCS2
			&& ATGEN_IsHex(input, length)
			&& Priv->Manufacturer == AT_Motorola
			&& input[length - 1] == '0'
			&& input[length - 2] == '0'
			) {
			charset = AT_CHARSET_HEX;
		}
		/*
		 * For phone numbers, we can assume all unicode chars
		 * will be < 256, so they will fit one byte.
		 */
		if  (charset == AT_CHARSET_UCS2
			&& (! ATGEN_IsUCS2(input, length) ||
				(phone &&
				(input[0] != '0' ||
				 input[1] != '0' ||
				 input[4] != '0' ||
				 input[5] != '0'
				)))) {
			charset = AT_CHARSET_GSM;
		}
		/*
		 * Phone number can also contain email, catch it by @.
		 */
		if  (charset == AT_CHARSET_GSM
			&& phone
			&& (! ATGEN_IsUCS2(input, length))
			&& strchr(input, '@') != NULL) {
			charset = AT_CHARSET_UTF8;
		}
		/*
		 * Phone number are unusally not that long
		 */
		if  (charset == AT_CHARSET_GSM
			&& phone
			&& length >= 16
			&& ATGEN_IsUCS2(input, length)) {
			charset = AT_CHARSET_UCS2;
		}
	}

	/* Check for broken phones */
	if (charset == AT_CHARSET_GSM &&
		GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_FORCE_UTF8)) {
		charset = AT_CHARSET_UTF8;
	}

	/* Finally do conversion */
	switch (charset) {
		case AT_CHARSET_HEX:
			/* Length must be enough, because we have two chars for byte */
			buffer = (unsigned char *)malloc(length);
			if (buffer == NULL) {
				return ERR_MOREMEMORY;
			}
 			DecodeHexBin(buffer, input, length);
			if (2 * strlen(buffer) >= outlength) return ERR_MOREMEMORY;
   			DecodeDefault(output, buffer, strlen(buffer), FALSE, NULL);
			free(buffer);
  			break;
  		case AT_CHARSET_GSM:
			if (2 * length >= outlength) return ERR_MOREMEMORY;
  			DecodeDefault(output, input, length, FALSE, NULL);
  			break;
  		case AT_CHARSET_UCS2:
  		case AT_CHARSET_UCS_2:
			if (length / 2 >= outlength) return ERR_MOREMEMORY;
 			DecodeHexUnicode(output, input, length);
  			break;
  		case AT_CHARSET_IRA: /* IRA is ASCII only, so it's safe to treat is as UTF-8 */
		case AT_CHARSET_ASCII:
  		case AT_CHARSET_UTF8:
  		case AT_CHARSET_UTF_8:
			if (2 * length >= outlength) return ERR_MOREMEMORY;
 			DecodeUTF8(output, input, length);
  			break;
#ifdef ICONV_FOUND
  		case AT_CHARSET_PCCP437:
			IconvDecode("CP437", input, length, output, outlength);
			break;
  		case AT_CHARSET_ISO88591:
			IconvDecode("ISO-8859-1", input, length, output, outlength);
			break;
  		case AT_CHARSET_ISO88592:
			IconvDecode("ISO-8859-2", input, length, output, outlength);
			break;
  		case AT_CHARSET_ISO88593:
			IconvDecode("ISO-8859-3", input, length, output, outlength);
			break;
  		case AT_CHARSET_ISO88594:
			IconvDecode("ISO-8859-4", input, length, output, outlength);
			break;
  		case AT_CHARSET_ISO88595:
			IconvDecode("ISO-8859-5", input, length, output, outlength);
			break;
  		case AT_CHARSET_ISO88596:
			IconvDecode("ISO-8859-6", input, length, output, outlength);
			break;
#else
  		case AT_CHARSET_PCCP437:
  			/* FIXME: correctly decode PCCP437 */
			if (2 * length >= outlength) return ERR_MOREMEMORY;
  			DecodeDefault(output, input, length, FALSE, NULL);
			break;
#endif
		default:
			smprintf(s, "Unsupported charset! (%d)\n", charset);
			return ERR_SOURCENOTAVAILABLE;
	}

	return ERR_NONE;
}

int ATGEN_ExtractOneParameter(unsigned char *input, unsigned char *output)
{
	int	position=0;
	gboolean	inside_quotes = FALSE;

	while ((*input!=',' || inside_quotes) && *input!=0x0d && *input!=0x00) {
		if (*input == '"') inside_quotes = ! inside_quotes;
		*output = *input;
		input	++;
		output	++;
		position++;
	}
	*output=0;
	position++;
	return position;
}

/**
 * Grabs single string parameter from AT command reply. Removing possible quotes.
 *
 * \param s State machine structure.
 * \param input Input string to parse.
 * \param output Pointer to pointer to char, buffer will be allocated.
 *
 * \return Length of parsed string.
 */
size_t ATGEN_GrabString(GSM_StateMachine *s, const unsigned char *input, unsigned char **output)
{
	size_t size = 4, position = 0;
	gboolean inside_quotes = FALSE;

	/* Allocate initial buffer in case string is empty */
	*output = (unsigned char *)malloc(size);
	if (*output == NULL) {
		smprintf(s, "Ran out of memory!\n");
		return 0;
	}

	while (inside_quotes ||
			(  *input != ','
			&& *input != ')'
			&& *input != 0x0d
			&& *input != 0x0a
			&& *input != 0x00)) {
		/* Check for quotes */
		if (*input == '"') {
			inside_quotes = ! inside_quotes;
		}

		/* We also allocate space for traling zero */
		if (position + 2 > size) {
			size += 10;
			*output = (unsigned char *)realloc(*output, size);
			if (*output == NULL) {
				smprintf(s, "Ran out of memory!\n");
				return 0;
			}
		}

		/* Copy to output */
		(*output)[position] = *input;
		position++;
		input	++;
	}

	(*output)[position] = 0;

	/* Strip quotes */
	if ((*output)[0] == '"' && (*output)[position - 1]) {
		memmove(*output, (*output) + 1, position - 2);
		(*output)[position - 2] = 0;
	}

	smprintf(s, "Grabbed string from reply: \"%s\" (parsed %ld bytes)\n", *output, (long)position);
	return position;
}

/**
 * This function parses datetime strings in the format:
 * [YY[YY]/MM/DD,]hh:mm[:ss[+TZ]] , [] enclosed parts are optional
 * (or the same hex/unicode encoded).
 *
 * @todo Too much static buffers are used here.
 */
GSM_Error ATGEN_DecodeDateTime(GSM_StateMachine *s, GSM_DateTime *dt, unsigned char *_input)
{
	unsigned char		buffer[100];
	unsigned char		*pos;
	unsigned char		buffer_unicode[200];
	unsigned char		input[100];
	GSM_Error error;

	strncpy(input, _input, 100);
	input[99] = '\0';
	pos = input;

	/* Strip possible quotes */
	if (*pos == '"') pos++;
	if (input[strlen(pos) - 1] == '"') input[strlen(pos) - 1] = 0;

	/* Convert to normal charset */
	error = ATGEN_DecodeText(s,
			pos, strlen(pos),
			buffer_unicode, sizeof(buffer_unicode),
			TRUE, FALSE);
	if (error != ERR_NONE) return error;
	DecodeUnicode(buffer_unicode, buffer);

	pos = buffer;

	/* Strip possible quotes again */
	if (*pos == '"') pos++;
	if (buffer[strlen(pos) - 1] == '"') buffer[strlen(pos) - 1] = 0;
	/* some phones report only time (HH:MM) in the alarm */
	if (strchr(pos, '/')) {
		/* date present */
		/* Samsung phones report year as %d instead of %02d */
		dt->Year = atoi(pos);
		if (dt->Year > 80 && dt->Year < 1000) {
			dt->Year += 1900;
		} else if (dt->Year < 100) {
			dt->Year += 2000;
		}
		pos = strchr(pos, '/');
		pos++;
		dt->Month = atoi(pos);
		pos = strchr(pos, '/');
		if (pos == NULL) return ERR_UNKNOWN;
		pos++;
		dt->Day = atoi(pos);
		pos = strchr(pos, ',');
		if (pos == NULL) return ERR_UNKNOWN;
		pos++;
	} else {
		/* if date was not found, it is still necessary to initialize
		   the variables, maybe Today() would be better in some replies */
		dt->Year=0;
		dt->Month=0;
		dt->Day=0;
	}

	/* time present */
	dt->Hour = atoi(pos);
	pos = strchr(pos, ':');
	if (pos == NULL) return ERR_UNKNOWN;
	pos++;
	dt->Minute = atoi(pos);
	pos = strchr(pos, ':');
	if (pos!=NULL) {
	        /* seconds present */
		pos++;
		dt->Second = atoi(pos);
	} else {
		dt->Second=0;
	}

	if ((pos != NULL) && (*pos == '+' || *pos == '-')) {
		/* timezone present */
		dt->Timezone = (*pos == '+' ? 1 : -1) * atoi(pos+1) * 3600;
	} else {
		dt->Timezone = 0;
	}
	return ERR_NONE;
}

GSM_Error ATGEN_ParseReply(GSM_StateMachine *s, const unsigned char *input, const char *format, ...)
{
	const char *fmt = format;
	const char *inp = input;
	char *endptr;
	GSM_DateTime *out_dt;
	char *out_s, *search_pos;
	unsigned char *out_us;
	unsigned char *buffer;
	unsigned char *buffer2;
	size_t length;
	size_t storage_size;
	int *out_i;
	long int *out_l;
	va_list ap;
	GSM_Error error = ERR_NONE;

	smprintf(s, "Parsing %s with %s\n", input, format);

	va_start(ap, format);
	while (*fmt) {
		switch(*fmt++) {
			case '@':
				if (*fmt == 0) {
					smprintf(s, "Invalid format string: %s\n", format);
					error = ERR_BUG;
					goto end;
				}
				switch(*fmt++) {
					case 'i':
						out_i = va_arg(ap, int *);
						*out_i = strtol(inp, &endptr, 10);
						if (endptr == inp) {
							error = ERR_UNKNOWNRESPONSE;
							goto end;
						}
						smprintf(s, "Parsed int %d\n", *out_i);
						inp = endptr;
						break;
					case 'I':
						out_i = va_arg(ap, int *);
						*out_i = strtol(inp, &endptr, 10);
						if (endptr == inp) {
							smprintf(s, "Number empty\n");
							*out_i = 0;
						} else {
							smprintf(s, "Parsed int %d\n", *out_i);
							inp = endptr;
						}
						break;
					case 'l':
						out_l = va_arg(ap, long int *);
						*out_l = strtol(inp, &endptr, 10);
						if (endptr == inp) {
							error = ERR_UNKNOWNRESPONSE;
							goto end;
						}
						smprintf(s, "Parsed long int %ld\n", *out_l);
						inp = endptr;
						break;
					case 'p':
						out_s = va_arg(ap, char *);
						storage_size = va_arg(ap, size_t);
						length = ATGEN_GrabString(s, inp, &buffer);
						smprintf(s, "Parsed phone string \"%s\"\n", buffer);
						error = ATGEN_DecodeText(s,
								buffer, strlen(buffer),
								out_s, storage_size,
								TRUE, TRUE);
						if (error == ERR_NONE) {
							smprintf(s, "Phone string decoded as \"%s\"\n", DecodeUnicodeString(out_s));
						}
						free(buffer);
						if (error != ERR_NONE) {
							goto end;
						}
						inp += length;
						break;
					case 's':
						out_s = va_arg(ap, char *);
						storage_size = va_arg(ap, size_t);
						length = ATGEN_GrabString(s, inp, &buffer);
						smprintf(s, "Parsed generic string \"%s\"\n", buffer);
						error = ATGEN_DecodeText(s,
								buffer, strlen(buffer),
								out_s, storage_size,
								TRUE, FALSE);
						if (error == ERR_NONE) {
							smprintf(s, "Generic string decoded as \"%s\"\n", DecodeUnicodeString(out_s));
						}
						free(buffer);
						if (error != ERR_NONE) {
							goto end;
						}
						inp += length;
						break;
					case 't':
						out_s = va_arg(ap, char *);
						storage_size = va_arg(ap, size_t);
						length = ATGEN_GrabString(s, inp, &buffer);
						smprintf(s, "Parsed string with length \"%s\"\n", buffer);
						if (!isdigit((int)buffer[0])) {
							free(buffer);
							error = ERR_UNKNOWNRESPONSE;
							goto end;
						}
						search_pos = strchr(buffer, ',');
						if (search_pos == NULL) {
							free(buffer);
							error = ERR_UNKNOWNRESPONSE;
							goto end;
						}
						search_pos++;
						error = ATGEN_DecodeText(s,
								search_pos, strlen(search_pos),
								out_s, storage_size,
								TRUE, FALSE);
						if (error == ERR_NONE) {
							smprintf(s, "String with length decoded as \"%s\"\n", DecodeUnicodeString(out_s));
						}
						free(buffer);
						if (error != ERR_NONE) {
							goto end;
						}
						inp += length;
						break;
					case 'e':
						out_s = va_arg(ap, char *);
						storage_size = va_arg(ap, size_t);
						length = ATGEN_GrabString(s, inp, &buffer);
						smprintf(s, "Parsed generic string \"%s\"\n", buffer);
						error = ATGEN_DecodeText(s,
								buffer, strlen(buffer),
								out_s, storage_size,
								FALSE, FALSE);
						if (error == ERR_NONE) {
							smprintf(s, "Generic string decoded as \"%s\"\n", DecodeUnicodeString(out_s));
						}
						free(buffer);
						if (error != ERR_NONE) {
							goto end;
						}
						inp += length;
						break;
					case 'S':
						out_s = va_arg(ap, char *);
						storage_size = va_arg(ap, size_t);
						length = ATGEN_GrabString(s, inp, &buffer);
						if (buffer[0] == 0x02 && buffer[strlen(buffer) - 1] == 0x03) {
							memmove(buffer, buffer + 1, strlen(buffer) - 2);
							buffer[strlen(buffer) - 2] = 0;
						}
						smprintf(s, "Parsed Samsung string \"%s\"\n", buffer);
						DecodeUTF8(out_s, buffer, strlen(buffer));
						smprintf(s, "Samsung string decoded as \"%s\"\n", DecodeUnicodeString(out_s));
						free(buffer);
						inp += length;
						break;
					case 'r':
						out_us = va_arg(ap, unsigned char *);
						storage_size = va_arg(ap, size_t);
						length = ATGEN_GrabString(s, inp, &buffer);
						smprintf(s, "Parsed raw string \"%s\"\n", buffer);
						if (strlen(buffer) > storage_size) {
							free(buffer);
							error = ERR_MOREMEMORY;
							goto end;
						}
						strcpy(out_us, buffer);
						free(buffer);
						inp += length;
						break;
					case 'd':
						out_dt = va_arg(ap, GSM_DateTime *);
						length = ATGEN_GrabString(s, inp, &buffer);
						/* Fix up reply from broken phones which split
						 * date to two strings */
						if ( *(inp + length) == ',' &&
								strchr(buffer, ',') == NULL
								) {
							length++;
							length += ATGEN_GrabString(s, inp + length, &buffer2);
							buffer = realloc(buffer, length + 2);
							strcat(buffer, ",");
							strcat(buffer, buffer2);
							free(buffer2);
						}
						smprintf(s, "Parsed string for date \"%s\"\n", buffer);
						error = ATGEN_DecodeDateTime(s, out_dt, buffer);
						free(buffer);
						if (error != ERR_NONE) {
							goto end;
						}
						inp += length;
						break;
					case '@':
						if (*inp++ != '@') {
							error = ERR_UNKNOWNRESPONSE;
							goto end;
						}
						break;
					case '0':
						/* Just skip the rest */
						goto end;
					default:
						smprintf(s, "Invalid format string (@%c): %s\n", *(fmt - 1), format);
						error = ERR_BUG;
						goto end;
				}
				break;
			case ' ':
				while (isspace((int)*inp)) inp++;
				break;
			default:
				if (*inp++ != *(fmt - 1)) {
					error = ERR_UNKNOWNRESPONSE;
					goto end;
				}
				break;
		}
	}

	/* Ignore trailing spaces */
	while (isspace((int)*inp)) inp++;

	if (*inp != 0) {
		smprintf(s, "String do not end same!\n");
		error = ERR_UNKNOWNRESPONSE;
		goto end;
	}
end:
	va_end(ap);
	return error;
}

GSM_Error ATGEN_DispatchMessage(GSM_StateMachine *s)
{
	GSM_Phone_ATGENData 	*Priv 	= &s->Phone.Data.Priv.ATGEN;
	GSM_Protocol_Message	*msg	= s->Phone.Data.RequestMsg;
	int 			i	= 0, j, k;
	const char		*err, *line;
	ATErrorCode		*ErrorCodes = NULL;

	SplitLines(msg->Buffer, msg->Length, &Priv->Lines, "\x0D\x0A", 2, TRUE);

	/* Find number of lines */
	while (Priv->Lines.numbers[i*2+1] != 0) {
		/* FIXME: handle special chars correctly */
		smprintf(s, "%i \"%s\"\n",i+1,GetLineString(msg->Buffer,&Priv->Lines,i+1));
		i++;
	}

	Priv->ReplyState 	= AT_Reply_Unknown;
	Priv->ErrorText     	= NULL;
	Priv->ErrorCode     	= 0;

	line = GetLineString(msg->Buffer,&Priv->Lines,i);
	if (!strcmp(line,"OK"))		Priv->ReplyState = AT_Reply_OK;
	if (!strcmp(line,"> "))		Priv->ReplyState = AT_Reply_SMSEdit;
	if (!strcmp(line,"CONNECT"))	Priv->ReplyState = AT_Reply_Connect;
	if (!strcmp(line,"ERROR"  ))	Priv->ReplyState = AT_Reply_Error;
	if (!strcmp(line,"NO CARRIER"  ))	Priv->ReplyState = AT_Reply_Error;
	if (!strncmp(line,"+CME ERROR:",11)) {
		Priv->ReplyState = AT_Reply_CMEError;
		ErrorCodes = CMEErrorCodes;
	}
	if (!strncmp(line,"+CMS ERROR:",11)) {
		Priv->ReplyState = AT_Reply_CMSError;
		ErrorCodes = CMSErrorCodes;
	}

	/* Huawei E220 returns COMMAND NOT SUPPORT on AT+MODE=2 */
	if (!strncmp(line, "COMMAND NOT SUPPORT", 19)) {
		Priv->ReplyState = AT_Reply_Error;
	}

	/* FIXME: Samsung phones can answer +CME ERROR:-1 meaning empty location */
	if (Priv->ReplyState == AT_Reply_CMEError && Priv->Manufacturer == AT_Samsung) {
		err = line + 11;
		Priv->ErrorCode = atoi(err);

		if (Priv->ErrorCode == -1) {
			Priv->ErrorText = samsung_location_error;
			return GSM_DispatchMessage(s);
		}
	}

	if (Priv->ReplyState == AT_Reply_CMEError || Priv->ReplyState == AT_Reply_CMSError) {
	        j = 0;
		/* One char behind +CM[SE] ERROR */
		err = line + 11;
		while (err[j] && !isalnum((int)err[j])) j++;
		if (isdigit((int)err[j])) {
			Priv->ErrorCode = atoi(&(err[j]));
			k = 0;
			while (ErrorCodes[k].Number != -1) {
				if (ErrorCodes[k].Number == Priv->ErrorCode) {
					Priv->ErrorText = ErrorCodes[k].Text;
					break;
				}
				k++;
			}
		} else if (isalpha((int)err[j])) {
			k = 0;
			while (ErrorCodes[k].Number != -1) {
				if (!strncmp(err + j, ErrorCodes[k].Text, strlen(ErrorCodes[k].Text))) {
					Priv->ErrorCode = ErrorCodes[k].Number;
					Priv->ErrorText = ErrorCodes[k].Text;
					break;
				}
				k++;
			}
		}
	}
	return GSM_DispatchMessage(s);
}

GSM_Error ATGEN_GenericReplyIgnore(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s UNUSED)
{
	return ERR_NONE;
}

GSM_Error ATGEN_GenericReply(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
		case AT_Reply_OK:
		case AT_Reply_Connect:
			return ERR_NONE;
		case AT_Reply_Error:
			return ERR_UNKNOWN;
		case AT_Reply_CMSError:
			return ATGEN_HandleCMSError(s);
		case AT_Reply_CMEError:
			return ATGEN_HandleCMEError(s);
		default:
			break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_SQWEReply(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;

	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
		case AT_Reply_OK:
			/* Parse reply code */
			return ATGEN_ParseReply(s,
					GetLineString(msg.Buffer, &Priv->Lines, 2),
					"^SQWE: @i",
					&Priv->SQWEMode);
		case AT_Reply_Connect:
			return ERR_NONE;
		case AT_Reply_Error:
			return ERR_NOTSUPPORTED;
		case AT_Reply_CMSError:
			return ATGEN_HandleCMSError(s);
		case AT_Reply_CMEError:
			return ATGEN_HandleCMEError(s);
		default:
			break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_ReplyGetUSSD(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_USSDMessage ussd;
	GSM_Error error;
	unsigned char *pos;
	int code;

	/*
	 * Reply format:
	 * +CUSD: 2,"...",15
	 */
	smprintf(s, "Incoming USSD received\n");

	if (s->Phone.Data.EnableIncomingUSSD) {
		/* Find start of reply */
		pos = strstr(msg.Buffer, "+CUSD:");
		if (pos == NULL) {
			if (s->Phone.Data.RequestID == ID_GetUSSD) {
				/*
				 * We usually get reply right after AT+CUSD=, but
				 * if this is not the case, we should wait.
				 */
				return ERR_NONE;
			}
			return ERR_UNKNOWNRESPONSE;
		}

		/* Parse reply code */
		error = ATGEN_ParseReply(s, pos,
				"+CUSD: @i @0",
				&code);

		if (error != ERR_NONE) return error;

		/* Decode status */
		smprintf(s, "Status: %d\n", code);
		switch(code) {
			case 0:
				ussd.Status = USSD_NoActionNeeded;
				break;
			case 1:
				ussd.Status = USSD_ActionNeeded;
				break;
			case 2:
				ussd.Status = USSD_Terminated;
				break;
			case 3:
				ussd.Status = USSD_AnotherClient;
				break;
			case 4:
				ussd.Status = USSD_NotSupported;
				break;
			case 5:
				ussd.Status = USSD_Timeout;
				break;
			default:
				ussd.Status = USSD_Unknown;
		}

		/* Try to parse text here, we ignore error code intentionally */
		ussd.Text[0] = 0;
		ussd.Text[1] = 0;
		ATGEN_ParseReply(s, pos,
				"+CUSD: @i, @s @0",
				&code,
				ussd.Text, sizeof(ussd.Text));

		/* Notify application */
		if (s->User.IncomingUSSD!=NULL) {
			s->User.IncomingUSSD(s, ussd, s->User.IncomingUSSDUserData);
		}
	}

	return ERR_NONE;
}

GSM_Error ATGEN_SetIncomingUSSD(GSM_StateMachine *s, gboolean enable)
{
	GSM_Error error;

	error = ATGEN_SetCharset(s, AT_PREF_CHARSET_NORMAL);
	if (error != ERR_NONE) return error;

	if (enable) {
		smprintf(s, "Enabling incoming USSD\n");
		ATGEN_WaitFor(s, "AT+CUSD=1\r", 10, 0x00, 3, ID_SetUSSD);
	} else {
		smprintf(s, "Terminating possible incoming USSD\n");
		ATGEN_WaitFor(s, "AT+CUSD=2\r", 10, 0x00, 3, ID_SetUSSD);
		smprintf(s, "Disabling incoming USSD\n");
		ATGEN_WaitFor(s, "AT+CUSD=0\r", 10, 0x00, 3, ID_SetUSSD);
	}
	if (error==ERR_NONE) s->Phone.Data.EnableIncomingUSSD = enable;
	if (error==ERR_UNKNOWN) return ERR_NOTSUPPORTED;
	return error;
}

GSM_Error ATGEN_ReplyGetModel(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Phone_Data		*Data = &s->Phone.Data;
	const char *pos, *pos2 = NULL;
	const char *line;

	if (s->Phone.Data.Priv.ATGEN.ReplyState != AT_Reply_OK) return ERR_NOTSUPPORTED;

	line = GetLineString(msg.Buffer, &Priv->Lines, 2);
	pos = line;

	/* Samsungs gives all information at once */
	if (strstr(line, "Manufacturer") != NULL) {
		line = GetLineString(msg.Buffer, &Priv->Lines, 3);
		if (strstr(line, "Model") == NULL) {
			line = GetLineString(msg.Buffer, &Priv->Lines, 2);
		}
		pos = line;
	}

	/*
	 * Motorola returns something like:
	 * "+CGMM: "GSM900","GSM1800","GSM1900","GSM850","MODEL=V3""
	 */
	if ((pos2 = strstr(line, "\"MODEL=")) != NULL) {
		pos = pos2 + 7; /* Skip above string */
		pos2 = strchr(pos, '"'); /* Find end quote */
	/* Sometimes phone adds this before manufacturer (Motorola) */
	} else if (strncmp("+CGMM: \"", line, 8) == 0) {
		pos += 8; /* Skip above string */
		pos2 = strchr(pos, '"'); /* Find end quote */
	/* Sometimes phone adds this before manufacturer (Sagem) */
	} else if (strncmp("+CGMM: ", line, 7) == 0) {
		pos += 7; /* Skip above string */
	}
	/* Samsung */
	if (strncmp("Model: ", pos, 7) == 0) {
		pos += 7; /* Skip above string */
	}
	/* Samsung */
	if (strncmp("I: ", pos, 3) == 0) {
		pos += 3; /* Skip above string */
	}

	/* Skip white spaces */
	while (iswspace((int)*pos)) {
		pos++;
	}
	if (pos2 == NULL) {
		pos2 = pos + strlen(pos);
	}
	/* Go before last char */
	pos2--;
	while(isspace((int)*pos2) && pos2 > pos) {
		pos2--;
	}

	/* Now store string if it fits */
	if (1 + pos2 - pos > GSM_MAX_MODEL_LENGTH) {
		smprintf(s, "WARNING: Model name too long, increase GSM_MAX_MODEL_LENGTH to at least %ld (currently %d)\n",
				(long int)(1 + pos2 - pos),
				GSM_MAX_MODEL_LENGTH);
	}

	strncpy(Data->Model, pos, MIN(1 + pos2 - pos, GSM_MAX_MODEL_LENGTH));
	Data->Model[1 + pos2 - pos] = 0;

	Data->ModelInfo = GetModelData(s, NULL, Data->Model, NULL);

	if (Data->ModelInfo->number[0] == 0)
		Data->ModelInfo = GetModelData(s, NULL, NULL, Data->Model);

	if (Data->ModelInfo->number[0] == 0)
		Data->ModelInfo = GetModelData(s, Data->Model, NULL, NULL);

	if (Data->ModelInfo->number[0] == 0) {
		smprintf(s, "Unknown model, but it should still work\n");
	}

	smprintf(s, "[Model name: `%s']\n", Data->Model);
	smprintf(s, "[Model data: `%s']\n", Data->ModelInfo->number);
	smprintf(s, "[Model data: `%s']\n", Data->ModelInfo->model);

	return ERR_NONE;
}

GSM_Error ATGEN_GetModel(GSM_StateMachine *s)
{
	GSM_Error error;

	if (s->Phone.Data.Model[0] != 0) return ERR_NONE;

	smprintf(s, "Getting model\n");
	ATGEN_WaitFor(s, "AT+CGMM\r", 8, 0x00, 3, ID_GetModel);
	if (error != ERR_NONE) {
		ATGEN_WaitFor(s, "ATI4\r", 5, 0x00, 3, ID_GetModel);
	}
	if (error == ERR_NONE) {
		smprintf_level(s, D_TEXT, "[Connected model  - \"%s\"]\n",
				s->Phone.Data.Model);
	}

	return error;
}

GSM_Error ATGEN_ReplyGetManufacturer(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Manufacturer info received\n");
		Priv->Manufacturer = AT_Unknown;
		if (GetLineLength(msg.Buffer, &Priv->Lines, 2) <= GSM_MAX_MANUFACTURER_LENGTH) {
			CopyLineString(s->Phone.Data.Manufacturer, msg.Buffer, &Priv->Lines, 2);
		} else {
			smprintf(s, "WARNING: Manufacturer name too long, increase GSM_MAX_MANUFACTURER_LENGTH to at least %d\n", GetLineLength(msg.Buffer, &Priv->Lines, 2));
			s->Phone.Data.Manufacturer[0] = 0;
		}
		/* Sometimes phone adds this before manufacturer (Sagem) */
		if (strncmp("+CGMI: ", s->Phone.Data.Manufacturer, 7) == 0) {
			memmove(s->Phone.Data.Manufacturer, s->Phone.Data.Manufacturer + 7, strlen(s->Phone.Data.Manufacturer + 7) + 1);
		}
		/* Samsung */
		if (strncmp("Manufacturer: ", s->Phone.Data.Manufacturer, 14) == 0) {
			memmove(s->Phone.Data.Manufacturer, s->Phone.Data.Manufacturer + 14, strlen(s->Phone.Data.Manufacturer + 14) + 1);
		}
		if (strncmp("I: ", s->Phone.Data.Manufacturer, 3) == 0) {
			memmove(s->Phone.Data.Manufacturer, s->Phone.Data.Manufacturer + 3, strlen(s->Phone.Data.Manufacturer + 3) + 1);
		}

		if (strstr(msg.Buffer,"Falcom")) {
			strcpy(s->Phone.Data.Manufacturer,"Falcom");
			Priv->Manufacturer = AT_Falcom;
			if (strstr(msg.Buffer,"A2D")) {
				strcpy(s->Phone.Data.Model,"A2D");
				s->Phone.Data.ModelInfo = GetModelData(s, NULL, s->Phone.Data.Model, NULL);
				smprintf(s, "Model A2D\n");
			}
		}
		if (strstr(msg.Buffer,"Nokia")) {
			strcpy(s->Phone.Data.Manufacturer,"Nokia");
			smprintf(s, "HINT: Consider using Nokia specific protocol instead of generic AT.\n");
			Priv->Manufacturer = AT_Nokia;
		}
		if (strstr(msg.Buffer,"SIEMENS")) {
			strcpy(s->Phone.Data.Manufacturer,"Siemens");
			Priv->Manufacturer = AT_Siemens;
		}
		if (strstr(msg.Buffer,"SHARP")) {
			strcpy(s->Phone.Data.Manufacturer,"Sharp");
			Priv->Manufacturer = AT_Sharp;
		}
		if (strstr(msg.Buffer,"ERICSSON")) {
			strcpy(s->Phone.Data.Manufacturer,"Ericsson");
			Priv->Manufacturer = AT_Ericsson;
		}
		if (strstr(msg.Buffer,"Sony Ericsson")) {
			strcpy(s->Phone.Data.Manufacturer,"Sony Ericsson");
			Priv->Manufacturer = AT_Ericsson;
		}
		if (strstr(msg.Buffer,"iPAQ")) {
			strcpy(s->Phone.Data.Manufacturer,"HP");
			Priv->Manufacturer = AT_HP;
		}
		if (strstr(msg.Buffer,"ALCATEL")) {
			strcpy(s->Phone.Data.Manufacturer,"Alcatel");
			Priv->Manufacturer = AT_Alcatel;
		}
		if (strstr(msg.Buffer,"SAGEM")) {
			strcpy(s->Phone.Data.Manufacturer,"Sagem");
			Priv->Manufacturer = AT_Sagem;
		}
		if (strstr(msg.Buffer,"Samsung")) {
			strcpy(s->Phone.Data.Manufacturer,"Samsung");
			Priv->Manufacturer = AT_Samsung;
		}
		if (strstr(msg.Buffer,"SAMSUNG")) {
			strcpy(s->Phone.Data.Manufacturer,"Samsung");
			Priv->Manufacturer = AT_Samsung;
		}
		if (strstr(msg.Buffer,"Mitsubishi")) {
			strcpy(s->Phone.Data.Manufacturer,"Mitsubishi");
			Priv->Manufacturer = AT_Mitsubishi;
		}
		if (strstr(msg.Buffer,"Motorola")) {
			strcpy(s->Phone.Data.Manufacturer,"Motorola");
			Priv->Manufacturer = AT_Motorola;
		}
		if (strstr(msg.Buffer,"Option")) {
			strcpy(s->Phone.Data.Manufacturer,"Option");
			Priv->Manufacturer = AT_Option;
		}
		if (strstr(msg.Buffer, "WAVECOM")) {
			strcpy(s->Phone.Data.Manufacturer, "Wavecom");
			Priv->Manufacturer = AT_Wavecom;
		}
		/*
		 * IAXmodem can not currently reasonably work with Gammu,
		 * but we can try to fixup at least something.
		 */
		if (strstr(msg.Buffer, "www.soft-switch.org")) {
			/* It replies OK to anything, but this just clutters output */
			Priv->Mode = FALSE;
		}
		smprintf(s, "[Manufacturer: %s]\n", s->Phone.Data.Manufacturer);
		return ERR_NONE;
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(s);
	case AT_Reply_Error:
		return ERR_NOTSUPPORTED;
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetManufacturer(GSM_StateMachine *s)
{
	GSM_Error error;
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;

	if (Priv->Manufacturer != 0 && s->Phone.Data.Manufacturer[0] != 0) return ERR_NONE;

	ATGEN_WaitFor(s, "AT+CGMI\r", 8, 0x00, 4, ID_GetManufacturer);
	if (error != ERR_NONE) {
		ATGEN_WaitFor(s, "ATI3\r", 5, 0x00, 4, ID_GetManufacturer);
	}
	return error;
}

GSM_Error ATGEN_ReplyGetFirmware(GSM_Protocol_Message msg, GSM_StateMachine *s)
{

	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	int line;

	strcpy(s->Phone.Data.Version, "Unknown");
	if (s->Phone.Data.Priv.ATGEN.ReplyState != AT_Reply_OK) return ERR_NOTSUPPORTED;

	s->Phone.Data.VerNum = 0;
	if (Priv->ReplyState == AT_Reply_OK) {
		line = 2;
		if (strstr(GetLineString(msg.Buffer, &Priv->Lines, line), "Manufacturer:") != NULL) {
			line ++;
		}
		if (strstr(GetLineString(msg.Buffer, &Priv->Lines, line), "Model:") != NULL) {
			line ++;
		}
		if (GetLineLength(msg.Buffer, &Priv->Lines, line) > GSM_MAX_VERSION_LENGTH - 1) {
			smprintf(s, "Please increase GSM_MAX_VERSION_LENGTH!\n");
			return ERR_MOREMEMORY;
		}
		CopyLineString(s->Phone.Data.Version, msg.Buffer, &Priv->Lines, line);
		/* Sometimes phone adds this before version (Sagem) */
		if (strncmp("+CGMR: ", s->Phone.Data.Version, 7) == 0) {
			/* Need to use memmove as strcpy does not correctly handle overlapping regions */
			memmove(s->Phone.Data.Version, s->Phone.Data.Version + 7, strlen(s->Phone.Data.Version + 7) + 1);
		}
		/* Sometimes phone adds this before version (Shrap) */
		if (strncmp("Revision: ", s->Phone.Data.Version, 10) == 0) {
			/* Need to use memmove as strcpy does not correctly handle overlapping regions */
			memmove(s->Phone.Data.Version, s->Phone.Data.Version + 10, strlen(s->Phone.Data.Version + 10) + 1);
		}
		/* Samsung */
		if (strncmp("I: ", s->Phone.Data.Version, 3) == 0) {
			/* Need to use memmove as strcpy does not correctly handle overlapping regions */
			memmove(s->Phone.Data.Version, s->Phone.Data.Version + 3, strlen(s->Phone.Data.Version + 3) + 1);
		}
		/* Add second line if it also contains version information */
		if (strcmp(GetLineString(msg.Buffer, &Priv->Lines, 3), "OK") != 0) {
			if (GetLineLength(msg.Buffer, &Priv->Lines, 3) + 1 + strlen(s->Phone.Data.Version) < GSM_MAX_VERSION_LENGTH - 1) {
				strcat(s->Phone.Data.Version, ",");
				CopyLineString(s->Phone.Data.Version + strlen(s->Phone.Data.Version), msg.Buffer, &Priv->Lines, 3);
			}
		}
	}
	smprintf(s, "Received firmware version: \"%s\"\n",s->Phone.Data.Version);
	GSM_CreateFirmwareNumber(s);
	return ERR_NONE;
}

GSM_Error ATGEN_GetFirmware(GSM_StateMachine *s)
{
	GSM_Error error;

	if (s->Phone.Data.Version[0] != 0) return ERR_NONE;

	error=ATGEN_GetManufacturer(s);
	if (error != ERR_NONE) return error;

	smprintf(s, "Getting firmware versions\n");
	ATGEN_WaitFor(s, "AT+CGMR\r", 16, 0x00, 3, ID_GetFirmware);

	if (error != ERR_NONE) {
		ATGEN_WaitFor(s, "ATI5\r", 5, 0x00, 3, ID_GetFirmware);
	}

	if (error == ERR_NONE) {
		smprintf_level(s, D_TEXT, "[Firmware version - \"%s\"]\n",
				s->Phone.Data.Version);
	}
	return error;
}

GSM_Error ATGEN_Initialise(GSM_StateMachine *s)
{
	GSM_Phone_ATGENData     *Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error               error;
    	char                    buff[2];

	InitLines(&Priv->Lines);

	Priv->SMSMode			= 0;
	Priv->SQWEMode			= -1;
	Priv->SMSTextDetails		= FALSE;
	Priv->Manufacturer		= 0;
	Priv->MotorolaSMS		= FALSE;
	Priv->PhoneSMSMemory		= 0;
	Priv->PhoneSaveSMS		= 0;
	Priv->SIMSaveSMS		= 0;
	Priv->SIMSMSMemory		= 0;
	Priv->SMSMemory			= 0;
	Priv->SMSMemoryWrite		= FALSE;
	Priv->PBKMemory			= 0;
	Priv->PBKSBNR			= 0;
	Priv->PBK_SPBR			= 0;
	Priv->PBK_MPBR			= 0;
	Priv->Charset			= 0;
	Priv->EncodedCommands		= FALSE;
	Priv->NormalCharset		= 0;
	Priv->IRACharset		= 0;
	Priv->UnicodeCharset		= 0;
	Priv->PBKMemories[0]		= 0;
	Priv->FirstCalendarPos		= 0;
	Priv->FirstFreeCalendarPos	= 0;
	Priv->NextMemoryEntry		= 0;
	Priv->FirstMemoryEntry		= -1;
	Priv->file.Used 		= 0;
	Priv->file.Buffer 		= NULL;
	Priv->Mode			= FALSE;
	Priv->MemorySize		= 0;
	Priv->MemoryUsed		= 0;
	Priv->TextLength		= 0;
	Priv->NumberLength		= 0;

	Priv->CNMIMode			= -1;
	Priv->CNMIProcedure		= -1;
	Priv->CNMIDeliverProcedure	= -1;
#ifdef GSM_ENABLE_CELLBROADCAST
	Priv->CNMIBroadcastProcedure	= -1;
#endif

	Priv->ErrorText			= NULL;

	Priv->SMSCount			= 0;
	Priv->SMSCache			= NULL;

	if (s->ConnectionType != GCT_IRDAAT && s->ConnectionType != GCT_BLUEAT) {
		/* We try to escape AT+CMGS mode, at least Siemens M20
		 * then needs to get some rest
		 */
		smprintf(s, "Escaping SMS mode\n");
		error = s->Protocol.Functions->WriteMessage(s, "\x1B\r", 2, 0x00);
		if (error!=ERR_NONE) return error;

	    	/* Grab any possible garbage */
	    	while (s->Device.Functions->ReadDevice(s, buff, 2) > 0) usleep(10000);
	}

    	/* When some phones (Alcatel BE5) is first time connected, it needs extra
     	 * time to react, sending just AT wakes up the phone and it then can react
     	 * to ATE1. We don't need to check whether this fails as it is just to
     	 * wake up the phone and does nothing.
     	 */
    	smprintf(s, "Sending simple AT command to wake up some devices\n");
	error = GSM_WaitFor(s, "AT\r", 3, 0x00, 2, ID_IncomingFrame);

	/* We want to see our commands to allow easy detection of reply functions */
	smprintf(s, "Enabling echo\n");
	error = GSM_WaitFor(s, "ATE1\r", 5, 0x00, 3, ID_EnableEcho);

	/* Some modems (Sony Ericsson GC 79, GC 85) need to enable functionality
	 * (with reset), otherwise they return ERROR on anything!
	 */
	if (error == ERR_UNKNOWN) {
		error = GSM_WaitFor(s, "AT+CFUN=1,1\r", 12, 0x00, 3, ID_Reset);
		if (error != ERR_NONE) return error;
		error = GSM_WaitFor(s, "ATE1\r", 5, 0x00, 3, ID_EnableEcho);
	}
	if (error != ERR_NONE) {
		smprintf(s, "Phone does not support enabled echo, it can not work with Gammu!\n");
		return error;
	}

	/* Try whether phone supports mode switching as Motorola phones. */
	smprintf(s, "Trying Motorola mode switch\n");
	error = GSM_WaitFor(s, "AT+MODE=2\r", 10, 0x00, 3, ID_ModeSwitch);
	if (error != ERR_NONE) {
		smprintf(s, "Seems not to be supported\n");
		Priv->Mode = FALSE;
	} else {
		smprintf(s, "Works, will use it\n");
		Priv->Mode = TRUE;
		Priv->CurrentMode = 2;
	}

	smprintf(s, "Enabling CME errors\n");
	/* Try numeric errors */
	ATGEN_WaitFor(s, "AT+CMEE=1\r", 10, 0x00, 3, ID_EnableErrorInfo);
	if (error != ERR_NONE) {
		/* Try textual errors */
		ATGEN_WaitFor(s, "AT+CMEE=2\r", 10, 0x00, 3, ID_EnableErrorInfo);
		if (error != ERR_NONE) {
			smprintf(s, "CME errors could not be enabled, some error types won't be detected.\n");
		}
	}

	/* Switch to GSM charset */
	error = ATGEN_SetCharset(s, AT_PREF_CHARSET_NORMAL);
	if (error != ERR_NONE && error != ERR_SECURITYERROR) return error;

	/* Get model, it is useful to know it now */
	error = ATGEN_GetModel(s);
	if (error != ERR_NONE && error != ERR_SECURITYERROR) return error;

	/* Get manufacturer, needed for some detection */
	error = ATGEN_GetManufacturer(s);
	if (error != ERR_NONE && error != ERR_SECURITYERROR) return error;

	/* Mode switching cabaple phones can switch using AT+MODE */
	if (!Priv->Mode) {
		smprintf(s, "Checking for OBEX support\n");
		/* We don't care about error here */
		ATGEN_WaitFor(s, "AT+CPROT=?\r", 11, 0x00, 3, ID_SetOBEX);
		error = ERR_NONE;
	} else {
		/*
		 * Enable OBEX for Motorolas, they usually support this and
		 * AT+OBEX can fallback to pure AT.
		 *
		 * This usually does not work on Bluetooth and IrDA, as there
		 * you can access OBEX another way.
		 */
		if (s->ConnectionType != GCT_IRDAAT &&
				s->ConnectionType != GCT_BLUEAT &&
				!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NO_ATOBEX) &&
				!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_OBEX)
				) {
			smprintf(s, "Automatically enabling F_OBEX, please report bug if it causes problems\n");
			GSM_AddPhoneFeature(s->Phone.Data.ModelInfo, F_OBEX);
			GSM_AddPhoneFeature(s->Phone.Data.ModelInfo, F_MODE22);
			GSM_AddPhoneFeature(s->Phone.Data.ModelInfo, F_IRMC_LEVEL_2);
		}
	}

#ifdef GSM_ENABLE_ATOBEX
	if (Priv->Manufacturer == AT_Siemens) {
		ATGEN_WaitFor(s, "AT^SQWE?\r", 9, 0x00, 3, ID_GetProtocol);
		if (error == ERR_NONE) {
			smprintf(s, "Phone seems to support Siemens like mode switching, adding OBEX feature.\n");
			GSM_AddPhoneFeature(s->Phone.Data.ModelInfo, F_OBEX);
			GSM_AddPhoneFeature(s->Phone.Data.ModelInfo, F_SQWE);
			/* Switch to mode 0 if we're in different mode */
			if (Priv->SQWEMode != 0) {
				ATGEN_WaitFor(s, "AT^SQWE=0\r", 10, 0x00, 3, ID_SetOBEX);
				if (error != ERR_NONE) return error;
				Priv->SQWEMode = 0;
			}
		}
		/* Clear error flag */
		error = ERR_NONE;
	}
#endif

	if (!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SLOWWRITE)) {
		s->Protocol.Data.AT.FastWrite = TRUE;
	}

	return error;
}

GSM_Error ATGEN_ReplyGetCharset(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	/*
	 * Reply we get here:
	 * AT+CSCS?
	 * +CSCS: "GSM"
	 * OK
	 *
	 * Or
	 *
	 * AT+CSCS?
	 * +CSCS:0
	 * OK
	 */
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	const char		*line;
	int			i = 0;

	switch (Priv->ReplyState) {
		case AT_Reply_OK:
			/* Can not use ATGEN_ParseReply safely here as we do not know charset yet */
			line = GetLineString(msg.Buffer, &Priv->Lines, 2);
			if (strcmp(line, "+CSCS:0") == 0) {
				smprintf(s, "WARNING: Charsets support broken! Assuming GSM as default!\n");
				Priv->Charset = AT_CHARSET_GSM;
			}
			/* First current charset: */
			while (AT_Charsets[i].charset != 0) {
				if (strstr(line, AT_Charsets[i].text) != NULL) {
					Priv->Charset = AT_Charsets[i].charset;
					break;
				}
				/* We detect encoded UCS2 reply here so that we can handle encoding of values later. */
				if (strstr(line, "0055004300530032") != NULL) {
					Priv->Charset = AT_CHARSET_UCS2;
					Priv->EncodedCommands = TRUE;
					break;
				}
				i++;
			}
			if (Priv->Charset == 0) {
				smprintf(s, "Could not determine charset returned by phone, probably not supported!\n");
				return ERR_NOTSUPPORTED;
			}
			return ERR_NONE;
		case AT_Reply_Error:
			return ERR_NOTSUPPORTED;
		case AT_Reply_CMSError:
			return ATGEN_HandleCMSError(s);
		case AT_Reply_CMEError:
			return ATGEN_HandleCMEError(s);
		default:
			return ERR_UNKNOWNRESPONSE;
	}
}

GSM_Error ATGEN_ReplyGetCharsets(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	/* Reply we get here:
		AT+CSCS=?
		+CSCS: ("GSM","UCS2")

		OK
	 */
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	const char	*line;
	int			i = 0;
	gboolean			IgnoredUTF8 = FALSE;

	switch (Priv->ReplyState) {
		case AT_Reply_OK:
			line = GetLineString(msg.Buffer, &Priv->Lines, 2);
			if (strcmp(line, "+CSCS:") == 0) {
				smprintf(s, "WARNING: Charsets support broken! Assuming that only GSM is supported!\n");
				Priv->NormalCharset = AT_CHARSET_GSM;
				Priv->IRACharset = AT_CHARSET_GSM;
				Priv->UnicodeCharset = AT_CHARSET_GSM;
				return ERR_NONE;
			}
			/* First find good charset for non-unicode: */
			while (AT_Charsets[i].charset != 0) {
				if (strstr(line, AT_Charsets[i].text) != NULL) {
					Priv->NormalCharset = AT_Charsets[i].charset;
					Priv->IRACharset = AT_Charsets[i].charset;
					smprintf(s, "Chosen %s as normal charset\n", AT_Charsets[i].text);
					break;
				}
				i++;
			}
			/* Check if we have proper normal charset */
			if (Priv->NormalCharset == 0) {
				smprintf(s, "Could not find supported charset in list returned by phone!\n");
				return ERR_UNKNOWNRESPONSE;
			}
			/* Then find good charset for unicode and IRA */
			Priv->UnicodeCharset = 0;
			while (AT_Charsets[i].charset != 0) {
				if (AT_Charsets[i].unicode && (strstr(line, AT_Charsets[i].text) != NULL)) {
					if ((AT_Charsets[i].charset == AT_CHARSET_UTF8 ||
						AT_Charsets[i].charset == AT_CHARSET_UTF_8) &&
							Priv->Manufacturer == AT_Motorola) {
						IgnoredUTF8 = TRUE;
						smprintf(s, "Skipped %s because it is usually wrongly implemented on Motorola phones\n", AT_Charsets[i].text);
					} else if ((AT_Charsets[i].charset != AT_CHARSET_UCS2 &&
							AT_Charsets[i].charset != AT_CHARSET_UCS_2) ||
							!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NO_UCS2)) {
						Priv->UnicodeCharset = AT_Charsets[i].charset;
						smprintf(s, "Chosen %s as unicode charset\n", AT_Charsets[i].text);
						break;
					}
				}
				if (AT_Charsets[i].ira && (strstr(line, AT_Charsets[i].text) != NULL)) {
					Priv->IRACharset = AT_Charsets[i].charset;
				}
				i++;
			}
			/* Fallback for unicode charset */
			if (Priv->UnicodeCharset == 0) {
				if (IgnoredUTF8) {
					Priv->UnicodeCharset = AT_CHARSET_UTF8;
					smprintf(s, "Switched back to UTF8 charset, expect problems\n");
				} else {
					Priv->UnicodeCharset = Priv->NormalCharset;
				}
			}
			/* If we have unicode charset, it's better than GSM for IRA */
			if (Priv->IRACharset == AT_CHARSET_GSM) {
				Priv->IRACharset = Priv->UnicodeCharset;
			}
			return ERR_NONE;
		case AT_Reply_Error:
			/* Phone does not support charsets, everything should
			 * be in GSM. */
			smprintf(s, "INFO: assuming GSM charset\n");
			Priv->IRACharset = AT_CHARSET_GSM;
			Priv->UnicodeCharset = AT_CHARSET_GSM;
			Priv->NormalCharset = AT_CHARSET_GSM;
			Priv->Charset = AT_CHARSET_GSM;
			return ERR_NONE;
		case AT_Reply_CMSError:
			return ATGEN_HandleCMSError(s);
		case AT_Reply_CMEError:
			return ATGEN_HandleCMEError(s);
		default:
			return ERR_UNKNOWNRESPONSE;
	}
}


GSM_Error ATGEN_SetCharset(GSM_StateMachine *s, GSM_AT_Charset_Preference Prefer)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error		error;
	char			buffer[100];
	char			buffer2[100];
	char			buffer3[100];
	int			i = 0;
	GSM_AT_Charset		cset;

	/* Do we know current charset? */
	if (Priv->Charset == 0) {
		/* Get current charset */
		ATGEN_WaitFor(s, "AT+CSCS?\r", 9, 0x00, 3, ID_GetMemoryCharset);
		/* ERR_NOTSUPPORTED means that we do not know charset phone returned */
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) return error;
	}

	/* Do we know available charsets? */
	if (Priv->NormalCharset == 0) {
		/* Switch to GSM to be safe (UCS2 can give us encoded result) */
		if (Priv->Charset == AT_CHARSET_UCS2 && Priv->EncodedCommands) {
			ATGEN_WaitFor(s, "AT+CSCS=\"00470053004D\"\r", 23, 0x00, 3, ID_SetMemoryCharset);
			if (error == ERR_NONE) {
				Priv->Charset = AT_CHARSET_GSM;
			}
		}
		/* We need to know manufacturer */
		error = ATGEN_GetManufacturer(s);
		if (error != ERR_NONE) return error;

		/* Get available charsets */
		ATGEN_WaitFor(s, "AT+CSCS=?\r", 10, 0x00, 3, ID_GetMemoryCharset);
		if (error != ERR_NONE) return error;
	}

	/* Find charset we want */
	if (Prefer == AT_PREF_CHARSET_UNICODE) {
		cset = Priv->UnicodeCharset;
	} else if (Prefer == AT_PREF_CHARSET_NORMAL) {
		cset = Priv->NormalCharset;
	} else if (Prefer == AT_PREF_CHARSET_IRA) {
		if (Priv->IRACharset == Priv->UnicodeCharset &&
				GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_CKPD_NO_UNICODE)) {
			cset = Priv->NormalCharset;
		} else {
			cset = Priv->IRACharset;
		}
	} else if (Prefer == AT_PREF_CHARSET_RESET) {
		cset = Priv->Charset;
		Priv->Charset = 0;
	} else {
		return ERR_BUG;
	}

	/* If we already have set our prefered charset there is nothing to do*/
	if (Priv->Charset == cset) return ERR_NONE;

	/* Find text representation */
	while (AT_Charsets[i].charset != 0) {
		if (AT_Charsets[i].charset == cset) {
			break;
		}
		i++;
	}

	/* Should not happen! */
	if (AT_Charsets[i].charset == 0) {
		smprintf(s, "Could not find string representation for charset (%d)!\n",
				cset);
		return ERR_BUG;
	}

	/* And finally set the charset */
	if (Priv->EncodedCommands && Priv->Charset == AT_CHARSET_UCS2) {
		EncodeUnicode(buffer2, AT_Charsets[i].text, strlen(AT_Charsets[i].text));
		EncodeHexUnicode(buffer3, buffer2, strlen(AT_Charsets[i].text));
		sprintf(buffer, "AT+CSCS=\"%s\"\r", buffer3);
	} else {
		sprintf(buffer, "AT+CSCS=\"%s\"\r", AT_Charsets[i].text);
	}
	ATGEN_WaitFor(s, buffer, strlen(buffer), 0x00, 3, ID_SetMemoryCharset);
	if (error == ERR_NONE) Priv->Charset = cset;
	else return error;

	/* Verify we have charset we wanted (this is especially needed to detect whether phone encodes also control information and not only data) */
	ATGEN_WaitFor(s, "AT+CSCS?\r", 9, 0x00, 3, ID_GetMemoryCharset);

	return error;
}

GSM_Error ATGEN_ReplyGetIMEI(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	if (s->Phone.Data.Priv.ATGEN.ReplyState != AT_Reply_OK) return ERR_NOTSUPPORTED;

	if (GetLineLength(msg.Buffer, &s->Phone.Data.Priv.ATGEN.Lines, 2) > GSM_MAX_IMEI_LENGTH) {
		smprintf(s, "IMEI too long!\n");
		return ERR_MOREMEMORY;
	}

	CopyLineString(s->Phone.Data.IMEI, msg.Buffer, &s->Phone.Data.Priv.ATGEN.Lines, 2);
	/* Remove various prefies some phones add */
	if (strncmp(s->Phone.Data.IMEI, "+CGSN: IMEI", 11) == 0) { /* Motorola */
		memmove(s->Phone.Data.IMEI, s->Phone.Data.IMEI + 11, strlen(s->Phone.Data.IMEI + 11) + 1);
	} else if (strncmp(s->Phone.Data.IMEI, "+CGSN: ", 7) == 0) {
		memmove(s->Phone.Data.IMEI, s->Phone.Data.IMEI + 7, strlen(s->Phone.Data.IMEI + 7) + 1);
	}
	smprintf(s, "Received IMEI %s\n",s->Phone.Data.IMEI);
	return ERR_NONE;
}

GSM_Error ATGEN_GetIMEI (GSM_StateMachine *s)
{
	GSM_Error error;

	if (s->Phone.Data.IMEI[0] != 0) return ERR_NONE;
	smprintf(s, "Getting IMEI\n");
	ATGEN_WaitFor(s, "AT+CGSN\r", 8, 0x00, 2, ID_GetIMEI);

	return error;
}

GSM_Error ATGEN_ReplyGetDateTime(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		return ATGEN_ParseReply(s,
				GetLineString(msg.Buffer, &Priv->Lines, 2),
				"+CCLK: @d",
				Data->DateTime);
	case AT_Reply_Error:
		return ERR_NOTSUPPORTED;
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(s);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}


GSM_Error ATGEN_ReplyGetAlarm(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data		*Data = &s->Phone.Data;
	unsigned char		buffer[100];
	GSM_Error		error;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	int i;
	int location;
	const char *str;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		/* Try simple date string as alarm */
		error = ATGEN_ParseReply(s,
				GetLineString(msg.Buffer, &Priv->Lines, 2),
				"+CALA: @d",
				&(Data->Alarm->DateTime));
		if (error == ERR_NONE) {
			if (Data->Alarm->Location != 1) return ERR_INVALIDLOCATION;
			return ERR_NONE;
		}

		/* Ok we have something more complex, try to handle it */
		i = 2;
		/* Need to scan over all reply lines */
		while (strcmp("OK", str = GetLineString(msg.Buffer, &Priv->Lines, i)) != 0) {
			i++;
			/**
			 * +CALA: [<time1>,<n1>,<type1>,[<text1>],[<recurr1>],<silent1>]
			 */
			error = ATGEN_ParseReply(s, str,
					"+CALA: @d, @i, @s, @s, @s",
					&(Data->Alarm->DateTime),
					&location,
					buffer, sizeof(buffer),
					Data->Alarm->Text, sizeof(Data->Alarm->Text),
					buffer, sizeof(buffer));
			if (error == ERR_NONE && location == Data->Alarm->Location) {
				/**
				 * \todo This is not exact, repeating
				 * can be set for only limited
				 * set of days (eg. "4,5,6").
				 */
				if (!strcmp(buffer, "\"1,2,3,4,5,6,7\"")) {
					Data->Alarm->Repeating = TRUE;
				} else {
					Data->Alarm->Repeating = FALSE;
				}
				return ERR_NONE;
			}
		}

		return ERR_EMPTY;
	case AT_Reply_Error:
		return ERR_NOTSUPPORTED;
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(s);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	GSM_Error		error;
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;

	/* If phone encodes also values in command, we need normal charset */
	if (Priv->EncodedCommands) {
		error = ATGEN_SetCharset(s, AT_PREF_CHARSET_NORMAL);
		if (error != ERR_NONE) return error;
	}

	s->Phone.Data.DateTime=date_time;
	smprintf(s, "Getting date & time\n");
	ATGEN_WaitFor(s, "AT+CCLK?\r", 9, 0x00, 4, ID_GetDateTime);

	return error;
}

GSM_Error ATGEN_PrivSetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time, gboolean set_timezone)
{
	char			tz[4] = "";
	char			req[128];
	GSM_Error		error;

	if (set_timezone) {
		sprintf(tz, "%+03i", date_time->Timezone / 3600);
	}

	sprintf(req, "AT+CCLK=\"%02i/%02i/%02i,%02i:%02i:%02i%s\"\r",
		     (date_time->Year > 2000 ? date_time->Year-2000 : date_time->Year-1900),
		     date_time->Month ,
		     date_time->Day,
		     date_time->Hour,
		     date_time->Minute,
		     date_time->Second,
		     tz);

	smprintf(s, "Setting date & time\n");

	ATGEN_WaitFor(s, req, strlen(req), 0x00, 4, ID_SetDateTime);
	if (error == ERR_UNKNOWN) error = ERR_NOTSUPPORTED;

	if (set_timezone && (
		(error == ERR_INVALIDDATA
		&& s->Phone.Data.Priv.ATGEN.ReplyState == AT_Reply_CMEError
		&& s->Phone.Data.Priv.ATGEN.ErrorCode == 24) ||
		(error == ERR_INVALIDLOCATION
		&& s->Phone.Data.Priv.ATGEN.ReplyState == AT_Reply_CMEError
		&& s->Phone.Data.Priv.ATGEN.ErrorCode == 21)
		)) {
		/*
		 * Some firmwares of Ericsson R320s don't like the timezone part,
		 * even though it is in its command reference. Similar issue
		 * exists for MC75
		 */
		smprintf(s, "Retrying without timezone suffix\n");
		error = ATGEN_PrivSetDateTime(s, date_time, FALSE);
	}
	return error;
}

GSM_Error ATGEN_SetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error		error;

	/* If phone encodes also values in command, we need normal charset */
	if (Priv->EncodedCommands) {
		error = ATGEN_SetCharset(s, AT_PREF_CHARSET_NORMAL);
		if (error != ERR_NONE) return error;
	}
	return ATGEN_PrivSetDateTime(s, date_time, TRUE);
}

GSM_Error ATGEN_GetAlarm(GSM_StateMachine *s, GSM_Alarm *Alarm)
{
	GSM_Error		error;
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;

	/* If phone encodes also values in command, we need normal charset */
	if (Priv->EncodedCommands) {
		error = ATGEN_SetCharset(s, AT_PREF_CHARSET_NORMAL);
		if (error != ERR_NONE) return error;
	}

	s->Phone.Data.Alarm = Alarm;
	smprintf(s, "Getting alarm\n");
	ATGEN_WaitFor(s, "AT+CALA?\r", 9, 0x00, 4, ID_GetAlarm);

	return error;
}

/* R320 only takes HH:MM. Do other phones understand full date? */
GSM_Error ATGEN_SetAlarm(GSM_StateMachine *s, GSM_Alarm *Alarm)
{
	char			req[20];
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error		error;

	if (Alarm->Location != 1) return ERR_INVALIDLOCATION;

	/* If phone encodes also values in command, we need normal charset */
	if (Priv->EncodedCommands) {
		error = ATGEN_SetCharset(s, AT_PREF_CHARSET_NORMAL);
		if (error != ERR_NONE) return error;
	}

	sprintf(req, "AT+CALA=\"%02i:%02i\"\r",Alarm->DateTime.Hour,Alarm->DateTime.Minute);

	smprintf(s, "Setting Alarm\n");
	ATGEN_WaitFor(s, req, strlen(req), 0x00, 3, ID_SetAlarm);

	return error;
}

GSM_Error ATGEN_ReplyGetNetworkLAC_CID(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_NetworkInfo		*NetworkInfo = s->Phone.Data.NetworkInfo;
	GSM_CutLines		Lines;
	int			i=0;
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	const char			*answer;
	char			*tmp;
	size_t pos;

  	if (s->Phone.Data.RequestID != ID_GetNetworkInfo) {
		smprintf(s, "Incoming LAC & CID info\n");
		return ERR_NONE;
	}

	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		break;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(s);
	default:
		return ERR_UNKNOWNRESPONSE;
	}

	InitLines(&Lines);
	SplitLines(GetLineString(msg.Buffer,&Priv->Lines,2),
		GetLineLength(msg.Buffer,&Priv->Lines,2),
		&Lines, ",", 1, TRUE);

	/* Find number of lines */
	while (Lines.numbers[i*2+1] != 0) {
		/* FIXME: handle special chars correctly */
		tmp = strdup(GetLineString(msg.Buffer,&Priv->Lines,2));
		smprintf(s, "%i \"%s\"\n",i+1,GetLineString(tmp,&Lines,i+1));
		free(tmp);
		i++;
	}

	smprintf(s, "Network LAC & CID & state received\n");
	tmp = strdup(GetLineString(msg.Buffer,&Priv->Lines,2));
	answer = GetLineString(tmp,&Lines,2);
	free(tmp);
	while (*answer == 0x20) answer++;
#ifdef DEBUG
	switch (answer[0]) {
		case '0': smprintf(s, "Not registered into any network. Not searching for network\n"); 	  break;
		case '1': smprintf(s, "Home network\n"); 						  break;
		case '2': smprintf(s, "Not registered into any network. Searching for network\n"); 	  break;
		case '3': smprintf(s, "Registration denied\n"); 					  break;
		case '4': smprintf(s, "Unknown\n"); 							  break;
		case '5': smprintf(s, "Registered in roaming network\n"); 				  break;
		default : smprintf(s, "Unknown\n");
	}
#endif
	switch (answer[0]) {
		case '0': NetworkInfo->State = GSM_NoNetwork;		break;
		case '1': NetworkInfo->State = GSM_HomeNetwork; 	break;
		case '2': NetworkInfo->State = GSM_RequestingNetwork; 	break;
		case '3': NetworkInfo->State = GSM_RegistrationDenied;	break;
		case '4': NetworkInfo->State = GSM_NetworkStatusUnknown;break;
		case '5': NetworkInfo->State = GSM_RoamingNetwork; 	break;
		default : NetworkInfo->State = GSM_NetworkStatusUnknown;break;
	}
	if (NetworkInfo->State == GSM_HomeNetwork ||
	    NetworkInfo->State == GSM_RoamingNetwork) {
		NetworkInfo->LAC[0] = 0;
		NetworkInfo->CID[0] = 0;

		if (Lines.numbers[3*2+1]==0) {
			FreeLines(&Lines);
			return ERR_NONE;
		}

 		tmp = strdup(GetLineString(msg.Buffer,&Priv->Lines,2));
 		answer = GetLineString(tmp,&Lines,3);
 		free(tmp);
		while (*answer == 0x20) answer++;
		if (*answer == '"') answer++;
		pos = 0;
		while (*answer != '"' && *answer != ',' && *answer != 0 && *answer != '\n') {
			NetworkInfo->LAC[pos++] = *answer;
			answer++;
			if (pos >= sizeof(NetworkInfo->LAC)) {
				smprintf(s, "LAC too big!\n");
				FreeLines(&Lines);
				return ERR_MOREMEMORY;
			}
		}
		NetworkInfo->LAC[pos++] = 0;

 		tmp = strdup(GetLineString(msg.Buffer,&Priv->Lines,2));
 		answer = GetLineString(tmp,&Lines,4);
 		free(tmp);
		while (*answer == 0x20) answer++;
		if (*answer == '"') answer++;
		pos = 0;
		while (*answer != '"' && *answer != ',' && *answer != 0 && *answer != '\n') {
			NetworkInfo->CID[pos++] = *answer;
			answer++;
			if (pos >= sizeof(NetworkInfo->CID)) {
				smprintf(s, "CID too big!\n");
				FreeLines(&Lines);
				return ERR_MOREMEMORY;
			}
		}
		NetworkInfo->CID[pos++] = 0;

		smprintf(s, "LAC   : %s\n",NetworkInfo->LAC);
		smprintf(s, "CID   : %s\n",NetworkInfo->CID);
	}
	FreeLines(&Lines);
	return ERR_NONE;
}

GSM_Error ATGEN_ReplyGetNetworkCode(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_NetworkInfo		*NetworkInfo = s->Phone.Data.NetworkInfo;
	int i;
	GSM_Error error;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Network code received\n");
		error = ATGEN_ParseReply(s,
				GetLineString(msg.Buffer, &Priv->Lines, 2),
				"+COPS: @i, @i, @r",
				&i, /* Mode, ignored for now */
				&i, /* Format of reply, we set this */
				NetworkInfo->NetworkCode, sizeof(NetworkInfo->NetworkCode));

		/* Some Sony-Ericsson phones use this */
		if (error == ERR_UNKNOWNRESPONSE) {
			error = ATGEN_ParseReply(s,
					GetLineString(msg.Buffer, &Priv->Lines, 2),
					"+COPS: @i, @i, @r, @i",
					&i, /* Mode, ignored for now */
					&i, /* Format of reply, we set this */
					NetworkInfo->NetworkCode, sizeof(NetworkInfo->NetworkCode),
					&i);
		}

		if (error != ERR_NONE) {
			/* Cleanup if something went wrong */
			NetworkInfo->NetworkCode[0] = 0;
			NetworkInfo->NetworkCode[1] = 0;

			return error;
		}

		/* Split network code for country and operator */
		if (strlen(NetworkInfo->NetworkCode) == 5) {
			NetworkInfo->NetworkCode[6] = 0;
			NetworkInfo->NetworkCode[5] = NetworkInfo->NetworkCode[4];
			NetworkInfo->NetworkCode[4] = NetworkInfo->NetworkCode[3];
			NetworkInfo->NetworkCode[3] = ' ';
		}

		smprintf(s, "   Network code              : %s\n",
				NetworkInfo->NetworkCode);
		smprintf(s, "   Network name for Gammu    : %s ",
				DecodeUnicodeString(GSM_GetNetworkName(NetworkInfo->NetworkCode)));
		smprintf(s, "(%s)\n",
				DecodeUnicodeString(GSM_GetCountryName(NetworkInfo->NetworkCode)));
		return ERR_NONE;
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(s);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_ReplyGetNetworkName(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_NetworkInfo		*NetworkInfo = s->Phone.Data.NetworkInfo;
	int i;
	GSM_Error error;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Network name received\n");
		error = ATGEN_ParseReply(s,
				GetLineString(msg.Buffer, &Priv->Lines, 2),
				"+COPS: @i, @i, @s",
				&i, /* Mode, ignored for now */
				&i, /* Format of reply, we set this */
				NetworkInfo->NetworkName, sizeof(NetworkInfo->NetworkName));

		/* Some Sony-Ericsson phones use this */
		if (error == ERR_UNKNOWNRESPONSE) {
			error = ATGEN_ParseReply(s,
					GetLineString(msg.Buffer, &Priv->Lines, 2),
					"+COPS: @i, @i, @s, @i",
					&i, /* Mode, ignored for now */
					&i, /* Format of reply, we set this */
					NetworkInfo->NetworkName, sizeof(NetworkInfo->NetworkName),
					&i);
		}

		/* Cleanup if something went wrong */
		if (error != ERR_NONE) {
			NetworkInfo->NetworkName[0] = 0;
			NetworkInfo->NetworkName[1] = 0;
		}

		return error;
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(s);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetNetworkInfo(GSM_StateMachine *s, GSM_NetworkInfo *netinfo)
{
	GSM_Error error;

	s->Phone.Data.NetworkInfo=netinfo;

	netinfo->NetworkName[0] = 0;
	netinfo->NetworkName[1] = 0;
	netinfo->NetworkCode[0] = 0;

	smprintf(s, "Enable full network info\n");
	ATGEN_WaitFor(s, "AT+CREG=2\r", 10, 0x00, 4, ID_GetNetworkInfo);
	if ((error != ERR_NONE) &&
	    (s->Phone.Data.Priv.ATGEN.Manufacturer!=AT_Siemens) &&
	    (s->Phone.Data.Priv.ATGEN.Manufacturer!=AT_Ericsson)) return error;

	smprintf(s, "Getting network LAC and CID and state\n");
	ATGEN_WaitFor(s, "AT+CREG?\r", 9, 0x00, 4, ID_GetNetworkInfo);
	if (error != ERR_NONE) return error;

	if (netinfo->State == GSM_HomeNetwork || netinfo->State == GSM_RoamingNetwork) {
		/* Set numeric format for AT+COPS? */
		smprintf(s, "Setting short network name format\n");
		ATGEN_WaitFor(s, "AT+COPS=3,2\r", 12, 0x00, 4, ID_GetNetworkInfo);

		/* Get operator code */
		smprintf(s, "Getting network code\n");
		ATGEN_WaitFor(s, "AT+COPS?\r", 9, 0x00, 4, ID_GetNetworkCode);

		/* Set string format for AT+COPS? */
		smprintf(s, "Setting long string network name format\n");
		ATGEN_WaitFor(s, "AT+COPS=3,0\r", 12, 0x00, 4, ID_GetNetworkInfo);

		/* Get operator code */
		smprintf(s, "Getting network code\n");
		ATGEN_WaitFor(s, "AT+COPS?\r", 9, 0x00, 4, ID_GetNetworkName);

		/* All information here is optional */
		error = ERR_NONE;
	}
	return error;
}

/**
 * Stores available phonebook memories in PBKMemories.
 *
 * @todo Should parse reply, not copy it as is.
 */
GSM_Error ATGEN_ReplyGetPBKMemories(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		break;
	case AT_Reply_Error:
		return ERR_NOTSUPPORTED;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(s);
	default:
		return ERR_UNKNOWNRESPONSE;
	}

	if (GetLineLength(msg.Buffer, &Priv->Lines, 2) >= AT_PBK_MAX_MEMORIES) {
		smprintf(s, "ERROR: Too long phonebook memories information received! (Recevided %d, AT_PBK_MAX_MEMORIES is %d\n",
			GetLineLength(msg.Buffer, &Priv->Lines, 2), AT_PBK_MAX_MEMORIES);
		return ERR_MOREMEMORY;
	}
	CopyLineString(Priv->PBKMemories, msg.Buffer, &Priv->Lines, 2);
	smprintf(s, "PBK memories received: %s\n", s->Phone.Data.Priv.ATGEN.PBKMemories);
	return ERR_NONE;
}

GSM_Error ATGEN_ReplySetPBKMemory(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
		case AT_Reply_OK:
		case AT_Reply_Connect:
			return ERR_NONE;
		case AT_Reply_Error:
			return ERR_NOTSUPPORTED;
		case AT_Reply_CMSError:
			return ATGEN_HandleCMSError(s);
		case AT_Reply_CMEError:
			return ATGEN_HandleCMEError(s);
		default:
			break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_CheckSBNR(GSM_StateMachine *s)
{
	GSM_Error 	error;
	char		req[20];

	sprintf(req, "AT^SBNR=?\r");
	smprintf(s, "Checking availablity of SBNR\n");
	ATGEN_WaitFor(s, req, strlen(req), 0x00, 4, ID_GetMemory);
	return error;
}

GSM_Error ATGEN_CheckSPBR(GSM_StateMachine *s)
{
	GSM_Error 	error;
	char		req[20];

	sprintf(req, "AT+SPBR=?\r");
	smprintf(s, "Checking availablity of SPBR\n");
	ATGEN_WaitFor(s, req, strlen(req), 0x00, 4, ID_GetMemory);
	return error;
}

GSM_Error ATGEN_CheckMPBR(GSM_StateMachine *s)
{
	GSM_Error 	error;
	char		req[20];

	sprintf(req, "AT+MPBR=?\r");
	smprintf(s, "Checking availablity of MPBR\n");
	ATGEN_WaitFor(s, req, strlen(req), 0x00, 4, ID_GetMemory);
	return error;
}


GSM_Error ATGEN_SetPBKMemory(GSM_StateMachine *s, GSM_MemoryType MemType)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	char 			req[] = "AT+CPBS=\"XX\"\r";
	GSM_Error		error;

	if (MemType == MEM_ME) {
		if (Priv->PBKSBNR == 0) {
			ATGEN_CheckSBNR(s);
		}
	}
	if (Priv->PBK_SPBR == 0) {
		ATGEN_CheckSPBR(s);
	}
	if (Priv->PBK_MPBR == 0) {
		ATGEN_CheckMPBR(s);
	}
	if (Priv->PBKMemory == MemType) return ERR_NONE;

	/* Zero values that are for actual memory */
	Priv->MemorySize		= 0;
	Priv->MemoryUsed		= 0;
	Priv->FirstMemoryEntry		= -1;
	Priv->NextMemoryEntry		= 0;
	Priv->TextLength		= 0;
	Priv->NumberLength		= 0;

	/* If phone encodes also values in command, we need normal charset */
	error = ATGEN_SetCharset(s, AT_PREF_CHARSET_NORMAL);
	if (error != ERR_NONE) return error;

	if (Priv->PBKMemories[0] == 0) {
		ATGEN_WaitFor(s, "AT+CPBS=?\r", 10, 0x00, 3, ID_SetMemoryType);
		if (error != ERR_NONE) {
			/*
			 * We weren't able to read available memories, let's
			 * guess that phone supports all. This is TRUE at least
			 * for Samsung.
			 */
			strcpy(s->Phone.Data.Priv.ATGEN.PBKMemories, "\"ME\",\"SM\",\"DC\",\"ON\",\"LD\",\"FD\",\"MC\",\"RC\"");
			smprintf(s, "Falling back to default memories list: %s\n", s->Phone.Data.Priv.ATGEN.PBKMemories);
		}
	}

	switch (MemType) {
		case MEM_SM:
			req[9] = 'S'; req[10] = 'M';
			break;
		case MEM_ME:
		        if (strstr(Priv->PBKMemories,"ME") != NULL) {
				req[9] = 'M'; req[10] = 'E';
				break;
			}
		        if (strstr(Priv->PBKMemories,"MT") != NULL) {
				req[9] = 'M'; req[10] = 'T';
				break;
			}
			return ERR_NOTSUPPORTED;
		case MEM_RC:
		        if (strstr(Priv->PBKMemories,"RC")==NULL) return ERR_NOTSUPPORTED;
			req[9] = 'R'; req[10] = 'C';
			break;
		case MEM_MC:
		        if (strstr(Priv->PBKMemories,"MC")==NULL) return ERR_NOTSUPPORTED;
			req[9] = 'M'; req[10] = 'C';
			break;
		case MEM_ON:
		        if (strstr(Priv->PBKMemories,"ON")==NULL) return ERR_NOTSUPPORTED;
			req[9] = 'O'; req[10] = 'N';
			break;
		case MEM_FD:
		        if (strstr(Priv->PBKMemories,"FD")==NULL) return ERR_NOTSUPPORTED;
			req[9] = 'F'; req[10] = 'D';
			break;
		case MEM_QD:
		        if (strstr(Priv->PBKMemories,"QD")==NULL) return ERR_NOTSUPPORTED;
			req[9] = 'Q'; req[10] = 'D';
			break;
		case MEM_DC:
			if (strstr(Priv->PBKMemories,"DC")!=NULL) {
				req[9] = 'D'; req[10] = 'C';
				break;
			}
			if (strstr(Priv->PBKMemories,"LD")!=NULL) {
				req[9] = 'L'; req[10] = 'D';
				break;
			}
			return ERR_NOTSUPPORTED;
		default:
			return ERR_NOTSUPPORTED;
	}

	smprintf(s, "Setting memory type\n");
	ATGEN_WaitFor(s, req, 13, 0x00, 3, ID_SetMemoryType);
	if (error == ERR_NONE) Priv->PBKMemory = MemType;
	return error;
}

GSM_Error ATGEN_ReplyGetCPBSMemoryStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
 	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	unsigned char tmp[200];
	GSM_Error error;
	const char *str;

	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Memory status received\n");
		str = GetLineString(msg.Buffer, &Priv->Lines, 2);

		error = ATGEN_ParseReply(s, str,
					"+CPBS: @s, @i, @i",
					tmp, sizeof(tmp) / 2,
					&Priv->MemoryUsed,
					&Priv->MemorySize);
		if (error == ERR_UNKNOWNRESPONSE) {
			return ERR_NOTSUPPORTED;
		}
		return error;
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(s);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

/**
 * Parses reply from phone about available entries.
 *
 * Standard format:
 * \verbatim
 * +CPBR: (first-last),max_number_len,max_name_len
 * \endverbatim
 * \verbatim
 * +CPBR: (location),max_number_len,max_name_len
 * \endverbatim
 *
 * Some phones (eg. Motorola C350) reply is different:
 * \verbatim
 * +CPBR: first-last,max_number_len,max_name_len
 * \endverbatim
 *
 * Some phones do not list positions (Sharp):
 * \verbatim
 * +CPBR: (),max_number_len,max_name_len
 * \endverbatim
 *
 * Some phones (eg. Nokia 6600 slide) append some additional values to
 * standard format.
 *
 * Samsung phones sometimes have additional number after standard format.
 * I currently have no idea what does this number mean.
 *
 * \todo
 * We currently guess memory size for Sharp to 1000.
 */
GSM_Error ATGEN_ReplyGetCPBRMemoryInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
 	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	const char *str;
	GSM_Error error;
	int ignore;

 	switch (Priv->ReplyState) {
 	case AT_Reply_OK:
		smprintf(s, "Memory info received\n");

		str = GetLineString(msg.Buffer, &Priv->Lines, 2);

		/* Check for empty reply */
		if (strcmp("OK", str) == 0) {
			return ERR_UNKNOWN;
		}

		/* Try standard format first */
		error = ATGEN_ParseReply(s, str,
					"+CPBR: (@i-@i), @i, @i",
					&Priv->FirstMemoryEntry,
					&Priv->MemorySize,
					&Priv->NumberLength,
					&Priv->TextLength);
		if (error == ERR_NONE) {
			/* Calculate memory size from last position we got from phone */
			Priv->MemorySize = Priv->MemorySize + 1 - Priv->FirstMemoryEntry;
			return ERR_NONE;
		}

		/* Try Motorola format then */
		error = ATGEN_ParseReply(s, str,
					"+CPBR: @i-@i, @i, @i",
					&Priv->FirstMemoryEntry,
					&Priv->MemorySize,
					&Priv->NumberLength,
					&Priv->TextLength);
		if (error == ERR_NONE) {
			/* Calculate memory size from last position we got from phone */
			Priv->MemorySize = Priv->MemorySize + 1 - Priv->FirstMemoryEntry;
			return ERR_NONE;
		}

		/* Try Sharp format */
		error = ATGEN_ParseReply(s, str,
					"+CPBR: (), @i, @i",
					&Priv->NumberLength,
					&Priv->TextLength);
		if (error == ERR_NONE) {
			/* Hardcode size, we have no other choice here */
			Priv->FirstMemoryEntry = 1;
			Priv->MemorySize = 1000;
			return ERR_NONE;
		}

		/* Try single entry format */
		error = ATGEN_ParseReply(s, str,
					"+CPBR: (@i), @i, @i",
					&Priv->FirstMemoryEntry,
					&Priv->NumberLength,
					&Priv->TextLength);
		if (error == ERR_NONE) {
			/* Hardcode size, we have no other choice here */
			Priv->MemorySize = 1;
			return ERR_NONE;
		}

		/* Try Samsung format at the end */
		error = ATGEN_ParseReply(s, str,
					"+CPBR: (@i-@i), @i, @i, @i",
					&Priv->FirstMemoryEntry,
					&Priv->MemorySize,
					&Priv->NumberLength,
					&Priv->TextLength,
					&ignore);
		if (error == ERR_NONE) {
			/* Calculate memory size from last position we got from phone */
			Priv->MemorySize = Priv->MemorySize + 1 - Priv->FirstMemoryEntry;
			return ERR_NONE;
		}


		/* Try standard format + unknown field */
		error = ATGEN_ParseReply(s, str,
					"+CPBR: (@i-@i), @i, @i, @0",
					&Priv->FirstMemoryEntry,
					&Priv->MemorySize,
					&Priv->NumberLength,
					&Priv->TextLength);
		if (error == ERR_NONE) {
			/* Calculate memory size from last position we got from phone */
			Priv->MemorySize = Priv->MemorySize + 1 - Priv->FirstMemoryEntry;
			return ERR_NONE;
		}

		/* Try cripled standard format */
		error = ATGEN_ParseReply(s, str,
					"+CPBR: (@i-@i)",
					&Priv->FirstMemoryEntry,
					&Priv->MemorySize);
		if (error == ERR_NONE) {
			/* Calculate memory size from last position we got from phone */
			Priv->MemorySize = Priv->MemorySize + 1 - Priv->FirstMemoryEntry;
			return ERR_NONE;
		}

		/* We don't get reply on first attempt on some Samsung phones */
		if (Priv->Manufacturer == AT_Samsung) {
			return ERR_NONE;
		}
		return ERR_UNKNOWNRESPONSE;
	case AT_Reply_Error:
		return ERR_UNKNOWN;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(s);
 	default:
		return ERR_UNKNOWNRESPONSE;
	}
}

GSM_Error ATGEN_ReplyGetCPBRMemoryStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Error		error;
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	int			line = 1;
	const char			*str;
	int			cur, last = -1;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Memory entries for status received\n");
		/* Walk through lines with +CPBR: */
		while (strcmp("OK", str = GetLineString(msg.Buffer, &Priv->Lines, line + 1)) != 0) {

			/* Parse reply */
			error = ATGEN_ParseReply(s, str, "+CPBR: @i, @0", &cur);
			if (error != ERR_NONE) {
				return error;
			}

			/* Some phones wrongly return several lines with same location,
			 * we need to catch it here to get correct count. */
			if (cur != last) {
				Priv->MemoryUsed++;
			}
			last = cur;
			cur -= Priv->FirstMemoryEntry - 1;
			if (cur == Priv->NextMemoryEntry || Priv->NextMemoryEntry == 0)
				Priv->NextMemoryEntry = cur + 1;

			/* Go to next line */
			line++;
		}
		smprintf(s, "Memory status: Used: %d, Next: %d\n",
				Priv->MemoryUsed,
				Priv->NextMemoryEntry);
		return ERR_NONE;
	case AT_Reply_Error:
		return ERR_UNKNOWN;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(s);
	default:
		return ERR_UNKNOWNRESPONSE;
	}
}

GSM_Error ATGEN_GetMemoryInfo(GSM_StateMachine *s, GSM_MemoryStatus *Status, GSM_AT_NeededMemoryInfo NeededInfo)
{
	GSM_Error		error;
	char			req[20];
	int			start;
	int			end;
	int			memory_end;
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	gboolean			free_read = FALSE;

	/* This can be NULL at this point */
	if (Status != NULL) {
		Status->MemoryUsed = 0;
		Status->MemoryFree = 0;
	}

	/* For reading we prefer unicode */
	error = ATGEN_SetCharset(s, AT_PREF_CHARSET_UNICODE);
	if (error != ERR_NONE) return error;

	Priv->MemorySize		= 0;
	Priv->MemoryUsed		= 0;
	/* Safe default values */
	Priv->TextLength		= 20;
	Priv->NumberLength		= 20;
	Priv->FirstMemoryEntry		= 1;

	/*
	 * First we try AT+CPBS?. It should return size of memory and
	 * number of used entries, but some vendors do not support this
	 * (SE).
	 */
	/*
	 * Some workaround for buggy mobile, that hangs after "AT+CPBS?" for other
	 * memory than SM.
	 */
	if (!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_BROKENCPBS) || (Priv->PBKMemory == MEM_SM)) {
		smprintf(s, "Getting memory status\n");
		ATGEN_WaitFor(s, "AT+CPBS?\r", 9, 0x00, 4, ID_GetMemoryStatus);
		if (error == ERR_NONE) free_read = TRUE;
	}

	/**
	 * Try to get memory size, first entry and length of entries
	 * this way.
	 */
	smprintf(s, "Getting memory information\n");
	ATGEN_WaitFor(s, "AT+CPBR=?\r", 10, 0x00, 10, ID_GetMemoryStatus);
	/* Did we fail to get size in either way? */
	if (error != ERR_NONE && Priv->MemorySize == 0) return error;
	/* Fill in Status structure if we were asked for it */
	if (Priv->MemorySize != 0 && Status != NULL) {
		Status->MemoryUsed = Priv->MemoryUsed;
		Status->MemoryFree = Priv->MemorySize - Priv->MemoryUsed;
	}
	if ((NeededInfo != AT_NextEmpty) &&
			(NeededInfo != AT_Status || free_read)) {
		return ERR_NONE;
	}

	smprintf(s, "Getting memory status by reading values\n");

	Status->MemoryUsed		= 0;
	Status->MemoryFree		= 0;
	start				= Priv->FirstMemoryEntry;
	Priv->NextMemoryEntry		= Priv->FirstMemoryEntry;
	memory_end = Priv->MemorySize + Priv->FirstMemoryEntry - 1;
	while (1) {
		/* Calculate end of next request */
		end	= start + 20;
		if (end > memory_end)
			end = memory_end;

		/* Read next interval */
		sprintf(req, "AT+CPBR=%i,%i\r", start, end);
		ATGEN_WaitFor(s, req, strlen(req), 0x00, 20, ID_GetMemoryStatus);
		if (error != ERR_NONE) return error;

		/* Do we already have first empty record? */
		if (NeededInfo == AT_NextEmpty &&
				Priv->NextMemoryEntry != end + 1)
			return ERR_NONE;

		/* Did we hit memory end? */
		if (end == memory_end) {
			Status->MemoryUsed = Priv->MemoryUsed;
			Status->MemoryFree = Priv->MemorySize - Priv->MemoryUsed;
			return ERR_NONE;
		}

		/* Continue on next location */
		start = end + 1;
	}
}

GSM_Error ATGEN_GetMemoryStatus(GSM_StateMachine *s, GSM_MemoryStatus *Status)
{
	GSM_Error		error;
 	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;

	error = ATGEN_SetPBKMemory(s, Status->MemoryType);
	if (error != ERR_NONE) return error;

	/* Catch errorneous 0 returned by some Siemens phones for ME. There is
	 * probably no way to get status there. */
	if (Priv->PBKSBNR == AT_AVAILABLE && Status->MemoryType == MEM_ME && Status->MemoryFree == 0)
		return ERR_NOTSUPPORTED;

	return ATGEN_GetMemoryInfo(s, Status, AT_Status);
}

/**
 * Parses reply on AT+CPBR=n.
 *
 * \todo Handle special replies from some phones:
 * LG C1200:
 * +CPBR: 23,"Primary Number",145,"Name",3,"0123456789",145,2,"0123456789",145,1,"E-Mail-Address without domain","Fax-Number",255
 * 3 = Home Number
 * 2 = Office Number
 * 1 = Mobile Number
 *
 * Samsung SGH-P900 reply:
 * +CPBR: 81,"#121#",129,"My Tempo",0
 */
GSM_Error ATGEN_ReplyGetMemory(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
 	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
 	GSM_MemoryEntry		*Memory = s->Phone.Data.Memory;
	GSM_Error		error;
	unsigned char		buffer[500];
	int offset, i;
	int number_type, types[10];

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
 		smprintf(s, "Phonebook entry received\n");
		/* Check for empty entries */
		if (strcmp("OK", GetLineString(msg.Buffer, &Priv->Lines, 2)) == 0) {
			return ERR_EMPTY;
		}

		/* Set number type */
		Memory->Entries[0].EntryType = PBK_Number_General;
		Memory->Entries[0].VoiceTag = 0;
		Memory->Entries[0].SMSList[0] = 0;

		/* Set name type */
		Memory->Entries[1].EntryType = PBK_Text_Name;

		/* Try standard reply */
		error = ATGEN_ParseReply(s,
					GetLineString(msg.Buffer, &Priv->Lines, 2),
					"+CPBR: @i, @p, @I, @e",
					&Memory->Location,
					Memory->Entries[0].Text, sizeof(Memory->Entries[0].Text),
					&number_type,
					Memory->Entries[1].Text, sizeof(Memory->Entries[1].Text));
		if (error == ERR_NONE) {
			smprintf(s, "Generic AT reply detected\n");
			/* Adjust location */
			Memory->Location = Memory->Location + 1 - Priv->FirstMemoryEntry;
			/* Adjust number */
			GSM_TweakInternationalNumber(Memory->Entries[0].Text, number_type);
			/* Set number of entries */
			Memory->EntriesNum = 2;
			return ERR_NONE;
		}

		/* Try reply with extra unknown number (maybe group?), seen on Samsung SGH-P900 */
		error = ATGEN_ParseReply(s,
					GetLineString(msg.Buffer, &Priv->Lines, 2),
					"+CPBR: @i, @p, @I, @e, @i",
					&Memory->Location,
					Memory->Entries[0].Text, sizeof(Memory->Entries[0].Text),
					&number_type,
					Memory->Entries[1].Text, sizeof(Memory->Entries[1].Text),
					&i /* Don't know what this means */
					);
		if (error == ERR_NONE) {
			smprintf(s, "AT reply with extra number detected\n");
			/* Adjust location */
			Memory->Location = Memory->Location + 1 - Priv->FirstMemoryEntry;
			/* Adjust number */
			GSM_TweakInternationalNumber(Memory->Entries[0].Text, number_type);
			/* Set number of entries */
			Memory->EntriesNum = 2;
			return ERR_NONE;
		}

		/* Try reply with call date */
		error = ATGEN_ParseReply(s,
					GetLineString(msg.Buffer, &Priv->Lines, 2),
					"+CPBR: @i, @p, @I, @s, @d",
					&Memory->Location,
					Memory->Entries[0].Text, sizeof(Memory->Entries[0].Text),
					&number_type,
					Memory->Entries[1].Text, sizeof(Memory->Entries[1].Text),
					&Memory->Entries[2].Date);
		if (error == ERR_NONE) {
			smprintf(s, "Reply with date detected\n");
			/* Adjust location */
			Memory->Location = Memory->Location + 1 - Priv->FirstMemoryEntry;
			/* Adjust number */
			GSM_TweakInternationalNumber(Memory->Entries[0].Text, number_type);
			/* Set date type */
			Memory->Entries[2].EntryType = PBK_Date;
			/* Set number of entries */
			Memory->EntriesNum = 3;
			/* Check whether date is correct */
			if (!CheckTime(&Memory->Entries[2].Date) || !CheckDate(&Memory->Entries[2].Date)) {
				smprintf(s, "Date looks invalid, ignoring!\n");
				Memory->EntriesNum = 2;
			}
			return ERR_NONE;
		}

		/*
		 * Try reply with call date and some additional string.
		 * I have no idea what should be stored there.
		 * We store it in Entry 3, but do not use it for now.
		 * Seen on T630.
		 */
		error = ATGEN_ParseReply(s,
					GetLineString(msg.Buffer, &Priv->Lines, 2),
					"+CPBR: @i, @s, @p, @I, @s, @d",
					&Memory->Location,
					Memory->Entries[3].Text, sizeof(Memory->Entries[3].Text),
					Memory->Entries[0].Text, sizeof(Memory->Entries[0].Text),
					&number_type,
					Memory->Entries[1].Text, sizeof(Memory->Entries[1].Text),
					&Memory->Entries[2].Date);
		if (error == ERR_NONE) {
			smprintf(s, "Reply with date detected\n");
			/* Adjust location */
			Memory->Location = Memory->Location + 1 - Priv->FirstMemoryEntry;
			/* Adjust number */
			GSM_TweakInternationalNumber(Memory->Entries[0].Text, number_type);
			/* Set date type */
			Memory->Entries[2].EntryType = PBK_Date;
			/* Set number of entries */
			Memory->EntriesNum = 3;
			return ERR_NONE;
		}

		/**
		 * Samsung format:
		 * location,"number",type,"0x02surname0x03","0x02firstname0x03","number",
		 * type,"number",type,"number",type,"number",type,"email","NA",
		 * "0x02note0x03",category?,x,x,x,ringtone?,"NA","photo"
		 *
		 * NA fields were empty
		 * x fields are some numbers, default is 1,65535,255,255,65535
		 *
		 * Samsung number types:
		 * 2 - fax
		 * 4 - cell
		 * 5 - other
		 * 6 - home
		 * 7 - office
		 */
		if (Priv->Manufacturer == AT_Samsung) {
			/* Parse reply */
			error = ATGEN_ParseReply(s,
					GetLineString(msg.Buffer, &Priv->Lines, 2),
					"+CPBR: @i,@p,@i,@S,@S,@p,@i,@p,@i,@p,@i,@p,@i,@s,@s,@S,@i,@i,@i,@i,@i,@s,@s",
					&Memory->Location,
					Memory->Entries[0].Text, sizeof(Memory->Entries[0].Text),
					&types[0],
					Memory->Entries[1].Text, sizeof(Memory->Entries[1].Text), /* surname */
					Memory->Entries[2].Text, sizeof(Memory->Entries[2].Text), /* first name */
					Memory->Entries[3].Text, sizeof(Memory->Entries[3].Text),
					&types[3],
					Memory->Entries[4].Text, sizeof(Memory->Entries[4].Text),
					&types[4],
					Memory->Entries[5].Text, sizeof(Memory->Entries[5].Text),
					&types[5],
					Memory->Entries[6].Text, sizeof(Memory->Entries[6].Text),
					&types[6],
					Memory->Entries[7].Text, sizeof(Memory->Entries[7].Text), /* email */
					buffer, sizeof(buffer), /* We don't know this */
					Memory->Entries[8].Text, sizeof(Memory->Entries[8].Text), /* note */
					&Memory->Entries[9].Number, /* category */
					&number_type, /* We don't know this */
					&number_type, /* We don't know this */
					&number_type, /* We don't know this */
					&Memory->Entries[10].Number, /* ringtone ID */
					buffer, sizeof(buffer), /* We don't know this */
					Memory->Entries[11].Text, sizeof(Memory->Entries[11].Text) /* photo ID */
					);

			if (error == ERR_NONE) {
				smprintf(s, "Samsung reply detected\n");
				/* Set types */
				Memory->Entries[1].EntryType = PBK_Text_LastName;
				Memory->Entries[2].EntryType = PBK_Text_FirstName;
				Memory->Entries[7].EntryType = PBK_Text_Email;
				Memory->Entries[8].EntryType = PBK_Text_Note;
				Memory->Entries[9].EntryType = PBK_Category;
				Memory->Entries[10].EntryType = PBK_RingtoneID;
				Memory->Entries[11].EntryType = PBK_Text_PictureName;

				/* Adjust location */
				Memory->Location = Memory->Location + 1 - Priv->FirstMemoryEntry;

				/* Shift entries when needed */
				offset = 0;

#define SHIFT_ENTRIES(index) \
	for (i = index - offset + 1; i < GSM_PHONEBOOK_ENTRIES; i++) { \
		Memory->Entries[i - 1] = Memory->Entries[i]; \
	} \
	offset++;

#define CHECK_TEXT(index) \
				if (UnicodeLength(Memory->Entries[index - offset].Text) == 0) { \
					smprintf(s, "Entry %d is empty\n", index); \
					SHIFT_ENTRIES(index); \
				}
#define CHECK_NUMBER(index) \
				if (UnicodeLength(Memory->Entries[index - offset].Text) == 0) { \
					smprintf(s, "Entry %d is empty\n", index); \
					SHIFT_ENTRIES(index); \
				} else { \
					Memory->Entries[index - offset].VoiceTag   = 0; \
					Memory->Entries[index - offset].SMSList[0] = 0; \
					switch (types[index]) { \
						case 2: \
							Memory->Entries[index - offset].EntryType  = PBK_Number_Fax; \
							break; \
						case 4: \
							Memory->Entries[index - offset].EntryType  = PBK_Number_Mobile; \
							break; \
						case 5: \
							Memory->Entries[index - offset].EntryType  = PBK_Number_Other; \
							break; \
						case 6: \
							Memory->Entries[index - offset].EntryType  = PBK_Number_Home; \
							break; \
						case 7: \
							Memory->Entries[index - offset].EntryType  = PBK_Number_Work; \
							break; \
						default: \
							Memory->Entries[index - offset].EntryType  = PBK_Number_Other; \
							smprintf(s, "WARNING: Unknown memory entry type %d\n", types[index]); \
							break; \
					} \
				}
				CHECK_NUMBER(0);
				CHECK_TEXT(1);
				CHECK_TEXT(2);
				CHECK_NUMBER(3);
				CHECK_NUMBER(4);
				CHECK_NUMBER(5);
				CHECK_NUMBER(6);
				CHECK_TEXT(7);
				CHECK_TEXT(8);
				if (Memory->Entries[10 - offset].Number == 65535) {
					SHIFT_ENTRIES(10);
				}
				CHECK_TEXT(11);

#undef CHECK_NUMBER
#undef CHECK_TEXT
#undef SHIFT_ENTRIES
				/* Set number of entries */
				Memory->EntriesNum = 12 - offset;
				return ERR_NONE;
			}

		}

		return ERR_UNKNOWNRESPONSE;
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(s);
	case AT_Reply_Error:
 		smprintf(s, "Error - too high location ?\n");
		return ERR_INVALIDLOCATION;
	case AT_Reply_CMSError:
 	        return ATGEN_HandleCMSError(s);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_PrivGetMemory (GSM_StateMachine *s, GSM_MemoryEntry *entry, int endlocation)
{
	GSM_Error 		error;
	char		req[20];
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;

	if (entry->Location==0x00) return ERR_INVALIDLOCATION;

	if (entry->MemoryType == MEM_ME) {
		if (Priv->PBKSBNR == 0) {
			ATGEN_CheckSBNR(s);
		}
		if (Priv->PBKSBNR == AT_AVAILABLE) {
			sprintf(req, "AT^SBNR=vcf,%i\r",entry->Location + Priv->FirstMemoryEntry - 1);
			s->Phone.Data.Memory=entry;
			smprintf(s, "Getting phonebook entry\n");
			ATGEN_WaitFor(s, req, strlen(req), 0x00, 4, ID_GetMemory);

			return error;
		}
	}

	if (Priv->PBK_SPBR == 0) {
		ATGEN_CheckSPBR(s);
	}
	if (Priv->PBK_MPBR == 0) {
		ATGEN_CheckMPBR(s);
	}

	error = ATGEN_GetManufacturer(s);
	if (error != ERR_NONE) return error;

	error = ATGEN_SetPBKMemory(s, entry->MemoryType);
	if (error != ERR_NONE) return error;

	/* For reading we prefer unicode */
	error = ATGEN_SetCharset(s, AT_PREF_CHARSET_UNICODE);
	if (error != ERR_NONE) return error;

	if (Priv->FirstMemoryEntry == -1) {
		error = ATGEN_GetMemoryInfo(s, NULL, AT_First);
		if (error != ERR_NONE) return error;
	}

	if (Priv->PBK_SPBR == AT_AVAILABLE) {
		sprintf(req, "AT+SPBR=%i\r", entry->Location + Priv->FirstMemoryEntry - 1);
	} else if (Priv->PBK_MPBR == AT_AVAILABLE) {
		sprintf(req, "AT+MPBR=%i\r", entry->Location + Priv->FirstMemoryEntry - 1);
	} else {
		if (endlocation == 0) {
			sprintf(req, "AT+CPBR=%i\r", entry->Location + Priv->FirstMemoryEntry - 1);
		} else {
			sprintf(req, "AT+CPBR=%i,%i\r", entry->Location + Priv->FirstMemoryEntry - 1, endlocation + Priv->FirstMemoryEntry - 1);
		}
	}

	s->Phone.Data.Memory=entry;
	smprintf(s, "Getting phonebook entry\n");
	ATGEN_WaitFor(s, req, strlen(req), 0x00, 30, ID_GetMemory);

	return error;
}

GSM_Error ATGEN_GetMemory (GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	return ATGEN_PrivGetMemory(s, entry, 0);
}

GSM_Error ATGEN_GetNextMemory (GSM_StateMachine *s, GSM_MemoryEntry *entry, gboolean start)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error		error;
	int			step = 0;

	error = ATGEN_GetManufacturer(s);
	if (error != ERR_NONE) return error;

	if (entry->MemoryType == MEM_ME) {
		if (Priv->PBKSBNR == 0) {
			ATGEN_CheckSBNR(s);
		}
	}
	if (Priv->PBK_SPBR == 0) {
		ATGEN_CheckSPBR(s);
	}
	if (Priv->PBK_MPBR == 0) {
		ATGEN_CheckMPBR(s);
	}
	/* There are no status functions for SBNR */
	if (entry->MemoryType != MEM_ME || Priv->PBKSBNR != AT_AVAILABLE) {
		error = ATGEN_SetPBKMemory(s, entry->MemoryType);
		if (error != ERR_NONE) return error;

		if (Priv->MemorySize == 0) {
			error = ATGEN_GetMemoryInfo(s, NULL, AT_Total);
			if (error != ERR_NONE) return error;
		}
	}

	if (start) {
		entry->Location = 1;
	} else {
		entry->Location++;
	}
	while ((error = ATGEN_PrivGetMemory(s, entry, step == 0 ? 0 : MIN(Priv->MemorySize, entry->Location + step))) == ERR_EMPTY) {
		entry->Location += step + 1;
		if (entry->Location > Priv->MemorySize) break;
		/* SBNR works only for one location */
		if ((entry->MemoryType != MEM_ME || Priv->PBKSBNR != AT_AVAILABLE) &&
				Priv->PBK_MPBR != AT_AVAILABLE &&
				Priv->PBK_SPBR != AT_AVAILABLE) {
			step = MIN(step + 2, 20);
		}
	}
	if (error == ERR_INVALIDLOCATION) return ERR_EMPTY;
	return error;
}

GSM_Error ATGEN_DeleteAllMemory(GSM_StateMachine *s, GSM_MemoryType type)
{
	GSM_Error 		error;
	unsigned char		req[100];
	int			i;
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;

	error = ATGEN_SetPBKMemory(s, type);
	if (error != ERR_NONE) return error;

	if (Priv->MemorySize == 0) {
		error = ATGEN_GetMemoryInfo(s, NULL, AT_Total);
		if (error != ERR_NONE) return error;
	}

	if (Priv->FirstMemoryEntry == -1) {
		error = ATGEN_GetMemoryInfo(s, NULL, AT_First);
		if (error != ERR_NONE) return error;
	}


	smprintf(s, "Deleting all phonebook entries\n");
	for (i = Priv->FirstMemoryEntry; i < Priv->FirstMemoryEntry + Priv->MemorySize; i++) {
		sprintf(req, "AT+CPBW=%d\r",i);
		ATGEN_WaitFor(s, req, strlen(req), 0x00, 4, ID_SetMemory);
		if (error != ERR_NONE) return error;
	}
	return ERR_NONE;
}

GSM_Error ATGEN_ReplyDialVoice(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Dial voice OK\n");
		return ERR_NONE;
	case AT_Reply_Error:
		smprintf(s, "Dial voice error\n");
		return ERR_UNKNOWN;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
	        return ATGEN_HandleCMEError(s);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_DialService(GSM_StateMachine *s, char *number)
{
	char *req;
	const char format[] = "AT+CUSD=%d,\"%s\"\r";
	GSM_Error error;

	req = (char *)malloc(strlen(format) + strlen(number) + 1);
	if (req == NULL) {
		return ERR_MOREMEMORY;
	}

	error = ATGEN_SetCharset(s, AT_PREF_CHARSET_NORMAL);
	if (error != ERR_NONE) {
		free(req);
		return error;
	}

	sprintf(req, format, s->Phone.Data.EnableIncomingUSSD ? 1 : 0, number);

	ATGEN_WaitFor(s, req, strlen(req), 0x00, 30, ID_GetUSSD);

	free(req);
	return error;
}

GSM_Error ATGEN_DialVoice(GSM_StateMachine *s, char *number, GSM_CallShowNumber ShowNumber)
{
	char buffer[GSM_MAX_NUMBER_LENGTH + 6];
	size_t len;
	GSM_Error error;

	if (ShowNumber != GSM_CALL_DefaultNumberPresence) return ERR_NOTSUPPORTED;
	if (strlen(number) > GSM_MAX_NUMBER_LENGTH) return ERR_MOREMEMORY;

	smprintf(s, "Making voice call\n");
	len = sprintf(buffer, "ATDT%s;\r", number);
	ATGEN_WaitFor(s, buffer, len, 0x00, 20, ID_DialVoice);

	if (error == ERR_INVALIDLOCATION) {
		smprintf(s, "Making voice call without forcing to tone dial\n");
		len = sprintf(buffer, "ATD%s;\r", number);
		ATGEN_WaitFor(s, buffer, len, 0x00, 20, ID_DialVoice);
	}

	return error;
}

GSM_Error ATGEN_ReplyEnterSecurityCode(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Security code was OK\n");
		return ERR_NONE;
	case AT_Reply_Error:
		smprintf(s, "Incorrect security code\n");
		return ERR_SECURITYERROR;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
	        return ATGEN_HandleCMEError(s);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_EnterSecurityCode(GSM_StateMachine *s, GSM_SecurityCode Code)
{
	unsigned char req[50];
	GSM_Error error;

	if (Code.Type == SEC_Pin2 &&
			s->Phone.Data.Priv.ATGEN.Manufacturer == AT_Siemens) {
		sprintf(req, "AT+CPIN2=\"%s\"\r", Code.Code);
	} else {
		sprintf(req, "AT+CPIN=\"%s\"\r" , Code.Code);
	}

	smprintf(s, "Entering security code\n");
	ATGEN_WaitFor(s, req, strlen(req), 0x00, 6, ID_EnterSecurityCode);
	return error;
}

GSM_Error ATGEN_ReplyGetSecurityStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_SecurityCodeType *Status = s->Phone.Data.SecurityStatus;

	smprintf(s, "Security status received - ");
	if (strstr(msg.Buffer,"READY")) {
		*Status = SEC_None;
		smprintf(s, "nothing to enter\n");
		return ERR_NONE;
	}
	if (strstr(msg.Buffer,"PH-SIM PIN")) {
		*Status = SEC_Phone;
		smprintf(s, "Phone code needed\n");
		return ERR_NONE;
	}
	if (strstr(msg.Buffer,"PH-NET PIN")) {
		*Status = SEC_Network;
		smprintf(s, "Network code needed\n");
		return ERR_NONE;
	}
	if (strstr(msg.Buffer,"PH_SIM PIN")) {
		smprintf(s, "no SIM inside or other error\n");
		return ERR_UNKNOWN;
	}
	if (strstr(msg.Buffer,"SIM PIN2")) {
		*Status = SEC_Pin2;
		smprintf(s, "waiting for PIN2\n");
		return ERR_NONE;
	}
	if (strstr(msg.Buffer,"SIM PUK2")) {
		*Status = SEC_Puk2;
		smprintf(s, "waiting for PUK2\n");
		return ERR_NONE;
	}
	if (strstr(msg.Buffer,"SIM PIN")) {
		*Status = SEC_Pin;
		smprintf(s, "waiting for PIN\n");
		return ERR_NONE;
	}
	if (strstr(msg.Buffer,"SIM PUK")) {
		*Status = SEC_Puk;
		smprintf(s, "waiting for PUK\n");
		return ERR_NONE;
	}
	smprintf(s, "unknown\n");
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetSecurityStatus(GSM_StateMachine *s, GSM_SecurityCodeType *Status)
{
	GSM_Error error;

	s->Phone.Data.SecurityStatus=Status;

	smprintf(s, "Getting security code status\n");
	/* Please note, that A2D doesn't return OK on the end.
 	 * Because of it we try to read another reply after reading
	 * status.
	 */
	ATGEN_WaitFor(s, "AT+CPIN?\r", 9, 0x00, 4, ID_GetSecurityStatus);
	/* Read the possible left over OK */
	GSM_WaitForOnce(s, NULL, 0x00, 0x00, 4);
	return error;
}

GSM_Error ATGEN_AnswerCall(GSM_StateMachine *s, int ID UNUSED, gboolean all)
{
	GSM_Error error;

	if (all) {
		smprintf(s, "Answering all calls\n");
		ATGEN_WaitFor(s, "ATA\r", 4, 0x00, 4, ID_AnswerCall);
		return error;
	}
	return ERR_NOTSUPPORTED;
}

GSM_Error ATGEN_ReplyCancelCall(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	GSM_Call call;

	switch(s->Phone.Data.Priv.ATGEN.ReplyState) {
        case AT_Reply_OK:
		smprintf(s, "Calls canceled\n");
		call.CallIDAvailable = FALSE;
		call.Status 	 = GSM_CALL_CallLocalEnd;
		if (s->User.IncomingCall) {
			s->User.IncomingCall(s, call, s->User.IncomingCallUserData);
		}

		return ERR_NONE;
    	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(s);
        default:
    	    return ERR_UNKNOWN;
	}
}

GSM_Error ATGEN_CancelCall(GSM_StateMachine *s, int ID UNUSED, gboolean all)
{
	GSM_Error error, error_ath;

	if (all) {
		smprintf(s, "Dropping all calls\n");
		ATGEN_WaitFor(s, "ATH\r", 4, 0x00, 4, ID_CancelCall);
		error_ath = error;
		ATGEN_WaitFor(s, "AT+CHUP\r", 8, 0x00, 4, ID_CancelCall);
		if (error_ath == ERR_NONE || error == ERR_NONE)
			return ERR_NONE;
		return error;
	}
	return ERR_NOTSUPPORTED;
}

GSM_Error ATGEN_ReplyReset(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Reset done\n");
	return ERR_NONE;
}

GSM_Error ATGEN_Reset(GSM_StateMachine *s, gboolean hard)
{
	GSM_Error error;

	if (!hard) return ERR_NOTSUPPORTED;

	smprintf(s, "Resetting device\n");
	/* Siemens 35 */
	ATGEN_WaitFor(s, "AT+CFUN=1,1\r", 12, 0x00, 8, ID_Reset);
	if (error != ERR_NONE) {
		/* Siemens M20 */
		ATGEN_WaitFor(s, "AT^SRESET\r", 10, 0x00, 8, ID_Reset);
	}
	return error;
}

GSM_Error ATGEN_ReplyResetPhoneSettings(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Reset done\n");
	return ERR_NONE;
}

GSM_Error ATGEN_ResetPhoneSettings(GSM_StateMachine *s, GSM_ResetSettingsType Type UNUSED)
{
	GSM_Error error;

	smprintf(s, "Resetting settings to default\n");
	ATGEN_WaitFor(s, "AT&F\r", 5, 0x00, 4, ID_ResetPhoneSettings);

	return error;
}

GSM_Error ATGEN_SetAutoNetworkLogin(GSM_StateMachine *s)
{
	GSM_Error error;

	smprintf(s, "Enabling automatic network login\n");
	ATGEN_WaitFor(s, "AT+COPS=0\r", 10, 0x00, 4, ID_SetAutoNetworkLogin);

	return error;
}

GSM_Error ATGEN_SendDTMF(GSM_StateMachine *s, char *sequence)
{
	char req[80] = "AT+VTS=";
	int n, len, pos;
	GSM_Error error;

	len = strlen(sequence);

	if (len > 32) return ERR_INVALIDDATA;

	pos = strlen(req);

	for (n = 0; n < len; n++) {
		if (n != 0) req[pos++] = ',';
		req[pos++] = sequence[n];
	}

	req[pos++] = '\r';
	req[pos++] = 0;

	smprintf(s, "Sending DTMF\n");
	ATGEN_WaitFor(s, req, strlen(req), 0x00, 4, ID_SendDTMF);

	return error;
}

GSM_Error ATGEN_ReplySetMemory(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Phonebook entry written OK\n");
		return ERR_NONE;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
		if (s->Phone.Data.Priv.ATGEN.ErrorCode == 255 && s->Phone.Data.Priv.ATGEN.Manufacturer == AT_Ericsson) {
			smprintf(s, "CME Error %i, probably means empty entry\n", s->Phone.Data.Priv.ATGEN.ErrorCode);
			return ERR_EMPTY;
		}
		if (s->Phone.Data.Priv.ATGEN.ErrorCode == 100) {
			return ERR_NOTSUPPORTED;
		}
	        return ATGEN_HandleCMEError(s);
	case AT_Reply_Error:
		return ERR_INVALIDDATA;
	default:
		return ERR_UNKNOWNRESPONSE;
	}
}

GSM_Error ATGEN_DeleteMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	GSM_Error 		error;
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	unsigned char		req[100];

	if (entry->Location < 1) return ERR_INVALIDLOCATION;

	error = ATGEN_SetPBKMemory(s, entry->MemoryType);
	if (error != ERR_NONE) return error;

	if (Priv->FirstMemoryEntry == -1) {
		error = ATGEN_GetMemoryInfo(s, NULL, AT_First);
		if (error != ERR_NONE) return error;
	}

	sprintf(req, "AT+CPBW=%d\r",entry->Location + Priv->FirstMemoryEntry - 1);

	smprintf(s, "Deleting phonebook entry\n");
	ATGEN_WaitFor(s, req, strlen(req), 0x00, 4, ID_SetMemory);
	if (error == ERR_EMPTY) return ERR_NONE;
	return error;
}

GSM_Error ATGEN_PrivSetMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	/* REQUEST_SIZE should be big enough to handle all possibl cases
	 * correctly, especially with unicode entries */
#define REQUEST_SIZE	((4 * GSM_PHONEBOOK_TEXT_LENGTH) + 30)
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	int			Group, Name, Number;
	/* 129 seems to be safer option for empty number */
	int			NumberType = 129;
	size_t len;
	GSM_Error 		error;
	unsigned char		req[REQUEST_SIZE + 1];
	unsigned char		name[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
	unsigned char		uname[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
	unsigned char		number[GSM_PHONEBOOK_TEXT_LENGTH + 1];
	unsigned char		unumber[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
	int			reqlen, i;
	GSM_AT_Charset_Preference	Prefer = AT_PREF_CHARSET_NORMAL;

	if (entry->Location == 0) return ERR_INVALIDLOCATION;

	error = ATGEN_SetPBKMemory(s, entry->MemoryType);
	if (error != ERR_NONE) return error;

	if (Priv->PBK_SPBR == 0) {
		ATGEN_CheckSPBR(s);
	}
	if (Priv->PBK_MPBR == 0) {
		ATGEN_CheckMPBR(s);
	}

	if (Priv->PBK_SPBR == AT_AVAILABLE) {
		return SAMSUNG_SetMemory(s, entry);
	}

	if (Priv->PBK_MPBR == AT_AVAILABLE) {
		smprintf(s, "WARNING: setting memory for Motorola not implemented yet!\n");
	}

	for (i=0;i<entry->EntriesNum;i++) {
		entry->Entries[i].AddError = ERR_NOTSUPPORTED;
	}

	GSM_PhonebookFindDefaultNameNumberGroup(entry, &Name, &Number, &Group);

	name[0] = 0;
	if (Name != -1) {
		len = UnicodeLength(entry->Entries[Name].Text);

		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_PBK_UNICODE)) {
			Prefer = AT_PREF_CHARSET_UNICODE;
		} else {
			/* Compare if we would loose some information when not using
			 * unicode */
			EncodeDefault(name, entry->Entries[Name].Text, &len, TRUE, NULL);
			DecodeDefault(uname, name, len, TRUE, NULL);
			if (!mywstrncmp(uname, entry->Entries[Name].Text, len)) {
				/* Get maximal text length */
				if (Priv->TextLength == 0) {
					ATGEN_GetMemoryInfo(s, NULL, AT_Sizes);
				}

				/* I char stored in GSM alphabet takes 7 bits, one
				 * unicode 16, if storing in unicode would truncate
				 * text, do not use it, otherwise we will use it */
				if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_LENGTH_BYTES) &&
						(Priv->TextLength != 0) &&
						((Priv->TextLength * 7 / 16) <= len)
						) {
					Prefer = AT_PREF_CHARSET_NORMAL;
				} else {
					Prefer = AT_PREF_CHARSET_UNICODE;
				}
			}
		}

		error = ATGEN_SetCharset(s, Prefer);
		if (error != ERR_NONE) return error;

		len = UnicodeLength(entry->Entries[Name].Text);
		error = ATGEN_EncodeText(s, entry->Entries[Name].Text, len, name, sizeof(name), &len);
		if (error != ERR_NONE) return error;

		entry->Entries[Name].AddError = ERR_NONE;
	} else {
		smprintf(s, "WARNING: No usable name found!\n");
		len = 0;
	}

	if (Number != -1) {
		GSM_PackSemiOctetNumber(entry->Entries[Number].Text, number, FALSE);
		NumberType = number[0];
		/* We need to encode number, however
		 *  - it is not encoded in UCS2
		 *  - no encoding is needed for most charsets
		 */
		if (Priv->Charset == AT_CHARSET_HEX &&
				GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_PBK_ENCODENUMBER)) {
			len = UnicodeLength(entry->Entries[Number].Text);
			EncodeDefault(unumber, entry->Entries[Number].Text, &len, TRUE, NULL);
			EncodeHexBin(number, unumber, len);
		} else {
			sprintf(number, "%s", DecodeUnicodeString(entry->Entries[Number].Text));
		}
		entry->Entries[Number].AddError = ERR_NONE;
	} else {
		smprintf(s, "WARNING: No usable number found!\n");
		number[0] = 0;
	}

	if (Priv->FirstMemoryEntry == -1) {
		error = ATGEN_GetMemoryInfo(s, NULL, AT_First);
		if (error != ERR_NONE) return error;
	}

	/* We can't use here:
	 * sprintf(req, "AT+CPBW=%d, \"%s\", %i, \"%s\"\r",
	 *         entry->Location, number, NumberType, name);
	 * because name can contain 0 when using GSM alphabet.
	 */
	sprintf(req, "AT+CPBW=%d,\"%s\",%i,\"", entry->Location + Priv->FirstMemoryEntry - 1, number, NumberType);
	reqlen = strlen(req);
	if (reqlen + len > REQUEST_SIZE - 4) {
		smprintf(s, "WARNING: Text truncated to fit in buffer!\n");
		len = REQUEST_SIZE - 4 - reqlen;
	}
	/* Add name */
	memcpy(req + reqlen, name, len);
	reqlen += len;
	/* Terminate quotes */
	memcpy(req + reqlen, "\"", 1);
	reqlen += 1;
	/* Some phones need ,0 at the end, whatever this number means */
	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_EXTRA_PBK_FIELD)) {
		memcpy(req + reqlen, ",0", 2);
		reqlen += 2;
	}
	/* Terminate request */
	memcpy(req + reqlen, "\r", 1);
	reqlen += 1;

	smprintf(s, "Writing phonebook entry\n");
	ATGEN_WaitFor(s, req, reqlen, 0x00, 40, ID_SetMemory);

	return error;
#undef REQUEST_SIZE
}

GSM_Error ATGEN_SetMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	if (entry->Location == 0) return ERR_INVALIDLOCATION;
	return ATGEN_PrivSetMemory(s, entry);
}

GSM_Error ATGEN_AddMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	GSM_Error 		error;
	GSM_MemoryStatus	Status;
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;

 	/* Switch to desired memory type */
 	error = ATGEN_SetPBKMemory(s, entry->MemoryType);
 	if (error != ERR_NONE) return error;

	/* Find out empty location */
	error = ATGEN_GetMemoryInfo(s, &Status, AT_NextEmpty);
	if (error != ERR_NONE) return error;
	if (Priv->NextMemoryEntry == 0) return ERR_FULL;
	entry->Location = Priv->NextMemoryEntry;

	return ATGEN_PrivSetMemory(s, entry);
}

GSM_Error ATGEN_SetIncomingCall(GSM_StateMachine *s, gboolean enable)
{
	GSM_Error error;
	if (enable) {
		smprintf(s, "Enabling incoming call\n");
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NO_CLIP)) {
			/* Some (especiall SE) phones are fucked up when we want to
			 * see CLIP information */
			ATGEN_WaitFor(s, "AT+CLIP=1\r", 10, 0x00, 3, ID_SetIncomingCall);
			if (error != ERR_NONE) return error;
			ATGEN_WaitFor(s, "AT+CRC=1\r", 9, 0x00, 3, ID_SetIncomingCall);
			if (error != ERR_NONE) return error;
		} else {
			ATGEN_WaitFor(s, "AT+CRC=0\r", 9, 0x00, 3, ID_SetIncomingCall);
			if (error != ERR_NONE) return error;
		}
		ATGEN_WaitFor(s, "AT+CCWA=1\r", 10, 0x00, 3, ID_SetIncomingCall);
		/* We don't care if phone does not support this */
	} else {
		ATGEN_WaitFor(s, "AT+CCWA=0\r", 10, 0x00, 3, ID_SetIncomingCall);
		/* We don't care if phone does not support this */
		smprintf(s, "Disabling incoming call\n");
	}
	s->Phone.Data.EnableIncomingCall = enable;
	return ERR_NONE;
}

/**
 * Extract number of incoming call from +CLIP: response.
 */
void ATGEN_Extract_CLIP_number(GSM_StateMachine *s, unsigned char *dest, size_t destsize, const char *buf)
{
	ATGEN_ParseReply(s, buf, "+CLIP: @p,@0", dest, destsize);
}

/**
 * Extract number of incoming call from +CLIP: response.
 */
void ATGEN_Extract_CCWA_number(GSM_StateMachine *s, unsigned char *dest, size_t destsize, const char *buf)
{
	ATGEN_ParseReply(s, buf, "+CCWA: @p,@0", dest, destsize);
}

GSM_Error ATGEN_ReplyIncomingCallInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Call 		call;

	memset(&call, 0, sizeof(call));

	smprintf(s, "Incoming call info\n");
	if (s->Phone.Data.EnableIncomingCall && s->User.IncomingCall!=NULL) {
		call.Status 		= 0;
		call.StatusCode		= 0;
		call.CallIDAvailable 	= FALSE;
		if (strstr(msg.Buffer, "RING")) {
			smprintf(s, "Ring detected - ");
			/* We ignore RING for most phones, see ATGEN_SetIncomingCall */
			if (s->Phone.Data.Priv.ATGEN.Manufacturer != AT_Ericsson) {
				smprintf(s, "ignoring\n");
				return ERR_NONE;
			}
			smprintf(s, "generating event\n");
			call.Status = GSM_CALL_IncomingCall;
			call.CallIDAvailable 	= TRUE;
			ATGEN_Extract_CLIP_number(s, call.PhoneNumber, sizeof(call.PhoneNumber), msg.Buffer);
		} else if (strstr(msg.Buffer, "CLIP:")) {
			smprintf(s, "CLIP detected\n");
			call.Status = GSM_CALL_IncomingCall;
			call.CallIDAvailable 	= TRUE;
			ATGEN_Extract_CLIP_number(s, call.PhoneNumber, sizeof(call.PhoneNumber), msg.Buffer);
		} else if (strstr(msg.Buffer, "CCWA:")) {
			smprintf(s, "CCWA detected\n");
			call.Status = GSM_CALL_IncomingCall;
			ATGEN_Extract_CCWA_number(s, call.PhoneNumber, sizeof(call.PhoneNumber), msg.Buffer);
			call.CallIDAvailable 	= TRUE;
		} else if (strstr(msg.Buffer, "NO CARRIER")) {
			smprintf(s, "Call end detected\n");
			call.Status = GSM_CALL_CallEnd;
			call.CallIDAvailable 	= TRUE;
		} else if (strstr(msg.Buffer, "COLP:")) {
			smprintf(s, "CLIP detected\n");
			call.Status = GSM_CALL_CallStart;
			call.CallIDAvailable 	= TRUE;
			ATGEN_Extract_CLIP_number(s, call.PhoneNumber, sizeof(call.PhoneNumber), msg.Buffer);
		} else {
			smprintf(s, "Incoming call error\n");
			return ERR_NONE;
		}

		s->User.IncomingCall(s, call, s->User.IncomingCallUserData);
	}

	return ERR_NONE;
}

GSM_Error ATGEN_IncomingGPRS(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	/* "+CGREG: 1,1" */
	smprintf(s, "GPRS change\n");
	return ERR_NONE;
}

GSM_Error ATGEN_IncomingBattery(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int 	level = 0;
	char 	*p;

	/* "_OBS: 92,1" */
	p = strstr(msg.Buffer, "_OBS:");
	if (p) level = atoi(p + 5);
	smprintf(s, "Battery level changed to %d\n", level);
	return ERR_NONE;
}

GSM_Error ATGEN_IncomingNetworkLevel(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int 	level = 0;
	char 	*p;

	/* "_OSIGQ: 12,0" */
	p = strstr(msg.Buffer, "_OSIGQ: ");
	if (p) level = atoi(p + 7);
	smprintf(s, "Network level changed to %d\n", level);
	return ERR_NONE;
}

GSM_Error ATGEN_ReplyGetSIMIMSI(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Phone_Data		*Data = &s->Phone.Data;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		CopyLineString(Data->PhoneString, msg.Buffer, &Priv->Lines, 2);

		/* Remove various prefies some phones add */
		if (strncmp(s->Phone.Data.IMEI, "<IMSI>: ", 7) == 0) { /* Alcatel */
			memmove(s->Phone.Data.IMEI, s->Phone.Data.IMEI + 7, strlen(s->Phone.Data.IMEI + 7) + 1);
		} else if (strncmp(s->Phone.Data.IMEI, "+CIMI: ", 7) == 0) { /* Motorola */
			memmove(s->Phone.Data.IMEI, s->Phone.Data.IMEI + 7, strlen(s->Phone.Data.IMEI + 7) + 1);
		}

		smprintf(s, "Received IMSI %s\n",Data->PhoneString);
		return ERR_NONE;
	case AT_Reply_Error:
		smprintf(s, "No access to SIM card or not supported by device\n");
		return ERR_SECURITYERROR;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
	        return ATGEN_HandleCMEError(s);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetSIMIMSI(GSM_StateMachine *s, char *IMSI)
{
	GSM_Error error;

	s->Phone.Data.PhoneString = IMSI;
	smprintf(s, "Getting SIM IMSI\n");
	ATGEN_WaitFor(s, "AT+CIMI\r", 8, 0x00, 4, ID_GetSIMIMSI);

	return error;
}

GSM_Error ATGEN_GetDisplayStatus(GSM_StateMachine *s UNUSED, GSM_DisplayFeatures *features UNUSED)
{
	return ERR_NOTIMPLEMENTED;
#if 0
	/**
	 * \todo Parsing of response is not implemented.
	 */

	s->Phone.Data.DisplayFeatures = features;
	smprintf(s, "Getting display status\n");
	ATGEN_WaitFor(s, "AT+CIND?\r",9, 0x00, 4, ID_GetDisplayStatus);

	return error;
#endif
}

GSM_Error ATGEN_ReplyGetBatteryCharge(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
    GSM_Phone_Data		*Data = &s->Phone.Data;
    int 			i;

    switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
        case AT_Reply_OK:
            smprintf(s, "Battery level received\n");
            Data->BatteryCharge->BatteryPercent = atoi(msg.Buffer+17);
            i = atoi(msg.Buffer+14);
            if (i >= 0 && i <= 3) {
                Data->BatteryCharge->ChargeState = i + 1;
            }
            return ERR_NONE;
        case AT_Reply_Error:
            smprintf(s, "Can't get battery level\n");
            return ERR_NOTSUPPORTED;
        case AT_Reply_CMSError:
            smprintf(s, "Can't get battery level\n");
            return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
	        return ATGEN_HandleCMEError(s);
        default:
            break;
    }
    return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetBatteryCharge(GSM_StateMachine *s, GSM_BatteryCharge *bat)
{
	GSM_Error error;

	GSM_ClearBatteryCharge(bat);
	s->Phone.Data.BatteryCharge = bat;
	smprintf(s, "Getting battery charge\n");
	ATGEN_WaitFor(s, "AT+CBC\r", 7, 0x00, 4, ID_GetBatteryCharge);

	return error;
}

GSM_Error ATGEN_ReplyGetSignalQuality(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_SignalQuality	*Signal = s->Phone.Data.SignalQuality;
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	int rssi, ber;
	GSM_Error error;

	Signal->SignalStrength 	= -1;
	Signal->SignalPercent 	= -1;
	Signal->BitErrorRate 	= -1;

	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
        case AT_Reply_OK:
		smprintf(s, "Signal quality info received\n");
		error = ATGEN_ParseReply(s,
				GetLineString(msg.Buffer, &Priv->Lines, 2),
				"+CSQ: @i, @i",
				&rssi,
				&ber);

		if (error != ERR_NONE) {
			/* Some phones do not prepend CSQ */
			error = ATGEN_ParseReply(s,
					GetLineString(msg.Buffer, &Priv->Lines, 2),
					"@i, @i",
					&rssi,
					&ber);
			if (error != ERR_NONE) {
				return error;
			}
		}

		/* 99 is Not known or not detectable. */
		if (rssi != 99) {
			/* from GSM 07.07 section 8.5 */
			Signal->SignalStrength = 2 * rssi - 113;

			/* received signal strength indication is in range 0 - 31 */
			if (rssi == 31) {
				Signal->SignalPercent = 100;
			} else {
				Signal->SignalPercent = 3 * rssi;
			}
			if (Signal->SignalPercent > 100) {
				Signal->SignalPercent = 100;
			}
		}

                /* from GSM 05.08 section 8.2.4 */
                switch (ber) {
			case 0: Signal->BitErrorRate =  0; break; /* 0.14 */
			case 1: Signal->BitErrorRate =  0; break; /* 0.28 */
			case 2: Signal->BitErrorRate =  1; break; /* 0.57 */
			case 3: Signal->BitErrorRate =  1; break; /* 1.13 */
			case 4: Signal->BitErrorRate =  2; break; /* 2.26 */
			case 5: Signal->BitErrorRate =  5; break; /* 4.53 */
			case 6: Signal->BitErrorRate =  9; break; /* 9.05 */
			case 7: Signal->BitErrorRate = 18; break; /* 18.10 */
                }
		return ERR_NONE;
        case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
	        return ATGEN_HandleCMEError(s);
	case AT_Reply_Error:
		return ERR_NOTSUPPORTED;
        default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetSignalQuality(GSM_StateMachine *s, GSM_SignalQuality *sig)
{
	GSM_Error error;

	s->Phone.Data.SignalQuality = sig;
	smprintf(s, "Getting signal quality info\n");
	ATGEN_WaitFor(s, "AT+CSQ\r", 7, 0x00, 4, ID_GetSignalQuality);

	return error;
}

/* When use AT+CPIN?, A2D returns it without OK and because of it Gammu
   parses answer without it.
   MC35 and other return OK after answer for AT+CPIN?. Here we handle it.
   Any better idea ?
 */
GSM_Error ATGEN_ReplyOK(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s UNUSED)
{
	return ERR_NONE;
}

/**
 * Just ignores reply we got.
 */
GSM_Error ATGEN_ReplyIgnore(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s UNUSED)
{
	return ERR_NONE;
}

static GSM_Error ATGEN_GetNextCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note, gboolean start)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;

	if (Priv->Manufacturer==AT_Siemens ) return SIEMENS_GetNextCalendar(s,Note,start);
	return ERR_NOTSUPPORTED;
}

GSM_Error ATGEN_GetCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;

	if (Priv->Manufacturer==AT_Siemens)  return SIEMENS_GetCalendar(s, Note);
	return ERR_NOTSUPPORTED;
}

GSM_Error ATGEN_Terminate(GSM_StateMachine *s)
{
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;

	FreeLines(&Priv->Lines);
	/* Free static buffer inside GetLineString */
	GetLineString(NULL, NULL, 0);
	free(Priv->file.Buffer);
	free(Priv->SMSCache);
	return ERR_NONE;
}

GSM_Error ATGEN_SetCalendarNote(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;

	if (Priv->Manufacturer==AT_Siemens)  return SIEMENS_SetCalendarNote(s, Note);
	return ERR_NOTSUPPORTED;
}

GSM_Error ATGEN_AddCalendarNote(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;

	if (Priv->Manufacturer==AT_Siemens)  return SIEMENS_AddCalendarNote(s, Note);
	return ERR_NOTSUPPORTED;
}

GSM_Error ATGEN_DelCalendarNote(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;

	if (Priv->Manufacturer==AT_Siemens)  return SIEMENS_DelCalendarNote(s, Note);
	return ERR_NOTSUPPORTED;
}


GSM_Error ATGEN_GetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;

	if (Priv->Manufacturer==AT_Siemens) return SIEMENS_GetBitmap(s, Bitmap);
	if (Priv->Manufacturer==AT_Samsung) return SAMSUNG_GetBitmap(s, Bitmap);
	return ERR_NOTSUPPORTED;
}

GSM_Error ATGEN_SetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;

	if (Priv->Manufacturer==AT_Siemens) return SIEMENS_SetBitmap(s, Bitmap);
	if (Priv->Manufacturer==AT_Samsung) return SAMSUNG_SetBitmap(s, Bitmap);
	return ERR_NOTSUPPORTED;
}

GSM_Error ATGEN_GetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, gboolean PhoneRingtone)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;

	if (Priv->Manufacturer==AT_Siemens) return SIEMENS_GetRingtone(s, Ringtone, PhoneRingtone);
	if (Priv->Manufacturer==AT_Samsung) return SAMSUNG_GetRingtone(s, Ringtone, PhoneRingtone);
	return ERR_NOTSUPPORTED;
}

GSM_Error ATGEN_SetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, int *maxlength)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;

	if (Priv->Manufacturer==AT_Siemens) return SIEMENS_SetRingtone(s, Ringtone, maxlength);
	if (Priv->Manufacturer==AT_Samsung) return SAMSUNG_SetRingtone(s, Ringtone, maxlength);
	return ERR_NOTSUPPORTED;
}

GSM_Error ATGEN_PressKey(GSM_StateMachine *s, GSM_KeyCode Key, gboolean Press)
{
	GSM_Error	error;
	unsigned char 	frame[40];
	char key[20];
	size_t len;
	unsigned char unicode_key[20];
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;

	/* We do nothing on release event */
	if (!Press) {
		return ERR_NONE;
	}

	/* Prefer IRA charaset to avoid tricky conversions */
	error = ATGEN_SetCharset(s, AT_PREF_CHARSET_IRA);

	/* Check error */
	if (error != ERR_NONE) {
		return error;
	}


	frame[0] = 0;

	strcat(frame, "AT+CKPD=\"");

	/* Get key code */
	switch (Key) {
		case GSM_KEY_1 			: strcpy(key, "1"); break;
		case GSM_KEY_2			: strcpy(key, "2"); break;
		case GSM_KEY_3			: strcpy(key, "3"); break;
		case GSM_KEY_4			: strcpy(key, "4"); break;
		case GSM_KEY_5			: strcpy(key, "5"); break;
		case GSM_KEY_6			: strcpy(key, "6"); break;
		case GSM_KEY_7			: strcpy(key, "7"); break;
		case GSM_KEY_8			: strcpy(key, "8"); break;
		case GSM_KEY_9			: strcpy(key, "9"); break;
		case GSM_KEY_0			: strcpy(key, "0"); break;
		case GSM_KEY_HASH		: strcpy(key, "#"); break;
		case GSM_KEY_ASTERISK		: strcpy(key, "*"); break;
		case GSM_KEY_POWER		: strcpy(key, "P"); break;
		case GSM_KEY_GREEN		: strcpy(key, "S"); break;
		case GSM_KEY_RED		: strcpy(key, "E"); break;
		case GSM_KEY_INCREASEVOLUME	: strcpy(key, "U"); break;
		case GSM_KEY_DECREASEVOLUME	: strcpy(key, "D"); break;
		case GSM_KEY_UP			: strcpy(key, "^"); break;
		case GSM_KEY_DOWN		: strcpy(key, "V"); break;
		case GSM_KEY_MENU		: strcpy(key, "F"); break;
		case GSM_KEY_LEFT		: strcpy(key, "<"); break;
		case GSM_KEY_RIGHT		: strcpy(key, ">"); break;
		case GSM_KEY_SOFT1		: strcpy(key, "["); break;
		case GSM_KEY_SOFT2		: strcpy(key, "]"); break;
		case GSM_KEY_HEADSET		: strcpy(key, "H"); break;
		case GSM_KEY_JOYSTICK		: strcpy(key, ":J"); break;
		case GSM_KEY_CAMERA		: strcpy(key, ":C"); break;
		case GSM_KEY_OPERATOR		: strcpy(key, ":O"); break;
		case GSM_KEY_RETURN		: strcpy(key, ":R"); break;
		case GSM_KEY_CLEAR		: strcpy(key, "C"); break;
		case GSM_KEY_MEDIA		: strcpy(key, ":S"); break;
		case GSM_KEY_DESKTOP		: strcpy(key, ":D"); break;
		case GSM_KEY_NONE		: return ERR_NONE; /* Nothing to do here */
		case GSM_KEY_NAMES		: return ERR_NOTSUPPORTED;
	}

	/* Convert charset if needed */
	EncodeUnicode(unicode_key, key, strlen(key));
	len = UnicodeLength(unicode_key);

	switch (Priv->Charset) {
		case AT_CHARSET_GSM:
			/* No extensions here */
			EncodeDefault(key, unicode_key, &len, FALSE, NULL);
			if (strcmp(key, "?") == 0) {
				smprintf(s, "Could not encode key to GSM charset!\n");
				return ERR_NOTSUPPORTED;
			}
			break;
		case AT_CHARSET_IRA:
		case AT_CHARSET_ASCII:
		case AT_CHARSET_UTF8:
		case AT_CHARSET_UTF_8:
		case AT_CHARSET_ISO88591:
			/* Nothing to do here */
			break;
		case AT_CHARSET_UCS2:
		case AT_CHARSET_UCS_2:
			EncodeHexUnicode(key, unicode_key, len);
			break;
		default:
			smprintf(s, "Not supported charset for key presses (%d)!\n", Priv->Charset);
			return ERR_NOTIMPLEMENTED;
	}

	strcat(frame, key);

	strcat(frame, "\"\r");
	smprintf(s, "Pressing key\n");
	ATGEN_WaitFor(s, frame, strlen(frame), 0x00, 4, ID_PressKey);
	if (error != ERR_NONE) return error;

	/* Strange. My T310 needs it */
	ATGEN_WaitFor(s, "ATE1\r", 5, 0x00, 4, ID_EnableEcho);
	return error;
}

gboolean InRange(int *range, int i) {
	while (*range != -1) {
		if (*range == i) return TRUE;
		range++;
	}
	return FALSE;
}

int *GetRange(char *buffer)
{
	int	*result;
	int	commas = 0, dashes = 0, i1, i2, i;
	char	*c = buffer, *c2;

	if (c[0] != '(') return NULL;
	c++;
	c2 = c;

	while (*c2 != ')') {
		if (*c2 == ',') commas++;
		else if (*c2 == '-') dashes++;
		c2++;
	}

	if ((commas != 0 && dashes != 0) || dashes > 1) {
		return NULL;
	} else if (commas == 0 && dashes == 0) {
		result = calloc(2, sizeof(int));
		if (result == NULL) return NULL;
		result[0] = atoi(c);
		result[1] = -1;
	} else if (dashes == 1) {
		i1 = atoi(c);
		c2 = c;
		while (*c2 != '-') c2++;
		c2++;
		i2  = atoi(c2);
		if (i2 < i1) return NULL;
		result = calloc(2 + i2 - i1, sizeof(int));
		if (result == NULL) return NULL;
		for (i = i1; i <= i2; i++) {
			result[i - i1] = i;
		}
		result[1 + i2 - i1] = -1;
	} else {
		result = calloc(2 + commas, sizeof(int));
		if (result == NULL) return NULL;
		i = 1;
		c2 = c;
		result[0] = atoi(c2);
		while (*c2 != ')') {
			if (*c2 == ',') {
				result[i] = atoi(c2 + 1);
				i++;
			}
			c2++;
		}
		result[i] = -1;
	}
	i = 0;
	return result;
}

GSM_Error ATGEN_ReplyGetCNMIMode(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	char			*buffer;
	int			*range;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		break;
	case AT_Reply_Error:
		return ERR_NOTSUPPORTED;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
	        return ATGEN_HandleCMEError(s);
	default:
		return ERR_UNKNOWNRESPONSE;
	}

	/* Sample resposne we get here:
	AT+CNMI=?
	+CNMI: (0-2),(0,1,3),(0),(0,1),(0,1)

	Or:
	+CNMI:(0-3),(0-3),(0-3),(0,1),(0,1)
	*/
	Priv->CNMIMode			= 0;
	Priv->CNMIProcedure		= 0;
	Priv->CNMIDeliverProcedure	= 0;
#ifdef GSM_ENABLE_CELLBROADCAST
	Priv->CNMIBroadcastProcedure	= 0;
#endif

	buffer = strchr(msg.Buffer, '\n');
	if (buffer == NULL) return  ERR_UNKNOWNRESPONSE;
	while (isspace((int)*buffer)) buffer++;

	if (strncmp(buffer, "+CNMI:", 6) != 0) return ERR_UNKNOWNRESPONSE;
	buffer += 7;

	buffer = strchr(buffer, '(');
	if (buffer == NULL) return  ERR_UNKNOWNRESPONSE;
	range = GetRange(buffer);
	if (range == NULL) return  ERR_UNKNOWNRESPONSE;
	if (InRange(range, 2)) Priv->CNMIMode = 2; 	/* 2 = buffer messages and send them when link is free */
	else if (InRange(range, 3)) Priv->CNMIMode = 3; /* 3 = send messages directly */
	else return ERR_NONE; /* we don't want: 1 = ignore new messages, 0 = store messages and no indication */
	free(range);

	buffer++;
	buffer = strchr(buffer, '(');
	if (buffer == NULL) return  ERR_UNKNOWNRESPONSE;
	range = GetRange(buffer);
	if (range == NULL) return  ERR_UNKNOWNRESPONSE;
	if (InRange(range, 1)) Priv->CNMIProcedure = 1; 	/* 1 = store message and send where it is stored */
	else if (InRange(range, 2)) Priv->CNMIProcedure = 2; 	/* 2 = route message to TE */
	else if (InRange(range, 3)) Priv->CNMIProcedure = 3; 	/* 3 = 1 + route class 3 to TE */
	/* we don't want: 0 = just store to memory */
	free(range);

	buffer++;
	buffer = strchr(buffer, '(');
#ifdef GSM_ENABLE_CELLBROADCAST
	if (buffer == NULL) return  ERR_UNKNOWNRESPONSE;
	range = GetRange(buffer);
	if (range == NULL) return  ERR_UNKNOWNRESPONSE;
	if (InRange(range, 2)) Priv->CNMIBroadcastProcedure = 2; /* 2 = route message to TE */
	else if (InRange(range, 1)) Priv->CNMIBroadcastProcedure = 1; /* 1 = store message and send where it is stored */
	else if (InRange(range, 3)) Priv->CNMIBroadcastProcedure = 3; /* 3 = 1 + route class 3 to TE */
	/* we don't want: 0 = just store to memory */
	free(range);
#endif

	buffer++;
	buffer = strchr(buffer, '(');
	if (buffer == NULL) return  ERR_UNKNOWNRESPONSE;
	range = GetRange(buffer);
	if (range == NULL) return  ERR_UNKNOWNRESPONSE;
	if (InRange(range, 1)) Priv->CNMIDeliverProcedure = 1; /* 1 = route message to TE */
	else if (InRange(range, 2)) Priv->CNMIDeliverProcedure = 2; /* 1 = store message and send where it is stored */
	/* we don't want: 0 = no routing */
	free(range);

	return ERR_NONE;

}

/**
 * Detects what additional protocols are being supported
 */
GSM_Error ATGEN_ReplyCheckProt(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	int line = 1;
	int protocol_id;
	char protocol_version[100];
	int protocol_level;
	const char *string;
	GSM_Error error;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Protocol entries received\n");
		/* Walk through lines with +CPROT: */
		while (strcmp("OK", string = GetLineString(msg.Buffer,&Priv->Lines,line+1)) != 0) {

			/*
			 * This is what Sony Ericsson phones usually
			 * give.
			 * +CPROT: (0),("1.2"),(8)
			 */
			error = ATGEN_ParseReply(s, string,
				"+CPROT: (@i), (@r), (@i)",
				&protocol_id,
				protocol_version, sizeof(protocol_version),
				&protocol_level);

			/*
			 * This reply comes from Alcatel and Samsung.
			 * +CPROT: 0,"1.0",8"
			 */
			if (error != ERR_NONE) {
				error = ATGEN_ParseReply(s, string,
					"+CPROT: @i, @r, @i",
					&protocol_id,
					protocol_version, sizeof(protocol_version),
					&protocol_level);
			}

			/* Check for OBEX */
			if (error == ERR_NONE && protocol_id == 0) {
				smprintf(s, "OBEX seems to be supported, version %s, level %d!\n", protocol_version, protocol_level);
#ifdef GSM_ENABLE_ATOBEX
				/* Tell OBEX driver that AT+CPROT=0 is supported */
				s->Phone.Data.Priv.ATOBEX.HasOBEX = ATOBEX_OBEX_CPROT0;
#endif
				/*
				 * Level 1 is almost useless, require
				 * higher levels.
				 */
				if (protocol_level > 1 &&
						strcmp(protocol_version, "1.2") == 0 &&
						strcmp(protocol_version, "1.3") == 0 &&
						!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NO_ATOBEX) &&
						!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_OBEX)
						) {
					/* As AT+OBEX has automatic fallback we can try to enable OBEX here. */
					smprintf(s, "Automatically enabling F_OBEX, please report bug if it causes problems\n");
					GSM_AddPhoneFeature(s->Phone.Data.ModelInfo, F_OBEX);
				}
			}
			/* Check for Alcatel protocol */
			if (error == ERR_NONE && protocol_id == 16) {
				if (!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_ALCATEL)) {
					smprintf(s, "HINT: Please consider adding F_ALCATEL to your phone capabilities in common/gsmphones.c\n");
				}
			}
			line++;
		}
		return ERR_NONE;
	case AT_Reply_Error:
		return ERR_UNKNOWN;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
	        return ATGEN_HandleCMEError(s);
	default:
		return ERR_UNKNOWNRESPONSE;
	}
}

GSM_Reply_Function ATGENReplyFunctions[] = {
{ATGEN_GenericReply,		"AT\r"			,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_GenericReply,		"ATE1" 	 		,0x00,0x00,ID_EnableEcho	 },
{ATGEN_GenericReply,		"ERROR" 	 		,0x00,0x00,ID_EnableEcho	 },
{ATGEN_GenericReply,		"AT+CMEE=" 		,0x00,0x00,ID_EnableErrorInfo	 },
{ATGEN_GenericReply,		"AT+CKPD="		,0x00,0x00,ID_PressKey		 },
{ATGEN_ReplyGetSIMIMSI,		"AT+CIMI" 	 	,0x00,0x00,ID_GetSIMIMSI	 },
{ATGEN_ReplyCheckProt,		"AT+CPROT=?" 	 	,0x00,0x00,ID_SetOBEX		 },
{ATGEN_GenericReply,		"AT+XLNK=?" 	 	,0x00,0x00,ID_SetOBEX		 },

{ATGEN_ReplyGetCNMIMode,	"AT+CNMI=?"		,0x00,0x00,ID_GetCNMIMode	 },
#ifdef GSM_ENABLE_CELLBROADCAST
{ATGEN_ReplyIncomingCB,		"+CBM:" 	 	,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_GenericReply,		"AT+CNMI"		,0x00,0x00,ID_SetIncomingCB	 },
#endif

{ATGEN_IncomingBattery,		"_OBS:"		 	,0x00,0x00,ID_IncomingFrame      },
{ATGEN_ReplyGetBatteryCharge,	"AT+CBC"		,0x00,0x00,ID_GetBatteryCharge	 },

{ATGEN_ReplyGetModel,		"AT+CGMM"		,0x00,0x00,ID_GetModel           },
{ATGEN_ReplyGetModel,		"ATI4"			,0x00,0x00,ID_GetModel           },
{ATGEN_ReplyGetManufacturer,	"AT+CGMI"		,0x00,0x00,ID_GetManufacturer	 },
{ATGEN_ReplyGetManufacturer,	"ATI3"			,0x00,0x00,ID_GetManufacturer	 },
{ATGEN_ReplyGetFirmware,	"AT+CGMR"		,0x00,0x00,ID_GetFirmware	 },
{ATGEN_ReplyGetFirmware,	"ATI5"			,0x00,0x00,ID_GetFirmware	 },
{ATGEN_ReplyGetIMEI,		"AT+CGSN"		,0x00,0x00,ID_GetIMEI	 	 },

{ATGEN_ReplySendSMS,		"AT+CMGS"		,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_ReplySendSMS,		"AT+CMSS"		,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_GenericReply,		"AT+CNMI"		,0x00,0x00,ID_SetIncomingSMS	 },
{ATGEN_GenericReply,		"AT+CMGF"		,0x00,0x00,ID_GetSMSMode	 },
{ATGEN_GenericReply,		"AT+CSDH"		,0x00,0x00,ID_GetSMSMode	 },
{ATGEN_ReplyGetSMSMessage,	"AT+CMGR"		,0x00,0x00,ID_GetSMSMessage	 },
{ATGEN_ReplyGetMessageList,	"AT+CMGL"		,0x00,0x00,ID_GetSMSMessage	 },
{ATGEN_GenericReply,		"AT+CPMS"		,0x00,0x00,ID_SetMemoryType	 },
{ATGEN_ReplyGetSMSStatus,	"AT+CPMS"		,0x00,0x00,ID_GetSMSStatus	 },
{ATGEN_ReplyGetSMSMemories,	"AT+CPMS=?"		,0x00,0x00,ID_GetSMSMemories	 },
{ATGEN_ReplyAddSMSMessage,	"AT+CMGW"		,0x00,0x00,ID_SaveSMSMessage	 },
{ATGEN_GenericReply,		"AT+CSMP"		,0x00,0x00,ID_SetSMSParameters	 },
{ATGEN_GenericReply,		"AT+CSCA"		,0x00,0x00,ID_SetSMSC		 },
{ATGEN_ReplyGetSMSC,		"AT+CSCA?"		,0x00,0x00,ID_GetSMSC		 },
{ATGEN_ReplyDeleteSMSMessage,	"AT+CMGD"		,0x00,0x00,ID_DeleteSMSMessage	 },
{ATGEN_GenericReply,		"ATE1"			,0x00,0x00,ID_SetSMSParameters	 },
{ATGEN_GenericReply,		"\x1b\x0D"		,0x00,0x00,ID_SetSMSParameters	 },
{ATGEN_GenericReply,		"AT+CMMS"		,0x00,0x00,ID_SetFastSMSSending  },
{ATGEN_IncomingSMSInfo,		"+CMTI:" 	 	,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_IncomingSMSDeliver,	"+CMT:" 	 	,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_IncomingSMSReport,	"+CDS:" 	 	,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_IncomingSMSCInfo,	"^SCN:"			,0x00,0x00,ID_IncomingFrame	 },

{ATGEN_ReplyGetDateTime,	"AT+CCLK?"		,0x00,0x00,ID_GetDateTime	 },
{ATGEN_GenericReply,		"AT+CCLK="		,0x00,0x00,ID_SetDateTime	 },
{ATGEN_GenericReply,		"AT+CALA="		,0x00,0x00,ID_SetAlarm		 },
{ATGEN_ReplyGetAlarm,		"AT+CALA?"		,0x00,0x00,ID_GetAlarm		 },

{ATGEN_ReplyGetNetworkLAC_CID,	"AT+CREG?"		,0x00,0x00,ID_GetNetworkInfo	 },
{ATGEN_GenericReply,		"AT+CREG=2"		,0x00,0x00,ID_GetNetworkInfo	 },
{ATGEN_GenericReply,		"AT+COPS="		,0x00,0x00,ID_GetNetworkInfo	 },
{ATGEN_GenericReply,		"AT+COPS="		,0x00,0x00,ID_SetAutoNetworkLogin},
{ATGEN_ReplyGetNetworkCode,	"AT+COPS"		,0x00,0x00,ID_GetNetworkCode	 },
{ATGEN_ReplyGetNetworkName,	"AT+COPS"		,0x00,0x00,ID_GetNetworkName	 },
{ATGEN_ReplyGetSignalQuality,	"AT+CSQ"		,0x00,0x00,ID_GetSignalQuality	 },
{ATGEN_IncomingNetworkLevel,	"_OSIGQ:"	 	,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_IncomingGPRS,		"+CGREG:"	 	,0x00,0x00,ID_IncomingFrame      },
{ATGEN_ReplyGetNetworkLAC_CID,	"+CREG:"		,0x00,0x00,ID_IncomingFrame	 },

{ATGEN_ReplyGetPBKMemories,	"AT+CPBS=?"		,0x00,0x00,ID_SetMemoryType	 },
{ATGEN_ReplySetPBKMemory,	"AT+CPBS="		,0x00,0x00,ID_SetMemoryType	 },
{ATGEN_ReplyGetCPBSMemoryStatus,"AT+CPBS?"		,0x00,0x00,ID_GetMemoryStatus	 },
/* Samsung phones reply +CPBR: after OK --claudio */
{ATGEN_ReplyGetCPBRMemoryInfo,	"AT+CPBR=?"		,0x00,0x00,ID_GetMemoryStatus	 },
{ATGEN_ReplyGetCPBRMemoryInfo,	"+CPBR:"		,0x00,0x00,ID_GetMemoryStatus	 },
{ATGEN_ReplyGetCPBRMemoryStatus,"AT+CPBR="		,0x00,0x00,ID_GetMemoryStatus	 },
{ATGEN_ReplyGetCharsets,	"AT+CSCS=?"		,0x00,0x00,ID_GetMemoryCharset	 },
{ATGEN_ReplyGetCharset,		"AT+CSCS?"		,0x00,0x00,ID_GetMemoryCharset	 },
{ATGEN_GenericReply,		"AT+CSCS="		,0x00,0x00,ID_SetMemoryCharset	 },
{ATGEN_ReplyGetMemory,		"AT+CPBR="		,0x00,0x00,ID_GetMemory		 },
{SIEMENS_ReplyGetMemoryInfo,	"AT^SBNR=?"		,0x00,0x00,ID_GetMemory		 },
{SAMSUNG_ReplyGetMemoryInfo,	"AT+SPBR=?"		,0x00,0x00,ID_GetMemory		 },
{MOTOROLA_ReplyGetMemoryInfo,	"AT+MPBR=?"		,0x00,0x00,ID_GetMemory		 },
{SIEMENS_ReplyGetMemory,	"AT^SBNR"		,0x00,0x00,ID_GetMemory		 },
{SAMSUNG_ReplyGetMemory,	"AT+SPBR"		,0x00,0x00,ID_GetMemory		 },
{MOTOROLA_ReplyGetMemory,	"AT+MPBR"		,0x00,0x00,ID_GetMemory		 },
{ATGEN_ReplySetMemory,		"AT+CPBW"		,0x00,0x00,ID_SetMemory		 },

{SIEMENS_ReplyGetBitmap,	"AT^SBNR=\"bmp\""	,0x00,0x00,ID_GetBitmap	 	 },
{SIEMENS_ReplySetBitmap,	"AT^SBNW=\"bmp\""	,0x00,0x00,ID_SetBitmap	 	 },

{SIEMENS_ReplyGetRingtone,	"AT^SBNR=\"mid\""	,0x00,0x00,ID_GetRingtone	 },
{SIEMENS_ReplySetRingtone,	"AT^SBNW=\"mid\""	,0x00,0x00,ID_SetRingtone	 },

{SIEMENS_ReplyGetNextCalendar,	"AT^SBNR=\"vcs\""	,0x00,0x00,ID_GetCalendarNote	 },
{SIEMENS_ReplyAddCalendarNote,	"AT^SBNW=\"vcs\""	,0x00,0x00,ID_SetCalendarNote	 },
{SIEMENS_ReplyDelCalendarNote,	"AT^SBNW=\"vcs\""	,0x00,0x00,ID_DeleteCalendarNote },

{ATGEN_ReplyEnterSecurityCode,	"AT+CPIN="		,0x00,0x00,ID_EnterSecurityCode	 },
{ATGEN_ReplyEnterSecurityCode,	"AT+CPIN2="		,0x00,0x00,ID_EnterSecurityCode	 },
{ATGEN_ReplyGetSecurityStatus,	"AT+CPIN?"		,0x00,0x00,ID_GetSecurityStatus	 },
{ATGEN_ReplyOK,			"OK"			,0x00,0x00,ID_IncomingFrame	 },

/* No need to take care about this, we just need to ignore it */
{MOTOROLA_Banner,		"+MBAN:"		,0x00,0x00,ID_IncomingFrame	 },

{ATGEN_GenericReply, 		"AT+VTS"		,0x00,0x00,ID_SendDTMF		 },
{ATGEN_ReplyCancelCall,		"AT+CHUP"		,0x00,0x00,ID_CancelCall	 },
{ATGEN_ReplyDialVoice,		"ATD"			,0x00,0x00,ID_DialVoice		 },
{ATGEN_ReplyCancelCall,		"ATH"			,0x00,0x00,ID_CancelCall	 },
{ATGEN_GenericReply, 		"AT+CRC"		,0x00,0x00,ID_SetIncomingCall	 },
{ATGEN_GenericReply, 		"AT+CLIP"		,0x00,0x00,ID_SetIncomingCall	 },
{ATGEN_GenericReply, 		"AT+CCWA"		,0x00,0x00,ID_SetIncomingCall	 },
{ATGEN_GenericReply, 		"AT+CUSD"		,0x00,0x00,ID_SetUSSD		 },
{ATGEN_ReplyGetUSSD, 		"AT+CUSD"		,0x00,0x00,ID_GetUSSD		 },
{ATGEN_ReplyGetUSSD, 		"+CUSD"			,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_GenericReply,            "AT+CLIP=1"      	,0x00,0x00,ID_IncomingFrame      },
{ATGEN_ReplyIncomingCallInfo,	"+CLIP"			,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_ReplyIncomingCallInfo,	"+CCWA"			,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_ReplyIncomingCallInfo,	"+COLP"    		,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_ReplyIncomingCallInfo,	"RING"			,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_ReplyIncomingCallInfo,	"+CRING"		,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_ReplyIncomingCallInfo,	"NO CARRIER"		,0x00,0x00,ID_IncomingFrame	 },

{ATGEN_GenericReply, 		"AT+MODE"		,0x00,0x00,ID_ModeSwitch	 },

{ATGEN_ReplyReset,		"AT^SRESET"		,0x00,0x00,ID_Reset		 },
{ATGEN_ReplyReset,		"AT+CFUN=1,1"		,0x00,0x00,ID_Reset		 },
{ATGEN_ReplyResetPhoneSettings, "AT&F"			,0x00,0x00,ID_ResetPhoneSettings },

{SAMSUNG_ReplyGetBitmap,	"AT+IMGR="		,0x00,0x00,ID_GetBitmap	 	 },
{SAMSUNG_ReplySetBitmap,	"SDNDCRC ="		,0x00,0x00,ID_SetBitmap		 },

{SAMSUNG_ReplyGetRingtone,	"AT+MELR="		,0x00,0x00,ID_GetRingtone	 },
{SAMSUNG_ReplySetRingtone,	"SDNDCRC ="		,0x00,0x00,ID_SetRingtone	 },

{ATGEN_GenericReplyIgnore, 	"^RSSI:"		,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_GenericReplyIgnore, 	"^BOOT:"		,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_GenericReplyIgnore, 	"^DSFLOWRPT:"		,0x00,0x00,ID_IncomingFrame	 },

#ifdef GSM_ENABLE_ATOBEX
{ATGEN_GenericReply,		"AT*EOBEX=?"		,0x00,0x00,ID_SetOBEX		 },
{ATGEN_GenericReply,		"AT*EOBEX"		,0x00,0x00,ID_SetOBEX		 },
{ATGEN_GenericReply,		"AT+CPROT=0" 	 	,0x00,0x00,ID_SetOBEX		 },
{ATGEN_GenericReply,		"AT+MODE=22" 	 	,0x00,0x00,ID_SetOBEX		 },
{ATGEN_GenericReply,		"AT+XLNK" 	 	,0x00,0x00,ID_SetOBEX		 },
{ATGEN_GenericReply,		"AT^SQWE=3" 	 	,0x00,0x00,ID_SetOBEX		 },
{ATGEN_GenericReply,		"AT^SQWE=0" 	 	,0x00,0x00,ID_SetOBEX		 },
{ATGEN_SQWEReply,		"AT^SQWE?" 	 	,0x00,0x00,ID_GetProtocol	 },

{ATGEN_GenericReply,		"AT*ESDF="		,0x00,0x00,ID_SetLocale		 },
{ATGEN_GenericReply,		"AT*ESTF="		,0x00,0x00,ID_SetLocale		 },

{ATOBEX_ReplyGetDateLocale,	"AT*ESDF?"	,0x00,0x00,ID_GetLocale		 },
{ATOBEX_ReplyGetTimeLocale,	"AT*ESTF?"	,0x00,0x00,ID_GetLocale	 	 },
{ATOBEX_ReplyGetFileSystemStatus,	"AT*EMEM"	,0x00,0x00,ID_FileSystemStatus 	 },
{ATGEN_GenericReply,			"AT*EBCA"	,0x00,0x00,ID_GetBatteryCharge 	 },
{ATOBEX_ReplyGetBatteryCharge,	"*EBCA:"	,0x00,0x00,ID_IncomingFrame	 },
#endif
#ifdef GSM_ENABLE_ALCATEL
/*  Why do I give Alcatel specific things here? It's simple, Alcatel needs
 *  some AT commands to start it's binary mode, so this needs to be in AT
 *  related stuff.
 *
 *  XXX: AT+IFC could later move outside this ifdef, because it is not Alcatel
 *  specific and it's part of ETSI specifications
 */
{ATGEN_GenericReply,		"AT+IFC" 	 	,0x00,0x00,ID_SetFlowControl  	 },
{ALCATEL_ProtocolVersionReply,	"AT+CPROT=?" 	 	,0x00,0x00,ID_AlcatelProtocol    },
{ATGEN_GenericReply,		"AT+CPROT=16" 	 	,0x00,0x00,ID_AlcatelConnect	 },
#endif

{NULL,				"\x00"			,0x00,0x00,ID_None		 }
};

GSM_Phone_Functions ATGENPhone = {
	"A2D|iPAQ|at|M20|S25|MC35|TC35|C35i|S65|S300|5110|5130|5190|5210|6110|6130|6150|6190|6210|6250|6310|6310i|6510|7110|8210|8250|8290|8310|8390|8850|8855|8890|8910|9110|9210",
	ATGENReplyFunctions,
	ATGEN_Initialise,
	ATGEN_Terminate,
	ATGEN_DispatchMessage,
	NOTSUPPORTED,			/* 	ShowStartInfo		*/
	ATGEN_GetManufacturer,
	ATGEN_GetModel,
	ATGEN_GetFirmware,
	ATGEN_GetIMEI,
	NOTSUPPORTED,			/*	GetOriginalIMEI		*/
	NOTSUPPORTED,			/*	GetManufactureMonth	*/
	NOTSUPPORTED,			/*	GetProductCode		*/
	NOTSUPPORTED,			/*	GetHardware		*/
	NOTSUPPORTED,			/*	GetPPM			*/
	ATGEN_GetSIMIMSI,
	ATGEN_GetDateTime,
	ATGEN_SetDateTime,
	ATGEN_GetAlarm,
	ATGEN_SetAlarm,
	NOTSUPPORTED,			/*	GetLocale		*/
	NOTSUPPORTED,			/*	SetLocale		*/
	ATGEN_PressKey,
	ATGEN_Reset,
	ATGEN_ResetPhoneSettings,
	ATGEN_EnterSecurityCode,
	ATGEN_GetSecurityStatus,
	ATGEN_GetDisplayStatus,
	ATGEN_SetAutoNetworkLogin,
	ATGEN_GetBatteryCharge,
	ATGEN_GetSignalQuality,
	ATGEN_GetNetworkInfo,
 	NOTSUPPORTED,       		/*  	GetCategory 		*/
 	NOTSUPPORTED,       		/*  	AddCategory 		*/
 	NOTSUPPORTED,        		/*  	GetCategoryStatus 	*/
	ATGEN_GetMemoryStatus,
	ATGEN_GetMemory,
	ATGEN_GetNextMemory,
	ATGEN_SetMemory,
	ATGEN_AddMemory,
	ATGEN_DeleteMemory,
	ATGEN_DeleteAllMemory,
	NOTSUPPORTED,			/*	GetSpeedDial		*/
	NOTSUPPORTED,			/*	SetSpeedDial		*/
	ATGEN_GetSMSC,
	ATGEN_SetSMSC,
	ATGEN_GetSMSStatus,
	ATGEN_GetSMS,
	ATGEN_GetNextSMS,
	NOTSUPPORTED,			/*	SetSMS			*/
	ATGEN_AddSMS,
	ATGEN_DeleteSMS,
	ATGEN_SendSMS,
	ATGEN_SendSavedSMS,
	ATGEN_SetFastSMSSending,
	ATGEN_SetIncomingSMS,
	ATGEN_SetIncomingCB,
	ATGEN_GetSMSFolders,
 	NOTSUPPORTED,			/* 	AddSMSFolder		*/
 	NOTSUPPORTED,			/* 	DeleteSMSFolder		*/
	ATGEN_DialVoice,
	ATGEN_DialService,
	ATGEN_AnswerCall,
	ATGEN_CancelCall,
 	NOTSUPPORTED,			/* 	HoldCall 		*/
 	NOTSUPPORTED,			/* 	UnholdCall 		*/
 	NOTSUPPORTED,			/* 	ConferenceCall 		*/
 	NOTSUPPORTED,			/* 	SplitCall		*/
 	NOTSUPPORTED,			/* 	TransferCall		*/
 	NOTSUPPORTED,			/* 	SwitchCall		*/
 	NOTSUPPORTED,			/* 	GetCallDivert		*/
 	NOTSUPPORTED,			/* 	SetCallDivert		*/
 	NOTSUPPORTED,			/* 	CancelAllDiverts	*/
	ATGEN_SetIncomingCall,
	ATGEN_SetIncomingUSSD,
	ATGEN_SendDTMF,
	ATGEN_GetRingtone,
	ATGEN_SetRingtone,
	NOTSUPPORTED,			/*	GetRingtonesInfo	*/
	NOTSUPPORTED,			/* 	DeleteUserRingtones	*/
	NOTSUPPORTED,			/*	PlayTone		*/
	NOTSUPPORTED,			/* 	GetWAPBookmark		*/
	NOTSUPPORTED,			/* 	SetWAPBookmark 		*/
	NOTSUPPORTED,	 		/* 	DeleteWAPBookmark 	*/
	NOTSUPPORTED,			/* 	GetWAPSettings 		*/
	NOTSUPPORTED,			/* 	SetWAPSettings 		*/
	NOTSUPPORTED,			/*	GetSyncMLSettings	*/
	NOTSUPPORTED,			/*	SetSyncMLSettings	*/
	NOTSUPPORTED,			/*	GetChatSettings		*/
	NOTSUPPORTED,			/*	SetChatSettings		*/
	NOTSUPPORTED,			/* 	GetMMSSettings		*/
	NOTSUPPORTED,			/* 	SetMMSSettings		*/
	NOTSUPPORTED,			/*	GetMMSFolders		*/
	NOTSUPPORTED,			/*	GetNextMMSFileInfo	*/
	ATGEN_GetBitmap,		/* 	GetBitmap		*/
	ATGEN_SetBitmap,		/*	SetBitmap		*/
	NOTSUPPORTED,			/*	GetToDoStatus		*/
	NOTSUPPORTED,			/*	GetToDo			*/
	NOTSUPPORTED,			/*	GetNextToDo		*/
	NOTSUPPORTED,			/*	SetToDo			*/
	NOTSUPPORTED,			/*	AddToDo			*/
	NOTSUPPORTED,			/*	DeleteToDo		*/
	NOTSUPPORTED,			/*	DeleteAllToDo		*/
	NOTSUPPORTED,			/*	GetCalendarStatus	*/
	ATGEN_GetCalendar,
	ATGEN_GetNextCalendar,
	ATGEN_SetCalendarNote,
	ATGEN_AddCalendarNote,
	ATGEN_DelCalendarNote,
	NOTIMPLEMENTED,			/*	DeleteAllCalendar	*/
	NOTSUPPORTED,			/* 	GetCalendarSettings	*/
	NOTSUPPORTED,			/* 	SetCalendarSettings	*/
	NOTSUPPORTED,			/*	GetNoteStatus		*/
	NOTSUPPORTED,			/*	GetNote			*/
	NOTSUPPORTED,			/*	GetNextNote		*/
	NOTSUPPORTED,			/*	SetNote			*/
	NOTSUPPORTED,			/*	AddNote			*/
	NOTSUPPORTED,			/* 	DeleteNote		*/
	NOTSUPPORTED,			/*	DeleteAllNotes		*/
	NOTSUPPORTED, 			/*	GetProfile		*/
	NOTSUPPORTED, 			/*	SetProfile		*/
    	NOTSUPPORTED,			/*  	GetFMStation        	*/
    	NOTSUPPORTED,			/* 	SetFMStation        	*/
    	NOTSUPPORTED,			/* 	ClearFMStations       	*/
	NOTSUPPORTED,			/* 	GetNextFileFolder	*/
	NOTSUPPORTED,			/*	GetFolderListing	*/
	NOTSUPPORTED,			/*	GetNextRootFolder	*/
	NOTSUPPORTED,			/*	SetFileAttributes	*/
	NOTSUPPORTED,			/* 	GetFilePart		*/
	NOTSUPPORTED,			/* 	AddFilePart		*/
	NOTSUPPORTED,			/* 	SendFilePart		*/
	NOTSUPPORTED, 			/* 	GetFileSystemStatus	*/
	NOTSUPPORTED,			/* 	DeleteFile		*/
	NOTSUPPORTED,			/* 	AddFolder		*/
	NOTSUPPORTED,			/* 	DeleteFolder		*/
	NOTSUPPORTED,			/* 	GetGPRSAccessPoint	*/
	NOTSUPPORTED			/* 	SetGPRSAccessPoint	*/
};

#endif
/*@}*/
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

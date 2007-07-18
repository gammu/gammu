/* (c) 2002-2005 by Marcin Wiacek and Michal Cihar */

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
#include "../pfunc.h"

#include "atgen.h"
#include "atfunc.h"

#include "samsung.h"
#include "siemens.h"
#include "motorola.h"

#ifdef GSM_ENABLE_ALCATEL
GSM_Error ALCATEL_ProtocolVersionReply (GSM_Protocol_Message, GSM_StateMachine *);
#endif

#ifdef GSM_ENABLE_SONYERICSSON
#include "../sonyeric/sefunc.h"
#endif


typedef struct {
	GSM_AT_Charset	charset;
	char		*text;
	bool		unicode;
} GSM_AT_Charset_Info;

/**
 * List of charsets and text identifying them in phone responses, order
 * defines their preferences, so if first is found it is used.
 */
static GSM_AT_Charset_Info AT_Charsets[] = {
	{AT_CHARSET_HEX,	"HEX",		false},
	{AT_CHARSET_GSM,	"GSM",		false},
	{AT_CHARSET_PCCP437,	"PCCP437",	false},
	{AT_CHARSET_UTF8,	"UTF-8",	true},
	{AT_CHARSET_UCS2,	"UCS2",		true},
	{AT_CHARSET_IRA,	"IRA",		false},
#ifdef ICONV_FOUND
	{AT_CHARSET_ISO88591,	"8859-1",	false},
	{AT_CHARSET_ISO88592,	"8859-2",	false},
	{AT_CHARSET_ISO88593,	"8859-3",	false},
	{AT_CHARSET_ISO88594,	"8859-4",	false},
	{AT_CHARSET_ISO88595,	"8859-5",	false},
	{AT_CHARSET_ISO88596,	"8859-6",	false},
#endif
	{0,			NULL, false}
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
	{100, "unknown"},
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
	case 304:
            	return ERR_NOTSUPPORTED;
        case 305:
            	return ERR_BUG;
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
inline bool ATGEN_HasOnlyHexChars(const char *text, const size_t length)
{
	size_t i;

	for (i = 0; i < length; i++) {
		if (!isxdigit(text[i])) return false;
	}

	return true;
}

/**
 * Detects whether given text can be UCS2.
 *
 * \param s State machine structure.
 * \param len Length of string.
 * \param text Text.
 * \return True when text can be UCS2.
 */
inline bool ATGEN_IsUCS2(const char *text, const size_t length)
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
inline bool ATGEN_IsHex(const char *text, const size_t length)
{
	return (length > 4) &&
		(length % 2 == 0) &&
		ATGEN_HasOnlyHexChars(text, length);
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
		const bool guess,
		const bool phone)
{
	unsigned char *buffer;
	GSM_AT_Charset charset;

	/* Default to charset from state machine */
	charset = s->Phone.Data.Priv.ATGEN.Charset;

	/* Can we do guesses? */
	if (guess) {
		if  (charset == AT_CHARSET_HEX
			&& ! ATGEN_IsHex(input, length)) {
			charset = AT_CHARSET_GSM;
		}
		/* For phone numbers, we can assume all unicode chars
		 * will be < 256, so they will fit one byte */
		if  (charset == AT_CHARSET_UCS2
			&& ! ATGEN_IsUCS2(input, length) && (
				!phone ||
				input[0] != '0' ||
				input[1] != '0' ||
				input[4] != '0' ||
				input[5] != '0'
				)) {
			charset = AT_CHARSET_GSM;
		}
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
   			DecodeDefault(output, buffer, strlen(buffer), false, NULL);
			free(buffer);
  			break;
  		case AT_CHARSET_GSM:
  			DecodeDefault(output, input, length, false, NULL);
  			break;
  		case AT_CHARSET_UCS2:
 			DecodeHexUnicode(output, input, length);
  			break;
  		case AT_CHARSET_IRA: /* IRA is ASCII only, so it's safe to treat is as UTF-8 */
  		case AT_CHARSET_UTF8:
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
  			DecodeDefault(output, input, length, false, NULL);
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
	bool	inside_quotes = false;

	while ((*input!=',' || inside_quotes) && *input!=0x0d && *input!=0x00) {
		if (*input == '"') inside_quotes = ! inside_quotes;
		*output=*input;
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
	bool inside_quotes = false;

	/* Allocate initial buffer in case string is empty */
	*output = (unsigned char *)malloc(size);
	if (*output == NULL) {
		smprintf(s, "Ran out of memory!\n");
		return 0;
	}

	while (((*input != ',' && *input != ')') || inside_quotes)
			&& *input != 0x0d
			&& *input != 0x0a
			&& *input != 0x00) {
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
		(*output)[position]=*input;
		position++;
		input	++;
	}

	(*output)[position] = 0;

	/* Strip quotes */
	if ((*output)[0] == '"' && (*output)[position - 1]) {
		memmove(*output, (*output) + 1, position - 2);
		(*output)[position - 2] = 0;
	}

	smprintf(s, "Grabbed string from reply: \"%s\" (parsed %zd bytes)\n", *output, position);
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
			true, false);
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
		dt->Timezone = (*pos == '+' ? 1 : -1);
		dt->Timezone = dt->Timezone*atoi(pos);
	} else {
		dt->Timezone = 0;
	}
	return ERR_NONE;
}

/**
 * Parses AT formatted reply.
 *
 * Format strings:
 * @i - number, expects pointer to int
 * @l - number, expects pointer to long int
 * @s - string, will be converted from phone encoding, stripping quotes, expects pointer to unsigned char and size of storage
 * @S - string with Samsung specials (0x02 at beginning and 0x03 at the end), otherwise same as @s
 * @p - phone number hint for heuristics, otherwise same as @s
 * @r - raw string, no conversion will be done, only stripped quotes, expects pointer to char and size of storage
 * @d - date, expects pointer to GSM_DateTime
 * @@ - @ literal
 * @0 - ignore rest of input
 *
 * Special behaviour:
 * Any space is automatically treated as [[:space:]]* regexp.
 */
GSM_Error ATGEN_ParseReply(GSM_StateMachine *s, const unsigned char *input, const char *format, ...)
{
	const char *fmt = format;
	const char *inp = input;
	char *endptr;
	GSM_DateTime *out_dt;
	char *out_s;
	unsigned char *out_us;
	unsigned char *buffer;
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
						smprintf(s, "Parsed int %d\n", *out_i);
						if (endptr == inp) {
							error = ERR_UNKNOWNRESPONSE;
							goto end;
						}
						inp = endptr;
						break;
					case 'l':
						out_l = va_arg(ap, long int *);
						*out_l = strtol(inp, &endptr, 10);
						smprintf(s, "Parsed long int %ld\n", *out_l);
						if (endptr == inp) {
							error = ERR_UNKNOWNRESPONSE;
							goto end;
						}
						inp = endptr;
						break;
					case 'p':
						smprintf(s, "Should parse phone\n");
						out_s = va_arg(ap, char *);
						storage_size = va_arg(ap, size_t);
						length = ATGEN_GrabString(s, inp, &buffer);
						error = ATGEN_DecodeText(s,
								buffer, strlen(buffer),
								out_s, storage_size,
								true, true);
						free(buffer);
						if (error != ERR_NONE) {
							goto end;
						}
						inp += length;
						break;
					case 's':
						smprintf(s, "Should parse string\n");
						out_s = va_arg(ap, char *);
						storage_size = va_arg(ap, size_t);
						length = ATGEN_GrabString(s, inp, &buffer);
						error = ATGEN_DecodeText(s,
								buffer, strlen(buffer),
								out_s, storage_size,
								true, false);
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
						error = ATGEN_DecodeText(s,
								buffer, strlen(buffer),
								out_s, storage_size,
								true, false);
						free(buffer);
						if (error != ERR_NONE) {
							goto end;
						}
						inp += length;
						break;
					case 'r':
						smprintf(s, "Should parse raw string\n");
						out_us = va_arg(ap, unsigned char *);
						storage_size = va_arg(ap, size_t);
						length = ATGEN_GrabString(s, inp, &buffer);
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
						smprintf(s, "Should parse date\n");
						out_dt = va_arg(ap, GSM_DateTime *);
						length = ATGEN_GrabString(s, inp, &buffer);
						error = ATGEN_DecodeDateTime(s, out_dt, buffer);
						free(buffer);
						if (error != ERR_NONE) {
							goto end;
						}
						inp += length;
						break;
					case '@':
						smprintf(s, "Should see @ [%c]\n", *inp);
						if (*inp++ != '@') {
							error = ERR_UNKNOWNRESPONSE;
							goto end;
						}
						break;
					case '0':
						/* Just skip the rest */
						goto end;
						break;
					default:
						smprintf(s, "Invalid format string (@%c): %s\n", *(fmt - 1), format);
						error = ERR_BUG;
						goto end;
				}
				break;
			case ' ':
				smprintf(s, "Should skip spaces\n");
				while (isspace(*inp)) inp++;
				break;
			default:
				smprintf(s, "Should see %c [%c]\n", *(fmt - 1), *inp);
				if (*inp++ != *(fmt - 1)) {
					error = ERR_UNKNOWNRESPONSE;
					goto end;
				}
				break;
		}
	}

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
	char                    *err, *line;
	ATErrorCode		*ErrorCodes = NULL;

	SplitLines(msg->Buffer, msg->Length, &Priv->Lines, "\x0D\x0A", 2, true);

	/* Find number of lines */
	while (Priv->Lines.numbers[i*2+1] != 0) {
		/* FIXME: handle special chars correctly */
		smprintf(s, "%i \"%s\"\n",i+1,GetLineString(msg->Buffer,Priv->Lines,i+1));
		i++;
	}

	Priv->ReplyState 	= AT_Reply_Unknown;
	Priv->ErrorText     	= NULL;
	Priv->ErrorCode     	= 0;

	line = GetLineString(msg->Buffer,Priv->Lines,i);
	if (!strcmp(line,"OK"))		Priv->ReplyState = AT_Reply_OK;
	if (!strcmp(line,"> "))		Priv->ReplyState = AT_Reply_SMSEdit;
	if (!strcmp(line,"CONNECT"))	Priv->ReplyState = AT_Reply_Connect;
	if (!strcmp(line,"ERROR"  ))	Priv->ReplyState = AT_Reply_Error;
	if (!strncmp(line,"+CME ERROR:",11)) {
		Priv->ReplyState = AT_Reply_CMEError;
		ErrorCodes = CMEErrorCodes;
	}
	if (!strncmp(line,"+CMS ERROR:",11)) {
		Priv->ReplyState = AT_Reply_CMSError;
		ErrorCodes = CMSErrorCodes;
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
		err = line + 12;
		while (err[j] && !isalnum(err[j])) j++;
		if (isdigit(err[j])) {
			Priv->ErrorCode = atoi(&(err[j]));
			k = 0;
			while (ErrorCodes[k].Number != -1) {
				if (ErrorCodes[k].Number == Priv->ErrorCode) {
					Priv->ErrorText = ErrorCodes[k].Text;
					break;
				}
				k++;
			}
		} else if (isalpha(err[j])) {
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

GSM_Error ATGEN_ReplyGetUSSD(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_USSDMessage ussd;
	GSM_Error error;
	unsigned char *pos;

	/*
	 * Reply format:
	 * +CUSD: 2,"...",15
	 */
	smprintf(s, "Incoming USSD received\n");

	if (s->Phone.Data.EnableIncomingUSSD) {
		/* Find start of reply */
		pos = strstr(msg.Buffer, "+CUSD:");
		if (pos == NULL) return ERR_UNKNOWN;

		/* Parse reply */
		error = ATGEN_ParseReply(s, pos,
				"+CUSD: @i, @s @0",
				&ussd.Status,
				ussd.Text, sizeof(ussd.Text));

		if (error != ERR_NONE) return error;

		/* Decode status */
		smprintf(s, "Status: %d\n", ussd.Status);
		switch(ussd.Status) {
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

		/* Notify application */
		if (s->User.IncomingUSSD!=NULL) {
			s->User.IncomingUSSD(s, ussd);
		}
	}

	return ERR_NONE;
}

GSM_Error ATGEN_SetIncomingUSSD(GSM_StateMachine *s, bool enable)
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
	char			*pos, *pos2;
	char			*line;

	if (s->Phone.Data.Priv.ATGEN.ReplyState != AT_Reply_OK) return ERR_NOTSUPPORTED;

	line = GetLineString(msg.Buffer, Priv->Lines, 2);
	pos = line;

	/*
	 * Motorola returns something like:
	 * "+CGMM: "GSM900","GSM1800","GSM1900","GSM850","MODEL=V3""
	 */
	if ((pos2 = strstr(line, "\"MODEL=")) != NULL) {
		pos = pos2 + 7; /* Skip above string */
		pos2 = strchr(pos, '"'); /* Find end quote */
		if (pos2 != NULL) {
			/* Terminate string at the end, otherwise we keep it full */
			*pos2 = 0;
		}
	/* Sometimes phone adds this before manufacturer (Sagem) */
	} else if (strncmp("+CGMM: ", Data->Model, 7) == 0) {
		pos += 7; /* Skip above string */
	}

	/* Now store string if it fits */
	if (strlen(pos) <= GSM_MAX_MODEL_LENGTH) {
		strcpy(Data->Model, pos);

		Data->ModelInfo = GetModelData(NULL,Data->Model,NULL);
		if (Data->ModelInfo->number[0] == 0) Data->ModelInfo = GetModelData(NULL,NULL,Data->Model);
		if (Data->ModelInfo->number[0] == 0) Data->ModelInfo = GetModelData(Data->Model,NULL,NULL);

		if (Data->ModelInfo->number[0] != 0) strcpy(Data->Model,Data->ModelInfo->number);
	} else {
		smprintf(s, "WARNING: Model name too long, increase GSM_MAX_MODEL_LENGTH to at least %zd\n", strlen(pos));
	}

	return ERR_NONE;
}

GSM_Error ATGEN_GetModel(GSM_StateMachine *s)
{
	GSM_Error error;

	if (s->Phone.Data.Model[0] != 0) return ERR_NONE;

	smprintf(s, "Getting model\n");
	ATGEN_WaitFor(s, "AT+CGMM\r", 8, 0x00, 3, ID_GetModel);
	if (error==ERR_NONE) {
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL ||
		    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE) {
			smprintf(s, "[Connected model  - \"%s\"]\n",s->Phone.Data.Model);
		}
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
		if (GetLineLength(msg.Buffer, Priv->Lines, 2) <= GSM_MAX_MANUFACTURER_LENGTH) {
			CopyLineString(s->Phone.Data.Manufacturer, msg.Buffer, Priv->Lines, 2);
		} else {
			smprintf(s, "WARNING: Manufacturer name too long, increase GSM_MAX_MANUFACTURER_LENGTH to at least %d\n", GetLineLength(msg.Buffer, Priv->Lines, 2));
			s->Phone.Data.Manufacturer[0] = 0;
		}
		/* Sometimes phone adds this before manufacturer (Sagem) */
		if (strncmp("+CGMI: ", s->Phone.Data.Manufacturer, 7) == 0) {
			memmove(s->Phone.Data.Manufacturer, s->Phone.Data.Manufacturer + 7, strlen(s->Phone.Data.Manufacturer + 7) + 1);
		}
		if (strstr(msg.Buffer,"Falcom")) {
			smprintf(s, "Falcom\n");
			strcpy(s->Phone.Data.Manufacturer,"Falcom");
			Priv->Manufacturer = AT_Falcom;
			if (strstr(msg.Buffer,"A2D")) {
				strcpy(s->Phone.Data.Model,"A2D");
				s->Phone.Data.ModelInfo = GetModelData(NULL,s->Phone.Data.Model,NULL);
				smprintf(s, "Model A2D\n");
			}
		}
		if (strstr(msg.Buffer,"Nokia")) {
			smprintf(s, "Nokia\n");
			strcpy(s->Phone.Data.Manufacturer,"Nokia");
			smprintf(s, "HINT: Consider using Nokia specific protocol instead of generic AT.\n");
			Priv->Manufacturer = AT_Nokia;
		}
		if (strstr(msg.Buffer,"SIEMENS")) {
			smprintf(s, "Siemens\n");
			strcpy(s->Phone.Data.Manufacturer,"Siemens");
			Priv->Manufacturer = AT_Siemens;
		}
		if (strstr(msg.Buffer,"SHARP")) {
			smprintf(s, "Sharp\n");
			strcpy(s->Phone.Data.Manufacturer,"Sharp");
			Priv->Manufacturer = AT_Sharp;
		}
		if (strstr(msg.Buffer,"ERICSSON")) {
			smprintf(s, "Ericsson\n");
			strcpy(s->Phone.Data.Manufacturer,"Ericsson");
			Priv->Manufacturer = AT_Ericsson;
		}
		if (strstr(msg.Buffer,"Sony Ericsson")) {
			smprintf(s, "Sony Ericsson\n");
			strcpy(s->Phone.Data.Manufacturer,"Sony Ericsson");
			Priv->Manufacturer = AT_Ericsson;
		}
		if (strstr(msg.Buffer,"iPAQ")) {
			smprintf(s, "iPAQ\n");
			strcpy(s->Phone.Data.Manufacturer,"HP");
			Priv->Manufacturer = AT_HP;
		}
		if (strstr(msg.Buffer,"ALCATEL")) {
			smprintf(s, "Alcatel\n");
			strcpy(s->Phone.Data.Manufacturer,"Alcatel");
			Priv->Manufacturer = AT_Alcatel;
		}
		if (strstr(msg.Buffer,"SAGEM")) {
			smprintf(s, "Sagem\n");
			strcpy(s->Phone.Data.Manufacturer,"Sagem");
			Priv->Manufacturer = AT_Sagem;
		}
		if (strstr(msg.Buffer,"Samsung")) {
			smprintf(s, "Samsung\n");
			strcpy(s->Phone.Data.Manufacturer,"Samsung");
			Priv->Manufacturer = AT_Samsung;
		}
		if (strstr(msg.Buffer,"SAMSUNG")) {
			smprintf(s, "Samsung\n");
			strcpy(s->Phone.Data.Manufacturer,"Samsung");
			Priv->Manufacturer = AT_Samsung;
		}
		if (strstr(msg.Buffer,"Mitsubishi")) {
			smprintf(s, "Mitsubishi\n");
			strcpy(s->Phone.Data.Manufacturer,"Mitsubishi");
			Priv->Manufacturer = AT_Mitsubishi;
		}
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

GSM_Error ATGEN_GetManufacturer(GSM_StateMachine *s)
{
	GSM_Error error;

	if (s->Phone.Data.Manufacturer[0] != 0) return ERR_NONE;

	ATGEN_WaitFor(s, "AT+CGMI\r", 8, 0x00, 4, ID_GetManufacturer);
	return error;
}

GSM_Error ATGEN_ReplyGetFirmwareCGMR(GSM_Protocol_Message msg, GSM_StateMachine *s)
{

	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;

	strcpy(s->Phone.Data.Version, "Unknown");
	s->Phone.Data.VerNum = 0;
	if (Priv->ReplyState == AT_Reply_OK) {
		if (GetLineLength(msg.Buffer, Priv->Lines, 2) > GSM_MAX_VERSION_LENGTH - 1) {
			smprintf(s, "Please increase GSM_MAX_VERSION_LENGTH!\n");
			return ERR_MOREMEMORY;
		}
		CopyLineString(s->Phone.Data.Version, msg.Buffer, Priv->Lines, 2);
		/* Sometimes phone adds this before version (Sagem) */
		if (strncmp("+CGMR: ", s->Phone.Data.Version, 7) == 0) {
			/* Need to use memmove as strcpy does not correctly handle overlapping regions */
			memmove(s->Phone.Data.Version, s->Phone.Data.Version + 7, strlen(s->Phone.Data.Version + 7) + 1);
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
	ATGEN_WaitFor(s, "AT+CGMR\r", 8, 0x00, 3, ID_GetFirmware);

	if (error==ERR_NONE) {
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL ||
		    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE) {
			smprintf(s, "[Firmware version - \"%s\"]\n",s->Phone.Data.Version);
		}
	}
	return error;
}

GSM_Error ATGEN_Initialise(GSM_StateMachine *s)
{
	GSM_Phone_ATGENData     *Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error               error;
    	char                    buff[2];

	Priv->SMSMode			= 0;
	Priv->Manufacturer		= 0;
	Priv->PhoneSMSMemory		= 0;
	Priv->PhoneSaveSMS		= 0;
	Priv->SIMSaveSMS		= 0;
	Priv->SIMSMSMemory		= 0;
	Priv->SMSMemory			= 0;
	Priv->PBKMemory			= 0;
	Priv->PBKSBNR			= 0;
	Priv->Charset			= 0;
	Priv->EncodedCommands		= false;
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
	Priv->Mode			= false;
	Priv->MemorySize		= 0;
	Priv->TextLength		= 0;
	Priv->NumberLength		= 0;

	Priv->CNMIMode			= -1;
	Priv->CNMIProcedure		= -1;
	Priv->CNMIDeliverProcedure	= -1;
#ifdef GSM_ENABLE_CELLBROADCAST
	Priv->CNMIBroadcastProcedure	= -1;
#endif

	Priv->ErrorText			= NULL;

	if (s->ConnectionType != GCT_IRDAAT && s->ConnectionType != GCT_BLUEAT) {
		/* We try to escape AT+CMGS mode, at least Siemens M20
		 * then needs to get some rest
		 */
		smprintf(s, "Escaping SMS mode\n");
		error = s->Protocol.Functions->WriteMessage(s, "\x1B\r", 2, 0x00);
		if (error!=ERR_NONE) return error;

	    	/* Grab any possible garbage */
	    	while (s->Device.Functions->ReadDevice(s, buff, 2) > 0) my_sleep(10);
	}

    	/* When some phones (Alcatel BE5) is first time connected, it needs extra
     	 * time to react, sending just AT wakes up the phone and it then can react
     	 * to ATE1. We don't need to check whether this fails as it is just to
     	 * wake up the phone and does nothing.
     	 */
    	smprintf(s, "Sending simple AT command to wake up some devices\n");
	GSM_WaitFor(s, "AT\r", 3, 0x00, 2, ID_IncomingFrame);

	/* We want to see our commands to allow easy detection of reply functions */
	smprintf(s, "Enabling echo\n");
	error = GSM_WaitFor(s, "ATE1\r", 5, 0x00, 3, ID_EnableEcho);

	/* Some modems (Sony Ericsson GC 79, GC 85) need to enable functionality
	 * (with reset), otherwise they return ERROR on anything!
	 */
	if (error == ERR_UNKNOWN) {
		GSM_WaitFor(s, "AT+CFUN=1,1\r", 12, 0x00, 3, ID_Reset);
		if (error != ERR_NONE) return error;
		GSM_WaitFor(s, "ATE1\r", 5, 0x00, 3, ID_EnableEcho);
	}
	if (error != ERR_NONE) return error;

	/* Try whether phone supports mode switching as Motorola phones. */
	smprintf(s, "Trying Motorola mode switch\n");
	if (GSM_WaitFor(s, "AT+MODE=2\r", 10, 0x00, 3, ID_ModeSwitch) != ERR_NONE) {
		smprintf(s, "Seems not to be supported\n");
		Priv->Mode = false;
	} else {
		smprintf(s, "Works, will use it\n");
		Priv->Mode = true;
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
	if (error != ERR_NONE) return error;

	/* Get model, it is useful to know it now */
	error = ATGEN_GetModel(s);
	if (error != ERR_NONE) return error;

	/* Mode switching cabaple phones can switch using AT+MODE */
	if (!Priv->Mode) {
		smprintf(s, "Checking for OBEX support\n");
		/* We don't care about error here */
		ATGEN_WaitFor(s, "AT+CPROT=?\r", 11, 0x00, 3, ID_SetOBEX);
		error = ERR_NONE;
	}

	if (!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SLOWWRITE)) {
		s->Protocol.Data.AT.FastWrite = true;
	}

	return error;
}

GSM_Error ATGEN_ReplyGetCharset(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	/* Reply we get here:
		AT+CSCS?
		+CSCS: "GSM"

		OK
	 */
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	char			*line;
	int			i = 0;

	switch (Priv->ReplyState) {
		case AT_Reply_OK:
			/* Can not use ATGEN_ParseReply safely here as we do not know charset yet */
			line = GetLineString(msg.Buffer, Priv->Lines, 2);
			/* First current charset: */
			while (AT_Charsets[i].charset != 0) {
				if (strstr(line, AT_Charsets[i].text) != NULL) {
					Priv->Charset = AT_Charsets[i].charset;
					break;
				}
				/* We detect encoded UCS2 reply here so that we can handle encoding of values later. */
				if (strstr(line, "0055004300530032") != NULL) {
					Priv->Charset = AT_CHARSET_UCS2;
					Priv->EncodedCommands = true;
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
	char			*line;
	int			i = 0;

	switch (Priv->ReplyState) {
		case AT_Reply_OK:
			line = GetLineString(msg.Buffer, Priv->Lines, 2);
			/* First find good charset for non-unicode: */
			while (AT_Charsets[i].charset != 0) {
				if (strstr(line, AT_Charsets[i].text) != NULL) {
					Priv->NormalCharset = AT_Charsets[i].charset;
					Priv->IRACharset = AT_Charsets[i].charset;
					break;
				}
				i++;
			}
			/* Use IRA charset if we support it */
			if (strstr(line, "IRA") != NULL) {
				Priv->IRACharset = AT_CHARSET_IRA;
			}
			if (Priv->NormalCharset == 0) {
				smprintf(s, "Could not find supported charset in list returned by phone!\n");
				return ERR_UNKNOWNRESPONSE;
			}
			/* Then find good charset for unicode: */
			Priv->UnicodeCharset = 0;
			while (AT_Charsets[i].charset != 0) {
				if (AT_Charsets[i].unicode && (strstr(line, AT_Charsets[i].text) != NULL)) {
					if (AT_Charsets[i].charset != AT_CHARSET_UCS2 ||
							!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NO_UCS2)) {
						Priv->UnicodeCharset = AT_Charsets[i].charset;
						break;
					}
				}
				i++;
			}
			/* Fallback for unicode charset */
			if (Priv->UnicodeCharset == 0) {
				Priv->UnicodeCharset = Priv->NormalCharset;
			}
			/* If we have unicode charset, it's better than GSM for IRA */
			if (Priv->IRACharset == AT_CHARSET_GSM) {
				Priv->IRACharset = Priv->UnicodeCharset;
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
		cset = Priv->IRACharset;
	} else {
		return ERR_BUG;
	}

	/* If we already have set our preffered charset there is nothing to do*/
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
		smprintf(s, "Could not find string representation for charset!\n");
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

GSM_Error ATGEN_SetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	unsigned char req[50];
	GSM_Error error;

	if (smsc->Location!=1) return ERR_INVALIDLOCATION;

	sprintf(req, "AT+CSCA=\"%s\"\r",DecodeUnicodeString(smsc->Number));

	smprintf(s, "Setting SMSC\n");
	ATGEN_WaitFor(s, req, strlen(req), 0x00, 4, ID_SetSMSC);

	return error;
}

GSM_Error ATGEN_ReplyGetSMSMemories(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	char *pos_start, *pos_end, *pos_tmp;
	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		/* Reply here is:
		 * (memories for reading)[, (memories for writing)[, (memories for storing received messages)]]
		 * each memory is in quotes,
		 * Example: ("SM"), ("SM"), ("SM")
		 *
		 * We need to get from this supported memories. For this case
		 * we assume, that just appearence of memory makes it
		 * available for everything. Then we need to find out whether
		 * phone supports writing to memory. This is done by searching
		 * for "), (", which will appear between lists.
		 */
		s->Phone.Data.Priv.ATGEN.PhoneSaveSMS = AT_NOTAVAILABLE;
		s->Phone.Data.Priv.ATGEN.SIMSaveSMS = AT_NOTAVAILABLE;

		pos_start = strstr(msg.Buffer, "), (");
		if (pos_start == NULL) pos_start = strstr(msg.Buffer, "),(");
		if (pos_start != NULL) {
			/* Detect which memories we can use for saving */
			pos_end = strchrnul(pos_start + 1, ')');
			pos_tmp = strstr(pos_start, "\"SM\"");
			if (pos_tmp != NULL && pos_tmp < pos_end) {
				s->Phone.Data.Priv.ATGEN.SIMSaveSMS = AT_AVAILABLE;
			}
			pos_tmp = strstr(pos_start, "\"ME\"");
			if (pos_tmp != NULL && pos_tmp < pos_end) {
				s->Phone.Data.Priv.ATGEN.PhoneSaveSMS = AT_AVAILABLE;
			}
		}

		if (strstr(msg.Buffer, "\"SM\"") != NULL) s->Phone.Data.Priv.ATGEN.SIMSMSMemory = AT_AVAILABLE;
		else s->Phone.Data.Priv.ATGEN.SIMSMSMemory = AT_NOTAVAILABLE;

		if (strstr(msg.Buffer, "\"ME\"") != NULL) s->Phone.Data.Priv.ATGEN.PhoneSMSMemory = AT_AVAILABLE;
		else s->Phone.Data.Priv.ATGEN.PhoneSMSMemory = AT_NOTAVAILABLE;

		smprintf(s, "Available SMS memories received: read: ME = %d, SM = %d, save: ME = %d, SM = %d\n",
				s->Phone.Data.Priv.ATGEN.PhoneSMSMemory,
				s->Phone.Data.Priv.ATGEN.SIMSMSMemory,
				s->Phone.Data.Priv.ATGEN.PhoneSaveSMS,
				s->Phone.Data.Priv.ATGEN.SIMSaveSMS
				);
		return ERR_NONE;
	case AT_Reply_Error:
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(s);
	default:
		return ERR_UNKNOWNRESPONSE;
	}
}

GSM_Error ATGEN_GetSMSMemories(GSM_StateMachine *s)
{
	GSM_Error error;

	smprintf(s, "Getting available SMS memories\n");
	ATGEN_WaitFor(s, "AT+CPMS=?\r", 10, 0x00, 4, ID_GetSMSMemories);

	return error;
}

GSM_Error ATGEN_SetSMSMemory(GSM_StateMachine *s, bool SIM, bool for_write)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	char 			req[] = "AT+CPMS=\"XX\",\"XX\"\r";
	int			reqlen = 18;
	GSM_Error		error;

	/* If phone encodes also values in command, we need normal charset */
	if (Priv->EncodedCommands) {
		error = ATGEN_SetCharset(s, AT_PREF_CHARSET_NORMAL);
		if (error != ERR_NONE) return error;
	}

	if ((SIM && Priv->SIMSMSMemory == 0) || (!SIM && Priv->PhoneSMSMemory == 0)) {
		/* We silently ignore error here, because when this fails, we can try to setmemory anyway */
		ATGEN_GetSMSMemories(s);
	}

	/* If phone can not save SMS, don't try to set memory for saving */
	if (for_write) {
		if (SIM && Priv->SIMSaveSMS == AT_NOTAVAILABLE) return ERR_NOTSUPPORTED;
		if (!SIM && Priv->PhoneSaveSMS == AT_NOTAVAILABLE) return ERR_NOTSUPPORTED;
	} else {
		/* No need to set memory for writing */
		req[12] = '\r';
		reqlen = 13;
	}

	if (SIM) {
		if (Priv->SMSMemory == MEM_SM) return ERR_NONE;
		if (Priv->SIMSMSMemory == AT_NOTAVAILABLE) return ERR_NOTSUPPORTED;

		req[9]  = 'S'; req[10] = 'M';
		req[14] = 'S'; req[15] = 'M';

		smprintf(s, "Setting SMS memory type to SM\n");
		ATGEN_WaitFor(s, req, reqlen, 0x00, 3, ID_SetMemoryType);
		if (Priv->SIMSMSMemory == 0 && error == ERR_NONE) {
			Priv->SIMSMSMemory = AT_AVAILABLE;
		}
		if (error == ERR_NOTSUPPORTED) {
			smprintf(s, "Can't access SIM card?\n");
			return ERR_SECURITYERROR;
		}
		if (error != ERR_NONE) return error;
		Priv->SMSMemory = MEM_SM;
	} else {
		if (Priv->SMSMemory == MEM_ME) return ERR_NONE;
		if (Priv->PhoneSMSMemory == AT_NOTAVAILABLE) return ERR_NOTSUPPORTED;

		req[9]  = 'M'; req[10] = 'E';
		req[14] = 'M'; req[15] = 'E';

		smprintf(s, "Setting SMS memory type to ME\n");
		ATGEN_WaitFor(s, req, reqlen, 0x00, 3, ID_SetMemoryType);
		if (Priv->PhoneSMSMemory == 0 && error == ERR_NONE) {
			Priv->PhoneSMSMemory = AT_AVAILABLE;
		}
		if (error == ERR_NONE) Priv->SMSMemory = MEM_ME;
	}
	return error;
}

GSM_Error ATGEN_GetSMSMode(GSM_StateMachine *s)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
  	GSM_Error 		error;

	if (Priv->SMSMode != 0) return ERR_NONE;

	smprintf(s, "Trying SMS PDU mode\n");
	ATGEN_WaitFor(s, "AT+CMGF=0\r", 10, 0x00, 3, ID_GetSMSMode);
	if (error==ERR_NONE) {
		Priv->SMSMode = SMS_AT_PDU;
		return ERR_NONE;
	}

	smprintf(s, "Trying SMS text mode\n");
	ATGEN_WaitFor(s, "AT+CMGF=1\r", 10, 0x00, 3, ID_GetSMSMode);
	if (error==ERR_NONE) {
		smprintf(s, "Enabling displaying all parameters in text mode\n");
		ATGEN_WaitFor(s, "AT+CSDH=1\r", 10, 0x00, 3, ID_GetSMSMode);
		if (error == ERR_NONE) Priv->SMSMode = SMS_AT_TXT;
	}

	return error;
}

GSM_Error ATGEN_GetSMSLocation(GSM_StateMachine *s, GSM_SMSMessage *sms, unsigned char *folderid, int *location, bool for_write)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	int			ifolderid, maxfolder;
	GSM_Error		error;

	if (Priv->PhoneSMSMemory == 0) {
		error = ATGEN_SetSMSMemory(s, false, for_write);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) return error;
	}
	if (Priv->SIMSMSMemory == 0) {
		error = ATGEN_SetSMSMemory(s, true, for_write);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) return error;
	}

	if (Priv->SIMSMSMemory != AT_AVAILABLE && Priv->PhoneSMSMemory != AT_AVAILABLE) {
		/* No SMS memory at all */
		return ERR_NOTSUPPORTED;
	}
	if (Priv->SIMSMSMemory == AT_AVAILABLE && Priv->PhoneSMSMemory == AT_AVAILABLE) {
		/* Both available */
		maxfolder = 2;
	} else {
		/* One available */
		maxfolder = 1;
	}

	/* simulate flat SMS memory */
	if (sms->Folder == 0x00) {
		ifolderid = sms->Location / AT_MAX_SMS_LOCATION;
		if (ifolderid + 1 > maxfolder) return ERR_NOTSUPPORTED;
		*folderid = ifolderid + 1;
		*location = sms->Location - ifolderid * AT_MAX_SMS_LOCATION;
	} else {
		if (sms->Folder > 2 * maxfolder) return ERR_NOTSUPPORTED;
		*folderid = sms->Folder <= 2 ? 1 : 2;
		*location = sms->Location;
	}

	/* Some phones start locations from 0, handle them here */
	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SMS_LOCATION_0)) {
		(*location)--;
	}

	smprintf(s, "SMS folder %i & location %i -> ATGEN folder %i & location %i\n",
		sms->Folder,sms->Location,*folderid,*location);

	if (Priv->SIMSMSMemory == AT_AVAILABLE && *folderid == 1) {
		return ATGEN_SetSMSMemory(s, true, false);
	} else {
		return ATGEN_SetSMSMemory(s, false, false);
	}
}

/**
 * Converts location from AT internal to Gammu API. We need to ensure
 * locations in API are sequential over all folders.
 */
void ATGEN_SetSMSLocation(GSM_StateMachine *s, GSM_SMSMessage *sms, unsigned char folderid, int location)
{
	sms->Folder	= 0; /* Flat memory */
	sms->Location	= (folderid - 1) * AT_MAX_SMS_LOCATION + location;
	smprintf(s, "ATGEN folder %i & location %i -> SMS folder %i & location %i\n",
		folderid, location, sms->Folder, sms->Location);
}

GSM_Error ATGEN_ReplyGetSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData 	*Priv 	= &s->Phone.Data.Priv.ATGEN;
	GSM_SMSMessage		*sms	= &s->Phone.Data.GetSMSMessage->SMS[0];
	int 			current = 0, current2, i;
	unsigned char 		buffer[300],smsframe[800];
	unsigned char		firstbyte, TPDCS, TPUDL, TPStatus;
	GSM_Error		error=ERR_UNKNOWN;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		if (Priv->Lines.numbers[4] == 0x00) return ERR_EMPTY;
		s->Phone.Data.GetSMSMessage->Number 	 	= 1;
		s->Phone.Data.GetSMSMessage->SMS[0].Name[0] 	= 0;
		s->Phone.Data.GetSMSMessage->SMS[0].Name[1]	= 0;
		switch (Priv->SMSMode) {
		case SMS_AT_PDU:
			CopyLineString(buffer, msg.Buffer, Priv->Lines, 2);
			switch (buffer[7]) {
				case '0': sms->State = SMS_UnRead; 	break;
				case '1': sms->State = SMS_Read;	break;
				case '2': sms->State = SMS_UnSent;	break;
				default : sms->State = SMS_Sent;	break;//case '3'
			}
			DecodeHexBin (buffer, GetLineString(msg.Buffer,Priv->Lines,3), GetLineLength(msg.Buffer,Priv->Lines,3));
			/* Siemens MC35 (only ?) */
			if (strstr(msg.Buffer,"+CMGR: 0,,0")!=NULL) return ERR_EMPTY;
			/* Siemens M20 */
			if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_M20SMS)) {
				/* we check for the most often visible */
				if (buffer[1]!=NUMBER_UNKNOWN_NUMBERING_PLAN_ISDN && buffer[1]!=NUMBER_INTERNATIONAL_NUMBERING_PLAN_ISDN &&
				    buffer[1]!=NUMBER_ALPHANUMERIC_NUMBERING_PLAN_UNKNOWN) {
					/* Seems to be Delivery Report */
					smprintf(s, "SMS type - status report (M20 style)\n");
					sms->PDU 	 = SMS_Status_Report;
					sms->Folder 	 = 1;	/*INBOX SIM*/
					sms->InboxFolder = true;

					smsframe[12]=buffer[current++];
					smsframe[PHONE_SMSStatusReport.TPMR]=buffer[current++];
					current2=((buffer[current])+1)/2+1;
					for(i=0;i<current2+1;i++) smsframe[PHONE_SMSStatusReport.Number+i]=buffer[current++];
					for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReport.DateTime+i]=buffer[current++];
					smsframe[0] = 0;
					for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReport.SMSCTime+i]=buffer[current++];
					smsframe[PHONE_SMSStatusReport.TPStatus]=buffer[current];
					GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSStatusReport);
					return ERR_NONE;
				}
			}
			/* We use locations from SMS layouts like in ../phone2.c(h) */
			for(i=0;i<buffer[0]+1;i++) smsframe[i]=buffer[current++];
			smsframe[12]=buffer[current++];
			/* See GSM 03.40 section 9.2.3.1 */
			switch (smsframe[12] & 0x03) {
			case 0x00:
				smprintf(s, "SMS type - deliver\n");
				sms->PDU 	 = SMS_Deliver;
				if (Priv->SMSMemory == MEM_SM) {
					sms->Folder = 1; /*INBOX SIM*/
				} else {
					sms->Folder = 3; /*INBOX ME*/
				}
				sms->InboxFolder = true;
				current2=((buffer[current])+1)/2+1;
				if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_M20SMS)) {
					if (buffer[current+1]==NUMBER_ALPHANUMERIC_NUMBERING_PLAN_UNKNOWN) {
						smprintf(s, "Trying to read alphanumeric number\n");
						for(i=0;i<4;i++) smsframe[PHONE_SMSDeliver.Number+i]=buffer[current++];
						current+=6;
						for(i=0;i<current2-3;i++) smsframe[PHONE_SMSDeliver.Number+i+4]=buffer[current++];
					} else {
						for(i=0;i<current2+1;i++) smsframe[PHONE_SMSDeliver.Number+i]=buffer[current++];
					}
				} else {
					for(i=0;i<current2+1;i++) smsframe[PHONE_SMSDeliver.Number+i]=buffer[current++];
				}
				smsframe[PHONE_SMSDeliver.TPPID] = buffer[current++];
				smsframe[PHONE_SMSDeliver.TPDCS] = buffer[current++];
				for(i=0;i<7;i++) smsframe[PHONE_SMSDeliver.DateTime+i]=buffer[current++];
				smsframe[PHONE_SMSDeliver.TPUDL] = buffer[current++];
				for(i=0;i<smsframe[PHONE_SMSDeliver.TPUDL];i++) smsframe[i+PHONE_SMSDeliver.Text]=buffer[current++];
				GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSDeliver);
				return ERR_NONE;
			case 0x01:
				smprintf(s, "SMS type - submit\n");
				sms->PDU 	 = SMS_Submit;
				if (Priv->SMSMemory == MEM_SM) {
					sms->Folder = 2; /*OUTBOX SIM*/
					smprintf(s, "Outbox SIM\n");
				} else {
					sms->Folder = 4; /*OUTBOX ME*/
				}
				sms->InboxFolder = false;
				smsframe[PHONE_SMSSubmit.TPMR] = buffer[current++];
				current2=((buffer[current])+1)/2+1;
				if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_M20SMS)) {
					if (buffer[current+1]==NUMBER_ALPHANUMERIC_NUMBERING_PLAN_UNKNOWN) {
						smprintf(s, "Trying to read alphanumeric number\n");
						for(i=0;i<4;i++) smsframe[PHONE_SMSSubmit.Number+i]=buffer[current++];
						current+=6;
						for(i=0;i<current2-3;i++) smsframe[PHONE_SMSSubmit.Number+i+4]=buffer[current++];
					} else {
						for(i=0;i<current2+1;i++) smsframe[PHONE_SMSSubmit.Number+i]=buffer[current++];
					}
				} else {
					for(i=0;i<current2+1;i++) smsframe[PHONE_SMSSubmit.Number+i]=buffer[current++];
				}
				smsframe[PHONE_SMSSubmit.TPPID] = buffer[current++];
				smsframe[PHONE_SMSSubmit.TPDCS] = buffer[current++];
				/* See GSM 03.40 9.2.3.3 - TPVP can not exist in frame */
				if ((smsframe[12] & 0x18)!=0) current++; //TPVP is ignored now
				smsframe[PHONE_SMSSubmit.TPUDL] = buffer[current++];
				for(i=0;i<smsframe[PHONE_SMSSubmit.TPUDL];i++) smsframe[i+PHONE_SMSSubmit.Text]=buffer[current++];
				GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSSubmit);
				return ERR_NONE;
			case 0x02:
				smprintf(s, "SMS type - status report\n");
				sms->PDU 	 = SMS_Status_Report;
				sms->Folder 	 = 1;	/*INBOX SIM*/
				sms->InboxFolder = true;
				smprintf(s, "TPMR is %d\n",buffer[current]);
				smsframe[PHONE_SMSStatusReport.TPMR] = buffer[current++];
				current2=((buffer[current])+1)/2+1;
				for(i=0;i<current2+1;i++) smsframe[PHONE_SMSStatusReport.Number+i]=buffer[current++];
				for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReport.DateTime+i]=buffer[current++];
				for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReport.SMSCTime+i]=buffer[current++];
				smsframe[PHONE_SMSStatusReport.TPStatus]=buffer[current];
				GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSStatusReport);
				return ERR_NONE;
			}
			break;
		case SMS_AT_TXT:
			current = 0;
			while (msg.Buffer[current]!='"') current++;
			current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
			if (!strcmp(buffer,"\"0\"") || !strcmp(buffer,"\"REC UNREAD\"")) {
				smprintf(s, "SMS type - deliver\n");
				sms->State 	 = SMS_UnRead;
				sms->PDU 	 = SMS_Deliver;
				if (Priv->SMSMemory == MEM_SM) {
					sms->Folder = 1; /*INBOX SIM*/
				} else {
					sms->Folder = 3; /*INBOX ME*/
				}
				sms->InboxFolder = true;
			} else if (!strcmp(buffer,"\"1\"") || !strcmp(buffer,"\"REC READ\"")) {
				smprintf(s, "SMS type - deliver\n");
				sms->State 	 = SMS_Read;
				sms->PDU 	 = SMS_Deliver;
				if (Priv->SMSMemory == MEM_SM) {
					sms->Folder = 1; /*INBOX SIM*/
				} else {
					sms->Folder = 3; /*INBOX ME*/
				}
				sms->InboxFolder = true;
			} else if (!strcmp(buffer,"\"2\"") || !strcmp(buffer,"\"STO UNSENT\"")) {
				smprintf(s, "SMS type - submit\n");
				sms->State 	 = SMS_UnSent;
				sms->PDU 	 = SMS_Submit;
				if (Priv->SMSMemory == MEM_SM) {
					sms->Folder = 2; /*OUTBOX SIM*/
				} else {
					sms->Folder = 4; /*OUTBOX ME*/
				}
				sms->InboxFolder = false;
			} else if (!strcmp(buffer,"\"3\"") || !strcmp(buffer,"\"STO SENT\"")) {
				smprintf(s, "SMS type - submit\n");
				sms->State 	 = SMS_Sent;
				sms->PDU 	 = SMS_Submit;
				if (Priv->SMSMemory == MEM_SM) {
					sms->Folder = 2; /*OUTBOX SIM*/
				} else {
					sms->Folder = 4; /*OUTBOX ME*/
				}
				sms->InboxFolder = false;
			}
			current += ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
			/* It's delivery report according to Nokia AT standards */
			if (sms->Folder==1 && buffer[0]!=0 && buffer[0]!='"') {
				/* ??? */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				/* format of sender number */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				/* Sender number */
				/* FIXME: support for all formats */
				EncodeUnicode(sms->Number,buffer+1,strlen(buffer)-2);
				smprintf(s, "Sender \"%s\"\n",DecodeUnicodeString(sms->Number));
				/* ??? */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				/* Sending datetime */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				i = strlen(buffer);
				buffer[i] = ',';
				i++;
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer+i);
				smprintf(s, "\"%s\"\n",buffer);
				error = ATGEN_DecodeDateTime(s, &sms->DateTime, buffer);
				if (error!=ERR_NONE) return error;
				/* Date of SMSC response */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				i = strlen(buffer);
				buffer[i] = ',';
				i++;
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer+i);
				smprintf(s, "\"%s\"\n",buffer);
				error = ATGEN_DecodeDateTime(s, &sms->SMSCTime, buffer);
				if (error!=ERR_NONE) return error;
				/* TPStatus */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				TPStatus=atoi(buffer);
				buffer[PHONE_SMSStatusReport.TPStatus] = TPStatus;
				error=GSM_DecodeSMSFrameStatusReportData(sms, buffer, PHONE_SMSStatusReport);
				if (error!=ERR_NONE) return error;
				/* NO SMSC number */
				sms->SMSC.Number[0]=0;
				sms->SMSC.Number[1]=0;
				sms->PDU = SMS_Status_Report;
				sms->ReplyViaSameSMSC=false;
			} else {
				/* Sender number */
				/* FIXME: support for all formats */
				EncodeUnicode(sms->Number,buffer+1,strlen(buffer)-2);
				/* Sender number in alphanumeric format ? */
				current += ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				if (strlen(buffer)!=0) EncodeUnicode(sms->Number,buffer+1,strlen(buffer)-2);
				smprintf(s, "Sender \"%s\"\n",DecodeUnicodeString(sms->Number));
				/* Sending datetime */
				if (sms->Folder==1 || sms->Folder==3) {
					current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
					/* FIXME: ATGEN_ExtractOneParameter() is broken as it doesn't respect
					 * quoting of parameters and thus +FOO: "ab","cd,ef" will consider
					 * as three arguments: "ab" >> "cd >> ef"
					 */
					if (*buffer=='"') {
						i = strlen(buffer);
						buffer[i] = ',';
						i++;
						current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer+i);
					}
					smprintf(s, "\"%s\"\n",buffer);
					if (*buffer)
						error = ATGEN_DecodeDateTime(s, &sms->DateTime, buffer);
						if (error!=ERR_NONE) return error;
					else {
						/* FIXME: What is the proper undefined GSM_DateTime ? */
						memset(&sms->DateTime, 0, sizeof(sms->DateTime));
					}
					error = ATGEN_DecodeDateTime(s, &sms->DateTime, buffer);
					if (error!=ERR_NONE) return error;
				}
				/* Sender number format */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				/* First byte */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				firstbyte=atoi(buffer);
				sms->ReplyViaSameSMSC=false;
				/* GSM 03.40 section 9.2.3.17 (TP-Reply-Path) */
				if ((firstbyte & 128)==128) sms->ReplyViaSameSMSC=true;
				/* TP PID */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				sms->ReplaceMessage = 0;
				if (atoi(buffer) > 0x40 && atoi(buffer) < 0x48) {
					sms->ReplaceMessage = atoi(buffer) - 0x40;
				}
				smprintf(s, "TPPID: %02x %i\n",atoi(buffer),atoi(buffer));
				/* TP DCS */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				TPDCS=atoi(buffer);
				if (sms->Folder==2 || sms->Folder==4) {
					/*TP VP */
					current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				}
				/* SMSC number */
				/* FIXME: support for all formats */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				EncodeUnicode(sms->SMSC.Number,buffer+1,strlen(buffer)-2);
				/* Format of SMSC number */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				/* TPUDL */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				TPUDL=atoi(buffer);
				current++;
				sms->Coding = SMS_Coding_8bit;
				/* GSM 03.40 section 9.2.3.10 (TP-Data-Coding-Scheme)
				 * and GSM 03.38 section 4
				 */
				if ((TPDCS & 0xC0) == 0) {
					/* bits 7..4 set to 00xx */
					if ((TPDCS & 0xC) == 0xC) {
						smprintf(s, "WARNING: reserved alphabet value in TPDCS\n");
					} else {
						if (TPDCS == 0) 		sms->Coding=SMS_Coding_Default_No_Compression;
						if ((TPDCS & 0x2C) == 0x00) 	sms->Coding=SMS_Coding_Default_No_Compression;
						if ((TPDCS & 0x2C) == 0x20) 	sms->Coding=SMS_Coding_Default_Compression;
						if ((TPDCS & 0x2C) == 0x08) 	sms->Coding=SMS_Coding_Unicode_No_Compression;
						if ((TPDCS & 0x2C) == 0x28) 	sms->Coding=SMS_Coding_Unicode_Compression;
					}
				} else if ((TPDCS & 0xF0) >= 0x40 &&
					   (TPDCS & 0xF0) <= 0xB0) {
					/* bits 7..4 set to 0100 ... 1011 */
					smprintf(s, "WARNING: reserved coding group in TPDCS\n");
				} else if (((TPDCS & 0xF0) == 0xC0) ||
				      	   ((TPDCS & 0xF0) == 0xD0)) {
					/* bits 7..4 set to 1100 or 1101 */
					if ((TPDCS & 4) == 4) {
						smprintf(s, "WARNING: set reserved bit 2 in TPDCS\n");
					} else {
						sms->Coding=SMS_Coding_Default_No_Compression;
					}
				} else if ((TPDCS & 0xF0) == 0xE0) {
					/* bits 7..4 set to 1110 */
					if ((TPDCS & 4) == 4) {
						smprintf(s, "WARNING: set reserved bit 2 in TPDCS\n");
					} else {
						sms->Coding=SMS_Coding_Unicode_No_Compression;
					}
				} else if ((TPDCS & 0xF0) == 0xF0) {
					/* bits 7..4 set to 1111 */
					if ((TPDCS & 8) == 8) {
						smprintf(s, "WARNING: set reserved bit 3 in TPDCS\n");
					} else {
						if ((TPDCS & 4) == 0) sms->Coding=SMS_Coding_Default_No_Compression;
					}
				}
				sms->Class = -1;
				if ((TPDCS & 0xD0) == 0x10) {
					/* bits 7..4 set to 00x1 */
					if ((TPDCS & 0xC) == 0xC) {
						smprintf(s, "WARNING: reserved alphabet value in TPDCS\n");
					} else {
						sms->Class = (TPDCS & 3);
					}
				} else if ((TPDCS & 0xF0) == 0xF0) {
					/* bits 7..4 set to 1111 */
					if ((TPDCS & 8) == 8) {
						smprintf(s, "WARNING: set reserved bit 3 in TPDCS\n");
					} else {
						sms->Class = (TPDCS & 3);
					}
				}
				smprintf(s, "SMS class: %i\n",sms->Class);
				switch (sms->Coding) {
				case SMS_Coding_Default_No_Compression:
					/* GSM 03.40 section 9.2.3.23 (TP-User-Data-Header-Indicator) */
					/* If not SMS with UDH, it's coded normal */
					/* If UDH available, treat it as Unicode or 8 bit */
					if ((firstbyte & 0x40)!=0x40) {
						sms->UDH.Type	= UDH_NoUDH;
						sms->Length	= TPUDL;
						EncodeUnicode(sms->Text,msg.Buffer+Priv->Lines.numbers[2*2],TPUDL);
						break;
					}
				case SMS_Coding_Unicode_No_Compression:
				case SMS_Coding_8bit:
					DecodeHexBin(buffer+PHONE_SMSDeliver.Text, msg.Buffer+current, TPUDL*2);
					buffer[PHONE_SMSDeliver.firstbyte] 	= firstbyte;
					buffer[PHONE_SMSDeliver.TPDCS] 		= TPDCS;
					buffer[PHONE_SMSDeliver.TPUDL] 		= TPUDL;
					return GSM_DecodeSMSFrameText(sms, buffer, PHONE_SMSDeliver);
				default:
					break;
				}
			}
			return ERR_NONE;
		default:
			break;
		}
		break;
	case AT_Reply_CMSError:
		if (Priv->ErrorCode == 320 || Priv->ErrorCode == 500) {
			return ERR_EMPTY;
		} else {
			return ATGEN_HandleCMSError(s);
		}
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(s);
	case AT_Reply_Error:
		/* A2D returns Error with empty location */
		return ERR_EMPTY;
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetSMS(GSM_StateMachine *s, GSM_MultiSMSMessage *sms)
{
	unsigned char		req[20], folderid;
	int			back_folder, back_location;
	GSM_Error		error;
	int			location, getfolder, add = 0;
	GSM_Phone_ATGENData 	*Priv 	= &s->Phone.Data.Priv.ATGEN;

	/* Clear SMS structure of any possible junk */
	back_folder = sms->SMS[0].Folder;
	back_location = sms->SMS[0].Location;
	GSM_SetDefaultSMSData(&sms->SMS[0]);
	sms->SMS[0].Folder = back_folder;
	sms->SMS[0].Location = back_location;

	error=ATGEN_GetSMSLocation(s, &sms->SMS[0], &folderid, &location, false);
	if (error!=ERR_NONE) return error;
	if (Priv->SMSMemory == MEM_ME && GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SMSME900)) add = 899;
	sprintf(req, "AT+CMGR=%i\r", location + add);

	error=ATGEN_GetSMSMode(s);
	if (error != ERR_NONE) return error;

	/* There is possibility that date will be encoded in text mode */
	if (Priv->SMSMode == SMS_AT_TXT) {
		error = ATGEN_SetCharset(s, AT_PREF_CHARSET_NORMAL);
		if (error != ERR_NONE) return error;
	}

	error=ATGEN_GetManufacturer(s);
	if (error != ERR_NONE) return error;

	s->Phone.Data.GetSMSMessage=sms;
	smprintf(s, "Getting SMS\n");
	ATGEN_WaitFor(s, req, strlen(req), 0x00, 5, ID_GetSMSMessage);
	if (error==ERR_NONE) {
		getfolder = sms->SMS[0].Folder;
//		if (getfolder != 0 && getfolder != sms->SMS[0].Folder) return ERR_EMPTY;
		ATGEN_SetSMSLocation(s, &sms->SMS[0], folderid, location);
		sms->SMS[0].Folder = getfolder;
		sms->SMS[0].Memory = MEM_SM;
		if (getfolder > 2) sms->SMS[0].Memory = MEM_ME;
	}
	return error;
}

GSM_Error ATGEN_GetNextSMS(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, bool start)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error 		error;
	int			usedsms;

	if (Priv->PhoneSMSMemory == 0) {
		error = ATGEN_SetSMSMemory(s, false, false);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) return error;
	}
	if (Priv->SIMSMSMemory == 0) {
		error = ATGEN_SetSMSMemory(s, true, false);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) return error;
	}
	if (Priv->SIMSMSMemory == AT_NOTAVAILABLE && Priv->PhoneSMSMemory == AT_NOTAVAILABLE) return ERR_NOTSUPPORTED;

	if (start) {
		error=s->Phone.Functions->GetSMSStatus(s,&Priv->LastSMSStatus);
		if (error!=ERR_NONE) return error;
		Priv->LastSMSRead		= 0;
		sms->SMS[0].Location 		= 0;
	}
	while (true) {
		sms->SMS[0].Location++;
		if (sms->SMS[0].Location < AT_MAX_SMS_LOCATION) {
			if (Priv->SIMSMSMemory == AT_AVAILABLE) {
				usedsms = Priv->LastSMSStatus.SIMUsed;
			} else {
				usedsms = Priv->LastSMSStatus.PhoneUsed;
			}

			if (Priv->LastSMSRead >= usedsms) {
				if (Priv->PhoneSMSMemory == AT_NOTAVAILABLE || Priv->LastSMSStatus.PhoneUsed==0) return ERR_EMPTY;
				Priv->LastSMSRead	= 0;
				sms->SMS[0].Location 	= AT_MAX_SMS_LOCATION + 1;
			}
		} else {
			if (Priv->PhoneSMSMemory == AT_NOTAVAILABLE) return ERR_EMPTY;
			if (Priv->LastSMSRead>=Priv->LastSMSStatus.PhoneUsed) return ERR_EMPTY;
		}
		sms->SMS[0].Folder = 0;
		error=s->Phone.Functions->GetSMS(s, sms);
		if (error==ERR_NONE) {
			Priv->LastSMSRead++;
			break;
		}
		if (error != ERR_EMPTY && error != ERR_INVALIDLOCATION) return error;
	}
	return error;
}

GSM_Error ATGEN_ReplyGetSMSStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_SMSMemoryStatus	*SMSStatus = s->Phone.Data.SMSStatus;
	char 			*start;
	int			current = 0;
	unsigned char		buffer[50];

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "SMS status received\n");
		/* Check for +CPMS: 0,30,0,30,8,330, this is according to ETSI */
		start = strstr(msg.Buffer, "+CPMS: ");
		/*
		 * Samsung formats this different way, sample response:
		 * 1 "AT+CPMS="SM","SM""
		 * 2 "+CPMS:"SM",3,30,"SM",3,30,"SM",3,30"
		 * 3 "OK"
		 */
		if (start == NULL) {
			start = strstr(msg.Buffer, "+CPMS:\"");
			if (start == NULL) {
				/* Check for +CPMS:0,30,0,30,8,330 */
				start = strstr(msg.Buffer, "+CPMS:");
				if (start == NULL) {
					return ERR_UNKNOWNRESPONSE;
				}
				start += 6;
				goto parse_sizes;
			}
			start += 6;
			current+=ATGEN_ExtractOneParameter(start+current, buffer);
			if (strcmp(buffer, "\"ME\"") == 0) {
				current+=ATGEN_ExtractOneParameter(start+current, buffer);
				SMSStatus->PhoneUsed = atoi(buffer);
				current+=ATGEN_ExtractOneParameter(start+current, buffer);
				SMSStatus->PhoneSize = atoi(buffer);
				smprintf(s, "Used : %i\n",SMSStatus->PhoneUsed);
				smprintf(s, "Size : %i\n",SMSStatus->PhoneSize);
				return ERR_NONE;
			} else if (strcmp(buffer, "\"SM\"") == 0) {
				current+=ATGEN_ExtractOneParameter(start+current, buffer);
				SMSStatus->SIMUsed = atoi(buffer);
				current+=ATGEN_ExtractOneParameter(start+current, buffer);
				SMSStatus->SIMSize = atoi(buffer);
				smprintf(s, "Used : %i\n",SMSStatus->SIMUsed);
				smprintf(s, "Size : %i\n",SMSStatus->SIMSize);
				return ERR_NONE;
			} else {
				smprintf(s, "Unknown memory: %s\n", buffer);
				return ERR_UNKNOWNRESPONSE;
			}
		}
		/* Skip +CPMS: */
		start += 7;
parse_sizes:
		if (strstr(msg.Buffer,"ME")!=NULL) {
			SMSStatus->PhoneUsed 	= atoi(start);
			current+=ATGEN_ExtractOneParameter(start+current, buffer);
			current+=ATGEN_ExtractOneParameter(start+current, buffer);
			SMSStatus->PhoneSize	= atoi(buffer);
			smprintf(s, "Used : %i\n",SMSStatus->PhoneUsed);
			smprintf(s, "Size : %i\n",SMSStatus->PhoneSize);
		} else {
			SMSStatus->SIMUsed 	= atoi(start);
			current+=ATGEN_ExtractOneParameter(start+current, buffer);
			current+=ATGEN_ExtractOneParameter(start+current, buffer);
			SMSStatus->SIMSize	= atoi(buffer);
			smprintf(s, "Used : %i\n",SMSStatus->SIMUsed);
			smprintf(s, "Size : %i\n",SMSStatus->SIMSize);
			if (SMSStatus->SIMSize == 0) {
				smprintf(s, "Can't access SIM card\n");
				return ERR_SECURITYERROR;
			}
		}
		return ERR_NONE;
	case AT_Reply_Error:
		if (strstr(msg.Buffer,"SM")!=NULL) {
			smprintf(s, "Can't access SIM card\n");
			return ERR_SECURITYERROR;
		}
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

GSM_Error ATGEN_GetSMSStatus(GSM_StateMachine *s, GSM_SMSMemoryStatus *status)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error 		error;

	/* No templates at all */
	status->TemplatesUsed	= 0;

	status->SIMUsed		= 0;
	status->SIMUnRead 	= 0;
	status->SIMSize		= 0;

	s->Phone.Data.SMSStatus=status;

	if ((Priv->SIMSMSMemory == 0) || (Priv->PhoneSMSMemory == 0)) {
		/* We silently ignore error here, because when this fails, we can try to setmemory anyway */
		ATGEN_GetSMSMemories(s);
	}

	if (Priv->PhoneSMSMemory == 0) {
		error = ATGEN_SetSMSMemory(s, false, false);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) return error;
	}
	if (Priv->SIMSMSMemory == 0) {
		error = ATGEN_SetSMSMemory(s, true, false);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) return error;
	}

	if (Priv->SIMSMSMemory == AT_AVAILABLE) {
		smprintf(s, "Getting SIM SMS status\n");
		if (Priv->SIMSaveSMS == AT_AVAILABLE) {
			ATGEN_WaitFor(s, "AT+CPMS=\"SM\",\"SM\"\r", 18, 0x00, 4, ID_GetSMSStatus);
		} else {
			ATGEN_WaitFor(s, "AT+CPMS=\"SM\"\r", 13, 0x00, 4, ID_GetSMSStatus);
		}
		if (error!=ERR_NONE) return error;
		Priv->SMSMemory = MEM_SM;
	}

	status->PhoneUsed	= 0;
	status->PhoneUnRead 	= 0;
	status->PhoneSize	= 0;

	if (Priv->PhoneSMSMemory == AT_AVAILABLE) {
		smprintf(s, "Getting phone SMS status\n");
		if (Priv->PhoneSaveSMS == AT_AVAILABLE) {
			ATGEN_WaitFor(s, "AT+CPMS=\"ME\",\"ME\"\r", 18, 0x00, 4, ID_GetSMSStatus);
		} else {
			ATGEN_WaitFor(s, "AT+CPMS=\"ME\"\r", 13, 0x00, 4, ID_GetSMSStatus);
		}
		if (error!=ERR_NONE) return error;
		Priv->SMSMemory = MEM_ME;
	}

	return ERR_NONE;
}

GSM_Error ATGEN_ReplyGetIMEI(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	CopyLineString(s->Phone.Data.IMEI, msg.Buffer, s->Phone.Data.Priv.ATGEN.Lines, 2);
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

GSM_Error ATGEN_ReplyAddSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	char 	*start;
	int	i;

	if (s->Protocol.Data.AT.EditMode) {
		if (s->Phone.Data.Priv.ATGEN.ReplyState != AT_Reply_SMSEdit) {
			return ATGEN_HandleCMSError(s);
		}
		s->Protocol.Data.AT.EditMode = false;
		return ERR_NONE;
	}

	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "SMS saved OK\n");
		for(i=0;i<msg.Length;i++) {
			if (msg.Buffer[i] == 0x00) msg.Buffer[i] = 0x20;
		}
		start = strstr(msg.Buffer, "+CMGW: ");
		if (start == NULL) return ERR_UNKNOWN;
		s->Phone.Data.SaveSMSMessage->Location = atoi(start+7);
		smprintf(s, "Saved at location %i\n",s->Phone.Data.SaveSMSMessage->Location);
		return ERR_NONE;
	case AT_Reply_Error:
		smprintf(s, "Error\n");
		return ERR_NOTSUPPORTED;
	case AT_Reply_CMSError:
		/* This error occurs in case that phone couldn't save SMS */
		return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(s);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_MakeSMSFrame(GSM_StateMachine *s, GSM_SMSMessage *message, unsigned char *hexreq, int *current, int *length2)
{
	GSM_Error 		error;
	int			i, length;
	unsigned char		req[1000], buffer[1000];
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_SMSC	 	SMSC;

	error=ATGEN_GetSMSMode(s);
	if (error != ERR_NONE) return error;

	length 	 = 0;
	*current = 0;
	switch (Priv->SMSMode) {
	case SMS_AT_PDU:
		if (message->PDU == SMS_Deliver) {
 			smprintf(s, "SMS Deliver\n");
			error=PHONE_EncodeSMSFrame(s,message,buffer,PHONE_SMSDeliver,&length,true);
			if (error != ERR_NONE) return error;
			length = length - PHONE_SMSDeliver.Text;
			for (i=0;i<buffer[PHONE_SMSDeliver.SMSCNumber]+1;i++) {
				req[(*current)++]=buffer[PHONE_SMSDeliver.SMSCNumber+i];
			}
			req[(*current)++]=buffer[PHONE_SMSDeliver.firstbyte];
			for (i=0;i<((buffer[PHONE_SMSDeliver.Number]+1)/2+1)+1;i++) {
				req[(*current)++]=buffer[PHONE_SMSDeliver.Number+i];
			}
			req[(*current)++]=buffer[PHONE_SMSDeliver.TPPID];
			req[(*current)++]=buffer[PHONE_SMSDeliver.TPDCS];
			for(i=0;i<7;i++) req[(*current)++]=buffer[PHONE_SMSDeliver.DateTime+i];
			req[(*current)++]=buffer[PHONE_SMSDeliver.TPUDL];
			for(i=0;i<length;i++) req[(*current)++]=buffer[PHONE_SMSDeliver.Text+i];
			EncodeHexBin(hexreq, req, *current);
			*length2 = *current * 2;
			*current = *current - (req[PHONE_SMSDeliver.SMSCNumber]+1);
		} else {
			smprintf(s, "SMS Submit\n");
			error=PHONE_EncodeSMSFrame(s,message,buffer,PHONE_SMSSubmit,&length,true);
			if (error != ERR_NONE) return error;
			length = length - PHONE_SMSSubmit.Text;
			for (i=0;i<buffer[PHONE_SMSSubmit.SMSCNumber]+1;i++) {
				req[(*current)++]=buffer[PHONE_SMSSubmit.SMSCNumber+i];
			}
			req[(*current)++]=buffer[PHONE_SMSSubmit.firstbyte];
			req[(*current)++]=buffer[PHONE_SMSSubmit.TPMR];
			for (i=0;i<((buffer[PHONE_SMSSubmit.Number]+1)/2+1)+1;i++) {
				req[(*current)++]=buffer[PHONE_SMSSubmit.Number+i];
			}
			req[(*current)++]=buffer[PHONE_SMSSubmit.TPPID];
			req[(*current)++]=buffer[PHONE_SMSSubmit.TPDCS];
			req[(*current)++]=buffer[PHONE_SMSSubmit.TPVP];
			req[(*current)++]=buffer[PHONE_SMSSubmit.TPUDL];
			for(i=0;i<length;i++) req[(*current)++]=buffer[PHONE_SMSSubmit.Text+i];
			EncodeHexBin(hexreq, req, *current);
			*length2 = *current * 2;
			*current = *current - (req[PHONE_SMSSubmit.SMSCNumber]+1);
		}
		break;
	case SMS_AT_TXT:
		if (Priv->Manufacturer == 0) {
			error=ATGEN_GetManufacturer(s);
			if (error != ERR_NONE) return error;
		}
		if (Priv->Manufacturer != AT_Nokia) {
			if (message->Coding != SMS_Coding_Default_No_Compression) return ERR_NOTSUPPORTED;
		}
		error=PHONE_EncodeSMSFrame(s,message,req,PHONE_SMSDeliver,&i,true);
		if (error != ERR_NONE) return error;
		CopyUnicodeString(SMSC.Number,message->SMSC.Number);
		SMSC.Location=1;
		error=ATGEN_SetSMSC(s,&SMSC);
		if (error!=ERR_NONE) return error;
		sprintf(buffer, "AT+CSMP=%i,%i,%i,%i\r",
			req[PHONE_SMSDeliver.firstbyte],
			req[PHONE_SMSDeliver.TPVP],
			req[PHONE_SMSDeliver.TPPID],
			req[PHONE_SMSDeliver.TPDCS]);
		ATGEN_WaitFor(s, buffer, strlen(buffer), 0x00, 4, ID_SetSMSParameters);
		if (error==ERR_NOTSUPPORTED) {
			/* Nokia Communicator 9000i doesn't support <vp> parameter */
			sprintf(buffer, "AT+CSMP=%i,,%i,%i\r",
				req[PHONE_SMSDeliver.firstbyte],
				req[PHONE_SMSDeliver.TPPID],
				req[PHONE_SMSDeliver.TPDCS]);
			ATGEN_WaitFor(s, buffer, strlen(buffer), 0x00, 4, ID_SetSMSParameters);
		}
		if (error!=ERR_NONE) return error;
		switch (message->Coding) {
		case SMS_Coding_Default_No_Compression:
			/* If not SMS with UDH, it's as normal text */
			if (message->UDH.Type==UDH_NoUDH) {
				strcpy(hexreq,DecodeUnicodeString(message->Text));
				*length2 = UnicodeLength(message->Text);
				break;
			}
	        case SMS_Coding_Unicode_No_Compression:
	        case SMS_Coding_8bit:
			error=PHONE_EncodeSMSFrame(s,message,buffer,PHONE_SMSDeliver,current,true);
			if (error != ERR_NONE) return error;
			EncodeHexBin (hexreq, buffer+PHONE_SMSDeliver.Text, buffer[PHONE_SMSDeliver.TPUDL]);
			*length2 = buffer[PHONE_SMSDeliver.TPUDL] * 2;
			break;
		default:
			break;
		}
		break;
	}
	return ERR_NONE;
}

GSM_Error ATGEN_AddSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	GSM_Error 		error, error2;
	int			state,Replies,reply, current, current2;
	unsigned char		buffer[1000], hexreq[1000];
	GSM_Phone_Data		*Phone = &s->Phone.Data;
	unsigned char		*statetxt;

	/* This phone supports only sent/unsent messages on SIM */
	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SMSONLYSENT)) {
		if (sms->Folder != 2) {
			smprintf(s, "This phone supports only folder = 2!\n");
			return ERR_NOTSUPPORTED;
		}
	}

	sms->PDU = SMS_Submit;
	switch (sms->Folder) {
	case 1:  sms->PDU 	= SMS_Deliver;		/* Inbox SIM */
		 sms->Memory 	= MEM_SM;
		 error=ATGEN_SetSMSMemory(s, true, true);
		 break;
	case 2:  error=ATGEN_SetSMSMemory(s, true, true);	/* Outbox SIM */
		 sms->Memory 	= MEM_SM;
	 	 break;
	case 3:  sms->PDU = SMS_Deliver;
		 sms->Memory 	= MEM_ME;
		 error=ATGEN_SetSMSMemory(s, false, true);	/* Inbox phone */
		 break;
	case 4:  error=ATGEN_SetSMSMemory(s, false, true);	/* Outbox phone */
		 sms->Memory 	= MEM_ME;
		 break;
	default: return ERR_NOTSUPPORTED;
	}
	if (error!=ERR_NONE) return error;

	error = ATGEN_MakeSMSFrame(s, sms, hexreq, &current, &current2);
	if (error != ERR_NONE) return error;

	switch (Phone->Priv.ATGEN.SMSMode) {
	case SMS_AT_PDU:
		if (sms->PDU == SMS_Deliver) {
			state = 0;
			if (sms->State == SMS_Read || sms->State == SMS_Sent) state = 1;
		} else {
			state = 2;
			if (sms->State == SMS_Read || sms->State == SMS_Sent) state = 3;
		}
		/* Siemens M20 */
		if (GSM_IsPhoneFeatureAvailable(Phone->ModelInfo, F_M20SMS)) {
			/* No (good and 100% working) support for alphanumeric numbers */
			if (sms->Number[1]!='+' && (sms->Number[1]<'0' || sms->Number[1]>'9')) {
				EncodeUnicode(sms->Number,"123",3);
				error = ATGEN_MakeSMSFrame(s, sms, hexreq, &current, &current2);
				if (error != ERR_NONE) return error;
			}
		}
		sprintf(buffer, "AT+CMGW=%i,%i\r",current,state);
		break;
	case SMS_AT_TXT:
		if (sms->PDU == SMS_Deliver) {
			statetxt = "REC UNREAD";
			if (sms->State == SMS_Read || sms->State == SMS_Sent) statetxt = "REC READ";
		} else {
			statetxt = "STO UNSENT";
			if (sms->State == SMS_Read || sms->State == SMS_Sent) statetxt = "STO SENT";
		}
		/* Siemens M20 */
		if (GSM_IsPhoneFeatureAvailable(Phone->ModelInfo, F_M20SMS)) {
			/* No (good and 100% working) support for alphanumeric numbers */
			/* FIXME: Try to autodetect support for <stat> (statetxt) parameter although:
			 * Siemens M20 supports +CMGW <stat> specification but on my model it just
			 * reports ERROR (and <stat> is not respected).
			 * Fortunately it will write "+CMGW: <index>\n" before and the message gets written
			 */
			if (sms->Number[1]!='+' && (sms->Number[1]<'0' || sms->Number[1]>'9')) {
		        	sprintf(buffer, "AT+CMGW=\"123\",,\"%s\"\r",statetxt);
			} else {
		        	sprintf(buffer, "AT+CMGW=\"%s\",,\"%s\"\r",DecodeUnicodeString(sms->Number),statetxt);
			}
		} else {
			sprintf(buffer, "AT+CMGW=\"%s\",,\"%s\"\r",DecodeUnicodeString(sms->Number),statetxt);
		}
	}

	Phone->SaveSMSMessage = sms;

	for (reply=0;reply<s->ReplyNum;reply++) {
		if (reply!=0) {
			if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR ||
			    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl==DL_TEXTERRORDATE) {
			    smprintf(s, "[Retrying %i]\n", reply+1);
			}
		}
		s->Protocol.Data.AT.EditMode 	= true;
		Replies 			= s->ReplyNum;
		s->ReplyNum			= 1;
		smprintf(s,"Waiting for modem prompt\n");
		ATGEN_WaitFor(s, buffer, strlen(buffer), 0x00, 3, ID_SaveSMSMessage);
		s->ReplyNum			 = Replies;
		if (error == ERR_NONE) {
			Phone->DispatchError 	= ERR_TIMEOUT;
			Phone->RequestID 	= ID_SaveSMSMessage;
			smprintf(s, "Saving SMS\n");
			error = s->Protocol.Functions->WriteMessage(s, hexreq, current2, 0x00);
			if (error!=ERR_NONE) return error;
			my_sleep(500);
			/* CTRL+Z ends entering */
			error = s->Protocol.Functions->WriteMessage(s, "\x1A", 1, 0x00);
			if (error!=ERR_NONE) return error;
			GSM_WaitForOnce(s, NULL, 0x00, 0x00, 4);
			if (error != ERR_TIMEOUT) return error;
		} else {
			smprintf(s, "Escaping SMS mode\n");
			error2 = s->Protocol.Functions->WriteMessage(s, "\x1B\r", 2, 0x00);
			if (error2 != ERR_NONE) return error2;
			return error;
		}
        }

	return Phone->DispatchError;
}

GSM_Error ATGEN_ReplySendSMS(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	char			*start;

	if (s->Protocol.Data.AT.EditMode) {
		if (s->Phone.Data.Priv.ATGEN.ReplyState != AT_Reply_SMSEdit) {
			return ERR_UNKNOWN;
		}
		s->Protocol.Data.AT.EditMode = false;
		return ERR_NONE;
	}

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
 		smprintf(s, "SMS sent OK\n");
 		if (s->User.SendSMSStatus!=NULL) {
			start = strstr(msg.Buffer, "+CMGS: ");
			if (start != NULL) {
				s->User.SendSMSStatus(s,0,atoi(start+7));
			} else {
				s->User.SendSMSStatus(s,0,-1);
			}
		}
		return ERR_NONE;
	case AT_Reply_CMSError:
 		smprintf(s, "Error %i\n",Priv->ErrorCode);
 		if (s->User.SendSMSStatus != NULL) {
			s->User.SendSMSStatus(s, Priv->ErrorCode, -1);
		}
 		return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
 		smprintf(s, "Error %i\n",Priv->ErrorCode);
 		if (s->User.SendSMSStatus != NULL) {
			s->User.SendSMSStatus(s, Priv->ErrorCode, -1);
		}
		return ATGEN_HandleCMEError(s);
	case AT_Reply_Error:
 		if (s->User.SendSMSStatus != NULL) {
			s->User.SendSMSStatus(s, -1, -1);
		}
		return ERR_UNKNOWN;
	default:
 		if (s->User.SendSMSStatus != NULL) {
			s->User.SendSMSStatus(s, -1, -1);
		}
		return ERR_UNKNOWNRESPONSE;
	}
}

GSM_Error ATGEN_SendSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	GSM_Error 		error,error2;
	int			current, current2, Replies;
	unsigned char		buffer[1000], hexreq[1000];
	GSM_Phone_Data		*Phone = &s->Phone.Data;

	if (sms->PDU == SMS_Deliver) sms->PDU = SMS_Submit;

	error = ATGEN_MakeSMSFrame(s, sms, hexreq, &current, &current2);
	if (error != ERR_NONE) return error;

	switch (Phone->Priv.ATGEN.SMSMode) {
	case SMS_AT_PDU:
		sprintf(buffer, "AT+CMGS=%i\r",current);
		break;
	case SMS_AT_TXT:
		sprintf(buffer, "AT+CMGS=\"%s\"\r",DecodeUnicodeString(sms->Number));
	}

	s->Protocol.Data.AT.EditMode 	= true;
	Replies 			= s->ReplyNum;
	s->ReplyNum			= 1;
	smprintf(s,"Waiting for modem prompt\n");
	ATGEN_WaitFor(s, buffer, strlen(buffer), 0x00, 3, ID_IncomingFrame);
	s->ReplyNum			 = Replies;
	if (error == ERR_NONE) {
		smprintf(s, "Sending SMS\n");
		error = s->Protocol.Functions->WriteMessage(s, hexreq, current2, 0x00);
		if (error!=ERR_NONE) return error;
		my_sleep(500);
		/* CTRL+Z ends entering */
		error=s->Protocol.Functions->WriteMessage(s, "\x1A", 1, 0x00);
		my_sleep(100);
		return error;
	} else {
		smprintf(s, "Escaping SMS mode\n");
		error2=s->Protocol.Functions->WriteMessage(s, "\x1B\r", 2, 0x00);
		if (error2 != ERR_NONE) return error2;
	}
	return error;
}

GSM_Error ATGEN_SendSavedSMS(GSM_StateMachine *s, int Folder, int Location)
{
	GSM_Error 	error;
	int		location;
	unsigned char	smsfolder;
	unsigned char	req[100];
	GSM_MultiSMSMessage	msms;

	msms.Number = 0;
	msms.SMS[0].Folder 	= Folder;
	msms.SMS[0].Location 	= Location;

	/* By reading SMS we check if it is really inbox/outbox */
	error = ATGEN_GetSMS(s, &msms);
	if (error != ERR_NONE) return error;

	/* Can not send from other folder that outbox */
	if (msms.SMS[0].Folder != 2 && msms.SMS[0].Folder != 4) return ERR_NOTSUPPORTED;

	error=ATGEN_GetSMSLocation(s, &msms.SMS[0], &smsfolder, &location, false);
	if (error != ERR_NONE) return error;

	sprintf(req, "AT+CMSS=%i\r",location);
	return s->Protocol.Functions->WriteMessage(s, req, strlen(req), 0x00);
}

GSM_Error ATGEN_ReplyGetDateTime(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		return ATGEN_ParseReply(s,
				GetLineString(msg.Buffer, Priv->Lines, 2),
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
	char *str;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		/* Try simple date string as alarm */
		error = ATGEN_ParseReply(s,
				GetLineString(msg.Buffer, Priv->Lines, 2),
				"+CALA: @d",
				&(Data->Alarm->DateTime));
		if (error == ERR_NONE) {
			if (Data->Alarm->Location != 1) return ERR_INVALIDLOCATION;
			return ERR_NONE;
		}

		/* Ok we have something more complex, try to handle it */
		i = 2;
		/* Need to scan over all reply lines */
		while (strcmp("OK", str = GetLineString(msg.Buffer, Priv->Lines, i)) != 0) {
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
					Data->Alarm->Repeating = true;
				} else {
					Data->Alarm->Repeating = false;
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

GSM_Error ATGEN_PrivSetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time, bool set_timezone)
{
	char			tz[4] = "";
	char			req[128];
	GSM_Error		error;

	if (set_timezone) {
		sprintf(tz, "+%02i", date_time->Timezone);
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
		error = ATGEN_PrivSetDateTime(s, date_time, false);
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
	return ATGEN_PrivSetDateTime(s, date_time, true);
}

GSM_Error ATGEN_GetAlarm(GSM_StateMachine *s, GSM_Alarm *alarm)
{
	GSM_Error		error;
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;

	/* If phone encodes also values in command, we need normal charset */
	if (Priv->EncodedCommands) {
		error = ATGEN_SetCharset(s, AT_PREF_CHARSET_NORMAL);
		if (error != ERR_NONE) return error;
	}

	s->Phone.Data.Alarm = alarm;
	smprintf(s, "Getting alarm\n");
	ATGEN_WaitFor(s, "AT+CALA?\r", 9, 0x00, 4, ID_GetAlarm);

	return error;
}

/* R320 only takes HH:MM. Do other phones understand full date? */
GSM_Error ATGEN_SetAlarm(GSM_StateMachine *s, GSM_Alarm *alarm)
{
	char			req[20];
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error		error;

	if (alarm->Location != 1) return ERR_INVALIDLOCATION;

	/* If phone encodes also values in command, we need normal charset */
	if (Priv->EncodedCommands) {
		error = ATGEN_SetCharset(s, AT_PREF_CHARSET_NORMAL);
		if (error != ERR_NONE) return error;
	}

	sprintf(req, "AT+CALA=\"%02i:%02i\"\r",alarm->DateTime.Hour,alarm->DateTime.Minute);

	smprintf(s, "Setting Alarm\n");
	ATGEN_WaitFor(s, req, strlen(req), 0x00, 3, ID_SetAlarm);

	return error;
}

GSM_Error ATGEN_ReplyGetSMSC(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_SMSC		*SMSC = s->Phone.Data.SMSC;
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error		error;
	int number_type;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "SMSC info received\n");

		/* Parse reply */
		error = ATGEN_ParseReply(s,
					GetLineString(msg.Buffer, Priv->Lines, 2),
					"+CSCA: @p, @i",
					SMSC->Number, sizeof(SMSC->Number),
					&number_type);

		/* International number */
		GSM_TweakInternationalNumber(SMSC->Number, number_type);

		/* Some things we can not find out */
		SMSC->Format 		= SMS_FORMAT_Text;
		SMSC->Validity.Format = SMS_Validity_RelativeFormat;
		SMSC->Validity.Relative	= SMS_VALID_Max_Time;
		SMSC->Name[0]		= 0;
		SMSC->Name[1]		= 0;
		SMSC->DefaultNumber[0]	= 0;
		SMSC->DefaultNumber[1]	= 0;
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

GSM_Error ATGEN_GetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	GSM_Error error;
	const char req[] = "AT+CSCA?\r";

	/* Only one location supported */
	if (smsc->Location != 1) {
		return ERR_INVALIDLOCATION;
	}

	/* We prefer normal charset */
	error = ATGEN_SetCharset(s, AT_PREF_CHARSET_NORMAL);
	if (error != ERR_NONE) return error;

	/* Issue command */
	s->Phone.Data.SMSC = smsc;
	smprintf(s, "Getting SMSC\n");
	ATGEN_WaitFor(s, req, strlen(req), 0x00, 4, ID_GetSMSC);

	return error;
}

GSM_Error ATGEN_ReplyGetNetworkLAC_CID(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_NetworkInfo		*NetworkInfo = s->Phone.Data.NetworkInfo;
	GSM_Lines		Lines;
	int			i=0;
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	char			*answer;
	char			*tmp;
	size_t pos;

  	if (s->Phone.Data.RequestID == ID_IncomingFrame) {
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

	SplitLines(GetLineString(msg.Buffer,Priv->Lines,2),
		GetLineLength(msg.Buffer,Priv->Lines,2),
		&Lines, ",", 1, true);

	/* Find number of lines */
	while (Lines.numbers[i*2+1] != 0) {
		/* FIXME: handle special chars correctly */
		tmp = strdup(GetLineString(msg.Buffer,Priv->Lines,2));
		smprintf(s, "%i \"%s\"\n",i+1,GetLineString(tmp,Lines,i+1));
		free(tmp);
		i++;
	}

	smprintf(s, "Network LAC & CID & state received\n");
	tmp = strdup(GetLineString(msg.Buffer,Priv->Lines,2));
	answer = GetLineString(tmp,Lines,2);
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

		if (Lines.numbers[3*2+1]==0) return ERR_NONE;

 		tmp = strdup(GetLineString(msg.Buffer,Priv->Lines,2));
 		answer = GetLineString(tmp,Lines,3);
 		free(tmp);
		while (*answer == 0x20) answer++;
		if (*answer == '"') answer++;
		pos = 0;
		while (*answer != '"' && *answer != ',' && *answer != 0 && *answer != '\n') {
			NetworkInfo->LAC[pos++] = *answer;
			answer++;
			if (pos >= sizeof(NetworkInfo->LAC)) {
				smprintf(s, "LAC too big!\n");
				return ERR_MOREMEMORY;
			}
		}
		NetworkInfo->LAC[pos++] = 0;

 		tmp = strdup(GetLineString(msg.Buffer,Priv->Lines,2));
 		answer = GetLineString(tmp,Lines,4);
 		free(tmp);
		while (*answer == 0x20) answer++;
		if (*answer == '"') answer++;
		pos = 0;
		while (*answer != '"' && *answer != ',' && *answer != 0 && *answer != '\n') {
			NetworkInfo->CID[pos++] = *answer;
			answer++;
			if (pos >= sizeof(NetworkInfo->CID)) {
				smprintf(s, "CID too big!\n");
				return ERR_MOREMEMORY;
			}
		}
		NetworkInfo->CID[pos++] = 0;

		smprintf(s, "LAC   : %s\n",NetworkInfo->LAC);
		smprintf(s, "CID   : %s\n",NetworkInfo->CID);
	}
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
				GetLineString(msg.Buffer, Priv->Lines, 2),
				"+COPS: @i, @i, @r",
				&i, /* Mode, ignored for now */
				&i, /* Format of reply, we set this */
				NetworkInfo->NetworkCode, sizeof(NetworkInfo->NetworkCode));

		if (error != ERR_NONE) {
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

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Network name received\n");
		return ATGEN_ParseReply(s,
				GetLineString(msg.Buffer, Priv->Lines, 2),
				"+COPS: @i, @i, @s",
				&i, /* Mode, ignored for now */
				&i, /* Format of reply, we set this */
				NetworkInfo->NetworkName, sizeof(NetworkInfo->NetworkName));
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
	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
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

	if (strlen(msg.Buffer) > AT_PBK_MAX_MEMORIES) {
		smprintf(s, "ERROR: Too long phonebook memories information received! (Recevided %zd, AT_PBK_MAX_MEMORIES is %d\n", strlen(msg.Buffer), AT_PBK_MAX_MEMORIES);
		return ERR_MOREMEMORY;
	}
	strcpy(s->Phone.Data.Priv.ATGEN.PBKMemories, msg.Buffer);
	smprintf(s, "PBK memories received: %s\n", s->Phone.Data.Priv.ATGEN.PBKMemories);
	return ERR_NONE;
}

GSM_Error ATGEN_SetPBKMemory(GSM_StateMachine *s, GSM_MemoryType MemType)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	char 			req[] = "AT+CPBS=\"XX\"\r";
	GSM_Error		error;

	if (Priv->PBKMemory == MemType) return ERR_NONE;

	/* Zero values that are for actual memory */
	Priv->MemorySize		= 0;
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
			 * guess that phone supports all. This is tru at least
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
		        if (strstr(Priv->PBKMemories,"ME")==NULL) return ERR_NOTSUPPORTED;
			req[9] = 'M'; req[10] = 'E';
			break;
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
			break;
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
	GSM_MemoryStatus	*MemoryStatus = s->Phone.Data.MemoryStatus;
	char 			*start;

	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Memory status received\n");
  		MemoryStatus->MemoryUsed = 0;
		MemoryStatus->MemoryFree = 0;
		start = strchr(msg.Buffer, ',');
		if (start) {
			start++;
			MemoryStatus->MemoryUsed = atoi(start);
			start = strchr(start, ',');
			if (start) {
				start++;
				MemoryStatus->MemoryFree = atoi(start) - MemoryStatus->MemoryUsed;
				return ERR_NONE;
			} else return ERR_UNKNOWN;
		} else return ERR_UNKNOWN;
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
 *
 * Some phones (eg. Motorola C350) reply is different:
 * \verbatim
 * +CPBR: (first-last),max_number_len,max_name_len
 * \endverbatim
 *
 * Some phones do not list positions (Sharp):
 * \verbatim
 * +CPBR: (),max_number_len,max_name_len
 * \endverbatim
 *
 * \todo
 * We currently guess memory size for Sharp to 1000.
 */
GSM_Error ATGEN_ReplyGetCPBRMemoryInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
 	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	char 			*pos;
	char *tmppos;

 	switch (Priv->ReplyState) {
 	case AT_Reply_OK:
		smprintf(s, "Memory info received\n");

		/* Parse first location */
		pos = strchr(msg.Buffer, '(');
 		if (!pos) {
 			pos = strchr(msg.Buffer, ':');
 			if (!pos) {
				/* We don't get reply on first attempt on Samsung */
				if (Priv->Manufacturer == AT_Samsung) {
					return ERR_NONE;
				}
				return ERR_UNKNOWNRESPONSE;
			}
 			pos++;
 			if (*pos ==  ' ') pos++;
 			if (!isdigit(*pos)) return ERR_UNKNOWNRESPONSE;
 		} else {
 			pos++;
 		}
		Priv->FirstMemoryEntry = atoi(pos);

		/* Parse last location*/
		tmppos = pos;
		pos = strchr(pos, '-');
 		if (!pos) {
			/* Sharp does not show locations */
			if (*tmppos == ')') {
				pos = tmppos + 1;
				Priv->FirstMemoryEntry = 1;
				Priv->MemorySize = 1000;
				return ERR_NONE;
			}
			/* There might be also only one location */
			pos = strchr(pos, ')');
			if (pos != NULL) {
				Priv->MemorySize = 1;
				return ERR_NONE;
			}

			return ERR_UNKNOWNRESPONSE;
		} else {
			pos++;
			Priv->MemorySize = atoi(pos) + 1 - Priv->FirstMemoryEntry;
		}

		/* Parse number length*/
		pos = strchr(pos, ',');
 		if (!pos) return ERR_UNKNOWNRESPONSE;
		pos++;
		Priv->NumberLength = atoi(pos);

		/* Parse text length*/
		pos = strchr(pos, ',');
 		if (!pos) return ERR_UNKNOWNRESPONSE;
		pos++;
		Priv->TextLength = atoi(pos);

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

GSM_Error ATGEN_ReplyGetCPBRMemoryStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_MemoryStatus	*MemoryStatus = s->Phone.Data.MemoryStatus;
	int			line=0;
	char			*str;
	int			cur, last = -1;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Memory entries received\n");
		/* Walk through lines with +CPBR: */
		while (Priv->Lines.numbers[line*2+1]!=0) {
			str = GetLineString(msg.Buffer,Priv->Lines,line+1);
			if (strncmp(str, "+CPBR: ", 7) == 0) {
				if (sscanf(str, "+CPBR: %d,", &cur) == 1) {
					/* Some phones wrongly return several lines with same location,
					 * we need to catch it here to get correct count. */
					if (cur != last) {
						MemoryStatus->MemoryUsed++;
					}
					last = cur;
					cur -= Priv->FirstMemoryEntry - 1;
					if (cur == Priv->NextMemoryEntry || Priv->NextMemoryEntry == 0)
						Priv->NextMemoryEntry = cur + 1;
				} else {
					MemoryStatus->MemoryUsed++;
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

GSM_Error ATGEN_GetMemoryInfo(GSM_StateMachine *s, GSM_MemoryStatus *Status, GSM_AT_NeededMemoryInfo NeededInfo)
{
	GSM_Error		error;
	char			req[20];
	int			start;
	int			end;
	int			memory_end;
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;

	smprintf(s, "Getting memory information\n");

	Priv->MemorySize		= 0;
	Priv->TextLength		= 0;
	Priv->NumberLength		= 0;

	ATGEN_WaitFor(s, "AT+CPBR=?\r", 10, 0x00, 4, ID_GetMemoryStatus);
	/* We don't get reply on first attempt on Samsung */
	if (Priv->Manufacturer == AT_Samsung) {
		ATGEN_WaitFor(s, "", 0, 0x00, 4, ID_GetMemoryStatus);
	}
	if (error != ERR_NONE) return error;
	if (NeededInfo == AT_Total || NeededInfo == AT_Sizes || NeededInfo == AT_First) return ERR_NONE;

	smprintf(s, "Getting memory status by reading values\n");

	s->Phone.Data.MemoryStatus	= Status;
	Status->MemoryUsed		= 0;
	Status->MemoryFree		= 0;
	start				= Priv->FirstMemoryEntry;
	Priv->NextMemoryEntry		= 0;
	memory_end = Priv->MemorySize + Priv->FirstMemoryEntry - 1;
	while (1) {
		/* Calculate end of next request */
		end	= start + 20;
		if (end > memory_end)
			end = memory_end;

		/* Read next interval */
		sprintf(req, "AT+CPBR=%i,%i\r", start, end);
		ATGEN_WaitFor(s, req, strlen(req), 0x00, 4, ID_GetMemoryStatus);
		if (error != ERR_NONE) return error;

		/* Do we already have first empty record? */
		if (NeededInfo == AT_NextEmpty &&
				Priv->NextMemoryEntry != 0 &&
				Priv->NextMemoryEntry != end + 1)
			return ERR_NONE;

		/* Did we hit memory end? */
		if (end == memory_end) {
			Status->MemoryFree = Priv->MemorySize - Status->MemoryUsed;
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

	s->Phone.Data.MemoryStatus=Status;

	/* in some phones doesn't work or doesn't return memory status inside */
	/* Some workaround for buggy mobile, that hangs after "AT+CPBS?" for other
	 * memory than SM.
	 */
	if (!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_BROKENCPBS) || (Status->MemoryType == MEM_SM)) {
		smprintf(s, "Getting memory status\n");
		ATGEN_WaitFor(s, "AT+CPBS?\r", 9, 0x00, 4, ID_GetMemoryStatus);
		if (error == ERR_NONE) return ERR_NONE;
	}

	/* Catch errorneous 0 returned by some Siemens phones for ME. There is
	 * probably no way to get status there. */
	if (Priv->PBKSBNR == AT_SBNR_AVAILABLE && Status->MemoryType == MEM_ME && Status->MemoryFree == 0)
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
		if (strcmp("OK", GetLineString(msg.Buffer, Priv->Lines, 2)) == 0) {
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
					GetLineString(msg.Buffer, Priv->Lines, 2),
					"+CPBR: @i, @p, @i, @s",
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

		/* Try reply with call date */
		error = ATGEN_ParseReply(s,
					GetLineString(msg.Buffer, Priv->Lines, 2),
					"+CPBR: @i, @p, @i, @s, @d",
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
					GetLineString(msg.Buffer, Priv->Lines, 2),
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
	unsigned char		req[20];
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;

	if (entry->Location==0x00) return ERR_INVALIDLOCATION;

	if (entry->MemoryType == MEM_ME) {
		if (Priv->PBKSBNR == 0) {
			sprintf(req, "AT^SBNR=?\r");
			smprintf(s, "Checking availablity of SBNR\n");
			ATGEN_WaitFor(s, req, strlen(req), 0x00, 4, ID_GetMemory);
		}
		if (Priv->PBKSBNR == AT_SBNR_AVAILABLE) {
			sprintf(req, "AT^SBNR=vcf,%i\r",entry->Location + Priv->FirstMemoryEntry - 1);
			s->Phone.Data.Memory=entry;
			smprintf(s, "Getting phonebook entry\n");
			ATGEN_WaitFor(s, req, strlen(req), 0x00, 4, ID_GetMemory);

			return error;
		}
	}

	error=ATGEN_GetManufacturer(s);
	if (error != ERR_NONE) return error;

	error=ATGEN_SetPBKMemory(s, entry->MemoryType);
	if (error != ERR_NONE) return error;

	if (Priv->FirstMemoryEntry == -1) {
		error = ATGEN_GetMemoryInfo(s, NULL, AT_First);
		if (error != ERR_NONE) return error;
	}


	error=ATGEN_SetCharset(s, AT_PREF_CHARSET_UNICODE); /* For reading we prefer unicode */
	if (error != ERR_NONE) return error;

	if (endlocation == 0) {
		sprintf(req, "AT+CPBR=%i\r", entry->Location + Priv->FirstMemoryEntry - 1);
	} else {
		sprintf(req, "AT+CPBR=%i,%i\r", entry->Location + Priv->FirstMemoryEntry - 1, endlocation + Priv->FirstMemoryEntry - 1);
	}

	s->Phone.Data.Memory=entry;
	smprintf(s, "Getting phonebook entry\n");
	ATGEN_WaitFor(s, req, strlen(req), 0x00, 4, ID_GetMemory);

	return error;
}

GSM_Error ATGEN_GetMemory (GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	return ATGEN_PrivGetMemory(s, entry, 0);
}

GSM_Error ATGEN_GetNextMemory (GSM_StateMachine *s, GSM_MemoryEntry *entry, bool start)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error		error;
	int			step = 0;

	if (Priv->MemorySize == 0) {
		error = ATGEN_GetMemoryInfo(s, NULL, AT_Total);
		if (error != ERR_NONE) return error;
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
		if (entry->MemoryType != MEM_ME || Priv->PBKSBNR != AT_SBNR_AVAILABLE) step = MIN(step + 2, 20);
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
	if (error != ERR_NONE) return error;

	sprintf(req, format, s->Phone.Data.EnableIncomingUSSD ? 1 : 0, number);

	ATGEN_WaitFor(s, req, strlen(req), 0x00, 30, ID_GetUSSD);

	return error;
}

GSM_Error ATGEN_DialVoice(GSM_StateMachine *s, char *number, GSM_CallShowNumber ShowNumber)
{
	char req[39] = "ATDT";
	GSM_Error error;

	if (ShowNumber != GSM_CALL_DefaultNumberPresence) return ERR_NOTSUPPORTED;
	if (strlen(number) > 32) return (ERR_UNKNOWN);

	strcat(req, number);
	strcat(req, ";\r");

	smprintf(s, "Making voice call\n");
	ATGEN_WaitFor(s, req, 4+2+strlen(number), 0x00, 5, ID_DialVoice);

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

	switch (Code.Type) {
	case SEC_Pin :
		sprintf(req, "AT+CPIN=\"%s\"\r" , Code.Code);
		break;
	case SEC_Pin2 :
		if (s->Phone.Data.Priv.ATGEN.Manufacturer == AT_Siemens) {
			sprintf(req, "AT+CPIN2=\"%s\"\r", Code.Code);
		} else {
			sprintf(req, "AT+CPIN=\"%s\"\r" , Code.Code);
		}
		break;
	default : return ERR_NOTIMPLEMENTED;
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
 	 * Because of it ReplyGetSecurityStatus is called after receiving line
	 * with +CPIN:
	 */
	ATGEN_WaitFor(s, "AT+CPIN?\r", 9, 0x00, 4, ID_GetSecurityStatus);
	return error;
}

GSM_Error ATGEN_AnswerCall(GSM_StateMachine *s, int ID UNUSED, bool all)
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
            call.CallIDAvailable = false;
            call.Status 	 = GSM_CALL_CallLocalEnd;
            if (s->User.IncomingCall) s->User.IncomingCall(s, call);

            return ERR_NONE;
    	case AT_Reply_CMSError:
            return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(s);
        default:
    	    return ERR_UNKNOWN;
	}
}

GSM_Error ATGEN_CancelCall(GSM_StateMachine *s, int ID UNUSED, bool all)
{
	GSM_Error error;

	if (all) {
		smprintf(s, "Dropping all calls\n");
		ATGEN_WaitFor(s, "ATH\r", 4, 0x00, 4, ID_CancelCall);
		if (error == ERR_UNKNOWN) {
			ATGEN_WaitFor(s, "AT+CHUP\r", 8, 0x00, 4, ID_CancelCall);
			return error;
		}
		return error;
	}
	return ERR_NOTSUPPORTED;
}

GSM_Error ATGEN_ReplyReset(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Reset done\n");
	return ERR_NONE;
}

GSM_Error ATGEN_Reset(GSM_StateMachine *s, bool hard)
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
	unsigned char 	req[80] = "AT+VTS=";
	int 		n;
	GSM_Error error;

	for (n = 0; n < 32; n++) {
		if (sequence[n] == '\0') break;
		if (n != 0) req[6 + 2 * n] = ',';
		req[7 + 2 * n] = sequence[n];
	}

	strcat(req, ";\r");

	smprintf(s, "Sending DTMF\n");
	ATGEN_WaitFor(s, req, 7+2+2*strlen(sequence), 0x00, 4, ID_SendDTMF);

	return error;
}

GSM_Error ATGEN_ReplyDeleteSMSMessage(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "SMS deleted OK\n");
		return ERR_NONE;
	case AT_Reply_Error:
		smprintf(s, "Invalid location\n");
		return ERR_INVALIDLOCATION;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
	        return ATGEN_HandleCMEError(s);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_DeleteSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	unsigned char		req[20], folderid;
	GSM_Error		error;
	int			location;
	GSM_MultiSMSMessage	msms;

	msms.Number = 0;
	msms.SMS[0] = *sms;

	/* By reading SMS we check if it is really inbox/outbox */
	error = ATGEN_GetSMS(s, &msms);
	if (error != ERR_NONE) return error;

	error = ATGEN_GetSMSLocation(s, sms, &folderid, &location, true);
	if (error != ERR_NONE) return error;

	sprintf(req, "AT+CMGD=%i\r",location);

	smprintf(s, "Deleting SMS\n");
	ATGEN_WaitFor(s, req, strlen(req), 0x00, 5, ID_DeleteSMSMessage);

	return error;
}

GSM_Error ATGEN_GetSMSFolders(GSM_StateMachine *s, GSM_SMSFolders *folders)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error 		error;
	int			used = 0;

	if (Priv->PhoneSMSMemory == 0) {
		error = ATGEN_SetSMSMemory(s, false, false);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) return error;
	}
	if (Priv->SIMSMSMemory == 0) {
		error = ATGEN_SetSMSMemory(s, true, false);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) return error;
	}

	folders->Number = 0;
	if (Priv->PhoneSMSMemory == AT_NOTAVAILABLE && Priv->SIMSMSMemory == AT_NOTAVAILABLE) {
		return ERR_NONE;
	}

	PHONE_GetSMSFolders(s,folders);

	if (Priv->SIMSMSMemory == AT_AVAILABLE) {
		used = 2;
	}

	if (Priv->PhoneSMSMemory == AT_AVAILABLE) {
		if (used != 0) {
			CopyUnicodeString(folders->Folder[used    ].Name,folders->Folder[0].Name);
			CopyUnicodeString(folders->Folder[used + 1].Name,folders->Folder[1].Name);
			folders->Folder[used    ].InboxFolder 	= folders->Folder[0].InboxFolder;
			folders->Folder[used + 1].InboxFolder 	= folders->Folder[1].InboxFolder;
			folders->Folder[used    ].OutboxFolder 	= folders->Folder[0].OutboxFolder;
			folders->Folder[used + 1].OutboxFolder 	= folders->Folder[1].OutboxFolder;
		}
		folders->Folder[used    ].Memory 	= MEM_ME;
		folders->Folder[used + 1].Memory 	= MEM_ME;
		folders->Number += 2;
		used += 2;
	}

	return ERR_NONE;
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
	int			Group, Name, Number,NumberType=0, len;
	GSM_Error 		error;
	unsigned char		req[REQUEST_SIZE + 1];
	unsigned char		name[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
	unsigned char		uname[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
	unsigned char		number[GSM_PHONEBOOK_TEXT_LENGTH + 1];
	int			reqlen, i;
	GSM_AT_Charset_Preference	Prefer = AT_PREF_CHARSET_NORMAL;

	if (entry->Location == 0) return ERR_INVALIDLOCATION;

	error = ATGEN_SetPBKMemory(s, entry->MemoryType);
	if (error != ERR_NONE) return error;

	for (i=0;i<entry->EntriesNum;i++) {
		entry->Entries[i].AddError = ERR_NOTSUPPORTED;
	}

	GSM_PhonebookFindDefaultNameNumberGroup(entry, &Name, &Number, &Group);

	name[0] = 0;
	if (Name != -1) {
		len = UnicodeLength(entry->Entries[Name].Text);

		/* Compare if we would loose some information when not using
		 * unicode */
		EncodeDefault(name, entry->Entries[Name].Text, &len, true, NULL);
		DecodeDefault(uname, name, len, true, NULL);
		if (!mywstrncmp(uname, entry->Entries[Name].Text, len)) {
			/* Get maximal text length */
			if (Priv->TextLength == 0) {
				ATGEN_GetMemoryInfo(s, NULL, AT_Sizes);
			}

			/* I char stored in GSM alphabet takes 7 bits, one
			 * unicode 16, if storing in unicode would truncate
			 * text, do not use it, otherwise we will use it */
			if ((Priv->TextLength != 0) && ((Priv->TextLength * 7 / 16) <= len)) {
				Prefer = AT_PREF_CHARSET_NORMAL;
			} else {
				Prefer = AT_PREF_CHARSET_UNICODE;
			}
		}

		error = ATGEN_SetCharset(s, Prefer);
		if (error != ERR_NONE) return error;

		switch (Priv->Charset) {
		case AT_CHARSET_HEX:
			EncodeDefault(uname, entry->Entries[Name].Text, &len, true, NULL);
			EncodeHexBin(name, uname, len);
			len = strlen(name);
			break;
		case AT_CHARSET_GSM:
			smprintf(s, "str: %s\n", DecodeUnicodeString(entry->Entries[Name].Text));
			len = UnicodeLength(entry->Entries[Name].Text);
			EncodeDefault(name, entry->Entries[Name].Text, &len, true, NULL);
			break;
		case AT_CHARSET_UCS2:
			EncodeHexUnicode(name, entry->Entries[Name].Text, UnicodeLength(entry->Entries[Name].Text));
			len = strlen(name);
			break;
  		case AT_CHARSET_IRA:
			return ERR_NOTSUPPORTED;
  		case AT_CHARSET_UTF8:
			EncodeUTF8(name, entry->Entries[Name].Text);
			len = strlen(name);
  			break;
#ifdef ICONV_FOUND
  		case AT_CHARSET_PCCP437:
			len = UnicodeLength(entry->Entries[Name].Text);
			IconvEncode("CP437", entry->Entries[Name].Text, len, name, sizeof(name));
			break;
  		case AT_CHARSET_ISO88591:
			len = UnicodeLength(entry->Entries[Name].Text);
			IconvEncode("ISO-8859-1", entry->Entries[Name].Text, len, name, sizeof(name));
			break;
  		case AT_CHARSET_ISO88592:
			len = UnicodeLength(entry->Entries[Name].Text);
			IconvEncode("ISO-8859-2", entry->Entries[Name].Text, len, name, sizeof(name));
			break;
  		case AT_CHARSET_ISO88593:
			len = UnicodeLength(entry->Entries[Name].Text);
			IconvEncode("ISO-8859-3", entry->Entries[Name].Text, len, name, sizeof(name));
			break;
  		case AT_CHARSET_ISO88594:
			len = UnicodeLength(entry->Entries[Name].Text);
			IconvEncode("ISO-8859-4", entry->Entries[Name].Text, len, name, sizeof(name));
			break;
  		case AT_CHARSET_ISO88595:
			len = UnicodeLength(entry->Entries[Name].Text);
			IconvEncode("ISO-8859-5", entry->Entries[Name].Text, len, name, sizeof(name));
			break;
  		case AT_CHARSET_ISO88596:
			len = UnicodeLength(entry->Entries[Name].Text);
			IconvEncode("ISO-8859-6", entry->Entries[Name].Text, len, name, sizeof(name));
			break;
#else
		case AT_CHARSET_PCCP437:
			/* FIXME: correctly decode PCCP437 */
			smprintf(s, "str: %s\n", DecodeUnicodeString(entry->Entries[Name].Text));
			len = UnicodeLength(entry->Entries[Name].Text);
			EncodeDefault(name, entry->Entries[Name].Text, &len, true, NULL);
			break;
#endif
		default:
			smprintf(s, "Unsupported charset! (%d)\n", Priv->Charset);
			return ERR_SOURCENOTAVAILABLE;
		}
		entry->Entries[Name].AddError = ERR_NONE;
	} else {
		smprintf(s, "WARNING: No usable name found!\n");
		len = 0;
	}

	if (Number != -1) {
		GSM_PackSemiOctetNumber(entry->Entries[Number].Text, number, false);
		NumberType = number[0];
		sprintf(number,"%s",DecodeUnicodeString(entry->Entries[Number].Text));
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
	sprintf(req, "AT+CPBW=%d, \"%s\", %i, \"", entry->Location + Priv->FirstMemoryEntry - 1, number, NumberType);
	reqlen = strlen(req);
	if (reqlen + len > REQUEST_SIZE - 2) {
		smprintf(s, "WARNING: Text truncated to fit in buffer!\n");
		len = REQUEST_SIZE - 2 - reqlen;
	}
	memcpy(req + reqlen, name, len);
	reqlen += len;
	memcpy(req + reqlen, "\"\r", 2);
	reqlen += 2;

	smprintf(s, "Writing phonebook entry\n");
	ATGEN_WaitFor(s, req, reqlen, 0x00, 4, ID_SetMemory);

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

GSM_Error ATGEN_SetIncomingCall(GSM_StateMachine *s, bool enable)
{
	GSM_Error error;
	if (enable) {
		smprintf(s, "Enabling incoming call\n");
		/* SE phones are fucked up when we want to see CLIP information */
		if (s->Phone.Data.Priv.ATGEN.Manufacturer != AT_Ericsson) {
			ATGEN_WaitFor(s, "AT+CLIP=1\r", 10, 0x00, 3, ID_SetIncomingCall);
			if (error != ERR_NONE) return error;
			ATGEN_WaitFor(s, "AT+CRC=1\r", 9, 0x00, 3, ID_SetIncomingCall);
			if (error != ERR_NONE) return error;
		} else {
			ATGEN_WaitFor(s, "AT+CRC=0\r", 9, 0x00, 3, ID_SetIncomingCall);
			if (error != ERR_NONE) return error;
		}
	} else {
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

GSM_Error ATGEN_ReplyIncomingCallInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Call 		call;

	memset(&call, 0, sizeof(call));

	smprintf(s, "Incoming call info\n");
	if (s->Phone.Data.EnableIncomingCall && s->User.IncomingCall!=NULL) {
		call.CallIDAvailable 	= false;
		if (strstr(msg.Buffer, "RING")) {
			smprintf(s, "Ring detected - ");
			/* We ignore RING for most phones, see ATGEN_SetIncomingCall */
			if (s->Phone.Data.Priv.ATGEN.Manufacturer != AT_Ericsson) {
				smprintf(s, "ignoring\n");
				return ERR_NONE;
			}
			smprintf(s, "generating event\n");
			call.Status = GSM_CALL_IncomingCall;
			ATGEN_Extract_CLIP_number(s, call.PhoneNumber, sizeof(call.PhoneNumber), msg.Buffer);
		} else if (strstr(msg.Buffer, "CLIP:")) {
			smprintf(s, "CLIP detected\n");
			call.Status = GSM_CALL_IncomingCall;
			ATGEN_Extract_CLIP_number(s, call.PhoneNumber, sizeof(call.PhoneNumber), msg.Buffer);
		} else if (strstr(msg.Buffer, "NO CARRIER")) {
			smprintf(s, "Call end detected\n");
			call.Status = GSM_CALL_CallEnd;
		} else if (strstr(msg.Buffer, "COLP:")) {
			smprintf(s, "CLIP detected\n");
			call.Status = GSM_CALL_CallStart;
			ATGEN_Extract_CLIP_number(s, call.PhoneNumber, sizeof(call.PhoneNumber), msg.Buffer);
		} else {
			smprintf(s, "Incoming call error\n");
			return ERR_NONE;
		}

		s->User.IncomingCall(s, call);
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
		CopyLineString(Data->PhoneString, msg.Buffer, Priv->Lines, 2);

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

GSM_Error ATGEN_IncomingSMSCInfo(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s UNUSED)
{
	return ERR_NONE;
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
            return ERR_UNKNOWN;
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
	int 			i;
	char 			*pos;

	Signal->SignalStrength 	= -1;
	Signal->SignalPercent 	= -1;
	Signal->BitErrorRate 	= -1;

	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
        case AT_Reply_OK:
            smprintf(s, "Signal quality info received\n");
            i = atoi(msg.Buffer+15);
            if (i != 99) {
                /* from GSM 07.07 section 8.5 */
                Signal->SignalStrength = 2 * i - 113;

                /* FIXME: this is wild guess and probably will be phone dependant */
                Signal->SignalPercent = 15 * i;
                if (Signal->SignalPercent > 100) Signal->SignalPercent = 100;
            }
            pos = strchr(msg.Buffer + 15, ',');
            if (pos != NULL) {
                i = atoi(pos + 1);
                /* from GSM 05.08 section 8.2.4 */
                switch (i) {
                    case 0: Signal->BitErrorRate =  0; break; /* 0.14 */
                    case 1: Signal->BitErrorRate =  0; break; /* 0.28 */
                    case 2: Signal->BitErrorRate =  1; break; /* 0.57 */
                    case 3: Signal->BitErrorRate =  1; break; /* 1.13 */
                    case 4: Signal->BitErrorRate =  2; break; /* 2.26 */
                    case 5: Signal->BitErrorRate =  5; break; /* 4.53 */
                    case 6: Signal->BitErrorRate =  9; break; /* 9.05 */
                    case 7: Signal->BitErrorRate = 18; break; /* 18.10 */
                }
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

static GSM_Error ATGEN_GetNextCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool start)
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

	free(Priv->file.Buffer);
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

GSM_Error ATGEN_GetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, bool PhoneRingtone)
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

GSM_Error ATGEN_PressKey(GSM_StateMachine *s, GSM_KeyCode Key, bool Press)
{
	GSM_Error	error;
	unsigned char 	frame[40];
	char key[20];
	int len;
	unsigned char unicode_key[20];
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;

	/* We do nothing on release event */
	if (!Press) {
		return ERR_NONE;
	}

	/* Prefer IRA charaset to avoid tricky conversions */
	ATGEN_SetCharset(s, AT_PREF_CHARSET_IRA);

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
			EncodeDefault(key, unicode_key, &len, false, NULL);
			if (strcmp(key, "?") == 0) {
				smprintf(s, "Could not encode key to GSM charset!\n");
				return ERR_NOTSUPPORTED;
			}
			break;
		case AT_CHARSET_IRA:
		case AT_CHARSET_UTF8:
		case AT_CHARSET_ISO88591:
			/* Nothing to do here */
			break;
		case AT_CHARSET_UCS2:
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

#ifdef GSM_ENABLE_CELLBROADCAST

GSM_Error ATGEN_ReplyIncomingCB(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_CBMessage 	CB;
	int		i,j;
	char		Buffer[300],Buffer2[300];

	smprintf(s, "CB received\n");
	return ERR_NONE;

	DecodeHexBin (Buffer,msg.Buffer+6,msg.Length-6);
	DumpMessage(&di ,Buffer,msg.Length-6);

	CB.Channel = Buffer[4];

	for (j=0;j<msg.Length;j++) {
	smprintf(s, "j=%i\n",j);
	i=GSM_UnpackEightBitsToSeven(0, msg.Buffer[6], msg.Buffer[6], msg.Buffer+j, Buffer2);
//	i = msg.Buffer[6] - 1;
//	while (i!=0) {
//		if (Buffer[i] == 13) i = i - 1; else break;
//	}
	DecodeDefault(CB.Text, Buffer2, msg.Buffer[6], false, NULL);
	smprintf(s, "Channel %i, text \"%s\"\n",CB.Channel,DecodeUnicodeString(CB.Text));
	}
	if (s->Phone.Data.EnableIncomingCB && s->User.IncomingCB!=NULL) {
		s->User.IncomingCB(s,CB);
	}
	return ERR_NONE;
}

#endif

bool InRange(int *range, int i) {
	while (*range != -1) {
		if (*range == i) return true;
		range++;
	}
	return false;
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

	/* Sample resposne we get here:
	AT+CNMI=?
	+CNMI: (0-2),(0,1,3),(0),(0,1),(0,1)
	*/
	Priv->CNMIMode			= 0;
	Priv->CNMIProcedure		= 0;
	Priv->CNMIDeliverProcedure	= 0;
#ifdef GSM_ENABLE_CELLBROADCAST
	Priv->CNMIBroadcastProcedure	= 0;
#endif

	buffer = strchr(msg.Buffer, '\n');
	if (buffer == NULL) return  ERR_UNKNOWNRESPONSE;
	while (isspace(*buffer)) buffer++;

	if (strncmp(buffer, "+CNMI: ", 7) != 0) return ERR_UNKNOWNRESPONSE;
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

GSM_Error ATGEN_GetCNMIMode(GSM_StateMachine *s)
{
	GSM_Error 		error;
	ATGEN_WaitFor(s, "AT+CNMI=?\r", 10, 0x00, 4, ID_GetCNMIMode);
	return error;
}

GSM_Error ATGEN_SetIncomingCB(GSM_StateMachine *s, bool enable)
{
#ifdef GSM_ENABLE_CELLBROADCAST
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error 		error;
	char			buffer[100];

	if (Priv->CNMIMode == -1) {
		error = ATGEN_GetCNMIMode(s);
		if (error != ERR_NONE) return error;
	}

	if (Priv->CNMIMode == 0) return ERR_NOTSUPPORTED;
	if (Priv->CNMIBroadcastProcedure == 0) return ERR_NOTSUPPORTED;

	if (s->Phone.Data.EnableIncomingCB!=enable) {
		s->Phone.Data.EnableIncomingCB 	= enable;
		if (enable) {
			smprintf(s, "Enabling incoming CB\n");
			sprintf(buffer, "AT+CNMI=%d,,%d\r", Priv->CNMIMode, Priv->CNMIBroadcastProcedure);
			ATGEN_WaitFor(s, buffer, strlen(buffer), 0x00, 4, ID_SetIncomingCB);
			return error;
		} else {
			smprintf(s, "Disabling incoming CB\n");
			sprintf(buffer, "AT+CNMI=%d,,%d\r", Priv->CNMIMode, 0);
			ATGEN_WaitFor(s, buffer, strlen(buffer), 0x00, 4, ID_SetIncomingCB);
			return error;
		}
	}
	return ERR_NONE;
#else
	return ERR_SOURCENOTAVAILABLE;
#endif
}

GSM_Error ATGEN_SetFastSMSSending(GSM_StateMachine *s, bool enable)
{
	GSM_Error 		error;

	if (enable) {
		smprintf(s, "Enabling fast SMS sending\n");
		ATGEN_WaitFor(s, "AT+CMMS=2\r", 10, 0x00, 4, ID_SetFastSMSSending);
		return error;
	} else {
		smprintf(s, "Disabling fast SMS sending\n");
		ATGEN_WaitFor(s, "AT+CMMS=0\r", 10, 0x00, 4, ID_SetFastSMSSending);
		return error;
	}
}

GSM_Error ATGEN_IncomingSMSInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	/* We get here: +CMTI: SM, 19 */
	char			*buffer;
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Phone_Data		*Data = &s->Phone.Data;
	GSM_SMSMessage		sms;

	memset(&sms, 0, sizeof(sms));
	smprintf(s, "Incoming SMS\n");
	if (Data->EnableIncomingSMS && s->User.IncomingSMS!=NULL) {
		sms.State 	 = 0;
		sms.InboxFolder  = true;
		sms.PDU 	 = 0;

		buffer = strchr(msg.Buffer, ':');
		if (buffer == NULL) return ERR_UNKNOWNRESPONSE;
		buffer++;
		while (isspace(*buffer)) buffer++;

		if (strncmp(buffer, "ME", 2) == 0 || strncmp(buffer, "\"ME\"", 4) == 0) {
			if (Priv->SIMSMSMemory == AT_AVAILABLE) sms.Folder = 3;
			else sms.Folder = 1;
		} else if (strncmp(buffer, "SM", 2) == 0 || strncmp(buffer, "\"SM\"", 4) == 0) {
			sms.Folder = 1;
		} else {
			return ERR_UNKNOWNRESPONSE;
		}

		buffer = strchr(msg.Buffer, ',');
		if (buffer == NULL) return ERR_UNKNOWNRESPONSE;
		buffer++;
		while (isspace(*buffer)) buffer++;

		sms.Location = atoi(buffer);

		s->User.IncomingSMS(s, sms);
	}
	return ERR_NONE;
}

GSM_Error ATGEN_IncomingSMSDeliver(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data		*Data = &s->Phone.Data;
	GSM_SMSMessage 		sms;
	int 			current = 0, current2, i=0;
	unsigned char 		buffer[300],smsframe[800];

	smprintf(s, "Incoming SMS received (Deliver)\n");
	if (Data->EnableIncomingSMS && s->User.IncomingSMS!=NULL) {
		sms.State 	 = SMS_UnRead;
		sms.InboxFolder  = true;
		sms.PDU 	 = SMS_Deliver;

		/* T310 with larger SMS goes crazy and mix this incoming
                 * frame with normal answers. PDU is always last frame
		 * We find its' number and parse it */
		while (Data->Priv.ATGEN.Lines.numbers[i*2+1] != 0) {
			/* FIXME: handle special chars correctly */
			i++;
		}
		DecodeHexBin (buffer,
			GetLineString(msg.Buffer,Data->Priv.ATGEN.Lines,i),
			GetLineLength(msg.Buffer,Data->Priv.ATGEN.Lines,i));

		/* We use locations from SMS layouts like in ../phone2.c(h) */
		for(i=0;i<buffer[0]+1;i++) smsframe[i]=buffer[current++];
		smsframe[12]=buffer[current++];

		current2=((buffer[current])+1)/2+1;
		for(i=0;i<current2+1;i++) smsframe[PHONE_SMSDeliver.Number+i]=buffer[current++];
		smsframe[PHONE_SMSDeliver.TPPID] = buffer[current++];
		smsframe[PHONE_SMSDeliver.TPDCS] = buffer[current++];
		for(i=0;i<7;i++) smsframe[PHONE_SMSDeliver.DateTime+i]=buffer[current++];
		smsframe[PHONE_SMSDeliver.TPUDL] = buffer[current++];
		for(i=0;i<smsframe[PHONE_SMSDeliver.TPUDL];i++) smsframe[i+PHONE_SMSDeliver.Text]=buffer[current++];
		GSM_DecodeSMSFrame(&sms,smsframe,PHONE_SMSDeliver);

		s->User.IncomingSMS(s,sms);
	}
	return ERR_NONE;
}

/* I don't have phone able to do it and can't fill it */
GSM_Error ATGEN_IncomingSMSReport(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Incoming SMS received (Report)\n");
	return ERR_NONE;
}

GSM_Error ATGEN_SetIncomingSMS(GSM_StateMachine *s, bool enable)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error 		error;
	char			buffer[100];

	/* We will need this when incoming message, but we can invoke AT commands there: */
	if (Priv->PhoneSMSMemory == 0) {
		error = ATGEN_SetSMSMemory(s, false, false);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) return error;
	}
	if (Priv->SIMSMSMemory == 0) {
		error = ATGEN_SetSMSMemory(s, true, false);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) return error;
	}

	if (Priv->CNMIMode == -1) {
		error = ATGEN_GetCNMIMode(s);
		if (error != ERR_NONE) return error;
	}

	if (Priv->CNMIMode == 0) return ERR_NOTSUPPORTED;
	if (Priv->CNMIProcedure == 0 && Priv->CNMIDeliverProcedure == 0) return ERR_NOTSUPPORTED;

	if (s->Phone.Data.EnableIncomingSMS!=enable) {
		s->Phone.Data.EnableIncomingSMS = enable;
		if (enable) {
			smprintf(s, "Enabling incoming SMS\n");

			/* Delivery reports */
			if (Priv->CNMIDeliverProcedure != 0) {
				sprintf(buffer, "AT+CNMI=%d,,,%d\r", Priv->CNMIMode, Priv->CNMIDeliverProcedure);
				ATGEN_WaitFor(s, buffer, strlen(buffer), 0x00, 4, ID_SetIncomingSMS);
				if (error != ERR_NONE) return error;
			}

			/* Normal messages */
			if (Priv->CNMIProcedure != 0) {
				sprintf(buffer, "AT+CNMI=%d,%d\r", Priv->CNMIMode, Priv->CNMIProcedure);
				ATGEN_WaitFor(s, buffer, strlen(buffer), 0x00, 4, ID_SetIncomingSMS);
				if (error != ERR_NONE) return error;
			}
		} else {
			smprintf(s, "Disabling incoming SMS\n");

			/* Delivery reports */
			sprintf(buffer,"AT+CNMI=%d,,,%d\r", Priv->CNMIMode, 0);
			ATGEN_WaitFor(s, buffer, strlen(buffer), 0x00, 4, ID_SetIncomingSMS);
			if (error != ERR_NONE) return error;

			/* Normal messages */
			sprintf(buffer, "AT+CNMI=%d,%d\r", Priv->CNMIMode, 0);
			ATGEN_WaitFor(s, buffer, strlen(buffer), 0x00, 4, ID_SetIncomingSMS);
			if (error != ERR_NONE) return error;
		}
	}
	return ERR_NONE;
}

/**
 * Detects what additional protocols are being supported
 */
GSM_Error ATGEN_ReplyCheckProt(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	int line = 0;
	int protocol_id;
	char protocol_version[100];
	int protocol_level;
	char *string;
	GSM_Error error;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Protocol entries received\n");
		/* Walk through lines with +CPROT: */
		while (strcmp("OK", string = GetLineString(msg.Buffer,Priv->Lines,line+1)) != 0) {

			/*
			 * This is what Sony Ericsson phones usually
			 * give.
			 */
			error = ATGEN_ParseReply(s, string,
				"+CPROT: (@i), (@r), (@i)",
				&protocol_id,
				protocol_version, sizeof(protocol_version),
				&protocol_level);

			/*
			 * This reply comes from Alcatel ans Samsung.
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
#ifdef GSM_ENABLE_SONYERICSSON
				/* Tell OBEX driver that AT+CPROT=0 is supported */
				s->Phone.Data.Priv.SONYERICSSON.HasOBEX = SONYERICSSON_OBEX_CPROT0;
#endif
				/*
				 * Level 1 is almost useless, require
				 * higher levels.
				 */
				if (!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_OBEX) && protocol_level > 1) {
					/*
					 * We do not enable this automatically,
					 * because some phones provide less features
					 * over OBEX than AT using AT commands.
					 */
					smprintf(s, "HINT: Please consider adding F_OBEX to your phone capabilities in common/gsmphones.c\n");
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
{ATGEN_GenericReply,		"AT+CMEE=" 		,0x00,0x00,ID_EnableErrorInfo	 },
{ATGEN_GenericReply,		"AT+CKPD="		,0x00,0x00,ID_PressKey		 },
{ATGEN_ReplyGetSIMIMSI,		"AT+CIMI" 	 	,0x00,0x00,ID_GetSIMIMSI	 },
{ATGEN_ReplyCheckProt,		"AT+CPROT=?" 	 	,0x00,0x00,ID_SetOBEX		 },

{ATGEN_ReplyGetCNMIMode,	"AT+CNMI=?"		,0x00,0x00,ID_GetCNMIMode	 },
#ifdef GSM_ENABLE_CELLBROADCAST
{ATGEN_ReplyIncomingCB,		"+CBM:" 	 	,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_GenericReply,		"AT+CNMI"		,0x00,0x00,ID_SetIncomingCB	 },
#endif

{ATGEN_IncomingBattery,		"_OBS:"		 	,0x00,0x00,ID_IncomingFrame      },
{ATGEN_ReplyGetBatteryCharge,	"AT+CBC"		,0x00,0x00,ID_GetBatteryCharge	 },

{ATGEN_ReplyGetModel,		"AT+CGMM"		,0x00,0x00,ID_GetModel           },
{ATGEN_ReplyGetManufacturer,	"AT+CGMI"		,0x00,0x00,ID_GetManufacturer	 },
{ATGEN_ReplyGetFirmwareCGMR,	"AT+CGMR"		,0x00,0x00,ID_GetFirmware	 },
{ATGEN_ReplyGetIMEI,		"AT+CGSN"		,0x00,0x00,ID_GetIMEI	 	 },

{ATGEN_ReplySendSMS,		"AT+CMGS"		,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_ReplySendSMS,		"AT+CMSS"		,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_GenericReply,		"AT+CNMI"		,0x00,0x00,ID_SetIncomingSMS	 },
{ATGEN_GenericReply,		"AT+CMGF"		,0x00,0x00,ID_GetSMSMode	 },
{ATGEN_GenericReply,		"AT+CSDH"		,0x00,0x00,ID_GetSMSMode	 },
{ATGEN_ReplyGetSMSMessage,	"AT+CMGR"		,0x00,0x00,ID_GetSMSMessage	 },
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
{ATGEN_GenericReply,		"AT+CPBS="		,0x00,0x00,ID_SetMemoryType	 },
{ATGEN_ReplyGetCPBSMemoryStatus,"AT+CPBS?"		,0x00,0x00,ID_GetMemoryStatus	 },
// /* Samsung phones reply +CPBR: after OK --claudio*/
{ATGEN_ReplyGetCPBRMemoryInfo,	"AT+CPBR=?"		,0x00,0x00,ID_GetMemoryStatus	 },
{ATGEN_ReplyGetCPBRMemoryInfo,	"+CPBR:"		,0x00,0x00,ID_GetMemoryStatus	 },
{ATGEN_ReplyGetCPBRMemoryStatus,"AT+CPBR="		,0x00,0x00,ID_GetMemoryStatus	 },
{ATGEN_ReplyGetCharsets,	"AT+CSCS=?"		,0x00,0x00,ID_GetMemoryCharset	 },
{ATGEN_ReplyGetCharset,		"AT+CSCS?"		,0x00,0x00,ID_GetMemoryCharset	 },
{ATGEN_GenericReply,		"AT+CSCS="		,0x00,0x00,ID_SetMemoryCharset	 },
{ATGEN_ReplyGetMemory,		"AT+CPBR="		,0x00,0x00,ID_GetMemory		 },
{SIEMENS_ReplyGetMemoryInfo,	"AT^SBNR=?"		,0x00,0x00,ID_GetMemory		 },
{SIEMENS_ReplyGetMemory,	"AT^SBNR"		,0x00,0x00,ID_GetMemory		 },
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

{ATGEN_GenericReply, 		"AT+VTS"		,0x00,0x00,ID_SendDTMF		 },
{ATGEN_ReplyCancelCall,		"AT+CHUP"		,0x00,0x00,ID_CancelCall	 },
{ATGEN_ReplyDialVoice,		"ATDT"			,0x00,0x00,ID_DialVoice		 },
{ATGEN_ReplyCancelCall,		"ATH"			,0x00,0x00,ID_CancelCall	 },
{ATGEN_GenericReply, 		"AT+CRC"		,0x00,0x00,ID_SetIncomingCall	 },
{ATGEN_GenericReply, 		"AT+CLIP"		,0x00,0x00,ID_SetIncomingCall	 },
{ATGEN_GenericReply, 		"AT+CUSD"		,0x00,0x00,ID_SetUSSD		 },
{ATGEN_ReplyGetUSSD, 		"AT+CUSD"		,0x00,0x00,ID_GetUSSD		 },
{ATGEN_ReplyGetUSSD, 		"+CUSD"			,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_GenericReply,            "AT+CLIP=1"      	,0x00,0x00,ID_IncomingFrame      },
{ATGEN_ReplyIncomingCallInfo,	"+CLIP"			,0x00,0x00,ID_IncomingFrame	 },
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

#ifdef GSM_ENABLE_SONYERICSSON
{ATGEN_GenericReply,		"AT*EOBEX=?"		,0x00,0x00,ID_SetOBEX		 },
{ATGEN_GenericReply,		"AT*EOBEX"		,0x00,0x00,ID_SetOBEX		 },
{ATGEN_GenericReply,		"AT+CPROT=0" 	 	,0x00,0x00,ID_SetOBEX		 },
{ATGEN_GenericReply,		"AT+MODE=22" 	 	,0x00,0x00,ID_SetOBEX		 },

{ATGEN_GenericReply,		"AT*ESDF="		,0x00,0x00,ID_SetLocale		 },
{ATGEN_GenericReply,		"AT*ESTF="		,0x00,0x00,ID_SetLocale		 },

{SONYERICSSON_ReplyGetDateLocale,	"AT*ESDF?"	,0x00,0x00,ID_GetLocale		 },
{SONYERICSSON_ReplyGetTimeLocale,	"AT*ESTF?"	,0x00,0x00,ID_GetLocale	 	 },
{SONYERICSSON_ReplyGetFileSystemStatus,	"AT*EMEM"	,0x00,0x00,ID_FileSystemStatus 	 },
{ATGEN_GenericReply,			"AT*EBCA"	,0x00,0x00,ID_GetBatteryCharge 	 },
{SONYERICSSON_ReplyGetBatteryCharge,	"*EBCA:"	,0x00,0x00,ID_IncomingFrame	 },
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

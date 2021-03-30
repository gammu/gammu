/* (c) 2002-2004 by Marcin Wiacek and others */

#ifndef __coding_h
#define __coding_h

#if defined(_MSC_VER) && defined(__cplusplus)
    extern "C" {
#endif

#include <gammu-config.h>

#include <stdlib.h>
#ifdef HAVE_WCHAR_H
#  include <wchar.h>
#endif
#ifdef HAVE_WCTYPE_H
#  include <wctype.h>
#endif

#include <gammu-unicode.h>
#include <gammu-error.h>
#include <gammu-debug.h>

#ifdef USE_WCHAR_T
typedef wchar_t gammu_char_t;
typedef wint_t gammu_int_t;
#else
typedef unsigned long gammu_char_t;
typedef unsigned long gammu_int_t;
#endif

/* ---------------------------- Unicode ------------------------------------ */
gboolean 		myiswspace	  		(const unsigned char *src);


void 		ReverseUnicodeString		(unsigned char *String);

void 		DecodeUnicodeSpecialNOKIAChars	(unsigned char *dest, const unsigned char *src, size_t len);
void 		EncodeUnicodeSpecialNOKIAChars	(unsigned char *dest, const unsigned char *src, size_t len);

unsigned char *EncodeUnicodeSpecialChars(unsigned char *dest, const unsigned char *buffer);
unsigned char *DecodeUnicodeSpecialChars(unsigned char *dest, const unsigned char *buffer);

/* ------------------------------- BCD ------------------------------------- */
unsigned char	EncodeWithBCDAlphabet		(int value);
int		DecodeWithBCDAlphabet		(unsigned char value);

void		DecodeBCD			(unsigned char *dest, const unsigned char *src, size_t len);
void		EncodeBCD			(unsigned char *dest, const unsigned char *src, size_t len, gboolean fill);

/* ------------------------------ UTF7 ------------------------------------- */
void 		DecodeUTF7			(unsigned char *dest, const unsigned char *src, size_t len);


/* ---------------------------- ISO88591 ----------------------------------- */
void		DecodeISO88591			(unsigned char *dest, const char *src, size_t len);
void		DecodeISO88591QuotedPrintable	(unsigned char *dest, const unsigned char *src, size_t len);

/**
 * Decodes UTF-8 text with XML entities to Unicode.
 */
void DecodeXMLUTF8(unsigned char *dest, const char *src, size_t len);

/* ------------------------------- BASE64 ---------------------------------- */
void 		EncodeBASE64			(const unsigned char *Input, char *Output, const size_t Length);
int 		DecodeBASE64			(const char *Input, unsigned char *Output, const size_t Length);

/* ---------------------- DefaultAlphabet for SMS -------------------------- */
void 		EncodeDefault			(unsigned char *dest, const unsigned char *src, size_t *len, gboolean UseExtensions, unsigned char *ExtraAlphabet);
void		DecodeDefault			(unsigned char *dest, const unsigned char *src, size_t len, gboolean UseExtensions,  unsigned char *ExtraAlphabet);
void 		FindDefaultAlphabetLen		(const unsigned char *src, size_t *srclen, size_t *smslen, size_t maxlen);

int GSM_PackSevenBitsToEight	(size_t offset, const unsigned char *input, unsigned char *output, size_t length);
int GSM_UnpackEightBitsToSeven	(size_t offset, size_t in_length, size_t out_length,
				 const unsigned char *input, unsigned char *output);

/* ----------------- Phone numbers according to GSM specs ------------------ */

/**
 * Enum to handle types of phones numbers like
 * specified in GSM 03.40 section 9.1.2.5
 */
typedef enum {
	/**
	 * Unknown number type
	 */
	NUMBER_UNKNOWN_NUMBERING_PLAN_ISDN		= 0x81,
	/**
	 * International number (full number with code of country)
	 */
	NUMBER_INTERNATIONAL_NUMBERING_PLAN_ISDN	= 0x91,
	/**
	 * Alphanumeric number (with chars too)
	 */
	NUMBER_ALPHANUMERIC_NUMBERING_PLAN_UNKNOWN	= 0xD0

	/* specification give also other values */
} GSM_NumberType;

//int GSM_UnpackSemiOctetNumber(GSM_Debug_Info *di, unsigned char *retval, const unsigned char *Number, gboolean semioctet);
GSM_Error GSM_UnpackSemiOctetNumber(GSM_Debug_Info *di, unsigned char *retval, const unsigned char *Number, size_t *pos, size_t bufferlength, gboolean semioctet);
int		GSM_PackSemiOctetNumber		(const unsigned char *Number, unsigned char *Output, gboolean semioctet);

/* ---------------------------- Bits --------------------------------------- */

void BufferAlign      (unsigned char *Destination, size_t *CurrentBit);
void BufferAlignNumber(size_t *CurrentBit);

void AddBuffer	  (unsigned char *Destination, size_t *CurrentBit, unsigned char *Source, size_t BitsToProcess);
void AddBufferByte(unsigned char *Destination, size_t *CurrentBit, unsigned char Source, size_t BitsToProcess);

void GetBuffer    (unsigned char *Source, size_t *CurrentBit, unsigned char *Destination, size_t BitsToProcess);
void GetBufferInt (unsigned char *Source, size_t *CurrentBit, int *integer, size_t BitsToProcess);
void GetBufferI   (unsigned char *Source, size_t *CurrentBit, int *result, size_t BitsToProcess);

int GetBit   (unsigned char *Buffer, size_t BitNum);
int SetBit   (unsigned char *Buffer, size_t BitNum);
int ClearBit (unsigned char *Buffer, size_t BitNum);

/* ---------------------------- Other -------------------------------------- */

void StringToDouble	(char *text, double *d);

/**
 * Gets VCS line from buffer.
 *
 * @param MergeLines: Determine whether merge lines as vCard style
 * continuation or quoted printable continutaion.
 * @param Buffer: Data source to parse.
 * @param Pos: Current position in data.
 * @param OutBuffer: Pointer to buffer pointer, which will be allocated.
 * @param MaxLen: Maximal length of data to process.
 *
 * \return ERR_NONE on success, ERR_MOREMEMORY if buffer is too small.
 */
GSM_Error GSM_GetVCSLine(char **OutBuffer, char *Buffer, size_t *Pos, size_t MaxLen, gboolean MergeLines);

/**
 * Gets line from buffer.
 *
 * @param MergeLines: Determine whether merge lines as vCard style
 * continuation or quoted printable continutaion.
 * @param Buffer: Data source to parse.
 * @param Pos: Current position in data.
 * @param OutBuffer: Buffer where line will be written.
 * @param MaxLen: Maximal length of data to process.
 * @param MaxOutLen: Size of output buffer.
 *
 * \return ERR_NONE on success, ERR_MOREMEMORY if buffer is too small.
 */
GSM_Error MyGetLine(char *Buffer, size_t *Pos, char *OutBuffer, size_t MaxLen, size_t MaxOutLen, gboolean MergeLines);

char *EncodeSpecialChars(char *dest, const char *buffer);
char *DecodeSpecialChars(char *dest, const char *buffer);

/**
 * Decodes string from UTF-8.
 *
 * \ingroup Unicode
 */
int DecodeWithUTF8Alphabet(const unsigned char *src, gammu_char_t * dest, size_t len);

/**
 * Converts single character from unicode to gammu_char_t.
 */
int EncodeWithUnicodeAlphabet(const unsigned char *value, gammu_char_t *dest);

/**
 * Converts single character from gammu_char_t to unicode.
 */
int DecodeWithUnicodeAlphabet(gammu_char_t value, unsigned char *dest);

#ifdef ICONV_FOUND

gboolean IconvDecode(const char *charset, const char *input, const size_t inlen, unsigned char *output, size_t outlen);
gboolean IconvEncode(const char *charset, const unsigned char *input, const size_t inlen, char *output, size_t outlen);
#endif

#if defined(_MSC_VER) && defined(__cplusplus)
    }
#endif

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

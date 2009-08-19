/* (c) 2002-2004 by Marcin Wiacek and others */

#ifndef __coding_h
#define __coding_h

#if defined(_MSC_VER) && defined(__cplusplus)
    extern "C" {
#endif

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

#ifndef HAVE_WCHAR_T
typedef		int wchar_t;
#endif

#ifndef HAVE_WINT_T
typedef		int wint_t;
#endif

/* MSVC provides same function under different name */
#if _MSC_VER >= 1300
#include <stdlib.h>
#define strtoull _strtoui64
#endif

/* ---------------------------- Unicode ------------------------------------ */
gboolean 		myiswspace	  		(unsigned const char *src);

int		EncodeWithUnicodeAlphabet	(const unsigned char *value, wchar_t *dest);
int		DecodeWithUnicodeAlphabet	(wchar_t value, unsigned char *dest);


void 		ReverseUnicodeString		(unsigned char *String);

void 		DecodeUnicodeSpecialNOKIAChars	(unsigned char *dest, const unsigned char *src, int len);
void 		EncodeUnicodeSpecialNOKIAChars	(unsigned char *dest, const unsigned char *src, int len);

char 		*EncodeUnicodeSpecialChars	(const unsigned char *buffer);
char 		*DecodeUnicodeSpecialChars	(const unsigned char *buffer);

/* ------------------------------- BCD ------------------------------------- */
unsigned char	EncodeWithBCDAlphabet		(int value);
int		DecodeWithBCDAlphabet		(unsigned char value);

void		DecodeBCD			(unsigned char *dest, const unsigned char *src, int len);
void		EncodeBCD			(unsigned char *dest, const unsigned char *src, int len, gboolean fill);

/* ------------------------------ UTF7 ------------------------------------- */
void 		DecodeUTF7			(unsigned char *dest, const unsigned char *src, int len);


/* ---------------------------- ISO88591 ----------------------------------- */
void		DecodeISO88591			(unsigned char *dest, const char *src, int len);
void		DecodeISO88591QuotedPrintable	(unsigned char *dest, const unsigned char *src, int len);

/**
 * Decodes UTF-8 text with XML entities to Unicode.
 */
void DecodeXMLUTF8(unsigned char *dest, const char *src, int len);

/* ------------------------------- BASE64 ---------------------------------- */
void 		EncodeBASE64			(const unsigned char *Input, char *Output, const size_t Length);
int 		DecodeBASE64			(const char *Input, unsigned char *Output, const size_t Length);

/* ---------------------- DefaultAlphabet for SMS -------------------------- */
void 		EncodeDefault			(unsigned char *dest, const unsigned char *src, size_t *len, gboolean UseExtensions, unsigned char *ExtraAlphabet);
void		DecodeDefault			(unsigned char *dest, const unsigned char *src, size_t len, gboolean UseExtensions,  unsigned char *ExtraAlphabet);
void 		FindDefaultAlphabetLen		(const unsigned char *src, size_t *srclen, size_t *smslen, size_t maxlen);

int GSM_PackSevenBitsToEight	(int offset, unsigned char *input, unsigned char *output, int length);
int GSM_UnpackEightBitsToSeven	(int offset, int in_length, int out_length,
				 unsigned char *input, unsigned char *output);

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

int GSM_UnpackSemiOctetNumber(GSM_Debug_Info *di, unsigned char *retval, unsigned char *Number, gboolean semioctet);
int		GSM_PackSemiOctetNumber		(unsigned char *Number, unsigned char *Output, gboolean semioctet);

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

char *EncodeSpecialChars(const char *buffer);
char *DecodeSpecialChars(const char *buffer);

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

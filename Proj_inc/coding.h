/* (c) 2002-2004 by Marcin Wiacek and others */
#ifndef __coding_h
#define __coding_h
#include <stdlib.h>

//#include "ConvertUTF.h"


#ifdef __OpenBSD__
typedef		int wint_t;
#endif

/* ---------------------------- Unicode ------------------------------------ */
bool 		mywstrncasecmp			(unsigned const char *a, unsigned const char *b, int num);
unsigned char	*mystrstr			(unsigned const char *haystack, unsigned const char *needle);
bool 		mywstrncmp			(unsigned const char *a, unsigned const char *b, int num);
bool 		myiswspace	  		(unsigned const char *src);
int 		mytowlower			(wchar_t c);

unsigned int 	EncodeWithUnicodeAlphabet	(const unsigned char *value, wchar_t *dest);
unsigned int 	DecodeWithUnicodeAlphabet	(wchar_t value, unsigned char *dest);
void UnicodeCat(unsigned char *str,unsigned char *substr);

unsigned int	UnicodeLength			(const unsigned char *str);
unsigned char	*DecodeUnicodeString		(const unsigned char *src);
unsigned char   *DecodeUnicodeConsole		(const unsigned char *src);
void		DecodeUnicode			(const unsigned char *src, unsigned char *dest);
void		EncodeUnicode			(unsigned char *dest, const unsigned char *src, int len);
int EncodeUcs2(unsigned char* pDst , const char* pSrc , int nSrcLength);

void 		CopyUnicodeString		(unsigned char *Dest, unsigned char *Source);
void 		ReverseUnicodeString		(unsigned char *String);
void 		ReverseUnicode		(unsigned char *Src, unsigned char *Det);

void 		ReadUnicodeFile			(unsigned char *Dest, unsigned char *Source);

void 		DecodeUnicodeSpecialNOKIAChars	(unsigned char *dest, const unsigned char *src, int len);
void 		EncodeUnicodeSpecialNOKIAChars	(unsigned char *dest, const unsigned char *src, int len);
int			EncodeUnicode2UTF8( unsigned char* src, unsigned char *dest);
int			EncodeUnicode2UTF8Ex( unsigned char* src, unsigned char *dest,int MaxByte);
void UnicodeReplace(unsigned char *str, char *szOld,char* szNew);
void SplitStringEx(unsigned char *pString ,char*  Split,unsigned char* pStr1,unsigned char* pStr2);

/* ------------------------------- BCD ------------------------------------- */
unsigned char	EncodeWithBCDAlphabet		(int value);
int		DecodeWithBCDAlphabet		(unsigned char value);

void		DecodeBCD			(unsigned char *dest, const unsigned char *src, int len);
void		EncodeBCD			(unsigned char *dest, const unsigned char *src, int len, bool fill);

/* ------------------------------ UTF7 ------------------------------------- */
void 		DecodeUTF7			(unsigned char *dest, const unsigned char *src, int len);
int EncodeCString2UTF7(unsigned char* Src, unsigned char *dest);
int EncodeUnicode2UTF7(unsigned char* Src, unsigned char *dest);

/* ------------------------------ UTF8 ------------------------------------- */
wchar_t		DecodeWithUTF8Alphabet		(unsigned char mychar3, unsigned char mychar4);
bool 		EncodeWithUTF8Alphabet		(unsigned char mychar1, unsigned char mychar2, unsigned char *ret1, unsigned char *ret2);

bool		EncodeUTF8QuotedPrintable	(unsigned char *dest, const unsigned char *src);
void 		DecodeUTF8QuotedPrintable	(unsigned char *dest, const unsigned char *src, int len);

bool		EncodeQuotedPrintable(unsigned char *dest, const unsigned char *src);

bool 		EncodeUTF8			(unsigned char *dest, const unsigned char *src);
void 		DecodeUTF8			(unsigned char *dest, const unsigned char *src, int len);
int  EncodeCString2UTF8( unsigned char* text, unsigned char *utfstr);
int  EncodeCString2UTF8Ex( unsigned char* text, unsigned char *utfstr,int MaxByte);

/* ------------------------------- BASE64 ---------------------------------- */
void 		EncodeBASE64			(const unsigned char *Input, unsigned char *Output, int Length);
int 		DecodeBASE64			(const unsigned char *Input, unsigned char *Output, int Length);

/* ----------------------------- HexBin ------------------------------------ */
void		DecodeHexBin			(unsigned char *dest, const unsigned char *src, int len);
void		EncodeHexBin			(unsigned char *dest, const unsigned char *src, int len);

/* ----------------------------- HexUnicode -------------------------------- */
void		DecodeHexUnicode		(unsigned char *dest, const unsigned char *src, int len);
void		EncodeHexUnicode		(unsigned char *dest, const unsigned char *src, int len);

/* ---------------------- DefaultAlphabet for SMS -------------------------- */
void 		EncodeDefault			(unsigned char *dest, const unsigned char *src, int *len, bool UseExtensions, unsigned char *ExtraAlphabet);
void		DecodeDefault			(unsigned char *dest, const unsigned char *src, int len, bool UseExtensions,  unsigned char *ExtraAlphabet);
void 		FindDefaultAlphabetLen		(const unsigned char *src, int *srclen, int *smslen, int maxlen);

int GSM_PackSevenBitsToEight	(int offset, unsigned char *input, unsigned char *output, int length);
int GSM_UnpackEightBitsToSeven	(int offset, int in_length, int out_length,
				 unsigned char *input, unsigned char *output);
BOOL Win_DecodeUnicode(unsigned char *dest, const unsigned char *src, int len);
int  DecodeUTF2String( char* text, unsigned char *utfstr, int str_len	);
int DecodeUTF82Wchar(const unsigned char* utfchar,  wchar_t *ch );
void  DecodeUTF8ToUnicode(unsigned char* text, unsigned char *utfstr, int str_len	);
/* ----------------- Phone numbers according to GSM specs ------------------ */
int ConvertToGreek(int j);
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
	NUMBER_ALPHANUMERIC_NUMBERING_PLAN_UNKNOWN	= 0xD0,
	/**
	 *Number Starting with *
	 */
	 NUMBER_STAR_NUMBERING_PLAN	= 0x80

	/* specification give also other values */
} GSM_NumberType;

void		GSM_UnpackSemiOctetNumber	(unsigned char *retval, unsigned char *Number, bool semioctet);
int		GSM_PackSemiOctetNumber		(unsigned char *Number, unsigned char *Output, bool semioctet);

int		SAMSUNG_PackSemiOctetNumber		(unsigned char *Number, unsigned char *Output, bool semioctet);

/* ---------------------------- Bits --------------------------------------- */

void BufferAlign      (unsigned char *Destination, int *CurrentBit);
void BufferAlignNumber(int *CurrentBit);

void AddBuffer	  (unsigned char *Destination, int *CurrentBit, unsigned char *Source, int BitsToProcess);
void AddBufferByte(unsigned char *Destination, int *CurrentBit, unsigned char Source, int BitsToProcess);

void GetBuffer    (unsigned char *Source, int *CurrentBit, unsigned char *Destination, int BitsToProcess);
void GetBufferInt (unsigned char *Source, int *CurrentBit, int *integer, int BitsToProcess);
void GetBufferI   (unsigned char *Source, int *CurrentBit, int *result, int BitsToProcess);

int GetBit   (unsigned char *Buffer, int BitNum);
int SetBit   (unsigned char *Buffer, int BitNum);
int ClearBit (unsigned char *Buffer, int BitNum);

/* ---------------------------- Other -------------------------------------- */

void StringToDouble	(char *text, double *d);

bool mystrncasecmp (unsigned const char *a, unsigned const char *b, int num);

void SamsungZGetLine(unsigned char *Buffer, int *Pos, unsigned char *OutBuffer, int MaxLen);
void MyGetLine(unsigned char *Buffer, int *Pos, unsigned char *OutBuffer, int MaxLen);
//by karl
int Searchstr(unsigned const char *a, unsigned const char *b, int num );
int wstrstr(unsigned const char *a, unsigned const char *b);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

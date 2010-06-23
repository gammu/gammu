
#ifndef __coding_h
#define __coding_h

#include <stdlib.h>

#include "misc.h"

#ifdef __OpenBSD__
typedef		int wint_t;
#endif

/* ---------------------------- Unicode ------------------------------------ */
bool 		mywstrncasecmp			(unsigned char *a, unsigned char *b, int num);
bool 		mywstrncmp			(unsigned char *a, unsigned char *b, int num);
bool 		myiswspace	  		(unsigned char *src);
int 		mytowlower			(wchar_t c);

unsigned int 	EncodeWithUnicodeAlphabet	(const unsigned char *value, wchar_t *dest);
unsigned int 	DecodeWithUnicodeAlphabet	(wchar_t value, unsigned char *dest);

unsigned char	*DecodeUnicodeString		(const unsigned char *src);
void		DecodeUnicode			(const unsigned char *src, unsigned char *dest);
void		EncodeUnicode			(unsigned char *dest, const unsigned char *src, int len);

void 		CopyUnicodeString		(unsigned char *Dest, unsigned char *Source);
void 		ReverseUnicodeString		(unsigned char *String);

void 		ReadUnicodeFile			(unsigned char *Dest, unsigned char *Source);

void 		DecodeUnicodeSpecialNOKIAChars	(unsigned char *dest, const unsigned char *src, int len);
void 		EncodeUnicodeSpecialNOKIAChars	(unsigned char *dest, const unsigned char *src, int len);

/* ------------------------------- BCD ------------------------------------- */
unsigned char	EncodeWithBCDAlphabet		(int value);
int		DecodeWithBCDAlphabet		(unsigned char value);

void		DecodeBCD			(unsigned char *dest, const unsigned char *src, int len);
void		EncodeBCD			(unsigned char *dest, const unsigned char *src, int len, bool fill);

/* ------------------------------ UTF8 ------------------------------------- */
wchar_t		DecodeWithUTF8Alphabet		(unsigned char mychar3, unsigned char mychar4);
bool 		EncodeWithUTF8Alphabet		(unsigned char mychar1, unsigned char mychar2, unsigned char *ret1, unsigned char *ret2);

void		EncodeUTF8			(unsigned char *dest, const unsigned char *src);

/* ----------------------------- HexBin ------------------------------------ */
void		DecodeHexBin			(unsigned char *dest, const unsigned char *src, int len);
void		EncodeHexBin			(unsigned char *dest, const unsigned char *src, int len);

/* ---------------------- DefaultAlphabet for SMS -------------------------- */
void 		EncodeDefault			(unsigned char *dest, const unsigned char *src, int *len, bool UseExtensions, unsigned char *ExtraAlphabet);
void		DecodeDefault			(unsigned char *dest, const unsigned char *src, int len, bool UseExtensions,  unsigned char *ExtraAlphabet);
void 		FindDefaultAlphabetLen		(const unsigned char *src, int *srclen, int *smslen, int maxlen);

int GSM_PackSevenBitsToEight	(int offset, unsigned char *input, unsigned char *output, int length);
int GSM_UnpackEightBitsToSeven	(int offset, int in_length, int out_length,
				 unsigned char *input, unsigned char *output);

/* ----------------- Phone numbers according to GSM specs ------------------ */
/* This data-type is used to specify the type of the number.
 * See the official GSM specification 03.40, section 9.1.2.5
 */
typedef enum {
	GNT_UNKNOWN		= 0x81,	/* Unknown number */
	GNT_INTERNATIONAL	= 0x91,	/* International number */
	GNT_ALPHANUMERIC	= 0xD0	/* Alphanumeric number */
	/*...*/
} GSM_NumberType;

void		GSM_UnpackSemiOctetNumber	(unsigned char *retval, unsigned char *Number, bool semioctet);
int		GSM_PackSemiOctetNumber		(unsigned char *Number, unsigned char *Output, bool semioctet);

/* ---------------------------- Other -------------------------------------- */
int OctetAlign(unsigned char *Dest, int CurrentBit);
int OctetAlignNumber(int CurrentBit);
int BitPack(unsigned char *Dest, int CurrentBit, unsigned char *Source, int Bits);
int BitPackByte(unsigned char *Dest, int CurrentBit, unsigned char Command, int Bits);

int BitUnPack(unsigned char *Dest, int CurrentBit, unsigned char *Source, int Bits);
int BitUnPackInt(unsigned char *Src, int CurrentBit, int *integer, int Bits);
int OctetUnAlign(int CurrentBit);

#define GetBit(Stream,BitNr)	Stream[(BitNr)/8] & 1<<(7-((BitNr)%8))
#define SetBit(Stream,BitNr)	Stream[(BitNr)/8] |= 1<<(7-((BitNr)%8))
#define ClearBit(Stream,BitNr)	Stream[(BitNr)/8] &= 255 - (1 << (7-((BitNr)%8)))

bool mystrncasecmp (unsigned char *a, unsigned char *b, int num);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

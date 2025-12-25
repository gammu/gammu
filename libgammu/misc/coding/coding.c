/* (c) 2002-2004 by Marcin Wiacek, Michal Cihar and others */
/* based on some work from MyGnokii (www.mwiacek.com) */
/* based on some work from Gnokii (www.gnokii.org)
 * (C) 1999-2000 Hugh Blemings & Pavel Janik ml. (C) 2001-2004 Pawel Kot
 * GNU GPL version 2 or later
 */
/* Due to a problem in the source code management, the names of some of
 * the authors have unfortunately been lost. We do not mean to belittle
 * their efforts and hope they will contact us to see their names
 * properly added to the Copyright notice above.
 * Having published their contributions under the terms of the GNU
 * General Public License (GPL) [version 2], the Copyright of these
 * authors will remain respected by adhering to the license they chose
 * to publish their code under.
 */

#include <gammu-config.h>

#ifdef HAVE_WCHAR_H
#  include <wchar.h>
#endif
#ifdef HAVE_WCTYPE_H
#  include <wctype.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <limits.h>
#ifdef WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

#include "../../debug.h"
#include "coding.h"

/* function changes #10 #13 chars to \n \r */
unsigned char *EncodeUnicodeSpecialChars(unsigned char *dest, const unsigned char *buffer)
{
	int 			Pos=0, Pos2=0;

	while (buffer[Pos*2]!=0x00 || buffer[Pos*2+1]!=0x00) {
		if (buffer[Pos*2] == 0x00 && buffer[Pos*2+1] == 10) {
			dest[Pos2*2]   = 0x00;
			dest[Pos2*2+1] = '\\';
			Pos2++;
			dest[Pos2*2]   = 0x00;
			dest[Pos2*2+1] = 'n';
			Pos2++;
		} else if (buffer[Pos*2] == 0x00 && buffer[Pos*2+1] == 13) {
			dest[Pos2*2]   = 0x00;
			dest[Pos2*2+1] = '\\';
			Pos2++;
			dest[Pos2*2]   = 0x00;
			dest[Pos2*2+1] = 'r';
			Pos2++;
		} else if (buffer[Pos*2] == 0x00 && buffer[Pos*2+1] == '\\') {
			dest[Pos2*2]   = 0x00;
			dest[Pos2*2+1] = '\\';
			Pos2++;
			dest[Pos2*2]   = 0x00;
			dest[Pos2*2+1] = '\\';
			Pos2++;
		} else if (buffer[Pos*2] == 0x00 && buffer[Pos*2+1] == ';') {
			dest[Pos2*2]   = 0x00;
			dest[Pos2*2+1] = '\\';
			Pos2++;
			dest[Pos2*2]   = 0x00;
			dest[Pos2*2+1] = ';';
			Pos2++;
		} else if (buffer[Pos*2] == 0x00 && buffer[Pos*2+1] == ',') {
			dest[Pos2*2]   = 0x00;
			dest[Pos2*2+1] = '\\';
			Pos2++;
			dest[Pos2*2]   = 0x00;
			dest[Pos2*2+1] = ',';
			Pos2++;
		} else {
			dest[Pos2*2]   = buffer[Pos*2];
			dest[Pos2*2+1] = buffer[Pos*2+1];
			Pos2++;
		}
		Pos++;
	}
	dest[Pos2*2]   = 0;
	dest[Pos2*2+1] = 0;
	return dest;
}

/* function changes #10 #13 chars to \n \r */
char *EncodeSpecialChars(char *dest, const char *buffer)
{
	int 			Pos=0, Pos2=0;

	while (buffer[Pos]!=0x00) {
		switch (buffer[Pos]) {
		case 10:
			dest[Pos2++] = '\\';
			dest[Pos2++] = 'n';
			break;
		case 13:
			dest[Pos2++] = '\\';
			dest[Pos2++] = 'r';
			break;
		case '\\':
			dest[Pos2++] = '\\';
			dest[Pos2++] = '\\';
			break;
		default:
			dest[Pos2++] = buffer[Pos];
		}
		Pos++;
	}
	dest[Pos2] = 0;
	return dest;
}

unsigned char *DecodeUnicodeSpecialChars(unsigned char *dest, const unsigned char *buffer)
{
	int 			Pos=0, Pos2=0, level=0;

	while (buffer[Pos*2]!=0x00 || buffer[Pos*2+1]!=0x00) {
		dest[Pos2*2]   = buffer[Pos*2];
		dest[Pos2*2+1] = buffer[Pos*2+1];
		switch (level) {
		case 0:
			if (buffer[Pos*2] == 0x00 && buffer[Pos*2+1] == '\\') {
				level = 1;
			} else {
				Pos2++;
			}
			break;
		case 1:
			if (buffer[Pos*2] == 0x00 && buffer[Pos*2+1] == 'n') {
				dest[Pos2*2]   = 0;
				dest[Pos2*2+1] = 10;
			} else
			if (buffer[Pos*2] == 0x00 && buffer[Pos*2+1] == 'r') {
				dest[Pos2*2]   = 0;
				dest[Pos2*2+1] = 13;
			} else
			if (buffer[Pos*2] == 0x00 && buffer[Pos*2+1] == '\\') {
				dest[Pos2*2]   = 0;
				dest[Pos2*2+1] = '\\';
			}
			Pos2++;
			level = 0;
		}
		Pos++;
	}
	dest[Pos2*2]   = 0;
	dest[Pos2*2+1] = 0;
	return dest;
}

char *DecodeSpecialChars(char *dest, const char *buffer)
{
	int 			Pos=0, Pos2=0, level=0;

	while (buffer[Pos]!=0x00) {
		dest[Pos2] = buffer[Pos];
		switch (level) {
		case 0:
			if (buffer[Pos] == '\\') {
				level = 1;
			} else {
				Pos2++;
			}
			break;
		case 1:
			if (buffer[Pos] == 'n') dest[Pos2] = 10;
			if (buffer[Pos] == 'r') dest[Pos2] = 13;
			if (buffer[Pos] == '\\') dest[Pos2] = '\\';
			Pos2++;
			level = 0;
		}
		Pos++;
	}
	dest[Pos2] = 0;
	return dest;
}

size_t UnicodeLength(const unsigned char *str)
{
	size_t len = 0;

	if (str == NULL) return 0;

	while(str[len*2] != 0 || str[len*2+1] != 0) len++;

	return len;
}

/* Convert Unicode char saved in src to dest */
int EncodeWithUnicodeAlphabet(const unsigned char *src, gammu_char_t *dest)
{
	int retval;
	wchar_t out = 0;

	retval = mbtowc(&out, src, MB_CUR_MAX);
	*dest = out;

	switch (retval) {
		case -1 :
		case  0 : return 1;
		default : return retval;
	}
}

/* Convert Unicode char saved in src to dest */
int DecodeWithUnicodeAlphabet(gammu_char_t src, unsigned char *dest)
{
        int retval;

        switch (retval = wctomb(dest, src)) {
                case -1:
                        *dest = '?';
                        return 1;
                default:
                        return retval;
        }
}

void DecodeUnicode (const unsigned char *src, char *dest)
{
	int		i=0,o=0;
	gammu_char_t		value, second;

	while (src[(2*i)+1]!=0x00 || src[2*i]!=0x00) {
		value = src[i * 2] * 256 + src[i * 2 + 1];
		/* Decode UTF-16 */
		if (value >= 0xD800 && value <= 0xDBFF) {
			second = src[(i + 1) * 2] * 256 + src[(i + 1) * 2 + 1];
			if (second >= 0xDC00 && second <= 0xDFFF) {
				value = ((value - 0xD800) << 10) + (second - 0xDC00) + 0x010000;
				i++;
			} else if (second == 0) {
				/* Surrogate at the end of string */
				value = 0xFFFD; /* REPLACEMENT CHARACTER */
			}
		}
		o += DecodeWithUnicodeAlphabet(value, dest + o);
		i++;
	}
	dest[o]=0;
}

/* Decode Unicode string and return as function result */
char *DecodeUnicodeString (const unsigned char *src)
{
 	static char dest[500];

	DecodeUnicode(src,dest);
	return dest;
}

/* Decode Unicode string to UTF8 or other console charset
 * and return as function result
 */
char *DecodeUnicodeConsole(const unsigned char *src)
{
 	static char dest[500];

	if (GSM_global_debug.coding[0] != 0) {
		if (!strcmp(GSM_global_debug.coding,"utf8")) {
			EncodeUTF8(dest, src);
		} else {
#ifdef WIN32
			setlocale(LC_ALL, GSM_global_debug.coding);
#endif
			DecodeUnicode(src,dest);
		}
	} else {
#ifdef WIN32
		setlocale(LC_ALL, ".OCP");
#endif
		DecodeUnicode(src,dest);
#ifdef WIN32
		setlocale(LC_ALL, ".ACP");
#endif
	}
	return dest;
}

/* Encode string to Unicode. Len is number of input chars */
void DecodeISO88591 (unsigned char *dest, const char *src, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++) {
		/* Hack for Euro sign */
		if ((unsigned char)src[i] == 0x80) {
			dest[2 * i] = 0x20;
			dest[(2 * i) + 1] = 0xac;
		} else {
			dest[2 * i] = 0;
			dest[(2 * i) + 1] = src[i];
		}
	}
	dest[2 * i] = 0;
	dest[(2 * i) + 1] = 0;
}

/**
 * Stores UTF16 char in output
 *
 * Returns 1 if additional output was used
 */
size_t StoreUTF16 (unsigned char *dest, gammu_char_t wc)
{
	gammu_char_t tmp;

	if (wc > 0xffff) {
		wc = wc - 0x10000;
		tmp = 0xD800 | (wc >> 10);
		dest[0]	= (tmp >> 8) & 0xff;
		dest[1]	= tmp & 0xff;

		tmp = 0xDC00 | (wc & 0x3ff);

		dest[2]	= (tmp >> 8) & 0xff;
		dest[3]	= tmp & 0xff;

		return 1;
	}

	dest[0]	= (wc >> 8) & 0xff;
	dest[1]	= wc & 0xff;
	return 0;
}

/* Encode string to Unicode. Len is number of input chars */
void EncodeUnicode (unsigned char *dest, const char *src, size_t len)
{
	size_t 		i_len = 0, o_len;
 	gammu_char_t 	wc;

	for (o_len = 0; i_len < len; o_len++) {
		i_len += EncodeWithUnicodeAlphabet(&src[i_len], &wc);
		if (StoreUTF16(dest + o_len * 2, wc)) {
			o_len++;
		}
 	}
	dest[o_len*2]		= 0;
	dest[(o_len*2)+1]	= 0;
}

unsigned char EncodeWithBCDAlphabet(int value)
{
	div_t division;

	division=div(value,10);
	return ( ( (value-division.quot*10) & 0x0f) << 4) | (division.quot & 0xf);
}

int DecodeWithBCDAlphabet(unsigned char value)
{
	return 10*(value & 0x0f)+(value >> 4);
}

void DecodeBCD (unsigned char *dest, const unsigned char *src, size_t len)
{
	size_t i,current=0;
	int digit;

	for (i = 0; i < len; i++) {
	        digit=src[i] & 0x0f;
                if (digit<10) dest[current++]=digit + '0';
	        digit=src[i] >> 4;
                if (digit<10) dest[current++]=digit + '0';
	}
	dest[current]=0;
}

void EncodeBCD (unsigned char *dest, const unsigned char *src, size_t len, gboolean fill)
{
	size_t i,current=0;

	for (i = 0; i < len; i++) {
        	if (i & 0x01) {
			dest[current]=dest[current] | ((src[i]-'0') << 4);
			current++;
		} else {
			dest[current]=src[i]-'0';
		}
	}

        /* When fill is set: if number consist of odd number of digits,
	   we fill last bits in last byte with 0x0f
	 */
	if (fill && (len & 0x01)) dest[current]=dest[current] | 0xf0;
}

int DecodeWithHexBinAlphabet (unsigned char mychar)
{
	if (mychar >= 'A' && mychar <= 'F')
		return mychar - 'A' + 10;

	if (mychar >= 'a' && mychar <= 'f')
		return mychar - 'a' + 10;

	if (mychar >= '0' && mychar <= '9')
		return mychar - '0';

	return -1;
}

char EncodeWithHexBinAlphabet (int digit)
{
	if (digit >= 0 && digit <= 9) return '0'+(digit);
	if (digit >=10 && digit <=15) return 'A'+(digit-10);
	return 0;
}

gboolean DecodeHexUnicode (unsigned char *dest, const char *src, size_t len)
{
	size_t i, current = 0;
	int val0, val1, val2, val3;

	for (i = 0; i < len ; i += 4) {
		val0 = DecodeWithHexBinAlphabet(src[i + 0]);
		val1 = DecodeWithHexBinAlphabet(src[i + 1]);
		val2 = DecodeWithHexBinAlphabet(src[i + 2]);
		val3 = DecodeWithHexBinAlphabet(src[i + 3]);

		if (val0 < 0 || val1 < 0 || val2 < 0 || val3 < 0) {
			return FALSE;
		}

		dest[current++] = (val0 << 4) + val1;
		dest[current++] = (val2 << 4) + val3;
	}
	dest[current++] = 0;
	dest[current] = 0;

	return TRUE;
}

void EncodeHexUnicode (char *dest, const unsigned char *src, size_t len)
{
	EncodeHexBin(dest, src, len * 2);
}

gboolean DecodeHexBin (unsigned char *dest, const unsigned char *src, size_t len)
{
	size_t i,current=0;
	int low, high;

	for (i = 0; i < len/2 ; i++) {
		low = DecodeWithHexBinAlphabet(src[i*2+1]);
		high = DecodeWithHexBinAlphabet(src[i*2]);
		if (low < 0 || high < 0) return FALSE;
		dest[current++] = (high << 4) | low;
	}
	dest[current] = 0;
	return TRUE;
}

void EncodeHexBin (char *dest, const unsigned char *src, size_t len)
{
	size_t i, outpos = 0;

	for (i = 0; i < len; i++) {
		dest[outpos++] = EncodeWithHexBinAlphabet(src[i] >> 4);
		dest[outpos++] = EncodeWithHexBinAlphabet(src[i] & 0xF);
	}
	dest[outpos] = 0;
}

/* ETSI GSM 03.38, section 6.2.1: Default alphabet for SMS messages */
static unsigned char GSM_DefaultAlphabetUnicode[128+1][2] =
{
	{0x00,0x40},{0x00,0xa3},{0x00,0x24},{0x00,0xA5},
	{0x00,0xE8},{0x00,0xE9},{0x00,0xF9},{0x00,0xEC},/*0x08*/
	{0x00,0xF2},{0x00,0xC7},{0x00,'\n'},{0x00,0xD8},
	{0x00,0xF8},{0x00,'\r'},{0x00,0xC5},{0x00,0xE5},
	{0x03,0x94},{0x00,0x5f},{0x03,0xA6},{0x03,0x93},
	{0x03,0x9B},{0x03,0xA9},{0x03,0xA0},{0x03,0xA8},
	{0x03,0xA3},{0x03,0x98},{0x03,0x9E},{0x00,0xb9},
	{0x00,0xC6},{0x00,0xE6},{0x00,0xDF},{0x00,0xC9},/*0x20*/
	{0x00,' ' },{0x00,'!' },{0x00,'\"'},{0x00,'#' },
	{0x00,0xA4},{0x00,'%' },{0x00,'&' },{0x00,'\''},
	{0x00,'(' },{0x00,')' },{0x00,'*' },{0x00,'+' },
	{0x00,',' },{0x00,'-' },{0x00,'.' },{0x00,'/' },/*0x30*/
	{0x00,'0' },{0x00,'1' },{0x00,'2' },{0x00,'3' },
	{0x00,'4' },{0x00,'5' },{0x00,'6' },{0x00,'7' },
	{0x00,'8' },{0x00,'9' },{0x00,':' },{0x00,';' },
	{0x00,'<' },{0x00,'=' },{0x00,'>' },{0x00,'?' },/*0x40*/
	{0x00,0xA1},{0x00,'A' },{0x00,'B' },{0x00,'C' },
	{0x00,'D' },{0x00,'E' },{0x00,'F' },{0x00,'G' },
	{0x00,'H' },{0x00,'I' },{0x00,'J' },{0x00,'K' },
	{0x00,'L' },{0x00,'M' },{0x00,'N' },{0x00,'O' },
	{0x00,'P' },{0x00,'Q' },{0x00,'R' },{0x00,'S' },
	{0x00,'T' },{0x00,'U' },{0x00,'V' },{0x00,'W' },
	{0x00,'X' },{0x00,'Y' },{0x00,'Z' },{0x00,0xC4},
	{0x00,0xD6},{0x00,0xD1},{0x00,0xDC},{0x00,0xA7},
	{0x00,0xBF},{0x00,'a' },{0x00,'b' },{0x00,'c' },
	{0x00,'d' },{0x00,'e' },{0x00,'f' },{0x00,'g' },
	{0x00,'h' },{0x00,'i' },{0x00,'j' },{0x00,'k' },
	{0x00,'l' },{0x00,'m' },{0x00,'n' },{0x00,'o' },
	{0x00,'p' },{0x00,'q' },{0x00,'r' },{0x00,'s' },
	{0x00,'t' },{0x00,'u' },{0x00,'v' },{0x00,'w' },
	{0x00,'x' },{0x00,'y' },{0x00,'z' },{0x00,0xE4},
	{0x00,0xF6},{0x00,0xF1},{0x00,0xFC},{0x00,0xE0},
	{0x00,0x00}
};

/* ETSI GSM 3.38
 * Some sequences of 2 default alphabet chars (for example,
 * 0x1b, 0x65) are visible as one single additional char (for example,
 * 0x1b, 0x65 gives Euro char saved in Unicode as 0x20, 0xAC)
 * This table contains:
 * 1. two first char means second char from the sequence of chars from GSM default alphabet (first being 0x1b)
 * 2. two second is target (encoded) char saved in Unicode
 */
static unsigned char GSM_DefaultAlphabetCharsExtension[][3] =
{
	{0x0a,0x00,0x0c},	/* \r	*/
	{0x14,0x00,0x5e},	/* ^	*/
	{0x28,0x00,0x7b},	/* {	*/
	{0x29,0x00,0x7d},	/* }	*/
	{0x2f,0x00,0x5c},	/* \	*/
	{0x3c,0x00,0x5b},	/* [	*/
	{0x3d,0x00,0x7E},	/* ~	*/
	{0x3e,0x00,0x5d},	/* ]	*/
	{0x40,0x00,0x7C},	/* |	*/
	{0x65,0x20,0xAC},	/* Euro */
	{0x00,0x00,0x00}
};

void DecodeDefault (unsigned char *dest, const unsigned char *src, size_t len, gboolean UseExtensions, unsigned char *ExtraAlphabet)
{
	size_t 	pos, current = 0, i;

#ifdef DEBUG
	DumpMessageText(&GSM_global_debug, src, len);
#endif

	for (pos = 0; pos < len; pos++) {
		if ((pos < (len - 1)) && UseExtensions && src[pos] == 0x1b) {
			for (i = 0; GSM_DefaultAlphabetCharsExtension[i][0] != 0x00; i++) {
				if (GSM_DefaultAlphabetCharsExtension[i][0] == src[pos + 1]) {
					dest[current++] = GSM_DefaultAlphabetCharsExtension[i][1];
					dest[current++] = GSM_DefaultAlphabetCharsExtension[i][2];
					pos++;
					break;
				}
			}
			/* Skip rest if we've found something */
			if (GSM_DefaultAlphabetCharsExtension[i][0] != 0x00) {
				continue;
			}
		}
		if (ExtraAlphabet != NULL) {
			for (i = 0; ExtraAlphabet[i] != 0x00; i += 3) {
				if (ExtraAlphabet[i] == src[pos]) {
					dest[current++] = ExtraAlphabet[i + 1];
					dest[current++] = ExtraAlphabet[i + 2];
					break;
				}
			}
			/* Skip rest if we've found something */
			if (ExtraAlphabet[i] != 0x00) {
				continue;
			}
		}
		dest[current++] = GSM_DefaultAlphabetUnicode[src[pos]][0];
		dest[current++] = GSM_DefaultAlphabetUnicode[src[pos]][1];
	}
	dest[current++]=0;
	dest[current]=0;
#ifdef DEBUG
	DumpMessageText(&GSM_global_debug, dest, UnicodeLength(dest)*2);
#endif
}

/* There are many national chars with "adds". In phone they're normally
 * changed to "plain" Latin chars. We have such functionality too.
 * This table is automatically created from convert.txt file (see
 * /docs/developers) using --makeconverttable. It contains such chars
 * to replace in order:
 * 1. original char (Unicode) 2. destination char (Unicode)
 */
static unsigned char ConvertTable[] =
"\x00\xc0\x00\x41" \
"\x00\xe0\x00\x61" \
"\x00\xc1\x00\x41" \
"\x00\xe1\x00\x61" \
"\x00\xc2\x00\x41" \
"\x00\xe2\x00\x61" \
"\x00\xc3\x00\x41" \
"\x00\xe3\x00\x61" \
"\x1e\xa0\x00\x41" \
"\x1e\xa1\x00\x61" \
"\x1e\xa2\x00\x41" \
"\x1e\xa3\x00\x61" \
"\x1e\xa4\x00\x41" \
"\x1e\xa5\x00\x61" \
"\x1e\xa6\x00\x41" \
"\x1e\xa7\x00\x61" \
"\x1e\xa8\x00\x41" \
"\x1e\xa9\x00\x61" \
"\x1e\xaa\x00\x41" \
"\x1e\xab\x00\x61" \
"\x1e\xac\x00\x41" \
"\x1e\xad\x00\x61" \
"\x1e\xae\x00\x41" \
"\x1e\xaf\x00\x61" \
"\x1e\xb0\x00\x41" \
"\x1e\xb1\x00\x61" \
"\x1e\xb2\x00\x41" \
"\x1e\xb3\x00\x61" \
"\x1e\xb4\x00\x41" \
"\x1e\xb5\x00\x61" \
"\x1e\xb6\x00\x41" \
"\x1e\xb7\x00\x61" \
"\x01\xcd\x00\x41" \
"\x01\xce\x00\x61" \
"\x01\x00\x00\x41" \
"\x01\x01\x00\x61" \
"\x01\x02\x00\x41" \
"\x01\x03\x00\x61" \
"\x01\x04\x00\x41" \
"\x01\x05\x00\x61" \
"\x01\xfb\x00\x61" \
"\x01\x06\x00\x43" \
"\x01\x07\x00\x63" \
"\x01\x08\x00\x43" \
"\x01\x09\x00\x63" \
"\x01\x0a\x00\x43" \
"\x01\x0b\x00\x63" \
"\x01\x0c\x00\x43" \
"\x01\x0d\x00\x63" \
"\x00\xe7\x00\x63" \
"\x01\x0e\x00\x44" \
"\x01\x0f\x00\x64" \
"\x01\x10\x00\x44" \
"\x01\x11\x00\x64" \
"\x00\xc8\x00\x45" \
"\x00\xca\x00\x45" \
"\x00\xea\x00\x65" \
"\x00\xcb\x00\x45" \
"\x00\xeb\x00\x65" \
"\x1e\xb8\x00\x45" \
"\x1e\xb9\x00\x65" \
"\x1e\xba\x00\x45" \
"\x1e\xbb\x00\x65" \
"\x1e\xbc\x00\x45" \
"\x1e\xbd\x00\x65" \
"\x1e\xbe\x00\x45" \
"\x1e\xbf\x00\x65" \
"\x1e\xc0\x00\x45" \
"\x1e\xc1\x00\x65" \
"\x1e\xc2\x00\x45" \
"\x1e\xc3\x00\x65" \
"\x1e\xc4\x00\x45" \
"\x1e\xc5\x00\x65" \
"\x1e\xc6\x00\x45" \
"\x1e\xc7\x00\x65" \
"\x01\x12\x00\x45" \
"\x01\x13\x00\x65" \
"\x01\x14\x00\x45" \
"\x01\x15\x00\x65" \
"\x01\x16\x00\x45" \
"\x01\x17\x00\x65" \
"\x01\x18\x00\x45" \
"\x01\x19\x00\x65" \
"\x01\x1a\x00\x45" \
"\x01\x1b\x00\x65" \
"\x01\x1c\x00\x47" \
"\x01\x1d\x00\x67" \
"\x01\x1e\x00\x47" \
"\x01\x1f\x00\x67" \
"\x01\x20\x00\x47" \
"\x01\x21\x00\x67" \
"\x01\x22\x00\x47" \
"\x01\x23\x00\x67" \
"\x01\x24\x00\x48" \
"\x01\x25\x00\x68" \
"\x01\x26\x00\x48" \
"\x01\x27\x00\x68" \
"\x00\xcc\x00\x49" \
"\x00\xcd\x00\x49" \
"\x00\xed\x00\x69" \
"\x00\xce\x00\x49" \
"\x00\xee\x00\x69" \
"\x00\xcf\x00\x49" \
"\x00\xef\x00\x69" \
"\x01\x28\x00\x49" \
"\x01\x29\x00\x69" \
"\x01\x2a\x00\x49" \
"\x01\x2b\x00\x69" \
"\x01\x2c\x00\x49" \
"\x01\x2d\x00\x69" \
"\x01\x2e\x00\x49" \
"\x01\x2f\x00\x69" \
"\x01\x30\x00\x49" \
"\x01\x31\x00\x69" \
"\x01\xcf\x00\x49" \
"\x01\xd0\x00\x69" \
"\x1e\xc8\x00\x49" \
"\x1e\xc9\x00\x69" \
"\x1e\xca\x00\x49" \
"\x1e\xcb\x00\x69" \
"\x01\x34\x00\x4a" \
"\x01\x35\x00\x6a" \
"\x01\x36\x00\x4b" \
"\x01\x37\x00\x6b" \
"\x01\x39\x00\x4c" \
"\x01\x3a\x00\x6c" \
"\x01\x3b\x00\x4c" \
"\x01\x3c\x00\x6c" \
"\x01\x3d\x00\x4c" \
"\x01\x3e\x00\x6c" \
"\x01\x3f\x00\x4c" \
"\x01\x40\x00\x6c" \
"\x01\x41\x00\x4c" \
"\x01\x42\x00\x6c" \
"\x01\x43\x00\x4e" \
"\x01\x44\x00\x6e" \
"\x01\x45\x00\x4e" \
"\x01\x46\x00\x6e" \
"\x01\x47\x00\x4e" \
"\x01\x48\x00\x6e" \
"\x01\x49\x00\x6e" \
"\x00\xd2\x00\x4f" \
"\x00\xd3\x00\x4f" \
"\x00\xf3\x00\x6f" \
"\x00\xd4\x00\x4f" \
"\x00\xf4\x00\x6f" \
"\x00\xd5\x00\x4f" \
"\x00\xf5\x00\x6f" \
"\x01\x4c\x00\x4f" \
"\x01\x4d\x00\x6f" \
"\x01\x4e\x00\x4f" \
"\x01\x4f\x00\x6f" \
"\x01\x50\x00\x4f" \
"\x01\x51\x00\x6f" \
"\x01\xa0\x00\x4f" \
"\x01\xa1\x00\x6f" \
"\x01\xd1\x00\x4f" \
"\x01\xd2\x00\x6f" \
"\x1e\xcc\x00\x4f" \
"\x1e\xcd\x00\x6f" \
"\x1e\xce\x00\x4f" \
"\x1e\xcf\x00\x6f" \
"\x1e\xd0\x00\x4f" \
"\x1e\xd1\x00\x6f" \
"\x1e\xd2\x00\x4f" \
"\x1e\xd3\x00\x6f" \
"\x1e\xd4\x00\x4f" \
"\x1e\xd5\x00\x6f" \
"\x1e\xd6\x00\x4f" \
"\x1e\xd7\x00\x6f" \
"\x1e\xd8\x00\x4f" \
"\x1e\xd9\x00\x6f" \
"\x1e\xda\x00\x4f" \
"\x1e\xdb\x00\x6f" \
"\x1e\xdc\x00\x4f" \
"\x1e\xdd\x00\x6f" \
"\x1e\xde\x00\x4f" \
"\x1e\xdf\x00\x6f" \
"\x1e\xe0\x00\x4f" \
"\x1e\xe1\x00\x6f" \
"\x1e\xe2\x00\x4f" \
"\x1e\xe3\x00\x6f" \
"\x01\x54\x00\x52" \
"\x01\x55\x00\x72" \
"\x01\x56\x00\x52" \
"\x01\x57\x00\x72" \
"\x01\x58\x00\x52" \
"\x01\x59\x00\x72" \
"\x01\x5a\x00\x53" \
"\x01\x5b\x00\x73" \
"\x01\x5c\x00\x53" \
"\x01\x5d\x00\x73" \
"\x01\x5e\x00\x53" \
"\x01\x5f\x00\x73" \
"\x01\x60\x00\x53" \
"\x01\x61\x00\x73" \
"\x01\x62\x00\x54" \
"\x01\x63\x00\x74" \
"\x01\x64\x00\x54" \
"\x01\x65\x00\x74" \
"\x01\x66\x00\x54" \
"\x01\x67\x00\x74" \
"\x00\xd9\x00\x55" \
"\x00\xda\x00\x55" \
"\x00\xfa\x00\x75" \
"\x00\xdb\x00\x55" \
"\x00\xfb\x00\x75" \
"\x01\x68\x00\x55" \
"\x01\x69\x00\x75" \
"\x01\x6a\x00\x55" \
"\x01\x6b\x00\x75" \
"\x01\x6c\x00\x55" \
"\x01\x6d\x00\x75" \
"\x01\x6e\x00\x55" \
"\x01\x6f\x00\x75" \
"\x01\x70\x00\x55" \
"\x01\x71\x00\x75" \
"\x01\x72\x00\x55" \
"\x01\x73\x00\x75" \
"\x01\xaf\x00\x55" \
"\x01\xb0\x00\x75" \
"\x01\xd3\x00\x55" \
"\x01\xd4\x00\x75" \
"\x01\xd5\x00\x55" \
"\x01\xd6\x00\x75" \
"\x01\xd7\x00\x55" \
"\x01\xd8\x00\x75" \
"\x01\xd9\x00\x55" \
"\x01\xda\x00\x75" \
"\x01\xdb\x00\x55" \
"\x01\xdc\x00\x75" \
"\x1e\xe4\x00\x55" \
"\x1e\xe5\x00\x75" \
"\x1e\xe6\x00\x55" \
"\x1e\xe7\x00\x75" \
"\x1e\xe8\x00\x55" \
"\x1e\xe9\x00\x75" \
"\x1e\xea\x00\x55" \
"\x1e\xeb\x00\x75" \
"\x1e\xec\x00\x55" \
"\x1e\xed\x00\x75" \
"\x1e\xee\x00\x55" \
"\x1e\xef\x00\x75" \
"\x1e\xf0\x00\x55" \
"\x1e\xf1\x00\x75" \
"\x01\x74\x00\x57" \
"\x01\x75\x00\x77" \
"\x1e\x80\x00\x57" \
"\x1e\x81\x00\x77" \
"\x1e\x82\x00\x57" \
"\x1e\x83\x00\x77" \
"\x1e\x84\x00\x57" \
"\x1e\x85\x00\x77" \
"\x00\xdd\x00\x59" \
"\x00\xfd\x00\x79" \
"\x00\xff\x00\x79" \
"\x01\x76\x00\x59" \
"\x01\x77\x00\x79" \
"\x01\x78\x00\x59" \
"\x1e\xf2\x00\x59" \
"\x1e\xf3\x00\x75" \
"\x1e\xf4\x00\x59" \
"\x1e\xf5\x00\x79" \
"\x1e\xf6\x00\x59" \
"\x1e\xf7\x00\x79" \
"\x1e\xf8\x00\x59" \
"\x1e\xf9\x00\x79" \
"\x01\x79\x00\x5a" \
"\x01\x7a\x00\x7a" \
"\x01\x7b\x00\x5a" \
"\x01\x7c\x00\x7a" \
"\x01\x7d\x00\x5a" \
"\x01\x7e\x00\x7a" \
"\x01\xfc\x00\xc6" \
"\x01\xfd\x00\xe6" \
"\x01\xfe\x00\xd8" \
"\x01\xff\x00\xf8" \
"\x00\x00";

void EncodeDefault(unsigned char *dest, const unsigned char *src, size_t *len, gboolean UseExtensions, unsigned char *ExtraAlphabet)
{
	size_t 	i,current=0;
	int j,z;
	char 	ret;
	gboolean	FoundSpecial,FoundNormal;

#ifdef DEBUG
	DumpMessageText(&GSM_global_debug, src, (*len)*2);
#endif

	for (i = 0; i < *len; i++) {
		FoundSpecial = FALSE;
		j = 0;
		while (GSM_DefaultAlphabetCharsExtension[j][0]!=0x00 && UseExtensions) {
			if (src[i*2] 	== GSM_DefaultAlphabetCharsExtension[j][1] &&
			    src[i*2+1] 	== GSM_DefaultAlphabetCharsExtension[j][2]) {
				dest[current++] = 0x1b;
				dest[current++] = GSM_DefaultAlphabetCharsExtension[j][0];
				FoundSpecial 	= TRUE;
				break;
			}
			j++;
		}
		if (!FoundSpecial) {
			ret 		= '?';
			FoundNormal 	= FALSE;
			j 		= 0;
			while (GSM_DefaultAlphabetUnicode[j][1]!=0x00) {
				if (src[i*2]	== GSM_DefaultAlphabetUnicode[j][0] &&
				    src[i*2+1]	== GSM_DefaultAlphabetUnicode[j][1]) {
					ret 		= j;
					FoundNormal 	= TRUE;
					break;
				}
				j++;
			}
			if (ExtraAlphabet!=NULL && !FoundNormal) {
				j = 0;
				while (ExtraAlphabet[j] != 0x00 || ExtraAlphabet[j+1] != 0x00 || ExtraAlphabet[j+2] != 0x00) {
					if (ExtraAlphabet[j+1] == src[i*2] &&
					    ExtraAlphabet[j+2] == src[i*2 + 1]) {
						ret		= ExtraAlphabet[j];
						FoundSpecial	= TRUE;
						break;
					}
					j=j+3;
				}
			}
			if (!FoundNormal && !FoundSpecial) {
				j = 0;
				FoundNormal = FALSE;
				while (ConvertTable[j*4]   != 0x00 ||
				       ConvertTable[j*4+1] != 0x00) {
					if (src[i*2]   == ConvertTable[j*4] &&
					    src[i*2+1] == ConvertTable[j*4+1]) {
						z = 0;
						while (GSM_DefaultAlphabetUnicode[z][1]!=0x00) {
							if (ConvertTable[j*4+2]	== GSM_DefaultAlphabetUnicode[z][0] &&
							    ConvertTable[j*4+3]	== GSM_DefaultAlphabetUnicode[z][1]) {
								ret 		= z;
								FoundNormal 	= TRUE;
								break;
							}
							z++;
						}
						if (FoundNormal) break;
					}
					j++;
				}
			}
			dest[current++]=ret;
		}
	}
	dest[current]=0;
#ifdef DEBUG
	DumpMessageText(&GSM_global_debug, dest, current);
#endif

	*len = current;
}

/* You don't have to use ConvertTable here - 1 char is replaced there by 1 char */
void FindDefaultAlphabetLen(const unsigned char *src, size_t *srclen, size_t *smslen, size_t maxlen)
{
	size_t 	current=0,j,i;
	gboolean	FoundSpecial;

	i = 0;
	while (src[i*2] != 0x00 || src[i*2+1] != 0x00) {
		FoundSpecial = FALSE;
		j = 0;
		while (GSM_DefaultAlphabetCharsExtension[j][0]!=0x00) {
			if (src[i*2] 	== GSM_DefaultAlphabetCharsExtension[j][1] &&
			    src[i*2+1] 	== GSM_DefaultAlphabetCharsExtension[j][2]) {
				FoundSpecial = TRUE;
				if (current+2 > maxlen) {
					*srclen = i;
					*smslen = current;
					return;
				}
				current+=2;
				break;
			}
			j++;
		}
		if (!FoundSpecial) {
			if (current+1 > maxlen) {
				*srclen = i;
				*smslen = current;
				return;
			}
			current++;
		}
		i++;
	}
	*srclen = i;
	*smslen = current;
}

#define ByteMask ((1 << Bits) - 1)

int GSM_UnpackEightBitsToSeven(size_t offset, size_t in_length, size_t out_length,
                           const unsigned char *input, unsigned char *output)
{
	/* (c) by Pavel Janik and Pawel Kot */

        unsigned char *output_pos 	= output; /* Current pointer to the output buffer */
        const unsigned char *input_pos  	= input;  /* Current pointer to the input buffer */
        unsigned char Rest 	= 0x00;
        size_t	      Bits;

        Bits = offset ? offset : 7;

        while ((size_t)(input_pos - input) < in_length) {

                *output_pos = ((*input_pos & ByteMask) << (7 - Bits)) | Rest;
                Rest = *input_pos >> Bits;

                /* If we don't start from 0th bit, we shouldn't go to the
                   next char. Under *output_pos we have now 0 and under Rest -
                   _first_ part of the char. */
                if ((input_pos != input) || (Bits == 7)) output_pos++;
                input_pos++;

                if ((size_t)(output_pos - output) >= out_length) break;

                /* After reading 7 octets we have read 7 full characters but
                   we have 7 bits as well. This is the next character */
                if (Bits == 1) {
                        *output_pos = Rest;
                        output_pos++;
                        Bits = 7;
                        Rest = 0x00;
                } else {
                        Bits--;
                }
        }

        return output_pos - output;
}

int GSM_PackSevenBitsToEight(size_t offset, const unsigned char *input, unsigned char *output, size_t length)
{
	/* (c) by Pavel Janik and Pawel Kot */

        unsigned char 	*output_pos = output; /* Current pointer to the output buffer */
        const unsigned char 	*input_pos  = input;  /* Current pointer to the input buffer */
        int		Bits;             /* Number of bits directly copied to
                                           * the output buffer */
        Bits = (7 + offset) % 8;

        /* If we don't begin with 0th bit, we will write only a part of the
           first octet */
        if (offset) {
                *output_pos = 0x00;
                output_pos++;
        }

        while ((size_t)(input_pos - input) < length) {
                unsigned char Byte = *input_pos;

                *output_pos = Byte >> (7 - Bits);
                /* If we don't write at 0th bit of the octet, we should write
                   a second part of the previous octet */
                if (Bits != 7)
                        *(output_pos-1) |= (Byte & ((1 << (7-Bits)) - 1)) << (Bits+1);

                Bits--;

                if (Bits == -1) Bits = 7; else output_pos++;

                input_pos++;
        }
        return (output_pos - output);
}

GSM_Error GSM_UnpackSemiOctetNumber(GSM_Debug_Info *di, unsigned char *retval, const unsigned char *Number, size_t *pos, size_t bufferlength, gboolean semioctet)
{
	unsigned char	Buffer[GSM_MAX_NUMBER_LENGTH + 1];
	size_t		length		= Number[*pos];
	GSM_Error ret = ERR_NONE;

	smfprintf(di, "Number Length=%ld\n", (long)length);

	if (length == 0) {
		strcpy(Buffer, "");
		goto out;
	}

	/* Default ouput on error */
	strcpy(Buffer, "<NOT DECODED>");

	if (length > bufferlength) {
		smfprintf(di, "Number too long!\n");
		return ERR_UNKNOWN;
	}

	if (semioctet) {
		/* Convert number of semioctets to number of chars */
		if (length % 2) length++;
		length=length / 2 + 1;
	}

	/* Check length */
	if (length > GSM_MAX_NUMBER_LENGTH) {
		smfprintf(di, "Number too big, not decoding! (Length=%ld, MAX=%d)\n", (long)length, GSM_MAX_NUMBER_LENGTH);
		ret = ERR_UNKNOWN;
		goto out;
	}

	/*without leading byte with format of number*/
	length--;

	smfprintf(di, "Number type %02x (%d %d %d %d|%d %d %d %d)\n", Number[*pos + 1],
			Number[*pos + 1] & 0x80 ? 1 : 0,
			Number[*pos + 1] & 0x40 ? 1 : 0,
			Number[*pos + 1] & 0x20 ? 1 : 0,
			Number[*pos + 1] & 0x10 ? 1 : 0,
			Number[*pos + 1] & 0x08 ? 1 : 0,
			Number[*pos + 1] & 0x04 ? 1 : 0,
			Number[*pos + 1] & 0x02 ? 1 : 0,
			Number[*pos + 1] & 0x01 ? 1 : 0
			);

	if ((Number[*pos + 1] & 0x80) == 0) {
		smfprintf(di, "Numbering plan not supported!\n");
		ret = ERR_UNKNOWN;
		goto out;
	}

	switch ((Number[*pos + 1] & 0x70)) {
	case (NUMBER_ALPHANUMERIC_NUMBERING_PLAN_UNKNOWN & 0x70):
		if (length > 6) length++;
		smfprintf(di, "Alphanumeric number, length %ld\n", (long)length);
		GSM_UnpackEightBitsToSeven(0, length, length, Number+*pos+2, Buffer);
		Buffer[length]=0;
		break;
	case (NUMBER_INTERNATIONAL_NUMBERING_PLAN_ISDN & 0x70):
		smfprintf(di, "International number\n");
		Buffer[0]='+';
		DecodeBCD(Buffer+1,Number+*pos+2, length);
		break;
	default:
		DecodeBCD (Buffer, Number+*pos+2, length);
		break;
	}

	smfprintf(di, "Len %ld\n", (long)length);
out:
	EncodeUnicode(retval,Buffer,strlen(Buffer));
	if (semioctet) {
		*pos += 2 + ((Number[*pos] + 1) / 2);
	} else {
		*pos += 1 + Number[*pos];
	}
	return ret;
}

/**
 * Packing some phone numbers (SMSC, SMS destination and others)
 *
 * See GSM 03.40 9.1.1:
 * 1 byte  - length of number given in semioctets or bytes (when given in
 *           bytes, includes one byte for byte with number format).
 *           Returned by function (set semioctet to TRUE, if want result
 *           in semioctets).
 * 1 byte  - format of number (see GSM_NumberType in coding.h). Returned
 *           in unsigned char *Output.
 * n bytes - 2n or 2n-1 semioctets with number. Returned in unsigned char
 *           *Output.
 *
 * 1 semioctet = 4 bits = half of byte
 */
int GSM_PackSemiOctetNumber(const unsigned char *Number, unsigned char *Output, gboolean semioctet)
{
	unsigned char	format;
	size_t		length, i, skip = 0;
	unsigned char    *buffer;

	length = UnicodeLength(Number);
	buffer = (unsigned char*)malloc(length + 2);

	if (buffer == NULL) {
		return 0;
	}

	DecodeUnicode(Number, buffer);

	/* Checking for format number */
	if (buffer[0] == '+') {
		format = NUMBER_INTERNATIONAL_NUMBERING_PLAN_ISDN;
		skip = 1;
	} else if (buffer[0] == '0' && buffer[1] == '0' && buffer[2] == '0') {
		/* Most likely local provider number */
		format = NUMBER_UNKNOWN_NUMBERING_PLAN_ISDN;
	} else if (buffer[0] == '0' && buffer[1] == '0') {
		format = NUMBER_INTERNATIONAL_NUMBERING_PLAN_ISDN;
		skip = 2;
	} else if (buffer[0] == '+' && buffer[1] == '0' && buffer[2] == '0') {
		/* This is obviously wrong, but try to cope with that */
		format = NUMBER_INTERNATIONAL_NUMBERING_PLAN_ISDN;
		skip = 3;
	} else {
		format = NUMBER_UNKNOWN_NUMBERING_PLAN_ISDN;
	}
	for (i = 0; i < length; i++) {
		/* If there is something which can not be in normal
		 * number, mark it as alphanumberic */
		if (strchr("+0123456789*#pP", buffer[i]) == NULL) {
			format = NUMBER_ALPHANUMERIC_NUMBERING_PLAN_UNKNOWN;
		}
	}

	/**
	 * First byte is used for saving type of number. See GSM 03.40
	 * section 9.1.2.5
	 */
	Output[0]=format;

	/* After number type we will have number. GSM 03.40 section 9.1.2 */
	switch (format) {
	case NUMBER_ALPHANUMERIC_NUMBERING_PLAN_UNKNOWN:
		length=GSM_PackSevenBitsToEight(0, buffer, Output+1, strlen(buffer))*2;
		if (strlen(buffer)==7) length--;
		break;
	case NUMBER_INTERNATIONAL_NUMBERING_PLAN_ISDN:
		length -= skip;
		EncodeBCD (Output+1, buffer + skip, length, TRUE);
		break;
	default:
		EncodeBCD (Output+1, buffer, length, TRUE);
		break;
	}

	free(buffer);

	if (semioctet) return length;

	/* Convert number of semioctets to number of chars */
	if (length % 2) length++;
	return length / 2 + 1;
}

void CopyUnicodeString(unsigned char *Dest, const unsigned char *Source)
{
	int j = 0;

	/* No need to copy if both are on same address */
	if (Dest == Source) return;

	while (Source[j]!=0x00 || Source[j+1]!=0x00) {
		Dest[j]		= Source[j];
		Dest[j+1]	= Source[j+1];
		j=j+2;
	}
	Dest[j]		= 0;
	Dest[j+1]	= 0;
}

/* Changes minor/major order in Unicode string */
void ReverseUnicodeString(unsigned char *String)
{
	int 		j = 0;
	unsigned char	byte1, byte2;

	while (String[j]!=0x00 || String[j+1]!=0x00) {
		byte1		= String[j];
		byte2		= String[j+1];
		String[j+1]	= byte1;
		String[j]	= byte2;
		j=j+2;
	}
	String[j]	= 0;
	String[j+1]	= 0;
}

/* All input is in Unicode. First char can show Unicode minor/major order.
   Output is Unicode string in Gammu minor/major order */
void ReadUnicodeFile(unsigned char *Dest, const unsigned char *Source)
{
	int j = 0, current = 0;

	if (Source[0] == 0xFF && Source[1] == 0xFE) j = 2;
	if (Source[0] == 0xFE && Source[1] == 0xFF) j = 2;

	while (Source[j]!=0x00 || Source[j+1]!=0x00) {
		if (Source[0] == 0xFF) {
			Dest[current++] = Source[j+1];
			Dest[current++]	= Source[j];
		} else {
			Dest[current++] = Source[j];
			Dest[current++]	= Source[j+1];
		}
		j=j+2;
	}
	Dest[current++] = 0;
	Dest[current]	= 0;
}

int GetBit(unsigned char *Buffer, size_t BitNum)
{
	return Buffer[BitNum / 8] & (1 << (7 - (BitNum % 8)));
}

int SetBit(unsigned char *Buffer, size_t BitNum)
{
	return Buffer[BitNum / 8] |= 1 << (7 - (BitNum % 8));
}

int ClearBit(unsigned char *Buffer, size_t BitNum)
{
	return Buffer[BitNum / 8] &= 255 - (1 << (7 - (BitNum % 8)));
}

void BufferAlign(unsigned char *Destination, size_t *CurrentBit)
{
	int i=0;

	while(((*CurrentBit) + i) % 8 != 0) {
		ClearBit(Destination, (*CurrentBit)+i);
		i++;
	}

	(*CurrentBit) = (*CurrentBit) + i;
}

void BufferAlignNumber(size_t *CurrentBit)
{
	int i=0;

	while(((*CurrentBit) + i) % 8 != 0) {
		i++;
	}

	(*CurrentBit) = (*CurrentBit) + i;
}

void AddBuffer(unsigned char 	*Destination,
	       size_t		*CurrentBit,
	       unsigned char 	*Source,
	       size_t 		BitsToProcess)
{
	size_t i;

	for (i = 0; i < BitsToProcess; i++) {
		if (GetBit(Source, i)) {
			SetBit(Destination, (*CurrentBit)+i);
		} else {
			ClearBit(Destination, (*CurrentBit)+i);
		}
	}
	(*CurrentBit) = (*CurrentBit) + BitsToProcess;
}

void AddBufferByte(unsigned char *Destination,
		   size_t		 *CurrentBit,
		   unsigned char Source,
		   size_t		 BitsToProcess)
{
	AddBuffer(Destination, CurrentBit, &Source, BitsToProcess);
}

void GetBuffer(unsigned char *Source,
	       size_t	     *CurrentBit,
	       unsigned char *Destination,
	       size_t	     BitsToProcess)
{
	size_t i=0;

	while (i!=BitsToProcess) {
		if (GetBit(Source, (*CurrentBit)+i)) {
			SetBit(Destination, i);
		} else {
			ClearBit(Destination, i);
		}
		i++;
	}
	(*CurrentBit) = (*CurrentBit) + BitsToProcess;
}

void GetBufferInt(unsigned char *Source,
		  size_t	*CurrentBit,
		  int 		*integer,
		  size_t	BitsToProcess)
{
	size_t l=0,z=128,i=0;

	while (i!=BitsToProcess) {
		if (GetBit(Source, (*CurrentBit)+i)) l=l+z;
		z=z/2;
		i++;
	}
	*integer=l;
	(*CurrentBit) = (*CurrentBit) + i;
}

void GetBufferI(unsigned char 	*Source,
		size_t 		*CurrentBit,
		int 		*result,
		size_t 		BitsToProcess)
{
	size_t l=0,z,i=0;

	z = 1 << (BitsToProcess - 1);

	while (i!=BitsToProcess) {
		if (GetBit(Source, (*CurrentBit)+i)) l=l+z;
		z=z>>1;
		i++;
	}
	*result=l;
	(*CurrentBit) = (*CurrentBit) + i;
}

/* Unicode char 0x00 0x01 makes blinking in some Nokia phones.
 * We replace single ~ chars into it. When user give double ~, it's replaced
 * to single ~
 */
void EncodeUnicodeSpecialNOKIAChars(unsigned char *dest, const unsigned char *src, size_t len)
{
	size_t 	i,current = 0;
	gboolean 	special=FALSE;

	for (i = 0; i < len; i++) {
		if (special) {
			if (src[i*2] == 0x00 && src[i*2+1] == '~') {
				dest[current++]	= 0x00;
				dest[current++]	= '~';
			} else {
				dest[current++]	= 0x00;
				dest[current++]	= 0x01;
				dest[current++]	= src[i*2];
				dest[current++]	= src[i*2+1];
			}
			special = FALSE;
		} else {
			if (src[i*2] == 0x00 && src[i*2+1] == '~') {
				special = TRUE;
			} else {
				dest[current++]	= src[i*2];
				dest[current++]	= src[i*2+1];
			}
		}
	}
	if (special) {
		dest[current++]	= 0x00;
		dest[current++]	= 0x01;
	}
	dest[current++] = 0x00;
	dest[current] = 0x00;
}

void DecodeUnicodeSpecialNOKIAChars(unsigned char *dest, const unsigned char *src, size_t len)
{
	size_t i=0,current=0;

	for (i=0;i<len;i++) {
		switch (src[2*i]) {
		case 0x00:
			switch (src[2*i+1]) {
			case 0x01:
				dest[current++] = 0x00;
				dest[current++] = '~';
				break;
			case '~':
				dest[current++] = 0x00;
				dest[current++] = '~';
				dest[current++] = 0x00;
				dest[current++] = '~';
				break;
			default:
				dest[current++] = src[i*2];
				dest[current++] = src[i*2+1];
			}
			break;
		default:
			dest[current++] = src[i*2];
			dest[current++] = src[i*2+1];
		}
	}
	dest[current++] = 0x00;
	dest[current] = 0x00;
}


/* Compares two Unicode strings without regarding to case.
 * Return TRUE, when they're equal
 */
gboolean mywstrncasecmp(unsigned const  char *a, unsigned const  char *b, int num)
{
 	int 		i;
  	gammu_char_t 	wc,wc2;

        if (a == NULL || b == NULL) return FALSE;

	if (num == 0) num = -1;

	for (i = 0; i != num; i++) {
		if ((a[i*2] == 0x00 && a[i*2+1] == 0x00) && (b[i*2] == 0x00 && b[i*2+1] == 0x00)) return TRUE;
		if ((a[i*2] == 0x00 && a[i*2+1] == 0x00) || (b[i*2] == 0x00 && b[i*2+1] == 0x00)) return FALSE;
 		wc  = a[i*2+1] | (a[i*2] << 8);
 		wc2 = b[i*2+1] | (b[i*2] << 8);
 		if (towlower(wc) != towlower(wc2)) return FALSE;
 	}
	return TRUE;
}

/* wcscmp in Mandrake 9.0 is wrong */
gboolean mywstrncmp(unsigned const  char *a, unsigned const  char *b, int num)
{
	int i=0;

	while (1) {
		if (a[i*2] != b[i*2] || a[i*2+1] != b[i*2+1]) return FALSE;
		if (a[i*2] == 0x00 && a[i*2+1] == 0x00) return TRUE;
		i++;
		if (num == i) return TRUE;
	}
}

/* FreeBSD boxes 4.7-STABLE does't have it, although it's ANSI standard */
gboolean myiswspace(const unsigned char *src)
{
#ifndef HAVE_ISWSPACE
 	int 		o;
	unsigned char	dest[10];
#endif
 	gammu_char_t 	wc;

	wc = src[1] | (src[0] << 8);

#ifndef HAVE_ISWSPACE
	o = DecodeWithUnicodeAlphabet(wc, dest);
	if (o == 1) {
		if (isspace(((int)dest[0]))!=0) return TRUE;
		return FALSE;
	}
	return FALSE;
#else
	if (iswspace(wc)) return TRUE;
	return FALSE;
#endif
}

/*
 * Following code is based on wcsstr from the GNU C Library, original
 * comment follows:
 */
/*
 * The original strstr() file contains the following comment:
 *
 * My personal strstr() implementation that beats most other algorithms.
 * Until someone tells me otherwise, I assume that this is the
 * fastest implementation of strstr() in C.
 * I deliberately chose not to comment it.  You should have at least
 * as much fun trying to understand it, as I had to write it :-).
 *
 * Stephen R. van den Berg, berg@pool.informatik.rwth-aachen.de */

unsigned char *mywstrstr (const unsigned char *haystack, const unsigned char *needle)
{
/* One crazy define to convert unicode used in Gammu to standard gammu_char_t */
#define tolowerwchar(x) (towlower((gammu_char_t)( (((&(x))[0] & 0xff) << 8) | (((&(x))[1] & 0xff)) )))
	register gammu_int_t a, b, c;
	register const unsigned char *rhaystack, *rneedle;


	if ((b = tolowerwchar(*needle)) != L'\0') {
		haystack -= 2;				/* possible ANSI violation */
		do {
			haystack += 2;
			if ((c = tolowerwchar(*haystack)) == L'\0')
				goto ret0;
		} while (c != b);

		needle += 2;
		if ((c = tolowerwchar(*needle)) == L'\0')
			goto foundneedle;
		needle += 2;
		goto jin;

		for (;;) {
			do {
				haystack += 2;
				if ((a = tolowerwchar(*haystack)) == L'\0')
					goto ret0;
				if (a == b)
					break;
				haystack += 2;
				if ((a = tolowerwchar(*haystack)) == L'\0')
					goto ret0;
shloop:				;
			} while (a != b);

jin:			haystack += 2;
			if ((a = tolowerwchar(*haystack)) == L'\0')
				goto ret0;

			if (a != c)
				goto shloop;

			rhaystack = haystack + 2;
			haystack -= 2;
			rneedle = needle;
			if (tolowerwchar(*rhaystack) == (a = tolowerwchar(*rneedle)))
				do {
					if (a == L'\0')
						goto foundneedle;
					rhaystack += 2;
					needle += 2;
					if (tolowerwchar(*rhaystack) != (a = tolowerwchar(*needle)))
						break ;
					if (a == L'\0')
						goto foundneedle;
					rhaystack += 2;
					needle += 2;
				} while (tolowerwchar(*rhaystack) == (a = tolowerwchar(*needle)));

			needle = rneedle;		  /* took the register-poor approach */

			if (a == L'\0')
				break;
		}
	}
foundneedle:
	return (unsigned char *)haystack;
ret0:
	return NULL;
#undef tolowerwchar
}

GSM_Error MyGetLine(char *Buffer, size_t *Pos, char *OutBuffer, size_t MaxLen, size_t MaxOutLen, gboolean MergeLines)
{
	gboolean skip = FALSE;
	gboolean quoted_printable = FALSE;
	gboolean was_cr = FALSE, was_lf = FALSE;
	size_t pos;
	int tmp;

	OutBuffer[0] = 0;
	pos = 0;
	if (Buffer == NULL) return ERR_NONE;
	for (; (*Pos) < MaxLen; (*Pos)++) {
		switch (Buffer[*Pos]) {
		case 0x00:
			return ERR_NONE;
		case 0x0A:
		case 0x0D:
			if (skip) {
				if (Buffer[*Pos] == 0x0d) {
					if (was_cr && skip) return ERR_NONE;
					was_cr = TRUE;
				} else {
					if (was_lf && skip) return ERR_NONE;
					was_lf = TRUE;
				}
			} else {
				if (MergeLines) {
					/* (Quote printable new line) Does string end with = ? */
					if (quoted_printable && pos > 0 && OutBuffer[pos - 1] == '=') {
						pos--;
						OutBuffer[pos] = 0;
						skip = TRUE;
						was_cr = (Buffer[*Pos] == 0x0d);
						was_lf = (Buffer[*Pos] == 0x0a);
						break;
					}
					/* (vCard continuation) Next line start with space? */
					tmp = *Pos + 1;
					if (Buffer[*Pos + 1] == 0x0a || Buffer[*Pos + 1] == 0x0d) {
						tmp += 1;
					}
					if (Buffer[tmp] == ' ') {
						*Pos = tmp;
						break;
					}
					/* We ignore empty lines in this mode */
					if (pos == 0) {
						continue;
					}
				}
				if (Buffer[*Pos] == 0x0d && (*Pos)+1 < MaxLen && Buffer[*Pos + 1] == 0x0a) {
					/* Skip \r\n */
					(*Pos) += 2;
				} else {
					/* Skip single \r or \n */
					(*Pos)++;
				}
				return ERR_NONE;
			}
			break;
		default:
			/* Detect quoted printable for possible escaping */
			if (Buffer[*Pos] == ':' &&
					strstr(OutBuffer, ";ENCODING=QUOTED-PRINTABLE") != NULL) {
				quoted_printable = TRUE;
			}
			skip = FALSE;
			OutBuffer[pos]     = Buffer[*Pos];
			pos++;
			OutBuffer[pos] = 0;
			if (pos + 1 >= MaxOutLen) return ERR_MOREMEMORY;
		}
	}
	return ERR_NONE;
}

GSM_Error GSM_GetVCSLine(char **OutBuffer, char *Buffer, size_t *Pos, size_t MaxLen, gboolean MergeLines)
{
	gboolean skip = FALSE;
	gboolean quoted_printable = FALSE;
	gboolean was_cr = FALSE, was_lf = FALSE;
	size_t pos=0;
	int tmp=0;
	size_t OutLen = 200;

	*OutBuffer = (char *)malloc(OutLen);
	if (*OutBuffer == NULL) return ERR_MOREMEMORY;
	(*OutBuffer)[0] = 0;
	pos = 0;
	if (Buffer == NULL) return ERR_NONE;
	while ((*Pos) < MaxLen) {
		switch (Buffer[*Pos]) {
		case 0x00:
			return ERR_NONE;
		case 0x0A:
		case 0x0D:
			if (skip) {
				if (Buffer[*Pos] == 0x0d) {
					if (was_cr && skip) return ERR_NONE;
					was_cr = TRUE;
				} else {
					if (was_lf && skip) return ERR_NONE;
					was_lf = TRUE;
				}
			}
			if (pos != 0 && !skip) {
				if (MergeLines) {
					/* (Quote printable new line) Does string end with = ? */
					if ((*OutBuffer)[pos - 1] == '=' && quoted_printable) {
						pos--;
						(*OutBuffer)[pos] = 0;
						skip = TRUE;
						was_cr = (Buffer[*Pos] == 0x0d);
						was_lf = (Buffer[*Pos] == 0x0a);
						break;
					}
					/* (vCard continuation) Next line start with space? */
					tmp = *Pos + 1;
					if (Buffer[*Pos + 1] == 0x0a || Buffer[*Pos + 1] == 0x0d) {
						tmp += 1;
					}
					if (Buffer[tmp] == ' ') {
						*Pos = tmp;
						break;
					}
				}
				return ERR_NONE;
			}
			break;
		default:
			/* Detect quoted printable for possible escaping */
			if (Buffer[*Pos] == ':' &&
					strstr(*OutBuffer, ";ENCODING=QUOTED-PRINTABLE") != NULL) {
				quoted_printable = TRUE;
			}
			skip = FALSE;
			(*OutBuffer)[pos]     = Buffer[*Pos];
			pos++;
			(*OutBuffer)[pos] = 0;
			if (pos + 2 >= OutLen) {
				OutLen += 100;
				*OutBuffer = (char *)realloc(*OutBuffer, OutLen);
				if (*OutBuffer == NULL) return ERR_MOREMEMORY;
			}
		}
		(*Pos)++;
	}
	return ERR_NONE;
}


void StringToDouble(char *text, double *d)
{
	gboolean 		before=TRUE;
	double		multiply = 1;
	unsigned int 	i;

	*d = 0;
	for (i=0;i<strlen(text);i++) {
		if (isdigit((int)text[i])) {
			if (before) {
				(*d)=(*d)*10+(text[i]-'0');
			} else {
				multiply=multiply*0.1;
				(*d)=(*d)+(text[i]-'0')*multiply;
			}
		}
		if (text[i]=='.' || text[i]==',') before=FALSE;
	}
}

/* When char can be converted, convert it from Unicode to UTF8 */
int EncodeWithUTF8Alphabet(unsigned long src, unsigned char *ret)
{
	if (src < 0x80) {
		ret[0] = src;
		return 1;
	} else if (src < 0x800) {
		ret[0] = 192 + (src / 64);
		ret[1] = 128 + (src % 64);
		return 2;
	} else if (src < 0x10000) {
		ret[0] = 224 + (src / (64 * 64));
		ret[1] = 128 + ((src / 64) % 64);
		ret[2] = 128 + (src % 64);
		return 3;
	} else if (src < 0x200000) {
		ret[0] = 240 + (src / (64 * 64 * 64));
		ret[1] = 128 + ((src / (64 * 64)) % 64);
		ret[2] = 128 + ((src / 64) % 64);
		ret[3] = 128 + (src % 64);
		return 4;
	} else if (src < 0x4000000) {
		ret[0] = 248 + (src / (64 * 64 * 64 * 64));
		ret[1] = 128 + ((src / (64 * 64 * 64)) % 64);
		ret[2] = 128 + ((src / (64 * 64)) % 64);
		ret[3] = 128 + ((src / 64) % 64);
		ret[4] = 128 + (src % 64);
		return 5;
	} else if (src < 0x80000000L) {
		ret[0] = 252 + (src / (64 * 64 * 64 * 64 * 64));
		ret[1] = 128 + ((src / (64 * 64 * 64 * 64)) % 64);
		ret[2] = 128 + ((src / (64 * 64 * 64)) % 64);
		ret[3] = 128 + ((src / (64 * 64)) % 64);
		ret[4] = 128 + ((src / 64) % 64);
		ret[5] = 128 + (src % 64);
		return 6;
	}

	ret[0] = src;
	return 1;
}

/* Make UTF8 string from Unicode input string */
gboolean EncodeUTF8QuotedPrintable(char *dest, const unsigned char *src)
{
	size_t i, j=0, z, w, len;
	unsigned char	mychar[8];
	gboolean		retval = FALSE;
	unsigned long value, second;

	len = UnicodeLength(src);

	for (i = 0; i < len; i++) {
		value = src[i * 2] * 256 + src[i * 2 + 1];
		/* Decode UTF-16 */
		if (value >= 0xD800 && value <= 0xDBFF) {
			if ((i + 1) < len) {
				second = src[(i + 1) * 2] * 256 + src[(i + 1) * 2 + 1];
				if (second >= 0xDC00 && second <= 0xDFFF) {
					value = ((value - 0xD800) << 10) + (second - 0xDC00) + 0x010000;
				}
			} else {
				/* Surrogate at the end of string */
				value = 0xFFFD; /* REPLACEMENT CHARACTER */
			}
		}
		z = EncodeWithUTF8Alphabet(value, mychar);
		if (z == 1 && mychar[0] < 32) {
			/* Need quoted printable for chars < 32 */
			sprintf(dest + j, "=%02X", mychar[0]);
			j = j + 3;
		} else if (z == 1) {
			memcpy(dest + j, mychar, z);
			j += z;
		} else {
			/* Quoted printable unicode */
			for (w = 0; w < z; w++) {
				sprintf(dest + j, "=%02X", mychar[w]);
				j = j + 3;
			}
			if (z > 1) {
				retval = TRUE;
			}
	    	}
	}
	dest[j] = 0;
	return retval;
}

gboolean EncodeUTF8(char *dest, const unsigned char *src)
{
	size_t i, j=0, z, len;
	unsigned char	mychar[8];
	gboolean		retval = FALSE;
	unsigned long value, second;

	len = UnicodeLength(src);

	for (i = 0; i < len; i++) {
		value = src[i * 2] * 256 + src[i * 2 + 1];
		/* Decode UTF-16 */
		if (value >= 0xD800 && value <= 0xDBFF ) {
			if ((i + 1) < len) {
				second = src[(i + 1) * 2] * 256 + src[(i + 1) * 2 + 1];
				if (second >= 0xDC00 && second <= 0xDFFF) {
					i++;
					value = ((value - 0xD800) << 10) + (second - 0xDC00) + 0x010000;
				}
			} else {
				/* Surrogate at the end of string */
				value = 0xFFFD; /* REPLACEMENT CHARACTER */
			}
		}
		z = EncodeWithUTF8Alphabet(value, mychar);
		memcpy(dest + j, mychar, z);
		j += z;
		if (z > 1) {
			retval = TRUE;
		}
	}
	dest[j] = 0;
	return retval;
}

/* Decode UTF8 char to Unicode char */
int DecodeWithUTF8Alphabet(const unsigned char *src, gammu_char_t *dest, size_t len)
{
	gammu_char_t src0, src1, src2, src3;
	if (len < 1) {
		return 0;
	}
	src0 = src[0];

	// 1-byte sequence (no continuation bytes)
	if ((src0 & 0x80) == 0) {
		(*dest) = src0;
		return 1;
	}

	if (len < 2) {
		return 0;
	}
	src1 = src[1];

	// 2-byte sequence
	if ((src0 & 0xE0) == 0xC0) {
		(*dest) = ((src0 & 0x1F) << 6) | (src1 & 0x3f);
		if (*dest >= 0x80) {
			return 2;
		} else {
			return 0;
		}
	}

	if (len < 3) {
		return 0;
	}
	src2 = src[2];

	// 3-byte sequence (may include unpaired surrogates)
	if ((src0 & 0xF0) == 0xE0) {
		(*dest) = ((src0 & 0x0F) << 12) | ((src1 & 0x3f) << 6) | (src2 & 0x3f);
		if ((*dest) >= 0x0800) {
			if ((*dest) >= 0xD800 && (*dest) <= 0xDFFF) {
				return 0;
			}
			return 3;
		}
	}

	if (len < 4) {
		return 0;
	}
	src3 = src[3];

	// 4-byte sequence
	if ((src0 & 0xF8) == 0xF0) {
		(*dest) = ((src0 & 0x07) << 0x12) | ((src1 & 0x3f) << 0x0C) |
			((src2 & 0x3f) << 0x06) | (src3 & 0x3f);
		if ((*dest) >= 0x010000 && (*dest) <= 0x10FFFF) {
			return 4;
		}
	}

	return 0;
}


/* Make Unicode string from ISO-8859-1 string */
void DecodeISO88591QuotedPrintable(unsigned char *dest, const unsigned char *src, size_t len)
{
	size_t 		i = 0, j = 0;

	while (i < len) {
		if (src[i] == '=' && i + 2 < len
			&& DecodeWithHexBinAlphabet(src[i + 1]) != -1
			&& DecodeWithHexBinAlphabet(src[i + 2]) != -1) {
			dest[j++] = 0;
			dest[j++] = 16 * DecodeWithHexBinAlphabet(src[i + 1]) + DecodeWithHexBinAlphabet(src[i + 2]);
			i += 2;
		} else {
			dest[j++] = 0;
			dest[j++] = src[i];
		}
		i++;
	}
	dest[j++] = 0;
	dest[j] = 0;
}

/* Make Unicode string from UTF8 string */
void DecodeUTF8QuotedPrintable(unsigned char *dest, const char *src, size_t len)
{
	size_t 		i,j=0;
	int		z;
	unsigned char	mychar[10];
	gammu_char_t		ret;

	for (i = 0; i<=len; ) {
		z=0;
		while (TRUE) {
			if (src[z*3+i] != '=' || z*3+i+3>len ||
			    DecodeWithHexBinAlphabet(src[z*3+i+1])==-1 ||
			    DecodeWithHexBinAlphabet(src[z*3+i+2])==-1) {
				break;
			}
			mychar[z] = 16*DecodeWithHexBinAlphabet(src[z*3+i+1])+DecodeWithHexBinAlphabet(src[z*3+i+2]);
			z++;
			/* Is it plain ASCII? */
			if (z == 1 && mychar[0] < 194) break;
			/* Do we already have valid UTF-8 char? */
			if (DecodeWithUTF8Alphabet(mychar, &ret, z) == z) break;
		}
		if (z>0) {
			i += z * 3;
			/*  we ignore wrong sequence */
			if (DecodeWithUTF8Alphabet(mychar, &ret, z)==0) continue;
		} else {
			i += EncodeWithUnicodeAlphabet(&src[i], &ret);
		}
		if (StoreUTF16(dest + j, ret)) {
			j += 4;
		} else {
			j += 2;
		}
	}
	dest[j++] = 0;
	dest[j] = 0;
}

void DecodeUTF8(unsigned char *dest, const char *src, size_t len)
{
	size_t 		i=0,j=0,z;
	gammu_char_t		ret;

	while (i < len) {
		z = DecodeWithUTF8Alphabet(src+i, &ret, len - i);
		if (z < 1) {
			break;
		}
		i += z;
		if (StoreUTF16(dest + j, ret)) {
			j += 4;
		} else {
			j += 2;
		}
	}
	dest[j++] = 0;
	dest[j] = 0;
}

void DecodeXMLUTF8(unsigned char *dest, const char *src, size_t len)
{
	char *tmp;
	char *pos, *pos_end;
	const char *lastpos;
	char *entity;
	unsigned long long int c;
	int tmplen;

	/* Allocate buffer */
	tmp = (char *)calloc(2 * len, sizeof(char));
	if (tmp == NULL) {
		/* We have no memory for XML decoding */
		DecodeUTF8(dest, src, len);
		return;
	}
	if (src == NULL) {
		*dest = 0;
		free(tmp);
		return;
	}

	/* Find ampersand and decode the */
	lastpos = src;
	while ((*lastpos != 0) && ((pos = strchr(lastpos, '&')) != NULL)) {
		/* Store current string */
		strncat(tmp, lastpos, pos - lastpos);
		lastpos = pos;
		/* Skip ampersand */
		pos++;
		/* Detect end of string */
		if (*pos == 0) break;
		/* Find entity length */
		pos_end = strchr(pos, ';');
		if (pos_end - pos > 6 || pos_end == NULL) {
			if (pos_end == NULL) {
				dbgprintf(NULL, "No entity end found, ignoring!\n");
			} else {
				dbgprintf(NULL, "Too long html entity, ignoring!\n");
			}
			strncat(tmp, lastpos, 1);
			lastpos++;
			continue;
		}
		/* Create entity */
		/* strndup would be better, but not portable */
		entity = strdup(pos);
		if (entity == NULL) break;
		entity[pos_end - pos] = 0;
		dbgprintf(NULL, "Found XML entity: %s\n", entity);
		if (entity[0] == '#') {
			if (entity[1] == 'x' || entity[1] == 'X') {
				c = strtoull(entity + 2, NULL, 16);
			} else {
				c = strtoull(entity + 1, NULL, 10);
			}
			dbgprintf(NULL, "Unicode char 0x%04llx\n", c);
			tmplen = strlen(tmp);
			tmplen += EncodeWithUTF8Alphabet(c, tmp + tmplen);
			tmp[tmplen] = 0;
		} else if (strcmp(entity, "amp") == 0) {
			strcat(tmp, "&");
		} else if (strcmp(entity, "apos") == 0) {
			strcat(tmp, "'");
		} else if (strcmp(entity, "gt") == 0) {
			strcat(tmp, ">");
		} else if (strcmp(entity, "lt") == 0) {
			strcat(tmp, "<");
		} else if (strcmp(entity, "quot") == 0) {
			strcat(tmp, "\"");
		} else {
			dbgprintf(NULL, "Could not decode XML entity!\n");
			strncat(tmp, lastpos, pos_end - pos + 1);
		}
		free(entity);
		entity=NULL;
		lastpos = pos_end + 1;
	}
	/* Copy rest of string */
	strcat(tmp, lastpos);
	DecodeUTF8(dest, tmp, strlen(tmp));
	free(tmp);
	tmp=NULL;
}

void DecodeUTF7(unsigned char *dest, const unsigned char *src, size_t len)
{
	size_t 		i=0,j=0,z,p;
	gammu_char_t		ret;

	while (i<=len) {
		if (len-5>=i) {
			if (src[i] == '+') {
				z=0;
				while (src[z+i+1] != '-' && z+i+1<len) z++;
				p=DecodeBASE64(src+i+1, dest+j, z);
				if (p%2 != 0) p--;
				j+=p;
				i+=z+2;
			} else {
				i+=EncodeWithUnicodeAlphabet(&src[i], &ret);
				if (StoreUTF16(dest + j, ret)) {
					j += 4;
				} else {
					j += 2;
				}
			}
		} else {
			i+=EncodeWithUnicodeAlphabet(&src[i], &ret);
			if (StoreUTF16(dest + j, ret)) {
				j += 4;
			} else {
				j += 2;
			}
		}
	}
	dest[j++] = 0;
	dest[j] = 0;
}

/*
Bob Trower 08/04/01
Copyright (c) Trantor Standard Systems Inc., 2001

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the
Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall
be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

static void EncodeBASE64Block(const unsigned char in[3], char out[4], const size_t len)
{
	/* BASE64 translation Table as described in RFC1113 */
	unsigned char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	out[0] = cb64[ in[0] >> 2 ];
	out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
	out[2] = (unsigned char) (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
	out[3] = (unsigned char) (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}

void EncodeBASE64(const unsigned char *Input, char *Output, const size_t Length)
{
	unsigned char 	in[3], out[4];
	size_t 		i, pos = 0, len, outpos = 0;

	while (pos < Length) {
		len = 0;
		for (i = 0; i < 3; i++) {
			in[i] = 0;
			if (pos < Length) {
				in[i] = Input[pos];
				len++;
				pos++;
			}
		}
	        if(len) {
			EncodeBASE64Block(in, out, len);
			for (i = 0; i < 4; i++) Output[outpos++] = out[i];
		}
        }

	Output[outpos] = 0;
}

static void DecodeBASE64Block(const char in[4], unsigned char out[3])
{
	out[0] = (unsigned char) ((in[0] << 2) | (in[1] >> 4));
	out[1] = (unsigned char) ((in[1] << 4) | (in[2] >> 2));
	out[2] = (unsigned char) (((in[2] << 6) & 0xc0) | in[3]);
}

int DecodeBASE64(const char *Input, unsigned char *Output, const size_t Length)
{
	unsigned char 	cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";
	unsigned char 	in[4], out[3], v;
	size_t 		i, len, pos = 0, outpos = 0;

	while (pos < Length) {
		len = 0;
	        for(i = 0; i < 4; i++) {
	       		v = 0;
			while(v == 0) {
				if (pos >= Length) break;
		                v = (unsigned char) Input[pos++];
                		v = (unsigned char) ((v < 43 || v > 122) ? 0 : cd64[ v - 43 ]);
				if (v) v = (unsigned char) ((v == '$') ? 0 : v - 61);
			}
			if(pos<=Length) {
				if (v) {
					len++;
					in[i] = (unsigned char) (v - 1);
				}
			}
		}
		if (len) {
			DecodeBASE64Block(in, out);
			for(i = 0; i < len - 1; i++) Output[outpos++] = out[i];
		}
	}
	Output[outpos] = 0;
	return outpos;
}

#ifdef ICONV_FOUND

#include <iconv.h>

#ifdef ICONV_SECOND_ARGUMENT_IS_CONST
#  define SECOND_ICONV_ARG const char *
#else
#  define SECOND_ICONV_ARG char *
#endif

gboolean IconvDecode(const char *charset, const char *input, const size_t inlen, unsigned char *output, size_t outlen)
{
	iconv_t ic;
	/* Add one to convert also trailing zero, this is broken for
	 * multibyte input, but we don't use iconv for this so far */
	size_t rest = inlen + 1;
	SECOND_ICONV_ARG in;
	char *out;

	ic = iconv_open("UCS-2BE", charset);
	if (ic == (iconv_t)(-1)) return FALSE;

	/* I know I loose const here, but it's iconv choice... */
	in = (SECOND_ICONV_ARG)input;
	out = output;
	iconv(ic, &in, &rest, &out, &outlen);

	iconv_close(ic);

	return (rest == 0);
}

gboolean IconvEncode(const char *charset, const unsigned char *input, const size_t inlen, char *output, size_t outlen)
{
	iconv_t ic;
	size_t rest = inlen;
	SECOND_ICONV_ARG in;
	char *out;

	ic = iconv_open(charset, "UCS-2BE");
	if (ic == (iconv_t)(-1)) return FALSE;

	/* I know I loose const here, but it's iconv choice... */
	in = (SECOND_ICONV_ARG)input;
	out = output;
	iconv(ic, &in, &rest, &out, &outlen);

	iconv_close(ic);

	return (rest == 0);
}
#endif


/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

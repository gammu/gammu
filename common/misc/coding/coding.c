/* (c) 2002-2004 by Marcin Wiacek, Michal Cihar and others */
/* based on some work from MyGnokii (www.mwiacek.com) */
/* based on some work from Gnokii (www.gnokii.org)
 * (C) 1999-2000 Hugh Blemings & Pavel Janik ml. (C) 2001-2004 Pawel Kot 
 * GNU GPL version 2 or later
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#ifndef __OpenBSD__
#  include <wctype.h>
#endif
#ifdef WIN32
#  include "windows.h"
#endif

#include "../misc.h"
#include "coding.h"

/* function changes #10 #13 chars to \n \r */
char *EncodeUnicodeSpecialChars(unsigned char *buffer)
{
	int 			Pos=0, Pos2=0;
	static unsigned char	Buf[20000];

	while (buffer[Pos*2]!=0x00 || buffer[Pos*2+1]!=0x00) {
		if (buffer[Pos*2] == 0x00 && buffer[Pos*2+1] == 10) {
			Buf[Pos2*2]   = 0x00;
			Buf[Pos2*2+1] = '\\';
			Pos2++;
			Buf[Pos2*2]   = 0x00;
			Buf[Pos2*2+1] = 'n';
			Pos2++;
		} else if (buffer[Pos*2] == 0x00 && buffer[Pos*2+1] == 13) {
			Buf[Pos2*2]   = 0x00;
			Buf[Pos2*2+1] = '\\';
			Pos2++;
			Buf[Pos2*2]   = 0x00;
			Buf[Pos2*2+1] = 'r';
			Pos2++;
		} else if (buffer[Pos*2] == 0x00 && buffer[Pos*2+1] == '\\') {
			Buf[Pos2*2]   = 0x00;
			Buf[Pos2*2+1] = '\\';
			Pos2++;
			Buf[Pos2*2]   = 0x00;
			Buf[Pos2*2+1] = '\\';
			Pos2++;
		} else {
			Buf[Pos2*2]   = buffer[Pos*2];
			Buf[Pos2*2+1] = buffer[Pos*2+1];
			Pos2++;
		}
		Pos++;
	}
	Buf[Pos2*2]   = 0;
	Buf[Pos2*2+1] = 0;
	return Buf;
}

/* function changes #10 #13 chars to \n \r */
char *EncodeSpecialChars(unsigned char *buffer)
{
	int 			Pos=0, Pos2=0;
	static unsigned char	Buf[10000];

	while (buffer[Pos]!=0x00) {
		switch (buffer[Pos]) {
		case 10:
			Buf[Pos2++] = '\\';
			Buf[Pos2++] = 'n';
			break;
		case 13: 
			Buf[Pos2++] = '\\';
			Buf[Pos2++] = 'r';
			break;
		case '\\':
			Buf[Pos2++] = '\\';
			Buf[Pos2++] = '\\';
			break;
		default:
			Buf[Pos2++] = buffer[Pos];
		}
		Pos++;
	}
	Buf[Pos2] = 0;
	return Buf;
}

char *DecodeUnicodeSpecialChars(unsigned char *buffer)
{
	int 			Pos=0, Pos2=0, level=0;
	static unsigned char	Buf[10000];

	while (buffer[Pos*2]!=0x00 || buffer[Pos*2+1]!=0x00) {
		Buf[Pos2*2]   = buffer[Pos*2];
		Buf[Pos2*2+1] = buffer[Pos*2+1];
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
				Buf[Pos2*2]   = 0;
				Buf[Pos2*2+1] = 10;
			}
			if (buffer[Pos*2] == 0x00 && buffer[Pos*2+1] == 'r') {
				Buf[Pos2*2]   = 0;
				Buf[Pos2*2+1] = 13;
			}
			if (buffer[Pos*2] == 0x00 && buffer[Pos*2+1] == '\\') {
				Buf[Pos2*2]   = 0;
				Buf[Pos2*2+1] = '\\';
			}
			Pos2++;
			level = 0;
		}
		Pos++;
	}
	Buf[Pos2*2]   = 0;
	Buf[Pos2*2+1] = 0;
	return Buf;
}

char *DecodeSpecialChars(unsigned char *buffer)
{
	int 			Pos=0, Pos2=0, level=0;
	static unsigned char	Buf[10000];

	while (buffer[Pos]!=0x00) {
		Buf[Pos2] = buffer[Pos];
		switch (level) {
		case 0:
			if (buffer[Pos] == '\\') {
				level = 1;
			} else {
				Pos2++;
			}
			break;
		case 1:
			if (buffer[Pos] == 'n') Buf[Pos2] = 10;
			if (buffer[Pos] == 'r') Buf[Pos2] = 13;
			if (buffer[Pos] == '\\') Buf[Pos2] = '\\';
			Pos2++;
			level = 0;
		}
		Pos++;
	}
	Buf[Pos2] = 0;
	return Buf;
}

char *mystrcasestr(unsigned const char *a, unsigned const char *b)
{
	unsigned char 	A[2000], B[200];
	int 		i;

	memset(A,0,sizeof(A));
	memset(B,0,sizeof(B));
	for (i=0;i<(int)strlen(a);i++) A[i] = tolower(a[i]);
	for (i=0;i<(int)strlen(b);i++) B[i] = tolower(b[i]);

	return strstr(A,B);
}

unsigned int UnicodeLength(const unsigned char *str)
{
	unsigned int len = 0;

	if (str == NULL) return 0;

	while(str[len*2] != 0 || str[len*2+1] != 0) len++;

	return len;
}

/* Convert Unicode char saved in src to dest */
unsigned int EncodeWithUnicodeAlphabet(const unsigned char *src, wchar_t *dest)
{
	char retval;

        switch (retval = mbtowc(dest, src, MB_CUR_MAX)) {
                case -1 :
		case  0 : return 1;
                default : return retval;
        }
}

/* Convert Unicode char saved in src to dest */
unsigned int DecodeWithUnicodeAlphabet(wchar_t src, unsigned char *dest)
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

void DecodeUnicode (const unsigned char *src, unsigned char *dest)
{
 	int 		i=0,o=0;
 	wchar_t 	wc;

 	while (src[(2*i)+1]!=0x00 || src[2*i]!=0x00) {
 		wc = src[(2*i)+1] | (src[2*i] << 8);
		o += DecodeWithUnicodeAlphabet(wc, dest + o);
 		i++;
 	}
	dest[o]=0;
}

/* Decode Unicode string and return as function result */
unsigned char *DecodeUnicodeString (const unsigned char *src)
{
 	static char dest[500];

	DecodeUnicode(src,dest);
	return dest;
}

/* Decode Unicode string to UTF8 or other console charset
 * and return as function result
 */
unsigned char *DecodeUnicodeConsole(const unsigned char *src)
{
 	static char dest[500];

	if (di.coding[0] != 0) {
		if (!strcmp(di.coding,"utf8")) {
			EncodeUTF8(dest, src);
		} else {
#ifdef WIN32
			setlocale(LC_ALL, di.coding);
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
void EncodeUnicode (unsigned char *dest, const unsigned char *src, int len)
{
	int 		i_len = 0, o_len;
 	wchar_t 	wc;
 
	for (o_len = 0; i_len < len; o_len++) {
		i_len += EncodeWithUnicodeAlphabet(&src[i_len], &wc);
		dest[o_len*2]		= (wc >> 8) & 0xff;
		dest[(o_len*2)+1]	= wc & 0xff;
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

void DecodeBCD (unsigned char *dest, const unsigned char *src, int len)
{
	int i,current=0,digit;

	for (i = 0; i < len; i++) {
	        digit=src[i] & 0x0f;
                if (digit<10) dest[current++]=digit + '0';
	        digit=src[i] >> 4;
                if (digit<10) dest[current++]=digit + '0';
	}
	dest[current++]=0;
}

void EncodeBCD (unsigned char *dest, const unsigned char *src, int len, bool fill)
{
	int i,current=0;

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
	if (mychar>='A' && mychar<='F') return mychar-'A'+10;
	if (mychar>='a' && mychar<='f') return mychar-'a'+10;
	if (mychar>='0' && mychar<='9') return mychar-'0';
	return -1;
}

unsigned char EncodeWithHexBinAlphabet (int digit)
{
	if (digit >= 0 && digit <= 9) return '0'+(digit);
	if (digit >=10 && digit <=15) return 'A'+(digit-10);
	return 0;
}

void DecodeHexUnicode (unsigned char *dest, const unsigned char *src, int len)
{
	int 	i,current=0;
	bool 	first = false;

	if (len != 0 && src[0] == '0' && src[1] == '0') {
		first = true;
	} else if (len != 0 && src[2] == '0' && src[3] == '0') {
		first = false;
	} else {
		first = (10 * (src[0] - '0') + (src[1] - '0')) < (10 * (src[2] - '0')+ (src[3] - '0'));
	}
	for (i = 0; i < len/4 ; i++) {
		if (first) {
			dest[current++] = DecodeWithHexBinAlphabet(src[i*4+0])*16+
					  DecodeWithHexBinAlphabet(src[i*4+1]);
			dest[current++] = DecodeWithHexBinAlphabet(src[i*4+2])*16+
					  DecodeWithHexBinAlphabet(src[i*4+3]);
		} else {
			dest[current++] = DecodeWithHexBinAlphabet(src[i*4+2])*16+
					  DecodeWithHexBinAlphabet(src[i*4+3]);
			dest[current++] = DecodeWithHexBinAlphabet(src[i*4+0])*16+
					  DecodeWithHexBinAlphabet(src[i*4+1]);
		}
	}
	dest[current++] = 0;
	dest[current++] = 0;
}

void EncodeHexUnicode (unsigned char *dest, const unsigned char *src, int len)
{
	int i,current=0;

	for (i = 0; i < len; i++) {
		dest[current++] = EncodeWithHexBinAlphabet(src[2*i] >> 0x04);
		dest[current++] = EncodeWithHexBinAlphabet(src[2*i] & 0x0f);
		dest[current++] = EncodeWithHexBinAlphabet(src[2*i+1] >> 0x04);
		dest[current++] = EncodeWithHexBinAlphabet(src[2*i+1] & 0x0f);
	}
	dest[current++] = 0;
}

void DecodeHexBin (unsigned char *dest, const unsigned char *src, int len)
{
	int i,current=0;

	for (i = 0; i < len/2 ; i++) {
		dest[current++] = DecodeWithHexBinAlphabet(src[i*2])*16+
				  DecodeWithHexBinAlphabet(src[i*2+1]);
	}
	dest[current++] = 0;
}

void EncodeHexBin (unsigned char *dest, const unsigned char *src, int len)
{
	int i,current=0;

	for (i = 0; i < len; i++) {
		dest[current++] = EncodeWithHexBinAlphabet(src[i] >> 0x04);
		dest[current++] = EncodeWithHexBinAlphabet(src[i] & 0x0f);
	}
	dest[current++] = 0;
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
 * 1. two first chars means sequence of chars from GSM default alphabet
 * 2. two second is target (encoded) char saved in Unicode
 */
static unsigned char GSM_DefaultAlphabetCharsExtension[][4] =
{
	{0x1b,0x14,0x00,0x5e},	/* ^	*/
	{0x1b,0x28,0x00,0x7b},	/* {	*/
	{0x1b,0x29,0x00,0x7d},	/* }	*/
	{0x1b,0x2f,0x00,0x5c},	/* \	*/
	{0x1b,0x3c,0x00,0x5b},	/* [	*/
	{0x1b,0x3d,0x00,0x7E},	/* ~	*/
	{0x1b,0x3e,0x00,0x5d},	/* ]	*/
	{0x1b,0x40,0x00,0x7C},	/* |	*/
	{0x1b,0x65,0x20,0xAC},	/* Euro */
	{0x00,0x00,0x00,0x00}
};

void DecodeDefault (unsigned char *dest, const unsigned char *src, int len, bool UseExtensions, unsigned char *ExtraAlphabet)
{
	int 	i,current=0,j;
	bool	FoundSpecial = false;

#ifdef DEBUG
	if (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE) DumpMessage(di.df, di.dl, src, len);
#endif

	for (i = 0; i < len; i++) {
		FoundSpecial = false;
		if ((i < (len-1)) && UseExtensions) {
			j=0;
			while (GSM_DefaultAlphabetCharsExtension[j][0]!=0x00) {
				if (GSM_DefaultAlphabetCharsExtension[j][0]==src[i] && 
				    GSM_DefaultAlphabetCharsExtension[j][1]==src[i+1]) {
					FoundSpecial = true;
					dest[current++] = GSM_DefaultAlphabetCharsExtension[j][2];
					dest[current++] = GSM_DefaultAlphabetCharsExtension[j][3];
					i++;
					break;
				}
				j++;
			}
		}
       		if (ExtraAlphabet!=NULL && !FoundSpecial) {
			j = 0;
			while (ExtraAlphabet[j] != 0x00 || ExtraAlphabet[j+1] != 0x00 || ExtraAlphabet[j+2] != 0x00) {
				if (ExtraAlphabet[j] == src[i]) {
					dest[current++] = ExtraAlphabet[j+1];
					dest[current++] = ExtraAlphabet[j+2];
					FoundSpecial 	= true;
                            		break;
                        	}
                        	j=j+3;
                    	}
                }
		if (!FoundSpecial) {
			dest[current++] = GSM_DefaultAlphabetUnicode[src[i]][0];
			dest[current++] = GSM_DefaultAlphabetUnicode[src[i]][1];
		}
	}
	dest[current++]=0;
	dest[current++]=0;
#ifdef DEBUG
	if (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE) DumpMessage(di.df, di.dl, dest, UnicodeLength(dest)*2);
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
"\x00\xc0\x00\x41\x00\xe0\x00\x61\x00\xc1\x00\x41\x00\xe1\x00\x61\x00\xc2\x00\x41\x00\xe2\x00\x61\x00\xc3\x00\x41\x00\xe3\x00\x61\x1e\xa0\x00\x41\x1e\xa1\x00\x61\x1e\xa2\x00\x41\x1e\xa3\x00\x61\x1e\xa4\x00\x41\x1e\xa5\x00\x61\x1e\xa6\x00\x41\x1e\xa7\x00\x61\x1e\xa8\x00\x41\x1e\xa9\x00\x61\x1e\xaa\x00\x41\x1e\xab\x00\x61\x1e\xac\x00\x41\x1e\xad\x00\x61\x1e\xae\x00\x41\x1e\xaf\x00\x61\x1e\xb0\x00\x41\x1e\xb1\x00\x61\x1e\xb2\x00\x41\x1e\xb3\x00\x61\x1e\xb4\x00\x41\x1e\xb5\x00\x61\x1e\xb6\x00\x41\x1e\xb7\x00\x61\x01\xcd\x00\x41\x01\xce\x00\x61\x01\x00\x00\x41\x01\x01\x00\x61\x01\x02\x00\x41\x01\x03\x00\x61\x01\x04\x00\x41\x01\x05\x00\x61\x01\xfb\x00\x61\x01\x06\x00\x43\x01\x07\x00\x63\x01\x08\x00\x43\x01\x09\x00\x63\x01\x0a\x00\x43\x01\x0b\x00\x63\x01\x0c\x00\x43\x01\x0d\x00\x63\x00\xe7"\
"\x00\x63\x01\x0e\x00\x44\x01\x0f\x00\x64\x01\x10\x00\x44\x01\x11\x00\x64\x00\xc8\x00\x45\x00\xca\x00\x45\x00\xea\x00\x65\x00\xcb\x00\x45\x00\xeb\x00\x65\x1e\xb8\x00\x45\x1e\xb9\x00\x65\x1e\xba\x00\x45\x1e\xbb\x00\x65\x1e\xbc\x00\x45\x1e\xbd\x00\x65\x1e\xbe\x00\x45\x1e\xbf\x00\x65\x1e\xc0\x00\x45\x1e\xc1\x00\x65\x1e\xc2\x00\x45\x1e\xc3\x00\x65\x1e\xc4\x00\x45\x1e\xc5\x00\x65\x1e\xc6\x00\x45\x1e\xc7\x00\x65\x01\x12\x00\x45\x01\x13\x00\x65\x01\x14\x00\x45\x01\x15\x00\x65\x01\x16\x00\x45\x01\x17\x00\x65\x01\x18\x00\x45\x01\x19\x00\x65\x01\x1a\x00\x45\x01\x1b\x00\x65\x01\x1c\x00\x47\x01\x1d\x00\x67\x01\x1e\x00\x47\x01\x1f\x00\x67\x01\x20\x00\x47\x01\x21\x00\x67\x01\x22\x00\x47\x01\x23\x00\x67\x01\x24\x00\x48\x01\x25\x00\x68\x01\x26\x00\x48\x01\x27\x00\x68\x00\xcc\x00\x49\x00\xcd\x00\x49\x00\xed"\
"\x00\x69\x00\xce\x00\x49\x00\xee\x00\x69\x00\xcf\x00\x49\x00\xef\x00\x69\x01\x28\x00\x49\x01\x29\x00\x69\x01\x2a\x00\x49\x01\x2b\x00\x69\x01\x2c\x00\x49\x01\x2d\x00\x69\x01\x2e\x00\x49\x01\x2f\x00\x69\x01\x30\x00\x49\x01\x31\x00\x69\x01\xcf\x00\x49\x01\xd0\x00\x69\x1e\xc8\x00\x49\x1e\xc9\x00\x69\x1e\xca\x00\x49\x1e\xcb\x00\x69\x01\x34\x00\x4a\x01\x35\x00\x6a\x01\x36\x00\x4b\x01\x37\x00\x6b\x01\x39\x00\x4c\x01\x3a\x00\x6c\x01\x3b\x00\x4c\x01\x3c\x00\x6c\x01\x3d\x00\x4c\x01\x3e\x00\x6c\x01\x3f\x00\x4c\x01\x40\x00\x6c\x01\x41\x00\x4c\x01\x42\x00\x6c\x01\x43\x00\x4e\x01\x44\x00\x6e\x01\x45\x00\x4e\x01\x46\x00\x6e\x01\x47\x00\x4e\x01\x48\x00\x6e\x01\x49\x00\x6e\x00\xd2\x00\x4f\x00\xd3\x00\x4f\x00\xf3\x00\x6f\x00\xd4\x00\x4f\x00\xf4\x00\x6f\x00\xd5\x00\x4f\x00\xf5\x00\x6f\x01\x4c\x00\x4f\x01\x4d"\
"\x00\x6f\x01\x4e\x00\x4f\x01\x4f\x00\x6f\x01\x50\x00\x4f\x01\x51\x00\x6f\x01\xa0\x00\x4f\x01\xa1\x00\x6f\x01\xd1\x00\x4f\x01\xd2\x00\x6f\x1e\xcc\x00\x4f\x1e\xcd\x00\x6f\x1e\xce\x00\x4f\x1e\xcf\x00\x6f\x1e\xd0\x00\x4f\x1e\xd1\x00\x6f\x1e\xd2\x00\x4f\x1e\xd3\x00\x6f\x1e\xd4\x00\x4f\x1e\xd5\x00\x6f\x1e\xd6\x00\x4f\x1e\xd7\x00\x6f\x1e\xd8\x00\x4f\x1e\xd9\x00\x6f\x1e\xda\x00\x4f\x1e\xdb\x00\x6f\x1e\xdc\x00\x4f\x1e\xdd\x00\x6f\x1e\xde\x00\x4f\x1e\xdf\x00\x6f\x1e\xe0\x00\x4f\x1e\xe1\x00\x6f\x1e\xe2\x00\x4f\x1e\xe3\x00\x6f\x01\x54\x00\x52\x01\x55\x00\x72\x01\x56\x00\x52\x01\x57\x00\x72\x01\x58\x00\x52\x01\x59\x00\x72\x01\x5a\x00\x53\x01\x5b\x00\x73\x01\x5c\x00\x53\x01\x5d\x00\x73\x01\x5e\x00\x53\x01\x5f\x00\x73\x01\x60\x00\x53\x01\x61\x00\x73\x01\x62\x00\x54\x01\x63\x00\x74\x01\x64\x00\x54\x01\x65"\
"\x00\x74\x01\x66\x00\x54\x01\x67\x00\x74\x00\xd9\x00\x55\x00\xda\x00\x55\x00\xfa\x00\x75\x00\xdb\x00\x55\x00\xfb\x00\x75\x01\x68\x00\x55\x01\x69\x00\x75\x01\x6a\x00\x55\x01\x6b\x00\x75\x01\x6c\x00\x55\x01\x6d\x00\x75\x01\x6e\x00\x55\x01\x6f\x00\x75\x01\x70\x00\x55\x01\x71\x00\x75\x01\x72\x00\x55\x01\x73\x00\x75\x01\xaf\x00\x55\x01\xb0\x00\x75\x01\xd3\x00\x55\x01\xd4\x00\x75\x01\xd5\x00\x55\x01\xd6\x00\x75\x01\xd7\x00\x55\x01\xd8\x00\x75\x01\xd9\x00\x55\x01\xda\x00\x75\x01\xdb\x00\x55\x01\xdc\x00\x75\x1e\xe4\x00\x55\x1e\xe5\x00\x75\x1e\xe6\x00\x55\x1e\xe7\x00\x75\x1e\xe8\x00\x55\x1e\xe9\x00\x75\x1e\xea\x00\x55\x1e\xeb\x00\x75\x1e\xec\x00\x55\x1e\xed\x00\x75\x1e\xee\x00\x55\x1e\xef\x00\x75\x1e\xf0\x00\x55\x1e\xf1\x00\x75\x01\x74\x00\x57\x01\x75\x00\x77\x1e\x80\x00\x57\x1e\x81\x00\x77\x1e\x82"\
"\x00\x57\x1e\x83\x00\x77\x1e\x84\x00\x57\x1e\x85\x00\x77\x00\xdd\x00\x59\x00\xfd\x00\x79\x00\xff\x00\x79\x01\x76\x00\x59\x01\x77\x00\x79\x01\x78\x00\x59\x1e\xf2\x00\x59\x1e\xf3\x00\x75\x1e\xf4\x00\x59\x1e\xf5\x00\x79\x1e\xf6\x00\x59\x1e\xf7\x00\x79\x1e\xf8\x00\x59\x1e\xf9\x00\x79\x01\x79\x00\x5a\x01\x7a\x00\x7a\x01\x7b\x00\x5a\x01\x7c\x00\x7a\x01\x7d\x00\x5a\x01\x7e\x00\x7a\x01\xfc\x00\xc6\x01\xfd\x00\xe6\x01\xfe\x00\xd8\x01\xff\x00\xf8\x00\x00";

void EncodeDefault(unsigned char *dest, const unsigned char *src, int *len, bool UseExtensions, unsigned char *ExtraAlphabet)
{
	int 	i,current=0,j,z;
	char 	ret;
	bool	FoundSpecial,FoundNormal;

#ifdef DEBUG
	if (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE) DumpMessage(di.df, di.dl, src, (*len)*2);
#endif

	for (i = 0; i < *len; i++) {
		FoundSpecial = false;
		j = 0;
		while (GSM_DefaultAlphabetCharsExtension[j][0]!=0x00 && UseExtensions) {
			if (src[i*2] 	== GSM_DefaultAlphabetCharsExtension[j][2] &&
			    src[i*2+1] 	== GSM_DefaultAlphabetCharsExtension[j][3]) {
				dest[current++] = GSM_DefaultAlphabetCharsExtension[j][0];
				dest[current++] = GSM_DefaultAlphabetCharsExtension[j][1];
				FoundSpecial 	= true;
				break;
			}
			j++;
		}
		if (!FoundSpecial) {
			ret 		= '?';
			FoundNormal 	= false;
			j 		= 0;
			while (GSM_DefaultAlphabetUnicode[j][1]!=0x00) {
				if (src[i*2]	== GSM_DefaultAlphabetUnicode[j][0] &&
				    src[i*2+1]	== GSM_DefaultAlphabetUnicode[j][1]) {
					ret 		= j;
					FoundNormal 	= true;
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
						FoundSpecial	= true;
						break;
					}
					j=j+3;
				}
			}
			if (!FoundNormal && !FoundSpecial) {
				j = 0;
				FoundNormal = false;
				while (ConvertTable[j*4]   != 0x00 ||
				       ConvertTable[j*4+1] != 0x00) {
					if (src[i*2]   == ConvertTable[j*4] &&
					    src[i*2+1] == ConvertTable[j*4+1]) {
						z = 0;
						while (GSM_DefaultAlphabetUnicode[z][1]!=0x00) {
							if (ConvertTable[j*4+2]	== GSM_DefaultAlphabetUnicode[z][0] &&
							    ConvertTable[j*4+3]	== GSM_DefaultAlphabetUnicode[z][1]) {
								ret 		= z;
								FoundNormal 	= true;
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
	if (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE) DumpMessage(di.df, di.dl, dest, current);
#endif

	*len = current;
}

/* You don't have to use ConvertTable here - 1 char is replaced there by 1 char */
void FindDefaultAlphabetLen(const unsigned char *src, int *srclen, int *smslen, int maxlen)
{
	int 	current=0,j,i;
	bool	FoundSpecial;

	i = 0;
	while (src[i*2] != 0x00 || src[i*2+1] != 0x00) {
		FoundSpecial = false;
		j = 0;
		while (GSM_DefaultAlphabetCharsExtension[j][0]!=0x00) {
			if (src[i*2] 	== GSM_DefaultAlphabetCharsExtension[j][2] &&
			    src[i*2+1] 	== GSM_DefaultAlphabetCharsExtension[j][3]) {
				FoundSpecial = true;
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

#ifndef ENABLE_LGPL
#  define ByteMask ((1 << Bits) - 1)
#endif

int GSM_UnpackEightBitsToSeven(int offset, int in_length, int out_length,
                           unsigned char *input, unsigned char *output)
{
#ifndef ENABLE_LGPL
	/* (c) by Pavel Janik and Pawel Kot */

        unsigned char *OUTPUT 	= output; /* Current pointer to the output buffer */
        unsigned char *INPUT  	= input;  /* Current pointer to the input buffer */
        unsigned char Rest 	= 0x00;
        int	      Bits;

        Bits = offset ? offset : 7;

        while ((INPUT - input) < in_length) {

                *OUTPUT = ((*INPUT & ByteMask) << (7 - Bits)) | Rest;
                Rest = *INPUT >> Bits;

                /* If we don't start from 0th bit, we shouldn't go to the
                   next char. Under *OUTPUT we have now 0 and under Rest -
                   _first_ part of the char. */
                if ((INPUT != input) || (Bits == 7)) OUTPUT++;
                INPUT++;

                if ((OUTPUT - output) >= out_length) break;

                /* After reading 7 octets we have read 7 full characters but
                   we have 7 bits as well. This is the next character */
                if (Bits == 1) {
                        *OUTPUT = Rest;
                        OUTPUT++;
                        Bits = 7;
                        Rest = 0x00;
                } else {
                        Bits--;
                }
        }

        return OUTPUT - output;
#else
	return 0;
#endif
}

int GSM_PackSevenBitsToEight(int offset, unsigned char *input, unsigned char *output, int length)
{
#ifndef ENABLE_LGPL
	/* (c) by Pavel Janik and Pawel Kot */

        unsigned char 	*OUTPUT = output; /* Current pointer to the output buffer */
        unsigned char 	*INPUT  = input;  /* Current pointer to the input buffer */
        int		Bits;             /* Number of bits directly copied to
                                           * the output buffer */
        Bits = (7 + offset) % 8;

        /* If we don't begin with 0th bit, we will write only a part of the
           first octet */
        if (offset) {
                *OUTPUT = 0x00;
                OUTPUT++;
        }

        while ((INPUT - input) < length) {
                unsigned char Byte = *INPUT;

                *OUTPUT = Byte >> (7 - Bits);
                /* If we don't write at 0th bit of the octet, we should write
                   a second part of the previous octet */
                if (Bits != 7)
                        *(OUTPUT-1) |= (Byte & ((1 << (7-Bits)) - 1)) << (Bits+1);

                Bits--;

                if (Bits == -1) Bits = 7; else OUTPUT++;

                INPUT++;
        }
        return (OUTPUT - output);
#else
	return 0;
#endif
}

void GSM_UnpackSemiOctetNumber(unsigned char *retval, unsigned char *Number, bool semioctet)
{
	unsigned char	Buffer[50]	= "";
	int		length		= Number[0];

	if (semioctet) {
		/* Convert number of semioctets to number of chars */
		if (length % 2) length++;
		length=length / 2 + 1;
	}

	/*without leading byte with format of number*/  
	length--;

	switch ((Number[1] & 112)) {
	case (NUMBER_ALPHANUMERIC_NUMBERING_PLAN_UNKNOWN & 112):
		if (length > 6) length++;
		dbgprintf("Alphanumeric number, length %i\n",length);
		GSM_UnpackEightBitsToSeven(0, length, length, Number+2, Buffer);
		Buffer[length]=0;
		break;
	case (NUMBER_INTERNATIONAL_NUMBERING_PLAN_ISDN & 112):
		dbgprintf("International number\n");
		Buffer[0]='+';
		DecodeBCD(Buffer+1,Number+2, length);
		break;
	default:
		dbgprintf("Default number %02x\n",Number[1]);
		DecodeBCD (Buffer, Number+2, length);
		break;
	}

	EncodeUnicode(retval,Buffer,strlen(Buffer));
}

/**
 * Packing some phone numbers (SMSC, SMS destination and others)
 *
 * See GSM 03.40 9.1.1:
 * 1 byte  - length of number given in semioctets or bytes (when given in
 *           bytes, includes one byte for byte with number format).
 *           Returned by function (set semioctet to true, if want result
 *           in semioctets).
 * 1 byte  - format of number (see GSM_NumberType in coding.h). Returned
 *           in unsigned char *Output.
 * n bytes - 2n or 2n-1 semioctets with number. Returned in unsigned char
 *           *Output.
 *
 * 1 semioctet = 4 bits = half of byte
 */
int GSM_PackSemiOctetNumber(unsigned char *Number, unsigned char *Output, bool semioctet)
{
	unsigned char	format, buffer[50];
	int		length, i;

	length=UnicodeLength(Number);
	memcpy(buffer,DecodeUnicodeString(Number),length+1);

	/* Checking for format number */
	format = NUMBER_UNKNOWN_NUMBERING_PLAN_ISDN;
	for (i=0;i<length;i++) {
		/* first byte is '+'. Number can be international */
		if (i==0 && buffer[i]=='+') {
			format=NUMBER_INTERNATIONAL_NUMBERING_PLAN_ISDN;  
		} else {
			/*char is not number. It must be alphanumeric*/
			if (!isdigit(buffer[i])) format=NUMBER_ALPHANUMERIC_NUMBERING_PLAN_UNKNOWN;
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
		length--;
		EncodeBCD (Output+1, buffer+1, length, true);
		break;
	default:
		EncodeBCD (Output+1, buffer, length, true);
		break;
	}

	if (semioctet) return length;

	/* Convert number of semioctets to number of chars */
	if (length % 2) length++;
	return length / 2 + 1;
}

void CopyUnicodeString(unsigned char *Dest, unsigned char *Source)
{
	int j = 0;

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
void ReadUnicodeFile(unsigned char *Dest, unsigned char *Source)
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
	Dest[current++]	= 0;
}

int GetBit(unsigned char *Buffer, int BitNum)
{
	return Buffer[BitNum/8] & 1<<(7-(BitNum%8));
}

int SetBit(unsigned char *Buffer, int BitNum)
{
	return Buffer[BitNum/8] |= 1<<(7-(BitNum%8));
}

int ClearBit(unsigned char *Buffer, int BitNum)
{
	return Buffer[BitNum/8] &= 255 - (1 << (7-(BitNum%8)));
}

void BufferAlign(unsigned char *Destination, int *CurrentBit)
{
	int i=0;

	while(((*CurrentBit) + i) % 8 != 0) {
		ClearBit(Destination, (*CurrentBit)+i);
		i++;
	}

	(*CurrentBit) = (*CurrentBit) + i;
}

void BufferAlignNumber(int *CurrentBit)
{
	int i=0;

	while(((*CurrentBit) + i) % 8 != 0) {
		i++;
	}

	(*CurrentBit) = (*CurrentBit) + i;
}

void AddBuffer(unsigned char 	*Destination,
	       int 		*CurrentBit,
	       unsigned char 	*Source,
	       int 		BitsToProcess)
{
	int i=0;

	while (i!=BitsToProcess) {
		if (GetBit(Source, i)) {
			SetBit(Destination, (*CurrentBit)+i);
		} else {
			ClearBit(Destination, (*CurrentBit)+i);
		}
		i++;
	}
	(*CurrentBit) = (*CurrentBit) + BitsToProcess;
}

void AddBufferByte(unsigned char *Destination,
		   int 		 *CurrentBit,
		   unsigned char Source,
		   int 		 BitsToProcess)
{
	unsigned char Byte;

	Byte = Source;

	AddBuffer(Destination, CurrentBit, &Byte, BitsToProcess);
}

void GetBuffer(unsigned char *Source,
	       int 	     *CurrentBit,
	       unsigned char *Destination,
	       int 	     BitsToProcess)
{
	int i=0;

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
		  int 		*CurrentBit,
		  int 		*integer,
		  int 		BitsToProcess)
{
	int l=0,z=128,i=0;

	while (i!=BitsToProcess) {
		if (GetBit(Source, (*CurrentBit)+i)) l=l+z;
		z=z/2;
		i++;
	}
	*integer=l;  
	(*CurrentBit) = (*CurrentBit) + i;
}

void GetBufferI(unsigned char 	*Source,
		int 		*CurrentBit,
		int 		*result,
		int 		BitsToProcess)
{
	int l=0,z,i=0;
	
	z = 1<<(BitsToProcess-1);

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
void EncodeUnicodeSpecialNOKIAChars(unsigned char *dest, const unsigned char *src, int len)
{
	int 	i,current = 0;
	bool 	special=false;

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
			special = false;
		} else {
			if (src[i*2] == 0x00 && src[i*2+1] == '~') {
				special = true;
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
	dest[current++] = 0x00;
}

void DecodeUnicodeSpecialNOKIAChars(unsigned char *dest, const unsigned char *src, int len)
{
	int i=0,current=0;

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
	dest[current++] = 0x00;
}

bool mystrncasecmp(unsigned const char *a, unsigned const char *b, int num)
{
	int i;

	if (a == NULL || b == NULL) return false;

	if (num == 0) num = -1;
   
	for (i = 0; i != num; i++) {
		if (a[i] == 0x00 && b[i] == 0x00) return true;
		if (a[i] == 0x00 || b[i] == 0x00) return false;
		if (tolower(a[i]) != tolower(b[i])) return false;
 	}
	return true;
}

/* Compares two Unicode strings without regarding to case.
 * Return true, when they're equal
 */
bool mywstrncasecmp(unsigned const  char *a, unsigned const  char *b, int num)
{
 	int 		i;
  	wchar_t 	wc,wc2;

        if (a == NULL || b == NULL) return false;

	if (num == 0) num = -1;
	
	for (i = 0; i != num; i++) {
		if ((a[i*2] == 0x00 && a[i*2+1] == 0x00) && (b[i*2] == 0x00 && b[i*2+1] == 0x00)) return true;
		if ((a[i*2] == 0x00 && a[i*2+1] == 0x00) || (b[i*2] == 0x00 && b[i*2+1] == 0x00)) return false;
 		wc  = a[i*2+1] | (a[i*2] << 8);
 		wc2 = b[i*2+1] | (b[i*2] << 8);
 		if (mytowlower(wc) != mytowlower(wc2)) return false;
 	}
	return true;
}

/* wcscmp in Mandrake 9.0 is wrong */
bool mywstrncmp(unsigned const  char *a, unsigned const  char *b, int num)
{
	int i=0;
  
	while (1) {
		if (a[i*2] != b[i*2] || a[i*2+1] != b[i*2+1]) return false;
		if (a[i*2] == 0x00 && a[i*2+1] == 0x00) return true;
		i++;
		if (num == i) return true;
	}
}

/* FreeBSD boxes 4.7-STABLE does't have it, although it's ANSI standard */
bool myiswspace(unsigned const char *src)
{
#ifndef HAVE_ISWSPACE
 	int 		o;
	unsigned char	dest[10];
#endif
 	wchar_t 	wc;

	wc = src[1] | (src[0] << 8);

#ifndef HAVE_ISWSPACE
	o = DecodeWithUnicodeAlphabet(wc, dest);
	if (o == 1) {
		if (isspace(((int)dest[0]))!=0) return true;
		return false;
	}
	return false;
#else
	return iswspace(wc);
#endif
}

/* FreeBSD boxes 4.7-STABLE does't have it, although it's ANSI standard */
int mytowlower(wchar_t c)
{
#ifndef HAVE_TOWLOWER
	unsigned char dest[10];

	DecodeWithUnicodeAlphabet(c, dest);
	return tolower(dest[0]);
#else
	return towlower(c);
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
/* One crazy define to convert unicode used in Gammu to standard wchar_t */
#define tolowerwchar(x) (mytowlower((wchar_t)( (((&(x))[0] & 0xff) << 8) | (((&(x))[1] & 0xff)) )))
	 register wchar_t b, c;

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
			register wchar_t a;
			register const unsigned char *rhaystack, *rneedle;

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

void MyGetLine(unsigned char *Buffer, int *Pos, unsigned char *OutBuffer, int MaxLen)
{
	OutBuffer[0] = 0;
	if (Buffer == NULL) return;
	while (1) {
		if ((*Pos) >= MaxLen) return;
		switch (Buffer[*Pos]) {
		case 0x00:
			return;
		case 0x0A:
			if (strlen(OutBuffer) != 0) return;
			break;
		case 0x0D:
			if (strlen(OutBuffer) != 0) return;
			break;
		default  :
			OutBuffer[strlen(OutBuffer) + 1] = 0;
			OutBuffer[strlen(OutBuffer)]     = Buffer[*Pos];
		}
		(*Pos)++;
	}
}

void StringToDouble(char *text, double *d)
{
	bool 		before=true;
	double		multiply = 1;
	unsigned int 	i;

	*d = 0;
	for (i=0;i<strlen(text);i++) {
		if (isdigit(text[i])) {
			if (before) {
				(*d)=(*d)*10+(text[i]-'0');
			} else {
				multiply=multiply*0.1;
				(*d)=(*d)+(text[i]-'0')*multiply;
			}
		}
		if (text[i]=='.' || text[i]==',') before=false;
	}
}

/* When char can be converted, convert it from Unicode to UTF8 */
bool EncodeWithUTF8Alphabet(unsigned char mychar1, unsigned char mychar2, unsigned char *ret1, unsigned char *ret2)
{
	unsigned char	mychar3,mychar4;
	int		j=0;
      
	if (mychar1>0x00 || mychar2>128) {
		mychar3=0x00;
		mychar4=128;
		while (true) {
			if (mychar3==mychar1) {
				if (mychar4+64>=mychar2) {
					*ret1=j+0xc2;
					*ret2=0x80+(mychar2-mychar4);
					return true;
				}
			}
			if (mychar4==192) {
				mychar3++;
				mychar4=0;
			} else {
				mychar4=mychar4+64;
			}
			j++;
		}
	}
	return false;
}

/* Make UTF8 string from Unicode input string */
bool EncodeUTF8QuotedPrintable(unsigned char *dest, const unsigned char *src)
{
	int		i,j=0;
	unsigned char	mychar1, mychar2;
	bool		retval = false;
	
	for (i = 0; i < (int)(UnicodeLength(src)); i++) {
		if (EncodeWithUTF8Alphabet(src[i*2],src[i*2+1],&mychar1,&mychar2)) {
			sprintf(dest+j, "=%02X=%02X",mychar1,mychar2);
			j	= j+6;
			retval  = true;
		} else {
			j += DecodeWithUnicodeAlphabet(((wchar_t)(src[i*2]*256+src[i*2+1])), dest + j);
	    	}
	}
	dest[j++]=0;
	return retval;
}

bool EncodeUTF8(unsigned char *dest, const unsigned char *src)
{
	int		i,j=0;
	unsigned char	mychar1, mychar2;
	bool		retval = false;
	
	for (i = 0; i < (int)(UnicodeLength(src)); i++) {
		if (EncodeWithUTF8Alphabet(src[i*2],src[i*2+1],&mychar1,&mychar2)) {
			sprintf(dest+j, "%c%c",mychar1,mychar2);
			j	= j+2;
			retval  = true;
	    	} else {
			j += DecodeWithUnicodeAlphabet(((wchar_t)(src[i*2]*256+src[i*2+1])), dest + j);
	    	}
	}
	dest[j++]=0;
	return retval;
}

/* Decode UTF8 char to Unicode char */
wchar_t DecodeWithUTF8Alphabet(unsigned char mychar3, unsigned char mychar4)
{
	unsigned char	mychar1, mychar2;
	int		j;
	
	mychar1=0x00;
	mychar2=128;
	for(j=0;j<mychar3-0xc2;j++) {
		if (mychar2==192) {
			mychar1++;
			mychar2 = 0;
		} else {
			mychar2 = mychar2+64;
		}
	}
	mychar2 = mychar2+(mychar4-0x80);
	return mychar2 | (mychar1 << 8);
}

/* Make Unicode string from UTF8 string */
void DecodeUTF8QuotedPrintable(unsigned char *dest, const unsigned char *src, int len)
{
	int 		i=0,j=0;
	unsigned char	mychar1, mychar2;
	wchar_t		ret;
	
	while (i<=len) {
		if (len-6>=i) {
			/* Need to have correct chars */
			if (src[i]  =='=' && DecodeWithHexBinAlphabet(src[i+1])!=-1
	                                  && DecodeWithHexBinAlphabet(src[i+2])!=-1 &&
			    src[i+3]=='=' && DecodeWithHexBinAlphabet(src[i+4])!=-1 &&
	                                     DecodeWithHexBinAlphabet(src[i+5])!=-1) {
				mychar1	= 16*DecodeWithHexBinAlphabet(src[i+1])+DecodeWithHexBinAlphabet(src[i+2]);
				mychar2	= 16*DecodeWithHexBinAlphabet(src[i+4])+DecodeWithHexBinAlphabet(src[i+5]);
				ret 	= DecodeWithUTF8Alphabet(mychar1,mychar2);
				i 	= i+6;
			} else {
				i+=EncodeWithUnicodeAlphabet(&src[i], &ret);
			}   
		} else {
			i+=EncodeWithUnicodeAlphabet(&src[i], &ret);
		}
		dest[j++] = (ret >> 8) & 0xff;
		dest[j++] = ret & 0xff;
	}
	dest[j++] = 0;
	dest[j++] = 0;
}

void DecodeUTF8(unsigned char *dest, const unsigned char *src, int len)
{
	int 		i=0,j=0;
	wchar_t		ret;
	
	while (i<=len) {
		if (len-2>=i) {
			if (src[i] >= 0xC2) {
				ret 	= DecodeWithUTF8Alphabet(src[i],src[i+1]);
				i 	= i+2;
			} else {
				i+=EncodeWithUnicodeAlphabet(&src[i], &ret);
			}
		} else {
			i+=EncodeWithUnicodeAlphabet(&src[i], &ret);
		}
		dest[j++] = (ret >> 8) & 0xff;
		dest[j++] = ret & 0xff;
	}
	dest[j++] = 0;
	dest[j++] = 0;
}

void DecodeUTF7(unsigned char *dest, const unsigned char *src, int len)
{
	int 		i=0,j=0,z,p;
	wchar_t		ret;
	
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
				dest[j++] = (ret >> 8) & 0xff;
				dest[j++] = ret & 0xff;
			}
		} else {
			i+=EncodeWithUnicodeAlphabet(&src[i], &ret);
			dest[j++] = (ret >> 8) & 0xff;
			dest[j++] = ret & 0xff;
		}
	}
	dest[j++] = 0;
	dest[j++] = 0;
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

static void EncodeBASE64Block(unsigned char in[3], unsigned char out[4], int len)
{
	/* BASE64 translation Table as described in RFC1113 */
	unsigned char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	out[0] = cb64[ in[0] >> 2 ];
	out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
	out[2] = (unsigned char) (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
	out[3] = (unsigned char) (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}

void EncodeBASE64(const unsigned char *Input, unsigned char *Output, int Length)
{
	unsigned char 	in[3], out[4];
	int 		i, pos = 0, len, outpos = 0;

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

	Output[outpos++] = 0;
}

static void DecodeBASE64Block(unsigned char in[4], unsigned char out[3])
{   
	out[0] = (unsigned char) (in[0] << 2 | in[1] >> 4);
	out[1] = (unsigned char) (in[1] << 4 | in[2] >> 2);
	out[2] = (unsigned char) (((in[2] << 6) & 0xc0) | in[3]);
}

int DecodeBASE64(const unsigned char *Input, unsigned char *Output, int Length)
{
	unsigned char 	cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";
	unsigned char 	in[4], out[3], v;
	int 		i, len, pos = 0, outpos = 0;

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

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */


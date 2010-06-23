/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

*/

#include <stdlib.h>
#include <string.h>

#include "gsm-common.h"
#include "gsm-coding.h"

#ifdef WIN32
  #include <windows.h>
#else
  #include "devices/device.h"
#endif

/* Coding functions */
#define NUMBER_OF_7_BIT_ALPHABET_ELEMENTS 128

#ifndef USE_NLS        
  static unsigned char GSM_DefaultAlphabet[NUMBER_OF_7_BIT_ALPHABET_ELEMENTS] = {

	/* ETSI GSM 03.38, version 6.0.1, section 6.2.1; Default alphabet */
	/* Generally table shows chars in Latin 1, but not only - Greek
	   chars are visible correctly in ... */
	
	'@',  0xa3, '$',  0xa5, 0xe8, 0xe9, 0xf9, 0xec,   // 0x08
	0xf2, 0xc7, '\n', 0xd8, 0xf8, '\r', 0xc5, 0xe5,  

/* from v13@priest.com codes for Greek chars. Not confirmed and commented */
//	0xc4, '_' , 0xd6, 0xc3, 0xcb, 0xd9, 0xd0, 0xd8,
//	0xd3, 0xc8, 0xce, 0xcb, 0xc6, 0xe6, 0xdf, 0xc9,   // 0x20

	'?',  '_',  '?',  '?',  '?',  '?',  '?',  '?',
	'?',  '?',  '?',  '?',  0xc6, 0xe6, 0xdf, 0xc9,   // 0x20
	' ',  '!',  '\"', '#',  0xa4,  '%',  '&', '\'',
	'(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',    // 0x30
	'0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',
	'8',  '9',  ':',  ';',  '<',  '=',  '>',  '?',    // 0x40
	0xa1, 'A',  'B',  'C',  'D',  'E',  'F',  'G',
	'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
	'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',
	'X',  'Y',  'Z',  0xc4, 0xd6, 0xd1, 0xdc, 0xa7,
	0xbf, 'a',  'b',  'c',  'd',  'e',  'f',  'g',
	'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
	'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
	'x',  'y',  'z',  0xe4, 0xf6, 0xf1, 0xfc, 0xe0
  };

  #ifndef WIN32
    /*Simple UNICODE decoding and encoding from/to iso-8859-2
    First version prepared by Martin Kacer <M.Kacer@sh.cvut.cz>

    Following table contains triplets:
    first unicode byte, second unicode byte, iso-8859-2 character*/
    unsigned char unicode_table[][3] =
    {
	/* C< D< E< N< R< S< T< Uo Z< */
	{0x01, 0x0C, 0xC8}, {0x01, 0x0E, 0xCF}, {0x01, 0x1A, 0xCC},
	{0x01, 0x47, 0xD2}, {0x01, 0x58, 0xD8}, {0x01, 0x60, 0xA9},
	{0x01, 0x64, 0xAB}, {0x01, 0x6E, 0xD9}, {0x01, 0x7D, 0xAE},
	/* c< d< e< n< r< s< t< uo z< */
	{0x01, 0x0D, 0xE8}, {0x01, 0x0F, 0xEF}, {0x01, 0x1B, 0xEC},
	{0x01, 0x48, 0xF2}, {0x01, 0x59, 0xF8}, {0x01, 0x61, 0xB9},
	{0x01, 0x65, 0xBB}, {0x01, 0x6F, 0xF9}, {0x01, 0x7E, 0xBE},
	/* A< A, C' D/ E, L< L' L/ */
	{0x01, 0x02, 0xC3}, {0x01, 0x04, 0xA1}, {0x01, 0x06, 0xC6},
	{0x01, 0x10, 0xD0}, {0x01, 0x18, 0xCA}, {0x01, 0x3D, 0xA5},
	{0x01, 0x39, 0xC5}, {0x01, 0x41, 0xA3},
	/* N' O" R' S' S, T, U" Z' Z. */
	{0x01, 0x43, 0xD1}, {0x01, 0x50, 0xD5}, {0x01, 0x54, 0xC0},
	{0x01, 0x5A, 0xA6}, {0x01, 0x5E, 0xAA}, {0x01, 0x62, 0xDE},
	{0x01, 0x70, 0xDB}, {0x01, 0x79, 0xAC}, {0x01, 0x7B, 0xAF},
	/* a< a, c' d/ e, l< l' l/ */
	{0x01, 0x03, 0xE3}, {0x01, 0x05, 0xB1}, {0x01, 0x07, 0xE6},
	{0x01, 0x11, 0xF0}, {0x01, 0x19, 0xEA}, {0x01, 0x3E, 0xB5},
	{0x01, 0x3A, 0xE5}, {0x01, 0x42, 0xB3},
	/* n' o" r' s' s, t, u" z' z. */
	{0x01, 0x44, 0xF1}, {0x01, 0x51, 0xF5}, {0x01, 0x55, 0xE0},
	{0x01, 0x5B, 0xB6}, {0x01, 0x5F, 0xBA}, {0x01, 0x63, 0xFE},
	{0x01, 0x71, 0xFB}, {0x01, 0x7A, 0xBC}, {0x01, 0x7C, 0xBF},

	{0x00, 0x00, 0x00}
    };
  #else
    unsigned char unicode_table[][3] =
    {

	/* o' */
        {0x00, 0xF3, 0xA2},

        /* O' */
        {0x00, 0xD3, 0xE0},

	/* A, C' E, L/ */
	                    {0x01, 0x04, 0xA4}, {0x01, 0x06, 0x8F},
	                    {0x01, 0x18, 0xA8},
	                    {0x01, 0x41, 0x9D},
	/* N' S' Z' Z. */
	{0x01, 0x43, 0xE3},
	{0x01, 0x5A, 0x97},
	                    {0x01, 0x79, 0x8D}, {0x01, 0x7B, 0xBD},
	/* a, c' e, l/ */
	                    {0x01, 0x05, 0xA5}, {0x01, 0x07, 0x86},
	                    {0x01, 0x19, 0xA9}, 
	                    {0x01, 0x42, 0x88},
	/* n' s' z' z. */
	{0x01, 0x44, 0xE4},
	{0x01, 0x5B, 0x98},
                            {0x01, 0x7A, 0xAB}, {0x01, 0x7C, 0xBE},

	{0x00, 0x00, 0x00}
    };
  #endif

unsigned char EncodeWithDefaultAlphabet(unsigned char value)
{
	unsigned char i;

	if (value == '?') return  0x3f;

	for (i = 0; i < NUMBER_OF_7_BIT_ALPHABET_ELEMENTS; i++)
		if (GSM_DefaultAlphabet[i] == value)
			return i;
	
	return '?';
}

unsigned char DecodeWithDefaultAlphabet(unsigned char value)
{
	return GSM_DefaultAlphabet[value];
}

wchar_t EncodeWithUnicodeAlphabet(unsigned char value)
{
	wchar_t retval;

	int j;

	/*If character is not found, first unicode byte is set to zero
	  and second one is the same as iso-8859-2 character*/
	retval = value | (0x00 << 8);

	for ( j = 0;  unicode_table[j][2] != 0x00;  ++j )
		if ( value == unicode_table[j][2] )
		{
			retval = unicode_table[j][1] | (unicode_table[j][0] << 8);
			break;
		}

	return retval;
}

unsigned char DecodeWithUnicodeAlphabet(wchar_t value)
{
	unsigned char retval;

	int j;

	retval=value & 0xff; /* default is to cut off the first byte */

	for ( j = 0;  unicode_table[j][2] != 0x00;  ++j )
		if (((value >> 8) & 0xff) == unicode_table[j][0] &&
		    (value & 0xff) == unicode_table[j][1] ) {
			retval = unicode_table[j][2];
			break;
		}

	return retval;
}

#else

  /* ETSI GSM 03.38, version 6.0.1, section 6.2.1; Default alphabet */
  unsigned char GSM_DefaultAlphabetUnicode[NUMBER_OF_7_BIT_ALPHABET_ELEMENTS+1][2] =
  {
	{0x00,0x40},{0x00,0xa3},{0x00,0x24},{0x00,0xA5},
	{0x00,0xE8},{0x00,0xE9},{0x00,0xF9},{0x00,0xEC},//0x08
	{0x00,0xF2},{0x00,0xC7},{0x00,'\n'},{0x00,0xD8},
	{0x00,0xD9},{0x00,'\r'},{0x00,0xC5},{0x00,0xE5},
	{0x03,0x94},{0x00,0xb9}/*not exactly, but*/,{0x03,0xA6},{0x03,0x93},
	{0x03,0x9B},{0x03,0xA9},{0x03,0xA0},{0x03,0xA8},
	{0x03,0xA3},{0x03,0x98},{0x03,0x9E},{0x00,0xb9},/*not exactly, but*/
	{0x00,0xC6},{0x00,0xE6},{0x00,0xDF},{0x00,0xC9},//0x20
	{0x00,' ' },{0x00,'!' },{0x00,'\"'},{0x00,'#' },
	{0x00,0xA4},{0x00,'%' },{0x00,'&' },{0x00,'\''},
	{0x00,'(' },{0x00,')' },{0x00,'*' },{0x00,'+' },
	{0x00,',' },{0x00,'-' },{0x00,'.' },{0x00,'/' }, //0x30
	{0x00,'0' },{0x00,'1' },{0x00,'2' },{0x00,'3' },
	{0x00,'4' },{0x00,'5' },{0x00,'6' },{0x00,'7' },
	{0x00,'8' },{0x00,'9' },{0x00,':' },{0x00,';' },
	{0x00,'<' },{0x00,'=' },{0x00,'>' },{0x00,'?' }, //0x40
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

unsigned char EncodeWithDefaultAlphabet(unsigned char value)
{
	unsigned char i;

	wchar_t value2;

	if (value == '?') return  0x3f;

	for (i = 0; i < NUMBER_OF_7_BIT_ALPHABET_ELEMENTS; i++) {

		value2 = GSM_DefaultAlphabetUnicode[i][1] | ( GSM_DefaultAlphabetUnicode[i][0] << 8);

		if (EncodeWithUnicodeAlphabet(value) == value2)
			return i;
	}

	return '?';
}

unsigned char DecodeWithDefaultAlphabet(unsigned char value)
{
	wchar_t value2;

	value2 = GSM_DefaultAlphabetUnicode[value][1] | ( GSM_DefaultAlphabetUnicode[value][0] << 8);

	return DecodeWithUnicodeAlphabet(value2);
}

wchar_t EncodeWithUnicodeAlphabet(unsigned char value)
{
	wchar_t retval;

	if (mbtowc(&retval, &value, 1) == -1) return '?';
	else return retval;
}

unsigned char DecodeWithUnicodeAlphabet(wchar_t value)
{
	unsigned char retval;

	if (wctomb(&retval, value) == -1) return '?';
	else return retval;
}

#endif

void DecodeDefault (unsigned char* dest, const unsigned char* src, int len)
{
	int i;

	for (i = 0; i < len; i++)
		dest[i] = DecodeWithDefaultAlphabet(src[i]);
	dest[len]=0;
}

void EncodeDefault (unsigned char* dest, const unsigned char* src, int len)
{
	int i;

	for (i = 0; i < len; i++)
		dest[i] = EncodeWithDefaultAlphabet(src[i]);
	return;
}

void DecodeUnicode (unsigned char* dest, const unsigned char* src, int len)
{
	int i;
	wchar_t wc;

	for (i = 0; i < len; i++) {
	  wc = src[(2*i)+1] | (src[2*i] << 8);
	  dest[i] = DecodeWithUnicodeAlphabet(wc);
	}
	dest[len]=0;
	return;
}

void EncodeUnicode (unsigned char* dest, const unsigned char* src, int len)
{
	int i;
	wchar_t wc;

	for (i = 0; i < len; i++) {
		wc = EncodeWithUnicodeAlphabet(src[i]);
		dest[i*2] = (wc >> 8) &0xff;
		dest[(i*2)+1] = wc & 0xff;
	}
}

bool EncodeWithUTF8Alphabet(u8 mychar, u8 *ret1, u8 *ret2)
{
      u8 mychar1,mychar2,mychar3,mychar4;
      int j=0;
      
      mychar1=((EncodeWithUnicodeAlphabet(mychar)>>8)&0xff);
      mychar2=EncodeWithUnicodeAlphabet(mychar)&0xff;
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

void DecodeWithUTF8Alphabet(u8 mychar3, u8 mychar4, u8 *ret)
{
    u8 mychar1, mychar2;
    int j;
    wchar_t wc;    
	
    mychar1=0x00;
    mychar2=128;
    for(j=0;j<mychar3-0xc2;j++) {
	if (mychar2==192) {
	    mychar1++;
	    mychar2=0;
	} else {
	    mychar2=mychar2+64;
	}
    }
    mychar2=mychar2+(mychar4-0x80);
    wc = mychar2 | (mychar1 << 8);
    *ret=DecodeWithUnicodeAlphabet(wc);
    j=-1;
}

void EncodeUTF8 (unsigned char* dest, const unsigned char* src, int len)
{
	int i,j=0,z;
	u8 mychar1, mychar2;
	u8 buf[7];
	
	for (i = 0; i < len; i++) {	    
	    if (EncodeWithUTF8Alphabet(src[i],&mychar1,&mychar2)) {
		sprintf(buf, "=%02X=%02X",mychar1,mychar2);
		for (z=0;z<6;z++) dest[j++]=buf[z];
	    } else {
		dest[j++]=src[i];
	    }
	}
	dest[j++]=0;
}

void DecodeUTF8 (unsigned char* dest, const unsigned char* src, int len)
{
	int i=0,j=0;
	u8 mychar1, mychar2,ret;
	
	while (i<=len) {
	    if (len-6>=i) {
		/* Need to have correct chars */
		if (src[i]  =='=' && DecodeWithHexBinAlphabet(src[i+1])!=-1
                                  && DecodeWithHexBinAlphabet(src[i+2])!=-1 &&
		    src[i+3]=='=' && DecodeWithHexBinAlphabet(src[i+4])!=-1 &&
                                     DecodeWithHexBinAlphabet(src[i+5])!=-1) {
		    mychar1=16*DecodeWithHexBinAlphabet(src[i+1])+DecodeWithHexBinAlphabet(src[i+2]);
		    mychar2=16*DecodeWithHexBinAlphabet(src[i+4])+DecodeWithHexBinAlphabet(src[i+5]);
		    DecodeWithUTF8Alphabet(mychar1,mychar2,&ret);
		    i=i+5;
		    dest[j++]=ret;
		} else {
		    dest[j++]=src[i];
		}   
	    } else {
		dest[j++]=src[i];
	    }
	    i++;
	}
	dest[j++]=0;
}

int DecodeWithHexBinAlphabet (unsigned char mychar) {
    if (mychar>='A' && mychar<='F') return mychar-'A'+10;
    if (mychar>='a' && mychar<='f') return mychar-'a'+10;
    if (mychar>='0' && mychar<='9') return mychar-'0';
    return -1;
}

unsigned char EncodeWithHexBinAlphabet (int digit) {
  if (digit >= 0 && digit <= 9) return '0'+(digit);
  if (digit >=10 && digit <=15) return 'A'+(digit-10);
  return 0;
}

void DecodeHexBin (unsigned char* dest, const unsigned char* src, int len)
{
	int i,current=0;

	for (i = 0; i < len/2 ; i++) {
          dest[current++]=DecodeWithHexBinAlphabet(src[i*2])*16+
                          DecodeWithHexBinAlphabet(src[i*2+1]);
	}
	dest[current++]=0;
}

void EncodeHexBin (unsigned char* dest, const unsigned char* src, int len)
{
	int i,current=0;

	for (i = 0; i < len; i++) {
           dest[current++]=EncodeWithHexBinAlphabet(src[i] >> 0x04);
           dest[current++]=EncodeWithHexBinAlphabet(src[i] & 0x0f);
	}
}

void DecodeBCD (unsigned char* dest, const unsigned char* src, int len)
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

void EncodeBCD (unsigned char* dest, const unsigned char* src, int len, bool fill)
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

        /* When fill is set: we fill in the most significant bits of the
           last byte with 0x0f (1111 binary) if the number is represented
           with odd number of digits. */
	if (fill && (len & 0x01)) {
	     dest[current]=dest[current] | 0xf0;
        }
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

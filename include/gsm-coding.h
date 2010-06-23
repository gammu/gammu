/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

*/

#ifndef __gsm_coding_h
#define __gsm_coding_h    

extern void EncodeDefault (unsigned char* dest, const unsigned char* src, int len);
extern void DecodeDefault (unsigned char* dest, const unsigned char* src, int len);
extern void EncodeUnicode (unsigned char* dest, const unsigned char* src, int len);
extern void DecodeUnicode (unsigned char* dest, const unsigned char* src, int len);
extern void EncodeUTF8    (unsigned char* dest, const unsigned char* src, int len);
extern void DecodeUTF8    (unsigned char* dest, const unsigned char* src, int len);
extern void DecodeHexBin  (unsigned char* dest, const unsigned char* src, int len);
extern void EncodeHexBin  (unsigned char* dest, const unsigned char* src, int len);
extern void DecodeBCD     (unsigned char* dest, const unsigned char* src, int len);
extern void EncodeBCD     (unsigned char* dest, const unsigned char* src, int len, bool fill);

extern unsigned char EncodeWithDefaultAlphabet(unsigned char);
extern unsigned char DecodeWithDefaultAlphabet(unsigned char);
extern wchar_t       EncodeWithUnicodeAlphabet(unsigned char);
extern unsigned char DecodeWithUnicodeAlphabet(wchar_t);
extern void          DecodeWithUTF8Alphabet   (u8 mychar3, u8 mychar4, u8 *ret);
extern bool          EncodeWithUTF8Alphabet   (u8 mychar, u8 *ret1, u8 *ret2);
extern int           DecodeWithHexBinAlphabet (unsigned char mychar);
extern unsigned char EncodeWithHexBinAlphabet (int digit);
extern unsigned char EncodeWithBCDAlphabet    (int value);
extern int           DecodeWithBCDAlphabet    (unsigned char value);

#endif

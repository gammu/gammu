/**
 * \file gammu-unicode.h
 * \author Michal Čihař
 *
 * Unicode manipulation functions.
 */
#ifndef __gammu_unicode_h
#define __gammu_unicode_h

/**
 * \defgroup Unicode Unicode
 * Unicode manipulation functions. Please note that most of functions
 * here rely on initialised libc char conversions, what is usually done
 * by locales initialisation. Recommended way for doing this is calling
 * \ref GSM_InitLocales.
 */

#include <wchar.h>

#include <gammu-types.h>
#include <gammu-config.h>

/**
 * Returns length of unicode string.
 *
 * \ingroup Unicode
 */
size_t UnicodeLength(const unsigned char *str);

/**
 * Converts string to locale charset.
 *
 * \return Pointer to static string.
 *
 * \ingroup Unicode
 */
char *DecodeUnicodeString(const unsigned char *src);

/**
 * Converts string to console charset.
 *
 * \return Pointer to static string.
 *
 * \ingroup Unicode
 */
char *DecodeUnicodeConsole(const unsigned char *src);

/**
 * Converts string from unicode to local charset.
 *
 * \ingroup Unicode
 */
void DecodeUnicode(const unsigned char *src, char *dest);

/**
 * Encodes string from local charset to unicode.
 *
 * \ingroup Unicode
 */
void EncodeUnicode(unsigned char *dest, const char *src, int len);

/**
 * Decodes unicode file data with byte order mark (BOM).
 *
 * \ingroup Unicode
 */
void ReadUnicodeFile(unsigned char *Dest, const unsigned char *Source);

/**
 * Copies unicode string.
 *
 * \ingroup Unicode
 */
void CopyUnicodeString(unsigned char *Dest, const unsigned char *Source);

/**
 * Encodes string to UTF-8 quoted printable.
 *
 * \ingroup Unicode
 */
gboolean EncodeUTF8QuotedPrintable(char *dest, const unsigned char *src);

/**
 * Decodes UTF-8 quoted printable string.
 *
 * \ingroup Unicode
 */
void DecodeUTF8QuotedPrintable(unsigned char *dest, const char *src,
			       int len);
/**
 * Encodes string to UTF-8.
 *
 * \ingroup Unicode
 */
int EncodeWithUTF8Alphabet(unsigned char mychar1, unsigned char mychar2,
			   char *ret);
/**
 * Decodes string from UTF-8.
 *
 * \ingroup Unicode
 */
int DecodeWithUTF8Alphabet(const unsigned char *src, wchar_t * dest, int len);

/**
 * Decodes string from hex quoted unicode.
 *
 * \ingroup Unicode
 */
void DecodeHexUnicode(unsigned char *dest, const char *src, size_t len);

/**
 * Encodes string to hex quoted unicode.
 *
 * \ingroup Unicode
 */
void EncodeHexUnicode(char *dest, const unsigned char *src, size_t len);

/**
 * Compares two unicode strings.
 *
 * \ingroup Unicode
 */
gboolean mywstrncmp(unsigned const char *a, unsigned const char *b, int num);

/**
 * Locates unicode substring.
 *
 * \ingroup Unicode
 */
unsigned char *mywstrstr(unsigned const char *haystack,
			 unsigned const char *needle);

/**
 * Compares two unicode strings case insensitive.
 *
 * \ingroup Unicode
 */
gboolean mywstrncasecmp(unsigned const char *a, unsigned const char *b, int num);

/**
 * Encode text to UTF-8.
 *
 * \ingroup Unicode
 */
gboolean EncodeUTF8(char *dest, const unsigned char *src);

/**
 * Decode text from UTF-8.
 *
 * \ingroup Unicode
 */
void DecodeUTF8(unsigned char *dest, const char *src, int len);

/**
 * Decode hex encoded binary text.
 *
 * \ingroup Unicode
 */
gboolean DecodeHexBin(unsigned char *dest, const unsigned char *src, int len);

/**
 * Converts single character from unicode to wchar_t.
 */
int EncodeWithUnicodeAlphabet(const unsigned char *value, wchar_t *dest);

/**
 * Converts single character from wchar_t to unicode.
 */
int DecodeWithUnicodeAlphabet(wchar_t value, unsigned char *dest);
#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

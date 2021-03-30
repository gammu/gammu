/**
 * \file gammu-unicode.h
 * \author Michal Čihař
 *
 * Unicode manipulation functions.
 */
#ifndef __gammu_unicode_h
#define __gammu_unicode_h

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * \defgroup Unicode Unicode
 * Unicode manipulation functions. Please note that most of functions
 * here rely on initialised libc char conversions, what is usually done
 * by locales initialisation. Recommended way for doing this is calling
 * \ref GSM_InitLocales.
 */

#include <gammu-config.h>
#include <stdlib.h>
#include <gammu-types.h>

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
void EncodeUnicode(unsigned char *dest, const char *src, size_t len);

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
			       size_t len);
/**
 * Encodes string to UTF-8.
 *
 * \ingroup Unicode
 */
int EncodeWithUTF8Alphabet(unsigned long src, unsigned char *ret);

/**
 * Decodes string from hex quoted unicode.
 *
 * \ingroup Unicode
 */
gboolean DecodeHexUnicode(unsigned char *dest, const char *src, size_t len);

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
gboolean mywstrncmp(const unsigned char *a, const unsigned char *b, int num);

/**
 * Locates unicode substring.
 *
 * \ingroup Unicode
 */
unsigned char *mywstrstr(const unsigned char *haystack,
			 const unsigned char *needle);

/**
 * Compares two unicode strings case insensitive.
 *
 * \ingroup Unicode
 */
gboolean mywstrncasecmp(const unsigned char *a, const unsigned char *b, int num);

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
void DecodeUTF8(unsigned char *dest, const char *src, size_t len);

/**
 * Decode hex encoded binary text.
 *
 * \ingroup Unicode
 */
gboolean DecodeHexBin(unsigned char *dest, const unsigned char *src, size_t len);
#ifdef	__cplusplus
}
#endif
#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

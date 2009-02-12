/**
 * Compatibility layer for string operations on different platforms.
 */

#ifndef __helper__string_h__
#define __helper__string_h__

#define _GNU_SOURCE

#include <gammu-config.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include <string.h>


#ifndef HAVE_STRCASESTR
/**
 * Case insensitive substring location, libc compatibility.
 *
 * \ingroup Unicode
 */
extern char *strcasestr(const char *s, const char *find);
#endif

#ifndef HAVE_STRCASECMP
/**
 * Case insensitive string comparing, libc compatibility.
 *
 * \ingroup Unicode
 */
extern int strcasecmp(const char *s1, const char *s2);
#endif

#ifndef HAVE_STRCHRNUL
extern char *strchrnul(char *s, int find);
#endif
#ifndef HAVE_STRNCASECMP
/** #define TOLOWER(Ch) (isupper (Ch) ? tolower (Ch) : (Ch)) */
extern int strncasecmp (const char *s1, const char *s2, size_t n);
#endif
#ifndef HAVE_STRCASECMP
extern int strcasecmp (const char *s1, const char *s2);
#endif

#ifndef HAVE_TOWLOWER
wchar_t		towlower			(wchar_t c);
#endif
#endif

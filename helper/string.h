/**
 * Compatibility layer for string operations on different platforms.
 */

#ifndef __helper__string_h__
#define __helper__string_h__

#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#include <gammu-config.h>

#include <string.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif


#ifndef HAVE_STRCASESTR
/**
 * Case insensitive substring location, libc compatibility.
 *
 * \ingroup Unicode
 */
extern char *strcasestr(const char *s, const char *find);
#endif

#ifndef HAVE_STRCHRNUL
extern char *strchrnul(char *s, int find);
#endif

#ifndef HAVE_STRNCASECMP
#ifdef HAVE_STRNICMP
#define strncasecmp _strnicmp
#else
# define INTERNAL_STRNCASECMP
extern int strncasecmp (const char *s1, const char *s2, size_t n);
#endif
#endif

#ifndef HAVE_STRCASECMP
#ifdef HAVE_STRICMP
#define strcasecmp _stricmp
#else
# define INTERNAL_STRCASECMP
/**
 * Case insensitive string comparing, libc compatibility.
 *
 * \ingroup Unicode
 */
extern int strcasecmp (const char *s1, const char *s2);
#endif
#endif

#ifndef HAVE_TOWLOWER
wchar_t		towlower			(wchar_t c);
#endif
#endif

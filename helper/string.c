#include "string.h"

#include <ctype.h>

#ifndef HAVE_STRCASESTR
/**
 * Find the first occurrence of find in s, ignore case.
 * Copyright (c) 1990, 1993 The Regents of the University of California.
 */
char *strcasestr(const char *s, const char *find)
{
	char c, sc;
	size_t len;

	if ((c = *find++) != 0) {
		c = tolower((unsigned char)c);
		len = strlen(find);
		do {
			do {
				if ((sc = *s++) == 0)
					return (NULL);
			} while ((char)tolower((unsigned char)sc) != c);
		} while (strncasecmp(s, find, len) != 0);
		s--;
	}
	return (char *)s;
}
#endif

#ifndef HAVE_STRCHRNUL
char *strchrnul(char *s, int find)
{
	char *ret;
	ret = strchr(s, find);
	if (ret == NULL) return s + strlen(s);
	return ret;
}
#endif


#ifndef HAVE_STRNCASECMP
#define TOLOWER(Ch) (isupper (Ch) ? tolower (Ch) : (Ch))
/**
 * Case insensitive string comparator
 * Copyright (C) 1998, 1999, 2005 Free Software Foundation, Inc.
 */
int strncasecmp (const char *s1, const char *s2, size_t n)
{
	register const unsigned char *p1 = (const unsigned char *) s1;
	register const unsigned char *p2 = (const unsigned char *) s2;
	unsigned char c1, c2;

	if (p1 == p2 || n == 0)
		return 0;

	do {
		c1 = TOLOWER (*p1);
		c2 = TOLOWER (*p2);

		if (--n == 0 || c1 == '\0')
			break;

		++p1;
		++p2;
	} while (c1 == c2);

	return (c1 > c2 ? 1 : c1 < c2 ? -1 : 0);
}
#undef TOLOWER
#endif

#ifndef HAVE_STRCASECMP
#define TOLOWER(Ch) (isupper (Ch) ? tolower (Ch) : (Ch))
/**
 * Case insensitive string comparator
 * Copyright (C) 1998, 1999, 2005 Free Software Foundation, Inc.
 */
int strcasecmp (const char *s1, const char *s2)
{
	register const unsigned char *p1 = (const unsigned char *) s1;
	register const unsigned char *p2 = (const unsigned char *) s2;
	unsigned char c1, c2;

	if (p1 == p2)
		return 0;

	do {
		c1 = TOLOWER (*p1);
		c2 = TOLOWER (*p2);

		if (c1 == '\0')
			break;

		++p1;
		++p2;
	} while (c1 == c2);

	return (c1 > c2 ? 1 : c1 < c2 ? -1 : 0);
}
#undef TOLOWER
#endif

#ifndef HAVE_TOWLOWER
/* FreeBSD boxes 4.7-STABLE does't have it, although it's ANSI standard */
wchar_t towlower(wchar_t c)
{
	unsigned char dest[10];

	DecodeWithUnicodeAlphabet(c, dest);
	return tolower(dest[0]);
}
#endif


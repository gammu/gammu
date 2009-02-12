#ifndef __helper__strptime_h__
#define __helper__strptime_h__

#include <gammu-config.h>

#ifdef HAVE_STRPTIME
#define _XOPEN_SOURCE
#define _BSD_SOURCE
#include <time.h>
#else
char *strptime(const char * __restrict, const char * __restrict, struct tm * __restrict);
#endif

#endif

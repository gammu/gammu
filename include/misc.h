/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  Header file for miscellaneous defines, typedefs etc.

*/

#ifndef __misc_h
#define __misc_h    

#include <stdio.h>
#include <stdlib.h>

#ifndef VC6
  #include "config.h"

  /* Use gsprintf instead of sprintf and sprintf */
  #ifdef HAVE_SNPRINTF
  # define gsprintf(a, b, c...) snprintf(a, b, c)
  #else
  # define gsprintf(a, b, c...) sprintf(a, c)
  #endif

#endif

/* Some general defines. */

#ifndef false
  #define false (0)
#endif

#ifndef true
  #define true (!false)
#endif

#ifndef bool    
  #define bool int
#endif

/* This one is for NLS. */

#ifdef USE_NLS

  #ifndef VC6
    #include <libintl.h>
    #define _(x) gettext(x)
  #else
    #define _(x) (x)
  #endif

#else
  #define _(x) (x)
#endif /* USE_NLS */

/* Definitions for u8, u16, u32 and u64, borrowed from
   /usr/src/linux/include/asm-i38/types.h */

#ifndef u8
  typedef unsigned char u8;
#endif

#ifndef u16
  typedef unsigned short u16;
#endif

#ifndef u32
  typedef unsigned int u32;
#endif

#ifndef s32
  typedef int s32;
#endif

#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
  #ifndef u64
    typedef unsigned long long u64;
  #endif

  #ifndef s64
    typedef signed long long s64;
  #endif
#endif 

/* This one is for FreeBSD and similar systems without __ptr_t_ */
/* FIXME: autoconf should take care of this. */

#ifndef __ptr_t
  typedef void * __ptr_t;
#endif /* __ptr_t */


/* Add here any timer operations which are not supported by libc5 */

#ifndef HAVE_TIMEOPS
#include <sys/time.h>

#ifndef timersub
#define timersub(a, b, result)                                                \
  do {                                                                        \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;                             \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;                          \
    if ((result)->tv_usec < 0) {                                              \
      --(result)->tv_sec;                                                     \
      (result)->tv_usec += 1000000;                                           \
    }                                                                         \
  } while (0)
#endif

#endif /* HAVE_TIMEOPS */

#include <stdio.h>

extern int GetLine(FILE *File, char *Line, int count);

int mem_to_int(const char str[], int len);

void hexdump(u16 MessageLength, u8 *MessageBuffer);
void txhexdump(u16 MessageLength, u8 *MessageBuffer);

#endif /* __misc_h */

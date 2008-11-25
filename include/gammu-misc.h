/**
 * \file gammu-misc.h
 * \author Michal Čihař
 *
 * Miscellaneous helper functions.
 */
#ifndef __gammu_misc_h
#define __gammu_misc_h

#ifdef WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif
#include <stdio.h>
#include <gammu-file.h>
#include <gammu-config.h>
#if defined(HAVE_UNISTD_H)
#  include <unistd.h>
#endif

/**
 * Reads single line from file.
 *
 * \param File File descriptor to read from.
 * \param Line Buffer where t ostore result.
 * \param count Maximal length of text which can be stored in buffer.
 *
 * \return Length of read line, -1 on error.
 */
int GetLine(FILE * File, char *Line, int count);

/**
 * Gets Gammu library version.
 */
const char *GetGammuVersion(void);

/**
 * Gets compiler which was used to compile Gammu library.
 */
const char *GetCompiler(void);

/**
 * Gets host OS.
 */
const char *GetOS(void);

/**
 * Returns path to Gammu locales.
 */
const char *GetGammuLocalePath(void);

/**
 * Initializes locales. This sets up things needed for proper string
 * conversion from local charset as well as initializes gettext based
 * translation.
 *
 * \param path Path to gettext translation. If NULL compiled in default
 * is used.
 */
extern void GSM_InitLocales(const char *path);

#undef MAX
#define MAX(a,b) ((a)>(b) ? (a) : (b))
#undef MIN
#define MIN(a,b) ((a)<(b) ? (a) : (b))

#ifdef WIN32
#  if !defined(HAVE_UNISTD_H) || defined(__MINGW32__)
#    define sleep(x) Sleep((x) * 1000)
#    define usleep(x) Sleep(((x) < 1000) ? 1 : ((x) / 1000))
#  endif /* HAVE_UNISTD_H */
#endif

/* Easy check for GCC */
#if defined __GNUC__ && defined __GNUC_MINOR__
# define GSM_GNUC_PREREQ(maj, min) \
        ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
# define GSM_GNUC_PREREQ(maj, min) 0
#endif

/* Various hints for compilers */
#if GSM_GNUC_PREREQ (2,8)
#define PRINTF_STYLE(f, a) __attribute__ ((format(__printf__, f, a)))
#define SCANF_STYLE(f, a) __attribute__ ((format(__scanf__, f, a)))
#else
#define PRINTF_STYLE(f, a)
#define SCANF_STYLE(f, a)
#endif

#if GSM_GNUC_PREREQ (3,4)
#define WARNUNUSED __attribute__ ((__warn_unused_result__))
#else
#define WARNUNUSED
#endif

#if GSM_GNUC_PREREQ (3,1)
#define UNUSED __attribute__ ((unused))
#else
#define UNUSED
#endif

#if __GNUC__
#define NORETURN __attribute__ ((noreturn))
#define INLINE inline
#else
#define NORETURN
#define INLINE
#endif

/* Printf string for size_t */
#ifdef WIN32
#define SIZE_T_FORMAT "%Id"
#define SIZE_T_HEX_FORMAT "%02IX"
#define SIZE_T_HEX4_FORMAT "%04IX"
#else
#define SIZE_T_FORMAT "%zd"
#define SIZE_T_HEX_FORMAT "%02zX"
#define SIZE_T_HEX4_FORMAT "%04zX"
#endif

/* Working snprintf on MSVC */
#ifdef _MSC_VER
#define snprintf _snprintf
#endif

/* Working __FUNCTION__ on BCC */
#ifdef __BORLANDC__
 #ifndef __FUNCTION__
  #define __FUNCTION__ __FUNC__
 #endif
#endif

/* strtoull for BCC (and maybe others) */
#ifndef HAVE_STRTOULL
#define strtoull(A,B,C) strtoul((A),(B),(C))
#endif

/* ssize_t for compilers where it does not exist (BCC) */
#ifndef HAVE_SSIZE_T
typedef long ssize_t;
#endif

/* intptr_t for compilers where it does not exist (BCC) */
#ifndef HAVE_INTPTR_T
typedef int intptr_t;
#endif

/**
 * Encodes text to hexadecimal binary representation.
 */
void EncodeHexBin(unsigned char *dest, const unsigned char *src, int len);

/**
 * Returns true if firmware version is newer.
 *
 * \param latest_version String containing version (eg. latest available).
 * \param current_version String containing version (eg. current one).
 *
 * \return True if latest_version > current_version.
 */
bool GSM_IsNewerVersion(const char *latest_version, const char *current_version);
#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

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
#  endif			/* HAVE_UNISTD_H */
#endif

/* Easy check for GCC */
#if defined __GNUC__ && defined __GNUC_MINOR__
# define GSM_GNUC_PREREQ(maj, min) \
        ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
# define GSM_GNUC_PREREQ(maj, min) 0
#endif

/* Various hints for compilers */
#if GSM_GNUC_PREREQ (2,8) || defined(__clang__)
#define PRINTF_STYLE(f, a) __attribute__ ((format(__printf__, f, a)))
#define SCANF_STYLE(f, a) __attribute__ ((format(__scanf__, f, a)))
#else
#define PRINTF_STYLE(f, a)
#define SCANF_STYLE(f, a)
#endif

#if GSM_GNUC_PREREQ (3,4) || defined(__clang__)
#define WARNUNUSED __attribute__ ((__warn_unused_result__))
#else
#define WARNUNUSED
#endif

#if GSM_GNUC_PREREQ (3,1) || defined(__clang__)
#define UNUSED __attribute__ ((unused))
#else
#define UNUSED
#endif

#if defined(__GNUC__) || defined(__clang__)
#define NORETURN __attribute__((__noreturn__))
#else
#define NORETURN
#endif

/* Clang has bug in handling inline functions */
#if defined(__GNUC__) && !defined(__clang__)
#define INLINE inline
#else
#define INLINE
#endif

/* Working snprintf on MSVC */
#ifdef _MSC_VER
#define snprintf _snprintf
#endif

/* Working __FUNCTION__ on BCC */
#ifndef HAVE_MACRO_FUNCTION
# ifdef HAVE_MACRO_FUNC
#  define __FUNCTION__ __FUNC__
#  define __FUNCTION__WORKING
# else
#  define __FUNCTION__ "unknown"
# endif
#else
# define __FUNCTION__WORKING
#endif

/* strtoull for BCC (and maybe others) */
#ifndef HAVE_STRTOULL
/* MSVC provides same function under different name */
#if _MSC_VER >= 1300
#include <stdlib.h>
#define strtoull _strtoui64
#else
#define strtoull(A,B,C) strtoul((A),(B),(C))
#endif
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
void EncodeHexBin(char *dest, const unsigned char *src, size_t len);

/**
 * Returns TRUE if firmware version is newer.
 *
 * \param latest_version String containing version (eg. latest available).
 * \param current_version String containing version (eg. current one).
 *
 * \return True if latest_version > current_version.
 */
gboolean GSM_IsNewerVersion(const char *latest_version,
			const char *current_version);
#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

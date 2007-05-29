/**
 * \file gammu-misc.h
 * \author Michal Čihař
 *
 * Message data and functions.
 */
#ifndef __gammu_message_h
#define __gammu_message_h

#include <stdio.h>
#include <gammu-file.h>

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
 * Reads file over HTTP.
 *
 * \param server Server to connect.
 * \param filename File to grab on that server.
 * \param file Storage for resulting file.
 *
 * \return True on success.
 */
bool GSM_ReadHTTPFile(unsigned char *server, unsigned char *filename,
		      GSM_File * file);

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
 * Initializes locales.
 */
extern void GSM_InitLocales(const char *path);

#undef MAX
#define MAX(a,b) ((a)>(b) ? (a) : (b))
#undef MIN
#define MIN(a,b) ((a)<(b) ? (a) : (b))

#ifdef WIN32
#  ifdef __BORLANDC__
/* BCC has a proper Sleep(), which takes milliseconds */
#    define my_sleep(x) Sleep(x)
#  else
#    define my_sleep(x) ((x)<1000 ? Sleep(1) : Sleep((x)/1000))
#  endif
#else
#  define my_sleep(x) usleep(x*1000)
#endif

/**
 * Encodes text to hexadecimal binary representation.
 */
void EncodeHexBin(unsigned char *dest, const unsigned char *src, int len);
#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

#include <stdio.h>
#include <gammu-file.h>

int GetLine(FILE *File, char *Line, int count);
bool GSM_ReadHTTPFile(unsigned char *server, unsigned char *filename, GSM_File *file);

const char *GetGammuVersion(void);
const char 				*GetCompiler(void);
const char 				*GetOS(void);

const char *GetGammuLocalePath(void);
/**
 * Initializes locales.
 */
extern void InitLocales(const char *path);

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

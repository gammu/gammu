#include <stdio.h>
#include <gammu-file.h>

int GetLine(FILE *File, char *Line, int count);
bool GSM_ReadHTTPFile(unsigned char *server, unsigned char *filename, GSM_File *file);

const char *GetGammuVersion(void);

const char *GetGammuLocalePath(void);
/**
 * Initializes locales.
 */
extern void InitLocales(const char *path);

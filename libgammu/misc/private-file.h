#ifndef __gammu_private_file_h
#define __gammu_private_file_h

#include <stdio.h>

/* Create new files with mode 0600 on POSIX, retaining the inherited directory
 * ACL on Windows. Existing file permissions are preserved. Supported modes
 * are w, a, optional b and + suffixes, and the MSVC commit suffix c. */
FILE *GSM_OpenPrivateFile(const char *path, const char *mode);

/* Create a new file exclusively and return its descriptor. The binary flag
 * selects untranslated I/O on Windows and has no effect on POSIX. */
int GSM_CreatePrivateFileExclusive(const char *path, int binary);

#endif

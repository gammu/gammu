
#ifndef _CFGREADER_H
#define _CFGREADER_H

#include "misc.h"

/* Structure definitions */

/* A linked list of key/value pairs */

typedef struct _CFG_Entry CFG_Entry;

struct _CFG_Entry {
	CFG_Entry 		*next, *prev;
	unsigned char 		*key;
        unsigned char 		*value;
};

typedef struct _CFG_Header CFG_Header;

struct _CFG_Header {
        CFG_Header 		*next, *prev;
        CFG_Entry 		*entries;
        unsigned char		*section;
};

/* Function prototypes */

CFG_Header 	*CFG_ReadFile(char *filename, bool Unicode);
unsigned char   *CFG_Get(CFG_Header *cfg, unsigned char *section, unsigned char *key, bool Unicode);
CFG_Entry 	*CFG_FindLastSectionEntry(CFG_Header *file_info, unsigned char *section, bool Unicode);

#endif /* _CFGREADER_H */

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

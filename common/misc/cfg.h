/* (c) 2003 by Marcin Wiacek */

#ifndef _cfg_h
#define _cfg_h

#include "misc.h"

/* -------------------------------- structures ----------------------------- */

typedef struct _INI_Entry INI_Entry;

/* 
 * Structure used to save value for single key in INI style file
 */
struct _INI_Entry {
	INI_Entry 		*Next, *Prev;
	unsigned char 		*EntryName;
        unsigned char 		*EntryValue;
};

typedef struct _INI_Section INI_Section;

/* 
 * Structure used to save section in INI style file
 */
struct _INI_Section {
        INI_Section 		*Next, *Prev;
        INI_Entry 		*SubEntries;
        unsigned char		*SectionName;
};

/* ------------------------- function prototypes --------------------------- */

INI_Section 	*INI_ReadFile	 	  (char *FileName, bool Unicode);
INI_Entry 	*INI_FindLastSectionEntry (INI_Section *file_info, unsigned char *section, bool Unicode);
unsigned char   *INI_GetValue		  (INI_Section *cfg, unsigned char *section, unsigned char *key, bool Unicode);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

/** \file cfg.h
 * \defgroup CFG INI file parsing
 *
 * Functions for reading and processing INI file structure.
 *
 * @author Marcin Wiacek
 * @author Michal Čihař
 * @date 2004-2007
 *
 * @{
 */

#ifndef _cfg_h
#define _cfg_h

#include <gammu-error.h>
#include "misc.h"

/* -------------------------------- structures ----------------------------- */

typedef struct _INI_Entry INI_Entry;

/**
 * Structure used to save value for single key in INI style file
 */
struct _INI_Entry {
	INI_Entry 		*Next, *Prev;
	unsigned char 		*EntryName;
        unsigned char 		*EntryValue;
};

typedef struct _INI_Section INI_Section;

/**
 * Structure used to save section in INI style file
 */
struct _INI_Section {
        INI_Section 		*Next, *Prev;
        INI_Entry 		*SubEntries;
        unsigned char		*SectionName;
};

/* ------------------------- function prototypes --------------------------- */

/**
 * Free INI data.
 */
void		INI_Free		  (INI_Section *head);
/**
 * Reads INI data.
 */
GSM_Error INI_ReadFile(char *FileName, bool Unicode, INI_Section **result);
INI_Entry 	*INI_FindLastSectionEntry (INI_Section *file_info, unsigned char *section, bool Unicode);
unsigned char   *INI_GetValue		  (INI_Section *cfg, unsigned char *section, unsigned char *key, bool Unicode);

#endif
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

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
#include <gammu-inifile.h>
#include "misc.h"

/* -------------------------------- structures ----------------------------- */

/**
 * Structure used to save value for single key in INI style file
 */
struct _INI_Entry {
	INI_Entry 		*Next, *Prev;
	unsigned char 		*EntryName;
        unsigned char 		*EntryValue;
};

/**
 * Structure used to save section in INI style file
 */
struct _INI_Section {
        INI_Section 		*Next, *Prev;
        INI_Entry 		*SubEntries;
        unsigned char		*SectionName;
};

/* ------------------------- function prototypes --------------------------- */


#endif
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

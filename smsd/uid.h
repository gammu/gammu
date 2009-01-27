/**
 * SMSD [GU]ID changing handling
 */
#ifndef __smsd_uid_h__
#define __smsd_uid_h__

#include <gammu-types.h>
#include "common.h"

/**
 * Sets UID and GID.
 */
extern bool set_uid_gid(SMSD_Parameters *params);

/**
 * Gets UID from string.
 */
extern bool fill_uid(SMSD_Parameters *params, const char *name);

/**
 * Gets GID from string.
 */
extern bool fill_gid(SMSD_Parameters *params, const char *name);


#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */


/**
 * SMSD [GU]ID changing handling
 */
#ifndef __smsd_uid_h__
#define __smsd_uid_h__

#include <gammu-types.h>

/**
 * Sets UID.
 */
extern bool set_uid(int uid);

/**
 * Sets GID.
 */
extern bool set_gid(int gid);

/**
 * Gets UID from string.
 */
extern int get_uid(const char *name);

/**
 * Gets GID from string.
 */
extern int get_gid(const char *name);


#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */


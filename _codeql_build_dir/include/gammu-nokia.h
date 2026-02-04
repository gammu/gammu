/**
 * \file gammu-nokia.h
 * \author Michal Čihař
 *
 * Nokia data and functions.
 */
#ifndef __gammu_nokia_h
#define __gammu_nokia_h

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * \defgroup Nokia Nokia
 * Nokias manipulations.
 */

#include <gammu-bitmap.h>
#include <gammu-settings.h>

/**
 * Gets default caller group name.
 *
 * \param Bitmap Storage for default bitmap.
 *
 * \ingroup Nokia
 */
void NOKIA_GetDefaultCallerGroupName(GSM_Bitmap * Bitmap);

/**
 * Gets default profile name.
 *
 * \param Profile Storage for default profile.
 *
 * \ingroup Nokia
 */
void NOKIA_GetDefaultProfileName(GSM_Profile * Profile);
#ifdef	__cplusplus
}
#endif
#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

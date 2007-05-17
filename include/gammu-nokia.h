/**
 * \file gammu-nokia.h
 * \author Michal Čihař
 * 
 * Nokia data and functions.
 */
#ifndef __gammu_nokia_h
#define __gammu_nokia_h

/**
 * \defgroup Nokia Nokia
 * Nokias manipulations.
 */

#include <gammu-statemachine.h>
#include <gammu-bitmap.h>

void 		NOKIA_GetDefaultCallerGroupName	(GSM_StateMachine *s, GSM_Bitmap *Bitmap);

#endif

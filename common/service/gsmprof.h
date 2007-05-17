/* (c) 2002-2003 by Marcin Wiacek */

#ifndef __gsm_prof_h
#define __gsm_prof_h


typedef struct {
	GSM_Profile_Feat_ID	ID;
	GSM_Profile_Feat_Value	Value;
	unsigned char		PhoneID;
	unsigned char		PhoneValue;
} GSM_Profile_PhoneTableValue;

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

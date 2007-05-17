/* (c) 2002-2004 by Marcin Wiacek */

#ifndef __gsm_ring_h
#define __gsm_ring_h

#include <gammu-ringtone.h>
/* --------------- Smart Messaging Specification 2.0 & 3.0 ----------------- */

#define SM_CommandEnd_CommandEnd 		0x00

/* specification gives also other */
#define SM_Command_RingingToneProgramming 	0x25<<1
#define SM_Command_Sound                  	0x1d<<1
/* specification gives also other */

#define SM_Song_BasicSongType			0x01<<5
/* specification gives also other */

#define SM_PatternID_A_part 			0x00<<6
/* specification gives also other */

#define SM_InstructionID_PatternHeaderId      	0x00<<5
#define SM_InstructionID_NoteInstructionId    	0x01<<5
#define SM_InstructionID_ScaleInstructionId   	0x02<<5
#define SM_InstructionID_StyleInstructionId   	0x03<<5
#define SM_InstructionID_TempoInstructionId   	0x04<<5
#define SM_InstructionID_VolumeInstructionId  	0x05<<5

/* ------ end of Smart Messaging Specification 2.0 & 3.0 definitions ------- */


GSM_Error GSM_SaveRingtoneFile(char *FileName, GSM_Ringtone *ringtone);

void saveott(FILE *file, GSM_Ringtone *ringtone);
void savemid(FILE *file, GSM_Ringtone *ringtone);
void saverng(FILE *file, GSM_Ringtone *ringtone);
void saveimelody(FILE *file, GSM_Ringtone *ringtone);
GSM_Error savewav(FILE *file, GSM_Ringtone *ringtone);
GSM_Error saverttl(FILE *file, GSM_Ringtone *ringtone);

unsigned char	GSM_EncodeNokiaRTTLRingtone	(GSM_Ringtone ringtone, unsigned char *package, int *maxlength);
unsigned char 	GSM_EncodeEMSSound		(GSM_Ringtone ringtone, unsigned char *package, int *maxlength, double version, bool start);

GSM_Error	GSM_DecodeNokiaRTTLRingtone	(GSM_Ringtone *ringtone, unsigned char *package, int maxlength);


int GSM_RTTLGetTempo		(int Beats);
int GSM_RingNoteGetFrequency	(GSM_RingNote Note);
int GSM_RingNoteGetFullDuration	(GSM_RingNote Note);

char *GSM_GetRingtoneName(GSM_AllRingtonesInfo *Info, int ID);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

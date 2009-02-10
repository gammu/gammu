/* (c) 2002-2004 by Marcin Wiacek */

#ifndef __gsm_ring_h
#define __gsm_ring_h

#include <gammu-ringtone.h>
/* --------------- Smart Messaging Specification 2.0 & 3.0 ----------------- */

#define SM_CommandEnd_CommandEnd 		0x00

/* specification gives also other */
#define SM_Command_RingingToneProgramming 	(0x25 << 1)
#define SM_Command_Sound                  	(0x1d << 1)
/* specification gives also other */

#define SM_Song_BasicSongType			(0x01 << 5)
/* specification gives also other */

#define SM_PatternID_A_part 			(0x00 << 6)
/* specification gives also other */

#define SM_InstructionID_PatternHeaderId      	(0x00 << 5)
#define SM_InstructionID_NoteInstructionId    	(0x01 << 5)
#define SM_InstructionID_ScaleInstructionId   	(0x02 << 5)
#define SM_InstructionID_StyleInstructionId   	(0x03 << 5)
#define SM_InstructionID_TempoInstructionId   	(0x04 << 5)
#define SM_InstructionID_VolumeInstructionId  	(0x05 << 5)

/* ------ end of Smart Messaging Specification 2.0 & 3.0 definitions ------- */

typedef enum {
	/**
	 * No header and footer.
	 */
	GSM_Ring_NoHeader = 1,
	/**
	 * Partial header and footer.
	 */
	GSM_Ring_PartialHeader,
	/**
	 * IMelody 1.0.
	 */
	GSM_Ring_IMelody10,
	/**
	 * IMelody 1.2.
	 */
	GSM_Ring_IMelody12,
} GSM_RingtoneVersion;


unsigned char	GSM_EncodeNokiaRTTLRingtone	(GSM_Ringtone ringtone, unsigned char *package, size_t *maxlength);
unsigned char 	GSM_EncodeEMSSound		(GSM_Ringtone ringtone, unsigned char *package, size_t *maxlength, GSM_RingtoneVersion version, bool start);

GSM_Error	GSM_DecodeNokiaRTTLRingtone	(GSM_Ringtone *ringtone, unsigned char *package, size_t maxlength);


int GSM_RingNoteGetFrequency	(GSM_RingNote Note);
int GSM_RingNoteGetFullDuration	(GSM_RingNote Note);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

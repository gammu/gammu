/* (c) 2002-2004 by Marcin Wiacek */

#ifndef __gsm_ring_h
#define __gsm_ring_h

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

#define MAX_RINGTONE_NOTES 255

typedef enum {
	/**
	 * Natural style (rest between notes)
	 */
	NaturalStyle				= 0x00<<6,
	/**
	 * Continuous style (no rest between notes)
	 */
	ContinuousStyle				= 0x01<<6,
	/**
	 * Staccato style (shorter notes and longer rest period)
	 */
	StaccatoStyle				= 0x02<<6
} GSM_RingNoteStyle;

typedef enum {
	Note_Pause 				= 0x00<<4,
	Note_C    				= 0x01<<4,
	Note_Cis   				= 0x02<<4,
	Note_D     				= 0x03<<4,
	Note_Dis   				= 0x04<<4,
	Note_E     				= 0x05<<4,
	Note_F     				= 0x06<<4,
	Note_Fis   				= 0x07<<4,
	Note_G     				= 0x08<<4,
	Note_Gis   				= 0x09<<4,
	Note_A     				= 0x0a<<4,
	Note_Ais   				= 0x0b<<4,
	Note_H     				= 0x0c<<4 
} GSM_RingNoteNote;

typedef enum {
	Duration_Full 				= 0x00<<5,
	Duration_1_2  				= 0x01<<5,
	Duration_1_4  				= 0x02<<5,
	Duration_1_8  				= 0x03<<5,
	Duration_1_16 				= 0x04<<5,
	Duration_1_32 				= 0x05<<5
} GSM_RingNoteDuration;

typedef enum {
	NoSpecialDuration 			= 0x00<<6,
	DottedNote        			= 0x01<<6,
	DoubleDottedNote  			= 0x02<<6,
	Length_2_3        			= 0x03<<6
} GSM_RingNoteDurationSpec;

typedef enum {
	Scale_55 = 1,				/* 55 Hz for note A 	 */
	Scale_110,				/* 110 Hz for note A 	 */
	Scale_220,
	Scale_440,				/* first scale for Nokia */
	Scale_880,
	Scale_1760,
	Scale_3520,				/* last scale for Nokia  */
	Scale_7040,
	Scale_14080
} GSM_RingNoteScale;

typedef struct {
	GSM_RingNoteDuration			Duration;
	GSM_RingNoteDurationSpec		DurationSpec;
	GSM_RingNoteNote			Note;
	GSM_RingNoteStyle			Style;
	GSM_RingNoteScale			Scale;
	int					Tempo;
} GSM_RingNote;

typedef enum {
	RING_Note = 1,
	RING_EnableVibra,
	RING_DisableVibra,
	RING_EnableLight,
	RING_DisableLight,
	RING_EnableLED,
	RING_DisableLED,
	RING_Repeat
} GSM_RingCommandType;

typedef struct {
	GSM_RingCommandType			Type;
	GSM_RingNote				Note;
	unsigned char				Value;
} GSM_RingCommand;

typedef struct {
	int					NrCommands;
	GSM_RingCommand				Commands[MAX_RINGTONE_NOTES];
	bool					AllNotesScale;
} GSM_NoteRingtone;

/* FIXME: should use BinaryTone instead? */
/* Structure to hold Nokia binary ringtones. */
typedef struct {
	unsigned char				Frame[50000];
	int					Length;
} GSM_NokiaBinaryRingtone;

typedef struct {
	unsigned char				*Buffer;
	int					Length;
} GSM_BinaryTone;

typedef enum {
	RING_NOTETONE = 1,
	RING_NOKIABINARY,
	RING_MIDI,
	RING_MMF
} GSM_RingtoneFormat;

/**
 * Structure for saving various ringtones formats
 */
typedef struct {
	/**
	 * Ringtone saved in one of three formats
	 */
	GSM_NokiaBinaryRingtone			NokiaBinary;
	GSM_BinaryTone				BinaryTone;
	GSM_NoteRingtone			NoteTone;
	/**
	 * Ringtone format
	 */
	GSM_RingtoneFormat			Format;
	/**
	 * Ringtone name
	 */
	char					Name[20*2];
	/**
	 * Ringtone location
	 */
	int					Location;
} GSM_Ringtone;

typedef struct {
	int					Group;	//Nokia specific
	int					ID;
	char					Name[30*2];
} GSM_RingtoneInfo;

typedef struct {
	int					Number;
 	GSM_RingtoneInfo			*Ringtone;
} GSM_AllRingtonesInfo;

GSM_Error GSM_SaveRingtoneFile(char *FileName, GSM_Ringtone *ringtone);
GSM_Error GSM_ReadRingtoneFile(char *FileName, GSM_Ringtone *ringtone);

void saveott(FILE *file, GSM_Ringtone *ringtone);
void savemid(FILE *file, GSM_Ringtone *ringtone);
void saverng(FILE *file, GSM_Ringtone *ringtone);
void saveimelody(FILE *file, GSM_Ringtone *ringtone);
GSM_Error savewav(FILE *file, GSM_Ringtone *ringtone);
GSM_Error saverttl(FILE *file, GSM_Ringtone *ringtone);

unsigned char	GSM_EncodeNokiaRTTLRingtone	(GSM_Ringtone ringtone, unsigned char *package, int *maxlength);
unsigned char 	GSM_EncodeEMSSound		(GSM_Ringtone ringtone, unsigned char *package, int *maxlength, double version, bool start);

GSM_Error	GSM_DecodeNokiaRTTLRingtone	(GSM_Ringtone *ringtone, unsigned char *package, int maxlength);

GSM_Error GSM_RingtoneConvert(GSM_Ringtone *dest, GSM_Ringtone *src, GSM_RingtoneFormat	Format);

int GSM_RTTLGetTempo		(int Beats);
int GSM_RingNoteGetFrequency	(GSM_RingNote Note);
int GSM_RingNoteGetFullDuration	(GSM_RingNote Note);

char *GSM_GetRingtoneName(GSM_AllRingtonesInfo *Info, int ID);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

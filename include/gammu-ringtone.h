/**
 * \file gammu-ringtone.h
 * \author Michal Čihař
 * 
 * Ringtone data and functions.
 */
#ifndef __gammu_ringtone_h
#define __gammu_ringtone_h

/**
 * \defgroup Ringtone Ringtone
 * Ringtones manipulations.
 */

#include <gammu-types.h>
#include <gammu-error.h>
#include <gammu-statemachine.h>


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
	unsigned char				Name[20*2];
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

/**
 * Play one note using state machine interface.
 */
GSM_Error PHONE_RTTLPlayOneNote		(GSM_StateMachine *s, GSM_RingNote note, bool first);

/**
 * Makes phone beek using state machine interface.
 */
GSM_Error PHONE_Beep			(GSM_StateMachine *s);


#endif

#ifndef __gsm_ringtones_h
#define __gsm_ringtones_h

/* --------------- Smart Messaging Specification 2.0 & 3.0 ----------------- */

/* Command-Part Encoding */
#define RingingToneProgramming 	(0x25<<1) /* binary 0100 101 */
#define Sound                  	(0x1d<<1) /* binary 0011 101 */
/*...*/

/* Song-Type Encoding */
#define BasicSongType		(0x01<<5) /* binary 001 */
/*...*/

/* Instruction ID Encoding */
#define PatternHeaderId      	(0x00<<5) /* binary 000 */
#define NoteInstructionId    	(0x01<<5) /* binary 001 */
#define ScaleInstructionId   	(0x02<<5) /* binary 010 */
#define StyleInstructionId   	(0x03<<5) /* binary 011 */
#define TempoInstructionId   	(0x04<<5) /* binary 100 */
#define VolumeInstructionId  	(0x05<<5) /* binary 101 */

/* Pattern ID Encoding */
#define A_part 			(0x00<<6) /* binary 00 	*/
/*...*/

/* Command-End */
#define CommandEnd (0x00) /* binary 00000000 */

/* ------ end of Smart Messaging Specification 2.0 & 3.0 definitions ------- */

#define MAX_RINGTONE_NOTES 255

/* Style-Value Encoding*/
typedef enum {
	NaturalStyle	= (0x00<<6),	/* binary 00 */
	ContinuousStyle	= (0x01<<6),	/* binary 01 */
	StaccatoStyle	= (0x02<<6)	/* binary 10 */
} GSM_RingNoteStyle;

/* Note-Value Encoding */
typedef enum {
	Note_Pause 	= (0x00<<4), 	/* binary 0000 */
	Note_C    	= (0x01<<4),	/* binary 0001 */
	Note_Cis   	= (0x02<<4), 	/* binary 0010 */
	Note_D     	= (0x03<<4), 	/* binary 0011 */
	Note_Dis   	= (0x04<<4), 	/* binary 0100 */
	Note_E     	= (0x05<<4), 	/* binary 0101 */
	Note_F     	= (0x06<<4), 	/* binary 0110 */
	Note_Fis   	= (0x07<<4), 	/* binary 0111 */
	Note_G     	= (0x08<<4), 	/* binary 1000 */
	Note_Gis   	= (0x09<<4), 	/* binary 1001 */
	Note_A     	= (0x0a<<4), 	/* binary 1010 */
	Note_Ais   	= (0x0b<<4), 	/* binary 1011 */
	Note_H     	= (0x0c<<4) 	/* binary 1100 */
} GSM_RingNoteNote;

/* Note-Duration Encoding */
typedef enum {
	Duration_Full 	= (0x00<<5), 	/* binary 000 */
	Duration_1_2  	= (0x01<<5), 	/* binary 001 */
	Duration_1_4  	= (0x02<<5), 	/* binary 010 */
	Duration_1_8  	= (0x03<<5), 	/* binary 011 */
	Duration_1_16 	= (0x04<<5), 	/* binary 100 */
	Duration_1_32 	= (0x05<<5) 	/* binary 101 */
} GSM_RingNoteDuration;

/* Note-Duration-Specifier Encoding */
typedef enum {
	NoSpecialDuration = (0x00<<6),	/* binary 00 */
	DottedNote        = (0x01<<6), 	/* binary 01 */
	DoubleDottedNote  = (0x02<<6), 	/* binary 10 */
	Length_2_3        = (0x03<<6), 	/* binary 11 */
} GSM_RingNoteDurationSpec;

typedef enum {
	Scale_55 = 1,	/* 55 Hz for note A 		*/
	Scale_110,	/* 110 Hz for note A 		*/
	Scale_220,
	Scale_440,	/* first scale for Nokia phones */
	Scale_880,
	Scale_1760,
	Scale_3520,	/* last scale for Nokia phones 	*/
	Scale_7040,
	Scale_14080
} GSM_RingNoteScale;

typedef struct {
	GSM_RingNoteDuration		Duration;
	GSM_RingNoteDurationSpec	DurationSpec;
	GSM_RingNoteNote		Note;
	GSM_RingNoteStyle		Style;
	GSM_RingNoteScale		Scale;
	int				Tempo;
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
	GSM_RingCommandType	Type;
	GSM_RingNote		Note;
	unsigned char		Value;
} GSM_RingCommand;

typedef struct {
	unsigned char		NrCommands;
	GSM_RingCommand		Commands[MAX_RINGTONE_NOTES];
	bool			AllNotesScale;
} GSM_NoteRingtone;

/* Structure to hold Nokia binary ringtones. */
typedef struct {
	unsigned char		Frame[1000];
	int			Length;
} GSM_NokiaBinaryRingtone;

typedef enum {
	RING_NOTETONE = 1,
	RING_NOKIABINARY,
	RING_MIDI
} GSM_RingtoneFormat;

typedef struct {
	GSM_NokiaBinaryRingtone	NokiaBinary;
	GSM_NoteRingtone	NoteTone;
	GSM_RingtoneFormat	Format;
	char			Name[20*2];
	int			Location;
} GSM_Ringtone;

typedef struct {
	int			ID;
	char			Name[30*2];
} GSM_RingtoneInfo;

typedef struct {
	int			Number;
	GSM_RingtoneInfo	Ringtone[50];
} GSM_AllRingtonesInfo;

GSM_Error GSM_SaveRingtoneFile(char *FileName, GSM_Ringtone *ringtone);
GSM_Error GSM_ReadRingtoneFile(char *FileName, GSM_Ringtone *ringtone);

GSM_Error saverttl(FILE *file, GSM_Ringtone *ringtone);

unsigned char	GSM_EncodeNokiaRTTLRingtone	(GSM_Ringtone ringtone, unsigned char *package, int *maxlength);
unsigned char   GSM_EncodeEMSSound		(GSM_Ringtone ringtone, unsigned char *package, int *maxlength, double version);

GSM_Error	GSM_DecodeNokiaRTTLRingtone	(GSM_Ringtone *ringtone, unsigned char *package, int maxlength);

GSM_Error GSM_RingtoneConvert(GSM_Ringtone *dest, GSM_Ringtone *src, GSM_RingtoneFormat	Format);

int GSM_RTTLGetTempo		(int Beats);
int GSM_RingNoteGetFrequency	(GSM_RingNote Note);
int GSM_RingNoteGetFullDuration	(GSM_RingNote Note);

char *GSM_GetRingtoneName(GSM_AllRingtonesInfo *Info, int ID);

#endif	/* __gsm_ringtones_h */

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

#ifndef __gsm_calendar_h
#define __gsm_calendar_h

#include "../gsmcomon.h"

/* Define enums for Calendar Note types */
typedef enum {
	GCN_REMINDER=1, /* Reminder 		     */
	GCN_CALL,       /* Call 		     */
	GCN_MEETING,    /* Meeting 		     */
	GCN_BIRTHDAY,   /* Birthday 		     */
	GCN_T_ATHL,     /* Training - Athletism      */
        GCN_T_BALL,     /* Training - Ball Games     */
        GCN_T_CYCL,     /* Training - Cycling        */
        GCN_T_BUDO,     /* Training - Budo           */
        GCN_T_DANC,     /* Training - Dance          */
        GCN_T_EXTR,     /* Training - Extreme Sports */
        GCN_T_FOOT,     /* Training - Fotball        */
        GCN_T_GOLF,     /* Training - Golf           */
        GCN_T_GYM,      /* Training - Gym            */
        GCN_T_HORS,     /* Training - Horse Race     */
        GCN_T_HOCK,     /* Training - Hockey         */
        GCN_T_RACE,     /* Training - Races          */
        GCN_T_RUGB,     /* Training - Rugby          */
        GCN_T_SAIL,     /* Training - Sailing        */
        GCN_T_STRE,     /* Training - Street Games   */
        GCN_T_SWIM,     /* Training - Swimming       */
        GCN_T_TENN,     /* Training - Tennis         */
        GCN_T_TRAV,     /* Training - Travels        */
        GCN_T_WINT      /* Training - Winter Games   */
} GSM_CalendarNoteType;

#define MAX_CALENDAR_TEXT_LENGTH	257*2 /* In 6310 max. 256 chars */
#define MAX_CALENDAR_PHONE_LENGTH	72

/* Calendar note type */
typedef struct {
	int			Location;	/* The number of the note in the phone memory */
	GSM_CalendarNoteType	Type;		/* The type of the note */
	GSM_DateTime		Time;		/* The time of the note */
	GSM_DateTime		Alarm;		/* The alarm of the note */
	unsigned char		Text [MAX_CALENDAR_TEXT_LENGTH];	/* The text of the note */
  	unsigned char		Phone[MAX_CALENDAR_PHONE_LENGTH];	/* For Call only: the phone number */
	int			Recurrance;	/* after how many hours note repeats. 0x0000 = No Repeat */
	bool			SilentAlarm;	/* Silent/Tone alarm */
} GSM_CalendarNote;

void NOKIA_EncodeVCALENDAR10SMSText(char *Buffer, int *Length, GSM_CalendarNote note);

typedef enum {
	GSM_Priority_Low = 1,
	GSM_Priority_Medium,
	GSM_Priority_High
} GSM_TODO_Priority;

typedef struct {
	int			Location;
	GSM_TODO_Priority	Priority;
	unsigned char		Text[50*2];	/* text. 6310i = max. 48 chars */
} GSM_TODO;

#endif	/* __gsm_calendar_h */

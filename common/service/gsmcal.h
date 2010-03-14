#ifndef __gsm_calendar_h
#define __gsm_calendar_h

#include "../gsmcomon.h"

/* ---------------------------- calendar ----------------------------------- */

#define GSM_CALENDAR_ENTRIES		16
#define MAX_CALENDAR_TEXT_LENGTH	256 /* In 6310 max. 256 chars */

/* Define enums for Calendar Note types */
typedef enum {
	GCN_REMINDER=1, /* Reminder 		     */
	GCN_CALL,       /* Call 		     */
	GCN_MEETING,    /* Meeting 		     */
	GCN_BIRTHDAY,   /* Birthday 		     */
	GCN_MEMO,
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
        GCN_T_WINT,     /* Training - Winter Games   */

	GCN_ALARM,
	GCN_DAILY_ALARM
} GSM_CalendarNoteType;

typedef enum {
	CAL_START_DATETIME = 1,
	CAL_END_DATETIME,
	CAL_ALARM_DATETIME,
	CAL_SILENT_ALARM_DATETIME,
	CAL_RECURRANCE,
	CAL_TEXT,
	CAL_LOCATION,
	CAL_PHONE,
	CAL_PRIVATE,
	CAL_CONTACTID,
	CAL_REPEAT_DAYOFWEEK,
	CAL_REPEAT_DAY,
	CAL_REPEAT_WEEKOFMONTH,
	CAL_REPEAT_MONTH,
	CAL_REPEAT_FREQUENCY,
	CAL_REPEAT_STARTDATE,
	CAL_REPEAT_STOPDATE,
} GSM_CalendarType;

typedef struct {
	GSM_CalendarType	EntryType;
	unsigned char		Text[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
	GSM_DateTime		Date;
	unsigned int		Number;
} GSM_SubCalendarEntry;

typedef struct {
	GSM_CalendarNoteType	Type;
	int 			Location;           /* Location */
	int 			EntriesNum;         /* Number of entries */
	GSM_SubCalendarEntry   	Entries[GSM_CALENDAR_ENTRIES];
} GSM_CalendarEntry;

void GSM_CalendarFindDefaultTextTimeAlarmPhoneRecurrance(GSM_CalendarEntry *entry, int *Text, int *Time, int *Alarm, int *Phone, int *Recurrance, int *EndTime, int *Location);

typedef enum {
	Nokia_VCalendar = 1,
	Siemens_VCalendar
} GSM_VCalendarVersion;

GSM_Error GSM_EncodeVCALENDAR(char *Buffer, int *Length, GSM_CalendarEntry *note, bool header, GSM_VCalendarVersion Version);

bool IsCalendarNoteFromThePast(GSM_CalendarEntry *note);

typedef struct {
	int			StartDay;   // Monday = 1, Tuesday = 2,...
	int			AutoDelete; // 0 = no delete, 1 = after day,...
} GSM_CalendarSettings;

/* ------------------------------ to-do ------------------------------------ */

#define GSM_TODO_ENTRIES		7
#define MAX_TODO_TEXT_LENGTH    	50 /* Alcatel BE5 50 chars */

typedef enum {
	TODO_END_DATETIME = 1,
    	TODO_COMPLETED,
	TODO_ALARM_DATETIME,
	TODO_SILENT_ALARM_DATETIME,
	TODO_TEXT,
    	TODO_PRIVATE,
    	TODO_CATEGORY,
    	TODO_CONTACTID,
	TODO_PHONE
} GSM_ToDoType;

typedef enum {
	GSM_Priority_High = 1,
	GSM_Priority_Medium,
	GSM_Priority_Low
} GSM_ToDo_Priority;

typedef struct {
	GSM_ToDoType	EntryType;
	unsigned char	Text[(MAX_TODO_TEXT_LENGTH + 1)*2];
	GSM_DateTime	Date;
	unsigned int	Number;
} GSM_SubToDoEntry;

typedef struct {
	GSM_ToDo_Priority	Priority;
	int 			Location;           /* Location */
	int 			EntriesNum;         /* Number of entries */
	GSM_SubToDoEntry   	Entries[GSM_TODO_ENTRIES];
} GSM_ToDoEntry;

void GSM_ToDoFindDefaultTextTimeAlarmCompleted(GSM_ToDoEntry *entry, int *Text, int *Alarm, int *Completed, int *EndTime);

typedef enum {
	Nokia_VToDo = 1
} GSM_VToDoVersion;

GSM_Error GSM_EncodeVTODO(char *Buffer, int *Length, GSM_ToDoEntry *note, bool header, GSM_VToDoVersion Version);

typedef struct {
	int		Used;		/* Number of used positions */
} GSM_ToDoStatus;

/* ------------------------------ both ------------------------------------- */

GSM_Error GSM_DecodeVCALENDAR_VTODO(unsigned char *Buffer, int *Pos, GSM_CalendarEntry *Calendar, GSM_ToDoEntry *ToDo, GSM_VCalendarVersion CalVer, GSM_VToDoVersion ToDoVer);

#endif	/* __gsm_calendar_h */

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

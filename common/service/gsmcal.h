/* (c) 2002-2004 by Marcin Wiacek */
/* 5210 calendar IDs by Frederick Ros */

#ifndef __gsm_cal_h
#define __gsm_cal_h

#include "../gsmcomon.h"

/* ---------------------------- calendar ----------------------------------- */

#define GSM_CALENDAR_ENTRIES	    	16
#define MAX_CALENDAR_TEXT_LENGTH	256 /* In 6310 max. 256 chars */

/**
 * Enum defines types of calendar notes
 */
typedef enum {
	/**
	 * Reminder or Date
	 */
	GSM_CAL_REMINDER=1,
	/**
	 * Call
	 */
	GSM_CAL_CALL,
	/**
	 * Meeting
	 */
	GSM_CAL_MEETING,
	/**
	 * Birthday or Anniversary or Special Occasion
	 */
	GSM_CAL_BIRTHDAY,
	/**
	 * Memo or Miscellaneous
	 */
	GSM_CAL_MEMO,
	/**
	 * Travel
	 */
	GSM_CAL_TRAVEL,
	/**
	 * Vacation
	 */
	GSM_CAL_VACATION,
	/**
	 * Training - Athletism
	 */
	GSM_CAL_T_ATHL,
	/**
	 * Training - Ball Games
	 */
	GSM_CAL_T_BALL,
	/**
	 * Training - Cycling
	 */
	GSM_CAL_T_CYCL,
	/**
	 * Training - Budo
	 */
	GSM_CAL_T_BUDO,
	/**
	 * Training - Dance
	 */
	GSM_CAL_T_DANC,
	/**
	 * Training - Extreme Sports
	 */
	GSM_CAL_T_EXTR,
	/**
	 * Training - Football
	 */
	GSM_CAL_T_FOOT,
	/**
	 * Training - Golf
	 */
	GSM_CAL_T_GOLF,
	/**
	 * Training - Gym
	 */
	GSM_CAL_T_GYM,
	/**
	 * Training - Horse Race
	 */
	GSM_CAL_T_HORS,
	/**
	 * Training - Hockey
	 */
	GSM_CAL_T_HOCK,
	/**
	 * Training - Races
	 */
	GSM_CAL_T_RACE,
	/**
	 * Training - Rugby
	 */
	GSM_CAL_T_RUGB,
	/**
	 * Training - Sailing
	 */
	GSM_CAL_T_SAIL,
	/**
	 * Training - Street Games
	 */
	GSM_CAL_T_STRE,
	/**
	 * Training - Swimming
	 */
	GSM_CAL_T_SWIM,
	/**
	 * Training - Tennis
	 */
	GSM_CAL_T_TENN,
	/**
	 * Training - Travels
	 */
	GSM_CAL_T_TRAV,
	/**
	 * Training - Winter Games
	 */
	GSM_CAL_T_WINT,
	/**
	 * Alarm
	 */
	GSM_CAL_ALARM,
	/**
	 * Alarm repeating each day.
	 */
	GSM_CAL_DAILY_ALARM
} GSM_CalendarNoteType;

/**
 * One value of calendar event.
 */
typedef enum {
	/**
	 * Date and time of event start.
	 */
	CAL_START_DATETIME = 1,
	/**
	 * Date and time of event end.
	 */
	CAL_END_DATETIME,
	/**
	 * Alarm date and time.
	 */
	CAL_ALARM_DATETIME,
	/**
	 * Date and time of silent alarm.
	 */
	CAL_SILENT_ALARM_DATETIME,
	/**
	 * Text.
	 */
	CAL_TEXT,
	/**
	 * Location.
	 */
	CAL_LOCATION,
	/**
	 * Phone number.
	 */
	CAL_PHONE,
	/**
	 * Whether this entry is private.
	 */
	 CAL_PRIVATE,
	/**
	 * Related contact id.
	 */
	CAL_CONTACTID,
	/**
	 * Repeat each x'th day of week.
	 */
	CAL_REPEAT_DAYOFWEEK,
	/**
	 * Repeat each x'th day of month.
	 */
	CAL_REPEAT_DAY,
	/**
	 * Repeat x'th week of month.
	 */
	CAL_REPEAT_WEEKOFMONTH,
	/**
	 * Repeat x'th month.
	 */
	CAL_REPEAT_MONTH,
	/**
	 * Repeating frequency.
	 */
	CAL_REPEAT_FREQUENCY,
	/**
	 * Repeating start.
	 */
	CAL_REPEAT_STARTDATE,
	/**
	 * Repeating end.
	 */
	CAL_REPEAT_STOPDATE
} GSM_CalendarType;

/**
 * One value of calendar event.
 */
typedef struct {
	/**
	 * Type of value.
	 */
	 GSM_CalendarType	EntryType;
	/**
	 * Text of value, if applicable.
	 */
	 unsigned char	   	Text[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
	/**
	 * Date and time of value, if applicable.
	 */
	 GSM_DateTime	    	Date;
	/**
	 * Number of value, if applicable.
	 */
	 unsigned int	    	Number;
} GSM_SubCalendarEntry;

/**
 * Calendar note values.
 */
typedef struct {
	/**
	 * Type of calendar note.
	 */
	GSM_CalendarNoteType    Type;
	/**
	 * Location in memory.
	 */
	int		     	Location;
	/**
	 * Number of entries.
	 */
	int		     	EntriesNum;
	/**
	 * Values of entries.
	 */
	GSM_SubCalendarEntry    Entries[GSM_CALENDAR_ENTRIES];
} GSM_CalendarEntry;

void GSM_CalendarFindDefaultTextTimeAlarmPhone(GSM_CalendarEntry *entry, int *Text, int *Time, int *Alarm, int *Phone, int *EndTime, int *Location);

typedef enum {
	Nokia_VCalendar = 1,
	Siemens_VCalendar,
	SonyEricsson_VCalendar
} GSM_VCalendarVersion;

GSM_Error GSM_EncodeVCALENDAR(char *Buffer, int *Length, GSM_CalendarEntry *note, bool header, GSM_VCalendarVersion Version);

bool IsCalendarNoteFromThePast(GSM_CalendarEntry *note);

typedef struct {
	/**
	 * Monday = 1, Tuesday = 2,...
	 */
	int		     StartDay;
	/**
	 * 0 = no delete, 1 = after day,...
	 */
	int		     AutoDelete;
} GSM_CalendarSettings;

/**
 * Structure used for returning calendar status.
 */
typedef struct {
	/**
	 * Number of used positions.
	 */
	int		     Used;
} GSM_CalendarStatus;

void GSM_GetCalendarRecurranceRepeat(unsigned char *rec, unsigned char *endday, GSM_CalendarEntry *entry);
void GSM_SetCalendarRecurranceRepeat(unsigned char *rec, unsigned char *endday, GSM_CalendarEntry *entry);

/* ------------------------------ to-do ------------------------------------ */

#define GSM_TODO_ENTRIES		7
#define MAX_TODO_TEXT_LENGTH	    	50 /* Alcatel BE5 50 chars */

/**
 * Types of to do values. In parenthesis is member of @ref GSM_SubToDoEntry,
 * where value is stored.
 */
typedef enum {
	/**
	 * Due date. (Date)
	 */
	TODO_END_DATETIME = 1,
	/**
	 * Whether is completed. (Number)
	 */
	TODO_COMPLETED,
	/**
	 * When should alarm be fired (Date).
	 */
	TODO_ALARM_DATETIME,
	/**
	 * When should silent alarm be fired (Date).
	 */
	TODO_SILENT_ALARM_DATETIME,
	/**
	 * Text of to do (Text).
	 */
	TODO_TEXT,
	/**
	 * Whether entry is private (Number).
	 */
	TODO_PRIVATE,
	/**
	 * Category of entry (Number).
	 */
	TODO_CATEGORY,
	/**
	 * Related contact ID (Number).
	 */
	TODO_CONTACTID,
	/**
	 * Number to call (Text).
	 */
	TODO_PHONE
} GSM_ToDoType;

/**
 * Priority of to do.
 */
typedef enum {
	GSM_Priority_High = 1,
	GSM_Priority_Medium,
	GSM_Priority_Low
} GSM_ToDo_Priority;

/**
 * Value of to do entry.
 */
typedef struct {
	/**
	 * Type of entry.
	 */
	GSM_ToDoType    EntryType;
	/**
	 * Text of value, if appropriate, see @ref GSM_ToDoType.
	 */
	unsigned char   Text[(MAX_TODO_TEXT_LENGTH + 1)*2];
	/**
	 * Date of value, if appropriate, see @ref GSM_ToDoType.
	 */
	GSM_DateTime    Date;
	/**
	 * Number of value, if appropriate, see @ref GSM_ToDoType.
	 */
	unsigned int    Number;
} GSM_SubToDoEntry;

/**
 * To do entry.
 */
typedef struct {
	/**
	 * Priority of entry.
	 */
	GSM_ToDo_Priority       Priority;
	/**
	 * Location in memory.
	 */
	int		     	Location;
	/**
	 * Number of entries.
	 */
	int		     	EntriesNum;
	/**
	 * Values of current entry.
	 */
	GSM_SubToDoEntry	Entries[GSM_TODO_ENTRIES];
} GSM_ToDoEntry;

void GSM_ToDoFindDefaultTextTimeAlarmCompleted(GSM_ToDoEntry *entry, int *Text, int *Alarm, int *Completed, int *EndTime, int *Phone);

typedef enum {
	Nokia_VToDo = 1,
	SonyEricsson_VToDo
} GSM_VToDoVersion;

GSM_Error GSM_EncodeVTODO(char *Buffer, int *Length, GSM_ToDoEntry *note, bool header, GSM_VToDoVersion Version);

/**
 * Status of to do entries.
 */
typedef struct {
	/**
	 * Number of used positions.
	 */
	int	     Used;
} GSM_ToDoStatus;

/* --------------------------- note ---------------------------------------- */

typedef struct {
	int	     Location;
	char	     Text[3000*2];
} GSM_NoteEntry;

GSM_Error GSM_EncodeVNTFile(unsigned char *Buffer, int *Length, GSM_NoteEntry *Note);

/* --------------------------- alarm --------------------------------------- */

/**
 * Alarm values.
 */
typedef struct {
	/**
	 * Location where it is stored.
	 */
	int	     	Location;
	/**
	 * Date and time of alarm.
	 */
	GSM_DateTime    DateTime;
	/**
	 * Whether it repeats each day.
	 */
	bool	    	Repeating;
	/**
	 * Text that is shown on display.
	 */
	char	    	Text[(MAX_CALENDAR_TEXT_LENGTH + 1) * 2];
} GSM_Alarm;

/* --------------------------- calendar & todo ----------------------------- */

GSM_Error GSM_DecodeVCALENDAR_VTODO(unsigned char *Buffer, int *Pos, GSM_CalendarEntry *Calendar, GSM_ToDoEntry *ToDo, GSM_VCalendarVersion CalVer, GSM_VToDoVersion ToDoVer);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

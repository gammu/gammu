/**
 * \file gammu-calendar.h
 * \author Michal Čihař
 * 
 * Calendar data and functions.
 */
#ifndef __gammu_calendar_h
#define __gammu_calendar_h

/**
 * \defgroup Calendar Calendar
 * Calendar events manipulations.
 */

#define GSM_CALENDAR_ENTRIES	    	16
#define MAX_CALENDAR_TEXT_LENGTH	256 /* In 6310 max. 256 chars */

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
 * Status of to do entries.
 */
typedef struct {
	/**
	 * Number of free positions.
	 */
	int	     Free;
	/**
	 * Number of used positions.
	 */
	int	     Used;
} GSM_ToDoStatus;
/**
 * Structure used for returning calendar status.
 */
typedef struct {
	/**
	 * Number of free positions.
	 */
	int		     Free;
	/**
	 * Number of used positions.
	 */
	int		     Used;
} GSM_CalendarStatus;
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
	CAL_TONE_ALARM_DATETIME,
	/**
	 * Date and time of silent alarm.
	 */
	CAL_SILENT_ALARM_DATETIME,
	/**
	 * Text.
	 */
	CAL_TEXT,
	/**
	 * Detailed description.
	 */
	CAL_DESCRIPTION,
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
	 * Repeat each x'th day of year.
	 */
	CAL_REPEAT_DAYOFYEAR,
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
	CAL_REPEAT_STOPDATE,
	/**
	 * Number of repetitions.
	 */
	CAL_REPEAT_COUNT,
	/**
	 * IrMC LUID which can be used for synchronisation.
	 */
	CAL_LUID,
	/**
	 * Date and time of last modification.
	 */
	CAL_LAST_MODIFIED,
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
	/**
	 * During adding SubEntry Gammu can return here info, if it was done OK
	 */
	GSM_Error		AddError;
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

#define GSM_TODO_ENTRIES		7
#define MAX_TODO_TEXT_LENGTH	    	160 /* N6230i */

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
	 * Description of to do (Text).
	 */
	TODO_DESCRIPTION,
	/**
	 * Location of to do (Text).
	 */
	TODO_LOCATION,
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
	TODO_PHONE,
	/**
	 * IrMC LUID which can be used for synchronisation.
	 */
	TODO_LUID,
	/**
	 * Date and time of last modification.
	 */
	TODO_LAST_MODIFIED,
} GSM_ToDoType;

/**
 * Priority of to do.
 */
typedef enum {
	GSM_Priority_None = 0,
	GSM_Priority_High,
	GSM_Priority_Medium,
	GSM_Priority_Low
} GSM_ToDo_Priority;

/**
 * Time Units.
 */
typedef enum {
	GSM_TimeUnit_Unknown = 0,
	GSM_TimeUnit_Days,
	GSM_TimeUnit_Hours,
	GSM_TimeUnit_Minutes,
	GSM_TimeUnit_Seconds
} GSM_TimeUnit;

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
	 * Type of todo note.
	 */
	GSM_CalendarNoteType    Type;
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

typedef struct {
	int	     Location;
	char	     Text[3000*2];
} GSM_NoteEntry;

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
	unsigned char	Text[(MAX_CALENDAR_TEXT_LENGTH + 1) * 2];
} GSM_Alarm;

/**
 * Reads alarm set in phone.
 */
GSM_Error GSM_GetAlarm(GSM_StateMachine *s, GSM_Alarm	*alarm);
/**
 * Sets alarm in phone.
 */
GSM_Error GSM_SetAlarm(GSM_StateMachine *s, GSM_Alarm *alarm);

/**
 * Gets status of ToDos (count of used entries).
 */
GSM_Error GSM_GetToDoStatus(GSM_StateMachine *s, GSM_ToDoStatus *status);
/**
 * Reads ToDo from phone.
 */
GSM_Error GSM_GetToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo);
/**
 * Reads ToDo from phone.
 */
GSM_Error GSM_GetNextToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo, bool start);
/**
 * Sets ToDo in phone.
 */
GSM_Error GSM_SetToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo);
/**
 * Adds ToDo in phone.
 */
GSM_Error GSM_AddToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo);
/**
 * Deletes ToDo entry in phone.
 */
GSM_Error GSM_DeleteToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo);
/**
 * Deletes all todo entries in phone.
 */
GSM_Error GSM_DeleteAllToDo(GSM_StateMachine *s);
/**
 * Retrieves calendar status (number of used entries).
 */
GSM_Error GSM_GetCalendarStatus(GSM_StateMachine *s, GSM_CalendarStatus *Status);
/**
 * Retrieves calendar entry.
 */
GSM_Error GSM_GetCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note);
/**
 * Retrieves calendar entry. This is useful for continuous reading of all
 * calendar entries.
 */
GSM_Error GSM_GetNextCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool start);
/**
 * Sets calendar entry
 */
GSM_Error GSM_SetCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note);
/**
 * Adds calendar entry.
 */
GSM_Error GSM_AddCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note);
/**
 * Deletes calendar entry.
 */
GSM_Error GSM_DeleteCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note);
/**
 * Deletes all calendar entries.
 */
GSM_Error GSM_DeleteAllCalendar(GSM_StateMachine *s);
/**
 * Reads calendar settings.
 */
GSM_Error GSM_GetCalendarSettings(GSM_StateMachine *s, GSM_CalendarSettings *settings);
/**
 * Sets calendar settings.
 */
GSM_Error GSM_SetCalendarSettings(GSM_StateMachine *s, GSM_CalendarSettings *settings);
/**
 * Retrieves notes status (number of used entries).
 */
GSM_Error GSM_GetNotesStatus(GSM_StateMachine *s, GSM_ToDoStatus *status);
/**
 * Retrieves notes entry.
 */
GSM_Error GSM_GetNote(GSM_StateMachine *s, GSM_NoteEntry *Note);
/**
 * Retrieves note entry. This is useful for continuous reading of all
 * notes entries.
 */
GSM_Error GSM_GetNextNote(GSM_StateMachine *s, GSM_NoteEntry *Note, bool start);
/**
 * Sets note entry
 */
GSM_Error GSM_SetNote(GSM_StateMachine *s, GSM_NoteEntry *Note);
/**
 * Adds note entry.
 */
GSM_Error GSM_AddNote(GSM_StateMachine *s, GSM_NoteEntry *Note);
/**
 * Deletes note entry.
 */
GSM_Error GSM_DeleteNote(GSM_StateMachine *s, GSM_NoteEntry *Note);
/**
 * Deletes all notes entries.
 */
GSM_Error GSM_DeleteAllNotes(GSM_StateMachine *s);
#endif

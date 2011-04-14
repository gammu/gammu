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

#include <gammu-datetime.h>
#include <gammu-limits.h>
#include <gammu-debug.h>

/**
 * \defgroup Note Note
 * Notes manipulations.
 */

/**
 * \defgroup Todo Todo
 * Todo entries manipulations.
 */

/**
 * Calendar settings structure.
 *
 * \ingroup Calendar
 */
typedef struct {
	/**
	 * Monday = 1, Tuesday = 2,...
	 */
	int StartDay;
	/**
	 * 0 = no delete, 1 = after day,...
	 */
	int AutoDelete;
} GSM_CalendarSettings;

/**
 * Status of to do entries.
 *
 * \ingroup Todo
 */
typedef struct {
	/**
	 * Number of free positions.
	 */
	int Free;
	/**
	 * Number of used positions.
	 */
	int Used;
} GSM_ToDoStatus;

/**
 * Structure used for returning calendar status.
 *
 * \ingroup Calendar
 */
typedef struct {
	/**
	 * Number of free positions.
	 */
	int Free;
	/**
	 * Number of used positions.
	 */
	int Used;
} GSM_CalendarStatus;

/**
 * Enum defines types of calendar notes
 *
 * \ingroup Calendar
 */
typedef enum {
	/**
	 * Reminder or Date
	 */
	GSM_CAL_REMINDER = 1,
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
	GSM_CAL_DAILY_ALARM,
	/**
	 * Shopping
	 */
	GSM_CAL_SHOPPING,
} GSM_CalendarNoteType;

/**
 * One value of calendar event.
 *
 * \ingroup Calendar
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
	CAL_CONTACTID,		/* 10 */
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
	CAL_LUID,		/* 20 */
	/**
	 * Date and time of last modification.
	 */
	CAL_LAST_MODIFIED,
} GSM_CalendarType;

/**
 * One value of calendar event.
 *
 * \ingroup Calendar
 */
typedef struct {
	/**
	 * Type of value.
	 */
	GSM_CalendarType EntryType;
	/**
	 * Date and time of value, if applicable.
	 */
	GSM_DateTime Date;
	/**
	 * Number of value, if applicable.
	 */
	int Number;
	/**
	 * During adding SubEntry Gammu can return here info, if it was done OK
	 */
	GSM_Error AddError;
	/**
	 * Text of value, if applicable.
	 */
	unsigned char Text[(GSM_MAX_CALENDAR_TEXT_LENGTH + 1) * 2];
} GSM_SubCalendarEntry;

/**
 * Calendar note values.
 *
 * \ingroup Calendar
 */
typedef struct {
	/**
	 * Type of calendar note.
	 */
	GSM_CalendarNoteType Type;
	/**
	 * Location in memory.
	 */
	int Location;
	/**
	 * Number of entries.
	 */
	int EntriesNum;
	/**
	 * Values of entries.
	 */
	GSM_SubCalendarEntry Entries[GSM_CALENDAR_ENTRIES];
} GSM_CalendarEntry;

/**
 * Finds inxedes of default entries.
 *
 * \ingroup Calendar
 */
void GSM_CalendarFindDefaultTextTimeAlarmPhone(GSM_CalendarEntry * entry,
					       int *Text, int *Time, int *Alarm,
					       int *Phone, int *EndTime,
					       int *Location);

/**
 * Types of to do values. In parenthesis is member of @ref GSM_SubToDoEntry,
 * where value is stored.
 *
 * \ingroup Todo
 */
typedef enum {
	/**
	 * Due date (Date).
	 */
	TODO_END_DATETIME = 1,
	/**
	 * Whether is completed (Number).
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
	 * IrMC LUID which can be used for synchronisation (Text).
	 */
	TODO_LUID,
	/**
	 * Date and time of last modification (Date).
	 */
	TODO_LAST_MODIFIED,
	/**
	 * Start date (Date).
	 */
	TODO_START_DATETIME,
	/**
	 * Completed date (Date).
	 */
	TODO_COMPLETED_DATETIME,
} GSM_ToDoType;

/**
 * Priority of to do.
 *
 * \ingroup Todo
 */
typedef enum {
	GSM_Priority_None = 0,
	GSM_Priority_High,
	GSM_Priority_Medium,
	GSM_Priority_Low
} GSM_ToDo_Priority;

/**
 * Value of to do entry.
 *
 * \ingroup Todo
 */
typedef struct {
	/**
	 * Type of entry.
	 */
	GSM_ToDoType EntryType;
	/**
	 * Date of value, if appropriate, see @ref GSM_ToDoType.
	 */
	GSM_DateTime Date;
	/**
	 * Number of value, if appropriate, see @ref GSM_ToDoType.
	 */
	unsigned int Number;
	/**
	 * Text of value, if appropriate, see @ref GSM_ToDoType.
	 */
	unsigned char Text[(GSM_MAX_TODO_TEXT_LENGTH + 1) * 2];
} GSM_SubToDoEntry;

/**
 * To do entry.
 *
 * \ingroup Todo
 */
typedef struct {
	/**
	 * Type of todo note.
	 */
	GSM_CalendarNoteType Type;
	/**
	 * Priority of entry.
	 */
	GSM_ToDo_Priority Priority;
	/**
	 * Location in memory.
	 */
	int Location;
	/**
	 * Number of entries.
	 */
	int EntriesNum;
	/**
	 * Values of current entry.
	 */
	GSM_SubToDoEntry Entries[GSM_TODO_ENTRIES];
} GSM_ToDoEntry;

/**
 * Note entry.
 *
 * \ingroup Note
 */
typedef struct {
	/**
	 * Location in memory.
	 */
	int Location;
	/**
	 * Text of note.
	 */
	char Text[(GSM_MAX_NOTE_TEXT_LENGTH + 1) * 2];
} GSM_NoteEntry;

/**
 * Alarm values.
 *
 * \ingroup Calendar
 */
typedef struct {
	/**
	 * Location where it is stored.
	 */
	int Location;
	/**
	 * Date and time of alarm.
	 */
	GSM_DateTime DateTime;
	/**
	 * Whether it repeats each day.
	 */
	gboolean Repeating;
	/**
	 * Text that is shown on display.
	 */
	unsigned char Text[(GSM_MAX_CALENDAR_TEXT_LENGTH + 1) * 2];
} GSM_Alarm;

/**
 * Format of vTodo.
 *
 * \ingroup Todo
 */
typedef enum {
	/**
	 * Format compatible with Nokia - limited subsed of standard.
	 */
	Nokia_VToDo = 1,
	/**
	 * Format compatible with SonyEricsson - complete standard.
	 */
	SonyEricsson_VToDo,
	/**
	 * Format compatible with Mozilla - iCalendar based.
	 */
	Mozilla_VToDo,
} GSM_VToDoVersion;

/**
 * Format of vCalendar export.
 *
 * \ingroup Calendar
 */
typedef enum {
	/**
	 * vCalendar specially hacked for Nokia .
	 */
	Nokia_VCalendar = 1,
	/**
	 * vCalendar specially hacked for Siemens.
	 */
	Siemens_VCalendar,
	/**
	 * Standard vCalendar (which works for Sony-Ericsson phones)
	 */
	SonyEricsson_VCalendar,
	/**
	 * iCalendar as compatible with Mozilla.
	 */
	Mozilla_iCalendar,
} GSM_VCalendarVersion;

/**
 * Encodes vTodo to buffer.
 *
 * \param Buffer Storage for data.
 * \param[in] buff_len Size of output buffer.
 * \param Length Pointer to current position in data (will be incremented).
 * \param note Note to encode.
 * \param header Whether to include vCalendar header.
 * \param Version Format of vTodo to create.
 *
 * \return Error code.
 *
 * \ingroup Todo
 */
GSM_Error GSM_EncodeVTODO(char *Buffer, const size_t buff_len, size_t * Length,
			  const GSM_ToDoEntry * note, const gboolean header,
			  const GSM_VToDoVersion Version);

/**
 * Encodes vCalendar to buffer.
 *
 * \param Buffer Storage for data.
 * \param[in] buff_len Size of output buffer.
 * \param Length Pointer to current position in data (will be incremented).
 * \param note Note to encode.
 * \param header Whether to include vCalendar header.
 * \param Version Format of vCalendar to create.
 *
 * \return Error code.
 *
 * \ingroup Calendar
 */
GSM_Error GSM_EncodeVCALENDAR(char *Buffer, const size_t buff_len,
			      size_t * Length, GSM_CalendarEntry * note,
			      const gboolean header,
			      const GSM_VCalendarVersion Version);

/**
 * Decodes vNote from buffer.
 *
 * \param Buffer Buffer to decode.
 * \param Pos Current position in buffer (will be updated).
 * \param Note Storage for note entry.
 *
 * \return Error code.
 *
 * \ingroup Note
 */
GSM_Error GSM_DecodeVNOTE(char *Buffer, size_t * Pos, GSM_NoteEntry * Note);

/**
 * Encodes vNote to buffer.
 *
 * \param Buffer Storage for data.
 * \param[in] buff_len Size of output buffer.
 * \param Length Pointer to current position in data (will be incremented).
 * \param Note Note to encode.
 *
 * \return Error code.
 *
 * \ingroup Note
 */
GSM_Error GSM_EncodeVNTFile(char *Buffer, const size_t buff_len,
			    size_t * Length, GSM_NoteEntry * Note);

/**
 * Decodes vCalendar and vTodo buffer.
 *
 * \param di Pointer to debugging description.
 * \param Buffer Buffer to decode.
 * \param Pos Current position in buffer (will be updated).
 * \param Calendar Storage for calendar entry.
 * \param ToDo Storage for todo entry.
 * \param CalVer Format of vCalendar.
 * \param ToDoVer Format of vTodo.
 *
 * \return Error code
 *
 * \ingroup Calendar
 */
GSM_Error GSM_DecodeVCALENDAR_VTODO(GSM_Debug_Info * di, char *Buffer,
				    size_t * Pos, GSM_CalendarEntry * Calendar,
				    GSM_ToDoEntry * ToDo,
				    GSM_VCalendarVersion CalVer,
				    GSM_VToDoVersion ToDoVer);

/**
 * Detects whether calendar note is in past.
 *
 * \param note Note to check.
 *
 * \return Whether entry is in past.
 *
 * \ingroup Calendar
 */
gboolean GSM_IsCalendarNoteFromThePast(GSM_CalendarEntry * note);

/**
 * Reads alarm set in phone.
 *
 * \param s State machine pointer.
 * \param Alarm Storage for alarm.
 *
 * \return Error code
 *
 * \ingroup Calendar
 */
GSM_Error GSM_GetAlarm(GSM_StateMachine * s, GSM_Alarm * Alarm);

/**
 * Sets alarm in phone.
 *
 * \param s State machine pointer.
 * \param Alarm Alarm to set.
 *
 * \return Error code
 *
 * \ingroup Calendar
 */
GSM_Error GSM_SetAlarm(GSM_StateMachine * s, GSM_Alarm * Alarm);

/**
 * Gets status of ToDos (count of used entries).
 *
 * \param s State machine pointer.
 * \param status Storage for todo status.
 *
 * \return Error code
 *
 * \ingroup Todo
 */
GSM_Error GSM_GetToDoStatus(GSM_StateMachine * s, GSM_ToDoStatus * status);

/**
 * Reads ToDo from phone.
 *
 * \param s State machine pointer.
 * \param ToDo Storage for note.
 *
 * \return Error code
 *
 * \ingroup Todo
 */
GSM_Error GSM_GetToDo(GSM_StateMachine * s, GSM_ToDoEntry * ToDo);

/**
 * Reads ToDo from phone.
 *
 * \param s State machine pointer.
 * \param ToDo Storage for note, if start is FALSE, should contain
 * data from previous read (at least position).
 * \param start Whether we're doing initial read or continue in reading.
 *
 * \return Error code
 *
 * \ingroup Todo
 */
GSM_Error GSM_GetNextToDo(GSM_StateMachine * s, GSM_ToDoEntry * ToDo,
			  gboolean start);
/**
 * Sets ToDo in phone.
 *
 * \param s State machine pointer.
 * \param ToDo ToDo to set, should contain valid location.
 *
 * \return Error code
 *
 * \ingroup Todo
 */
GSM_Error GSM_SetToDo(GSM_StateMachine * s, GSM_ToDoEntry * ToDo);

/**
 * Adds ToDo in phone.
 *
 * \param s State machine pointer.
 * \param ToDo ToDo to add.
 *
 * \return Error code
 *
 * \ingroup Todo
 */
GSM_Error GSM_AddToDo(GSM_StateMachine * s, GSM_ToDoEntry * ToDo);

/**
 * Deletes ToDo entry in phone.
 *
 * \param s State machine pointer.
 * \param ToDo ToDo to delete, only location is actually used.
 *
 * \return Error code
 *
 * \ingroup Todo
 */
GSM_Error GSM_DeleteToDo(GSM_StateMachine * s, GSM_ToDoEntry * ToDo);

/**
 * Deletes all todo entries in phone.
 *
 * \param s State machine pointer.
 *
 * \return Error code
 *
 * \ingroup Todo
 */
GSM_Error GSM_DeleteAllToDo(GSM_StateMachine * s);

/**
 * Retrieves calendar status (number of used entries).
 *
 * \param s State machine pointer.
 * \param Status Storage for status.
 *
 * \return Error code
 *
 * \ingroup Calendar
 */
GSM_Error GSM_GetCalendarStatus(GSM_StateMachine * s,
				GSM_CalendarStatus * Status);
/**
 * Retrieves calendar entry.
 *
 * \param s State machine pointer.
 * \param Note Storage for note.
 *
 * \return Error code
 *
 * \ingroup Calendar
 */
GSM_Error GSM_GetCalendar(GSM_StateMachine * s, GSM_CalendarEntry * Note);

/**
 * Retrieves calendar entry. This is useful for continuous reading of all
 * calendar entries.
 *
 * \param s State machine pointer.
 * \param Note Storage for note, if start is FALSE, should contain
 * data from previous read (at least position).
 * \param start Whether we're doing initial read or continue in reading.
 *
 * \return Error code
 *
 * \ingroup Calendar
 */
GSM_Error GSM_GetNextCalendar(GSM_StateMachine * s, GSM_CalendarEntry * Note,
			      gboolean start);
/**
 * Sets calendar entry
 *
 * \param s State machine pointer.
 * \param Note New note values, needs to contain valid position.
 *
 * \return Error code
 *
 * \ingroup Calendar
 */
GSM_Error GSM_SetCalendar(GSM_StateMachine * s, GSM_CalendarEntry * Note);

/**
 * Adds calendar entry.
 *
 * \param s State machine pointer.
 * \param Note Note to add.
 *
 * \return Error code
 *
 * \ingroup Calendar
 */
GSM_Error GSM_AddCalendar(GSM_StateMachine * s, GSM_CalendarEntry * Note);

/**
 * Deletes calendar entry.
 *
 * \param s State machine pointer.
 * \param Note Note to delete, must contain position.
 *
 * \return Error code
 *
 * \ingroup Calendar
 */
GSM_Error GSM_DeleteCalendar(GSM_StateMachine * s, GSM_CalendarEntry * Note);

/**
 * Deletes all calendar entries.
 *
 * \param s State machine pointer.
 *
 * \return Error code
 *
 * \ingroup Calendar
 */
GSM_Error GSM_DeleteAllCalendar(GSM_StateMachine * s);

/**
 * Reads calendar settings.
 *
 * \param s State machine pointer.
 * \param settings Storage for settings.
 *
 * \return Error code
 *
 * \ingroup Calendar
 */
GSM_Error GSM_GetCalendarSettings(GSM_StateMachine * s,
				  GSM_CalendarSettings * settings);
/**
 * Sets calendar settings.
 *
 * \param s State machine pointer.
 * \param settings New calendar settings.
 *
 * \return Error code
 *
 * \ingroup Calendar
 */
GSM_Error GSM_SetCalendarSettings(GSM_StateMachine * s,
				  GSM_CalendarSettings * settings);
/**
 * Retrieves notes status (number of used entries).
 *
 * \param s State machine pointer.
 * \param status Storage for status.
 *
 * \return Error code
 *
 * \ingroup Note
 */
GSM_Error GSM_GetNotesStatus(GSM_StateMachine * s, GSM_ToDoStatus * status);

/**
 * Retrieves notes entry.
 *
 * \param s State machine pointer.
 * \param Note Storage for note.
 *
 * \return Error code
 *
 * \ingroup Note
 */
GSM_Error GSM_GetNote(GSM_StateMachine * s, GSM_NoteEntry * Note);

/**
 * Retrieves note entry. This is useful for continuous reading of all
 * notes entries.
 *
 * \param s State machine pointer.
 * \param Note Storage for note, if start is FALSE, should contain
 * data from previous read (at least position).
 * \param start Whether we're doing initial read or continue in reading.
 *
 * \return Error code
 *
 * \ingroup Note
 */
GSM_Error GSM_GetNextNote(GSM_StateMachine * s, GSM_NoteEntry * Note,
			  gboolean start);
/**
 * Sets note entry
 *
 * \param s State machine pointer.
 * \param Note New note values, needs to contain valid position.
 *
 * \return Error code
 *
 * \ingroup Note
 */
GSM_Error GSM_SetNote(GSM_StateMachine * s, GSM_NoteEntry * Note);

/**
 * Adds note entry.
 *
 * \param s State machine pointer.
 * \param Note Note to add.
 *
 * \return Error code
 *
 * \ingroup Note
 */
GSM_Error GSM_AddNote(GSM_StateMachine * s, GSM_NoteEntry * Note);

/**
 * Deletes note entry.
 *
 * \param s State machine pointer.
 * \param Note Note to delete, must contain position.
 *
 * \return Error code
 *
 * \ingroup Note
 */
GSM_Error GSM_DeleteNote(GSM_StateMachine * s, GSM_NoteEntry * Note);

/**
 * Deletes all notes entries.
 *
 * \param s State machine pointer.
 *
 * \return Error code
 *
 * \ingroup Note
 */
GSM_Error GSM_DeleteAllNotes(GSM_StateMachine * s);

#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

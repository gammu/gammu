/* (c) 2002-2004 by Marcin Wiacek */
/* 5210 calendar IDs by Frederick Ros */

#ifndef __gsm_cal_h
#define __gsm_cal_h
#include "mbglobals.h"

void GSM_CalendarFindDefaultTextTimeAlarmPhoneRecurrance(GSM_CalendarEntry *entry, int *Text, int *Time, int *Alarm, int *Phone, int *Recurrance, int *RecurranceFreq, int *EndTime, int *Location);
void GSM_CalendarFindEntryIndex(GSM_CalendarEntry *entry, int *Text, int *Time, int *Alarm, int *Phone, int *Recurrance, int *RecurranceFreq, int *EndTime, int *Location ,int *RepeatEndDate,int *Description,int *DayofweekMask);

typedef enum {
	Nokia_VCalendar = 1,
	Siemens_VCalendar,
	SonyEricsson_VCalendar,
	Samsung_VCalendar
} GSM_VCalendarVersion;
void SaveVCALText(char *Buffer, int *Length, char *Text, char *Start);
void ReadVCALDateTime(char *Buffer, GSM_DateTime *dt,int nTimeZone=0);
void SaveVCALDateTime(char *Buffer, int *Length, GSM_DateTime *Date, char *Start);
//bool ReadVCALText(char *Buffer, char *Start,unsigned char *Value);
void GetTimeDifference(unsigned long diff, GSM_DateTime *DT, bool Plus, int multi);
void SaveVCALDateTimeWithoutTimeZone(char *Buffer, int *Length, GSM_DateTime *Date, char *Start);
void ReadVCALDateTimeWithDayLight(char *Buffer, GSM_DateTime *dt,COleDateTime dtDaylightStart,COleDateTime dtDaylightEnd,int nTimeZone=0,int nDaylightZone=0);


//GSM_Error GSM_EncodeVCALENDAR(char *Buffer, int *Length, GSM_CalendarEntry *note, bool header, GSM_VCalendarVersion Version);
GSM_Error SONYERIC_EncodeVCALENDAR(char *Buffer, int *Length, GSM_CalendarEntry *note, bool header, GSM_VCalendarVersion Version,int Timezone);
GSM_Error S55_EncodeVCALENDAR(char *Buffer, int *Length, GSM_CalendarEntry *note, bool header, GSM_VCalendarVersion Version,int Timezone);
GSM_Error NokiaS6_EncodeVCALENDAR(char *Buffer, int *Length, GSM_CalendarEntry *note, bool header, GSM_VCalendarVersion Version,int Timezone = 0);
GSM_Error MOTOE2_EncodeVCALENDAR(char *Buffer, int *Length, GSM_CalendarEntry *note, bool header, int Timezone = 0);
void SESaveVCALText(char *Buffer, int *Length, unsigned char *Text, char *Start,char *beforetext);
GSM_Error SAGEM_EncodeVCALENDAR(char *Buffer, int *Length, GSM_CalendarEntry *note, bool header);
GSM_Error Samsung_EncodeVCALENDAR(char *Buffer, int *Length, GSM_CalendarEntry *note, bool header, GSM_VCalendarVersion Version);

bool IsCalendarNoteFromThePast(GSM_CalendarEntry *note);
time_t Fill_Time_T(GSM_DateTime DT, int TZ);

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


/* ------------------------------ to-do ------------------------------------ */

#define GSM_TODO_ENTRIES		16
#define MAX_TODO_TEXT_LENGTH	    	300 /* Alcatel BE5 50 chars */


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
	GSM_CalendarNoteType    Type;
	/**
	 * Priority of entry.
	 */
	int       Priority;
	/**
	 * Location in memory.
	 */
	char		     	Location[100];
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
void GSM_ToDoFindFindEntryIndex(GSM_ToDoEntry *entry, int *Text, int *Alarm, int *Completed, int *EndTime, int *Phone
								,int *Recurrance,int *RecurranceFreq, int *RepeatEndDate,int *Description,int *DayofweekMask,int *CompletedDate);
void GSM_ToDoFindFindEntryIndexEx(GSM_ToDoEntry *entry, int *Text, int *Alarm, int *Completed, int *StartTime, int *EndTime, int *Phone
								,int *Recurrance,int *RecurranceFreq, int *RepeatEndDate,int *Description,int *DayofweekMask,int *CompletedDate);
typedef enum {
	Nokia_VToDo = 1,
	SonyEricsson_VToDo,
	Samsung_VToDo,
	Sagem_VToDo
} GSM_VToDoVersion;

GSM_Error GSM_EncodeVTODO(char *Buffer, int *Length, GSM_ToDoEntry *note, bool header, GSM_VToDoVersion Version,int Timezone);

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
	char	     Text[100];
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

GSM_Error SamsungZ_DecodeVCALENDAR_VTODO(unsigned char *Buffer, int *Pos, GSM_CalendarEntry *Calendar, GSM_ToDoEntry *ToDo, GSM_VCalendarVersion CalVer, GSM_VToDoVersion ToDoVer,int Timezone);
GSM_Error GSM_DecodeVCALENDAR_VTODO(unsigned char *Buffer, int *Pos, GSM_CalendarEntry *Calendar, GSM_ToDoEntry *ToDo, GSM_VCalendarVersion CalVer, GSM_VToDoVersion ToDoVer,int Timezone,bool bSupportDescription);
GSM_Error NokiaS6_DecodeVCALENDAR(unsigned char *Buffer, int *Pos, GSM_CalendarEntry *Calendar, GSM_ToDoEntry *ToDo, int* Timezone);
GSM_Error SAGEM_DecodeVCALENDAR_VTODO(unsigned char *Buffer, int *Pos, GSM_CalendarEntry *Calendar, GSM_ToDoEntry *ToDo, int Timezone);
GSM_Error MOTOE2_DecodeVCALENDAR(unsigned char *Buffer, int *Pos, GSM_CalendarEntry *Calendar, GSM_ToDoEntry *ToDo, int* Timezone);
/* --------------------------- recurrence ----------------------------- */
bool GetRepeatDatesCount_Daily(COleDateTime dtStart,int interval,int* nCount ,COleDateTime& dtRepeatEndDate);
bool GetRepeatDatesCount_Weekly(COleDateTime dtStart,int interval,int* nCount ,COleDateTime& dtRepeatEndDate,int DayofWeekMask);
bool GetRepeatDatesCount_Monthly(COleDateTime dtStart,int interval,int* nCount ,COleDateTime& dtRepeatEndDate);
bool GetRepeatDatesCount_MonthNth(COleDateTime dtStart,int interval,int* nCount ,COleDateTime& dtRepeatEndDate,int Week,int Day);

bool GetRepeatDate_Daily(COleDateTime dtStart,int interval,int nCount ,COleDateTime& dtRepeatEndDate);
bool GetRepeatDate_Weekly(COleDateTime dtStart,int interval,int nCount ,COleDateTime& dtRepeatEndDate,int DayofWeekMask);
bool GetRepeatDate_Monthly(COleDateTime dtStart,int interval,int nCount ,COleDateTime& dtRepeatEndDate);
bool GetRepeatDate_MonthNth(COleDateTime dtStart,int interval,int nCount ,COleDateTime& dtRepeatEndDate,int Week,int Day);

void GetwhichWeekDay(GSM_DateTime gsmtime, UINT &whichWeek, UINT &whichDay);

#endif
GSM_Error SF_DecodeVCALENDAR_VTODO(unsigned char *Buffer, int *Pos, GSM_CalendarEntry *Calendar, GSM_ToDoEntry *ToDo, int Timezone);

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

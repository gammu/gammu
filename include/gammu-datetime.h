/**
 * \file gammu-datetime.h
 * \author Michal Čihař
 * 
 * Date and time manipulations.
 */
#ifndef __gammu_datetime_h
#define __gammu_datetime_h

/**
 * \defgroup DateTime Date and time
 * Date and time handling.
 */

#include <time.h>
#include <gammu-statemachine.h>

/**
 * Structure used for saving date and time
 */
typedef struct {
	/**
	 * The difference between local time and GMT in hours
	 */
	int			Timezone;

	unsigned int		Second;
	unsigned int 		Minute;
	unsigned int		Hour;

	unsigned int 		Day;
	/**
	 * January = 1, February = 2, etc.
	 */
	unsigned int 		Month;
	/**
	 * Complete year number. Not 03, but 2003
	 */
	unsigned int		Year;
} GSM_DateTime;

typedef struct {
	/* for relative times */
	int			Timezone;

	int			Second;
	int 			Minute;
	int			Hour;

	int 			Day;
	int 			Month;
	int			Year;
} GSM_DeltaTime;

void GSM_GetCurrentDateTime 	(GSM_DateTime *Date);
GSM_DateTime GSM_AddTime (GSM_DateTime DT , GSM_DeltaTime delta);
bool HeapCheck(char* loc);
char *OSDateTime 		(GSM_DateTime dt, bool TimeZone);
char *OSDate 			(GSM_DateTime dt);
int GetDayOfYear		(int year, int month, int day);
int GetWeekOfMonth		(int year, int month, int day);
int GetDayOfWeek		(int year, int month, int day);
char *DayOfWeek 		(int year, int month, int day);
time_t Fill_Time_T		(GSM_DateTime DT);
void GetTimeDifference		(unsigned long diff, GSM_DateTime *DT, bool Plus, int multi);
void Fill_GSM_DateTime		(GSM_DateTime *Date, time_t timet);
bool CheckDate			(GSM_DateTime *date);
bool CheckTime			(GSM_DateTime *date);
/**
 * Reads date and time from phone.
 */
GSM_Error GSM_GetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time);
/**
 * Sets date and time in phone.
 */
GSM_Error GSM_SetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time);
#endif

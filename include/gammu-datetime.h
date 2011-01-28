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
#include <gammu-error.h>
#include <gammu-statemachine.h>

/**
 * Structure used for saving date and time
 *
 * \ingroup DateTime
 */
typedef struct {
	/**
	 * The difference between local time and GMT in seconds
	 */
	int Timezone;

	/**
	 * Seconds.
	 */
	int Second;
	/**
	 * Minutes.
	 */
	int Minute;
	/**
	 * Hours.
	 */
	int Hour;

	/**
	 * Days.
	 */
	int Day;
	/**
	 * January = 1, February = 2, etc.
	 */
	int Month;
	/**
	 * Complete year number. Not 03, but 2003
	 */
	int Year;
} GSM_DateTime;

/**
 * Structure used for saving relative date and time
 *
 * \ingroup DateTime
 */
typedef struct {
	/**
	 * The difference of timezones in seconds
	 */
	int Timezone;

	/**
	 * Seconds diff.
	 */
	int Second;
	/**
	 * Minutes diff.
	 */
	int Minute;
	/**
	 * Hours diff.
	 */
	int Hour;

	/**
	 * Days diff.
	 */
	int Day;
	/**
	 * Months diff.
	 */
	int Month;
	/**
	 * Years diff.
	 */
	int Year;
} GSM_DeltaTime;

/**
 * Returns string for current day of week.
 *
 * \param year Year.
 * \param month Month.
 * \param day Day.
 *
 * \return Pointer to static buffer containing day of week string.
 *
 * \ingroup DateTime
 */
char *DayOfWeek(unsigned int year, unsigned int month, unsigned int day);

/**
 * Returns current timestamp.
 *
 * \param Date Storage for date time structure.
 *
 * \ingroup DateTime
 */
void GSM_GetCurrentDateTime(GSM_DateTime * Date);

/**
 * Converts \ref GSM_DateTime to time_t.
 *
 * \param DT Input timestamp.
 *
 * \return time_t value.
 *
 * \ingroup DateTime
 */
time_t Fill_Time_T(GSM_DateTime DT);

/**
 * Returns the local timezone offset in seconds.
 * For example 7200 for CEST.
 *
 * \return Timezone offset seconds.
 *
 * \ingroup DateTime
 */
int GSM_GetLocalTimezoneOffset(void);

/**
 * Converts time_t to gammu \ref GSM_DateTime structure.
 *
 * \param Date Storage for date.
 * \param timet Input date.
 *
 * \ingroup DateTime
 */
void Fill_GSM_DateTime(GSM_DateTime * Date, time_t timet);

/**
 * Converts string (seconds since epoch) to gammu \ref GSM_DateTime structure.
 *
 * \param Date Storage for date.
 * \param str Input date.
 *
 * \ingroup DateTime
 */
void GSM_DateTimeFromTimestamp(GSM_DateTime *Date, const char *str);

/**
 * Converts gammu \ref GSM_DateTime structure to string (seconds since epoch).
 *
 * \param Date Date.
 * \param str Strorage for string.
 *
 * \ingroup DateTime
 */
void GSM_DateTimeToTimestamp(GSM_DateTime *Date, char *str);

/**
 * Converts timestamp to string according to OS settings.
 *
 * \param dt Input timestamp.
 * \param TimeZone Whether to include time zone.
 *
 * \return Pointer to static buffer containing string.
 *
 * \ingroup DateTime
 */
char *OSDateTime(GSM_DateTime dt, gboolean TimeZone);

/**
 * Converts date from timestamp to string according to OS settings.
 *
 * \param dt Input timestamp.
 *
 * \return Pointer to static buffer containing string.
 *
 * \ingroup DateTime
 */
char *OSDate(GSM_DateTime dt);

/**
 * Checks whether date is valid. This does not check time, see
 * \ref CheckTime for this.
 *
 * \param date Structure where to check date.
 *
 * \return True if date is correct.
 *
 * \ingroup DateTime
 */
gboolean CheckDate(GSM_DateTime * date);

/**
 * Checks whether time is valid. This does not check date, see
 * \ref CheckDate for this.
 *
 * \param date Structure where to check time.
 *
 * \return True if time is correct.
 *
 * \ingroup DateTime
 */
gboolean CheckTime(GSM_DateTime * date);

/**
 * Reads date and time from phone.
 *
 * \param s State machine pointer.
 * \param date_time Storage for date.
 *
 * \return Error code
 *
 * \ingroup Category
 */
GSM_Error GSM_GetDateTime(GSM_StateMachine * s, GSM_DateTime * date_time);

/**
 * Sets date and time in phone.
 *
 * \param s State machine pointer.
 * \param date_time Date to set.
 *
 * \return Error code
 *
 * \ingroup Category
 */
GSM_Error GSM_SetDateTime(GSM_StateMachine * s, GSM_DateTime * date_time);
#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

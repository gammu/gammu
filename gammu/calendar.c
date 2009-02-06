#include "../helper/locales.h"

#define _GNU_SOURCE		/* For strcasestr */
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <stdlib.h>
#include <ctype.h>
#include <gammu.h>

#include "common.h"
#include "calendar.h"

#include "../helper/formats.h"

void PrintCalendar(GSM_CalendarEntry * Note)
{
	GSM_Error error;
	int i_age = 0, i;
	GSM_DateTime Alarm, DateTime;
	GSM_MemoryEntry entry;
	unsigned char *name;

	bool repeating = false;
	int repeat_dayofweek = -1;
	int repeat_day = -1;
	int repeat_dayofyear = -1;
	int repeat_weekofmonth = -1;
	int repeat_month = -1;
	int repeat_count = -1;
	int repeat_frequency = -1;
	GSM_DateTime repeat_startdate = { 0, 0, 0, 0, 0, 0, 0 };
	GSM_DateTime repeat_stopdate = { 0, 0, 0, 0, 0, 0, 0 };

	printf(LISTFORMAT, _("Note type"));
	switch (Note->Type) {
		case GSM_CAL_REMINDER:
			printf("%s\n", _("Reminder (Date)"));
			break;
		case GSM_CAL_CALL:
			printf("%s\n", _("Call"));
			break;
		case GSM_CAL_MEETING:
			printf("%s\n", _("Meeting"));
			break;
		case GSM_CAL_BIRTHDAY:
			printf("%s\n", _("Birthday (Anniversary)"));
			break;
		case GSM_CAL_MEMO:
			printf("%s\n", _("Memo (Miscellaneous)"));
			break;
		case GSM_CAL_TRAVEL:
			printf("%s\n", _("Travel"));
			break;
		case GSM_CAL_VACATION:
			printf("%s\n", _("Vacation"));
			break;
		case GSM_CAL_ALARM:
			printf("%s\n", _("Alarm"));
			break;
		case GSM_CAL_DAILY_ALARM:
			printf("%s\n", _("Daily alarm"));
			break;
		case GSM_CAL_T_ATHL:
			printf("%s\n", _("Training/Athletism"));
			break;
		case GSM_CAL_T_BALL:
			printf("%s\n", _("Training/Ball Games"));
			break;
		case GSM_CAL_T_CYCL:
			printf("%s\n", _("Training/Cycling"));
			break;
		case GSM_CAL_T_BUDO:
			printf("%s\n", _("Training/Budo"));
			break;
		case GSM_CAL_T_DANC:
			printf("%s\n", _("Training/Dance"));
			break;
		case GSM_CAL_T_EXTR:
			printf("%s\n", _("Training/Extreme Sports"));
			break;
		case GSM_CAL_T_FOOT:
			printf("%s\n", _("Training/Football"));
			break;
		case GSM_CAL_T_GOLF:
			printf("%s\n", _("Training/Golf"));
			break;
		case GSM_CAL_T_GYM:
			printf("%s\n", _("Training/Gym"));
			break;
		case GSM_CAL_T_HORS:
			printf("%s\n", _("Training/Horse Races"));
			break;
		case GSM_CAL_T_HOCK:
			printf("%s\n", _("Training/Hockey"));
			break;
		case GSM_CAL_T_RACE:
			printf("%s\n", _("Training/Races"));
			break;
		case GSM_CAL_T_RUGB:
			printf("%s\n", _("Training/Rugby"));
			break;
		case GSM_CAL_T_SAIL:
			printf("%s\n", _("Training/Sailing"));
			break;
		case GSM_CAL_T_STRE:
			printf("%s\n", _("Training/Street Games"));
			break;
		case GSM_CAL_T_SWIM:
			printf("%s\n", _("Training/Swimming"));
			break;
		case GSM_CAL_T_TENN:
			printf("%s\n", _("Training/Tennis"));
			break;
		case GSM_CAL_T_TRAV:
			printf("%s\n", _("Training/Travels"));
			break;
		case GSM_CAL_T_WINT:
			printf("%s\n", _("Training/Winter Games"));
			break;
#ifndef CHECK_CASES
		default:
			printf("%s\n", _("unknown type!"));
#endif
	}
	Alarm.Year = 0;

	repeating = false;
	repeat_dayofweek = -1;
	repeat_day = -1;
	repeat_dayofyear = -1;
	repeat_weekofmonth = -1;
	repeat_month = -1;
	repeat_frequency = -1;
	repeat_startdate.Day = 0;
	repeat_stopdate.Day = 0;

	for (i = 0; i < Note->EntriesNum; i++) {
		switch (Note->Entries[i].EntryType) {
			case CAL_START_DATETIME:
				printf(LISTFORMAT "%s\n", _("Start"),
				       OSDateTime(Note->Entries[i].Date,
						  false));
				memcpy(&DateTime, &Note->Entries[i].Date,
				       sizeof(GSM_DateTime));
				break;
			case CAL_END_DATETIME:
				printf(LISTFORMAT "%s\n", _("Stop"),
				       OSDateTime(Note->Entries[i].Date,
						  false));
				memcpy(&DateTime, &Note->Entries[i].Date,
				       sizeof(GSM_DateTime));
				break;
			case CAL_LAST_MODIFIED:
				printf(LISTFORMAT "%s\n", _("Last modified"),
				       OSDateTime(Note->Entries[i].Date,
						  false));
				break;
			case CAL_TONE_ALARM_DATETIME:
				if (Note->Type == GSM_CAL_BIRTHDAY) {
					printf(LISTFORMAT, _("Tone alarm"));
					printf(_
					       ("forever on each %i. day of %s"),
					       Note->Entries[i].Date.Day,
					       GetMonthName(Note->Entries[i].
							    Date.Month));
					printf(" %02i:%02i:%02i\n",
					       Note->Entries[i].Date.Hour,
					       Note->Entries[i].Date.Minute,
					       Note->Entries[i].Date.Second);
				} else {
					printf(LISTFORMAT "%s\n",
					       _("Tone alarm"),
					       OSDateTime(Note->Entries[i].Date,
							  false));
				}
				memcpy(&Alarm, &Note->Entries[i].Date,
				       sizeof(GSM_DateTime));
				break;
			case CAL_SILENT_ALARM_DATETIME:
				if (Note->Type == GSM_CAL_BIRTHDAY) {
					printf(LISTFORMAT, _("Silent alarm"));
					printf(_
					       ("forever on each %i. day of %s"),
					       Note->Entries[i].Date.Day,
					       GetMonthName(Note->Entries[i].
							    Date.Month));
					printf(" %02i:%02i:%02i\n",
					       Note->Entries[i].Date.Hour,
					       Note->Entries[i].Date.Minute,
					       Note->Entries[i].Date.Second);
				} else {
					printf(LISTFORMAT "%s\n",
					       _("Silent alarm"),
					       OSDateTime(Note->Entries[i].Date,
							  false));
				}
				memcpy(&Alarm, &Note->Entries[i].Date,
				       sizeof(GSM_DateTime));
				break;
			case CAL_TEXT:
				printf(LISTFORMAT "\"%s\"\n", _("Text"),
				       DecodeUnicodeConsole(Note->Entries[i].
							    Text));
				break;
			case CAL_DESCRIPTION:
				printf(LISTFORMAT "\"%s\"\n", _("Description"),
				       DecodeUnicodeConsole(Note->Entries[i].
							    Text));
				break;
			case CAL_LUID:
				printf(LISTFORMAT "\"%s\"\n", _("LUID"),
				       DecodeUnicodeConsole(Note->Entries[i].
							    Text));
				break;
			case CAL_LOCATION:
				printf(LISTFORMAT "\"%s\"\n", _("Location"),
				       DecodeUnicodeConsole(Note->Entries[i].
							    Text));
				break;
			case CAL_PHONE:
				printf(LISTFORMAT "\"%s\"\n", _("Phone"),
				       DecodeUnicodeConsole(Note->Entries[i].
							    Text));
				break;
			case CAL_PRIVATE:
				printf(LISTFORMAT "%s\n", _("Private"),
				       Note->Entries[i].Number ==
				       1 ? _("Yes") : _("No"));
				break;
			case CAL_CONTACTID:
				entry.Location = Note->Entries[i].Number;
				entry.MemoryType = MEM_ME;
				error = GSM_GetMemory(gsm, &entry);
				if (error == ERR_NONE) {
					name =
					    GSM_PhonebookGetEntryName(&entry);
					if (name != NULL) {
						printf(LISTFORMAT
						       "\"%s\" (%d)\n",
						       _("Contact ID"),
						       DecodeUnicodeConsole
						       (name),
						       Note->Entries[i].Number);
					} else {
						printf(LISTFORMAT "%d\n",
						       _("Contact ID"),
						       Note->Entries[i].Number);
					}
				} else {
					printf(LISTFORMAT "%d\n",
					       _("Contact ID"),
					       Note->Entries[i].Number);
				}
				break;
			case CAL_REPEAT_DAYOFWEEK:
				repeat_dayofweek = Note->Entries[i].Number;
				repeating = true;
				break;
			case CAL_REPEAT_DAY:
				repeat_day = Note->Entries[i].Number;
				repeating = true;
				break;
			case CAL_REPEAT_DAYOFYEAR:
				repeat_dayofyear = Note->Entries[i].Number;
				repeating = true;
				break;
			case CAL_REPEAT_WEEKOFMONTH:
				repeat_weekofmonth = Note->Entries[i].Number;
				repeating = true;
				break;
			case CAL_REPEAT_MONTH:
				repeat_month = Note->Entries[i].Number;
				repeating = true;
				break;
			case CAL_REPEAT_FREQUENCY:
				repeat_frequency = Note->Entries[i].Number;
				repeating = true;
				break;
			case CAL_REPEAT_COUNT:
				repeat_count = Note->Entries[i].Number;
				repeating = true;
				break;
			case CAL_REPEAT_STARTDATE:
				repeat_startdate = Note->Entries[i].Date;
				repeating = true;
				break;
			case CAL_REPEAT_STOPDATE:
				repeat_stopdate = Note->Entries[i].Date;
				repeating = true;
				break;
		}
	}
	if (repeating) {
		printf(LISTFORMAT, _("Repeating"));
		if (repeat_count > 0) {
			printf(ngettext
			       ("for %d time ", "for %d times ", repeat_count),
			       repeat_count);
		}
		if ((repeat_startdate.Day == 0) && (repeat_stopdate.Day == 0)) {
			printf(_("forever"));
		} else if (repeat_startdate.Day == 0) {
			printf(_("till %s"), OSDate(repeat_stopdate));
		} else if (repeat_stopdate.Day == 0) {
			printf(_("since %s"), OSDate(repeat_startdate));
		} else {
			printf(_("since %s till %s"), OSDate(repeat_startdate),
			       OSDate(repeat_stopdate));
		}
		if (repeat_frequency != -1) {
			if (repeat_frequency == 1) {
				printf(_(" on each "));
			} else {
				printf(_(" on each %d. "), repeat_frequency);
			}
			if (repeat_dayofweek > 0) {
				printf("%s", GetDayName(repeat_dayofweek));
				if (repeat_weekofmonth > 0) {
					printf(_(" in %d. week of "),
					       repeat_weekofmonth);
				} else {
					printf(_(" in "));
				}
				if (repeat_month > 0) {
					printf("%s",
					       GetMonthName(repeat_month));
				} else {
					printf(_("each month"));
				}
			} else if (repeat_day > 0) {
				printf(_("%d. day of "), repeat_day);
				if (repeat_month > 0) {
					printf("%s",
					       GetMonthName(repeat_month));
				} else {
					printf(_("each month"));
				}
			} else if (repeat_dayofyear > 0) {
				printf(_("%d. day of year"), repeat_dayofyear);
			} else {
				printf(_("day"));
			}
		}
		printf("\n");
	}
	if (Note->Type == GSM_CAL_BIRTHDAY) {
		if (Alarm.Year == 0x00)
			GSM_GetCurrentDateTime(&Alarm);
		if (DateTime.Year != 0) {
			i_age = Alarm.Year - DateTime.Year;
			if (DateTime.Month < Alarm.Month)
				i_age++;
			if (DateTime.Month == Alarm.Month &&
			    DateTime.Day < Alarm.Day) {
				i_age++;
			}
			printf(LISTFORMAT, _("Age"));
			PRINTYEARS(i_age);
			printf("\n");
		}
	}
	printf("\n");
}

void GetDateTime(int argc UNUSED, char *argv[]UNUSED)
{
	GSM_Error error;
	GSM_DateTime date_time;
	GSM_Locale locale;

	GSM_Init(true);

	error = GSM_GetDateTime(gsm, &date_time);
	switch (error) {
		case ERR_EMPTY:
			printf("%s\n", _("Date and time not set in phone"));
			break;
		case ERR_NONE:
			printf(_("Phone time is %s\n"),
			       OSDateTime(date_time, false));
			break;
		default:
			Print_Error(error);
	}

	error = GSM_GetLocale(gsm, &locale);
	switch (error) {
		case ERR_NOTSUPPORTED:
		case ERR_NOTIMPLEMENTED:
			break;
		default:
			Print_Error(error);
			printf(_("Time format is "));
			if (locale.AMPMTime) {
				/* l10n: AM/PM time */
				printf("%s\n", _("12 hours"));
			} else {
				/* l10n: 24 hours time */
				printf("%s\n", _("24 hours"));
			}
			printf(_("Date format is "));
			switch (locale.DateFormat) {
				case GSM_Date_DDMMYYYY:
					printf(_("DD MM YYYY"));
					break;
				case GSM_Date_MMDDYYYY:
					printf(_("MM DD YYYY"));
					break;
				case GSM_Date_YYYYMMDD:
					printf(_("YYYY MM DD"));
					break;
				case GSM_Date_DDMMMYY:
					printf(_("DD MMM YY"));
					break;
				case GSM_Date_MMDDYY:
					printf(_("MM DD YY"));
					break;
				case GSM_Date_DDMMYY:
					printf(_("DD MM YY"));
					break;
				case GSM_Date_YYMMDD:
					printf(_("YY MM DD"));
					break;
				case GSM_Date_OFF:
					printf(_("OFF"));
					break;
				default:
					break;
			}
			printf(_(", date separator is %c\n"),
			       locale.DateSeparator);
	}

	GSM_Terminate();
}

void SetDateTime(int argc, char *argv[])
{
	GSM_DateTime date_time;
	char shift, *parse;
	GSM_Error error;

	GSM_Init(true);
	error = ERR_NONE;
	if (argc < 3) {
		/* set datetime to the current datetime in the PC */
		printf("%s\n", _("Setting time in phone to the time on PC."));
		GSM_GetCurrentDateTime(&date_time);
	} else {
		/* update only parts the user specified,
		   leave the rest in the phone as is */
		error = GSM_GetDateTime(gsm, &date_time);
		Print_Error(error);

		if (error == ERR_NONE) {
			printf("%s\n",
			       _
			       ("Updating specified parts of date and time in phone."));
			shift = 0;
			parse = strchr(argv[2], ':');
			if (parse != NULL) {
				date_time.Hour = atoi(argv[2]);
				date_time.Minute = atoi(parse + 1);
				parse = strchr(parse + 1, ':');
				if (parse != NULL) {
					date_time.Second = atoi(parse + 1);
				}
				shift = 1;
			}
			if (argc - 1 >= 2 + shift) {
				parse = strchr(argv[2 + shift], '/');
				if (parse != NULL) {
					date_time.Year = atoi(argv[2 + shift]);
					date_time.Month = atoi(parse + 1);
					parse = strchr(parse + 1, '/');
					if (parse != NULL) {
						date_time.Day = atoi(parse + 1);
					}
				}
			}
			if (!CheckDate(&date_time) || !CheckTime(&date_time))
				error = ERR_INVALIDDATETIME;
			/* we got the timezone from the phone */
		}
	}
	if (error == ERR_NONE) {
		error = GSM_SetDateTime(gsm, &date_time);
	}
	Print_Error(error);

	GSM_Terminate();
}

void GetAlarm(int argc, char *argv[])
{
	GSM_Alarm Alarm;
	GSM_Error error;

	GSM_Init(true);

	if (argc < 3) {
		Alarm.Location = 1;
	} else {
		Alarm.Location = atoi(argv[2]);
	}
	error = GSM_GetAlarm(gsm, &Alarm);
	switch (error) {
		case ERR_EMPTY:
			printf(_("Alarm (%i) not set in phone\n"),
			       Alarm.Location);
			break;
		case ERR_NONE:
			printf(_("Alarm in location %i:\n"), Alarm.Location);
			if (Alarm.Repeating) {
				printf(LISTFORMAT "%s\n", _("Date"),
				       _("Every day"));
			} else if (Alarm.DateTime.Day != 0) {
				printf(LISTFORMAT "%s\n", _("Date"),
				       OSDate(Alarm.DateTime));
			}
			printf(_("Time: %02d:%02d\n"), Alarm.DateTime.Hour,
			       Alarm.DateTime.Minute);
			if (Alarm.Text[0] != 0 || Alarm.Text[1] != 0) {
				printf(LISTFORMAT "\"%s\"\n", _("Text"),
				       DecodeUnicodeConsole(Alarm.Text));
			}
			break;
		default:
			Print_Error(error);
	}

	GSM_Terminate();
}

void SetAlarm(int argc UNUSED, char *argv[])
{
	GSM_Alarm Alarm;
	GSM_Error error;

	Alarm.DateTime.Hour = atoi(argv[2]);
	Alarm.DateTime.Minute = atoi(argv[3]);
	Alarm.DateTime.Second = 0;
	Alarm.Location = 1;
	Alarm.Repeating = true;
	Alarm.Text[0] = 0;
	Alarm.Text[1] = 0;

	GSM_Init(true);

	error = GSM_SetAlarm(gsm, &Alarm);
	Print_Error(error);

	GSM_Terminate();
}

void GetCalendar(int argc UNUSED, char *argv[])
{
	GSM_Error error;
	GSM_CalendarEntry Note;
	int start, stop, i;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i = start; i <= stop; i++) {
		Note.Location = i;
		error = GSM_GetCalendar(gsm, &Note);
		if (error == ERR_EMPTY)
			continue;
		Print_Error(error);
		printf(LISTFORMAT "%d\n", _("Location"), Note.Location);
		PrintCalendar(&Note);
	}

	GSM_Terminate();
}

void DeleteCalendar(int argc, char *argv[])
{
	GSM_Error error;
	GSM_CalendarEntry Note;
	int start, stop, i;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i = start; i <= stop; i++) {
		Note.Location = i;
		error = GSM_DeleteCalendar(gsm, &Note);
		Print_Error(error);
	}

	GSM_Terminate();
}

void GetAllCalendar(int argc UNUSED, char *argv[]UNUSED)
{
	GSM_Error error;
	GSM_CalendarEntry Note;
	bool refresh = true;

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));

	GSM_Init(true);

	Note.Location = 0;

	while (!gshutdown) {
		error = GSM_GetNextCalendar(gsm, &Note, refresh);
		if (error == ERR_EMPTY)
			break;
		Print_Error(error);
		printf(LISTFORMAT "%d\n", _("Location"), Note.Location);
		PrintCalendar(&Note);
		refresh = false;
	}

	GSM_Terminate();
}

void GetCalendarSettings(int argc UNUSED, char *argv[]UNUSED)
{
	GSM_Error error;
	GSM_CalendarSettings settings;

	GSM_Init(true);

	error = GSM_GetCalendarSettings(gsm, &settings);
	Print_Error(error);

	if (settings.AutoDelete == 0) {
		printf(_("Auto deleting disabled"));
	} else {
		printf(_("Auto deleting notes after %i day(s)"),
		       settings.AutoDelete);
	}
	printf("\n");
	printf(_("Week starts on %s"), GetDayName(settings.StartDay));
	printf("\n");

	GSM_Terminate();
}

void DeleteToDo(int argc, char *argv[])
{
	GSM_Error error;
	GSM_ToDoEntry ToDo;
	int i;
	int start, stop;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i = start; i <= stop; i++) {
		ToDo.Location = i;
		printf(LISTFORMAT "%i\n", _("Location"), i);
		error = GSM_DeleteToDo(gsm, &ToDo);
		if (error != ERR_EMPTY)
			Print_Error(error);

		if (error == ERR_EMPTY) {
			printf("%s\n", _("Entry was empty"));
		} else {
			printf("%s\n", _("Entry was deleted"));
		}
		printf("\n");
	}

	GSM_Terminate();
}

void PrintToDo(GSM_ToDoEntry * ToDo)
{
	GSM_Error error;
	int j;
	GSM_MemoryEntry entry;
	unsigned char *name;
	GSM_Category Category;

	printf(LISTFORMAT "%i\n", _("Location"), ToDo->Location);
	printf(LISTFORMAT, _("Note type"));
	switch (ToDo->Type) {
		case GSM_CAL_REMINDER:
			printf("%s\n", _("Reminder (Date)"));
			break;
		case GSM_CAL_CALL:
			printf("%s\n", _("Call"));
			break;
		case GSM_CAL_MEETING:
			printf("%s\n", _("Meeting"));
			break;
		case GSM_CAL_BIRTHDAY:
			printf("%s\n", _("Birthday (Anniversary)"));
			break;
		case GSM_CAL_MEMO:
			printf("%s\n", _("Memo (Miscellaneous)"));
			break;
		case GSM_CAL_TRAVEL:
			printf("%s\n", _("Travel"));
			break;
		case GSM_CAL_VACATION:
			printf("%s\n", _("Vacation"));
			break;
		case GSM_CAL_ALARM:
			printf("%s\n", _("Alarm"));
			break;
		case GSM_CAL_DAILY_ALARM:
			printf("%s\n", _("Daily alarm"));
			break;
		case GSM_CAL_T_ATHL:
			printf("%s\n", _("Training/Athletism"));
			break;
		case GSM_CAL_T_BALL:
			printf("%s\n", _("Training/Ball Games"));
			break;
		case GSM_CAL_T_CYCL:
			printf("%s\n", _("Training/Cycling"));
			break;
		case GSM_CAL_T_BUDO:
			printf("%s\n", _("Training/Budo"));
			break;
		case GSM_CAL_T_DANC:
			printf("%s\n", _("Training/Dance"));
			break;
		case GSM_CAL_T_EXTR:
			printf("%s\n", _("Training/Extreme Sports"));
			break;
		case GSM_CAL_T_FOOT:
			printf("%s\n", _("Training/Football"));
			break;
		case GSM_CAL_T_GOLF:
			printf("%s\n", _("Training/Golf"));
			break;
		case GSM_CAL_T_GYM:
			printf("%s\n", _("Training/Gym"));
			break;
		case GSM_CAL_T_HORS:
			printf("%s\n", _("Training/Horse Races"));
			break;
		case GSM_CAL_T_HOCK:
			printf("%s\n", _("Training/Hockey"));
			break;
		case GSM_CAL_T_RACE:
			printf("%s\n", _("Training/Races"));
			break;
		case GSM_CAL_T_RUGB:
			printf("%s\n", _("Training/Rugby"));
			break;
		case GSM_CAL_T_SAIL:
			printf("%s\n", _("Training/Sailing"));
			break;
		case GSM_CAL_T_STRE:
			printf("%s\n", _("Training/Street Games"));
			break;
		case GSM_CAL_T_SWIM:
			printf("%s\n", _("Training/Swimming"));
			break;
		case GSM_CAL_T_TENN:
			printf("%s\n", _("Training/Tennis"));
			break;
		case GSM_CAL_T_TRAV:
			printf("%s\n", _("Training/Travels"));
			break;
		case GSM_CAL_T_WINT:
			printf("%s\n", _("Training/Winter Games"));
			break;
#ifndef CHECK_CASES
		default:
			printf("%s\n", _("Unknown"));
#endif
	}
	printf(LISTFORMAT, _("Priority"));
	switch (ToDo->Priority) {
		case GSM_Priority_Low:
			printf("%s\n", _("Low"));
			break;
		case GSM_Priority_Medium:
			printf("%s\n", _("Medium"));
			break;
		case GSM_Priority_High:
			printf("%s\n", _("High"));
			break;
		case GSM_Priority_None:
			printf("%s\n", _("None"));
			break;
#ifndef CHECK_CASES
		default:
			printf("%s\n", _("Unknown"));
			break;
#endif
	}
	for (j = 0; j < ToDo->EntriesNum; j++) {
		switch (ToDo->Entries[j].EntryType) {
			case TODO_END_DATETIME:
				printf(LISTFORMAT "%s\n", _("Due time"),
				       OSDateTime(ToDo->Entries[j].Date,
						  false));
				break;
			case TODO_START_DATETIME:
				printf(LISTFORMAT "%s\n", _("Start time"),
				       OSDateTime(ToDo->Entries[j].Date,
						  false));
				break;
			case TODO_COMPLETED:
				printf(LISTFORMAT "%s\n", _("Completed"),
				       ToDo->Entries[j].Number ==
				       1 ? "Yes" : "No");
				break;
			case TODO_ALARM_DATETIME:
				printf(LISTFORMAT "%s\n", _("Alarm"),
				       OSDateTime(ToDo->Entries[j].Date,
						  false));
				break;
			case TODO_SILENT_ALARM_DATETIME:
				printf(LISTFORMAT "%s\n", _("Silent alarm"),
				       OSDateTime(ToDo->Entries[j].Date,
						  false));
				break;
			case TODO_LAST_MODIFIED:
				printf(LISTFORMAT "%s\n", _("Last modified"),
				       OSDateTime(ToDo->Entries[j].Date,
						  false));
				break;
			case TODO_TEXT:
				printf(LISTFORMAT "\"%s\"\n", _("Text"),
				       DecodeUnicodeConsole(ToDo->Entries[j].
							    Text));
				break;
			case TODO_PRIVATE:
				printf(LISTFORMAT "%s\n", _("Private"),
				       ToDo->Entries[j].Number ==
				       1 ? "Yes" : "No");
				break;
			case TODO_CATEGORY:
				Category.Location = ToDo->Entries[j].Number;
				Category.Type = Category_ToDo;
				error = GSM_GetCategory(gsm, &Category);
				if (error == ERR_NONE) {
					printf(LISTFORMAT "\"%s\" (%i)\n",
					       _("Category"),
					       DecodeUnicodeConsole(Category.
								    Name),
					       ToDo->Entries[j].Number);
				} else {
					printf(LISTFORMAT "%i\n", _("Category"),
					       ToDo->Entries[j].Number);
				}
				break;
			case TODO_CONTACTID:
				entry.Location = ToDo->Entries[j].Number;
				entry.MemoryType = MEM_ME;
				error = GSM_GetMemory(gsm, &entry);
				if (error == ERR_NONE) {
					name =
					    GSM_PhonebookGetEntryName(&entry);
					if (name != NULL) {
						printf(LISTFORMAT
						       "\"%s\" (%d)\n",
						       _("Contact ID"),
						       DecodeUnicodeConsole
						       (name),
						       ToDo->Entries[j].Number);
					} else {
						printf(LISTFORMAT "%d\n",
						       _("Contact ID"),
						       ToDo->Entries[j].Number);
					}
				} else {
					printf(LISTFORMAT "%d\n", _("Contact"),
					       ToDo->Entries[j].Number);
				}
				break;
			case TODO_PHONE:
				printf(LISTFORMAT "\"%s\"\n", _("Phone"),
				       DecodeUnicodeConsole(ToDo->Entries[j].
							    Text));
				break;
			case TODO_DESCRIPTION:
				printf(LISTFORMAT "\"%s\"\n", _("Description"),
				       DecodeUnicodeConsole(ToDo->Entries[j].
							    Text));
				break;
			case TODO_LOCATION:
				printf(LISTFORMAT "\"%s\"\n", _("Location"),
				       DecodeUnicodeConsole(ToDo->Entries[j].
							    Text));
				break;
			case TODO_LUID:
				printf(LISTFORMAT "\"%s\"\n", _("LUID"),
				       DecodeUnicodeConsole(ToDo->Entries[j].
							    Text));
				break;
		}
	}
	printf("\n");
}

void ListToDoCategoryEntries(int Category)
{
	GSM_Error error;
	GSM_ToDoEntry Entry;
	bool start = true;
	int j;

	Entry.Location = 0;

	while (!gshutdown) {
		error = GSM_GetNextToDo(gsm, &Entry, start);
		if (error == ERR_EMPTY)
			break;
		Print_Error(error);
		for (j = 0; j < Entry.EntriesNum; j++) {
			if (Entry.Entries[j].EntryType == TODO_CATEGORY
			    && Entry.Entries[j].Number ==
			    (unsigned int)Category)
				PrintToDo(&Entry);
		}
		start = false;
	}
}

void ListToDoCategory(int argc UNUSED, char *argv[])
{
	GSM_Error error;
	GSM_Category Category;
	GSM_CategoryStatus Status;
	int j, count;

	unsigned char Text[(GSM_MAX_CATEGORY_NAME_LENGTH + 1) * 2];
	int Length;
	bool Number = true;;

	GSM_Init(true);

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));

	Length = strlen(argv[2]);
	for (j = 0; j < Length; j++) {
		if (!isdigit(argv[2][j])) {
			Number = false;
			break;
		}
	}

	if (Number) {
		j = atoi(argv[2]);
		if (j > 0) {
			ListToDoCategoryEntries(j);
		}
	} else {
		if (Length > GSM_MAX_CATEGORY_NAME_LENGTH) {
			printf(_
			       ("Search text too long, truncating to %d chars!\n"),
			       GSM_MAX_CATEGORY_NAME_LENGTH);
			Length = GSM_MAX_CATEGORY_NAME_LENGTH;
		}
		EncodeUnicode(Text, argv[2], Length);

		Category.Type = Category_ToDo;
		Status.Type = Category_ToDo;

		if (GSM_GetCategoryStatus(gsm, &Status) == ERR_NONE) {
			for (count = 0, j = 1; count < Status.Used; j++) {
				Category.Location = j;
				error = GSM_GetCategory(gsm, &Category);

				if (error != ERR_EMPTY) {
					count++;
					if (mywstrstr(Category.Name, Text) !=
					    NULL) {
						ListToDoCategoryEntries(j);
					}
				}
			}
		}
	}
	GSM_Terminate();
}

void GetToDo(int argc, char *argv[])
{
	GSM_Error error;
	GSM_ToDoEntry ToDo;
	int i;
	int start, stop;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i = start; i <= stop; i++) {
		ToDo.Location = i;
		error = GSM_GetToDo(gsm, &ToDo);
		if (error == ERR_EMPTY)
			continue;
		Print_Error(error);
		PrintToDo(&ToDo);
	}

	GSM_Terminate();
}

void GetAllToDo(int argc UNUSED, char *argv[]UNUSED)
{
	GSM_Error error;
	GSM_ToDoEntry ToDo;
	bool start = true;

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));

	GSM_Init(true);

	ToDo.Location = 0;

	while (!gshutdown) {
		error = GSM_GetNextToDo(gsm, &ToDo, start);
		if (error == ERR_EMPTY)
			break;
		Print_Error(error);
		PrintToDo(&ToDo);
		start = false;
	}

	GSM_Terminate();
}

void GetAllNotes(int argc UNUSED, char *argv[]UNUSED)
{
	GSM_Error error;
	GSM_NoteEntry Note;
	bool start = true;

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));

	GSM_Init(true);

	Note.Location = 0;

	while (!gshutdown) {
		error = GSM_GetNextNote(gsm, &Note, start);
		if (error == ERR_EMPTY)
			break;
		Print_Error(error);
		printf(LISTFORMAT "\"%s\"\n", _("Text"),
		       DecodeUnicodeConsole(Note.Text));
		printf("\n");
		start = false;
	}
	GSM_Terminate();
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

/* (c) 2002-2006 by Marcin Wiacek and Michal Cihar */
/* FM stuff by Walek */

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <locale.h>
#include <signal.h>
#include <ctype.h>
#include <wchar.h>
#include <unistd.h>
#ifdef WIN32
#  include <windows.h>
#  include <process.h>
#  ifdef _MSC_VER
#    include <sys/utime.h>
#  else
#    include <utime.h>
#  endif
#else
#  include <utime.h>
#endif

#include <gammu.h>
#include <gammu-config.h>
#include "gammu.h"
#include "common.h"
#include "memory.h"
#include "message.h"
#include "search.h"
#include "nokia.h"
#include "backup.h"

#include "smsd/smsdcore.h"
#include "../common/misc/locales.h"
#ifdef DEBUG
#  include "sniff.h"
#endif
#ifdef GSM_ENABLE_NOKIA_DCT3
#  include "depend/nokia/dct3.h"
#  include "depend/nokia/dct3trac/wmx.h"
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
#  include "depend/nokia/dct4.h"
#endif
#ifdef GSM_ENABLE_ATGEN
#  include "depend/siemens/dsiemens.h"
#endif

#ifdef HAVE_SYS_IOCTL_H
#  include <sys/ioctl.h>
#endif

/**
 * Hides default case in switch, to allow checking whether all cases are handled.
 */
#undef CHECK_CASES

/**
 * Global state machine used in Gammu.
 */
static int			i;


#define ALL_MEMORY_TYPES "DC|MC|RC|ON|VM|SM|ME|MT|FD|SL"


void PrintNetworkInfo(GSM_NetworkInfo NetInfo)
{
	printf(LISTFORMAT, _("Network state"));
	switch (NetInfo.State) {
		case GSM_HomeNetwork		: printf("%s\n", _("home network")); 		 	break;
		case GSM_RoamingNetwork		: printf("%s\n", _("roaming network")); 	 	break;
		case GSM_RequestingNetwork	: printf("%s\n", _("requesting network")); 	 	break;
		case GSM_NoNetwork		: printf("%s\n", _("not logged into network")); 	break;
		case GSM_RegistrationDenied	: printf("%s\n", _("registration to network denied"));	break;
		case GSM_NetworkStatusUnknown	: printf("%s\n", _("unknown"));			break;
#ifndef CHECK_CASES
		default				: printf("%s\n", _("unknown"));
#endif
	}
	if (NetInfo.State == GSM_HomeNetwork || NetInfo.State == GSM_RoamingNetwork) {
		printf(LISTFORMAT, _("Network"));
		printf("%s (%s",
			NetInfo.NetworkCode,
			DecodeUnicodeConsole(GSM_GetNetworkName(NetInfo.NetworkCode)));
		printf(", %s)",
			DecodeUnicodeConsole(GSM_GetCountryName(NetInfo.NetworkCode)));
		printf(", LAC %s, CID %s\n",
			NetInfo.LAC,NetInfo.CID);
		if (NetInfo.NetworkName[0] != 0x00 || NetInfo.NetworkName[1] != 0x00) {
			printf(LISTFORMAT "\"%s\"\n",
				_("Name in phone"),
				DecodeUnicodeConsole(NetInfo.NetworkName));
		}
	}
}

void PrintCalendar(GSM_CalendarEntry *Note)
{
	int			i_age = 0,i;
	GSM_DateTime		Alarm,DateTime;
	GSM_MemoryEntry		entry;
	unsigned char		*name;

    	bool 			repeating 		= false;
    	int 			repeat_dayofweek 	= -1;
    	int 			repeat_day 		= -1;
    	int 			repeat_dayofyear	= -1;
    	int 			repeat_weekofmonth 	= -1;
    	int 			repeat_month 		= -1;
    	int 			repeat_count 		= -1;
    	int 			repeat_frequency 	= -1;
    	GSM_DateTime 		repeat_startdate 	= {0,0,0,0,0,0,0};
    	GSM_DateTime 		repeat_stopdate 	= {0,0,0,0,0,0,0};

	printf(LISTFORMAT, _("Note type"));
	switch (Note->Type) {
		case GSM_CAL_REMINDER 	: printf("%s\n", _("Reminder (Date)"));		break;
		case GSM_CAL_CALL     	: printf("%s\n", _("Call"));			   	break;
		case GSM_CAL_MEETING  	: printf("%s\n", _("Meeting"));		   	break;
		case GSM_CAL_BIRTHDAY 	: printf("%s\n", _("Birthday (Anniversary)"));		break;
		case GSM_CAL_MEMO	: printf("%s\n", _("Memo (Miscellaneous)"));		break;
		case GSM_CAL_TRAVEL	: printf("%s\n", _("Travel"));			   	break;
		case GSM_CAL_VACATION	: printf("%s\n", _("Vacation"));			break;
		case GSM_CAL_ALARM    	: printf("%s\n", _("Alarm"));		   		break;
		case GSM_CAL_DAILY_ALARM: printf("%s\n", _("Daily alarm"));		   	break;
		case GSM_CAL_T_ATHL   	: printf("%s\n", _("Training/Athletism")); 	   	break;
		case GSM_CAL_T_BALL   	: printf("%s\n", _("Training/Ball Games")); 	   	break;
		case GSM_CAL_T_CYCL   	: printf("%s\n", _("Training/Cycling")); 	   	break;
		case GSM_CAL_T_BUDO   	: printf("%s\n", _("Training/Budo")); 	   		break;
		case GSM_CAL_T_DANC   	: printf("%s\n", _("Training/Dance")); 	   	break;
		case GSM_CAL_T_EXTR   	: printf("%s\n", _("Training/Extreme Sports")); 	break;
		case GSM_CAL_T_FOOT   	: printf("%s\n", _("Training/Football")); 	   	break;
		case GSM_CAL_T_GOLF   	: printf("%s\n", _("Training/Golf")); 	   		break;
		case GSM_CAL_T_GYM    	: printf("%s\n", _("Training/Gym")); 	   		break;
		case GSM_CAL_T_HORS   	: printf("%s\n", _("Training/Horse Races"));    	break;
		case GSM_CAL_T_HOCK   	: printf("%s\n", _("Training/Hockey")); 	  	break;
		case GSM_CAL_T_RACE   	: printf("%s\n", _("Training/Races")); 	   	break;
		case GSM_CAL_T_RUGB   	: printf("%s\n", _("Training/Rugby")); 	   	break;
		case GSM_CAL_T_SAIL   	: printf("%s\n", _("Training/Sailing")); 	   	break;
		case GSM_CAL_T_STRE   	: printf("%s\n", _("Training/Street Games"));   	break;
		case GSM_CAL_T_SWIM   	: printf("%s\n", _("Training/Swimming")); 	   	break;
		case GSM_CAL_T_TENN   	: printf("%s\n", _("Training/Tennis")); 	   	break;
		case GSM_CAL_T_TRAV   	: printf("%s\n", _("Training/Travels"));        	break;
		case GSM_CAL_T_WINT   	: printf("%s\n", _("Training/Winter Games"));   	break;
#ifndef CHECK_CASES
		default           	: printf("%s\n", _("unknown type!"));
#endif
	}
	Alarm.Year = 0;

	repeating 		= false;
	repeat_dayofweek 	= -1;
	repeat_day 		= -1;
	repeat_dayofyear	= -1;
	repeat_weekofmonth 	= -1;
	repeat_month 		= -1;
	repeat_frequency 	= -1;
	repeat_startdate.Day	= 0;
	repeat_stopdate.Day 	= 0;

	for (i=0;i<Note->EntriesNum;i++) {
		switch (Note->Entries[i].EntryType) {
		case CAL_START_DATETIME:
			printf(LISTFORMAT "%s\n", _("Start"), OSDateTime(Note->Entries[i].Date,false));
			memcpy(&DateTime,&Note->Entries[i].Date,sizeof(GSM_DateTime));
			break;
		case CAL_END_DATETIME:
			printf(LISTFORMAT "%s\n", _("Stop"), OSDateTime(Note->Entries[i].Date,false));
			memcpy(&DateTime,&Note->Entries[i].Date,sizeof(GSM_DateTime));
			break;
		case CAL_LAST_MODIFIED:
			printf(LISTFORMAT "%s\n", _("Last modified"), OSDateTime(Note->Entries[i].Date,false));
			break;
		case CAL_TONE_ALARM_DATETIME:
			if (Note->Type==GSM_CAL_BIRTHDAY) {
				printf(LISTFORMAT, _("Tone alarm"));
				printf(_("forever on each %i. day of %s"),
					Note->Entries[i].Date.Day,
					GetMonthName(Note->Entries[i].Date.Month));
				printf(" %02i:%02i:%02i\n",
					Note->Entries[i].Date.Hour,
					Note->Entries[i].Date.Minute,
					Note->Entries[i].Date.Second);
			} else {
				printf(LISTFORMAT "%s\n", _("Tone alarm"), OSDateTime(Note->Entries[i].Date,false));
			}
			memcpy(&Alarm,&Note->Entries[i].Date,sizeof(GSM_DateTime));
			break;
		case CAL_SILENT_ALARM_DATETIME:
			if (Note->Type==GSM_CAL_BIRTHDAY) {
				printf(LISTFORMAT, _("Silent alarm"));
				printf(_("forever on each %i. day of %s"),
					Note->Entries[i].Date.Day,
					GetMonthName(Note->Entries[i].Date.Month));
				printf(" %02i:%02i:%02i\n",
					Note->Entries[i].Date.Hour,
					Note->Entries[i].Date.Minute,
					Note->Entries[i].Date.Second);
			} else {
				printf(LISTFORMAT "%s\n", _("Silent alarm"), OSDateTime(Note->Entries[i].Date,false));
			}
			memcpy(&Alarm,&Note->Entries[i].Date,sizeof(GSM_DateTime));
			break;
		case CAL_TEXT:
			printf(LISTFORMAT "\"%s\"\n", _("Text"),DecodeUnicodeConsole(Note->Entries[i].Text));
			break;
		case CAL_DESCRIPTION:
			printf(LISTFORMAT "\"%s\"\n", _("Description"),DecodeUnicodeConsole(Note->Entries[i].Text));
			break;
		case CAL_LUID:
			printf(LISTFORMAT "\"%s\"\n", _("LUID"),DecodeUnicodeConsole(Note->Entries[i].Text));
			break;
		case CAL_LOCATION:
			printf(LISTFORMAT "\"%s\"\n", _("Location"),DecodeUnicodeConsole(Note->Entries[i].Text));
			break;
		case CAL_PHONE:
			printf(LISTFORMAT "\"%s\"\n", _("Phone"),DecodeUnicodeConsole(Note->Entries[i].Text));
			break;
		case CAL_PRIVATE:
			printf(LISTFORMAT "%s\n", _("Private"),Note->Entries[i].Number == 1 ? _("Yes") : _("No"));
			break;
		case CAL_CONTACTID:
			entry.Location = Note->Entries[i].Number;
			entry.MemoryType = MEM_ME;
			error=GSM_GetMemory(s, &entry);
			if (error == ERR_NONE) {
				name = GSM_PhonebookGetEntryName(&entry);
				if (name != NULL) {
					printf(LISTFORMAT "\"%s\" (%d)\n", _("Contact ID"), DecodeUnicodeConsole(name), Note->Entries[i].Number);
				} else {
					printf(LISTFORMAT "%d\n", _("Contact ID"), Note->Entries[i].Number);
				}
			} else {
				printf(LISTFORMAT "%d\n", _("Contact ID"), Note->Entries[i].Number);
			}
			break;
		case CAL_REPEAT_DAYOFWEEK:
			repeat_dayofweek 	= Note->Entries[i].Number;
			repeating 		= true;
			break;
		case CAL_REPEAT_DAY:
			repeat_day 		= Note->Entries[i].Number;
			repeating 		= true;
			break;
		case CAL_REPEAT_DAYOFYEAR:
			repeat_dayofyear	= Note->Entries[i].Number;
			repeating 		= true;
			break;
		case CAL_REPEAT_WEEKOFMONTH:
			repeat_weekofmonth 	= Note->Entries[i].Number;
			repeating 		= true;
			break;
		case CAL_REPEAT_MONTH:
			repeat_month 		= Note->Entries[i].Number;
			repeating 		= true;
			break;
		case CAL_REPEAT_FREQUENCY:
			repeat_frequency 	= Note->Entries[i].Number;
			repeating 		= true;
			break;
		case CAL_REPEAT_COUNT:
			repeat_count	 	= Note->Entries[i].Number;
			repeating 		= true;
			break;
		case CAL_REPEAT_STARTDATE:
			repeat_startdate 	= Note->Entries[i].Date;
			repeating 		= true;
			break;
		case CAL_REPEAT_STOPDATE:
			repeat_stopdate 	= Note->Entries[i].Date;
			repeating 		= true;
			break;
		}
	}
	if (repeating) {
		printf(LISTFORMAT, _("Repeating"));
		if (repeat_count > 0) {
			printf(ngettext("for %d time ", "for %d times ", repeat_count), repeat_count);
		}
		if ((repeat_startdate.Day == 0) && (repeat_stopdate.Day == 0)) {
			printf(_("forever"));
		} else if (repeat_startdate.Day == 0) {
			printf(_("till %s"), OSDate(repeat_stopdate));
		} else if (repeat_stopdate.Day == 0) {
			printf(_("since %s"), OSDate(repeat_startdate));
		} else {
			printf(_("since %s till %s"), OSDate(repeat_startdate), OSDate(repeat_stopdate));
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
					printf(_(" in %d. week of "), repeat_weekofmonth);
				} else {
					printf(_(" in "));
				}
				if (repeat_month > 0) {
					printf("%s", GetMonthName(repeat_month));
				} else {
					printf(_("each month"));
				}
			} else if (repeat_day > 0) {
				printf(_("%d. day of "), repeat_day);
				if (repeat_month > 0) {
					printf("%s", GetMonthName(repeat_month));
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
		if (Alarm.Year == 0x00) GSM_GetCurrentDateTime (&Alarm);
		if (DateTime.Year != 0) {
			i_age = Alarm.Year - DateTime.Year;
			if (DateTime.Month < Alarm.Month) i_age++;
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

GSM_Error GSM_PlayRingtone(GSM_Ringtone ringtone)
{
	int 		i;
	bool 		first=true;
	GSM_Error 	error;

	signal(SIGINT, interrupt);
	printf("%s\n", _("Press Ctrl+C to break..."));

	for (i=0;i<ringtone.NoteTone.NrCommands;i++) {
		if (gshutdown) break;
		if (ringtone.NoteTone.Commands[i].Type != RING_NOTETONE) continue;
		error=PHONE_RTTLPlayOneNote(s,ringtone.NoteTone.Commands[i].Note,first);
		if (error!=ERR_NONE) return error;
		first = false;
	}

	/* Disables buzzer */
	return GSM_PlayTone(s,255*255,0,false);
}

static void PlayRingtone(int argc UNUSED, char *argv[])
{
	GSM_Ringtone ringtone,ringtone2;

	ringtone.Format	= 0;
	error=GSM_ReadRingtoneFile(argv[2],&ringtone);
	Print_Error(error);

	error=GSM_RingtoneConvert(&ringtone2,&ringtone,RING_NOTETONE);
	Print_Error(error);

	GSM_Init(true);

	error=GSM_PlayRingtone(ringtone2);
	Print_Error(error);

	GSM_Terminate();
}

static void Identify(int argc, char *argv[])
{
	char buffer[100];
	char date[100];
	double num;

	GSM_Init(true);

	error=GSM_GetManufacturer(s, buffer);
	Print_Error(error);
	printf(LISTFORMAT "%s\n", _("Manufacturer"), buffer);
	error=GSM_GetModel(s, buffer);
	Print_Error(error);
	printf(LISTFORMAT "%s (%s)\n", _("Model"),
			GSM_GetModelInfo(s)->model,
			buffer);

	error=GSM_GetFirmware(s, buffer, date, &num);
	Print_Error(error);
	printf(LISTFORMAT "%s", _("Firmware"), buffer);
	error=GSM_GetPPM(s, buffer);
	if (error != ERR_NOTSUPPORTED) {
		if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
		if (error == ERR_NONE) printf(" %s", buffer);
	}
	if (date[0] != 0) printf(" (%s)", date);
	printf("\n");

	error=GSM_GetHardware(s, buffer);
	if (error != ERR_NOTSUPPORTED) {
		if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
		if (error == ERR_NONE) printf(LISTFORMAT "%s\n", _("Hardware"),buffer);
	}

	error=GSM_GetIMEI(s, buffer);
	if (error != ERR_NOTSUPPORTED) {
		if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
		if (error == ERR_NONE) printf(LISTFORMAT "%s\n", _("IMEI"), buffer);

		error=GSM_GetOriginalIMEI(s, buffer);
		if (error != ERR_NOTSUPPORTED && error != ERR_SECURITYERROR) {
			if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
			if (error == ERR_NONE) printf(LISTFORMAT "%s\n", _("Original IMEI"), buffer);
		}
	}

	error=GSM_GetManufactureMonth(s, buffer);
	if (error != ERR_NOTSUPPORTED && error != ERR_SECURITYERROR) {
		if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
		if (error == ERR_NONE) printf(LISTFORMAT "%s\n", _("Manufactured"),buffer);
	}

	error=GSM_GetProductCode(s, buffer);
	if (error != ERR_NOTSUPPORTED) {
		if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
		if (error == ERR_NONE) printf(LISTFORMAT "%s\n", _("Product code"),buffer);
	}

	error=GSM_GetSIMIMSI(s, buffer);
	switch (error) {
		case ERR_SECURITYERROR:
		case ERR_NOTSUPPORTED:
		case ERR_NOTIMPLEMENTED:
			break;
		case ERR_NONE:
			printf(LISTFORMAT "%s\n", _("SIM IMSI"),buffer);
			break;
		default:
			Print_Error(error);
	}

#ifdef GSM_ENABLE_NOKIA_DCT3
	DCT3Info(argc, argv);
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
	DCT4Info(argc, argv);
#endif

	GSM_Terminate();
}

static void GetDateTime(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_DateTime 	date_time;
	GSM_Locale	locale;

	GSM_Init(true);

	error=GSM_GetDateTime(s, &date_time);
	switch (error) {
	case ERR_EMPTY:
		printf("%s\n", _("Date and time not set in phone"));
		break;
	case ERR_NONE:
		printf(_("Phone time is %s\n"),OSDateTime(date_time,false));
		break;
	default:
		Print_Error(error);
	}

	error=GSM_GetLocale(s, &locale);
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
			case GSM_Date_DDMMYYYY  :printf(_("DD MM YYYY"));break;
			case GSM_Date_MMDDYYYY  :printf(_("MM DD YYYY"));break;
			case GSM_Date_YYYYMMDD  :printf(_("YYYY MM DD"));break;
			case GSM_Date_DDMMMYY	:printf(_("DD MMM YY"));break;
			case GSM_Date_MMDDYY	:printf(_("MM DD YY"));break;
			case GSM_Date_DDMMYY	:printf(_("DD MM YY"));break;
			case GSM_Date_YYMMDD	:printf(_("YY MM DD"));break;
			case GSM_Date_OFF	:printf(_("OFF"));break;
			default			:break;
		}
		printf(_(", date separator is %c\n"),locale.DateSeparator);
	}

	GSM_Terminate();
}

static void SetDateTime(int argc, char *argv[])
{
	GSM_DateTime	date_time;
	char		shift,*parse;

	GSM_Init(true);
	error=ERR_NONE;
	if (argc<3) {
		/* set datetime to the current datetime in the PC */
		printf("%s\n", _("Setting time in phone to the time on PC."));
		GSM_GetCurrentDateTime(&date_time);
	} else {
		/* update only parts the user specified,
		leave the rest in the phone as is */
		error=GSM_GetDateTime(s, &date_time);
		Print_Error(error);

		if (error==ERR_NONE) {
			printf("%s\n", _("Updating specified parts of date and time in phone."));
			shift=0;
			parse=strchr(argv[2],':');
			if (parse!=NULL) {
				date_time.Hour=atoi(argv[2]);
                        	date_time.Minute=atoi(parse+1);
                        	parse=strchr(parse+1,':');
                        	if (parse!=NULL) {
                        		date_time.Second=atoi(parse+1);
                        	}
				shift=1;
			}
			if (argc-1>=2+shift) {
				parse=strchr(argv[2+shift],'/');
				if(parse!=NULL) {
					date_time.Year=atoi(argv[2+shift]);
                		        date_time.Month=atoi(parse+1);
                        	       	parse=strchr(parse+1,'/');
                               		if (parse!=NULL) {
                                  		date_time.Day=atoi(parse+1);
	                                }
				}
			}
			if (!CheckDate(&date_time) || !CheckTime(&date_time))
				error=ERR_INVALIDDATETIME;
			/* we got the timezone from the phone */
		}
	}
	if (error==ERR_NONE) {
		error=GSM_SetDateTime(s, &date_time);
	}
	Print_Error(error);

	GSM_Terminate();
}

static void GetAlarm(int argc, char *argv[])
{
	GSM_Alarm alarm;

	GSM_Init(true);

	if (argc<3) {
		alarm.Location = 1;
	} else {
		alarm.Location = atoi(argv[2]);
	}
	error=GSM_GetAlarm(s, &alarm);
	switch (error) {
	case ERR_EMPTY:
		printf(_("Alarm (%i) not set in phone\n"), alarm.Location);
		break;
	case ERR_NONE:
		printf(_("Alarm in location %i:\n"), alarm.Location);
		if (alarm.Repeating) {
			printf(LISTFORMAT "%s\n", _("Date"), _("Every day"));
		} else if (alarm.DateTime.Day!=0) {
			printf(LISTFORMAT "%s\n", _("Date"),OSDate(alarm.DateTime));
		}
		printf(_("Time: %02d:%02d\n"),alarm.DateTime.Hour, alarm.DateTime.Minute);
		if (alarm.Text[0] != 0 || alarm.Text[1] != 0) {
			printf(LISTFORMAT "\"%s\"\n", _("Text"), DecodeUnicodeConsole(alarm.Text));
		}
		break;
	default:
		Print_Error(error);
	}

	GSM_Terminate();
}

static void SetAlarm(int argc UNUSED, char *argv[])
{
	GSM_Alarm alarm;

	alarm.DateTime.Hour 	= atoi(argv[2]);
	alarm.DateTime.Minute 	= atoi(argv[3]);
	alarm.DateTime.Second 	= 0;
	alarm.Location		= 1;
	alarm.Repeating		= true;
	alarm.Text[0]		= 0;
	alarm.Text[1]		= 0;

	GSM_Init(true);

	error=GSM_SetAlarm(s, &alarm);
	Print_Error(error);

	GSM_Terminate();
}

static void NetworkInfo(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_NetworkInfo		NetInfo;

	GSM_Init(true);

	if (GSM_GetNetworkInfo(s,&NetInfo)==ERR_NONE) {
		PrintNetworkInfo(NetInfo);
	}
	GSM_Terminate();
}

static void IncomingCall(GSM_StateMachine *s UNUSED, GSM_Call call)
{
	printf(LISTFORMAT, _("Call info"));
	if (call.CallIDAvailable) printf(_("ID %i, "),call.CallID);
	switch(call.Status) {
		case GSM_CALL_IncomingCall  	: printf(_("incoming call from \"%s\"\n"),DecodeUnicodeConsole(call.PhoneNumber));  	break;
		case GSM_CALL_OutgoingCall  	: printf(_("outgoing call to \"%s\"\n"),DecodeUnicodeConsole(call.PhoneNumber));    	break;
		case GSM_CALL_CallStart     	: printf("%s\n", _("call started")); 					  	  		break;
		case GSM_CALL_CallEnd	   	: printf("%s\n", _("end of call (unknown side)")); 					  	break;
		case GSM_CALL_CallLocalEnd  	: printf("%s\n", _("call end from our side"));						  	break;
		case GSM_CALL_CallRemoteEnd 	: printf(_("call end from remote side (code %i)\n"),call.StatusCode);			break;
		case GSM_CALL_CallEstablished   : printf("%s\n", _("call established. Waiting for answer"));				  	break;
		case GSM_CALL_CallHeld		: printf("%s\n", _("call held"));								break;
		case GSM_CALL_CallResumed	: printf("%s\n", _("call resumed"));								break;
		case GSM_CALL_CallSwitched	: printf("%s\n", _("call switched"));								break;
	}
}

#define PRINTUSED(name, used, free) \
{ \
	printf(LISTFORMAT, name);	\
	printf(_("%3d used"), used); \
	printf(", "); \
	printf(_("%3d free"), free); \
	printf("\n"); \
}

#define CHECKMEMORYSTATUS(x, m, name) \
{ \
	x.MemoryType=m;							\
	if ( (error = GSM_GetMemoryStatus(s, &x)) == ERR_NONE)			\
		PRINTUSED(name, x.MemoryUsed, x.MemoryFree);  \
}

#define CHECK_EXIT \
{ \
	if (gshutdown) break; \
	if (error != ERR_NONE && error != ERR_UNKNOWN && error != ERR_NOTSUPPORTED && error != ERR_EMPTY && error != ERR_SOURCENOTAVAILABLE && error != ERR_NOTIMPLEMENTED) break; \
}

static void Monitor(int argc, char *argv[])
{
	GSM_MemoryStatus	MemStatus;
	GSM_SMSMemoryStatus	SMSStatus;
	GSM_ToDoStatus		ToDoStatus;
	GSM_CalendarStatus	CalendarStatus;
	GSM_NetworkInfo		NetInfo;
    	GSM_BatteryCharge   	BatteryCharge;
    	GSM_SignalQuality   	SignalQuality;
	GSM_Error		error;
	int 			count = -1;

	if (argc >= 3) {
		count = atoi(argv[2]);
		if (count <= 0) count = -1;
	}

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));
	printf("%s\n\n", _("Entering monitor mode..."));

	GSM_Init(true);

	GSM_SetIncomingSMSCallback(s, IncomingSMS);
	GSM_SetIncomingCBCallback(s, IncomingCB);
	GSM_SetIncomingCallCallback(s, IncomingCall);
	GSM_SetIncomingUSSDCallback(s, IncomingUSSD);

	error=GSM_SetIncomingSMS  		(s,true);
	printf("%-35s : %s\n", _("Enabling info about incoming SMS"), GSM_ErrorString(error));
	error=GSM_SetIncomingCB   		(s,true);
	printf("%-35s : %s\n", _("Enabling info about incoming CB"), GSM_ErrorString(error));
	error=GSM_SetIncomingCall 		(s,true);
	printf("%-35s : %s\n", _("Enabling info about calls"), GSM_ErrorString(error));
	error=GSM_SetIncomingUSSD 		(s,true);
	printf("%-35s : %s\n", _("Enabling info about USSD"), GSM_ErrorString(error));

	while (!gshutdown && count != 0) {
		if (count > 0) count--;
		CHECKMEMORYSTATUS(MemStatus,MEM_SM,"SIM phonebook");
		CHECK_EXIT;
		CHECKMEMORYSTATUS(MemStatus,MEM_DC,"Dialled numbers");
		CHECK_EXIT;
		CHECKMEMORYSTATUS(MemStatus,MEM_RC,"Received numbers");
		CHECK_EXIT;
		CHECKMEMORYSTATUS(MemStatus,MEM_MC,"Missed numbers");
		CHECK_EXIT;
		CHECKMEMORYSTATUS(MemStatus,MEM_ON,"Own numbers");
		CHECK_EXIT;
		CHECKMEMORYSTATUS(MemStatus,MEM_ME,"Phone phonebook");
		CHECK_EXIT;
		if ( (error = GSM_GetToDoStatus(s, &ToDoStatus)) == ERR_NONE) {
			PRINTUSED(_("ToDos"), ToDoStatus.Used, ToDoStatus.Free);
		}
		CHECK_EXIT;
		if ( (error = GSM_GetCalendarStatus(s, &CalendarStatus)) == ERR_NONE) {
			PRINTUSED(_("Calendar"), CalendarStatus.Used, CalendarStatus.Free);
		}
		CHECK_EXIT;
		if ( (error = GSM_GetBatteryCharge(s,&BatteryCharge)) == ERR_NONE) {
            		if (BatteryCharge.BatteryPercent != -1) {
				printf(LISTFORMAT, _("Battery level"));
				printf(_("%i percent"), BatteryCharge.BatteryPercent);
				printf("\n");
			}
            		if (BatteryCharge.BatteryCapacity != -1) {
				printf(LISTFORMAT, _("Battery capacity"));
				printf(_("%i mAh"), BatteryCharge.BatteryCapacity);
				printf("\n");
			}
            		if (BatteryCharge.BatteryTemperature != -1) {
				printf(LISTFORMAT, _("Battery temperature"));
				/* l10n: This means degrees Celsius */
				printf(_("%i C"), BatteryCharge.BatteryTemperature);
				printf("\n");
			}
            		if (BatteryCharge.PhoneTemperature != -1) {
				printf(LISTFORMAT, _("Phone temperature"));
				printf(_("%i C"), BatteryCharge.PhoneTemperature);
				printf("\n");
			}
            		if (BatteryCharge.BatteryVoltage != -1) {
				printf(LISTFORMAT, _("Battery voltage"));
				printf(_("%i mV"), BatteryCharge.BatteryVoltage);
				printf("\n");
			}
            		if (BatteryCharge.ChargeVoltage != -1) {
				printf(LISTFORMAT, _("Charge voltage"));
				printf(_("%i mV"), BatteryCharge.ChargeVoltage);
				printf("\n");
			}
            		if (BatteryCharge.ChargeCurrent != -1) {
				printf(LISTFORMAT, _("Charge current"));
				printf(_("%i mA"), BatteryCharge.ChargeCurrent);
				printf("\n");
			}
            		if (BatteryCharge.PhoneCurrent != -1) {
				printf(LISTFORMAT, _("Phone current"));
				printf(_("%i mA"), BatteryCharge.PhoneCurrent);
				printf("\n");
			}
            		if (BatteryCharge.ChargeState != 0) {
                		printf(LISTFORMAT, _("Charge state"));
                		switch (BatteryCharge.ChargeState) {
                    			case GSM_BatteryPowered:
						printf(_("powered from battery"));
						break;
                    			case GSM_BatteryConnected:
						printf(_("battery connected, but not powered from battery"));
                        			break;
                    			case GSM_BatteryCharging:
						printf(_("battery connected and is being charged"));
                        			break;
                    			case GSM_BatteryFull:
						printf(_("battery connected and is fully charged"));
                        			break;
                    			case GSM_BatteryNotConnected:
                        			printf(_("battery not connected"));
                        			break;
                    			case GSM_PowerFault:
                        			printf(_("detected power failure"));
                        			break;
#ifndef CHECK_CASES
                    			default:
                        			printf(_("unknown"));
                       				break;
#endif
                		}
                		printf("\n");
            		}
            		if (BatteryCharge.BatteryType != 0) {
                		printf(LISTFORMAT, _("Battery type"));
                		switch (BatteryCharge.BatteryType) {
                    			case GSM_BatteryLiIon:
						printf(_("Lithium Ion"));
						break;
                    			case GSM_BatteryLiPol:
						printf(_("Lithium Polymer"));
						break;
                    			case GSM_BatteryNiMH:
						printf(_("NiMH"));
						break;
					case GSM_BatteryUnknown:
#ifndef CHECK_CASES
                    			default:
#endif
                        			printf(_("unknown"));
                       				break;
                		}
                		printf("\n");
            		}
        	}
		CHECK_EXIT;
		if ( (error = GSM_GetSignalQuality(s,&SignalQuality)) == ERR_NONE) {
            		if (SignalQuality.SignalStrength != -1) {
				printf(LISTFORMAT, _("Signal strength"));
				printf(_("%i dBm"), SignalQuality.SignalStrength);
                		printf("\n");
			}
            		if (SignalQuality.SignalPercent  != -1) {
				printf(LISTFORMAT, _("Network level"));
				printf(_("%i percent"), SignalQuality.SignalPercent);
                		printf("\n");
			}
            		if (SignalQuality.BitErrorRate   != -1) {
				printf(LISTFORMAT, _("Bit error rate"));
				printf(_("%i percent"), SignalQuality.BitErrorRate);
                		printf("\n");
			}
        	}
		CHECK_EXIT;
		if ( (error = GSM_GetSMSStatus(s,&SMSStatus)) == ERR_NONE) {
			if (SMSStatus.SIMSize > 0) {
				printf(LISTFORMAT, _("SIM SMS status"));
				printf(_("%i used"), SMSStatus.SIMUsed);
				printf(", ");
				printf(_("%i unread"), SMSStatus.SIMUnRead);
				printf(", ");
				printf(_("%i locations"), SMSStatus.SIMSize);
				printf("\n");
			}

			if (SMSStatus.PhoneSize > 0) {
				printf(LISTFORMAT, _("Phone SMS status"));
				printf(_("%i used"), SMSStatus.PhoneUsed);
				printf(", ");
				printf(_("%i unread"), SMSStatus.PhoneUnRead);
				printf(", ");
				printf(_("%i locations"), SMSStatus.PhoneSize);
				if (SMSStatus.TemplatesUsed != 0) {
					printf(", ");
					printf(_("%i templates"), SMSStatus.TemplatesUsed);
				}
				printf("\n");
			}
		}
		CHECK_EXIT;
		if ( (error = GSM_GetNetworkInfo(s,&NetInfo)) == ERR_NONE) {
			PrintNetworkInfo(NetInfo);
		}
		if (wasincomingsms) DisplayIncomingSMS();
		printf("\n");
	}

	printf("%s\n", _("Leaving monitor mode..."));

	GSM_Terminate();
}

static void GetRingtone(int argc, char *argv[])
{
	GSM_Ringtone 	ringtone;
	bool		PhoneRingtone = false;

	if (strcasestr(argv[1], "getphoneringtone") != NULL) {
		PhoneRingtone = true;
	}

	GetStartStop(&ringtone.Location, NULL, 2, argc, argv);

	GSM_Init(true);

	ringtone.Format=0;

	error=GSM_GetRingtone(s,&ringtone,PhoneRingtone);
	Print_Error(error);

	switch (ringtone.Format) {
		case RING_NOTETONE	: printf(_("Smart Messaging"));	break;
		case RING_NOKIABINARY	: printf(_("Nokia binary"));	break;
		case RING_MIDI		: printf(_("MIDI"));		break;
		case RING_MMF		: printf(_("SMAF (MMF)"));	break;
	}
	printf(_(" format, ringtone \"%s\"\n"),DecodeUnicodeConsole(ringtone.Name));

	if (argc==4) {
		error=GSM_SaveRingtoneFile(argv[3], &ringtone);
		Print_Error(error);
	}

	GSM_Terminate();
}

static void GetRingtonesList(int argc UNUSED, char *argv[] UNUSED)
{
 	GSM_AllRingtonesInfo 	Info = {0, NULL};
	int			i;

	GSM_Init(true);

	error=GSM_GetRingtonesInfo(s,&Info);
 	if (error != ERR_NONE && Info.Ringtone) free(Info.Ringtone);
	Print_Error(error);

	GSM_Terminate();

	for (i=0;i<Info.Number;i++) printf("%i. \"%s\"\n",i+1,DecodeUnicodeConsole(Info.Ringtone[i].Name));

 	if (Info.Ringtone) free(Info.Ringtone);
}

static void DialVoice(int argc, char *argv[])
{
	GSM_CallShowNumber ShowNumber = GSM_CALL_DefaultNumberPresence;

	if (argc > 3) {
		if (strcasecmp(argv[3],"show") == 0) {		ShowNumber = GSM_CALL_ShowNumber;
		} else if (strcasecmp(argv[3],"hide") == 0) {	ShowNumber = GSM_CALL_HideNumber;
		} else {
			printf(_("Unknown parameter (\"%s\")\n"),argv[3]);
			exit(-1);
		}
	}

	GSM_Init(true);

	error=GSM_DialVoice(s, argv[2], ShowNumber);
	Print_Error(error);

	GSM_Terminate();
}

int TerminateID = -1;

static void IncomingCall0(GSM_StateMachine *s UNUSED, GSM_Call call)
{
	if (call.CallIDAvailable) TerminateID = call.CallID;
}

static void MakeTerminatedCall(int argc, char *argv[])
{
	GSM_CallShowNumber 	ShowNumber = GSM_CALL_DefaultNumberPresence;
//	GSM_DateTime		DT;
//	time_t			one,two;

	if (argc > 4) {
		if (strcasecmp(argv[4],"show") == 0) {		ShowNumber = GSM_CALL_ShowNumber;
		} else if (strcasecmp(argv[4],"hide") == 0) {	ShowNumber = GSM_CALL_HideNumber;
		} else {
			printf(_("Unknown parameter (\"%s\")\n"),argv[4]);
			exit(-1);
		}
	}

	GSM_Init(true);

	TerminateID = -1;
	GSM_SetIncomingCallCallback(s, IncomingCall0);

	error=GSM_SetIncomingCall(s,true);
	Print_Error(error);

	error=GSM_DialVoice(s, argv[2], ShowNumber);
	Print_Error(error);

//	GSM_GetCurrentDateTime (&DT);
//	one = Fill_Time_T(DT);

//	while (true) {
		my_sleep(atoi(argv[3]));
//		GSM_GetCurrentDateTime (&DT);
//		two = Fill_Time_T(DT);
//		if (two - one > atoi(argv[3])) break;
		GSM_ReadDevice(s,true);
//	}

	if (TerminateID != -1) {
		error=GSM_CancelCall(s,TerminateID,false);
		Print_Error(error);
	}

	GSM_Terminate();
}


static void CancelCall(int argc, char *argv[])
{
	GSM_Init(true);

	if (argc>2) {
		error=GSM_CancelCall(s,atoi(argv[2]),false);
	} else {
		error=GSM_CancelCall(s,0,true);
	}
	Print_Error(error);

	GSM_Terminate();
}

static void AnswerCall(int argc, char *argv[])
{
	GSM_Init(true);

	if (argc>2) {
		error=GSM_AnswerCall(s,atoi(argv[2]),false);
	} else {
		error=GSM_AnswerCall(s,0,true);
	}
	Print_Error(error);

	GSM_Terminate();
}

static void UnholdCall(int argc UNUSED, char *argv[])
{
	GSM_Init(true);

	error=GSM_UnholdCall(s,atoi(argv[2]));
	Print_Error(error);

	GSM_Terminate();
}

static void HoldCall(int argc UNUSED, char *argv[])
{
	GSM_Init(true);

	error=GSM_HoldCall(s,atoi(argv[2]));
	Print_Error(error);

	GSM_Terminate();
}

static void ConferenceCall(int argc UNUSED, char *argv[])
{
	GSM_Init(true);

	error=GSM_ConferenceCall(s,atoi(argv[2]));
	Print_Error(error);

	GSM_Terminate();
}

static void SplitCall(int argc UNUSED, char *argv[])
{
	GSM_Init(true);

	error=GSM_SplitCall(s,atoi(argv[2]));
	Print_Error(error);

	GSM_Terminate();
}

static void SwitchCall(int argc, char *argv[])
{
	GSM_Init(true);

	if (argc > 2) {
		error=GSM_SwitchCall(s,atoi(argv[2]),false);
	} else {
		error=GSM_SwitchCall(s,0,true);
	}
	Print_Error(error);

	GSM_Terminate();
}

static void TransferCall(int argc, char *argv[])
{
	GSM_Init(true);

	if (argc > 2) {
		error=GSM_TransferCall(s,atoi(argv[2]),false);
	} else {
		error=GSM_TransferCall(s,0,true);
	}
	Print_Error(error);

	GSM_Terminate();
}

static void AddSMSFolder(int argc UNUSED, char *argv[])
{
	unsigned char buffer[200];

	GSM_Init(true);

	EncodeUnicode(buffer,argv[2],strlen(argv[2]));
	error=GSM_AddSMSFolder(s,buffer);
	Print_Error(error);

	GSM_Terminate();
}

static void Reset(int argc UNUSED, char *argv[])
{
	bool hard;

	if (strcasecmp(argv[2],"SOFT") == 0) {		hard=false;
	} else if (strcasecmp(argv[2],"HARD") == 0) {	hard=true;
	} else {
		printf(_("What type of reset do you want (\"%s\") ?\n"),argv[2]);
		exit(-1);
	}

	GSM_Init(true);

	error=GSM_Reset(s, hard);
	Print_Error(error);

	GSM_Terminate();
}

static void GetCalendar(int argc UNUSED, char *argv[])
{
	GSM_CalendarEntry	Note;
	int			start,stop;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		Note.Location=i;
		error = GSM_GetCalendar(s, &Note);
		if (error == ERR_EMPTY) continue;
		Print_Error(error);
		printf(LISTFORMAT "%d\n", _("Location"), Note.Location);
		PrintCalendar(&Note);
	}

	GSM_Terminate();
}

static void DeleteCalendar(int argc, char *argv[])
{
	GSM_CalendarEntry	Note;
	int			start,stop;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		Note.Location=i;
		error = GSM_DeleteCalendar(s, &Note);
		Print_Error(error);
	}

	GSM_Terminate();
}

static void GetAllCalendar(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_CalendarEntry	Note;
	bool			refresh	= true;

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));

	GSM_Init(true);

	while (!gshutdown) {
		error=GSM_GetNextCalendar(s,&Note,refresh);
		if (error == ERR_EMPTY) break;
		Print_Error(error);
		printf(LISTFORMAT "%d\n", _("Location"), Note.Location);
		PrintCalendar(&Note);
		refresh=false;
	}

	GSM_Terminate();
}

static void GetCalendarSettings(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_CalendarSettings settings;

	GSM_Init(true);

	error=GSM_GetCalendarSettings(s,&settings);
	Print_Error(error);

	if (settings.AutoDelete == 0) {
		printf(_("Auto deleting disabled"));
	} else {
		printf(_("Auto deleting notes after %i day(s)"),settings.AutoDelete);
	}
	printf("\n");
	printf(_("Week starts on %s"), GetDayName(settings.StartDay));
	printf("\n");

	GSM_Terminate();
}

static void GetWAPBookmark(int argc, char *argv[])
{
	GSM_WAPBookmark		bookmark;
	int			start,stop;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		bookmark.Location=i;
		error=GSM_GetWAPBookmark(s,&bookmark);
		Print_Error(error);
		printf(LISTFORMAT "\"%s\"\n", _("Name"),DecodeUnicodeConsole(bookmark.Title));
		printf(LISTFORMAT "\"%s\"\n", _("Address"),DecodeUnicodeConsole(bookmark.Address));
	}

	GSM_Terminate();
}

static void DeleteWAPBookmark(int argc, char *argv[])
{
	GSM_WAPBookmark	bookmark;
	int		start, stop;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		bookmark.Location=i;
		error=GSM_DeleteWAPBookmark(s, &bookmark);
		Print_Error(error);
	}

	GSM_Terminate();
}

static void GetGPRSPoint(int argc, char *argv[])
{
	GSM_GPRSAccessPoint	point;
	int			start,stop;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		point.Location=i;
		error=GSM_GetGPRSAccessPoint(s,&point);
		if (error != ERR_EMPTY) {
			Print_Error(error);
			printf("%i. \"%s\"",point.Location,DecodeUnicodeConsole(point.Name));
		} else {
			printf(_("%i. Access point %i"),point.Location,point.Location);
		}
		if (point.Active) printf(_(" (active)"));
		printf("\n");
		if (error != ERR_EMPTY) {
			printf(LISTFORMAT "\"%s\"\n\n", _("Address"),DecodeUnicodeConsole(point.URL));
		} else {
			printf("\n");
		}
	}

	GSM_Terminate();
}

static void GetBitmap(int argc, char *argv[])
{
	GSM_File		File;
	GSM_MultiBitmap 	MultiBitmap;
	int			location=0, Handle, Size;
 	GSM_AllRingtonesInfo 	Info = {0, NULL};
	unsigned char		buffer[10];

	if (strcasecmp(argv[2],"STARTUP") == 0) {
		MultiBitmap.Bitmap[0].Type=GSM_StartupLogo;
	} else if (strcasecmp(argv[2],"CALLER") == 0) {
		MultiBitmap.Bitmap[0].Type=GSM_CallerGroupLogo;
		GetStartStop(&location, NULL, 3, argc, argv);
		if (location>5) {
			printf("%s\n", _("Maximal location for caller logo can be 5"));
			exit (-1);
		}
	} else if (strcasecmp(argv[2],"PICTURE") == 0) {
		MultiBitmap.Bitmap[0].Type=GSM_PictureImage;
		GetStartStop(&location, NULL, 3, argc, argv);
	} else if (strcasecmp(argv[2],"TEXT") == 0) {
		MultiBitmap.Bitmap[0].Type=GSM_WelcomeNote_Text;
	} else if (strcasecmp(argv[2],"DEALER") == 0) {
		MultiBitmap.Bitmap[0].Type=GSM_DealerNote_Text;
	} else if (strcasecmp(argv[2],"OPERATOR") == 0) {
		MultiBitmap.Bitmap[0].Type=GSM_OperatorLogo;
	} else {
		printf(_("What type of logo do you want to get (\"%s\") ?\n"),argv[2]);
		exit(-1);
	}
	MultiBitmap.Bitmap[0].Location=location;

	GSM_Init(true);

	error=GSM_GetBitmap(s,&MultiBitmap.Bitmap[0]);
	Print_Error(error);

	MultiBitmap.Number = 1;

	error=ERR_NONE;
	switch (MultiBitmap.Bitmap[0].Type) {
	case GSM_CallerGroupLogo:
		if (!MultiBitmap.Bitmap[0].DefaultBitmap) GSM_PrintBitmap(stdout,&MultiBitmap.Bitmap[0]);
		printf(LISTFORMAT "\"%s\"", _("Group name"),DecodeUnicodeConsole(MultiBitmap.Bitmap[0].Text));
		if (MultiBitmap.Bitmap[0].DefaultName) printf(_(" (default)"));
		printf("\n");
		if (MultiBitmap.Bitmap[0].DefaultRingtone) {
			printf(LISTFORMAT "%s\n", _("Ringtone"), _("default"));
		} else if (GSM_IsPhoneFeatureAvailable(GSM_GetModelInfo(s), F_6230iCALLER)) {
			printf(LISTFORMAT "%i\n", _("Ringtone"),MultiBitmap.Bitmap[0].RingtoneID);
		} else if (MultiBitmap.Bitmap[0].FileSystemRingtone) {
			sprintf(buffer,"%i",MultiBitmap.Bitmap[0].RingtoneID);
			EncodeUnicode(File.ID_FullName,buffer,strlen(buffer));

			File.Buffer 	= NULL;
			File.Used 	= 0;

			error = ERR_NONE;
//			while (error == ERR_NONE) {
				error = GSM_GetFilePart(s,&File,&Handle,&Size);
//			}
		    	if (error != ERR_EMPTY && error != ERR_WRONGCRC) Print_Error(error);
			error = ERR_NONE;

			printf(LISTFORMAT "\"%s\" ", _("Ringtone"), DecodeUnicodeString(File.Name));
			printf(_("(file with ID %i)\n"), MultiBitmap.Bitmap[0].RingtoneID);
		} else {
			error = GSM_GetRingtonesInfo(s,&Info);
			if (error != ERR_NONE) Info.Number = 0;
			error = ERR_NONE;

			printf(LISTFORMAT, _("Ringtone"));
			if (UnicodeLength(GSM_GetRingtoneName(&Info,MultiBitmap.Bitmap[0].RingtoneID))!=0) {
				printf(_("\"%s\" (ID %i)\n"),
					DecodeUnicodeConsole(GSM_GetRingtoneName(&Info,MultiBitmap.Bitmap[0].RingtoneID)),
					MultiBitmap.Bitmap[0].RingtoneID);
			} else {
				printf(_("ID %i\n"),MultiBitmap.Bitmap[0].RingtoneID);
			}

			if (Info.Ringtone) free(Info.Ringtone);
		}
		if (MultiBitmap.Bitmap[0].BitmapEnabled) {
			printf(LISTFORMAT "%s\n", _("Bitmap"), _("enabled"));
		} else {
			printf(LISTFORMAT "%s\n", _("Bitmap"), _("disabled"));
		}
		if (MultiBitmap.Bitmap[0].FileSystemPicture) {
			printf(LISTFORMAT "%i\n", _("Bitmap ID"),MultiBitmap.Bitmap[0].PictureID);
		}
		if (argc>4 && !MultiBitmap.Bitmap[0].DefaultBitmap) error=GSM_SaveBitmapFile(argv[4],&MultiBitmap);
		break;
	case GSM_StartupLogo:
		GSM_PrintBitmap(stdout,&MultiBitmap.Bitmap[0]);
		if (argc>3) error=GSM_SaveBitmapFile(argv[3],&MultiBitmap);
		break;
	case GSM_OperatorLogo:
		if (strcmp(MultiBitmap.Bitmap[0].NetworkCode,"000 00")!=0) {
			GSM_PrintBitmap(stdout,&MultiBitmap.Bitmap[0]);
			if (argc>3) error=GSM_SaveBitmapFile(argv[3],&MultiBitmap);
		} else {
			printf("%s\n", _("No operator logo in phone"));
		}
		break;
	case GSM_PictureImage:
		GSM_PrintBitmap(stdout,&MultiBitmap.Bitmap[0]);
		printf(LISTFORMAT "\"%s\"\n", _("Text"),DecodeUnicodeConsole(MultiBitmap.Bitmap[0].Text));
		printf(LISTFORMAT "\"%s\"\n", _("Sender"),DecodeUnicodeConsole(MultiBitmap.Bitmap[0].Sender));
		if (MultiBitmap.Bitmap[0].Name)
			printf(LISTFORMAT "\"%s\"\n", _("Name"),DecodeUnicodeConsole(MultiBitmap.Bitmap[0].Name));
		if (argc>4) error=GSM_SaveBitmapFile(argv[4],&MultiBitmap);
		break;
	case GSM_WelcomeNote_Text:
		printf(_("Welcome note text is \"%s\"\n"),DecodeUnicodeConsole(MultiBitmap.Bitmap[0].Text));
		break;
	case GSM_DealerNote_Text:
		printf(_("Dealer note text is \"%s\"\n"),DecodeUnicodeConsole(MultiBitmap.Bitmap[0].Text));
		break;
	default:
		break;
	}
	Print_Error(error);

	GSM_Terminate();
}

static void SetBitmap(int argc, char *argv[])
{
	GSM_Bitmap		Bitmap, NewBitmap;
	GSM_MultiBitmap		MultiBitmap;
	GSM_NetworkInfo		NetInfo;
	bool			init = true;

	if (strcasecmp(argv[2],"STARTUP") == 0) {
		if (argc<4) {
			printf("%s\n", _("More parameters required!"));
			exit(-1);
		}
		MultiBitmap.Bitmap[0].Type=GSM_StartupLogo;
		MultiBitmap.Bitmap[0].Location=1;
		if (!strcmp(argv[3],"1")) MultiBitmap.Bitmap[0].Location = 2;
		if (!strcmp(argv[3],"2")) MultiBitmap.Bitmap[0].Location = 3;
		if (!strcmp(argv[3],"3")) MultiBitmap.Bitmap[0].Location = 4;
		if (MultiBitmap.Bitmap[0].Location == 1) {
			error=GSM_ReadBitmapFile(argv[3],&MultiBitmap);
			Print_Error(error);
		}
		memcpy(&Bitmap,&MultiBitmap.Bitmap[0],sizeof(GSM_Bitmap));
	} else if (strcasecmp(argv[2],"TEXT") == 0) {
		if (argc<4) {
			printf("%s\n", _("More parameters required!"));
			exit(-1);
		}
		Bitmap.Type=GSM_WelcomeNote_Text;
		EncodeUnicode(Bitmap.Text,argv[3],strlen(argv[3]));
	} else if (strcasecmp(argv[2],"DEALER") == 0) {
		if (argc<4) {
			printf("%s\n", _("More parameters required!"));
			exit(-1);
		}
		Bitmap.Type=GSM_DealerNote_Text;
		EncodeUnicode(Bitmap.Text,argv[3],strlen(argv[3]));
	} else if (strcasecmp(argv[2],"CALLER") == 0) {
		if (argc<4) {
			printf("%s\n", _("More parameters required!"));
			exit(-1);
		}
		GetStartStop(&i, NULL, 3, argc, argv);
		if (i>5 && i!=255) {
			printf("%s\n", _("Maximal location for caller logo can be 5"));
			exit (-1);
		}
		MultiBitmap.Bitmap[0].Type 	= GSM_CallerGroupLogo;
		MultiBitmap.Bitmap[0].Location	= i;
		if (argc>4) {
			error=GSM_ReadBitmapFile(argv[4],&MultiBitmap);
			Print_Error(error);
		}
		memcpy(&Bitmap,&MultiBitmap.Bitmap[0],sizeof(GSM_Bitmap));
		if (i!=255) {
			GSM_Init(true);
			init = false;
			NewBitmap.Type 	   = GSM_CallerGroupLogo;
			NewBitmap.Location = i;
			error=GSM_GetBitmap(s,&NewBitmap);
			Print_Error(error);
			Bitmap.RingtoneID	  = NewBitmap.RingtoneID;
			Bitmap.DefaultRingtone 	  = NewBitmap.DefaultRingtone;
			Bitmap.FileSystemRingtone = false;
			CopyUnicodeString(Bitmap.Text, NewBitmap.Text);
			Bitmap.DefaultName	  = NewBitmap.DefaultName;
		}
	} else if (strcasecmp(argv[2],"PICTURE") == 0) {
		if (argc<5) {
			printf("%s\n", _("More parameters required!"));
			exit(-1);
		}
		MultiBitmap.Bitmap[0].Type		= GSM_PictureImage;
		MultiBitmap.Bitmap[0].Location		= atoi(argv[4]);
		error=GSM_ReadBitmapFile(argv[3],&MultiBitmap);
		Print_Error(error);
		memcpy(&Bitmap,&MultiBitmap.Bitmap[0],sizeof(GSM_Bitmap));
		Bitmap.Text[0]=0;
		Bitmap.Text[1]=0;
		if (argc == 6) EncodeUnicode(Bitmap.Text,argv[5],strlen(argv[5]));
		Bitmap.Sender[0]=0;
		Bitmap.Sender[1]=0;
	} else if (strcasecmp(argv[2],"COLOUROPERATOR") == 0) {
		Bitmap.Type = GSM_ColourOperatorLogo_ID;
		strcpy(Bitmap.NetworkCode,"000 00");
		if (argc > 3) {
			Bitmap.ID = atoi(argv[3]);
			if (argc>4) {
				strncpy(Bitmap.NetworkCode,argv[4],6);
			} else {
				GSM_Init(true);
				init = false;
				error=GSM_GetNetworkInfo(s,&NetInfo);
				Print_Error(error);
				strcpy(Bitmap.NetworkCode,NetInfo.NetworkCode);
			}
		}
	} else if (strcasecmp(argv[2],"COLOURSTARTUP") == 0) {
		Bitmap.Type 	= GSM_ColourStartupLogo_ID;
		Bitmap.Location = 0;
		if (argc > 3) {
			Bitmap.Location = 1;
			Bitmap.ID 	= atoi(argv[3]);
		}
	} else if (strcasecmp(argv[2],"WALLPAPER") == 0) {
		Bitmap.Type 		= GSM_ColourWallPaper_ID;
		Bitmap.ID		= 0;
		if (argc > 3) Bitmap.ID = atoi(argv[3]);
	} else if (strcasecmp(argv[2],"OPERATOR") == 0) {
		MultiBitmap.Bitmap[0].Type	= GSM_OperatorLogo;
		MultiBitmap.Bitmap[0].Location 	= 1;
		strcpy(MultiBitmap.Bitmap[0].NetworkCode,"000 00");
		if (argc>3) {
			error=GSM_ReadBitmapFile(argv[3],&MultiBitmap);
			Print_Error(error);
			if (argc>4) {
				strncpy(MultiBitmap.Bitmap[0].NetworkCode,argv[4],6);
			} else {
				GSM_Init(true);
				init = false;
				error=GSM_GetNetworkInfo(s,&NetInfo);
				Print_Error(error);
				strcpy(MultiBitmap.Bitmap[0].NetworkCode,NetInfo.NetworkCode);
			}
		}
		memcpy(&Bitmap,&MultiBitmap.Bitmap[0],sizeof(GSM_Bitmap));
	} else {
		printf(_("What type of logo do you want to set (\"%s\") ?\n"),argv[2]);
		exit(-1);
	}

	if (init) GSM_Init(true);

	error=GSM_SetBitmap(s,&Bitmap);
	Print_Error(error);

#ifdef GSM_ENABLE_BEEP
	GSM_PhoneBeep();
#endif

	GSM_Terminate();
}

static void SetRingtone(int argc, char *argv[])
{
	GSM_Ringtone 	ringtone;
	int		i,nextlong=0;

	ringtone.Format	= 0;
	error=GSM_ReadRingtoneFile(argv[2],&ringtone);
	Print_Error(error);
	ringtone.Location = 255;
	for (i=3;i<argc;i++) {
		switch (nextlong) {
		case 0:
			if (strcasecmp(argv[i],"-scale") == 0) {
				ringtone.NoteTone.AllNotesScale = true;
				break;
			}
			if (strcasecmp(argv[i],"-location") == 0) {
				nextlong = 1;
				break;
			}
			if (strcasecmp(argv[i],"-name") == 0) {
				nextlong = 2;
				break;
			}
			printf(_("Unknown parameter (\"%s\")"),argv[i]);
			exit(-1);
		case 1:
			ringtone.Location=atoi(argv[i]);
			nextlong = 0;
			break;
		case 2:
			EncodeUnicode(ringtone.Name,argv[i],strlen(argv[i]));
			nextlong = 0;
			break;
		}
	}
	if (nextlong!=0) {
		printf_err("%s\n", _("Parameter missing!"));
		exit(-1);
	}
	if (ringtone.Location==0) {
		printf_err("%s\n", _("Please enumerate locations from 1"));
		exit (-1);
	}

	GSM_Init(true);
	error=GSM_SetRingtone(s, &ringtone, &i);
	Print_Error(error);
#ifdef GSM_ENABLE_BEEP
	GSM_PhoneBeep();
#endif
	GSM_Terminate();
}

static void ClearMemory(GSM_MemoryType type, const char *question)
{
	GSM_MemoryStatus	MemStatus;
	GSM_MemoryEntry		Pbk;
	bool			DoClear;

	DoClear = false;
	MemStatus.MemoryType = type;
	error = GSM_GetMemoryStatus(s, &MemStatus);
	if (error == ERR_NONE && MemStatus.MemoryUsed !=0) {
		if (answer_yes(question)) DoClear = true;
	}
	if (DoClear) {
		error = GSM_DeleteAllMemory(s, type);
		if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
			for (i = 0; i < MemStatus.MemoryUsed + MemStatus.MemoryFree; i++) {
				Pbk.MemoryType 	= type;
				Pbk.Location	= i + 1;
				Pbk.EntriesNum	= 0;
				error=GSM_DeleteMemory(s, &Pbk);
				Print_Error(error);
				fprintf(stderr, _("%cClearing: %i percent"), 13,
						(i + 1) * 100 / (MemStatus.MemoryUsed + MemStatus.MemoryFree));
				if (gshutdown) {
					GSM_Terminate();
					exit(0);
				}
			}
			fprintf(stderr, "\n");
		} else {
			fprintf(stderr, "%s\n", _("Done"));
			Print_Error(error);
		}
	}
}


static void ClearAll(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_ToDoStatus		ToDoStatus;
	GSM_CalendarEntry	Calendar;
	GSM_ToDoEntry		ToDo;
	GSM_NoteEntry		Note;
	GSM_WAPBookmark		Bookmark;
	GSM_FMStation 		Station;
	bool			DoClear;

	GSM_Init(true);

	ClearMemory(MEM_ME, _("Delete phone phonebook"));
	ClearMemory(MEM_SM, _("Delete SIM phonebook"));
	ClearMemory(MEM_MC, _("Delete missed calls"));
	ClearMemory(MEM_DC, _("Delete dialled calls"));
	ClearMemory(MEM_RC, _("Delete received calls"));

	DoClear = false;
	error = GSM_GetNextCalendar(s,&Calendar,true);
	if (error == ERR_NONE) {
 		if (answer_yes(_("Delete phone calendar notes"))) DoClear = true;
	}
	if (DoClear) {
		fprintf(stderr, LISTFORMAT, _("Deleting"));
		error=GSM_DeleteAllCalendar(s);
		if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
 			while (1) {
				error = GSM_GetNextCalendar(s,&Calendar,true);
				if (error != ERR_NONE) break;
				error = GSM_DeleteCalendar(s,&Calendar);
 				Print_Error(error);
				fprintf(stderr, "*");
			}
			fprintf(stderr, "\n");
		} else {
			fprintf(stderr, "%s\n", _("Done"));
			Print_Error(error);
		}
	}

	DoClear = false;
	error = GSM_GetToDoStatus(s,&ToDoStatus);
	if (error == ERR_NONE && ToDoStatus.Used != 0) {
		if (answer_yes(_("Delete phone ToDo"))) DoClear = true;
	}
	if (DoClear) {
		fprintf(stderr, LISTFORMAT, _("Deleting"));
		error=GSM_DeleteAllToDo(s);
		if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
 			while (1) {
				error = GSM_GetNextToDo(s,&ToDo,true);
				if (error != ERR_NONE) break;
				error = GSM_DeleteToDo(s,&ToDo);
 				Print_Error(error);
				fprintf(stderr, "*");
			}
			fprintf(stderr, "\n");
		} else {
			fprintf(stderr, "%s\n", _("Done"));
			Print_Error(error);
		}
	}

	DoClear = false;
	error = GSM_GetNotesStatus(s,&ToDoStatus);
	if (error == ERR_NONE && ToDoStatus.Used != 0) {
		if (answer_yes(_("Delete phone Notes"))) DoClear = true;
	}
	if (DoClear) {
		fprintf(stderr, LISTFORMAT, _("Deleting"));
		while (1) {
			error = GSM_GetNextNote(s,&Note,true);
			if (error != ERR_NONE) break;
			error = GSM_DeleteNote(s,&Note);
			Print_Error(error);
			fprintf(stderr, "*");
		}
		fprintf(stderr, "\n");
	}

	Bookmark.Location = 1;
	error = GSM_GetWAPBookmark(s,&Bookmark);
	if (error == ERR_NONE || error == ERR_INVALIDLOCATION) {
		if (answer_yes(_("Delete phone WAP bookmarks"))) {
			fprintf(stderr, LISTFORMAT, _("Deleting"));
			/* One thing to explain: DCT4 phones seems to have bug here.
			 * When delete for example first bookmark, phone change
			 * numeration for getting frame, not for deleting. So, we try to
			 * get 1'st bookmark. Inside frame is "correct" location. We use
			 * it later
			 */
			while (error==ERR_NONE) {
				error = GSM_DeleteWAPBookmark(s,&Bookmark);
				Bookmark.Location = 1;
				error = GSM_GetWAPBookmark(s,&Bookmark);
				fprintf(stderr, "*");
			}
			fprintf(stderr, "\n");
		}
	}
	if (GSM_DeleteUserRingtones != NOTSUPPORTED) {
		if (answer_yes(_("Delete all phone user ringtones"))) {
			fprintf(stderr, LISTFORMAT, _("Deleting"));
			error=GSM_DeleteUserRingtones(s);
			Print_Error(error);
			fprintf(stderr, "%s\n", _("Done"));
		}
	}
	Station.Location=i;
	error=GSM_GetFMStation(s,&Station);
	if (error == ERR_NONE || error == ERR_EMPTY) {
	 	if (answer_yes(_("Delete all phone FM radio stations"))) {
 			error=GSM_ClearFMStations(s);
 			Print_Error(error);
		}
 	}

	GSM_Terminate();
}

static void DisplayConnectionSettings(GSM_MultiWAPSettings *settings,int j)
{
	if (settings->Settings[j].IsContinuous) {
		printf(_("Connection type     : Continuous\n"));
	} else {
		printf(_("Connection type     : Temporary\n"));
	}
	if (settings->Settings[j].IsSecurity) {
		printf(_("Connection security : On\n"));
	} else {
		printf(_("Connection security : Off\n"));
	}
	printf(_("Proxy               : address \"%s\", port %i\n"),DecodeUnicodeConsole(settings->Proxy),settings->ProxyPort);
	printf(_("2'nd proxy          : address \"%s\", port %i\n"),DecodeUnicodeConsole(settings->Proxy2),settings->Proxy2Port);
	switch (settings->Settings[j].Bearer) {
	case WAPSETTINGS_BEARER_SMS:
		printf(LISTFORMAT "%s", _("Bearer"), _("SMS"));
		if (settings->ActiveBearer == WAPSETTINGS_BEARER_SMS) printf(_(" (active)"));
		printf("\n");
		printf(LISTFORMAT "\"%s\"\n", _("Server number"),DecodeUnicodeConsole(settings->Settings[j].Server));
		printf(LISTFORMAT "\"%s\"\n", _("Service number"),DecodeUnicodeConsole(settings->Settings[j].Service));
		break;
	case WAPSETTINGS_BEARER_DATA:
		printf(LISTFORMAT "%s", _("Bearer"), _("Data (CSD)"));
		if (settings->ActiveBearer == WAPSETTINGS_BEARER_DATA) printf(_(" (active)"));
		printf("\n");
		printf(LISTFORMAT "\"%s\"\n", _("Dial-up number"),DecodeUnicodeConsole(settings->Settings[j].DialUp));
		printf(LISTFORMAT "\"%s\"\n", _("IP address"),DecodeUnicodeConsole(settings->Settings[j].IPAddress));
		if (settings->Settings[j].ManualLogin) {
			printf(_("Login Type          : Manual\n"));
		} else {
			printf(_("Login Type          : Automatic\n"));
		}
		if (settings->Settings[j].IsNormalAuthentication) {
			printf(_("Authentication type : Normal\n"));
		} else {
			printf(_("Authentication type : Secure\n"));
		}
		if (settings->Settings[j].IsISDNCall) {
			printf(_("Data call type      : ISDN\n"));
              	} else {
			printf(_("Data call type      : Analogue\n"));
		}
		switch (settings->Settings[j].Speed) {
			case WAPSETTINGS_SPEED_9600  : printf(_("Data call speed     : 9600\n"));  break;
			case WAPSETTINGS_SPEED_14400 : printf(_("Data call speed     : 14400\n")); break;
			case WAPSETTINGS_SPEED_AUTO  : printf(_("Data call speed     : Auto\n"));  break;
		}
		printf(LISTFORMAT "\"%s\"\n", _("User name"),DecodeUnicodeConsole(settings->Settings[j].User));
		printf(LISTFORMAT "\"%s\"\n", _("Password"),DecodeUnicodeConsole(settings->Settings[j].Password));
		break;
	case WAPSETTINGS_BEARER_USSD:
		printf(LISTFORMAT "%s", _("Bearer"), _("USSD"));
		if (settings->ActiveBearer == WAPSETTINGS_BEARER_USSD) printf(_(" (active)"));
		printf("\n");
		printf(LISTFORMAT "\"%s\"\n", _("Service code"),DecodeUnicodeConsole(settings->Settings[j].Code));
		if (settings->Settings[j].IsIP) {
			printf(_("Address type        : IP address\nIPaddress           : \"%s\"\n"),DecodeUnicodeConsole(settings->Settings[j].Service));
		} else {
			printf(_("Address type        : Service number\nService number      : \"%s\"\n"),DecodeUnicodeConsole(settings->Settings[j].Service));
		}
		break;
	case WAPSETTINGS_BEARER_GPRS:
		printf(LISTFORMAT "%s", _("Bearer"), _("GPRS"));
		if (settings->ActiveBearer == WAPSETTINGS_BEARER_GPRS) printf(_(" (active)"));
		printf("\n");
		if (settings->Settings[j].ManualLogin) {
			printf(_("Login Type          : Manual\n"));
		} else {
			printf(_("Login Type          : Automatic\n"));
		}
		if (settings->Settings[j].IsNormalAuthentication) {
			printf(_("Authentication type : Normal\n"));
		} else {
			printf(_("Authentication type : Secure\n"));
		}
		printf(LISTFORMAT "\"%s\"\n", _("Access point"),DecodeUnicodeConsole(settings->Settings[j].DialUp));
		printf(LISTFORMAT "\"%s\"\n", _("IP address"),DecodeUnicodeConsole(settings->Settings[j].IPAddress));
		printf(LISTFORMAT "\"%s\"\n", _("User name"),DecodeUnicodeConsole(settings->Settings[j].User));
		printf(LISTFORMAT "\"%s\"\n", _("Password"),DecodeUnicodeConsole(settings->Settings[j].Password));
	}
}

static void GetSyncMLSettings(int argc, char *argv[])
{
	GSM_SyncMLSettings	settings;
	int			start,stop,j;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		settings.Location=i;
		error=GSM_GetSyncMLSettings(s,&settings);
		Print_Error(error);
		printf("%i. ", i);
		if (settings.Name[0]==0 && settings.Name[1]==0) {
			printf(_("Set %i"),i);
		} else {
			printf("%s", DecodeUnicodeConsole(settings.Name));
		}
		if (settings.Active) printf(_(" (active)"));
//		if (settings.ReadOnly) printf(_("\nRead only           : yes"));
		printf("\n");
		printf(LISTFORMAT "\"%s\"\n", _("User"),DecodeUnicodeConsole(settings.User));
		printf(LISTFORMAT "\"%s\"\n", _("Password"),DecodeUnicodeConsole(settings.Password));
		printf(LISTFORMAT "\"%s\"\n", _("Phonebook database"),DecodeUnicodeConsole(settings.PhonebookDataBase));
		printf(LISTFORMAT "\"%s\"\n", _("Calendar database"),DecodeUnicodeConsole(settings.CalendarDataBase));
		printf(LISTFORMAT "\"%s\"\n", _("Server"),DecodeUnicodeConsole(settings.Server));
		printf(LISTFORMAT, _("Sync. phonebook"));
		if (settings.SyncPhonebook) printf("%s\n", _("enabled"));
		if (!settings.SyncPhonebook) printf("%s\n", _("disabled"));
		printf(LISTFORMAT, _("Sync. calendar"));
		if (settings.SyncCalendar) printf("%s\n", _("enabled"));
		if (!settings.SyncCalendar) printf("%s\n", _("disabled"));
		printf("\n");
		for (j=0;j<settings.Connection.Number;j++) {
			printf(LISTFORMAT, _("Connection set name"));
			if (settings.Connection.Settings[j].Title[0]==0 && settings.Connection.Settings[j].Title[1]==0) {
				printf(_("Set %i"),i);
			} else {
				printf("%s",DecodeUnicodeConsole(settings.Connection.Settings[j].Title));
			}
			printf("\n");
			DisplayConnectionSettings(&settings.Connection,j);
			printf("\n");
		}
	}
	GSM_Terminate();
}

static void GetChatSettings(int argc, char *argv[])
{
	GSM_ChatSettings	settings;
	int			start,stop,j;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		settings.Location=i;
		error=GSM_GetChatSettings(s,&settings);
		Print_Error(error);
		printf("%i. ",i);
		if (settings.Name[0]==0 && settings.Name[1]==0) {
			printf(_("Set %i"),i);
		} else {
			printf("%s", DecodeUnicodeConsole(settings.Name));
		}
		if (settings.Active) printf(_(" (active)"));
//		if (settings.ReadOnly) printf(_("\nRead only           : yes"));
		printf("\n");
		printf(LISTFORMAT "\"%s\"\n", _("Homepage"),DecodeUnicodeConsole(settings.HomePage));
		printf(LISTFORMAT "\"%s\"\n", _("User"),DecodeUnicodeConsole(settings.User));
		printf(LISTFORMAT "\"%s\"\n", _("Password"),DecodeUnicodeConsole(settings.Password));
		printf("\n");
		for (j=0;j<settings.Connection.Number;j++) {
			if (settings.Connection.Settings[j].Title[0]==0 && settings.Connection.Settings[j].Title[1]==0) {
				printf(_("Connection set name : Set %i\n"),i);
			} else {
				printf(_("Connection set name : %s\n"),DecodeUnicodeConsole(settings.Connection.Settings[j].Title));
			}
			DisplayConnectionSettings(&settings.Connection,j);
			printf("\n");
		}
	}
	GSM_Terminate();
}

static void GetWAPMMSSettings(int argc, char *argv[])
{
	GSM_MultiWAPSettings	settings;
	int			start,stop,j;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		settings.Location=i;
		if (strcasestr(argv[1], "getwapsettings") != NULL) {
			error=GSM_GetWAPSettings(s,&settings);
		} else {
			error=GSM_GetMMSSettings(s,&settings);
		}
		Print_Error(error);
		for (j=0;j<settings.Number;j++) {
			printf("%i. ", i);
			if (settings.Settings[j].Title[0]==0 && settings.Settings[j].Title[1]==0) {
				printf(_("Set %i"),i);
			} else {
				printf("%s", DecodeUnicodeConsole(settings.Settings[j].Title));
			}
			if (settings.Active) printf(_(" (active)"));
			if (settings.ReadOnly) printf(_("\nRead only           : yes"));
			printf(LISTFORMAT "\"%s\"\n", _("\nHomepage"),DecodeUnicodeConsole(settings.Settings[j].HomePage));
			DisplayConnectionSettings(&settings,j);
			printf("\n");
		}
	}
	GSM_Terminate();
}

static void CopyBitmap(int argc, char *argv[])
{
	GSM_MultiBitmap Bitmap;
	int		i;

	Bitmap.Bitmap[0].Type = GSM_None;

	error=GSM_ReadBitmapFile(argv[2],&Bitmap);
	Print_Error(error);

	if (argc==3) {
		for (i=0;i<Bitmap.Number;i++) {
			switch (Bitmap.Bitmap[i].Type) {
				case GSM_StartupLogo :	printf(_("Startup logo"));		break;
				case GSM_OperatorLogo:	printf(_("Operator logo")); 	break;
				case GSM_PictureImage:	printf(_("Picture Image")); 	break;
				case GSM_CallerGroupLogo  :	printf(_("Caller group logo")); 	break;
				default		     : 					break;
			}
			printf(_(", width %i, height %i\n"),Bitmap.Bitmap[i].BitmapWidth,Bitmap.Bitmap[i].BitmapHeight);
			GSM_PrintBitmap(stdout,&Bitmap.Bitmap[i]);
		}
	} else {
		if (argc == 5) {
			for (i=0;i<Bitmap.Number;i++) {
				if (strcasecmp(argv[4],"PICTURE") == 0) {
					Bitmap.Bitmap[i].Type = GSM_PictureImage;
				} else if (strcasecmp(argv[4],"STARTUP") == 0) {
					Bitmap.Bitmap[i].Type = GSM_StartupLogo;
				} else if (strcasecmp(argv[4],"CALLER") == 0) {
					Bitmap.Bitmap[i].Type = GSM_CallerGroupLogo;
				} else if (strcasecmp(argv[4],"OPERATOR") == 0) {
					Bitmap.Bitmap[i].Type = GSM_OperatorLogo;
				} else {
					printf(_("What format of output file logo (\"%s\") ?\n"),argv[4]);
					exit(-1);
				}
			}
		}
		error=GSM_SaveBitmapFile(argv[3],&Bitmap);
		Print_Error(error);
	}
}

static void CopyRingtone(int argc, char *argv[])
{
	GSM_Ringtone 		ringtone, ringtone2;
	GSM_RingtoneFormat	Format;

	ringtone.Format	= 0;
	error=GSM_ReadRingtoneFile(argv[2],&ringtone);
	Print_Error(error);

	Format = ringtone.Format;
	if (argc == 5) {
		if (strcasecmp(argv[4],"RTTL") == 0) {		Format = RING_NOTETONE;
		} else if (strcasecmp(argv[4],"BINARY") == 0) {	Format = RING_NOKIABINARY;
		} else {
			printf(_("What format of output ringtone file (\"%s\") ?\n"),argv[4]);
			exit(-1);
		}
	}

	error=GSM_RingtoneConvert(&ringtone2,&ringtone,Format);
	Print_Error(error);

	error=GSM_SaveRingtoneFile(argv[3],&ringtone2);
	Print_Error(error);
}

static void PressKeySequence(int argc UNUSED, char *argv[])
{
	int 		i,Length;
	GSM_KeyCode	KeyCode[500];

	error = MakeKeySequence(argv[2], KeyCode, &Length);
	if (error == ERR_NOTSUPPORTED) {
		printf(_("Unknown key/function name: \"%c\"\n"),argv[2][Length]);
		exit(-1);
	}

	GSM_Init(true);

	for (i=0;i<Length;i++) {
		error=GSM_PressKey(s, KeyCode[i], true);
		Print_Error(error);
		error=GSM_PressKey(s, KeyCode[i], false);
		Print_Error(error);
	}

	GSM_Terminate();
}

static void GetAllCategories(int argc UNUSED, char *argv[])
{
	GSM_Category		Category;
	GSM_CategoryStatus	Status;
	int			j, count;

	if (strcasecmp(argv[2],"TODO") == 0) {
        	Category.Type 	= Category_ToDo;
        	Status.Type 	= Category_ToDo;
	} else if (strcasecmp(argv[2],"PHONEBOOK") == 0) {
	        Category.Type 	= Category_Phonebook;
        	Status.Type 	= Category_Phonebook;
	} else {
		printf(_("What type of categories do you want to get (\"%s\") ?\n"),argv[2]);
		exit(-1);
	}

	GSM_Init(true);

	error=GSM_GetCategoryStatus(s, &Status);
	Print_Error(error);

	for (count=0,j=1;count<Status.Used;j++)
	{
    		Category.Location=j;
		error=GSM_GetCategory(s, &Category);

		if (error != ERR_EMPTY) {
			printf(LISTFORMAT "%i\n", _("Location"),j);

			Print_Error(error);

			printf(LISTFORMAT "\"%s\"\n\n", _("Name"),DecodeUnicodeConsole(Category.Name));
			count++;
    		}
	}

	GSM_Terminate();
}

static void GetCategory(int argc, char *argv[])
{
	GSM_Category	Category;
	int		start,stop,j;

	if (strcasecmp(argv[2],"TODO") == 0) {
    		Category.Type = Category_ToDo;
	} else if (strcasecmp(argv[2],"PHONEBOOK") == 0) {
    		Category.Type = Category_Phonebook;
	} else {
		printf(_("What type of categories do you want to get (\"%s\") ?\n"),argv[2]);
		exit(-1);
	}

	GetStartStop(&start, &stop, 2, argc - 1, argv + 1);

	GSM_Init(true);

	for (j=start;j<=stop;j++)
	{
    		printf(LISTFORMAT "%i\n", _("Location"),j);

		Category.Location=j;

		error=GSM_GetCategory(s, &Category);
		if (error != ERR_EMPTY) Print_Error(error);

		if (error == ERR_EMPTY) {
			printf("%s\n", _("Entry is empty"));
		} else {
        		printf(LISTFORMAT "\"%s\"\n\n", _("Name"),DecodeUnicodeConsole(Category.Name));
    		}
	}

	GSM_Terminate();
}

static void AddCategory(int argc UNUSED, char *argv[])
{
	GSM_Category	Category;
	int		Length;

	if (strcasecmp(argv[2],"TODO") == 0) {
    		Category.Type = Category_ToDo;
	} else if (strcasecmp(argv[2],"PHONEBOOK") == 0) {
    		Category.Type = Category_Phonebook;
	} else {
		printf(_("What type of category do you want to add (\"%s\") ?\n"),argv[2]);
		exit(-1);
	}

	GSM_Init(true);

	Length = strlen(argv[3]);
	if (Length > GSM_MAX_CATEGORY_NAME_LENGTH) {
		printf(_("Text too long, truncating to %d chars!\n"), GSM_MAX_CATEGORY_NAME_LENGTH);
		Length = GSM_MAX_CATEGORY_NAME_LENGTH;
	}
	EncodeUnicode(Category.Name, argv[3], Length);

	Category.Location = 0;

	error = GSM_AddCategory(s, &Category);

	Print_Error(error);

	GSM_Terminate();
}

static void DeleteToDo(int argc, char *argv[])
{
	GSM_ToDoEntry	ToDo;
	int		i;
	int		start,stop;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		ToDo.Location=i;
		printf(LISTFORMAT "%i\n", _("Location"),i);
		error=GSM_DeleteToDo(s,&ToDo);
		if (error != ERR_EMPTY) Print_Error(error);

		if (error == ERR_EMPTY) {
			printf("%s\n", _("Entry was empty"));
		} else {
			printf("%s\n", _("Entry was deleted"));
	       	}
		printf("\n");
	}

	GSM_Terminate();
}

static void PrintToDo(GSM_ToDoEntry *ToDo)
{
	int			j;
	GSM_MemoryEntry		entry;
	unsigned char		*name;
	GSM_Category		Category;

	printf(LISTFORMAT "%i\n", _("Location"),ToDo->Location);
	printf(LISTFORMAT, _("Note type"));
	switch (ToDo->Type) {
		case GSM_CAL_REMINDER 	: printf("%s\n", _("Reminder (Date)"));		break;
		case GSM_CAL_CALL     	: printf("%s\n", _("Call"));			   	break;
		case GSM_CAL_MEETING  	: printf("%s\n", _("Meeting"));		   	break;
		case GSM_CAL_BIRTHDAY 	: printf("%s\n", _("Birthday (Anniversary)"));		break;
		case GSM_CAL_MEMO	: printf("%s\n", _("Memo (Miscellaneous)"));		break;
		case GSM_CAL_TRAVEL	: printf("%s\n", _("Travel"));			   	break;
		case GSM_CAL_VACATION	: printf("%s\n", _("Vacation"));			break;
		case GSM_CAL_ALARM    	: printf("%s\n", _("Alarm"));		   		break;
		case GSM_CAL_DAILY_ALARM: printf("%s\n", _("Daily alarm"));		   	break;
		case GSM_CAL_T_ATHL   	: printf("%s\n", _("Training/Athletism")); 	   	break;
		case GSM_CAL_T_BALL   	: printf("%s\n", _("Training/Ball Games")); 	   	break;
		case GSM_CAL_T_CYCL   	: printf("%s\n", _("Training/Cycling")); 	   	break;
		case GSM_CAL_T_BUDO   	: printf("%s\n", _("Training/Budo")); 	   		break;
		case GSM_CAL_T_DANC   	: printf("%s\n", _("Training/Dance")); 	   	break;
		case GSM_CAL_T_EXTR   	: printf("%s\n", _("Training/Extreme Sports")); 	break;
		case GSM_CAL_T_FOOT   	: printf("%s\n", _("Training/Football")); 	   	break;
		case GSM_CAL_T_GOLF   	: printf("%s\n", _("Training/Golf")); 	   		break;
		case GSM_CAL_T_GYM    	: printf("%s\n", _("Training/Gym")); 	   		break;
		case GSM_CAL_T_HORS   	: printf("%s\n", _("Training/Horse Races"));    	break;
		case GSM_CAL_T_HOCK   	: printf("%s\n", _("Training/Hockey")); 	  	break;
		case GSM_CAL_T_RACE   	: printf("%s\n", _("Training/Races")); 	   	break;
		case GSM_CAL_T_RUGB   	: printf("%s\n", _("Training/Rugby")); 	   	break;
		case GSM_CAL_T_SAIL   	: printf("%s\n", _("Training/Sailing")); 	   	break;
		case GSM_CAL_T_STRE   	: printf("%s\n", _("Training/Street Games"));   	break;
		case GSM_CAL_T_SWIM   	: printf("%s\n", _("Training/Swimming")); 	   	break;
		case GSM_CAL_T_TENN   	: printf("%s\n", _("Training/Tennis")); 	   	break;
		case GSM_CAL_T_TRAV   	: printf("%s\n", _("Training/Travels"));        	break;
		case GSM_CAL_T_WINT   	: printf("%s\n", _("Training/Winter Games"));   	break;
#ifndef CHECK_CASES
		default           	: printf("%s\n", _("Unknown"));
#endif
	}
	printf(LISTFORMAT, _("Priority"));
	switch (ToDo->Priority) {
		case GSM_Priority_Low	 : printf("%s\n", _("Low"));	 	break;
		case GSM_Priority_Medium : printf("%s\n", _("Medium")); 	break;
		case GSM_Priority_High	 : printf("%s\n", _("High"));		break;
		case GSM_Priority_None	 : printf("%s\n", _("None"));		break;
#ifndef CHECK_CASES
		default			 : printf("%s\n", _("Unknown"));	break;
#endif
	}
	for (j=0;j<ToDo->EntriesNum;j++) {
		switch (ToDo->Entries[j].EntryType) {
		case TODO_END_DATETIME:
			printf(LISTFORMAT "%s\n", _("Due time"),OSDateTime(ToDo->Entries[j].Date,false));
			break;
		case TODO_COMPLETED:
			printf(LISTFORMAT "%s\n", _("Completed"),ToDo->Entries[j].Number == 1 ? "Yes" : "No");
			break;
		case TODO_ALARM_DATETIME:
			printf(LISTFORMAT "%s\n", _("Alarm"),OSDateTime(ToDo->Entries[j].Date,false));
			break;
		case TODO_SILENT_ALARM_DATETIME:
			printf(LISTFORMAT "%s\n", _("Silent alarm"),OSDateTime(ToDo->Entries[j].Date,false));
			break;
		case TODO_LAST_MODIFIED:
			printf(LISTFORMAT "%s\n", _("Last modified"),OSDateTime(ToDo->Entries[j].Date,false));
			break;
		case TODO_TEXT:
			printf(LISTFORMAT "\"%s\"\n", _("Text"),DecodeUnicodeConsole(ToDo->Entries[j].Text));
			break;
		case TODO_PRIVATE:
			printf(LISTFORMAT "%s\n", _("Private"),ToDo->Entries[j].Number == 1 ? "Yes" : "No");
			break;
		case TODO_CATEGORY:
			Category.Location = ToDo->Entries[j].Number;
			Category.Type = Category_ToDo;
			error=GSM_GetCategory(s, &Category);
			if (error == ERR_NONE) {
				printf(LISTFORMAT "\"%s\" (%i)\n", _("Category"), DecodeUnicodeConsole(Category.Name), ToDo->Entries[j].Number);
			} else {
				printf(LISTFORMAT "%i\n", _("Category"), ToDo->Entries[j].Number);
			}
			break;
		case TODO_CONTACTID:
			entry.Location = ToDo->Entries[j].Number;
			entry.MemoryType = MEM_ME;
			error=GSM_GetMemory(s, &entry);
			if (error == ERR_NONE) {
				name = GSM_PhonebookGetEntryName(&entry);
				if (name != NULL) {
					printf(LISTFORMAT "\"%s\" (%d)\n", _("Contact ID"), DecodeUnicodeConsole(name), ToDo->Entries[j].Number);
				} else {
					printf(LISTFORMAT "%d\n", _("Contact ID"),ToDo->Entries[j].Number);
				}
			} else {
				printf(LISTFORMAT "%d\n", _("Contact"),ToDo->Entries[j].Number);
			}
			break;
		case TODO_PHONE:
			printf(LISTFORMAT "\"%s\"\n", _("Phone"),DecodeUnicodeConsole(ToDo->Entries[j].Text));
			break;
		case TODO_DESCRIPTION:
			printf(LISTFORMAT "\"%s\"\n", _("Description"),DecodeUnicodeConsole(ToDo->Entries[j].Text));
			break;
		case TODO_LOCATION:
			printf(LISTFORMAT "\"%s\"\n", _("Location"),DecodeUnicodeConsole(ToDo->Entries[j].Text));
			break;
		case TODO_LUID:
			printf(LISTFORMAT "\"%s\"\n", _("LUID"),DecodeUnicodeConsole(ToDo->Entries[j].Text));
			break;
		}
	}
	printf("\n");
}

static void ListToDoCategoryEntries(int Category)
{
	GSM_ToDoEntry		Entry;
	bool			start = true;
	int			j;

	while (!gshutdown) {
		error = GSM_GetNextToDo(s, &Entry, start);
		if (error == ERR_EMPTY) break;
		Print_Error(error);
		for (j=0;j<Entry.EntriesNum;j++) {
			if (Entry.Entries[j].EntryType == TODO_CATEGORY && Entry.Entries[j].Number == (unsigned int)Category)
				PrintToDo(&Entry);
		}
 		start = false;
	}
}

static void ListToDoCategory(int argc UNUSED, char *argv[])
{
	GSM_Category		Category;
	GSM_CategoryStatus	Status;
	int			j, count;

	unsigned char		Text[(GSM_MAX_CATEGORY_NAME_LENGTH+1)*2];
	int			Length;
	bool			Number = true;;

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
			printf(_("Search text too long, truncating to %d chars!\n"), GSM_MAX_CATEGORY_NAME_LENGTH);
			Length = GSM_MAX_CATEGORY_NAME_LENGTH;
		}
		EncodeUnicode(Text, argv[2], Length);

		Category.Type 	= Category_ToDo;
		Status.Type 	= Category_ToDo;

		if (GSM_GetCategoryStatus(s, &Status) == ERR_NONE) {
			for (count=0,j=1;count<Status.Used;j++) {
				Category.Location=j;
				error=GSM_GetCategory(s, &Category);

				if (error != ERR_EMPTY) {
					count++;
					if (mywstrstr(Category.Name, Text) != NULL) {
						ListToDoCategoryEntries(j);
					}
				}
			}
		}
	}
	GSM_Terminate();
}


static void GetToDo(int argc, char *argv[])
{
	GSM_ToDoEntry	ToDo;
	int		i;
	int		start,stop;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		ToDo.Location=i;
		error = GSM_GetToDo(s,&ToDo);
		if (error == ERR_EMPTY) continue;
		Print_Error(error);
		PrintToDo(&ToDo);
	}

	GSM_Terminate();
}

static void GetAllToDo(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_ToDoEntry		ToDo;
	bool			start = true;

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));

	GSM_Init(true);

	while (!gshutdown) {
		error = GSM_GetNextToDo(s, &ToDo, start);
		if (error == ERR_EMPTY) break;
		Print_Error(error);
		PrintToDo(&ToDo);
 		start = false;
	}

	GSM_Terminate();
}

static void GetAllNotes(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_NoteEntry		Note;
	bool			start = true;

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));

	GSM_Init(true);

	while (!gshutdown) {
		error = GSM_GetNextNote(s, &Note, start);
		if (error == ERR_EMPTY) break;
		Print_Error(error);
		printf(LISTFORMAT "\"%s\"\n", _("Text"),DecodeUnicodeConsole(Note.Text));
       	    	printf("\n");
 		start = false;
	}
	GSM_Terminate();
}

static void GetSecurityStatus(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_Init(true);

	PrintSecurityStatus();

	GSM_Terminate();
}

static void EnterSecurityCode(int argc UNUSED, char *argv[])
{
	GSM_SecurityCode Code;

	if (strcasecmp(argv[2],"PIN") == 0) {		Code.Type = SEC_Pin;
	} else if (strcasecmp(argv[2],"PUK") == 0) {	Code.Type = SEC_Puk;
	} else if (strcasecmp(argv[2],"PIN2") == 0) {	Code.Type = SEC_Pin2;
	} else if (strcasecmp(argv[2],"PUK2") == 0) {	Code.Type = SEC_Puk2;
	} else {
		printf(_("What security code (\"%s\") ?\n"),argv[2]);
		exit(-1);
	}

	strcpy(Code.Code,argv[3]);

	GSM_Init(true);

	error=GSM_EnterSecurityCode(s,Code);
	Print_Error(error);

	GSM_Terminate();
}

static void GetProfile(int argc, char *argv[])
{
	GSM_Profile 		Profile;
	int			start,stop,j,k;
	GSM_Bitmap		caller[5];
	bool			callerinit[5],special;
 	GSM_AllRingtonesInfo 	Info = {0, NULL};

	GetStartStop(&start, &stop, 2, argc, argv);

	for (i=0;i<5;i++) callerinit[i] = false;

	GSM_Init(true);

	error=GSM_GetRingtonesInfo(s,&Info);
	if (error != ERR_NONE) Info.Number = 0;

	for (i=start;i<=stop;i++) {
		Profile.Location=i;
		error=GSM_GetProfile(s,&Profile);
		if (error != ERR_NONE && Info.Ringtone) free(Info.Ringtone);
		Print_Error(error);

		printf("%i. \"%s\"",i,DecodeUnicodeConsole(Profile.Name));
		if (Profile.Active)		printf(_(" (active)"));
		if (Profile.DefaultName) 	printf(_(" (default name)"));
		if (Profile.HeadSetProfile) 	printf(_(" (Head set profile)"));
		if (Profile.CarKitProfile) 	printf(_(" (Car kit profile)"));
		printf("\n");
		for (j=0;j<Profile.FeaturesNumber;j++) {
			special = false;
			switch (Profile.FeatureID[j]) {
			case Profile_MessageToneID:
			case Profile_RingtoneID:
				special = true;
				if (Profile.FeatureID[j] == Profile_RingtoneID) {
					printf(LISTFORMAT, _("Ringtone ID"));
				} else {
					printf(LISTFORMAT, _("Message alert tone ID"));
				}
				if (UnicodeLength(GSM_GetRingtoneName(&Info,Profile.FeatureValue[j]))!=0) {
					printf("\"%s\"\n",DecodeUnicodeConsole(GSM_GetRingtoneName(&Info,Profile.FeatureValue[j])));
				} else {
					printf("%i\n",Profile.FeatureValue[j]);
				}
				break;
			case Profile_CallerGroups:
				special = true;
				printf(LISTFORMAT, _("Call alert for"));
				for (k=0;k<5;k++) {
					if (Profile.CallerGroups[k]) {
						if (!callerinit[k]) {
							caller[k].Type	 	= GSM_CallerGroupLogo;
							caller[k].Location 	= k + 1;
							error=GSM_GetBitmap(s,&caller[k]);
							if (error == ERR_SECURITYERROR) {
								NOKIA_GetDefaultCallerGroupName(&caller[k]);
							} else {
								Print_Error(error);
							}
							callerinit[k]	= true;
						}
						printf(" \"%s\"",DecodeUnicodeConsole(caller[k].Text));
					}
				}
				printf("\n");
				break;
			case Profile_ScreenSaverNumber:
				special = true;
				printf(LISTFORMAT, _("Screen saver number"));
				printf("%i\n",Profile.FeatureValue[j]);
				break;
			case Profile_CallAlert  	: printf(LISTFORMAT, _("Incoming call alert")); break;
			case Profile_RingtoneVolume 	: printf(LISTFORMAT, _("Ringtone volume")); break;
			case Profile_Vibration		: printf(LISTFORMAT, _("Vibrating alert")); break;
			case Profile_MessageTone	: printf(LISTFORMAT, _("Message alert tone")); break;
			case Profile_KeypadTone		: printf(LISTFORMAT, _("Keypad tones")); break;
			case Profile_WarningTone	: printf(LISTFORMAT, _("Warning (games) tones")); break;
			case Profile_ScreenSaver	: printf(LISTFORMAT, _("Screen saver")); break;
			case Profile_ScreenSaverTime	: printf(LISTFORMAT, _("Screen saver timeout")); break;
			case Profile_AutoAnswer		: printf(LISTFORMAT, _("Automatic answer")); break;
			case Profile_Lights		: printf(LISTFORMAT, _("Lights")); break;
			default:
				printf("%s\n", _("Unknown"));
				special = true;
			}
			if (!special) {
				switch (Profile.FeatureValue[j]) {
				case PROFILE_VOLUME_LEVEL1 		:
				case PROFILE_KEYPAD_LEVEL1 		: printf("%s\n", _("Level 1")); 		break;
				case PROFILE_VOLUME_LEVEL2 		:
				case PROFILE_KEYPAD_LEVEL2 		: printf("%s\n", _("Level 2"));		break;
				case PROFILE_VOLUME_LEVEL3 		:
				case PROFILE_KEYPAD_LEVEL3 		: printf("%s\n", _("Level 3")); 		break;
				case PROFILE_VOLUME_LEVEL4 		: printf("%s\n", _("Level 4")); 		break;
				case PROFILE_VOLUME_LEVEL5 		: printf("%s\n", _("Level 5")); 		break;
				case PROFILE_MESSAGE_NOTONE 		:
				case PROFILE_AUTOANSWER_OFF		:
				case PROFILE_LIGHTS_OFF  		:
				case PROFILE_SAVER_OFF			:
				case PROFILE_WARNING_OFF		:
				case PROFILE_CALLALERT_OFF	 	:
				case PROFILE_VIBRATION_OFF 		:
				case PROFILE_KEYPAD_OFF	   		: printf("%s\n", _("Off"));	  	break;
				case PROFILE_CALLALERT_RINGING   	: printf("%s\n", _("Ringing"));		break;
				case PROFILE_CALLALERT_BEEPONCE  	:
				case PROFILE_MESSAGE_BEEPONCE 		: printf("%s\n", _("Beep once")); 	break;
				case PROFILE_CALLALERT_RINGONCE  	: printf("%s\n", _("Ring once"));	break;
				case PROFILE_CALLALERT_ASCENDING 	: printf("%s\n", _("Ascending"));        break;
				case PROFILE_CALLALERT_CALLERGROUPS	: printf("%s\n", _("Caller groups"));	break;
				case PROFILE_MESSAGE_STANDARD 		: printf("%s\n", _("Standard"));  	break;
				case PROFILE_MESSAGE_SPECIAL 		: printf("%s\n", _("Special"));	 	break;
				case PROFILE_MESSAGE_ASCENDING		: printf("%s\n", _("Ascending")); 	break;
				case PROFILE_MESSAGE_PERSONAL		: printf("%s\n", _("Personal"));		break;
				case PROFILE_AUTOANSWER_ON		:
				case PROFILE_WARNING_ON			:
				case PROFILE_SAVER_ON			:
				case PROFILE_VIBRATION_ON 		: printf("%s\n", _("On"));  		break;
				case PROFILE_VIBRATION_FIRST 		: printf("%s\n", _("Vibrate first"));	break;
				case PROFILE_LIGHTS_AUTO 		: printf("%s\n", _("Auto")); 		break;
				case PROFILE_SAVER_TIMEOUT_5SEC	 	: PRINTSECONDS(5); printf("\n"); break;
				case PROFILE_SAVER_TIMEOUT_20SEC 	: PRINTSECONDS(20); printf("\n"); break;
				case PROFILE_SAVER_TIMEOUT_1MIN	 	: PRINTMINUTES(1); printf("\n"); break;
				case PROFILE_SAVER_TIMEOUT_2MIN	 	: PRINTMINUTES(2); printf("\n"); break;
				case PROFILE_SAVER_TIMEOUT_5MIN	 	: PRINTMINUTES(5); printf("\n"); break;
				case PROFILE_SAVER_TIMEOUT_10MIN 	: PRINTMINUTES(10); printf("\n"); break;
				default					: printf("%s\n", _("Unknown"));
				}
			}
		}
		printf("\n");
	}

	GSM_Terminate();

	if (Info.Ringtone) free(Info.Ringtone);
}

static void GetSpeedDial(int argc, char *argv[])
{
	GSM_SpeedDial		SpeedDial;
	GSM_MemoryEntry		Phonebook;
	int			start,stop,Name,Number,Group;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		SpeedDial.Location=i;
		error=GSM_GetSpeedDial(s,&SpeedDial);
		printf(LISTFORMAT "%i\n", _("Location"), i);
		switch (error) {
		case ERR_EMPTY:
			printf("%s\n", _(" speed dial not assigned"));
			break;
		default:
			Print_Error(error);

			Phonebook.Location	= SpeedDial.MemoryLocation;
			Phonebook.MemoryType 	= SpeedDial.MemoryType;
			error=GSM_GetMemory(s,&Phonebook);

			GSM_PhonebookFindDefaultNameNumberGroup(&Phonebook, &Name, &Number, &Group);

			if (Name != -1) printf(LISTFORMAT "\"%s\"\n", _("Name"), DecodeUnicodeConsole(Phonebook.Entries[Name].Text));
			printf(LISTFORMAT "\"%s\"\"", _("Number"), DecodeUnicodeConsole(Phonebook.Entries[SpeedDial.MemoryNumberID-1].Text));
		}
		printf("\n");
	}

	GSM_Terminate();
}

static void ResetPhoneSettings(int argc UNUSED, char *argv[])
{
	GSM_ResetSettingsType Type;

	if (strcasecmp(argv[2],"PHONE") == 0) {		 Type = GSM_RESET_PHONESETTINGS;
	} else if (strcasecmp(argv[2],"UIF") == 0) {	 Type = GSM_RESET_USERINTERFACE;
	} else if (strcasecmp(argv[2],"ALL") == 0) {	 Type = GSM_RESET_USERINTERFACE_PHONESETTINGS;
	} else if (strcasecmp(argv[2],"DEV") == 0) {	 Type = GSM_RESET_DEVICE;
	} else if (strcasecmp(argv[2],"FACTORY") == 0) { Type = GSM_RESET_FULLFACTORY;
	} else {
		printf(_("What type of reset phone settings (\"%s\") ?\n"),argv[2]);
		exit(-1);
	}

	GSM_Init(true);

	error=GSM_ResetPhoneSettings(s,Type);
	Print_Error(error);

 	GSM_Terminate();
}

static void DeleteAllSMS(int argc, char *argv[])
{
	GSM_MultiSMSMessage 	sms;
	GSM_SMSFolders		folders;
	int			foldernum;
	bool			start = true;

	GSM_Init(true);

	error=GSM_GetSMSFolders(s, &folders);
	Print_Error(error);

	GetStartStop(&foldernum, NULL, 2, argc, argv);
	if (foldernum > folders.Number) {
		printf(_("Too high folder number (max. %i)\n"),folders.Number);
		GSM_Terminate();
		exit(-1);
	}

	printf(_("Deleting SMS from \"%s\" folder: "),DecodeUnicodeConsole(folders.Folder[foldernum-1].Name));

	while (error == ERR_NONE) {
		sms.SMS[0].Folder=0x00;
		error=GSM_GetNextSMS(s, &sms, start);
		switch (error) {
		case ERR_EMPTY:
			break;
		default:
			Print_Error(error);
			if (sms.SMS[0].Folder == foldernum) {
				sms.SMS[0].Folder=0x00;
				error=GSM_DeleteSMS(s, &sms.SMS[0]);
				Print_Error(error);
				printf("*");
			}
		}
		start=false;
	}

	printf("\n");

	GSM_Terminate();
}

static void SendDTMF(int argc UNUSED, char *argv[])
{
	GSM_Init(true);

	error=GSM_SendDTMF(s,argv[2]);
	Print_Error(error);

 	GSM_Terminate();
}

static void GetDisplayStatus(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_DisplayFeatures 	Features;
	int			i;

	GSM_Init(true);

	error=GSM_GetDisplayStatus(s,&Features);
	Print_Error(error);

	printf(LISTFORMAT "\n", _("Current display features"));

	for (i=0;i<Features.Number;i++) {
		switch(Features.Feature[i]) {
		case GSM_CallActive	: printf("%s\n", _("Call active"));	break;
		case GSM_UnreadSMS	: printf("%s\n", _("Unread SMS"));	break;
		case GSM_VoiceCall	: printf("%s\n", _("Voice call"));	break;
		case GSM_FaxCall	: printf("%s\n", _("Fax call"));	break;
		case GSM_DataCall	: printf("%s\n", _("Data call"));	break;
		case GSM_KeypadLocked	: printf("%s\n", _("Keypad locked"));	break;
		case GSM_SMSMemoryFull	: printf("%s\n", _("SMS memory full"));
		}
	}

 	GSM_Terminate();
}

static void SetAutoNetworkLogin(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_Init(true);

	error=GSM_SetAutoNetworkLogin(s);
	Print_Error(error);

 	GSM_Terminate();
}

#ifdef DEBUG
static void MakeConvertTable(int argc UNUSED, char *argv[])
{
	unsigned char 	InputBuffer[10000], Buffer[10000];
	FILE		*file;
	int		size,i,j=0;

	file = fopen(argv[2], "rb");
	if (file == NULL) Print_Error(ERR_CANTOPENFILE);
	size=fread(InputBuffer, 1, 10000-1, file);
	fclose(file);
	InputBuffer[size]   = 0;
	InputBuffer[size+1] = 0;

	ReadUnicodeFile(Buffer,InputBuffer);

	for(i=0;i<((int)UnicodeLength(Buffer));i++) {
		j++;
		if (j==100) {
			printf("\"\\\n\"");
			j=0;
		}
		printf("\\x%02x\\x%02x",Buffer[i*2],Buffer[i*2+1]);
	}
	printf("\\x00\\x00");
}
#endif

static void ListNetworks(int argc, char *argv[])
{
	extern unsigned char 	*GSM_Networks[];
	extern unsigned char 	*GSM_Countries[];
	int 			i=0;
	char			country[4]="";

	if (argc>2) {
		while (GSM_Countries[i*2]!=NULL) {
			if (strncmp(GSM_Countries[i * 2 + 1], argv[2], strlen(argv[2])) == 0 ||
				strncmp(GSM_Countries[i * 2], argv[2], strlen(argv[2])) == 0) {
				strcpy(country,GSM_Countries[i*2]);
				printf(_("Networks for %s:"), GSM_Countries[i * 2 + 1]);
				printf("\n\n");
				break;
			}
			i++;
		}
		if (!*country) {
			printf(_("Unknown country name: %s."), argv[2]);
			printf("\n");
			exit(-1);
		}
	}
	printf("%-10s %s\n", _("Network"), _("Name"));
	i=0;
	while (GSM_Networks[i*2]!=NULL) {
		if (argc>2) {
		        if (!strncmp(GSM_Networks[i*2],country,strlen(country))) {
				printf("%-10s %s\n", GSM_Networks[i*2], GSM_Networks[i*2+1]);
			}
		} else {
			printf("%-10s %s\n", GSM_Networks[i*2], GSM_Networks[i*2+1]);
		}
		i++;
	}
}


static void PrintVersion()
{
	printf(_("[Gammu version %s built %s on %s using %s]"),
		VERSION,
		__TIME__,
		__DATE__,
		GetCompiler());
	printf("\n\n");
}

static void Features(int argc UNUSED, char *argv[] UNUSED)
{
	PrintVersion();

	printf("%s\n", _("Compiled in features:"));

	printf(" * %s\n", _("Protocols"));
#ifdef GSM_ENABLE_MBUS2
	printf("  - %s\n", "MBUS2");
#endif
#ifdef GSM_ENABLE_FBUS2
	printf("  - %s\n", "FBUS2");
#endif
#ifdef GSM_ENABLE_FBUS2DLR3
	printf("  - %s\n", "FBUS2DLR3");
#endif
#ifdef GSM_ENABLE_FBUS2PL2303
	printf("  - %s\n", "FBUS2PL2303");
#endif
#ifdef GSM_ENABLE_FBUS2BLUE
	printf("  - %s\n", "FBUS2BLUE");
#endif
#ifdef GSM_ENABLE_FBUS2IRDA
	printf("  - %s\n", "FBUS2IRDA");
#endif
#ifdef GSM_ENABLE_DKU2PHONET
	printf("  - %s\n", "DKU2PHONET");
#endif
#ifdef GSM_ENABLE_DKU2AT
	printf("  - %s\n", "DKU2AT");
#endif
#ifdef GSM_ENABLE_DKU5FBUS2
	printf("  - %s\n", "DKU5FBUS2");
#endif
#ifdef GSM_ENABLE_PHONETBLUE
	printf("  - %s\n", "PHONETBLUE");
#endif
#ifdef GSM_ENABLE_AT
	printf("  - %s\n", "AT");
#endif
#ifdef GSM_ENABLE_ALCABUS
	printf("  - %s\n", "ALCABUS");
#endif
#ifdef GSM_ENABLE_IRDAPHONET
	printf("  - %s\n", "IRDAPHONET");
#endif
#ifdef GSM_ENABLE_IRDAAT
	printf("  - %s\n", "IRDAAT");
#endif
#ifdef GSM_ENABLE_IRDAOBEX
	printf("  - %s\n", "IRDAOBEX");
#endif
#ifdef GSM_ENABLE_IRDAGNAPBUS
	printf("  - %s\n", "IRDAGNAPBUS");
#endif
#ifdef GSM_ENABLE_BLUEGNAPBUS
	printf("  - %s\n", "BLUEGNAPBUS");
#endif
#ifdef GSM_ENABLE_BLUEFBUS2
	printf("  - %s\n", "BLUEFBUS2");
#endif
#ifdef GSM_ENABLE_BLUEPHONET
	printf("  - %s\n", "BLUEPHONET");
#endif
#ifdef GSM_ENABLE_BLUEAT
	printf("  - %s\n", "BLUEAT");
#endif
#ifdef GSM_ENABLE_BLUEOBEX
	printf("  - %s\n", "BLUEOBEX");
#endif

	printf(" * %s\n", _("Phones"));
#ifdef GSM_ENABLE_NOKIA650
	printf("  - %s\n", "NOKIA650");
#endif
#ifdef GSM_ENABLE_NOKIA3320
	printf("  - %s\n", "NOKIA3320");
#endif
#ifdef GSM_ENABLE_NOKIA6110
	printf("  - %s\n", "NOKIA6110");
#endif
#ifdef GSM_ENABLE_NOKIA7110
	printf("  - %s\n", "NOKIA7110");
#endif
#ifdef GSM_ENABLE_NOKIA9210
	printf("  - %s\n", "NOKIA9210");
#endif
#ifdef GSM_ENABLE_NOKIA6510
	printf("  - %s\n", "NOKIA6510");
#endif
#ifdef GSM_ENABLE_NOKIA3650
	printf("  - %s\n", "NOKIA3650");
#endif
#ifdef GSM_ENABLE_NOKIA_DCT3
	printf("  - %s\n", "DCT3");
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
	printf("  - %s\n", "DCT4");
#endif
#ifdef GSM_ENABLE_ATGEN
	printf("  - %s\n", "ATGEN");
#endif
#ifdef GSM_ENABLE_ALCATEL
	printf("  - %s\n", "ALCATEL");
#endif
#ifdef GSM_ENABLE_SONYERICSSON
	printf("  - %s\n", "SONYERICSSON");
#endif
#ifdef GSM_ENABLE_OBEXGEN
	printf("  - %s\n", "OBEXGEN");
#endif
#ifdef GSM_ENABLE_GNAPGEN
	printf("  - %s\n", "GNAPGEN");
#endif

	printf(" * %s\n", _("Miscellaneous"));
#ifdef GSM_ENABLE_CELLBROADCAST
	printf("  - %s\n", "CELLBROADCAST");
#endif
#ifdef GSM_ENABLE_BACKUP
	printf("  - %s\n", "BACKUP");
#endif
#ifdef GETTEXTLIBS_FOUND
	printf("  - %s\n", "GETTEXT");
#endif
#ifdef ICONV_FOUND
	printf("  - %s\n", "ICONV");
#endif
#ifdef HAVE_MYSQL_MYSQL_H
	printf("  - %s\n", "MYSQL");
#endif
#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
	printf("  - %s\n", "POSTGRESQL");
#endif
}

static void Version(int argc UNUSED, char *argv[] UNUSED)
{
	PrintVersion();

	printf("%s\n", _("This is free software.  You may redistribute copies of it under the terms of"));
	printf("%s\n", _("the GNU General Public License <http://www.gnu.org/licenses/gpl.html>."));
	printf("%s\n", _("There is NO WARRANTY, to the extent permitted by law."));
	printf("\n\n");
}

static void GetFMStation(int argc, char *argv[])
{
	GSM_FMStation 	Station;
	int		start,stop;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		Station.Location=i;
		error=GSM_GetFMStation(s,&Station);
		printf(LISTFORMAT "%i\n", _("Location"), i);
		switch (error) {
		case ERR_EMPTY:
			printf("%s\n", _("Entry is empty"));
		    	break;
		case ERR_NONE:
			printf(LISTFORMAT "\"%s\"\n", _("Station name"), DecodeUnicodeConsole(Station.StationName));
			printf(LISTFORMAT "%.1f MHz\n", _("Frequency"), Station.Frequency);
			break;
		default:
			Print_Error(error);
		}
	}
	GSM_Terminate();
}

/**
 * Displays status of filesystem (if available).
 */
GSM_Error PrintFileSystemStatus()
{
	GSM_FileSystemStatus	Status;

	error = GSM_GetFileSystemStatus(s,&Status);
	if (error != ERR_NOTSUPPORTED && error != ERR_NOTIMPLEMENTED) {
	    	Print_Error(error);
		printf("\n");
		printf(_("Phone memory: %i bytes (free %i bytes, used %i bytes)"),Status.Free+Status.Used,Status.Free,Status.Used);
		printf("\n");
		if (Status.UsedImages != 0 || Status.UsedSounds != 0 || Status.UsedThemes != 0) {
			printf(_("Used by: Images: %i, Sounds: %i, Themes: %i"), Status.UsedImages, Status.UsedSounds, Status.UsedThemes);
			printf("\n");
		}
	}
	return error;
}

static void GetFileSystemStatus(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_Init(true);

	PrintFileSystemStatus();

	GSM_Terminate();
}

static void GetFileSystem(int argc, char *argv[])
{
	bool 			Start = true, MemoryCard = false;
	GSM_File	 	Files;
	int			j;
	long			usedphone=0,usedcard=0;
	char 			FolderName[256],IDUTF[200];

	GSM_Init(true);

	while (1) {
		error = GSM_GetNextFileFolder(s,&Files,Start);
		if (error == ERR_EMPTY) break;
	    	if (error != ERR_FOLDERPART) Print_Error(error);

		if (!Files.Folder) {
			if (GSM_IsPhoneFeatureAvailable(GSM_GetModelInfo(s), F_FILES2)) {
				if (DecodeUnicodeString(Files.ID_FullName)[0] == 'a') {
					MemoryCard = true;
					usedcard+=Files.Used;
				} else {
					usedphone+=Files.Used;
				}
			} else {
				usedphone+=Files.Used;
			}
		}

		if (argc <= 2 || !strcasecmp(argv[2],"-flatall") == 0) {
			//Nokia filesystem 1
			if (UnicodeLength(Files.ID_FullName) != 0 &&
			    (DecodeUnicodeString(Files.ID_FullName)[0]=='C' ||
			    DecodeUnicodeString(Files.ID_FullName)[0]=='c')) {
				printf("%8s.",DecodeUnicodeString(Files.ID_FullName));
			}
			if (Files.Protected) {
				/* l10n: One char to indicate protected file */
				printf(_("P"));
			} else {
				printf(" ");
			}
			if (Files.ReadOnly) {
				/* l10n: One char to indicate read only file */
				printf(_("R"));
			} else {
				printf(" ");
			}
			if (Files.Hidden) {
				/* l10n: One char to indicate hidden file */
				printf(_("H"));
			} else {
				printf(" ");
			}
			if (Files.System) {
				/* l10n: One char to indicate system file */
				printf(_("S"));
			} else {
				printf(" ");
			}
			if (argc > 2 &&  strcasecmp(argv[2],"-flat") == 0) {
				if (!Files.Folder) {
					if (strcasecmp(argv[2],"-flatall") == 0) {
						if (!Files.ModifiedEmpty) {
							printf(" %30s",OSDateTime(Files.Modified,false));
						} else printf(" %30c",0x20);
						printf(" %9zi",Files.Used);
						printf(" ");
					} else printf("|-- ");
				} else {
					if (error == ERR_FOLDERPART) {
						printf(_("Part of folder "));
					} else {
						printf(_("Folder "));
					}
				}
			} else {
				if (Files.Level != 1) {
					for (j=0;j<Files.Level-2;j++) printf(" |   ");
					printf(" |-- ");
				}
				if (Files.Folder) {
					if (error == ERR_FOLDERPART) {
						printf(_("Part of folder "));
					} else {
						printf(_("Folder "));
					}
				}
			}
			printf("\"%s\"\n",DecodeUnicodeConsole(Files.Name));
		} else if (argc > 2 && strcasecmp(argv[2],"-flatall") == 0) {
			/* format for a folder ID;Folder;FOLDER_NAME;[FOLDER_PARAMETERS]
			 * format for a file   ID;File;FOLDER_NAME;FILE_NAME;DATESTAMP;FILE_SIZE;[FILE_PARAMETERS]  */
			EncodeUTF8QuotedPrintable(IDUTF,Files.ID_FullName);
			printf("%s;",IDUTF);
			if (!Files.Folder) {
				printf(_("File;"));
				printf("\"%s\";",FolderName);
				printf("\"%s\";",DecodeUnicodeConsole(Files.Name));
				if (!Files.ModifiedEmpty) {
					printf("\"%s\";",OSDateTime(Files.Modified,false));
				} else  printf("\"%c\";",0x20);
				printf("%zi;",Files.Used);
			} else {
				if (error == ERR_FOLDERPART) {
					printf(_("Part of folder;"));
				} else {
					printf(_("Folder;"));
				}
				printf("\"%s\";",DecodeUnicodeConsole(Files.Name));
				strcpy(FolderName,DecodeUnicodeConsole(Files.Name));
			}

			if (Files.Protected)  	printf(_("P"));
			if (Files.ReadOnly)  	printf(_("R"));
			if (Files.Hidden)  	printf(_("H"));
			if (Files.System)  	printf(_("S"));
			printf("\n");
		}
		Start = false;
	}

	error = PrintFileSystemStatus();

	if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
		printf("\n");
		printf(_("Used in phone: %li bytes"),usedphone);
		if (MemoryCard) printf(_(", used in card: %li bytes"),usedcard);
		printf("\n");
	}

	GSM_Terminate();
}

static void SetFileAttrib(int argc, char *argv[])
{
	GSM_File	 	Files;
	int			i;

	Files.ReadOnly  = false;
	Files.Protected = false;
	Files.System    = false;
	Files.Hidden    = false;

	DecodeUTF8QuotedPrintable(Files.ID_FullName,argv[2],strlen(argv[2]));

	for (i=3;i<argc;i++) {
		if (strcasecmp(argv[i],"-readonly") == 0) {
			Files.ReadOnly = true;
		} else if (strcasecmp(argv[i],"-protected") == 0) {
			Files.Protected = true;
		} else if (strcasecmp(argv[i],"-system") == 0) {
			Files.System = true;
		} else if (strcasecmp(argv[i],"-hidden") == 0) {
			Files.Hidden = true;
		} else {
			fprintf(stderr, _("Unknown attribute (%s)\n"),argv[i]);
		}
	}

	GSM_Init(true);

	error = GSM_SetFileAttributes(s,&Files);
    	Print_Error(error);

	GSM_Terminate();
}

static void GetRootFolders(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_File 	File;
	char 		IDUTF[200];

	GSM_Init(true);

	File.ID_FullName[0] = 0;
	File.ID_FullName[1] = 0;

	while (1) {
		if (GSM_GetNextRootFolder(s,&File)!=ERR_NONE) break;
		EncodeUTF8QuotedPrintable(IDUTF,File.ID_FullName);
		printf("%s ",IDUTF);
		printf("- %s\n",DecodeUnicodeString(File.Name));
	}

	GSM_Terminate();
}

static void GetFolderListing(int argc UNUSED, char *argv[])
{
	bool 			Start = true;
	GSM_File	 	Files;
	char 			IDUTF[200];

	GSM_Init(true);

	DecodeUTF8QuotedPrintable(Files.ID_FullName,argv[2],strlen(argv[2]));

	while (1) {
		error = GSM_GetFolderListing(s,&Files,Start);
		if (error == ERR_EMPTY) break;
		if (error != ERR_FOLDERPART) {
			Print_Error(error);
		} else {
			printf("%s\n\n", _("Part of folder only"));
		}

		/* format for a folder ID;Folder;[FOLDER_PARAMETERS]
		 * format for a file   ID;File;FILE_NAME;DATESTAMP;FILE_SIZE;[FILE_PARAMETERS]  */
		EncodeUTF8QuotedPrintable(IDUTF,Files.ID_FullName);
		printf("%s;",IDUTF);
		if (!Files.Folder) {
			printf(_("File;"));
			printf("\"%s\";",DecodeUnicodeConsole(Files.Name));
			if (!Files.ModifiedEmpty) {
				printf("\"%s\";",OSDateTime(Files.Modified,false));
			} else  printf("\"%c\";",0x20);
			printf("%zi;",Files.Used);
		} else {
			printf(_("Folder"));
			printf(";\"%s\";",DecodeUnicodeConsole(Files.Name));
		}

		if (Files.Protected)  	printf(_("P"));
		if (Files.ReadOnly)  	printf(_("R"));
		if (Files.Hidden)  	printf(_("H"));
		if (Files.System)  	printf(_("S"));
		printf("\n");

		Start = false;
	}

	GSM_Terminate();
}

static void GetOneFile(GSM_File *File, bool newtime, int i)
{
	FILE			*file;
	bool			start;
	unsigned char		buffer[5000];
	struct utimbuf		filedate;
	int			Handle,Size,p,q,j,old1;
	time_t     		t_time1,t_time2;
	long			diff;

	if (File->Buffer != NULL) {
		free(File->Buffer);
		File->Buffer = NULL;
	}
	File->Used 	= 0;
	start		= true;

	t_time1 	= time(NULL);
	old1 		= 65536;

	error = ERR_NONE;
	while (error == ERR_NONE) {
		error = GSM_GetFilePart(s,File,&Handle,&Size);
		if (error == ERR_NONE || error == ERR_EMPTY || error == ERR_WRONGCRC) {
			if (start) {
				printf(_("Getting \"%s\"\n"), DecodeUnicodeConsole(File->Name));
				start = false;
			}
			if (File->Folder) {
				free(File->Buffer);
				GSM_Terminate();
				printf("%s\n", _("it's folder. Please give only file names"));
				exit(-1);
			}
			if (Size==0) {
				printf("*");
			} else {
				fprintf(stderr, _("%c  %i percent"), 13, (int)(File->Used*100/Size));
				if (File->Used*100/Size >= 2) {
					t_time2 = time(NULL);
					diff 	= t_time2-t_time1;
					p 	= diff*(Size-File->Used)/File->Used;
					if (p != 0) {
						if (p<old1) old1 = p;
						q = old1/60;
						fprintf(stderr, _(" (%02i:%02i minutes left)"),q,old1-q*60);
					} else {
						fprintf(stderr, "%30c",0x20);
					}
				}
			}
			if (error == ERR_EMPTY) break;
			if (error == ERR_WRONGCRC) {
				printf_warn("%s\n", _("File checksum calculated by phone doesn't match with value calculated by Gammu. File damaged or error in Gammu"));
				break;
			}
		}
	    	Print_Error(error);
	}
	printf("\n");
	if (error == ERR_NONE || error == ERR_EMPTY || error == ERR_WRONGCRC) {
		if (File->Used != 0) {
			sprintf(buffer,"%s",DecodeUnicodeConsole(File->Name));
			for (j=strlen(buffer)-1;j>0;j--) {
				if (buffer[j] == '\\' || buffer[j] == '/') break;
			}
			if (buffer[j] == '\\' || buffer[j] == '/') {
				sprintf(buffer,"%s",DecodeUnicodeConsole(File->Name+j*2+2));
			}
			file = fopen(buffer,"wb");
			if (file == NULL) {
				sprintf(buffer,"file%s",DecodeUnicodeString(File->ID_FullName));
				file = fopen(buffer,"wb");
			}
			if (file == NULL) {
				sprintf(buffer,"file%i",i);
				file = fopen(buffer,"wb");
			}
			printf(_("  Saving to %s\n"),buffer);
			if (!file) Print_Error(ERR_CANTOPENFILE);
			fwrite(File->Buffer,1,File->Used,file);
			fclose(file);
			if (!newtime && !File->ModifiedEmpty) {
				/* access time */
				filedate.actime  = Fill_Time_T(File->Modified);
				/* modification time */
				filedate.modtime = Fill_Time_T(File->Modified);
				dbgprintf("Setting date of %s\n",buffer);
				utime(buffer,&filedate);
			}
		}
	}
}

static void GetFiles(int argc, char *argv[])
{
	GSM_File		File;
	int			i;
	bool			newtime = false;

	File.Buffer = NULL;

	GSM_Init(true);

	for (i=2;i<argc;i++) {
		if (strcasecmp(argv[i],"-newtime") == 0) {
			newtime = true;
			continue;
		}

		DecodeUTF8QuotedPrintable(File.ID_FullName,argv[i],strlen(argv[i]));
		dbgprintf("grabbing '%s' '%s'\n",DecodeUnicodeString(File.ID_FullName),argv[i]);
		GetOneFile(&File, newtime, i);
	}

	free(File.Buffer);
	GSM_Terminate();
}

static void GetFileFolder(int argc, char *argv[])
{
	bool 			Start = true;
	GSM_File	 	File;
	int			level=0,allnum=0,num=0,filelevel=0,i=0;
	bool			newtime = false, found;
	unsigned char		IDUTF[200];

	File.Buffer = NULL;

	GSM_Init(true);

	for (i=2;i<argc;i++) {
		if (strcasecmp(argv[i],"-newtime") == 0) {
			newtime = true;
			continue;
		}
		allnum++;
	}

	while (allnum != num) {
		error = GSM_GetNextFileFolder(s,&File,Start);
		if (error == ERR_EMPTY) break;
	    	Print_Error(error);

		if (level == 0) {
			/* We search for file or folder */
			found = false;
			for (i=2;i<argc;i++) {
				if (strcasecmp(argv[i],"-newtime") == 0) {
					continue;
				}
				dbgprintf("comparing %s %s\n",DecodeUnicodeString(File.ID_FullName),argv[i]);
				DecodeUTF8QuotedPrintable(IDUTF,argv[i],strlen(argv[i]));
				if (mywstrncasecmp(File.ID_FullName,IDUTF,0)) {
					dbgprintf("found folder");
					found = true;
					if (File.Folder) {
						level 	  = 1;
						filelevel = File.Level + 1;
						Start 	  = false;
					} else {
						level = 2;
					}
					break;
				}
			}
			if (found && File.Folder) continue;
		}
		if (level == 1) {
			/* We have folder */
			dbgprintf("%i %i\n",File.Level,filelevel);
			if (File.Level != filelevel) {
				level = 0;
				num++;
			}
		}

		if (level != 0 && !File.Folder) {
			GetOneFile(&File, newtime,i);
			i++;
		}

		if (level == 2) {
			level = 0;
			num++;
		}

		Start = false;
	}

	free(File.Buffer);
	GSM_Terminate();
}

void AddOneFile(GSM_File *File, char *text, bool send)
{
	int 		Pos,Handle,i,j,old1;
	time_t     	t_time1,t_time2;
	GSM_DateTime	dt;
	long		diff;

	GSM_GetCurrentDateTime(&dt);
	t_time1 = Fill_Time_T(dt);
	old1 = 65536;

	dbgprintf("Adding file to filesystem now\n");
	error 	= ERR_NONE;
	Pos	= 0;
	while (error == ERR_NONE) {
		if (send) {
			error = GSM_SendFilePart(s,File,&Pos,&Handle);
		} else {
			error = GSM_AddFilePart(s,File,&Pos,&Handle);
		}
	    	if (error != ERR_EMPTY && error != ERR_WRONGCRC) Print_Error(error);
		if (File->Used != 0) {
			fprintf(stderr, "\r");
			fprintf(stderr, "%s", text);
			fprintf(stderr, _("%3i percent"), (int)(Pos * 100 / File->Used));
			if (Pos*100/File->Used >= 2) {
				GSM_GetCurrentDateTime(&dt);
				t_time2 = Fill_Time_T(dt);
				diff = t_time2-t_time1;
				i = diff*(File->Used-Pos)/Pos;
				if (i != 0) {
					if (i<old1) old1 = i;
					j = old1/60;
					fprintf(stderr, _(" (%02i:%02i minutes left)"), j , old1 - (j * 60));
				} else {
					fprintf(stderr, "%30c", ' ');
				}
			}
		}
	}
	fprintf(stderr, "\n");
	if (error == ERR_WRONGCRC) {
		printf_warn("%s\n", _("File checksum calculated by phone doesn't match with value calculated by Gammu. File damaged or error in Gammu"));
	}
}

static void AddSendFile(int argc, char *argv[])
{
	GSM_File		File;
	int			i,nextlong;
	char			IDUTF[200];
	bool			sendfile = false;
	int			optint = 2;

	if (strcasestr(argv[1], "sendfile") != NULL) {
		sendfile = true;
	}

	File.Buffer = NULL;
	if (!sendfile) {
		DecodeUTF8QuotedPrintable(File.ID_FullName,argv[optint],strlen(argv[optint]));
		optint++;
	}
	error = GSM_ReadFile(argv[optint], &File);
	Print_Error(error);
	EncodeUnicode(File.Name,argv[optint],strlen(argv[optint]));
	for (i=strlen(argv[optint])-1;i>0;i--) {
		if (argv[optint][i] == '\\' || argv[optint][i] == '/') break;
	}
	if (argv[optint][i] == '\\' || argv[optint][i] == '/') {
		EncodeUnicode(File.Name,argv[optint]+i+1,strlen(argv[optint])-i-1);
	}
	optint++;

	GSM_IdentifyFileFormat(&File);

	File.Protected 	= false;
	File.ReadOnly	= false;
	File.Hidden	= false;
	File.System	= false;

	if (argc > optint) {
		nextlong = 0;
		for (i = optint; i < argc; i++) {
			switch(nextlong) {
			case 0:
				if (strcasecmp(argv[i],"-type") == 0) {
					nextlong = 1;
					continue;
				}
				if (strcasecmp(argv[i],"-protected") == 0) {
					File.Protected = true;
					continue;
				}
				if (strcasecmp(argv[i],"-readonly") == 0) {
					File.ReadOnly = true;
					continue;
				}
				if (strcasecmp(argv[i],"-hidden") == 0) {
					File.Hidden = true;
					continue;
				}
				if (strcasecmp(argv[i],"-system") == 0) {
					File.System = true;
					continue;
				}
				if (strcasecmp(argv[i],"-newtime") == 0) {
					File.ModifiedEmpty = true;
					continue;
				}
				printf(_("Parameter \"%s\" unknown\n"),argv[i]);
				exit(-1);
			case 1:
				if (strcasecmp(argv[i],"JAR") == 0) {
					File.Type = GSM_File_Java_JAR;
				} else if (strcasecmp(argv[i],"JPG") == 0) {
					File.Type = GSM_File_Image_JPG;
				} else if (strcasecmp(argv[i],"BMP") == 0) {
					File.Type = GSM_File_Image_BMP;
				} else if (strcasecmp(argv[i],"WBMP") == 0) {
					File.Type = GSM_File_Image_WBMP;
				} else if (strcasecmp(argv[i],"GIF") == 0) {
					File.Type = GSM_File_Image_GIF;
				} else if (strcasecmp(argv[i],"PNG") == 0) {
					File.Type = GSM_File_Image_PNG;
                                } else if (strcasecmp(argv[i],"MIDI") == 0) {
                                        File.Type = GSM_File_Sound_MIDI;
                                } else if (strcasecmp(argv[i],"AMR") == 0) {
                                        File.Type = GSM_File_Sound_AMR;
                                } else if (strcasecmp(argv[i],"NRT") == 0) {
                                        File.Type = GSM_File_Sound_NRT;
                                } else if (strcasecmp(argv[i],"3GP") == 0) {
                                        File.Type = GSM_File_Video_3GP;
				} else {
					printf(_("What file type (\"%s\") ?\n"),argv[i]);
					exit(-1);
				}
				nextlong = 0;
				break;
			}
		}
		if (nextlong!=0) {
			printf_err("%s\n", _("Parameter missing!"));
			exit(-1);
		}
	}

	GSM_Init(true);

	AddOneFile(&File, "Writing: ", sendfile);
	EncodeUTF8QuotedPrintable(IDUTF,File.ID_FullName);
	printf(_("ID of new file is \"%s\"\n"),IDUTF);

	free(File.Buffer);
	GSM_Terminate();
}

static void AddFolder(int argc UNUSED, char *argv[])
{
	char			IDUTF[200];
	GSM_File 		File;

	DecodeUTF8QuotedPrintable(File.ID_FullName,argv[2],strlen(argv[2]));
	EncodeUnicode(File.Name,argv[3],strlen(argv[3]));
	File.ReadOnly = false;

	GSM_Init(true);

	error = GSM_AddFolder(s,&File);
    	Print_Error(error);
	EncodeUTF8QuotedPrintable(IDUTF,File.ID_FullName);
	printf(_("ID of new folder is \"%s\"\n"),IDUTF);

	GSM_Terminate();
}

static void DeleteFolder(int argc UNUSED, char *argv[] UNUSED)
{
	unsigned char buffer[500];

	GSM_Init(true);

	DecodeUTF8QuotedPrintable(buffer,argv[2],strlen(argv[2]));

	error = GSM_DeleteFolder(s,buffer);
    	Print_Error(error);

	GSM_Terminate();
}

static void DeleteFiles(int argc, char *argv[])
{
	int		i;
	unsigned char	buffer[500];

	GSM_Init(true);

	for (i=2;i<argc;i++) {
		DecodeUTF8QuotedPrintable(buffer,argv[i],strlen(argv[i]));
		error = GSM_DeleteFile(s,buffer);
	    	Print_Error(error);
	}

	GSM_Terminate();
}

static void CallDivert(int argc, char *argv[])
{
	GSM_MultiCallDivert cd;

	     if (strcasecmp("get",	argv[2]) == 0) {}
	else if (strcasecmp("set",	argv[2]) == 0) {}
	else {
		printf(_("Unknown divert action (\"%s\")\n"),argv[2]);
		exit(-1);
	}

	     if (strcasecmp("all"       , argv[3]) == 0) {cd.Request.DivertType = GSM_DIVERT_AllTypes  ;}
	else if (strcasecmp("busy"      , argv[3]) == 0) {cd.Request.DivertType = GSM_DIVERT_Busy      ;}
	else if (strcasecmp("noans"     , argv[3]) == 0) {cd.Request.DivertType = GSM_DIVERT_NoAnswer  ;}
	else if (strcasecmp("outofreach", argv[3]) == 0) {cd.Request.DivertType = GSM_DIVERT_OutOfReach;}
	else {
		printf(_("Unknown divert type (\"%s\")\n"),argv[3]);
		exit(-1);
	}

	     if (strcasecmp("all"  , argv[4]) == 0) {cd.Request.CallType = GSM_DIVERT_AllCalls  ;}
	else if (strcasecmp("voice", argv[4]) == 0) {cd.Request.CallType = GSM_DIVERT_VoiceCalls;}
	else if (strcasecmp("fax"  , argv[4]) == 0) {cd.Request.CallType = GSM_DIVERT_FaxCalls  ;}
	else if (strcasecmp("data" , argv[4]) == 0) {cd.Request.CallType = GSM_DIVERT_DataCalls ;}
	else {
		printf(_("Unknown call type (\"%s\")\n"),argv[4]);
		exit(-1);
	}

	GSM_Init(true);

	if (strcasecmp("get", argv[2]) == 0) {
		error = GSM_GetCallDivert(s,&cd);
	    	Print_Error(error);
		printf(_("Query:\n   Divert type: "));
	} else {
		cd.Request.Number[0] = 0;
		cd.Request.Number[1] = 0;
		if (argc > 5) EncodeUnicode(cd.Request.Number,argv[5],strlen(argv[5]));

		cd.Request.Timeout = 0;
		if (argc > 6) cd.Request.Timeout = atoi(argv[6]);

		error = GSM_SetCallDivert(s,&cd);
	    	Print_Error(error);
		printf(_("Changed:\n   Divert type: "));
	}

      	switch (cd.Request.DivertType) {
   		case GSM_DIVERT_Busy      : printf(_("when busy"));				break;
	        case GSM_DIVERT_NoAnswer  : printf(_("when not answered"));			break;
      	        case GSM_DIVERT_OutOfReach: printf(_("when phone off or no coverage"));	break;
                case GSM_DIVERT_AllTypes  : printf(_("all types of diverts"));			break;
                default		          : printf(_("unknown %i"),cd.Request.DivertType);			break;
        }
        printf(_("\n   Calls type : "));
	switch (cd.Request.CallType) {
	 	case GSM_DIVERT_VoiceCalls: printf(_("voice"));				break;
                case GSM_DIVERT_FaxCalls  : printf(_("fax"));				break;
                case GSM_DIVERT_DataCalls : printf(_("data"));		 		break;
		case GSM_DIVERT_AllCalls  : printf(_("data & fax & voice"));		break;
                default                   : printf(_("unknown %i"),cd.Request.CallType);   		break;
        }
	printf(_("\nResponse:"));

	for (i=0;i<cd.Response.EntriesNum;i++) {
	        printf(_("\n   Calls type : "));
        	switch (cd.Response.Entries[i].CallType) {
                	case GSM_DIVERT_VoiceCalls: printf(_("voice"));		 	break;
                	case GSM_DIVERT_FaxCalls  : printf(_("fax"));		 	break;
                	case GSM_DIVERT_DataCalls : printf(_("data"));		 	break;
                	default                   : printf(_("unknown %i"),cd.Response.Entries[i].CallType);break;
              	}
		printf("\n");
               	printf(_("   Timeout    : "));
		PRINTSECONDS(cd.Response.Entries[i].Timeout);
		printf("\n");
                printf(LISTFORMAT "%s\n", _("Number"),DecodeUnicodeString(cd.Response.Entries[i].Number));
        }
	printf("\n");

	GSM_Terminate();
}

static void CancelAllDiverts(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_Init(true);

	error = GSM_CancelAllDiverts(s);
    	Print_Error(error);

	GSM_Terminate();
}

#ifdef DEBUG
/**
 * Function for testing purposes.
 */
static void Foo(int argc UNUSED, char *argv[] UNUSED)
{
}
#endif

int ProcessParameters(char start, int argc, char *argv[]);

/**
 * Reads commands from file (argv[2]) or stdin and executes them
 * sequentially as if they were given on the command line. Also allows
 * recursive calling (nested batches in the batch files).
 */
static void RunBatch(int argc, char *argv[]) {
	FILE *bf;
	/**
	 * @todo Allocate memory dynamically.
	 */
	char ln[2000];
	size_t i;
	int j,c=0,argsc;
	char* argsv[20];
	bool origbatch;
	char *name;
	char std_name[] = N_("standard input");

	if (argc == 2 || strcmp(argv[2], "-") == 0) {
		bf = stdin;
		name = gettext(std_name);
	} else {
		bf = fopen(argv[2], "r");
		name = argv[2];
	}

	if (bf == NULL) {
		printf(_("Batch file could not be opened: %s\n"), argv[2]);
		return; /* not exit(), so that any parent batch can continue */
	}

	argsv[0] = argv[0];
	origbatch = batch;
	batch = true;
	while (!feof(bf)) {
		ln[0] = 0;
		fgets(ln, sizeof(ln) - 2, bf);
		if (ln[strlen(ln) - 2] == 0x0D ) {
		/* reduce CRLF to LF so we have the same EOL on Windows and Linux */
			ln[strlen(ln) - 2] = 0x0A;
			ln[strlen(ln) - 1] = 0;
		}
		if (strlen(ln) > 1 && ln[0] != '#') {
			/* if line is not empty and is not a comment, split words into strings in the array argsv */
			i=0;j=0;
			argsc=0;
			while (i < strlen(ln)) {
				if (ln[i]==' ' || ln[i]==0x0A) {
					argsc++;
					argsv[argsc] = malloc(i-j+1);
					strncpy(argsv[argsc], ln + j, i - j);
					argsv[argsc][i - j] = 0;
					j = i + 1;
				}
				i++;
			}
			if(argsc > 0) {
				/* we have some usable command and parameters, send them into standard processing */
				printf("----------------------------------------------------------------------------\n");
				printf(_("Executing batch \"%s\" - command %i: %s"), name, ++c, ln);
				/**
				 * @todo Handle return value from ProcessParameters.
				 */
				ProcessParameters(0, argsc + 1, argsv);
				for (j = 1; j <= argsc; j++) {
					free(argsv[j]);
				}
			}
		}
	}
	if (!origbatch) {
		/* only close the batch if we are not in a nested batch */
		batch=false;
		if (batchConn) { GSM_Terminate(); }
	}
	fclose(bf);
}

void Help(int argc, char *argv[]);

static GSM_Parameters Parameters[] = {
/* *INDENT-OFF* */
#ifdef DEBUG
	{"foo",			0, 0, Foo,			{0},				""},
#endif
	{"help",			0, 1, Help,			{H_Gammu,0},			""},
	{"identify",			0, 0, Identify,			{H_Info,0},			""},
	{"version",			0, 0, Version,			{H_Gammu,0},			""},
	{"features",			0, 0, Features,			{H_Gammu,0},			""},
	{"getdisplaystatus",		0, 0, GetDisplayStatus,		{H_Info,0},			""},
	{"monitor",			0, 1, Monitor,			{H_Info,H_Network,H_Call,0},	"[times]"},
	{"setautonetworklogin",	0, 0, SetAutoNetworkLogin,	{H_Network,0},			""},
	{"listnetworks",		0, 1, ListNetworks,		{H_Network,0},			"[country]"},
	{"getgprspoint",		1, 2, GetGPRSPoint,		{H_Network,0},			"start [stop]"},
	{"getfilesystemstatus",	0, 0, GetFileSystemStatus,	{H_Filesystem,0},		""},
	{"getfilesystem",		0, 1, GetFileSystem,		{H_Filesystem,0},		"[-flatall|-flat]"},
	{"getfilefolder",		1,40, GetFileFolder,		{H_Filesystem,0},		"ID1, ID2, ..."},
	{"addfolder",			2, 2, AddFolder,		{H_Filesystem,0},		"parentfolderID name"},
	{"deletefolder",		1, 1, DeleteFolder,		{H_Filesystem,0},		"name"},
	{"getfolderlisting",		1, 1, GetFolderListing,		{H_Filesystem,0},		"folderID"},
	{"getrootfolders",		0, 0, GetRootFolders,		{H_Filesystem,0},		""},
	{"setfileattrib",		1, 5, SetFileAttrib,		{H_Filesystem,0},		"folderID [-system] [-readonly] [-hidden] [-protected]"},
	{"getfiles",			1,40, GetFiles,			{H_Filesystem,0},		"ID1, ID2, ..."},
	{"addfile",			2, 6, AddSendFile,		{H_Filesystem,0},		"folderID name [-type JAR|BMP|PNG|GIF|JPG|MIDI|WBMP|AMR|3GP|NRT][-readonly][-protected][-system][-hidden][-newtime]"},
	{"sendfile",			1, 1, AddSendFile,		{H_Filesystem,0},		"name"},
	{"deletefiles",		1,20, DeleteFiles,		{H_Filesystem,0},		"fileID"},
#if defined(GSM_ENABLE_NOKIA_DCT3) || defined(GSM_ENABLE_NOKIA_DCT4)
	{"nokiaaddplaylists",		0, 0, NokiaAddPlayLists,	{H_Filesystem,H_Nokia,0},	""},
	{"nokiaaddfile",		2, 5, NokiaAddFile,		{H_Filesystem,H_Nokia,0},	"Application|Game file [-readonly][-overwrite]"},
	{"nokiaaddfile",		2, 5, NokiaAddFile,		{H_Filesystem,H_Nokia,0},	"Gallery|Gallery2|Camera|Tones|Tones2|Records|Video|Playlist|MemoryCard file [-name name][-protected][-readonly][-system][-hidden][-newtime]"},
	{"playsavedringtone",		1, 1, DCT4PlaySavedRingtone, 	{H_Ringtone,0},			"number"},
#endif
	{"playringtone",		1, 1, PlayRingtone, 		{H_Ringtone,0},			"file"},
	{"getdatetime",		0, 0, GetDateTime,		{H_DateTime,0},			""},
	{"setdatetime",		0, 2, SetDateTime,		{H_DateTime,0},			"[HH:MM[:SS]] [YYYY/MM/DD]"},
	{"getalarm",			0, 1, GetAlarm,			{H_DateTime,0},			"[start]"},
	{"setalarm",			2, 2, SetAlarm,			{H_DateTime,0},			"hour minute"},
	{"resetphonesettings",	1, 1, ResetPhoneSettings,	{H_Settings,0},			"PHONE|DEV|UIF|ALL|FACTORY"},
	{"getmemory",			2, 4, GetMemory,		{H_Memory,0},			ALL_MEMORY_TYPES " start [stop [-nonempty]]"},
	{"deletememory",		2, 3, DeleteMemory,		{H_Memory,0},			ALL_MEMORY_TYPES " start [stop]"},
	{"getallmemory",		1, 2, GetAllMemory,		{H_Memory,0},			ALL_MEMORY_TYPES},
	{"searchmemory",		1, 1, SearchMemory,		{H_Memory,0},			"text"},
	{"listmemorycategory",	1, 1, ListMemoryCategory,	{H_Memory, H_Category,0},	"text|number"},
	{"getfmstation",		1, 2, GetFMStation,		{H_FM,0},			"start [stop]"},
	{"getsmsc",			0, 2, GetSMSC,			{H_SMS,0},			"[start [stop]]"},
	{"getsms",			2, 3, GetSMS,			{H_SMS,0},			"folder start [stop]"},
	{"deletesms",			2, 3, DeleteSMS,		{H_SMS,0},			"folder start [stop]"},
	{"deleteallsms",		1, 1, DeleteAllSMS,		{H_SMS,0},			"folder"},
	{"getsmsfolders",		0, 0, GetSMSFolders,		{H_SMS,0},			""},
	{"getallsms",			0, 1, GetAllSMS,		{H_SMS,0},			"-pbk"},
	{"geteachsms",		0, 1, GetEachSMS,		{H_SMS,0},			"-pbk"},

#define SMS_TEXT_OPTIONS	"[-inputunicode][-16bit][-flash][-len len][-autolen len][-unicode][-enablevoice][-disablevoice][-enablefax][-disablefax][-enableemail][-disableemail][-voidsms][-replacemessages ID][-replacefile file]"
#define SMS_PICTURE_OPTIONS	"[-text text][-unicode][-alcatelbmmi]"
#define SMS_PROFILE_OPTIONS	"[-name name][-bitmap bitmap][-ringtone ringtone]"
#define SMS_EMS_OPTIONS		"[-unicode][-16bit][-format lcrasbiut][-text text][-unicodefiletext file][-defsound ID][-defanimation ID][-tone10 file][-tone10long file][-tone12 file][-tone12long file][-toneSE file][-toneSElong file][-fixedbitmap file][-variablebitmap file][-variablebitmaplong file][-animation frames file1 ...][-protected number]"
#define SMS_SMSTEMPLATE_OPTIONS	"[-unicode][-text text][-unicodefiletext file][-defsound ID][-defanimation ID][-tone10 file][-tone10long file][-tone12 file][-tone12long file][-toneSE file][-toneSElong file][-variablebitmap file][-variablebitmaplong file][-animation frames file1 ...]"
#define SMS_ANIMATION_OPTIONS	""
#define SMS_OPERATOR_OPTIONS	"[-netcode netcode][-biglogo]"
#define SMS_RINGTONE_OPTIONS	"[-long][-scale]"
#define SMS_SAVE_OPTIONS	"[-folder id][-unread][-read][-unsent][-sent][-sender number][-smsname name]"
#define SMS_SEND_OPTIONS	"[-report][-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX][-save [-folder number]]"
#define SMS_COMMON_OPTIONS	"[-smscset number][-smscnumber number][-reply][-maxsms num]"

	{"savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,0},			"TEXT " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS SMS_TEXT_OPTIONS},
	{"savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_Ringtone,0},		"RINGTONE file " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS SMS_RINGTONE_OPTIONS},
	{"savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_Logo,0},		"OPERATOR file " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS SMS_OPERATOR_OPTIONS},
	{"savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_Logo,0},		"CALLER file " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS},
	{"savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_Logo,0},		"PICTURE file " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS SMS_PICTURE_OPTIONS},
	{"savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_Logo,0},		"ANIMATION frames file1 file2... " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS SMS_ANIMATION_OPTIONS},
	{"savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_MMS,0},		"MMSINDICATOR URL Title Sender " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS},
	{"savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_WAP,0},		"WAPINDICATOR URL Title " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS},
#ifdef GSM_ENABLE_BACKUP
	{"savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_WAP,0},		"BOOKMARK file location " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS},
	{"savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_WAP,0},		"WAPSETTINGS file location DATA|GPRS " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS},
	{"savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_MMS,0},		"MMSSETTINGS file location  " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS},
	{"savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_Calendar,0},		"CALENDAR file location " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS},
	{"savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_ToDo,0},		"TODO file location " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS},
	{"savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_Memory,0},		"VCARD10|VCARD21 file SM|ME location [-nokia]" SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS},
#endif
	{"savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_Settings,0},		"PROFILE " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS SMS_PROFILE_OPTIONS},
	{"savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,0},			"EMS " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS SMS_EMS_OPTIONS},
	{"savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,0},			"SMSTEMPLATE " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS SMS_SMSTEMPLATE_OPTIONS},

	{"sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,0},			"TEXT destination " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS SMS_TEXT_OPTIONS},
	{"sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_Ringtone,0},		"RINGTONE destination file " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS SMS_RINGTONE_OPTIONS},
	{"sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_Logo,0},		"OPERATOR destination file " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS SMS_OPERATOR_OPTIONS},
	{"sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_Logo,0},		"CALLER destination file " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS},
	{"sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_Logo,0},		"PICTURE destination file " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS SMS_PICTURE_OPTIONS},
	{"sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_Logo,0},		"ANIMATION destination frames file1 file2... " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS SMS_ANIMATION_OPTIONS},
	{"sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_MMS,0},		"MMSINDICATOR destination URL Title Sender " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS},
	{"sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_WAP,0},		"WAPINDICATOR destination URL Title " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS},
#ifdef GSM_ENABLE_BACKUP
	{"sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_WAP,0},		"BOOKMARK destination file location " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS},
	{"sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_WAP,0},		"WAPSETTINGS destination file location DATA|GPRS " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS},
	{"sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_MMS,0},		"MMSSETTINGS destination file location " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS},
	{"sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_Calendar,0},		"CALENDAR destination file location " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS},
	{"sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_ToDo,0},		"TODO destination file location " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS},
	{"sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_Memory,0},		"VCARD10|VCARD21 destination file SM|ME location [-nokia]" SMS_SEND_OPTIONS SMS_COMMON_OPTIONS},
#endif
	{"sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_Settings,0},		"PROFILE destination " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS ""SMS_PROFILE_OPTIONS},
	{"sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,0},			"EMS destination " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS SMS_EMS_OPTIONS},
	{"sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,0},			"SMSTEMPLATE destination " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS SMS_SMSTEMPLATE_OPTIONS},

	{"displaysms",		2,30, SendSaveDisplaySMS,	{H_SMS,0},			"... (options like in sendsms)"},

	{"addsmsfolder",		1, 1, AddSMSFolder,		{H_SMS,0},			"name"},
#ifdef HAVE_MYSQL_MYSQL_H
	{"smsd",			2, 2, SMSDaemon,		{H_SMS,H_SMSD,0},		"MYSQL configfile"},
#endif
#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
	{"smsd",			2, 2, SMSDaemon,		{H_SMS,H_SMSD,0},		"PGSQL configfile"},
#endif
	{"smsd",			2, 2, SMSDaemon,		{H_SMS,H_SMSD,0},		"FILES configfile"},
	{"sendsmsdsms",		2,30, SendSaveDisplaySMS,	{H_SMS,H_SMSD,0},		"TEXT|WAPSETTINGS|... destination FILES|MYSQL|PGSQL configfile ... (options like in sendsms)"},
	{"getmmsfolders",		0, 0, GetMMSFolders,		{H_MMS,0},			""},
	{"getallmms",			0, 1, GetEachMMS,		{H_MMS,0},			"[-save]"},
	{"geteachmms",		0, 1, GetEachMMS,		{H_MMS,0},			"[-save]"},
	{"getringtone",		1, 2, GetRingtone,		{H_Ringtone,0},			"location [file]"},
	{"getphoneringtone",		1, 2, GetRingtone,		{H_Ringtone,0},			"location [file]"},
	{"getringtoneslist",		0, 0, GetRingtonesList,		{H_Ringtone,0},			""},
	{"setringtone",		1, 6, SetRingtone,		{H_Ringtone,0},			"file [-location location][-scale][-name name]"},
#if defined(GSM_ENABLE_NOKIA_DCT3) || defined(GSM_ENABLE_NOKIA_DCT4)
	{"nokiacomposer",		1, 1, NokiaComposer,		{H_Ringtone,H_Nokia,0},		"file"},
#endif
	{"copyringtone",		2, 3, CopyRingtone,		{H_Ringtone,0},			"source destination [RTTL|BINARY]"},
	{"getussd",			1, 1, GetUSSD,			{H_Call,0},			"code"},
	{"dialvoice",			1, 2, DialVoice,		{H_Call,0},			"number [show|hide]"},
	{"maketerminatedcall",	2, 3, MakeTerminatedCall,	{H_Call,0},			"number length [show|hide]"},
	{"getspeeddial",		1, 2, GetSpeedDial,		{H_Call,H_Memory,0},		"start [stop]"},
	{"cancelcall",		0, 1, CancelCall,		{H_Call,0},			"[ID]"},
	{"answercall",		0, 1, AnswerCall,		{H_Call,0},			"[ID]"},
	{"unholdcall",		1, 1, UnholdCall,		{H_Call,0},			"ID"},
	{"holdcall",			1, 1, HoldCall,			{H_Call,0},			"ID"},
	{"conferencecall",		1, 1, ConferenceCall,		{H_Call,0},			"ID"},
	{"splitcall",			1, 1, SplitCall,		{H_Call,0},			"ID"},
	{"switchcall",		0, 1, SwitchCall,		{H_Call,0},			"[ID]"},
	{"transfercall",		0, 1, TransferCall,		{H_Call,0},			"[ID]"},
	{"divert",			3, 5, CallDivert,		{H_Call,0},			"get|set all|busy|noans|outofreach all|voice|fax|data [number timeout]"},
	{"canceldiverts",		0, 0, CancelAllDiverts,		{H_Call,0},			""},
	{"senddtmf",			1, 1, SendDTMF,			{H_Call,0},			"sequence"},
	{"getcalendarsettings",	0, 0, GetCalendarSettings,	{H_Calendar,H_Settings,0},	""},
	{"getalltodo",		0, 0, GetAllToDo,		{H_ToDo,0},			""},
	{"listtodocategory",		1, 1, ListToDoCategory,		{H_ToDo, H_Category,0},		"text|number"},
	{"gettodo",			1, 2, GetToDo,			{H_ToDo,0},			"start [stop]"},
	{"deletetodo",		1, 2, DeleteToDo,		{H_ToDo,0},			"start [stop]"},
	{"getallnotes",		0, 0, GetAllNotes,		{H_Note,0},			""},
	{"deletecalendar",		1, 2, DeleteCalendar,		{H_Calendar,0},			"start [stop]"},
	{"getallcalendar",		0, 0, GetAllCalendar,		{H_Calendar,0},			""},
	{"getcalendar",		1, 2, GetCalendar,		{H_Calendar,0},			"start [stop]"},
	{"addcategory",       	2, 2, AddCategory,       	{H_Category,H_ToDo,H_Memory,0},	"TODO|PHONEBOOK text"},
	{"getcategory",       	2, 3, GetCategory,       	{H_Category,H_ToDo,H_Memory,0},	"TODO|PHONEBOOK start [stop]"},
	{"getallcategory",	  	1, 1, GetAllCategories,  	{H_Category,H_ToDo,H_Memory,0},	"TODO|PHONEBOOK"},
	{"reset",			1, 1, Reset,			{H_Other,0},			"SOFT|HARD"},
	{"getprofile",		1, 2, GetProfile,		{H_Settings,0},			"start [stop]"},
	{"getsecuritystatus",		0, 0, GetSecurityStatus,	{H_Info,0},			""},
	{"entersecuritycode",		2, 2, EnterSecurityCode,	{H_Other,0},			"PIN|PUK|PIN2|PUK2 code"},
	{"deletewapbookmark", 	1, 2, DeleteWAPBookmark, 	{H_WAP,0},			"start [stop]"},
	{"getwapbookmark",		1, 2, GetWAPBookmark,		{H_WAP,0},			"start [stop]"},
	{"getwapsettings",		1, 2, GetWAPMMSSettings,	{H_WAP,0},			"start [stop]"},
	{"getmmssettings",		1, 2, GetWAPMMSSettings,	{H_MMS,0},			"start [stop]"},
	{"getsyncmlsettings",		1, 2, GetSyncMLSettings,	{H_WAP,0},			"start [stop]"},
	{"getchatsettings",		1, 2, GetChatSettings,		{H_WAP,0},			"start [stop]"},
	{"readmmsfile",		1, 2, ReadMMSFile,		{H_MMS,0},			"file [-save]"},
	{"getbitmap",			1, 3, GetBitmap,		{H_Logo,0},			"STARTUP [file]"},
	{"getbitmap",			1, 3, GetBitmap,		{H_Logo,0},			"CALLER location [file]"},
	{"getbitmap",			1, 3, GetBitmap,		{H_Logo,0},			"OPERATOR [file]"},
	{"getbitmap",			1, 3, GetBitmap,		{H_Logo,0},			"PICTURE location [file]"},
	{"getbitmap",			1, 3, GetBitmap,		{H_Logo,0},			"TEXT"},
	{"getbitmap",			1, 3, GetBitmap,		{H_Logo,0},			"DEALER"},
	{"setbitmap",			1, 4, SetBitmap,		{H_Logo,0},			"STARTUP file|1|2|3"},
	{"setbitmap",			1, 4, SetBitmap,		{H_Logo,0},			"COLOURSTARTUP [fileID]"},
	{"setbitmap",			1, 4, SetBitmap,		{H_Logo,0},			"WALLPAPER fileID"},
	{"setbitmap",			1, 4, SetBitmap,		{H_Logo,0},			"CALLER location [file]"},
	{"setbitmap",			1, 4, SetBitmap,		{H_Logo,0},			"OPERATOR [file [netcode]]"},
	{"setbitmap",			1, 4, SetBitmap,		{H_Logo,0},			"COLOUROPERATOR [fileID [netcode]]"},
	{"setbitmap",			1, 4, SetBitmap,		{H_Logo,0},			"PICTURE file location [text]"},
	{"setbitmap",			1, 4, SetBitmap,		{H_Logo,0},			"TEXT text"},
	{"setbitmap",			1, 4, SetBitmap,		{H_Logo,0},			"DEALER text"},
	{"copybitmap",		1, 3, CopyBitmap,		{H_Logo,0},			"inputfile [outputfile [OPERATOR|PICTURE|STARTUP|CALLER]]"},
	{"presskeysequence",		1, 1, PressKeySequence,		{H_Other,0},			"mMnNpPuUdD+-123456789*0#gGrR<>[]hHcCjJfFoOmMdD@"},
#if defined(WIN32) || defined(HAVE_PTHREAD)
	{"searchphone",		0, 1, SearchPhone,		{H_Other,0},			"[-debug]"},
#endif
#ifdef GSM_ENABLE_BACKUP
	{"savefile",			4, 5, SaveFile,			{H_Backup,H_Calendar,0},	"CALENDAR target.vcs file location"},
	{"savefile",			4, 5, SaveFile,			{H_Backup,H_ToDo,0},		"TODO target.vcs file location"},
	{"savefile",			4, 5, SaveFile,			{H_Backup,H_Memory,0},		"VCARD10|VCARD21 target.vcf file SM|ME location"},
	{"savefile",			4, 5, SaveFile,			{H_Backup,H_WAP,0},		"BOOKMARK target.url file location"},
	{"backup",			1, 2, Backup,			{H_Backup,H_Memory,H_Calendar,H_ToDo,H_Category,H_Ringtone,H_WAP,H_FM,0},			"file [-yes]"},
	{"backupsms",			1, 1, BackupSMS,		{H_Backup,H_SMS,0},		"file"},
	{"restore",			1, 2, Restore,			{H_Backup,H_Memory,H_Calendar,H_ToDo,H_Category,H_Ringtone,H_WAP,H_FM,0},			"file [-yes]"},
	{"addnew",			1, 2, AddNew,			{H_Backup,H_Memory,H_Calendar,H_ToDo,H_Category,H_Ringtone,H_WAP,H_FM,0},			"file [-yes]"},
	{"restoresms",		1, 1, RestoreSMS,		{H_Backup,H_SMS,0},		"file"},
	{"addsms",			2, 2, AddSMS,			{H_Backup,H_SMS,0},		"folder file"},
#endif
	{"clearall",			0, 0, ClearAll,			{H_Memory,H_Calendar,H_ToDo,H_Category,H_Ringtone,H_WAP,H_FM,0},	""},
	{"networkinfo",		0, 0, NetworkInfo,		{H_Network,0},			""},
#ifdef GSM_ENABLE_AT
	{"siemenssatnetmon",		0, 0, ATSIEMENSSATNetmon,	{H_Siemens,H_Network,0},	""},
	{"siemensnetmonact",		1, 1, ATSIEMENSActivateNetmon,	{H_Siemens,H_Network,0},	"netmon_type (1-full, 2-simple)"},
	{"siemensnetmonitor",		1, 1, ATSIEMENSNetmonitor,	{H_Siemens,H_Network,0},	"test"},
#endif
#ifdef GSM_ENABLE_NOKIA6110
	{"nokiagetoperatorname", 	0, 0, DCT3GetOperatorName,	{H_Nokia,H_Network,0},		""},
	{"nokiasetoperatorname", 	0, 2, DCT3SetOperatorName,	{H_Nokia,H_Network,0},		"[networkcode name]"},
	{"nokiadisplayoutput", 	0, 0, DCT3DisplayOutput,	{H_Nokia,0},			""},
#endif
#ifdef GSM_ENABLE_NOKIA_DCT3
	{"nokianetmonitor",		1, 1, DCT3netmonitor,		{H_Nokia,H_Network,0},		"test"},
	{"nokianetmonitor36",		0, 0, DCT3ResetTest36,		{H_Nokia,0},			""},
	{"nokiadebug",		1, 2, DCT3SetDebug,		{H_Nokia,H_Network,0},		"filename [[v11-22][,v33-44]...]"},
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
	{"nokiagetpbkfeatures",	1, 1, DCT4GetPBKFeatures,	{H_Nokia,H_Memory,0},		"memorytype"},
	{"nokiasetvibralevel",	1, 1, DCT4SetVibraLevel,	{H_Nokia,H_Other,0},		"level"},
	{"nokiagetvoicerecord",	1, 1, DCT4GetVoiceRecord,	{H_Nokia,H_Other,0},		"location"},
#ifdef GSM_ENABLE_NOKIA6510
	{"nokiasetlights",		2, 2, DCT4SetLight,		{H_Nokia,H_Tests,0},		"keypad|display|torch on|off"},
	{"nokiatuneradio",		0, 0, DCT4TuneRadio,		{H_Nokia,H_FM,0},		""},
#endif
	{"nokiamakecamerashoot",	0, 0, DCT4MakeCameraShoot,	{H_Nokia,H_Other,0},		""},
	{"nokiagetscreendump",	0, 0, DCT4GetScreenDump,	{H_Nokia,H_Other,0},		""},
#endif
#if defined(GSM_ENABLE_NOKIA_DCT3) || defined(GSM_ENABLE_NOKIA_DCT4)
	{"nokiavibratest",		0, 0, NokiaVibraTest,		{H_Nokia,H_Tests,0},		""},
	{"nokiagett9",		0, 0, NokiaGetT9,		{H_Nokia,H_SMS,0},		""},
	{"nokiadisplaytest",		1, 1, NokiaDisplayTest,		{H_Nokia,H_Tests,0},		"number"},
	{"nokiagetadc",		0, 0, NokiaGetADC,		{H_Nokia,H_Tests,0},		""},
	{"nokiasecuritycode",		0, 0, NokiaSecurityCode,	{H_Nokia,H_Info,0},		""},
	{"nokiaselftests",		0, 0, NokiaSelfTests,		{H_Nokia,H_Tests,0},		""},
	{"nokiasetphonemenus",	0, 0, NokiaSetPhoneMenus,	{H_Nokia,H_Other,0},		""},
#endif
#ifdef DEBUG
	{"decodesniff",		2, 3, decodesniff,		{H_Decode,0},			"MBUS2|IRDA file [phonemodel]"},
	{"decodebinarydump",		1, 2, decodebinarydump,		{H_Decode,0},			"file [phonemodel]"},
	{"makeconverttable",		1, 1, MakeConvertTable,		{H_Decode,0},			"file"},
#endif
	{"batch",			0, 1, RunBatch,			{H_Other,0},			"[file]"},
	{"",				0, 0, NULL,			{0}, ""}
};

static HelpCategoryDescriptions HelpDescriptions[] = {
	{H_Call,	"call",		N_("Calls")},
	{H_SMS,		"sms",		N_("SMS and EMS")},
	{H_Memory,	"memory",	N_("Memory (phonebooks and calls)")},
	{H_Filesystem,	"filesystem",	N_("Filesystem")},
	{H_Logo,	"logo",		N_("Logo and pictures")},
	{H_Ringtone,	"ringtone",	N_("Ringtones")},
	{H_Calendar,	"calendar",	N_("Calendar notes")},
	{H_ToDo,	"todo",		N_("To do lists")},
	{H_Note,	"note",		N_("Notes")},
	{H_DateTime,	"datetime",	N_("Date, time and alarms")},
	{H_Category,	"category",	N_("Categories")},
#ifdef GSM_ENABLE_BACKUP
	{H_Backup,	"backup",	N_("Backing up and restoring")},
#endif
#if defined(GSM_ENABLE_NOKIA_DCT3) || defined(GSM_ENABLE_NOKIA_DCT4)
	{H_Nokia,	"nokia",	N_("Nokia specific")},
#endif
#ifdef GSM_ENABLE_AT
	{H_Siemens,	"siemens",	N_("Siemens specific")},
#endif
	{H_Network,	"network",	N_("Network")},
	{H_WAP,		"wap",		N_("WAP settings and bookmarks")},
	{H_MMS,		"mms",		N_("MMS and MMS settings")},
	{H_Tests,	"tests",	N_("Phone tests")},
	{H_FM,		"fm",		N_("FM radio")},
	{H_Info,	"info",		N_("Phone information")},
	{H_Settings,	"settings",	N_("Phone settings")},
#ifdef DEBUG
	{H_Decode,	"decode",	N_("Dumps decoding")},
#endif
	{H_Other,	"other",	N_("Functions that don't fit elsewhere")},
	{H_Gammu,	"gammu",	N_("Gammu information")},
	{H_SMSD,	"smsd",		N_("SMS daemon")},
	{0,		NULL,		NULL}
/* *INDENT-ON* */
};


void HelpHeader(void)
{
	PrintVersion();
}

void HelpGeneral(void)
{
	int	i=0;

	HelpHeader();

 	printf("%s\n\n", _("Usage: gammu [confign] [nothing|text|textall|binary|errors] <command> [options]"));
 	printf("%s\n", _("First parameter optionally specifies which config section to use (all are probed by default)."));
 	printf("%s\n\n", _("Second parameter optionally controls debug level, next one specifies actions."));

 	printf("%s\n\n", _("Commands can be specified with or without leading --."));

	/* We might want to put here some most used commands */
	printf("%s\n\n", _("For more details, call help on specific topic (gammu --help topic). Topics are:"));

	while (HelpDescriptions[i].category != 0) {
		printf("%11s - %s\n", HelpDescriptions[i].option, gettext(HelpDescriptions[i].description));
		i++;
	}
	printf("\n");
}

void HelpSplit(int cols, int len, unsigned char *buff)
{
	int		l, len2, pos, split;
	bool		in_opt,first=true;
	char		*remain, spaces[50], buffer[500];

	if (cols == 0) {
		printf(" %s\n", buff);
	} else {
		printf(" ");
		spaces[0] = 0;
		len2 = strlen(buff);
		if (len + len2 < cols) {
			printf("%s\n", buff);
		} else {
			for(l = 0; l < len; l++) strcat(spaces, " ");

			remain = buff;

			while (strlen(remain) > 0) {
				split	= 0;
				pos	= 0;
				in_opt	= false;
				if (!first) printf(spaces);
				while (pos < cols - len && remain[pos] != 0) {
					if (in_opt && remain[pos] == ']') {
						in_opt = false;
						split  = pos;
					} else if (remain[pos] == '[') {
						in_opt = true;
					} else if (!in_opt && remain[pos] == ' ') {
						split = pos - 1;
					}
					pos++;
				}
				/* Can not be split */
				if (split == 0) {
					printf("%s\n", remain);
					remain += strlen(remain);
				} else {
					first = false;
					split++;
					strncpy(buffer, remain, split);
					buffer[split] = 0;
					printf("%s\n", buffer);
					remain += split;
					if (remain[0] == ' ') remain++;
				}
			}
		}
	}
}

void Help(int argc, char *argv[])
{
	int				i = 0, j = 0, k, cols;
	bool				disp;
#ifdef TIOCGWINSZ
	struct winsize			w;
#endif
#if defined(WIN32) || defined(DJGPP)
#else
	char				*columns;
#endif

	/* Just --help */
	if (argc == 2) {
		HelpGeneral();
		return;
	}

	if (!strcmp(argv[2],"all")) {
		HelpHeader();
	} else {
		while (HelpDescriptions[i].category != 0) {
			if (strcasecmp(argv[2], HelpDescriptions[i].option) == 0) break;
			i++;
		}
		if (HelpDescriptions[i].category == 0) {
			HelpGeneral();
			printf("%s\n", _("Unknown help topic specified!"));
			return;
		}
		HelpHeader();
		printf(_("Gammu commands, topic: %s\n\n"), HelpDescriptions[i].description);
	}

#if defined(WIN32) || defined(DJGPP)
	cols = 80;
#else
	cols = 0;
	/* If stdout is a tty, we will wrap to columns it has */
	if (isatty(1)) {
#ifdef TIOCGWINSZ
		if (ioctl(2, TIOCGWINSZ, &w) == 0) {
			if (w.ws_col > 0) cols = w.ws_col;
		}
#endif
		if (cols == 0) {
			columns = getenv("COLUMNS");
			if (columns != NULL) {
				cols = atoi(columns);
				if (cols <= 0) cols = 0;
			}
		}

		if (cols == 0) {
			/* Fallback */
			cols = 80;
		}
	}
#endif

	while (Parameters[j].Function != NULL) {
		k 	= 0;
		disp 	= false;
		if (!strcmp(argv[2],"all")) {
			if (j==0) disp = true;
			if (j!=0) {
				if (strcmp(Parameters[j].help,Parameters[j-1].help)) {
					disp = true;
				} else {
					if (strcmp(Parameters[j].parameter,Parameters[j-1].parameter)) {
						disp = true;
					}
				}
			}
		} else {
			while (Parameters[j].help_cat[k] != 0) {
				if (Parameters[j].help_cat[k] == HelpDescriptions[i].category) {
					disp = true;
					break;
				}
				k++;
			}
		}
		if (disp) {
			printf("%s", Parameters[j].parameter);
			if (Parameters[j].help[0] == 0) {
				printf("\n");
			} else {
				HelpSplit(cols - 1, strlen(Parameters[j].parameter) + 1, gettext(Parameters[j].help));
			}
		}
		j++;
	}
}

int FoundVersion(unsigned char *Buffer)
{
	size_t retval = 0, pos = 0;

	retval = atoi(Buffer) * 10000;
	while (Buffer[pos] != '.') {
		pos++;
		if (pos == strlen(Buffer)) return retval;
	}
	pos++;
	retval += atoi(Buffer+pos) * 100;
	while (Buffer[pos] != '.') {
		pos++;
		if (pos == strlen(Buffer)) return retval;
	}
	pos++;
	return retval + atoi(Buffer+pos);
}

int ProcessParameters(char start, int argc, char *argv[]) {
	int 		z = 0;
 	bool		count_failed = false;

	/* Check parameters */
	while (Parameters[z].Function != NULL) {
		if (strcasecmp(Parameters[z].parameter, argv[1 + start]) == 0 ||
			(strncmp(argv[1 + start], "--", 2) == 0 &&
			strcasecmp(Parameters[z].parameter, argv[1 + start] + 2) == 0)
			) {
			if (argc-2-start < Parameters[z].min_arg) {
				if (!count_failed) {
					if (Parameters[z].min_arg==Parameters[z].max_arg) {
						printf(_("More parameters required (function requires %d)\n"), Parameters[z].min_arg);
					} else {
						printf(_("More parameters required (function requires %d to %d)\n"), Parameters[z].min_arg, Parameters[z].max_arg);
					}
					if (Parameters[z].help[0] != 0) {
						printf("%s:\n", _("Parameters help"));
					}
				}
				if (Parameters[z].help[0] != 0) {
					printf("%s\n", gettext(Parameters[z].help));
				}
				count_failed = true;
			} else if (argc-2-start > Parameters[z].max_arg) {
				if (!count_failed) {
					if (Parameters[z].min_arg==Parameters[z].max_arg) {
						printf(_("Too many parameters (function accepts %d)\n"), Parameters[z].min_arg);
					} else {
						printf(_("Too many parameters (function accepts %d to %d)\n"), Parameters[z].min_arg, Parameters[z].max_arg);
					}
					if (Parameters[z].help[0] != 0) {
						printf("%s:\n", _("Parameters help"));
					}
				}
				if (Parameters[z].help[0] != 0) {
					printf("%s\n", gettext(Parameters[z].help));
				}
				count_failed = true;
			} else {
				Parameters[z].Function(argc - start, argv + start);
 				break;
 			}
 		}
		z++;
	}

	/* Tell user when we did nothing */
	if (Parameters[z].Function == NULL) {
		if (!count_failed) {
			HelpGeneral();
			printf("%s\n", _("Bad option!"));
			return 2;
		}
		return 1;
 	}
	return 0;
}

int main(int argc, char *argv[])
{
	GSM_File	RSS;
	int		rsslevel = 0,oldpos = 0;
	size_t pos = 0;
	int 		start = 0;
	unsigned int	i;
	int		only_config = -1;
	char		*cp,*rss,buff[200];
	GSM_Config *smcfg;
	GSM_Config *smcfg0;
	GSM_Debug_Info *di;

	s = GSM_AllocStateMachine();

	GSM_InitLocales(NULL);

	di = GSM_GetGlobalDebug();

#ifdef DEBUG
	GSM_SetDebugFileDescriptor(stdout, di);
	GSM_SetDebugLevel("textall", di);
#endif

 	/* Any parameters? */
	if (argc == 1) {
		HelpGeneral();
		printf("%s\n", _("Too few parameters!"));
		exit(1);
	}

 	/* Help? */
	if (strcasecmp(argv[1 + start], "--help") == 0 ||
		strcasecmp(argv[1 + start], "-h") == 0 ||
		strcasecmp(argv[1 + start], "help") == 0) {
		Help(argc - start, argv + start);
		exit(1);
	}

 	/* Is first parameter numeric? If so treat it as config that should be loaded. */
	if (isdigit(argv[1][0])) {
		only_config = atoi(argv[1]);
		if (only_config >= 0) start++; else only_config = -1;
	}

 	error = GSM_FindGammuRC(&cfg);
	if (error != ERR_NONE) {
		if (error == ERR_FILENOTSUPPORTED) {
			printf_warn("%s\n", _("Configuration could not be parsed!"));
		} else {
			printf_warn("%s\n", _("No configuration file found!"));
		}
	}
 	if (cfg == NULL) printf_warn("%s\n", _("No configuration read!"));

	smcfg0 = GSM_GetConfig(s, 0);

	for (i = 0; (smcfg = GSM_GetConfig(s, i)) != NULL; i++) {
		/* Wanted user specific configuration? */
		if (only_config != -1) {
			smcfg = smcfg0;
			/* Here we get only in first for loop */
			if (!GSM_ReadConfig(cfg, smcfg, only_config)) break;
		} else {
			if (!GSM_ReadConfig(cfg, smcfg, i) && i != 0) break;
		}
		GSM_SetConfigNum(s, GSM_GetConfigNum(s) + 1);

		if (cfg!=NULL) {
		        cp = INI_GetValue(cfg, "gammu", "gammucoding", false);
        		if (cp) {
				GSM_SetDebugCoding(cp, di);
			}

		        smcfg->Localize = INI_GetValue(cfg, "gammu", "gammuloc", false);
			/* It is safe to pass NULL here */
			GSM_InitLocales(smcfg->Localize);
		}

     		/* We want to use only one file descriptor for global and state machine debug output */
 	    	smcfg->UseGlobalDebugFile = true;

		/* It makes no sense to open several debug logs... */
		if (i != 0) {
			strcpy(smcfg->DebugLevel, smcfg0->DebugLevel);
			free(smcfg->DebugFile);
			smcfg->DebugFile = strdup(smcfg0->DebugFile);
 		} else {
			/* Just for first config */
			/* When user gave debug level on command line */
			if (argc > 1 + start && GSM_SetDebugLevel(argv[1 + start], di)) {
				/* Debug level from command line will be used with phone too */
				strcpy(smcfg->DebugLevel,argv[1 + start]);
				start++;
			} else {
				/* Try to set debug level from config file */
				GSM_SetDebugLevel(smcfg->DebugLevel, di);
			}
			/* If user gave debug file in gammurc, we will use it */
			error=GSM_SetDebugFile(smcfg->DebugFile, di);
			Print_Error(error);
 		}

		if (i==0) {
		        rss = INI_GetValue(cfg, "gammu", "rsslevel", false);
        		if (rss) {
				if (strcasecmp(rss,"teststable") == 0) {
					rsslevel = 2;
				} else if (strcasecmp(rss,"stable") == 0) {
					rsslevel = 1;
				}
			}
		        rss = INI_GetValue(cfg, "gammu", "usephonedb", false);
        		if (rss && strcasecmp(rss,"yes") == 0) phonedb = true;
		}

 		/* We wanted to read just user specified configuration. */
 		if (only_config != -1) {break;}
 	}

	/* Do we have enough parameters? */
 	if (argc == 1 + start) {
 		HelpGeneral();
 		printf("%s\n", _("Too few parameters!"));
		exit(-2);
	}

	/* Check used version vs. compiled */
	if (!strcasecmp(GetGammuVersion(),VERSION) == 0) {
		printf_err(_("Version of installed libGammu.so (%s) is different to version of Gammu (%s)\n"),
					GetGammuVersion(),VERSION);
		exit(-1);
	}

	if (rsslevel > 0) {
		RSS.Buffer = NULL;
		if (GSM_ReadHTTPFile("blog.cihar.com","archives/gammu_releases/index-rss.xml",&RSS)) {
			while (pos < RSS.Used) {
				if (RSS.Buffer[pos] != 10) {
					pos++;
					continue;
				}
				RSS.Buffer[pos] = 0;
				if (strstr(RSS.Buffer+oldpos,"<title>") ==NULL ||
				    strstr(RSS.Buffer+oldpos,"</title>")==NULL ||
				    strstr(RSS.Buffer+oldpos,"win32")   != NULL) {
					pos++;
					oldpos = pos;
					continue;
				}
				if (rsslevel > 0 && strstr(RSS.Buffer+oldpos,"stable version")!=NULL) {
					sprintf(buff,strstr(RSS.Buffer+oldpos,"stable version")+15);
					for (i=0;i<strlen(buff);i++) {
						if (buff[i] == '<') {
							buff[i] = 0;
							break;
						}
					}
					if (FoundVersion(buff) > FoundVersion(VERSION)) {
						printf(_("INFO: there is later stable Gammu (%s instead of %s) available!\n"),buff,VERSION);
						break;
					}
				}
				if (rsslevel == 2 && strstr(RSS.Buffer+oldpos,"test version")!=NULL) {
					sprintf(buff,strstr(RSS.Buffer+oldpos,"test version")+13);
					for (i=0;i<strlen(buff);i++) {
						if (buff[i] == '<') {
							buff[i] = 0;
							break;
						}
					}
					if (FoundVersion(buff) > FoundVersion(VERSION)) {
						printf(_("INFO: there is later testing Gammu (%s instead of %s) available!\n"),buff,VERSION);
						break;
					}
				}
				pos++;
				oldpos = pos;
			}
			free(RSS.Buffer);
		}
	}


	ProcessParameters(start, argc, argv);

     	/* Close debug output if opened */
	GSM_SetDebugFileDescriptor(NULL, di);

	GSM_FreeStateMachine(s);

	exit(0);
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */


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
#include "gammu.h"
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

#ifdef HAVE_PTHREAD
#  include <pthread.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#  include <sys/ioctl.h>
#endif

#define LISTFORMAT "%-20s : "
#define PRINTSECONDS(num) printf(ngettext("%d second", "%d seconds", num), num);
#define PRINTMINUTES(num) printf(ngettext("%d minute", "%d minutes", num), num);
#define PRINTHOURS(num) printf(ngettext("%d hour", "%d hours", num), num);
#define PRINTDAYS(num) printf(ngettext("%d day", "%d days", num), num);
#define PRINTWEEKS(num) printf(ngettext("%d week", "%d weeks", num), num);
#define PRINTYEARS(num) printf(ngettext("%d year", "%d years", num), num);

/**
 * Hides default case in switch, to allow checking whether all cases are handled.
 */
#undef CHECK_CASES

/**
 * Global state machine used in Gammu.
 */
GSM_StateMachine		s;
GSM_Phone_Functions		*Phone;
static INI_Section		*cfg 			= NULL;

GSM_Error			error 			= ERR_NONE;
static int			i;

volatile bool 			gshutdown 		= false;
volatile bool 			wasincomingsms 		= false;
GSM_MultiSMSMessage		IncomingSMSData;
bool 				phonedb 		= false;
bool				batch			= false;
bool				batchConn		= false;

/**
 * Write error to user.
 */
#if defined(__GNUC__) && !defined(printf)
__attribute__((format(printf, 1, 2)))
#endif
int printf_err(const char *format, ...)
{
	va_list ap;
	int ret;

	/* l10n: Generic prefix for error messages */
	printf("%s: ", _("Error"));

	va_start(ap, format);
	ret = vprintf(format, ap);
	va_end(ap);

	return ret;
}

/**
 * Write warning to user.
 */
#if defined(__GNUC__) && !defined(printf)
__attribute__((format(printf, 1, 2)))
#endif
int printf_warn(const char *format, ...)
{
	va_list ap;
	int ret;

	/* l10n: Generic prefix for warning messages */
	printf("%s: ", _("Warning"));

	va_start(ap, format);
	ret = vprintf(format, ap);
	va_end(ap);

	return ret;
}

void interrupt(int sign)
{
	signal(sign, SIG_IGN);
	gshutdown = true;
}

static void PrintSecurityStatus()
{
	GSM_SecurityCodeType Status;

	error=Phone->GetSecurityStatus(&s,&Status);
	Print_Error(error);
	switch(Status) {
		case SEC_SecurityCode:
			printf("%s\n", _("Waiting for Security Code."));
			break;
		case SEC_Pin:
			printf("%s\n", _("Waiting for PIN."));
			break;
		case SEC_Pin2:
			printf("%s\n", _("Waiting for PIN2."));
			break;
		case SEC_Puk:
			printf("%s\n", _("Waiting for PUK."));
			break;
		case SEC_Puk2:
			printf("%s\n", _("Waiting for PUK2."));
			break;
		case SEC_None:
			printf("%s\n", _("Nothing to enter."));
			break;
#ifndef CHECK_CASES
		default:
			printf("%s\n", _("Unknown security status."));
#endif
	}
}

void Print_Error(GSM_Error error)
{
	if (error != ERR_NONE) {
 		printf("%s\n",print_error(error,s.di.df));
		if (error == ERR_SECURITYERROR) {
			printf(LISTFORMAT, _("Security status"));
			PrintSecurityStatus();
		}
		if (s.opened) GSM_TerminateConnection(&s);
 		exit (-1);
 	}
}

void GSM_Init(bool checkerror)
{
	GSM_File 	PhoneDB;
	unsigned char 	buff[200],ver[200];
	int	 	pos=0,oldpos=0,i;
	if (batch && batchConn) return;

	error=GSM_InitConnection(&s,3);
	if (checkerror) Print_Error(error);

	if (batch) {
		if (error == ERR_NONE) { batchConn=true; }
	}

	Phone=s.Phone.Functions;

	if (!phonedb) return;

	error=Phone->GetModel(&s);
	Print_Error(error);

	sprintf(buff,"support/phones/phonedbxml.php?model=%s",s.Phone.Data.Model);
	PhoneDB.Buffer = NULL;
	if (!GSM_ReadHTTPFile("www.gammu.org",buff,&PhoneDB)) return;

	while (pos < PhoneDB.Used) {
		if (PhoneDB.Buffer[pos] != 10) {
			pos++;
			continue;
		}
		PhoneDB.Buffer[pos] = 0;
		if (strstr(PhoneDB.Buffer+oldpos,"<firmware>")==NULL) {
			pos++;
			oldpos = pos;
			continue;
		}
		sprintf(ver,strstr(PhoneDB.Buffer+oldpos,"<version>")+9);
		for (i=0;i<(int)strlen(ver);i++) {
			if (ver[i] == '<') {
				ver[i] = 0;
				break;
			}
		}
		pos++;
		oldpos = pos;
	}
	free(PhoneDB.Buffer);

	error=Phone->GetFirmware(&s);
	Print_Error(error);
	if (s.Phone.Data.Version[0] == '0') {
		i=1;
	} else {
		i=0;
	}
	while(i!=strlen(ver)) {
		if (ver[i] > s.Phone.Data.Version[i]) {
			printf(_("INFO: there is later phone firmware (%s instead of %s) available !\n"), ver, s.Phone.Data.Version);
			return;
		}
		i++;
	}
}

void GSM_Terminate(void)
{
	if (!batch) {
		error=GSM_TerminateConnection(&s);
		Print_Error(error);
	}
}

static void GetStartStop(int *start, int *stop, int num, int argc, char *argv[])
{
	int tmp;

	if (argc <= num) {
		printf_err("%s\n", _("More parameters required!"));
		exit (-1);
	}

	*start=atoi(argv[num]);
	if (*start==0) {
		printf_err("%s\n",_("Please enumerate locations from 1"));
		exit (-1);
	}

	if (stop!=NULL) {
		*stop=*start;
		if (argc>=num+2) *stop=atoi(argv[num+1]);
		if (*stop==0) {
			printf_err("%s\n", _("Please enumerate locations from 1"));
			exit (-1);
		}
		if (*stop < *start) {
			printf_warn("%s\n", _("Swapping start and end location"));
			tmp    = *stop;
			*stop  = *start;
			*start = tmp;
		}
	}
}

bool always_answer_yes = false;
bool always_answer_no  = false;

static bool answer_yes(const char *text)
{
    	int         len;
    	char        ans[99];

	while (1) {
		/* l10n: %s is replaced by question, answers have to match corresponding translations */
		fprintf(stderr, _("%s (yes/no/ALL/ONLY/NONE) ? "),text);
		if (always_answer_yes) {
			fprintf(stderr, "%s\n", _("YES (always)"));
			return true;
		}
		if (always_answer_no) {
			fprintf(stderr,"%s\n",  _("NO (always)"));
			return false;
		}
		len=GetLine(stdin, ans, 99);
		if (len==-1) exit(-1);
		/* l10n: Answer to (yes/no/ALL/ONLY/NONE) question */
		if (!strcmp(ans, _("NONE"))) {
			always_answer_no = true;
			return false;
		}
		/* l10n: Answer to (yes/no/ALL/ONLY/NONE) question */
		if (!strcmp(ans, _("ONLY"))) {
			always_answer_no = true;
			return true;
		}
		/* l10n: Answer to (yes/no/ALL/ONLY/NONE) question */
		if (!strcmp(ans, _("ALL"))) {
			always_answer_yes = true;
			return true;
		}
		/* l10n: Answer to (yes/no/ALL/ONLY/NONE) question */
		if (strcasecmp(ans, _("yes")) == 0) return true;
		/* l10n: Answer to (yes/no/ALL/ONLY/NONE) question */
		if (strcasecmp(ans, _("no")) == 0) return false;
	}
}

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

static void PrintCalendar(GSM_CalendarEntry *Note)
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
					Note->Entries[i].Date.Month == 1 ? _("January") : (
					Note->Entries[i].Date.Month == 2 ? _("February") : (
					Note->Entries[i].Date.Month == 3 ? _("March") : (
					Note->Entries[i].Date.Month == 4 ? _("April") : (
					Note->Entries[i].Date.Month == 5 ? _("May") : (
					Note->Entries[i].Date.Month == 6 ? _("June") : (
					Note->Entries[i].Date.Month == 7 ? _("July") : (
					Note->Entries[i].Date.Month == 8 ? _("August") : (
					Note->Entries[i].Date.Month == 9 ? _("September") : (
					Note->Entries[i].Date.Month == 10 ? _("October") : (
					Note->Entries[i].Date.Month == 11 ? _("November") : (
					Note->Entries[i].Date.Month == 12 ? _("December") :
					_("bad month!")))))))))))));
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
					Note->Entries[i].Date.Month == 1 ? _("January") : (
					Note->Entries[i].Date.Month == 2 ? _("February") : (
					Note->Entries[i].Date.Month == 3 ? _("March") : (
					Note->Entries[i].Date.Month == 4 ? _("April") : (
					Note->Entries[i].Date.Month == 5 ? _("May") : (
					Note->Entries[i].Date.Month == 6 ? _("June") : (
					Note->Entries[i].Date.Month == 7 ? _("July") : (
					Note->Entries[i].Date.Month == 8 ? _("August") : (
					Note->Entries[i].Date.Month == 9 ? _("September") : (
					Note->Entries[i].Date.Month == 10 ? _("October") : (
					Note->Entries[i].Date.Month == 11 ? _("November") : (
					Note->Entries[i].Date.Month == 12 ? _("December") :
					_("bad month!")))))))))))));
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
			error=Phone->GetMemory(&s, &entry);
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
				switch (repeat_dayofweek) {
					case 1 : printf(_("Monday")); 	break;
					case 2 : printf(_("Tuesday")); 	break;
					case 3 : printf(_("Wednesday")); break;
					case 4 : printf(_("Thursday")); 	break;
					case 5 : printf(_("Friday")); 	break;
					case 6 : printf(_("Saturday")); 	break;
					case 7 : printf(_("Sunday")); 	break;
					default: printf(_("Bad day!")); 	break;
				}
				if (repeat_weekofmonth > 0) {
					printf(_(" in %d. week of "), repeat_weekofmonth);
				} else {
					printf(_(" in "));
				}
				if (repeat_month > 0) {
					switch(repeat_month) {
						case 1 : printf(_("January")); 	 break;
						case 2 : printf(_("February")); 	 break;
						case 3 : printf(_("March")); 	 break;
						case 4 : printf(_("April")); 	 break;
						case 5 : printf(_("May")); 	 break;
						case 6 : printf(_("June")); 	 break;
						case 7 : printf(_("July")); 	 break;
						case 8 : printf(_("August")); 	 break;
						case 9 : printf(_("September"));  break;
						case 10: printf(_("October")); 	 break;
						case 11: printf(_("November")); 	 break;
						case 12: printf(_("December")); 	 break;
						default: printf(_("Bad month!")); break;
					}
				} else {
					printf(_("each month"));
				}
			} else if (repeat_day > 0) {
				printf(_("%d. day of "), repeat_day);
				if (repeat_month > 0) {
					switch(repeat_month) {
						case 1 : printf(_("January")); 	break;
						case 2 : printf(_("February")); 	break;
						case 3 : printf(_("March"));	break;
						case 4 : printf(_("April")); 	break;
						case 5 : printf(_("May")); 	break;
						case 6 : printf(_("June")); 	break;
						case 7 : printf(_("July")); 	break;
						case 8 : printf(_("August")); 	break;
						case 9 : printf(_("September")); break;
						case 10: printf(_("October")); 	break;
						case 11: printf(_("November")); 	break;
						case 12: printf(_("December")); 	break;
						default: printf(_("Bad month!"));break;
					}
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

#ifdef GSM_ENABLE_BEEP
void GSM_PhoneBeep(void)
{
	error = PHONE_Beep(&s);
	if (error != ERR_NOTSUPPORTED && error != ERR_NOTIMPLEMENTED) Print_Error(error);
}
#endif

static GSM_Error GSM_PlayRingtone(GSM_Ringtone ringtone)
{
	int 		i;
	bool 		first=true;
	GSM_Error 	error;

	signal(SIGINT, interrupt);
	printf("%s\n", _("Press Ctrl+C to break..."));

	for (i=0;i<ringtone.NoteTone.NrCommands;i++) {
		if (gshutdown) break;
		if (ringtone.NoteTone.Commands[i].Type != RING_NOTETONE) continue;
		error=PHONE_RTTLPlayOneNote(&s,ringtone.NoteTone.Commands[i].Note,first);
		if (error!=ERR_NONE) return error;
		first = false;
	}

	/* Disables buzzer */
	return s.Phone.Functions->PlayTone(&s,255*255,0,false);
}

static void PlayRingtone(int argc, char *argv[])
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
	unsigned char buffer[100];

	GSM_Init(true);

	error=Phone->GetManufacturer(&s);
	Print_Error(error);
	printf(LISTFORMAT "%s\n", _("Manufacturer"), s.Phone.Data.Manufacturer);
	error=Phone->GetModel(&s);
	Print_Error(error);
	printf(LISTFORMAT "%s (%s)\n", _("Model"),
			s.Phone.Data.ModelInfo->model,
			s.Phone.Data.Model);

	error=Phone->GetFirmware(&s);
	Print_Error(error);
	printf(LISTFORMAT "%s", _("Firmware"),s.Phone.Data.Version);
	error=Phone->GetPPM(&s, buffer);
	if (error != ERR_NOTSUPPORTED) {
		if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
		if (error == ERR_NONE) printf(" %s", buffer);
	}
	if (s.Phone.Data.VerDate[0]!=0) printf(" (%s)", s.Phone.Data.VerDate);
	printf("\n");

	error=Phone->GetHardware(&s, buffer);
	if (error != ERR_NOTSUPPORTED) {
		if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
		if (error == ERR_NONE) printf(LISTFORMAT "%s\n", _("Hardware"),buffer);
	}

	error=Phone->GetIMEI(&s);
	if (error != ERR_NOTSUPPORTED) {
		if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
		if (error == ERR_NONE) printf(LISTFORMAT "%s\n", _("IMEI"),s.Phone.Data.IMEI);

		error=Phone->GetOriginalIMEI(&s, buffer);
		if (error != ERR_NOTSUPPORTED && error != ERR_SECURITYERROR) {
			if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
			if (error == ERR_NONE) printf(LISTFORMAT "%s\n", _("Original IMEI"),buffer);
		}
	}

	error=Phone->GetManufactureMonth(&s, buffer);
	if (error != ERR_NOTSUPPORTED && error != ERR_SECURITYERROR) {
		if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
		if (error == ERR_NONE) printf(LISTFORMAT "%s\n", _("Manufactured"),buffer);
	}

	error=Phone->GetProductCode(&s, buffer);
	if (error != ERR_NOTSUPPORTED) {
		if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
		if (error == ERR_NONE) printf(LISTFORMAT "%s\n", _("Product code"),buffer);
	}

	error=Phone->GetSIMIMSI(&s, buffer);
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

static void GetDateTime(int argc, char *argv[])
{
	GSM_DateTime 	date_time;
	GSM_Locale	locale;

	GSM_Init(true);

	error=Phone->GetDateTime(&s, &date_time);
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

	error=Phone->GetLocale(&s, &locale);
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
		error=Phone->GetDateTime(&s, &date_time);
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
		error=Phone->SetDateTime(&s, &date_time);
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
	error=Phone->GetAlarm(&s, &alarm);
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

static void SetAlarm(int argc, char *argv[])
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

	error=Phone->SetAlarm(&s, &alarm);
	Print_Error(error);

	GSM_Terminate();
}

GSM_Bitmap		caller[5];
GSM_AllRingtonesInfo 	Info = {0, NULL};
bool			callerinit[5] = {false, false, false, false, false};
bool			ringinit = false;

static void PrintMemorySubEntry(GSM_SubMemoryEntry *entry)
{
	GSM_Category	Category;
	int		z;

	switch (entry->EntryType) {
	case PBK_CallLength:
		printf(_("Call length      : %02i:%02i:%02i\n"),entry->CallLength/(60*60),entry->CallLength/60,entry->CallLength%60);
		return;
	case PBK_Date:
		printf(LISTFORMAT "%s\n", _("Date and time"),OSDateTime(entry->Date,false));
		return;
	case PBK_LastModified:
		printf(LISTFORMAT "%s\n", _("Last modified"), OSDateTime(entry->Date,false));
		return;
	case PBK_Category:
		if (entry->Number == -1) {
			printf(LISTFORMAT "\"%s\"\n", _("Category"), DecodeUnicodeConsole(entry->Text));
		} else {
			Category.Location = entry->Number;
			Category.Type = Category_Phonebook;
			error=Phone->GetCategory(&s, &Category);
			if (error == ERR_NONE) {
				printf(LISTFORMAT "\"%s\" (%i)\n", _("Category"), DecodeUnicodeConsole(Category.Name), entry->Number);
			} else {
				printf(LISTFORMAT "%i\n", _("Category"), entry->Number);
			}
		}
		return;
	case PBK_Private:
		printf(LISTFORMAT "%s\n", _("Private"), entry->Number == 1 ? "Yes" : "No");
		return;
	case PBK_Caller_Group       :
		if (entry->Number > 5) {
			printf(LISTFORMAT "\"%d\"\n", _("Caller group"),entry->Number);
			fprintf(stderr, "%s\n", _("Caller group number too high, please increase buffer in sources!"));
			break;
		}
		if (!callerinit[entry->Number-1]) {
			caller[entry->Number-1].Type	    = GSM_CallerGroupLogo;
			caller[entry->Number-1].Location = entry->Number;
			error=Phone->GetBitmap(&s,&caller[entry->Number-1]);
			Print_Error(error);
			if (caller[entry->Number-1].DefaultName) {
				NOKIA_GetDefaultCallerGroupName(&s,&caller[entry->Number-1]);
			}
			callerinit[entry->Number-1]=true;
		}
		printf(LISTFORMAT "\"%s\"\n", _("Caller group"),DecodeUnicodeConsole(caller[entry->Number-1].Text));
		return;
	case PBK_RingtoneID	     :
		if (!ringinit) {
			error=Phone->GetRingtonesInfo(&s,&Info);
			if (error != ERR_NOTSUPPORTED) Print_Error(error);
			if (error == ERR_NONE) ringinit = true;
		}
		if (ringinit) {
			for (z=0;z<Info.Number;z++) {
				if (Info.Ringtone[z].ID == entry->Number) {
					printf(LISTFORMAT "\"%s\"\n", _("Ringtone"),DecodeUnicodeConsole(Info.Ringtone[z].Name));
					break;
				}
			}
		} else {
			printf(LISTFORMAT "%i\n", _("Ringtone ID"),entry->Number);
		}
		return;
	case PBK_Text_UserID:
		printf(LISTFORMAT "%s\n", _("User ID"),DecodeUnicodeString(entry->Text));
		return;
	case PBK_PictureID	     :
		printf(LISTFORMAT "0x%x\n", _("Picture ID"),entry->Number);
		return;
	case PBK_Number_General     : printf(LISTFORMAT, _("General number")); break;
	case PBK_Number_Mobile      : printf(LISTFORMAT, _("Mobile number")); break;
	case PBK_Number_Work        : printf(LISTFORMAT, _("Work number")); break;
	case PBK_Number_Fax         : printf(LISTFORMAT, _("Fax number")); break;
	case PBK_Number_Home        : printf(LISTFORMAT, _("Home number")); break;
	case PBK_Number_Pager       : printf(LISTFORMAT, _("Pager number")); break;
	case PBK_Number_Other       : printf(LISTFORMAT, _("Other number")); break;
	case PBK_Text_Note          : printf(LISTFORMAT, _("Text")); break;
	case PBK_Text_Postal        : printf(LISTFORMAT, _("Snail address")); break;
	case PBK_Text_Email         : printf(LISTFORMAT, _("Email address 1")); break;
	case PBK_Text_Email2        : printf(LISTFORMAT, _("Email address 2")); break;
	case PBK_Text_URL           : printf(LISTFORMAT, _("URL address")); break;
	case PBK_Text_LUID          : printf(LISTFORMAT, _("LUID")); break;
	case PBK_Text_Name          : printf(LISTFORMAT, _("Name")); break;
	case PBK_Text_LastName      : printf(LISTFORMAT, _("Last name")); break;
	case PBK_Text_FirstName     : printf(LISTFORMAT, _("First name")); break;
	case PBK_Text_Company       : printf(LISTFORMAT, _("Company")); break;
	case PBK_Text_JobTitle      : printf(LISTFORMAT, _("Job title")); break;
	case PBK_Text_StreetAddress : printf(LISTFORMAT, _("Street address")); break;
	case PBK_Text_City          : printf(LISTFORMAT, _("City")); break;
	case PBK_Text_State         : printf(LISTFORMAT, _("State")); break;
	case PBK_Text_Zip           : printf(LISTFORMAT, _("Zip code")); break;
	case PBK_Text_Country       : printf(LISTFORMAT, _("Country")); break;
	case PBK_Text_Custom1       : printf(LISTFORMAT, _("Custom text 1")); break;
	case PBK_Text_Custom2       : printf(LISTFORMAT, _("Custom text 2")); break;
	case PBK_Text_Custom3       : printf(LISTFORMAT, _("Custom text 3")); break;
	case PBK_Text_Custom4       : printf(LISTFORMAT, _("Custom text 4")); break;
#ifndef CHECK_CASES
	default:
		printf("%s\n", _("unknown field type"));
		return;
#endif
	}
	printf(" : \"%s\"\n", DecodeUnicodeConsole(entry->Text));
}

static void PrintMemoryEntry(GSM_MemoryEntry *entry)
{
	int i;

	for (i=0;i<entry->EntriesNum;i++) PrintMemorySubEntry(&entry->Entries[i]);
	printf("\n");
}

static void GetAllMemory(int argc, char *argv[])
{
	GSM_MemoryEntry		Entry;
	bool			start = true;

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));

	Entry.MemoryType = 0;

	if (strcasecmp(argv[2],"DC") == 0) Entry.MemoryType=MEM_DC;
	if (strcasecmp(argv[2],"ON") == 0) Entry.MemoryType=MEM_ON;
	if (strcasecmp(argv[2],"RC") == 0) Entry.MemoryType=MEM_RC;
	if (strcasecmp(argv[2],"MC") == 0) Entry.MemoryType=MEM_MC;
	if (strcasecmp(argv[2],"ME") == 0) Entry.MemoryType=MEM_ME;
	if (strcasecmp(argv[2],"SM") == 0) Entry.MemoryType=MEM_SM;
	if (strcasecmp(argv[2],"VM") == 0) Entry.MemoryType=MEM_VM;
	if (strcasecmp(argv[2],"FD") == 0) Entry.MemoryType=MEM_FD;
	if (strcasecmp(argv[2],"SL") == 0) Entry.MemoryType=MEM_SL;
	if (Entry.MemoryType==0) {
		printf_err(_("Unknown memory type (\"%s\")\n"),argv[2]);
		exit (-1);
	}

	GSM_Init(true);

	while (!gshutdown) {
		error = Phone->GetNextMemory(&s, &Entry, start);
		if (error == ERR_EMPTY) break;
		if (error != ERR_NONE && Info.Ringtone) free(Info.Ringtone);
		Print_Error(error);
		printf(_("Memory %s, Location %i\n"),argv[2],Entry.Location);
		PrintMemoryEntry(&Entry);
 		start = false;
	}

 	if (Info.Ringtone) free(Info.Ringtone);

	GSM_Terminate();
}

static void GetMemory(int argc, char *argv[])
{
	int			j, start, stop, emptynum = 0, fillednum = 0;
	GSM_MemoryEntry		entry;
	bool			empty = true;

	entry.MemoryType=0;

	if (strcasecmp(argv[2],"DC") == 0) entry.MemoryType=MEM_DC;
	if (strcasecmp(argv[2],"ON") == 0) entry.MemoryType=MEM_ON;
	if (strcasecmp(argv[2],"RC") == 0) entry.MemoryType=MEM_RC;
	if (strcasecmp(argv[2],"MC") == 0) entry.MemoryType=MEM_MC;
	if (strcasecmp(argv[2],"ME") == 0) entry.MemoryType=MEM_ME;
	if (strcasecmp(argv[2],"SM") == 0) entry.MemoryType=MEM_SM;
	if (strcasecmp(argv[2],"VM") == 0) entry.MemoryType=MEM_VM;
	if (strcasecmp(argv[2],"FD") == 0) entry.MemoryType=MEM_FD;
	if (strcasecmp(argv[2],"SL") == 0) entry.MemoryType=MEM_SL;
	if (entry.MemoryType==0) {
		printf_err(_("Unknown memory type (\"%s\")\n"),argv[2]);
		exit (-1);
	}

	GetStartStop(&start, &stop, 3, argc, argv);

	if (argc > 5 && strcmp(argv[5],"")) {
		if (strcasecmp(argv[5],"-nonempty") == 0) {
			empty = false;
		} else {
			printf_err(_("Unknown parameter (\"%s\")\n"),argv[5]);
			exit (-1);
		}
	}

	GSM_Init(true);

	if (!strcmp(s.Phone.Data.ModelInfo->model,"3310")) {
		if (s.Phone.Data.VerNum<=4.06) printf_warn("%s\n", _("You will have null names in entries. Upgrade firmware in phone to higher than 4.06"));
	}

	for (j=start;j<=stop;j++) {
		if (empty) printf(_("Memory %s, Location %i\n"),argv[2],j);

		entry.Location=j;

		error=Phone->GetMemory(&s, &entry);
		if (error != ERR_EMPTY) {
			if (Info.Ringtone) free(Info.Ringtone);
			Print_Error(error);
		}

		if (error == ERR_EMPTY) {
			emptynum++;
			if (empty) {
				printf("%s\n", _("Entry is empty"));
				printf("\n");
			}
		} else {
			fillednum++;
			if (!empty) printf(_("Memory %s, Location %i\n"),argv[2],j);
			PrintMemoryEntry(&entry);
		}
	}

	printf(_("%i entries empty, %i entries filled\n"),emptynum,fillednum);

 	if (Info.Ringtone) free(Info.Ringtone);

	GSM_Terminate();
}

#define MemoryLocationToString(x) ( \
	x == MEM_ON ? "ON" :			\
	x == MEM_RC ? "RC" :			\
	x == MEM_MC ? "MC" :			\
	x == MEM_ME ? "ME" :			\
	x == MEM_SM ? "SM" :			\
	x == MEM_SL ? "SL" :			\
	x == MEM_VM ? "VM" :			\
	x == MEM_FD ? "FD" : "XX")

static void SearchOneEntry(GSM_MemoryEntry *Entry, unsigned char *Text)
{
	int i;

	for (i=0;i<Entry->EntriesNum;i++) {
		switch (Entry->Entries[i].EntryType) {
			case PBK_Number_General     :
			case PBK_Number_Mobile      :
			case PBK_Number_Work        :
			case PBK_Number_Fax         :
			case PBK_Number_Home        :
			case PBK_Number_Pager       :
			case PBK_Number_Other       :
			case PBK_Text_Note          :
			case PBK_Text_Postal        :
			case PBK_Text_Email         :
			case PBK_Text_Email2        :
			case PBK_Text_URL           :
			case PBK_Text_LUID          :
			case PBK_Text_Name          :
			case PBK_Text_LastName      :
			case PBK_Text_FirstName     :
			case PBK_Text_Company       :
			case PBK_Text_JobTitle      :
			case PBK_Text_StreetAddress :
			case PBK_Text_City          :
			case PBK_Text_State         :
			case PBK_Text_Zip           :
			case PBK_Text_Country       :
			case PBK_Text_Custom1       :
			case PBK_Text_Custom2       :
			case PBK_Text_Custom3       :
			case PBK_Text_Custom4       :
			case PBK_Text_UserID:
			case PBK_Caller_Group       :
				if (mywstrstr(Entry->Entries[i].Text, Text) != NULL) {
					fprintf(stderr,"\n");
					printf(_("Memory %s, Location %i\n"),MemoryLocationToString(Entry->MemoryType),Entry->Location);
					PrintMemoryEntry(Entry);
					return;
				}
				break;
#ifndef CHECK_CASES
			default:
				break;
#endif
		}
	}
}

static void SearchOneMemory(GSM_MemoryType MemoryType, char *Title, unsigned char *Text)
{
	GSM_MemoryEntry		Entry;
	GSM_MemoryStatus	Status;
	int			i = 0, l = 1;
	bool			start = true;

	Status.MemoryType = MemoryType;
	Entry.MemoryType  = MemoryType;

	if (Phone->GetMemoryStatus(&s, &Status) == ERR_NONE) {
		fprintf(stderr,"%c%s: %i%%", 13, Title, (i+1)*100/(Status.MemoryUsed+1));
		if (Phone->GetNextMemory != NOTSUPPORTED && Phone->GetNextMemory != NOTIMPLEMENTED) {
			while (i < Status.MemoryUsed) {
				if (gshutdown) return;
				i++;
				fprintf(stderr,"\r%s: %i%%", Title, (i+1)*100/(Status.MemoryUsed+1));
				error = Phone->GetNextMemory(&s, &Entry, start);
				if (error == ERR_EMPTY) break;
				Print_Error(error);
				SearchOneEntry(&Entry, Text);
				start = false;
			}
		} else {
			while (i < Status.MemoryUsed) {
				Entry.Location = l;
				error = Phone->GetMemory(&s, &Entry);
				if (error != ERR_EMPTY) {
					Print_Error(error);
					i++;
					SearchOneEntry(&Entry, Text);
				}
				fprintf(stderr,"%c%s: %i%%", 13, Title, (i+1)*100/(Status.MemoryUsed+1));
				l++;
			}
		}
		fprintf(stderr,"\n");
	}
}

static void SearchMemory(int argc, char *argv[])
{
	unsigned char		Text[(GSM_PHONEBOOK_TEXT_LENGTH+1)*2];
	int			Length;

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));

	Length = strlen(argv[2]);
	if (Length > GSM_PHONEBOOK_TEXT_LENGTH) {
		printf(_("Search text too long, truncating to %d chars!\n"), GSM_PHONEBOOK_TEXT_LENGTH);
		Length = GSM_PHONEBOOK_TEXT_LENGTH;
	}
	EncodeUnicode(Text, argv[2], Length);

	GSM_Init(true);

	if (!gshutdown) SearchOneMemory(MEM_ME, "Phone phonebook", 	Text);
	if (!gshutdown) SearchOneMemory(MEM_SM, "SIM phonebook", 	Text);
	if (!gshutdown) SearchOneMemory(MEM_ON, "Own numbers", 		Text);
	if (!gshutdown) SearchOneMemory(MEM_DC, "Dialled numbers", 	Text);
	if (!gshutdown) SearchOneMemory(MEM_RC, "Received numbers", 	Text);
	if (!gshutdown) SearchOneMemory(MEM_MC, "Missed numbers", 	Text);
	if (!gshutdown) SearchOneMemory(MEM_FD, "Fix dialling", 	Text);
	if (!gshutdown) SearchOneMemory(MEM_VM, "Voice mailbox", 	Text);
	if (!gshutdown) SearchOneMemory(MEM_SL, "Sent SMS log", 	Text);

	GSM_Terminate();
}

static void ListMemoryCategoryEntries(int Category)
{
	GSM_MemoryEntry		Entry;
	bool			start = true;
	int			j;

	/* Category can be only for ME stored entries */
	Entry.MemoryType  = MEM_ME;

	while (!gshutdown) {
		error = Phone->GetNextMemory(&s, &Entry, start);
		if (error == ERR_EMPTY) break;
		Print_Error(error);
		for (j=0;j<Entry.EntriesNum;j++) {
			if (Entry.Entries[j].EntryType == PBK_Category && Entry.Entries[j].Number == Category) {
				printf(_("Memory %s, Location %i\n"),MemoryLocationToString(Entry.MemoryType),Entry.Location);
				PrintMemoryEntry(&Entry);
			}
		}
 		start = false;
	}
}

static void ListMemoryCategory(int argc, char *argv[])
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
			ListMemoryCategoryEntries(j);
		}
	} else {
		if (Length > GSM_MAX_CATEGORY_NAME_LENGTH) {
			printf(_("Search text too long, truncating to %d chars!\n"), GSM_MAX_CATEGORY_NAME_LENGTH);
			Length = GSM_MAX_CATEGORY_NAME_LENGTH;
		}
		EncodeUnicode(Text, argv[2], Length);

		Category.Type 	= Category_Phonebook;
		Status.Type 	= Category_Phonebook;

		if (Phone->GetCategoryStatus(&s, &Status) == ERR_NONE) {
			for (count=0,j=1;count<Status.Used;j++) {
				Category.Location=j;
				error=Phone->GetCategory(&s, &Category);

				if (error != ERR_EMPTY) {
					count++;
					if (mywstrstr(Category.Name, Text) != NULL) {
						ListMemoryCategoryEntries(j);
					}
				}
			}
		}
	}
	GSM_Terminate();
}

static void printsmsnumber(unsigned char *number,GSM_Backup *Info)
{
	bool 	found=false,found2=false;
	int 	i,j,z;

	printf("\"%s\"",DecodeUnicodeConsole(number));

	if (Info==NULL) return;

	i=0;
	while (Info->PhonePhonebook[i]!=NULL) {
		for (j=0;j<Info->PhonePhonebook[i]->EntriesNum;j++) {
			switch (Info->PhonePhonebook[i]->Entries[j].EntryType) {
			case PBK_Number_General:
			case PBK_Number_Mobile:
			case PBK_Number_Work:
			case PBK_Number_Fax:
			case PBK_Number_Home:
			case PBK_Number_Pager:
			case PBK_Number_Other:
				if (mywstrncmp(Info->PhonePhonebook[i]->Entries[j].Text,number,-1)) {
					found2=true;
					switch (Info->PhonePhonebook[i]->Entries[j].EntryType) {
					case PBK_Number_Mobile:
						printf("(%s", _("mobile"));
						break;
					case PBK_Number_Work:
						printf("(%s", _("work"));
						break;
					case PBK_Number_Fax:
						printf("(%s", _("fax"));
						break;
					case PBK_Number_Home:
						printf("(%s", _("home"));
						break;
					case PBK_Number_Pager:
						printf("(%s", _("pager"));
						break;
					default:
						found2=false;
						break;
					}
					found=true;
				}
			default:
				break;
			}
			if (found) break;
		}
		if (!found) {
			i++;
			continue;
		}
		found=false;
		for (z=0;z<Info->PhonePhonebook[i]->EntriesNum;z++) {
			switch (Info->PhonePhonebook[i]->Entries[z].EntryType) {
			case PBK_Text_LastName:
			case PBK_Text_FirstName:
				if (!found2) {
					printf(" (");
					found2=true;
				} else {
					if (!found) {
						printf(", ");
					} else {
						printf(" ");
					}
				}
				printf("%s",DecodeUnicodeConsole(Info->PhonePhonebook[i]->Entries[z].Text));
				found=true;
				break;
			default:
				break;
			}
		}
		for (z=0;z<Info->PhonePhonebook[i]->EntriesNum;z++) {
			switch (Info->PhonePhonebook[i]->Entries[z].EntryType) {
			case PBK_Text_Name:
				if (!found2) {
					printf(" (");
					found2=true;
				} else {
					printf(", ");
				}
				printf("%s",DecodeUnicodeConsole(Info->PhonePhonebook[i]->Entries[z].Text));
				break;
			default:
				break;
			}
		}
		printf(")");
		break;
	}
}

static void displaysinglesmsinfo(GSM_SMSMessage sms, bool displaytext, bool displayudh, GSM_Backup *Info)
{
	GSM_SiemensOTASMSInfo 	SiemensOTA;
	int			i;

	switch (sms.PDU) {
	case SMS_Status_Report:
		printf("%s\n", _("SMS status report"));

		printf(LISTFORMAT, _("Status"));
		switch (sms.State) {
			case SMS_Sent	: printf(_("Sent"));	break;
			case SMS_Read	: printf(_("Read"));	break;
			case SMS_UnRead	: printf(_("UnRead"));	break;
			case SMS_UnSent	: printf(_("UnSent"));	break;
		}
		printf("\n");

		printf(LISTFORMAT, _("Remote number"));
		printsmsnumber(sms.Number, Info);
		printf("\n");

		printf(LISTFORMAT "%d\n", _("Reference number"),sms.MessageReference);
		printf(LISTFORMAT "%s\n", _("Sent"),OSDateTime(sms.DateTime,true));
		printf(LISTFORMAT "\"%s\"\n", _("SMSC number"),DecodeUnicodeConsole(sms.SMSC.Number));
		printf(LISTFORMAT "%s\n", _("SMSC response"),OSDateTime(sms.SMSCTime,true));
		printf(LISTFORMAT "%s\n", _("Delivery status"),DecodeUnicodeConsole(sms.Text));
		printf(LISTFORMAT, _("Details"));
		if (sms.DeliveryStatus & 0x40) {
			if (sms.DeliveryStatus & 0x20) {
				printf(_("Temporary error, "));
			} else {
	     			printf(_("Permanent error, "));
			}
	    	} else if (sms.DeliveryStatus & 0x20) {
			printf(_("Temporary error, "));
		}
		switch (sms.DeliveryStatus) {
			case 0x00: printf(_("SM received by the SME"));				break;
			case 0x01: printf(_("SM forwarded by the SC to the SME but the SC is unable to confirm delivery"));break;
			case 0x02: printf(_("SM replaced by the SC"));				break;
			case 0x20: printf(_("Congestion"));					break;
			case 0x21: printf(_("SME busy"));					break;
			case 0x22: printf(_("No response from SME"));				break;
			case 0x23: printf(_("Service rejected"));				break;
			case 0x24: printf(_("Quality of service not aviable"));			break;
			case 0x25: printf(_("Error in SME"));					break;
		        case 0x40: printf(_("Remote procedure error"));				break;
		        case 0x41: printf(_("Incompatibile destination"));			break;
		        case 0x42: printf(_("Connection rejected by SME"));			break;
		        case 0x43: printf(_("Not obtainable"));					break;
		        case 0x44: printf(_("Quality of service not available"));		break;
		        case 0x45: printf(_("No internetworking available"));			break;
		        case 0x46: printf(_("SM Validity Period Expired"));			break;
		        case 0x47: printf(_("SM deleted by originating SME"));			break;
		        case 0x48: printf(_("SM Deleted by SC Administration"));			break;
		        case 0x49: printf(_("SM does not exist"));				break;
		        case 0x60: printf(_("Congestion"));					break;
		        case 0x61: printf(_("SME busy"));					break;
		        case 0x62: printf(_("No response from SME"));				break;
		        case 0x63: printf(_("Service rejected"));				break;
		        case 0x64: printf(_("Quality of service not available"));		break;
		        case 0x65: printf(_("Error in SME"));					break;
		        default  : printf(_("Reserved/Specific to SC: %x"),sms.DeliveryStatus);	break;
		}
		printf("\n");
		break;
	case SMS_Deliver:
		printf("%s\n", _("SMS message"));
		if (sms.State==SMS_UnSent && sms.Memory==MEM_ME) {
			printf(LISTFORMAT "%s\n", _("Saved"), OSDateTime(sms.DateTime,true));
		} else {
			printf(LISTFORMAT "\"%s\"", _("SMSC number"), DecodeUnicodeConsole(sms.SMSC.Number));
			if (sms.ReplyViaSameSMSC) printf(_(" (set for reply)"));
			printf("\n");
			printf(LISTFORMAT "%s\n", _("Sent"), OSDateTime(sms.DateTime,true));
		}
		/* No break. The only difference for SMS_Deliver and SMS_Submit is,
		 * that SMS_Deliver contains additional data. We wrote them and then go
		 * for data shared with SMS_Submit
		 */
	case SMS_Submit:
		if (sms.ReplaceMessage != 0) printf(LISTFORMAT "%i\n", _("SMS replacing ID"),sms.ReplaceMessage);
		/* If we went here from "case SMS_Deliver", we don't write "SMS Message" */
		if (sms.PDU==SMS_Submit) {
			printf("%s\n", _("SMS message"));
			if (sms.State==SMS_UnSent && sms.Memory==MEM_ME) {
			} else {
				printf(LISTFORMAT "%d\n", _("Reference number"),sms.MessageReference);
			}
		}
		if (sms.Name[0] != 0x00 || sms.Name[1] != 0x00) {
			printf(LISTFORMAT "\"%s\"\n", _("Name"),DecodeUnicodeConsole(sms.Name));
		}
		if (sms.Class != -1) {
			printf(LISTFORMAT "%i\n", _("Class"),sms.Class);
		}
		printf(LISTFORMAT, _("Coding"));
		switch (sms.Coding) {
			case SMS_Coding_Unicode_No_Compression 	: printf("%s\n", _("Unicode (no compression)"));		break;
			case SMS_Coding_Unicode_Compression 	: printf("%s\n", _("Unicode (compression)"));			break;
			case SMS_Coding_Default_No_Compression 	: printf("%s\n", _("Default GSM alphabet (no compression)"));	break;
			case SMS_Coding_Default_Compression 	: printf("%s\n", _("Default GSM alphabet (compression)"));	break;
			case SMS_Coding_8bit			: printf("%s\n", _("8 bit"));					break;
		}
		if (sms.State==SMS_UnSent && sms.Memory==MEM_ME) {
		} else {
			printf(LISTFORMAT, ngettext("Remote number", "Remote numbers", sms.OtherNumbersNum + 1));
			printsmsnumber(sms.Number, Info);
			for (i=0;i<sms.OtherNumbersNum;i++) {
				printf(_(", "));
				printsmsnumber(sms.OtherNumbers[i], Info);
			}
			printf("\n");
		}
		printf(LISTFORMAT, _("Status"));
		switch (sms.State) {
			case SMS_Sent	:	printf("%s\n", _("Sent"));	break;
			case SMS_Read	:	printf("%s\n", _("Read"));	break;
			case SMS_UnRead	:	printf("%s\n", _("UnRead"));	break;
			case SMS_UnSent	:	printf("%s\n", _("UnSent"));	break;
		}
		if (sms.UDH.Type != UDH_NoUDH) {
			printf(LISTFORMAT, _("User Data Header"));
			switch (sms.UDH.Type) {
			case UDH_ConcatenatedMessages	   : printf(_("Concatenated (linked) message")); 	 break;
			case UDH_ConcatenatedMessages16bit : printf(_("Concatenated (linked) message")); 	 break;
			case UDH_DisableVoice		   : printf(_("Disables voice indicator"));	 	 break;
			case UDH_EnableVoice		   : printf(_("Enables voice indicator"));	 	 break;
			case UDH_DisableFax		   : printf(_("Disables fax indicator"));	 	 break;
			case UDH_EnableFax		   : printf(_("Enables fax indicator"));	 		 break;
			case UDH_DisableEmail		   : printf(_("Disables email indicator"));	 	 break;
			case UDH_EnableEmail		   : printf(_("Enables email indicator"));	 	 break;
			case UDH_VoidSMS		   : printf(_("Void SMS"));			 	 break;
			case UDH_NokiaWAP		   : printf(_("Nokia WAP bookmark"));		 	 break;
			case UDH_NokiaOperatorLogoLong	   : printf(_("Nokia operator logo"));	 	 	 break;
			case UDH_NokiaWAPLong		   : printf(_("Nokia WAP bookmark or WAP/MMS settings")); break;
			case UDH_NokiaRingtone		   : printf(_("Nokia ringtone"));		 	 break;
			case UDH_NokiaRingtoneLong	   : printf(_("Nokia ringtone"));		 	 break;
			case UDH_NokiaOperatorLogo	   : printf(_("Nokia GSM operator logo"));	 	 break;
			case UDH_NokiaCallerLogo	   : printf(_("Nokia caller logo"));		 	 break;
			case UDH_NokiaProfileLong	   : printf(_("Nokia profile"));		 		 break;
			case UDH_NokiaCalendarLong	   : printf(_("Nokia calendar note"));	 		 break;
			case UDH_NokiaPhonebookLong	   : printf(_("Nokia phonebook entry"));	 		 break;
			case UDH_UserUDH		   : printf(_("User UDH"));			 	 break;
			case UDH_MMSIndicatorLong	   : printf(_("MMS indicator"));			 	 break;
			case UDH_NoUDH:								 		 break;
			}
			if (sms.UDH.Type != UDH_NoUDH) {
				if (sms.UDH.ID8bit != -1) printf(_(", ID (8 bit) %i"),sms.UDH.ID8bit);
				if (sms.UDH.ID16bit != -1) printf(_(", ID (16 bit) %i"),sms.UDH.ID16bit);
				if (sms.UDH.PartNumber != -1 && sms.UDH.AllParts != -1) {
					if (displayudh) {
						printf(_(", part %i of %i"),sms.UDH.PartNumber,sms.UDH.AllParts);
					} else {
						printf(_(", %i parts"),sms.UDH.AllParts);
					}
				}
			}
			printf("\n");
		}
		if (displaytext) {
			printf("\n");
			if (sms.Coding!=SMS_Coding_8bit) {
				printf("%s\n",DecodeUnicodeConsole(sms.Text));
			} else {
				if (GSM_DecodeSiemensOTASMS(&SiemensOTA,&sms)) {
					printf("%s\n", _("Siemens file"));
					break;
				}
				printf("%s\n", _("8 bit SMS, cannot be displayed here"));
			}
		}
		break;
	}
}

static void displaymultismsinfo (GSM_MultiSMSMessage sms, bool eachsms, bool ems, GSM_Backup *Info)
{
	GSM_SiemensOTASMSInfo 	SiemensOTA;
	GSM_MultiPartSMSInfo	SMSInfo;
	bool			RetVal,udhinfo=true;
	int			j,Pos;
	GSM_MemoryEntry		pbk;

	/* GSM_DecodeMultiPartSMS returns if decoded SMS content correctly */
	RetVal = GSM_DecodeMultiPartSMS(&SMSInfo,&sms,ems);

	if (eachsms) {
		if (GSM_DecodeSiemensOTASMS(&SiemensOTA,&sms.SMS[0])) udhinfo = false;
		if (sms.SMS[0].UDH.Type != UDH_NoUDH && sms.SMS[0].UDH.AllParts == sms.Number) udhinfo = false;
		if (RetVal && !udhinfo) {
			displaysinglesmsinfo(sms.SMS[0],false,false,Info);
			printf("\n");
		} else {
			for (j=0;j<sms.Number;j++) {
				displaysinglesmsinfo(sms.SMS[j],!RetVal,udhinfo,Info);
				printf("\n");
			}
		}
	} else {
		for (j=0;j<sms.Number;j++) {
			displaysinglesmsinfo(sms.SMS[j],!RetVal,true,Info);
			printf("\n");
		}
	}
	if (!RetVal) {
		GSM_FreeMultiPartSMSInfo(&SMSInfo);
		return;
	}

	if (SMSInfo.Unknown) printf("%s\n\n", _("Some details were ignored (unknown or not implemented in decoding functions)"));

	for (i=0;i<SMSInfo.EntriesNum;i++) {
		switch (SMSInfo.Entries[i].ID) {
		case SMS_SiemensFile:
			printf(_("Siemens OTA file"));
			if (strstr(DecodeUnicodeString(SMSInfo.Entries[i].File->Name),".vcf")) {
				printf("%s\n", _(" - VCARD"));
				SMSInfo.Entries[i].File->Buffer = realloc(SMSInfo.Entries[i].File->Buffer,1+SMSInfo.Entries[i].File->Used);
				SMSInfo.Entries[i].File->Buffer[SMSInfo.Entries[i].File->Used] = 0;
				SMSInfo.Entries[i].File->Used += 1;
				Pos = 0;
				error = GSM_DecodeVCARD(SMSInfo.Entries[i].File->Buffer, &Pos, &pbk, Nokia_VCard21);
				if (error == ERR_NONE) PrintMemoryEntry(&pbk);
			} else {
				printf("\n");
			}
			break;
		case SMS_NokiaRingtone:
			printf(_("Ringtone \"%s\"\n"),DecodeUnicodeConsole(SMSInfo.Entries[i].Ringtone->Name));
			saverttl(stdout,SMSInfo.Entries[i].Ringtone);
			printf("\n");
			if (s.Phone.Functions->PlayTone!=NOTSUPPORTED &&
			    s.Phone.Functions->PlayTone!=NOTIMPLEMENTED) {
				if (answer_yes(_("Do you want to play it"))) GSM_PlayRingtone(*SMSInfo.Entries[i].Ringtone);
			}
			break;
		case SMS_NokiaCallerLogo:
			printf("%s\n\n", _("Caller logo"));
			GSM_PrintBitmap(stdout,&SMSInfo.Entries[i].Bitmap->Bitmap[0]);
			break;
		case SMS_NokiaOperatorLogo:
			printf(_("Operator logo for %s network (%s, %s)\n\n"),
				SMSInfo.Entries[i].Bitmap->Bitmap[0].NetworkCode,
				DecodeUnicodeConsole(GSM_GetNetworkName(SMSInfo.Entries[i].Bitmap->Bitmap[0].NetworkCode)),
				DecodeUnicodeConsole(GSM_GetCountryName(SMSInfo.Entries[i].Bitmap->Bitmap[0].NetworkCode)));
			GSM_PrintBitmap(stdout,&SMSInfo.Entries[i].Bitmap->Bitmap[0]);
			break;
		case SMS_NokiaScreenSaverLong:
			printf("%s\n", _("Screen saver"));
			GSM_PrintBitmap(stdout,&SMSInfo.Entries[i].Bitmap->Bitmap[0]);
			break;
		case SMS_NokiaPictureImageLong:
			printf("%s\n", _("Picture Image"));
			if (UnicodeLength(SMSInfo.Entries[i].Bitmap->Bitmap[0].Text)!=0) printf(LISTFORMAT "\"%s\"\n\n", _("Text"),DecodeUnicodeConsole(SMSInfo.Entries[i].Bitmap->Bitmap[0].Text));
			GSM_PrintBitmap(stdout,&SMSInfo.Entries[i].Bitmap->Bitmap[0]);
			break;
		case SMS_NokiaProfileLong:
			printf("%s\n", _("Profile"));
			GSM_PrintBitmap(stdout,&SMSInfo.Entries[i].Bitmap->Bitmap[0]);
			break;
		case SMS_ConcatenatedTextLong:
		case SMS_ConcatenatedAutoTextLong:
		case SMS_ConcatenatedTextLong16bit:
		case SMS_ConcatenatedAutoTextLong16bit:
		case SMS_NokiaVCARD21Long:
		case SMS_NokiaVCALENDAR10Long:
			printf("%s\n",DecodeUnicodeConsole(SMSInfo.Entries[i].Buffer));
			break;
		case SMS_EMSFixedBitmap:
		case SMS_EMSVariableBitmap:
			GSM_PrintBitmap(stdout,&SMSInfo.Entries[i].Bitmap->Bitmap[0]);
			break;
		case SMS_EMSAnimation:
			/* Can't show animation, we show first frame */
			GSM_PrintBitmap(stdout,&SMSInfo.Entries[i].Bitmap->Bitmap[0]);
			break;
		case SMS_EMSPredefinedSound:
			printf("\n" LISTFORMAT "%i\n", _("EMS sound ID"),SMSInfo.Entries[i].Number);
			break;
		case SMS_EMSPredefinedAnimation:
			printf("\n" LISTFORMAT "%i\n", _("EMS animation ID"),SMSInfo.Entries[i].Number);
			break;
#ifndef CHECK_CASES
		default:
			printf("%s\n", _("Error"));
			break;
#endif
		}
	}
	printf("\n");
	GSM_FreeMultiPartSMSInfo(&SMSInfo);
}

static void NetworkInfo(int argc, char *argv[])
{
	GSM_NetworkInfo		NetInfo;

	GSM_Init(true);

	if (Phone->GetNetworkInfo(&s,&NetInfo)==ERR_NONE) {
		PrintNetworkInfo(NetInfo);
	}
	GSM_Terminate();
}

static void IncomingSMS(GSM_StateMachine *s, GSM_SMSMessage sms)
{
	printf("%s\n", _("SMS message received"));
 	if (wasincomingsms) {
 		printf("%s\n", _("We already have one pending, ignoring!"));
 		return;
 	}
 	wasincomingsms = true;
 	memcpy(&IncomingSMSData.SMS[0],&sms,sizeof(GSM_SMSMessage));
 	IncomingSMSData.Number = 1;
}

static void DisplayIncomingSMS()
{
 	GSM_SMSFolders folders;

 	if (IncomingSMSData.SMS[0].State == 0) {
 		error=Phone->GetSMSFolders(&s, &folders);
 		Print_Error(error);

 		error=Phone->GetSMS(&s, &IncomingSMSData);
 		switch (error) {
 		case ERR_EMPTY:
 			printf(_("Location %i\n"),IncomingSMSData.SMS[0].Location);
 			printf("%s\n", _("Empty"));
 			break;
 		default:
 			Print_Error(error);
 			printf(_("Location %i, folder \"%s\""),IncomingSMSData.SMS[0].Location,DecodeUnicodeConsole(folders.Folder[IncomingSMSData.SMS[0].Folder-1].Name));
 			switch(IncomingSMSData.SMS[0].Memory) {
 				case MEM_SM: printf(_(", SIM memory")); 		break;
 				case MEM_ME: printf(_(", phone memory")); 	break;
 				case MEM_MT: printf(_(", phone or SIM memory")); break;
 				default    : break;
 			}
 			if (IncomingSMSData.SMS[0].InboxFolder) printf(_(", Inbox folder"));
 			printf("\n");
 		}
 	}
 	displaymultismsinfo(IncomingSMSData,false,false,NULL);
 	wasincomingsms = false;
}

static void IncomingCB(GSM_StateMachine *s, GSM_CBMessage CB)
{
	printf("%s\n", _("CB message received"));
	printf(_("Channel %i, text \"%s\"\n"),CB.Channel,DecodeUnicodeConsole(CB.Text));
}

static void IncomingCall(GSM_StateMachine *s, GSM_Call call)
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

static void IncomingUSSD(GSM_StateMachine *s, char *Buffer)
{
	printf(LISTFORMAT "\"%s\"\n", _("Service reply"),DecodeUnicodeConsole(Buffer));
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
	if ( (error = Phone->GetMemoryStatus(&s, &x)) == ERR_NONE)			\
		PRINTUSED(name, x.MemoryUsed, x.MemoryFree);  \
}

#define CHECK_EXIT \
{ \
	if (gshutdown) break; \
	if (error != ERR_NONE && error != ERR_NOTSUPPORTED && error != ERR_EMPTY && error != ERR_SOURCENOTAVAILABLE && error != ERR_NOTIMPLEMENTED) break; \
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

	s.User.IncomingSMS 	= IncomingSMS;
	s.User.IncomingCB 	= IncomingCB;
	s.User.IncomingCall 	= IncomingCall;
	s.User.IncomingUSSD 	= IncomingUSSD;

	error=Phone->SetIncomingSMS  		(&s,true);
	printf("%-35s : %s\n", _("Enabling info about incoming SMS"), print_error(error,NULL));
	error=Phone->SetIncomingCB   		(&s,true);
	printf("%-35s : %s\n", _("Enabling info about incoming CB"), print_error(error,NULL));
	error=Phone->SetIncomingCall 		(&s,true);
	printf("%-35s : %s\n", _("Enabling info about calls"), print_error(error,NULL));
	error=Phone->SetIncomingUSSD 		(&s,true);
	printf("%-35s : %s\n", _("Enabling info about USSD"), print_error(error,NULL));

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
		if ( (error = Phone->GetToDoStatus(&s, &ToDoStatus)) == ERR_NONE) {
			PRINTUSED(_("ToDos"), ToDoStatus.Used, ToDoStatus.Free);
		}
		CHECK_EXIT;
		if ( (error = Phone->GetCalendarStatus(&s, &CalendarStatus)) == ERR_NONE) {
			PRINTUSED(_("Calendar"), CalendarStatus.Used, CalendarStatus.Free);
		}
		CHECK_EXIT;
		if ( (error = Phone->GetBatteryCharge(&s,&BatteryCharge)) == ERR_NONE) {
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
		if ( (error = Phone->GetSignalQuality(&s,&SignalQuality)) == ERR_NONE) {
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
		if ( (error = Phone->GetSMSStatus(&s,&SMSStatus)) == ERR_NONE) {
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
		if ( (error = Phone->GetNetworkInfo(&s,&NetInfo)) == ERR_NONE) {
			PrintNetworkInfo(NetInfo);
		}
		if (wasincomingsms) DisplayIncomingSMS();
		printf("\n");
	}

	printf("%s\n", _("Leaving monitor mode..."));

	GSM_Terminate();
}

static void IncomingUSSD2(GSM_StateMachine *s, char *Buffer)
{
	printf(LISTFORMAT "\"%s\"\n", _("Service reply"),DecodeUnicodeConsole(Buffer));

	gshutdown = true;
}

static void GetUSSD(int argc, char *argv[])
{
	GSM_Init(true);

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));

	s.User.IncomingUSSD = IncomingUSSD2;

	error=Phone->SetIncomingUSSD(&s,true);
	Print_Error(error);

	error=Phone->DialVoice(&s, argv[2], GSM_CALL_DefaultNumberPresence);
	Print_Error(error);

	while (!gshutdown) GSM_ReadDevice(&s,true);

	GSM_Terminate();
}

static void GetSMSC(int argc, char *argv[])
{
	GSM_SMSC 	smsc;
	int 		start, stop;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		smsc.Location=i;

		error=Phone->GetSMSC(&s, &smsc);
		Print_Error(error);

		if (!strcmp(DecodeUnicodeConsole(smsc.Name),"")) {
			printf(_("%i. Set %i\n"),smsc.Location, smsc.Location);
		} else {
			printf("%i. \"%s\"\n",smsc.Location, DecodeUnicodeConsole(smsc.Name));
		}
		printf(LISTFORMAT "\"%s\"\n", _("Number"),DecodeUnicodeConsole(smsc.Number));
		printf(LISTFORMAT "\"%s\"\n", _("Default number"),DecodeUnicodeConsole(smsc.DefaultNumber));

		printf(LISTFORMAT, _("Format"));
		switch (smsc.Format) {
			case SMS_FORMAT_Text	: printf(_("Text"));	break;
			case SMS_FORMAT_Fax	: printf(_("Fax"));	break;
			case SMS_FORMAT_Email	: printf(_("Email"));	break;
			case SMS_FORMAT_Pager	: printf(_("Pager"));	break;
		}
		printf("\n");

		printf(LISTFORMAT, _("Validity"));

		switch (smsc.Validity.Relative) {
			case SMS_VALID_1_Hour	: 
				PRINTHOURS(1);
				break;
			case SMS_VALID_6_Hours 	: 
				PRINTHOURS(6);
				break;
			case SMS_VALID_1_Day	: 
				PRINTDAYS(1);
				break;
			case SMS_VALID_3_Days	: 
				PRINTDAYS(3);
				break;
			case SMS_VALID_1_Week  	: 
				PRINTWEEKS(1);
				break;
			case SMS_VALID_Max_Time	: 
				printf(_("Maximum time")); 
				break;
			default           	:
				if (smsc.Validity.Relative >= 0 && smsc.Validity.Relative <= 143) {
					PRINTMINUTES((smsc.Validity.Relative + 1) * 5);
				} else if (smsc.Validity.Relative >= 144 && smsc.Validity.Relative <= 167) {
					PRINTMINUTES(12 * 60 + (smsc.Validity.Relative - 143) * 30);
				} else if (smsc.Validity.Relative >= 168 && smsc.Validity.Relative <= 196) {
					PRINTDAYS(smsc.Validity.Relative - 166);
				} else if (smsc.Validity.Relative >= 197 && smsc.Validity.Relative <= 255) {
					PRINTWEEKS(smsc.Validity.Relative - 192);
				}
		}
		printf("\n");
	}

	GSM_Terminate();
}

static void GetSMS(int argc, char *argv[])
{
	GSM_MultiSMSMessage	sms;
	GSM_SMSFolders		folders;
	int			start, stop;
	int			j;

	GetStartStop(&start, &stop, 3, argc, argv);

	GSM_Init(true);

	error=Phone->GetSMSFolders(&s, &folders);
	Print_Error(error);

	for (j = start; j <= stop; j++) {
		sms.SMS[0].Folder	= atoi(argv[2]);
		sms.SMS[0].Location	= j;
		error=Phone->GetSMS(&s, &sms);
		switch (error) {
		case ERR_EMPTY:
			printf(_("Location %i\n"),sms.SMS[0].Location);
			printf("%s\n", _("Empty"));
			break;
		default:
			Print_Error(error);
			printf(_("Location %i, folder \"%s\""),sms.SMS[0].Location,DecodeUnicodeConsole(folders.Folder[sms.SMS[0].Folder-1].Name));
			switch(sms.SMS[0].Memory) {
				case MEM_SM: printf(_(", SIM memory")); 		break;
				case MEM_ME: printf(_(", phone memory")); 	break;
				case MEM_MT: printf(_(", phone or SIM memory")); break;
				default    : break;
			}
			if (sms.SMS[0].InboxFolder) printf(_(", Inbox folder"));
			printf("\n");
			displaymultismsinfo(sms,false,false,NULL);
		}
	}

	GSM_Terminate();
}

static void DeleteSMS(int argc, char *argv[])
{
	GSM_SMSMessage	sms;
	int		start, stop;

	sms.Folder=atoi(argv[2]);

	GetStartStop(&start, &stop, 3, argc, argv);

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		sms.Location	= i;
		error=Phone->DeleteSMS(&s, &sms);
		Print_Error(error);
	}
#ifdef GSM_ENABLE_BEEP
	GSM_PhoneBeep();
#endif
	GSM_Terminate();
}

static void GetAllSMS(int argc, char *argv[])
{
	GSM_MultiSMSMessage 	sms;
	GSM_SMSFolders		folders;
	bool			start = true;
	int			smsnum=0,smspos=0;
#ifdef GSM_ENABLE_BACKUP
	int			used,i;
	GSM_MemoryStatus	MemStatus;
	GSM_MemoryEntry		Pbk;
	GSM_Backup		Backup;

	GSM_ClearBackup(&Backup);
#endif

	GSM_Init(true);

#ifdef GSM_ENABLE_BACKUP
	if (argc == 3 && strcasecmp(argv[2],"-pbk") == 0) {
		MemStatus.MemoryType = MEM_ME;
		error=Phone->GetMemoryStatus(&s, &MemStatus);
		if (error==ERR_NONE && MemStatus.MemoryUsed != 0) {
			Pbk.MemoryType  = MEM_ME;
			i		= 1;
			used 		= 0;
			while (used != MemStatus.MemoryUsed) {
				Pbk.Location = i;
				error=Phone->GetMemory(&s, &Pbk);
				if (error != ERR_EMPTY) {
					Print_Error(error);
					if (used < GSM_BACKUP_MAX_PHONEPHONEBOOK) {
						Backup.PhonePhonebook[used] = malloc(sizeof(GSM_MemoryEntry));
					        if (Backup.PhonePhonebook[used] == NULL) Print_Error(ERR_MOREMEMORY);
						Backup.PhonePhonebook[used+1] = NULL;
					} else {
						printf(_("\n   Only part of data saved - increase %s") , "GSM_BACKUP_MAX_PHONEPHONEBOOK");
						break;
					}
					*Backup.PhonePhonebook[used]=Pbk;
					used++;
				}
				fprintf(stderr, _("%cReading phone phonebook: %i percent"),13,used*100/MemStatus.MemoryUsed);
				i++;
				if (gshutdown) {
					GSM_Terminate();
					exit(0);
				}
			}
			fprintf(stderr, "\n");
		}
	}
#endif

	error=Phone->GetSMSFolders(&s, &folders);
	Print_Error(error);

	while (error == ERR_NONE) {
		sms.SMS[0].Folder=0x00;
		error=Phone->GetNextSMS(&s, &sms, start);
		switch (error) {
		case ERR_EMPTY:
			break;
		default:
			Print_Error(error);
			printf(_("Location %i, folder \"%s\""),sms.SMS[0].Location,DecodeUnicodeConsole(folders.Folder[sms.SMS[0].Folder-1].Name));
			switch(sms.SMS[0].Memory) {
				case MEM_SM: printf(_(", SIM memory")); 		break;
				case MEM_ME: printf(_(", phone memory")); 	break;
				case MEM_MT: printf(_(", phone or SIM memory")); break;
				default    : break;
			}
			if (sms.SMS[0].InboxFolder) printf(_(", Inbox folder"));
			printf("\n");
			smspos++;
			smsnum+=sms.Number;
#ifdef GSM_ENABLE_BACKUP
			if (Backup.PhonePhonebook[0]!=NULL) {
				displaymultismsinfo(sms,false,false,&Backup);
			} else {
#endif
				displaymultismsinfo(sms,false,false,NULL);
#ifdef GSM_ENABLE_BACKUP
			}
#endif
		}
		start=false;
	}
	printf(_("\n\n%i SMS parts in %i SMS sequences\n"),smsnum,smspos);

#ifdef GSM_ENABLE_BEEP
	GSM_PhoneBeep();
#endif
	GSM_Terminate();
}

static void GetEachSMS(int argc, char *argv[])
{
	GSM_MultiSMSMessage	*GetSMS[PHONE_MAXSMSINFOLDER],*SortedSMS[PHONE_MAXSMSINFOLDER],sms;
	int			GetSMSNumber = 0,i,j;
	int			smsnum=0,smspos=0;
	GSM_SMSFolders		folders;
	bool			start = true, ems = true;
#ifdef GSM_ENABLE_BACKUP
	GSM_MemoryStatus	MemStatus;
	GSM_MemoryEntry		Pbk;
	int			used;
	GSM_Backup		Backup;

	GSM_ClearBackup(&Backup);
#endif

	GetSMS[0] = NULL;

	GSM_Init(true);

#ifdef GSM_ENABLE_BACKUP
	if (argc == 3 && strcasecmp(argv[2],"-pbk") == 0) {
		MemStatus.MemoryType = MEM_ME;
		error=Phone->GetMemoryStatus(&s, &MemStatus);
		if (error==ERR_NONE && MemStatus.MemoryUsed != 0) {
			Pbk.MemoryType  = MEM_ME;
			i		= 1;
			used 		= 0;
			while (used != MemStatus.MemoryUsed) {
				Pbk.Location = i;
				error=Phone->GetMemory(&s, &Pbk);
				if (error != ERR_EMPTY) {
					Print_Error(error);
					if (used < GSM_BACKUP_MAX_PHONEPHONEBOOK) {
						Backup.PhonePhonebook[used] = malloc(sizeof(GSM_MemoryEntry));
					        if (Backup.PhonePhonebook[used] == NULL) Print_Error(ERR_MOREMEMORY);
						Backup.PhonePhonebook[used+1] = NULL;
					} else {
						printf(_("\n   Only part of data saved - increase %s") , "GSM_BACKUP_MAX_PHONEPHONEBOOK");
						break;
					}
					*Backup.PhonePhonebook[used]=Pbk;
					used++;
				}
				fprintf(stderr, _("%cReading phone phonebook: %i percent"),13,used*100/MemStatus.MemoryUsed);
				i++;
				if (gshutdown) {
					GSM_Terminate();
					exit(0);
				}
			}
			fprintf(stderr, "\n");
		}
	}
#endif

	error=Phone->GetSMSFolders(&s, &folders);
	Print_Error(error);

	fprintf(stderr, LISTFORMAT, _("Reading"));
	while (error == ERR_NONE) {
		if (GetSMSNumber==PHONE_MAXSMSINFOLDER-1) {
			fprintf(stderr, "\n%s\n", _("SMS counter overflow"));
			break;
		}
		sms.SMS[0].Folder=0x00;
		error=Phone->GetNextSMS(&s, &sms, start);
		switch (error) {
		case ERR_EMPTY:
			break;
		default:
			Print_Error(error);
			GetSMS[GetSMSNumber] = malloc(sizeof(GSM_MultiSMSMessage));
		        if (GetSMS[GetSMSNumber] == NULL) Print_Error(ERR_MOREMEMORY);
			GetSMS[GetSMSNumber+1] = NULL;
			memcpy(GetSMS[GetSMSNumber],&sms,sizeof(GSM_MultiSMSMessage));
			GetSMSNumber++;
		}
		fprintf(stderr,"*");
		start=false;
	}
	fprintf(stderr,"\n");

#ifdef GSM_ENABLE_BEEP
	GSM_PhoneBeep();
#endif

	error = GSM_LinkSMS(GetSMS, SortedSMS, ems);
	Print_Error(error);

	i=0;
	while(GetSMS[i] != NULL) {
		free(GetSMS[i]);
		GetSMS[i] = NULL;
		i++;
	}

	i=0;
	while(SortedSMS[i] != NULL) {
		smspos++;
		for (j=0;j<SortedSMS[i]->Number;j++) {
			smsnum++;
			if ((j==0) || (j!=0 && SortedSMS[i]->SMS[j].Location != SortedSMS[i]->SMS[j-1].Location)) {
				printf(_("Location %i, folder \"%s\""),SortedSMS[i]->SMS[j].Location,DecodeUnicodeConsole(folders.Folder[SortedSMS[i]->SMS[j].Folder-1].Name));
				switch(SortedSMS[i]->SMS[j].Memory) {
					case MEM_SM: printf(_(", SIM memory")); 		break;
					case MEM_ME: printf(_(", phone memory")); 	break;
					case MEM_MT: printf(_(", phone or SIM memory")); break;
					default    : break;
				}
				if (SortedSMS[i]->SMS[j].InboxFolder) printf(_(", Inbox folder"));
				printf("\n");
			}
		}
#ifdef GSM_ENABLE_BACKUP
		if (Backup.PhonePhonebook[0]!=NULL) {
			displaymultismsinfo(*SortedSMS[i],true,ems,&Backup);
		} else {
#endif
			displaymultismsinfo(*SortedSMS[i],true,ems,NULL);
#ifdef GSM_ENABLE_BACKUP
		}
#endif

		free(SortedSMS[i]);
		SortedSMS[i] = NULL;
		i++;
	}

	printf(_("\n%i SMS parts in %i SMS sequences\n"),smsnum,smspos);

	GSM_Terminate();
}

static void GetSMSFolders(int argc, char *argv[])
{
	GSM_SMSFolders folders;

	GSM_Init(true);

	error=Phone->GetSMSFolders(&s,&folders);
	Print_Error(error);

	for (i=0;i<folders.Number;i++) {
		printf("%i. \"%30s\"",i+1,DecodeUnicodeConsole(folders.Folder[i].Name));
		switch(folders.Folder[i].Memory) {
			case MEM_SM: printf(_(", SIM memory")); 		break;
			case MEM_ME: printf(_(", phone memory")); 	break;
			case MEM_MT: printf(_(", phone or SIM memory")); break;
			default    : break;
		}
		if (folders.Folder[i].InboxFolder) printf(_(", Inbox folder"));
		if (folders.Folder[i].OutboxFolder) printf(_(", Outbox folder"));
		printf("\n");
	}

	GSM_Terminate();
}

static void GetMMSFolders(int argc, char *argv[])
{
	GSM_MMSFolders folders;

	GSM_Init(true);

	error=Phone->GetMMSFolders(&s,&folders);
	Print_Error(error);

	for (i=0;i<folders.Number;i++) {
		printf("%i. \"%s\"",i+1,DecodeUnicodeConsole(folders.Folder[i].Name));
		if (folders.Folder[i].InboxFolder) printf(_(", Inbox folder"));
		printf("\n");
	}

	GSM_Terminate();
}

void DecodeMMSFile(GSM_File *file, int num)
{
	int				i,Pos;
	char				buff[200];
	GSM_EncodedMultiPartMMSInfo2 	info;
	GSM_Error			error;
	FILE				*file2;
	GSM_MemoryEntry			pbk;
	GSM_CalendarEntry 		Calendar;
	GSM_ToDoEntry 			ToDo;

	if (num != -1 && answer_yes(_("Do you want to save this MMS file"))) {
		sprintf(buff,"%i_0",num);
		file2 = fopen(buff,"wb");
		fwrite(file->Buffer, 1, file->Used, file2);
		fclose(file2);
		printf(_("Saved to file %s\n"),buff);
	}

	for (i=0;i<MAX_MULTI_MMS;i++) info.Entries[i].File.Buffer = NULL;
	GSM_ClearMMSMultiPart(&info);

	error = GSM_DecodeMMSFileToMultiPart(file, &info);
	if (error == ERR_FILENOTSUPPORTED) {
		printf_warn("%s\n", _("Some MMS file features unknown for Gammu decoder"));
		return;
	}
	Print_Error(error);

	if (UnicodeLength(info.Source) != 0) {
		printf(LISTFORMAT, _("Sender"));
		switch (info.SourceType) {
			case MMSADDRESS_PHONE: printf(_("phone ")); break;
			default: 				 break;
		}
		printf("%s\n",DecodeUnicodeString(info.Source));
	}
	if (UnicodeLength(info.Destination) != 0) {
		printf(LISTFORMAT, _("Recipient"));
		switch (info.DestinationType) {
			case MMSADDRESS_PHONE: printf(_("phone ")); break;
			default: 				 break;
		}
		printf("%s\n",DecodeUnicodeString(info.Destination));
	}
	if (UnicodeLength(info.CC) != 0) {
		printf(LISTFORMAT, _("CC"));
		switch (info.CCType) {
			case MMSADDRESS_PHONE: printf(_("phone ")); break;
			default: 				 break;
		}
		printf("%s\n",DecodeUnicodeString(info.CC));
	}
	printf(LISTFORMAT "%s\n", _("Message type"), info.MSGType);
	if (info.DateTimeAvailable) printf(LISTFORMAT "%s\n", _("Date"), OSDateTime(info.DateTime,0));
	if (UnicodeLength(info.Subject) != 0) {
		printf(LISTFORMAT "%s\n", _("Subject"),DecodeUnicodeString(info.Subject));
	}
	if (info.MMSReportAvailable) {
		printf(LISTFORMAT, _("Delivery report"));
		if (info.MMSReport) {
			printf("%s\n", _("enabled"));
		} else {
			printf("%s\n", _("disabled"));
		}
	}
	printf(LISTFORMAT "%s\n", _("Content type"), DecodeUnicodeString(info.ContentType));

	for (i=0;i<info.EntriesNum;i++) {
		printf(LISTFORMAT "%s\n", _("Content type"), DecodeUnicodeString(info.Entries[i].ContentType));
		if (UnicodeLength(info.Entries[i].File.Name) != 0) {
			printf(LISTFORMAT "%s", _("Name"),DecodeUnicodeString(info.Entries[i].File.Name));
			if (UnicodeLength(info.Entries[i].SMIL) != 0) {
				printf(_(" (%s in SMIL)"),DecodeUnicodeString(info.Entries[i].SMIL));
			}
			printf("\n");
		}
		if (!strcmp(DecodeUnicodeString(info.Entries[i].ContentType),"text/x-vCard")) {
			Pos = 0;
			printf("\n");
			error = GSM_DecodeVCARD(info.Entries[i].File.Buffer, &Pos, &pbk, Nokia_VCard21);
			if (error == ERR_NONE) PrintMemoryEntry(&pbk);
		}
		if (!strcmp(DecodeUnicodeString(info.Entries[i].ContentType),"text/x-vCalendar")) {
			Pos = 0;
			printf("\n");
			error = GSM_DecodeVCALENDAR_VTODO(info.Entries[i].File.Buffer, &Pos, &Calendar, &ToDo, Nokia_VCalendar, Nokia_VToDo);
			if (error == ERR_NONE) PrintCalendar(&Calendar);
		}
		if (num != -1 && answer_yes(_("Do you want to save this attachment"))) {
			sprintf(buff,"%i_%i_%s",num,i+1,DecodeUnicodeString(info.Entries[i].File.Name));
			file2 = fopen(buff,"wb");
			fwrite(info.Entries[i].File.Buffer, 1, info.Entries[i].File.Used, file2);
			fclose(file2);
			printf(_("Saved to file %s\n"),buff);
		}

	}

	GSM_ClearMMSMultiPart(&info);
}

static void GetEachMMS(int argc, char *argv[])
{
	int FileFolder;
	GSM_File		File;
	bool			start = true;
	GSM_MMSFolders 		folders;
	int			Handle,Size,num=-1;

	if (argc>2 && strcasecmp(argv[2],"-save") == 0) num=0;

	GSM_Init(true);

	error=Phone->GetMMSFolders(&s,&folders);
	Print_Error(error);

	File.Buffer = NULL;

	while (1) {
		error=Phone->GetNextMMSFileInfo(&s,File.ID_FullName,&FileFolder,start);
		if (error==ERR_EMPTY) break;
		Print_Error(error);
		start = false;

		printf(_("Folder %s\n"),DecodeUnicodeConsole(folders.Folder[FileFolder-1].Name));
		printf(LISTFORMAT "\"%s\"\n", _("  File filesystem ID"),DecodeUnicodeConsole(File.ID_FullName));
		if (!File.ModifiedEmpty) {
			printf(_("  File last changed  : %s\n"),OSDateTime(File.Modified,0));
		}
		if (File.Buffer != NULL) {
			free(File.Buffer);
			File.Buffer = NULL;
		}
		File.Used = 0;
		while (true) {
			error = Phone->GetFilePart(&s,&File,&Handle,&Size);
			if (error == ERR_EMPTY) break;
			Print_Error(error);
			fprintf(stderr, _("%c  Reading: %i percent"),13,File.Used*100/Size);
		}
		fprintf(stderr, "%c",13);

		if (IsPhoneFeatureAvailable(s.Phone.Data.ModelInfo, F_SERIES40_30)) {
			memcpy(File.Buffer,File.Buffer+176,File.Used-176);
			File.Used-=176;
			File.Buffer = realloc(File.Buffer,File.Used);
		}

		DecodeMMSFile(&File,num);
		if (num!=-1) num++;
	}

	if (File.Buffer != NULL) free(File.Buffer);

	GSM_Terminate();
}

static void GetRingtone(int argc, char *argv[])
{
	GSM_Ringtone 	ringtone;
	bool		PhoneRingtone = false;

	if (strcasecmp(argv[1],"--getphoneringtone") == 0) PhoneRingtone = true;

	GetStartStop(&ringtone.Location, NULL, 2, argc, argv);

	GSM_Init(true);

	ringtone.Format=0;

	error=Phone->GetRingtone(&s,&ringtone,PhoneRingtone);
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

static void GetRingtonesList(int argc, char *argv[])
{
 	GSM_AllRingtonesInfo 	Info = {0, NULL};
	int			i;

	GSM_Init(true);

	error=Phone->GetRingtonesInfo(&s,&Info);
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

	error=Phone->DialVoice(&s, argv[2], ShowNumber);
	Print_Error(error);

	GSM_Terminate();
}

int TerminateID = -1;

static void IncomingCall0(GSM_StateMachine *s, GSM_Call call)
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
	s.User.IncomingCall = IncomingCall0;

	error=Phone->SetIncomingCall(&s,true);
	Print_Error(error);

	error=Phone->DialVoice(&s, argv[2], ShowNumber);
	Print_Error(error);

//	GSM_GetCurrentDateTime (&DT);
//	one = Fill_Time_T(DT);

//	while (true) {
		my_sleep(atoi(argv[3]));
//		GSM_GetCurrentDateTime (&DT);
//		two = Fill_Time_T(DT);
//		if (two - one > atoi(argv[3])) break;
		GSM_ReadDevice(&s,true);
//	}

	if (TerminateID != -1) {
		error=Phone->CancelCall(&s,TerminateID,false);
		Print_Error(error);
	}

	GSM_Terminate();
}


static void CancelCall(int argc, char *argv[])
{
	GSM_Init(true);

	if (argc>2) {
		error=Phone->CancelCall(&s,atoi(argv[2]),false);
	} else {
		error=Phone->CancelCall(&s,0,true);
	}
	Print_Error(error);

	GSM_Terminate();
}

static void AnswerCall(int argc, char *argv[])
{
	GSM_Init(true);

	if (argc>2) {
		error=Phone->AnswerCall(&s,atoi(argv[2]),false);
	} else {
		error=Phone->AnswerCall(&s,0,true);
	}
	Print_Error(error);

	GSM_Terminate();
}

static void UnholdCall(int argc, char *argv[])
{
	GSM_Init(true);

	error=Phone->UnholdCall(&s,atoi(argv[2]));
	Print_Error(error);

	GSM_Terminate();
}

static void HoldCall(int argc, char *argv[])
{
	GSM_Init(true);

	error=Phone->HoldCall(&s,atoi(argv[2]));
	Print_Error(error);

	GSM_Terminate();
}

static void ConferenceCall(int argc, char *argv[])
{
	GSM_Init(true);

	error=Phone->ConferenceCall(&s,atoi(argv[2]));
	Print_Error(error);

	GSM_Terminate();
}

static void SplitCall(int argc, char *argv[])
{
	GSM_Init(true);

	error=Phone->SplitCall(&s,atoi(argv[2]));
	Print_Error(error);

	GSM_Terminate();
}

static void SwitchCall(int argc, char *argv[])
{
	GSM_Init(true);

	if (argc > 2) {
		error=Phone->SwitchCall(&s,atoi(argv[2]),false);
	} else {
		error=Phone->SwitchCall(&s,0,true);
	}
	Print_Error(error);

	GSM_Terminate();
}

static void TransferCall(int argc, char *argv[])
{
	GSM_Init(true);

	if (argc > 2) {
		error=Phone->TransferCall(&s,atoi(argv[2]),false);
	} else {
		error=Phone->TransferCall(&s,0,true);
	}
	Print_Error(error);

	GSM_Terminate();
}

static void AddSMSFolder(int argc, char *argv[])
{
	unsigned char buffer[200];

	GSM_Init(true);

	EncodeUnicode(buffer,argv[2],strlen(argv[2]));
	error=Phone->AddSMSFolder(&s,buffer);
	Print_Error(error);

	GSM_Terminate();
}

static void Reset(int argc, char *argv[])
{
	bool hard;

	if (strcasecmp(argv[2],"SOFT") == 0) {		hard=false;
	} else if (strcasecmp(argv[2],"HARD") == 0) {	hard=true;
	} else {
		printf(_("What type of reset do you want (\"%s\") ?\n"),argv[2]);
		exit(-1);
	}

	GSM_Init(true);

	error=Phone->Reset(&s, hard);
	Print_Error(error);

	GSM_Terminate();
}

static void GetCalendar(int argc, char *argv[])
{
	GSM_CalendarEntry	Note;
	int			start,stop;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		Note.Location=i;
		error = Phone->GetCalendar(&s, &Note);
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
		error = Phone->DeleteCalendar(&s, &Note);
		Print_Error(error);
	}

	GSM_Terminate();
}

static void GetAllCalendar(int argc, char *argv[])
{
	GSM_CalendarEntry	Note;
	bool			refresh	= true;

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));

	GSM_Init(true);

	while (!gshutdown) {
		error=Phone->GetNextCalendar(&s,&Note,refresh);
		if (error == ERR_EMPTY) break;
		Print_Error(error);
		printf(LISTFORMAT "%d\n", _("Location"), Note.Location);
		PrintCalendar(&Note);
		refresh=false;
	}

	GSM_Terminate();
}

static void GetCalendarSettings(int argc, char *argv[])
{
	GSM_CalendarSettings settings;

	GSM_Init(true);

	error=Phone->GetCalendarSettings(&s,&settings);
	Print_Error(error);

	if (settings.AutoDelete == 0) {
		printf(_("Auto deleting disabled"));
	} else {
		printf(_("Auto deleting notes after %i day(s)"),settings.AutoDelete);
	}
	printf(_("\nWeek start on "));
	switch(settings.StartDay) {
		case 1: printf(_("Monday")); 	break;
		case 6: printf(_("Saturday")); 	break;
		case 7: printf(_("Sunday")); 	break;
	}
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
		error=Phone->GetWAPBookmark(&s,&bookmark);
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
		error=Phone->DeleteWAPBookmark(&s, &bookmark);
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
		error=Phone->GetGPRSAccessPoint(&s,&point);
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

	error=Phone->GetBitmap(&s,&MultiBitmap.Bitmap[0]);
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
		} else if (IsPhoneFeatureAvailable(s.Phone.Data.ModelInfo, F_6230iCALLER)) {
			printf(LISTFORMAT "%i\n", _("Ringtone"),MultiBitmap.Bitmap[0].RingtoneID);
		} else if (MultiBitmap.Bitmap[0].FileSystemRingtone) {
			sprintf(buffer,"%i",MultiBitmap.Bitmap[0].RingtoneID);
			EncodeUnicode(File.ID_FullName,buffer,strlen(buffer));

			File.Buffer 	= NULL;
			File.Used 	= 0;

			error = ERR_NONE;
//			while (error == ERR_NONE) {
				error = Phone->GetFilePart(&s,&File,&Handle,&Size);
//			}
		    	if (error != ERR_EMPTY && error != ERR_WRONGCRC) Print_Error(error);
			error = ERR_NONE;

			printf(LISTFORMAT "\"%s\" ", _("Ringtone"), DecodeUnicodeString(File.Name));
			printf(_("(file with ID %i)\n"), MultiBitmap.Bitmap[0].RingtoneID);
		} else {
			error = Phone->GetRingtonesInfo(&s,&Info);
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
			error=Phone->GetBitmap(&s,&NewBitmap);
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
				error=Phone->GetNetworkInfo(&s,&NetInfo);
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
				error=Phone->GetNetworkInfo(&s,&NetInfo);
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

	error=Phone->SetBitmap(&s,&Bitmap);
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
	error=Phone->SetRingtone(&s, &ringtone, &i);
	Print_Error(error);
#ifdef GSM_ENABLE_BEEP
	GSM_PhoneBeep();
#endif
	GSM_Terminate();
}

static void DisplaySMSFrame(GSM_SMSMessage *SMS)
{
	GSM_Error 		error;
	int			i, length, current = 0;
	unsigned char		req[1000], buffer[1000], hexreq[1000];
#ifdef OSCAR
        unsigned char           hexmsg[1000], hexudh[1000];
#endif
	error=PHONE_EncodeSMSFrame(&s,SMS,buffer,PHONE_SMSSubmit,&length,true);
	if (error != ERR_NONE) {
		printf("%s\n", _("Error"));
		exit(-1);
	}
        length = length - PHONE_SMSSubmit.Text;
#ifdef OSCAR
        for(i=SMS->UDH.Length;i<length;i++) {
		req[i-SMS->UDH.Length]=buffer[PHONE_SMSSubmit.Text+i];
	}
        EncodeHexBin(hexmsg, req, length-SMS->UDH.Length);

        for(i=0;i<SMS->UDH.Length;i++) {
		req[i]=buffer[PHONE_SMSSubmit.Text+i];
	}
        EncodeHexBin(hexudh, req, SMS->UDH.Length);

        printf(_("msg:%s nb:%i udh:%s\n"),
                hexmsg,
                (buffer[PHONE_SMSSubmit.TPUDL]-SMS->UDH.Length)*8,
                hexudh);
#else
	for (i=0;i<buffer[PHONE_SMSSubmit.SMSCNumber]+1;i++) {
		req[current++]=buffer[PHONE_SMSSubmit.SMSCNumber+i];
	}
	req[current++]=buffer[PHONE_SMSSubmit.firstbyte];
	req[current++]=buffer[PHONE_SMSSubmit.TPMR];
	for (i=0;i<((buffer[PHONE_SMSSubmit.Number]+1)/2+1)+1;i++) {
		req[current++]=buffer[PHONE_SMSSubmit.Number+i];
	}
	req[current++]=buffer[PHONE_SMSSubmit.TPPID];
	req[current++]=buffer[PHONE_SMSSubmit.TPDCS];
	req[current++]=buffer[PHONE_SMSSubmit.TPVP];
	req[current++]=buffer[PHONE_SMSSubmit.TPUDL];
	for(i=0;i<length;i++) req[current++]=buffer[PHONE_SMSSubmit.Text+i];
	EncodeHexBin(hexreq, req, current);
	printf("%s\n\n",hexreq);
#endif
}

#define SEND_SAVE_SMS_BUFFER_SIZE 10000

static GSM_Error SMSStatus;

static void SendSMSStatus (GSM_StateMachine *s, int status, int MessageReference)
{
	dbgprintf("Sent SMS on device: \"%s\"\n",s->CurrentConfig->Device);
	if (status==0) {
		printf(_("..OK"));
		SMSStatus = ERR_NONE;
	} else {
		printf(_("..error %i"),status);
		SMSStatus = ERR_UNKNOWN;
	}
	printf(_(", message reference=%d\n"),MessageReference);
}

static void SendSaveDisplaySMS(int argc, char *argv[])
{
#ifdef GSM_ENABLE_BACKUP
	GSM_Backup			Backup;
#endif
	int				i,j,z,FramesNum = 0;
	int				Protected = 0;
	GSM_SMSFolders			folders;
	GSM_MultiSMSMessage		sms;
	GSM_Ringtone			ringtone[MAX_MULTI_SMS];
	GSM_MultiBitmap			bitmap[MAX_MULTI_SMS],bitmap2;
	GSM_MultiPartSMSInfo		SMSInfo;
	GSM_NetworkInfo			NetInfo;
	GSM_MMSIndicator		MMSInfo;
	FILE 				*ReplaceFile,*f;
	char				ReplaceBuffer2	[200],ReplaceBuffer[200];
	char				InputBuffer	[SEND_SAVE_SMS_BUFFER_SIZE/2+1];
	char				Buffer		[MAX_MULTI_SMS][SEND_SAVE_SMS_BUFFER_SIZE];
	char				Sender		[(GSM_MAX_NUMBER_LENGTH+1)*2];
	char				Name		[(GSM_MAX_SMS_NAME_LENGTH+1)*2];
	char				SMSC		[(GSM_MAX_NUMBER_LENGTH+1)*2];
	int				startarg		= 0;
	int				chars_read		= 0;
	int 				nextlong		= 0;
	bool				ReplyViaSameSMSC 	= false;
	int				SMSCSet			= 1;
	int				MaxSMS			= -1;
	bool				EMS16Bit		= false;
	bool				SendSaved		= false;

	/* Parameters required only during saving */
	int				Folder			= 1; /*Inbox by default */
	GSM_SMS_State			State			= SMS_Sent;

	/* Required only during sending */
	GSM_SMSValidity			Validity;
	GSM_SMSC		    	PhoneSMSC;
	bool				DeliveryReport		= false;

	ReplaceBuffer[0] = 0;
	ReplaceBuffer[1] = 0;
	GSM_ClearMultiPartSMSInfo(&SMSInfo);
	SMSInfo.ReplaceMessage		= 0;
	SMSInfo.EntriesNum		= 1;

	if (strcasecmp(argv[1],"--savesms") == 0) {
		EncodeUnicode(Sender,"Gammu",5);
		Name[0]  	= 0;
		Name[1]  	= 0;
		startarg 	= 0;
	} else {
		EncodeUnicode(Sender,argv[3],strlen(argv[3]));
		startarg 	= 1;
		Validity.Format = 0;
	}
	if (strcasecmp(argv[1],"--sendsmsdsms") == 0) {
		startarg=startarg+2;
		EncodeUnicode(SMSC,"1234",4);
		SMSCSet	= 0;
	}

	if (strcasecmp(argv[2],"TEXT") == 0) {
		if (isatty(fileno(stdin))) {
			printf("%s\n", _("Enter message text and press ^D:"));
		}
		chars_read = fread(InputBuffer, 1, SEND_SAVE_SMS_BUFFER_SIZE/2, stdin);
		if (chars_read == 0) printf_warn("%s\n", _("0 chars read !"));
		InputBuffer[chars_read] 		= 0x00;
		InputBuffer[chars_read+1] 		= 0x00;
		EncodeUnicode(Buffer[0],InputBuffer,strlen(InputBuffer));
		SMSInfo.Entries[0].Buffer  		= Buffer[0];
		SMSInfo.Entries[0].ID			= SMS_Text;
		SMSInfo.UnicodeCoding   		= false;
		startarg += 3;
	} else if (strcasecmp(argv[2],"SMSTEMPLATE") == 0) {
		SMSInfo.UnicodeCoding   		= false;
		SMSInfo.EntriesNum 			= 1;
		Buffer[0][0]				= 0x00;
		Buffer[0][1]				= 0x00;
		SMSInfo.Entries[0].Buffer  		= Buffer[0];
		SMSInfo.Entries[0].ID			= SMS_AlcatelSMSTemplateName;
		startarg += 3;
	} else if (strcasecmp(argv[2],"EMS") == 0) {
		SMSInfo.UnicodeCoding   		= false;
		SMSInfo.EntriesNum 			= 0;
		startarg += 3;
	} else if (strcasecmp(argv[2],"MMSINDICATOR") == 0) {
		if (argc<6+startarg) {
			printf("%s\n", _("Where are parameters?"));
			exit(-1);
		}
		SMSInfo.Entries[0].ID 	 	= SMS_MMSIndicatorLong;
		SMSInfo.Entries[0].MMSIndicator = &MMSInfo;
		if (strcasecmp(argv[1],"--savesms") == 0) {
			EncodeUnicode(Sender,"MMS Info",8);
		}
		strcpy(MMSInfo.Address,	argv[3+startarg]);
		strcpy(MMSInfo.Title,	argv[4+startarg]);
		strcpy(MMSInfo.Sender,	argv[5+startarg]);
		startarg += 6;
	} else if (strcasecmp(argv[2],"WAPINDICATOR") == 0) {
		if (argc<5+startarg) {
			printf("%s\n", _("Where are parameters?"));
			exit(-1);
		}
		SMSInfo.Entries[0].ID 	 	= SMS_WAPIndicatorLong;
		SMSInfo.Entries[0].MMSIndicator = &MMSInfo;
		if (strcasecmp(argv[1],"--savesms") == 0) {
			EncodeUnicode(Sender,"WAP Info",8);
		}
		strcpy(MMSInfo.Address,	argv[3+startarg]);
		strcpy(MMSInfo.Title,	argv[4+startarg]);
		startarg += 5;
	} else if (strcasecmp(argv[2],"RINGTONE") == 0) {
		if (argc<4+startarg) {
			printf("%s\n", _("Where is ringtone filename?"));
			exit(-1);
		}
		ringtone[0].Format=RING_NOTETONE;
		error=GSM_ReadRingtoneFile(argv[3+startarg],&ringtone[0]);
		Print_Error(error);
		SMSInfo.Entries[0].ID 	 = SMS_NokiaRingtone;
		SMSInfo.Entries[0].Ringtone = &ringtone[0];
		if (strcasecmp(argv[1],"--savesms") == 0) {
			CopyUnicodeString(Sender, ringtone[0].Name);
			EncodeUnicode(Name,"Ringtone ",9);
			CopyUnicodeString(Name+9*2, ringtone[0].Name);
		}
		startarg += 4;
	} else if (strcasecmp(argv[2],"OPERATOR") == 0) {
		if (argc<4+startarg) {
			printf("%s\n", _("Where is logo filename?"));
			exit(-1);
		}
		bitmap[0].Bitmap[0].Type=GSM_OperatorLogo;
		error=GSM_ReadBitmapFile(argv[3+startarg],&bitmap[0]);
		Print_Error(error);
		strcpy(bitmap[0].Bitmap[0].NetworkCode,"000 00");
		SMSInfo.Entries[0].ID 	 = SMS_NokiaOperatorLogo;
		SMSInfo.Entries[0].Bitmap   = &bitmap[0];
		if (strcasecmp(argv[1],"--savesms") == 0) {
			EncodeUnicode(Sender, "OpLogo",6);
			EncodeUnicode(Name,"OpLogo ",7);
		}
		startarg += 4;
	} else if (strcasecmp(argv[2],"CALLER") == 0) {
		if (argc<4+startarg) {
			printf("%s\n", _("Where is logo filename?"));
			exit(-1);
		}
		bitmap[0].Bitmap[0].Type=GSM_CallerGroupLogo;
		error=GSM_ReadBitmapFile(argv[3+startarg],&bitmap[0]);
		Print_Error(error);
		SMSInfo.Entries[0].ID 	    = SMS_NokiaCallerLogo;
		SMSInfo.Entries[0].Bitmap   = &bitmap[0];
		if (strcasecmp(argv[1],"--savesms") == 0) {
			EncodeUnicode(Sender, "Caller",6);
		}
		startarg += 4;
	} else if (strcasecmp(argv[2],"ANIMATION") == 0) {
		SMSInfo.UnicodeCoding   		= false;
		SMSInfo.EntriesNum 			= 1;
		if (argc<4+startarg) {
			printf("%s\n", _("Where is number of frames?"));
			exit(-1);
		}
		bitmap[0].Number 		= 0;
		i				= 1;
		while (1) {
			bitmap2.Bitmap[0].Type=GSM_StartupLogo;
			error=GSM_ReadBitmapFile(argv[3+startarg+i],&bitmap2);
			Print_Error(error);
			for (j=0;j<bitmap2.Number;j++) {
				if (bitmap[0].Number == atoi(argv[3+startarg])) break;
				memcpy(&bitmap[0].Bitmap[bitmap[0].Number],&bitmap2.Bitmap[j],sizeof(GSM_Bitmap));
				bitmap[0].Number++;
			}
			if (bitmap[0].Number == atoi(argv[3+startarg])) break;
			i++;
		}
		SMSInfo.Entries[0].ID  		= SMS_AlcatelMonoAnimationLong;
		SMSInfo.Entries[0].Bitmap   	= &bitmap[0];
		bitmap[0].Bitmap[0].Text[0]	= 0;
		bitmap[0].Bitmap[0].Text[1]	= 0;
		startarg += 4 + atoi(argv[3+startarg]);
	} else if (strcasecmp(argv[2],"PICTURE") == 0) {
		if (argc<4+startarg) {
			printf("%s\n", _("Where is logo filename?"));
			exit(-1);
		}
		bitmap[0].Bitmap[0].Type=GSM_PictureImage;
		error=GSM_ReadBitmapFile(argv[3+startarg],&bitmap[0]);
		printf(_("File \"%s\"\n"),argv[3+startarg]);
		Print_Error(error);
		SMSInfo.Entries[0].ID 	 	= SMS_NokiaPictureImageLong;
		SMSInfo.Entries[0].Bitmap   	= &bitmap[0];
		SMSInfo.UnicodeCoding 		= false;
		bitmap[0].Bitmap[0].Text[0]	= 0;
		bitmap[0].Bitmap[0].Text[1]	= 0;
		if (strcasecmp(argv[1],"--savesms") == 0) {
			EncodeUnicode(Sender, "Picture",7);
			EncodeUnicode(Name,"Picture Image",13);
		}
		startarg += 4;
#ifdef GSM_ENABLE_BACKUP
	} else if (strcasecmp(argv[2],"BOOKMARK") == 0) {
		if (argc<5+startarg) {
			printf("%s\n", _("Where is backup filename and location?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[3+startarg],&Backup);
		if (error!=ERR_NOTIMPLEMENTED) Print_Error(error);
		i = 0;
		while (Backup.WAPBookmark[i]!=NULL) {
			if (i == atoi(argv[4+startarg])-1) break;
			i++;
		}
		if (i != atoi(argv[4+startarg])-1) {
			printf("%s\n", _("Bookmark not found in file"));
			exit(-1);
		}
		SMSInfo.Entries[0].ID 	    = SMS_NokiaWAPBookmarkLong;
		SMSInfo.Entries[0].Bookmark = Backup.WAPBookmark[i];
		if (strcasecmp(argv[1],"--savesms") == 0) {
			EncodeUnicode(Sender, "Bookmark",8);
			EncodeUnicode(Name,"WAP Bookmark",12);
		}
		startarg += 5;
	} else if (strcasecmp(argv[2],"WAPSETTINGS") == 0) {
		if (argc<6+startarg) {
			printf("%s\n", _("Where is backup filename and location?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[3+startarg],&Backup);
		if (error!=ERR_NOTIMPLEMENTED) Print_Error(error);
		i = 0;
		while (Backup.WAPSettings[i]!=NULL) {
			if (i == atoi(argv[4+startarg])-1) break;
			i++;
		}
		if (i != atoi(argv[4+startarg])-1) {
			printf("%s\n", _("WAP settings not found in file"));
			exit(-1);
		}
		SMSInfo.Entries[0].Settings = NULL;
		for (j=0;j<Backup.WAPSettings[i]->Number;j++) {
			switch (Backup.WAPSettings[i]->Settings[j].Bearer) {
			case WAPSETTINGS_BEARER_GPRS:
				if (strcasecmp(argv[5+startarg],"GPRS") == 0) {
					SMSInfo.Entries[0].Settings = &Backup.WAPSettings[i]->Settings[j];
					break;
				}
			case WAPSETTINGS_BEARER_DATA:
				if (strcasecmp(argv[5+startarg],"DATA") == 0) {
					SMSInfo.Entries[0].Settings = &Backup.WAPSettings[i]->Settings[j];
					break;
				}
			default:
				break;
			}
		}
		if (SMSInfo.Entries[0].Settings == NULL) {
			printf("%s\n", _("Sorry. For now there is only support for GPRS or DATA bearers end"));
			exit(-1);
		}
		SMSInfo.Entries[0].ID = SMS_NokiaWAPSettingsLong;
		if (strcasecmp(argv[1],"--savesms") == 0) {
			EncodeUnicode(Sender, "Settings",8);
			EncodeUnicode(Name,"WAP Settings",12);
		}
		startarg += 6;
	} else if (strcasecmp(argv[2],"MMSSETTINGS") == 0) {
		if (argc<5+startarg) {
			printf("%s\n", _("Where is backup filename and location?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[3+startarg],&Backup);
		if (error!=ERR_NOTIMPLEMENTED) Print_Error(error);
		i = 0;
		while (Backup.MMSSettings[i]!=NULL) {
			if (i == atoi(argv[4+startarg])-1) break;
			i++;
		}
		if (i != atoi(argv[4+startarg])-1) {
			printf("%s\n", _("MMS settings not found in file"));
			exit(-1);
		}
		SMSInfo.Entries[0].Settings = NULL;
		for (j=0;j<Backup.MMSSettings[i]->Number;j++) {
			switch (Backup.MMSSettings[i]->Settings[j].Bearer) {
			case WAPSETTINGS_BEARER_GPRS:
				SMSInfo.Entries[0].Settings = &Backup.MMSSettings[i]->Settings[j];
				break;
			default:
				break;
			}
		}
		if (SMSInfo.Entries[0].Settings == NULL) {
			printf("%s\n", _("Sorry. No GPRS bearer found in MMS settings"));
			exit(-1);
		}
		SMSInfo.Entries[0].ID = SMS_NokiaMMSSettingsLong;
		if (strcasecmp(argv[1],"--savesms") == 0) {
			EncodeUnicode(Sender, "Settings",8);
			EncodeUnicode(Name,"MMS Settings",12);
		}
		startarg += 5;
	} else if (strcasecmp(argv[2],"CALENDAR") == 0) {
		if (argc<5+startarg) {
			printf("%s\n", _("Where is backup filename and location?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[3+startarg],&Backup);
		if (error!=ERR_NOTIMPLEMENTED) Print_Error(error);
		i = 0;
		while (Backup.Calendar[i]!=NULL) {
			if (i == atoi(argv[4+startarg])-1) break;
			i++;
		}
		if (i != atoi(argv[4+startarg])-1) {
			printf("%s\n", _("Calendar note not found in file"));
			exit(-1);
		}
		SMSInfo.Entries[0].ID 	    = SMS_NokiaVCALENDAR10Long;
		SMSInfo.Entries[0].Calendar = Backup.Calendar[i];
		if (strcasecmp(argv[1],"--savesms") == 0) {
			EncodeUnicode(Sender, "Calendar",8);
		}
		startarg += 5;
	} else if (strcasecmp(argv[2],"TODO") == 0) {
		if (argc<5+startarg) {
			printf("%s\n", _("Where is backup filename and location?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[3+startarg],&Backup);
		if (error!=ERR_NOTIMPLEMENTED) Print_Error(error);
		i = 0;
		while (Backup.ToDo[i]!=NULL) {
			if (i == atoi(argv[4+startarg])-1) break;
			i++;
		}
		if (i != atoi(argv[4+startarg])-1) {
			printf("%s\n", _("ToDo note not found in file"));
			exit(-1);
		}
		SMSInfo.Entries[0].ID 	 	= SMS_NokiaVTODOLong;
		SMSInfo.Entries[0].ToDo 	= Backup.ToDo[i];
		if (strcasecmp(argv[1],"--savesms") == 0) {
			EncodeUnicode(Sender, "ToDo",8);
		}
		startarg += 5;
	} else if (strcasecmp(argv[2],"VCARD10") == 0 || strcasecmp(argv[2],"VCARD21") == 0) {
		if (argc<6+startarg) {
			printf("%s\n", _("Where is backup filename and location and memory type?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[3+startarg],&Backup);
		if (error!=ERR_NOTIMPLEMENTED) Print_Error(error);
		i = 0;
		if (strcasecmp(argv[4+startarg],"SM") == 0) {
			while (Backup.SIMPhonebook[i]!=NULL) {
				if (i == atoi(argv[5+startarg])-1) break;
				i++;
			}
			if (i != atoi(argv[5+startarg])-1) {
				printf("%s\n", _("Phonebook entry not found in file"));
				exit(-1);
			}
			SMSInfo.Entries[0].Phonebook = Backup.SIMPhonebook[i];
		} else if (strcasecmp(argv[4+startarg],"ME") == 0) {
			while (Backup.PhonePhonebook[i]!=NULL) {
				if (i == atoi(argv[5+startarg])-1) break;
				i++;
			}
			if (i != atoi(argv[5+startarg])-1) {
				printf("%s\n", _("Phonebook entry not found in file"));
				exit(-1);
			}
			SMSInfo.Entries[0].Phonebook = Backup.PhonePhonebook[i];
		} else {
			printf(_("Unknown memory type: \"%s\"\n"),argv[4+startarg]);
			exit(-1);
		}
		if (strcasecmp(argv[2],"VCARD10") == 0) {
			SMSInfo.Entries[0].ID = SMS_VCARD10Long;
		} else {
			SMSInfo.Entries[0].ID = SMS_VCARD21Long;
		}
		if (strcasecmp(argv[1],"--savesms") == 0) {
			EncodeUnicode(Sender, "VCARD",5);
			EncodeUnicode(Name, "Phonebook entry",15);
		}
		startarg += 6;
#endif
	} else if (strcasecmp(argv[2],"PROFILE") == 0) {
		SMSInfo.Entries[0].ID = SMS_NokiaProfileLong;
		if (strcasecmp(argv[1],"--savesms") == 0) {
			EncodeUnicode(Sender, "Profile",7);
		}
		startarg += 3;
	} else {
		printf(_("What format of sms (\"%s\") ?\n"),argv[2]);
		exit(-1);
	}

	for (i=startarg;i<argc;i++) {
		switch (nextlong) {
		case 0:
			if (strcasecmp(argv[1],"--savesms") == 0 || SendSaved) {
				if (strcasecmp(argv[i],"-folder") == 0) {
					nextlong=1;
					continue;
				}
			}
			if (strcasecmp(argv[1],"--savesms") == 0) {
				if (strcasecmp(argv[i],"-unread") == 0) {
					State = SMS_UnRead;
					continue;
				}
				if (strcasecmp(argv[i],"-read") == 0) {
					State = SMS_Read;
					continue;
				}
				if (strcasecmp(argv[i],"-unsent") == 0) {
					State = SMS_UnSent;
					continue;
				}
				if (strcasecmp(argv[i],"-sent") == 0) {
					State = SMS_Sent;
					continue;
				}
				if (strcasecmp(argv[i],"-sender") == 0) {
					nextlong=2;
					continue;
				}
				if (strcasecmp(argv[i],"-smsname") == 0) {
					nextlong=25;
					continue;
				}
			} else {
				if (strcasecmp(argv[i],"-save") == 0) {
					SendSaved=true;
					continue;
				}
				if (strcasecmp(argv[i],"-report") == 0) {
					DeliveryReport=true;
					continue;
				}
				if (strcasecmp(argv[i],"-validity") == 0) {
					nextlong=10;
					continue;
				}
			}
			if (strcasecmp(argv[i],"-smscset") == 0) {
				nextlong=3;
				continue;
			}
			if (strcasecmp(argv[i],"-smscnumber") == 0) {
				nextlong=4;
				continue;
			}
			if (strcasecmp(argv[i],"-protected") == 0) {
				nextlong=19;
				continue;
			}
			if (strcasecmp(argv[i],"-reply") == 0) {
				ReplyViaSameSMSC=true;
				continue;
			}
			if (strcasecmp(argv[i],"-maxsms") == 0) {
				nextlong=21;
				continue;
			}
			if (strcasecmp(argv[2],"RINGTONE") == 0) {
				if (strcasecmp(argv[i],"-long") == 0) {
					SMSInfo.Entries[0].ID = SMS_NokiaRingtoneLong;
					break;
				}
				if (strcasecmp(argv[i],"-scale") == 0) {
					ringtone[0].NoteTone.AllNotesScale=true;
					break;
				}
			}
			if (strcasecmp(argv[2],"TEXT") == 0) {
				if (strcasecmp(argv[i],"-inputunicode") == 0) {
					ReadUnicodeFile(Buffer[0],InputBuffer);
					break;
				}
				if (strcasecmp(argv[i],"-16bit") == 0) {
					if (SMSInfo.Entries[0].ID == SMS_ConcatenatedTextLong) SMSInfo.Entries[0].ID = SMS_ConcatenatedTextLong16bit;
					if (SMSInfo.Entries[0].ID == SMS_ConcatenatedAutoTextLong) SMSInfo.Entries[0].ID = SMS_ConcatenatedAutoTextLong16bit;
					break;
				}
				if (strcasecmp(argv[i],"-flash") == 0) {
					SMSInfo.Class = 0;
					break;
				}
				if (strcasecmp(argv[i],"-len") == 0) {
					nextlong = 5;
					break;
				}
				if (strcasecmp(argv[i],"-autolen") == 0) {
					nextlong = 5;
					break;
				}
				if (strcasecmp(argv[i],"-unicode") == 0) {
					SMSInfo.UnicodeCoding = true;
					break;
				}
				if (strcasecmp(argv[i],"-enablevoice") == 0) {
					SMSInfo.Entries[0].ID = SMS_EnableVoice;
					break;
				}
				if (strcasecmp(argv[i],"-disablevoice") == 0) {
					SMSInfo.Entries[0].ID = SMS_DisableVoice;
					break;
				}
				if (strcasecmp(argv[i],"-enablefax") == 0) {
					SMSInfo.Entries[0].ID = SMS_EnableFax;
					break;
				}
				if (strcasecmp(argv[i],"-disablefax") == 0) {
					SMSInfo.Entries[0].ID = SMS_DisableFax;
					break;
				}
				if (strcasecmp(argv[i],"-enableemail") == 0) {
					SMSInfo.Entries[0].ID = SMS_EnableEmail;
					break;
				}
				if (strcasecmp(argv[i],"-disableemail") == 0) {
					SMSInfo.Entries[0].ID = SMS_DisableEmail;
					break;
				}
				if (strcasecmp(argv[i],"-voidsms") == 0) {
					SMSInfo.Entries[0].ID = SMS_VoidSMS;
					break;
				}
				if (strcasecmp(argv[i],"-replacemessages") == 0 &&
				    SMSInfo.Entries[0].ID != SMS_ConcatenatedTextLong) {
					nextlong = 8;
					break;
				}
				if (strcasecmp(argv[i],"-replacefile") == 0) {
					nextlong = 9;
					continue;
				}
			}
			if (strcasecmp(argv[2],"PICTURE") == 0) {
				if (strcasecmp(argv[i],"-text") == 0) {
					nextlong = 6;
					break;
				}
				if (strcasecmp(argv[i],"-unicode") == 0) {
					SMSInfo.UnicodeCoding = true;
					break;
				}
				if (strcasecmp(argv[i],"-alcatelbmmi") == 0) {
					bitmap[0].Bitmap[0].Type=GSM_StartupLogo;
					error=GSM_ReadBitmapFile(argv[startarg-1],&bitmap[0]);
					Print_Error(error);
					SMSInfo.UnicodeCoding = true;
					SMSInfo.Entries[0].ID = SMS_AlcatelMonoBitmapLong;
					break;
				}
				break;
			}
			if (strcasecmp(argv[2],"VCARD10") == 0) {
				if (strcasecmp(argv[i],"-nokia") == 0) {
					SMSInfo.Entries[0].ID = SMS_NokiaVCARD10Long;
					break;
				}
				break;
			}
			if (strcasecmp(argv[2],"VCARD21") == 0) {
				if (strcasecmp(argv[i],"-nokia") == 0) {
					SMSInfo.Entries[0].ID = SMS_NokiaVCARD21Long;
					break;
				}
				break;
			}
			if (strcasecmp(argv[2],"PROFILE") == 0) {
				if (strcasecmp(argv[i],"-name") == 0) {
					nextlong = 22;
					break;
				}
				if (strcasecmp(argv[i],"-ringtone") == 0) {
					nextlong = 23;
					break;
				}
				if (strcasecmp(argv[i],"-bitmap") == 0) {
					nextlong = 24;
					break;
				}
			}
			if (strcasecmp(argv[2],"SMSTEMPLATE") == 0) {
				if (strcasecmp(argv[i],"-unicode") == 0) {
					SMSInfo.UnicodeCoding = true;
					break;
				}
				if (strcasecmp(argv[i],"-text") == 0) {
					nextlong = 11;
					break;
				}
				if (strcasecmp(argv[i],"-unicodefiletext") == 0) {
					nextlong = 18;
					break;
				}
				if (strcasecmp(argv[i],"-defsound") == 0) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSPredefinedSound;
					nextlong = 12;
					break;
				}
				if (strcasecmp(argv[i],"-defanimation") == 0) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSPredefinedAnimation;
					nextlong = 12;
					break;
				}
				if (strcasecmp(argv[i],"-tone10") == 0) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSound10;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (strcasecmp(argv[i],"-tone10long") == 0) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSound10Long;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (strcasecmp(argv[i],"-tone12") == 0) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSound12;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (strcasecmp(argv[i],"-tone12long") == 0) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSound12Long;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (strcasecmp(argv[i],"-toneSE") == 0) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSonyEricssonSound;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (strcasecmp(argv[i],"-toneSElong") == 0) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSonyEricssonSoundLong;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (strcasecmp(argv[i],"-variablebitmap") == 0) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSVariableBitmap;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 15;
					break;
				}
				if (strcasecmp(argv[i],"-variablebitmaplong") == 0) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSVariableBitmapLong;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 15;
					break;
				}
				if (strcasecmp(argv[i],"-animation") == 0) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID  = SMS_EMSAnimation;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					bitmap[SMSInfo.EntriesNum].Number 	= 0;
					nextlong = 16;
					break;
				}
			}
			if (strcasecmp(argv[2],"EMS") == 0) {
				if (strcasecmp(argv[i],"-unicode") == 0) {
					SMSInfo.UnicodeCoding = true;
					break;
				}
				if (strcasecmp(argv[i],"-16bit") == 0) {
					EMS16Bit = true;
					break;
				}
				if (strcasecmp(argv[i],"-format") == 0) {
					nextlong = 20;
					break;
				}
				if (strcasecmp(argv[i],"-text") == 0) {
					nextlong = 11;
					break;
				}
				if (strcasecmp(argv[i],"-unicodefiletext") == 0) {
					nextlong = 18;
					break;
				}
				if (strcasecmp(argv[i],"-defsound") == 0) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSPredefinedSound;
					nextlong = 12;
					break;
				}
				if (strcasecmp(argv[i],"-defanimation") == 0) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSPredefinedAnimation;
					nextlong = 12;
					break;
				}
				if (strcasecmp(argv[i],"-tone10") == 0) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSound10;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (strcasecmp(argv[i],"-tone10long") == 0) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSound10Long;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (strcasecmp(argv[i],"-tone12") == 0) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSound12;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (strcasecmp(argv[i],"-tone12long") == 0) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSound12Long;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (strcasecmp(argv[i],"-toneSE") == 0) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSonyEricssonSound;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (strcasecmp(argv[i],"-toneSElong") == 0) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSonyEricssonSoundLong;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (strcasecmp(argv[i],"-fixedbitmap") == 0) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSFixedBitmap;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 15;
					break;
				}
				if (strcasecmp(argv[i],"-variablebitmap") == 0) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSVariableBitmap;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 15;
					break;
				}
				if (strcasecmp(argv[i],"-variablebitmaplong") == 0) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSVariableBitmapLong;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 15;
					break;
				}
				if (strcasecmp(argv[i],"-animation") == 0) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID  = SMS_EMSAnimation;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					bitmap[SMSInfo.EntriesNum].Number 	= 0;
					nextlong = 16;
					break;
				}
			}
			if (strcasecmp(argv[2],"OPERATOR") == 0) {
				if (strcasecmp(argv[i],"-netcode") == 0) {
					nextlong = 7;
					break;
				}
				if (strcasecmp(argv[i],"-biglogo") == 0) {
					SMSInfo.Entries[0].ID = SMS_NokiaOperatorLogoLong;
					break;
				}
				break;
			}
			printf(_("Unknown parameter (\"%s\")\n"),argv[i]);
			exit(-1);
			break;
		case 1: /* SMS folder - only during saving SMS */
			Folder	 = atoi(argv[i]);
			nextlong = 0;
			break;
		case 2: /* Sender number - only during saving SMS */
			EncodeUnicode(Sender,argv[i],strlen(argv[i]));
			nextlong = 0;
			break;
		case 3: /* SMSC set number */
			SMSCSet	 = atoi(argv[i]);
			nextlong = 0;
			break;
		case 4: /* Number of SMSC */
			EncodeUnicode(SMSC,argv[i],strlen(argv[i]));
			SMSCSet		= 0;
			nextlong	= 0;
			break;
		case 5: /* Length of text SMS */
			if (atoi(argv[i])<chars_read)
			{
				Buffer[0][atoi(argv[i])*2]	= 0x00;
				Buffer[0][atoi(argv[i])*2+1]	= 0x00;
			}
			SMSInfo.Entries[0].ID = SMS_ConcatenatedTextLong;
			if (strcasecmp(argv[i-1],"-autolen") == 0) SMSInfo.Entries[0].ID = SMS_ConcatenatedAutoTextLong;
			nextlong = 0;
			break;
		case 6:	/* Picture Images - text */
			EncodeUnicode(bitmap[0].Bitmap[0].Text,argv[i],strlen(argv[i]));
			nextlong = 0;
			break;
		case 7:	/* Operator Logo - network code */
			strncpy(bitmap[0].Bitmap[0].NetworkCode,argv[i],7);
			if (!strcmp(DecodeUnicodeConsole(GSM_GetNetworkName(bitmap[0].Bitmap[0].NetworkCode)),"unknown")) {
				printf(_("Unknown GSM network code (\"%s\")\n"),argv[i]);
				exit(-1);
			}
			if (strcasecmp(argv[1],"--savesms") == 0) {
				EncodeUnicode(Sender, "OpLogo",6);
				EncodeUnicode(Sender+6*2,bitmap[0].Bitmap[0].NetworkCode,3);
				EncodeUnicode(Sender+6*2+3*2,bitmap[0].Bitmap[0].NetworkCode+4,2);
				if (UnicodeLength(GSM_GetNetworkName(bitmap[0].Bitmap[0].NetworkCode))<GSM_MAX_SMS_NAME_LENGTH-7) {
					EncodeUnicode(Name,"OpLogo ",7);
					CopyUnicodeString(Name+7*2,GSM_GetNetworkName(bitmap[0].Bitmap[0].NetworkCode));
				} else {
					CopyUnicodeString(Name,Sender);
				}
			}
			nextlong = 0;
			break;
		case 8:/* Reject duplicates ID */
			SMSInfo.ReplaceMessage = atoi(argv[i]);
			if (SMSInfo.ReplaceMessage < 1 || SMSInfo.ReplaceMessage > 7) {
				printf(_("You have to give number between 1 and 7 (\"%s\")\n"),argv[i]);
				exit(-1);
			}
			nextlong = 0;
			break;
		case 9:/* Replace file for text SMS */
			ReplaceFile = fopen(argv[i], "rb");
			if (ReplaceFile == NULL) Print_Error(ERR_CANTOPENFILE);
			memset(ReplaceBuffer,0,sizeof(ReplaceBuffer));
			fread(ReplaceBuffer,1,sizeof(ReplaceBuffer),ReplaceFile);
			fclose(ReplaceFile);
			ReadUnicodeFile(ReplaceBuffer2,ReplaceBuffer);
			for(j=0;j<(int)(UnicodeLength(Buffer[0]));j++) {
				for (z=0;z<(int)(UnicodeLength(ReplaceBuffer2)/2);z++) {
					if (ReplaceBuffer2[z*4]   == Buffer[0][j] &&
					    ReplaceBuffer2[z*4+1] == Buffer[0][j+1]) {
						Buffer[0][j]   = ReplaceBuffer2[z*4+2];
						Buffer[0][j+1] = ReplaceBuffer2[z*4+3];
						break;
					}
				}
			}
			nextlong = 0;
			break;
		case 10:
			Validity.Format = SMS_Validity_RelativeFormat;
			if (strcasecmp(argv[i],"HOUR") == 0) 		Validity.Relative = SMS_VALID_1_Hour;
			else if (strcasecmp(argv[i],"6HOURS") == 0)	Validity.Relative = SMS_VALID_6_Hours;
			else if (strcasecmp(argv[i],"DAY") == 0) 	Validity.Relative = SMS_VALID_1_Day;
			else if (strcasecmp(argv[i],"3DAYS") == 0) 	Validity.Relative = SMS_VALID_3_Days;
			else if (strcasecmp(argv[i],"WEEK") == 0) 	Validity.Relative = SMS_VALID_1_Week;
			else if (strcasecmp(argv[i],"MAX") == 0) 	Validity.Relative = SMS_VALID_Max_Time;
			else {
				printf(_("Unknown validity string (\"%s\")\n"),argv[i]);
				exit(-1);
			}
			nextlong = 0;
			break;
		case 11:/* EMS text from parameter */
			EncodeUnicode(Buffer[SMSInfo.EntriesNum],argv[i],strlen(argv[i]));
			dbgprintf("buffer is \"%s\"\n",DecodeUnicodeConsole(Buffer[SMSInfo.EntriesNum]));
			SMSInfo.Entries[SMSInfo.EntriesNum].ID 		= SMS_ConcatenatedTextLong;
			SMSInfo.Entries[SMSInfo.EntriesNum].Buffer 	= Buffer[SMSInfo.EntriesNum];
			SMSInfo.EntriesNum++;
			nextlong = 0;
			break;
		case 12:/* EMS predefined sound/animation number */
			SMSInfo.Entries[SMSInfo.EntriesNum].Number = atoi(argv[i]);
			SMSInfo.EntriesNum++;
			nextlong = 0;
			break;
		case 14: /* EMS ringtone - IMelody */
			ringtone[SMSInfo.EntriesNum].Format=RING_NOTETONE;
			error=GSM_ReadRingtoneFile(argv[i],&ringtone[SMSInfo.EntriesNum]);
			Print_Error(error);
			SMSInfo.Entries[SMSInfo.EntriesNum].Ringtone = &ringtone[SMSInfo.EntriesNum];
			SMSInfo.EntriesNum++;
			nextlong = 0;
			break;
		case 15:/* EMS bitmap file */
			bitmap[SMSInfo.EntriesNum].Bitmap[0].Type=GSM_StartupLogo;
			error=GSM_ReadBitmapFile(argv[i],&bitmap[SMSInfo.EntriesNum]);
			Print_Error(error);
			SMSInfo.Entries[SMSInfo.EntriesNum].Bitmap = &bitmap[SMSInfo.EntriesNum];
			SMSInfo.EntriesNum++;
			nextlong = 0;
			break;
		case 16:/* Number of frames for EMS animation */
			FramesNum = atoi(argv[i]);
			if (FramesNum < 1 || FramesNum > 4) {
				printf(_("You have to give number of EMS frames between 1 and 4 (\"%s\")\n"),argv[i]);
				exit(-1);
			}
			bitmap[SMSInfo.EntriesNum].Number = 0;
			nextlong = 17;
			break;
		case 17:/*File for EMS animation */
			bitmap2.Bitmap[0].Type=GSM_StartupLogo;
			error=GSM_ReadBitmapFile(argv[i],&bitmap2);
			for (j=0;j<bitmap2.Number;j++) {
				if (bitmap[SMSInfo.EntriesNum].Number == FramesNum) break;
				memcpy(&bitmap[SMSInfo.EntriesNum].Bitmap[bitmap[SMSInfo.EntriesNum].Number],&bitmap2.Bitmap[j],sizeof(GSM_Bitmap));
				bitmap[SMSInfo.EntriesNum].Number++;
			}
			if (bitmap[SMSInfo.EntriesNum].Number == FramesNum) {
				SMSInfo.Entries[SMSInfo.EntriesNum].Bitmap = &bitmap[SMSInfo.EntriesNum];
				SMSInfo.EntriesNum++;
				nextlong = 0;
			}
			break;
		case 18:/* EMS text from Unicode file */
			f = fopen(argv[i],"rb");
			if (f == NULL) {
				printf(_("Can't open file \"%s\"\n"),argv[i]);
				exit(-1);
			}
			z=fread(InputBuffer,1,2000,f);
			InputBuffer[z]   = 0;
			InputBuffer[z+1] = 0;
			fclose(f);
			ReadUnicodeFile(Buffer[SMSInfo.EntriesNum],InputBuffer);
			dbgprintf("buffer is \"%s\"\n",DecodeUnicodeConsole(Buffer[SMSInfo.EntriesNum]));
			SMSInfo.Entries[SMSInfo.EntriesNum].ID 		= SMS_ConcatenatedTextLong;
			SMSInfo.Entries[SMSInfo.EntriesNum].Buffer 	= Buffer[SMSInfo.EntriesNum];
			SMSInfo.EntriesNum++;
			nextlong = 0;
			break;
		case 19:/* Number of protected items */
			Protected 	= atoi(argv[i]);
			nextlong 	= 0;
			break;
		case 20:/* Formatting text for EMS */
			if (SMSInfo.Entries[SMSInfo.EntriesNum-1].ID == SMS_ConcatenatedTextLong) {
				for(j=0;j<(int)strlen(argv[i]);j++) {
				switch(argv[i][j]) {
				case 'l': case 'L':
					SMSInfo.Entries[SMSInfo.EntriesNum-1].Left = true;
					break;
				case 'c': case 'C':
					SMSInfo.Entries[SMSInfo.EntriesNum-1].Center = true;
					break;
				case 'r': case 'R':
					SMSInfo.Entries[SMSInfo.EntriesNum-1].Right = true;
					break;
				case 'a': case 'A':
					SMSInfo.Entries[SMSInfo.EntriesNum-1].Large = true;
					break;
				case 's': case 'S':
					SMSInfo.Entries[SMSInfo.EntriesNum-1].Small = true;
					break;
				case 'b': case 'B':
					SMSInfo.Entries[SMSInfo.EntriesNum-1].Bold = true;
					break;
				case 'i': case 'I':
					SMSInfo.Entries[SMSInfo.EntriesNum-1].Italic = true;
					break;
				case 'u': case 'U':
					SMSInfo.Entries[SMSInfo.EntriesNum-1].Underlined = true;
					break;
				case 't': case 'T':
					SMSInfo.Entries[SMSInfo.EntriesNum-1].Strikethrough = true;
					break;
				default:
					printf(_("Unknown parameter (\"%c\")\n"),argv[i][j]);
					exit(-1);
				}
				}
			} else {
				printf("%s\n", _("Last parameter wasn't text"));
				exit(-1);
			}
			nextlong = 0;
			break;
		case 21:/*MaxSMS*/
			MaxSMS   = atoi(argv[i]);
			nextlong = 0;
			break;
		case 22:/* profile name */
			EncodeUnicode(Buffer[0],argv[i],strlen(argv[i]));
			SMSInfo.Entries[0].Buffer   = Buffer[0];
			nextlong 		    = 0;
			break;
		case 23:/* profile ringtone */
			ringtone[0].Format	    = RING_NOTETONE;
			error=GSM_ReadRingtoneFile(argv[i],&ringtone[0]);
			Print_Error(error);
			SMSInfo.Entries[0].Ringtone = &ringtone[0];
			nextlong 		    = 0;
			break;
		case 24:/* profile bitmap */
			bitmap[0].Bitmap[0].Type    = GSM_PictureImage;
			error=GSM_ReadBitmapFile(argv[i],&bitmap[0]);
			Print_Error(error);
			bitmap[0].Bitmap[0].Text[0] = 0;
			bitmap[0].Bitmap[0].Text[1] = 0;
			SMSInfo.Entries[0].Bitmap   = &bitmap[0];
			nextlong 		    = 0;
			break;
		case 25:/* sms name */
			if (strlen(argv[i])>GSM_MAX_SMS_NAME_LENGTH) {
				printf(_("Too long SMS name (\"%s\"), ignored\n"),argv[i]);
			} else {
				EncodeUnicode(Name, argv[i],strlen(argv[i]));
			}
			nextlong = 0;
			break;
		}
	}
	if (nextlong!=0) {
		printf_err("%s\n", _("Parameter missing!"));
		exit(-1);
	}

	if (strcasecmp(argv[2],"EMS") == 0 && EMS16Bit) {
		for (i=0;i<SMSInfo.EntriesNum;i++) {
			switch (SMSInfo.Entries[i].ID) {
			case SMS_ConcatenatedTextLong:
				SMSInfo.Entries[i].ID = SMS_ConcatenatedTextLong16bit;
			default:
				break;

			}
		}

	}
	if (strcasecmp(argv[2],"TEXT") == 0) {
		chars_read = UnicodeLength(Buffer[0]);
		if (chars_read != 0) {
			/* Trim \n at the end of string */
			if (Buffer[0][chars_read*2-1] == '\n' && Buffer[0][chars_read*2-2] == 0)
			{
				Buffer[0][chars_read*2-1] = 0;
			}
		}
	}

	if (strcasecmp(argv[1],"--displaysms") == 0 || strcasecmp(argv[1],"--sendsmsdsms") == 0) {
		if (strcasecmp(argv[2],"OPERATOR") == 0) {
			if (bitmap[0].Bitmap[0].Type==GSM_OperatorLogo && strcmp(bitmap[0].Bitmap[0].NetworkCode,"000 00")==0) {
				printf("%s\n", _("No network code"));
				exit(-1);
			}
		}
	} else {
		GSM_Init(true);

		if (strcasecmp(argv[2],"OPERATOR") == 0) {
			if (bitmap[0].Bitmap[0].Type==GSM_OperatorLogo && strcmp(bitmap[0].Bitmap[0].NetworkCode,"000 00")==0) {
				error=Phone->GetNetworkInfo(&s,&NetInfo);
				Print_Error(error);
				strcpy(bitmap[0].Bitmap[0].NetworkCode,NetInfo.NetworkCode);
				if (strcasecmp(argv[1],"--savesms") == 0) {
					EncodeUnicode(Sender, "OpLogo",6);
					EncodeUnicode(Sender+6*2,bitmap[0].Bitmap[0].NetworkCode,3);
					EncodeUnicode(Sender+6*2+3*2,bitmap[0].Bitmap[0].NetworkCode+4,2);
					if (UnicodeLength(GSM_GetNetworkName(bitmap[0].Bitmap[0].NetworkCode))<GSM_MAX_SMS_NAME_LENGTH-7) {
						EncodeUnicode(Name,"OpLogo ",7);
						CopyUnicodeString(Name+7*2,GSM_GetNetworkName(bitmap[0].Bitmap[0].NetworkCode));
					} else {
						CopyUnicodeString(Name,Sender);
					}
				}
			}
		}
	}

	error=GSM_EncodeMultiPartSMS(&SMSInfo,&sms);
	Print_Error(error);

	for (i=0;i<SMSInfo.EntriesNum;i++) {
		switch (SMSInfo.Entries[i].ID) {
			case SMS_NokiaRingtone:
			case SMS_NokiaRingtoneLong:
			case SMS_NokiaProfileLong:
			case SMS_EMSSound10:
			case SMS_EMSSound12:
			case SMS_EMSSonyEricssonSound:
			case SMS_EMSSound10Long:
			case SMS_EMSSound12Long:
			case SMS_EMSSonyEricssonSoundLong:
				if (SMSInfo.Entries[i].RingtoneNotes!=SMSInfo.Entries[i].Ringtone->NoteTone.NrCommands) {
					printf_warn(_("Ringtone too long. %i percent part cut\n"),
						(SMSInfo.Entries[i].Ringtone->NoteTone.NrCommands-SMSInfo.Entries[i].RingtoneNotes)*100/SMSInfo.Entries[i].Ringtone->NoteTone.NrCommands);
				}
			default:
				break;

		}
	}
	if (MaxSMS != -1 && sms.Number > MaxSMS) {
		printf(_("There is %i SMS packed and %i limit. Exiting\n"),sms.Number,MaxSMS);
		if (!strcasecmp(argv[1],"--displaysms") == 0 && !strcasecmp(argv[1],"--sendsmsdsms") == 0) GSM_Terminate();
		exit(-1);
	}

	if (strcasecmp(argv[1],"--displaysms") == 0) {
		if (SMSCSet != 0) {
			printf("%s\n", _("Use -smscnumber option to give SMSC number"));
			exit(-1);
		}

		for (i=0;i<sms.Number;i++) {
			sms.SMS[i].Location			= 0;
			sms.SMS[i].ReplyViaSameSMSC		= ReplyViaSameSMSC;
			sms.SMS[i].SMSC.Location		= 0;
			sms.SMS[i].PDU				= SMS_Submit;
			if (DeliveryReport) sms.SMS[i].PDU	= SMS_Status_Report;
			CopyUnicodeString(sms.SMS[i].Number, Sender);
			CopyUnicodeString(sms.SMS[i].SMSC.Number, SMSC);
			if (Validity.Format != 0) memcpy(&sms.SMS[i].SMSC.Validity,&Validity,sizeof(GSM_SMSValidity));
			DisplaySMSFrame(&sms.SMS[i]);
		}

		printf("\n");
		printf(_("Number of messages: %i"), sms.Number);
		printf("\n");
		exit(sms.Number);
	}
	if (strcasecmp(argv[1],"--sendsmsdsms") == 0) {
		if (SMSCSet != 0) {
			printf("%s\n", _("Use -smscnumber option to give SMSC number"));
			exit(-1);
		}

		for (i=0;i<sms.Number;i++) {
			sms.SMS[i].Location			= 0;
			sms.SMS[i].ReplyViaSameSMSC		= ReplyViaSameSMSC;
			sms.SMS[i].SMSC.Location		= 0;
			sms.SMS[i].PDU				= SMS_Submit;
			if (DeliveryReport) sms.SMS[i].PDU	= SMS_Status_Report;
			CopyUnicodeString(sms.SMS[i].Number, Sender);
			CopyUnicodeString(sms.SMS[i].SMSC.Number, SMSC);
			if (Validity.Format != 0) memcpy(&sms.SMS[i].SMSC.Validity,&Validity,sizeof(GSM_SMSValidity));
		}
		SMSDaemonSendSMS(argv[4],argv[5],&sms);
		exit(0);
	}
	if (strcasecmp(argv[1],"--savesms") == 0 || SendSaved) {
		error=Phone->GetSMSFolders(&s, &folders);
		Print_Error(error);

		if (SendSaved)	{
			if (Validity.Format != 0 && SMSCSet != 0) {
				PhoneSMSC.Location = SMSCSet;
				error=Phone->GetSMSC(&s,&PhoneSMSC);
				Print_Error(error);
				CopyUnicodeString(SMSC,PhoneSMSC.Number);
				SMSCSet = 0;
			}

			s.User.SendSMSStatus = SendSMSStatus;

			signal(SIGINT, interrupt);
			fprintf(stderr, "%s\n", _("If you want break, press Ctrl+C..."));
		}

		for (i=0;i<sms.Number;i++) {
			printf(_("Saving SMS %i/%i\n"),i+1,sms.Number);
//			sms.SMS[i].Location		= 0;
			sms.SMS[i].Folder		= Folder;
			sms.SMS[i].State		= State;
			sms.SMS[i].ReplyViaSameSMSC	= ReplyViaSameSMSC;
			sms.SMS[i].SMSC.Location	= SMSCSet;

			if (SendSaved)	{
				sms.SMS[i].PDU	= SMS_Submit;
				if (DeliveryReport) sms.SMS[i].PDU = SMS_Status_Report;
				if (Validity.Format != 0) sms.SMS[i].SMSC.Validity = Validity;
			} else {
				sms.SMS[i].PDU	= SMS_Deliver;
			}

			CopyUnicodeString(sms.SMS[i].Number, Sender);
			CopyUnicodeString(sms.SMS[i].Name, Name);
			if (SMSCSet==0) CopyUnicodeString(sms.SMS[i].SMSC.Number, SMSC);
			error=Phone->AddSMS(&s, &sms.SMS[i]);
			Print_Error(error);
			printf(_("Saved in folder \"%s\", location %i"),
				DecodeUnicodeConsole(folders.Folder[sms.SMS[i].Folder-1].Name),sms.SMS[i].Location);
			if (sms.SMS[i].Memory == MEM_SM) {
				printf(", %s\n", _("SIM"));
				if (UnicodeLength(Name) != 0) {
					printf("%s\n", _("SMS name ignored"));
				}
			} else {
				printf(", %s\n", _("phone"));
			}

			if (SendSaved) {
				printf(_("Sending sms from folder \"%s\", location %i\n"),
					DecodeUnicodeString(folders.Folder[sms.SMS[i].Folder-1].Name),sms.SMS[i].Location);
				SMSStatus = ERR_TIMEOUT;
				error=Phone->SendSavedSMS(&s, 0, sms.SMS[i].Location);
				Print_Error(error);
				printf(_("....waiting for network answer"));
				while (!gshutdown) {
					GSM_ReadDevice(&s,true);
					if (SMSStatus == ERR_UNKNOWN) {
						GSM_Terminate();
						exit(-1);
					}
					if (SMSStatus == ERR_NONE) break;
				}
			}
		}
	} else {
		if (Validity.Format != 0 && SMSCSet != 0) {
			PhoneSMSC.Location = SMSCSet;
			error=Phone->GetSMSC(&s,&PhoneSMSC);
			Print_Error(error);
			CopyUnicodeString(SMSC,PhoneSMSC.Number);
			SMSCSet = 0;
		}

		signal(SIGINT, interrupt);
		fprintf(stderr, "%s\n", _("If you want break, press Ctrl+C..."));

		s.User.SendSMSStatus = SendSMSStatus;

		for (i=0;i<sms.Number;i++) {
			printf(_("Sending SMS %i/%i"),i+1,sms.Number);
			fflush(stdout);
			sms.SMS[i].Location			= 0;
			sms.SMS[i].ReplyViaSameSMSC		= ReplyViaSameSMSC;
			sms.SMS[i].SMSC.Location		= SMSCSet;
			sms.SMS[i].PDU				= SMS_Submit;
			if (DeliveryReport) sms.SMS[i].PDU	= SMS_Status_Report;
			CopyUnicodeString(sms.SMS[i].Number, Sender);
			if (SMSCSet==0) CopyUnicodeString(sms.SMS[i].SMSC.Number, SMSC);
			if (Validity.Format != 0) memcpy(&sms.SMS[i].SMSC.Validity,&Validity,sizeof(GSM_SMSValidity));
			SMSStatus = ERR_TIMEOUT;
			error=Phone->SendSMS(&s, &sms.SMS[i]);
			Print_Error(error);
			printf(_("....waiting for network answer"));
			fflush(stdout);
			while (!gshutdown) {
				GSM_ReadDevice(&s,true);
				if (SMSStatus == ERR_UNKNOWN) {
					GSM_Terminate();
					exit(-1);
				}
				if (SMSStatus == ERR_NONE) break;
			}
		}
	}

	GSM_Terminate();
}

#ifdef GSM_ENABLE_BACKUP
static void SaveFile(int argc, char *argv[])
{
	GSM_Backup		Backup;
	int			i,j;
	FILE			*file;
	unsigned char		Buffer[10000];
	GSM_MemoryEntry		*pbk;

	if (strcasecmp(argv[2],"CALENDAR") == 0) {
		if (argc<5) {
			printf("%s\n", _("Where is backup filename and location?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[4],&Backup);
		if (error!=ERR_NOTIMPLEMENTED) Print_Error(error);
		i = 0;
		while (Backup.Calendar[i]!=NULL) {
			if (i == atoi(argv[5])-1) break;
			i++;
		}
		if (i != atoi(argv[5])-1) {
			printf("%s\n", _("Calendar note not found in file"));
			exit(-1);
		}
		j = 0;
		GSM_EncodeVCALENDAR(Buffer, &j, Backup.Calendar[i],true,Nokia_VCalendar);
	} else if (strcasecmp(argv[2],"BOOKMARK") == 0) {
		if (argc<5) {
			printf("%s\n", _("Where is backup filename and location?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[4],&Backup);
		if (error!=ERR_NOTIMPLEMENTED) Print_Error(error);
		i = 0;
		while (Backup.WAPBookmark[i]!=NULL) {
			if (i == atoi(argv[5])-1) break;
			i++;
		}
		if (i != atoi(argv[5])-1) {
			printf("%s\n", _("WAP bookmark not found in file"));
			exit(-1);
		}
		j = 0;
		GSM_EncodeURLFile(Buffer, &j, Backup.WAPBookmark[i]);
	} else if (strcasecmp(argv[2],"NOTE") == 0) {
		if (argc<5) {
			printf("%s\n", _("Where is backup filename and location?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[4],&Backup);
		if (error!=ERR_NOTIMPLEMENTED) Print_Error(error);
		i = 0;
		while (Backup.Note[i]!=NULL) {
			if (i == atoi(argv[5])-1) break;
			i++;
		}
		if (i != atoi(argv[5])-1) {
			printf("%s\n", _("Note not found in file"));
			exit(-1);
		}
		j = 0;
		GSM_EncodeVNTFile(Buffer, &j, Backup.Note[i]);
	} else if (strcasecmp(argv[2],"TODO") == 0) {
		if (argc<5) {
			printf("%s\n", _("Where is backup filename and location?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[4],&Backup);
		if (error!=ERR_NOTIMPLEMENTED) Print_Error(error);
		i = 0;
		while (Backup.ToDo[i]!=NULL) {
			if (i == atoi(argv[5])-1) break;
			i++;
		}
		if (i != atoi(argv[5])-1) {
			printf("%s\n", _("ToDo note not found in file"));
			exit(-1);
		}
		j = 0;
		GSM_EncodeVTODO(Buffer, &j, Backup.ToDo[i], true, Nokia_VToDo);
	} else if (strcasecmp(argv[2],"VCARD10") == 0 || strcasecmp(argv[2],"VCARD21") == 0) {
		if (argc<6) {
			printf("%s\n", _("Where is backup filename and location and memory type?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[4],&Backup);
		if (error!=ERR_NOTIMPLEMENTED) Print_Error(error);
		i = 0;
		if (strcasecmp(argv[5],"SM") == 0) {
			while (Backup.SIMPhonebook[i]!=NULL) {
				if (i == atoi(argv[6])-1) break;
				i++;
			}
			if (i != atoi(argv[6])-1) {
				printf("%s\n", _("Phonebook entry not found in file"));
				exit(-1);
			}
			pbk = Backup.SIMPhonebook[i];
		} else if (strcasecmp(argv[5],"ME") == 0) {
			while (Backup.PhonePhonebook[i]!=NULL) {
				if (i == atoi(argv[6])-1) break;
				i++;
			}
			if (i != atoi(argv[6])-1) {
				printf("%s\n", _("Phonebook entry not found in file"));
				exit(-1);
			}
			pbk = Backup.PhonePhonebook[i];
		} else {
			printf(_("Unknown memory type: \"%s\"\n"),argv[5]);
			exit(-1);
		}
		j = 0;
		if (strcasecmp(argv[2],"VCARD10") == 0) {
			GSM_EncodeVCARD(Buffer,&j,pbk,true,Nokia_VCard10);
		} else {
			GSM_EncodeVCARD(Buffer,&j,pbk,true,Nokia_VCard21);
		}
	} else {
		printf(_("What format of file (\"%s\") ?\n"),argv[2]);
		exit(-1);
	}

	file = fopen(argv[3],"wb");
	fwrite(Buffer,1,j,file);
	fclose(file);
}

static void Backup(int argc, char *argv[])
{
	int			i, used;
	GSM_MemoryStatus	MemStatus;
	GSM_ToDoEntry		ToDo;
	GSM_ToDoStatus		ToDoStatus;
	GSM_MemoryEntry		Pbk;
	GSM_CalendarEntry	Calendar;
	GSM_Bitmap		Bitmap;
	GSM_WAPBookmark		Bookmark;
	GSM_Profile		Profile;
	GSM_MultiWAPSettings	Settings;
	GSM_SyncMLSettings	SyncML;
	GSM_ChatSettings	Chat;
	GSM_Ringtone		Ringtone;
	GSM_SMSC		SMSC;
	GSM_Backup		Backup;
	GSM_NoteEntry		Note;
	GSM_Backup_Info		Info;
 	GSM_FMStation		FMStation;
 	GSM_GPRSAccessPoint	GPRSPoint;
	bool			DoBackup;

	if (argc == 4 && strcasecmp(argv[3],"-yes") == 0) always_answer_yes = true;

	GSM_ClearBackup(&Backup);
	GSM_GetBackupFormatFeatures(argv[2],&Info);

	sprintf(Backup.Creator,"Gammu %s",VERSION);
	if (strlen(GetOS()) != 0) {
		strcat(Backup.Creator+strlen(Backup.Creator),", ");
		strcat(Backup.Creator+strlen(Backup.Creator),GetOS());
	}
	if (strlen(GetCompiler()) != 0) {
		strcat(Backup.Creator+strlen(Backup.Creator),", ");
		strcat(Backup.Creator+strlen(Backup.Creator),GetCompiler());
	}

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));

	GSM_Init(true);

	if (Info.UseUnicode) {
		Info.UseUnicode=answer_yes(_("Use Unicode subformat of backup file"));
	}
	if (Info.DateTime) {
		GSM_GetCurrentDateTime (&Backup.DateTime);
		Backup.DateTimeAvailable=true;
	}
	if (Info.Model) {
		error=Phone->GetManufacturer(&s);
		Print_Error(error);
		sprintf(Backup.Model,"%s ",s.Phone.Data.Manufacturer);
		strcat(Backup.Model,s.Phone.Data.Model);
		if (s.Phone.Data.ModelInfo->model[0]!=0) {
			strcat(Backup.Model," (");
			strcat(Backup.Model,s.Phone.Data.ModelInfo->model);
			strcat(Backup.Model,")");
		}
		strcat(Backup.Model," ");
		strcat(Backup.Model,s.Phone.Data.Version);
	}
	if (Info.IMEI) {
		error=Phone->GetIMEI(&s);
		if (error != ERR_NOTSUPPORTED) {
			strcpy(Backup.IMEI, s.Phone.Data.IMEI);
			Print_Error(error);
		} else {
			Backup.IMEI[0] = 0;
		}
	}
	printf("\n");

	DoBackup = false;
	if (Info.PhonePhonebook) {
		printf("%s\n", _("Checking phone phonebook"));
		MemStatus.MemoryType = MEM_ME;
		error=Phone->GetMemoryStatus(&s, &MemStatus);
		if (error==ERR_NONE && MemStatus.MemoryUsed != 0) {
			if (answer_yes(_("   Backup phone phonebook"))) DoBackup = true;
		}
	}
	if (DoBackup) {
		Pbk.MemoryType  = MEM_ME;
		i		= 1;
		used 		= 0;
		while (used != MemStatus.MemoryUsed) {
			Pbk.Location = i;
			error=Phone->GetMemory(&s, &Pbk);
			if (error != ERR_EMPTY) {
				Print_Error(error);
				if (used < GSM_BACKUP_MAX_PHONEPHONEBOOK) {
					Backup.PhonePhonebook[used] = malloc(sizeof(GSM_MemoryEntry));
				        if (Backup.PhonePhonebook[used] == NULL) Print_Error(ERR_MOREMEMORY);
					Backup.PhonePhonebook[used+1] = NULL;
				} else {
					printf(_("\n   Only part of data saved - increase %s") , "GSM_BACKUP_MAX_PHONEPHONEBOOK");
					break;
				}
				*Backup.PhonePhonebook[used]=Pbk;
				used++;
			}
			fprintf(stderr, _("%c   Reading: %i percent"),13,used*100/MemStatus.MemoryUsed);
			i++;
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoBackup = false;
	if (Info.SIMPhonebook) {
		printf("%s\n", _("Checking SIM phonebook"));
		MemStatus.MemoryType = MEM_SM;
		error=Phone->GetMemoryStatus(&s, &MemStatus);
		if (error==ERR_NONE && MemStatus.MemoryUsed != 0) {
			if (answer_yes(_("   Backup SIM phonebook"))) DoBackup=true;
		}
	}
	if (DoBackup) {
		Pbk.MemoryType 	= MEM_SM;
		i		= 1;
		used 		= 0;
		while (used != MemStatus.MemoryUsed) {
			Pbk.Location = i;
			error=Phone->GetMemory(&s, &Pbk);
			if (error != ERR_EMPTY) {
				Print_Error(error);
				if (used < GSM_BACKUP_MAX_SIMPHONEBOOK) {
					Backup.SIMPhonebook[used] = malloc(sizeof(GSM_MemoryEntry));
				        if (Backup.SIMPhonebook[used] == NULL) Print_Error(ERR_MOREMEMORY);
					Backup.SIMPhonebook[used + 1] = NULL;
				} else {
					printf(_("\n   Only part of data saved - increase %s") , "GSM_BACKUP_MAX_SIMPHONEBOOK");
					break;
				}
				*Backup.SIMPhonebook[used]=Pbk;
				used++;
			}
			fprintf(stderr, _("%c   Reading: %i percent"),13,used*100/MemStatus.MemoryUsed);
			i++;
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoBackup = false;
	if (Info.Calendar) {
		printf("%s\n", _("Checking phone calendar"));
		error=Phone->GetNextCalendar(&s,&Calendar,true);
		if (error==ERR_NONE) {
			if (answer_yes(_("   Backup phone calendar notes"))) DoBackup = true;
		}
	}
	if (DoBackup) {
		used 		= 0;
		fprintf(stderr, LISTFORMAT, _("Reading"));
		while (error == ERR_NONE) {
			if (used < GSM_MAXCALENDARTODONOTES) {
				Backup.Calendar[used] = malloc(sizeof(GSM_CalendarEntry));
			        if (Backup.Calendar[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.Calendar[used+1] = NULL;
			} else {
				printf(_("\n   Only part of data saved - increase %s") , "GSM_MAXCALENDARTODONOTES");
				break;
			}
			*Backup.Calendar[used]=Calendar;
			used ++;
			error=Phone->GetNextCalendar(&s,&Calendar,false);
			fprintf(stderr, "*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoBackup = false;
	if (Info.ToDo) {
		printf("%s\n", _("Checking phone ToDo"));
		error=Phone->GetToDoStatus(&s,&ToDoStatus);
		if (error == ERR_NONE && ToDoStatus.Used != 0) {
			if (answer_yes(_("   Backup phone ToDo"))) DoBackup = true;
		}
	}
	if (DoBackup) {
		used = 0;
		error=Phone->GetNextToDo(&s,&ToDo,true);
		while (error == ERR_NONE) {
			if (used < GSM_MAXCALENDARTODONOTES) {
				Backup.ToDo[used] = malloc(sizeof(GSM_ToDoEntry));
				if (Backup.ToDo[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.ToDo[used+1] = NULL;
			} else {
				printf(_("\n   Only part of data saved - increase %s") , "GSM_MAXCALENDARTODONOTES");
				break;
			}
			*Backup.ToDo[used]=ToDo;
			used ++;
			error=Phone->GetNextToDo(&s,&ToDo,false);
			fprintf(stderr, _("%c   Reading: %i percent"),13,used*100/ToDoStatus.Used);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoBackup = false;
	if (Info.Note) {
		printf("%s\n", _("Checking phone notes"));
		error=Phone->GetNextNote(&s,&Note,true);
		if (error==ERR_NONE) {
			if (answer_yes(_("   Backup phone notes"))) DoBackup = true;
		}
	}
	if (DoBackup) {
		used 		= 0;
		fprintf(stderr, LISTFORMAT, _("Reading"));
		while (error == ERR_NONE) {
			if (used < GSM_BACKUP_MAX_NOTE) {
				Backup.Note[used] = malloc(sizeof(GSM_NoteEntry));
			        if (Backup.Note[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.Note[used+1] = NULL;
			} else {
				printf(_("\n   Only part of data saved - increase %s") , "GSM_BACKUP_MAX_NOTE");
				break;
			}
			*Backup.Note[used]=Note;
			used ++;
			error=Phone->GetNextNote(&s,&Note,false);
			fprintf(stderr, "*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoBackup = false;
	if (Info.CallerLogos) {
		printf("%s\n", _("Checking phone caller logos"));
		Bitmap.Type 	= GSM_CallerGroupLogo;
		Bitmap.Location = 1;
		error=Phone->GetBitmap(&s,&Bitmap);
		if (error == ERR_NONE) {
			if (answer_yes(_("   Backup phone caller groups and logos"))) DoBackup = true;
		}
	}
	if (DoBackup) {
		fprintf(stderr, LISTFORMAT, _("Reading"));
		error = ERR_NONE;
		used  = 0;
		while (error == ERR_NONE) {
			if (used < GSM_BACKUP_MAX_CALLER) {
				Backup.CallerLogos[used] = malloc(sizeof(GSM_Bitmap));
			        if (Backup.CallerLogos[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.CallerLogos[used+1] = NULL;
			} else {
				printf(_("\n   Only part of data saved - increase %s") , "GSM_BACKUP_MAX_CALLER");
				break;
			}
			*Backup.CallerLogos[used] = Bitmap;
			used ++;
			Bitmap.Location = used + 1;
			error=Phone->GetBitmap(&s,&Bitmap);
			fprintf(stderr, "*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoBackup = false;
	if (Info.SMSC) {
		printf("%s\n", _("Checking SIM SMS profiles"));
		if (answer_yes(_("   Backup SIM SMS profiles"))) DoBackup = true;
	}
	if (DoBackup) {
		used = 0;
		fprintf(stderr, LISTFORMAT, _("Reading"));
		while (true) {
			SMSC.Location = used + 1;
			error = Phone->GetSMSC(&s,&SMSC);
			if (error != ERR_NONE) break;
			if (used < GSM_BACKUP_MAX_SMSC) {
				Backup.SMSC[used] = malloc(sizeof(GSM_SMSC));
			        if (Backup.SMSC[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.SMSC[used + 1] = NULL;
			} else {
				printf(_("\n   Only part of data saved - increase %s") , "GSM_BACKUP_MAX_SMSC");
				break;
			}
			*Backup.SMSC[used]=SMSC;
			used++;
			fprintf(stderr, "*");
		}
		fprintf(stderr, "\n");
	}
	DoBackup = false;
	if (Info.StartupLogo) {
		printf("%s\n", _("Checking phone startup text"));
		Bitmap.Type = GSM_WelcomeNote_Text;
		error = Phone->GetBitmap(&s,&Bitmap);
		if (error == ERR_NONE) {
			if (answer_yes(_("   Backup phone startup logo/text"))) DoBackup = true;
		}
	}
	if (DoBackup) {
		Backup.StartupLogo = malloc(sizeof(GSM_Bitmap));
	        if (Backup.StartupLogo == NULL) Print_Error(ERR_MOREMEMORY);
		*Backup.StartupLogo = Bitmap;
		if (Bitmap.Text[0]==0 && Bitmap.Text[1]==0) {
			Bitmap.Type = GSM_StartupLogo;
			error = Phone->GetBitmap(&s,&Bitmap);
			if (error == ERR_NONE) *Backup.StartupLogo = Bitmap;
		}
	}
	DoBackup = false;
	if (Info.OperatorLogo) {
		printf("%s\n", _("Checking phone operator logo"));
		Bitmap.Type = GSM_OperatorLogo;
		error=Phone->GetBitmap(&s,&Bitmap);
		if (error == ERR_NONE) {
			if (strcmp(Bitmap.NetworkCode,"000 00")!=0) {
				if (answer_yes(_("   Backup phone operator logo"))) DoBackup = true;
			}
		}
	}
	if (DoBackup) {
		Backup.OperatorLogo = malloc(sizeof(GSM_Bitmap));
	        if (Backup.OperatorLogo == NULL) Print_Error(ERR_MOREMEMORY);
		*Backup.OperatorLogo = Bitmap;
	}
	DoBackup = false;
	if (Info.WAPBookmark) {
		printf("%s\n", _("Checking phone WAP bookmarks"));
		Bookmark.Location = 1;
		error=Phone->GetWAPBookmark(&s,&Bookmark);
		if (error==ERR_NONE) {
			if (answer_yes(_("   Backup phone WAP bookmarks"))) DoBackup = true;
		}
	}
	if (DoBackup) {
		used = 0;
		fprintf(stderr, LISTFORMAT, _("Reading"));
		while (error == ERR_NONE) {
			if (used < GSM_BACKUP_MAX_WAPBOOKMARK) {
				Backup.WAPBookmark[used] = malloc(sizeof(GSM_WAPBookmark));
			        if (Backup.WAPBookmark[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.WAPBookmark[used+1] = NULL;
			} else {
				printf(_("\n   Only part of data saved - increase %s") , "GSM_BACKUP_MAX_WAPBOOKMARK");
				break;
			}
			*Backup.WAPBookmark[used]=Bookmark;
			used ++;
			Bookmark.Location = used+1;
			error=Phone->GetWAPBookmark(&s,&Bookmark);
			fprintf(stderr, "*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoBackup = false;
	if (Info.WAPSettings) {
		printf("%s\n", _("Checking phone WAP settings"));
		Settings.Location = 1;
		error=Phone->GetWAPSettings(&s,&Settings);
		if (error==ERR_NONE) {
			if (answer_yes(_("   Backup phone WAP settings"))) DoBackup = true;
		}
	}
	if (DoBackup) {
		used = 0;
		fprintf(stderr, LISTFORMAT, _("Reading"));
		while (error == ERR_NONE) {
			if (used < GSM_BACKUP_MAX_WAPSETTINGS) {
				Backup.WAPSettings[used] = malloc(sizeof(GSM_MultiWAPSettings));
			        if (Backup.WAPSettings[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.WAPSettings[used+1] = NULL;
			} else {
				printf(_("\n   Only part of data saved - increase %s") , "GSM_BACKUP_MAX_WAPSETTINGS");
				break;
			}
			*Backup.WAPSettings[used]=Settings;
			used ++;
			Settings.Location = used+1;
			error=Phone->GetWAPSettings(&s,&Settings);
			fprintf(stderr, "*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoBackup = false;
	if (Info.MMSSettings) {
		printf("%s\n", _("Checking phone MMS settings"));
		Settings.Location = 1;
		error=Phone->GetMMSSettings(&s,&Settings);
		if (error==ERR_NONE) {
			if (answer_yes(_("   Backup phone MMS settings"))) DoBackup = true;
		}
	}
	if (DoBackup) {
		used = 0;
		fprintf(stderr, LISTFORMAT, _("Reading"));
		while (error == ERR_NONE) {
			if (used < GSM_BACKUP_MAX_MMSSETTINGS) {
				Backup.MMSSettings[used] = malloc(sizeof(GSM_MultiWAPSettings));
			        if (Backup.MMSSettings[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.MMSSettings[used+1] = NULL;
			} else {
				printf(_("\n   Only part of data saved - increase %s") , "GSM_BACKUP_MAX_MMSSETTINGS");
				break;
			}
			*Backup.MMSSettings[used]=Settings;
			used ++;
			Settings.Location = used+1;
			error=Phone->GetMMSSettings(&s,&Settings);
			fprintf(stderr, "*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoBackup = false;
	if (Info.ChatSettings) {
		printf("%s\n", _("Checking phone Chat settings"));
		Chat.Location = 1;
		error=Phone->GetChatSettings(&s,&Chat);
		if (error==ERR_NONE) {
			if (answer_yes(_("   Backup phone Chat settings"))) DoBackup = true;
		}
	}
	if (DoBackup) {
		used = 0;
		fprintf(stderr, LISTFORMAT, _("Reading"));
		while (error == ERR_NONE) {
			if (used < GSM_BACKUP_MAX_CHATSETTINGS) {
				Backup.ChatSettings[used] = malloc(sizeof(GSM_ChatSettings));
			        if (Backup.ChatSettings[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.ChatSettings[used+1] = NULL;
			} else {
				printf(_("\n   Only part of data saved - increase %s") , "GSM_BACKUP_MAX_CHATSETTINGS");
				break;
			}
			*Backup.ChatSettings[used]=Chat;
			used ++;
			Chat.Location = used+1;
			error=Phone->GetChatSettings(&s,&Chat);
			fprintf(stderr, "*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoBackup = false;
	if (Info.SyncMLSettings) {
		printf("%s\n", _("Checking phone SyncML settings"));
		SyncML.Location = 1;
		error=Phone->GetSyncMLSettings(&s,&SyncML);
		if (error==ERR_NONE) {
			if (answer_yes(_("   Backup phone SyncML settings"))) DoBackup = true;
		}
	}
	if (DoBackup) {
		used = 0;
		fprintf(stderr, LISTFORMAT, _("Reading"));
		while (error == ERR_NONE) {
			if (used < GSM_BACKUP_MAX_SYNCMLSETTINGS) {
				Backup.SyncMLSettings[used] = malloc(sizeof(GSM_SyncMLSettings));
			        if (Backup.SyncMLSettings[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.SyncMLSettings[used+1] = NULL;
			} else {
				printf(_("\n   Only part of data saved - increase %s") , "GSM_BACKUP_MAX_SYNCMLSETTINGS");
				break;
			}
			*Backup.SyncMLSettings[used]=SyncML;
			used ++;
			SyncML.Location = used+1;
			error=Phone->GetSyncMLSettings(&s,&SyncML);
			fprintf(stderr, "*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoBackup = false;
	if (Info.Ringtone) {
		printf("%s\n", _("Checking phone user ringtones"));
		Ringtone.Location 	= 1;
		Ringtone.Format		= 0;
		error=Phone->GetRingtone(&s,&Ringtone,false);
		if (error==ERR_EMPTY || error == ERR_NONE) {
			if (answer_yes(_("   Backup phone user ringtones"))) DoBackup = true;
		}
	}
	if (DoBackup) {
		used 	= 0;
		i	= 1;
		fprintf(stderr, LISTFORMAT, _("Reading"));
		while (error == ERR_NONE || error == ERR_EMPTY) {
			if (error == ERR_NONE) {
				if (used < GSM_BACKUP_MAX_RINGTONES) {
					Backup.Ringtone[used] = malloc(sizeof(GSM_Ringtone));
				        if (Backup.Ringtone[used] == NULL) Print_Error(ERR_MOREMEMORY);
					Backup.Ringtone[used+1] = NULL;
				} else {
					printf(_("\n   Only part of data saved - increase %s") , "GSM_BACKUP_MAX_RINGTONES");
					break;
				}
				*Backup.Ringtone[used]=Ringtone;
				used ++;
			}
			i++;
			Ringtone.Location = i;
			Ringtone.Format	  = 0;
			error=Phone->GetRingtone(&s,&Ringtone,false);
			fprintf(stderr, "*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoBackup = false;
	if (Info.Profiles) {
		printf("%s\n", _("Checking phone profiles"));
		Profile.Location = 1;
		error = Phone->GetProfile(&s,&Profile);
	        if (error == ERR_NONE) {
			if (answer_yes(_("   Backup phone profiles"))) DoBackup = true;
		}
	}
	if (DoBackup) {
		used = 0;
		fprintf(stderr, LISTFORMAT, _("Reading"));
		while (true) {
			Profile.Location = used + 1;
			error = Phone->GetProfile(&s,&Profile);
			if (error != ERR_NONE) break;
			if (used < GSM_BACKUP_MAX_PROFILES) {
				Backup.Profiles[used] = malloc(sizeof(GSM_Profile));
				if (Backup.Profiles[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.Profiles[used + 1] = NULL;
			} else {
				printf(_("\n   Only part of data saved - increase %s") , "GSM_BACKUP_MAX_PROFILES");
				break;
			}
			*Backup.Profiles[used]=Profile;
			used++;
			fprintf(stderr, "*");
		}
		fprintf(stderr, "\n");
	}
	DoBackup = false;
 	if (Info.FMStation) {
		printf("%s\n", _("Checking phone FM radio stations"));
 		FMStation.Location = 1;
 		error = Phone->GetFMStation(&s,&FMStation);
 	        if (error == ERR_NONE || error == ERR_EMPTY) {
 			if (answer_yes(_("   Backup phone FM radio stations"))) DoBackup=true;
		}
	}
	if (DoBackup) {
		used	= 0;
		i	= 1;
		fprintf(stderr, LISTFORMAT, _("Reading"));
		while (error == ERR_NONE || error == ERR_EMPTY) {
			error = Phone->GetFMStation(&s,&FMStation);
			if (error == ERR_NONE) {
 				if (used < GSM_BACKUP_MAX_FMSTATIONS) {
 					Backup.FMStation[used] = malloc(sizeof(GSM_FMStation));
					if (Backup.FMStation[used] == NULL) Print_Error(ERR_MOREMEMORY);
 					Backup.FMStation[used + 1] = NULL;
 				} else {
					printf(_("\n   Only part of data saved - increase %s") , "GSM_BACKUP_MAX_FMSTATIONS");
					break;
 				}
 				*Backup.FMStation[used]=FMStation;
 				used++;
 			}
 			i++;
 			FMStation.Location = i;
 			fprintf(stderr, "*");
 		}
 		fprintf(stderr, "\n");
 	}
	DoBackup = false;
 	if (Info.GPRSPoint) {
		printf("%s\n", _("Checking phone GPRS access points"));
 		GPRSPoint.Location = 1;
 		error = Phone->GetGPRSAccessPoint(&s,&GPRSPoint);
 	        if (error == ERR_NONE || error == ERR_EMPTY) {
 			if (answer_yes(_("   Backup phone GPRS access points"))) DoBackup = true;
		}
	}
	if (DoBackup) {
		used	= 0;
		i	= 1;
		fprintf(stderr, LISTFORMAT, _("Reading"));
		while (error == ERR_NONE || error == ERR_EMPTY) {
			error = Phone->GetGPRSAccessPoint(&s,&GPRSPoint);
 			if (error == ERR_NONE) {
 				if (used < GSM_BACKUP_MAX_GPRSPOINT) {
 					Backup.GPRSPoint[used] = malloc(sizeof(GSM_GPRSAccessPoint));
					if (Backup.GPRSPoint[used] == NULL) Print_Error(ERR_MOREMEMORY);
 					Backup.GPRSPoint[used + 1] = NULL;
 				} else {
					printf(_("\n   Only part of data saved - increase %s") , "GSM_BACKUP_MAX_GPRSPOINT");
					break;
 				}
 				*Backup.GPRSPoint[used]=GPRSPoint;
 				used++;
 			}
 			i++;
 			GPRSPoint.Location = i;
 			fprintf(stderr, "*");
 		}
 		fprintf(stderr, "\n");
 	}

	GSM_Terminate();

	GSM_SaveBackupFile(argv[2],&Backup, Info.UseUnicode);
    	GSM_FreeBackup(&Backup);
}

static void Restore(int argc, char *argv[])
{
	GSM_Backup		Backup;
	GSM_FMStation		FMStation;
	GSM_DateTime 		date_time;
	GSM_CalendarEntry	Calendar;
	GSM_Bitmap		Bitmap;
	GSM_Ringtone		Ringtone;
	GSM_MemoryEntry		Pbk;
	GSM_MemoryStatus	MemStatus;
	GSM_ToDoEntry		ToDo;
	GSM_ToDoStatus		ToDoStatus;
	GSM_NoteEntry		Note;
	GSM_Profile		Profile;
	GSM_MultiWAPSettings	Settings;
	GSM_GPRSAccessPoint	GPRSPoint;
	GSM_WAPBookmark		Bookmark;
	int			i, j, used, max = 0;
	bool			Past = true, First;
	bool			Found, DoRestore;

	error=GSM_ReadBackupFile(argv[2],&Backup);
	if (error!=ERR_NOTIMPLEMENTED) {
		Print_Error(error);
	} else {
		printf_warn("%s\n", _("Some data not read from file. It can be damaged or restoring some settings from this file format not implemented (maybe higher Gammu required ?)"));
	}

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));

	if (Backup.DateTimeAvailable) 	fprintf(stderr, LISTFORMAT "%s\n", _("Time of backup"),OSDateTime(Backup.DateTime,false));
	if (Backup.Model[0]!=0) 	fprintf(stderr, LISTFORMAT "%s\n", _("Phone"),Backup.Model);
	if (Backup.IMEI[0]!=0) 		fprintf(stderr, LISTFORMAT "%s\n", _("IMEI"),Backup.IMEI);
	if (Backup.Creator[0]!=0) 	fprintf(stderr, LISTFORMAT "%s\n", _("File created by"),Backup.Creator);

	if (argc == 4 && strcasecmp(argv[3],"-yes") == 0) always_answer_yes = true;

	if (Backup.MD5Calculated[0]!=0) {
		dbgprintf("\"%s\"\n",Backup.MD5Original);
		dbgprintf("\"%s\"\n",Backup.MD5Calculated);
		if (strcmp(Backup.MD5Original,Backup.MD5Calculated)) {
			if (!answer_yes(_("Checksum in backup file do not match. Continue"))) return;
		}
	}

	GSM_Init(true);

	DoRestore = false;
	if (Backup.CallerLogos[0] != NULL) {
		Bitmap.Type 	= GSM_CallerGroupLogo;
		Bitmap.Location = 1;
		error=Phone->GetBitmap(&s,&Bitmap);
		if (error == ERR_NONE) {
			if (answer_yes(_("Restore phone caller groups and logos"))) DoRestore = true;
		}
	}
	if (DoRestore) {
		max = 0;
		while (Backup.CallerLogos[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			error=Phone->SetBitmap(&s,Backup.CallerLogos[i]);
			Print_Error(error);
			fprintf(stderr, _("%cWriting: %i percent"),13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}

	DoRestore = false;
	if (Backup.PhonePhonebook[0] != NULL) {
		MemStatus.MemoryType = MEM_ME;
		error=Phone->GetMemoryStatus(&s, &MemStatus);
		if (error==ERR_NONE) {
			max = 0;
			while (Backup.PhonePhonebook[max]!=NULL) max++;
			fprintf(stderr, _("%i entries in backup file\n"),max);
			if (answer_yes(_("Restore phone phonebook"))) DoRestore = true;
		}
	}
	if (DoRestore) {
		used = 0;
		for (i=0;i<MemStatus.MemoryUsed+MemStatus.MemoryFree;i++) {
			Pbk.MemoryType 	= MEM_ME;
			Pbk.Location	= i + 1;
			Pbk.EntriesNum	= 0;
			if (used<max && Backup.PhonePhonebook[used]->Location == Pbk.Location) {
				Pbk = *Backup.PhonePhonebook[used];
				used++;
				dbgprintf("Location %i\n",Pbk.Location);
				if (Pbk.EntriesNum != 0) error=Phone->SetMemory(&s, &Pbk);
				if (error == ERR_PERMISSION && IsPhoneFeatureAvailable(s.Phone.Data.ModelInfo, F_6230iCALLER)) {
					error=Phone->DeleteMemory(&s, &Pbk);
					Print_Error(error);
					error=Phone->SetMemory(&s, &Pbk);
				}
				if (error == ERR_MEMORY && IsPhoneFeatureAvailable(s.Phone.Data.ModelInfo, F_6230iCALLER)) {
					printf("\n%s\n", _("Error - try to (1) add enough number of/restore caller groups and (2) use --restore again"));
					GSM_TerminateConnection(&s);
					exit (-1);
				}
				if (Pbk.EntriesNum != 0 && error==ERR_NONE) {
					First = true;
					for (j=0;j<Pbk.EntriesNum;j++) {
			 			if (Pbk.Entries[j].AddError == ERR_NONE) continue;
						if (First) printf(_("%cLocation %i                 \n  "),13,Pbk.Location);
						First = false;
						PrintMemorySubEntry(&Pbk.Entries[j]);
						printf("    %s\n",print_error(Pbk.Entries[j].AddError,s.di.df));
					}
				}
			}
			if (Pbk.EntriesNum == 0) error=Phone->DeleteMemory(&s, &Pbk);
			Print_Error(error);
			fprintf(stderr, _("%cWriting: %i percent"),13,(i+1)*100/(MemStatus.MemoryUsed+MemStatus.MemoryFree));
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}

	DoRestore = false;
	if (Backup.SIMPhonebook[0] != NULL) {
		MemStatus.MemoryType = MEM_SM;
		error=Phone->GetMemoryStatus(&s, &MemStatus);
		if (error==ERR_NONE) {
			max = 0;
			while (Backup.SIMPhonebook[max]!=NULL) max++;
			fprintf(stderr, _("%i entries in backup file\n"),max);
			if (answer_yes(_("Restore SIM phonebook"))) DoRestore = true;
		}
	}
	if (DoRestore) {
		used = 0;
		for (i=0;i<MemStatus.MemoryUsed+MemStatus.MemoryFree;i++) {
			Pbk.MemoryType 	= MEM_SM;
			Pbk.Location	= i + 1;
			Pbk.EntriesNum	= 0;
			if (used<max && Backup.SIMPhonebook[used]->Location == Pbk.Location) {
				Pbk = *Backup.SIMPhonebook[used];
				used++;
				dbgprintf("Location %i\n",Pbk.Location);
				if (Pbk.EntriesNum != 0) {
					error=Phone->SetMemory(&s, &Pbk);
					if (error==ERR_NONE) {
						First = true;
						for (j=0;j<Pbk.EntriesNum;j++) {
					 		if (Pbk.Entries[j].AddError == ERR_NONE) continue;
							if (First) printf(_("%cLocation %i                 \n  "),13,Pbk.Location);
							First = false;
							PrintMemorySubEntry(&Pbk.Entries[j]);
							printf("    %s\n",print_error(Pbk.Entries[j].AddError,s.di.df));
						}
					}
				}
			}
			if (Pbk.EntriesNum == 0) error=Phone->DeleteMemory(&s, &Pbk);
			Print_Error(error);
			fprintf(stderr, _("%cWriting: %i percent"),13,(i+1)*100/(MemStatus.MemoryUsed+MemStatus.MemoryFree));
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}

	if (!strcasecmp(s.CurrentConfig->SyncTime,"yes") == 0) {
		if (answer_yes(_("Do you want to set phone date/time (NOTE: in some phones it's required to correctly restore calendar notes and other items)"))) {
			GSM_GetCurrentDateTime(&date_time);

			error=Phone->SetDateTime(&s, &date_time);
			Print_Error(error);
		}
	}
	DoRestore = false;
	if (Backup.Calendar[0] != NULL) {
		/* N6110 doesn't support getting calendar status */
		error = Phone->GetNextCalendar(&s,&Calendar,true);
		if (error == ERR_NONE || error == ERR_INVALIDLOCATION || error == ERR_EMPTY) {
			max = 0;
			while (Backup.Calendar[max] != NULL) max++;
			fprintf(stderr, _("%i entries in backup file\n"),max);
			if (answer_yes(_("Restore phone calendar notes"))) {
				Past    = answer_yes(_("  Restore notes from the past"));
				DoRestore = true;
			}
		}
	}
	if (DoRestore) {
		fprintf(stderr, _("Deleting old notes: "));
		error = Phone->DeleteAllCalendar(&s);
		if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
 			while (1) {
				error = Phone->GetNextCalendar(&s,&Calendar,true);
				if (error != ERR_NONE) break;
				error = Phone->DeleteCalendar(&s,&Calendar);
 				Print_Error(error);
				fprintf(stderr, "*");
			}
			fprintf(stderr, "\n");
		} else {
			fprintf(stderr, "%s\n", _("Done"));
			Print_Error(error);
		}

		for (i=0;i<max;i++) {
			if (!Past && IsCalendarNoteFromThePast(Backup.Calendar[i])) continue;

			Calendar = *Backup.Calendar[i];
			error=Phone->AddCalendar(&s,&Calendar);
			Print_Error(error);
			fprintf(stderr, _("%cWriting: %i percent"),13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}

	DoRestore = false;
	if (Backup.ToDo[0] != NULL) {
		error = Phone->GetToDoStatus(&s,&ToDoStatus);
		if (error == ERR_NONE) {
			max = 0;
			while (Backup.ToDo[max]!=NULL) max++;
			fprintf(stderr, _("%i entries in backup file\n"),max);

			if (answer_yes(_("Restore phone ToDo"))) DoRestore = true;
		}
	}
	if (DoRestore) {
		ToDo  = *Backup.ToDo[0];
		error = Phone->SetToDo(&s,&ToDo);
	}
	if (DoRestore && (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED)) {
		fprintf(stderr, _("Deleting old ToDo: "));
		error=Phone->DeleteAllToDo(&s);
		if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
			while (1) {
				error = Phone->GetNextToDo(&s,&ToDo,true);
				if (error != ERR_NONE) break;
				error = Phone->DeleteToDo(&s,&ToDo);
 				Print_Error(error);
				fprintf(stderr, "*");
			}
			fprintf(stderr, "\n");
		} else {
			fprintf(stderr, "%s\n", _("Done"));
			Print_Error(error);
		}

		for (i=0;i<max;i++) {
			ToDo 		= *Backup.ToDo[i];
			ToDo.Location 	= 0;
			error=Phone->AddToDo(&s,&ToDo);
			Print_Error(error);
			fprintf(stderr, _("%cWriting: %i percent"),13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	} else if (DoRestore) {
		/* At first delete entries, that were deleted */
		used  = 0;
		error = Phone->GetNextToDo(&s,&ToDo,true);
		while (error == ERR_NONE) {
			used++;
			Found = false;
			for (i=0;i<max;i++) {
				if (Backup.ToDo[i]->Location == ToDo.Location) {
					Found = true;
					break;
				}
			}
			if (!Found) {
				error=Phone->DeleteToDo(&s,&ToDo);
				Print_Error(error);
			}
			error = Phone->GetNextToDo(&s,&ToDo,false);
			fprintf(stderr, _("%cCleaning: %i percent"),13,used*100/ToDoStatus.Used);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");

		/* Now write modified/new entries */
		for (i=0;i<max;i++) {
			ToDo  = *Backup.ToDo[i];
			error = Phone->SetToDo(&s,&ToDo);
			Print_Error(error);
			fprintf(stderr, _("%cWriting: %i percent"),13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
 	}

	DoRestore = false;
	if (Backup.Note[0] != NULL) {
		error = Phone->GetNotesStatus(&s,&ToDoStatus);
		if (error == ERR_NONE) {
			max = 0;
			while (Backup.Note[max]!=NULL) max++;
			fprintf(stderr, _("%i entries in backup file\n"),max);

			if (answer_yes(_("Restore phone Notes"))) DoRestore = true;
		}
	}
	if (DoRestore) {
		fprintf(stderr, _("Deleting old Notes: "));
		while (1) {
			error = Phone->GetNextNote(&s,&Note,true);
			if (error != ERR_NONE) break;
			error = Phone->DeleteNote(&s,&Note);
 			Print_Error(error);
			fprintf(stderr, "*");
		}
		fprintf(stderr, "\n");

		for (i=0;i<max;i++) {
			Note 		= *Backup.Note[i];
			Note.Location 	= 0;
			error=Phone->AddNote(&s,&Note);
			Print_Error(error);
			fprintf(stderr, _("%cWriting: %i percent"),13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}

	if (Backup.SMSC[0] != NULL && answer_yes(_("Restore SIM SMSC profiles"))) {
		max = 0;
		while (Backup.SMSC[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			error=Phone->SetSMSC(&s,Backup.SMSC[i]);
			Print_Error(error);
			fprintf(stderr, _("%cWriting: %i percent"),13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	if (Backup.StartupLogo != NULL && answer_yes(_("Restore phone startup logo/text"))) {
		error=Phone->SetBitmap(&s,Backup.StartupLogo);
		Print_Error(error);
	}
	if (Backup.OperatorLogo != NULL && answer_yes(_("Restore phone operator logo"))) {
		error=Phone->SetBitmap(&s,Backup.OperatorLogo);
		Print_Error(error);
	}
	DoRestore = false;
	if (Backup.WAPBookmark[0] != NULL) {
		Bookmark.Location = 1;
		error = Phone->GetWAPBookmark(&s,&Bookmark);
		if (error == ERR_NONE || error == ERR_INVALIDLOCATION) {
			if (answer_yes(_("Restore phone WAP bookmarks"))) DoRestore = true;
		}
	}
	if (DoRestore) {
		fprintf(stderr, _("Deleting old bookmarks: "));
		/* One thing to explain: DCT4 phones seems to have bug here.
		 * When delete for example first bookmark, phone change
		 * numeration for getting frame, not for deleting. So, we try to
		 * get 1'st bookmark. Inside frame is "correct" location. We use
		 * it later
		 */
		while (error==ERR_NONE) {
			error = Phone->DeleteWAPBookmark(&s,&Bookmark);
			Bookmark.Location = 1;
			error = Phone->GetWAPBookmark(&s,&Bookmark);
			fprintf(stderr, "*");
		}
		fprintf(stderr, "\n");
		max = 0;
		while (Backup.WAPBookmark[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			Bookmark 	  = *Backup.WAPBookmark[i];
			Bookmark.Location = 0;
			error=Phone->SetWAPBookmark(&s,&Bookmark);
			Print_Error(error);
			fprintf(stderr, _("%cWriting: %i percent"),13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoRestore = false;
	if (Backup.WAPSettings[0] != NULL) {
		Settings.Location = 1;
		error = Phone->GetWAPSettings(&s,&Settings);
		if (error == ERR_NONE) {
			if (answer_yes(_("Restore phone WAP settings"))) DoRestore = true;
		}
	}
	if (DoRestore) {
		max = 0;
		while (Backup.WAPSettings[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			error=Phone->SetWAPSettings(&s,Backup.WAPSettings[i]);
			Print_Error(error);
			fprintf(stderr, _("%cWriting: %i percent"),13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoRestore = false;
	if (Backup.MMSSettings[0] != NULL) {
		Settings.Location = 1;
		error = Phone->GetMMSSettings(&s,&Settings);
		if (error == ERR_NONE) {
			if (answer_yes(_("Restore phone MMS settings"))) DoRestore = true;
		}
	}
	if (DoRestore) {
		max = 0;
		while (Backup.MMSSettings[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			error=Phone->SetMMSSettings(&s,Backup.MMSSettings[i]);
			Print_Error(error);
			fprintf(stderr, _("%cWriting: %i percent"),13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoRestore = false;
	if (Backup.Ringtone[0] != NULL) {
		Ringtone.Location 	= 1;
		Ringtone.Format		= 0;
		error = Phone->GetRingtone(&s,&Ringtone,false);
		if (error == ERR_NONE || error ==ERR_EMPTY) {
			if (Phone->DeleteUserRingtones != NOTSUPPORTED) {
				if (answer_yes(_("Delete all phone user ringtones"))) DoRestore = true;
			}
		}
	}
	if (DoRestore) {
		fprintf(stderr, LISTFORMAT, _("Deleting"));
		error=Phone->DeleteUserRingtones(&s);
		Print_Error(error);
		fprintf(stderr, "%s\n", _("Done"));
		DoRestore = false;
		if (answer_yes(_("Restore user ringtones"))) DoRestore = true;
	}
	if (DoRestore) {
		max = 0;
		while (Backup.Ringtone[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			error=GSM_RingtoneConvert(&Ringtone, Backup.Ringtone[i], Ringtone.Format);
			Print_Error(error);
			error=Phone->SetRingtone(&s,&Ringtone,&i);
			Print_Error(error);
			fprintf(stderr, _("%cWriting: %i percent"),13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoRestore = false;
	if (Backup.Profiles[0] != NULL) {
		Profile.Location = 1;
		error = Phone->GetProfile(&s,&Profile);
		if (error == ERR_NONE) {
			if (answer_yes(_("Restore phone profiles"))) DoRestore = true;
		}
	}
	if (DoRestore) {
		Profile.Location= 0;
		max = 0;
		while (Backup.Profiles[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			Profile	= *Backup.Profiles[i];
			error=Phone->SetProfile(&s,&Profile);
			Print_Error(error);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoRestore = false;
	if (Backup.FMStation[0] != NULL) {
		FMStation.Location = 1;
		error = Phone->GetFMStation(&s,&FMStation);
		if (error == ERR_NONE || error == ERR_EMPTY) {
			if (answer_yes(_("Restore phone FM radio stations"))) DoRestore = true;
		}
	}
	if (DoRestore) {
		fprintf(stderr, _("Deleting old FM stations: "));
		error=Phone->ClearFMStations(&s);
		Print_Error(error);
		fprintf(stderr, "%s\n", _("Done"));
		max = 0;
		while (Backup.FMStation[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			FMStation = *Backup.FMStation[i];
			error=Phone->SetFMStation(&s,&FMStation);
			Print_Error(error);
			fprintf(stderr, _("%cWriting: %i percent"),13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoRestore = false;
	if (Backup.GPRSPoint[0] != NULL) {
		GPRSPoint.Location = 1;
		error = Phone->GetGPRSAccessPoint(&s,&GPRSPoint);
		if (error == ERR_NONE || error == ERR_EMPTY) {
			if (answer_yes(_("Restore phone GPRS Points"))) DoRestore = true;
		}
	}
	if (DoRestore) {
		max = 0;
		while (Backup.GPRSPoint[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			error=Phone->SetGPRSAccessPoint(&s,Backup.GPRSPoint[i]);
			Print_Error(error);
			fprintf(stderr, _("%cWriting: %i percent"),13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}

	GSM_FreeBackup(&Backup);
	GSM_Terminate();
}

static void AddNew(int argc, char *argv[])
{
	GSM_Backup		Backup;
	GSM_DateTime 		date_time;
	GSM_MemoryEntry		Pbk;
	GSM_MemoryStatus	MemStatus;
	GSM_ToDoEntry		ToDo;
	GSM_ToDoStatus		ToDoStatus;
	GSM_CalendarEntry	Calendar;
	GSM_WAPBookmark		Bookmark;
	int			i, max, j;

	error=GSM_ReadBackupFile(argv[2],&Backup);
	if (error!=ERR_NOTIMPLEMENTED) Print_Error(error);

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));

	if (Backup.DateTimeAvailable) 	fprintf(stderr, LISTFORMAT "%s\n", _("Time of backup"),OSDateTime(Backup.DateTime,false));
	if (Backup.Model[0]!=0) 	fprintf(stderr, LISTFORMAT "%s\n", _("Phone"),Backup.Model);
	if (Backup.IMEI[0]!=0) 		fprintf(stderr, LISTFORMAT "%s\n", _("IMEI"),Backup.IMEI);

	GSM_Init(true);

	if (Backup.PhonePhonebook[0] != NULL) {
		MemStatus.MemoryType = MEM_ME;
		error=Phone->GetMemoryStatus(&s, &MemStatus);
		if (error==ERR_NONE) {
			max = 0;
			while (Backup.PhonePhonebook[max]!=NULL) max++;
			fprintf(stderr, _("%i entries in backup file\n"),max);
			if (MemStatus.MemoryFree < max) {
				fprintf(stderr, _("Memory has only %i free locations.Exiting\n"),MemStatus.MemoryFree);
			} else if (answer_yes(_("Add phone phonebook entries"))) {
				if (Phone->AddMemory == NOTIMPLEMENTED) {
					j = 1;
					for (i=0;i<max;i++) {
						error = ERR_UNKNOWN;
						while (true) {
							Pbk.MemoryType  = MEM_ME;
							Pbk.Location 	= j;
							error=Phone->GetMemory(&s, &Pbk);
							if (error == ERR_EMPTY) break;
							if (error != ERR_NONE) Print_Error(error);
							j++;
							if (gshutdown) {
								GSM_Terminate();
								exit(0);
							}
						}
						Pbk 		= *Backup.PhonePhonebook[i];
						Pbk.MemoryType 	= MEM_ME;
						Pbk.Location 	= j;
						error=Phone->SetMemory(&s, &Pbk);
						if (error == ERR_PERMISSION && IsPhoneFeatureAvailable(s.Phone.Data.ModelInfo, F_6230iCALLER)) {
							error=Phone->DeleteMemory(&s, &Pbk);
							Print_Error(error);
							error=Phone->SetMemory(&s, &Pbk);
						}
						Print_Error(error);
						fprintf(stderr, _("%cWriting: %i percent"),13,(i+1)*100/max);
						if (gshutdown) {
							GSM_Terminate();
							exit(0);
						}
					}
				} else {
					for (i=0;i<max;i++) {
						Pbk 		= *Backup.PhonePhonebook[i];
						Pbk.MemoryType 	= MEM_ME;
						error=Phone->AddMemory(&s, &Pbk);
						Print_Error(error);
						fprintf(stderr, _("%cWriting: %i percent"),13,(i+1)*100/max);
						if (gshutdown) {
							GSM_Terminate();
							exit(0);
						}
					}
				}
				fprintf(stderr, "\n");
			}
		}
	}
	if (Backup.SIMPhonebook[0] != NULL) {
		MemStatus.MemoryType = MEM_SM;
		error=Phone->GetMemoryStatus(&s, &MemStatus);
		if (error==ERR_NONE) {
			max = 0;
			while (Backup.SIMPhonebook[max]!=NULL) max++;
			fprintf(stderr, _("%i entries in backup file\n"),max);
			if (MemStatus.MemoryFree < max) {
				fprintf(stderr, _("Memory has only %i free locations.Exiting\n"),MemStatus.MemoryFree);
			} else if (answer_yes(_("Add SIM phonebook entries"))) {
				if (Phone->AddMemory == NOTIMPLEMENTED) {
					j = 1;
					for (i=0;i<max;i++) {
						error = ERR_UNKNOWN;
						while (true) {
							Pbk.MemoryType  = MEM_SM;
							Pbk.Location 	= j;
							error=Phone->GetMemory(&s, &Pbk);
							if (error == ERR_EMPTY) break;
							if (error != ERR_NONE) Print_Error(error);
							j++;
							if (gshutdown) {
								GSM_Terminate();
								exit(0);
							}
						}
						Pbk 		= *Backup.SIMPhonebook[i];
						Pbk.MemoryType 	= MEM_SM;
						Pbk.Location 	= j;
						error=Phone->SetMemory(&s, &Pbk);
						Print_Error(error);
						fprintf(stderr, _("%cWriting: %i percent"),13,(i+1)*100/max);
						if (gshutdown) {
							GSM_Terminate();
							exit(0);
						}
					}
				} else {
					for (i=0;i<max;i++) {
						Pbk 		= *Backup.SIMPhonebook[i];
						Pbk.MemoryType 	= MEM_SM;
						error=Phone->AddMemory(&s, &Pbk);
						Print_Error(error);
						fprintf(stderr, _("%cWriting: %i percent"),13,(i+1)*100/max);
						if (gshutdown) {
							GSM_Terminate();
							exit(0);
						}
					}
				}
				fprintf(stderr, "\n");
			}
		}
	}

	if (!strcasecmp(s.CurrentConfig->SyncTime,"yes") == 0) {
		if (answer_yes(_("Do you want to set phone date/time (NOTE: in some phones it's required to correctly restore calendar notes and other items)"))) {
			GSM_GetCurrentDateTime(&date_time);

			error=Phone->SetDateTime(&s, &date_time);
			Print_Error(error);
		}
	}
	if (Backup.Calendar[0] != NULL) {
		error = Phone->GetNextCalendar(&s,&Calendar,true);
		if (error == ERR_NONE || error == ERR_INVALIDLOCATION || error == ERR_EMPTY) {
			if (answer_yes(_("Add phone calendar notes"))) {
				max = 0;
				while (Backup.Calendar[max]!=NULL) max++;
				for (i=0;i<max;i++) {
					Calendar = *Backup.Calendar[i];
					error=Phone->AddCalendar(&s,&Calendar);
					Print_Error(error);
					fprintf(stderr, _("%cWriting: %i percent"),13,(i+1)*100/max);
					if (gshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				fprintf(stderr, "\n");
			}
		}
	}
	if (Backup.ToDo[0] != NULL) {
		ToDo.Location = 1;
		error=Phone->GetToDoStatus(&s,&ToDoStatus);
		if (error == ERR_NONE) {
			if (answer_yes(_("Add phone ToDo"))) {
				max = 0;
				while (Backup.ToDo[max]!=NULL) max++;
				for (i=0;i<max;i++) {
					ToDo  = *Backup.ToDo[i];
					error = Phone->AddToDo(&s,&ToDo);
					Print_Error(error);
					fprintf(stderr, _("%cWriting: %i percent"),13,(i+1)*100/max);
					if (gshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				fprintf(stderr, "\n");
			}
		}
	}
	if (Backup.WAPBookmark[0] != NULL) {
		Bookmark.Location = 1;
		error = Phone->GetWAPBookmark(&s,&Bookmark);
		if (error == ERR_NONE || error == ERR_INVALIDLOCATION) {
			if (answer_yes(_("Add phone WAP bookmarks"))) {
				max = 0;
				while (Backup.WAPBookmark[max]!=NULL) max++;
				for (i=0;i<max;i++) {
					Bookmark 	  = *Backup.WAPBookmark[i];
					Bookmark.Location = 0;
					error=Phone->SetWAPBookmark(&s,&Bookmark);
					Print_Error(error);
					fprintf(stderr, _("%cWriting: %i percent"),13,(i+1)*100/max);
					if (gshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				fprintf(stderr, "\n");
			}
		}
	}

	GSM_FreeBackup(&Backup);
	GSM_Terminate();
}
#endif

static void ClearMemory(GSM_MemoryType type, const char *question)
{
	GSM_MemoryStatus	MemStatus;
	GSM_MemoryEntry		Pbk;
	bool			DoClear;

	DoClear = false;
	MemStatus.MemoryType = type;
	error = Phone->GetMemoryStatus(&s, &MemStatus);
	if (error == ERR_NONE && MemStatus.MemoryUsed !=0) {
		if (answer_yes(question)) DoClear = true;
	}
	if (DoClear) {
		error = Phone->DeleteAllMemory(&s, type);
		if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
			for (i = 0; i < MemStatus.MemoryUsed + MemStatus.MemoryFree; i++) {
				Pbk.MemoryType 	= type;
				Pbk.Location	= i + 1;
				Pbk.EntriesNum	= 0;
				error=Phone->DeleteMemory(&s, &Pbk);
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


static void ClearAll(int argc, char *argv[])
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
	error = Phone->GetNextCalendar(&s,&Calendar,true);
	if (error == ERR_NONE) {
 		if (answer_yes(_("Delete phone calendar notes"))) DoClear = true;
	}
	if (DoClear) {
		fprintf(stderr, LISTFORMAT, _("Deleting"));
		error=Phone->DeleteAllCalendar(&s);
		if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
 			while (1) {
				error = Phone->GetNextCalendar(&s,&Calendar,true);
				if (error != ERR_NONE) break;
				error = Phone->DeleteCalendar(&s,&Calendar);
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
	error = Phone->GetToDoStatus(&s,&ToDoStatus);
	if (error == ERR_NONE && ToDoStatus.Used != 0) {
		if (answer_yes(_("Delete phone ToDo"))) DoClear = true;
	}
	if (DoClear) {
		fprintf(stderr, LISTFORMAT, _("Deleting"));
		error=Phone->DeleteAllToDo(&s);
		if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
 			while (1) {
				error = Phone->GetNextToDo(&s,&ToDo,true);
				if (error != ERR_NONE) break;
				error = Phone->DeleteToDo(&s,&ToDo);
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
	error = Phone->GetNotesStatus(&s,&ToDoStatus);
	if (error == ERR_NONE && ToDoStatus.Used != 0) {
		if (answer_yes(_("Delete phone Notes"))) DoClear = true;
	}
	if (DoClear) {
		fprintf(stderr, LISTFORMAT, _("Deleting"));
		while (1) {
			error = Phone->GetNextNote(&s,&Note,true);
			if (error != ERR_NONE) break;
			error = Phone->DeleteNote(&s,&Note);
			Print_Error(error);
			fprintf(stderr, "*");
		}
		fprintf(stderr, "\n");
	}

	Bookmark.Location = 1;
	error = Phone->GetWAPBookmark(&s,&Bookmark);
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
				error = Phone->DeleteWAPBookmark(&s,&Bookmark);
				Bookmark.Location = 1;
				error = Phone->GetWAPBookmark(&s,&Bookmark);
				fprintf(stderr, "*");
			}
			fprintf(stderr, "\n");
		}
	}
	if (Phone->DeleteUserRingtones != NOTSUPPORTED) {
		if (answer_yes(_("Delete all phone user ringtones"))) {
			fprintf(stderr, LISTFORMAT, _("Deleting"));
			error=Phone->DeleteUserRingtones(&s);
			Print_Error(error);
			fprintf(stderr, "%s\n", _("Done"));
		}
	}
	Station.Location=i;
	error=Phone->GetFMStation(&s,&Station);
	if (error == ERR_NONE || error == ERR_EMPTY) {
	 	if (answer_yes(_("Delete all phone FM radio stations"))) {
 			error=Phone->ClearFMStations(&s);
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
		error=Phone->GetSyncMLSettings(&s,&settings);
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
		error=Phone->GetChatSettings(&s,&settings);
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
		if (strcasecmp(argv[1],"--getwapsettings") == 0) {
			error=Phone->GetWAPSettings(&s,&settings);
		} else {
			error=Phone->GetMMSSettings(&s,&settings);
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

#ifdef GSM_ENABLE_BACKUP
static void BackupSMS(int argc, char *argv[])
{
	GSM_SMS_Backup		Backup;
	GSM_MultiSMSMessage 	sms;
	GSM_SMSFolders		folders;
	bool			BackupFromFolder[GSM_MAX_SMS_FOLDERS];
	bool			start = true;
	bool			DeleteAfter;
	int			j, smsnum = 0;
	char			buffer[200];

	GSM_Init(true);

	error=Phone->GetSMSFolders(&s, &folders);
	Print_Error(error);

	DeleteAfter=answer_yes(_("Delete each sms after backup"));

	for (j=0;j<folders.Number;j++) {
		BackupFromFolder[j] = false;
		sprintf(buffer,"Backup sms from folder \"%s\"",DecodeUnicodeConsole(folders.Folder[j].Name));
		if (folders.Folder[j].Memory == MEM_SM) strcat(buffer," (SIM)");
		if (answer_yes(buffer)) BackupFromFolder[j] = true;
	}

	while (error == ERR_NONE) {
		sms.SMS[0].Folder=0x00;
		error=Phone->GetNextSMS(&s, &sms, start);
		switch (error) {
		case ERR_EMPTY:
			break;
		default:
			Print_Error(error);
			for (j=0;j<sms.Number;j++) {
				if (BackupFromFolder[sms.SMS[j].Folder-1]) {
					switch (sms.SMS[j].PDU) {
					case SMS_Status_Report:
						break;
					case SMS_Submit:
					case SMS_Deliver:
						if (sms.SMS[j].Length == 0) break;
						if (smsnum < GSM_BACKUP_MAX_SMS) {
							Backup.SMS[smsnum] = malloc(sizeof(GSM_SMSMessage));
						        if (Backup.SMS[smsnum] == NULL) Print_Error(ERR_MOREMEMORY);
							Backup.SMS[smsnum + 1] = NULL;
						} else {
							printf(_("   Increase %s\n") , "GSM_BACKUP_MAX_SMS");
							GSM_Terminate();
							exit(-1);
						}
						*Backup.SMS[smsnum] = sms.SMS[j];
						smsnum++;
						break;
					}
				}
			}
		}
		start=false;
	}

	error = GSM_AddSMSBackupFile(argv[2],&Backup);
	Print_Error(error);

	if (DeleteAfter) {
		for (j=0;j<smsnum;j++) {
			Backup.SMS[j]->Folder = 0;
			error=Phone->DeleteSMS(&s, Backup.SMS[j]);
			Print_Error(error);
			fprintf(stderr, _("%cDeleting: %i percent"),13,(j+1)*100/smsnum);
		}
	}

	GSM_Terminate();
}

static void AddSMS(int argc, char *argv[])
{
	GSM_MultiSMSMessage 	SMS;
	GSM_SMS_Backup		Backup;
	int			smsnum = 0;
	int			folder;

	folder = atoi(argv[2]);

	error = GSM_ReadSMSBackupFile(argv[3], &Backup);
	Print_Error(error);

	GSM_Init(true);

	while (Backup.SMS[smsnum] != NULL) {
		Backup.SMS[smsnum]->Folder = folder;
		Backup.SMS[smsnum]->SMSC.Location = 1;
		SMS.Number = 1;
		SMS.SMS[0] = *Backup.SMS[smsnum];
		displaymultismsinfo(SMS,false,false,NULL);
		if (answer_yes(_("Restore sms"))) {
			error=Phone->AddSMS(&s, Backup.SMS[smsnum]);
			Print_Error(error);
		}
		smsnum++;
	}

	GSM_Terminate();
}

static void RestoreSMS(int argc, char *argv[])
{
	GSM_MultiSMSMessage 	SMS;
	GSM_SMS_Backup		Backup;
	GSM_SMSFolders		folders;
	int			smsnum = 0;
	char			buffer[200];
	bool			restore8bit,doit;

	error=GSM_ReadSMSBackupFile(argv[2], &Backup);
	Print_Error(error);

	sprintf(buffer, _("Do you want to restore binary SMS"));
	restore8bit = answer_yes(buffer);

	GSM_Init(true);

	error=Phone->GetSMSFolders(&s, &folders);
	Print_Error(error);

	while (Backup.SMS[smsnum] != NULL) {
		doit = true;
		if (!restore8bit && Backup.SMS[smsnum]->Coding == SMS_Coding_8bit) doit = false;
		if (doit) {
			SMS.Number = 1;
			memcpy(&SMS.SMS[0],Backup.SMS[smsnum],sizeof(GSM_SMSMessage));
			displaymultismsinfo(SMS,false,false,NULL);
			sprintf(buffer, _("Restore %03i sms to folder \"%s\""),smsnum+1,DecodeUnicodeConsole(folders.Folder[Backup.SMS[smsnum]->Folder-1].Name));
			if (folders.Folder[Backup.SMS[smsnum]->Folder-1].Memory == MEM_SM) strcat(buffer, _(" (SIM)"));
			if (answer_yes(buffer)) {
				smprintf(&s, _("saving %i SMS\n"),smsnum);
				error=Phone->AddSMS(&s, Backup.SMS[smsnum]);
				Print_Error(error);
			}
		}
		smsnum++;
	}

	GSM_Terminate();
}
#endif

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

#if defined(GSM_ENABLE_NOKIA_DCT3) || defined(GSM_ENABLE_NOKIA_DCT4)
static void NokiaComposer(int argc, char *argv[])
{
	GSM_Ringtone 		ringtone;
	bool			started;
	int 			i,j;
	GSM_RingNote 		*Note;
	GSM_RingNoteDuration 	Duration;
	GSM_RingNoteDuration 	DefNoteDuration = 32; /* 32 = Duration_1_4 */
	unsigned int		DefNoteScale 	= Scale_880;

	ringtone.Format	= 0;
	error=GSM_ReadRingtoneFile(argv[2],&ringtone);

	if (ringtone.Format != RING_NOTETONE) {
		printf("%s\n", _("It can be RTTL ringtone only used with this option"));
		exit(-1);
	}

	started = false;
	j	= 0;
	for (i=0;i<ringtone.NoteTone.NrCommands;i++) {
		if (ringtone.NoteTone.Commands[i].Type == RING_Note) {
			Note = &ringtone.NoteTone.Commands[i].Note;
			if (!started) {
				if (Note->Note != Note_Pause) {
					printf(_("Ringtone \"%s\" (tempo = %i Beats Per Minute)\n\n"),DecodeUnicodeConsole(ringtone.Name),GSM_RTTLGetTempo(Note->Tempo));
					started = true;
				}
			}
			if (started) j++;
		}
	}
    	if (j>50) printf_warn(_("length=%i notes, but you will enter only first 50 tones."),j);

	printf(_("\n\nThis ringtone in Nokia Composer in phone should look: "));
	started = false;
	for (i=0;i<ringtone.NoteTone.NrCommands;i++) {
		if (ringtone.NoteTone.Commands[i].Type == RING_Note) {
			Note = &ringtone.NoteTone.Commands[i].Note;
			if (!started) {
				if (Note->Note != Note_Pause) started = true;
			}
			if (started) {
				switch (Note->Duration) {
					case Duration_Full: printf("1"); break;
					case Duration_1_2 : printf("2"); break;
					case Duration_1_4 : printf("4"); break;
					case Duration_1_8 : printf("8"); break;
					case Duration_1_16: printf("16");break;
					case Duration_1_32: printf("32");break;
				}
				if (Note->DurationSpec == DottedNote) printf(_("."));
				switch (Note->Note) {
					case Note_C  	: printf("c");	break;
					case Note_Cis	: printf("#c");	break;
					case Note_D  	 :printf("d");	break;
					case Note_Dis	: printf("#d");	break;
					case Note_E  	: printf("e");	break;
					case Note_F  	: printf("f");	break;
					case Note_Fis	: printf("#f");	break;
					case Note_G  	: printf("g");	break;
					case Note_Gis	: printf("#g");	break;
					case Note_A  	: printf("a");	break;
					case Note_Ais	: printf("#a");	break;
					case Note_H  	: printf("h");	break;
					case Note_Pause : printf("-");	break;
				}
				if (Note->Note != Note_Pause) printf("%i",Note->Scale - 4);
				printf(" ");
			}
		}
	}

	printf(_("\n\nTo enter it please press: "));
	started = false;
	for (i=0;i<ringtone.NoteTone.NrCommands;i++) {
		if (ringtone.NoteTone.Commands[i].Type == RING_Note) {
			Note = &ringtone.NoteTone.Commands[i].Note;
			if (!started) {
				if (Note->Note != Note_Pause) started = true;
			}
			if (started) {
				switch (Note->Note) {
	      				case Note_C  : case Note_Cis:	printf("1");break;
	      				case Note_D  : case Note_Dis:	printf("2");break;
	      				case Note_E  :			printf("3");break;
	      				case Note_F  : case Note_Fis:	printf("4");break;
	      				case Note_G  : case Note_Gis:	printf("5");break;
	      				case Note_A  : case Note_Ais:	printf("6");break;
	      				case Note_H  :			printf("7");break;
	      				default      :			printf("0");break;
				}
				if (Note->DurationSpec == DottedNote) printf(_("(longer)"));
	    			switch (Note->Note) {
      					case Note_Cis: case Note_Dis:
      					case Note_Fis: case Note_Gis:
      					case Note_Ais:
						printf("#");
						break;
      					default      :
						break;
    				}
				if (Note->Note != Note_Pause) {
					if ((unsigned int)Note->Scale != DefNoteScale) {
						while (DefNoteScale != (unsigned int)Note->Scale) {
							printf("*");
							DefNoteScale++;
							if (DefNoteScale==Scale_7040) DefNoteScale = Scale_880;
						}
					}
				}
				Duration = 0;
				switch (Note->Duration) {
					case Duration_Full : Duration = 128;	break;
					case Duration_1_2  : Duration = 64;	break;
					case Duration_1_4  : Duration = 32;	break;
					case Duration_1_8  : Duration = 16;	break;
					case Duration_1_16 : Duration = 8;	break;
					case Duration_1_32 : Duration = 4;	break;
					default		   : dbgprintf("error\n");break;
				}
				if (Duration > DefNoteDuration) {
		        		while (DefNoteDuration != Duration) {
						printf("9");
			  			DefNoteDuration = DefNoteDuration * 2;
					}
			      	}
				if (Duration < DefNoteDuration) {
		        		while (DefNoteDuration != Duration) {
						printf("8");
			  			DefNoteDuration = DefNoteDuration / 2;
					}
			      	}
				printf(" ");
			}
		}
	}

	printf("\n");
}
#endif

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

static void PressKeySequence(int argc, char *argv[])
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
		error=Phone->PressKey(&s, KeyCode[i], true);
		Print_Error(error);
		error=Phone->PressKey(&s, KeyCode[i], false);
		Print_Error(error);
	}

	GSM_Terminate();
}

static void GetAllCategories(int argc, char *argv[])
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

	error=Phone->GetCategoryStatus(&s, &Status);
	Print_Error(error);

	for (count=0,j=1;count<Status.Used;j++)
	{
    		Category.Location=j;
		error=Phone->GetCategory(&s, &Category);

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

		error=Phone->GetCategory(&s, &Category);
		if (error != ERR_EMPTY) Print_Error(error);

		if (error == ERR_EMPTY) {
			printf("%s\n", _("Entry is empty"));
		} else {
        		printf(LISTFORMAT "\"%s\"\n\n", _("Name"),DecodeUnicodeConsole(Category.Name));
    		}
	}

	GSM_Terminate();
}

static void AddCategory(int argc, char *argv[])
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

	error = Phone->AddCategory(&s, &Category);

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
		error=Phone->DeleteToDo(&s,&ToDo);
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
			printf(LISTFORMAT "%s\n", _("DueTime"),OSDateTime(ToDo->Entries[j].Date,false));
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
			error=Phone->GetCategory(&s, &Category);
			if (error == ERR_NONE) {
				printf(LISTFORMAT "\"%s\" (%i)\n", _("Category"), DecodeUnicodeConsole(Category.Name), ToDo->Entries[j].Number);
			} else {
				printf(LISTFORMAT "%i\n", _("Category"), ToDo->Entries[j].Number);
			}
			break;
		case TODO_CONTACTID:
			entry.Location = ToDo->Entries[j].Number;
			entry.MemoryType = MEM_ME;
			error=Phone->GetMemory(&s, &entry);
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
		error = Phone->GetNextToDo(&s, &Entry, start);
		if (error == ERR_EMPTY) break;
		Print_Error(error);
		for (j=0;j<Entry.EntriesNum;j++) {
			if (Entry.Entries[j].EntryType == TODO_CATEGORY && Entry.Entries[j].Number == (unsigned int)Category)
				PrintToDo(&Entry);
		}
 		start = false;
	}
}

static void ListToDoCategory(int argc, char *argv[])
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

		if (Phone->GetCategoryStatus(&s, &Status) == ERR_NONE) {
			for (count=0,j=1;count<Status.Used;j++) {
				Category.Location=j;
				error=Phone->GetCategory(&s, &Category);

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
		error = Phone->GetToDo(&s,&ToDo);
		if (error == ERR_EMPTY) continue;
		Print_Error(error);
		PrintToDo(&ToDo);
	}

	GSM_Terminate();
}

static void GetAllToDo(int argc, char *argv[])
{
	GSM_ToDoEntry		ToDo;
	bool			start = true;

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));

	GSM_Init(true);

	while (!gshutdown) {
		error = Phone->GetNextToDo(&s, &ToDo, start);
		if (error == ERR_EMPTY) break;
		Print_Error(error);
		PrintToDo(&ToDo);
 		start = false;
	}

	GSM_Terminate();
}

static void GetAllNotes(int argc, char *argv[])
{
	GSM_NoteEntry		Note;
	bool			start = true;

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));

	GSM_Init(true);

	while (!gshutdown) {
		error = Phone->GetNextNote(&s, &Note, start);
		if (error == ERR_EMPTY) break;
		Print_Error(error);
		printf(LISTFORMAT "\"%s\"\n", _("Text"),DecodeUnicodeConsole(Note.Text));
       	    	printf("\n");
 		start = false;
	}
	GSM_Terminate();
}

static void GetSecurityStatus(int argc, char *argv[])
{
	GSM_Init(true);

	PrintSecurityStatus();

	GSM_Terminate();
}

static void EnterSecurityCode(int argc, char *argv[])
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

	error=Phone->EnterSecurityCode(&s,Code);
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

	error=Phone->GetRingtonesInfo(&s,&Info);
	if (error != ERR_NONE) Info.Number = 0;

	for (i=start;i<=stop;i++) {
		Profile.Location=i;
		error=Phone->GetProfile(&s,&Profile);
		if (error != ERR_NONE && Info.Ringtone) free(Info.Ringtone);
		Print_Error(error);

		printf("%i. \"%s\"",i,DecodeUnicodeConsole(Profile.Name));
		if (Profile.Active)		printf(_(" (active)"));
		if (Profile.DefaultName) 	printf(_(" (default name)"));
		if (Profile.HeadSetProfile) 	printf(_(" (HeadSet profile)"));
		if (Profile.CarKitProfile) 	printf(_(" (CarKit profile)"));
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
							error=Phone->GetBitmap(&s,&caller[k]);
							if (error == ERR_SECURITYERROR) {
								NOKIA_GetDefaultCallerGroupName(&s,&caller[k]);
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
		error=Phone->GetSpeedDial(&s,&SpeedDial);
		printf(LISTFORMAT "%i\n", _("Location"), i);
		switch (error) {
		case ERR_EMPTY:
			printf("%s\n", _(" speed dial not assigned"));
			break;
		default:
			Print_Error(error);

			Phonebook.Location	= SpeedDial.MemoryLocation;
			Phonebook.MemoryType 	= SpeedDial.MemoryType;
			error=Phone->GetMemory(&s,&Phonebook);

			GSM_PhonebookFindDefaultNameNumberGroup(&Phonebook, &Name, &Number, &Group);

			if (Name != -1) printf(LISTFORMAT "\"%s\"\n", _("Name"), DecodeUnicodeConsole(Phonebook.Entries[Name].Text));
			printf(LISTFORMAT "\"%s\"\"", _("Number"), DecodeUnicodeConsole(Phonebook.Entries[SpeedDial.MemoryNumberID-1].Text));
		}
		printf("\n");
	}

	GSM_Terminate();
}

static void ResetPhoneSettings(int argc, char *argv[])
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

	error=Phone->ResetPhoneSettings(&s,Type);
	Print_Error(error);

 	GSM_Terminate();
}

#if defined(GSM_ENABLE_NOKIA_DCT3) || defined(GSM_ENABLE_NOKIA_DCT4)
static void NokiaSecurityCode(int argc, char *argv[])
{
	GSM_Init(true);

#ifdef GSM_ENABLE_NOKIA_DCT3
	DCT3GetSecurityCode(argc,argv);
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
//	DCT4ResetSecurityCode(argc, argv);
	DCT4GetSecurityCode(argc,argv);
#endif

	GSM_Terminate();
}

static void NokiaSetPhoneMenus(int argc, char *argv[])
{
	GSM_Init(true);

#ifdef GSM_ENABLE_NOKIA_DCT3
	DCT3SetPhoneMenus (argc, argv);
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
	DCT4SetPhoneMenus (argc, argv);
#endif

	GSM_Terminate();
}

static void NokiaSelfTests(int argc, char *argv[])
{
	GSM_Init(true);

#ifdef GSM_ENABLE_NOKIA_DCT3
	DCT3SelfTests(argc, argv);
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
	DCT4SelfTests(argc, argv);
#endif

	GSM_Terminate();
}
#endif

static void DeleteAllSMS(int argc, char *argv[])
{
	GSM_MultiSMSMessage 	sms;
	GSM_SMSFolders		folders;
	int			foldernum;
	bool			start = true;

	GSM_Init(true);

	error=Phone->GetSMSFolders(&s, &folders);
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
		error=Phone->GetNextSMS(&s, &sms, start);
		switch (error) {
		case ERR_EMPTY:
			break;
		default:
			Print_Error(error);
			if (sms.SMS[0].Folder == foldernum) {
				sms.SMS[0].Folder=0x00;
				error=Phone->DeleteSMS(&s, &sms.SMS[0]);
				Print_Error(error);
				printf("*");
			}
		}
		start=false;
	}

	printf("\n");

	GSM_Terminate();
}

static void SendDTMF(int argc, char *argv[])
{
	GSM_Init(true);

	error=Phone->SendDTMF(&s,argv[2]);
	Print_Error(error);

 	GSM_Terminate();
}

static void GetDisplayStatus(int argc, char *argv[])
{
	GSM_DisplayFeatures 	Features;
	int			i;

	GSM_Init(true);

	error=Phone->GetDisplayStatus(&s,&Features);
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

static void SetAutoNetworkLogin(int argc, char *argv[])
{
	GSM_Init(true);

	error=Phone->SetAutoNetworkLogin(&s);
	Print_Error(error);

 	GSM_Terminate();
}

#ifdef DEBUG
static void MakeConvertTable(int argc, char *argv[])
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

static void Version(int argc, char *argv[])
{
#ifdef DEBUG
	GSM_DateTime	dt;
	unsigned char	bzz[4];
	int		j,z,w;
#endif

	printf(_("[Gammu version %s built %s on %s using %s]"),
		VERSION,
		__TIME__,
		__DATE__,
		GetCompiler());
	printf("\n\n");
	printf("%s\n", _("This is free software.  You may redistribute copies of it under the terms of"));
	printf("%s\n", _("the GNU General Public License <http://www.gnu.org/licenses/gpl.html>."));
	printf("%s\n", _("There is NO WARRANTY, to the extent permitted by law."));
	printf("\n\n");

#ifdef DEBUG
	for (w = 1; w < 65535; w++) {
		j = EncodeWithUTF8Alphabet2(w / 256, w % 256, bzz);
		for (z = 0; z < j; z++) {
			if (bzz[z] == 0x00) printf("%i\n", w);
		}
	}

	printf("GSM_SMSMessage  - %zd\n", sizeof(GSM_SMSMessage));
	printf("GSM_SMSC        - %zd\n", sizeof(GSM_SMSC));
	printf("GSM_SMS_State   - %zd\n", sizeof(GSM_SMS_State));
	printf("GSM_UDHHeader   - %zd\n", sizeof(GSM_UDHHeader));
	printf("bool            - %zd\n", sizeof(bool));
	printf("GSM_DateTime    - %zd\n", sizeof(GSM_DateTime));
	printf("int             - %zd\n", sizeof(int));
	printf("GSM_NetworkInfo - %zd\n", sizeof(GSM_NetworkInfo));
	dt.Year = 2005;
	dt.Month = 2;
	dt.Day=29;
	if (CheckDate(&dt)) printf("ok1");
	dt.Year = 2008;
	dt.Month = 2;
	dt.Day=29;
	if (CheckDate(&dt)) printf("ok2");
#endif
}

static void GetFMStation(int argc, char *argv[])
{
	GSM_FMStation 	Station;
	int		start,stop;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		Station.Location=i;
		error=Phone->GetFMStation(&s,&Station);
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

	error = Phone->GetFileSystemStatus(&s,&Status);
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

static void GetFileSystemStatus(int argc, char *argv[])
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
		error = Phone->GetNextFileFolder(&s,&Files,Start);
		if (error == ERR_EMPTY) break;
	    	if (error != ERR_FOLDERPART) Print_Error(error);

		if (!Files.Folder) {
			if (IsPhoneFeatureAvailable(s.Phone.Data.ModelInfo, F_FILES2)) {
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
						printf(" %9i",Files.Used);
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
				printf("%i;",Files.Used);
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

	error = Phone->SetFileAttributes(&s,&Files);
    	Print_Error(error);

	GSM_Terminate();
}

static void GetRootFolders(int argc, char *argv[])
{
	GSM_File 	File;
	char 		IDUTF[200];

	GSM_Init(true);

	File.ID_FullName[0] = 0;
	File.ID_FullName[1] = 0;

	while (1) {
		if (Phone->GetNextRootFolder(&s,&File)!=ERR_NONE) break;
		EncodeUTF8QuotedPrintable(IDUTF,File.ID_FullName);
		printf("%s ",IDUTF);
		printf("- %s\n",DecodeUnicodeString(File.Name));
	}

	GSM_Terminate();
}

static void GetFolderListing(int argc, char *argv[])
{
	bool 			Start = true;
	GSM_File	 	Files;
	char 			IDUTF[200];

	GSM_Init(true);

	DecodeUTF8QuotedPrintable(Files.ID_FullName,argv[2],strlen(argv[2]));

	while (1) {
		error = Phone->GetFolderListing(&s,&Files,Start);
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
			printf("%i;",Files.Used);
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
	GSM_DateTime		dt;
	long			diff;

	if (File->Buffer != NULL) {
		free(File->Buffer);
		File->Buffer = NULL;
	}
	File->Used 	= 0;
	start		= true;

	GSM_GetCurrentDateTime(&dt);
	t_time1 	= Fill_Time_T(dt);
	old1 		= 65536;

	error = ERR_NONE;
	while (error == ERR_NONE) {
		error = Phone->GetFilePart(&s,File,&Handle,&Size);
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
				fprintf(stderr, _("%c  %i percent"), 13, File->Used*100/Size);
				if (File->Used*100/Size >= 2) {
					GSM_GetCurrentDateTime(&dt);
					t_time2 = Fill_Time_T(dt);
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
		error = Phone->GetNextFileFolder(&s,&File,Start);
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

static void AddOneFile(GSM_File *File, char *text, bool send)
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
			error = Phone->SendFilePart(&s,File,&Pos,&Handle);
		} else {
			error = Phone->AddFilePart(&s,File,&Pos,&Handle);
		}
	    	if (error != ERR_EMPTY && error != ERR_WRONGCRC) Print_Error(error);
		if (File->Used != 0) {
			fprintf(stderr, _("%c%s%03i percent"),13,text,Pos*100/File->Used);
			if (Pos*100/File->Used >= 2) {
				GSM_GetCurrentDateTime(&dt);
				t_time2 = Fill_Time_T(dt);
				diff = t_time2-t_time1;
				i = diff*(File->Used-Pos)/Pos;
				if (i != 0) {
					if (i<old1) old1 = i;
					j = old1/60;
					fprintf(stderr, _(" (%02i:%02i minutes left)"),j,old1-j*60);
				} else {
					fprintf(stderr, "%30c",0x20);
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

	if (strcasecmp(argv[1], "--sendfile") == 0) {
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

static void AddFolder(int argc, char *argv[])
{
	char			IDUTF[200];
	GSM_File 		File;

	DecodeUTF8QuotedPrintable(File.ID_FullName,argv[2],strlen(argv[2]));
	EncodeUnicode(File.Name,argv[3],strlen(argv[3]));
	File.ReadOnly = false;

	GSM_Init(true);

	error = Phone->AddFolder(&s,&File);
    	Print_Error(error);
	EncodeUTF8QuotedPrintable(IDUTF,File.ID_FullName);
	printf(_("ID of new folder is \"%s\"\n"),IDUTF);

	GSM_Terminate();
}

static void DeleteFolder(int argc, char *argv[])
{
	unsigned char buffer[500];

	GSM_Init(true);

	DecodeUTF8QuotedPrintable(buffer,argv[2],strlen(argv[2]));

	error = Phone->DeleteFolder(&s,buffer);
    	Print_Error(error);

	GSM_Terminate();
}

#if defined(GSM_ENABLE_NOKIA_DCT3) || defined(GSM_ENABLE_NOKIA_DCT4)
typedef struct _PlayListEntry PlayListEntry;

struct _PlayListEntry {
	unsigned char		*Name;
	unsigned char		*NameUP;
	PlayListEntry		*Next;
};

static void NokiaAddPlayLists2(unsigned char *ID,unsigned char *Name,unsigned char *IDFolder)
{
	bool 			Start = true, Available = false;
	GSM_File	 	Files,Files2,Files3;
	int 			i,j,NamesPos=0,NamesPos2=0;
	unsigned char		Buffer[20],Buffer2[500];
	unsigned char		*Names,*Names2,*Pointer;
	PlayListEntry		*First,*Entry=NULL,*Prev;

	First = NULL; Names=NULL; Names2=NULL;

	CopyUnicodeString(Files.ID_FullName,ID);

	printf(_("Checking %s\n"),DecodeUnicodeString(Name));
	//looking into folder content (searching for mp3 and similiar)
	while (1) {
		error = Phone->GetFolderListing(&s,&Files,Start);
		if (error == ERR_FOLDERPART) {
			printf("%s\n", _("  Only part handled!"));
			break;
		}
		if (error == ERR_EMPTY) break;
		if (error == ERR_FILENOTEXIST) return;
	    	Print_Error(error);

		if (!Files.Folder) {
			if (strcasestr(DecodeUnicodeConsole(Files.Name),".mp3")!=NULL ||
			    strcasestr(DecodeUnicodeConsole(Files.Name),".aac")!=NULL) {
				if (First==NULL) {
					First = malloc(sizeof(PlayListEntry));
					Entry = First;
				} else {
					Entry->Next = malloc(sizeof(PlayListEntry));
					Entry = Entry->Next;
				}
				Entry->Next = NULL;
				Entry->Name = malloc(strlen(DecodeUnicodeString(Files.ID_FullName))+1);
				sprintf(Entry->Name,"%s",DecodeUnicodeString(Files.ID_FullName));
				//converting Gammu drives to phone drives
				if (Entry->Name[0]=='a' || Entry->Name[0]=='A') {
					Entry->Name[0]='b';
				} else if (Entry->Name[0]=='d' || Entry->Name[0]=='D') {
					Entry->Name[0]='a';
				}

				Entry->NameUP = malloc(strlen(DecodeUnicodeString(Files.ID_FullName))+1);
				for (i=0;i<(int)strlen(DecodeUnicodeString(Files.ID_FullName))+1;i++) {
					Entry->NameUP[i] = tolower(Entry->Name[i]);
				}
			}
		} else {
			Names = (unsigned char *)realloc(Names,NamesPos+UnicodeLength(Files.ID_FullName)*2+2);
			CopyUnicodeString(Names+NamesPos,Files.ID_FullName);
			NamesPos+=UnicodeLength(Files.ID_FullName)*2+2;

			Names2 = (unsigned char *)realloc(Names2,NamesPos2+UnicodeLength(Files.Name)*2+2);
			CopyUnicodeString(Names2+NamesPos2,Files.Name);
			NamesPos2+=UnicodeLength(Files.Name)*2+2;
		}

		Start = false;
	}
	if (First!=NULL) {
		//sorting songs names
		Entry=First;
		while (Entry->Next!=NULL) {
			if (strcmp(Entry->NameUP,Entry->Next->NameUP)>0) {
				Pointer=Entry->Next->Name;
				Entry->Next->Name = Entry->Name;
				Entry->Name = Pointer;

				Pointer=Entry->Next->NameUP;
				Entry->Next->NameUP = Entry->NameUP;
				Entry->NameUP = Pointer;

				Entry=First;
				continue;
			}
			Entry=Entry->Next;
		}
		//we checking, if file already exist.if yes, we look for another...
		i 		= 0;
		Files3.Buffer 	= NULL;
		while (1) {
			CopyUnicodeString(Files3.ID_FullName,IDFolder);
	        	CopyUnicodeString(Buffer2,Name);
			if (i!=0) {
				sprintf(Buffer,"%i",i);
		        	EncodeUnicode(Buffer2+UnicodeLength(Buffer2)*2,Buffer,strlen(Buffer));
			}
	        	EncodeUnicode(Buffer2+UnicodeLength(Buffer2)*2,".m3u",4);

			Start = true;
			Available = false;
			while (1) {
				error = Phone->GetFolderListing(&s,&Files3,Start);
				if (error == ERR_FOLDERPART) {
					printf("%s\n", _("  Problem with adding playlist"));
					break;
				}
				if (error == ERR_EMPTY) break;
			    	Print_Error(error);

				if (!Files3.Folder) {
					if (mywstrncasecmp(Buffer2,Files3.Name,-1)) {
						Available = true;
						break;
					}
				}
				Start = false;
			}
			if (!Available) break;
			i++;
		}

		//preparing new playlist file date
		Files2.System	 = false;
		Files2.Folder 	 = false;
		Files2.ReadOnly	 = false;
		Files2.Hidden	 = false;
		Files2.Protected = false;
		Files2.ModifiedEmpty = false;
		GSM_GetCurrentDateTime (&Files2.Modified);
		CopyUnicodeString(Files2.ID_FullName,IDFolder);
	        CopyUnicodeString(Files2.Name,Buffer2);

		//preparing new playlist file content
		Files2.Buffer=NULL;
		Files2.Buffer = (unsigned char *)realloc(Files2.Buffer,10);
		sprintf(Files2.Buffer,"#EXTM3U%c%c",13,10);
		Files2.Used = 9;
		Entry=First;
		while (Entry!=NULL) {
			Files2.Buffer = (unsigned char *)realloc(Files2.Buffer,Files2.Used+strlen(Entry->Name)+2+1);
			sprintf(Files2.Buffer+Files2.Used,"%s%c%c",Entry->Name,13,10);
			Files2.Used+=strlen(Entry->Name)+2;
			Entry=Entry->Next;
		}
		Files2.Used	 -= 2;
		for (i=0;i<Files2.Used;i++) {
			if (Files2.Buffer[i]=='/') Files2.Buffer[i]='\\';
		}

		//adding new playlist file
		sprintf(Buffer2,"  Writing %s: ",DecodeUnicodeString(Files2.Name));
		AddOneFile(&Files2, Buffer2, false);

		//cleaning buffers
		free(Files2.Buffer);
		Files2.Buffer=NULL;
		while (Entry!=NULL) {
			Entry=First;
			Prev=NULL;
			while (Entry->Next!=NULL) {
				Prev=Entry;
				Entry=Entry->Next;
			}
			free(Entry->Name);
			free(Entry->NameUP);
			free(Entry);
			Entry=NULL;
			if (Prev!=NULL) Prev->Next = NULL;
		}
	}

	//going into subfolders
	if (NamesPos!=0) {
		i = 0; j = 0;
		while (i!=NamesPos) {
			NokiaAddPlayLists2(Names+i,Names2+j,IDFolder);
			i+=UnicodeLength(Names+i)*2+2;
			j+=UnicodeLength(Names2+j)*2+2;
		}
	}
	free(Names);
	free(Names2);
}

static void NokiaAddPlayLists(int argc, char *argv[])
{
	bool 			Start = true;
	GSM_File	 	Files;
	unsigned char		buffer[20],buffer2[20],IDFolder[100];

	GSM_Init(true);

	//delete old playlists
	EncodeUnicode(IDFolder,"d:\\predefplaylist",17);
	CopyUnicodeString(Files.ID_FullName,IDFolder);
	error = Phone->GetFolderListing(&s,&Files,Start);
	if (error == ERR_FILENOTEXIST) {
		EncodeUnicode(IDFolder,"d:\\predefgallery\\predefplaylist",17+14);
		CopyUnicodeString(Files.ID_FullName,IDFolder);
		error = Phone->GetFolderListing(&s,&Files,Start);
	} else if (error != ERR_EMPTY) {
	    	Print_Error(error);
	}
	if (error == ERR_FILENOTEXIST) {
		printf("%s\n", _("Your phone model is not supported. Please report"));
		GSM_Terminate();
		exit(-1);
	} else if (error != ERR_EMPTY) {
	    	Print_Error(error);
	}
	while (1) {
		if (!Files.Folder) {
			if (strstr(DecodeUnicodeConsole(Files.Name),".m3u")!=NULL) {
				error = Phone->DeleteFile(&s,Files.ID_FullName);
			    	Print_Error(error);
			}
		}
		Start = false;
		error = Phone->GetFolderListing(&s,&Files,Start);
		if (error == ERR_FOLDERPART) {
			printf("%s\n", _("Problem with deleting playlist"));
			break;
		}
		if (error == ERR_EMPTY) break;
	    	Print_Error(error);
	}

	//go over phone memory and add new one playlists
	EncodeUnicode(buffer,"d:",2);
	EncodeUnicode(buffer2,"root",4);
	NokiaAddPlayLists2(buffer,buffer2,IDFolder);
	//go over memory card and add new one playlists
	EncodeUnicode(buffer,"a:",2);
	EncodeUnicode(buffer2,"root",4);
	NokiaAddPlayLists2(buffer,buffer2,IDFolder);

	GSM_Terminate();
}

struct NokiaFolderInfo {
	char	*model;
	char 	*parameter;
	char	*folder;
	char 	*level;
};

static struct NokiaFolderInfo Folder[] = {
	/* Language indepedent in DCT4 in filesystem 1 */
	{"",	 "Application",	   "applications",	"3"},
	{"",	 "Game",	   "games",		"3"},
	/* Language indepedent in DCT4/TIKU/BB5 in filesystem 2 */
	{"", 	 "Gallery",	   "d:/predefgallery/predefgraphics",			""},
	{"", 	 "Gallery2",	   "d:/predefgallery/predefgraphics/predefcliparts",	""},
	{"", 	 "Camera",	   "d:/predefgallery/predefphotos",			""},
	{"", 	 "Tones",	   "d:/predefgallery/predeftones",			""},
	{"", 	 "Tones2",	   "d:/predefgallery/predefmusic",			""},
	{"", 	 "Records",	   "d:/predefgallery/predefrecordings",			""},
	{"", 	 "Video",	   "d:/predefgallery/predefvideos",			""},
	{"", 	 "Playlist",	   "d:/predefplaylist",					""},
	{"", 	 "MemoryCard",	   "a:",						""},
	    //now values first seen in S40 3.0
	{"",	 "Application",	   "d:/predefjava/predefcollections",			""},
	{"",	 "Game",	   "d:/predefjava/predefgames",				""},

	/* Language depedent in DCT4 filesystem 1 */
	{"",	 "Gallery",	   "Clip-arts",					"3"},
	{"",	 "Gallery",	   "004F006200720061007A006B0069",		"3"},//obrazki PL 6220
	{"",	 "Gallery",	   "Pictures",					"2"},//3510
	{"",	 "Gallery2",	   "Graphics",					"3"},
	{"",	 "Gallery2",	   "00470072006100660069006B0061",		"3"},//grafika PL 6220
	{"",	 "Camera",	   "Images",					"3"},
	{"",	 "Camera",	   "005A0064006A0119006300690061",		"3"},//zdjecia PL 6220
	{"",	 "Tones",	   "Tones",					"3"},
	{"",	 "Tones",	   "0044017A007700690119006B0069",		"3"},//dzwieki pl 6220
	{"",	 "Records",	   "Recordings",				"3"},
	{"",	 "Records",	   "004E0061006700720061006E00690061",		"3"},//nagrania pl 6220
	{"",	 "Video",	   "Video clips",				"3"},
	{"",	 "Video",	   "0057006900640065006F006B006C006900700079",	"3"},//wideoklipy pl 6220

	/* Language indepedent in OBEX */
	{"obex", "MMSUnreadInbox", "predefMessages\\predefINBOX", 		""},
	{"obex", "MMSReadInbox",   "predefMessages\\predefINBOX", 		""},
	{"obex", "MMSOutbox",	   "predefMessages\\predefOUTBOX", 		""},
	{"obex", "MMSSent",	   "predefMessages\\predefSENT", 		""},
	{"obex", "MMSDrafts",	   "predefMessages\\predefDRAFTS", 		""},
//	{"obex", "Application,	   "predefjava\\predefapplications", 		""},
//	{"obex", "Game",	   "predefjava\\predefgames", 			""},
	{"obex", "Gallery",	   "predefgallery\\predefgraphics", 		""},
	{"obex", "Tones",	   "predefgallery\\predeftones", 		""},

	/* End of list */
	{"",	 "",		   "",		""}
};

static void NokiaAddFile(int argc, char *argv[])
{
	GSM_File		File, Files, File2;
	FILE			*file;
	unsigned char 		buffer[10000],JAR[500],Vendor[500],Name[500],Version[500],FileID[400];
	bool 			Start = true, Found = false, wasclr;
	bool			ModEmpty = false, Overwrite = false;
	int			i = 0, Pos, Size, Size2, nextlong;

	while (Folder[i].parameter[0] != 0) {
		if (strcasecmp(argv[2],Folder[i].parameter) == 0) {
			Found = true;
			break;
		}
		i++;
	}
	if (!Found) {
		printf(_("What folder type (\"%s\") ?\n"),argv[2]);
		exit(-1);
	}

	if (strcasecmp(argv[2],"Application") == 0 || strcasecmp(argv[2],"Game") == 0) {
		sprintf(buffer,"%s.jad",argv[3]);
		file = fopen(buffer,"rb");
		if (file == NULL) Print_Error(ERR_CANTOPENFILE);
		fclose(file);
		sprintf(buffer,"%s.jar",argv[3]);
		file = fopen(buffer,"rb");
		if (file == NULL) Print_Error(ERR_CANTOPENFILE);
		fclose(file);
	} else {
		file = fopen(argv[3],"rb");
		if (file == NULL) Print_Error(ERR_CANTOPENFILE);
		fclose(file);
	}

	GSM_Init(true);

	Found = false;
    	if (s.ConnectionType == GCT_IRDAOBEX || s.ConnectionType == GCT_BLUEOBEX) {
		i = 0;
		while (Folder[i].parameter[0] != 0) {
			if (!strcmp("obex",Folder[i].model) &&
			     strcasecmp(argv[2],Folder[i].parameter) == 0) {
				EncodeUnicode(Files.ID_FullName,Folder[i].folder,strlen(Folder[i].folder));
				Found = true;
				break;
			}
			i++;
		}
		if (!Found) {
			printf("%s\n", _("Folder not found. Probably function not supported !"));
			GSM_Terminate();
			exit(-1);
		}
	} else if (IsPhoneFeatureAvailable(s.Phone.Data.ModelInfo, F_FILES2)) {
		i = 0;
		while (Folder[i].parameter[0] != 0) {
			if ((Folder[i].folder[0] == 'a' || Folder[i].folder[0] == 'd') &&
			    Folder[i].level[0] == 0x00 &&
			    strcasecmp(argv[2],Folder[i].parameter) == 0) {
				if (strstr(Folder[i].folder,"d:/predefjava/")!= NULL &&
				    !IsPhoneFeatureAvailable(s.Phone.Data.ModelInfo, F_SERIES40_30)) {
					i++;
					continue;
				}
				EncodeUnicode(Files.ID_FullName,Folder[i].folder,strlen(Folder[i].folder));
				Found = true;
				break;
			}
			i++;
		}
	}
	if (!Found) {
		fprintf(stderr, _("Searching for phone folder: "));
		while (1) {
			error = Phone->GetNextFileFolder(&s,&Files,Start);
			if (error == ERR_EMPTY) break;
		    	Print_Error(error);

			if (Files.Folder) {
				dbgprintf("folder %s level %i\n",DecodeUnicodeConsole(Files.Name),Files.Level);
				Found 	= false;
				i 	= 0;
				while (Folder[i].parameter[0] != 0) {
					EncodeUnicode(buffer,Folder[i].folder,strlen(Folder[i].folder));
					dbgprintf("comparing \"%s\" \"%s\" \"%s\"\n",s.Phone.Data.ModelInfo->model,DecodeUnicodeString(Files.ID_FullName),Folder[i].level);
					if (strcasecmp(argv[2],Folder[i].parameter) == 0  &&
					    mywstrncasecmp(Files.Name,buffer,0) &&
					    Files.Level == atoi(Folder[i].level)) {
						dbgprintf("found 1\n");
						Found = true;
						break;
					}
					if (strcasecmp(argv[2],Folder[i].parameter) == 0 &&
					    !strcmp(DecodeUnicodeString(Files.ID_FullName),Folder[i].folder) &&
					    Folder[i].level[0] == 0x00) {
						Found = true;
						dbgprintf("found 2\n");
						break;
					}
					if (Folder[i].folder[0]>='0'&&Folder[i].folder[0] <='9') {
						DecodeHexUnicode (buffer, Folder[i].folder,strlen(Folder[i].folder));
						dbgprintf("comparing \"%s\"",DecodeUnicodeString(buffer));
						dbgprintf("and \"%s\"\n",DecodeUnicodeString(Files.Name));
						if (strcasecmp(argv[2],Folder[i].parameter) == 0  &&
						    mywstrncasecmp(Files.Name,buffer,0) &&
						    Files.Level == atoi(Folder[i].level)) {
							Found = true;
							dbgprintf("found 3\n");
							break;
						}
					}
					i++;
				}
				if (Found) break;
			}
			fprintf(stderr, "*");

			Start = false;
		}
		fprintf(stderr, "\n");
	}
	if (!Found) {
		printf("%s\n", _("Folder not found. Probably function not supported !"));
		GSM_Terminate();
		exit(-1);
	}
	File.Buffer 	= NULL;
	File.Protected  = false;
	File.ReadOnly   = false;
	File.Hidden	= false;
	File.System	= false;

	if (strcasecmp(argv[2],"Application") == 0 || strcasecmp(argv[2],"Game") == 0) {
		/* reading jar file */
		sprintf(buffer,"%s.jar",argv[3]);
		error = GSM_ReadFile(buffer, &File);
		Print_Error(error);
		Size2 = File.Used;

		/* reading jad file */
		sprintf(buffer,"%s.jad",argv[3]);
		error = GSM_ReadFile(buffer, &File);
		Print_Error(error);

		/* Getting values from JAD file */
		error = GSM_JADFindData(File, Vendor, Name, JAR, Version, &Size);
		if (error == ERR_FILENOTSUPPORTED) {
			if (Vendor[0] == 0x00) {
				fprintf(stderr, "%s\n", _("No vendor info in JAD file"));
				GSM_Terminate();
				return;
			}
			if (Name[0] == 0x00) {
				fprintf(stderr, "%s\n", _("No name info in JAD file"));
				GSM_Terminate();
				return;
			}
			if (JAR[0] == 0x00) {
				fprintf(stderr, "%s\n", _("No JAR URL info in JAD file"));
				GSM_Terminate();
				return;
			}
		}
		if (Size != Size2) {
			fprintf(stderr, "%s\n", _("INFO: declared JAR file size is different than real. Fixed by Gammu"));
			for (i=0;i<File.Used;i++) {
				if (strncasecmp(File.Buffer+i,"MIDlet-Jar-Size: ",17) == 0) {
					break;
				}
			}
			Pos = i;
			while (true) {
				if (Pos ==0 || File.Buffer[Pos] == 13 || File.Buffer[Pos] == 10) break;
				Pos--;
			}
			i+= 15;
			while (true) {
				if (i == File.Used || File.Buffer[i] == 13 || File.Buffer[i] == 10) break;
				i++;
			}
			while (i != File.Used) {
				File.Buffer[Pos] = File.Buffer[i];
				i++;
				Pos++;
			}
			File.Used = File.Used - (i - Pos);
			File.Buffer = realloc(File.Buffer,File.Used);
		} else if (Size == -1) {
			fprintf(stderr, "%s\n", _("INFO: no JAR size info in JAD file. Added by Gammu"));
		}
		if (Size != Size2) {
			sprintf(buffer,"\nMIDlet-Jar-Size: %i",Size2);
			File.Buffer = realloc(File.Buffer,File.Used + strlen(buffer));
			memcpy(File.Buffer+File.Used,buffer,strlen(buffer));
			File.Used += strlen(buffer);
			Size = Size2;
		}
  		fprintf(stderr, _("Adding \"%s\""),Name);
		if (Version[0] != 0x00) fprintf(stderr, _(" version %s"),Version);
		fprintf(stderr, _(" created by %s\n"),Vendor);

		/* Bostjan Muller 3200 RH-30 3.08 */
		if (strstr(JAR,"http://") != NULL) {
			i = strlen(JAR)-1;
			while (JAR[i] != '/') i--;
			strcpy(buffer,JAR+i+1);
			strcpy(JAR,buffer);
			dbgprintf("New file name is \"%s\"\n",JAR);
		}

		/* Changing all #13 or #10 to #13#10 in JAD */
		Pos    = 0;
		wasclr = false;
		for (i=0;i<File.Used;i++) {
			switch (File.Buffer[i]) {
			case 0x0D:
			case 0x0A:
				if (!wasclr) {
					buffer[Pos++] = 0x0D;
					buffer[Pos++] = 0x0A;
					wasclr = true;
				} else wasclr = false;
				break;
			default:
				buffer[Pos++] 	= File.Buffer[i];
				wasclr		= false;
			}
		}
		File.Buffer = realloc(File.Buffer, Pos);
		File.Used   = Pos;
		memcpy(File.Buffer,buffer,Pos);

		if (argc > 4) {
			for (i=4;i<argc;i++) {
				if (strcasecmp(argv[i],"-overwrite") == 0) Overwrite = true;
			}
		}

		/* adding folder */
		if (strstr(DecodeUnicodeString(Files.ID_FullName),"d:/predefjava/")== NULL) {
			strcpy(buffer,Vendor);
			strcat(buffer,Name);
			EncodeUnicode(File.Name,buffer,strlen(buffer));
			CopyUnicodeString(File.ID_FullName,Files.ID_FullName);
			error = Phone->AddFolder(&s,&File);
			if (Overwrite && (error == ERR_FILEALREADYEXIST)) {
				fprintf(stderr, "%s\n", _("INFO: Application already exist. Deleting by Gammu"));

				Start = true;
				CopyUnicodeString(File2.ID_FullName,Files.ID_FullName);
				while (1) {
					error = Phone->GetFolderListing(&s,&File2,Start);
					if (error == ERR_EMPTY) break;
					Print_Error(error);

					if (File2.Folder && !strcmp(DecodeUnicodeString(File2.Name),buffer)) {
						break;
					}

					Start = false;
				}

				CopyUnicodeString(buffer,File2.ID_FullName);
				while (1) {
					CopyUnicodeString(File2.ID_FullName,buffer);
					error = Phone->GetFolderListing(&s,&File2,true);
					if (error == ERR_EMPTY) break;
					Print_Error(error);

					fprintf(stderr, _("  Deleting %s\n"),DecodeUnicodeString(File2.Name));

					error = Phone->DeleteFile(&s,File2.ID_FullName);
					Print_Error(error);
				}

				CopyUnicodeString(File.ID_FullName,buffer);
			} else {
			    	Print_Error(error);
			}
			CopyUnicodeString(FileID,File.ID_FullName);
		} else {
			CopyUnicodeString(FileID,Files.ID_FullName);
			CopyUnicodeString(File.ID_FullName,Files.ID_FullName);
		}

		/* adding jad file */
		strcpy(buffer,JAR);
		buffer[strlen(buffer) - 1] = 'd';
		EncodeUnicode(File.Name,buffer,strlen(buffer));
		File.Type 	   = GSM_File_Other;
		File.ModifiedEmpty = true;
		dbgprintf("file id is \"%s\"\n",DecodeUnicodeString(File.ID_FullName));
		AddOneFile(&File, "Writing JAD file: ", false);

		if (argc > 4) {
			for (i=4;i<argc;i++) {
				if (strcasecmp(argv[i],"-readonly") == 0) File.ReadOnly = true;
			}
		}

		/* reading jar file */
		sprintf(buffer,"%s.jar",argv[3]);
		error = GSM_ReadFile(buffer, &File);
		Print_Error(error);

		/* adding jar file */
		CopyUnicodeString(File.ID_FullName,FileID);
		strcpy(buffer,JAR);
		EncodeUnicode(File.Name,buffer,strlen(buffer));
		File.Type 	   = GSM_File_Java_JAR;
		File.ModifiedEmpty = true;
		AddOneFile(&File, "Writing JAR file: ", false);
		free(File.Buffer);
		GSM_Terminate();
		return;
	}

	if (strcasecmp(argv[2],"Gallery" 	 ) == 0 ||
	    strcasecmp(argv[2],"Gallery2"	 ) == 0 ||
	    strcasecmp(argv[2],"Camera"  	 ) == 0 ||
	    strcasecmp(argv[2],"Tones"   	 ) == 0 ||
	    strcasecmp(argv[2],"Tones2"  	 ) == 0 ||
	    strcasecmp(argv[2],"Records" 	 ) == 0 ||
	    strcasecmp(argv[2],"Video"   	 ) == 0 ||
	    strcasecmp(argv[2],"Playlist"	 ) == 0 ||
	    strcasecmp(argv[2],"MemoryCard"   ) == 0) {
		strcpy(buffer,argv[3]);
		if (argc > 4) {
			nextlong = 0;
			for (i=4;i<argc;i++) {
				switch(nextlong) {
				case 0:
					if (strcasecmp(argv[i],"-name") == 0) {
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
						ModEmpty = true;
						continue;
					}
					printf(_("Parameter \"%s\" unknown\n"),argv[i]);
					exit(-1);
				case 1:
					strcpy(buffer,argv[i]);
					nextlong = 0;
					break;
				}
			}
			if (nextlong!=0) {
				printf_err("%s\n", _("Parameter missing!"));
				exit(-1);
			}
		}
	}

	error = GSM_ReadFile(argv[3], &File);
	Print_Error(error);
	if (ModEmpty) File.ModifiedEmpty = true;

	CopyUnicodeString(File.ID_FullName,Files.ID_FullName);
	EncodeUnicode(File.Name,buffer,strlen(buffer));
	for (i=strlen(buffer)-1;i>0;i--) {
		if (buffer[i] == '\\' || buffer[i] == '/') break;
	}
	if (buffer[i] == '\\' || buffer[i] == '/') {
		EncodeUnicode(File.Name,buffer+i+1,strlen(buffer)-i-1);
	}

	GSM_IdentifyFileFormat(&File);

	AddOneFile(&File, "Writing file: ", false);
	free(File.Buffer);
	GSM_Terminate();
}
#endif

static void DeleteFiles(int argc, char *argv[])
{
	int		i;
	unsigned char	buffer[500];

	GSM_Init(true);

	for (i=2;i<argc;i++) {
		DecodeUTF8QuotedPrintable(buffer,argv[i],strlen(argv[i]));
		error = Phone->DeleteFile(&s,buffer);
	    	Print_Error(error);
	}

	GSM_Terminate();
}

static void ReadMMSFile(int argc, char *argv[])
{
	GSM_File		File;
	int			num = -1;

	File.Buffer = NULL;
	error = GSM_ReadFile(argv[2], &File);
	Print_Error(error);

	if (argc>3 && strcasecmp(argv[3],"-save") == 0) num=0;

	DecodeMMSFile(&File,num);

	free(File.Buffer);
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
		error = Phone->GetCallDivert(&s,&cd);
	    	Print_Error(error);
		printf(_("Query:\n   Divert type: "));
	} else {
		cd.Request.Number[0] = 0;
		cd.Request.Number[1] = 0;
		if (argc > 5) EncodeUnicode(cd.Request.Number,argv[5],strlen(argv[5]));

		cd.Request.Timeout = 0;
		if (argc > 6) cd.Request.Timeout = atoi(argv[6]);

		error = Phone->SetCallDivert(&s,&cd);
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

static void CancelAllDiverts(int argc, char *argv[])
{
	GSM_Init(true);

	error = Phone->CancelAllDiverts(&s);
    	Print_Error(error);

	GSM_Terminate();
}

/**
 * Structure to hold information about connection for searching.
 */
typedef struct {
	unsigned char		Connection[50]; /**< Connection name */
} OneConnectionInfo;

/**
 * Structure to hold device information for phone searching.
 */
typedef struct {
	unsigned char 		Device[50]; /**< Device name */
	OneConnectionInfo 	Connections[5]; /**< List of connections to try */
} OneDeviceInfo;

int				num;
bool				SearchOutput;

void SearchPhoneThread(OneDeviceInfo *Info)
{
	int 		 	j;
	GSM_Error	 	error;
	GSM_StateMachine	ss;

	j = 0;
	while(strlen(Info->Connections[j].Connection) != 0) {
		memcpy(&ss.di,&s.di,sizeof(Debug_Info));
		ss.ConfigNum			= 1;
		ss.opened 			= false;
	    	ss.Config[0].UseGlobalDebugFile = s.Config[0].UseGlobalDebugFile;
		ss.Config[0].Localize		= s.Config[0].Localize;
		ss.Config[0].Device 		= Info->Device;
		ss.Config[0].Connection		= Info->Connections[j].Connection;
		ss.Config[0].SyncTime		= "no";
		ss.Config[0].DebugFile		= s.Config[0].DebugFile;
		ss.Config[0].Model[0]		= 0;
		strcpy(ss.Config[0].DebugLevel,s.Config[0].DebugLevel);
		ss.Config[0].LockDevice		= "no";
		ss.Config[0].StartInfo		= "no";

		error = GSM_InitConnection(&ss,1);
		if (SearchOutput) printf(_("Connection \"%s\" on device \"%s\"\n"),Info->Connections[j].Connection,Info->Device);
		if (error == ERR_NONE) {
			error=ss.Phone.Functions->GetManufacturer(&ss);
			if (error == ERR_NONE) {
				error=ss.Phone.Functions->GetModel(&ss);
				if (error == ERR_NONE) {
					if (!SearchOutput) printf(_("Connection \"%s\" on device \"%s\"\n"),Info->Connections[j].Connection,Info->Device);
					printf("\t" LISTFORMAT "%s\n", _("Manufacturer"),
						ss.Phone.Data.Manufacturer);
					printf("\t" LISTFORMAT "%s (%s)\n", _("Model"),
						ss.Phone.Data.ModelInfo->model,
						ss.Phone.Data.Model);
				} else {
					if (SearchOutput) printf("\t%s\n",print_error(error,ss.di.df));
				}
			} else {
				if (SearchOutput) printf("\t%s\n",print_error(error,ss.di.df));
			}
		} else {
			if (SearchOutput) printf("\t%s\n",print_error(error,ss.di.df));
		}
		if (error != ERR_DEVICEOPENERROR) {
			GSM_TerminateConnection(&ss);
			dbgprintf("Closing done\n");
		}
		if (error == ERR_DEVICEOPENERROR) break;
		j++;
	}
	num--;
}

#if defined(WIN32) || defined(HAVE_PTHREAD)
#ifdef HAVE_PTHREAD
	pthread_t Thread[100];
#endif

OneDeviceInfo SearchDevices[60];

void MakeSearchThread(int i)
{
	num++;
#ifdef HAVE_PTHREAD
	if (pthread_create(&Thread[i],NULL,(void *)SearchPhoneThread,&SearchDevices[i])!=0) {
		dbgprintf("Error creating thread\n");
	}
#else
	if (CreateThread((LPSECURITY_ATTRIBUTES)NULL,0,
		     (LPTHREAD_START_ROUTINE)SearchPhoneThread,&SearchDevices[i],
		     0,NULL)==NULL) {
		dbgprintf("Error creating thread\n");
	}
#endif
}

static void SearchPhone(int argc, char *argv[])
{
	int i,dev = 0, dev2 = 0;

	SearchOutput = false;
	if (argc == 3 && strcasecmp(argv[2], "-debug") == 0) SearchOutput = true;

	num = 0;
#ifdef WIN32
#  ifdef GSM_ENABLE_IRDADEVICE
	SearchDevices[dev].Device[0] = 0;
	sprintf(SearchDevices[dev].Connections[0].Connection,"irdaphonet");
	sprintf(SearchDevices[dev].Connections[1].Connection,"irdaat");
	SearchDevices[dev].Connections[2].Connection[0] = 0;
	dev++;
#  endif
#  ifdef GSM_ENABLE_SERIALDEVICE
	dev2 = dev;
	for(i=0;i<20;i++) {
		sprintf(SearchDevices[dev2].Device,"com%i:",i+1);
		sprintf(SearchDevices[dev2].Connections[0].Connection,"fbusdlr3");
		sprintf(SearchDevices[dev2].Connections[1].Connection,"fbus");
		sprintf(SearchDevices[dev2].Connections[2].Connection,"at19200");
		sprintf(SearchDevices[dev2].Connections[3].Connection,"mbus");
		SearchDevices[dev2].Connections[4].Connection[0] = 0;
		dev2++;
	}
#  endif
#endif
#ifdef __linux__
#  ifdef GSM_ENABLE_IRDADEVICE
	for(i=0;i<6;i++) {
		sprintf(SearchDevices[dev].Device,"/dev/ircomm%i",i);
		sprintf(SearchDevices[dev].Connections[0].Connection,"irdaphonet");
		sprintf(SearchDevices[dev].Connections[1].Connection,"at19200");
		SearchDevices[dev].Connections[2].Connection[0] = 0;
		dev++;
	}
#  endif
#  ifdef GSM_ENABLE_SERIALDEVICE
	dev2 = dev;
	for(i=0;i<10;i++) {
		sprintf(SearchDevices[dev2].Device,"/dev/ttyS%i",i);
		sprintf(SearchDevices[dev2].Connections[0].Connection,"fbusdlr3");
		sprintf(SearchDevices[dev2].Connections[1].Connection,"fbus");
		sprintf(SearchDevices[dev2].Connections[2].Connection,"at19200");
		sprintf(SearchDevices[dev2].Connections[3].Connection,"mbus");
		SearchDevices[dev2].Connections[4].Connection[0] = 0;
		dev2++;
	}
	for(i=0;i<8;i++) {
		sprintf(SearchDevices[dev2].Device,"/dev/ttyD00%i",i);
		sprintf(SearchDevices[dev2].Connections[0].Connection,"fbusdlr3");
		sprintf(SearchDevices[dev2].Connections[1].Connection,"fbus");
		sprintf(SearchDevices[dev2].Connections[2].Connection,"at19200");
		sprintf(SearchDevices[dev2].Connections[3].Connection,"mbus");
		SearchDevices[dev2].Connections[4].Connection[0] = 0;
		dev2++;
	}
        for(i=0;i<4;i++) {
		sprintf(SearchDevices[dev2].Device,"/dev/usb/tts/%i",i);
		sprintf(SearchDevices[dev2].Connections[0].Connection,"fbusdlr3");
		sprintf(SearchDevices[dev2].Connections[1].Connection,"fbus");
		sprintf(SearchDevices[dev2].Connections[2].Connection,"at19200");
		sprintf(SearchDevices[dev2].Connections[3].Connection,"mbus");
		SearchDevices[dev2].Connections[4].Connection[0] = 0;
		dev2++;
	}
#  endif
#endif
	for(i=0;i<dev;i++) MakeSearchThread(i);
	while (num != 0) my_sleep(5);
	for(i=dev;i<dev2;i++) MakeSearchThread(i);
	while (num != 0) my_sleep(5);
}
#endif /*Support for threads */

#if defined(GSM_ENABLE_NOKIA_DCT3) || defined(GSM_ENABLE_NOKIA_DCT4)
static void NokiaGetADC(int argc, char *argv[])
{
	GSM_Init(true);

#ifdef GSM_ENABLE_NOKIA_DCT3
	DCT3GetADC(argc,argv);
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
	DCT4GetADC(argc, argv);
#endif

	GSM_Terminate();
}

static void NokiaDisplayTest(int argc, char *argv[])
{
	GSM_Init(true);

#ifdef GSM_ENABLE_NOKIA_DCT3
	DCT3DisplayTest(argc,argv);
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
	DCT4DisplayTest(argc, argv);
#endif

	GSM_Terminate();
}

static void NokiaGetT9(int argc, char *argv[])
{
	GSM_Init(true);

#ifdef GSM_ENABLE_NOKIA_DCT3
	DCT3GetT9(argc,argv);
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
	DCT4GetT9(argc, argv);
#endif

	GSM_Terminate();
}

static void NokiaVibraTest(int argc, char *argv[])
{
	GSM_Init(true);

#ifdef GSM_ENABLE_NOKIA_DCT3
	DCT3VibraTest(argc,argv);
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
	DCT4VibraTest(argc, argv);
#endif

	GSM_Terminate();
}
#endif

#ifdef DEBUG
/**
 * Function for testing purposes.
 */
static void Foo(int argc, char *argv[])
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
	int i,j,c=0,argsc;
	char* argsv[20];
	bool origbatch;
	char *name;
	char std_name[] = "stdin";

	if (argc == 2 || strcmp(argv[2], "-") == 0) {
		bf = stdin;
		name = std_name;
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
				for (i = 1; i <= argsc; i++) {
					free(argsv[i]);
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

static GSM_Parameters Parameters[] = {
#ifdef DEBUG
	{"foo",			0, 0, Foo,			{0},				""},
#endif
	{"identify",			0, 0, Identify,			{H_Info,0},			""},
	{"version",			0, 0, Version,			{H_Other,0},			""},
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
	{"getmemory",			2, 4, GetMemory,		{H_Memory,0},			"DC|MC|RC|ON|VM|SM|ME|FD|SL start [stop [-nonempty]]"},
	{"getallmemory",		1, 2, GetAllMemory,		{H_Memory,0},			"DC|MC|RC|ON|VM|SM|ME|FD|SL"},
	{"searchmemory",		1, 1, SearchMemory,		{H_Memory,0},			"text"},
	{"listmemorycategory",	1, 1, ListMemoryCategory,	{H_Memory, H_Category,0},	"text|number"},
	{"getfmstation",		1, 2, GetFMStation,		{H_FM,0},			"start [stop]"},
	{"getsmsc",			1, 2, GetSMSC,			{H_SMS,0},			"start [stop]"},
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
	{"smsd",			2, 2, SMSDaemon,		{H_SMS,H_Other,0},		"MYSQL configfile"},
#endif
#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
	{"smsd",			2, 2, SMSDaemon,		{H_SMS,H_Other,0},		"PGSQL configfile"},
#endif
	{"smsd",			2, 2, SMSDaemon,		{H_SMS,H_Other,0},		"FILES configfile"},
	{"sendsmsdsms",		2,30, SendSaveDisplaySMS,	{H_SMS,H_Other,0},		"TEXT|WAPSETTINGS|... destination FILES|MYSQL|PGSQL configfile ... (options like in sendsms)"},
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
	{"addnew",			1, 1, AddNew,			{H_Backup,H_Memory,H_Calendar,H_ToDo,H_Category,H_Ringtone,H_WAP,H_FM,0},			"file"},
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
	{"",				0, 0, NULL			}
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
	{0,		NULL,		NULL}
};


void HelpHeader(void)
{
	printf(_("[Gammu version %s built %s %s]\n\n"),
			VERSION,
			__TIME__,
			__DATE__);
}

static void HelpGeneral(void)
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

static void HelpSplit(int cols, int len, unsigned char *buff)
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

static void Help(int argc, char *argv[])
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
		printf(_("Gammu parameters, topic: %s\n\n"), HelpDescriptions[i].description);
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
			printf("--%s", Parameters[j].parameter);
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
	int retval = 0, pos = 0;

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
				count_failed = true;
				if (Parameters[z].min_arg==Parameters[z].max_arg) {
					printf(_("More parameters required (function requires %d)\n"), Parameters[z].min_arg);
				} else {
					printf(_("More parameters required (function requires %d to %d)\n"), Parameters[z].min_arg, Parameters[z].max_arg);
				}
				if (Parameters[z].help[0] != 0) {
					printf(_("Parameters help: %s\n"), gettext(Parameters[z].help));
				}
			} else if (argc-2-start > Parameters[z].max_arg) {
				count_failed = true;
				if (Parameters[z].min_arg==Parameters[z].max_arg) {
					printf(_("Too many parameters (function accepts %d)\n"), Parameters[z].min_arg);
				} else {
					printf(_("Too many parameters (function accepts %d to %d)\n"), Parameters[z].min_arg, Parameters[z].max_arg);
				}
				if (Parameters[z].help[0] != 0) {
					printf(_("Parameters help: %s\n"), gettext(Parameters[z].help));
				}
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
	int		rsslevel = 0,pos = 0,oldpos = 0;
	int 		start = 0;
	unsigned int	i;
	int		only_config = -1;
	char		*cp,*rss,buff[200];


	s.opened 	= false;
	s.ConfigNum 	= 0;

	InitLocales(NULL);

#ifdef DEBUG
	di.dl		= DL_TEXTALL;
	di.df	 	= stdout;
	di.was_lf	= true;
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

	for (i = 0; i <= MAX_CONFIG_NUM; i++) {
		if (cfg!=NULL) {
		        cp = INI_GetValue(cfg, "gammu", "gammucoding", false);
        		if (cp) di.coding = cp;

		        s.Config[i].Localize = INI_GetValue(cfg, "gammu", "gammuloc", false);
			/* It is safe to pass NULL here */
			InitLocales(s.Config[i].Localize);
		}

		/* Wanted user specific configuration? */
		if (only_config != -1) {
			/* Here we get only in first for loop */
			if (!GSM_ReadConfig(cfg, &s.Config[0], only_config)) break;
		} else {
			if (!GSM_ReadConfig(cfg, &s.Config[i], i) && i != 0) break;
		}
		s.ConfigNum++;

     		/* We want to use only one file descriptor for global and state machine debug output */
 	    	s.Config[i].UseGlobalDebugFile = true;

		/* It makes no sense to open several debug logs... */
		if (i != 0) {
			strcpy(s.Config[i].DebugLevel, s.Config[0].DebugLevel);
			free(s.Config[i].DebugFile);
			s.Config[i].DebugFile = strdup(s.Config[0].DebugFile);
 		} else {
			/* Just for first config */
			/* When user gave debug level on command line */
			if (argc > 1 + start && GSM_SetDebugLevel(argv[1 + start], &di)) {
				/* Debug level from command line will be used with phone too */
				strcpy(s.Config[i].DebugLevel,argv[1 + start]);
				start++;
			} else {
				/* Try to set debug level from config file */
				GSM_SetDebugLevel(s.Config[i].DebugLevel, &di);
			}
			/* If user gave debug file in gammurc, we will use it */
			error=GSM_SetDebugFile(s.Config[i].DebugFile, &di);
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
						printf(_("INFO: there is later stable Gammu (%s instead of %s) available !\n"),buff,VERSION);
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
						printf(_("INFO: there is later test Gammu (%s instead of %s) available !\n"),buff,VERSION);
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
     	if (di.df!=stdout) fclose(di.df);

	exit(0);
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */


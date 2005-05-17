/* (c) 2002-2005 by Marcin Wiacek and Michal Cihar */
/* FM stuff by Walek */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <locale.h>
#include <signal.h>
#include <ctype.h>
#include <wchar.h>
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

#include "../common/gammu.h"
#include "gammu.h"
#include "smsd/smsdcore.h"
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


GSM_StateMachine		s;
GSM_Phone_Functions		*Phone;
static INI_Section		*cfg 			= NULL;

GSM_Error			error 			= ERR_NONE;
static int			i;

volatile bool 			gshutdown 		= false;
volatile bool 			wasincomingsms 		= false;
GSM_MultiSMSMessage		IncomingSMSData;

void interrupt(int sign)
{
	signal(sign, SIG_IGN);
	gshutdown = true;
}

#ifdef __GNUC__
__attribute__((format(printf, 1, 2)))
#endif
int printmsg(char *format, ...)
{
	va_list		argp;
	int 		result;

	va_start(argp, format);
	result = vfprintf(stdout,GetMsg(s.msg,format),argp);
	va_end(argp);
	return result;
}

#ifdef __GNUC__
__attribute__((format(printf, 1, 2)))
#endif
int printmsgerr(char *format, ...)
{
	va_list		argp;
	int 		result;

	va_start(argp, format);
	result = vfprintf(stderr,GetMsg(s.msg,format), argp);
	va_end(argp);
	return result;
}

static void PrintSecurityStatus()
{
	GSM_SecurityCodeType Status;

	error=Phone->GetSecurityStatus(&s,&Status);
	Print_Error(error);
	switch(Status) {
		case SEC_SecurityCode:
			printmsg("Waiting for Security Code.\n");
			break;
		case SEC_Pin:
			printmsg("Waiting for PIN.\n");
			break;
		case SEC_Pin2:
			printmsg("Waiting for PIN2.\n");
			break;
		case SEC_Puk:
			printmsg("Waiting for PUK.\n");
			break;
		case SEC_Puk2:
			printmsg("Waiting for PUK2.\n");
			break;
		case SEC_None:
			printmsg("Nothing to enter.\n");
			break;
		default:
			printmsg("Unknown\n");
	}
}

void Print_Error(GSM_Error error)
{
	if (error != ERR_NONE) {
 		printf("%s\n",print_error(error,s.di.df,s.msg));
		if (error == ERR_SECURITYERROR) {
			printmsg("Security status: ");
			PrintSecurityStatus();
		}
		if (s.opened) GSM_TerminateConnection(&s);
 		exit (-1);
 	}
}

void GSM_Init(bool checkerror)
{
	error=GSM_InitConnection(&s,3);
	if (checkerror) Print_Error(error);

	Phone=s.Phone.Functions;
}

void GSM_Terminate(void)
{
	error=GSM_TerminateConnection(&s);
	Print_Error(error);
}

static void GetStartStop(int *start, int *stop, int num, int argc, char *argv[])
{
	*start=atoi(argv[num]);
	if (*start==0) {
		printmsg("ERROR: enumerate locations from 1\n");
		exit (-1);
	}

	if (stop!=NULL) {
		*stop=*start;
		if (argc>=num+2) *stop=atoi(argv[num+1]);
		if (*stop==0) {
			printmsg("ERROR: enumerate locations from 1\n");
			exit (-1);
		}
	}
}

bool always_answer_yes = false;
bool always_answer_no  = false;

static bool answer_yes(char *text)
{
    	int         len;
    	char        ans[99];

	while (1) {
		printmsgerr("%s (yes/no/ALL/ONLY/NONE) ? ",text);
		if (always_answer_yes) {
			printmsgerr("YES (always)\n");
			return true;
		}
		if (always_answer_no) {
			printmsgerr("NO (always)\n");
			return false;
		}
		len=GetLine(stdin, ans, 99);
		if (len==-1) exit(-1);
		if (!strcmp(ans, "NONE")) {
			always_answer_no = true;
			return false;
		}
		if (!strcmp(ans, "ONLY")) {
			always_answer_no = true;
			return true;
		}
		if (!strcmp(ans, "ALL")) {
			always_answer_yes = true;
			return true;
		}
		if (mystrncasecmp(ans, "yes",0)) return true;
		if (mystrncasecmp(ans, "no" ,0)) return false;
	}
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
	printmsg("Press Ctrl+C to break...\n");

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
	printmsg("Manufacturer  : %s\n", s.Phone.Data.Manufacturer);
	error=Phone->GetModel(&s);
	Print_Error(error);
	printmsg("Model         : %s (%s)\n",
			s.Phone.Data.ModelInfo->model,
			s.Phone.Data.Model);

	error=Phone->GetFirmware(&s);
	Print_Error(error);
	printmsg("Firmware      : %s",s.Phone.Data.Version);
	error=Phone->GetPPM(&s, buffer);
	if (error != ERR_NOTSUPPORTED) {
		if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
		if (error == ERR_NONE) printmsg(" %s",buffer);
	}
	if (s.Phone.Data.VerDate[0]!=0) printmsg(" (%s)",s.Phone.Data.VerDate);
	printf("\n");

	error=Phone->GetHardware(&s, buffer);
	if (error != ERR_NOTSUPPORTED) {
		if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
		if (error == ERR_NONE) printmsg("Hardware      : %s\n",buffer);
	}

	error=Phone->GetIMEI(&s);
	if (error != ERR_NOTSUPPORTED) {
		if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
		if (error == ERR_NONE) printmsg("IMEI          : %s\n",s.Phone.Data.IMEI);

		error=Phone->GetOriginalIMEI(&s, buffer);
		if (error != ERR_NOTSUPPORTED && error != ERR_SECURITYERROR) {
			if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
			if (error == ERR_NONE) printmsg("Original IMEI : %s\n",buffer);
		}
	}

	error=Phone->GetManufactureMonth(&s, buffer);
	if (error != ERR_NOTSUPPORTED && error != ERR_SECURITYERROR) {
		if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
		if (error == ERR_NONE) printmsg("Manufactured  : %s\n",buffer);
	}

	error=Phone->GetProductCode(&s, buffer);
	if (error != ERR_NOTSUPPORTED) {
		if (error != ERR_NOTIMPLEMENTED) Print_Error(error);
		if (error == ERR_NONE) printmsg("Product code  : %s\n",buffer);
	}

	error=Phone->GetSIMIMSI(&s, buffer);
	switch (error) {
		case ERR_SECURITYERROR:
		case ERR_NOTSUPPORTED:
		case ERR_NOTIMPLEMENTED:
			break;
		case ERR_NONE:
			printmsg("SIM IMSI      : %s\n",buffer);
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
		printmsg("Date and time not set in phone\n");
		break;
	case ERR_NONE:
		printmsg("Phone time is %s\n",OSDateTime(date_time,false));
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
		printmsg("Time format is ");
		if (locale.AMPMTime) printmsg("12 hours\n"); else printmsg("24 hours\n");
		printmsg("Date format is ");
		switch (locale.DateFormat) {
			case GSM_Date_DDMMYYYY:printmsg("DD MM YYYY");break;
			case GSM_Date_MMDDYYYY:printmsg("MM DD YYYY");break;
			case GSM_Date_YYYYMMDD:printmsg("YYYY MM DD");
			default               :break;
		}
		printmsg(", date separator is %c\n",locale.DateSeparator);
	}

	GSM_Terminate();
}

static void SetDateTime(int argc, char *argv[])
{
	GSM_DateTime date_time;

	GSM_GetCurrentDateTime(&date_time);

	GSM_Init(true);

	error=Phone->SetDateTime(&s, &date_time);
	Print_Error(error);

	GSM_Terminate();
}

static void GetAlarm(int argc, char *argv[])
{
	GSM_Alarm alarm;

	GSM_Init(true);

	alarm.Location = 1;
	error=Phone->GetAlarm(&s, &alarm);
	switch (error) {
	case ERR_EMPTY:
		printmsg("Alarm not set in phone\n");
		break;
	case ERR_NONE:
		if (alarm.Repeating) {
			printmsg("Date: %s\n","Every day");
		} else {
			printmsg("Date: %s\n",OSDate(alarm.DateTime));
		}
		printmsg("Time: %02d:%02d\n",alarm.DateTime.Hour, alarm.DateTime.Minute);
		if (alarm.Text[0] != 0 || alarm.Text[1] != 0) {
			printmsg("Text: \"%s\"\n", DecodeUnicodeConsole(alarm.Text));
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

static void PrintMemoryEntry(GSM_MemoryEntry *entry)
{
	GSM_Category		Category;
	bool			unknown;
	int			z;

	for (i=0;i<entry->EntriesNum;i++) {
		unknown = false;
		switch (entry->Entries[i].EntryType) {
			case PBK_Date:
				printmsg("Date and time    : %s\n",OSDateTime(entry->Entries[i].Date,false));
				continue;
			case PBK_Category:
				if (entry->Entries[i].Number == -1) {
					printmsg("Category         : \"%s\"\n", DecodeUnicodeConsole(entry->Entries[i].Text));
				} else {
					Category.Location = entry->Entries[i].Number;
					Category.Type = Category_Phonebook;
					error=Phone->GetCategory(&s, &Category);
					if (error == ERR_NONE) {
						printmsg("Category         : \"%s\" (%i)\n", DecodeUnicodeConsole(Category.Name), entry->Entries[i].Number);
					} else {
						printmsg("Category         : %i\n", entry->Entries[i].Number);
					}
				}
				continue;
			case PBK_Private:
				printmsg("Private          : %s\n", entry->Entries[i].Number == 1 ? "Yes" : "No");
				continue;
			case PBK_Number_General     : printmsg("General number  "); break;
			case PBK_Number_Mobile      : printmsg("Mobile number   "); break;
			case PBK_Number_Work        : printmsg("Work number     "); break;
			case PBK_Number_Fax         : printmsg("Fax number      "); break;
			case PBK_Number_Home        : printmsg("Home number     "); break;
			case PBK_Number_Pager       : printmsg("Pager number    "); break;
			case PBK_Number_Other       : printmsg("Other number    "); break;
			case PBK_Text_Note          : printmsg("Text            "); break;
			case PBK_Text_Postal        : printmsg("Snail address   "); break;
			case PBK_Text_Email         : printmsg("Email address 1 "); break;
			case PBK_Text_Email2        : printmsg("Email address 2 "); break;
			case PBK_Text_URL           : printmsg("URL address     "); break;
			case PBK_Text_Name          : printmsg("Name            "); break;
			case PBK_Text_LastName      : printmsg("Last name       "); break;
			case PBK_Text_FirstName     : printmsg("First name      "); break;
			case PBK_Text_Company       : printmsg("Company         "); break;
			case PBK_Text_JobTitle      : printmsg("Job title       "); break;
			case PBK_Text_StreetAddress : printmsg("Street address  "); break;
			case PBK_Text_City          : printmsg("City            "); break;
			case PBK_Text_State         : printmsg("State           "); break;
			case PBK_Text_Zip           : printmsg("Zip code        "); break;
			case PBK_Text_Country       : printmsg("Country         "); break;
			case PBK_Text_Custom1       : printmsg("Custom text 1   "); break;
			case PBK_Text_Custom2       : printmsg("Custom text 2   "); break;
			case PBK_Text_Custom3       : printmsg("Custom text 3   "); break;
			case PBK_Text_Custom4       : printmsg("Custom text 4   "); break;
			case PBK_Caller_Group       :
				unknown = true;
				if (!callerinit[entry->Entries[i].Number]) {
					caller[entry->Entries[i].Number].Type	  = GSM_CallerGroupLogo;
					caller[entry->Entries[i].Number].Location = entry->Entries[i].Number;
					error=Phone->GetBitmap(&s,&caller[entry->Entries[i].Number]);
					Print_Error(error);
					if (caller[entry->Entries[i].Number].DefaultName) {
						NOKIA_GetDefaultCallerGroupName(&s,&caller[entry->Entries[i].Number]);
					}
					callerinit[entry->Entries[i].Number]=true;
				}
				printmsg("Caller group     : \"%s\"\n",DecodeUnicodeConsole(caller[entry->Entries[i].Number].Text));
				break;
			case PBK_RingtoneID	     :
				unknown = true;
				if (!ringinit) {
					error=Phone->GetRingtonesInfo(&s,&Info);
					if (error != ERR_NOTSUPPORTED) Print_Error(error);
					if (error == ERR_NONE) ringinit = true;
				}
				if (ringinit) {
					for (z=0;z<Info.Number;z++) {
						if (Info.Ringtone[z].ID == entry->Entries[i].Number) {
							printmsg("Ringtone         : \"%s\"\n",DecodeUnicodeConsole(Info.Ringtone[z].Name));
							break;
						}
					}
				} else {
					printmsg("Ringtone ID      : %i\n",entry->Entries[i].Number);
				}
				break;
			case PBK_Text_UserID:
				unknown = true;
				printmsg("User ID          : %s\n",DecodeUnicodeString(entry->Entries[i].Text));
				break;
			case PBK_PictureID	     :
				unknown = true;
				printmsg("Picture ID       : 0x%x\n",entry->Entries[i].Number);
				break;
			default		       :
				printmsg("UNKNOWN\n");
				unknown = true;
				break;
		}
		if (!unknown) printmsg(" : \"%s\"\n", DecodeUnicodeConsole(entry->Entries[i].Text));
	}
	printf("\n");
}

static void GetAllMemory(int argc, char *argv[])
{
	GSM_MemoryEntry		Entry;
	bool			start = true;

	signal(SIGINT, interrupt);
	printmsgerr("Press Ctrl+C to break...\n");

	Entry.MemoryType = 0;

	if (mystrncasecmp(argv[2],"DC",0)) Entry.MemoryType=MEM_DC;
	if (mystrncasecmp(argv[2],"ON",0)) Entry.MemoryType=MEM_ON;
	if (mystrncasecmp(argv[2],"RC",0)) Entry.MemoryType=MEM_RC;
	if (mystrncasecmp(argv[2],"MC",0)) Entry.MemoryType=MEM_MC;
	if (mystrncasecmp(argv[2],"ME",0)) Entry.MemoryType=MEM_ME;
	if (mystrncasecmp(argv[2],"SM",0)) Entry.MemoryType=MEM_SM;
	if (mystrncasecmp(argv[2],"VM",0)) Entry.MemoryType=MEM_VM;
	if (mystrncasecmp(argv[2],"FD",0)) Entry.MemoryType=MEM_FD;
	if (Entry.MemoryType==0) {
		printmsg("ERROR: unknown memory type (\"%s\")\n",argv[2]);
		exit (-1);
	}

	GSM_Init(true);

	while (!gshutdown) {
		error = Phone->GetNextMemory(&s, &Entry, start);
		if (error == ERR_EMPTY) break;
		if (error != ERR_NONE && Info.Ringtone) free(Info.Ringtone);
		Print_Error(error);
		printmsg("Memory %s, Location %i\n",argv[2],Entry.Location);
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

	if (mystrncasecmp(argv[2],"DC",0)) entry.MemoryType=MEM_DC;
	if (mystrncasecmp(argv[2],"ON",0)) entry.MemoryType=MEM_ON;
	if (mystrncasecmp(argv[2],"RC",0)) entry.MemoryType=MEM_RC;
	if (mystrncasecmp(argv[2],"MC",0)) entry.MemoryType=MEM_MC;
	if (mystrncasecmp(argv[2],"ME",0)) entry.MemoryType=MEM_ME;
	if (mystrncasecmp(argv[2],"SM",0)) entry.MemoryType=MEM_SM;
	if (mystrncasecmp(argv[2],"VM",0)) entry.MemoryType=MEM_VM;
	if (mystrncasecmp(argv[2],"FD",0)) entry.MemoryType=MEM_FD;
	if (entry.MemoryType==0) {
		printmsg("ERROR: unknown memory type (\"%s\")\n",argv[2]);
		exit (-1);
	}

	GetStartStop(&start, &stop, 3, argc, argv);

	if (argc > 5 && strcmp(argv[5],"")) {
		if (mystrncasecmp(argv[5],"-nonempty",0)) {
			empty = false;
		} else {
			printmsg("ERROR: unknown parameter \"%s\"\n",argv[5]);
			exit (-1);
		}
	}

	GSM_Init(true);

	if (!strcmp(s.Phone.Data.ModelInfo->model,"3310")) {
		if (s.Phone.Data.VerNum<=4.06) printmsg("WARNING: you will have null names in entries. Upgrade firmware in phone to higher than 4.06\n");
	}

	for (j=start;j<=stop;j++) {
		if (empty) printmsg("Memory %s, Location %i\n",argv[2],j);

		entry.Location=j;

		error=Phone->GetMemory(&s, &entry);
		if (error != ERR_EMPTY) {
			if (Info.Ringtone) free(Info.Ringtone);
			Print_Error(error);
		}

		if (error == ERR_EMPTY) {
			emptynum++;
			if (empty) {
				printmsg("Entry is empty\n");
				printf("\n");
			}
		} else {
			fillednum++;
			if (!empty) printmsg("Memory %s, Location %i\n",argv[2],j);
			PrintMemoryEntry(&entry);
		}
	}

	printmsg("%i entries empty, %i entries filled\n",emptynum,fillednum);

 	if (Info.Ringtone) free(Info.Ringtone);

	GSM_Terminate();
}

#define MemoryLocationToString(x) ( \
	x == MEM_ON ? "ON" :			\
	x == MEM_RC ? "RC" :			\
	x == MEM_MC ? "MC" :			\
	x == MEM_ME ? "ME" :			\
	x == MEM_SM ? "SM" :			\
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
			case PBK_Caller_Group       :
				if (mywstrstr(Entry->Entries[i].Text, Text) != NULL) {
					fprintf(stderr,"\n");
					printmsg("Memory %s, Location %i\n",MemoryLocationToString(Entry->MemoryType),Entry->Location);
					PrintMemoryEntry(Entry);
					return;
				}
				break;
			default:
				break;
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
	printmsgerr("Press Ctrl+C to break...\n");

	Length = strlen(argv[2]);
	if (Length > GSM_PHONEBOOK_TEXT_LENGTH) {
		printmsg("Search text too long, truncating to %d chars!\n", GSM_PHONEBOOK_TEXT_LENGTH);
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
	if (!gshutdown) SearchOneMemory(MEM_FD, "Fix dialling", 		Text);
	if (!gshutdown) SearchOneMemory(MEM_VM, "Voice mailbox", 	Text);

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
				printmsg("Memory %s, Location %i\n",MemoryLocationToString(Entry.MemoryType),Entry.Location);
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
	printmsgerr("Press Ctrl+C to break...\n");

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
			printmsg("Search text too long, truncating to %d chars!\n", GSM_MAX_CATEGORY_NAME_LENGTH);
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

static void displaysinglesmsinfo(GSM_SMSMessage sms, bool displaytext, bool displayudh)
{
	switch (sms.PDU) {
	case SMS_Status_Report:
		printmsg("SMS status report\n");
		printmsg("Status          : ");
		switch (sms.State) {
			case SMS_Sent	: printmsg("Sent");	break;
			case SMS_Read	: printmsg("Read");	break;
			case SMS_UnRead	: printmsg("UnRead");	break;
			case SMS_UnSent	: printmsg("UnSent");	break;
		}
		printmsg("\nRemote number   : \"%s\"\n",DecodeUnicodeConsole(sms.Number));
		printmsg("Reference number: %d\n",sms.MessageReference);
		printmsg("Sent            : %s\n",OSDateTime(sms.DateTime,true));
		printmsg("SMSC number     : \"%s\"\n",DecodeUnicodeConsole(sms.SMSC.Number));
		printmsg("SMSC response   : %s\n",OSDateTime(sms.SMSCTime,true));
		printmsg("Delivery status : %s\n",DecodeUnicodeConsole(sms.Text));
		printmsg("Details         : ");
		if (sms.DeliveryStatus & 0x40) {
			if (sms.DeliveryStatus & 0x20) {
				printmsg("Temporary error, ");
			} else {
	     			printmsg("Permanent error, ");
			}
	    	} else if (sms.DeliveryStatus & 0x20) {
			printmsg("Temporary error, ");
		}
		switch (sms.DeliveryStatus) {
			case 0x00: printmsg("SM received by the SME");				break;
			case 0x01: printmsg("SM forwarded by the SC to the SME but the SC is unable to confirm delivery");break;
			case 0x02: printmsg("SM replaced by the SC");				break;
			case 0x20: printmsg("Congestion");					break;
			case 0x21: printmsg("SME busy");					break;
			case 0x22: printmsg("No response from SME");				break;
			case 0x23: printmsg("Service rejected");				break;
			case 0x24: printmsg("Quality of service not aviable");			break;
			case 0x25: printmsg("Error in SME");					break;
		        case 0x40: printmsg("Remote procedure error");				break;
		        case 0x41: printmsg("Incompatibile destination");			break;
		        case 0x42: printmsg("Connection rejected by SME");			break;
		        case 0x43: printmsg("Not obtainable");					break;
		        case 0x44: printmsg("Quality of service not available");		break;
		        case 0x45: printmsg("No internetworking available");			break;
		        case 0x46: printmsg("SM Validity Period Expired");			break;
		        case 0x47: printmsg("SM deleted by originating SME");			break;
		        case 0x48: printmsg("SM Deleted by SC Administration");			break;
		        case 0x49: printmsg("SM does not exist");				break;
		        case 0x60: printmsg("Congestion");					break;
		        case 0x61: printmsg("SME busy");					break;
		        case 0x62: printmsg("No response from SME");				break;
		        case 0x63: printmsg("Service rejected");				break;
		        case 0x64: printmsg("Quality of service not available");		break;
		        case 0x65: printmsg("Error in SME");					break;
		        default  : printmsg("Reserved/Specific to SC: %x",sms.DeliveryStatus);	break;
		}
		printf("\n");
		break;
	case SMS_Deliver:
		printmsg("SMS message\n");
		if (sms.State==SMS_UnSent && sms.Memory==MEM_ME) {
			printmsg("Saved            : %s\n",OSDateTime(sms.DateTime,true));
		} else {
			printmsg("SMSC number      : \"%s\"",DecodeUnicodeConsole(sms.SMSC.Number));
			if (sms.ReplyViaSameSMSC) printmsg(" (set for reply)");
			printmsg("\nSent             : %s\n",OSDateTime(sms.DateTime,true));
		}
		/* No break. The only difference for SMS_Deliver and SMS_Submit is,
		 * that SMS_Deliver contains additional data. We wrote them and then go
		 * for data shared with SMS_Submit
		 */
	case SMS_Submit:
		if (sms.ReplaceMessage != 0) printmsg("SMS replacing ID : %i\n",sms.ReplaceMessage);
		/* If we went here from "case SMS_Deliver", we don't write "SMS Message" */
		if (sms.PDU==SMS_Submit) {
			printmsg("SMS message\n");
			if (sms.State==SMS_UnSent && sms.Memory==MEM_ME) {
			} else {
				printmsg("Reference number : %d\n",sms.MessageReference);
			}
		}
		if (sms.Name[0] != 0x00 || sms.Name[1] != 0x00) {
			printmsg("Name             : \"%s\"\n",DecodeUnicodeConsole(sms.Name));
		}
		if (sms.Class != -1) {
			printmsg("Class            : %i\n",sms.Class);
		}
		printmsg("Coding           : ");
		switch (sms.Coding) {
			case SMS_Coding_Unicode : printmsg("Unicode\n");		break;
			case SMS_Coding_Default : printmsg("Default GSM alphabet\n");	break;
			case SMS_Coding_8bit	: printmsg("8 bit\n");			break;
		}
		if (sms.State==SMS_UnSent && sms.Memory==MEM_ME) {
		} else {
			printmsg("Remote number    : \"%s\"\n",DecodeUnicodeConsole(sms.Number));
		}
		printmsg("Status           : ");
		switch (sms.State) {
			case SMS_Sent	:	printmsg("Sent\n");	break;
			case SMS_Read	:	printmsg("Read\n");	break;
			case SMS_UnRead	:	printmsg("UnRead\n");	break;
			case SMS_UnSent	:	printmsg("UnSent\n");	break;
		}
		if (sms.UDH.Type != UDH_NoUDH) {
			printmsg("User Data Header : ");
			switch (sms.UDH.Type) {
			case UDH_ConcatenatedMessages	   : printmsg("Concatenated (linked) message"); 	 break;
			case UDH_ConcatenatedMessages16bit : printmsg("Concatenated (linked) message"); 	 break;
			case UDH_DisableVoice		   : printmsg("Disables voice indicator");	 	 break;
			case UDH_EnableVoice		   : printmsg("Enables voice indicator");	 	 break;
			case UDH_DisableFax		   : printmsg("Disables fax indicator");	 	 break;
			case UDH_EnableFax		   : printmsg("Enables fax indicator");	 		 break;
			case UDH_DisableEmail		   : printmsg("Disables email indicator");	 	 break;
			case UDH_EnableEmail		   : printmsg("Enables email indicator");	 	 break;
			case UDH_VoidSMS		   : printmsg("Void SMS");			 	 break;
			case UDH_NokiaWAP		   : printmsg("Nokia WAP bookmark");		 	 break;
			case UDH_NokiaOperatorLogoLong	   : printmsg("Nokia operator logo");	 	 	 break;
			case UDH_NokiaWAPLong		   : printmsg("Nokia WAP bookmark or WAP/MMS settings"); break;
			case UDH_NokiaRingtone		   : printmsg("Nokia ringtone");		 	 break;
			case UDH_NokiaRingtoneLong	   : printmsg("Nokia ringtone");		 	 break;
			case UDH_NokiaOperatorLogo	   : printmsg("Nokia GSM operator logo");	 	 break;
			case UDH_NokiaCallerLogo	   : printmsg("Nokia caller logo");		 	 break;
			case UDH_NokiaProfileLong	   : printmsg("Nokia profile");		 		 break;
			case UDH_NokiaCalendarLong	   : printmsg("Nokia calendar note");	 		 break;
			case UDH_NokiaPhonebookLong	   : printmsg("Nokia phonebook entry");	 		 break;
			case UDH_UserUDH		   : printmsg("User UDH");			 	 break;
			case UDH_MMSIndicatorLong	   : printmsg("MMS indicator");			 	 break;
			case UDH_NoUDH:								 		 break;
			}
			if (sms.UDH.Type != UDH_NoUDH) {
				if (sms.UDH.ID8bit != -1) printmsg(", ID (8 bit) %i",sms.UDH.ID8bit);
				if (sms.UDH.ID16bit != -1) printmsg(", ID (16 bit) %i",sms.UDH.ID16bit);
				if (sms.UDH.PartNumber != -1 && sms.UDH.AllParts != -1) {
					if (displayudh) {
						printmsg(", part %i of %i",sms.UDH.PartNumber,sms.UDH.AllParts);
					} else {
						printmsg(", %i parts",sms.UDH.AllParts);
					}
				}
			}
			printf("\n");
		}
		if (displaytext) {
			printf("\n");
			if (sms.Coding!=SMS_Coding_8bit) {
				printmsg("%s\n",DecodeUnicodeConsole(sms.Text));
			} else {
				printmsg("8 bit SMS, cannot be displayed here\n");
			}
		}
		break;
	}
}

static void displaymultismsinfo (GSM_MultiSMSMessage sms, bool eachsms, bool ems)
{
	GSM_MultiPartSMSInfo	SMSInfo;
	bool				RetVal,udhinfo=true;
	int				j;

	/* GSM_DecodeMultiPartSMS returns if decoded SMS contenst correctly */
	RetVal = GSM_DecodeMultiPartSMS(&SMSInfo,&sms,ems);

	if (eachsms) {
		if (sms.SMS[0].UDH.Type != UDH_NoUDH && sms.SMS[0].UDH.AllParts == sms.Number) udhinfo = false;
		if (RetVal && !udhinfo) {
			displaysinglesmsinfo(sms.SMS[0],false,false);
			printf("\n");
		} else {
			for (j=0;j<sms.Number;j++) {
				displaysinglesmsinfo(sms.SMS[j],!RetVal,udhinfo);
				printf("\n");
			}
		}
	} else {
		for (j=0;j<sms.Number;j++) {
			displaysinglesmsinfo(sms.SMS[j],!RetVal,true);
			printf("\n");
		}
	}
	if (!RetVal) {
		GSM_FreeMultiPartSMSInfo(&SMSInfo);
		return;
	}

	if (SMSInfo.Unknown) printmsg("Some details were ignored (unknown or not implemented in decoding functions)\n\n");

	for (i=0;i<SMSInfo.EntriesNum;i++) {
		switch (SMSInfo.Entries[i].ID) {
		case SMS_NokiaRingtone:
			printmsg("Ringtone \"%s\"\n",DecodeUnicodeConsole(SMSInfo.Entries[i].Ringtone->Name));
			saverttl(stdout,SMSInfo.Entries[i].Ringtone);
			printf("\n");
			if (s.Phone.Functions->PlayTone!=NOTSUPPORTED &&
			    s.Phone.Functions->PlayTone!=NOTIMPLEMENTED) {
				if (answer_yes("Do you want to play it")) GSM_PlayRingtone(*SMSInfo.Entries[i].Ringtone);
			}
			break;
		case SMS_NokiaCallerLogo:
			printmsg("Caller logo\n\n");
			GSM_PrintBitmap(stdout,&SMSInfo.Entries[i].Bitmap->Bitmap[0]);
			break;
		case SMS_NokiaOperatorLogo:
			printmsg("Operator logo for %s network (%s, %s)\n\n",
				SMSInfo.Entries[i].Bitmap->Bitmap[0].NetworkCode,
				DecodeUnicodeConsole(GSM_GetNetworkName(SMSInfo.Entries[i].Bitmap->Bitmap[0].NetworkCode)),
				DecodeUnicodeConsole(GSM_GetCountryName(SMSInfo.Entries[i].Bitmap->Bitmap[0].NetworkCode)));
			GSM_PrintBitmap(stdout,&SMSInfo.Entries[i].Bitmap->Bitmap[0]);
			break;
		case SMS_NokiaScreenSaverLong:
			printmsg("Screen saver\n");
			GSM_PrintBitmap(stdout,&SMSInfo.Entries[i].Bitmap->Bitmap[0]);
			break;
		case SMS_NokiaPictureImageLong:
			printmsg("Picture Image\n");
			if (UnicodeLength(SMSInfo.Entries[i].Bitmap->Bitmap[0].Text)!=0) printmsg("Text: \"%s\"\n\n",DecodeUnicodeConsole(SMSInfo.Entries[i].Bitmap->Bitmap[0].Text));
			GSM_PrintBitmap(stdout,&SMSInfo.Entries[i].Bitmap->Bitmap[0]);
			break;
		case SMS_NokiaProfileLong:
			printmsg("Profile\n");
			GSM_PrintBitmap(stdout,&SMSInfo.Entries[i].Bitmap->Bitmap[0]);
			break;
		case SMS_ConcatenatedTextLong:
		case SMS_ConcatenatedAutoTextLong:
		case SMS_ConcatenatedTextLong16bit:
		case SMS_ConcatenatedAutoTextLong16bit:
			printmsg("%s\n",DecodeUnicodeConsole(SMSInfo.Entries[i].Buffer));
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
			printmsg("\nEMS sound ID: %i\n",SMSInfo.Entries[i].Number);
			break;
		case SMS_EMSPredefinedAnimation:
			printmsg("\nEMS animation ID: %i\n",SMSInfo.Entries[i].Number);
			break;
		default:
			printf("Error\n");
			break;
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
		printmsg("Network state     : ");
		switch (NetInfo.State) {
			case GSM_HomeNetwork		: printmsg("home network\n");			break;
			case GSM_RoamingNetwork		: printmsg("roaming network\n");		break;
			case GSM_RequestingNetwork	: printmsg("requesting network\n");		break;
			case GSM_NoNetwork		: printmsg("not logged into network\n");	break;
			case GSM_RegistrationDenied	: printmsg("registration to network denied\n");	break;
			case GSM_NetworkStatusUnknown	: printmsg("unknown\n");			break;
			default				: printmsg("unknown\n");
		}
		if (NetInfo.State == GSM_HomeNetwork || NetInfo.State == GSM_RoamingNetwork) {
			printmsg("Network           : %s (%s",	NetInfo.NetworkCode,DecodeUnicodeConsole(GSM_GetNetworkName(NetInfo.NetworkCode)));
			printmsg(", %s)",				DecodeUnicodeConsole(GSM_GetCountryName(NetInfo.NetworkCode)));
			printmsg(", LAC %s, CellID %s\n",		NetInfo.LAC,NetInfo.CID);
			if (NetInfo.NetworkName[0] != 0x00 || NetInfo.NetworkName[1] != 0x00) {
				printmsg("Name in phone     : \"%s\"\n",DecodeUnicodeConsole(NetInfo.NetworkName));
			}
		}
	}
	GSM_Terminate();
}

static void IncomingSMS(char *Device, GSM_SMSMessage sms)
{
	printmsg("SMS message received\n");
 	if (wasincomingsms) {
 		printmsg("We already have one pending, ignoring!\n");
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
 			printmsg("Location %i\n",IncomingSMSData.SMS[0].Location);
 			printmsg("Empty\n");
 			break;
 		default:
 			Print_Error(error);
 			printmsg("Location %i, folder \"%s\"",IncomingSMSData.SMS[0].Location,DecodeUnicodeConsole(folders.Folder[IncomingSMSData.SMS[0].Folder-1].Name));
 			switch(IncomingSMSData.SMS[0].Memory) {
 				case MEM_SM: printmsg(", SIM memory"); 		break;
 				case MEM_ME: printmsg(", phone memory"); 	break;
 				case MEM_MT: printmsg(", phone or SIM memory"); break;
 				default    : break;
 			}
 			if (IncomingSMSData.SMS[0].InboxFolder) printmsg(", Inbox folder");
 			printf("\n");
 		}
 	}
 	displaymultismsinfo(IncomingSMSData,false,false);
 	wasincomingsms = false;
}

static void IncomingCB(char *Device, GSM_CBMessage CB)
{
	printmsg("CB message received\n");
	printmsg("Channel %i, text \"%s\"\n",CB.Channel,DecodeUnicodeConsole(CB.Text));
}

static void IncomingCall(char *Device, GSM_Call call)
{
	printmsg("Call info : ");
	if (call.CallIDAvailable) printmsg("ID %i, ",call.CallID);
	switch(call.Status) {
		case GSM_CALL_IncomingCall  	: printmsg("incoming call from \"%s\"\n",DecodeUnicodeConsole(call.PhoneNumber));  	break;
		case GSM_CALL_OutgoingCall  	: printmsg("outgoing call to \"%s\"\n",DecodeUnicodeConsole(call.PhoneNumber));    	break;
		case GSM_CALL_CallStart     	: printmsg("call started\n"); 					  	  		break;
		case GSM_CALL_CallEnd	   	: printmsg("end of call (unknown side)\n"); 					  	break;
		case GSM_CALL_CallLocalEnd  	: printmsg("call end from our side\n");						  	break;
		case GSM_CALL_CallRemoteEnd 	: printmsg("call end from remote side (code %i)\n",call.StatusCode);			break;
		case GSM_CALL_CallEstablished   : printmsg("call established. Waiting for answer\n");				  	break;
		case GSM_CALL_CallHeld		: printmsg("call held\n");								break;
		case GSM_CALL_CallResumed	: printmsg("call resumed\n");								break;
		case GSM_CALL_CallSwitched	: printmsg("call switched\n");								break;
	}
}

static void IncomingUSSD(char *Device, char *Buffer)
{
	printmsg("Service reply: \"%s\"\n",DecodeUnicodeConsole(Buffer));
}

#define CHECKMEMORYSTATUS(x, m, a1, b1) { 				\
	x.MemoryType=m;							\
	if (Phone->GetMemoryStatus(&s, &x) == ERR_NONE)			\
		printmsg("%s %03d, %s %03d\n", a1, x.MemoryUsed, b1, x.MemoryFree);	\
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
	int 			count = -1;

	if (argc >= 3) {
		count = atoi(argv[2]);
		if (count <= 0) count = -1;
	}

	signal(SIGINT, interrupt);
	printmsgerr("Press Ctrl+C to break...\n");
	printmsg("Entering monitor mode...\n\n");

	GSM_Init(true);

	s.User.IncomingSMS 	= IncomingSMS;
	s.User.IncomingCB 	= IncomingCB;
	s.User.IncomingCall 	= IncomingCall;
	s.User.IncomingUSSD 	= IncomingUSSD;

	error=Phone->SetIncomingSMS  		(&s,true);
	printmsg("Enabling info about incoming SMS : %s\n",print_error(error,NULL,s.msg));
	error=Phone->SetIncomingCB   		(&s,true);
	printmsg("Enabling info about incoming CB  : %s\n",print_error(error,NULL,s.msg));
	error=Phone->SetIncomingCall 		(&s,true);
	printmsg("Enabling info about calls        : %s\n",print_error(error,NULL,s.msg));
	error=Phone->SetIncomingUSSD 		(&s,true);
	printmsg("Enabling info about USSD         : %s\n\n",print_error(error,NULL,s.msg));

	while (!gshutdown && count != 0) {
		if (count > 0) count--;
		CHECKMEMORYSTATUS(MemStatus,MEM_SM,"SIM phonebook     : Used","Free");
		if (gshutdown) break;
		CHECKMEMORYSTATUS(MemStatus,MEM_DC,"Dialled numbers   : Used","Free");
		if (gshutdown) break;
		CHECKMEMORYSTATUS(MemStatus,MEM_RC,"Received numbers  : Used","Free");
		if (gshutdown) break;
		CHECKMEMORYSTATUS(MemStatus,MEM_MC,"Missed numbers    : Used","Free");
		if (gshutdown) break;
		CHECKMEMORYSTATUS(MemStatus,MEM_ON,"Own numbers       : Used","Free");
		if (gshutdown) break;
		CHECKMEMORYSTATUS(MemStatus,MEM_ME,"Phone phonebook   : Used","Free");
		if (gshutdown) break;
		if (Phone->GetToDoStatus(&s, &ToDoStatus) == ERR_NONE) {
			printmsg("ToDos             : Used %d\n", ToDoStatus.Used);
		}
		if (gshutdown) break;
		if (Phone->GetCalendarStatus(&s, &CalendarStatus) == ERR_NONE) {
			printmsg("Calendar          : Used %d\n", CalendarStatus.Used);
		}
		if (gshutdown) break;
		if (Phone->GetBatteryCharge(&s,&BatteryCharge)==ERR_NONE) {
            		if (BatteryCharge.BatteryPercent != -1) printmsg("Battery level     : %i percent\n", BatteryCharge.BatteryPercent);
            		if (BatteryCharge.ChargeState != 0) {
                		printmsg("Charge state      : ");
                		switch (BatteryCharge.ChargeState) {
                    			case GSM_BatteryPowered:
						printmsg("powered from battery");
						break;
                    			case GSM_BatteryConnected:
						printmsg("battery connected, but not powered from battery");
                        			break;
                    			case GSM_BatteryNotConnected:
                        			printmsg("battery not connected");
                        			break;
                    			case GSM_PowerFault:
                        			printmsg("detected power failure");
                        			break;
                    			default:
                        			printmsg("unknown");
                       				break;
                		}
                		printf("\n");
            		}
        	}
		if (gshutdown) break;
		if (Phone->GetSignalQuality(&s,&SignalQuality)==ERR_NONE) {
            		if (SignalQuality.SignalStrength != -1) printmsg("Signal strength   : %i dBm\n",     SignalQuality.SignalStrength);
            		if (SignalQuality.SignalPercent  != -1) printmsg("Network level     : %i percent\n", SignalQuality.SignalPercent);
            		if (SignalQuality.BitErrorRate   != -1) printmsg("Bit error rate    : %i percent\n", SignalQuality.BitErrorRate);
        	}
		if (gshutdown) break;
		if (Phone->GetSMSStatus(&s,&SMSStatus)==ERR_NONE) {
			if (SMSStatus.SIMSize > 0) {
				printmsg("SIM SMS status    : %i used, %i unread, %i locations\n",
					SMSStatus.SIMUsed,
					SMSStatus.SIMUnRead,
					SMSStatus.SIMSize);
			}

			if (SMSStatus.PhoneSize > 0) {
				printmsg("Phone SMS status  : %i used, %i unread, %i locations",
					SMSStatus.PhoneUsed,
					SMSStatus.PhoneUnRead,
					SMSStatus.PhoneSize);
				if (SMSStatus.TemplatesUsed!=0) printmsg(", %i templates", SMSStatus.TemplatesUsed);
				printf("\n");
			}
		}
		if (gshutdown) break;
		if (Phone->GetNetworkInfo(&s,&NetInfo)==ERR_NONE) {
			printmsg("Network state     : ");
                        switch (NetInfo.State) {
				case GSM_HomeNetwork		: printmsg("home network\n"); 		 	break;
				case GSM_RoamingNetwork		: printmsg("roaming network\n"); 	 	break;
				case GSM_RequestingNetwork	: printmsg("requesting network\n"); 	 	break;
				case GSM_NoNetwork		: printmsg("not logged into network\n"); 	break;
				case GSM_RegistrationDenied	: printmsg("registration to network denied\n");	break;
				case GSM_NetworkStatusUnknown	: printmsg("unknown\n");			break;
				default				: printmsg("unknown\n");
			}
			if (NetInfo.State == GSM_HomeNetwork || NetInfo.State == GSM_RoamingNetwork) {
				printmsg("Network           : %s (%s",	NetInfo.NetworkCode,DecodeUnicodeConsole(GSM_GetNetworkName(NetInfo.NetworkCode)));
				printmsg(", %s)",				DecodeUnicodeConsole(GSM_GetCountryName(NetInfo.NetworkCode)));
				printmsg(", LAC %s, CID %s\n",		NetInfo.LAC,NetInfo.CID);
				if (NetInfo.NetworkName[0] != 0x00 || NetInfo.NetworkName[1] != 0x00) {
					printmsg("Name in phone     : \"%s\"\n",DecodeUnicodeConsole(NetInfo.NetworkName));
				}
			}
		}
		if (wasincomingsms) DisplayIncomingSMS();
		printf("\n");
	}

	printmsg("Leaving monitor mode...\n");

	GSM_Terminate();
}

static void IncomingUSSD2(char *Device, char *Buffer)
{
	printmsg("Service reply: \"%s\"\n",DecodeUnicodeConsole(Buffer));

	gshutdown = true;
}

static void GetUSSD(int argc, char *argv[])
{
	GSM_Init(true);

	signal(SIGINT, interrupt);
	printmsgerr("Press Ctrl+C to break...\n");

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
			printmsg("%i. Set %i\n",smsc.Location, smsc.Location);
		} else {
			printmsg("%i. \"%s\"\n",smsc.Location, DecodeUnicodeConsole(smsc.Name));
		}
		printmsg("Number         : \"%s\"\n",DecodeUnicodeConsole(smsc.Number));
		printmsg("Default number : \"%s\"\n",DecodeUnicodeConsole(smsc.DefaultNumber));

		printmsg("Format         : ");
		switch (smsc.Format) {
			case SMS_FORMAT_Text	: printmsg("Text");	break;
			case SMS_FORMAT_Fax	: printmsg("Fax");	break;
			case SMS_FORMAT_Email	: printmsg("Email");	break;
			case SMS_FORMAT_Pager	: printmsg("Pager");	break;
		}
		printf("\n");

		printmsg("Validity       : ");
		switch (smsc.Validity.Relative) {
			case SMS_VALID_1_Hour	: printmsg("1 hour");	    break;
			case SMS_VALID_6_Hours 	: printmsg("6 hours");	    break;
			case SMS_VALID_1_Day	: printmsg("24 hours");	    break;
			case SMS_VALID_3_Days	: printmsg("72 hours");	    break;
			case SMS_VALID_1_Week  	: printmsg("1 week"); 	    break;
			case SMS_VALID_Max_Time	: printmsg("Maximum time"); break;
			default           	:
				if (smsc.Validity.Relative >= 0 && smsc.Validity.Relative <= 143) {
					printmsg("%i minutes",(smsc.Validity.Relative+1)*5);
				} else if (smsc.Validity.Relative >= 144 && smsc.Validity.Relative <= 167) {
					printmsg("%i minutes",12*60 + (smsc.Validity.Relative-143)*30);
				} else if (smsc.Validity.Relative >= 168 && smsc.Validity.Relative <= 196) {
					printmsg("%i days",smsc.Validity.Relative-166);
				} else if (smsc.Validity.Relative >= 197 && smsc.Validity.Relative <= 255) {
					printmsg("%i weeks",smsc.Validity.Relative-192);
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
			printmsg("Location %i\n",sms.SMS[0].Location);
			printmsg("Empty\n");
			break;
		default:
			Print_Error(error);
			printmsg("Location %i, folder \"%s\"",sms.SMS[0].Location,DecodeUnicodeConsole(folders.Folder[sms.SMS[0].Folder-1].Name));
			switch(sms.SMS[0].Memory) {
				case MEM_SM: printmsg(", SIM memory"); 		break;
				case MEM_ME: printmsg(", phone memory"); 	break;
				case MEM_MT: printmsg(", phone or SIM memory"); break;
				default    : break;
			}
			if (sms.SMS[0].InboxFolder) printmsg(", Inbox folder");
			printf("\n");
			displaymultismsinfo(sms,false,false);
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
		sms.Folder	= 0;
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

	GSM_Init(true);

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
			printmsg("Location %i, folder \"%s\"",sms.SMS[0].Location,DecodeUnicodeConsole(folders.Folder[sms.SMS[0].Folder-1].Name));
			switch(sms.SMS[0].Memory) {
				case MEM_SM: printmsg(", SIM memory"); 		break;
				case MEM_ME: printmsg(", phone memory"); 	break;
				case MEM_MT: printmsg(", phone or SIM memory"); break;
				default    : break;
			}
			if (sms.SMS[0].InboxFolder) printmsg(", Inbox folder");
			printf("\n");
			displaymultismsinfo(sms,false,false);
		}
		start=false;
	}
	fprintf(stderr,"\n");

#ifdef GSM_ENABLE_BEEP
	GSM_PhoneBeep();
#endif
	GSM_Terminate();
}

static void GetEachSMS(int argc, char *argv[])
{
	GSM_MultiSMSMessage	*GetSMS[PHONE_MAXSMSINFOLDER],*SortedSMS[PHONE_MAXSMSINFOLDER],sms;
	int			GetSMSNumber = 0,i,j;
	GSM_SMSFolders		folders;
	bool			start = true, ems = true;

	GetSMS[0] = NULL;

	GSM_Init(true);

	error=Phone->GetSMSFolders(&s, &folders);
	Print_Error(error);

	fprintf(stderr,"Reading: ");
	while (error == ERR_NONE) {
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
			if (GetSMSNumber==PHONE_MAXSMSINFOLDER) {
				fprintf(stderr,"SMS counter overflow\n");
				return;
			}
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
		for (j=0;j<SortedSMS[i]->Number;j++) {
			if ((j==0) || (j!=0 && SortedSMS[i]->SMS[j].Location != SortedSMS[i]->SMS[j-1].Location)) {
				printmsg("Location %i, folder \"%s\"",SortedSMS[i]->SMS[j].Location,DecodeUnicodeConsole(folders.Folder[SortedSMS[i]->SMS[j].Folder-1].Name));
				switch(SortedSMS[i]->SMS[j].Memory) {
					case MEM_SM: printmsg(", SIM memory"); 		break;
					case MEM_ME: printmsg(", phone memory"); 	break;
					case MEM_MT: printmsg(", phone or SIM memory"); break;
					default    : break;
				}
				if (SortedSMS[i]->SMS[j].InboxFolder) printmsg(", Inbox folder");
				printf("\n");
			}
		}
		displaymultismsinfo(*SortedSMS[i],true,ems);

		free(SortedSMS[i]);
		SortedSMS[i] = NULL;
		i++;
	}

	GSM_Terminate();
}

static void GetSMSFolders(int argc, char *argv[])
{
	GSM_SMSFolders folders;

	GSM_Init(true);

	error=Phone->GetSMSFolders(&s,&folders);
	Print_Error(error);

	for (i=0;i<folders.Number;i++) {
		printmsg("%i. \"%30s\"",i+1,DecodeUnicodeConsole(folders.Folder[i].Name));
		switch(folders.Folder[i].Memory) {
			case MEM_SM: printmsg(", SIM memory"); 		break;
			case MEM_ME: printmsg(", phone memory"); 	break;
			case MEM_MT: printmsg(", phone or SIM memory"); break;
			default    : break;
		}
		if (folders.Folder[i].InboxFolder) printmsg(", Inbox folder");
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
		printmsg("%i. \"%30s\"",i+1,DecodeUnicodeConsole(folders.Folder[i].Name));
		if (folders.Folder[i].InboxFolder) printmsg(", Inbox folder");
		printf("\n");
	}

	GSM_Terminate();
}

static void GetRingtone(int argc, char *argv[])
{
	GSM_Ringtone 	ringtone;
	bool		PhoneRingtone = false;

	if (mystrncasecmp(argv[1],"--getphoneringtone",0)) PhoneRingtone = true;

	GetStartStop(&ringtone.Location, NULL, 2, argc, argv);

	GSM_Init(true);

	ringtone.Format=0;

	error=Phone->GetRingtone(&s,&ringtone,PhoneRingtone);
	Print_Error(error);

	switch (ringtone.Format) {
		case RING_NOTETONE	: printmsg("Smart Messaging");	break;
		case RING_NOKIABINARY	: printmsg("Nokia binary");	break;
		case RING_MIDI		: printmsg("MIDI");		break;
		case RING_MMF		: printmsg("SMAF (MMF)");	break;
	}
	printmsg(" format, ringtone \"%s\"\n",DecodeUnicodeConsole(ringtone.Name));

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

	for (i=0;i<Info.Number;i++) printmsg("%i. \"%s\"\n",i+1,DecodeUnicodeConsole(Info.Ringtone[i].Name));

 	if (Info.Ringtone) free(Info.Ringtone);
}

static void DialVoice(int argc, char *argv[])
{
	GSM_CallShowNumber ShowNumber = GSM_CALL_DefaultNumberPresence;

	if (argc > 3) {
		if (mystrncasecmp(argv[3],"show",0)) {		ShowNumber = GSM_CALL_ShowNumber;
		} else if (mystrncasecmp(argv[3],"hide",0)) {	ShowNumber = GSM_CALL_HideNumber;
		} else {
			printmsg("Unknown parameter (\"%s\")\n",argv[3]);
			exit(-1);
		}
	}

	GSM_Init(true);

	error=Phone->DialVoice(&s, argv[2], ShowNumber);
	Print_Error(error);

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

	if (mystrncasecmp(argv[2],"SOFT",0)) {		hard=false;
	} else if (mystrncasecmp(argv[2],"HARD",0)) {	hard=true;
	} else {
		printmsg("What type of reset do you want (\"%s\") ?\n",argv[2]);
		exit(-1);
	}

	GSM_Init(true);

	error=Phone->Reset(&s, hard);
	Print_Error(error);

	GSM_Terminate();
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
    	int 			repeat_weekofmonth 	= -1;
    	int 			repeat_month 		= -1;
    	int 			repeat_frequency 	= -1;
    	GSM_DateTime 		repeat_startdate 	= {0,0,0,0,0,0,0};
    	GSM_DateTime 		repeat_stopdate 	= {0,0,0,0,0,0,0};

	printmsg("Location     : %d\n", Note->Location);
	printmsg("Note type    : ");
	switch (Note->Type) {
		case GSM_CAL_REMINDER 	: printmsg("Reminder (Date)\n");		break;
		case GSM_CAL_CALL     	: printmsg("Call\n");			   	break;
		case GSM_CAL_MEETING  	: printmsg("Meeting\n");		   	break;
		case GSM_CAL_BIRTHDAY 	: printmsg("Birthday (Anniversary)\n");		break;
		case GSM_CAL_MEMO		: printmsg("Memo (Miscellaneous)\n");		break;
		case GSM_CAL_TRAVEL		: printmsg("Travel\n");			   	break;
		case GSM_CAL_VACATION	: printmsg("Vacation\n");			break;
		case GSM_CAL_ALARM    	: printmsg("Alarm\n");		   		break;
		case GSM_CAL_DAILY_ALARM 	: printmsg("Daily alarm\n");		   	break;
		case GSM_CAL_T_ATHL   	: printmsg("Training/Athletism\n"); 	   	break;
		case GSM_CAL_T_BALL   	: printmsg("Training/Ball Games\n"); 	   	break;
		case GSM_CAL_T_CYCL   	: printmsg("Training/Cycling\n"); 	   	break;
		case GSM_CAL_T_BUDO   	: printmsg("Training/Budo\n"); 	   		break;
		case GSM_CAL_T_DANC   	: printmsg("Training/Dance\n"); 	   	break;
		case GSM_CAL_T_EXTR   	: printmsg("Training/Extreme Sports\n"); 	break;
		case GSM_CAL_T_FOOT   	: printmsg("Training/Football\n"); 	   	break;
		case GSM_CAL_T_GOLF   	: printmsg("Training/Golf\n"); 	   		break;
		case GSM_CAL_T_GYM    	: printmsg("Training/Gym\n"); 	   		break;
		case GSM_CAL_T_HORS   	: printmsg("Training/Horse Races\n");    	break;
		case GSM_CAL_T_HOCK   	: printmsg("Training/Hockey\n"); 	  	break;
		case GSM_CAL_T_RACE   	: printmsg("Training/Races\n"); 	   	break;
		case GSM_CAL_T_RUGB   	: printmsg("Training/Rugby\n"); 	   	break;
		case GSM_CAL_T_SAIL   	: printmsg("Training/Sailing\n"); 	   	break;
		case GSM_CAL_T_STRE   	: printmsg("Training/Street Games\n");   	break;
		case GSM_CAL_T_SWIM   	: printmsg("Training/Swimming\n"); 	   	break;
		case GSM_CAL_T_TENN   	: printmsg("Training/Tennis\n"); 	   	break;
		case GSM_CAL_T_TRAV   	: printmsg("Training/Travels\n");        	break;
		case GSM_CAL_T_WINT   	: printmsg("Training/Winter Games\n");   	break;
		default           	: printmsg("UNKNOWN\n");
	}
	Alarm.Year = 0;

	repeating 		= false;
	repeat_dayofweek 	= -1;
	repeat_day 		= -1;
	repeat_weekofmonth 	= -1;
	repeat_month 		= -1;
	repeat_frequency 	= -1;
	repeat_startdate.Day	= 0;
	repeat_stopdate.Day 	= 0;

	for (i=0;i<Note->EntriesNum;i++) {
		switch (Note->Entries[i].EntryType) {
		case CAL_START_DATETIME:
			printmsg("Start        : %s\n",OSDateTime(Note->Entries[i].Date,false));
			memcpy(&DateTime,&Note->Entries[i].Date,sizeof(GSM_DateTime));
			break;
		case CAL_END_DATETIME:
			printmsg("Stop         : %s\n",OSDateTime(Note->Entries[i].Date,false));
			memcpy(&DateTime,&Note->Entries[i].Date,sizeof(GSM_DateTime));
			break;
		case CAL_ALARM_DATETIME:
			printmsg("Tone alarm   : %s\n",OSDateTime(Note->Entries[i].Date,false));
			memcpy(&Alarm,&Note->Entries[i].Date,sizeof(GSM_DateTime));
			break;
		case CAL_SILENT_ALARM_DATETIME:
			printmsg("Silent alarm : %s\n",OSDateTime(Note->Entries[i].Date,false));
			memcpy(&Alarm,&Note->Entries[i].Date,sizeof(GSM_DateTime));
			break;
		case CAL_TEXT:
			printmsg("Text         : \"%s\"\n",DecodeUnicodeConsole(Note->Entries[i].Text));
			break;
		case CAL_LOCATION:
			printmsg("Location     : \"%s\"\n",DecodeUnicodeConsole(Note->Entries[i].Text));
			break;
		case CAL_PHONE:
			printmsg("Phone        : \"%s\"\n",DecodeUnicodeConsole(Note->Entries[i].Text));
			break;
		case CAL_PRIVATE:
			printmsg("Private      : %s\n",Note->Entries[i].Number == 1 ? "Yes" : "No");
			break;
		case CAL_CONTACTID:
			entry.Location = Note->Entries[i].Number;
			entry.MemoryType = MEM_ME;
			error=Phone->GetMemory(&s, &entry);
			if (error == ERR_NONE) {
				name = GSM_PhonebookGetEntryName(&entry);
				if (name != NULL) {
					printmsg("Contact ID   : \"%s\" (%d)\n", DecodeUnicodeConsole(name), Note->Entries[i].Number);
				} else {
					printmsg("Contact ID   : %d\n",Note->Entries[i].Number);
				}
			} else {
				printmsg("Contact ID   : %d\n",Note->Entries[i].Number);
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
		printmsg("Repeating    : ");
		if ((repeat_startdate.Day == 0) && (repeat_stopdate.Day == 0)) {
			printmsg("Forever");
		} else if (repeat_startdate.Day == 0) {
			printmsg("Till %s", OSDate(repeat_stopdate));
		} else if (repeat_stopdate.Day == 0) {
			printmsg("Since %s", OSDate(repeat_startdate));
		} else {
			printmsg("Since %s till %s", OSDate(repeat_startdate), OSDate(repeat_stopdate));
		}
		if (repeat_frequency != -1) {
			if (repeat_frequency == 1) {
				printmsg (" on each ");
			} else {
				printmsg(" on each %d. ", repeat_frequency);
			}
			if (repeat_dayofweek > 0) {
				switch (repeat_dayofweek) {
					case 1 : printmsg("Monday"); 	break;
					case 2 : printmsg("Tuesday"); 	break;
					case 3 : printmsg("Wednesday"); break;
					case 4 : printmsg("Thursday"); 	break;
					case 5 : printmsg("Friday"); 	break;
					case 6 : printmsg("Saturday"); 	break;
					case 7 : printmsg("Sunday"); 	break;
					default: printmsg("Bad day!"); 	break;
				}
				if (repeat_weekofmonth > 0) {
					printmsg(" in %d. week of ", repeat_weekofmonth);
				} else {
					printmsg(" in ");
				}
				if (repeat_month > 0) {
					switch(repeat_month) {
						case 1 : printmsg("January"); 	 break;
						case 2 : printmsg("February"); 	 break;
						case 3 : printmsg("March"); 	 break;
						case 4 : printmsg("April"); 	 break;
						case 5 : printmsg("May"); 	 break;
						case 6 : printmsg("June"); 	 break;
						case 7 : printmsg("July"); 	 break;
						case 8 : printmsg("August"); 	 break;
						case 9 : printmsg("September");  break;
						case 10: printmsg("October"); 	 break;
						case 11: printmsg("November"); 	 break;
						case 12: printmsg("December"); 	 break;
						default: printmsg("Bad month!"); break;
					}
				} else {
					printmsg("each month");
				}
			} else if (repeat_day > 0) {
				printmsg("%d. day of ", repeat_day);
				if (repeat_month > 0) {
					switch(repeat_month) {
						case 1 : printmsg("January"); 	break;
						case 2 : printmsg("February"); 	break;
						case 3 : printmsg("March");	break;
						case 4 : printmsg("April"); 	break;
						case 5 : printmsg("May"); 	break;
						case 6 : printmsg("June"); 	break;
						case 7 : printmsg("July"); 	break;
						case 8 : printmsg("August"); 	break;
						case 9 : printmsg("September"); break;
						case 10: printmsg("October"); 	break;
						case 11: printmsg("November"); 	break;
						case 12: printmsg("December"); 	break;
						default: printmsg("Bad month!");break;
					}
				} else {
					printmsg("each month");
				}
			} else {
				printmsg("day");
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
			printmsg("Age          : %d %s\n",i_age, (i_age==1)?"year":"years");
		}
	}
	printf("\n");
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
		if (error == ERR_EMPTY) continue;
		Print_Error(error);
		PrintCalendar(&Note);
	}

	GSM_Terminate();
}


static void GetAllCalendar(int argc, char *argv[])
{
	GSM_CalendarEntry	Note;
	bool			refresh	= true;

	signal(SIGINT, interrupt);
	printmsgerr("Press Ctrl+C to break...\n");

	GSM_Init(true);

	while (!gshutdown) {
		error=Phone->GetNextCalendar(&s,&Note,refresh);
		if (error == ERR_EMPTY) break;
		Print_Error(error);
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
		printmsg("Auto deleting disabled");
	} else {
		printmsg("Auto deleting notes after %i day(s)",settings.AutoDelete);
	}
	printmsg("\nWeek start on ");
	switch(settings.StartDay) {
		case 1: printmsg("Monday"); 	break;
		case 6: printmsg("Saturday"); 	break;
		case 7: printmsg("Sunday"); 	break;
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
		printmsg("Name    : \"%s\"\n",DecodeUnicodeConsole(bookmark.Title));
		printmsg("Address : \"%s\"\n",DecodeUnicodeConsole(bookmark.Address));
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
			printmsg("%i. \"%s\"",point.Location,DecodeUnicodeConsole(point.Name));
		} else {
			printmsg("%i. Access point %i",point.Location,point.Location);
		}
		if (point.Active) printmsg(" (active)");
		if (error != ERR_EMPTY) {
			printmsg("\nAddress : \"%s\"\n\n",DecodeUnicodeConsole(point.URL));
		} else {
			printmsg("\n\n");
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

	if (mystrncasecmp(argv[2],"STARTUP",0)) {
		MultiBitmap.Bitmap[0].Type=GSM_StartupLogo;
	} else if (mystrncasecmp(argv[2],"CALLER",0)) {
		MultiBitmap.Bitmap[0].Type=GSM_CallerGroupLogo;
		GetStartStop(&location, NULL, 3, argc, argv);
		if (location>5) {
			printmsg("Maximal location for caller logo can be 5\n");
			exit (-1);
		}
	} else if (mystrncasecmp(argv[2],"PICTURE",0)) {
		MultiBitmap.Bitmap[0].Type=GSM_PictureImage;
		GetStartStop(&location, NULL, 3, argc, argv);
	} else if (mystrncasecmp(argv[2],"TEXT",0)) {
		MultiBitmap.Bitmap[0].Type=GSM_WelcomeNote_Text;
	} else if (mystrncasecmp(argv[2],"DEALER",0)) {
		MultiBitmap.Bitmap[0].Type=GSM_DealerNote_Text;
	} else if (mystrncasecmp(argv[2],"OPERATOR",0)) {
		MultiBitmap.Bitmap[0].Type=GSM_OperatorLogo;
	} else {
		printmsg("What type of logo do you want to get (\"%s\") ?\n",argv[2]);
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
		printmsg("Group name  : \"%s\"",DecodeUnicodeConsole(MultiBitmap.Bitmap[0].Text));
		if (MultiBitmap.Bitmap[0].DefaultName) printmsg(" (default)");
		printf("\n");
		if (MultiBitmap.Bitmap[0].DefaultRingtone) {
			printmsg("Ringtone    : default\n");
		} else if (MultiBitmap.Bitmap[0].FileSystemRingtone) {
			sprintf(File.ID_FullName,"%i",MultiBitmap.Bitmap[0].RingtoneID);

			File.Buffer 	= NULL;
			File.Used 	= 0;

			error = ERR_NONE;
//			while (error == ERR_NONE) {
				error = Phone->GetFilePart(&s,&File,&Handle,&Size);
//			}
		    	if (error != ERR_EMPTY && error != ERR_WRONGCRC) Print_Error(error);
			error = ERR_NONE;

			printmsg("Ringtone    : \"%s\" (file with ID %i)\n",
				DecodeUnicodeString(File.Name),
				MultiBitmap.Bitmap[0].RingtoneID);
		} else {
			error = Phone->GetRingtonesInfo(&s,&Info);
			if (error != ERR_NONE) Info.Number = 0;
			error = ERR_NONE;

			printmsg("Ringtone    : ");
			if (UnicodeLength(GSM_GetRingtoneName(&Info,MultiBitmap.Bitmap[0].RingtoneID))!=0) {
				printmsg("\"%s\" (ID %i)\n",
					DecodeUnicodeConsole(GSM_GetRingtoneName(&Info,MultiBitmap.Bitmap[0].RingtoneID)),
					MultiBitmap.Bitmap[0].RingtoneID);
			} else {
				printmsg("ID %i\n",MultiBitmap.Bitmap[0].RingtoneID);
			}

			if (Info.Ringtone) free(Info.Ringtone);
		}
		if (MultiBitmap.Bitmap[0].BitmapEnabled) {
			printmsg("Bitmap      : enabled\n");
		} else {
			printmsg("Bitmap      : disabled\n");
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
			printmsg("No operator logo in phone\n");
		}
		break;
	case GSM_PictureImage:
		GSM_PrintBitmap(stdout,&MultiBitmap.Bitmap[0]);
		printmsg("Text   : \"%s\"\n",DecodeUnicodeConsole(MultiBitmap.Bitmap[0].Text));
		printmsg("Sender : \"%s\"\n",DecodeUnicodeConsole(MultiBitmap.Bitmap[0].Sender));
		if (MultiBitmap.Bitmap[0].Name)
			printmsg("Name   : \"%s\"\n",DecodeUnicodeConsole(MultiBitmap.Bitmap[0].Name));
		if (argc>4) error=GSM_SaveBitmapFile(argv[4],&MultiBitmap);
		break;
	case GSM_WelcomeNote_Text:
		printmsg("Welcome note text is \"%s\"\n",DecodeUnicodeConsole(MultiBitmap.Bitmap[0].Text));
		break;
	case GSM_DealerNote_Text:
		printmsg("Dealer note text is \"%s\"\n",DecodeUnicodeConsole(MultiBitmap.Bitmap[0].Text));
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

	if (mystrncasecmp(argv[2],"STARTUP",0)) {
		if (argc<4) {
			printmsg("More arguments required\n");
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
	} else if (mystrncasecmp(argv[2],"TEXT",0)) {
		if (argc<4) {
			printmsg("More arguments required\n");
			exit(-1);
		}
		Bitmap.Type=GSM_WelcomeNote_Text;
		EncodeUnicode(Bitmap.Text,argv[3],strlen(argv[3]));
	} else if (mystrncasecmp(argv[2],"DEALER",0)) {
		if (argc<4) {
			printmsg("More arguments required\n");
			exit(-1);
		}
		Bitmap.Type=GSM_DealerNote_Text;
		EncodeUnicode(Bitmap.Text,argv[3],strlen(argv[3]));
	} else if (mystrncasecmp(argv[2],"CALLER",0)) {
		if (argc<4) {
			printmsg("More arguments required\n");
			exit(-1);
		}
		GetStartStop(&i, NULL, 3, argc, argv);
		if (i>5 && i!=255) {
			printmsg("Maximal location for caller logo can be 5\n");
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
	} else if (mystrncasecmp(argv[2],"PICTURE",0)) {
		if (argc<5) {
			printmsg("More arguments required\n");
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
	} else if (mystrncasecmp(argv[2],"COLOUROPERATOR",0)) {
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
	} else if (mystrncasecmp(argv[2],"COLOURSTARTUP",0)) {
		Bitmap.Type 	= GSM_ColourStartupLogo_ID;
		Bitmap.Location = 0;
		if (argc > 3) {
			Bitmap.Location = 1;
			Bitmap.ID 	= atoi(argv[3]);
		}
	} else if (mystrncasecmp(argv[2],"WALLPAPER",0)) {
		Bitmap.Type 		= GSM_ColourWallPaper_ID;
		Bitmap.ID		= 0;
		if (argc > 3) Bitmap.ID = atoi(argv[3]);
	} else if (mystrncasecmp(argv[2],"OPERATOR",0)) {
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
		printmsg("What type of logo do you want to set (\"%s\") ?\n",argv[2]);
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
			if (mystrncasecmp(argv[i],"-scale",0)) {
				ringtone.NoteTone.AllNotesScale = true;
				break;
			}
			if (mystrncasecmp(argv[i],"-location",0)) {
				nextlong = 1;
				break;
			}
			if (mystrncasecmp(argv[i],"-name",0)) {
				nextlong = 2;
				break;
			}
			printmsg("Unknown parameter \"%s\"",argv[i]);
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
		printmsg("Parameter missed...\n");
		exit(-1);
	}
	if (ringtone.Location==0) {
		printmsg("ERROR: enumerate locations from 1\n");
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
		printmsg("Error\n");
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

        printf("msg:%s nb:%i udh:%s\n",
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
	printmsg("%s\n\n",hexreq);
#endif
}

#define SEND_SAVE_SMS_BUFFER_SIZE 10000

static GSM_Error SMSStatus;

static void SendSMSStatus (char *Device, int status, int MessageReference)
{
	dbgprintf("Sent SMS on device: \"%s\"\n",Device);
	if (status==0) {
		printmsg("..OK");
		SMSStatus = ERR_NONE;
	} else {
		printmsg("..error %i",status);
		SMSStatus = ERR_UNKNOWN;
	}
	printmsg(", message reference=%d\n",MessageReference);
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
	char				Name		[(GSM_MAX_NUMBER_LENGTH+1)*2];
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

	if (mystrncasecmp(argv[1],"--savesms",0)) {
		EncodeUnicode(Sender,"Gammu",5);
		Name[0]  	= 0;
		Name[1]  	= 0;
		startarg 	= 0;
	} else {
		EncodeUnicode(Sender,argv[3],strlen(argv[3]));
		startarg 	= 1;
		Validity.Format = 0;
	}
	if (mystrncasecmp(argv[1],"--sendsmsdsms",0)) {
		startarg=startarg+2;
		EncodeUnicode(SMSC,"1234",4);
		SMSCSet	= 0;
	}

	if (mystrncasecmp(argv[2],"TEXT",0)) {
		chars_read = fread(InputBuffer, 1, SEND_SAVE_SMS_BUFFER_SIZE/2, stdin);
		if (chars_read == 0) printmsg("Warning: 0 chars read !\n");
		InputBuffer[chars_read] 		= 0x00;
		InputBuffer[chars_read+1] 		= 0x00;
		EncodeUnicode(Buffer[0],InputBuffer,strlen(InputBuffer));
		SMSInfo.Entries[0].Buffer  		= Buffer[0];
		SMSInfo.Entries[0].ID			= SMS_Text;
		SMSInfo.UnicodeCoding   		= false;
		startarg += 3;
	} else if (mystrncasecmp(argv[2],"SMSTEMPLATE",0)) {
		SMSInfo.UnicodeCoding   		= false;
		SMSInfo.EntriesNum 			= 1;
		Buffer[0][0]				= 0x00;
		Buffer[0][1]				= 0x00;
		SMSInfo.Entries[0].Buffer  		= Buffer[0];
		SMSInfo.Entries[0].ID			= SMS_AlcatelSMSTemplateName;
		startarg += 3;
	} else if (mystrncasecmp(argv[2],"EMS",0)) {
		SMSInfo.UnicodeCoding   		= false;
		SMSInfo.EntriesNum 			= 0;
		startarg += 3;
	} else if (mystrncasecmp(argv[2],"MMSINDICATOR",0)) {
		if (argc<6+startarg) {
			printmsg("Where are parameters ?\n");
			exit(-1);
		}
		SMSInfo.Entries[0].ID 	 	= SMS_MMSIndicatorLong;
		SMSInfo.Entries[0].MMSIndicator = &MMSInfo;
		if (mystrncasecmp(argv[1],"--savesms",0)) {
			EncodeUnicode(Sender,"MMS Info",8);
		}
		strcpy(MMSInfo.Address,	argv[3+startarg]);
		strcpy(MMSInfo.Title,	argv[4+startarg]);
		strcpy(MMSInfo.Sender,	argv[5+startarg]);
		startarg += 6;
	} else if (mystrncasecmp(argv[2],"WAPINDICATOR",0)) {
		if (argc<5+startarg) {
			printmsg("Where are parameters ?\n");
			exit(-1);
		}
		SMSInfo.Entries[0].ID 	 	= SMS_WAPIndicatorLong;
		SMSInfo.Entries[0].MMSIndicator = &MMSInfo;
		if (mystrncasecmp(argv[1],"--savesms",0)) {
			EncodeUnicode(Sender,"WAP Info",8);
		}
		strcpy(MMSInfo.Address,	argv[3+startarg]);
		strcpy(MMSInfo.Title,	argv[4+startarg]);
		startarg += 5;
	} else if (mystrncasecmp(argv[2],"RINGTONE",0)) {
		if (argc<4+startarg) {
			printmsg("Where is ringtone filename ?\n");
			exit(-1);
		}
		ringtone[0].Format=RING_NOTETONE;
		error=GSM_ReadRingtoneFile(argv[3+startarg],&ringtone[0]);
		Print_Error(error);
		SMSInfo.Entries[0].ID 	 = SMS_NokiaRingtone;
		SMSInfo.Entries[0].Ringtone = &ringtone[0];
		if (mystrncasecmp(argv[1],"--savesms",0)) {
			CopyUnicodeString(Sender, ringtone[0].Name);
			EncodeUnicode(Name,"Ringtone ",9);
			CopyUnicodeString(Name+9*2, ringtone[0].Name);
		}
		startarg += 4;
	} else if (mystrncasecmp(argv[2],"OPERATOR",0)) {
		if (argc<4+startarg) {
			printmsg("Where is logo filename ?\n");
			exit(-1);
		}
		bitmap[0].Bitmap[0].Type=GSM_OperatorLogo;
		error=GSM_ReadBitmapFile(argv[3+startarg],&bitmap[0]);
		Print_Error(error);
		strcpy(bitmap[0].Bitmap[0].NetworkCode,"000 00");
		SMSInfo.Entries[0].ID 	 = SMS_NokiaOperatorLogo;
		SMSInfo.Entries[0].Bitmap   = &bitmap[0];
		if (mystrncasecmp(argv[1],"--savesms",0)) {
			EncodeUnicode(Sender, "OpLogo",6);
			EncodeUnicode(Name,"OpLogo ",7);
		}
		startarg += 4;
	} else if (mystrncasecmp(argv[2],"CALLER",0)) {
		if (argc<4+startarg) {
			printmsg("Where is logo filename ?\n");
			exit(-1);
		}
		bitmap[0].Bitmap[0].Type=GSM_CallerGroupLogo;
		error=GSM_ReadBitmapFile(argv[3+startarg],&bitmap[0]);
		Print_Error(error);
		SMSInfo.Entries[0].ID 	    = SMS_NokiaCallerLogo;
		SMSInfo.Entries[0].Bitmap   = &bitmap[0];
		if (mystrncasecmp(argv[1],"--savesms",0)) {
			EncodeUnicode(Sender, "Caller",6);
		}
		startarg += 4;
	} else if (mystrncasecmp(argv[2],"ANIMATION",0)) {
		SMSInfo.UnicodeCoding   		= false;
		SMSInfo.EntriesNum 			= 1;
		if (argc<4+startarg) {
			printmsg("Where is number of frames ?\n");
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
	} else if (mystrncasecmp(argv[2],"PICTURE",0)) {
		if (argc<4+startarg) {
			printmsg("Where is logo filename ?\n");
			exit(-1);
		}
		bitmap[0].Bitmap[0].Type=GSM_PictureImage;
		error=GSM_ReadBitmapFile(argv[3+startarg],&bitmap[0]);
		printmsg("File \"%s\"\n",argv[3+startarg]);
		Print_Error(error);
		SMSInfo.Entries[0].ID 	 	= SMS_NokiaPictureImageLong;
		SMSInfo.Entries[0].Bitmap   	= &bitmap[0];
		SMSInfo.UnicodeCoding 		= false;
		bitmap[0].Bitmap[0].Text[0]	= 0;
		bitmap[0].Bitmap[0].Text[1]	= 0;
		if (mystrncasecmp(argv[1],"--savesms",0)) {
			EncodeUnicode(Sender, "Picture",7);
			EncodeUnicode(Name,"Picture Image",13);
		}
		startarg += 4;
#ifdef GSM_ENABLE_BACKUP
	} else if (mystrncasecmp(argv[2],"BOOKMARK",0)) {
		if (argc<5+startarg) {
			printmsg("Where is backup filename and location ?\n");
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
			printmsg("Bookmark not found in file\n");
			exit(-1);
		}
		SMSInfo.Entries[0].ID 	    = SMS_NokiaWAPBookmarkLong;
		SMSInfo.Entries[0].Bookmark = Backup.WAPBookmark[i];
		if (mystrncasecmp(argv[1],"--savesms",0)) {
			EncodeUnicode(Sender, "Bookmark",8);
			EncodeUnicode(Name,"WAP Bookmark",12);
		}
		startarg += 5;
	} else if (mystrncasecmp(argv[2],"WAPSETTINGS",0)) {
		if (argc<6+startarg) {
			printmsg("Where is backup filename and location ?\n");
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
			printmsg("WAP settings not found in file\n");
			exit(-1);
		}
		SMSInfo.Entries[0].Settings = NULL;
		for (j=0;j<Backup.WAPSettings[i]->Number;j++) {
			switch (Backup.WAPSettings[i]->Settings[j].Bearer) {
			case WAPSETTINGS_BEARER_GPRS:
				if (mystrncasecmp(argv[5+startarg],"GPRS",0)) {
					SMSInfo.Entries[0].Settings = &Backup.WAPSettings[i]->Settings[j];
					break;
				}
			case WAPSETTINGS_BEARER_DATA:
				if (mystrncasecmp(argv[5+startarg],"DATA",0)) {
					SMSInfo.Entries[0].Settings = &Backup.WAPSettings[i]->Settings[j];
					break;
				}
			default:
				break;
			}
		}
		if (SMSInfo.Entries[0].Settings == NULL) {
			printmsg("Sorry. For now there is only support for GPRS or DATA bearers end\n");
			exit(-1);
		}
		SMSInfo.Entries[0].ID = SMS_NokiaWAPSettingsLong;
		if (mystrncasecmp(argv[1],"--savesms",0)) {
			EncodeUnicode(Sender, "Settings",8);
			EncodeUnicode(Name,"WAP Settings",12);
		}
		startarg += 6;
	} else if (mystrncasecmp(argv[2],"MMSSETTINGS",0)) {
		if (argc<5+startarg) {
			printmsg("Where is backup filename and location ?\n");
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
			printmsg("MMS settings not found in file\n");
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
			printmsg("Sorry. No GPRS bearer found in MMS settings\n");
			exit(-1);
		}
		SMSInfo.Entries[0].ID = SMS_NokiaMMSSettingsLong;
		if (mystrncasecmp(argv[1],"--savesms",0)) {
			EncodeUnicode(Sender, "Settings",8);
			EncodeUnicode(Name,"MMS Settings",12);
		}
		startarg += 5;
	} else if (mystrncasecmp(argv[2],"CALENDAR",0)) {
		if (argc<5+startarg) {
			printmsg("Where is backup filename and location ?\n");
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
			printmsg("Calendar note not found in file\n");
			exit(-1);
		}
		SMSInfo.Entries[0].ID 	    = SMS_NokiaVCALENDAR10Long;
		SMSInfo.Entries[0].Calendar = Backup.Calendar[i];
		if (mystrncasecmp(argv[1],"--savesms",0)) {
			EncodeUnicode(Sender, "Calendar",8);
		}
		startarg += 5;
	} else if (mystrncasecmp(argv[2],"TODO",0)) {
		if (argc<5+startarg) {
			printmsg("Where is backup filename and location ?\n");
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
			printmsg("ToDo note not found in file\n");
			exit(-1);
		}
		SMSInfo.Entries[0].ID 	 	= SMS_NokiaVTODOLong;
		SMSInfo.Entries[0].ToDo 	= Backup.ToDo[i];
		if (mystrncasecmp(argv[1],"--savesms",0)) {
			EncodeUnicode(Sender, "ToDo",8);
		}
		startarg += 5;
	} else if (mystrncasecmp(argv[2],"VCARD10",0) || mystrncasecmp(argv[2],"VCARD21",0)) {
		if (argc<6+startarg) {
			printmsg("Where is backup filename and location and memory type ?\n");
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[3+startarg],&Backup);
		if (error!=ERR_NOTIMPLEMENTED) Print_Error(error);
		i = 0;
		if (mystrncasecmp(argv[4+startarg],"SM",0)) {
			while (Backup.SIMPhonebook[i]!=NULL) {
				if (i == atoi(argv[5+startarg])-1) break;
				i++;
			}
			if (i != atoi(argv[5+startarg])-1) {
				printmsg("Phonebook entry not found in file\n");
				exit(-1);
			}
			SMSInfo.Entries[0].Phonebook = Backup.SIMPhonebook[i];
		} else if (mystrncasecmp(argv[4+startarg],"ME",0)) {
			while (Backup.PhonePhonebook[i]!=NULL) {
				if (i == atoi(argv[5+startarg])-1) break;
				i++;
			}
			if (i != atoi(argv[5+startarg])-1) {
				printmsg("Phonebook entry not found in file\n");
				exit(-1);
			}
			SMSInfo.Entries[0].Phonebook = Backup.PhonePhonebook[i];
		} else {
			printmsg("Unknown memory type: \"%s\"\n",argv[4+startarg]);
			exit(-1);
		}
		if (mystrncasecmp(argv[2],"VCARD10",0)) {
			SMSInfo.Entries[0].ID = SMS_VCARD10Long;
		} else {
			SMSInfo.Entries[0].ID = SMS_VCARD21Long;
		}
		if (mystrncasecmp(argv[1],"--savesms",0)) {
			EncodeUnicode(Sender, "VCARD",5);
			EncodeUnicode(Name, "Phonebook entry",15);
		}
		startarg += 6;
#endif
	} else if (mystrncasecmp(argv[2],"PROFILE",0)) {
		SMSInfo.Entries[0].ID = SMS_NokiaProfileLong;
		if (mystrncasecmp(argv[1],"--savesms",0)) {
			EncodeUnicode(Sender, "Profile",7);
		}
		startarg += 3;
	} else {
		printmsg("What format of sms (\"%s\") ?\n",argv[2]);
		exit(-1);
	}

	for (i=startarg;i<argc;i++) {
		switch (nextlong) {
		case 0:
			if (mystrncasecmp(argv[1],"--savesms",0) || SendSaved) {
				if (mystrncasecmp(argv[i],"-folder",0)) {
					nextlong=1;
					continue;
				}
			}
			if (mystrncasecmp(argv[1],"--savesms",0)) {
				if (mystrncasecmp(argv[i],"-unread",0)) {
					State = SMS_UnRead;
					continue;
				}
				if (mystrncasecmp(argv[i],"-read",0)) {
					State = SMS_Read;
					continue;
				}
				if (mystrncasecmp(argv[i],"-unsent",0)) {
					State = SMS_UnSent;
					continue;
				}
				if (mystrncasecmp(argv[i],"-sent",0)) {
					State = SMS_Sent;
					continue;
				}
				if (mystrncasecmp(argv[i],"-sender",0)) {
					nextlong=2;
					continue;
				}
			} else {
				if (mystrncasecmp(argv[i],"-save",0)) {
					SendSaved=true;
					continue;
				}
				if (mystrncasecmp(argv[i],"-report",0)) {
					DeliveryReport=true;
					continue;
				}
				if (mystrncasecmp(argv[i],"-validity",0)) {
					nextlong=10;
					continue;
				}
			}
			if (mystrncasecmp(argv[i],"-smscset",0)) {
				nextlong=3;
				continue;
			}
			if (mystrncasecmp(argv[i],"-smscnumber",0)) {
				nextlong=4;
				continue;
			}
			if (mystrncasecmp(argv[i],"-protected",0)) {
				nextlong=19;
				continue;
			}
			if (mystrncasecmp(argv[i],"-reply",0)) {
				ReplyViaSameSMSC=true;
				continue;
			}
			if (mystrncasecmp(argv[i],"-maxsms",0)) {
				nextlong=21;
				continue;
			}
			if (mystrncasecmp(argv[2],"RINGTONE",0)) {
				if (mystrncasecmp(argv[i],"-long",0)) {
					SMSInfo.Entries[0].ID = SMS_NokiaRingtoneLong;
					break;
				}
				if (mystrncasecmp(argv[i],"-scale",0)) {
					ringtone[0].NoteTone.AllNotesScale=true;
					break;
				}
			}
			if (mystrncasecmp(argv[2],"TEXT",0)) {
				if (mystrncasecmp(argv[i],"-inputunicode",0)) {
					ReadUnicodeFile(Buffer[0],InputBuffer);
					break;
				}
				if (mystrncasecmp(argv[i],"-16bit",0)) {
					if (SMSInfo.Entries[0].ID == SMS_ConcatenatedTextLong) SMSInfo.Entries[0].ID = SMS_ConcatenatedTextLong16bit;
					if (SMSInfo.Entries[0].ID == SMS_ConcatenatedAutoTextLong) SMSInfo.Entries[0].ID = SMS_ConcatenatedAutoTextLong16bit;
					break;
				}
				if (mystrncasecmp(argv[i],"-flash",0)) {
					SMSInfo.Class = 0;
					break;
				}
				if (mystrncasecmp(argv[i],"-len",0)) {
					nextlong = 5;
					break;
				}
				if (mystrncasecmp(argv[i],"-autolen",0)) {
					nextlong = 5;
					break;
				}
				if (mystrncasecmp(argv[i],"-unicode",0)) {
					SMSInfo.UnicodeCoding = true;
					break;
				}
				if (mystrncasecmp(argv[i],"-enablevoice",0)) {
					SMSInfo.Entries[0].ID = SMS_EnableVoice;
					break;
				}
				if (mystrncasecmp(argv[i],"-disablevoice",0)) {
					SMSInfo.Entries[0].ID = SMS_DisableVoice;
					break;
				}
				if (mystrncasecmp(argv[i],"-enablefax",0)) {
					SMSInfo.Entries[0].ID = SMS_EnableFax;
					break;
				}
				if (mystrncasecmp(argv[i],"-disablefax",0)) {
					SMSInfo.Entries[0].ID = SMS_DisableFax;
					break;
				}
				if (mystrncasecmp(argv[i],"-enableemail",0)) {
					SMSInfo.Entries[0].ID = SMS_EnableEmail;
					break;
				}
				if (mystrncasecmp(argv[i],"-disableemail",0)) {
					SMSInfo.Entries[0].ID = SMS_DisableEmail;
					break;
				}
				if (mystrncasecmp(argv[i],"-voidsms",0)) {
					SMSInfo.Entries[0].ID = SMS_VoidSMS;
					break;
				}
				if (mystrncasecmp(argv[i],"-replacemessages",0) &&
				    SMSInfo.Entries[0].ID != SMS_ConcatenatedTextLong) {
					nextlong = 8;
					break;
				}
				if (mystrncasecmp(argv[i],"-replacefile",0)) {
					nextlong = 9;
					continue;
				}
			}
			if (mystrncasecmp(argv[2],"PICTURE",0)) {
				if (mystrncasecmp(argv[i],"-text",0)) {
					nextlong = 6;
					break;
				}
				if (mystrncasecmp(argv[i],"-unicode",0)) {
					SMSInfo.UnicodeCoding = true;
					break;
				}
				if (mystrncasecmp(argv[i],"-alcatelbmmi",0)) {
					bitmap[0].Bitmap[0].Type=GSM_StartupLogo;
					error=GSM_ReadBitmapFile(argv[startarg-1],&bitmap[0]);
					Print_Error(error);
					SMSInfo.UnicodeCoding = true;
					SMSInfo.Entries[0].ID = SMS_AlcatelMonoBitmapLong;
					break;
				}
				break;
			}
			if (mystrncasecmp(argv[2],"VCARD10",0)) {
				if (mystrncasecmp(argv[i],"-nokia",0)) {
					SMSInfo.Entries[0].ID = SMS_NokiaVCARD10Long;
					break;
				}
				break;
			}
			if (mystrncasecmp(argv[2],"VCARD21",0)) {
				if (mystrncasecmp(argv[i],"-nokia",0)) {
					SMSInfo.Entries[0].ID = SMS_NokiaVCARD21Long;
					break;
				}
				break;
			}
			if (mystrncasecmp(argv[2],"PROFILE",0)) {
				if (mystrncasecmp(argv[i],"-name",0)) {
					nextlong = 22;
					break;
				}
				if (mystrncasecmp(argv[i],"-ringtone",0)) {
					nextlong = 23;
					break;
				}
				if (mystrncasecmp(argv[i],"-bitmap",0)) {
					nextlong = 24;
					break;
				}
			}
			if (mystrncasecmp(argv[2],"SMSTEMPLATE",0)) {
				if (mystrncasecmp(argv[i],"-unicode",0)) {
					SMSInfo.UnicodeCoding = true;
					break;
				}
				if (mystrncasecmp(argv[i],"-text",0)) {
					nextlong = 11;
					break;
				}
				if (mystrncasecmp(argv[i],"-unicodefiletext",0)) {
					nextlong = 18;
					break;
				}
				if (mystrncasecmp(argv[i],"-defsound",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSPredefinedSound;
					nextlong = 12;
					break;
				}
				if (mystrncasecmp(argv[i],"-defanimation",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSPredefinedAnimation;
					nextlong = 12;
					break;
				}
				if (mystrncasecmp(argv[i],"-tone10",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSound10;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (mystrncasecmp(argv[i],"-tone10long",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSound10Long;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (mystrncasecmp(argv[i],"-tone12",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSound12;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (mystrncasecmp(argv[i],"-tone12long",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSound12Long;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (mystrncasecmp(argv[i],"-toneSE",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSonyEricssonSound;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (mystrncasecmp(argv[i],"-toneSElong",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSonyEricssonSoundLong;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (mystrncasecmp(argv[i],"-variablebitmap",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSVariableBitmap;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 15;
					break;
				}
				if (mystrncasecmp(argv[i],"-variablebitmaplong",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSVariableBitmapLong;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 15;
					break;
				}
				if (mystrncasecmp(argv[i],"-animation",0)) {
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
			if (mystrncasecmp(argv[2],"EMS",0)) {
				if (mystrncasecmp(argv[i],"-unicode",0)) {
					SMSInfo.UnicodeCoding = true;
					break;
				}
				if (mystrncasecmp(argv[i],"-16bit",0)) {
					EMS16Bit = true;
					break;
				}
				if (mystrncasecmp(argv[i],"-format",0)) {
					nextlong = 20;
					break;
				}
				if (mystrncasecmp(argv[i],"-text",0)) {
					nextlong = 11;
					break;
				}
				if (mystrncasecmp(argv[i],"-unicodefiletext",0)) {
					nextlong = 18;
					break;
				}
				if (mystrncasecmp(argv[i],"-defsound",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSPredefinedSound;
					nextlong = 12;
					break;
				}
				if (mystrncasecmp(argv[i],"-defanimation",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSPredefinedAnimation;
					nextlong = 12;
					break;
				}
				if (mystrncasecmp(argv[i],"-tone10",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSound10;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (mystrncasecmp(argv[i],"-tone10long",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSound10Long;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (mystrncasecmp(argv[i],"-tone12",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSound12;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (mystrncasecmp(argv[i],"-tone12long",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSound12Long;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (mystrncasecmp(argv[i],"-toneSE",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSonyEricssonSound;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (mystrncasecmp(argv[i],"-toneSElong",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSonyEricssonSoundLong;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 14;
					break;
				}
				if (mystrncasecmp(argv[i],"-fixedbitmap",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSFixedBitmap;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 15;
					break;
				}
				if (mystrncasecmp(argv[i],"-variablebitmap",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSVariableBitmap;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 15;
					break;
				}
				if (mystrncasecmp(argv[i],"-variablebitmaplong",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSVariableBitmapLong;
					if (Protected != 0) {
						SMSInfo.Entries[SMSInfo.EntriesNum].Protected = true;
						Protected --;
					}
					nextlong = 15;
					break;
				}
				if (mystrncasecmp(argv[i],"-animation",0)) {
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
			if (mystrncasecmp(argv[2],"OPERATOR",0)) {
				if (mystrncasecmp(argv[i],"-netcode",0)) {
					nextlong = 7;
					break;
				}
				if (mystrncasecmp(argv[i],"-biglogo",0)) {
					SMSInfo.Entries[0].ID = SMS_NokiaOperatorLogoLong;
					break;
				}
				break;
			}
			printmsg("Unknown parameter (\"%s\")\n",argv[i]);
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
			if (mystrncasecmp(argv[i-1],"-autolen",0)) SMSInfo.Entries[0].ID = SMS_ConcatenatedAutoTextLong;
			nextlong = 0;
			break;
		case 6:	/* Picture Images - text */
			EncodeUnicode(bitmap[0].Bitmap[0].Text,argv[i],strlen(argv[i]));
			nextlong = 0;
			break;
		case 7:	/* Operator Logo - network code */
			strncpy(bitmap[0].Bitmap[0].NetworkCode,argv[i],7);
			if (!strcmp(DecodeUnicodeConsole(GSM_GetNetworkName(bitmap[0].Bitmap[0].NetworkCode)),"unknown")) {
				printmsg("Unknown GSM network code (\"%s\")\n",argv[i]);
				exit(-1);
			}
			if (mystrncasecmp(argv[1],"--savesms",0)) {
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
				printmsg("You have to give number between 1 and 7 (\"%s\")\n",argv[i]);
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
			if (mystrncasecmp(argv[i],"HOUR",0)) 		Validity.Relative = SMS_VALID_1_Hour;
			else if (mystrncasecmp(argv[i],"6HOURS",0))	Validity.Relative = SMS_VALID_6_Hours;
			else if (mystrncasecmp(argv[i],"DAY",0)) 	Validity.Relative = SMS_VALID_1_Day;
			else if (mystrncasecmp(argv[i],"3DAYS",0)) 	Validity.Relative = SMS_VALID_3_Days;
			else if (mystrncasecmp(argv[i],"WEEK",0)) 	Validity.Relative = SMS_VALID_1_Week;
			else if (mystrncasecmp(argv[i],"MAX",0)) 	Validity.Relative = SMS_VALID_Max_Time;
			else {
				printmsg("Unknown validity string (\"%s\")\n",argv[i]);
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
				printmsg("You have to give number of EMS frames between 1 and 4 (\"%s\")\n",argv[i]);
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
				printmsg("Can't open file \"%s\"\n",argv[i]);
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
					printmsg("Unknown parameter \"%c\"\n",argv[i][j]);
					exit(-1);
				}
				}
			} else {
				printmsg("Last parameter wasn't text\n");
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
		}
	}
	if (nextlong!=0) {
		printmsg("Parameter missed...\n");
		exit(-1);
	}

	if (mystrncasecmp(argv[2],"EMS",0) && EMS16Bit) {
		for (i=0;i<SMSInfo.EntriesNum;i++) {
			switch (SMSInfo.Entries[i].ID) {
			case SMS_ConcatenatedTextLong:
				SMSInfo.Entries[i].ID = SMS_ConcatenatedTextLong16bit;
			default:
				break;

			}
		}

	}
	if (mystrncasecmp(argv[2],"TEXT",0)) {
		chars_read = UnicodeLength(Buffer[0]);
		if (chars_read != 0) {
			/* Trim \n at the end of string */
			if (Buffer[0][chars_read*2-1] == '\n' && Buffer[0][chars_read*2-2] == 0)
			{
				Buffer[0][chars_read*2-1] = 0;
			}
		}
	}

	if (mystrncasecmp(argv[1],"--displaysms",0) || mystrncasecmp(argv[1],"--sendsmsdsms",0)) {
		if (mystrncasecmp(argv[2],"OPERATOR",0)) {
			if (bitmap[0].Bitmap[0].Type==GSM_OperatorLogo && strcmp(bitmap[0].Bitmap[0].NetworkCode,"000 00")==0) {
				printmsg("No network code\n");
				exit(-1);
			}
		}
	} else {
		GSM_Init(true);

		if (mystrncasecmp(argv[2],"OPERATOR",0)) {
			if (bitmap[0].Bitmap[0].Type==GSM_OperatorLogo && strcmp(bitmap[0].Bitmap[0].NetworkCode,"000 00")==0) {
				error=Phone->GetNetworkInfo(&s,&NetInfo);
				Print_Error(error);
				strcpy(bitmap[0].Bitmap[0].NetworkCode,NetInfo.NetworkCode);
				if (mystrncasecmp(argv[1],"--savesms",0)) {
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
					printmsg("Warning: ringtone too long. %i percent part cut\n",
						(SMSInfo.Entries[i].Ringtone->NoteTone.NrCommands-SMSInfo.Entries[i].RingtoneNotes)*100/SMSInfo.Entries[i].Ringtone->NoteTone.NrCommands);
				}
			default:
				break;

		}
	}
	if (MaxSMS != -1 && sms.Number > MaxSMS) {
		printmsg("There is %i SMS packed and %i limit. Exiting\n",sms.Number,MaxSMS);
		if (!mystrncasecmp(argv[1],"--displaysms",0) && !mystrncasecmp(argv[1],"--sendsmsdsms",0)) GSM_Terminate();
		exit(-1);
	}

	if (mystrncasecmp(argv[1],"--displaysms",0)) {
		if (SMSCSet != 0) {
			printmsg("Use -smscnumber option to give SMSC number\n");
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

		printmsg("\nNumber of SMS: %i\n",sms.Number);
		exit(sms.Number);
	}
	if (mystrncasecmp(argv[1],"--sendsmsdsms",0)) {
		if (SMSCSet != 0) {
			printmsg("Use -smscnumber option to give SMSC number\n");
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
	if (mystrncasecmp(argv[1],"--savesms",0) || SendSaved) {
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
			printmsgerr("If you want break, press Ctrl+C...\n");
		}

		for (i=0;i<sms.Number;i++) {
			printmsg("Saving SMS %i/%i\n",i+1,sms.Number);
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
			printmsg("Saved in folder \"%s\", location %i\n",
				DecodeUnicodeConsole(folders.Folder[sms.SMS[i].Folder-1].Name),sms.SMS[i].Location);

			if (SendSaved) {
				printmsg("Sending sms from folder \"%s\", location %i\n",
					DecodeUnicodeString(folders.Folder[sms.SMS[i].Folder-1].Name),sms.SMS[i].Location);
				SMSStatus = ERR_TIMEOUT;
				error=Phone->SendSavedSMS(&s, 0, sms.SMS[i].Location);
				Print_Error(error);
				printmsg("....waiting for network answer");
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
		printmsgerr("If you want break, press Ctrl+C...\n");

		s.User.SendSMSStatus = SendSMSStatus;

		for (i=0;i<sms.Number;i++) {
			printmsg("Sending SMS %i/%i",i+1,sms.Number);
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
			printmsg("....waiting for network answer");
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

	if (mystrncasecmp(argv[2],"CALENDAR",0)) {
		if (argc<5) {
			printmsg("Where is backup filename and location ?\n");
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
			printmsg("Calendar note not found in file\n");
			exit(-1);
		}
		j = 0;
		GSM_EncodeVCALENDAR(Buffer, &j, Backup.Calendar[i],true,Nokia_VCalendar);
	} else if (mystrncasecmp(argv[2],"BOOKMARK",0)) {
		if (argc<5) {
			printmsg("Where is backup filename and location ?\n");
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
			printmsg("WAP bookmark not found in file\n");
			exit(-1);
		}
		j = 0;
		GSM_EncodeURLFile(Buffer, &j, Backup.WAPBookmark[i]);
	} else if (mystrncasecmp(argv[2],"NOTE",0)) {
		if (argc<5) {
			printmsg("Where is backup filename and location ?\n");
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
			printmsg("Note not found in file\n");
			exit(-1);
		}
		j = 0;
		GSM_EncodeVNTFile(Buffer, &j, Backup.Note[i]);
	} else if (mystrncasecmp(argv[2],"TODO",0)) {
		if (argc<5) {
			printmsg("Where is backup filename and location ?\n");
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
			printmsg("ToDo note not found in file\n");
			exit(-1);
		}
		j = 0;
		GSM_EncodeVTODO(Buffer, &j, Backup.ToDo[i], true, Nokia_VToDo);
	} else if (mystrncasecmp(argv[2],"VCARD10",0) || mystrncasecmp(argv[2],"VCARD21",0)) {
		if (argc<6) {
			printmsg("Where is backup filename and location and memory type ?\n");
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[4],&Backup);
		if (error!=ERR_NOTIMPLEMENTED) Print_Error(error);
		i = 0;
		if (mystrncasecmp(argv[5],"SM",0)) {
			while (Backup.SIMPhonebook[i]!=NULL) {
				if (i == atoi(argv[6])-1) break;
				i++;
			}
			if (i != atoi(argv[6])-1) {
				printmsg("Phonebook entry not found in file\n");
				exit(-1);
			}
			pbk = Backup.SIMPhonebook[i];
		} else if (mystrncasecmp(argv[5],"ME",0)) {
			while (Backup.PhonePhonebook[i]!=NULL) {
				if (i == atoi(argv[6])-1) break;
				i++;
			}
			if (i != atoi(argv[6])-1) {
				printmsg("Phonebook entry not found in file\n");
				exit(-1);
			}
			pbk = Backup.PhonePhonebook[i];
		} else {
			printmsg("Unknown memory type: \"%s\"\n",argv[5]);
			exit(-1);
		}
		j = 0;
		if (mystrncasecmp(argv[2],"VCARD10",0)) {
			GSM_EncodeVCARD(Buffer,&j,pbk,true,Nokia_VCard10);
		} else {
			GSM_EncodeVCARD(Buffer,&j,pbk,true,Nokia_VCard21);
		}
	} else {
		printmsg("What format of file (\"%s\") ?\n",argv[2]);
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

	if (argc == 4 && mystrncasecmp(argv[3],"-yes",0)) always_answer_yes = true;

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
	printmsgerr("Press Ctrl+C to break...\n");

	GSM_Init(true);

	if (Info.UseUnicode) {
		Info.UseUnicode=answer_yes("Use Unicode subformat of backup file");
	}
	if (Info.DateTime) {
		GSM_GetCurrentDateTime (&Backup.DateTime);
		Backup.DateTimeAvailable=true;
	}
	if (Info.Model) {
		error=Phone->GetManufacturer(&s);
		Print_Error(error);
		sprintf(Backup.Model,"%s ",s.Phone.Data.Manufacturer);
		if (s.Phone.Data.ModelInfo->model[0]!=0) {
			strcat(Backup.Model,s.Phone.Data.ModelInfo->model);
		} else {
			strcat(Backup.Model,s.Phone.Data.Model);
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
		printmsg("Checking phone phonebook\n");
		MemStatus.MemoryType = MEM_ME;
		error=Phone->GetMemoryStatus(&s, &MemStatus);
		if (error==ERR_NONE && MemStatus.MemoryUsed != 0) {
			if (answer_yes("   Backup phone phonebook")) DoBackup = true;
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
					printmsg("   Increase %s\n" , "GSM_BACKUP_MAX_PHONEPHONEBOOK");
					GSM_Terminate();
					exit(-1);
				}
				*Backup.PhonePhonebook[used]=Pbk;
				used++;
			}
			printmsgerr("%c   Reading: %i percent",13,used*100/MemStatus.MemoryUsed);
			i++;
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}
	DoBackup = false;
	if (Info.SIMPhonebook) {
		printmsg("Checking SIM phonebook\n");
		MemStatus.MemoryType = MEM_SM;
		error=Phone->GetMemoryStatus(&s, &MemStatus);
		if (error==ERR_NONE && MemStatus.MemoryUsed != 0) {
			if (answer_yes("   Backup SIM phonebook")) DoBackup=true;
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
					printmsg("   Increase %s\n" , "GSM_BACKUP_MAX_SIMPHONEBOOK");
					GSM_Terminate();
					exit(-1);
				}
				*Backup.SIMPhonebook[used]=Pbk;
				used++;
			}
			printmsgerr("%c   Reading: %i percent",13,used*100/MemStatus.MemoryUsed);
			i++;
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}
	DoBackup = false;
	if (Info.Calendar) {
		printmsg("Checking calendar\n");
		error=Phone->GetNextCalendar(&s,&Calendar,true);
		if (error==ERR_NONE) {
			if (answer_yes("   Backup calendar notes")) DoBackup = true;
		}
	}
	if (DoBackup) {
		used 		= 0;
		printmsgerr("   Reading : ");
		while (error == ERR_NONE) {
			if (used < GSM_MAXCALENDARTODONOTES) {
				Backup.Calendar[used] = malloc(sizeof(GSM_CalendarEntry));
			        if (Backup.Calendar[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.Calendar[used+1] = NULL;
			} else {
				printmsg("   Increase %s\n" , "GSM_MAXCALENDARTODONOTES");
				GSM_Terminate();
				exit(-1);
			}
			*Backup.Calendar[used]=Calendar;
			used ++;
			error=Phone->GetNextCalendar(&s,&Calendar,false);
			printmsgerr("*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}
	DoBackup = false;
	if (Info.ToDo) {
		printmsg("Checking ToDo\n");
		error=Phone->GetToDoStatus(&s,&ToDoStatus);
		if (error == ERR_NONE && ToDoStatus.Used != 0) {
			if (answer_yes("   Backup ToDo")) DoBackup = true;
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
				printmsg("   Increase %s\n" , "GSM_MAXCALENDARTODONOTES");
				GSM_Terminate();
				exit(-1);
			}
			*Backup.ToDo[used]=ToDo;
			used ++;
			error=Phone->GetNextToDo(&s,&ToDo,false);
			printmsgerr("%c   Reading: %i percent",13,used*100/ToDoStatus.Used);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}
	DoBackup = false;
	if (Info.Note) {
		printmsg("Checking notes\n");
		error=Phone->GetNextNote(&s,&Note,true);
		if (error==ERR_NONE) {
			if (answer_yes("   Backup notes")) DoBackup = true;
		}
	}
	if (DoBackup) {
		used 		= 0;
		printmsgerr("   Reading : ");
		while (error == ERR_NONE) {
			if (used < GSM_BACKUP_MAX_NOTE) {
				Backup.Note[used] = malloc(sizeof(GSM_NoteEntry));
			        if (Backup.Note[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.Note[used+1] = NULL;
			} else {
				printmsg("   Increase %s\n" , "GSM_BACKUP_MAX_NOTE");
				GSM_Terminate();
				exit(-1);
			}
			*Backup.Note[used]=Note;
			used ++;
			error=Phone->GetNextNote(&s,&Note,false);
			printmsgerr("*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}
	DoBackup = false;
	if (Info.CallerLogos) {
		printmsg("Checking caller logos\n");
		Bitmap.Type 	= GSM_CallerGroupLogo;
		Bitmap.Location = 1;
		error=Phone->GetBitmap(&s,&Bitmap);
		if (error == ERR_NONE) {
			if (answer_yes("   Backup caller groups and logos")) DoBackup = true;
		}
	}
	if (DoBackup) {
		printmsgerr("   Reading : ");
		error = ERR_NONE;
		used  = 0;
		while (error == ERR_NONE) {
			if (used < GSM_BACKUP_MAX_CALLER) {
				Backup.CallerLogos[used] = malloc(sizeof(GSM_Bitmap));
			        if (Backup.CallerLogos[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.CallerLogos[used+1] = NULL;
			} else {
				printmsg("   Increase %s\n" , "GSM_BACKUP_MAX_CALLER");
				GSM_Terminate();
				exit(-1);
			}
			*Backup.CallerLogos[used] = Bitmap;
			used ++;
			Bitmap.Location = used + 1;
			error=Phone->GetBitmap(&s,&Bitmap);
			printmsgerr("*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}
	DoBackup = false;
	if (Info.SMSC) {
		printmsg("Checking SMS profiles\n");
		if (answer_yes("   Backup SMS profiles")) DoBackup = true;
	}
	if (DoBackup) {
		used = 0;
		printmsgerr("   Reading: ");
		while (true) {
			SMSC.Location = used + 1;
			error = Phone->GetSMSC(&s,&SMSC);
			if (error != ERR_NONE) break;
			if (used < GSM_BACKUP_MAX_SMSC) {
				Backup.SMSC[used] = malloc(sizeof(GSM_SMSC));
			        if (Backup.SMSC[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.SMSC[used + 1] = NULL;
			} else {
				printmsg("   Increase %s\n" , "GSM_BACKUP_MAX_SMSC");
				GSM_Terminate();
				exit(-1);
			}
			*Backup.SMSC[used]=SMSC;
			used++;
			printmsgerr("*");
		}
		printmsgerr("\n");
	}
	DoBackup = false;
	if (Info.StartupLogo) {
		printmsg("Checking startup text\n");
		Bitmap.Type = GSM_WelcomeNote_Text;
		error = Phone->GetBitmap(&s,&Bitmap);
		if (error == ERR_NONE) {
			if (answer_yes("   Backup startup logo/text")) DoBackup = true;
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
		printmsg("Checking operator logo\n");
		Bitmap.Type = GSM_OperatorLogo;
		error=Phone->GetBitmap(&s,&Bitmap);
		if (error == ERR_NONE) {
			if (strcmp(Bitmap.NetworkCode,"000 00")!=0) {
				if (answer_yes("   Backup operator logo")) DoBackup = true;
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
		printmsg("Checking WAP bookmarks\n");
		Bookmark.Location = 1;
		error=Phone->GetWAPBookmark(&s,&Bookmark);
		if (error==ERR_NONE) {
			if (answer_yes("   Backup WAP bookmarks")) DoBackup = true;
		}
	}
	if (DoBackup) {
		used = 0;
		printmsgerr("   Reading : ");
		while (error == ERR_NONE) {
			if (used < GSM_BACKUP_MAX_WAPBOOKMARK) {
				Backup.WAPBookmark[used] = malloc(sizeof(GSM_WAPBookmark));
			        if (Backup.WAPBookmark[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.WAPBookmark[used+1] = NULL;
			} else {
				printmsg("   Increase %s\n" , "GSM_BACKUP_MAX_WAPBOOKMARK");
				GSM_Terminate();
				exit(-1);
			}
			*Backup.WAPBookmark[used]=Bookmark;
			used ++;
			Bookmark.Location = used+1;
			error=Phone->GetWAPBookmark(&s,&Bookmark);
			printmsgerr("*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}
	DoBackup = false;
	if (Info.WAPSettings) {
		printmsg("Checking WAP settings\n");
		Settings.Location = 1;
		error=Phone->GetWAPSettings(&s,&Settings);
		if (error==ERR_NONE) {
			if (answer_yes("   Backup WAP settings")) DoBackup = true;
		}
	}
	if (DoBackup) {
		used = 0;
		printmsgerr("   Reading : ");
		while (error == ERR_NONE) {
			if (used < GSM_BACKUP_MAX_WAPSETTINGS) {
				Backup.WAPSettings[used] = malloc(sizeof(GSM_MultiWAPSettings));
			        if (Backup.WAPSettings[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.WAPSettings[used+1] = NULL;
			} else {
				printmsg("   Increase %s\n" , "GSM_BACKUP_MAX_WAPSETTINGS");
				GSM_Terminate();
				exit(-1);
			}
			*Backup.WAPSettings[used]=Settings;
			used ++;
			Settings.Location = used+1;
			error=Phone->GetWAPSettings(&s,&Settings);
			printmsgerr("*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}
	DoBackup = false;
	if (Info.MMSSettings) {
		printmsg("Checking MMS settings\n");
		Settings.Location = 1;
		error=Phone->GetMMSSettings(&s,&Settings);
		if (error==ERR_NONE) {
			if (answer_yes("   Backup MMS settings")) DoBackup = true;
		}
	}
	if (DoBackup) {
		used = 0;
		printmsgerr("   Reading : ");
		while (error == ERR_NONE) {
			if (used < GSM_BACKUP_MAX_MMSSETTINGS) {
				Backup.MMSSettings[used] = malloc(sizeof(GSM_MultiWAPSettings));
			        if (Backup.MMSSettings[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.MMSSettings[used+1] = NULL;
			} else {
				printmsg("   Increase %s\n" , "GSM_BACKUP_MAX_MMSSETTINGS");
				GSM_Terminate();
				exit(-1);
			}
			*Backup.MMSSettings[used]=Settings;
			used ++;
			Settings.Location = used+1;
			error=Phone->GetMMSSettings(&s,&Settings);
			printmsgerr("*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}
	DoBackup = false;
	if (Info.ChatSettings) {
		printmsg("Checking Chat settings\n");
		Chat.Location = 1;
		error=Phone->GetChatSettings(&s,&Chat);
		if (error==ERR_NONE) {
			if (answer_yes("   Backup Chat settings")) DoBackup = true;
		}
	}
	if (DoBackup) {
		used = 0;
		printmsgerr("   Reading : ");
		while (error == ERR_NONE) {
			if (used < GSM_BACKUP_MAX_CHATSETTINGS) {
				Backup.ChatSettings[used] = malloc(sizeof(GSM_ChatSettings));
			        if (Backup.ChatSettings[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.ChatSettings[used+1] = NULL;
			} else {
				printmsg("   Increase %s\n" , "GSM_BACKUP_MAX_CHATSETTINGS");
				GSM_Terminate();
				exit(-1);
			}
			*Backup.ChatSettings[used]=Chat;
			used ++;
			Chat.Location = used+1;
			error=Phone->GetChatSettings(&s,&Chat);
			printmsgerr("*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}
	DoBackup = false;
	if (Info.SyncMLSettings) {
		printmsg("Checking SyncML settings\n");
		SyncML.Location = 1;
		error=Phone->GetSyncMLSettings(&s,&SyncML);
		if (error==ERR_NONE) {
			if (answer_yes("   Backup SyncML settings")) DoBackup = true;
		}
	}
	if (DoBackup) {
		used = 0;
		printmsgerr("   Reading : ");
		while (error == ERR_NONE) {
			if (used < GSM_BACKUP_MAX_SYNCMLSETTINGS) {
				Backup.SyncMLSettings[used] = malloc(sizeof(GSM_SyncMLSettings));
			        if (Backup.SyncMLSettings[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.SyncMLSettings[used+1] = NULL;
			} else {
				printmsg("   Increase %s\n" , "GSM_BACKUP_MAX_SYNCMLSETTINGS");
				GSM_Terminate();
				exit(-1);
			}
			*Backup.SyncMLSettings[used]=SyncML;
			used ++;
			SyncML.Location = used+1;
			error=Phone->GetSyncMLSettings(&s,&SyncML);
			printmsgerr("*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}
	DoBackup = false;
	if (Info.Ringtone) {
		printmsg("Checking user ringtones\n");
		Ringtone.Location 	= 1;
		Ringtone.Format		= 0;
		error=Phone->GetRingtone(&s,&Ringtone,false);
		if (error==ERR_EMPTY || error == ERR_NONE) {
			if (answer_yes("   Backup user ringtones")) DoBackup = true;
		}
	}
	if (DoBackup) {
		used 	= 0;
		i	= 1;
		printmsgerr("   Reading : ");
		while (error == ERR_NONE || error == ERR_EMPTY) {
			if (error == ERR_NONE) {
				if (used < GSM_BACKUP_MAX_RINGTONES) {
					Backup.Ringtone[used] = malloc(sizeof(GSM_Ringtone));
				        if (Backup.Ringtone[used] == NULL) Print_Error(ERR_MOREMEMORY);
					Backup.Ringtone[used+1] = NULL;
				} else {
					printmsg("   Increase %s\n" , "GSM_BACKUP_MAX_RINGTONES");
					GSM_Terminate();
					exit(-1);
				}
				*Backup.Ringtone[used]=Ringtone;
				used ++;
			}
			i++;
			Ringtone.Location = i;
			Ringtone.Format	  = 0;
			error=Phone->GetRingtone(&s,&Ringtone,false);
			printmsgerr("*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}
	DoBackup = false;
	if (Info.Profiles) {
		printmsg("Checking phone profiles\n");
		Profile.Location = 1;
		error = Phone->GetProfile(&s,&Profile);
	        if (error == ERR_NONE) {
			if (answer_yes("   Backup phone profiles")) DoBackup = true;
		}
	}
	if (DoBackup) {
		used = 0;
		printmsgerr("   Reading: ");
		while (true) {
			Profile.Location = used + 1;
			error = Phone->GetProfile(&s,&Profile);
			if (error != ERR_NONE) break;
			if (used < GSM_BACKUP_MAX_PROFILES) {
				Backup.Profiles[used] = malloc(sizeof(GSM_Profile));
				if (Backup.Profiles[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.Profiles[used + 1] = NULL;
			} else {
				printmsg("   Increase %s\n" , "GSM_BACKUP_MAX_PROFILES");
				GSM_Terminate();
				exit(-1);
			}
			*Backup.Profiles[used]=Profile;
			used++;
			printmsgerr("*");
		}
		printmsgerr("\n");
	}
	DoBackup = false;
 	if (Info.FMStation) {
		printmsg("Checking FM stations\n");
 		FMStation.Location = 1;
 		error = Phone->GetFMStation(&s,&FMStation);
 	        if (error == ERR_NONE || error == ERR_EMPTY) {
 			if (answer_yes("   Backup phone FM stations")) DoBackup=true;
		}
	}
	if (DoBackup) {
		used	= 0;
		i	= 1;
		printmsgerr("   Reading: ");
		while (error == ERR_NONE || error == ERR_EMPTY) {
			error = Phone->GetFMStation(&s,&FMStation);
			if (error == ERR_NONE) {
 				if (used < GSM_BACKUP_MAX_FMSTATIONS) {
 					Backup.FMStation[used] = malloc(sizeof(GSM_FMStation));
					if (Backup.FMStation[used] == NULL) Print_Error(ERR_MOREMEMORY);
 					Backup.FMStation[used + 1] = NULL;
 				} else {
 					printmsg("   Increase %s\n" , "GSM_BACKUP_MAX_FMSTATIONS");
					GSM_Terminate();
 					exit(-1);
 				}
 				*Backup.FMStation[used]=FMStation;
 				used++;
 			}
 			i++;
 			FMStation.Location = i;
 			printmsgerr("*");
 		}
 		printmsgerr("\n");
 	}
	DoBackup = false;
 	if (Info.GPRSPoint) {
		printmsg("Checking GPRS access points\n");
 		GPRSPoint.Location = 1;
 		error = Phone->GetGPRSAccessPoint(&s,&GPRSPoint);
 	        if (error == ERR_NONE || error == ERR_EMPTY) {
 			if (answer_yes("   Backup GPRS access points")) DoBackup = true;
		}
	}
	if (DoBackup) {
		used	= 0;
		i	= 1;
		printmsgerr("   Reading: ");
		while (error == ERR_NONE || error == ERR_EMPTY) {
			error = Phone->GetGPRSAccessPoint(&s,&GPRSPoint);
 			if (error == ERR_NONE) {
 				if (used < GSM_BACKUP_MAX_GPRSPOINT) {
 					Backup.GPRSPoint[used] = malloc(sizeof(GSM_GPRSAccessPoint));
					if (Backup.GPRSPoint[used] == NULL) Print_Error(ERR_MOREMEMORY);
 					Backup.GPRSPoint[used + 1] = NULL;
 				} else {
 					printmsg("   Increase %s\n" , "GSM_BACKUP_MAX_GPRSPOINT");
					GSM_Terminate();
 					exit(-1);
 				}
 				*Backup.GPRSPoint[used]=GPRSPoint;
 				used++;
 			}
 			i++;
 			GPRSPoint.Location = i;
 			printmsgerr("*");
 		}
 		printmsgerr("\n");
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
	int			i, used, max = 0;
	bool			Past = true;
	bool			Found, DoRestore;

	error=GSM_ReadBackupFile(argv[2],&Backup);
	if (error!=ERR_NOTIMPLEMENTED) {
		Print_Error(error);
	} else {
		printmsgerr("WARNING: Some data not read from file. It can be damaged or restoring some settings from this file format not implemented (maybe higher Gammu required ?)\n");
	}

	signal(SIGINT, interrupt);
	printmsgerr("Press Ctrl+C to break...\n");

	if (Backup.DateTimeAvailable) 	printmsgerr("Time of backup  : %s\n",OSDateTime(Backup.DateTime,false));
	if (Backup.Model[0]!=0) 	printmsgerr("Phone           : %s\n",Backup.Model);
	if (Backup.IMEI[0]!=0) 		printmsgerr("IMEI            : %s\n",Backup.IMEI);
	if (Backup.Creator[0]!=0) 	printmsgerr("File created by : %s\n",Backup.Creator);

	if (Backup.MD5Calculated[0]!=0) {
		dbgprintf("\"%s\"\n",Backup.MD5Original);
		dbgprintf("\"%s\"\n",Backup.MD5Calculated);
		if (strcmp(Backup.MD5Original,Backup.MD5Calculated)) {
			if (!answer_yes("Checksum in backup file do not match. Continue")) return;
		}
	}

	GSM_Init(true);

	DoRestore = false;
	if (Backup.PhonePhonebook[0] != NULL) {
		MemStatus.MemoryType = MEM_ME;
		error=Phone->GetMemoryStatus(&s, &MemStatus);
		if (error==ERR_NONE) {
			max = 0;
			while (Backup.PhonePhonebook[max]!=NULL) max++;
			printmsgerr("%i entries in backup file\n",max);
			if (answer_yes("Restore phone phonebook")) DoRestore = true;
		}
	}
	if (DoRestore) {
		used = 0;
		for (i=0;i<MemStatus.MemoryUsed+MemStatus.MemoryFree;i++) {
			Pbk.MemoryType 	= MEM_ME;
			Pbk.Location	= i + 1;
			Pbk.EntriesNum	= 0;
			if (used<max) {
				if (Backup.PhonePhonebook[used]->Location == Pbk.Location) {
					Pbk = *Backup.PhonePhonebook[used];
					used++;
					dbgprintf("Location %i\n",Pbk.Location);
					if (Pbk.EntriesNum != 0) error=Phone->SetMemory(&s, &Pbk);
				}
			}
			if (Pbk.EntriesNum == 0) error=Phone->DeleteMemory(&s, &Pbk);
			Print_Error(error);
			printmsgerr("%cWriting: %i percent",13,(i+1)*100/(MemStatus.MemoryUsed+MemStatus.MemoryFree));
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}

	DoRestore = false;
	if (Backup.SIMPhonebook[0] != NULL) {
		MemStatus.MemoryType = MEM_SM;
		error=Phone->GetMemoryStatus(&s, &MemStatus);
		if (error==ERR_NONE) {
			max = 0;
			while (Backup.SIMPhonebook[max]!=NULL) max++;
			printmsgerr("%i entries in backup file\n",max);
			if (answer_yes("Restore SIM phonebook")) DoRestore = true;
		}
	}
	if (DoRestore) {
		used = 0;
		for (i=0;i<MemStatus.MemoryUsed+MemStatus.MemoryFree;i++) {
			Pbk.MemoryType 	= MEM_SM;
			Pbk.Location	= i + 1;
			Pbk.EntriesNum	= 0;
			if (used<max) {
				if (Backup.SIMPhonebook[used]->Location == Pbk.Location) {
					Pbk = *Backup.SIMPhonebook[used];
					used++;
					dbgprintf("Location %i\n",Pbk.Location);
					if (Pbk.EntriesNum != 0) error=Phone->SetMemory(&s, &Pbk);
				}
			}
			if (Pbk.EntriesNum == 0) error=Phone->DeleteMemory(&s, &Pbk);
			Print_Error(error);
			printmsgerr("%cWriting: %i percent",13,(i+1)*100/(MemStatus.MemoryUsed+MemStatus.MemoryFree));
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}

	DoRestore = false;
	if (Backup.CallerLogos[0] != NULL) {
		Bitmap.Type 	= GSM_CallerGroupLogo;
		Bitmap.Location = 1;
		error=Phone->GetBitmap(&s,&Bitmap);
		if (error == ERR_NONE) {
			if (answer_yes("Restore caller groups and logos")) DoRestore = true;
		}
	}
	if (DoRestore) {
		max = 0;
		while (Backup.CallerLogos[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			error=Phone->SetBitmap(&s,Backup.CallerLogos[i]);
			Print_Error(error);
			printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}

	if (!mystrncasecmp(s.CurrentConfig->SyncTime,"yes",0)) {
		if (answer_yes("Do you want to set date/time in phone (NOTE: in some phones it's required to correctly restore calendar notes and other items)")) {
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
			printmsgerr("%i entries in backup file\n",max);
			if (answer_yes("Restore calendar notes")) {
				Past    = answer_yes("Restore notes from the past");
				DoRestore = true;
			}
		}
	}
	if (DoRestore) {
		printmsgerr("Deleting old notes: ");
		error = Phone->DeleteAllCalendar(&s);
		if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
 			while (1) {
				error = Phone->GetNextCalendar(&s,&Calendar,true);
				if (error != ERR_NONE) break;
				error = Phone->DeleteCalendar(&s,&Calendar);
 				Print_Error(error);
				printmsgerr("*");
			}
			printmsgerr("\n");
		} else {
			printmsgerr("Done\n");
			Print_Error(error);
		}

		for (i=0;i<max;i++) {
			if (!Past && IsCalendarNoteFromThePast(Backup.Calendar[i])) continue;

			Calendar = *Backup.Calendar[i];
			error=Phone->AddCalendar(&s,&Calendar);
			Print_Error(error);
			printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}

	DoRestore = false;
	if (Backup.ToDo[0] != NULL) {
		error = Phone->GetToDoStatus(&s,&ToDoStatus);
		if (error == ERR_NONE) {
			max = 0;
			while (Backup.ToDo[max]!=NULL) max++;
			printmsgerr("%i entries in backup file\n",max);

			if (answer_yes("Restore ToDo")) DoRestore = true;
		}
	}
	if (DoRestore) {
		ToDo  = *Backup.ToDo[0];
		error = Phone->SetToDo(&s,&ToDo);
	}
	if (DoRestore && (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED)) {
		printmsgerr("Deleting old ToDo: ");
		error=Phone->DeleteAllToDo(&s);
		if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
			while (1) {
				error = Phone->GetNextToDo(&s,&ToDo,true);
				if (error != ERR_NONE) break;
				error = Phone->DeleteToDo(&s,&ToDo);
 				Print_Error(error);
				printmsgerr("*");
			}
			printmsgerr("\n");
		} else {
			printmsgerr("Done\n");
			Print_Error(error);
		}

		for (i=0;i<max;i++) {
			ToDo 		= *Backup.ToDo[i];
			ToDo.Location 	= 0;
			error=Phone->AddToDo(&s,&ToDo);
			Print_Error(error);
			printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
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
			printmsgerr("%cCleaning: %i percent",13,used*100/ToDoStatus.Used);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");

		/* Now write modified/new entries */
		for (i=0;i<max;i++) {
			ToDo  = *Backup.ToDo[i];
			error = Phone->SetToDo(&s,&ToDo);
			Print_Error(error);
			printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
 	}

	DoRestore = false;
	if (Backup.Note[0] != NULL) {
		error = Phone->GetNotesStatus(&s,&ToDoStatus);
		if (error == ERR_NONE) {
			max = 0;
			while (Backup.Note[max]!=NULL) max++;
			printmsgerr("%i entries in backup file\n",max);

			if (answer_yes("Restore Notes")) DoRestore = true;
		}
	}
	if (DoRestore) {
		printmsgerr("Deleting old Notes: ");
		while (1) {
			error = Phone->GetNextNote(&s,&Note,true);
			if (error != ERR_NONE) break;
			error = Phone->DeleteNote(&s,&Note);
 			Print_Error(error);
			printmsgerr("*");
		}
		printmsgerr("\n");

		for (i=0;i<max;i++) {
			Note 		= *Backup.Note[i];
			Note.Location 	= 0;
			error=Phone->AddNote(&s,&Note);
			Print_Error(error);
			printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}

	if (Backup.SMSC[0] != NULL && answer_yes("Restore SMSC profiles")) {
		max = 0;
		while (Backup.SMSC[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			error=Phone->SetSMSC(&s,Backup.SMSC[i]);
			Print_Error(error);
			printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}
	if (Backup.StartupLogo != NULL && answer_yes("Restore startup logo/text")) {
		error=Phone->SetBitmap(&s,Backup.StartupLogo);
		Print_Error(error);
	}
	if (Backup.OperatorLogo != NULL && answer_yes("Restore operator logo")) {
		error=Phone->SetBitmap(&s,Backup.OperatorLogo);
		Print_Error(error);
	}
	DoRestore = false;
	if (Backup.WAPBookmark[0] != NULL) {
		Bookmark.Location = 1;
		error = Phone->GetWAPBookmark(&s,&Bookmark);
		if (error == ERR_NONE || error == ERR_INVALIDLOCATION) {
			if (answer_yes("Restore WAP bookmarks")) DoRestore = true;
		}
	}
	if (DoRestore) {
		printmsgerr("Deleting old bookmarks: ");
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
			printmsgerr("*");
		}
		printmsgerr("\n");
		max = 0;
		while (Backup.WAPBookmark[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			Bookmark 	  = *Backup.WAPBookmark[i];
			Bookmark.Location = 0;
			error=Phone->SetWAPBookmark(&s,&Bookmark);
			Print_Error(error);
			printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}
	DoRestore = false;
	if (Backup.WAPSettings[0] != NULL) {
		Settings.Location = 1;
		error = Phone->GetWAPSettings(&s,&Settings);
		if (error == ERR_NONE) {
			if (answer_yes("Restore WAP settings")) DoRestore = true;
		}
	}
	if (DoRestore) {
		max = 0;
		while (Backup.WAPSettings[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			error=Phone->SetWAPSettings(&s,Backup.WAPSettings[i]);
			Print_Error(error);
			printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}
	DoRestore = false;
	if (Backup.MMSSettings[0] != NULL) {
		Settings.Location = 1;
		error = Phone->GetMMSSettings(&s,&Settings);
		if (error == ERR_NONE) {
			if (answer_yes("Restore MMS settings")) DoRestore = true;
		}
	}
	if (DoRestore) {
		max = 0;
		while (Backup.MMSSettings[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			error=Phone->SetMMSSettings(&s,Backup.MMSSettings[i]);
			Print_Error(error);
			printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}
	DoRestore = false;
	if (Backup.Ringtone[0] != NULL) {
		Ringtone.Location 	= 1;
		Ringtone.Format		= 0;
		error = Phone->GetRingtone(&s,&Ringtone,false);
		if (error == ERR_NONE || error ==ERR_EMPTY) {
			if (Phone->DeleteUserRingtones != NOTSUPPORTED) {
				if (answer_yes("Delete all user ringtones")) DoRestore = true;
			}
		}
	}
	if (DoRestore) {
		printmsgerr("Deleting: ");
		error=Phone->DeleteUserRingtones(&s);
		Print_Error(error);
		printmsgerr("Done\n");
		DoRestore = false;
		if (answer_yes("Restore user ringtones")) DoRestore = true;
	}
	if (DoRestore) {
		max = 0;
		while (Backup.Ringtone[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			error=GSM_RingtoneConvert(&Ringtone, Backup.Ringtone[i], Ringtone.Format);
			Print_Error(error);
			error=Phone->SetRingtone(&s,&Ringtone,&i);
			Print_Error(error);
			printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}
	DoRestore = false;
	if (Backup.Profiles[0] != NULL) {
		Profile.Location = 1;
		error = Phone->GetProfile(&s,&Profile);
		if (error == ERR_NONE) {
			if (answer_yes("Restore profiles")) DoRestore = true;
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
		printmsgerr("\n");
	}
	DoRestore = false;
	if (Backup.FMStation[0] != NULL) {
		FMStation.Location = 1;
		error = Phone->GetFMStation(&s,&FMStation);
		if (error == ERR_NONE || error == ERR_EMPTY) {
			if (answer_yes("Restore FM stations")) DoRestore = true;
		}
	}
	if (DoRestore) {
		printmsgerr("Deleting old FM stations: ");
		error=Phone->ClearFMStations(&s);
		Print_Error(error);
		printmsgerr("Done\n");
		max = 0;
		while (Backup.FMStation[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			FMStation = *Backup.FMStation[i];
			error=Phone->SetFMStation(&s,&FMStation);
			Print_Error(error);
			printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}
	DoRestore = false;
	if (Backup.GPRSPoint[0] != NULL) {
		GPRSPoint.Location = 1;
		error = Phone->GetGPRSAccessPoint(&s,&GPRSPoint);
		if (error == ERR_NONE || error == ERR_EMPTY) {
			if (answer_yes("Restore GPRS Points")) DoRestore = true;
		}
	}
	if (DoRestore) {
		max = 0;
		while (Backup.GPRSPoint[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			error=Phone->SetGPRSAccessPoint(&s,Backup.GPRSPoint[i]);
			Print_Error(error);
			printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		printmsgerr("\n");
	}

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
	printmsgerr("Press Ctrl+C to break...\n");

	if (Backup.DateTimeAvailable) 	printmsgerr("Time of backup : %s\n",OSDateTime(Backup.DateTime,false));
	if (Backup.Model[0]!=0) 	printmsgerr("Phone          : %s\n",Backup.Model);
	if (Backup.IMEI[0]!=0) 		printmsgerr("IMEI           : %s\n",Backup.IMEI);

	GSM_Init(true);

	if (Backup.PhonePhonebook[0] != NULL) {
		MemStatus.MemoryType = MEM_ME;
		error=Phone->GetMemoryStatus(&s, &MemStatus);
		if (error==ERR_NONE) {
			max = 0;
			while (Backup.PhonePhonebook[max]!=NULL) max++;
			printmsgerr("%i entries in backup file\n",max);
			if (MemStatus.MemoryFree < max) {
				printmsgerr("Memory has only %i free locations.Exiting\n",MemStatus.MemoryFree);
			} else if (answer_yes("Add phone phonebook entries")) {
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
						Print_Error(error);
						printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
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
						printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
						if (gshutdown) {
							GSM_Terminate();
							exit(0);
						}
					}
				}
				printmsgerr("\n");
			}
		}
	}
	if (Backup.SIMPhonebook[0] != NULL) {
		MemStatus.MemoryType = MEM_SM;
		error=Phone->GetMemoryStatus(&s, &MemStatus);
		if (error==ERR_NONE) {
			max = 0;
			while (Backup.SIMPhonebook[max]!=NULL) max++;
			printmsgerr("%i entries in backup file\n",max);
			if (MemStatus.MemoryFree < max) {
				printmsgerr("Memory has only %i free locations.Exiting\n",MemStatus.MemoryFree);
			} else if (answer_yes("Add SIM phonebook entries")) {
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
						printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
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
						printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
						if (gshutdown) {
							GSM_Terminate();
							exit(0);
						}
					}
				}
				printmsgerr("\n");
			}
		}
	}

	if (!mystrncasecmp(s.CurrentConfig->SyncTime,"yes",0)) {
		if (answer_yes("Do you want to set date/time in phone (NOTE: in some phones it's required to correctly restore calendar notes and other items)")) {
			GSM_GetCurrentDateTime(&date_time);

			error=Phone->SetDateTime(&s, &date_time);
			Print_Error(error);
		}
	}
	if (Backup.Calendar[0] != NULL) {
		error = Phone->GetNextCalendar(&s,&Calendar,true);
		if (error == ERR_NONE || error == ERR_INVALIDLOCATION || error == ERR_EMPTY) {
			if (answer_yes("Add calendar notes")) {
				max = 0;
				while (Backup.Calendar[max]!=NULL) max++;
				for (i=0;i<max;i++) {
					Calendar = *Backup.Calendar[i];
					error=Phone->AddCalendar(&s,&Calendar);
					Print_Error(error);
					printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
					if (gshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				printmsgerr("\n");
			}
		}
	}
	if (Backup.ToDo[0] != NULL) {
		ToDo.Location = 1;
		error=Phone->GetToDoStatus(&s,&ToDoStatus);
		if (error == ERR_NONE) {
			if (answer_yes("Add ToDo")) {
				max = 0;
				while (Backup.ToDo[max]!=NULL) max++;
				for (i=0;i<max;i++) {
					ToDo  = *Backup.ToDo[i];
					error = Phone->AddToDo(&s,&ToDo);
					Print_Error(error);
					printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
					if (gshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				printmsgerr("\n");
			}
		}
	}
	if (Backup.WAPBookmark[0] != NULL) {
		Bookmark.Location = 1;
		error = Phone->GetWAPBookmark(&s,&Bookmark);
		if (error == ERR_NONE || error == ERR_INVALIDLOCATION) {
			if (answer_yes("Add WAP bookmarks")) {
				max = 0;
				while (Backup.WAPBookmark[max]!=NULL) max++;
				for (i=0;i<max;i++) {
					Bookmark 	  = *Backup.WAPBookmark[i];
					Bookmark.Location = 0;
					error=Phone->SetWAPBookmark(&s,&Bookmark);
					Print_Error(error);
					printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
					if (gshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				printmsgerr("\n");
			}
		}
	}

	GSM_Terminate();
}
#endif

static void ClearAll(int argc, char *argv[])
{
	GSM_MemoryStatus	MemStatus;
	GSM_ToDoStatus		ToDoStatus;
	GSM_CalendarEntry	Calendar;
	GSM_ToDoEntry		ToDo;
	GSM_NoteEntry		Note;
	GSM_WAPBookmark		Bookmark;
	GSM_FMStation 		Station;
	GSM_MemoryEntry		Pbk;
	bool			DoClear;

	GSM_Init(true);

	DoClear 		= false;
	MemStatus.MemoryType 	= MEM_ME;
	error=Phone->GetMemoryStatus(&s, &MemStatus);
	if (error==ERR_NONE && MemStatus.MemoryUsed !=0) {
		if (answer_yes("Delete phone phonebook")) DoClear = true;
	}
	if (DoClear) {
		error = Phone->DeleteAllMemory(&s,MEM_ME);
		if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
			for (i=0;i<MemStatus.MemoryUsed+MemStatus.MemoryFree;i++) {
				Pbk.MemoryType 	= MEM_ME;
				Pbk.Location	= i + 1;
				Pbk.EntriesNum	= 0;
				error=Phone->DeleteMemory(&s, &Pbk);
				Print_Error(error);
				printmsgerr("%cWriting: %i percent",13,(i+1)*100/(MemStatus.MemoryUsed+MemStatus.MemoryFree));
				if (gshutdown) {
					GSM_Terminate();
					exit(0);
				}
			}
			printmsgerr("\n");
		} else {
			printmsgerr("Done\n");
			Print_Error(error);
		}
	}

	DoClear 		= false;
	MemStatus.MemoryType 	= MEM_SM;
	error=Phone->GetMemoryStatus(&s, &MemStatus);
	if (error==ERR_NONE && MemStatus.MemoryUsed !=0) {
		if (answer_yes("Delete SIM phonebook")) DoClear = true;
	}
	if (DoClear) {
		error = Phone->DeleteAllMemory(&s,MEM_SM);
		if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
			for (i=0;i<MemStatus.MemoryUsed+MemStatus.MemoryFree;i++) {
				Pbk.MemoryType 	= MEM_SM;
				Pbk.Location	= i + 1;
				Pbk.EntriesNum	= 0;
				error=Phone->DeleteMemory(&s, &Pbk);
				Print_Error(error);
				printmsgerr("%cWriting: %i percent",13,(i+1)*100/(MemStatus.MemoryUsed+MemStatus.MemoryFree));
				if (gshutdown) {
					GSM_Terminate();
					exit(0);
				}
			}
			printmsgerr("\n");
		} else {
			printmsgerr("Done\n");
			Print_Error(error);
		}
	}

	DoClear = false;
	error = Phone->GetNextCalendar(&s,&Calendar,true);
	if (error == ERR_NONE) {
 		if (answer_yes("Delete calendar notes")) DoClear = true;
	}
	if (DoClear) {
		printmsgerr("Deleting: ");
		error=Phone->DeleteAllCalendar(&s);
		if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
 			while (1) {
				error = Phone->GetNextCalendar(&s,&Calendar,true);
				if (error != ERR_NONE) break;
				error = Phone->DeleteCalendar(&s,&Calendar);
 				Print_Error(error);
				printmsgerr("*");
			}
			printmsgerr("\n");
		} else {
			printmsgerr("Done\n");
			Print_Error(error);
		}
	}

	DoClear = false;
	error = Phone->GetToDoStatus(&s,&ToDoStatus);
	if (error == ERR_NONE && ToDoStatus.Used != 0) {
		if (answer_yes("Delete ToDo")) DoClear = true;
	}
	if (DoClear) {
		printmsgerr("Deleting: ");
		error=Phone->DeleteAllToDo(&s);
		if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
 			while (1) {
				error = Phone->GetNextToDo(&s,&ToDo,true);
				if (error != ERR_NONE) break;
				error = Phone->DeleteToDo(&s,&ToDo);
 				Print_Error(error);
				printmsgerr("*");
			}
			printmsgerr("\n");
		} else {
			printmsgerr("Done\n");
			Print_Error(error);
		}
	}

	DoClear = false;
	error = Phone->GetNotesStatus(&s,&ToDoStatus);
	if (error == ERR_NONE && ToDoStatus.Used != 0) {
		if (answer_yes("Delete Notes")) DoClear = true;
	}
	if (DoClear) {
		printmsgerr("Deleting: ");
		while (1) {
			error = Phone->GetNextNote(&s,&Note,true);
			if (error != ERR_NONE) break;
			error = Phone->DeleteNote(&s,&Note);
			Print_Error(error);
			printmsgerr("*");
		}
		printmsgerr("\n");
	}

	Bookmark.Location = 1;
	error = Phone->GetWAPBookmark(&s,&Bookmark);
	if (error == ERR_NONE || error == ERR_INVALIDLOCATION) {
		if (answer_yes("Delete WAP bookmarks")) {
			printmsgerr("Deleting: ");
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
				printmsgerr("*");
			}
			printmsgerr("\n");
		}
	}
	if (Phone->DeleteUserRingtones != NOTSUPPORTED) {
		if (answer_yes("Delete all user ringtones")) {
			printmsgerr("Deleting: ");
			error=Phone->DeleteUserRingtones(&s);
			Print_Error(error);
			printmsgerr("Done\n");
		}
	}
	Station.Location=i;
	error=Phone->GetFMStation(&s,&Station);
	if (error == ERR_NONE || error == ERR_EMPTY) {
	 	if (answer_yes("Delete all FM station")) {
 			error=Phone->ClearFMStations(&s);
 			Print_Error(error);
		}
 	}

	GSM_Terminate();
}

static void DisplayConnectionSettings(GSM_MultiWAPSettings *settings,int j)
{
	if (settings->Settings[j].IsContinuous) {
		printmsg("Connection type     : Continuous\n");
	} else {
		printmsg("Connection type     : Temporary\n");
	}
	if (settings->Settings[j].IsSecurity) {
		printmsg("Connection security : On\n");
	} else {
		printmsg("Connection security : Off\n");
	}
	printmsg("Proxy               : address \"%s\", port %i\n",DecodeUnicodeConsole(settings->Proxy),settings->ProxyPort);
	printmsg("2'nd proxy          : address \"%s\", port %i\n",DecodeUnicodeConsole(settings->Proxy2),settings->Proxy2Port);
	switch (settings->Settings[j].Bearer) {
	case WAPSETTINGS_BEARER_SMS:
		printmsg("Bearer              : SMS");
		if (settings->ActiveBearer == WAPSETTINGS_BEARER_SMS) printf(" (active)");
		printmsg("\nServer number       : \"%s\"\n",DecodeUnicodeConsole(settings->Settings[j].Server));
		printmsg("Service number      : \"%s\"\n",DecodeUnicodeConsole(settings->Settings[j].Service));
		break;
	case WAPSETTINGS_BEARER_DATA:
		printmsg("Bearer              : Data (CSD)");
		if (settings->ActiveBearer == WAPSETTINGS_BEARER_DATA) printf(" (active)");
		printmsg("\nDial-up number      : \"%s\"\n",DecodeUnicodeConsole(settings->Settings[j].DialUp));
		printmsg("IP address          : \"%s\"\n",DecodeUnicodeConsole(settings->Settings[j].IPAddress));
		if (settings->Settings[j].ManualLogin) {
			printmsg("Login Type          : Manual\n");
		} else {
			printmsg("Login Type          : Automatic\n");
		}
		if (settings->Settings[j].IsNormalAuthentication) {
			printmsg("Authentication type : Normal\n");
		} else {
			printmsg("Authentication type : Secure\n");
		}
		if (settings->Settings[j].IsISDNCall) {
			printmsg("Data call type      : ISDN\n");
              	} else {
			printmsg("Data call type      : Analogue\n");
		}
		switch (settings->Settings[j].Speed) {
			case WAPSETTINGS_SPEED_9600  : printmsg("Data call speed     : 9600\n");  break;
			case WAPSETTINGS_SPEED_14400 : printmsg("Data call speed     : 14400\n"); break;
			case WAPSETTINGS_SPEED_AUTO  : printmsg("Data call speed     : Auto\n");  break;
		}
		printmsg("User name           : \"%s\"\n",DecodeUnicodeConsole(settings->Settings[j].User));
		printmsg("Password            : \"%s\"\n",DecodeUnicodeConsole(settings->Settings[j].Password));
		break;
	case WAPSETTINGS_BEARER_USSD:
		printmsg("Bearer              : USSD");
		if (settings->ActiveBearer == WAPSETTINGS_BEARER_USSD) printf(" (active)");
		printmsg("\nService code        : \"%s\"\n",DecodeUnicodeConsole(settings->Settings[j].Code));
		if (settings->Settings[j].IsIP) {
			printmsg("Address type        : IP address\nIPaddress           : \"%s\"\n",DecodeUnicodeConsole(settings->Settings[j].Service));
		} else {
			printmsg("Address type        : Service number\nService number      : \"%s\"\n",DecodeUnicodeConsole(settings->Settings[j].Service));
		}
		break;
	case WAPSETTINGS_BEARER_GPRS:
		printmsg("Bearer              : GPRS");
		if (settings->ActiveBearer == WAPSETTINGS_BEARER_GPRS) printf(" (active)");
		if (settings->Settings[j].ManualLogin) {
			printmsg("\nLogin Type          : Manual\n");
		} else {
			printmsg("\nLogin Type          : Automatic\n");
		}
		if (settings->Settings[j].IsNormalAuthentication) {
			printmsg("Authentication type : Normal\n");
		} else {
			printmsg("Authentication type : Secure\n");
		}
		printmsg("Access point        : \"%s\"\n",DecodeUnicodeConsole(settings->Settings[j].DialUp));
		printmsg("IP address          : \"%s\"\n",DecodeUnicodeConsole(settings->Settings[j].IPAddress));
		printmsg("User name           : \"%s\"\n",DecodeUnicodeConsole(settings->Settings[j].User));
		printmsg("Password            : \"%s\"\n",DecodeUnicodeConsole(settings->Settings[j].Password));
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
		printmsg("%i. ",i);
		if (settings.Name[0]==0 && settings.Name[1]==0) {
			printmsg("Set %i",i);
		} else {
			printmsg("%s",DecodeUnicodeConsole(settings.Name));
		}
		if (settings.Active) printmsg(" (active)");
//		if (settings.ReadOnly) printmsg("\nRead only           : yes");
		printmsg("\n");
		printmsg("User                : \"%s\"\n",DecodeUnicodeConsole(settings.User));
		printmsg("Password            : \"%s\"\n",DecodeUnicodeConsole(settings.Password));
		printmsg("Phonebook database  : \"%s\"\n",DecodeUnicodeConsole(settings.PhonebookDataBase));
		printmsg("Calendar database   : \"%s\"\n",DecodeUnicodeConsole(settings.CalendarDataBase));
		printmsg("Server              : \"%s\"\n",DecodeUnicodeConsole(settings.Server));
		printmsg("Sync. phonebook     : ");
		if (settings.SyncPhonebook) printmsg("yes\n");
		if (!settings.SyncPhonebook) printmsg("no\n");
		printmsg("Sync. calendar      : ");
		if (settings.SyncCalendar) printmsg("yes\n");
		if (!settings.SyncCalendar) printmsg("no\n");
		printmsg("\n");
		for (j=0;j<settings.Connection.Number;j++) {
			if (settings.Connection.Settings[j].Title[0]==0 && settings.Connection.Settings[j].Title[1]==0) {
				printmsg("Connection set name : Set %i\n",i);
			} else {
				printmsg("Connection set name : %s\n",DecodeUnicodeConsole(settings.Connection.Settings[j].Title));
			}
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
		printmsg("%i. ",i);
		if (settings.Name[0]==0 && settings.Name[1]==0) {
			printmsg("Set %i",i);
		} else {
			printmsg("%s",DecodeUnicodeConsole(settings.Name));
		}
		if (settings.Active) printmsg(" (active)");
//		if (settings.ReadOnly) printmsg("\nRead only           : yes");
		printmsg("\n");
		printmsg("Homepage            : \"%s\"\n",DecodeUnicodeConsole(settings.HomePage));
		printmsg("User                : \"%s\"\n",DecodeUnicodeConsole(settings.User));
		printmsg("Password            : \"%s\"\n",DecodeUnicodeConsole(settings.Password));
		printmsg("\n");
		for (j=0;j<settings.Connection.Number;j++) {
			if (settings.Connection.Settings[j].Title[0]==0 && settings.Connection.Settings[j].Title[1]==0) {
				printmsg("Connection set name : Set %i\n",i);
			} else {
				printmsg("Connection set name : %s\n",DecodeUnicodeConsole(settings.Connection.Settings[j].Title));
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
		if (mystrncasecmp(argv[1],"--getwapsettings",0)) {
			error=Phone->GetWAPSettings(&s,&settings);
		} else {
			error=Phone->GetMMSSettings(&s,&settings);
		}
		Print_Error(error);
		for (j=0;j<settings.Number;j++) {
			printmsg("%i. ",i);
			if (settings.Settings[j].Title[0]==0 && settings.Settings[j].Title[1]==0) {
				printmsg("Set %i",i);
			} else {
				printmsg("%s",DecodeUnicodeConsole(settings.Settings[j].Title));
			}
			if (settings.Active) printmsg(" (active)");
			if (settings.ReadOnly) printmsg("\nRead only           : yes");
			printmsg("\nHomepage            : \"%s\"\n",DecodeUnicodeConsole(settings.Settings[j].HomePage));
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

	DeleteAfter=answer_yes("Delete each sms after backup");

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
							Backup.SMS[smsnum+1] = NULL;
						} else {
							printmsg("   Increase %s\n" , "GSM_BACKUP_MAX_SMS");
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
			printmsgerr("%cDeleting: %i percent",13,(j+1)*100/smsnum);
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
		displaymultismsinfo(SMS,false,false);
		if (answer_yes("Restore sms")) {
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

	sprintf(buffer,"Do you want to restore binary SMS");
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
			displaymultismsinfo(SMS,false,false);
			sprintf(buffer,"Restore %03i sms to folder \"%s\"",smsnum+1,DecodeUnicodeConsole(folders.Folder[Backup.SMS[smsnum]->Folder-1].Name));
			if (folders.Folder[Backup.SMS[smsnum]->Folder-1].Memory == MEM_SM) strcat(buffer," (SIM)");
			if (answer_yes(buffer)) {
				smprintf(&s,"saving %i SMS\n",smsnum);
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
				case GSM_StartupLogo :	printmsg("Startup logo");		break;
				case GSM_OperatorLogo:	printmsg("Operator logo"); 	break;
				case GSM_PictureImage:	printmsg("Picture Image"); 	break;
				case GSM_CallerGroupLogo  :	printmsg("Caller group logo"); 	break;
				default		     : 					break;
			}
			printmsg(", width %i, height %i\n",Bitmap.Bitmap[i].BitmapWidth,Bitmap.Bitmap[i].BitmapHeight);
			GSM_PrintBitmap(stdout,&Bitmap.Bitmap[i]);
		}
	} else {
		if (argc == 5) {
			for (i=0;i<Bitmap.Number;i++) {
				if (mystrncasecmp(argv[4],"PICTURE",0)) {
					Bitmap.Bitmap[i].Type = GSM_PictureImage;
				} else if (mystrncasecmp(argv[4],"STARTUP",0)) {
					Bitmap.Bitmap[i].Type = GSM_StartupLogo;
				} else if (mystrncasecmp(argv[4],"CALLER",0)) {
					Bitmap.Bitmap[i].Type = GSM_CallerGroupLogo;
				} else if (mystrncasecmp(argv[4],"OPERATOR",0)) {
					Bitmap.Bitmap[i].Type = GSM_OperatorLogo;
				} else {
					printmsg("What format of output file logo (\"%s\") ?\n",argv[4]);
					exit(-1);
				}
			}
		}
		error=GSM_SaveBitmapFile(argv[3],&Bitmap);
		Print_Error(error);
	}
}

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
		printmsg("It can be RTTL ringtone only used with this option\n");
		exit(-1);
	}

	started = false;
	j	= 0;
	for (i=0;i<ringtone.NoteTone.NrCommands;i++) {
		if (ringtone.NoteTone.Commands[i].Type == RING_Note) {
			Note = &ringtone.NoteTone.Commands[i].Note;
			if (!started) {
				if (Note->Note != Note_Pause) {
					printmsg("Ringtone \"%s\" (tempo = %i Beats Per Minute)\n\n",DecodeUnicodeConsole(ringtone.Name),GSM_RTTLGetTempo(Note->Tempo));
					started = true;
				}
			}
			if (started) j++;
		}
	}
    	if (j>50) printmsg("WARNING: LENGTH=%i NOTES, BUT YOU WILL ENTER ONLY FIRST 50 TONES.",j);

	printmsg("\n\nThis ringtone in Nokia Composer in phone should look: ");
	started = false;
	for (i=0;i<ringtone.NoteTone.NrCommands;i++) {
		if (ringtone.NoteTone.Commands[i].Type == RING_Note) {
			Note = &ringtone.NoteTone.Commands[i].Note;
			if (!started) {
				if (Note->Note != Note_Pause) started = true;
			}
			if (started) {
				switch (Note->Duration) {
					case Duration_Full: printmsg("1"); break;
					case Duration_1_2 : printmsg("2"); break;
					case Duration_1_4 : printmsg("4"); break;
					case Duration_1_8 : printmsg("8"); break;
					case Duration_1_16: printmsg("16");break;
					case Duration_1_32: printmsg("32");break;
				}
				if (Note->DurationSpec == DottedNote) printmsg(".");
				switch (Note->Note) {
					case Note_C  	: printmsg("c");	break;
					case Note_Cis	: printmsg("#c");	break;
					case Note_D  	 :printmsg("d");	break;
					case Note_Dis	: printmsg("#d");	break;
					case Note_E  	: printmsg("e");	break;
					case Note_F  	: printmsg("f");	break;
					case Note_Fis	: printmsg("#f");	break;
					case Note_G  	: printmsg("g");	break;
					case Note_Gis	: printmsg("#g");	break;
					case Note_A  	: printmsg("a");	break;
					case Note_Ais	: printmsg("#a");	break;
					case Note_H  	: printmsg("h");	break;
					case Note_Pause : printmsg("-");	break;
				}
				if (Note->Note != Note_Pause) printmsg("%i",Note->Scale - 4);
				printmsg(" ");
			}
		}
	}

	printmsg("\n\nTo enter it please press: ");
	started = false;
	for (i=0;i<ringtone.NoteTone.NrCommands;i++) {
		if (ringtone.NoteTone.Commands[i].Type == RING_Note) {
			Note = &ringtone.NoteTone.Commands[i].Note;
			if (!started) {
				if (Note->Note != Note_Pause) started = true;
			}
			if (started) {
				switch (Note->Note) {
	      				case Note_C  : case Note_Cis:	printmsg("1");break;
	      				case Note_D  : case Note_Dis:	printmsg("2");break;
	      				case Note_E  :			printmsg("3");break;
	      				case Note_F  : case Note_Fis:	printmsg("4");break;
	      				case Note_G  : case Note_Gis:	printmsg("5");break;
	      				case Note_A  : case Note_Ais:	printmsg("6");break;
	      				case Note_H  :			printmsg("7");break;
	      				default      :			printmsg("0");break;
				}
				if (Note->DurationSpec == DottedNote) printmsg("(longer)");
	    			switch (Note->Note) {
      					case Note_Cis: case Note_Dis:
      					case Note_Fis: case Note_Gis:
      					case Note_Ais:
						printmsg("#");
						break;
      					default      :
						break;
    				}
				if (Note->Note != Note_Pause) {
					if ((unsigned int)Note->Scale != DefNoteScale) {
						while (DefNoteScale != (unsigned int)Note->Scale) {
							printmsg("*");
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
						printmsg("9");
			  			DefNoteDuration = DefNoteDuration * 2;
					}
			      	}
				if (Duration < DefNoteDuration) {
		        		while (DefNoteDuration != Duration) {
						printmsg("8");
			  			DefNoteDuration = DefNoteDuration / 2;
					}
			      	}
				printmsg(" ");
			}
		}
	}

	printf("\n");
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
		if (mystrncasecmp(argv[4],"RTTL",0)) {		Format = RING_NOTETONE;
		} else if (mystrncasecmp(argv[4],"BINARY",0)) {	Format = RING_NOKIABINARY;
		} else {
			printmsg("What format of output ringtone file (\"%s\") ?\n",argv[4]);
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
		printmsg("Unknown key/function name: \"%c\"\n",argv[2][Length]);
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

	if (mystrncasecmp(argv[2],"TODO",0)) {
        	Category.Type 	= Category_ToDo;
        	Status.Type 	= Category_ToDo;
	} else if (mystrncasecmp(argv[2],"PHONEBOOK",0)) {
	        Category.Type 	= Category_Phonebook;
        	Status.Type 	= Category_Phonebook;
	} else {
		printmsg("What type of categories do you want to get (\"%s\") ?\n",argv[2]);
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
			printmsg("Location: %i\n",j);

			Print_Error(error);

			printmsg("Name    : \"%s\"\n\n",DecodeUnicodeConsole(Category.Name));
			count++;
    		}
	}

	GSM_Terminate();
}

static void GetCategory(int argc, char *argv[])
{
	GSM_Category	Category;
	int		start,stop,j;

	if (mystrncasecmp(argv[2],"TODO",0)) {
    		Category.Type = Category_ToDo;
	} else if (mystrncasecmp(argv[2],"PHONEBOOK",0)) {
    		Category.Type = Category_Phonebook;
	} else {
		printmsg("What type of categories do you want to get (\"%s\") ?\n",argv[2]);
		exit(-1);
	}

	GetStartStop(&start, &stop, 2, argc - 1, argv + 1);

	GSM_Init(true);

	for (j=start;j<=stop;j++)
	{
    		printmsg("Location: %i\n",j);

		Category.Location=j;

		error=Phone->GetCategory(&s, &Category);
		if (error != ERR_EMPTY) Print_Error(error);

		if (error == ERR_EMPTY) {
			printmsg("Entry is empty\n\n");
		} else {
        		printmsg("Name    : \"%s\"\n\n",DecodeUnicodeConsole(Category.Name));
    		}
	}

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
		printmsg("Location  : %i\n",i);
		error=Phone->DeleteToDo(&s,&ToDo);
		if (error != ERR_EMPTY) Print_Error(error);

		if (error == ERR_EMPTY) {
			printmsg("Entry was empty\n");
		} else {
			printmsg("Entry was deleted\n");
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

	printmsg("Location     : %i\n",ToDo->Location);
	printmsg("Priority     : ");
	switch (ToDo->Priority) {
		case GSM_Priority_Low	 : printmsg("Low\n");	 	break;
		case GSM_Priority_Medium : printmsg("Medium\n"); 	break;
		case GSM_Priority_High	 : printmsg("High\n");		break;
		default			 : printmsg("Unknown\n");	break;
	}
	for (j=0;j<ToDo->EntriesNum;j++) {
		switch (ToDo->Entries[j].EntryType) {
		case TODO_END_DATETIME:
			printmsg("DueTime      : %s\n",OSDateTime(ToDo->Entries[j].Date,false));
			break;
		case TODO_COMPLETED:
			printmsg("Completed    : %s\n",ToDo->Entries[j].Number == 1 ? "Yes" : "No");
			break;
		case TODO_ALARM_DATETIME:
			printmsg("Alarm        : %s\n",OSDateTime(ToDo->Entries[j].Date,false));
			break;
		case TODO_SILENT_ALARM_DATETIME:
			printmsg("Silent alarm : %s\n",OSDateTime(ToDo->Entries[j].Date,false));
			break;
		case TODO_TEXT:
			printmsg("Text         : \"%s\"\n",DecodeUnicodeConsole(ToDo->Entries[j].Text));
			break;
		case TODO_PRIVATE:
			printmsg("Private      : %s\n",ToDo->Entries[j].Number == 1 ? "Yes" : "No");
			break;
		case TODO_CATEGORY:
			Category.Location = ToDo->Entries[j].Number;
			Category.Type = Category_ToDo;
			error=Phone->GetCategory(&s, &Category);
			if (error == ERR_NONE) {
				printmsg("Category     : \"%s\" (%i)\n", DecodeUnicodeConsole(Category.Name), ToDo->Entries[j].Number);
			} else {
				printmsg("Category     : %i\n", ToDo->Entries[j].Number);
			}
			break;
		case TODO_CONTACTID:
			entry.Location = ToDo->Entries[j].Number;
			entry.MemoryType = MEM_ME;
			error=Phone->GetMemory(&s, &entry);
			if (error == ERR_NONE) {
				name = GSM_PhonebookGetEntryName(&entry);
				if (name != NULL) {
					printmsg("Contact ID   : \"%s\" (%d)\n", DecodeUnicodeConsole(name), ToDo->Entries[j].Number);
				} else {
					printmsg("Contact ID   : %d\n",ToDo->Entries[j].Number);
				}
			} else {
				printmsg("Contact   : %d\n",ToDo->Entries[j].Number);
			}
			break;
		case TODO_PHONE:
			printmsg("Phone        : \"%s\"\n",DecodeUnicodeConsole(ToDo->Entries[j].Text));
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
	printmsgerr("Press Ctrl+C to break...\n");

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
			printmsg("Search text too long, truncating to %d chars!\n", GSM_MAX_CATEGORY_NAME_LENGTH);
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
	printmsgerr("Press Ctrl+C to break...\n");

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
	printmsgerr("Press Ctrl+C to break...\n");

	GSM_Init(true);

	while (!gshutdown) {
		error = Phone->GetNextNote(&s, &Note, start);
		if (error == ERR_EMPTY) break;
		Print_Error(error);
		printmsg("Text         : \"%s\"\n",DecodeUnicodeConsole(Note.Text));
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

	if (mystrncasecmp(argv[2],"PIN",0)) {		Code.Type = SEC_Pin;
	} else if (mystrncasecmp(argv[2],"PUK",0)) {	Code.Type = SEC_Puk;
	} else if (mystrncasecmp(argv[2],"PIN2",0)) {	Code.Type = SEC_Pin2;
	} else if (mystrncasecmp(argv[2],"PUK2",0)) {	Code.Type = SEC_Puk2;
	} else {
		printmsg("What security code (\"%s\") ?\n",argv[2]);
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

		printmsg("%i. \"%s\"",i,DecodeUnicodeConsole(Profile.Name));
		if (Profile.Active)		printmsg(" (active)");
		if (Profile.DefaultName) 	printmsg(" (default name)");
		if (Profile.HeadSetProfile) 	printmsg(" (HeadSet profile)");
		if (Profile.CarKitProfile) 	printmsg(" (CarKit profile)");
		printf("\n");
		for (j=0;j<Profile.FeaturesNumber;j++) {
			special = false;
			switch (Profile.FeatureID[j]) {
			case Profile_MessageToneID:
			case Profile_RingtoneID:
				special = true;
				if (Profile.FeatureID[j] == Profile_RingtoneID) {
					printmsg("Ringtone ID           : ");
				} else {
					printmsg("Message alert tone ID : ");
				}
				if (UnicodeLength(GSM_GetRingtoneName(&Info,Profile.FeatureValue[j]))!=0) {
					printmsg("\"%s\"\n",DecodeUnicodeConsole(GSM_GetRingtoneName(&Info,Profile.FeatureValue[j])));
				} else {
					printmsg("%i\n",Profile.FeatureValue[j]);
				}
				break;
			case Profile_CallerGroups:
				special = true;
				printmsg("Call alert for        :");
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
						printmsg(" \"%s\"",DecodeUnicodeConsole(caller[k].Text));
					}
				}
				printf("\n");
				break;
			case Profile_ScreenSaverNumber:
				special = true;
				printmsg("Screen saver number   : ");
				printmsg("%i\n",Profile.FeatureValue[j]);
				break;
			case Profile_CallAlert  	: printmsg("Incoming call alert   : "); break;
			case Profile_RingtoneVolume 	: printmsg("Ringtone volume       : "); break;
			case Profile_Vibration		: printmsg("Vibrating alert       : "); break;
			case Profile_MessageTone	: printmsg("Message alert tone    : "); break;
			case Profile_KeypadTone		: printmsg("Keypad tones          : "); break;
			case Profile_WarningTone	: printmsg("Warning (games) tones : "); break;
			case Profile_ScreenSaver	: printmsg("Screen saver          : "); break;
			case Profile_ScreenSaverTime	: printmsg("Screen saver timeout  : "); break;
			case Profile_AutoAnswer		: printmsg("Automatic answer      : "); break;
			case Profile_Lights		: printmsg("Lights                : "); break;
			default:
				printmsg("Unknown\n");
				special = true;
			}
			if (!special) {
				switch (Profile.FeatureValue[j]) {
				case PROFILE_VOLUME_LEVEL1 		:
				case PROFILE_KEYPAD_LEVEL1 		: printmsg("Level 1\n"); 		break;
				case PROFILE_VOLUME_LEVEL2 		:
				case PROFILE_KEYPAD_LEVEL2 		: printmsg("Level 2\n");		break;
				case PROFILE_VOLUME_LEVEL3 		:
				case PROFILE_KEYPAD_LEVEL3 		: printmsg("Level 3\n"); 		break;
				case PROFILE_VOLUME_LEVEL4 		: printmsg("Level 4\n"); 		break;
				case PROFILE_VOLUME_LEVEL5 		: printmsg("Level 5\n"); 		break;
				case PROFILE_MESSAGE_NOTONE 		:
				case PROFILE_AUTOANSWER_OFF		:
				case PROFILE_LIGHTS_OFF  		:
				case PROFILE_SAVER_OFF			:
				case PROFILE_WARNING_OFF		:
				case PROFILE_CALLALERT_OFF	 	:
				case PROFILE_VIBRATION_OFF 		:
				case PROFILE_KEYPAD_OFF	   		: printmsg("Off\n");	  	break;
				case PROFILE_CALLALERT_RINGING   	: printmsg("Ringing\n");		break;
				case PROFILE_CALLALERT_BEEPONCE  	:
				case PROFILE_MESSAGE_BEEPONCE 		: printmsg("Beep once\n"); 	break;
				case PROFILE_CALLALERT_RINGONCE  	: printmsg("Ring once\n");	break;
				case PROFILE_CALLALERT_ASCENDING 	: printmsg("Ascending\n");        break;
				case PROFILE_CALLALERT_CALLERGROUPS	: printmsg("Caller groups\n");	break;
				case PROFILE_MESSAGE_STANDARD 		: printmsg("Standard\n");  	break;
				case PROFILE_MESSAGE_SPECIAL 		: printmsg("Special\n");	 	break;
				case PROFILE_MESSAGE_ASCENDING		: printmsg("Ascending\n"); 	break;
				case PROFILE_MESSAGE_PERSONAL		: printmsg("Personal\n");		break;
				case PROFILE_AUTOANSWER_ON		:
				case PROFILE_WARNING_ON			:
				case PROFILE_SAVER_ON			:
				case PROFILE_VIBRATION_ON 		: printmsg("On\n");  		break;
				case PROFILE_VIBRATION_FIRST 		: printmsg("Vibrate first\n");	break;
				case PROFILE_LIGHTS_AUTO 		: printmsg("Auto\n"); 		break;
				case PROFILE_SAVER_TIMEOUT_5SEC	 	: printmsg("5 seconds\n"); 	break;
				case PROFILE_SAVER_TIMEOUT_20SEC 	: printmsg("20 seconds\n"); 	break;
				case PROFILE_SAVER_TIMEOUT_1MIN	 	: printmsg("1 minute\n");		break;
				case PROFILE_SAVER_TIMEOUT_2MIN	 	: printmsg("2 minutes\n");	break;
				case PROFILE_SAVER_TIMEOUT_5MIN	 	: printmsg("5 minutes\n");	break;
				case PROFILE_SAVER_TIMEOUT_10MIN 	: printmsg("10 minutes\n");	break;
				default					: printmsg("UNKNOWN\n");
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
		printmsg("%i.",i);
		switch (error) {
		case ERR_EMPTY:
			printmsg(" speed dial not assigned\n");
			break;
		default:
			Print_Error(error);

			Phonebook.Location	= SpeedDial.MemoryLocation;
			Phonebook.MemoryType 	= SpeedDial.MemoryType;
			error=Phone->GetMemory(&s,&Phonebook);

			GSM_PhonebookFindDefaultNameNumberGroup(&Phonebook, &Name, &Number, &Group);

			if (Name != -1) printmsg(" Name \"%s\",",DecodeUnicodeConsole(Phonebook.Entries[Name].Text));
			printmsg(" Number \"%s\"",DecodeUnicodeConsole(Phonebook.Entries[SpeedDial.MemoryNumberID-1].Text));
		}
		printf("\n");
	}

	GSM_Terminate();
}

static void ResetPhoneSettings(int argc, char *argv[])
{
	GSM_ResetSettingsType Type;

	if (mystrncasecmp(argv[2],"PHONE",0)) {		 Type = GSM_RESET_PHONESETTINGS;
	} else if (mystrncasecmp(argv[2],"UIF",0)) {	 Type = GSM_RESET_USERINTERFACE;
	} else if (mystrncasecmp(argv[2],"ALL",0)) {	 Type = GSM_RESET_USERINTERFACE_PHONESETTINGS;
	} else if (mystrncasecmp(argv[2],"DEV",0)) {	 Type = GSM_RESET_DEVICE;
	} else if (mystrncasecmp(argv[2],"FACTORY",0)) { Type = GSM_RESET_FULLFACTORY;
	} else {
		printmsg("What type of reset phone settings (\"%s\") ?\n",argv[2]);
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
		printmsg("Too high folder number (max. %i)\n",folders.Number);
		GSM_Terminate();
		exit(-1);
	}

	printmsg("Deleting SMS from \"%s\" folder: ",DecodeUnicodeConsole(folders.Folder[foldernum-1].Name));

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
				printmsg("*");
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

	printmsg("Current display features :\n");

	for (i=0;i<Features.Number;i++) {
		switch(Features.Feature[i]) {
		case GSM_CallActive	: printmsg("Call active\n");	break;
		case GSM_UnreadSMS	: printmsg("Unread SMS\n");	break;
		case GSM_VoiceCall	: printmsg("Voice call\n");	break;
		case GSM_FaxCall	: printmsg("Fax call\n");	break;
		case GSM_DataCall	: printmsg("Data call\n");	break;
		case GSM_KeypadLocked	: printmsg("Keypad locked\n");	break;
		case GSM_SMSMemoryFull	: printmsg("SMS memory full\n");
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
			if (!strncmp(GSM_Countries[i*2+1],argv[2],strlen(argv[2]))) {
				strcpy(country,GSM_Countries[i*2]);
				printmsg("Networks for %s:\n\n",GSM_Countries[i*2+1]);
				break;
			}
			i++;
		}
		if (!*country) {
			printmsg("Unknown country name.");
			exit(-1);
		}
	}
	printmsg("Network  Name\n");
	i=0;
	while (GSM_Networks[i*2]!=NULL) {
		if (argc>2) {
		        if (!strncmp(GSM_Networks[i*2],country,strlen(country))) {
				printmsg("%s   %s\n", GSM_Networks[i*2], GSM_Networks[i*2+1]);
			}
		} else {
			printmsg("%s   %s\n", GSM_Networks[i*2], GSM_Networks[i*2+1]);
		}
		i++;
	}
}

static void Version(int argc, char *argv[])
{
#ifdef DEBUG
	GSM_DateTime	dt;
#endif
//	unsigned char 	buff[10];
//	int		len;

	printmsg("[Gammu version %s built %s %s",VERSION,__TIME__,__DATE__);
	if (strlen(GetCompiler()) != 0) printmsg(" in %s",GetCompiler());
	printmsg("]\n\n");

#ifdef DEBUG
	printf("GSM_SMSMessage  - %i\n",sizeof(GSM_SMSMessage));
	printf("GSM_SMSC        - %i\n",sizeof(GSM_SMSC));
	printf("GSM_SMS_State   - %i\n",sizeof(GSM_SMS_State));
	printf("GSM_UDHHeader   - %i\n",sizeof(GSM_UDHHeader));
	printf("bool            - %i\n",sizeof(bool));
	printf("GSM_DateTime    - %i\n",sizeof(GSM_DateTime));
	printf("int             - %i\n",sizeof(int));
	printf("GSM_NetworkInfo - %i\n",sizeof(GSM_NetworkInfo));
	dt.Year = 2005;
	dt.Month = 2;
	dt.Day=29;
	if (CheckDate(&dt)) printf("ok1");
	dt.Year = 2008;
	dt.Month = 2;
	dt.Day=29;
	if (CheckDate(&dt)) printf("ok2");
#endif

//	len=DecodeBASE64("AXw", buff, 3);
//	DumpMessage(stdout, buff, len);
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
		switch (error) {
		case ERR_EMPTY:
 			printmsg("Entry number %i is empty\n",i);
		    	break;
		case ERR_NONE:
 			printmsg("Entry number %i\nStation name : \"%s\"\nFrequency    : %.1f MHz\n",
 				    i,DecodeUnicodeConsole(Station.StationName),
				    Station.Frequency);
			break;
		default:
			Print_Error(error);
		}
	}
	GSM_Terminate();
}

static void GetFileSystemStatus(int argc, char *argv[])
{
	GSM_FileSystemStatus	Status;

	GSM_Init(true);

	error = Phone->GetFileSystemStatus(&s,&Status);
	if (error != ERR_NOTSUPPORTED && error != ERR_NOTIMPLEMENTED) {
	    	Print_Error(error);
		printmsg("\nFree memory: %i, total memory: %i\n",Status.Free,Status.Free+Status.Used);
	}

	GSM_Terminate();
}

static void GetFileSystem(int argc, char *argv[])
{
	bool 			Start = true;
	GSM_File	 	Files;
	int			j;
	long			usedphone=0,usedcard=0;
	GSM_FileSystemStatus	Status;
	char 			FolderName[256];

	GSM_Init(true);

	if (!strcmp(s.Phone.Data.ModelInfo->model,"6230") &&
	    s.ConnectionType == GCT_IRDAPHONET) {
		printmsg("WARNING: firmware in your phone has bug in infrared support and only part of files will be listed. Use BT (or cable) or upgrade firmware (if there is something higher than 5.24 available)\n\n");
	}

	while (1) {
		error = Phone->GetNextFileFolder(&s,&Files,Start);
		if (error == ERR_EMPTY) break;
	    	Print_Error(error);

		if (!Files.Folder) {
			if (IsPhoneFeatureAvailable(s.Phone.Data.ModelInfo, F_FILES2)) {
				if (Files.ID_FullName[0] == 'b') {
					usedcard+=Files.Used;
				} else {
					usedphone+=Files.Used;
				}
			} else {
				usedphone+=Files.Used;
			}
		}

		if (argc <= 2 || !mystrncasecmp(argv[2],"-flatall",0)) {
			if (strlen(Files.ID_FullName) < 5 && strlen(Files.ID_FullName) != 0 && Files.ID_FullName[0]>='0' && Files.ID_FullName[0]<='9') {
				printf("%5s.",Files.ID_FullName);
			}
			if (Files.Protected) {
				printf("P");
			} else {
				printf(" ");
			}
			if (Files.ReadOnly) {
				printf("R");
			} else {
				printf(" ");
			}
			if (Files.Hidden) {
				printf("H");
			} else {
				printf(" ");
			}
			if (Files.System) {
				printf("S");
			} else {
				printf(" ");
			}
			if (argc > 2 &&  mystrncasecmp(argv[2],"-flat",0)) {
				if (!Files.Folder) {
					if (mystrncasecmp(argv[2],"-flatall",0)) {
						if (!Files.ModifiedEmpty) {
							printf(" %30s",OSDateTime(Files.Modified,false));
						} else printf(" %30c",0x20);
						printf(" %9i",Files.Used);
						printf(" ");
					} else printf("|-- ");
				} else printf("Folder ");
			} else {
				if (Files.Level != 1) {
					for (j=0;j<Files.Level-2;j++) printf(" |   ");
					printf(" |-- ");
				}
				if (Files.Folder) printf("Folder ");
			}
			printf("\"%s\"\n",DecodeUnicodeConsole(Files.Name));
		} else if (argc > 2 && mystrncasecmp(argv[2],"-flatall",0)) {
			/* format for a folder ID;Folder;FOLDER_NAME;[FOLDER_PARAMETERS]
			 * format for a file   ID;File;FOLDER_NAME;FILE_NAME;DATESTAMP;FILE_SIZE;[FILE_PARAMETERS]  */
			if (!Files.Folder) {
				printf("%s;File;",Files.ID_FullName);
				printf("\"%s\";",FolderName);
				printf("\"%s\";",DecodeUnicodeConsole(Files.Name));
				if (!Files.ModifiedEmpty) {
					printf("\"%s\";",OSDateTime(Files.Modified,false));
				} else  printf("\"%c\";",0x20);
				printf("%i;",Files.Used);
			} else {
				printf("%s;Folder;",Files.ID_FullName);
				printf("\"%s\";",DecodeUnicodeConsole(Files.Name));
				strcpy(FolderName,DecodeUnicodeConsole(Files.Name));
			}

			if (Files.Protected)  	printf("P");
			if (Files.ReadOnly)  	printf("R");
			if (Files.Hidden)  	printf("H");
			if (Files.System)  	printf("S");
			printf("\n");
		}
		Start = false;
	}

	printmsg("\nUsed in phone: %li, used in card: %li\n",usedphone,usedcard);

	error = Phone->GetFileSystemStatus(&s,&Status);
	if (error != ERR_NOTSUPPORTED && error != ERR_NOTIMPLEMENTED) {
	    	Print_Error(error);
		printmsg("\nFree memory: %i, total memory: %i\n",Status.Free,Status.Free+Status.Used);
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

	strcpy(Files.ID_FullName,argv[2]);
	for (i=3;i<argc;i++) {
		if (mystrncasecmp(argv[i],"-readonly",0)) {
			Files.ReadOnly = true;
		} else if (mystrncasecmp(argv[i],"-protected",0)) {
			Files.Protected = true;
		} else if (mystrncasecmp(argv[i],"-system",0)) {
			Files.System = true;
		} else if (mystrncasecmp(argv[i],"-hidden",0)) {
			Files.Hidden = true;
		} else {
			printmsgerr("Unknown attribute (%s)\n",argv[i]);
		}
	}

	GSM_Init(true);

	error = Phone->SetFileAttributes(&s,&Files);
    	Print_Error(error);

	GSM_Terminate();
}

static void GetRootFolders(int argc, char *argv[])
{
	GSM_File File;

	GSM_Init(true);

	File.ID_FullName[0] = 0;

	while (1) {
		if (Phone->GetNextRootFolder(&s,&File)!=ERR_NONE) break;
		printf("%s - %s\n",File.ID_FullName,DecodeUnicodeString(File.Name));
	}

	GSM_Terminate();
}

static void GetFolderListing(int argc, char *argv[])
{
	bool 			Start = true;
	GSM_File	 	Files;

	GSM_Init(true);

	strcpy(Files.ID_FullName,argv[2]);

	if (!strcmp(s.Phone.Data.ModelInfo->model,"6230") &&
	    s.ConnectionType == GCT_IRDAPHONET) {
		printmsg("WARNING: firmware in your phone has bug in infrared support and only part of files will be listed. Use BT (or cable) or upgrade firmware (if there is something higher than 5.24 available)\n\n");
	}

	while (1) {
		error = Phone->GetFolderListing(&s,&Files,Start);
		if (error == ERR_EMPTY) break;
	    	Print_Error(error);

		/* format for a folder ID;Folder;[FOLDER_PARAMETERS]
		 * format for a file   ID;File;FILE_NAME;DATESTAMP;FILE_SIZE;[FILE_PARAMETERS]  */
		if (!Files.Folder) {
			printf("%s;File;",Files.ID_FullName);
			printf("\"%s\";",DecodeUnicodeConsole(Files.Name));
			if (!Files.ModifiedEmpty) {
				printf("\"%s\";",OSDateTime(Files.Modified,false));
			} else  printf("\"%c\";",0x20);
			printf("%i;",Files.Used);
		} else {
			printf("%s;Folder;",Files.ID_FullName);
			printf("\"%s\";",DecodeUnicodeConsole(Files.Name));
		}

		if (Files.Protected)  	printf("P");
		if (Files.ReadOnly)  	printf("R");
		if (Files.Hidden)  	printf("H");
		if (Files.System)  	printf("S");
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
	t_time1 	= Fill_Time_T(dt,0);
	old1 		= 65536;

	error = ERR_NONE;
	while (error == ERR_NONE) {
		error = Phone->GetFilePart(&s,File,&Handle,&Size);
		if (error == ERR_NONE || error == ERR_EMPTY || error == ERR_WRONGCRC) {
			if (start) {
				printmsg("Getting \"%s\"\n", DecodeUnicodeConsole(File->Name));
				start = false;
			}
			if (File->Folder) {
				free(File->Buffer);
				GSM_Terminate();
				printmsg("it's folder. Please give only file names\n");
				exit(-1);
			}
			if (Size==0) {
				printmsg("*");
			} else {
				printmsg("%c  %i percent", 13, File->Used*100/Size);
				if (File->Used*100/Size >= 2) {
					GSM_GetCurrentDateTime(&dt);
					t_time2 = Fill_Time_T(dt,0);
					diff 	= t_time2-t_time1;
					p 	= diff*(Size-File->Used)/File->Used;
					if (p != 0) {
						if (p<old1) old1 = p;
						q = old1/60;
						printmsgerr(" (%02i:%02i minutes left)",q,old1-q*60);
					} else {
						printmsgerr("%30c",0x20);
					}
				}
			}
			if (error == ERR_EMPTY) break;
			if (error == ERR_WRONGCRC) {
				printmsg("WARNING: File checksum calculated by phone doesn't match with value calculated by Gammu. File damaged or error in Gammu\n");
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
				sprintf(buffer,"file%s",File->ID_FullName);
				file = fopen(buffer,"wb");
			}
			if (file == NULL) {
				sprintf(buffer,"file%i",i);
				file = fopen(buffer,"wb");
			}
			printmsg("  Saving to %s\n",buffer);
			if (!file) Print_Error(ERR_CANTOPENFILE);
			fwrite(File->Buffer,1,File->Used,file);
			fclose(file);
			if (!newtime && !File->ModifiedEmpty) {
				/* access time */
				filedate.actime  = Fill_Time_T(File->Modified, 8);
				/* modification time */
				filedate.modtime = Fill_Time_T(File->Modified, 8);
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
		if (mystrncasecmp(argv[i],"-newtime",0)) {
			newtime = true;
			continue;
		}

		strcpy(File.ID_FullName,argv[i]);
		GetOneFile(&File, newtime, i);
	}

	free(File.Buffer);
	GSM_Terminate();
}

static void GetFileFolder(int argc, char *argv[])
{
	bool 			Start = true;
	GSM_File	 	File;
	int			level=0,allnum=0,num=0,filelevel=0;
	bool			newtime = false, found;

	File.Buffer = NULL;

	GSM_Init(true);

	for (i=2;i<argc;i++) {
		if (mystrncasecmp(argv[i],"-newtime",0)) {
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
				if (mystrncasecmp(argv[i],"-newtime",0)) {
					continue;
				}
				dbgprintf("comparing %s %s\n",File.ID_FullName,argv[i]);
				if (!strcmp(File.ID_FullName,argv[i])) {
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

		if (level != 0 && !File.Folder) GetOneFile(&File, newtime,num);

		if (level == 2) {
			level = 0;
			num++;
		}

		Start = false;
	}

	free(File.Buffer);
	GSM_Terminate();
}

static void AddOneFile(GSM_File *File, char *text)
{
	int 		Pos,Handle,i,j,old1;
	time_t     	t_time1,t_time2;
	GSM_DateTime	dt;
	long		diff;

	GSM_GetCurrentDateTime(&dt);
	t_time1 = Fill_Time_T(dt,0);
	old1 = 65536;

	dbgprintf("Adding file to filesystem now\n");
	error 	= ERR_NONE;
	Pos	= 0;
	while (error == ERR_NONE) {
		error = Phone->AddFilePart(&s,File,&Pos,&Handle);
	    	if (error != ERR_EMPTY && error != ERR_WRONGCRC) Print_Error(error);
		if (File->Used != 0) {
			printmsgerr("%c%s%03i percent",13,text,Pos*100/File->Used);
			if (Pos*100/File->Used >= 2) {
				GSM_GetCurrentDateTime(&dt);
				t_time2 = Fill_Time_T(dt,0);
				diff = t_time2-t_time1;
				i = diff*(File->Used-Pos)/Pos;
				if (i != 0) {
					if (i<old1) old1 = i;
					j = old1/60;
					printmsgerr(" (%02i:%02i minutes left)",j,old1-j*60);
				} else {
					printmsgerr("%30c",0x20);
				}
			}
		}
	}
	printmsgerr("\n");
	if (error == ERR_WRONGCRC) {
		printmsg("WARNING: File checksum calculated by phone doesn't match with value calculated by Gammu. File damaged or error in Gammu\n");
	}
}

static void AddFile(int argc, char *argv[])
{
	GSM_File		File;
	int			i,nextlong;

	File.Buffer = NULL;
	strcpy(File.ID_FullName,argv[2]);
	error = GSM_ReadFile(argv[3], &File);
	Print_Error(error);
	EncodeUnicode(File.Name,argv[3],strlen(argv[3]));
	for (i=strlen(argv[3])-1;i>0;i--) {
		if (argv[3][i] == '\\' || argv[3][i] == '/') break;
	}
	if (argv[3][i] == '\\' || argv[3][i] == '/') {
		EncodeUnicode(File.Name,argv[3]+i+1,strlen(argv[3])-i-1);
	}

	GSM_IdentifyFileFormat(&File);

	File.Protected 	= false;
	File.ReadOnly	= false;
	File.Hidden	= false;
	File.System	= false;

	if (argc > 4) {
		nextlong = 0;
		for (i=4;i<argc;i++) {
			switch(nextlong) {
			case 0:
				if (mystrncasecmp(argv[i],"-type",0)) {
					nextlong = 1;
					continue;
				}
				if (mystrncasecmp(argv[i],"-protected",0)) {
					File.Protected = true;
					continue;
				}
				if (mystrncasecmp(argv[i],"-readonly",0)) {
					File.ReadOnly = true;
					continue;
				}
				if (mystrncasecmp(argv[i],"-hidden",0)) {
					File.Hidden = true;
					continue;
				}
				if (mystrncasecmp(argv[i],"-system",0)) {
					File.System = true;
					continue;
				}
				if (mystrncasecmp(argv[i],"-newtime",0)) {
					File.ModifiedEmpty = true;
					continue;
				}
				printmsg("Parameter \"%s\" unknown\n",argv[i]);
				exit(-1);
			case 1:
				if (mystrncasecmp(argv[i],"JAR",0)) {
					File.Type = GSM_File_Java_JAR;
				} else if (mystrncasecmp(argv[i],"JPG",0)) {
					File.Type = GSM_File_Image_JPG;
				} else if (mystrncasecmp(argv[i],"BMP",0)) {
					File.Type = GSM_File_Image_BMP;
				} else if (mystrncasecmp(argv[i],"WBMP",0)) {
					File.Type = GSM_File_Image_WBMP;
				} else if (mystrncasecmp(argv[i],"GIF",0)) {
					File.Type = GSM_File_Image_GIF;
				} else if (mystrncasecmp(argv[i],"PNG",0)) {
					File.Type = GSM_File_Image_PNG;
                                } else if (mystrncasecmp(argv[i],"MIDI",0)) {
                                        File.Type = GSM_File_Sound_MIDI;
                                } else if (mystrncasecmp(argv[i],"AMR",0)) {
                                        File.Type = GSM_File_Sound_AMR;
                                } else if (mystrncasecmp(argv[i],"NRT",0)) {
                                        File.Type = GSM_File_Sound_NRT;
                                } else if (mystrncasecmp(argv[i],"3GP",0)) {
                                        File.Type = GSM_File_Video_3GP;
				} else {
					printmsg("What file type (\"%s\") ?\n",argv[i]);
					exit(-1);
				}
				nextlong = 0;
				break;
			}
		}
		if (nextlong!=0) {
			printmsg("Parameter missed...\n");
			exit(-1);
		}
	}

	GSM_Init(true);

	AddOneFile(&File, "Writing: ");

	free(File.Buffer);
	GSM_Terminate();
}

static void AddFolder(int argc, char *argv[])
{
	GSM_File File;

	strcpy(File.ID_FullName,argv[2]);
	EncodeUnicode(File.Name,argv[3],strlen(argv[3]));
	File.ReadOnly = false;

	GSM_Init(true);

	error = Phone->AddFolder(&s,&File);
    	Print_Error(error);

	GSM_Terminate();
}

static void DeleteFolder(int argc, char *argv[])
{
	GSM_Init(true);

	error = Phone->DeleteFolder(&s,argv[2]);
    	Print_Error(error);

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
	{"",	 "MMSUnreadInbox", "INBOX",		"3"},
	{"",	 "MMSReadInbox",   "INBOX",		"3"},
	{"",	 "MMSOutbox",	   "OUTBOX",		"3"},
	{"",	 "MMSSent",	   "SENT",		"3"},
	{"",	 "MMSDrafts",	   "DRAFTS",		"3"},
	{"",	 "Application",	   "applications",	"3"},
	{"",	 "Game",	   "games",		"3"},
	/* Language indepedent in DCT4 in filesystem 2 */
	{"", 	 "Gallery",	   "a:/predefgallery/predefgraphics",			""},
	{"", 	 "Gallery2",	   "a:/predefgallery/predefgraphics/predefcliparts",	""},
	{"", 	 "Camera",	   "a:/predefgallery/predefphotos",			""},
	{"", 	 "Tones",	   "a:/predefgallery/predeftones",			""},
	{"", 	 "Tones2",	   "a:/predefgallery/predefmusic",			""},
	{"", 	 "Records",	   "a:/predefgallery/predefrecordings",			""},
	{"", 	 "Video",	   "a:/predefgallery/predefvideos",			""},
	{"", 	 "Playlist",	   "a:/predefplaylist",					""},
	{"", 	 "MemoryCard",	   "b:",						""},
	/* Language depedent in DCT4 filesystem 1 */
	{"",	 "Gallery",	   "Clip-arts",					"3"},
	{"",	 "Gallery",	   "004F006200720061007A006B0069",		"3"},//obrazki PL 6220
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
	GSM_File		File, Files;
	FILE			*file;
	GSM_DateTime		DT,DT2;
	time_t     		t_time1,t_time2;
	unsigned char 		buffer[10000],JAR[500],Vendor[500],Name[500],Version[500],FileID[400];
	bool 			Start = true, Found = false, wasclr;
	bool			ModEmpty = false;
	int			i = 0, Pos, Size, Size2, nextlong;

	while (Folder[i].parameter[0] != 0) {
		if (mystrncasecmp(argv[2],Folder[i].parameter,0)) {
			Found = true;
			break;
		}
		i++;
	}
	if (!Found) {
		printmsg("What folder type (\"%s\") ?\n",argv[2]);
		exit(-1);
	}

	if (mystrncasecmp(argv[2],"Application",0) || mystrncasecmp(argv[2],"Game",0)) {
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
			     mystrncasecmp(argv[2],Folder[i].parameter,0)) {
				strcpy(Files.ID_FullName,Folder[i].folder);
				Found = true;
				break;
			}
			i++;
		}
		if (!Found) {
			printmsg("Folder not found. Probably function not supported !\n");
			GSM_Terminate();
			exit(-1);
		}
	} else {
		if (IsPhoneFeatureAvailable(s.Phone.Data.ModelInfo, F_FILES2)) {
			i = 0;
			while (Folder[i].parameter[0] != 0) {
				if ((Folder[i].folder[0] == 'a' || Folder[i].folder[0] == 'b') &&
				    Folder[i].level[0] == 0x00 &&
				    mystrncasecmp(argv[2],Folder[i].parameter,0)) {
					strcpy(Files.ID_FullName,Folder[i].folder);
					Found = true;
					break;
				}
				i++;
			}
		}
	}
	if (!Found) {
		printmsgerr("Searching for phone folder: ");
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
					dbgprintf("comparing \"%s\" \"%s\" \"%s\"\n",s.Phone.Data.ModelInfo->model,Files.ID_FullName,Folder[i].level);
					if (mystrncasecmp(argv[2],Folder[i].parameter,0)  &&
					    mywstrncasecmp(Files.Name,buffer,0) &&
					    Files.Level == atoi(Folder[i].level)) {
						dbgprintf("found 1\n");
						Found = true;
						break;
					}
					if (mystrncasecmp(argv[2],Folder[i].parameter,0) &&
					    !strcmp(Files.ID_FullName,Folder[i].folder) &&
					    Folder[i].level[0] == 0x00) {
						Found = true;
						dbgprintf("found 2\n");
						break;
					}
					if (Folder[i].folder[0]>='0'&&Folder[i].folder[0] <='9') {
						DecodeHexUnicode (buffer, Folder[i].folder,strlen(Folder[i].folder));
						dbgprintf("comparing \"%s\"",DecodeUnicodeString(buffer));
						dbgprintf("and \"%s\"\n",DecodeUnicodeString(Files.Name));
						if (mystrncasecmp(argv[2],Folder[i].parameter,0)  &&
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
			printmsgerr("*");

			Start = false;
		}
		printmsgerr("\n");
	}
	if (!Found) {
		printmsg("Folder not found. Probably function not supported !\n");
		GSM_Terminate();
		exit(-1);
	}
	File.Buffer 	= NULL;
	File.Protected  = false;
	File.ReadOnly   = false;
	File.Hidden	= false;
	File.System	= false;

	if (mystrncasecmp(argv[2],"Application",0) || mystrncasecmp(argv[2],"Game",0)) {
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
				printmsgerr("No vendor info in JAD file\n");
				GSM_Terminate();
				return;
			}
			if (Name[0] == 0x00) {
				printmsgerr("No name info in JAD file\n");
				GSM_Terminate();
				return;
			}
			if (JAR[0] == 0x00) {
				printmsgerr("No JAR URL info in JAD file\n");
				GSM_Terminate();
				return;
			}
			if (Size == -1) {
				printmsgerr("No JAR size info in JAD file\n");
				GSM_Terminate();
				return;
			}
		}
		if (Size != Size2) {
			printmsgerr("Declared JAR file size is different than real\n");
			GSM_Terminate();
			return;
		}
  		printmsgerr("Adding \"%s\"",Name);
		if (Version[0] != 0x00) printmsgerr(" version %s",Version);
		printmsgerr(" created by %s\n",Vendor);

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

		/* adding folder */
		strcpy(buffer,Vendor);
		strcat(buffer,Name);
		EncodeUnicode(File.Name,buffer,strlen(buffer));
		strcpy(File.ID_FullName,Files.ID_FullName);
		error = Phone->AddFolder(&s,&File);
	    	Print_Error(error);
		strcpy(FileID,File.ID_FullName);

		/* adding jad file */
		strcpy(buffer,JAR);
		buffer[strlen(buffer) - 1] = 'd';
		EncodeUnicode(File.Name,buffer,strlen(buffer));
		File.Type 	   = GSM_File_Other;
		File.ModifiedEmpty = true;
		dbgprintf("file id is \"%s\"\n",File.ID_FullName);
		AddOneFile(&File, "Writing JAD file: ");

		if (argc > 4) {
			if (mystrncasecmp(argv[4],"-readonly",0)) File.ReadOnly = true;
		}

		/* reading jar file */
		sprintf(buffer,"%s.jar",argv[3]);
		error = GSM_ReadFile(buffer, &File);
		Print_Error(error);

		/* adding jar file */
		strcpy(File.ID_FullName,FileID);
		strcpy(buffer,JAR);
		EncodeUnicode(File.Name,buffer,strlen(buffer));
		File.Type 	   = GSM_File_Java_JAR;
		File.ModifiedEmpty = true;
		AddOneFile(&File, "Writing JAR file: ");
		free(File.Buffer);
		GSM_Terminate();
		return;
	}

	if (mystrncasecmp(argv[2],"Gallery" 	 ,0) ||
	    mystrncasecmp(argv[2],"Gallery2"	 ,0) ||
	    mystrncasecmp(argv[2],"Camera"  	 ,0) ||
	    mystrncasecmp(argv[2],"Tones"   	 ,0) ||
	    mystrncasecmp(argv[2],"Tones2"  	 ,0) ||
	    mystrncasecmp(argv[2],"Records" 	 ,0) ||
	    mystrncasecmp(argv[2],"Video"   	 ,0) ||
	    mystrncasecmp(argv[2],"Playlist"	 ,0) ||
	    mystrncasecmp(argv[2],"MemoryCard"   ,0)) {
		strcpy(buffer,argv[3]);
		if (argc > 4) {
			nextlong = 0;
			for (i=4;i<argc;i++) {
				switch(nextlong) {
				case 0:
					if (mystrncasecmp(argv[i],"-name",0)) {
						nextlong = 1;
						continue;
					}
					if (mystrncasecmp(argv[i],"-protected",0)) {
						File.Protected = true;
						continue;
					}
					if (mystrncasecmp(argv[i],"-readonly",0)) {
						File.ReadOnly = true;
						continue;
					}
					if (mystrncasecmp(argv[i],"-hidden",0)) {
						File.Hidden = true;
						continue;
					}
					if (mystrncasecmp(argv[i],"-system",0)) {
						File.System = true;
						continue;
					}
					if (mystrncasecmp(argv[i],"-newtime",0)) {
						ModEmpty = true;
						continue;
					}
					printmsg("Parameter \"%s\" unknown\n",argv[i]);
					exit(-1);
				case 1:
					strcpy(buffer,argv[i]);
					nextlong = 0;
					break;
				}
			}
			if (nextlong!=0) {
				printmsg("Parameter missed...\n");
				exit(-1);
			}
		}
	} else { /* MMS things */
		DT2.Year   = 2001;
		DT2.Month  = 12;
		DT2.Day    = 31;
		DT2.Hour   = 14;
		DT2.Minute = 00;
		DT2.Second = 00;
		t_time2    = Fill_Time_T(DT2,8);

		GSM_GetCurrentDateTime(&DT);
		t_time1    = Fill_Time_T(DT,8);

		sprintf(buffer,"%07X %07X ",(int)(t_time1-t_time2-40),(int)(t_time1-t_time2-40));
#ifdef DEVELOP
		sprintf(buffer,"2A947BD 2A947DB ");
#endif
		/* 40 = inbox "multimedia message received" message */
		/* 30 = outbox sending failed */
		if (mystrncasecmp(argv[2],"MMSUnreadInbox",0)) 	  strcat(buffer,"43 ");
		else if (mystrncasecmp(argv[2],"MMSReadInbox",0)) strcat(buffer,"50 ");
		else if (mystrncasecmp(argv[2],"MMSOutbox",0))    strcat(buffer,"10 ");
		else if (mystrncasecmp(argv[2],"MMSSent",0))      strcat(buffer,"20 ");
                else if (mystrncasecmp(argv[2],"MMSDrafts",0))    strcat(buffer,"61 ");
		if (argc > 4) {
			if (!mystrncasecmp(argv[2],"MMSOutbox",0) &&
			    !mystrncasecmp(argv[2],"MMSSent",0)) {
				sprintf(Name,"%s",argv[4]);
				strcat(buffer,Name);
			}
			if (argc > 5) {
				sprintf(Name,"%zd%s/TYPE=PLMN",strlen(argv[5])+10,argv[5]);
				strcat(buffer,Name);
			}
		}
		ModEmpty = true;
	}

	error = GSM_ReadFile(argv[3], &File);
	Print_Error(error);
	if (ModEmpty) File.ModifiedEmpty = true;

	strcpy(File.ID_FullName,Files.ID_FullName);
	EncodeUnicode(File.Name,buffer,strlen(buffer));
	for (i=strlen(buffer)-1;i>0;i--) {
		if (buffer[i] == '\\' || buffer[i] == '/') break;
	}
	if (buffer[i] == '\\' || buffer[i] == '/') {
		EncodeUnicode(File.Name,buffer+i+1,strlen(buffer)-i-1);
	}

	GSM_IdentifyFileFormat(&File);
#ifdef DEVELOP
	if (mystrncasecmp(argv[2],"Gallery",0) || mystrncasecmp(argv[2],"Tones",0)) {
	} else { /* MMS things */
		File.Type = GSM_File_MMS;
	}
#endif

	AddOneFile(&File, "Writing file: ");
	free(File.Buffer);
	GSM_Terminate();
}

static void DeleteFiles(int argc, char *argv[])
{
	int i;

	GSM_Init(true);

	for (i=2;i<argc;i++) {
		error = Phone->DeleteFile(&s,argv[i]);
	    	Print_Error(error);
	}

	GSM_Terminate();
}

static void SaveMMSFile(int argc, char *argv[])
{
	FILE 				*file;
	unsigned char	 		Buffer[50000],Buffer2[20][2010];
	int 				i,nextlong = 0,len = 0;
	GSM_EncodeMultiPartMMSInfo      Info;

	GSM_ClearMultiPartMMSInfo(&Info);

	for (i=3;i<argc;i++) {
		switch (nextlong) {
		case 0:
			if (mystrncasecmp(argv[i],"-subject",0)) {
				nextlong=1;
				continue;
			}
			if (mystrncasecmp(argv[i],"-text",0)) {
				nextlong=2;
				continue;
			}
			if (mystrncasecmp(argv[i],"-from",0)) {
				nextlong=3;
				continue;
			}
			if (mystrncasecmp(argv[i],"-to",0)) {
				nextlong=4;
				continue;
			}
			printmsg("Unknown parameter (\"%s\")\n",argv[i]);
			exit(-1);
			break;
		case 1: /* Subject */
			EncodeUnicode(Info.Subject,argv[i],strlen(argv[i]));
			nextlong = 0;
			break;
		case 2: /* Text */
			EncodeUnicode(Buffer2[Info.EntriesNum],argv[i],strlen(argv[i]));
			Info.Entries[Info.EntriesNum].ID 	= MMS_Text;
			Info.Entries[Info.EntriesNum].Buffer 	= Buffer2[Info.EntriesNum];
			Info.EntriesNum++;
			nextlong = 0;
			break;
		case 3: /* From */
			EncodeUnicode(Info.Source,argv[i],strlen(argv[i]));
			nextlong = 0;
			break;
		case 4: /* To */
			EncodeUnicode(Info.Destination,argv[i],strlen(argv[i]));
			nextlong = 0;
			break;
		}
	}
	if (nextlong!=0) {
		printmsg("Parameter missed...\n");
		exit(-1);
	}

	GSM_EncodeMMSFile(&Info,Buffer,&len);

	file = fopen(argv[2],"wb");
	if (file == NULL) Print_Error(ERR_CANTOPENFILE);
	fwrite(Buffer,1,len,file);
	fclose(file);
}

static void CallDivert(int argc, char *argv[])
{
	GSM_MultiCallDivert cd;

	     if (mystrncasecmp("get",	argv[2],0)) {}
	else if (mystrncasecmp("set",	argv[2],0)) {}
	else {
		printmsg("Unknown divert action (\"%s\")\n",argv[2]);
		exit(-1);
	}

	     if (mystrncasecmp("all"       , argv[3],0)) {cd.Request.DivertType = GSM_DIVERT_AllTypes  ;}
	else if (mystrncasecmp("busy"      , argv[3],0)) {cd.Request.DivertType = GSM_DIVERT_Busy      ;}
	else if (mystrncasecmp("noans"     , argv[3],0)) {cd.Request.DivertType = GSM_DIVERT_NoAnswer  ;}
	else if (mystrncasecmp("outofreach", argv[3],0)) {cd.Request.DivertType = GSM_DIVERT_OutOfReach;}
	else {
		printmsg("Unknown divert type (\"%s\")\n",argv[3]);
		exit(-1);
	}

	     if (mystrncasecmp("all"  , argv[4],0)) {cd.Request.CallType = GSM_DIVERT_AllCalls  ;}
	else if (mystrncasecmp("voice", argv[4],0)) {cd.Request.CallType = GSM_DIVERT_VoiceCalls;}
	else if (mystrncasecmp("fax"  , argv[4],0)) {cd.Request.CallType = GSM_DIVERT_FaxCalls  ;}
	else if (mystrncasecmp("data" , argv[4],0)) {cd.Request.CallType = GSM_DIVERT_DataCalls ;}
	else {
		printmsg("Unknown call type (\"%s\")\n",argv[4]);
		exit(-1);
	}

	GSM_Init(true);

	if (mystrncasecmp("get", argv[2],0)) {
		error = Phone->GetCallDivert(&s,&cd);
	    	Print_Error(error);
		printmsg("Query:\n   Divert type: ");
	} else {
		cd.Request.Number[0] = 0;
		cd.Request.Number[1] = 0;
		if (argc > 5) EncodeUnicode(cd.Request.Number,argv[5],strlen(argv[5]));

		cd.Request.Timeout = 0;
		if (argc > 6) cd.Request.Timeout = atoi(argv[6]);

		error = Phone->SetCallDivert(&s,&cd);
	    	Print_Error(error);
		printmsg("Changed:\n   Divert type: ");
	}

      	switch (cd.Request.DivertType) {
   		case GSM_DIVERT_Busy      : printmsg("when busy");				break;
	        case GSM_DIVERT_NoAnswer  : printmsg("when not answered");			break;
      	        case GSM_DIVERT_OutOfReach: printmsg("when phone off or no coverage");	break;
                case GSM_DIVERT_AllTypes  : printmsg("all types of diverts");			break;
                default		          : printmsg("unknown %i",cd.Request.DivertType);			break;
        }
        printmsg("\n   Calls type : ");
	switch (cd.Request.CallType) {
	 	case GSM_DIVERT_VoiceCalls: printmsg("voice");				break;
                case GSM_DIVERT_FaxCalls  : printmsg("fax");				break;
                case GSM_DIVERT_DataCalls : printmsg("data");		 		break;
		case GSM_DIVERT_AllCalls  : printmsg("data & fax & voice");		break;
                default                   : printmsg("unknown %i",cd.Request.CallType);   		break;
        }
	printmsg("\nResponse:");

	for (i=0;i<cd.Response.EntriesNum;i++) {
	        printmsg("\n   Calls type : ");
        	switch (cd.Response.Entries[i].CallType) {
                	case GSM_DIVERT_VoiceCalls: printmsg("voice");		 	break;
                	case GSM_DIVERT_FaxCalls  : printmsg("fax");		 	break;
                	case GSM_DIVERT_DataCalls : printmsg("data");		 	break;
                	default                   : printmsg("unknown %i",cd.Response.Entries[i].CallType);break;
              	}
		printf("\n");
               	printmsg("   Timeout    : %i seconds\n",cd.Response.Entries[i].Timeout);
                printmsg("   Number     : %s\n",DecodeUnicodeString(cd.Response.Entries[i].Number));
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

typedef struct {
	unsigned char		Connection[50];
} OneConnectionInfo;

typedef struct {
	unsigned char 		Device[50];
	OneConnectionInfo 	Connections[4];
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
		ss.msg				= s.msg;
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
		if (SearchOutput) printf("Connection \"%s\" on device \"%s\"\n",Info->Connections[j].Connection,Info->Device);
		if (error == ERR_NONE) {
			error=ss.Phone.Functions->GetManufacturer(&ss);
			if (error == ERR_NONE) {
				error=ss.Phone.Functions->GetModel(&ss);
				if (error == ERR_NONE) {
					if (!SearchOutput) printf("Connection \"%s\" on device \"%s\"\n",Info->Connections[j].Connection,Info->Device);
					printmsg("   Manufacturer  : %s\n",
						ss.Phone.Data.Manufacturer);
					printmsg("   Model         : %s (%s)\n",
						ss.Phone.Data.ModelInfo->model,
						ss.Phone.Data.Model);
				} else {
					if (SearchOutput) printf("   %s\n",print_error(error,ss.di.df,ss.msg));
				}
			} else {
				if (SearchOutput) printf("   %s\n",print_error(error,ss.di.df,ss.msg));
			}
		} else {
			if (SearchOutput) printf("   %s\n",print_error(error,ss.di.df,ss.msg));
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
	if (argc == 3 && mystrncasecmp(argv[2], "-debug",0)) SearchOutput = true;

	num = 0;
#ifdef WIN32
#  ifdef GSM_ENABLE_IRDADEVICE
	sprintf(SearchDevices[dev].Device,"");
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

static GSM_Parameters Parameters[] = {
	{"--identify",			0, 0, Identify,			{H_Info,0},			""},
	{"--version",			0, 0, Version,			{H_Other,0},			""},
	{"--getdisplaystatus",		0, 0, GetDisplayStatus,		{H_Info,0},			""},
	{"--monitor",			0, 1, Monitor,			{H_Info,H_Network,H_Call,0},	"[times]"},
	{"--setautonetworklogin",	0, 0, SetAutoNetworkLogin,	{H_Network,0},			""},
	{"--listnetworks",		0, 1, ListNetworks,		{H_Network,0},			"[country]"},
	{"--getgprspoint",		1, 2, GetGPRSPoint,		{H_Nokia,H_Network,0},		"start [stop]"},
	{"--getfilesystemstatus",	0, 0, GetFileSystemStatus,	{H_Filesystem,0},		""},
	{"--getfilesystem",		0, 1, GetFileSystem,		{H_Filesystem,0},		"[-flatall|-flat]"},
	{"--getfilefolder",		1,40, GetFileFolder,		{H_Filesystem,0},		"ID1, ID2, ..."},
	{"--addfolder",			2, 2, AddFolder,		{H_Filesystem,0},		"parentfolderID name"},
	{"--deletefolder",		1, 1, DeleteFolder,		{H_Filesystem,0},		"name"},
	{"--getfolderlisting",		1, 1, GetFolderListing,		{H_Filesystem,0},		"folderID"},
	{"--getrootfolders",		0, 0, GetRootFolders,		{H_Filesystem,0},		""},
	{"--setfileattrib",		1, 5, SetFileAttrib,		{H_Filesystem,0},		"folderID [-system] [-readonly] [-hidden] [-protected]"},
	{"--getfiles",			1,40, GetFiles,			{H_Filesystem,0},		"ID1, ID2, ..."},
	{"--addfile",			2, 6, AddFile,			{H_Filesystem,0},		"folderID name [-type JAR|BMP|PNG|GIF|JPG|MIDI|WBMP|AMR|3GP|NRT][-readonly][-protected][-system][-hidden][-newtime]"},
	{"--deletefiles",		1,20, DeleteFiles,		{H_Filesystem,0},		"fileID"},
	{"--nokiaaddfile",		2, 5, NokiaAddFile,		{H_Filesystem,H_Nokia,0},	"MMSUnreadInbox|MMSReadInbox|MMSOutbox|MMSDrafts|MMSSent file sender title"},
	{"--nokiaaddfile",		2, 5, NokiaAddFile,		{H_Filesystem,H_Nokia,0},	"Application|Game file [-readonly]"},
	{"--nokiaaddfile",		2, 5, NokiaAddFile,		{H_Filesystem,H_Nokia,0},	"Gallery|Gallery2|Camera|Tones|Tones2|Records|Video|Playlist|MemoryCard file [-name name][-protected][-readonly][-system][-hidden][-newtime]"},
	{"--playringtone",		1, 1, PlayRingtone, 		{H_Ringtone,0},			"file"},
	{"--playsavedringtone",		1, 1, DCT4PlaySavedRingtone, 	{H_Ringtone,0},			"number"},
	{"--getdatetime",		0, 0, GetDateTime,		{H_DateTime,0},			""},
	{"--setdatetime",		0, 0, SetDateTime,		{H_DateTime,0},			""},
	{"--getalarm",			0, 0, GetAlarm,			{H_DateTime,0},			""},
	{"--setalarm",			2, 2, SetAlarm,			{H_DateTime,0},			"hour minute"},
	{"--resetphonesettings",	1, 1, ResetPhoneSettings,	{H_Settings,0},			"PHONE|DEV|UIF|ALL|FACTORY"},
	{"--getmemory",			2, 4, GetMemory,		{H_Memory,0},			"DC|MC|RC|ON|VM|SM|ME|FD start [stop [-nonempty]]"},
	{"--getallmemory",		1, 2, GetAllMemory,		{H_Memory,0},			"DC|MC|RC|ON|VM|SM|ME|FD"},
	{"--searchmemory",		1, 1, SearchMemory,		{H_Memory,0},			"text"},
	{"--listmemorycategory",	1, 1, ListMemoryCategory,	{H_Memory, H_Category,0},	"text|number"},
	{"--getfmstation",		1, 2, GetFMStation,		{H_FM,0},			"start [stop]"},
	{"--getsmsc",			1, 2, GetSMSC,			{H_SMS,0},			"start [stop]"},
	{"--getsms",			2, 3, GetSMS,			{H_SMS,0},			"folder start [stop]"},
	{"--deletesms",			2, 3, DeleteSMS,		{H_SMS,0},			"folder start [stop]"},
	{"--deleteallsms",		1, 1, DeleteAllSMS,		{H_SMS,0},			"folder"},
	{"--getsmsfolders",		0, 0, GetSMSFolders,		{H_SMS,0},			""},
	{"--getallsms",			0, 0, GetAllSMS,		{H_SMS,0},			""},
	{"--geteachsms",		0, 0, GetEachSMS,		{H_SMS,0},			""},

#define SMS_TEXT_OPTIONS	"[-inputunicode][-16bit][-flash][-len len][-autolen len][-unicode][-enablevoice][-disablevoice][-enablefax][-disablefax][-enableemail][-disableemail][-voidsms][-replacemessages ID][-replacefile file]"
#define SMS_PICTURE_OPTIONS	"[-text text][-unicode][-alcatelbmmi]"
#define SMS_PROFILE_OPTIONS	"[-name name][-bitmap bitmap][-ringtone ringtone]"
#define SMS_EMS_OPTIONS		"[-unicode][-16bit][-format lcrasbiut][-text text][-unicodefiletext file][-defsound ID][-defanimation ID][-tone10 file][-tone10long file][-tone12 file][-tone12long file][-toneSE file][-toneSElong file][-fixedbitmap file][-variablebitmap file][-variablebitmaplong file][-animation frames file1 ...][-protected number]"
#define SMS_SMSTEMPLATE_OPTIONS	"[-unicode][-text text][-unicodefiletext file][-defsound ID][-defanimation ID][-tone10 file][-tone10long file][-tone12 file][-tone12long file][-toneSE file][-toneSElong file][-variablebitmap file][-variablebitmaplong file][-animation frames file1 ...]"
#define SMS_ANIMATION_OPTIONS	""
#define SMS_OPERATOR_OPTIONS	"[-netcode netcode][-biglogo]"
#define SMS_SAVE_OPTIONS	"[-folder id][-unread][-read][-unsent][-sent][-sender number]"
#define SMS_SEND_OPTIONS	"[-report][-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX][-save [-folder number]]"
#define SMS_COMMON_OPTIONS	"[-smscset number][-smscnumber number][-reply][-maxsms num]"

	{"--savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,0},			"TEXT " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS SMS_TEXT_OPTIONS},
	{"--savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_Ringtone,0},		"RINGTONE file " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS},
	{"--savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_Logo,0},		"OPERATOR file " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS SMS_OPERATOR_OPTIONS},
	{"--savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_Logo,0},		"CALLER file " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS},
	{"--savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_Logo,0},		"PICTURE file " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS SMS_PICTURE_OPTIONS},
	{"--savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_Logo,0},		"ANIMATION frames file1 file2... " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS SMS_ANIMATION_OPTIONS},
	{"--savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_MMS,0},		"MMSINDICATOR URL Title Sender " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS},
	{"--savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_WAP,0},		"WAPINDICATOR URL Title " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS},
#ifdef GSM_ENABLE_BACKUP
	{"--savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_WAP,0},		"BOOKMARK file location " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS},
	{"--savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_WAP,0},		"WAPSETTINGS file location DATA|GPRS " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS},
	{"--savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_MMS,0},		"MMSSETTINGS file location  " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS},
	{"--savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_Calendar,0},		"CALENDAR file location " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS},
	{"--savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_ToDo,0},		"TODO file location " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS},
	{"--savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_Memory,0},		"VCARD10|VCARD21 file SM|ME location [-nokia]" SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS},
#endif
	{"--savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,H_Settings,0},		"PROFILE " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS SMS_PROFILE_OPTIONS},
	{"--savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,0},			"EMS " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS SMS_EMS_OPTIONS},
	{"--savesms",			1,30, SendSaveDisplaySMS,	{H_SMS,0},			"SMSTEMPLATE " SMS_SAVE_OPTIONS SMS_COMMON_OPTIONS SMS_SMSTEMPLATE_OPTIONS},

	{"--sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,0},			"TEXT destination " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS SMS_TEXT_OPTIONS},
	{"--sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_Ringtone,0},		"RINGTONE destination file " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS},
	{"--sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_Logo,0},		"OPERATOR destination file " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS SMS_OPERATOR_OPTIONS},
	{"--sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_Logo,0},		"CALLER destination file " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS},
	{"--sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_Logo,0},		"PICTURE destination file " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS SMS_PICTURE_OPTIONS},
	{"--sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_Logo,0},		"ANIMATION destination frames file1 file2... " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS SMS_ANIMATION_OPTIONS},
	{"--sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_MMS,0},		"MMSINDICATOR destination URL Title Sender " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS},
	{"--sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_WAP,0},		"WAPINDICATOR destination URL Title " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS},
#ifdef GSM_ENABLE_BACKUP
	{"--sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_WAP,0},		"BOOKMARK destination file location " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS},
	{"--sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_WAP,0},		"WAPSETTINGS destination file location DATA|GPRS " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS},
	{"--sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_MMS,0},		"MMSSETTINGS destination file location " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS},
	{"--sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_Calendar,0},		"CALENDAR destination file location " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS},
	{"--sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_ToDo,0},		"TODO destination file location " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS},
	{"--sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_Memory,0},		"VCARD10|VCARD21 destination file SM|ME location [-nokia]" SMS_SEND_OPTIONS SMS_COMMON_OPTIONS},
#endif
	{"--sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,H_Settings,0},		"PROFILE destination " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS ""SMS_PROFILE_OPTIONS},
	{"--sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,0},			"EMS destination " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS SMS_EMS_OPTIONS},
	{"--sendsms",			2,30, SendSaveDisplaySMS,	{H_SMS,0},			"SMSTEMPLATE destination " SMS_SEND_OPTIONS SMS_COMMON_OPTIONS SMS_SMSTEMPLATE_OPTIONS},

	{"--displaysms",		2,30, SendSaveDisplaySMS,	{H_SMS,0},			"... (options like in sendsms)"},

	{"--addsmsfolder",		1, 1, AddSMSFolder,		{H_SMS,0},			"name"},
#ifdef HAVE_MYSQL_MYSQL_H
	{"--smsd",			2, 2, SMSDaemon,		{H_SMS,H_Other,0},		"MYSQL configfile"},
#endif
	{"--smsd",			2, 2, SMSDaemon,		{H_SMS,H_Other,0},		"FILES configfile"},
	{"--sendsmsdsms",		2,30, SendSaveDisplaySMS,	{H_SMS,H_Other,0},		"TEXT|WAPSETTINGS|... destination FILES|MYSQL configfile ... (options like in sendsms)"},
	{"--getmmsfolders",		0, 0, GetMMSFolders,		{H_MMS,0},			""},
	{"--getringtone",		1, 2, GetRingtone,		{H_Ringtone,0},			"location [file]"},
	{"--getphoneringtone",		1, 2, GetRingtone,		{H_Ringtone,0},			"location [file]"},
	{"--getringtoneslist",		0, 0, GetRingtonesList,		{H_Ringtone,0},			""},
	{"--setringtone",		1, 6, SetRingtone,		{H_Ringtone,0},			"file [-location location][-scale][-name name]"},
	{"--nokiacomposer",		1, 1, NokiaComposer,		{H_Ringtone,H_Nokia,0},		"file"},
	{"--copyringtone",		2, 3, CopyRingtone,		{H_Ringtone,0},			"source destination [RTTL|BINARY]"},
	{"--getussd",			1, 1, GetUSSD,			{H_Call,0},			"code"},
	{"--dialvoice",			1, 2, DialVoice,		{H_Call,0},			"number [show|hide]"},
	{"--getspeeddial",		1, 2, GetSpeedDial,		{H_Call,H_Memory,0},		"start [stop]"},
	{"--cancelcall",		0, 1, CancelCall,		{H_Call,0},			"[ID]"},
	{"--answercall",		0, 1, AnswerCall,		{H_Call,0},			"[ID]"},
	{"--unholdcall",		1, 1, UnholdCall,		{H_Call,0},			"ID"},
	{"--holdcall",			1, 1, HoldCall,			{H_Call,0},			"ID"},
	{"--conferencecall",		1, 1, ConferenceCall,		{H_Call,0},			"ID"},
	{"--splitcall",			1, 1, SplitCall,		{H_Call,0},			"ID"},
	{"--switchcall",		0, 1, SwitchCall,		{H_Call,0},			"[ID]"},
	{"--transfercall",		0, 1, TransferCall,		{H_Call,0},			"[ID]"},
	{"--divert",			3, 5, CallDivert,		{H_Call,0},			"get|set all|busy|noans|outofreach all|voice|fax|data [number timeout]"},
	{"--canceldiverts",		0, 0, CancelAllDiverts,		{H_Call,0},			""},
	{"--senddtmf",			1, 1, SendDTMF,			{H_Call,0},			"sequence"},
	{"--getcalendarsettings",	0, 0, GetCalendarSettings,	{H_Calendar,H_Settings,0},	""},
	{"--getalltodo",		0, 0, GetAllToDo,		{H_ToDo,0},			""},
	{"--listtodocategory",		1, 1, ListToDoCategory,		{H_ToDo, H_Category,0},		"text|number"},
	{"--gettodo",			1, 2, GetToDo,			{H_ToDo,0},			"start [stop]"},
	{"--deletetodo",		1, 2, DeleteToDo,		{H_ToDo,0},			"start [stop]"},
	{"--getallnotes",		0, 0, GetAllNotes,		{H_Note,0},			""},
	{"--deletecalendar",		1, 2, DeleteCalendar,		{H_Calendar,0},			"start [stop]"},
	{"--getallcalendar",		0, 0, GetAllCalendar,		{H_Calendar,0},			""},
	{"--getcalendar",		1, 2, GetCalendar,		{H_Calendar,0},			"start [stop]"},
	{"--getcategory",       	2, 3, GetCategory,       	{H_Category,H_ToDo,H_Memory,0},	"TODO|PHONEBOOK start [stop]"},
	{"--getallcategory",	  	1, 1, GetAllCategories,  	{H_Category,H_ToDo,H_Memory,0},	"TODO|PHONEBOOK"},
	{"--reset",			1, 1, Reset,			{H_Other,0},			"SOFT|HARD"},
	{"--getprofile",		1, 2, GetProfile,		{H_Settings,0},			"start [stop]"},
	{"--getsecuritystatus",		0, 0, GetSecurityStatus,	{H_Info,0},			""},
	{"--entersecuritycode",		2, 2, EnterSecurityCode,	{H_Other,0},			"PIN|PUK|PIN2|PUK2 code"},
	{"--deletewapbookmark", 	1, 2, DeleteWAPBookmark, 	{H_WAP,0},			"start [stop]"},
	{"--getwapbookmark",		1, 2, GetWAPBookmark,		{H_WAP,0},			"start [stop]"},
	{"--getwapsettings",		1, 2, GetWAPMMSSettings,	{H_WAP,0},			"start [stop]"},
	{"--getmmssettings",		1, 2, GetWAPMMSSettings,	{H_MMS,0},			"start [stop]"},
	{"--getsyncmlsettings",		1, 2, GetSyncMLSettings,	{H_WAP,0},			"start [stop]"},
	{"--getchatsettings",		1, 2, GetChatSettings,		{H_WAP,0},			"start [stop]"},
	{"--savemmsfile",		3, 15,SaveMMSFile,		{H_MMS,0},			"file [-subject text][-text text]"},
	{"--getbitmap",			1, 3, GetBitmap,		{H_Logo,0},			"STARTUP [file]"},
	{"--getbitmap",			1, 3, GetBitmap,		{H_Logo,0},			"CALLER location [file]"},
	{"--getbitmap",			1, 3, GetBitmap,		{H_Logo,0},			"OPERATOR [file]"},
	{"--getbitmap",			1, 3, GetBitmap,		{H_Logo,0},			"PICTURE location [file]"},
	{"--getbitmap",			1, 3, GetBitmap,		{H_Logo,0},			"TEXT"},
	{"--getbitmap",			1, 3, GetBitmap,		{H_Logo,0},			"DEALER"},
	{"--setbitmap",			1, 4, SetBitmap,		{H_Logo,0},			"STARTUP file|1|2|3"},
	{"--setbitmap",			1, 4, SetBitmap,		{H_Logo,0},			"COLOURSTARTUP [fileID]"},
	{"--setbitmap",			1, 4, SetBitmap,		{H_Logo,0},			"WALLPAPER fileID"},
	{"--setbitmap",			1, 4, SetBitmap,		{H_Logo,0},			"CALLER location [file]"},
	{"--setbitmap",			1, 4, SetBitmap,		{H_Logo,0},			"OPERATOR [file [netcode]]"},
	{"--setbitmap",			1, 4, SetBitmap,		{H_Logo,0},			"COLOUROPERATOR [fileID [netcode]]"},
	{"--setbitmap",			1, 4, SetBitmap,		{H_Logo,0},			"PICTURE file location [text]"},
	{"--setbitmap",			1, 4, SetBitmap,		{H_Logo,0},			"TEXT text"},
	{"--setbitmap",			1, 4, SetBitmap,		{H_Logo,0},			"DEALER text"},
	{"--copybitmap",		1, 3, CopyBitmap,		{H_Logo,0},			"inputfile [outputfile [OPERATOR|PICTURE|STARTUP|CALLER]]"},
	{"--presskeysequence",		1, 1, PressKeySequence,		{H_Other,0},			"mMnNpPuUdD+-123456789*0#gGrRwW"},
#if defined(WIN32) || defined(HAVE_PTHREAD)
	{"--searchphone",		0, 1, SearchPhone,		{H_Other,0},			"[-debug]"},
#endif
#ifdef GSM_ENABLE_BACKUP
	{"--savefile",			4, 5, SaveFile,			{H_Backup,H_Calendar,0},	"CALENDAR target.vcs file location"},
	{"--savefile",			4, 5, SaveFile,			{H_Backup,H_ToDo,0},		"TODO target.vcs file location"},
	{"--savefile",			4, 5, SaveFile,			{H_Backup,H_Memory,0},		"VCARD10|VCARD21 target.vcf file SM|ME location"},
	{"--savefile",			4, 5, SaveFile,			{H_Backup,H_WAP,0},		"BOOKMARK target.url file location"},
	{"--backup",			1, 2, Backup,			{H_Backup,H_Memory,H_Calendar,H_ToDo,H_Category,H_Ringtone,H_WAP,H_FM,0},			"file [-yes]"},
	{"--backupsms",			1, 1, BackupSMS,		{H_Backup,H_SMS,0},		"file"},
	{"--restore",			1, 1, Restore,			{H_Backup,H_Memory,H_Calendar,H_ToDo,H_Category,H_Ringtone,H_WAP,H_FM,0},			"file"},
	{"--addnew",			1, 1, AddNew,			{H_Backup,H_Memory,H_Calendar,H_ToDo,H_Category,H_Ringtone,H_WAP,H_FM,0},			"file"},
	{"--restoresms",		1, 1, RestoreSMS,		{H_Backup,H_SMS,0},		"file"},
	{"--addsms",			2, 2, AddSMS,			{H_Backup,H_SMS,0},		"folder file"},
#endif
	{"--clearall",			0, 0, ClearAll,			{H_Memory,H_Calendar,H_ToDo,H_Category,H_Ringtone,H_WAP,H_FM,0},	""},
	{"--networkinfo",		0, 0, NetworkInfo,		{H_Network,0},			""},
#ifdef GSM_ENABLE_AT
	{"--siemenssatnetmon",		0, 0, ATSIEMENSSATNetmon,	{H_Siemens,H_Network,0},	""},
	{"--siemensnetmonact",		1, 1, ATSIEMENSActivateNetmon,	{H_Siemens,H_Network,0},	"netmon_type (1-full, 2-simple)"},
	{"--siemensnetmonitor",		1, 1, ATSIEMENSNetmonitor,	{H_Siemens,H_Network,0},	"test"},
#endif
#ifdef GSM_ENABLE_NOKIA6110
	{"--nokiagetoperatorname", 	0, 0, DCT3GetOperatorName,	{H_Nokia,H_Network,0},		""},
	{"--nokiasetoperatorname", 	0, 2, DCT3SetOperatorName,	{H_Nokia,H_Network,0},		"[networkcode name]"},
	{"--nokiadisplayoutput", 	0, 0, DCT3DisplayOutput,	{H_Nokia,0},			""},
#endif
#ifdef GSM_ENABLE_NOKIA_DCT3
	{"--nokianetmonitor",		1, 1, DCT3netmonitor,		{H_Nokia,H_Network,0},		"test"},
	{"--nokianetmonitor36",		0, 0, DCT3ResetTest36,		{H_Nokia,0},			""},
	{"--nokiadebug",		1, 2, DCT3SetDebug,		{H_Nokia,H_Network,0},		"filename [[v11-22][,v33-44]...]"},
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
	{"--nokiasetvibralevel",	1, 1, DCT4SetVibraLevel,	{H_Nokia,H_Other,0},		"level"},
	{"--nokiagetvoicerecord",	1, 1, DCT4GetVoiceRecord,	{H_Nokia,H_Other,0},		"location"},
#ifdef GSM_ENABLE_NOKIA6510
	{"--nokiasetlights",		2, 2, DCT4SetLight,		{H_Nokia,H_Tests,0},		"keypad|display|torch on|off"},
	{"--nokiatuneradio",		0, 0, DCT4TuneRadio,		{H_Nokia,H_FM,0},		""},
#endif
	{"--nokiamakecamerashoot",	0, 0, DCT4MakeCameraShoot,	{H_Nokia,H_Other,0},		""},
	{"--nokiagetscreendump",	0, 0, DCT4GetScreenDump,	{H_Nokia,H_Other,0},		""},
#endif
#if defined(GSM_ENABLE_NOKIA_DCT3) || defined(GSM_ENABLE_NOKIA_DCT4)
	{"--nokiavibratest",		0, 0, NokiaVibraTest,		{H_Nokia,H_Tests,0},		""},
	{"--nokiagett9",		0, 0, NokiaGetT9,		{H_Nokia,H_SMS,0},		""},
	{"--nokiadisplaytest",		1, 1, NokiaDisplayTest,		{H_Nokia,H_Tests,0},		"number"},
	{"--nokiagetadc",		0, 0, NokiaGetADC,		{H_Nokia,H_Tests,0},		""},
	{"--nokiasecuritycode",		0, 0, NokiaSecurityCode,	{H_Nokia,H_Info,0},		""},
	{"--nokiaselftests",		0, 0, NokiaSelfTests,		{H_Nokia,H_Tests,0},		""},
	{"--nokiasetphonemenus",	0, 0, NokiaSetPhoneMenus,	{H_Nokia,H_Other,0},		""},
#endif
#ifdef DEBUG
	{"--decodesniff",		2, 3, decodesniff,		{H_Decode,0},			"MBUS2|IRDA file [phonemodel]"},
	{"--decodebinarydump",		1, 2, decodebinarydump,		{H_Decode,0},			"file [phonemodel]"},
	{"--makeconverttable",		1, 1, MakeConvertTable,		{H_Decode,0},			"file"},
#endif
	{"",				0, 0, NULL			}
};

static HelpCategoryDescriptions HelpDescriptions[] = {
	{H_Call,	"call",		"Calls",},
	{H_SMS,		"sms",		"SMS and EMS"},
	{H_Memory,	"memory",	"Memory (phonebooks and calls)"},
	{H_Filesystem,	"filesystem",	"Filesystem"},
	{H_Logo,	"logo",		"Logo and pictures"},
	{H_Ringtone,	"ringtone",	"Ringtones"},
	{H_Calendar,	"calendar",	"Calendar notes"},
	{H_ToDo,	"todo",		"To do lists"},
	{H_Note,	"note",		"Notes"},
	{H_DateTime,	"datetime",	"Date, time and alarms"},
	{H_Category,	"category",	"Categories"},
#ifdef GSM_ENABLE_BACKUP
	{H_Backup,	"backup",	"Backing up and restoring"},
#endif
#if defined(GSM_ENABLE_NOKIA_DCT3) || defined(GSM_ENABLE_NOKIA_DCT4)
	{H_Nokia,	"nokia",	"Nokia specific"},
#endif
#ifdef GSM_ENABLE_AT
	{H_Siemens,	"siemens",	"Siemens specific"},
#endif
	{H_Network,	"network",	"Network"},
	{H_WAP,		"wap",		"WAP settings and bookmarks"},
	{H_MMS,		"mms",		"MMS and MMS settings"},
	{H_Tests,	"tests",	"Phone tests"},
	{H_FM,		"fm",		"FM radio"},
	{H_Info,	"info",		"Phone information"},
	{H_Settings,	"settings",	"Phone settings"},
#ifdef DEBUG
	{H_Decode,	"decode",	"Dumps decoding"},
#endif
	{H_Other,	"other",	"Functions that don't fit elsewhere"},
	{0,		NULL,		NULL}
};


void HelpHeader(void)
{
	printmsg("[Gammu version %s built %s %s]\n\n",VERSION,__TIME__,__DATE__);
}

static void HelpGeneral(void)
{
	int	i=0;

	HelpHeader();

 	printmsg("Usage: gammu [confign] [nothing|text|textall|binary|errors] [options]\n\n");
 	printmsg("First parameter optionally specifies which config section to use (all are probed by default).\n");
 	printmsg("Second parameter optionally controls debug level, next one specifies actions.\n\n");

	/* We might want to put here some most used commands */
	printmsg("For more details, call help on specific topic (gammu --help topic). Topics are:\n\n");

	while (HelpDescriptions[i].category != 0) {
		printf("%11s - %s\n", HelpDescriptions[i].option, HelpDescriptions[i].description);
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
			if (mystrncasecmp(argv[2], HelpDescriptions[i].option,strlen(argv[2]))) break;
			i++;
		}
		if (HelpDescriptions[i].category == 0) {
			HelpGeneral();
			printmsg("Unknown help topic specified!\n");
			return;
		}
		HelpHeader();
		printmsg("Gammu parameters, topic: %s\n\n", HelpDescriptions[i].description);
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
				HelpSplit(cols - 1, strlen(Parameters[j].parameter) + 1, Parameters[j].help);
			}
		}
		j++;
	}
}

int main(int argc, char *argv[])
{
	int 	z = 0,start=0,i;
	int	only_config = -1;
#if !defined(WIN32) && !defined(DJGPP) && defined(LOCALE_PATH)
	char	*locale, locale_file[201];
#endif
	char	*cp;
 	bool	count_failed = false;

	s.opened 	= false;
	s.msg	 	= NULL;
	s.ConfigNum 	= 0;

	setlocale(LC_ALL, "");
#ifdef DEBUG
	di.dl		= DL_TEXTALL;
	di.df	 	= stdout;
#endif

 	/* Any parameters? */
	if (argc == 1) {
		HelpGeneral();
		printmsg("Too few parameters!\n");
		exit(1);
	}

 	/* Help? */
	if (strncmp(argv[1 + start], "--help", 6) == 0) {
		Help(argc - start, argv + start);
		exit(1);
	}

 	/* Is first parameter numeric? If so treat it as config that should be loaded. */
	if (isdigit(argv[1][0])) {
		only_config = atoi(argv[1]);
		if (only_config >= 0) start++; else only_config = -1;
	}

 	cfg = GSM_FindGammuRC();
 	if (cfg == NULL) printmsg("Warning: No configuration file found!\n");

	for (i = 0; i <= MAX_CONFIG_NUM; i++) {
		if (cfg!=NULL) {
		        cp = INI_GetValue(cfg, "gammu", "gammucoding", false);
        		if (cp) di.coding = cp;

		        s.Config[i].Localize = INI_GetValue(cfg, "gammu", "gammuloc", false);
        		if (s.Config[i].Localize) {
				s.msg=INI_ReadFile(s.Config[i].Localize, true);
			} else {
#if !defined(WIN32) && !defined(DJGPP) && defined(LOCALE_PATH)
 				locale = setlocale(LC_MESSAGES, NULL);
 				if (locale != NULL) {
					snprintf(locale_file, 200, "%s/gammu_%c%c.txt",
							LOCALE_PATH,
							tolower(locale[0]),
							tolower(locale[1]));
					s.msg = INI_ReadFile(locale_file, true);
				}
#endif
			}
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

 		/* We wanted to read just user specified configuration. */
 		if (only_config != -1) {break;}
 	}

	/* Do we have enough parameters? */
 	if (argc == 1 + start) {
 		HelpGeneral();
 		printmsg("Too few parameters!\n");
		exit(-2);
	}

	/* Check used version vs. compiled */
	if (!mystrncasecmp(GetGammuVersion(),VERSION,0)) {
		printmsg("ERROR: version of installed libGammu.so (%s) is different to version of Gammu (%s)\n",
					GetGammuVersion(),VERSION);
		exit(-1);
	}

	/* Check parameters */
	while (Parameters[z].Function != NULL) {
		if (mystrncasecmp(Parameters[z].parameter,argv[1+start], 0)) {
			if (argc-2-start >= Parameters[z].min_arg && argc-2-start <= Parameters[z].max_arg) {
				Parameters[z].Function(argc - start, argv + start);
 				break;
			} else {
				count_failed = true;
 			}
 		}
		z++;
	}

	/* Tell user when we did nothing */
	if (Parameters[z].Function == NULL) {
		HelpGeneral();
		if (count_failed) {
			printmsg("Bad parameter count!\n");
		} else {
			printmsg("Bad option!\n");
 		}
 	}

     	/* Close debug output if opened */
     	if (di.df!=stdout) fclose(di.df);

	exit(0);
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */


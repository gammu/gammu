
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#ifdef WIN32
#  include <windows.h>
#endif
#include <locale.h>
#include <signal.h>

#include "../common/gsmcomon.h"
#include "../common/gsmstate.h"
#include "../common/misc/cfg.h"
#include "../common/misc/misc.h"
#include "../common/misc/coding.h"
#include "../common/service/gsmpbk.h"
#include "../common/service/gsmring.h"
#include "../common/service/gsmlogo.h"
#include "../common/service/gsmback.h"
#include "../common/service/gsmnet.h"
#include "../common/phone/pfunc.h"
#include "../common/phone/nokia/nfunc.h"
#include "gammu.h"
#include "smsd/smsdcore.h"
#ifdef DEBUG
#  include "sniff.h"
#endif
#ifdef GSM_ENABLE_NOKIA_DCT3
#  include "depend/dct3.h"
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
#  include "depend/dct4.h"
#endif
                			
GSM_StateMachine		s;
GSM_Phone_Functions		*Phone;
static CFG_Header		*cfg 			= NULL;

GSM_Error			error 			= GE_NONE;
static int			i;

volatile bool 			bshutdown 		= false;

/* SIGINT signal handler. */
void interrupted(int sig)
{
	signal(sig, SIG_IGN);
	bshutdown = true;
}

int printmsg(char *format, ...)
{
	va_list		argp;
	int 		result;

	va_start(argp, format);
	result = vfprintf(stdout,GetMsg(s.msg,format),argp);
	va_end(argp);
	return result;
}

int printmsgerr(char *format, ...)
{
	va_list		argp;
	int 		result;

	va_start(argp, format);
	result = vfprintf(stderr,GetMsg(s.msg,format), argp);
	va_end(argp);
	return result;
}

void Print_Error(GSM_Error error)
{
	if (error!=GE_NONE) {
		printf("%s\n",print_error(error,s.di.df,s.msg));
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
		printmsg("ERROR: enumerate locations from 1");
		exit (-1);
	}

	if (stop!=NULL) {
		*stop=*start;
		if (argc>=num+2) *stop=atoi(argv[num+1]);
		if (*stop==0) {
			printmsg("ERROR: enumerate locations from 1");
			exit (-1);
		}
	}
}

static bool answer_yes(char *text)
{
    	int         len;
    	char        ans[99];
    	static bool always_answer_yes   =   false;
    	static bool always_answer_no    =   false;

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
	if (error != GE_NOTSUPPORTED && error != GE_NOTIMPLEMENTED) Print_Error(error);
}
#endif

static GSM_Error GSM_PlayRingtone(GSM_Ringtone ringtone)
{
	int 		i;
	bool 		first=true;
	GSM_Error 	error;

	/* We do not want to make it forever - 
	 * press Ctrl+C to stop the monitoring mode.
	 */
	signal(SIGINT, interrupted);
	printmsg("If you want break, press Ctrl+C...\n");

	for (i=0;i<ringtone.NoteTone.NrCommands;i++) {
		if (bshutdown) break;
		if (ringtone.NoteTone.Commands[i].Type == RING_NOTETONE) {
			error=PHONE_RTTLPlayOneNote(&s,ringtone.NoteTone.Commands[i].Note,first);
			if (error!=GE_NONE) return error;
			first = false;
		}
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

	error=Phone->GetManufacturer(&s, buffer);
	Print_Error(error);
	printmsg("Model         : %s %s (%s)\n",
			DecodeUnicodeString(buffer),
			GetModelData(NULL,s.Model,NULL)->model, s.Model);

	printmsg("Firmware      : %s",s.Ver);
	error=Phone->GetPPM(&s, buffer);
	if (error != GE_NOTSUPPORTED) {
		if (error != GE_NOTIMPLEMENTED) Print_Error(error);
		if (error == GE_NONE) printmsg(" %s",buffer);
	}
	if (s.VerDate[0]!=0) printmsg(" (%s)",s.VerDate);
	printmsg("\n");

	error=Phone->GetHardware(&s, buffer);
	if (error != GE_NOTSUPPORTED) {
		if (error != GE_NOTIMPLEMENTED) Print_Error(error);
		if (error == GE_NONE) printmsg("Hardware      : %s\n",buffer);
	}

	error=Phone->GetIMEI(&s, buffer);
	if (error != GE_NOTSUPPORTED) {
		if (error != GE_NOTIMPLEMENTED) Print_Error(error);
		if (error == GE_NONE) printmsg("IMEI          : %s\n",buffer);

		error=Phone->GetOriginalIMEI(&s, buffer);
		if (error != GE_NOTSUPPORTED && error != GE_SECURITYERROR) {
			if (error != GE_NOTIMPLEMENTED) Print_Error(error);
			if (error == GE_NONE) printmsg("Original IMEI : %s\n",buffer);
		}
	}

	error=Phone->GetManufactureMonth(&s, buffer);
	if (error != GE_NOTSUPPORTED && error != GE_SECURITYERROR) {
		if (error != GE_NOTIMPLEMENTED) Print_Error(error);
		if (error == GE_NONE) printmsg("Manufactured  : %s\n",buffer);
	}

	error=Phone->GetProductCode(&s, buffer);
	if (error != GE_NOTSUPPORTED) {
		if (error != GE_NOTIMPLEMENTED) Print_Error(error);
		if (error == GE_NONE) printmsg("Product code  : %s\n",buffer);
	}

	error=Phone->GetSIMIMSI(&s, buffer);
	switch (error) {
		case GE_SECURITYERROR:
		case GE_NOTSUPPORTED:
		case GE_NOTIMPLEMENTED:
			break;
		case GE_NONE:
			printmsg("SIM IMSI      : %s\n",buffer);
			break;
		default:
			Print_Error(error);
	}

#ifdef GSM_ENABLE_NOKIA_DCT3
	DCT3Info(argc, argv);
#endif

	GSM_Terminate();
}

static void GetDateTime(int argc, char *argv[])
{
	GSM_DateTime date_time;

	GSM_Init(true);

	error=Phone->GetDateTime(&s, &date_time);
	switch (error) {
	case GE_EMPTY:
		printmsg("Date and time not set in phone\n");
		break;
	case GE_NONE:
		printmsg("Phone time is %s\n",OSDateTime(date_time,false));
		break;
	default:
		Print_Error(error);
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
	GSM_DateTime alarm;

	GSM_Init(true);

	error=Phone->GetAlarm(&s, &alarm, 1);
	switch (error) {
	case GE_EMPTY:
		printmsg("Alarm not set in phone\n");
		break;
	case GE_NONE:
		printmsg("Time: %02d:%02d\n",alarm.Hour, alarm.Minute);
		break;
	default:
		Print_Error(error);
	}

	GSM_Terminate();
}

static void SetAlarm(int argc, char *argv[])
{
	GSM_DateTime alarm;

	alarm.Hour 	= atoi(argv[2]);
	alarm.Minute 	= atoi(argv[3]);

	GSM_Init(true);

	error=Phone->SetAlarm(&s, &alarm, 1);
	Print_Error(error);

	GSM_Terminate();
}

static void GetMemory(int argc, char *argv[])
{
	GSM_PhonebookEntry	entry;
	GSM_Bitmap		caller[5];
	bool			callerinit[5];
	int			j, start, stop;
	bool			unknown;

	entry.MemoryType=0;
	if (mystrncasecmp(argv[2],"DC",0)) entry.MemoryType=GMT_DC;
	if (mystrncasecmp(argv[2],"ON",0)) entry.MemoryType=GMT_ON;
	if (mystrncasecmp(argv[2],"RC",0)) entry.MemoryType=GMT_RC;
	if (mystrncasecmp(argv[2],"MC",0)) entry.MemoryType=GMT_MC;
	if (mystrncasecmp(argv[2],"ME",0)) entry.MemoryType=GMT_ME;
	if (mystrncasecmp(argv[2],"SM",0)) entry.MemoryType=GMT_SM;
	if (mystrncasecmp(argv[2],"VM",0)) entry.MemoryType=GMT_VM;
	if (entry.MemoryType==0) {
		printmsg("ERROR: unknown memory type (\"%s\")\n",argv[2]);
		exit (-1);
	}

	GetStartStop(&start, &stop, 3, argc, argv);

	for (i=0;i<5;i++) callerinit[i] = false;

	GSM_Init(true);

	if (!strcmp(GetModelData(NULL,s.Model,NULL)->model,"3310")) {
		if (s.VerNum<=4.06) printmsg("WARNING: you will have null names in entries. Upgrade firmware in phone to higher than 4.06\n");
	}

	for (j=start;j<=stop;j++)
	{
		printmsg("Memory %s, Location %i\n",argv[2],j);

		entry.Location=j;

		error=Phone->GetMemory(&s, &entry);
		if (error != GE_EMPTY) Print_Error(error);

		if (error == GE_EMPTY) {
			printmsg("Entry is empty\n");
		} else {
			for (i=0;i<entry.EntriesNum;i++) {
				unknown = false;
				switch (entry.Entries[i].EntryType) {
				case PBK_Date:
					printmsg("Date and time   : %s\n",OSDateTime(entry.Entries[i].Date,false));
					continue;
				case PBK_Category:
                    			printmsg("Category        : %i\n", entry.Entries[i].Number);
                    			continue;
                		case PBK_Private:
                    			printmsg("Private         : %s\n", entry.Entries[i].Number == 1 ? "Yes" : "No");
                    			continue;
				case PBK_Number_General     : printmsg("General number "); break;
				case PBK_Number_Mobile      : printmsg("Mobile number  "); break;
				case PBK_Number_Work        : printmsg("Work number    "); break;
				case PBK_Number_Fax         : printmsg("Fax number     "); break;
				case PBK_Number_Home        : printmsg("Home number    "); break;
				case PBK_Number_Pager       : printmsg("Pager number   "); break;
                		case PBK_Number_Other       : printmsg("Other number   "); break;
				case PBK_Text_Note          : printmsg("Text           "); break;
				case PBK_Text_Postal        : printmsg("Snail address  "); break;
				case PBK_Text_Email         : printmsg("Email address  "); break;
				case PBK_Text_URL           : printmsg("URL address    "); break;
				case PBK_Name               : printmsg("Name           "); break;
                		case PBK_Text_LastName      : printmsg("Last name      "); break;
				case PBK_Text_FirstName     : printmsg("First name     "); break;
				case PBK_Text_Company       : printmsg("Company        "); break;
				case PBK_Text_JobTitle      : printmsg("Job title      "); break;
				case PBK_Text_StreetAddress : printmsg("Street address "); break;
				case PBK_Text_City          : printmsg("City           "); break;
				case PBK_Text_State         : printmsg("State          "); break;
				case PBK_Text_Zip           : printmsg("Zip code       "); break;
				case PBK_Text_Country       : printmsg("Country        "); break;
				case PBK_Text_Custom1       : printmsg("Custom text 1  "); break;
				case PBK_Text_Custom2       : printmsg("Custom text 2  "); break;
				case PBK_Text_Custom3       : printmsg("Custom text 3  "); break;
				case PBK_Text_Custom4       : printmsg("Custom text 4  "); break;
				case PBK_Caller_Group       :
					unknown = true;
					if (!callerinit[entry.Entries[i].Number]) {
						caller[entry.Entries[i].Number].Type	 = GSM_CallerLogo;
						caller[entry.Entries[i].Number].Location = entry.Entries[i].Number;
						error=Phone->GetBitmap(&s,&caller[entry.Entries[i].Number]);
						if (error == GE_SECURITYERROR) {
							NOKIA_GetDefaultCallerGroupName(&s,&caller[entry.Entries[i].Number]);
						} else {
							Print_Error(error);
						}
						callerinit[entry.Entries[i].Number]=true;
					}
					printmsg("Caller group    : \"%s\"\n",DecodeUnicodeString(caller[entry.Entries[i].Number].Text));
					break;
				default		       :
					printmsg("UNKNOWN\n");
					unknown = true;
					break;
				}
				if (!unknown) printmsg(" : \"%s\"\n", DecodeUnicodeString(entry.Entries[i].Text));
			}
		}
		printmsg("\n");
	}
	
	GSM_Terminate();
}

static void displaysinglesmsinfo(GSM_SMSMessage sms, bool displaytext)
{
	switch (sms.PDU) {
	case SMS_Status_Report:
		printmsg("SMS status report\n");
		printmsg("Status          : ");
		switch (sms.State) {
			case GSM_Sent	: printmsg("Sent");	break;
			case GSM_Read	: printmsg("Read");	break;
			case GSM_UnRead	: printmsg("UnRead");	break;
			case GSM_UnSent	: printmsg("UnSent");	break;
		}
		printmsg("\nRemote number   : \"%s\"\n",DecodeUnicodeString(sms.Number));
		printmsg("Sent            : %s\n",OSDateTime(sms.DateTime,true));
		printmsg("SMSC number     : \"%s\"\n",DecodeUnicodeString(sms.SMSC.Number));
		printmsg("SMSC response   : %s\n",OSDateTime(sms.SMSCTime,true));
		printmsg("Delivery status : %s\n",DecodeUnicodeString(sms.Text));
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
			case 0x21: printmsg("SME busy");						break;
			case 0x22: printmsg("No response from SME");				break;
			case 0x23: printmsg("Service rejected");					break;
			case 0x24: printmsg("Quality of service not aviable");			break;
			case 0x25: printmsg("Error in SME");					break;
		        case 0x40: printmsg("Remote procedure error");				break;
		        case 0x41: printmsg("Incompatibile destination");				break;
		        case 0x42: printmsg("Connection rejected by SME");			break;
		        case 0x43: printmsg("Not obtainable");					break;
		        case 0x44: printmsg("Quality of service not available");			break;
		        case 0x45: printmsg("No internetworking available");			break;
		        case 0x46: printmsg("SM Validity Period Expired");			break;
		        case 0x47: printmsg("SM deleted by originating SME");			break;
		        case 0x48: printmsg("SM Deleted by SC Administration");			break;
		        case 0x49: printmsg("SM does not exist");					break;
		        case 0x60: printmsg("Congestion");					break;
		        case 0x61: printmsg("SME busy");						break;
		        case 0x62: printmsg("No response from SME");				break;
		        case 0x63: printmsg("Service rejected");					break;
		        case 0x64: printmsg("Quality of service not available");			break;
		        case 0x65: printmsg("Error in SME");					break;
		        default  : printmsg("Reserved/Specific to SC: %x",sms.DeliveryStatus);	break;
		}
		printmsg("\n");
		break;
	case SMS_Deliver:
		printmsg("SMS message\n");
		printmsg("SMSC number      : \"%s\"",DecodeUnicodeString(sms.SMSC.Number));
		if (sms.ReplyViaSameSMSC) printmsg(" (set for reply)");
		printmsg("\nSent             : %s\n",OSDateTime(sms.DateTime,true));
		/* No break. The only difference for SMS_Deliver and SMS_Submit is,
		 * that SMS_Deliver contains additional data. We wrote them and then go
		 * for data shared with SMS_Submit
		 */
	case SMS_Submit:
		if (sms.ReplaceMessage != 0) printmsg("SMS replacing ID : %i\n",sms.ReplaceMessage);
		/* If we went here from "case SMS_Deliver", we don't write "SMS Message" */
		if (sms.PDU==SMS_Submit) printmsg("SMS message\n");
		if (sms.Name[0] != 0x00 || sms.Name[1] != 0x00) {
			printmsg("Name             : \"%s\"\n",DecodeUnicodeString(sms.Name));
		}
		if (sms.Class != -1) {
			printmsg("Class            : %i\n",sms.Class);
		}
		printmsg("Coding           : ");
		switch (sms.Coding) {
			case GSM_Coding_Unicode : printmsg("Unicode\n");			break;
			case GSM_Coding_Default : printmsg("Default GSM alphabet\n");	break;
			case GSM_Coding_8bit	: printmsg("8 bit\n");			break;
		}
		printmsg("Status           : ");
		switch (sms.State) {
			case GSM_Sent	:	printmsg("Sent");		break;
			case GSM_Read	:	printmsg("Read");		break;
			case GSM_UnRead	:	printmsg("UnRead");	break;
			case GSM_UnSent	:	printmsg("UnSent");	break;
		}
		printmsg("\nRemote number    : \"%s\"\n",DecodeUnicodeString(sms.Number));
		if (sms.UDH.Type != UDH_NoUDH) printmsg("User Data Header : ");
		switch (sms.UDH.Type) {
		case UDH_ConcatenatedMessages:	printmsg("Concatenated (linked) message"); break;
		case UDH_DisableVoice:		printmsg("Disables voice indicator");	 break;
		case UDH_EnableVoice:		printmsg("Enables voice indicator");	 break;
		case UDH_DisableFax:		printmsg("Disables fax indicator");	 break;
		case UDH_EnableFax:		printmsg("Enables fax indicator");	 break;
		case UDH_DisableEmail:		printmsg("Disables email indicator");	 break;
		case UDH_EnableEmail:		printmsg("Enables email indicator");	 break;
		case UDH_VoidSMS:		printmsg("Void SMS");			 break;
		case UDH_NokiaWAPBookmark:	printmsg("Nokia WAP Bookmark");		 break;
		case UDH_NokiaOperatorLogoLong:	printmsg("Nokia operator logo");		 break;
		case UDH_NokiaWAPBookmarkLong:	printmsg("Nokia WAP Bookmark");		 break;
		case UDH_NokiaWAPSettingsLong:	printmsg("Nokia WAP Settings");		 break;
		case UDH_NokiaRingtone:		printmsg("Nokia ringtone");		 break;
		case UDH_NokiaOperatorLogo:	printmsg("Nokia GSM operator logo");	 break;
		case UDH_NokiaCallerLogo:	printmsg("Nokia caller logo");		 break;  	
		case UDH_NokiaProfileLong:	printmsg("Nokia profile");		 break;
		case UDH_NokiaCalendarLong:	printmsg("Nokia calendar note");		 break;
		case UDH_NokiaPhonebookLong:	printmsg("Nokia phonebook entry");	 break;
		case UDH_UserUDH:		printmsg("User UDH");			 break;
		case UDH_NoUDH:								 break;
		}
		if (sms.UDH.Type != UDH_NoUDH) {
			if (sms.UDH.ID != -1) printmsg(", ID %i",sms.UDH.ID);
			if (sms.UDH.PartNumber != -1 && sms.UDH.AllParts != -1) {
				printmsg(", part %i of %i",sms.UDH.PartNumber,sms.UDH.AllParts);
			}
		}
		printmsg("\n");
		if (displaytext) {
			if (sms.Coding!=GSM_Coding_8bit) {
				printmsg("%s\n",DecodeUnicodeString(sms.Text));
			} else {
				printmsg("8 bit SMS, cannot be displayed here\n");
			}
		}
		break;
	}
}

static void displaymultismsinfo (GSM_MultiSMSMessage sms)
{
	GSM_EncodeMultiPartSMSInfo	SMSInfo;
	GSM_MultiBitmap			Bitmap;
	GSM_Ringtone			Ringtone;
	char				Buffer[500];
	bool				RetVal;
	int				j;

	SMSInfo.Entries[0].Bitmap 		= &Bitmap;
	SMSInfo.Entries[0].Ringtone 	= &Ringtone;
	SMSInfo.Entries[0].Buffer		= Buffer;

	/* GSM_DecodeMultiPartSMS returns if decoded SMS contenst correctly */
	RetVal = GSM_DecodeMultiPartSMS(&SMSInfo,&sms);

	for (j=0;j<sms.Number;j++) {
		displaysinglesmsinfo(sms.SMS[j],!RetVal);
		printmsg("\n");
	}
	if (RetVal) {
		if (SMSInfo.Entries[0].Bitmap != NULL) {
			switch (SMSInfo.Entries[0].Bitmap->Bitmap[0].Type) {
			case GSM_CallerLogo:
				printmsg("Caller logo\n");
				break;
			case GSM_OperatorLogo:
				printmsg("Operator logo for %s network (%s, %s)\n",
					SMSInfo.Entries[0].Bitmap->Bitmap[0].NetworkCode,
					DecodeUnicodeString(GSM_GetNetworkName(SMSInfo.Entries[0].Bitmap->Bitmap[0].NetworkCode)),
					DecodeUnicodeString(GSM_GetCountryName(SMSInfo.Entries[0].Bitmap->Bitmap[0].NetworkCode)));
				break;
			case GSM_PictureImage:
				printmsg("Picture Image\n");
				printmsg("Text: \"%s\"\n",DecodeUnicodeString(SMSInfo.Entries[0].Bitmap->Bitmap[0].Text));
				break;
			default:
				break;
			}
			GSM_PrintBitmap(stdout,&SMSInfo.Entries[0].Bitmap->Bitmap[0]);
			printmsg("\n");
		}
		if (SMSInfo.Entries[0].Buffer != NULL) {
			printmsg("Text\n%s\n",DecodeUnicodeString(Buffer));
		}
		if (SMSInfo.Entries[0].Ringtone != NULL) {
			printmsg("Ringtone \"%s\"\n",DecodeUnicodeString(SMSInfo.Entries[0].Ringtone->Name));
			saverttl(stdout,SMSInfo.Entries[0].Ringtone);
			printmsg("\n");
			if (s.Phone.Functions->PlayTone!=NOTSUPPORTED &&
			    s.Phone.Functions->PlayTone!=NOTIMPLEMENTED) {
				if (answer_yes("Do you want to play it")) GSM_PlayRingtone(*SMSInfo.Entries[0].Ringtone);
			}
		}
	}
}

static void IncomingSMS(char *Device, GSM_SMSMessage sms)
{
	GSM_MultiSMSMessage SMS;

	printmsg("SMS message received\n");
	SMS.Number = 1;
	memcpy(&SMS.SMS[0],&sms,sizeof(GSM_SMSMessage));	
	displaymultismsinfo(SMS);
}

static void IncomingCB(char *Device, GSM_CBMessage CB)
{
	printmsg("CB message received\n");
	printmsg("Channel %i, text \"%s\"\n",CB.Channel,DecodeUnicodeString(CB.Text));
}

static void IncomingCall(char *Device, GSM_Call call)
{
	printmsg("Call info : ");
	switch(call.Status) {
		case GN_CALL_IncomingCall  : printmsg("incoming call from \"%s\"\n",DecodeUnicodeString(call.PhoneNumber)); break;
		case GN_CALL_OutgoingCall  : printmsg("outgoing call to \"%s\"\n",DecodeUnicodeString(call.PhoneNumber));   break;
		case GN_CALL_CallStart     : printmsg("outgoing call started\n"); 					  break;
		case GN_CALL_CallEnd	   : printmsg("end of call (unknown side)\n"); 					  break;
		case GN_CALL_CallLocalEnd  : printmsg("call end from our side\n");					  break;
		case GN_CALL_CallRemoteEnd : printmsg("call end from remote side\n");					  break;
	}
}

static void IncomingUSSD(char *Device, char *Buffer)
{
	printmsg("Service reply: \"%s\"\n",DecodeUnicodeString(Buffer));
}

#define CHECKMEMORYSTATUS(x, m, a1, b1) { 				\
	x.MemoryType=m;							\
	if (Phone->GetMemoryStatus(&s, &x) == GE_NONE)			\
		printmsg("%s %03d, %s %03d\n", a1, x.Used, b1, x.Free);	\
}

static void Monitor(int argc, char *argv[])
{
	GSM_MemoryStatus	MemStatus;
	GSM_SMSMemoryStatus	SMSStatus;
	GSM_NetworkInfo		NetInfo;
    	GSM_BatteryCharge   	BatteryCharge;
    	GSM_SignalQuality   	SignalQuality;
	int 			count = -1;

	if (argc >= 3) {
		count = atoi(argv[2]);
		if (count <= 0) count = -1;
	}

	/* We do not want to monitor serial line forever -
	 * press Ctrl+C to stop the monitoring mode.
	 */
	signal(SIGINT, interrupted);
	printmsg("If you want break, press Ctrl+C...\n");
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

	/* Loop here indefinitely -
	 * allows you to see messages from GSM code in
	 * response to unknown messages etc.
	 * The loops ends after pressing the Ctrl+C.
	 */
	while (!bshutdown && count != 0) {
		if (count > 0) count--;
		CHECKMEMORYSTATUS(MemStatus,GMT_SM,"SIM phonebook     : Used","Free");
		CHECKMEMORYSTATUS(MemStatus,GMT_ME,"Phone phonebook   : Used","Free");
		CHECKMEMORYSTATUS(MemStatus,GMT_DC,"Dialled numbers   : Used","Free");
		CHECKMEMORYSTATUS(MemStatus,GMT_RC,"Received numbers  : Used","Free");
		CHECKMEMORYSTATUS(MemStatus,GMT_MC,"Missed numbers    : Used","Free");
		CHECKMEMORYSTATUS(MemStatus,GMT_ON,"Own numbers       : Used","Free");
		if (Phone->GetBatteryCharge(&s,&BatteryCharge)==GE_NONE) {
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
                		printmsg("\n");
            		}
        	}
		if (Phone->GetSignalQuality(&s,&SignalQuality)==GE_NONE) {
            		if (SignalQuality.SignalStrength != -1) printmsg("Signal strength   : %i dBm\n",     SignalQuality.SignalStrength);
            		if (SignalQuality.SignalPercent  != -1) printmsg("Network level     : %i percent\n", SignalQuality.SignalPercent);
            		if (SignalQuality.BitErrorRate   != -1) printmsg("Bit error rate    : %i percent\n", SignalQuality.BitErrorRate);
        	}
		if (Phone->GetSMSStatus(&s,&SMSStatus)==GE_NONE) {
			printmsg("SIM SMS status    : %i used, %i unread",
				SMSStatus.SIMUsed,SMSStatus.SIMUnRead);
			if (SMSStatus.SIMSize!=0) printmsg(", %i locations", SMSStatus.SIMSize);
			printmsg("\n");
			printmsg("Phone SMS status  : %i used, %i unread",
				SMSStatus.PhoneUsed,SMSStatus.PhoneUnRead);
			if (SMSStatus.PhoneSize!=0) printmsg(", %i locations", SMSStatus.PhoneSize);
			if (SMSStatus.TemplatesUsed!=0) printmsg(", %i templates", SMSStatus.TemplatesUsed);
			printmsg("\n");
		}
		if (Phone->GetNetworkInfo(&s,&NetInfo)==GE_NONE) {
			printmsg("Network state     : ");
                        switch (NetInfo.State) {
				case GSM_HomeNetwork		: printmsg("home network\n"); 		break;
				case GSM_RoamingNetwork		: printmsg("roaming network\n"); 		break;
				case GSM_RequestingNetwork	: printmsg("requesting network\n"); 	break;
				case GSM_NoNetwork		: printmsg("not logged into network\n"); 	break;
				default				: printmsg("unknown\n");
			}
			if (NetInfo.State == GSM_HomeNetwork || NetInfo.State == GSM_RoamingNetwork) {
				printmsg("Network           : %s (%s",	NetInfo.NetworkCode,DecodeUnicodeString(GSM_GetNetworkName(NetInfo.NetworkCode)));
				printmsg(", %s)",				DecodeUnicodeString(GSM_GetCountryName(NetInfo.NetworkCode)));
				printmsg(", LAC %s, CellID %s\n",		NetInfo.LAC,NetInfo.CellID);
				if (NetInfo.NetworkName[0] != 0x00 || NetInfo.NetworkName[1] != 0x00) {
					printmsg("Name in phone     : \"%s\"\n",DecodeUnicodeString(NetInfo.NetworkName));
				}
			}
		}
		printmsg("\n");
	}

	printmsg("Leaving monitor mode...\n");

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

		if (!strcmp(DecodeUnicodeString(smsc.Name),"")) {
			printmsg("%i. Set %i\n",smsc.Location, smsc.Location);
		} else {
			printmsg("%i. \"%s\"\n",smsc.Location, DecodeUnicodeString(smsc.Name));
		}
		printmsg("Number         : \"%s\"\n",DecodeUnicodeString(smsc.Number));
		printmsg("Default number : \"%s\"\n",DecodeUnicodeString(smsc.DefaultNumber));
	
		printmsg("Format         : ");
		switch (smsc.Format) {
			case GSMF_Text	: printmsg("Text");	break;
			case GSMF_Fax	: printmsg("Fax");	break;
			case GSMF_Email	: printmsg("Email");	break;
			case GSMF_Pager	: printmsg("Pager");	break;
		}
		printmsg("\n");

		printmsg("Validity       : ");
		switch (smsc.Validity.Relative) {
			case GSMV_1_Hour	: printmsg("1 hour");	  break;
			case GSMV_6_Hours 	: printmsg("6 hours");	  break;
			case GSMV_24_Hours	: printmsg("24 hours");	  break;
			case GSMV_72_Hours	: printmsg("72 hours");	  break;
			case GSMV_1_Week  	: printmsg("1 week"); 	  break;
			case GSMV_Max_Time	: printmsg("Maximum time"); break;
			default           	: printmsg("Unknown");
		}
		printmsg("\n");
	}

	GSM_Terminate();
}

static void GetSMS(int argc, char *argv[])
{
	GSM_MultiSMSMessage	sms;
	GSM_SMSFolders		folders;
	int			start, stop;

	GetStartStop(&start, &stop, 3, argc, argv);

	GSM_Init(true);

	error=Phone->GetSMSFolders(&s, &folders);
	Print_Error(error);

	for (i=start;i<=stop;i++) {
		sms.SMS[0].Folder	= atoi(argv[2]);
		sms.SMS[0].Location	= i;
		error=Phone->GetSMSMessage(&s, &sms);
		switch (error) {
		case GE_EMPTY:
			printmsg("Location %i\n",sms.SMS[0].Location);
			printmsg("Empty\n");
			break;
		default:
			Print_Error(error);
			printmsg("Location %i, folder \"%s\"\n",sms.SMS[0].Location,DecodeUnicodeString(folders.Folder[sms.SMS[0].Folder-1].Name));
			displaymultismsinfo(sms);
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

	while (error == GE_NONE) {
		sms.SMS[0].Folder=0x00;
		error=Phone->GetNextSMSMessage(&s, &sms, start);
		switch (error) {
		case GE_EMPTY:
			break;
		default:
			Print_Error(error);
			printmsg("Location %i, folder \"%s\"\n",sms.SMS[0].Location,DecodeUnicodeString(folders.Folder[sms.SMS[0].Folder-1].Name));
			displaymultismsinfo(sms);
		}
		start=false;
	}

#ifdef GSM_ENABLE_BEEP
	GSM_PhoneBeep();
#endif
	GSM_Terminate();
}

static void GetEachSMS(int argc, char *argv[])
{
	GSM_MultiSMSMessage	*GetSMS[150],*SortedSMS[150],sms;
	int			GetSMSNumber = 0,i,j;
	GSM_SMSFolders		folders;
	bool			start = true;

	GetSMS[0] = NULL;

	GSM_Init(true);

	error=Phone->GetSMSFolders(&s, &folders);
	Print_Error(error);

	while (error == GE_NONE) {
		sms.SMS[0].Folder=0x00;
		error=Phone->GetNextSMSMessage(&s, &sms, start);
		switch (error) {
		case GE_EMPTY:
			break;
		default:
			Print_Error(error);
			GetSMS[GetSMSNumber] = malloc(sizeof(GSM_MultiSMSMessage));
		        if (GetSMS[GetSMSNumber] == NULL) Print_Error(GE_MOREMEMORY);
			GetSMS[GetSMSNumber+1] = NULL;
			memcpy(GetSMS[GetSMSNumber],&sms,sizeof(GSM_MultiSMSMessage));
			GetSMSNumber++;
		}
		start=false;
	}

#ifdef GSM_ENABLE_BEEP
	GSM_PhoneBeep();
#endif

	GSM_Terminate();

	error = GSM_SortSMS(GetSMS, SortedSMS);
	Print_Error(error);

	i=0;
	while(SortedSMS[i] != NULL) {
		if (SortedSMS[i] != NULL) {
			for (j=0;j<SortedSMS[i]->Number;j++) {
			    printmsg("Location %i, folder \"%s\"\n",SortedSMS[i]->SMS[j].Location,DecodeUnicodeString(folders.Folder[SortedSMS[i]->SMS[j].Folder-1].Name));
			}
			displaymultismsinfo(*SortedSMS[i]);
		}
		i++;
	}
}

static void GetSMSFolders(int argc, char *argv[])
{
	GSM_SMSFolders folders;

	GSM_Init(true);

	error=Phone->GetSMSFolders(&s,&folders);
	Print_Error(error);

	for (i=0;i<folders.Number;i++) {
		printmsg("%i. %s\n",i+1,DecodeUnicodeString(folders.Folder[i].Name));
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
		case RING_MIDI		: printmsg("Midi format");	break;
	}
	printmsg(" format, ringtone \"%s\"\n",DecodeUnicodeString(ringtone.Name));

	if (argc==4) {
		error=GSM_SaveRingtoneFile(argv[3], &ringtone);
		Print_Error(error);
	}

	GSM_Terminate();
}

static void GetRingtonesList(int argc, char *argv[])
{
	GSM_AllRingtonesInfo 	Info;
	int			i;

	GSM_Init(true);

	error=Phone->GetRingtonesInfo(&s,&Info);
	Print_Error(error);

	GSM_Terminate();

	for (i=0;i<Info.Number;i++) printmsg("%i. \"%s\"\n",i,DecodeUnicodeString(Info.Ringtone[i].Name));
}

static void DialVoice(int argc, char *argv[])
{
	GSM_Init(true);

	error=Phone->DialVoice(&s, argv[2]);
	Print_Error(error);

	GSM_Terminate();
}

static void CancelCall(int argc, char *argv[])
{
	GSM_Init(true);

	error=Phone->CancelCall(&s);
	Print_Error(error);

	GSM_Terminate();
}

static void AnswerCall(int argc, char *argv[])
{
	GSM_Init(true);

	error=Phone->AnswerCall(&s);
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

static void GetCalendarNotes(int argc, char *argv[])
{
	GSM_CalendarEntry	Note;
	bool			refresh			= true;
	int			i_age			= 0,i;
	GSM_DateTime		Alarm,DateTime;

    	bool 			repeating 		= false;
    	char 			s1[200], s2[100];
    	int 			repeat_dayofweek 	= -1;
    	int 			repeat_day 		= -1;
    	int 			repeat_weekofmonth 	= -1;
    	int 			repeat_month 		= -1;
    	int 			repeat_frequency 	= -1;
    	GSM_DateTime 		repeat_startdate 	= {0,0,0,0,0,0,0};
    	GSM_DateTime 		repeat_stopdate 	= {0,0,0,0,0,0,0};

	GSM_Init(true);

	while (1) {
		error=Phone->GetNextCalendarNote(&s,&Note,refresh);
		if (error == GE_EMPTY) break;
		Print_Error(error);
		printmsg("Location     : %d\n", Note.Location);
		printmsg("Note type    : ");
		switch (Note.Type) {
			case GCN_REMINDER 	: printmsg("Reminder\n");		   break;
			case GCN_CALL     	: printmsg("Call\n");			   break;
			case GCN_MEETING  	: printmsg("Meeting\n");		   break;
			case GCN_BIRTHDAY 	: printmsg("Birthday\n");		   break;
			case GCN_ALARM    	: printmsg("Alarm\n");		   break;
			case GCN_DAILY_ALARM 	: printmsg("Daily alarm\n");		   break;
			case GCN_T_ATHL   	: printmsg("Training/Athletism\n"); 	   break;
        		case GCN_T_BALL   	: printmsg("Training/Ball Games\n"); 	   break;
	                case GCN_T_CYCL   	: printmsg("Training/Cycling\n"); 	   break;
	                case GCN_T_BUDO   	: printmsg("Training/Budo\n"); 	   break;
	                case GCN_T_DANC   	: printmsg("Training/Dance\n"); 	   break;
	                case GCN_T_EXTR   	: printmsg("Training/Extreme Sports\n"); break;
	                case GCN_T_FOOT   	: printmsg("Training/Football\n"); 	   break;
	                case GCN_T_GOLF   	: printmsg("Training/Golf\n"); 	   break;
	                case GCN_T_GYM    	: printmsg("Training/Gym\n"); 	   break;
	                case GCN_T_HORS   	: printmsg("Training/Horse Races\n");    break;
	                case GCN_T_HOCK   	: printmsg("Training/Hockey\n"); 	   break;
	                case GCN_T_RACE   	: printmsg("Training/Races\n"); 	   break;
	                case GCN_T_RUGB   	: printmsg("Training/Rugby\n"); 	   break;
	                case GCN_T_SAIL   	: printmsg("Training/Sailing\n"); 	   break;
	                case GCN_T_STRE   	: printmsg("Training/Street Games\n");   break;
	                case GCN_T_SWIM   	: printmsg("Training/Swimming\n"); 	   break;
	                case GCN_T_TENN   	: printmsg("Training/Tennis\n"); 	   break;
	                case GCN_T_TRAV   	: printmsg("Training/Travels\n");        break;
	                case GCN_T_WINT   	: printmsg("Training/Winter Games\n");   break;
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
        
		for (i=0;i<Note.EntriesNum;i++) {
			switch (Note.Entries[i].EntryType) {
			case CAL_START_DATETIME:
				printmsg("Start        : %s\n",OSDateTime(Note.Entries[i].Date,false));
				memcpy(&DateTime,&Note.Entries[i].Date,sizeof(GSM_DateTime));
				break;
			case CAL_STOP_DATETIME:
				printmsg("Stop         : %s\n",OSDateTime(Note.Entries[i].Date,false));
				memcpy(&DateTime,&Note.Entries[i].Date,sizeof(GSM_DateTime));
				break;
			case CAL_ALARM_DATETIME:
				printmsg("Tone alarm   : %s\n",OSDateTime(Note.Entries[i].Date,false));
				memcpy(&Alarm,&Note.Entries[i].Date,sizeof(GSM_DateTime));
				break;
			case CAL_SILENT_ALARM_DATETIME:
				printmsg("Silent alarm : %s\n",OSDateTime(Note.Entries[i].Date,false));
				memcpy(&Alarm,&Note.Entries[i].Date,sizeof(GSM_DateTime));
				break;
			case CAL_RECURRANCE:
				printmsg("Repeat       : %d day%s\n",Note.Entries[i].Number/24,
					((Note.Entries[i].Number/24)>1) ? "s":"" );
				break;
			case CAL_TEXT:
				printmsg("Text         : \"%s\"\n",DecodeUnicodeString(Note.Entries[i].Text));
				break;
			case CAL_PHONE:
				printmsg("Phone        : \"%s\"\n",DecodeUnicodeString(Note.Entries[i].Text));
				break;               
            		case CAL_PRIVATE:
                		printmsg("Private      : %s\n",Note.Entries[i].Number == 1 ? "Yes" : "No");
                		break;
            		case CAL_CONTACTID:
                		printmsg("Contact ID   : %d\n",Note.Entries[i].Number);
                		break;                
            		case CAL_REPEAT_DAYOFWEEK:
                		repeat_dayofweek 	= Note.Entries[i].Number;
                		repeating 		= true;
                		break;
            		case CAL_REPEAT_DAY:
                		repeat_day 		= Note.Entries[i].Number;
                		repeating 		= true;
                		break;
            		case CAL_REPEAT_WEEKOFMONTH:
                		repeat_weekofmonth 	= Note.Entries[i].Number;
                		repeating 		= true;
                		break;
            		case CAL_REPEAT_MONTH:
                		repeat_month 		= Note.Entries[i].Number;
                		repeating 		= true;
                		break;
            		case CAL_REPEAT_FREQUENCY:
                		repeat_frequency 	= Note.Entries[i].Number;
                		repeating 		= true;
                		break;
           		 case CAL_REPEAT_STARTDATE:
                		repeat_startdate 	= Note.Entries[i].Date;
                		repeating 		= true;
                		break;
            		case CAL_REPEAT_STOPDATE:
                		repeat_stopdate 	= Note.Entries[i].Date;
                		repeating 		= true;
                		break;
			}
		}
        	s1[0] = 0;
        	s2[0] = 0;
        	if (repeating) {
           		if ((repeat_startdate.Day == 0) && (repeat_stopdate.Day == 0)) {
                		strcpy(s1, "Forever");
            		} else if (repeat_startdate.Day == 0) {
                		sprintf(s1, "Till %s", OSDate(repeat_stopdate));
            		} else if (repeat_stopdate.Day == 0) {
                		sprintf(s1, "Since %s", OSDate(repeat_startdate));
           		} else {
                		sprintf(s2, "Since %s", OSDate(repeat_startdate));
                		sprintf(s1, "%s till %s", s2, OSDate(repeat_stopdate));
            		}
            		if (repeat_frequency != -1) {
                		if (repeat_frequency == 1) {
                    			strcat (s1, " on each ");
                		} else {
                    			sprintf(s2, " on each %d. ", repeat_frequency);
                    			strcat (s1, s2);
	                	}
	                	if (repeat_dayofweek > 0) {
	                    		strcat (s1, 
	                            		repeat_dayofweek == 1 ? "Monday" :
	                            		repeat_dayofweek == 2 ? "Tuesday" :
	                            		repeat_dayofweek == 3 ? "Wednesday" :
	                            		repeat_dayofweek == 4 ? "Thursday" :
	                            		repeat_dayofweek == 5 ? "Friday" :
	                            		repeat_dayofweek == 6 ? "Saturday" :
	                            		repeat_dayofweek == 7 ? "Sunday" :
	                            		"Unknown day!");
	                    	if (repeat_weekofmonth > 0) {
	                        	sprintf(s1, " in %d. week of ", repeat_weekofmonth);
	                        	strcat (s1, s2);
	                   	}
	                    	if (repeat_month > 0) {
	                        	strcat (s1, 
	                                	repeat_month ==  1 ? "January" :
	                                	repeat_month ==  2 ? "February" :
	                                	repeat_month ==  3 ? "March" :
	                                	repeat_month ==  4 ? "April" :
	                                	repeat_month ==  5 ? "May" :
	                                	repeat_month ==  6 ? "June" :
	                                	repeat_month ==  7 ? "July" :
                                		repeat_month ==  8 ? "August" :
                                		repeat_month ==  9 ? "September" :
                                		repeat_month == 10 ? "October" :
                                		repeat_month == 11 ? "November" :
                                		repeat_month == 12 ? "December" :
                                		"Unknown month!");
                    		} else {
                        		strcat (s1, "each month");
                    		}
                	} else if (repeat_day > 0) {
                   		sprintf(s2, " %d. day of ", repeat_day);
                    		strcat(s1, s2); 
                    		if (repeat_month > 0) {
                       			strcat (s1, 
                                		repeat_month ==  1 ? "January" :
                                		repeat_month ==  2 ? "February" :
                                		repeat_month ==  3 ? "March" :
                                		repeat_month ==  4 ? "April" :
                                		repeat_month ==  5 ? "May" :
                                		repeat_month ==  6 ? "June" :
                                		repeat_month ==  7 ? "July" :
                                		repeat_month ==  8 ? "August" :
                                		repeat_month ==  9 ? "September" :
                                		repeat_month == 10 ? "October" :
                                		repeat_month == 11 ? "November" :
                                		repeat_month == 12 ? "December" :
                                		"Unknown month!");
                    			} else {
                       			 	strcat (s1, "each month");
                   			}
                		} else {
                    			strcat (s1, "day");
                		}
            		}
            		printmsg("Repeating    : %s\n", s1);
        	}
		if (Note.Type == GCN_BIRTHDAY) {
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
		printmsg("\n");
		refresh=false;
	}

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
		printmsg("Name    : \"%s\"\n",DecodeUnicodeString(bookmark.Title));
		printmsg("Address : \"%s\"\n",DecodeUnicodeString(bookmark.Address));
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

static void GetBitmap(int argc, char *argv[])
{
	GSM_MultiBitmap 	MultiBitmap;
	int			location;
	GSM_AllRingtonesInfo 	Info;

	if (mystrncasecmp(argv[2],"STARTUP",0)) {
		MultiBitmap.Bitmap[0].Type=GSM_StartupLogo;
	} else if (mystrncasecmp(argv[2],"CALLER",0)) {
		MultiBitmap.Bitmap[0].Type=GSM_CallerLogo;
		GetStartStop(&location, NULL, 3, argc, argv);
		if (location>5) {
			printmsg("Maximal location for caller logo can be 5\n");
			exit (-1);
		}
	} else if (mystrncasecmp(argv[2],"PICTURE",0)) {
		MultiBitmap.Bitmap[0].Type=GSM_PictureImage;
		GetStartStop(&location, NULL, 3, argc, argv);
	} else if (mystrncasecmp(argv[2],"TEXT",0)) {
		MultiBitmap.Bitmap[0].Type=GSM_WelcomeNoteText;
	} else if (mystrncasecmp(argv[2],"DEALER",0)) {
		MultiBitmap.Bitmap[0].Type=GSM_DealerNoteText;
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

	error=GE_NONE;
	switch (MultiBitmap.Bitmap[0].Type) {
	case GSM_CallerLogo:
		GSM_PrintBitmap(stdout,&MultiBitmap.Bitmap[0]);
		printmsg("Group name  : \"%s\"",DecodeUnicodeString(MultiBitmap.Bitmap[0].Text));
		if (MultiBitmap.Bitmap[0].DefaultName) printmsg(" (default)");
		printmsg("\n");
		if (MultiBitmap.Bitmap[0].Ringtone == 0xff) {
			printmsg("Ringtone    : default\n");
		} else {
			error=Phone->GetRingtonesInfo(&s,&Info);
			if (error != GE_NONE) Info.Number = 0;

			printmsg("Ringtone    : ");
			if (strlen(DecodeUnicodeString(GSM_GetRingtoneName(&Info,MultiBitmap.Bitmap[0].Ringtone)))!=0) {
				printmsg("\"%s\"\n",DecodeUnicodeString(GSM_GetRingtoneName(&Info,MultiBitmap.Bitmap[0].Ringtone)));
			} else {
				printmsg("%i\n",MultiBitmap.Bitmap[0].Ringtone);
			}
		}
		if (MultiBitmap.Bitmap[0].Enabled) {
			printmsg("Bitmap      : enabled\n");
		} else {
			printmsg("Bitmap      : disabled\n");
		}
		if (argc>4) error=GSM_SaveBitmapFile(argv[4],&MultiBitmap);
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
		printmsg("Text   : \"%s\"\n",DecodeUnicodeString(MultiBitmap.Bitmap[0].Text));
		printmsg("Sender : \"%s\"\n",DecodeUnicodeString(MultiBitmap.Bitmap[0].Sender));
		if (argc>4) error=GSM_SaveBitmapFile(argv[4],&MultiBitmap);
		break;
	case GSM_WelcomeNoteText:
		printmsg("Welcome note text is \"%s\"\n",DecodeUnicodeString(MultiBitmap.Bitmap[0].Text));
		break;
	case GSM_DealerNoteText:
		printmsg("Dealer note text is \"%s\"\n",DecodeUnicodeString(MultiBitmap.Bitmap[0].Text));
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
		Bitmap.Type=GSM_WelcomeNoteText;
		EncodeUnicode(Bitmap.Text,argv[3],strlen(argv[3]));
	} else if (mystrncasecmp(argv[2],"DEALER",0)) {
		if (argc<4) {
			printmsg("More arguments required\n");
			exit(-1);
		}
		Bitmap.Type=GSM_DealerNoteText;
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
		MultiBitmap.Bitmap[0].Type 	= GSM_CallerLogo;
		MultiBitmap.Bitmap[0].Location	= i;
		if (argc>4) {
			error=GSM_ReadBitmapFile(argv[4],&MultiBitmap);
			Print_Error(error);
		}
		memcpy(&Bitmap,&MultiBitmap.Bitmap[0],sizeof(GSM_Bitmap));
		if (i!=255) {
			GSM_Init(true);
			init = false;
			NewBitmap.Type 	   = GSM_CallerLogo;
			NewBitmap.Location = i;
			error=Phone->GetBitmap(&s,&NewBitmap);
			Print_Error(error);
			Bitmap.Ringtone = NewBitmap.Ringtone;
			CopyUnicodeString(Bitmap.Text, NewBitmap.Text);
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
	GSM_Ringtone ringtone;

	ringtone.Format	= 0;
	error=GSM_ReadRingtoneFile(argv[2],&ringtone);
	Print_Error(error);
	ringtone.Location = 255;
	if (argc>3) ringtone.Location=atoi(argv[3]);
	if (argc>4) {
		if (mystrncasecmp(argv[4],"-scale",0)) {
			ringtone.NoteTone.AllNotesScale = true;
		} else {
			printmsg("Unknown parameter \"%s\"",argv[4]);
			exit(-1);
		}
	}
	if (ringtone.Location==0) {
		printmsg("ERROR: enumerate locations from 1");
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

#define SEND_SAVE_SMS_BUFFER_SIZE 10000

static GSM_Error SMSStatus;

static void SendSMSStatus (char *Device, int status)
{
	dprintf("Incoming SMS device: \"%s\"\n",Device);
	if (status==0) {
		printmsg("..OK\n");
		SMSStatus = GE_NONE;
	} else {
		printmsg("..error %i\n",status);
		SMSStatus = GE_UNKNOWN;
	}
}

static void SendSaveSMS(int argc, char *argv[])
{
#ifdef GSM_ENABLE_BACKUP
	GSM_Backup			Backup;
#endif
	int				i,j,z,FramesNum=0;
	GSM_SMSFolders			folders;
	GSM_MultiSMSMessage		sms;
	GSM_Ringtone			ringtone[MAX_MULTI_SMS];
	GSM_MultiBitmap			bitmap[MAX_MULTI_SMS],bitmap2;
	GSM_EncodeMultiPartSMSInfo	SMSInfo;
	GSM_NetworkInfo			NetInfo;
	FILE 				*ReplaceFile;
	char				ReplaceBuffer2	[200],ReplaceBuffer[200];
	char				InputBuffer	[SEND_SAVE_SMS_BUFFER_SIZE/2+1];
	char				Buffer		[SEND_SAVE_SMS_BUFFER_SIZE][MAX_MULTI_SMS];
	char				Sender		[(GSM_MAX_NUMBER_LENGTH+1)*2];
	char				Name		[(GSM_MAX_NUMBER_LENGTH+1)*2];
	char				SMSC		[(GSM_MAX_NUMBER_LENGTH+1)*2];
	int				startarg		= 0;
	int				chars_read		= 0;
	bool 				nextlong		= 0;
	bool				ReplyViaSameSMSC 	= false;
	int				SMSCSet			= 1;

	/* Parameters required only during saving */
	int				Folder			= 1; /*Inbox by default */
	GSM_SMS_State			State			= GSM_Sent;

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
		Validity.VPF 	= 0;
	}

	if (mystrncasecmp(argv[2],"TEXT",0)) {
		chars_read = fread(InputBuffer, 1, SEND_SAVE_SMS_BUFFER_SIZE/2, stdin);
		if (chars_read == 0) {
			printmsg("Couldn't read from stdin!\n");	
			exit (-1);
		}                               	
		InputBuffer[chars_read] 		= 0x00;	
		InputBuffer[chars_read+1] 		= 0x00;	
		EncodeUnicode(Buffer[0],InputBuffer,strlen(InputBuffer));
		SMSInfo.Entries[0].Buffer  		= Buffer[0];
		SMSInfo.Entries[0].ID			= SMS_Text;
		SMSInfo.UnicodeCoding   		= false;
		SMSInfo.Class				= -1;
		startarg += 3;
	} else if (mystrncasecmp(argv[2],"EMS",0)) {
		SMSInfo.EntriesNum = 0;
		startarg += 3;
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
		bitmap[0].Bitmap[0].Type=GSM_CallerLogo;
		error=GSM_ReadBitmapFile(argv[3+startarg],&bitmap[0]);
		Print_Error(error);
		SMSInfo.Entries[0].ID 	    = SMS_NokiaCallerLogo;
		SMSInfo.Entries[0].Bitmap   = &bitmap[0];
		if (mystrncasecmp(argv[1],"--savesms",0)) {
			EncodeUnicode(Sender, "Caller",6);
		}
		startarg += 4;
	} else if (mystrncasecmp(argv[2],"PICTURE",0)) {
		if (argc<4+startarg) {
			printmsg("Where is logo filename ?\n");
			exit(-1);
		}
		bitmap[0].Bitmap[0].Type=GSM_PictureImage;
		error=GSM_ReadBitmapFile(argv[3+startarg],&bitmap[0]);
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
		Print_Error(error);
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
	} else if (mystrncasecmp(argv[2],"SETTINGS",0)) {
		if (argc<5+startarg) {
			printmsg("Where is backup filename and location ?\n");
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[3+startarg],&Backup);
		Print_Error(error);
		i = 0;
		while (Backup.WAPSettings[i]!=NULL) {
			if (i == atoi(argv[4+startarg])-1) break;
			i++;
		}
		if (i != atoi(argv[4+startarg])-1) {
			printmsg("WAP settings not found in file\n");
			exit(-1);
		}
		if (Backup.WAPSettings[i]->Settings[0].Bearer != WAPSETTINGS_BEARER_DATA &&
		    Backup.WAPSettings[i]->Settings[0].Bearer != WAPSETTINGS_BEARER_SMS) {
			printmsg("Sorry. This bearer in WAP settings not supported now\n");
			exit(-1);
		}
		SMSInfo.Entries[0].ID 	 = SMS_NokiaWAPSettingsLong;
		SMSInfo.Entries[0].Settings = &Backup.WAPSettings[i]->Settings[0];		
		if (mystrncasecmp(argv[1],"--savesms",0)) {
			EncodeUnicode(Sender, "Settings",8);
			EncodeUnicode(Name,"WAP Settings",12);
		}
		startarg += 5;
	} else if (mystrncasecmp(argv[2],"CALENDAR",0)) {
		if (argc<5+startarg) {
			printmsg("Where is backup filename and location ?\n");
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[3+startarg],&Backup);
		Print_Error(error);
		i = 0;
		while (Backup.Calendar[i]!=NULL) {
			if (i == atoi(argv[4+startarg])-1) break;
			i++;
		}
		if (i != atoi(argv[4+startarg])-1) {
			printmsg("Calendar note not found in file\n");
			exit(-1);
		}
		SMSInfo.Entries[0].ID 	 = SMS_NokiaVCALENDAR10Long;
		SMSInfo.Entries[0].Calendar = Backup.Calendar[i];		
		if (mystrncasecmp(argv[1],"--savesms",0)) {
			EncodeUnicode(Sender, "Calendar",8);
		}
		startarg += 5;
	} else if (mystrncasecmp(argv[2],"VCARD10",0) || mystrncasecmp(argv[2],"VCARD21",0)) {
		if (argc<6+startarg) {
			printmsg("Where is backup filename and location and memory type ?\n");
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[3+startarg],&Backup);
		Print_Error(error);
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
			SMSInfo.Entries[0].ID = SMS_NokiaVCARD10Long;
		} else {
			SMSInfo.Entries[0].ID = SMS_NokiaVCARD21Long;
		}
		if (mystrncasecmp(argv[1],"--savesms",0)) {
			EncodeUnicode(Sender, "VCARD",5);
			EncodeUnicode(Name, "Phonebook entry",15);
		}
		startarg += 6;
#endif
	} else if (mystrncasecmp(argv[2],"PROFILE",0)) {
		if (argc<6+startarg) {
			printmsg("Where is profile name, ringtone & bitmap file ?\n");
			exit(-1);
		}
		/* profile name */
		EncodeUnicode(Buffer[0],argv[3+startarg],strlen(argv[3+startarg]));
		/* bitmap */
		bitmap[0].Bitmap[0].Type=GSM_PictureImage;
		error=GSM_ReadBitmapFile(argv[4+startarg],&bitmap[0]);
		Print_Error(error);
		bitmap[0].Bitmap[0].Text[0]=0;
		bitmap[0].Bitmap[0].Text[1]=0;
		/* ringtone */
		ringtone[0].Format=RING_NOTETONE;
		error=GSM_ReadRingtoneFile(argv[5+startarg],&ringtone[0]);
		Print_Error(error);
		SMSInfo.Entries[0].ID 	 	= SMS_NokiaProfileLong;
		SMSInfo.Entries[0].Buffer	= Buffer[0];
		SMSInfo.Entries[0].Bitmap   	= &bitmap[0];
		SMSInfo.Entries[0].Ringtone	= &ringtone[0];
		if (mystrncasecmp(argv[1],"--savesms",0)) {
			EncodeUnicode(Sender, "Profile",7);
		}
		startarg += 6;
	} else {
		printmsg("What format of sms (\"%s\") ?\n",argv[2]);
		exit(-1);
	}

	for (i=startarg;i<argc;i++) {
		switch (nextlong) {
		case 0:
			if (mystrncasecmp(argv[1],"--savesms",0)) {
				if (mystrncasecmp(argv[i],"-folder",0)) {
					nextlong=1;
					continue;
				}
				if (mystrncasecmp(argv[i],"-unread",0)) {
					State = GSM_UnRead;
					continue;
				}
				if (mystrncasecmp(argv[i],"-read",0)) {
					State = GSM_Read;
					continue;
				}
				if (mystrncasecmp(argv[i],"-unsent",0)) {
					State = GSM_UnSent;
					continue;
				}
				if (mystrncasecmp(argv[i],"-sender",0)) {
					nextlong=2;
					continue;
				}
			} else {
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
			if (mystrncasecmp(argv[i],"-reply",0)) {
				ReplyViaSameSMSC=true;
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
				break;
			}
			if (mystrncasecmp(argv[2],"EMS",0)) {
				if (mystrncasecmp(argv[i],"-text",0)) {
					nextlong = 11;
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
				if (mystrncasecmp(argv[i],"-sound10",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSound10;
					nextlong = 14;
					break;
				}
				if (mystrncasecmp(argv[i],"-sound12",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSSound12;
					nextlong = 14;
					break;
				}
				if (mystrncasecmp(argv[i],"-fixedbitmap",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID = SMS_EMSFixedBitmap;
					nextlong = 15;
					break;
				}
				if (mystrncasecmp(argv[i],"-animation",0)) {
					SMSInfo.Entries[SMSInfo.EntriesNum].ID  = SMS_EMSAnimation;
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
				Buffer[atoi(argv[i])*2][0]	= 0x00;
				Buffer[atoi(argv[i])*2+1][0]	= 0x00;
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
			if (!strcmp(DecodeUnicodeString(GSM_GetNetworkName(bitmap[0].Bitmap[0].NetworkCode)),"unknown")) {
				printmsg("Unknown GSM network code (\"%s\")\n",argv[i]);
				exit(-1);
			}
			if (mystrncasecmp(argv[1],"--savesms",0)) {
				EncodeUnicode(Sender, "OpLogo",6);
				EncodeUnicode(Sender+6*2,bitmap[0].Bitmap[0].NetworkCode,3);
				EncodeUnicode(Sender+6*2+3*2,bitmap[0].Bitmap[0].NetworkCode+4,2);
				if (strlen(DecodeUnicodeString(GSM_GetNetworkName(bitmap[0].Bitmap[0].NetworkCode)))<GSM_MAX_SMS_NAME_LENGTH-7) {
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
			if (!ReplaceFile) Print_Error(GE_CANTOPENFILE);
			memset(ReplaceBuffer,0,sizeof(ReplaceBuffer));
			fread(ReplaceBuffer,1,sizeof(ReplaceBuffer),ReplaceFile);
			fclose(ReplaceFile);
			ReadUnicodeFile(ReplaceBuffer2,ReplaceBuffer);
			for(j=0;j<(int)(strlen(DecodeUnicodeString(Buffer[0])));j++) {
				for (z=0;z<(int)(strlen(DecodeUnicodeString(ReplaceBuffer2))/2);z++) {
					if (ReplaceBuffer2[z*4]   == Buffer[j][0] &&
					    ReplaceBuffer2[z*4+1] == Buffer[j+1][0]) {
						Buffer[j][0]   = ReplaceBuffer2[z*4+2];
						Buffer[j+1][0] = ReplaceBuffer2[z*4+3];
						break;
					}
				}
			}
			nextlong = 0;
			break;
		case 10:
			Validity.VPF = GSM_RelativeFormat;
			if (mystrncasecmp(argv[i],"HOUR",0)) 		Validity.Relative = GSMV_1_Hour;
			else if (mystrncasecmp(argv[i],"6HOURS",0))	Validity.Relative = GSMV_6_Hours;
			else if (mystrncasecmp(argv[i],"DAY",0)) 	Validity.Relative = GSMV_24_Hours;
			else if (mystrncasecmp(argv[i],"3DAYS",0)) 	Validity.Relative = GSMV_72_Hours;
			else if (mystrncasecmp(argv[i],"WEEK",0)) 	Validity.Relative = GSMV_1_Week;
			else if (mystrncasecmp(argv[i],"MAX",0)) 	Validity.Relative = GSMV_Max_Time;
			else {
				printmsg("Unknown validity string (\"%s\")\n",argv[i]);
				exit(-1);
			}
			nextlong = 0;
			break;
		case 11:/* EMS text from parameter */
			EncodeUnicode(Buffer[SMSInfo.EntriesNum],argv[i],strlen(argv[i]));
			dprintf("buffer is \"%s\"\n",DecodeUnicodeString(Buffer[SMSInfo.EntriesNum]));
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

		}
	}
	if (nextlong!=0) {
		printmsg("Parameter missed...\n");
		exit(-1);
	}

	GSM_Init(true);

	if (mystrncasecmp(argv[2],"OPERATOR",0)) {
		if (bitmap[0].Bitmap[0].Type==GSM_OperatorLogo && strcmp(bitmap[0].Bitmap[0].NetworkCode,"000 00")==0)
		{
			error=Phone->GetNetworkInfo(&s,&NetInfo);
			Print_Error(error);
			strcpy(bitmap[0].Bitmap[0].NetworkCode,NetInfo.NetworkCode);
			if (mystrncasecmp(argv[1],"--savesms",0)) {
				EncodeUnicode(Sender, "OpLogo",6);
				EncodeUnicode(Sender+6*2,bitmap[0].Bitmap[0].NetworkCode,3);
				EncodeUnicode(Sender+6*2+3*2,bitmap[0].Bitmap[0].NetworkCode+4,2);
				if (strlen(DecodeUnicodeString(GSM_GetNetworkName(bitmap[0].Bitmap[0].NetworkCode)))<GSM_MAX_SMS_NAME_LENGTH-7) {
					EncodeUnicode(Name,"OpLogo ",7);
					CopyUnicodeString(Name+7*2,GSM_GetNetworkName(bitmap[0].Bitmap[0].NetworkCode));
				} else {
					CopyUnicodeString(Name,Sender);
				}
			}
		}
	}
	if (mystrncasecmp(argv[1],"--savesms",0)) {
		error=Phone->GetSMSFolders(&s, &folders);
		Print_Error(error);
	} else {
		if (Validity.VPF != 0 && SMSCSet != 0) {
			PhoneSMSC.Location = SMSCSet;
			error=Phone->GetSMSC(&s,&PhoneSMSC);
			Print_Error(error);
			CopyUnicodeString(SMSC,PhoneSMSC.Number);
			SMSCSet = 0;
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
				if (SMSInfo.Entries[i].RingtoneNotes!=ringtone[i].NoteTone.NrCommands) {
					printmsg("Warning: ringtone too long. %i percent part cut\n",
						(ringtone[i].NoteTone.NrCommands-SMSInfo.Entries[i].RingtoneNotes)*100/ringtone[i].NoteTone.NrCommands);
				}
			default:
				break;
		}
	}

	if (mystrncasecmp(argv[1],"--savesms",0)) {
		for (i=0;i<sms.Number;i++) {
			printmsg("Saving SMS %i/%i\n",i+1,sms.Number);
			sms.SMS[i].Location		= 0;
			sms.SMS[i].Folder		= Folder;
			sms.SMS[i].State		= State;
			sms.SMS[i].ReplyViaSameSMSC	= ReplyViaSameSMSC;
			sms.SMS[i].SMSC.Location	= SMSCSet;
			sms.SMS[i].PDU			= SMS_Deliver;
			CopyUnicodeString(sms.SMS[i].Number, Sender);
			CopyUnicodeString(sms.SMS[i].Name, Name);
			if (SMSCSet==0) CopyUnicodeString(sms.SMS[i].SMSC.Number, SMSC);
			error=Phone->SaveSMSMessage(&s, &sms.SMS[i]);
			Print_Error(error);
			printmsg("Saved in folder \"%s\", location %i\n",
				DecodeUnicodeString(folders.Folder[sms.SMS[i].Folder-1].Name),sms.SMS[i].Location);
		}
	} else {
		/* We do not want to make it forever - press Ctrl+C to stop */
		signal(SIGINT, interrupted);
		printmsg("If you want break, press Ctrl+C...\n");

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
			if (Validity.VPF != 0) memcpy(&sms.SMS[i].SMSC.Validity,&Validity,sizeof(GSM_SMSValidity));
			SMSStatus = GE_TIMEOUT;
			error=Phone->SendSMSMessage(&s, &sms.SMS[i]);
			Print_Error(error);
			printmsg("....waiting for network answer");
			while (!bshutdown) {
				GSM_ReadDevice(&s);
				if (SMSStatus == GE_UNKNOWN) {
					GSM_Terminate();
					exit(-1);
				}
				if (SMSStatus == GE_NONE) break;
			}
		}
	}
	
	GSM_Terminate();
}

#ifdef GSM_ENABLE_BACKUP
static void Backup(int argc, char *argv[])
{
	int			i, used;
	GSM_MemoryStatus	MemStatus;
	GSM_ToDoEntry		ToDo;
	GSM_PhonebookEntry	Pbk;
	GSM_CalendarEntry	Note;
	GSM_Bitmap		Bitmap;
	GSM_WAPBookmark		Bookmark;
	GSM_Profile		Profile;
	GSM_MultiWAPSettings	Settings;
	GSM_Ringtone		Ringtone;
	GSM_SMSC		SMSC;
	char			buffer[50];
	GSM_Backup		Backup;
	GSM_Backup_Info		Info;

	GSM_ClearBackup(&Backup);
	GSM_GetBackupFeatures(argv[2],&Info);

	/* We do not want to make it forever - press Ctrl+C to stop */
	signal(SIGINT, interrupted);
	printmsgerr("If you want break, press Ctrl+C...\n");

	GSM_Init(true);

	if (Info.DateTime) {
		GSM_GetCurrentDateTime (&Backup.DateTime);
		Backup.DateTimeAvailable=true;
	}
	if (Info.Model) {
		error=Phone->GetManufacturer(&s, buffer);
		Print_Error(error);
		sprintf(Backup.Model,"%s ",DecodeUnicodeString(buffer));	
		if (GetModelData(NULL,s.Model,NULL)->model[0]!=0) {
		strcat(Backup.Model,GetModelData(NULL,s.Model,NULL)->model);
		} else {
			strcat(Backup.Model,s.Model);
		}
		strcat(Backup.Model," ");
		strcat(Backup.Model,s.Ver);
	}
	if (Info.IMEI) {
		error=Phone->GetIMEI(&s, Backup.IMEI);
		Print_Error(error);
	}
	if (Info.PhonePhonebook) {
		MemStatus.MemoryType = GMT_ME;
		error=Phone->GetMemoryStatus(&s, &MemStatus);
		if (error==GE_NONE && MemStatus.Used != 0) {
			if (answer_yes("Backup phone phonebook")) {
				Pbk.MemoryType  = GMT_ME;
				i		= 1;
				used 		= 0;
				while (used != MemStatus.Used) {
					Pbk.Location = i;
					error=Phone->GetMemory(&s, &Pbk);
					if (error != GE_EMPTY) {
						Print_Error(error);
						if (used < GSM_BACKUP_MAX_PHONEPHONEBOOK) {
							Backup.PhonePhonebook[used] = malloc(sizeof(GSM_PhonebookEntry));
						        if (Backup.PhonePhonebook[used] == NULL) Print_Error(GE_MOREMEMORY);
							Backup.PhonePhonebook[used+1] = NULL;
						} else {
							printmsg("Increase GSM_BACKUP_MAX_PHONEPHONEBOOK\n");
							exit(-1);
						}
						*Backup.PhonePhonebook[used]=Pbk;
						used++;
					}
					printmsgerr("%cReading: %i percent",13,used*100/MemStatus.Used);
					i++;
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				printmsgerr("\n");
			}
		}
	}
	if (Info.SIMPhonebook) {
		MemStatus.MemoryType 	= GMT_SM;
		error=Phone->GetMemoryStatus(&s, &MemStatus);
		if (error==GE_NONE && MemStatus.Used != 0) {
			if (answer_yes("Backup SIM phonebook")) {
				Pbk.MemoryType 	= GMT_SM;
				i		= 1;
				used 		= 0;
				while (used != MemStatus.Used) {
					Pbk.Location = i;
					error=Phone->GetMemory(&s, &Pbk);
					if (error != GE_EMPTY) {
						Print_Error(error);
						if (used < GSM_BACKUP_MAX_SIMPHONEBOOK) {
							Backup.SIMPhonebook[used] = malloc(sizeof(GSM_PhonebookEntry));
						        if (Backup.SIMPhonebook[used] == NULL) Print_Error(GE_MOREMEMORY);
							Backup.SIMPhonebook[used + 1] = NULL;
						} else {
							printmsg("Increase GSM_BACKUP_MAX_SIMPHONEBOOK\n");
							exit(-1);
						}
						*Backup.SIMPhonebook[used]=Pbk;
						used++;
					}
					printmsgerr("%cReading: %i percent",13,used*100/MemStatus.Used);
					i++;
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				printmsgerr("\n");
			}
		}
	}
	if (Info.Calendar) {
		error=Phone->GetNextCalendarNote(&s,&Note,true);
		if (error==GE_NONE) {
			if (answer_yes("Backup calendar notes")) {
				used = 0;
				printmsgerr("Reading : ");
				while (error == GE_NONE) {
					if (used < GSM_BACKUP_MAX_CALENDAR) {
						Backup.Calendar[used] = malloc(sizeof(GSM_CalendarEntry));
					        if (Backup.Calendar[used] == NULL) Print_Error(GE_MOREMEMORY);
						Backup.Calendar[used+1] = NULL;
					} else {
						printmsg("Increase GSM_BACKUP_MAX_CALENDAR\n");
						exit(-1);
					}
					*Backup.Calendar[used]=Note;
					used ++;
					error=Phone->GetNextCalendarNote(&s,&Note,false);
					printmsgerr("*");
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				printmsgerr("\n");
			}
		}
	}
	if (Info.ToDo) {
		ToDo.Location = 1;
		error=Phone->GetToDo(&s,&ToDo,true);
		if (error==GE_NONE) {
			if (answer_yes("Backup ToDo")) {
				used = 0;
				printmsgerr("Reading : ");
				while (error == GE_NONE) {
					if (used < GSM_BACKUP_MAX_TODO) {
						Backup.ToDo[used] = malloc(sizeof(GSM_ToDoEntry));
					        if (Backup.ToDo[used] == NULL) Print_Error(GE_MOREMEMORY);
						Backup.ToDo[used+1] = NULL;
					} else {
						printmsg("Increase GSM_BACKUP_MAX_TODO\n");
						exit(-1);
					}
					*Backup.ToDo[used]=ToDo;
					used ++;
					ToDo.Location = used+1;
					error=Phone->GetToDo(&s,&ToDo,false);
					printmsgerr("*");
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				printmsgerr("\n");
			}
		}
	}
	if (Info.CallerLogos) {
		Bitmap.Type 	= GSM_CallerLogo;
		Bitmap.Location = 1;
		error=Phone->GetBitmap(&s,&Bitmap);
		if (error == GE_NONE) {
			if (answer_yes("Backup caller groups and logos")) {
				printmsgerr("Reading : ");
				error = GE_NONE;
				used  = 0;
				while (error == GE_NONE) {
					if (used < GSM_BACKUP_MAX_CALLER) {
						Backup.CallerLogos[used] = malloc(sizeof(GSM_Bitmap));
					        if (Backup.CallerLogos[used] == NULL) Print_Error(GE_MOREMEMORY);
						Backup.CallerLogos[used+1] = NULL;
					} else {
						printmsg("Increase GSM_BACKUP_MAX_CALLER\n");
						exit(-1);
					}
					*Backup.CallerLogos[used] = Bitmap;
					used ++;
					Bitmap.Location = used + 1;
					error=Phone->GetBitmap(&s,&Bitmap);
					printmsgerr("*");
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				printmsgerr("\n");
			}
		}
	}
	if (Info.SMSC) {
		if (answer_yes("Backup SMS profiles")) {
			used = 0;
			printmsgerr("Reading: ");
			while (true)
			{
				SMSC.Location = used + 1;
				error = Phone->GetSMSC(&s,&SMSC);
				if (error != GE_NONE) break;
				if (used < GSM_BACKUP_MAX_SMSC) {
					Backup.SMSC[used] = malloc(sizeof(GSM_SMSC));
				        if (Backup.SMSC[used] == NULL) Print_Error(GE_MOREMEMORY);
					Backup.SMSC[used + 1] = NULL;
				} else {
					printmsg("Increase GSM_BACKUP_MAX_SMSC\n");
					exit(-1);
				}
				*Backup.SMSC[used]=SMSC;
				used++;
				printmsgerr("*");
			}
			printmsgerr("\n");
		}
	}
	if (Info.StartupLogo) {
		Bitmap.Type = GSM_WelcomeNoteText;
		error = Phone->GetBitmap(&s,&Bitmap);
		if (error == GE_NONE) {
			if (answer_yes("Backup startup logo/text")) {
				Backup.StartupLogo = malloc(sizeof(GSM_Bitmap));
			        if (Backup.StartupLogo == NULL) Print_Error(GE_MOREMEMORY);
				*Backup.StartupLogo = Bitmap;
				if (Bitmap.Text[0]==0 && Bitmap.Text[1]==0) {
					Bitmap.Type = GSM_StartupLogo;
					error = Phone->GetBitmap(&s,&Bitmap);
					if (error == GE_NONE) *Backup.StartupLogo = Bitmap;
				}
			}
		}
	}
	if (Info.OperatorLogo) {
		Bitmap.Type = GSM_OperatorLogo;
		error=Phone->GetBitmap(&s,&Bitmap);
		if (error == GE_NONE) {
			if (strcmp(Bitmap.NetworkCode,"000 00")!=0) {
				if (answer_yes("Backup operator logo")) {
					Backup.OperatorLogo = malloc(sizeof(GSM_Bitmap));
				        if (Backup.OperatorLogo == NULL) Print_Error(GE_MOREMEMORY);
					*Backup.OperatorLogo = Bitmap;
				}
			}
		}
	}
	if (Info.WAPBookmark) {
		Bookmark.Location = 1;
		error=Phone->GetWAPBookmark(&s,&Bookmark);
		if (error==GE_NONE) {
			if (answer_yes("Backup WAP bookmarks")) {
				used = 0;
				printmsgerr("Reading : ");
				while (error == GE_NONE) {
					if (used < GSM_BACKUP_MAX_WAPBOOKMARK) {
						Backup.WAPBookmark[used] = malloc(sizeof(GSM_WAPBookmark));
					        if (Backup.WAPBookmark[used] == NULL) Print_Error(GE_MOREMEMORY);
						Backup.WAPBookmark[used+1] = NULL;
					} else {
						printmsg("Increase GSM_BACKUP_MAX_WAPBOOKMARK\n");
						exit(-1);
					}
					*Backup.WAPBookmark[used]=Bookmark;
					used ++;
					Bookmark.Location = used+1;
					error=Phone->GetWAPBookmark(&s,&Bookmark);
					printmsgerr("*");
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				printmsgerr("\n");
			}
		}
	}
	if (Info.WAPSettings) {
		Settings.Location = 1;
		error=Phone->GetWAPSettings(&s,&Settings);
		if (error==GE_NONE) {
			if (answer_yes("Backup WAP settings")) {
				used = 0;
				printmsgerr("Reading : ");
				while (error == GE_NONE) {
					if (used < GSM_BACKUP_MAX_WAPSETTINGS) {
						Backup.WAPSettings[used] = malloc(sizeof(GSM_MultiWAPSettings));
					        if (Backup.WAPSettings[used] == NULL) Print_Error(GE_MOREMEMORY);
						Backup.WAPSettings[used+1] = NULL;
					} else {
						printmsg("Increase GSM_BACKUP_MAX_WAPSETTINGS\n");
						exit(-1);
					}
					*Backup.WAPSettings[used]=Settings;
					used ++;
					Settings.Location = used+1;
					error=Phone->GetWAPSettings(&s,&Settings);
					printmsgerr("*");
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				printmsgerr("\n");
			}
		}
	}
	if (Info.Ringtone) {
		Ringtone.Location 	= 1;
		Ringtone.Format		= 0;
		error=Phone->GetRingtone(&s,&Ringtone,false);
		if (error==GE_EMPTY || error == GE_NONE) {
			if (answer_yes("Backup user ringtones")) {
				used 	= 0;
				i	= 1;
				printmsgerr("Reading : ");
				while (error == GE_NONE || error == GE_EMPTY) {
					if (error == GE_NONE) {
						if (used < GSM_BACKUP_MAX_RINGTONES) {
							Backup.Ringtone[used] = malloc(sizeof(GSM_Ringtone));
						        if (Backup.Ringtone[used] == NULL) Print_Error(GE_MOREMEMORY);
							Backup.Ringtone[used+1] = NULL;
						} else {
							printmsg("Increase GSM_BACKUP_MAX_RINGTONES\n");
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
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				printmsgerr("\n");
			}

		}
	}
	if (Info.Profiles) {
		Profile.Location = 1;
		error = Phone->GetProfile(&s,&Profile);
	        if (error == GE_NONE) {
			if (answer_yes("Backup phone profiles")) {
				used = 0;
				printmsgerr("Reading: ");
				while (true)
				{
					Profile.Location = used + 1;
					error = Phone->GetProfile(&s,&Profile);
					if (error != GE_NONE) break;
					if (used < GSM_BACKUP_MAX_PROFILES) {
						Backup.Profiles[used] = malloc(sizeof(GSM_Profile));
							if (Backup.Profiles[used] == NULL) Print_Error(GE_MOREMEMORY);
						Backup.Profiles[used + 1] = NULL;
					} else {
						printmsg("Increase GSM_BACKUP_MAX_PROFILES\n");
						exit(-1);
					}
					*Backup.Profiles[used]=Profile;
					used++;
					printmsgerr("*");
				}
				printmsgerr("\n");
			}
		}
	}

	GSM_Terminate();

	GSM_SaveBackupFile(argv[2],&Backup);
    	GSM_FreeBackup(&Backup);
}

static void Restore(int argc, char *argv[])
{
	GSM_Backup		Backup;
	GSM_DateTime 		date_time;
	GSM_CalendarEntry	Calendar;
	GSM_Bitmap		Bitmap;
	GSM_Ringtone		Ringtone;
	GSM_PhonebookEntry	Pbk;
	GSM_MemoryStatus	MemStatus;
	GSM_ToDoEntry		ToDo;
	GSM_Profile		Profile;
	GSM_MultiWAPSettings	Settings;
	GSM_WAPBookmark		Bookmark;
	int			i, used, max;
	bool			Past = true;

	error=GSM_ReadBackupFile(argv[2],&Backup);
	Print_Error(error);

	/* We do not want to make it forever - press Ctrl+C to stop */
	signal(SIGINT, interrupted);
	printmsgerr("If you want break, press Ctrl+C...\n");

	if (Backup.DateTimeAvailable) 	printmsgerr("Time of backup : %s\n",OSDateTime(Backup.DateTime,false));
	if (Backup.Model[0]!=0) 	printmsgerr("Phone          : %s\n",Backup.Model);
	if (Backup.IMEI[0]!=0) 		printmsgerr("IMEI           : %s\n",Backup.IMEI);

	GSM_Init(true);

	if (Backup.PhonePhonebook[0] != NULL) {
		MemStatus.MemoryType = GMT_ME;
		error=Phone->GetMemoryStatus(&s, &MemStatus);
		if (error==GE_NONE) {
			max = 0;
			while (Backup.PhonePhonebook[max]!=NULL) max++;
			printmsgerr("%i entries in backup file\n",max);
			if (answer_yes("Restore phone phonebook")) {
				used = 0;
				for (i=0;i<MemStatus.Used+MemStatus.Free;i++)
				{
					Pbk.MemoryType 	= GMT_ME;
					Pbk.Location	= i + 1;
					Pbk.EntriesNum	= 0;
					if (used<max) {
						if (Backup.PhonePhonebook[used]->Location == Pbk.Location) {
							Pbk = *Backup.PhonePhonebook[used];
							used++;
						}
					}
					dprintf("Location %i\n",Pbk.Location);
					error=Phone->SetMemory(&s, &Pbk);
					Print_Error(error);
					printmsgerr("%cWriting: %i percent",13,(i+1)*100/(MemStatus.Used+MemStatus.Free));
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				printmsgerr("\n");
			}
		}
	}
	if (Backup.SIMPhonebook[0] != NULL) {
		MemStatus.MemoryType = GMT_SM;
		error=Phone->GetMemoryStatus(&s, &MemStatus);
		if (error==GE_NONE) {
			max = 0;
			while (Backup.SIMPhonebook[max]!=NULL) max++;
			printmsgerr("%i entries in backup file\n",max);
			if (answer_yes("Restore SIM phonebook")) {
				used = 0;
				for (i=0;i<MemStatus.Used+MemStatus.Free;i++)
				{
					Pbk.MemoryType 	= GMT_SM;
					Pbk.Location	= i + 1;
					Pbk.EntriesNum	= 0;
					if (used<max) {
						if (Backup.SIMPhonebook[used]->Location == Pbk.Location) {
							Pbk = *Backup.SIMPhonebook[used];
							used++;
						}
					}
					dprintf("Location %i\n",Pbk.Location);
					error=Phone->SetMemory(&s, &Pbk);
					Print_Error(error);
					printmsgerr("%cWriting: %i percent",13,(i+1)*100/(MemStatus.Used+MemStatus.Free));
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				printmsgerr("\n");
			}
		}
	}
	if (Backup.CallerLogos[0] != NULL) {
		Bitmap.Type 	= GSM_CallerLogo;
		Bitmap.Location = 1;
		error=Phone->GetBitmap(&s,&Bitmap);
		if (error == GE_NONE) {
			if (answer_yes("Restore caller groups and logos")) {
				max = 0;
				while (Backup.CallerLogos[max]!=NULL) max++;
				for (i=0;i<max;i++) {
					error=Phone->SetBitmap(&s,Backup.CallerLogos[i]);
					Print_Error(error);
					printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				printmsgerr("\n");
			}
		}
	}
	if (answer_yes("Do you want to set date/time in phone (NOTE: in some phones it's required to correctly restore calendar notes and other items)")) {
		GSM_GetCurrentDateTime(&date_time);

		error=Phone->SetDateTime(&s, &date_time);
		Print_Error(error);
	}
	if (Backup.Calendar[0] != NULL) {
		error = Phone->GetNextCalendarNote(&s,&Calendar,true);
		if (error == GE_NONE || error == GE_INVALIDLOCATION || error == GE_EMPTY) {
			if (answer_yes("Restore calendar notes")) {
				Past = answer_yes("Restore notes from the past");
				printmsgerr("Deleting old notes: ");
				while (error==GE_NONE) {
					error = Phone->GetNextCalendarNote(&s,&Calendar,true);
					if (error != GE_NONE) break;
					error = Phone->DeleteCalendar(&s,&Calendar);
					printmsgerr("*");
				}
				printmsgerr("\n");
				max = 0;
				while (Backup.Calendar[max]!=NULL) max++;
				for (i=0;i<max;i++) {
					Calendar = *Backup.Calendar[i];
					error=Phone->AddCalendarNote(&s,&Calendar, Past);
					Print_Error(error);
					printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
					if (bshutdown) {
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
		error = Phone->GetToDo(&s,&ToDo,true);
		if (error == GE_NONE || error == GE_INVALIDLOCATION) {
			if (answer_yes("Restore ToDo")) {
				printmsgerr("Deleting old ToDo: ");
				error=Phone->DeleteAllToDo(&s);
				Print_Error(error);
				printmsgerr("Done\n");
				max = 0;
				while (Backup.ToDo[max]!=NULL) max++;
				for (i=0;i<max;i++) {
					ToDo 		= *Backup.ToDo[i];
					ToDo.Location 	= 0;
					error=Phone->SetToDo(&s,&ToDo);
					Print_Error(error);
					printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				printmsgerr("\n");
			}
		}
	}
	if (Backup.SMSC[0] != NULL) {
		if (answer_yes("Restore SMSC profiles")) {
			max = 0;
			while (Backup.SMSC[max]!=NULL) max++;
			for (i=0;i<max;i++) {
				error=Phone->SetSMSC(&s,Backup.SMSC[i]);
				Print_Error(error);
				printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
				if (bshutdown) {
					GSM_Terminate();
					exit(0);
				}
			}
			printmsgerr("\n");
		}
	}
	if (Backup.StartupLogo != NULL) {
		if (answer_yes("Restore startup logo/text")) {
			error=Phone->SetBitmap(&s,Backup.StartupLogo);
			Print_Error(error);
		}
	}
	if (Backup.OperatorLogo != NULL) {
		if (answer_yes("Restore operator logo")) {
			error=Phone->SetBitmap(&s,Backup.OperatorLogo);
			Print_Error(error);
		}
	}
	if (Backup.WAPBookmark[0] != NULL) {
		Bookmark.Location = 1;
		error = Phone->GetWAPBookmark(&s,&Bookmark);
		if (error == GE_NONE || error == GE_INVALIDLOCATION) {
			if (answer_yes("Restore WAP bookmarks")) {
				printmsgerr("Deleting old bookmarks: ");
				/* One thing to explain: DCT4 phones seems to have bug here.
				 * When delete for example first bookmark, phone change
				 * numeration for getting frame, not for deleting. So, we try to
				 * get 1'st bookmark. Inside frame is "correct" location. We use
				 * it later
				 */
				while (error==GE_NONE) {
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
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				printmsgerr("\n");
			}
		}
	}
	if (Backup.WAPSettings[0] != NULL) {
		Settings.Location = 1;
		error = Phone->GetWAPSettings(&s,&Settings);
		if (error == GE_NONE) {
			if (answer_yes("Restore WAP settings")) {
				max = 0;
				while (Backup.WAPSettings[max]!=NULL) max++;
				for (i=0;i<max;i++) {
					error=Phone->SetWAPSettings(&s,Backup.WAPSettings[i]);
					Print_Error(error);
					printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				printmsgerr("\n");
			}
		}
	}
	if (Backup.Ringtone[0] != NULL) {
		Ringtone.Location 	= 1;
		Ringtone.Format		= 0;
		error = Phone->GetRingtone(&s,&Ringtone,false);
		if (error == GE_NONE || error ==GE_EMPTY) {
			if (answer_yes("Restore user ringtones")) {
				max = 0;
				while (Backup.Ringtone[max]!=NULL) max++;
				for (i=0;i<max;i++) {
					error=GSM_RingtoneConvert(&Ringtone, Backup.Ringtone[i], Ringtone.Format);
					Print_Error(error);
					error=Phone->SetRingtone(&s,&Ringtone,&i);
					Print_Error(error);
					printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				printmsgerr("\n");
			}
		}
	}
	if (Backup.Profiles[0] != NULL) {
		Profile.Location = 1;
		error = Phone->GetProfile(&s,&Profile);
		if (error == GE_NONE) {
			if (answer_yes("Restore profiles")) {
				Profile.Location= 0;
				max = 0;
				while (Backup.Profiles[max]!=NULL) max++;
				for (i=0;i<max;i++) {
					Profile	= *Backup.Profiles[i];
					error=Phone->SetProfile(&s,&Profile);
					Print_Error(error);
					if (bshutdown) {
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

static void AddNew(int argc, char *argv[])
{
	GSM_Backup		Backup;
	GSM_DateTime 		date_time;
	GSM_CalendarEntry	Calendar;
//	GSM_PhonebookEntry	Pbk;
//	GSM_MemoryStatus	MemStatus;
	GSM_ToDoEntry		ToDo;
	GSM_WAPBookmark		Bookmark;
	int			i, max;

	error=GSM_ReadBackupFile(argv[2],&Backup);
	Print_Error(error);

	/* We do not want to make it forever - press Ctrl+C to stop */
	signal(SIGINT, interrupted);
	printmsgerr("If you want break, press Ctrl+C...\n");

	if (Backup.DateTimeAvailable) 	printmsgerr("Time of backup : %s\n",OSDateTime(Backup.DateTime,false));
	if (Backup.Model[0]!=0) 	printmsgerr("Phone          : %s\n",Backup.Model);
	if (Backup.IMEI[0]!=0) 		printmsgerr("IMEI           : %s\n",Backup.IMEI);

	GSM_Init(true);

//	if (Backup.PhonePhonebook[0] != NULL) {
//		MemStatus.MemoryType = GMT_ME;
//		error=Phone->GetMemoryStatus(&s, &MemStatus);
//		if (error==GE_NONE) {
//			max = 0;
//			while (Backup.PhonePhonebook[max]!=NULL) max++;
//			printmsgerr("%i entries in backup file\n",max);
//			if (answer_yes("Add phone phonebook entries")) {
//				for (i=0;i<max;i++) {
//					Pbk 		= *Backup.PhonePhonebook[i];
//					Pbk.MemoryType 	= GMT_ME;
//					Pbk.Location	= 0;
//					error=Phone->SetMemory(&s, &Pbk);
//					Print_Error(error);
//					printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
//					if (bshutdown) {
//						GSM_Terminate();
//						exit(0);
//					}
//				}
//				printmsgerr("\n");
//			}
//		}
//	}
//	if (Backup.SIMPhonebook[0] != NULL) {
//		MemStatus.MemoryType = GMT_SM;
//		error=Phone->GetMemoryStatus(&s, &MemStatus);
//		if (error==GE_NONE) {
//			max = 0;
//			while (Backup.SIMPhonebook[max]!=NULL) max++;
//			printmsgerr("%i entries in backup file\n",max);
//			if (answer_yes("Add SIM phonebook entries")) {
//				for (i=0;i<max;i++) {
//					Pbk 		= *Backup.SIMPhonebook[i];
//					Pbk.MemoryType 	= GMT_SM;
//					Pbk.Location	= 0;
//					error=Phone->SetMemory(&s, &Pbk);
//					Print_Error(error);
//					printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
//					if (bshutdown) {
//						GSM_Terminate();
//						exit(0);
//					}
//				}
//				printmsgerr("\n");
//			}
//		}
//	}
	if (answer_yes("Do you want to set date/time in phone (NOTE: in some phones it's required to correctly restore calendar notes and other items)")) {
		GSM_GetCurrentDateTime(&date_time);

		error=Phone->SetDateTime(&s, &date_time);
		Print_Error(error);
	}
	if (Backup.Calendar[0] != NULL) {
		error = Phone->GetNextCalendarNote(&s,&Calendar,true);
		if (error == GE_NONE || error == GE_INVALIDLOCATION || error == GE_EMPTY) {
			if (answer_yes("Add calendar notes")) {
				max = 0;
				while (Backup.Calendar[max]!=NULL) max++;
				for (i=0;i<max;i++) {
					Calendar = *Backup.Calendar[i];
					error=Phone->AddCalendarNote(&s,&Calendar, true);
					Print_Error(error);
					printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
					if (bshutdown) {
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
		error = Phone->GetToDo(&s,&ToDo,true);
		if (error == GE_NONE || error == GE_INVALIDLOCATION) {
			if (answer_yes("Add ToDo")) {
				max = 0;
				while (Backup.ToDo[max]!=NULL) max++;
				for (i=0;i<max;i++) {
					ToDo 		= *Backup.ToDo[i];
					ToDo.Location 	= 0;
					error=Phone->SetToDo(&s,&ToDo);
					Print_Error(error);
					printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
					if (bshutdown) {
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
		if (error == GE_NONE || error == GE_INVALIDLOCATION) {
			if (answer_yes("Add WAP bookmarks")) {
				max = 0;
				while (Backup.WAPBookmark[max]!=NULL) max++;
				for (i=0;i<max;i++) {
					Bookmark 	  = *Backup.WAPBookmark[i];
					Bookmark.Location = 0;
					error=Phone->SetWAPBookmark(&s,&Bookmark);
					Print_Error(error);
					printmsgerr("%cWriting: %i percent",13,(i+1)*100/max);
					if (bshutdown) {
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
	GSM_CalendarEntry	Calendar;
	GSM_PhonebookEntry	Pbk;
	GSM_MemoryStatus	MemStatus;
	GSM_ToDoEntry		ToDo;
	GSM_WAPBookmark		Bookmark;

	GSM_Init(true);

	MemStatus.MemoryType = GMT_ME;
	error=Phone->GetMemoryStatus(&s, &MemStatus);
	if (error==GE_NONE && MemStatus.Used !=0) {
		if (answer_yes("Delete phone phonebook")) {
			for (i=0;i<MemStatus.Used+MemStatus.Free;i++)
			{
				Pbk.MemoryType 	= GMT_ME;
				Pbk.Location	= i + 1;
				Pbk.EntriesNum	= 0;
				error=Phone->SetMemory(&s, &Pbk);
				Print_Error(error);
				printmsgerr("%cWriting: %i percent",13,(i+1)*100/(MemStatus.Used+MemStatus.Free));
				if (bshutdown) {
					GSM_Terminate();
					exit(0);
				}
			}
			printmsgerr("\n");
		}
	}
	MemStatus.MemoryType = GMT_SM;
	error=Phone->GetMemoryStatus(&s, &MemStatus);
	if (error==GE_NONE && MemStatus.Used !=0) {
		if (answer_yes("Delete SIM phonebook")) {
			for (i=0;i<MemStatus.Used+MemStatus.Free;i++)
			{
				Pbk.MemoryType 	= GMT_SM;
				Pbk.Location	= i + 1;
				Pbk.EntriesNum	= 0;
				error=Phone->SetMemory(&s, &Pbk);
				Print_Error(error);
				printmsgerr("%cWriting: %i percent",13,(i+1)*100/(MemStatus.Used+MemStatus.Free));
				if (bshutdown) {
					GSM_Terminate();
					exit(0);
				}
			}
			printmsgerr("\n");
		}
	}
	error = Phone->GetNextCalendarNote(&s,&Calendar,true);
	if (error == GE_NONE || error == GE_INVALIDLOCATION || error == GE_EMPTY) {
		if (answer_yes("Delete calendar notes")) {
			printmsgerr("Deleting : ");
			while (error==GE_NONE) {
				error = Phone->GetNextCalendarNote(&s,&Calendar,true);
				if (error != GE_NONE) break;
				error = Phone->DeleteCalendar(&s,&Calendar);
				printmsgerr("*");
			}
			printmsgerr("\n");
		}
	}
	ToDo.Location = 1;
	error = Phone->GetToDo(&s,&ToDo,true);
	if (error == GE_NONE || error == GE_INVALIDLOCATION) {
		if (answer_yes("Delete ToDo")) {
			printmsgerr("Deleting: ");
			error=Phone->DeleteAllToDo(&s);
			Print_Error(error);
			printmsgerr("Done\n");
		}
	}
	Bookmark.Location = 1;
	error = Phone->GetWAPBookmark(&s,&Bookmark);
	if (error == GE_NONE || error == GE_INVALIDLOCATION) {
		if (answer_yes("Delete WAP bookmarks")) {
			printmsgerr("Deleting: ");
			/* One thing to explain: DCT4 phones seems to have bug here.
			 * When delete for example first bookmark, phone change
			 * numeration for getting frame, not for deleting. So, we try to
			 * get 1'st bookmark. Inside frame is "correct" location. We use
			 * it later
			 */
			while (error==GE_NONE) {
				error = Phone->DeleteWAPBookmark(&s,&Bookmark);
				Bookmark.Location = 1;
				error = Phone->GetWAPBookmark(&s,&Bookmark);
				printmsgerr("*");
			}
			printmsgerr("\n");
		}
	}

	GSM_Terminate();
}

static void GetWAPSettings(int argc, char *argv[])
{
	GSM_MultiWAPSettings	settings;
	int			start,stop,j;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		settings.Location=i;
		error=Phone->GetWAPSettings(&s,&settings);
		Print_Error(error);
		for (j=0;j<settings.Number;j++) {
			printmsg("%i. ",i);
			if (settings.Settings[j].Title[0]==0 && settings.Settings[j].Title[1]==0)
			{
				printmsg("Set %i\n",i);
			} else {
				printmsg("%s\n",DecodeUnicodeString(settings.Settings[j].Title));
			}
			printmsg("Homepage            : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].HomePage));
			if (settings.Settings[j].IsContinuous) {
				printmsg("Connection type     : Continuous\n");
			} else {
				printmsg("Connection type     : Temporary\n");
			}
			if (settings.Settings[j].IsSecurity) {
				printmsg("Connection security : On\n");
			} else {
				printmsg("Connection security : Off\n");
			}
			switch (settings.Settings[j].Bearer) {
			case WAPSETTINGS_BEARER_SMS:
				printmsg("Bearer              : SMS\n");
				printmsg("Server number       : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].Server));
				printmsg("Service number      : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].Service));
				break;
			case WAPSETTINGS_BEARER_DATA:
				printmsg("Bearer              : Data (CSD)\n");
				printmsg("Dial-up number      : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].DialUp));
				printmsg("IP address          : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].IPAddress));
				if (settings.Settings[j].ManualLogin) {
					printmsg("Login Type          : Manual\n");
				} else {
					printmsg("Login Type          : Automatic\n");
				}
				if (settings.Settings[j].IsNormalAuthentication) {
					printmsg("Authentication type : Normal\n");
				} else {
					printmsg("Authentication type : Secure\n");
				}
				if (settings.Settings[j].IsISDNCall) {
					printmsg("Data call type      : ISDN\n");
	                       	} else {
					printmsg("Data call type      : Analogue\n");  
				}
				switch (settings.Settings[j].Speed) {
					case WAPSETTINGS_SPEED_9600  : printmsg("Data call speed     : 9600\n");  break;
					case WAPSETTINGS_SPEED_14400 : printmsg("Data call speed     : 14400\n"); break;
					case WAPSETTINGS_SPEED_AUTO  : printmsg("Data call speed     : Auto\n");  break;
				}
				printmsg("User name           : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].User));
				printmsg("Password            : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].Password));
				break;
			case WAPSETTINGS_BEARER_USSD:
				printmsg("Bearer              : USSD\n");
				printmsg("Service code        : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].Code));
				if (settings.Settings[j].IsIP) {
					printmsg("Address type        : IP address\nIPaddress           : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].Service));
				} else {
					printmsg("Address type        : Service number\nService number      : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].Service));	
				}
				break;
			case WAPSETTINGS_BEARER_GPRS:
				printmsg("Bearer              : GPRS\n");
				if (settings.Settings[j].ManualLogin) {
					printmsg("Login Type          : Manual\n");
				} else {
					printmsg("Login Type          : Automatic\n");
				}
				if (settings.Settings[j].IsNormalAuthentication) {
					printmsg("Authentication type : Normal\n");
				} else {
					printmsg("Authentication type : Secure\n");
				}
				printmsg("Access point        : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].DialUp));
				printmsg("IP address          : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].IPAddress));
				printmsg("User name           : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].User));
				printmsg("Password            : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].Password));
			}
			printmsg("\n");
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
	bool			start 		= true;
	bool			DeleteAfter;
	int			j, smsnum;
	char			buffer[200];

	/* We ignore return code, because (when file doesn't exist) we
	 * will create new later
	 */
	GSM_ReadSMSBackupFile(argv[2], &Backup);
	smsnum = 0;
	while (Backup.SMS[smsnum]!=NULL) smsnum++;

	GSM_Init(true);

	error=Phone->GetSMSFolders(&s, &folders);
	Print_Error(error);

	DeleteAfter=answer_yes("Delete each sms after backup");

	for (j=0;j<folders.Number;j++) {
		BackupFromFolder[j] = false;
		sprintf(buffer,"Backup sms from folder \"%s\"",DecodeUnicodeString(folders.Folder[j].Name));
		if (answer_yes(buffer)) BackupFromFolder[j] = true;
	}

	while (error == GE_NONE) {
		sms.SMS[0].Folder=0x00;
		error=Phone->GetNextSMSMessage(&s, &sms, start);
		switch (error) {
		case GE_EMPTY:
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
						        if (Backup.SMS[smsnum] == NULL) Print_Error(GE_MOREMEMORY);
							Backup.SMS[smsnum+1] = NULL;
						} else {
							printmsg("Increase GSM_BACKUP_MAX_SMS\n");
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

	error = GSM_SaveSMSBackupFile(argv[2],&Backup);
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

static void RestoreSMS(int argc, char *argv[])
{
	GSM_MultiSMSMessage 	SMS;
	GSM_SMS_Backup		Backup;
	GSM_SMSFolders		folders;
	int			smsnum = 0;
	char			buffer[200];

	error=GSM_ReadSMSBackupFile(argv[2], &Backup);
	Print_Error(error);

	GSM_Init(true);

	error=Phone->GetSMSFolders(&s, &folders);
	Print_Error(error);

	while (Backup.SMS[smsnum] != NULL) {
		SMS.Number = 1;
		memcpy(&SMS.SMS[0],Backup.SMS[smsnum],sizeof(GSM_SMSMessage));	
		displaymultismsinfo(SMS);
		sprintf(buffer,"Restore sms to folder \"%s\"",DecodeUnicodeString(folders.Folder[Backup.SMS[smsnum]->Folder-1].Name));
		if (answer_yes(buffer)) {
			Backup.SMS[smsnum]->Location = 0;
			error=Phone->SaveSMSMessage(&s, Backup.SMS[smsnum]);
			Print_Error(error);
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
				case GSM_CallerLogo  :	printmsg("Caller group logo"); 	break;
				default		     : 					break;
			}
			printmsg(", width %i, height %i\n",Bitmap.Bitmap[i].Width,Bitmap.Bitmap[i].Height);
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
					Bitmap.Bitmap[i].Type = GSM_CallerLogo;
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
	int			DefNoteScale 	= Scale_880;
  
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
					printmsg("Ringtone \"%s\" (tempo = %i Beats Per Minute)\n\n",DecodeUnicodeString(ringtone.Name),GSM_RTTLGetTempo(Note->Tempo));
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
					if (Note->Scale != DefNoteScale) {
						while (DefNoteScale != Note->Scale) {
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
					default		   : dprintf("error\n");break;
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

	printmsg("\n");
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

struct keys_table_position {
	char 	whatchar;
	int 	whatcode;
};

static struct keys_table_position Keys[] = {
	{'m',GSM_KEY_MENU},		{'M',GSM_KEY_MENU},
	{'n',GSM_KEY_NAMES},		{'N',GSM_KEY_NAMES},
	{'p',GSM_KEY_POWER},		{'P',GSM_KEY_POWER},
	{'u',GSM_KEY_UP},		{'U',GSM_KEY_UP},
	{'d',GSM_KEY_DOWN},		{'D',GSM_KEY_DOWN},
	{'+',GSM_KEY_INCREASEVOLUME},	{'-',GSM_KEY_DECREASEVOLUME},
	{'1',GSM_KEY_1},		{'2',GSM_KEY_2},	{'3',GSM_KEY_3},
	{'4',GSM_KEY_4},		{'5',GSM_KEY_5},	{'6',GSM_KEY_6},
	{'7',GSM_KEY_7},		{'8',GSM_KEY_8},	{'9',GSM_KEY_9},
	{'*',GSM_KEY_ASTERISK},		{'0',GSM_KEY_0},	{'#',GSM_KEY_HASH},
	{'g',GSM_KEY_GREEN},		{'G',GSM_KEY_GREEN},
	{'r',GSM_KEY_RED},		{'R',GSM_KEY_RED},
	{' ',0}
};

static void PressKeySequence(int argc, char *argv[])
{
	int 		i,j;
	unsigned char 	key;
	GSM_KeyCode	KeyCode;

	GSM_Init(true);

	for (i=0;i<(int)(strlen(argv[2]));i++) {
		key = argv[2][i];
		if (key == 'w' || key == 'W') {
		} else {
			KeyCode = GSM_KEY_NONE;
			j = 0;
			while (Keys[j].whatchar!=' ') {
		        	if (Keys[j].whatchar==key) {    
					KeyCode=Keys[j].whatcode;
					break;
				}
				j++;
			}
			if (KeyCode == GSM_KEY_NONE) {
				printmsg("Unknown key/function name: \"%c\"\n",key);
				GSM_Terminate();
				exit(-1);
			}
			error=Phone->PressKey(&s, KeyCode, true);
			Print_Error(error);			
			error=Phone->PressKey(&s, KeyCode, false);
			Print_Error(error);			
		}
	}

	GSM_Terminate();
}

static void GetAllCategories(int argc, char *argv[])
{
	GSM_Category		Category;
	GSM_CategoryStatus	Status;
	int			j, count;

	if (mystrncasecmp(argv[2],"TODO",0)) {
        	Category.Type = Category_ToDo;
        	Status.Type = Category_ToDo;
	} else if (mystrncasecmp(argv[2],"PHONEBOOK",0)) {
	        Category.Type = Category_Phonebook;
        	Status.Type = Category_Phonebook;
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
        
		if (error != GE_EMPTY) {
        	    printmsg("Location: %i\n",j);

        	    Print_Error(error);

        	    printmsg("Name    : \"%s\"\n\n",DecodeUnicodeString(Category.Name));
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
		if (error != GE_EMPTY) Print_Error(error);

		if (error == GE_EMPTY) {
			printmsg("Entry is empty\n\n");
		} else {
        		printmsg("Name    : \"%s\"\n\n",DecodeUnicodeString(Category.Name));
    		}
	}

	GSM_Terminate();
}

static void GetToDo(int argc, char *argv[])
{
	GSM_ToDoEntry	ToDo;
	int		start,stop,j;
	bool		refresh=true;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		ToDo.Location=i;
		printmsg("Location  : %i\n",i);
		error=Phone->GetToDo(&s,&ToDo,refresh);
		if (error != GE_EMPTY) Print_Error(error);

		if (error == GE_EMPTY) {
			printmsg("Entry is empty\n");
		} else {
            		printmsg("Priority  : ");
            		switch (ToDo.Priority) {
                		case GSM_Priority_Low	 : printmsg("Low\n");	 	break;
                		case GSM_Priority_Medium : printmsg("Medium\n"); 	break;
                		case GSM_Priority_High	 : printmsg("High\n");		break;
	            	}
	            	for (j=0;j<ToDo.EntriesNum;j++) {
	                	switch (ToDo.Entries[j].EntryType) {
	                    	case TODO_DUEDATE:
	                        	printmsg("DueDate   : %s\n",OSDate(ToDo.Entries[j].Date));
	                        	break;
	                    	case TODO_COMPLETED:
	                        	printmsg("Completed : %s\n",ToDo.Entries[j].Number == 1 ? "Yes" : "No");
	                        	break;
	                    	case TODO_ALARM_DATETIME:
	                        	printmsg("Alarm     : %s\n",OSDateTime(ToDo.Entries[j].Date,false));
	                        	break;
	                    	case TODO_TEXT:
	                        	printmsg("Text      : \"%s\"\n",DecodeUnicodeString(ToDo.Entries[j].Text));
	                        	break;
	                    	case TODO_PRIVATE:
	                        	printmsg("Private   : %s\n",ToDo.Entries[j].Number == 1 ? "Yes" : "No");
	                        	break;
	                    	case TODO_CATEGORY:
	                        	printmsg("Category  : %i\n",ToDo.Entries[j].Number);
	                        	break;
	                    	case TODO_CONTACTID:
	                        	printmsg("Contact ID: %i\n",ToDo.Entries[j].Number);
        	                	break;
	                    	case TODO_PHONE:
	                        	printmsg("Phone     : \"%s\"\n",DecodeUnicodeString(ToDo.Entries[j].Text));
	                        	break;
	                	}
	       	    	}
        	    	printmsg("\n");
	            	refresh=false;
	       	 }
	}

	GSM_Terminate();        	
}

static void GetSecurityStatus(int argc, char *argv[])
{
	GSM_SecurityCodeType Status;

	GSM_Init(true);

	error=Phone->GetSecurityStatus(&s,&Status);
	Print_Error(error);
	switch(Status) {
		case GSCT_SecurityCode: printmsg("Waiting for Security Code.\n"); break;
		case GSCT_Pin         : printmsg("Waiting for PIN.\n"); 		break;
		case GSCT_Pin2        : printmsg("Waiting for PIN2.\n"); 		break;
		case GSCT_Puk         : printmsg("Waiting for PUK.\n");		break;
		case GSCT_Puk2        : printmsg("Waiting for PUK2.\n"); 		break;
		case GSCT_None        : printmsg("Nothing to enter.\n"); 		break;
		default		      : printmsg("Unknown\n");
	}      
	
	GSM_Terminate();
}

static void EnterSecurityCode(int argc, char *argv[])
{
	GSM_SecurityCode Code;

	if (mystrncasecmp(argv[2],"PIN",0)) {		Code.Type = GSCT_Pin;
	} else if (mystrncasecmp(argv[2],"PUK",0)) {	Code.Type = GSCT_Puk;
	} else if (mystrncasecmp(argv[2],"PIN2",0)) {	Code.Type = GSCT_Pin2;
	} else if (mystrncasecmp(argv[2],"PUK2",0)) {	Code.Type = GSCT_Puk2;
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
	GSM_AllRingtonesInfo 	Info;

	GetStartStop(&start, &stop, 2, argc, argv);

	for (i=0;i<5;i++) callerinit[i] = false;

	GSM_Init(true);

	error=Phone->GetRingtonesInfo(&s,&Info);
	if (error != GE_NONE) Info.Number = 0;

	for (i=start;i<=stop;i++) {
		Profile.Location=i;
		error=Phone->GetProfile(&s,&Profile);
		Print_Error(error);

		printmsg("%i. \"%s\"",i,DecodeUnicodeString(Profile.Name));
		if (Profile.DefaultName) 	printmsg(" (default name)");
		if (Profile.HeadSetProfile) 	printmsg(" (HeadSet profile)");
		if (Profile.CarKitProfile) 	printmsg(" (CarKit profile)");
		printmsg("\n");
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
				if (strlen(DecodeUnicodeString(GSM_GetRingtoneName(&Info,Profile.FeatureValue[j])))!=0) {
					printmsg("\"%s\"\n",DecodeUnicodeString(GSM_GetRingtoneName(&Info,Profile.FeatureValue[j])));
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
							caller[k].Type	 	= GSM_CallerLogo;
							caller[k].Location 	= k + 1;
							error=Phone->GetBitmap(&s,&caller[k]);
							if (error == GE_SECURITYERROR) {
								NOKIA_GetDefaultCallerGroupName(&s,&caller[k]);
							} else {
								Print_Error(error);
							}
							callerinit[k]	= true;
						}
						printmsg(" \"%s\"",DecodeUnicodeString(caller[k].Text));
					}
				}
				printmsg("\n");
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
		printmsg("\n");
	}

	GSM_Terminate();
}

static void GetSpeedDial(int argc, char *argv[])
{
	GSM_SpeedDial		SpeedDial;
	GSM_PhonebookEntry	Phonebook;
	int			start,stop,Name,Number,Group;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		SpeedDial.Location=i;
		error=Phone->GetSpeedDial(&s,&SpeedDial);
		printmsg("%i.",i);
		switch (error) {
		case GE_EMPTY:
			printmsg(" speed dial not assigned\n");
			break;
		default:
			Print_Error(error);

			Phonebook.Location	= SpeedDial.MemoryLocation;
			Phonebook.MemoryType 	= SpeedDial.MemoryType;
			error=Phone->GetMemory(&s,&Phonebook);

			GSM_PhonebookFindDefaultNameNumberGroup(Phonebook, &Name, &Number, &Group);
	
			if (Name != -1) printmsg(" Name \"%s\",",DecodeUnicodeString(Phonebook.Entries[Name].Text));
			printmsg(" Number \"%s\"",DecodeUnicodeString(Phonebook.Entries[SpeedDial.MemoryNumberID-1].Text));
		}
		printmsg("\n");
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
	DCT4ResetSecurityCode(argc, argv);
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

static void NokiaTests(int argc, char *argv[])
{
	GSM_Init(true);

#ifdef GSM_ENABLE_NOKIA_DCT3
	DCT3tests(argc, argv);
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
	DCT4tests(argc, argv);
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

	printmsg("Deleting SMS from \"%s\" folder: ",DecodeUnicodeString(folders.Folder[foldernum-1].Name));

	while (error == GE_NONE) {
		sms.SMS[0].Folder=0x00;
		error=Phone->GetNextSMSMessage(&s, &sms, start);
		switch (error) {
		case GE_EMPTY:
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

	printmsg("\n");

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
		case GSM_FaxCall	: printmsg("Fax call\n");		break;
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
	if (!file) Print_Error(GE_CANTOPENFILE);
	size=fread(InputBuffer, 1, 10000-1, file);
	fclose(file);
	InputBuffer[size]   = 0;
	InputBuffer[size+1] = 0;

	ReadUnicodeFile(Buffer,InputBuffer);

	for(i=0;i<((int)strlen(DecodeUnicodeString(Buffer)));i++) {
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
	extern GSM_Network 	GSM_Networks[];
	int 			i;

	printmsg("Network  Name\n");
	printmsg("-----------------------------------------\n");
	for (i = 0; strcmp(GSM_Networks[i].Name, "unknown"); i++)
		printmsg("%-7s  %s\n", GSM_Networks[i].Code, GSM_Networks[i].Name);
}

static void Version(int argc, char *argv[])
{
	printmsg("[Gammu version %s built %s %s]\n\n",VERSION,__TIME__,__DATE__);

#ifdef DEBUG
	printf("GSM_SMSMessage - %i\n",sizeof(GSM_SMSMessage));
	printf("GSM_SMSC       - %i\n",sizeof(GSM_SMSC));
	printf("GSM_SMS_State  - %i\n",sizeof(GSM_SMS_State));
	printf("GSM_UDHHeader  - %i\n",sizeof(GSM_UDHHeader));
	printf("bool           - %i\n",sizeof(bool));
	printf("GSM_DateTime   - %i\n",sizeof(GSM_DateTime));
	printf("int            - %i\n",sizeof(int));
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
		switch (error) {
    		    case GE_EMPTY:
			    printmsg("Empty entry\n");
			    break;
		    case GE_NONE:
			    printmsg("Station name: %s\nFrequency: %d.%d MHz\n",
				    DecodeUnicodeString(Station.StationName),
				    Station.Frequency/1000,Station.Frequency%1000);
			    break;
		    default:
			    Print_Error(error);
		}
	}
	GSM_Terminate();
}

static void usage(void)
{
	printmsg("[Gammu version %s built %s %s]\n\n",VERSION,__TIME__,__DATE__);

	printf("gammu --version\n");
	printf("gammu --identify\n");
	printf("gammu --monitor [times]\n");
	printf("gammu --reset SOFT|HARD\n");
	printf("gammu --resetphonesettings PHONE|DEV|UIF|ALL|FACTORY\n");
	printf("gammu --presskeysequence mMnNpPuUdD+-123456789*0#gGrRwW\n");
	printf("gammu --getdisplaystatus\n");
	printf("gammu --getprofile start [stop]\n");
	printf("gammu --setautonetworklogin\n");
	printf("gammu --getsecuritystatus\n");
	printf("gammu --entersecuritycode PIN|PUK|PIN2|PUK2 code\n");
	printf("gammu --listnetworks\n");
	printf("gammu --getfmstation start [stop]\n\n");

	printf("gammu --getdatetime\n");
	printf("gammu --setdatetime\n");
	printf("gammu --getalarm\n");
	printf("gammu --setalarm hour minute\n\n");

	printf("gammu --getmemory DC|MC|RC|ON|VM|SM|ME start [stop]\n");
	printf("gammu --senddtmf sequence\n");
	printf("gammu --getspeeddial start [stop]\n\n");

	printf("gammu --getsms folder start [stop]\n");
	printf("gammu --deletesms folder start [stop]\n");
	printf("gammu --deleteallsms folder\n");
	printf("gammu --getallsms\n");
	printf("gammu --geteachsms\n");
	printf("gammu --getsmsc start [stop]\n");
	printf("gammu --getsmsfolders\n");
	printf("gammu --smsd FILES configfile\n\n");

	printf("gammu --getphoneringtone location [file]\n");
	printf("gammu --getringtone location [file]\n");
	printf("gammu --setringtone file [location] [-scale]\n");
	printf("gammu --copyringtone source destination [RTTL|BINARY]\n");
	printf("gammu --playringtone file\n");
	printf("gammu --getringtoneslist\n");
	printf("gammu --nokiacomposer file\n\n");

	printf("gammu --dialvoice number\n");
	printf("gammu --answercall\n");
	printf("gammu --cancelcall\n\n");

	printf("gammu --gettodo start [stop]\n");
	printf("gammu --getcalendarnotes\n\n");
	
	printf("gammu --getcategory TODO|PHONEBOOK start [stop]\n");
	printf("gammu --getallcategies TODO|PHONEBOOK\n\n");

	printf("gammu --getwapbookmark start [stop]\n");
	printf("gammu --deletewapbookmark start [stop]\n");
	printf("gammu --getwapsettings start [stop]\n\n");

	printf("gammu --getbitmap STARTUP [file]\n");
	printf("gammu --getbitmap CALLER location [file]\n");
	printf("gammu --getbitmap OPERATOR [file]\n");
	printf("gammu --getbitmap PICTURE location [file]\n");
	printf("gammu --getbitmap TEXT\n");
	printf("gammu --getbitmap DEALER\n\n");

	printf("gammu --setbitmap STARTUP file|1|2|3\n");
	printf("gammu --setbitmap CALLER location [file]\n");
	printf("gammu --setbitmap OPERATOR [file [netcode]]\n");
	printf("gammu --setbitmap PICTURE file location [text]\n");
	printf("gammu --setbitmap TEXT text\n");
	printf("gammu --setbitmap DEALER text\n\n");

	printf("gammu --copybitmap inputfile [outputfile [OPERATOR|PICTURE|STARTUP|CALLER]]\n\n");

	printf("gammu --savesms TEXT [-folder number][-reply][-sender number][-flash]\n");
	printf("                     [-smscset number][-smscnumber number][-len len]\n");
	printf("                     [-enablefax][-disablefax][-enablevoice][-unsent]\n");
	printf("                     [-disablevoice][-enableemail][-disableemail][-read]\n");
	printf("                     [-voidsms][-unicode][-inputunicode][-unread][-autolen len]\n");
	printf("		     [-replacemessages ID][-replacefile file]\n");
	printf("gammu --savesms RINGTONE file [-folder number][-sender number][-reply]\n");
	printf("                              [-smscset number][-smscnumber number]\n");
	printf("                              [-long][-unread][-read][-unsent][-scale]\n");
	printf("gammu --savesms OPERATOR file [-folder number][-sender number][-reply]\n");
	printf("                              [-netcode netcode][-smscset number][-biglogo]\n");
	printf("                              [-smscnumber number][-unread][-unsent][-read]\n");
	printf("gammu --savesms CALLER file [-folder number][-sender number][-unread]\n");
	printf("                            [-reply][-smscset number][-smscnumber number]\n");
	printf("                            [-unsent][-read]\n");
	printf("gammu --savesms PICTURE file [-folder number][-sender number][-unicode]\n");
	printf("                             [-smscset number][-smscnumber number][-read]\n");
	printf("                             [-text text][-reply][-unread][-unsent]\n");
#ifdef GSM_ENABLE_BACKUP
	printf("gammu --savesms BOOKMARK file location [-folder number][-reply][-unsent]\n");
	printf("                                       [-sender number][-smscset number]\n");
	printf("                                       [-smscnumber number][-unread]\n");
	printf("                                       [-read]\n");
	printf("gammu --savesms SETTINGS file location [-folder number][-reply][-unsent]\n");
	printf("                                       [-sender number][-smscset number]\n");
	printf("                                       [-smscnumber number][-unread]\n");
	printf("                                       [-read]\n");
	printf("gammu --savesms CALENDAR file location [-folder number][-sender number]\n");
	printf("                                       [-smscset number][-unsent][-reply]\n");
	printf("                                       [-smscnumber number][-unread]\n");
	printf("                                       [-read]\n");
	printf("gammu --savesms VCARD10|VCARD21 file SM|ME location [-folder number]\n");
	printf("                                                    [-smscset number]\n");
	printf("                                                    [-reply][-unread]\n");
	printf("                                                    [-smscnumber number]\n");
	printf("                                                    [-sender number]\n");
	printf("                                                    [-read][-unsent]\n");
#endif
	printf("gammu --savesms PROFILE name bitmap ringtone [-folder number][-reply]\n");
	printf("                                             [-smscset number][-unsent]\n");
	printf("                                             [-smscnumber number][-read]\n");
	printf("                                             [-sender number][-unread]\n");
	printf("gammu --savesms EMS [-text \"text\"][-defanimation ID][-defsound ID]\n");
	printf("                    [-tone10 file][-tone12 file][-animation frames file1 ...]\n");
	printf("                    [-folder number][-reply][-smscset number][-unsent]\n");
	printf("                    [-smscnumber number][-read][-sender number][-unread]\n\n");

	printf("gammu --sendsms TEXT destination [-reply][-flash][-smscset number]\n");
	printf("                                 [-smscnumber number][-len len]\n");
	printf("                                 [-enablefax][-disablefax][-enablevoice]\n");
	printf("                                 [-disablevoice][-enableemail][-report]\n");
	printf("                                 [-disableemail][-voidsms][-unicode]\n");
	printf("                                 [-inputunicode][-replacemessages ID]\n");
	printf("                                 [-replacefile file][-autolen len]\n");
	printf("                                 [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX]\n");
	printf("gammu --sendsms RINGTONE destination file [-reply][-smscset number]\n");
	printf("                                          [-long][-smscnumber number]\n");
	printf("                                          [-report][-scale][-validity \n");
	printf("                                          HOUR|6HOURS|DAY|3DAYS|WEEK|MAX]\n");
	printf("gammu --sendsms OPERATOR destination file [-reply][-smscset number][-biglogo]\n");
	printf("                                          [-netcode netcode][-report]\n");
	printf("                                          [-smscnumber number][-validity \n");
	printf("                                          HOUR|6HOURS|DAY|3DAYS|WEEK|MAX]\n");
	printf("gammu --sendsms CALLER destination file [-reply][-smscset number]\n");
	printf("                                        [-report][-smscnumber number][-validity \n");
	printf("                                        HOUR|6HOURS|DAY|3DAYS|WEEK|MAX]\n");
	printf("gammu --sendsms PICTURE destination file [-reply][-smscset number]\n");
	printf("                                         [-report][-smscnumber number]\n");
	printf("                                         [-unicode][-text text][-validity \n");
	printf("                                         HOUR|6HOURS|DAY|3DAYS|WEEK|MAX]\n");

#ifdef GSM_ENABLE_BACKUP
	printf("gammu --sendsms BOOKMARK destination file location [-smscset number]\n");
	printf("                                                   [-reply][-report]\n");
	printf("                                                   [-smscnumber number]\n");
	printf("                                [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX]\n");
	printf("gammu --sendsms SETTINGS destination file location [-smscset number]\n");
	printf("                                                   [-smscnumber number]\n");
	printf("                                                   [-report][-reply]\n");
	printf("                                [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX]\n");
	printf("gammu --sendsms CALENDAR destination file location [-smscset number]\n");
	printf("                                                   [-smscnumber number]\n");
	printf("                                                   [-report][-reply]\n");
	printf("                                [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX]\n");
	printf("gammu --sendsms VCARD10|VCARD21 destination file SM|ME location\n");
	printf("                     [-reply][-smscset number][-smscnumber number][-report]\n");
	printf("                     [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX]\n");
#endif
	printf("gammu --sendsms PROFILE destination name bitmap ringtone\n");
	printf("                     [-reply][-smscset number][-smscnumber number][-report]\n");
	printf("                     [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX]\n");
	printf("gammu --sendsms EMS destination [-text \"text\"][-defanimation ID][-defsound ID]\n");
	printf("                                [-tone10 file][-tone12 file][-reply]\n");
        printf("                                [-animation frames file1 ...][-report]\n");
	printf("                                [-smscset number][-smscnumber number]\n");
	printf("                                [-validity HOUR|6HOURS|DAY|3DAYS|WEEK|MAX]\n");


#if defined(GSM_ENABLE_NOKIA_DCT3) || defined(GSM_ENABLE_NOKIA_DCT4)
	printf("\n");
#endif
#ifdef GSM_ENABLE_NOKIA6110
	printf("gammu --nokiagetoperatorname\n");
	printf("gammu --nokiasetoperatorname [networkcode name]\n");
#endif
#ifdef GSM_ENABLE_NOKIA_DCT3
	printf("gammu --nokianetmonitor test\n");
	printf("gammu --nokianetmonitor36\n");
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
	printf("gammu --nokiasetvibralevel level\n");
	printf("gammu --nokiagetvoicerecord location\n");
#endif
#if defined(GSM_ENABLE_NOKIA_DCT3) || defined(GSM_ENABLE_NOKIA_DCT4)
	printf("gammu --nokiasecuritycode\n");
	printf("gammu --nokiatests\n");
	printf("gammu --nokiasetphonemenus\n");
#endif

#ifdef GSM_ENABLE_BACKUP
	printf("\ngammu --backup file\n");
	printf("gammu --backupsms file\n");
	printf("gammu --restore file\n");
	printf("gammu --restoresms file\n");
	printf("gammu --addnew file\n");
#else
	printf("\n");
#endif
	printf("gammu --clearall\n");

#ifdef DEBUG
	printf("\ngammu --decodesniff MBUS2|IRDA file [phonemodel]\n");
	printf("gammu --decodebinarydump file [phonemodel]\n");
	printf("gammu --makeconverttable file\n");
#endif
}

static GSM_Parameters Parameters[] = {
	{"--identify",			0, 0, Identify			},
	{"--version",			0, 0, Version			},
	{"--monitor",			0, 1, Monitor			},
	{"--listnetworks",		0, 0, ListNetworks		},
#ifdef GSM_ENABLE_NOKIA6110
	{"--nokiagetoperatorname", 	0, 0, DCT3GetOperatorName	},
	{"--nokiasetoperatorname", 	0, 2, DCT3SetOperatorName	},
#endif
#ifdef GSM_ENABLE_NOKIA_DCT3
	{"--nokianetmonitor",		1, 1, DCT3netmonitor		},
	{"--nokianetmonitor36",		0, 0, DCT3ResetTest36		},
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
	{"--nokiasetvibralevel",	1, 1, DCT4SetVibraLevel		},
	{"--nokiagetvoicerecord",	1, 1, DCT4GetVoiceRecord	},
#endif
#if defined(GSM_ENABLE_NOKIA_DCT3) || defined(GSM_ENABLE_NOKIA_DCT4)
	{"--nokiasecuritycode",		0, 0, NokiaSecurityCode		},
	{"--nokiatests",		0, 0, NokiaTests		},
	{"--nokiasetphonemenus",	0, 0, NokiaSetPhoneMenus	},
#endif
	{"--playringtone",		1, 1, PlayRingtone 		},
	{"--setautonetworklogin",	0, 0, SetAutoNetworkLogin	},
	{"--getdisplaystatus",		0, 0, GetDisplayStatus		},
	{"--clearall",			0, 0, ClearAll			},
	{"--senddtmf",			1, 1, SendDTMF			},
	{"--getdatetime",		0, 0, GetDateTime		},
	{"--setdatetime",		0, 0, SetDateTime		},
	{"--getalarm",			0, 0, GetAlarm			},
	{"--setalarm",			2, 2, SetAlarm			},
	{"--getspeeddial",		1, 2, GetSpeedDial		},
	{"--resetphonesettings",	1, 1, ResetPhoneSettings	},
	{"--getmemory",			2, 3, GetMemory			},
	{"--getfmstation",		1, 2, GetFMStation		},
	{"--getsmsc",			1, 2, GetSMSC			},
	{"--getsms",			2, 3, GetSMS			},
	{"--deletesms",			2, 3, DeleteSMS			},
	{"--deleteallsms",		1, 1, DeleteAllSMS		},
	{"--getsmsfolders",		0, 0, GetSMSFolders		},
	{"--getallsms",			0, 0, GetAllSMS			},
	{"--geteachsms",		0, 0, GetEachSMS		},
	{"--getringtone",		1, 2, GetRingtone		},
	{"--getphoneringtone",		1, 2, GetRingtone		},
	{"--getringtoneslist",		0, 0, GetRingtonesList		},
	{"--setringtone",		1, 3, SetRingtone		},
	{"--nokiacomposer",		1, 1, NokiaComposer		},
	{"--copyringtone",		2, 3, CopyRingtone		},
	{"--dialvoice",			1, 1, DialVoice			},
	{"--cancelcall",		0, 0, CancelCall		},
	{"--answercall",		0, 0, AnswerCall		},
	{"--getcalendarnotes",		0, 0, GetCalendarNotes		},
	{"--gettodo",			1, 2, GetToDo			},
	{"--getcategory",       	2, 3, GetCategory       	},
	{"--getallcategories",  	1, 1, GetAllCategories  	},
	{"--reset",			1, 1, Reset			},
	{"--getprofile",		1, 2, GetProfile		},
	{"--getsecuritystatus",		0, 0, GetSecurityStatus		},
	{"--entersecuritycode",		2, 2, EnterSecurityCode		},
	{"--deletewapbookmark", 	1, 2, DeleteWAPBookmark 	},
	{"--getwapbookmark",		1, 2, GetWAPBookmark		},
	{"--getwapsettings",		1, 2, GetWAPSettings		},
	{"--getbitmap",			1, 3, GetBitmap			},
	{"--setbitmap",			1, 4, SetBitmap			},
	{"--savesms",			1,30, SendSaveSMS		},
	{"--sendsms",			2,30, SendSaveSMS		},
	{"--smsd",			2, 2, SMSDaemon			},
#ifdef GSM_ENABLE_BACKUP
	{"--backup",			1, 1, Backup			},
	{"--backupsms",			1, 1, BackupSMS			},
	{"--restore",			1, 1, Restore			},
	{"--addnew",			1, 1, AddNew			},
	{"--restoresms",		1, 1, RestoreSMS		},
#endif
	{"--copybitmap",		1, 3, CopyBitmap		},
	{"--presskeysequence",		1, 1, PressKeySequence		},
#ifdef DEBUG
	{"--decodesniff",		2, 3, decodesniff		},
	{"--decodebinarydump",		1, 2, decodebinarydump		},
	{"--makeconverttable",		1, 1, MakeConvertTable		},
#endif
	{"",				0, 0, NULL			}
};

int main(int argc, char *argv[])
{
	int 	z = 0,start=0,i;
	char	*argv2[50];

	s.opened = false;
	s.msg	 = NULL;

#ifndef WIN32
	setlocale(LC_ALL, "");
#else
	setlocale(LC_ALL, ".OCP");
#endif
#ifdef DEBUG
	di.dl	 = DL_TEXTALL;
	di.df 	 = stdout;
#endif

	cfg=CFG_FindGammuRC();
	if (cfg!=NULL) {
	        s.Config.Localize = CFG_Get(cfg, "gammu", "gammuloc", false);
        	if (s.Config.Localize) s.msg=CFG_ReadFile(s.Config.Localize, true);
	}
	CFG_ReadConfig(cfg, &s.Config);

    	/* We want to use only one file descriptor for global and state machine debug output */
    	s.Config.UseGlobalDebugFile = true;

	/* When user gave debug level on command line */
	if (argc > 1 && GSM_SetDebugLevel(argv[1], &di)) {
		/* Debug level from command line will be used with phone too */
		strcpy(s.Config.DebugLevel,argv[1]);
		start = 1;
	} else {
		/* Try to set debug level from config file */
		GSM_SetDebugLevel(s.Config.DebugLevel, &di);
	}

	/* If user gave debug file in gammurc, we will use it */
	error=GSM_SetDebugFile(s.Config.DebugFile, &di);
	Print_Error(error);

	if (argc==1) {
		usage();
	} else {
		while (Parameters[z].Function!=NULL) {
			if (mystrncasecmp(Parameters[z].parameter,argv[1+start],0) &&
			    argc-2-start>=Parameters[z].min_arg &&
			    argc-2-start<=Parameters[z].max_arg)
			{
				if (!mystrncasecmp(GetGammuVersion(),VERSION,0)) {
					printmsg("ERROR: version of installed libGammu.so (%s) is different to version of Gammu (%s)\n",
								GetGammuVersion(),VERSION);
					exit(-1);
				}

				if (start == 0) {
					Parameters[z].Function(argc,argv);
				} else {
					argv2[0] = argv[0];
					i = 2;
					while (argv[i] != NULL) {
						argv2[i-1] = argv[i];
						i++;
					}
					argv2[i] = NULL;
					Parameters[z].Function(argc-1,argv2);
				}
				break;
			}
			z++;
		}
		if (Parameters[z].Function==NULL) usage();
	}

    	/* Close debug output if opened */
    	if (di.df!=stdout) fclose(di.df);

	return 0;
}

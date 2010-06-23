
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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
#ifdef DEBUG
#  include "sniff.h"
#endif
#ifdef GSM_ENABLE_NOKIA_DCT3
#  include "dct3.h"
#endif
#ifdef GSM_ENABLE_NOKIA_DCT4
#  include "dct4.h"
#endif
                			
GSM_StateMachine		s;
GSM_Phone_Functions		*Phone;
static CFG_Header		*cfg 			= NULL;

GSM_Error			error 			= GE_NONE;
static int			i;

static volatile bool 		bshutdown 		= false;
static bool 			always_answer_yes 	= false;

/* SIGINT signal handler. */
static void interrupted(int sig)
{
	signal(sig, SIG_IGN);
	bshutdown = true;
}

char *Msg (int number, char *default_string)
{
	return GetMsg(s.msg,number,default_string);
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
		printf("%s",Msg(1020,"ERROR: enumerate locations from 1"));
		exit (-1);
	}

	if (stop!=NULL) {
		*stop=*start;
		if (argc>=num+2) *stop=atoi(argv[num+1]);
		if (*stop==0) {
			printf("%s",Msg(1020,"ERROR: enumerate locations from 1"));
			exit (-1);
		}
	}
}

static bool answer_yes(char *text)
{
	int	len;
	char	ans[99];

	while (1) {
		fprintf(stderr,"%s (yes/no/ALL) ? ",text);
		if (always_answer_yes) {
			fprintf(stderr,"YES (always)\n");
			return true;
		}
		len=GetLine(stdin, ans, 99);
		if (len==-1) exit(-1);
		if (!strcmp(ans, "ALL")) {
			always_answer_yes = true;
			return true;
		}
		if (!strcmp(ans, "yes")) return true;
		if (!strcmp(ans, "no" )) return false;
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
	printf("If you want break, press Ctrl+C...\n");

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
	printf("%s %s ", Msg(1000,"Model         :"), DecodeUnicodeString(buffer));	
	printf("%s (%s)\n", GetModelData(NULL,s.Model,NULL)->model, s.Model);
	printf("%s %s",Msg(1001,"Firmware      :"),s.Ver);

	error=Phone->GetPPM(&s, buffer);
	if (error != GE_NOTSUPPORTED) {
		if (error != GE_NOTIMPLEMENTED) Print_Error(error);
		if (error == GE_NONE) printf(" %s",buffer);
	}

	printf("\n");

	if (s.VerDate[0]!=0) printf("Firmware date : %s\n",s.VerDate);

	error=Phone->GetHardware(&s, buffer);
	if (error != GE_NOTSUPPORTED) {
		if (error != GE_NOTIMPLEMENTED) Print_Error(error);
		if (error == GE_NONE) printf("%s %s\n",Msg(1028,"Hardware      :"),buffer);
	}

	error=Phone->GetIMEI(&s, buffer);
	if (error != GE_NOTSUPPORTED) {
		if (error != GE_NOTIMPLEMENTED) Print_Error(error);
		if (error == GE_NONE) printf("%s %s\n",Msg(1002,"IMEI          :"),buffer);

		error=Phone->GetOriginalIMEI(&s, buffer);
		if (error != GE_NOTSUPPORTED && error != GE_SECURITYERROR) {
			if (error != GE_NOTIMPLEMENTED) Print_Error(error);
			if (error == GE_NONE) printf("%s %s\n",Msg(1029,"Original IMEI :"),buffer);
		}
	}

	error=Phone->GetManufactureMonth(&s, buffer);
	if (error != GE_NOTSUPPORTED && error != GE_SECURITYERROR) {
		if (error != GE_NOTIMPLEMENTED) Print_Error(error);
		if (error == GE_NONE) printf("%s %s\n",Msg(1030,"Manufactured  :"),buffer);
	}

	error=Phone->GetProductCode(&s, buffer);
	if (error != GE_NOTSUPPORTED) {
		if (error != GE_NOTIMPLEMENTED) Print_Error(error);
		if (error == GE_NONE) printf("%s %s\n",Msg(1031,"Product code  :"),buffer);
	}

	error=Phone->GetSIMIMSI(&s, buffer);
	switch (error) {
		case GE_SECURITYERROR:
		case GE_NOTSUPPORTED:
		case GE_NOTIMPLEMENTED:
			break;
		case GE_NONE:
			printf("SIM IMSI      : %s\n",buffer);
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
		printf("%s\n",Msg(1005,"Date and time not set in phone"));
		break;
	case GE_NONE:
		printf("%s %s %4d/%02d/%02d\n", Msg(1003,"Date:"),
			DayOfWeek(date_time.Year, date_time.Month, date_time.Day),
			date_time.Year, date_time.Month, date_time.Day);
		printf("%s %02d:%02d:%02d\n", Msg(1004,"Time:"),
			date_time.Hour, date_time.Minute, date_time.Second);
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
		printf("%s\n",Msg(1006,"Alarm not set in phone"));
		break;
	case GE_NONE:
		printf("%s %02d:%02d\n",Msg(1004,"Time: "),
					alarm.Hour, alarm.Minute);
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
	if (!strcmp(argv[2],"DC")) entry.MemoryType=GMT_DC;
	if (!strcmp(argv[2],"ON")) entry.MemoryType=GMT_ON;
	if (!strcmp(argv[2],"RC")) entry.MemoryType=GMT_RC;
	if (!strcmp(argv[2],"MC")) entry.MemoryType=GMT_MC;
	if (!strcmp(argv[2],"ME")) entry.MemoryType=GMT_ME;
	if (!strcmp(argv[2],"SM")) entry.MemoryType=GMT_SM;
	if (!strcmp(argv[2],"VM")) entry.MemoryType=GMT_VM;
	if (entry.MemoryType==0) {
		printf("%s (\"%s\")\n",Msg(1019,"ERROR: unknown memory type"),argv[2]);
		exit (-1);
	}

	GetStartStop(&start, &stop, 3, argc, argv);

	for (i=0;i<5;i++) callerinit[i] = false;

	GSM_Init(true);

	if (!strcmp(GetModelData(NULL,s.Model,NULL)->model,"3310")) {
		if (s.VerNum<=4.06) printf("%s\n",Msg(1032,"WARNING: you will have null names in entries. Upgrade firmware in phone to higher than 4.06"));
	}

	for (j=start;j<=stop;j++) {

		printf("%s %s, %s %i\n",Msg(1033,"Memory"),argv[2],Msg(1034,"Location"),j);

		entry.Location=j;

		error=Phone->GetMemory(&s, &entry);
		Print_Error(error);

		if (entry.EntriesNum==0) {
			printf("%s\n",Msg(1007,"Entry is empty"));
		} else {
			for (i=0;i<entry.EntriesNum;i++) {
				unknown = false;
				switch (entry.Entries[i].EntryType) {
				case PBK_Date:
					printf("%s",Msg(1008,"Date and time   : "));
					printf("%s %4d/%02d/%02d", 
						DayOfWeek(entry.Entries[i].Date.Year, entry.Entries[i].Date.Month, entry.Entries[i].Date.Day),
						entry.Entries[i].Date.Year, entry.Entries[i].Date.Month, entry.Entries[i].Date.Day);
					printf(" %02d:%02d:%02d\n",
						entry.Entries[i].Date.Hour, entry.Entries[i].Date.Minute, entry.Entries[i].Date.Second);
					continue;
				case PBK_Number_General: printf("%s",Msg(1009,"General number ")); break;
				case PBK_Number_Mobile : printf("%s",Msg(1010,"Mobile number  ")); break;
				case PBK_Number_Work   : printf("%s",Msg(1011,"Work number    ")); break;
				case PBK_Number_Fax    : printf("%s",Msg(1012,"Fax number     ")); break;
				case PBK_Number_Home   : printf("%s",Msg(1013,"Home number    ")); break;
				case PBK_Text_Note     : printf("%s",Msg(1014,"Text           ")); break;
				case PBK_Text_Postal   : printf("%s",Msg(1015,"Snail address  ")); break;
				case PBK_Text_Email    : printf("%s",Msg(1016,"Email address  ")); break;
				case PBK_Text_URL      : printf("URL address    "); 		   break;
				case PBK_Name          : printf("%s",Msg(1017,"Name           ")); break;
				case PBK_Caller_Group  :
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
					printf("%s\"%s\"\n",Msg(1035,"Caller group    : "),DecodeUnicodeString(caller[entry.Entries[i].Number].Text));
					break;
				default		       :
					printf("%s\n",Msg(1018,"UNKNOWN"));
					unknown = true;
					break;
				}
				if (!unknown) printf(" : \"%s\"\n", DecodeUnicodeString(entry.Entries[i].Text));
			}
		}
		printf("\n");
	}
	

	GSM_Terminate();
}

static void displaysinglesmsinfo(GSM_SMSMessage sms, bool displaytext)
{
	switch (sms.PDU) {
	case SMS_Status_Report:
		printf("%s\n",Msg(1036,"SMS status report"));
		printf("%s",Msg(1037,"Status          : "));
		switch (sms.State) {
			case GSM_Sent	:	printf("Sent");		break;
			case GSM_Read	:	printf("Read");		break;
			case GSM_UnRead	:	printf("UnRead");	break;
			case GSM_UnSent	:	printf("UnSent");	break;
		}
		printf("\n%s\"%s\"\n",Msg(1040,"Remote number   : "),DecodeUnicodeString(sms.Number));
		printf("%s%s %4d/%02d/%02d ",
			Msg(1041,"Sent            : "),
			DayOfWeek(sms.DateTime.Year, sms.DateTime.Month, sms.DateTime.Day),
			sms.DateTime.Year, sms.DateTime.Month, sms.DateTime.Day);
		printf("%02d:%02d:%02d %04d\n", sms.DateTime.Hour, sms.DateTime.Minute, sms.DateTime.Second, sms.DateTime.Timezone);
		printf("SMSC number     : \"%s\"\n",DecodeUnicodeString(sms.SMSC.Number));
		printf("SMSC response   : %s %4d/%02d/%02d ",
			DayOfWeek(sms.SMSCTime.Year, sms.SMSCTime.Month, sms.SMSCTime.Day),
			sms.SMSCTime.Year, sms.SMSCTime.Month, sms.SMSCTime.Day);
		printf("%02d:%02d:%02d %04d\n", sms.SMSCTime.Hour, sms.SMSCTime.Minute, sms.SMSCTime.Second, sms.SMSCTime.Timezone);
		printf("Delivery status : %s\n",DecodeUnicodeString(sms.Text));
		printf("Details         : ");
		if (sms.DeliveryStatus & 0x40) {
			if (sms.DeliveryStatus & 0x20) {
				printf("Temporary error, ");
			} else {
	     			printf("Permanent error, ");
			}
	    	} else if (sms.DeliveryStatus & 0x20) {
			printf("Temporary error, ");
		}
		switch (sms.DeliveryStatus) {
			case 0x00: printf("SM received by the SME");				break;
			case 0x01: printf("SM forwarded by the SC to the SME but the SC is unable to confirm delivery");break;
			case 0x02: printf("SM replaced by the SC");				break;
			case 0x20: printf("Congestion");					break;
			case 0x21: printf("SME busy");						break;
			case 0x22: printf("No response from SME");				break;
			case 0x23: printf("Service rejected");					break;
			case 0x24: printf("Quality of service not aviable");			break;
			case 0x25: printf("Error in SME");					break;
		        case 0x40: printf("Remote procedure error");				break;
		        case 0x41: printf("Incompatibile destination");				break;
		        case 0x42: printf("Connection rejected by SME");			break;
		        case 0x43: printf("Not obtainable");					break;
		        case 0x44: printf("Quality of service not available");			break;
		        case 0x45: printf("No internetworking available");			break;
		        case 0x46: printf("SM Validity Period Expired");			break;
		        case 0x47: printf("SM deleted by originating SME");			break;
		        case 0x48: printf("SM Deleted by SC Administration");			break;
		        case 0x49: printf("SM does not exist");					break;
		        case 0x60: printf("Congestion");					break;
		        case 0x61: printf("SME busy");						break;
		        case 0x62: printf("No response from SME");				break;
		        case 0x63: printf("Service rejected");					break;
		        case 0x64: printf("Quality of service not available");			break;
		        case 0x65: printf("Error in SME");					break;
		        default  : printf("Reserved/Specific to SC: %x",sms.DeliveryStatus);	break;
		}
		printf("\n");
		break;
	case SMS_Deliver:
		printf("SMS message\n");
		printf("SMSC number      : \"%s\"",DecodeUnicodeString(sms.SMSC.Number));
		if (sms.ReplyViaSameSMSC) printf(" (set for reply)");
		printf("\nSent             : %s %4d/%02d/%02d ",
			DayOfWeek(sms.DateTime.Year, sms.DateTime.Month, sms.DateTime.Day),
			sms.DateTime.Year, sms.DateTime.Month, sms.DateTime.Day);
		printf("%02d:%02d:%02d %04d\n", sms.DateTime.Hour, sms.DateTime.Minute, sms.DateTime.Second, sms.DateTime.Timezone);
		/* No break. The only difference for SMS_Deliver and SMS_Submit is,
		 * that SMS_Deliver contains additional data. We wrote them and then go
		 * for data shared with SMS_Submit
		 */
	case SMS_Submit:
		if (sms.ReplaceMessage != 0) printf("SMS replacing ID : %i\n",sms.ReplaceMessage);
		/* If we went here from "case SMS_Deliver", we don't write "SMS Message" */
		if (sms.PDU==SMS_Submit) printf("SMS message\n");
		if (sms.Name[0] != 0x00 || sms.Name[1] != 0x00) {
			printf("Name             : \"%s\"\n",DecodeUnicodeString(sms.Name));
		}
		if (sms.Class != -1) {
			printf("Class            : %i\n",sms.Class);
		}
		printf("Coding           : ");
		switch (sms.Coding) {
			case GSM_Coding_Unicode : printf("Unicode\n");			break;
			case GSM_Coding_Default : printf("Default GSM alphabet\n");	break;
			case GSM_Coding_8bit	: printf("8 bit\n");			break;
		}
		printf("%s",Msg(1037,"Status           : "));
		switch (sms.State) {
			case GSM_Sent	:	printf("Sent");		break;
			case GSM_Read	:	printf("Read");		break;
			case GSM_UnRead	:	printf("UnRead");	break;
			case GSM_UnSent	:	printf("UnSent");	break;
		}
		printf("\n%s\"%s\"\n",Msg(1040,"Remote number    : "),DecodeUnicodeString(sms.Number));
		if (sms.UDH.Type != UDH_NoUDH) printf("User Data Header : ");
		switch (sms.UDH.Type) {
		case UDH_ConcatenatedMessages:	printf("Concatenated (linked) message"); break;
		case UDH_DisableVoice:		printf("Disables voice indicator");	 break;
		case UDH_EnableVoice:		printf("Enables voice indicator");	 break;
		case UDH_DisableFax:		printf("Disables fax indicator");	 break;
		case UDH_EnableFax:		printf("Enables fax indicator");	 break;
		case UDH_DisableEmail:		printf("Disables email indicator");	 break;
		case UDH_EnableEmail:		printf("Enables email indicator");	 break;
		case UDH_VoidSMS:		printf("Void SMS");			 break;
		case UDH_NokiaWAPBookmark:	printf("Nokia WAP Bookmark");		 break;
		case UDH_NokiaOperatorLogoLong:	printf("Nokia operator logo");		 break;
		case UDH_NokiaWAPBookmarkLong:	printf("Nokia WAP Bookmark");		 break;
		case UDH_NokiaWAPSettingsLong:	printf("Nokia WAP Settings");		 break;
		case UDH_NokiaRingtone:		printf("Nokia ringtone");		 break;
		case UDH_NokiaOperatorLogo:	printf("Nokia GSM operator logo");	 break;
		case UDH_NokiaCallerLogo:	printf("Nokia caller logo");		 break;  	
		case UDH_NokiaProfileLong:	printf("Nokia profile");		 break;
		case UDH_NokiaCalendarLong:	printf("Nokia calendar note");		 break;
		case UDH_NokiaPhonebookLong:	printf("Nokia phonebook entry");	 break;
		case UDH_UserUDH:		printf("User UDH");			 break;
		case UDH_NoUDH:								 break;
		}
		if (sms.UDH.Type != UDH_NoUDH) {
			if (sms.UDH.ID != -1) printf(", ID %i",sms.UDH.ID);
			if (sms.UDH.PartNumber != -1 && sms.UDH.AllParts != -1) {
				printf(", part %i of %i",sms.UDH.PartNumber,sms.UDH.AllParts);
			}
		}
		if (displaytext) {
			if (sms.Coding!=GSM_Coding_8bit) {
				printf("%s\n",DecodeUnicodeString(sms.Text));
			} else {
				printf("8 bit SMS, cannot be displayed here\n");
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

	SMSInfo.Bitmap 		= &Bitmap;
	SMSInfo.Ringtone 	= &Ringtone;
	SMSInfo.Buffer		= Buffer;

	/* GSM_DecodeMultiPartSMS returns if decoded SMS contenst correctly */
	RetVal = GSM_DecodeMultiPartSMS(&SMSInfo,&sms);

	for (j=0;j<sms.Number;j++) {
		displaysinglesmsinfo(sms.SMS[j],!RetVal);
		printf("\n");
	}
	if (RetVal) {
		if (SMSInfo.Bitmap != NULL) {
			switch (SMSInfo.Bitmap->Bitmap[0].Type) {
			case GSM_CallerLogo:
				printf("Caller logo\n");
				break;
			case GSM_OperatorLogo:
				printf("Operator logo ");
				printf("for %s network (%s",
					SMSInfo.Bitmap->Bitmap[0].NetworkCode,
					DecodeUnicodeString(GSM_GetNetworkName(SMSInfo.Bitmap->Bitmap[0].NetworkCode)));
				printf(", %s)\n",
					DecodeUnicodeString(GSM_GetCountryName(SMSInfo.Bitmap->Bitmap[0].NetworkCode)));
				break;
			case GSM_PictureImage:
				printf("Picture Image\n");
				printf("Text: \"%s\"\n",DecodeUnicodeString(SMSInfo.Bitmap->Bitmap[0].Text));
				break;
			default:
				break;
			}
			GSM_PrintBitmap(stdout,&SMSInfo.Bitmap->Bitmap[0]);
			printf("\n");
		}
		if (SMSInfo.Buffer != NULL) {
			printf("Text\n%s\n",DecodeUnicodeString(Buffer));
		}
		if (SMSInfo.Ringtone != NULL) {
			printf("Ringtone \"%s\"\n",DecodeUnicodeString(SMSInfo.Ringtone->Name));
			saverttl(stdout,SMSInfo.Ringtone);
			printf("\n");
			if (s.Phone.Functions->PlayTone!=NOTSUPPORTED &&
			    s.Phone.Functions->PlayTone!=NOTIMPLEMENTED) {
				if (answer_yes("Do you want to play it")) GSM_PlayRingtone(*SMSInfo.Ringtone);
			}
		}
	}
}

static void IncomingSMS(char *Device, GSM_SMSMessage sms)
{
	GSM_MultiSMSMessage SMS;

	printf("SMS message received\n");
	SMS.Number = 1;
	memcpy(&SMS.SMS[0],&sms,sizeof(GSM_SMSMessage));	
	displaymultismsinfo(SMS);
}

static void IncomingCB(char *Device, GSM_CBMessage CB)
{
	printf("CB message received\n");
	printf("Channel %i, text \"%s\"\n",CB.Channel,DecodeUnicodeString(CB.Text));
}

#define CHECKMEMORYSTATUS(x, m, a, a1, b, b1) { 					\
	x.MemoryType=m;									\
	if (Phone->GetMemoryStatus(&s, &x) == GE_NONE)					\
		printf("%s %03d, %s %03d\n", Msg(a,a1), x.Used, Msg(b,b1), x.Free);	\
}

static void Monitor(int argc, char *argv[])
{
	GSM_MemoryStatus	MemStatus;
	GSM_SMSMemoryStatus	SMSStatus;
	GSM_NetworkInfo		NetInfo;

	/* We do not want to monitor serial line forever -
	 * press Ctrl+C to stop the monitoring mode.
	 */
	signal(SIGINT, interrupted);
	printf("If you want break, press Ctrl+C...\n");
	printf("Entering monitor mode...\n");

	GSM_Init(true);

	s.User.IncomingSMS = IncomingSMS;
	Phone->SetIncomingSMS(&s,true);
	s.User.IncomingCB = IncomingCB;
	Phone->SetIncomingCB(&s,true);

	/* Loop here indefinitely -
	 * allows you to see messages from GSM code in
	 * response to unknown messages etc.
	 * The loops ends after pressing the Ctrl+C.
	 */
	while (!bshutdown) {
		CHECKMEMORYSTATUS(MemStatus,GMT_SM,1021,"SIM phonebook     : Used",1022,"Free");
		CHECKMEMORYSTATUS(MemStatus,GMT_ME,1023,"Phone phonebook   : Used",1022,"Free");
		CHECKMEMORYSTATUS(MemStatus,GMT_DC,1024,"Dialled numbers   : Used",1022,"Free");
		CHECKMEMORYSTATUS(MemStatus,GMT_RC,1025,"Received numbers  : Used",1022,"Free");
		CHECKMEMORYSTATUS(MemStatus,GMT_MC,1026,"Missed numbers    : Used",1022,"Free");
		CHECKMEMORYSTATUS(MemStatus,GMT_ON,1027,"Own numbers       : Used",1022,"Free");
		if (Phone->GetBatteryLevel(&s,&i)==GE_NONE) printf("Battery level     : %i percent\n",i);
		if (Phone->GetNetworkLevel(&s,&i)==GE_NONE) printf("Network level     : %i percent\n",i);
		if (Phone->GetSMSStatus(&s,&SMSStatus)==GE_NONE) {
			printf("SIM SMS status    : %i used, %i unread",
				SMSStatus.SIMUsed,SMSStatus.SIMUnRead);
			if (SMSStatus.SIMSize!=0) printf(", %i locations", SMSStatus.SIMSize);
			printf("\n");
			printf("Phone SMS status  : %i used, %i unread",
				SMSStatus.PhoneUsed,SMSStatus.PhoneUnRead);
			if (SMSStatus.PhoneSize!=0) printf(", %i locations", SMSStatus.PhoneSize);
			if (SMSStatus.TemplatesUsed!=0) printf(", %i templates", SMSStatus.TemplatesUsed);
			printf("\n");
		}
		if (Phone->GetNetworkInfo(&s,&NetInfo)==GE_NONE) {
			printf("Network state     : ");
                        switch (NetInfo.State) {
				case GSM_HomeNetwork		: printf("home network\n"); 		break;
				case GSM_RoamingNetwork		: printf("roaming network\n"); 		break;
				case GSM_RequestingNetwork	: printf("requesting network\n"); 	break;
				case GSM_NoNetwork		: printf("not logged into network\n"); 	break;
				default				: printf("unknown\n");
			}
			if (NetInfo.State == GSM_HomeNetwork || NetInfo.State == GSM_RoamingNetwork) {
				printf("Network           : %s (%s",	NetInfo.NetworkCode,DecodeUnicodeString(GSM_GetNetworkName(NetInfo.NetworkCode)));
				printf(", %s)",				DecodeUnicodeString(GSM_GetCountryName(NetInfo.NetworkCode)));
				printf(", LAC %s, CellID %s\n",		NetInfo.LAC,NetInfo.CellID);
				if (NetInfo.NetworkName[0] != 0x00 || NetInfo.NetworkName[1] != 0x00) {
					printf("Name in phone     : \"%s\"\n",DecodeUnicodeString(NetInfo.NetworkName));
				}
			}
		}
		printf("\n");
	}

	printf("Leaving monitor mode...\n");

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
			printf("%i. Set %i\n",smsc.Location, smsc.Location);
		} else {
			printf("%i. \"%s\"\n",smsc.Location, DecodeUnicodeString(smsc.Name));
		}
		printf("Number         : \"%s\"\n",DecodeUnicodeString(smsc.Number));
		printf("Default number : \"%s\"\n",DecodeUnicodeString(smsc.DefaultNumber));
	
		printf("Format         : ");
		switch (smsc.Format) {
			case GSMF_Text	: printf("Text");	break;
			case GSMF_Fax	: printf("Fax");	break;
			case GSMF_Email	: printf("Email");	break;
			case GSMF_Pager	: printf("Pager");	break;
		}
		printf("\n");

		printf("Validity       : ");
		switch (smsc.Validity.Relative) {
			case GSMV_1_Hour	: printf("1 hour");	  break;
			case GSMV_6_Hours 	: printf("6 hours");	  break;
			case GSMV_24_Hours	: printf("24 hours");	  break;
			case GSMV_72_Hours	: printf("72 hours");	  break;
			case GSMV_1_Week  	: printf("1 week"); 	  break;
			case GSMV_Max_Time	: printf("Maximum time"); break;
			default           	: printf("Unknown");
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
			printf("Location %i\n",sms.SMS[0].Location);
			printf("Empty\n");
			break;
		default:
			Print_Error(error);
			printf("Location %i, folder \"%s\"\n",sms.SMS[0].Location,DecodeUnicodeString(folders.Folder[sms.SMS[0].Folder-1].Name));
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
		sms.Location=i;
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
			printf("Location %i, folder \"%s\"\n",sms.SMS[0].Location,DecodeUnicodeString(folders.Folder[sms.SMS[0].Folder-1].Name));
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
			    printf("Location %i, folder \"%s\"\n",SortedSMS[i]->SMS[j].Location,DecodeUnicodeString(folders.Folder[SortedSMS[i]->SMS[j].Folder-1].Name));
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
		printf("%i. %s\n",i+1,DecodeUnicodeString(folders.Folder[i].Name));
	}

	GSM_Terminate();
}

static void GetRingtone(int argc, char *argv[])
{
	GSM_Ringtone 	ringtone;
	bool		PhoneRingtone = false;

	if (!strcmp(argv[1],"--getphoneringtone")) PhoneRingtone = true;

	GetStartStop(&ringtone.Location, NULL, 2, argc, argv);

	GSM_Init(true);

	ringtone.Format=0;

	error=Phone->GetRingtone(&s,&ringtone,PhoneRingtone);
	Print_Error(error);

	switch (ringtone.Format) {
		case RING_NOTETONE	: printf("Smart Messaging");	break;
		case RING_NOKIABINARY	: printf("Nokia binary");	break;
	}
	printf(" format, ringtone \"%s\"\n",DecodeUnicodeString(ringtone.Name));

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

	for (i=0;i<Info.Number;i++) printf("%i. \"%s\"\n",i,DecodeUnicodeString(Info.Ringtone[i].Name));
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

	if (!strcmp(argv[2],"SOFT")) {
		hard=false;
	} else if (!strcmp(argv[2],"HARD")) {
		hard=true;
	} else {
		printf("What type of reset do you want (\"%s\") ?\n",argv[2]);
		exit(-1);
	}

	GSM_Init(true);

	error=Phone->Reset(&s, hard);
	Print_Error(error);

	GSM_Terminate();
}

static void GetCalendarNote(int argc, char *argv[])
{
	GSM_CalendarNote	Note;
	int			start,stop;
	bool			refresh=true;
	int			i_age;
	GSM_DateTime		DateTime;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		Note.Location=i;
		error=Phone->GetCalendarNote(&s,&Note,refresh);
		Print_Error(error);
		printf("Note type  : ");
		switch (Note.Type) {
			case GCN_REMINDER : printf("Reminder\n");		break;
			case GCN_CALL     : printf("Call\n"	);		break;
			case GCN_MEETING  : printf("Meeting\n"	);		break;
			case GCN_BIRTHDAY : printf("Birthday\n");		break;
			case GCN_T_ATHL   : printf("Training/Athletism\n"); 	break;
        		case GCN_T_BALL   : printf("Training/Ball Games\n"); 	break;
	                case GCN_T_CYCL   : printf("Training/Cycling\n"); 	break;
	                case GCN_T_BUDO   : printf("Training/Budo\n"); 		break;
	                case GCN_T_DANC   : printf("Training/Dance\n"); 	break;
	                case GCN_T_EXTR   : printf("Training/Extreme Sports\n");break;
	                case GCN_T_FOOT   : printf("Training/Football\n"); 	break;
	                case GCN_T_GOLF   : printf("Training/Golf\n"); 		break;
	                case GCN_T_GYM    : printf("Training/Gym\n"); 		break;
	                case GCN_T_HORS   : printf("Training/Horse Races\n"); 	break;
	                case GCN_T_HOCK   : printf("Training/Hockey\n"); 	break;
	                case GCN_T_RACE   : printf("Training/Races\n"); 	break;
	                case GCN_T_RUGB   : printf("Training/Rugby\n"); 	break;
	                case GCN_T_SAIL   : printf("Training/Sailing\n"); 	break;
	                case GCN_T_STRE   : printf("Training/Street Games\n"); 	break;
	                case GCN_T_SWIM   : printf("Training/Swimming\n"); 	break;
	                case GCN_T_TENN   : printf("Training/Tennis\n"); 	break;
	                case GCN_T_TRAV   : printf("Training/Travels\n"); 	break;
	                case GCN_T_WINT   : printf("Training/Winter Games\n"); 	break;
			default           : printf("UNKNOWN\n"	);
		}
		printf("Date       : %s %4d/%02d/%02d\n",
			DayOfWeek(Note.Time.Year, Note.Time.Month, Note.Time.Day),
			Note.Time.Year, Note.Time.Month, Note.Time.Day);
		printf("Time       : %02d:%02d:%02d\n",
			Note.Time.Hour, Note.Time.Minute, Note.Time.Second);
		if (Note.Alarm.Year!=0) {
			printf("Alarm date : %s %4d/%02d/%02d\n",
				DayOfWeek(Note.Alarm.Year, Note.Alarm.Month, Note.Alarm.Day),
				Note.Alarm.Year, Note.Alarm.Month, Note.Alarm.Day);
			printf("Alarm time : %02d:%02d:%02d\n",
				Note.Alarm.Hour, Note.Alarm.Minute, Note.Alarm.Second);
			printf("Alarm type : ");
			if (Note.SilentAlarm) printf("silent\n");
					 else printf("with tone\n");
		}
		if (Note.Recurrance!=0) {
			printf("Repeat     : %d day%s\n",Note.Recurrance/24,
				((Note.Recurrance/24)>1) ? "s":"" );
		}
		if (Note.Type == GCN_BIRTHDAY) {
			if (Note.Alarm.Year == 0x00) {
				GSM_GetCurrentDateTime (&DateTime);
				memcpy(&Note.Alarm,&DateTime,sizeof(GSM_DateTime));
			}
			i_age = Note.Alarm.Year - Note.Time.Year;
			if (Note.Time.Month < Note.Alarm.Month) {
				i_age++;
			}
			if (Note.Time.Month == Note.Alarm.Month &&
			    Note.Time.Day < Note.Alarm.Day) {
				i_age++;
			}

			printf("Text       : \"%s\" (%d %s)\n",
				DecodeUnicodeString(Note.Text),
				i_age, (i_age==1)?"year":"years");
		} else {
			printf("Text       : \"%s\"\n",DecodeUnicodeString(Note.Text));
		}
		if (Note.Type == GCN_CALL) {
			printf("Phone      : \"%s\"\n",DecodeUnicodeString(Note.Phone));
		}
		printf("\n");
		refresh=false;
	}

	GSM_Terminate();
}

static void DeleteCalendarNote(int argc, char *argv[])
{
	GSM_CalendarNote	Note;
	int			start,stop;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		Note.Location=i;
		error=Phone->DeleteCalendarNote(&s,&Note);
		Print_Error(error);
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
		printf("Name    : \"%s\"\n",DecodeUnicodeString(bookmark.Title));
		printf("Address : \"%s\"\n",DecodeUnicodeString(bookmark.Address));
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

	if (!strcmp(argv[2],"STARTUP")) {
		MultiBitmap.Bitmap[0].Type=GSM_StartupLogo;
	} else if (!strcmp(argv[2],"CALLER")) {
		MultiBitmap.Bitmap[0].Type=GSM_CallerLogo;
		GetStartStop(&location, NULL, 3, argc, argv);
		if (location>5) {
			printf("Maximal location for caller logo can be 5\n");
			exit (-1);
		}
	} else if (!strcmp(argv[2],"PICTURE")) {
		MultiBitmap.Bitmap[0].Type=GSM_PictureImage;
		GetStartStop(&location, NULL, 3, argc, argv);
	} else if (!strcmp(argv[2],"TEXT")) {
		MultiBitmap.Bitmap[0].Type=GSM_WelcomeNoteText;
	} else if (!strcmp(argv[2],"DEALER")) {
		MultiBitmap.Bitmap[0].Type=GSM_DealerNoteText;
	} else if (!strcmp(argv[2],"OPERATOR")) {
		MultiBitmap.Bitmap[0].Type=GSM_OperatorLogo;
	} else {
		printf("What type of logo do you want to get (\"%s\") ?\n",argv[2]);
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
		printf("Group name  : \"%s\"",DecodeUnicodeString(MultiBitmap.Bitmap[0].Text));
		if (MultiBitmap.Bitmap[0].DefaultName) printf(" (default)");
		printf("\n");
		if (MultiBitmap.Bitmap[0].Ringtone == 0xff) {
			printf("Ringtone    : default\n");
		} else {
			error=Phone->GetRingtonesInfo(&s,&Info);
			if (error != GE_NONE) Info.Number = 0;

			printf("Ringtone    : ");
			if (strlen(DecodeUnicodeString(GSM_GetRingtoneName(&Info,MultiBitmap.Bitmap[0].Ringtone)))!=0) {
				printf("\"%s\"\n",DecodeUnicodeString(GSM_GetRingtoneName(&Info,MultiBitmap.Bitmap[0].Ringtone)));
			} else {
				printf("%i\n",MultiBitmap.Bitmap[0].Ringtone);
			}
		}
		if (MultiBitmap.Bitmap[0].Enabled) {
			printf("Bitmap      : enabled\n");
		} else {
			printf("Bitmap      : disabled\n");
		}
		if (argc>4) error=GSM_SaveBitmapFile(argv[4],&MultiBitmap);
		break;
	case GSM_StartupLogo:
		GSM_PrintBitmap(stdout,&MultiBitmap.Bitmap[0]);
		if (argc>3) error=GSM_SaveBitmapFile(argv[3],&MultiBitmap);
		break;
	case GSM_OperatorLogo:
		if (strcmp(MultiBitmap.Bitmap[0].NetworkCode,"000 00")) {
			GSM_PrintBitmap(stdout,&MultiBitmap.Bitmap[0]);
			if (argc>3) error=GSM_SaveBitmapFile(argv[3],&MultiBitmap);
		} else {
			printf("No operator logo in phone\n");
		}
		break;
	case GSM_PictureImage:
		GSM_PrintBitmap(stdout,&MultiBitmap.Bitmap[0]);
		printf("Text   : \"%s\"\n",DecodeUnicodeString(MultiBitmap.Bitmap[0].Text));
		printf("Sender : \"%s\"\n",DecodeUnicodeString(MultiBitmap.Bitmap[0].Sender));
		if (argc>4) error=GSM_SaveBitmapFile(argv[4],&MultiBitmap);
		break;
	case GSM_WelcomeNoteText:
		printf("Welcome note text is \"%s\"\n",DecodeUnicodeString(MultiBitmap.Bitmap[0].Text));
		break;
	case GSM_DealerNoteText:
		printf("Dealer note text is \"%s\"\n",DecodeUnicodeString(MultiBitmap.Bitmap[0].Text));
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

	if (!strcmp(argv[2],"STARTUP")) {
		if (argc<4) {
			printf("More arguments required\n");
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
	} else if (!strcmp(argv[2],"TEXT")) {
		if (argc<4) {
			printf("More arguments required\n");
			exit(-1);
		}
		Bitmap.Type=GSM_WelcomeNoteText;
		EncodeUnicode(Bitmap.Text,argv[3],strlen(argv[3]));
	} else if (!strcmp(argv[2],"DEALER")) {
		if (argc<4) {
			printf("More arguments required\n");
			exit(-1);
		}
		Bitmap.Type=GSM_DealerNoteText;
		EncodeUnicode(Bitmap.Text,argv[3],strlen(argv[3]));
	} else if (!strcmp(argv[2],"CALLER")) {
		if (argc<4) {
			printf("More arguments required\n");
			exit(-1);
		}
		GetStartStop(&i, NULL, 3, argc, argv);
		if (i>5 && i!=255) {
			printf("Maximal location for caller logo can be 5\n");
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
	} else if (!strcmp(argv[2],"PICTURE")) {
		if (argc<5) {
			printf("More arguments required\n");
			exit(-1);
		}
		MultiBitmap.Bitmap[0].Type		= GSM_PictureImage;
		MultiBitmap.Bitmap[0].Location		= atoi(argv[4]);
		error=GSM_ReadBitmapFile(argv[3],&MultiBitmap);
		Print_Error(error);
		memcpy(&Bitmap,&MultiBitmap.Bitmap[0],sizeof(GSM_Bitmap));
		Bitmap.Text[0]=0;
		Bitmap.Text[1]=0;
		Bitmap.Sender[0]=0;
		Bitmap.Sender[1]=0;
	} else if (!strcmp(argv[2],"OPERATOR")) {
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
		printf("What type of logo do you want to set (\"%s\") ?\n",argv[2]);
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
		if (!strcmp(argv[4],"-scale")) {
			ringtone.NoteTone.AllNotesScale = true;
		} else {
			printf("Unknown parameter \"%s\"",argv[4]);
			exit(-1);
		}
	}
	if (ringtone.Location==0) {
		printf("%s",Msg(1020,"ERROR: enumerate locations from 1"));
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

#define SEND_SAVE_SMS_BUFFER_SIZE 500

static GSM_Error SMSStatus;

static void SendSMSStatus (char *Device, int status)
{
	dprintf("Incoming SMS device: \"%s\"\n",Device);
	if (status==0) {
		printf("..OK\n");
		SMSStatus = GE_NONE;
	} else {
		printf("..error %i\n",status);
		SMSStatus = GE_UNKNOWN;
	}
}

static void SendSaveSMS(int argc, char *argv[])
{
	GSM_SMSFolders			folders;
	GSM_MultiSMSMessage		sms;
	GSM_Ringtone			ringtone;
	GSM_MultiBitmap			bitmap, bitmap2;
	GSM_EncodeMultiPartSMSInfo	SMSInfo;
	GSM_NetworkInfo			NetInfo;
#ifdef GSM_ENABLE_BACKUP
	GSM_Backup			Backup;
#endif
	char				ReplaceBuffer2	[200],ReplaceBuffer[200];
	FILE 				*ReplaceFile;	
	int				startarg		= 0;
	int				chars_read		= 0;
	char				InputBuffer	[SEND_SAVE_SMS_BUFFER_SIZE/2+1];
	char				Buffer		[SEND_SAVE_SMS_BUFFER_SIZE];
	char				Sender		[GSM_MAX_NUMBER_LENGTH];
	char				Name		[GSM_MAX_NUMBER_LENGTH];
	char				SMSC		[GSM_MAX_NUMBER_LENGTH];
	bool 				nextlong		= 0;
	int				i,j,z;
	bool				ReplyViaSameSMSC 	= false;
	int				SMSCSet			= 1;
	/* Parameters required only during saving */
	int				Folder			= 1;	/*Inbox by default */
	GSM_SMS_State			State			= GSM_Sent;
	/* Required only during sending */
	bool				DeliveryReport		= false;

	ReplaceBuffer[0] = 0;
	ReplaceBuffer[1] = 0;
	GSM_ClearMultiPartSMSInfo(&SMSInfo);
	SMSInfo.ReplaceMessage = 0;

	if (!strcmp(argv[1],"--savesms")) {
		EncodeUnicode(Sender,"Gammu",5);
		Name[0] = 0;
		Name[1] = 0;
		startarg = 0;
	} else {
		EncodeUnicode(Sender,argv[3],strlen(argv[3]));
		startarg = 1;
	}

	if (!strcmp(argv[2],"TEXT")) {
		chars_read = fread(InputBuffer, 1, SEND_SAVE_SMS_BUFFER_SIZE/2, stdin);
		if (chars_read == 0) {
			printf("Couldn't read from stdin!\n");	
			exit (-1);
		}  
		InputBuffer[chars_read] 	= 0x00;	
		InputBuffer[chars_read+1] 	= 0x00;	
		EncodeUnicode(Buffer,InputBuffer,strlen(InputBuffer));
		SMSInfo.Buffer  	= Buffer;
		SMSInfo.ID		= SMS_Text;
		SMSInfo.UnicodeCoding   = false;
		SMSInfo.Class		= -1;
		startarg += 3;
	} else if (!strcmp(argv[2],"EMSSOUND")) {
		SMSInfo.ID		= SMS_EMSPredefinedSound;
		SMSInfo.Number		= 0;
		startarg += 3;
	} else if (!strcmp(argv[2],"EMSBITMAP")) {
		if (argc<4+startarg) {
			printf("Where is logo filename ?\n");
			exit(-1);
		}
		bitmap.Bitmap[0].Type=GSM_StartupLogo;
		error=GSM_ReadBitmapFile(argv[3+startarg],&bitmap);
		Print_Error(error);
		SMSInfo.Bitmap   = &bitmap;
		SMSInfo.ID 	 = SMS_EMSBitmap;
		startarg += 4;
	} else if (!strcmp(argv[2],"EMSANIMATION")) {
		SMSInfo.ID		= SMS_EMSPredefinedAnimation;
		SMSInfo.Number		= 0;
		bitmap.Number		= 0;
		startarg += 3;
	} else if (!strcmp(argv[2],"RINGTONE")) {
		if (argc<4+startarg) {
			printf("Where is ringtone filename ?\n");
			exit(-1);
		}
		ringtone.Format=RING_NOTETONE;
		error=GSM_ReadRingtoneFile(argv[3+startarg],&ringtone);
		Print_Error(error);
		SMSInfo.ID 	 = SMS_NokiaRingtone;
		SMSInfo.Ringtone = &ringtone;
		if (!strcmp(argv[1],"--savesms")) {
			CopyUnicodeString(Sender, ringtone.Name);
			EncodeUnicode(Name,"Ringtone ",9);
			CopyUnicodeString(Name+9*2, ringtone.Name);
		}
		startarg += 4;
	} else if (!strcmp(argv[2],"OPERATOR")) {
		if (argc<4+startarg) {
			printf("Where is logo filename ?\n");
			exit(-1);
		}
		bitmap.Bitmap[0].Type=GSM_OperatorLogo;
		error=GSM_ReadBitmapFile(argv[3+startarg],&bitmap);
		Print_Error(error);
		strcpy(bitmap.Bitmap[0].NetworkCode,"000 00");
		SMSInfo.ID 	 = SMS_NokiaOperatorLogo;
		SMSInfo.Bitmap   = &bitmap;
		if (!strcmp(argv[1],"--savesms")) {
			EncodeUnicode(Sender, "OpLogo",6);
			EncodeUnicode(Name,"OpLogo ",7);
		}
		startarg += 4;
	} else if (!strcmp(argv[2],"CALLER")) {
		if (argc<4+startarg) {
			printf("Where is logo filename ?\n");
			exit(-1);
		}
		bitmap.Bitmap[0].Type=GSM_CallerLogo;
		error=GSM_ReadBitmapFile(argv[3+startarg],&bitmap);
		Print_Error(error);
		SMSInfo.ID 	 = SMS_NokiaCallerLogo;
		SMSInfo.Bitmap   = &bitmap;
		if (!strcmp(argv[1],"--savesms")) {
			EncodeUnicode(Sender, "Caller",6);
		}
		startarg += 4;
	} else if (!strcmp(argv[2],"PICTURE")) {
		if (argc<4+startarg) {
			printf("Where is logo filename ?\n");
			exit(-1);
		}
		bitmap.Bitmap[0].Type=GSM_PictureImage;
		error=GSM_ReadBitmapFile(argv[3+startarg],&bitmap);
		Print_Error(error);
		SMSInfo.ID 	 	= SMS_NokiaPictureImageLong;
		SMSInfo.Bitmap   	= &bitmap;
		SMSInfo.UnicodeCoding 	= false;
		bitmap.Bitmap[0].Text[0]=0;
		bitmap.Bitmap[0].Text[1]=0;
		if (!strcmp(argv[1],"--savesms")) {
			EncodeUnicode(Sender, "Picture",7);
			EncodeUnicode(Name,"Picture Image",13);
		}
		startarg += 4;
#ifdef GSM_ENABLE_BACKUP
	} else if (!strcmp(argv[2],"BOOKMARK")) {
		if (argc<5+startarg) {
			printf("Where is backup filename and location ?\n");
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
			printf("Bookmark not found in file\n");
			exit(-1);
		}
		SMSInfo.ID 	 = SMS_NokiaWAPBookmarkLong;
		SMSInfo.Bookmark = Backup.WAPBookmark[i];		
		if (!strcmp(argv[1],"--savesms")) {
			EncodeUnicode(Sender, "Bookmark",8);
			EncodeUnicode(Name,"WAP Bookmark",12);
		}
		startarg += 5;
	} else if (!strcmp(argv[2],"SETTINGS")) {
		if (argc<5+startarg) {
			printf("Where is backup filename and location ?\n");
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
			printf("WAP settings not found in file\n");
			exit(-1);
		}
		if (Backup.WAPSettings[i]->Settings[0].Bearer != WAPSETTINGS_BEARER_DATA &&
		    Backup.WAPSettings[i]->Settings[0].Bearer != WAPSETTINGS_BEARER_SMS) {
			printf("Sorry. This bearer in WAP settings not supported now\n");
			exit(-1);
		}
		SMSInfo.ID 	 = SMS_NokiaWAPSettingsLong;
		SMSInfo.Settings = &Backup.WAPSettings[i]->Settings[0];		
		if (!strcmp(argv[1],"--savesms")) {
			EncodeUnicode(Sender, "Settings",8);
			EncodeUnicode(Name,"WAP Settings",12);
		}
		startarg += 5;
	} else if (!strcmp(argv[2],"CALENDAR")) {
		if (argc<5+startarg) {
			printf("Where is backup filename and location ?\n");
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
			printf("Calendar note not found in file\n");
			exit(-1);
		}
		SMSInfo.ID 	 = SMS_NokiaVCALENDAR10Long;
		SMSInfo.Calendar = Backup.Calendar[i];		
		if (!strcmp(argv[1],"--savesms")) {
			EncodeUnicode(Sender, "Calendar",8);
		}
		startarg += 5;
	} else if (!strcmp(argv[2],"VCARD10") || !strcmp(argv[2],"VCARD21")) {
		if (argc<6+startarg) {
			printf("Where is backup filename and location and memory type ?\n");
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[3+startarg],&Backup);
		Print_Error(error);
		i = 0;
		if (!strcmp(argv[4+startarg],"SM")) {
			while (Backup.SIMPhonebook[i]!=NULL) {
				if (i == atoi(argv[5+startarg])-1) break;
				i++;
			}
			if (i != atoi(argv[5+startarg])-1) {
				printf("Phonebook entry not found in file\n");
				exit(-1);
			}
			SMSInfo.Phonebook = Backup.SIMPhonebook[i];
		} else if (!strcmp(argv[4+startarg],"ME")) {
			while (Backup.PhonePhonebook[i]!=NULL) {
				if (i == atoi(argv[5+startarg])-1) break;
				i++;
			}
			if (i != atoi(argv[5+startarg])-1) {
				printf("Phonebook entry not found in file\n");
				exit(-1);
			}
			SMSInfo.Phonebook = Backup.PhonePhonebook[i];
		} else {
			printf("Unknown memory type: \"%s\"\n",argv[4+startarg]);
			exit(-1);
		}
		if (!strcmp(argv[2],"VCARD10")) {
			SMSInfo.ID = SMS_NokiaVCARD10Long;
		} else {
			SMSInfo.ID = SMS_NokiaVCARD21Long;
		}
		if (!strcmp(argv[1],"--savesms")) {
			EncodeUnicode(Sender, "VCARD",5);
			EncodeUnicode(Name, "Phonebook entry",15);
		}
		startarg += 6;
#endif
	} else if (!strcmp(argv[2],"PROFILE")) {
		if (argc<6+startarg) {
			printf("Where is profile name, ringtone & bitmap file ?\n");
			exit(-1);
		}
		/* profile name */
		EncodeUnicode(Buffer,argv[3+startarg],strlen(argv[3+startarg]));
		/* bitmap */
		bitmap.Bitmap[0].Type=GSM_PictureImage;
		error=GSM_ReadBitmapFile(argv[4+startarg],&bitmap);
		Print_Error(error);
		bitmap.Bitmap[0].Text[0]=0;
		bitmap.Bitmap[0].Text[1]=0;
		/* ringtone */
		ringtone.Format=RING_NOTETONE;
		error=GSM_ReadRingtoneFile(argv[5+startarg],&ringtone);
		Print_Error(error);
		SMSInfo.ID 	 	= SMS_NokiaProfileLong;
		SMSInfo.Buffer	 	= Buffer;
		SMSInfo.Bitmap   	= &bitmap;
		SMSInfo.Ringtone	= &ringtone;
		if (!strcmp(argv[1],"--savesms")) {
			EncodeUnicode(Sender, "Profile",7);
		}
		startarg += 6;
	} else {
		printf("What format of sms (\"%s\") ?\n",argv[2]);
		exit(-1);
	}

	for (i=startarg;i<argc;i++) {
		switch (nextlong) {
		case 0:
			if (!strcmp(argv[1],"--savesms")) {
				if (!strcmp(argv[i],"-folder")) {
					nextlong=1;
					continue;
				}
				if (!strcmp(argv[i],"-unread")) {
					State = GSM_UnRead;
					continue;
				}
				if (!strcmp(argv[i],"-read")) {
					State = GSM_Read;
					continue;
				}
				if (!strcmp(argv[i],"-unsent")) {
					State = GSM_UnSent;
					continue;
				}
				if (!strcmp(argv[i],"-sender")) {
					nextlong=2;
					continue;
				}
			} else {
				if (!strcmp(argv[i],"-report")) {
					DeliveryReport=true;
					continue;
				}
			}
			if (!strcmp(argv[i],"-smscset")) {
				nextlong=3;
				continue;
			}
			if (!strcmp(argv[i],"-smscnumber")) {
				nextlong=4;
				continue;
			}
			if (!strcmp(argv[i],"-reply")) {
				ReplyViaSameSMSC=true;
				continue;
			}
			if (!strcmp(argv[2],"RINGTONE")) {
				if (!strcmp(argv[i],"-long")) {
					SMSInfo.ID = SMS_NokiaRingtoneLong;
					break;
				}
				if (!strcmp(argv[i],"-scale")) {
					ringtone.NoteTone.AllNotesScale=true;
					break;
				}
			}
			if (!strcmp(argv[2],"EMSSOUND")) {
				if (!strcmp(argv[i],"-number")) {
					nextlong = 8;
					break;
				}
				if (!strcmp(argv[i],"-file")) {
					nextlong = 9;
					break;
				}
			}
			if (!strcmp(argv[2],"EMSANIMATION")) {
				if (!strcmp(argv[i],"-number")) {
					nextlong = 8;
					break;
				}
				if (!strcmp(argv[i],"-file")) {
					nextlong = 10;
					break;
				}
			}
			if (!strcmp(argv[2],"TEXT")) {
				if (!strcmp(argv[i],"-inputunicode")) {
					ReadUnicodeFile(Buffer,InputBuffer);
					break;
				}
				if (!strcmp(argv[i],"-flash")) {
					SMSInfo.Class = 0;
					break;
				}
				if (!strcmp(argv[i],"-len")) {
					nextlong = 5;
					break;
				}
				if (!strcmp(argv[i],"-unicode")) {
					SMSInfo.UnicodeCoding = true;
					break;
				}
				if (!strcmp(argv[i],"-enablevoice")) {
					SMSInfo.ID = SMS_EnableVoice;
					break;
				}
				if (!strcmp(argv[i],"-disablevoice")) {
					SMSInfo.ID = SMS_DisableVoice;
					break;
				}
				if (!strcmp(argv[i],"-enablefax")) {
					SMSInfo.ID = SMS_EnableFax;
					break;
				}
				if (!strcmp(argv[i],"-disablefax")) {
					SMSInfo.ID = SMS_DisableFax;
					break;
				}
				if (!strcmp(argv[i],"-enableemail")) {
					SMSInfo.ID = SMS_EnableEmail;
					break;
				}
				if (!strcmp(argv[i],"-disableemail")) {
					SMSInfo.ID = SMS_DisableEmail;
					break;
				}
				if (!strcmp(argv[i],"-voidsms")) {
					SMSInfo.ID = SMS_VoidSMS;
					break;
				}
				if (!strcmp(argv[i],"-replacemessages") &&
				    SMSInfo.ID != SMS_ConcatenatedTextLong) {
					nextlong = 11;
					break;
				}
				if (!strcmp(argv[i],"-replacefile")) {
					nextlong=12;
					continue;
				}
			}
			if (!strcmp(argv[2],"PICTURE")) {
				if (!strcmp(argv[i],"-text")) {
					nextlong = 6;
					break;
				}
				if (!strcmp(argv[i],"-unicode")) {
					SMSInfo.UnicodeCoding = true;
					break;
				}
				break;
			}
			if (!strcmp(argv[2],"OPERATOR")) {
				if (!strcmp(argv[i],"-netcode")) {
					nextlong = 7;
					break;
				}
				if (!strcmp(argv[i],"-biglogo")) {
					SMSInfo.ID = SMS_NokiaOperatorLogoLong;
					break;
				}
				break;
			}
			printf("Unknown parameter (\"%s\")\n",argv[i]);
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
				Buffer[atoi(argv[i])*2]		= 0x00;
				Buffer[atoi(argv[i])*2+1]	= 0x00;
			}
			SMSInfo.ID = SMS_ConcatenatedTextLong;
			nextlong = 0;
			break;
		case 6:	/* Picture Images - text */
			EncodeUnicode(bitmap.Bitmap[0].Text,argv[i],strlen(argv[i]));
			nextlong = 0;
			break;
		case 7:	/* Operator Logo - network code */
			strncpy(bitmap.Bitmap[0].NetworkCode,argv[i],7);
			if (!strcmp(DecodeUnicodeString(GSM_GetNetworkName(bitmap.Bitmap[0].NetworkCode)),"unknown")) {
				printf("Unknown GSM network code (\"%s\")\n",argv[i]);
				exit(-1);
			}
			if (!strcmp(argv[1],"--savesms")) {
				EncodeUnicode(Sender, "OpLogo",6);
				EncodeUnicode(Sender+6*2,bitmap.Bitmap[0].NetworkCode,3);
				EncodeUnicode(Sender+6*2+3*2,bitmap.Bitmap[0].NetworkCode+4,2);
				if (strlen(DecodeUnicodeString(GSM_GetNetworkName(bitmap.Bitmap[0].NetworkCode)))<GSM_MAX_SMS_NAME_LENGTH-7) {
					EncodeUnicode(Name,"OpLogo ",7);
					CopyUnicodeString(Name+7*2,GSM_GetNetworkName(bitmap.Bitmap[0].NetworkCode));
				} else {
					CopyUnicodeString(Name,Sender);
				}
			}
			nextlong = 0;
			break;
		case 8: /* EMS predefined sound/animation number */
			SMSInfo.Number = atoi(argv[i]);
			nextlong = 0;
			break;
		case 9: /* EMS ringtone - IMelody */
			ringtone.Format=RING_NOTETONE;
			error=GSM_ReadRingtoneFile(argv[i],&ringtone);
			Print_Error(error);
			SMSInfo.ID 	 = SMS_EMSSound;
			SMSInfo.Ringtone = &ringtone;
			nextlong = 0;
			break;
		case 10:/*EMS animation file */
			bitmap2.Bitmap[0].Type=GSM_StartupLogo;
			error=GSM_ReadBitmapFile(argv[i],&bitmap2);
			for (j=0;j<bitmap2.Number;j++) {
				/* EMS doesn't provide more than 4 frames */
				if (bitmap.Number == 4) break;
				memcpy(&bitmap.Bitmap[bitmap.Number],&bitmap2.Bitmap[j],sizeof(GSM_Bitmap));
				bitmap.Number++;
			}
			SMSInfo.ID 	 	= SMS_EMSAnimation;
			SMSInfo.Bitmap   	= &bitmap;
			nextlong = 0;
			break;
		case 11:/* Reject duplicates ID */
			SMSInfo.ReplaceMessage = atoi(argv[i]);
			if (SMSInfo.ReplaceMessage < 1 || SMSInfo.ReplaceMessage > 7) {
				printf("You have to give number between 1 and 7 (\"%s\")\n",argv[i]);
				exit(-1);
			}
			nextlong = 0;
			break;
		case 12:/* Replace file for text SMS */
			ReplaceFile = fopen(argv[i], "rb");      
			if (!ReplaceFile) Print_Error(GE_CANTOPENFILE);
			memset(ReplaceBuffer,0,sizeof(ReplaceBuffer));
			fread(ReplaceBuffer,1,sizeof(ReplaceBuffer),ReplaceFile);
			fclose(ReplaceFile);
			ReadUnicodeFile(ReplaceBuffer2,ReplaceBuffer);
			for(j=0;j<(int)(strlen(DecodeUnicodeString(Buffer)));j++) {
				for (z=0;z<(int)(strlen(DecodeUnicodeString(ReplaceBuffer2))/2);z++) {
					if (ReplaceBuffer2[z*4]   == Buffer[j] &&
					    ReplaceBuffer2[z*4+1] == Buffer[j+1]) {
						Buffer[j]   = ReplaceBuffer2[z*4+2];
						Buffer[j+1] = ReplaceBuffer2[z*4+3];
						break;
					}
				}
			}
			nextlong = 0;
			break;
		}
	}
	if (nextlong!=0) {
		printf("Parameter missed...\n");
		exit(-1);
	}

	GSM_Init(true);

	if (bitmap.Bitmap[0].Type==GSM_OperatorLogo && !strcmp(bitmap.Bitmap[0].NetworkCode,"000 00"))
	{
		error=Phone->GetNetworkInfo(&s,&NetInfo);
		Print_Error(error);
		strcpy(bitmap.Bitmap[0].NetworkCode,NetInfo.NetworkCode);
		if (!strcmp(argv[1],"--savesms")) {
			EncodeUnicode(Sender, "OpLogo",6);
			EncodeUnicode(Sender+6*2,bitmap.Bitmap[0].NetworkCode,3);
			EncodeUnicode(Sender+6*2+3*2,bitmap.Bitmap[0].NetworkCode+4,2);
			if (strlen(DecodeUnicodeString(GSM_GetNetworkName(bitmap.Bitmap[0].NetworkCode)))<GSM_MAX_SMS_NAME_LENGTH-7) {
				EncodeUnicode(Name,"OpLogo ",7);
				CopyUnicodeString(Name+7*2,GSM_GetNetworkName(bitmap.Bitmap[0].NetworkCode));
			} else {
				CopyUnicodeString(Name,Sender);
			}
		}
	}

	if (!strcmp(argv[1],"--savesms")) {
		error=Phone->GetSMSFolders(&s, &folders);
		Print_Error(error);
	}

	GSM_EncodeMultiPartSMS(&SMSInfo,&sms);

	switch (SMSInfo.ID) {
		case SMS_NokiaRingtone:
		case SMS_NokiaRingtoneLong:
		case SMS_NokiaProfileLong:
		case SMS_EMSSound:
			if (SMSInfo.RingtoneNotes!=ringtone.NoteTone.NrCommands) {
				printf("Warning: ringtone too long. %i percent part cut\n",
					(ringtone.NoteTone.NrCommands-SMSInfo.RingtoneNotes)*100/ringtone.NoteTone.NrCommands);
			}
		default:
			break;
	}

	if (!strcmp(argv[1],"--savesms")) {
		for (i=0;i<sms.Number;i++) {
			printf("Saving SMS %i/%i\n",i+1,sms.Number);
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
			printf("Saved in folder \"%s\", location %i\n",
				DecodeUnicodeString(folders.Folder[sms.SMS[i].Folder-1].Name),sms.SMS[i].Location);
		}
	} else {
		/* We do not want to make it forever - press Ctrl+C to stop */
		signal(SIGINT, interrupted);
		printf("If you want break, press Ctrl+C...\n");

		s.User.SendSMSStatus = SendSMSStatus;

		for (i=0;i<sms.Number;i++) {
			printf("Sending SMS %i/%i",i+1,sms.Number);
			sms.SMS[i].Location			= 0;
			sms.SMS[i].ReplyViaSameSMSC		= ReplyViaSameSMSC;
			sms.SMS[i].SMSC.Location		= SMSCSet;
			sms.SMS[i].PDU				= SMS_Submit;
			if (DeliveryReport) sms.SMS[i].PDU	= SMS_Status_Report;
			CopyUnicodeString(sms.SMS[i].Number, Sender);
			if (SMSCSet==0) CopyUnicodeString(sms.SMS[i].SMSC.Number, SMSC);
			SMSStatus = GE_TIMEOUT;
			error=Phone->SendSMSMessage(&s, &sms.SMS[i]);
			Print_Error(error);
			printf("....waiting for network answer");
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
	GSM_TODO		ToDo;
	GSM_PhonebookEntry	Pbk;
	GSM_CalendarNote	Note;
	GSM_Bitmap		Bitmap;
	GSM_WAPBookmark		Bookmark;
	GSM_Profile		Profile;
	GSM_MultiWAPSettings	Settings;
	GSM_Ringtone		Ringtone;
	GSM_SMSC		SMSC;
	GSM_DateTime		DateTime;
	char			buffer[50];
	GSM_Backup		Backup;
	GSM_Backup_Info		Info;

	GSM_ClearBackup(&Backup);
	GSM_GetBackupFeatures(argv[2],&Info);

	/* We do not want to make it forever - press Ctrl+C to stop */
	signal(SIGINT, interrupted);
	fprintf(stderr,"If you want break, press Ctrl+C...\n");

	GSM_Init(true);

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
	if (Info.DateTime) {
		GSM_GetCurrentDateTime (&DateTime);
		sprintf(Backup.DateTime,"%s %4d/%02d/%02d %02d:%02d:%02d",
			DayOfWeek(DateTime.Year, DateTime.Month, DateTime.Day),
			DateTime.Year, DateTime.Month, DateTime.Day,
			DateTime.Hour, DateTime.Minute, DateTime.Second);
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
					Print_Error(error);
					if (Pbk.EntriesNum!=0) {
						if (used < GSM_BACKUP_MAX_PHONEPHONEBOOK) {
							Backup.PhonePhonebook[used] = malloc(sizeof(GSM_PhonebookEntry));
						        if (Backup.PhonePhonebook[used] == NULL) Print_Error(GE_MOREMEMORY);
							Backup.PhonePhonebook[used+1] = NULL;
						} else {
							printf("Increase GSM_BACKUP_MAX_PHONEPHONEBOOK\n");
							exit(-1);
						}
						*Backup.PhonePhonebook[used]=Pbk;
						used++;
					}
					fprintf(stderr,"%cReading: %i percent",13,used*100/MemStatus.Used);
					i++;
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				fprintf(stderr,"\n");
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
					Print_Error(error);
					if (Pbk.EntriesNum!=0) {
						if (used < GSM_BACKUP_MAX_SIMPHONEBOOK) {
							Backup.SIMPhonebook[used] = malloc(sizeof(GSM_PhonebookEntry));
						        if (Backup.SIMPhonebook[used] == NULL) Print_Error(GE_MOREMEMORY);
							Backup.SIMPhonebook[used + 1] = NULL;
						} else {
							printf("Increase GSM_BACKUP_MAX_SIMPHONEBOOK\n");
							exit(-1);
						}
						*Backup.SIMPhonebook[used]=Pbk;
						used++;
					}
					fprintf(stderr,"%cReading: %i percent",13,used*100/MemStatus.Used);
					i++;
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				fprintf(stderr,"\n");
			}
		}
	}
	if (Info.Calendar) {
		Note.Location = 1;
		error=Phone->GetCalendarNote(&s,&Note,true);
		if (error==GE_NONE) {
			if (answer_yes("Backup calendar notes")) {
				used = 0;
				fprintf(stderr,"Reading : ");
				while (error == GE_NONE) {
					if (used < GSM_BACKUP_MAX_CALENDAR) {
						Backup.Calendar[used] = malloc(sizeof(GSM_CalendarNote));
					        if (Backup.Calendar[used] == NULL) Print_Error(GE_MOREMEMORY);
						Backup.Calendar[used+1] = NULL;
					} else {
						printf("Increase GSM_BACKUP_MAX_CALENDAR\n");
						exit(-1);
					}
					*Backup.Calendar[used]=Note;
					used ++;
					Note.Location = used+1;
					error=Phone->GetCalendarNote(&s,&Note,false);
					fprintf(stderr,"*");
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				fprintf(stderr,"\n");
			}
		}
	}
	if (Info.ToDo) {
		ToDo.Location = 1;
		error=Phone->GetToDo(&s,&ToDo,true);
		if (error==GE_NONE) {
			if (answer_yes("Backup ToDo")) {
				used = 0;
				fprintf(stderr,"Reading : ");
				while (error == GE_NONE) {
					if (used < GSM_BACKUP_MAX_TODO) {
						Backup.ToDo[used] = malloc(sizeof(GSM_TODO));
					        if (Backup.ToDo[used] == NULL) Print_Error(GE_MOREMEMORY);
						Backup.ToDo[used+1] = NULL;
					} else {
						printf("Increase GSM_BACKUP_MAX_TODO\n");
						exit(-1);
					}
					*Backup.ToDo[used]=ToDo;
					used ++;
					ToDo.Location = used+1;
					error=Phone->GetToDo(&s,&ToDo,false);
					fprintf(stderr,"*");
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				fprintf(stderr,"\n");
			}
		}
	}
	if (Info.CallerLogos) {
		Bitmap.Type 	= GSM_CallerLogo;
		Bitmap.Location = 1;
		error=Phone->GetBitmap(&s,&Bitmap);
		if (error == GE_NONE) {
			if (answer_yes("Backup caller groups and logos")) {
				fprintf(stderr,"Reading : ");
				error = GE_NONE;
				used  = 0;
				while (error == GE_NONE) {
					if (used < GSM_BACKUP_MAX_CALLER) {
						Backup.CallerLogos[used] = malloc(sizeof(GSM_Bitmap));
					        if (Backup.CallerLogos[used] == NULL) Print_Error(GE_MOREMEMORY);
						Backup.CallerLogos[used+1] = NULL;
					} else {
						printf("Increase GSM_BACKUP_MAX_CALLER\n");
						exit(-1);
					}
					*Backup.CallerLogos[used] = Bitmap;
					used ++;
					Bitmap.Location = used + 1;
					error=Phone->GetBitmap(&s,&Bitmap);
					fprintf(stderr,"*");
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				fprintf(stderr,"\n");
			}
		}
	}
	if (Info.SMSC) {
		if (answer_yes("Backup SMS profiles")) {
			used = 0;
			fprintf(stderr,"Reading: ");
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
					printf("Increase GSM_BACKUP_MAX_SMSC\n");
					exit(-1);
				}
				*Backup.SMSC[used]=SMSC;
				used++;
				fprintf(stderr,"*");
			}
			fprintf(stderr,"\n");
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
				fprintf(stderr,"Reading : ");
				while (error == GE_NONE) {
					if (used < GSM_BACKUP_MAX_WAPBOOKMARK) {
						Backup.WAPBookmark[used] = malloc(sizeof(GSM_WAPBookmark));
					        if (Backup.WAPBookmark[used] == NULL) Print_Error(GE_MOREMEMORY);
						Backup.WAPBookmark[used+1] = NULL;
					} else {
						printf("Increase GSM_BACKUP_MAX_WAPBOOKMARK\n");
						exit(-1);
					}
					*Backup.WAPBookmark[used]=Bookmark;
					used ++;
					Bookmark.Location = used+1;
					error=Phone->GetWAPBookmark(&s,&Bookmark);
					fprintf(stderr,"*");
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				fprintf(stderr,"\n");
			}
		}
	}
	if (Info.WAPSettings) {
		Settings.Location = 1;
		error=Phone->GetWAPSettings(&s,&Settings);
		if (error==GE_NONE) {
			if (answer_yes("Backup WAP settings")) {
				used = 0;
				fprintf(stderr,"Reading : ");
				while (error == GE_NONE) {
					if (used < GSM_BACKUP_MAX_WAPSETTINGS) {
						Backup.WAPSettings[used] = malloc(sizeof(GSM_MultiWAPSettings));
					        if (Backup.WAPSettings[used] == NULL) Print_Error(GE_MOREMEMORY);
						Backup.WAPSettings[used+1] = NULL;
					} else {
						printf("Increase GSM_BACKUP_MAX_WAPSETTINGS\n");
						exit(-1);
					}
					*Backup.WAPSettings[used]=Settings;
					used ++;
					Settings.Location = used+1;
					error=Phone->GetWAPSettings(&s,&Settings);
					fprintf(stderr,"*");
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				fprintf(stderr,"\n");
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
				fprintf(stderr,"Reading : ");
				while (error == GE_NONE || error == GE_EMPTY) {
					if (error == GE_NONE) {
						if (used < GSM_BACKUP_MAX_RINGTONES) {
							Backup.Ringtone[used] = malloc(sizeof(GSM_Ringtone));
						        if (Backup.Ringtone[used] == NULL) Print_Error(GE_MOREMEMORY);
							Backup.Ringtone[used+1] = NULL;
						} else {
							printf("Increase GSM_BACKUP_MAX_RINGTONES\n");
							exit(-1);
						}
						*Backup.Ringtone[used]=Ringtone;
						used ++;
					}
					i++;
					Ringtone.Location = i;
					Ringtone.Format	  = 0;
					error=Phone->GetRingtone(&s,&Ringtone,false);
					fprintf(stderr,"*");
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				fprintf(stderr,"\n");
			}

		}
	}
	if (Info.Profiles) {
		if (answer_yes("Backup phone profiles")) {
			used = 0;
			fprintf(stderr,"Reading: ");
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
					printf("Increase GSM_BACKUP_MAX_PROFILES\n");
					exit(-1);
				}
				*Backup.Profiles[used]=Profile;
				used++;
				fprintf(stderr,"*");
			}
			fprintf(stderr,"\n");
		}
	}

	GSM_Terminate();

	GSM_SaveBackupFile(argv[2],&Backup);
}

static void Restore(int argc, char *argv[])
{
	GSM_Backup		Backup;
	GSM_DateTime 		date_time;
	GSM_CalendarNote	Calendar;
	GSM_Bitmap		Bitmap;
	GSM_Ringtone		Ringtone;
	GSM_PhonebookEntry	Pbk;
	GSM_MemoryStatus	MemStatus;
	GSM_TODO		ToDo;
	GSM_Profile		Profile;
	GSM_MultiWAPSettings		Settings;
	GSM_WAPBookmark		Bookmark;
	int			i, used, max;

	error=GSM_ReadBackupFile(argv[2],&Backup);
	Print_Error(error);

	/* We do not want to make it forever - press Ctrl+C to stop */
	signal(SIGINT, interrupted);
	fprintf(stderr,"If you want break, press Ctrl+C...\n");

	if (Backup.DateTime[0]!=0) 	fprintf(stderr,"Time of backup : %s\n",Backup.DateTime);
	if (Backup.Model[0]!=0) 	fprintf(stderr,"Phone          : %s\n",Backup.Model);
	if (Backup.IMEI[0]!=0) 		fprintf(stderr,"IMEI           : %s\n",Backup.IMEI);

	GSM_Init(true);

	if (Backup.PhonePhonebook[0] != NULL) {
		MemStatus.MemoryType = GMT_ME;
		error=Phone->GetMemoryStatus(&s, &MemStatus);
		if (error==GE_NONE) {
			if (answer_yes("Restore phone phonebook")) {
				max = 0;
				while (Backup.PhonePhonebook[max]!=NULL) max++;
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
					fprintf(stderr,"%cWriting: %i percent",13,(i+1)*100/(MemStatus.Used+MemStatus.Free));
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				fprintf(stderr,"\n");
			}
		}
	}
	if (Backup.SIMPhonebook[0] != NULL) {
		MemStatus.MemoryType = GMT_SM;
		error=Phone->GetMemoryStatus(&s, &MemStatus);
		if (error==GE_NONE) {
			if (answer_yes("Restore SIM phonebook")) {
				max = 0;
				while (Backup.SIMPhonebook[max]!=NULL) max++;
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
					fprintf(stderr,"%cWriting: %i percent",13,(i+1)*100/(MemStatus.Used+MemStatus.Free));
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				fprintf(stderr,"\n");
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
					fprintf(stderr,"%cWriting: %i percent",13,(i+1)*100/max);
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				fprintf(stderr,"\n");
			}
		}
	}
	if (answer_yes("Do you want to set date/time in phone (NOTE: in some phones it's required to correctly restore calendar notes and other items)")) {
		GSM_GetCurrentDateTime(&date_time);

		error=Phone->SetDateTime(&s, &date_time);
		Print_Error(error);
	}
	if (Backup.Calendar[0] != NULL) {
		Calendar.Location = 1;
		error = Phone->GetCalendarNote(&s,&Calendar,true);
		if (error == GE_NONE || error == GE_INVALIDLOCATION) {
			if (answer_yes("Restore calendar notes")) {
				fprintf(stderr,"Deleting old notes: ");
				while (error==GE_NONE) {
					error = Phone->DeleteCalendarNote(&s,&Calendar);
					fprintf(stderr,"*");
				}
				fprintf(stderr,"\n");
				max = 0;
				while (Backup.Calendar[max]!=NULL) max++;
				for (i=0;i<max;i++) {
					Calendar = *Backup.Calendar[i];
					error=Phone->SetCalendarNote(&s,&Calendar);
					Print_Error(error);
					fprintf(stderr,"%cWriting: %i percent",13,(i+1)*100/max);
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				fprintf(stderr,"\n");
			}
		}
	}
	if (Backup.ToDo[0] != NULL) {
		ToDo.Location = 1;
		error = Phone->GetToDo(&s,&ToDo,true);
		if (error == GE_NONE || error == GE_INVALIDLOCATION) {
			if (answer_yes("Restore ToDo")) {
				fprintf(stderr,"Deleting old ToDo: ");
				error=Phone->DeleteAllToDo(&s);
				Print_Error(error);
				fprintf(stderr,"Done\n");
				max = 0;
				while (Backup.ToDo[max]!=NULL) max++;
				for (i=0;i<max;i++) {
					ToDo 		= *Backup.ToDo[i];
					ToDo.Location 	= 0;
					error=Phone->SetToDo(&s,&ToDo);
					Print_Error(error);
					fprintf(stderr,"%cWriting: %i percent",13,(i+1)*100/max);
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				fprintf(stderr,"\n");
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
				fprintf(stderr,"%cWriting: %i percent",13,(i+1)*100/max);
				if (bshutdown) {
					GSM_Terminate();
					exit(0);
				}
			}
			fprintf(stderr,"\n");
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
				fprintf(stderr,"Deleting old bookmarks: ");
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
					fprintf(stderr,"*");
				}
				fprintf(stderr,"\n");
				max = 0;
				while (Backup.WAPBookmark[max]!=NULL) max++;
				for (i=0;i<max;i++) {
					Bookmark = *Backup.WAPBookmark[i];
					Bookmark.Location = 0;
					error=Phone->SetWAPBookmark(&s,&Bookmark);
					Print_Error(error);
					fprintf(stderr,"%cWriting: %i percent",13,(i+1)*100/max);
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				fprintf(stderr,"\n");
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
					fprintf(stderr,"%cWriting: %i percent",13,(i+1)*100/max);
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				fprintf(stderr,"\n");
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
					fprintf(stderr,"%cWriting: %i percent",13,(i+1)*100/max);
					if (bshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				fprintf(stderr,"\n");
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
				fprintf(stderr,"\n");
			}
		}
	}

	GSM_Terminate();
}
#endif

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
			printf("%i. ",i);
			if (settings.Settings[j].Title[0]==0 && settings.Settings[j].Title[1]==0)
			{
				printf("Set %i\n",i);
			} else {
				printf("%s\n",DecodeUnicodeString(settings.Settings[j].Title));
			}
			printf("Homepage            : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].HomePage));
			if (settings.Settings[j].IsContinuous) {
				printf("Connection type     : Continuous\n");
			} else {
				printf("Connection type     : Temporary\n");
			}
			if (settings.Settings[j].IsSecurity) {
				printf("Connection security : On\n");
			} else {
				printf("Connection security : Off\n");
			}
			switch (settings.Settings[j].Bearer) {
			case WAPSETTINGS_BEARER_SMS:
				printf("Bearer              : SMS\n");
				printf("Server number       : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].Server));
				printf("Service number      : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].Service));
				break;
			case WAPSETTINGS_BEARER_DATA:
				printf("Bearer              : Data (CSD)\n");
				printf("Dial-up number      : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].DialUp));
				printf("IP address          : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].IPAddress));
				if (settings.Settings[j].ManualLogin) {
					printf("Login Type          : Manual\n");
				} else {
					printf("Login Type          : Automatic\n");
				}
				if (settings.Settings[j].IsNormalAuthentication) {
					printf("Authentication type : Normal\n");
				} else {
					printf("Authentication type : Secure\n");
				}
				if (settings.Settings[j].IsISDNCall) {
					printf("Data call type      : ISDN\n");
	                       	} else {
					printf("Data call type      : Analogue\n");  
				}
				switch (settings.Settings[j].Speed) {
					case WAPSETTINGS_SPEED_9600  : printf("Data call speed     : 9600\n");  break;
					case WAPSETTINGS_SPEED_14400 : printf("Data call speed     : 14400\n"); break;
					case WAPSETTINGS_SPEED_AUTO  : printf("Data call speed     : Auto\n");  break;
				}
				printf("User name           : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].User));
				printf("Password            : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].Password));
				break;
			case WAPSETTINGS_BEARER_USSD:
				printf("Bearer              : USSD\n");
				printf("Service code        : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].Code));
				if (settings.Settings[j].IsIP) {
					printf("Address type        : IP address\nIPaddress           : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].Service));
				} else {
					printf("Address type        : Service number\nService number      : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].Service));	
				}
				break;
			case WAPSETTINGS_BEARER_GPRS:
				printf("Bearer              : GPRS\n");
				if (settings.Settings[j].ManualLogin) {
					printf("Login Type          : Manual\n");
				} else {
					printf("Login Type          : Automatic\n");
				}
				if (settings.Settings[j].IsNormalAuthentication) {
					printf("Authentication type : Normal\n");
				} else {
					printf("Authentication type : Secure\n");
				}
				printf("Access point        : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].DialUp));
				printf("IP address          : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].IPAddress));
				printf("User name           : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].User));
				printf("Password            : \"%s\"\n",DecodeUnicodeString(settings.Settings[j].Password));
			}
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
							printf("Increase GSM_BACKUP_MAX_SMS\n");
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
			fprintf(stderr,"%cDeleting: %i percent",13,(j+1)*100/smsnum);
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
				case GSM_StartupLogo :	printf("Startup logo");		break;
				case GSM_OperatorLogo:	printf("Operator logo"); 	break;
				case GSM_PictureImage:	printf("Picture Image"); 	break;
				case GSM_CallerLogo  :	printf("Caller group logo"); 	break;
				default		     : 					break;
			}
			printf(", width %i, height %i\n",Bitmap.Bitmap[i].Width,Bitmap.Bitmap[i].Height);
			GSM_PrintBitmap(stdout,&Bitmap.Bitmap[i]);
		}
	} else {
		if (argc == 5) {
			for (i=0;i<Bitmap.Number;i++) {
				if (!strcmp(argv[4],"PICTURE")) {
					Bitmap.Bitmap[i].Type = GSM_PictureImage;
				} else if (!strcmp(argv[4],"STARTUP")) {
					Bitmap.Bitmap[i].Type = GSM_StartupLogo;
				} else if (!strcmp(argv[4],"CALLER")) {
					Bitmap.Bitmap[i].Type = GSM_CallerLogo;
				} else if (!strcmp(argv[4],"OPERATOR")) {
					Bitmap.Bitmap[i].Type = GSM_OperatorLogo;
				} else {
					printf("What format of output file logo (\"%s\") ?\n",argv[4]);
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
		printf("It can be RTTL ringtone only used with this option\n");
		exit(-1);
	}
 
	started = false; 
	j	= 0;
	for (i=0;i<ringtone.NoteTone.NrCommands;i++) {
		if (ringtone.NoteTone.Commands[i].Type == RING_Note) {
			Note = &ringtone.NoteTone.Commands[i].Note;
			if (!started) {
				if (Note->Note != Note_Pause) {
					printf("Ringtone \"%s\" (tempo = %i Beats Per Minute)\n\n",DecodeUnicodeString(ringtone.Name),GSM_RTTLGetTempo(Note->Tempo));
					started = true;
				}
			}
			if (started) j++;
		}
	}
    	if (j>50) printf("WARNING: LENGTH=%i NOTES, BUT YOU WILL ENTER ONLY FIRST 50 TONES.",j);

	printf("\n\nThis ringtone in Nokia Composer in phone should look: ");  
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
				if (Note->DurationSpec == DottedNote) printf(".");
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

	printf("\n\nTo enter it please press: ");    
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
				if (Note->DurationSpec == DottedNote) printf("(longer)");
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
					if (Note->Scale != DefNoteScale) {
						while (DefNoteScale != Note->Scale) {
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
					default		   : dprintf("error\n");break;
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

static void CopyRingtone(int argc, char *argv[])
{
	GSM_Ringtone 		ringtone, ringtone2;
	GSM_RingtoneFormat	Format;

	ringtone.Format	= 0;
	error=GSM_ReadRingtoneFile(argv[2],&ringtone);
	Print_Error(error);

	Format = ringtone.Format;
	if (argc == 5) {
		if (!strcmp(argv[4],"RTTL")) {		Format = RING_NOTETONE;
		} else if (!strcmp(argv[4],"BINARY")) {	Format = RING_NOKIABINARY;
		} else {
			printf("What format of output ringtone file (\"%s\") ?\n",argv[4]);
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
				printf("Unknown key/function name: \"%c\"\n",key);
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

static void GetToDo(int argc, char *argv[])
{
	GSM_TODO	ToDo;
	int		start,stop;
	bool		refresh=true;

	GetStartStop(&start, &stop, 2, argc, argv);

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		ToDo.Location=i;
		error=Phone->GetToDo(&s,&ToDo,refresh);
		Print_Error(error);
		printf("Location : %i\n",i);
		printf("Priority : ");
		switch (ToDo.Priority) {
			case GSM_Priority_Low	 : printf("Low (1)\n");	 	break;
			case GSM_Priority_Medium : printf("Medium (2)\n"); 	break;
			case GSM_Priority_High	 : printf("High (3)\n");	break;
		}
		printf("Text     : \"%s\"\n",DecodeUnicodeString(ToDo.Text));
		printf("\n");
		refresh=false;
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
		case GSCT_SecurityCode: printf("Waiting for Security Code.\n"); break;
		case GSCT_Pin         : printf("Waiting for PIN.\n"); 		break;
		case GSCT_Pin2        : printf("Waiting for PIN2.\n"); 		break;
		case GSCT_Puk         : printf("Waiting for PUK.\n");		break;
		case GSCT_Puk2        : printf("Waiting for PUK2.\n"); 		break;
		case GSCT_None        : printf("Nothing to enter.\n"); 		break;
		default		      : printf("Unknown\n");
	}      
	
	GSM_Terminate();
}

static void EnterSecurityCode(int argc, char *argv[])
{
	GSM_SecurityCode Code;

	if (!strcmp(argv[2],"PIN")) {		Code.Type = GSCT_Pin;
	} else if (!strcmp(argv[2],"PUK")) {	Code.Type = GSCT_Puk;
	} else if (!strcmp(argv[2],"PIN2")) {	Code.Type = GSCT_Pin2;
	} else if (!strcmp(argv[2],"PUK2")) {	Code.Type = GSCT_Puk2;
	} else {
		printf("What security code (\"%s\") ?\n",argv[2]);
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

		printf("%i. \"%s\"",i,DecodeUnicodeString(Profile.Name));
		if (Profile.DefaultName) 	printf(" (default name)");
		if (Profile.HeadSetProfile) 	printf(" (HeadSet profile)");
		if (Profile.CarKitProfile) 	printf(" (CarKit profile)");
		printf("\n");
		for (j=0;j<Profile.FeaturesNumber;j++) {
			special = false;
			switch (Profile.FeatureID[j]) {
			case Profile_MessageToneID:
			case Profile_RingtoneID:
				special = true;
				if (Profile.FeatureID[j] == Profile_RingtoneID) {
					printf("Ringtone ID           : ");
				} else {
					printf("Message alert tone ID : ");
				}
				if (strlen(DecodeUnicodeString(GSM_GetRingtoneName(&Info,Profile.FeatureValue[j])))!=0) {
					printf("\"%s\"\n",DecodeUnicodeString(GSM_GetRingtoneName(&Info,Profile.FeatureValue[j])));
				} else {
					printf("%i\n",Profile.FeatureValue[j]);
				}
				break;	
			case Profile_CallerGroups:
				special = true;
				printf("Call alert for        :");
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
						printf(" \"%s\"",DecodeUnicodeString(caller[k].Text));
					}
				}
				printf("\n");
				break;
			case Profile_ScreenSaverNumber:
				special = true;
				printf("Screen saver number   : ");
				printf("%i\n",Profile.FeatureValue[j]);
				break;
			case Profile_CallAlert  	: printf("Incoming call alert   : "); break;
			case Profile_RingtoneVolume 	: printf("Ringtone volume       : "); break;
			case Profile_Vibration		: printf("Vibrating alert       : "); break;
			case Profile_MessageTone	: printf("Message alert tone    : "); break;
			case Profile_KeypadTone		: printf("Keypad tones          : "); break;
			case Profile_WarningTone	: printf("Warning (games) tones : "); break;
			case Profile_ScreenSaver	: printf("Screen saver          : "); break;
			case Profile_ScreenSaverTime	: printf("Screen saver timeout  : "); break;
			case Profile_AutoAnswer		: printf("Automatic answer      : "); break;
			case Profile_Lights		: printf("Lights                : "); break;
			default:
				printf("Unknown\n");
				special = true;
			}
			if (!special) {
				switch (Profile.FeatureValue[j]) {
				case PROFILE_VOLUME_LEVEL1 		:
				case PROFILE_KEYPAD_LEVEL1 		: printf("Level 1\n"); 		break;
				case PROFILE_VOLUME_LEVEL2 		:
				case PROFILE_KEYPAD_LEVEL2 		: printf("Level 2\n");		break;
				case PROFILE_VOLUME_LEVEL3 		:
				case PROFILE_KEYPAD_LEVEL3 		: printf("Level 3\n"); 		break;
				case PROFILE_VOLUME_LEVEL4 		: printf("Level 4\n"); 		break;
				case PROFILE_VOLUME_LEVEL5 		: printf("Level 5\n"); 		break;
				case PROFILE_MESSAGE_NOTONE 		:
				case PROFILE_AUTOANSWER_OFF		:
				case PROFILE_LIGHTS_OFF  		:
				case PROFILE_SAVER_OFF			:
				case PROFILE_WARNING_OFF		:
				case PROFILE_CALLALERT_OFF	 	:
				case PROFILE_VIBRATION_OFF 		:
				case PROFILE_KEYPAD_OFF	   		: printf("Off\n");	  	break;
				case PROFILE_CALLALERT_RINGING   	: printf("Ringing\n");		break;
				case PROFILE_CALLALERT_BEEPONCE  	:
				case PROFILE_MESSAGE_BEEPONCE 		: printf("Beep once\n"); 	break;
				case PROFILE_CALLALERT_RINGONCE  	: printf("Ring once\n");	break;
				case PROFILE_CALLALERT_ASCENDING 	: printf("Ascending\n");        break;
				case PROFILE_CALLALERT_CALLERGROUPS	: printf("Caller groups\n");	break;
				case PROFILE_MESSAGE_STANDARD 		: printf("Standard\n");  	break;
				case PROFILE_MESSAGE_SPECIAL 		: printf("Special\n");	 	break;
				case PROFILE_MESSAGE_ASCENDING		: printf("Ascending\n"); 	break;
				case PROFILE_MESSAGE_PERSONAL		: printf("Personal\n");		break;
				case PROFILE_AUTOANSWER_ON		:
				case PROFILE_WARNING_ON			:
				case PROFILE_SAVER_ON			:
				case PROFILE_VIBRATION_ON 		: printf("On\n");  		break;
				case PROFILE_VIBRATION_FIRST 		: printf("Vibrate first\n");	break;
				case PROFILE_LIGHTS_AUTO 		: printf("Auto\n"); 		break;
				case PROFILE_SAVER_TIMEOUT_5SEC	 	: printf("5 seconds\n"); 	break;
				case PROFILE_SAVER_TIMEOUT_20SEC 	: printf("20 seconds\n"); 	break;
				case PROFILE_SAVER_TIMEOUT_1MIN	 	: printf("1 minute\n");		break;
				case PROFILE_SAVER_TIMEOUT_2MIN	 	: printf("2 minutes\n");	break;
				case PROFILE_SAVER_TIMEOUT_5MIN	 	: printf("5 minutes\n");	break;
				case PROFILE_SAVER_TIMEOUT_10MIN 	: printf("10 minutes\n");	break;
				default					: printf("UNKNOWN\n");
				}	
			}
		}
		printf("\n");
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
		printf("%i.",i);
		switch (error) {
		case GE_EMPTY:
			printf(" speed dial not assigned\n");
			break;
		default:
			Print_Error(error);

			Phonebook.Location	= SpeedDial.MemoryLocation;
			Phonebook.MemoryType 	= SpeedDial.MemoryType;
			error=Phone->GetMemory(&s,&Phonebook);

			GSM_PhonebookFindDefaultNameNumberGroup(Phonebook, &Name, &Number, &Group);
	
			if (Name != -1) printf(" Name \"%s\",",DecodeUnicodeString(Phonebook.Entries[Name].Text));
			printf(" Number \"%s\"",DecodeUnicodeString(Phonebook.Entries[SpeedDial.MemoryNumberID-1].Text));
		}
		printf("\n");
	}

	GSM_Terminate();
}

static void ResetPhoneSettings(int argc, char *argv[])
{
	GSM_ResetSettingsType Type;

	if (!strcmp(argv[2],"PHONE")) {		 Type = GSM_RESET_PHONESETTINGS;
	} else if (!strcmp(argv[2],"UIF")) {	 Type = GSM_RESET_USERINTERFACE;
	} else if (!strcmp(argv[2],"ALL")) {	 Type = GSM_RESET_USERINTERFACE_PHONESETTINGS;
	} else if (!strcmp(argv[2],"DEV")) {	 Type = GSM_RESET_DEVICE;
	} else if (!strcmp(argv[2],"FACTORY")) { Type = GSM_RESET_FULLFACTORY;
	} else {
		printf("What type of reset phone settings (\"%s\") ?\n",argv[2]);
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
		printf("Too high folder number (max. %i)\n",folders.Number);
		GSM_Terminate();
		exit(-1);
	}

	printf("Deleting SMS from \"%s\" folder: ",DecodeUnicodeString(folders.Folder[foldernum-1].Name));

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

	printf("Current display features :\n");

	for (i=0;i<Features.Number;i++) {
		switch(Features.Feature[i]) {
		case GSM_CallActive	: printf("Call active\n");	break;
		case GSM_UnreadSMS	: printf("Unread SMS\n");	break;
		case GSM_VoiceCall	: printf("Voice call\n");	break;
		case GSM_FaxCall	: printf("Fax call\n");		break;
		case GSM_DataCall	: printf("Data call\n");	break;
		case GSM_KeypadLocked	: printf("Keypad locked\n");	break;
		case GSM_SMSMemoryFull	: printf("SMS memory full\n");
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
	int		size,i;

	file = fopen(argv[2], "rb");
	if (!file) Print_Error(GE_CANTOPENFILE);
	size=fread(InputBuffer, 1, 10000-1, file);
	fclose(file);
	InputBuffer[size]   = 0;
	InputBuffer[size+1] = 0;

	ReadUnicodeFile(Buffer,InputBuffer);

	for(i=0;i<((int)strlen(DecodeUnicodeString(Buffer)));i++) {
		dprintf("\\x%02x\\x%02x",Buffer[i*2],Buffer[i*2+1]);
	}
	dprintf("\\x00\\x00");
}
#endif

static void Version(int argc, char *argv[])
{
	printf("[Gammu version %s built %s %s]\n\n",VERSION,__TIME__,__DATE__);
}

static void usage(void)
{
	printf("[Gammu version %s built %s %s]\n\n",VERSION,__TIME__,__DATE__);

	printf("gammu --version\n");
	printf("gammu --identify\n");
	printf("gammu --monitor\n");
	printf("gammu --reset SOFT|HARD\n");
	printf("gammu --resetphonesettings PHONE|DEV|UIF|ALL|FACTORY\n");
	printf("gammu --presskeysequence mMnNpPuUdD+-123456789*0#gGrRwW\n");
	printf("gammu --getdisplaystatus\n");
	printf("gammu --getprofile start [stop]\n");
	printf("gammu --setautonetworklogin\n");
	printf("gammu --getsecuritystatus\n");
	printf("gammu --entersecuritycode PIN|PUK|PIN2|PUK2 code\n\n");

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
	printf("gammu --getsmsfolders\n\n");

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
	printf("gammu --getcalendarnote start [stop]\n");
	printf("gammu --deletecalendarnote start [stop]\n\n");

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
	printf("gammu --setbitmap PICTURE file location\n");
	printf("gammu --setbitmap TEXT text\n");
	printf("gammu --setbitmap DEALER text\n\n");

	printf("gammu --copybitmap inputfile [outputfile [OPERATOR|PICTURE|STARTUP|CALLER]]\n\n");

	printf("gammu --savesms TEXT [-folder number][-reply][-sender number][-flash]\n");
	printf("                     [-smscset number][-smscnumber number][-len len]\n");
	printf("                     [-enablefax][-disablefax][-enablevoice][-unsent]\n");
	printf("                     [-disablevoice][-enableemail][-disableemail][-read]\n");
	printf("                     [-voidsms][-unicode][-inputunicode][-unread]\n");
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
	printf("gammu --savesms EMSSOUND [-number number][-file file][-folder number]\n");
	printf("                         [-reply][-smscset number][-smscnumber number]\n");
	printf("                         [-sender number][-unread][-read][-unsent]\n");
	printf("gammu --savesms EMSANIMATION [-number number][-file file][-folder number]\n");
	printf("                             [-smscset number][-smscnumber number][-read]\n");
	printf("                             [-sender number][-unread][-unsent][-reply]\n");
	printf("gammu --savesms EMSBITMAP file [-folder number][-reply][-smscset number]\n");
	printf("                               [-smscnumber number][-sender number]\n");
	printf("                               [-unread][-read][-unsent]\n\n");

	printf("gammu --sendsms TEXT destination [-reply][-flash][-smscset number]\n");
	printf("                                 [-smscnumber number][-len len]\n");
	printf("                                 [-enablefax][-disablefax][-enablevoice]\n");
	printf("                                 [-disablevoice][-enableemail][-report]\n");
	printf("                                 [-disableemail][-voidsms][-unicode]\n");
	printf("                                 [-inputunicode][-replacemessages ID]\n");
	printf("                                 [-replacefile file]\n");
	printf("gammu --sendsms RINGTONE destination file [-reply][-smscset number]\n");
	printf("                                          [-long][-smscnumber number]\n");
	printf("                                          [-report][-scale]\n");
	printf("gammu --sendsms OPERATOR destination file [-reply][-smscset number][-biglogo]\n");
	printf("                                          [-netcode netcode][-report]\n");
	printf("                                          [-smscnumber number]\n");
	printf("gammu --sendsms CALLER destination file [-reply][-smscset number]\n");
	printf("                                        [-report][-smscnumber number]\n");
	printf("gammu --sendsms PICTURE destination file [-reply][-smscset number]\n");
	printf("                                         [-report][-smscnumber number]\n");
	printf("                                         [-unicode][-text text]\n");
#ifdef GSM_ENABLE_BACKUP
	printf("gammu --sendsms BOOKMARK destination file location [-smscset number]\n");
	printf("                                                   [-reply][-report]\n");
	printf("                                                   [-smscnumber number]\n");
	printf("gammu --sendsms SETTINGS destination file location [-smscset number]\n");
	printf("                                                   [-smscnumber number]\n");
	printf("                                                   [-report][-reply]\n");
	printf("gammu --sendsms CALENDAR destination file location [-smscset number]\n");
	printf("                                                   [-smscnumber number]\n");
	printf("                                                   [-report][-reply]\n");
	printf("gammu --sendsms VCARD10|VCARD21 destination file SM|ME location\n");
	printf("                     [-reply][-smscset number][-smscnumber number][-report]\n");
#endif
	printf("gammu --sendsms PROFILE destination name bitmap ringtone\n");
	printf("                     [-reply][-smscset number][-smscnumber number][-report]\n");
	printf("gammu --sendsms EMSSOUND destination [-number number][-file file][-reply]\n");
	printf("                                     [-smscset number][-smscnumber number]\n");
	printf("                                     [-report]\n");
	printf("gammu --sendsms EMSANIMATION destination [-number number][-file file]\n");
	printf("                                         [-smscset number][-reply]\n");
	printf("                                         [-smscnumber number][-report]\n");
	printf("gammu --sendsms EMSBITMAP destination file [-reply][-smscset number]\n");
	printf("                                           [-smscnumber number][-report]\n");

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
#endif

#ifdef DEBUG
	printf("\ngammu --decodesniff MBUS2|IRDA file [phonemodel]\n");
	printf("gammu --decodebinarydump file [phonemodel]\n");
	printf("gammu --makeconverttable file\n");
#endif
}

static GSM_Parameters Parameters[] = {
	{"--identify",			0, 0, Identify			},
	{"--version",			0, 0, Version			},
	{"--monitor",			0, 0, Monitor			},
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
	{"--senddtmf",			1, 1, SendDTMF			},
	{"--getdatetime",		0, 0, GetDateTime		},
	{"--setdatetime",		0, 0, SetDateTime		},
	{"--getalarm",			0, 0, GetAlarm			},
	{"--setalarm",			2, 2, SetAlarm			},
	{"--getspeeddial",		1, 2, GetSpeedDial		},
	{"--resetphonesettings",	1, 1, ResetPhoneSettings	},
	{"--getmemory",			2, 3, GetMemory			},
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
	{"--getcalendarnote",		1, 2, GetCalendarNote		},
	{"--gettodo",			1, 2, GetToDo			},
	{"--deletecalendarnote",	1, 2, DeleteCalendarNote	},
	{"--reset",			1, 1, Reset			},
	{"--getprofile",		1, 2, GetProfile		},
	{"--getsecuritystatus",		0, 0, GetSecurityStatus		},
	{"--entersecuritycode",		2, 2, EnterSecurityCode		},
	{"--deletewapbookmark", 	1, 2, DeleteWAPBookmark 	},
	{"--getwapbookmark",		1, 2, GetWAPBookmark		},
	{"--getwapsettings",		1, 2, GetWAPSettings		},
	{"--getbitmap",			1, 3, GetBitmap			},
	{"--setbitmap",			1, 3, SetBitmap			},
	{"--savesms",			1,10, SendSaveSMS		},
	{"--sendsms",			2,10, SendSaveSMS		},
#ifdef GSM_ENABLE_BACKUP
	{"--backup",			1, 1, Backup			},
	{"--backupsms",			1, 1, BackupSMS			},
	{"--restore",			1, 1, Restore			},
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
	char	*model,*debuglevel;
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
	        s.CFGLocalize = CFG_Get(cfg, "gammu", "gammuloc");
        	if (s.CFGLocalize) s.msg=CFG_ReadFile(s.CFGLocalize);
	}
	CFG_ReadConfig(cfg, &model, &s.CFGDevice, &s.CFGConnection,
		&s.CFGSyncTime, &s.CFGDebugFile, &debuglevel, &s.CFGLockDevice);
	strcpy(s.CFGModel,model);
	strcpy(s.CFGDebugLevel,debuglevel);

	/* When user gave debug level on command line */
	if (argc > 1 && GSM_SetDebugLevel(argv[1], &di)) {
		/* Debug level from command line will be used with phone too */
		strcpy(s.CFGDebugLevel,argv[1]);
		start = 1;
	} else {
		/* Try to set debug level from file */
		GSM_SetDebugLevel(s.CFGDebugLevel, &di);
	}

	/* If user gave debug file in gammurc, we will use it */
	error=GSM_SetDebugFile(s.CFGDebugFile, &di);
	Print_Error(error);

	if (argc==1) {
		usage();
	} else {
		while (Parameters[z].Function!=NULL) {
			if (!strcmp(Parameters[z].parameter,argv[1+start]) &&
				argc-2-start>=Parameters[z].min_arg &&
				argc-2-start<=Parameters[z].max_arg)
			{
				if (strcmp(GetGammuVersion(),VERSION)!=0) {
					printf("ERROR: version of installed libGammu.so (%s) is different to version of Gammu (%s)\n",
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

	return 0;
}

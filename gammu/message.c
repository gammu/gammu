#define _GNU_SOURCE /* For strcasestr */
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef WIN32
#  include <io.h>
#else
#  include <unistd.h>
#endif

#include "../common/misc/locales.h"

#include "common.h"
#include "message.h"
#include "memory.h"
#include "formats.h"
#include "calendar.h"
#include "misc.h"

#include <gammu-smsd.h> /* For SMSD interaction */

volatile bool 			wasincomingsms 		= false;

GSM_MultiSMSMessage		IncomingSMSData;

/**
 * Prints location information about message.
 */
void PrintSMSLocation(const GSM_SMSMessage *sms, const GSM_SMSFolders *folders)
{
	printf(_("Location %i, folder \"%s\""),
			sms->Location,
			DecodeUnicodeConsole(folders->Folder[sms->Folder - 1].Name)
			);

	switch (sms->Memory) {
		case MEM_SM:
			printf(", %s", _("SIM memory"));
			break;
		case MEM_ME:
			printf(", %s", _("phone memory"));
			break;
		case MEM_MT:
			printf(", %s", _("phone or SIM memory"));
			break;
		default:
			break;
	}

	if (sms->InboxFolder) {
		printf(", %s", _("Inbox folder"));
	}

	printf("\n");
}

/**
 * Prints single phone number optionally showing name of contact from backup data.
 */
void PrintPhoneNumber(unsigned char *number, const GSM_Backup *Info)
{
	bool 	found=false,found2=false;
	int 	i,j,z;

	printf("\"%s\"",DecodeUnicodeConsole(number));

	if (Info == NULL) return;

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
						printf(" (%s", _("mobile"));
						break;
					case PBK_Number_Work:
						printf(" (%s", _("work"));
						break;
					case PBK_Number_Fax:
						printf(" (%s", _("fax"));
						break;
					case PBK_Number_Home:
						printf(" (%s", _("home"));
						break;
					case PBK_Number_Pager:
						printf(" (%s", _("pager"));
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

void DisplaySingleSMSInfo(GSM_SMSMessage sms, bool displaytext, bool displayudh, const GSM_Backup *Info)
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
		PrintPhoneNumber(sms.Number, Info);
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
			PrintPhoneNumber(sms.Number, Info);
			for (i=0;i<sms.OtherNumbersNum;i++) {
				printf(", ");
				PrintPhoneNumber(sms.OtherNumbers[i], Info);
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
				if (GSM_DecodeSiemensOTASMS(GSM_GetDebug(gsm), &SiemensOTA,&sms)) {
					printf("%s\n", _("Siemens file"));
					break;
				}
				printf("%s\n", _("8 bit SMS, cannot be displayed here"));
			}
		}
		break;
#ifndef CHECK_CASES
	default:
		printf(_("Unknown PDU type: 0x%x\n"), sms.PDU);
		break;
#endif
	}
}

void DisplayMultiSMSInfo (GSM_MultiSMSMessage *sms, bool eachsms, bool ems, const GSM_Backup *Info)
{
	GSM_SiemensOTASMSInfo 	SiemensOTA;
	GSM_MultiPartSMSInfo	SMSInfo;
	bool			RetVal,udhinfo=true;
	int			j,i;
	size_t Pos;
	GSM_MemoryEntry		pbk;
	GSM_Error error;

	/* GSM_DecodeMultiPartSMS returns if decoded SMS content correctly */
	RetVal = GSM_DecodeMultiPartSMS(GSM_GetDebug(gsm), &SMSInfo,sms,ems);

	if (eachsms) {
		if (GSM_DecodeSiemensOTASMS(GSM_GetDebug(gsm), &SiemensOTA,&sms->SMS[0])) udhinfo = false;
		if (sms->SMS[0].UDH.Type != UDH_NoUDH && sms->SMS[0].UDH.AllParts == sms->Number) udhinfo = false;
		if (RetVal && !udhinfo) {
			DisplaySingleSMSInfo(sms->SMS[0],false,false,Info);
			printf("\n");
		} else {
			for (j=0;j<sms->Number;j++) {
				DisplaySingleSMSInfo(sms->SMS[j],!RetVal,udhinfo,Info);
				printf("\n");
			}
		}
	} else {
		for (j=0;j<sms->Number;j++) {
			DisplaySingleSMSInfo(sms->SMS[j],!RetVal,true,Info);
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
				error = GSM_DecodeVCARD(GSM_GetDebug(gsm), SMSInfo.Entries[i].File->Buffer, &Pos, &pbk, Nokia_VCard21);
				if (error == ERR_NONE) PrintMemoryEntry(&pbk);
			} else {
				printf("\n");
			}
			break;
		case SMS_NokiaRingtone:
			printf(_("Ringtone \"%s\"\n"),DecodeUnicodeConsole(SMSInfo.Entries[i].Ringtone->Name));
			GSM_SaveRingtoneRttl(stdout,SMSInfo.Entries[i].Ringtone);
			printf("\n");
			if (answer_yes(_("Do you want to play it?")))
				GSM_PlayRingtone(*SMSInfo.Entries[i].Ringtone);
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
		case SMS_Text:
		case SMS_NokiaRingtoneLong:
		case SMS_NokiaOperatorLogoLong:
		case SMS_NokiaWAPBookmarkLong:
		case SMS_NokiaWAPSettingsLong:
		case SMS_NokiaMMSSettingsLong:
		case SMS_NokiaVCARD10Long:
		case SMS_NokiaVTODOLong:
		case SMS_VCARD10Long:
		case SMS_VCARD21Long:
		case SMS_DisableVoice:
		case SMS_DisableFax:
		case SMS_DisableEmail:
		case SMS_EnableVoice:
		case SMS_EnableFax:
		case SMS_EnableEmail:
		case SMS_VoidSMS:
		case SMS_EMSSound10:
		case SMS_EMSSound12:
		case SMS_EMSSonyEricssonSound:
		case SMS_EMSSound10Long:
		case SMS_EMSSound12Long:
		case SMS_EMSSonyEricssonSoundLong:
		case SMS_EMSVariableBitmapLong:
		case SMS_MMSIndicatorLong:
		case SMS_WAPIndicatorLong:
		case SMS_AlcatelMonoBitmapLong:
		case SMS_AlcatelMonoAnimationLong:
		case SMS_AlcatelSMSTemplateName:
#ifndef CHECK_CASES
		default:
#endif
			printf("%s\n", _("Error"));
			break;
		}
	}
	printf("\n");
	GSM_FreeMultiPartSMSInfo(&SMSInfo);
}

void IncomingSMS(GSM_StateMachine *sm UNUSED, GSM_SMSMessage sms, void *user_data)
{
	printf("%s\n", _("SMS message received"));
 	if (wasincomingsms) {
 		printf("%s\n", _("We already have one pending, ignoring this one!"));
 		return;
 	}
 	wasincomingsms = true;
 	memcpy(&IncomingSMSData.SMS[0],&sms,sizeof(GSM_SMSMessage));
 	IncomingSMSData.Number = 1;
}

void DisplayIncomingSMS(void)
{
 	GSM_SMSFolders folders;
	GSM_Error error;

 	if (IncomingSMSData.SMS[0].State == 0) {
 		error=GSM_GetSMSFolders(gsm, &folders);
 		Print_Error(error);

 		error=GSM_GetSMS(gsm, &IncomingSMSData);
 		switch (error) {
 		case ERR_EMPTY:
 			printf(_("Location %i\n"),IncomingSMSData.SMS[0].Location);
 			printf("%s\n", _("Empty"));
 			break;
 		default:
 			Print_Error(error);
			PrintSMSLocation(&IncomingSMSData.SMS[0], &folders);
 		}
 	}
 	DisplayMultiSMSInfo(&IncomingSMSData,false,false,NULL);
 	wasincomingsms = false;
}

void IncomingCB(GSM_StateMachine *sm UNUSED, GSM_CBMessage CB, void *user_data)
{
	printf("%s\n", _("CB message received"));
	printf(_("Channel %i, text \"%s\"\n"),CB.Channel,DecodeUnicodeConsole(CB.Text));
}

void IncomingUSSD(GSM_StateMachine *sm UNUSED, GSM_USSDMessage ussd, void *user_data)
{
	printf("%s\n", _("USSD received"));
	printf(LISTFORMAT, _("Status"));
	switch(ussd.Status) {
		case USSD_NoActionNeeded:
			printf("%s\n", _("No action needed"));
			break;
		case USSD_ActionNeeded:
			printf("%s\n", _("Action needed"));
			break;
		case USSD_Terminated:
			printf("%s\n", _("Terminated"));
			break;
		case USSD_AnotherClient:
			printf("%s\n", _("Another client replied"));
			break;
		case USSD_NotSupported:
			printf("%s\n", _("Not supported"));
			break;
		case USSD_Timeout:
			printf("%s\n", _("Timeout"));
			break;
#ifndef CHECK_CASES
		default:
#endif
		case USSD_Unknown:
			printf("%s\n", _("Unknown"));
			break;
	}
	printf(LISTFORMAT "\"%s\"\n", _("Service reply"), DecodeUnicodeConsole(ussd.Text));
}

void IncomingUSSD2(GSM_StateMachine *sm, GSM_USSDMessage ussd, void * user_data)
{
	IncomingUSSD(sm, ussd, user_data);

	gshutdown = true;
}

void GetUSSD(int argc UNUSED, char *argv[])
{
	GSM_Error error;
	GSM_Init(true);

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));

	GSM_SetIncomingUSSDCallback(gsm, IncomingUSSD2, NULL);

	error=GSM_SetIncomingUSSD(gsm,true);
	Print_Error(error);

	error=GSM_DialService(gsm, argv[2]);
	/* Fallback to voice call, it can work with some phones */
	if (error == ERR_NOTIMPLEMENTED || error == ERR_NOTSUPPORTED) {
		error=GSM_DialVoice(gsm, argv[2], GSM_CALL_DefaultNumberPresence);
	}
	Print_Error(error);

	while (!gshutdown) GSM_ReadDevice(gsm, false);

	error=GSM_SetIncomingUSSD(gsm, false);
	Print_Error(error);

	GSM_Terminate();
}

void GetSMSC(int argc, char *argv[])
{
	GSM_SMSC 	smsc;
	GSM_Error error;
	int 		start, stop, i;

	if (argc == 2) {
		start = 1;
		stop = 1;
	} else {
		GetStartStop(&start, &stop, 2, argc, argv);
	}

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		smsc.Location=i;

		error=GSM_GetSMSC(gsm, &smsc);
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
			default:
				/* Typecasting is here needed to silent GCC warning.
				 * Validity usually fits in some unsigned type so it is always >= 0 */
				if ((int)smsc.Validity.Relative >= 0 && smsc.Validity.Relative <= 143) {
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

void GetSMS(int argc, char *argv[])
{
	GSM_Error error;
	GSM_MultiSMSMessage	sms;
	GSM_SMSFolders		folders;
	int			start, stop;
	int			j;

	GetStartStop(&start, &stop, 3, argc, argv);

	GSM_Init(true);

	error=GSM_GetSMSFolders(gsm, &folders);
	Print_Error(error);

	for (j = start; j <= stop; j++) {
		sms.SMS[0].Folder	= atoi(argv[2]);
		sms.SMS[0].Location	= j;
		error=GSM_GetSMS(gsm, &sms);
		switch (error) {
		case ERR_EMPTY:
			printf(_("Location %i\n"),sms.SMS[0].Location);
			printf("%s\n", _("Empty"));
			break;
		default:
			Print_Error(error);
			PrintSMSLocation(&sms.SMS[0], &folders);
			DisplayMultiSMSInfo(&sms,false,false,NULL);
		}
	}

	GSM_Terminate();
}

void DeleteSMS(int argc, char *argv[])
{
	GSM_Error error;
	GSM_SMSMessage	sms;
	int		start, stop, i;

	sms.Folder=atoi(argv[2]);

	GetStartStop(&start, &stop, 3, argc, argv);

	GSM_Init(true);

	for (i=start;i<=stop;i++) {
		sms.Location	= i;
		error=GSM_DeleteSMS(gsm, &sms);
		Print_Error(error);
	}
#ifdef GSM_ENABLE_BEEP
	GSM_PhoneBeep();
#endif
	GSM_Terminate();
}

void GetAllSMS(int argc, char *argv[])
{
	GSM_Error error;
	GSM_MultiSMSMessage 	sms;
	GSM_SMSFolders		folders;
	bool			start = true;
	int			smsnum=0,smspos=0;
	GSM_Backup		*BackupPtr = NULL;
#ifdef GSM_ENABLE_BACKUP
	int			used,i;
	GSM_MemoryStatus	MemStatus;
	GSM_MemoryEntry		Pbk;
	GSM_Backup		Backup;

	GSM_ClearBackup(&Backup);
	BackupPtr = &Backup;
#endif

	GSM_Init(true);

#ifdef GSM_ENABLE_BACKUP
	if (argc == 3 && strcasecmp(argv[2],"-pbk") == 0) {
		MemStatus.MemoryType = MEM_ME;
		error=GSM_GetMemoryStatus(gsm, &MemStatus);
		if (error==ERR_NONE && MemStatus.MemoryUsed != 0) {
			Pbk.MemoryType  = MEM_ME;
			i		= 1;
			used 		= 0;
			while (used != MemStatus.MemoryUsed) {
				Pbk.Location = i;
				error=GSM_GetMemory(gsm, &Pbk);
				if (error != ERR_EMPTY) {
					Print_Error(error);
					if (used < GSM_BACKUP_MAX_PHONEPHONEBOOK) {
						Backup.PhonePhonebook[used] = malloc(sizeof(GSM_MemoryEntry));
					        if (Backup.PhonePhonebook[used] == NULL) Print_Error(ERR_MOREMEMORY);
						Backup.PhonePhonebook[used+1] = NULL;
					} else {
						printf("\n   ");
						printf(_("Only part of data saved, please increase %s.") , "GSM_BACKUP_MAX_PHONEPHONEBOOK");
						printf("\n");
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

	error=GSM_GetSMSFolders(gsm, &folders);
	Print_Error(error);

	while (error == ERR_NONE) {
		sms.SMS[0].Folder=0x00;
		error=GSM_GetNextSMS(gsm, &sms, start);
		switch (error) {
		case ERR_EMPTY:
			break;
		case ERR_CORRUPTED:
			fprintf(stderr, "\n%s\n", _("Corrupted message, skipping"));
			error = ERR_NONE;
			continue;
		default:
			Print_Error(error);
			PrintSMSLocation(&sms.SMS[0], &folders);
			smspos++;
			smsnum+=sms.Number;
			DisplayMultiSMSInfo(&sms, false, false, BackupPtr);
		}
		start=false;
	}
	printf(_("\n\n%i SMS parts in %i SMS sequences\n"),smsnum,smspos);

#ifdef GSM_ENABLE_BEEP
	GSM_PhoneBeep();
#endif
	GSM_Terminate();
}

void GetEachSMS(int argc, char *argv[])
{
	GSM_Error error;
	GSM_MultiSMSMessage	*GetSMSData[GSM_PHONE_MAXSMSINFOLDER],*SortedSMS[GSM_PHONE_MAXSMSINFOLDER],sms;
	int			GetSMSNumber = 0,i,j;
	int			smsnum=0,smspos=0;
	GSM_SMSFolders		folders;
	bool			start = true, ems = true;
	GSM_Backup		*BackupPtr = NULL;
#ifdef GSM_ENABLE_BACKUP
	GSM_MemoryStatus	MemStatus;
	GSM_MemoryEntry		Pbk;
	int			used;
	GSM_Backup		Backup;

	GSM_ClearBackup(&Backup);
	BackupPtr = &Backup;
#endif

	GetSMSData[0] = NULL;

	GSM_Init(true);

#ifdef GSM_ENABLE_BACKUP
	if (argc == 3 && strcasecmp(argv[2],"-pbk") == 0) {
		MemStatus.MemoryType = MEM_ME;
		error=GSM_GetMemoryStatus(gsm, &MemStatus);
		if (error==ERR_NONE && MemStatus.MemoryUsed != 0) {
			Pbk.MemoryType  = MEM_ME;
			i		= 1;
			used 		= 0;
			while (used != MemStatus.MemoryUsed) {
				Pbk.Location = i;
				error=GSM_GetMemory(gsm, &Pbk);
				if (error != ERR_EMPTY) {
					Print_Error(error);
					if (used < GSM_BACKUP_MAX_PHONEPHONEBOOK) {
						Backup.PhonePhonebook[used] = malloc(sizeof(GSM_MemoryEntry));
					        if (Backup.PhonePhonebook[used] == NULL) Print_Error(ERR_MOREMEMORY);
						Backup.PhonePhonebook[used+1] = NULL;
					} else {
						printf("\n   ");
						printf(_("Only part of data saved, please increase %s.") , "GSM_BACKUP_MAX_PHONEPHONEBOOK");
						printf("\n");
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

	error=GSM_GetSMSFolders(gsm, &folders);
	Print_Error(error);

	fprintf(stderr, LISTFORMAT, _("Reading"));
	while (error == ERR_NONE) {
		if (GetSMSNumber==GSM_PHONE_MAXSMSINFOLDER-1) {
			fprintf(stderr, "\n%s\n", _("SMS counter overflow"));
			break;
		}
		sms.SMS[0].Folder=0x00;
		error=GSM_GetNextSMS(gsm, &sms, start);
		switch (error) {
		case ERR_EMPTY:
			break;
		case ERR_CORRUPTED:
			fprintf(stderr, "\n%s\n", _("Corrupted message, skipping"));
			error = ERR_NONE;
			continue;
		default:
			Print_Error(error);
			GetSMSData[GetSMSNumber] = malloc(sizeof(GSM_MultiSMSMessage));
		        if (GetSMSData[GetSMSNumber] == NULL) Print_Error(ERR_MOREMEMORY);
			GetSMSData[GetSMSNumber+1] = NULL;
			memcpy(GetSMSData[GetSMSNumber],&sms,sizeof(GSM_MultiSMSMessage));
			GetSMSNumber++;
		}
		fprintf(stderr,"*");
		start=false;
	}
	fprintf(stderr,"\n");

#ifdef GSM_ENABLE_BEEP
	GSM_PhoneBeep();
#endif

	error = GSM_LinkSMS(GSM_GetDebug(gsm), GetSMSData, SortedSMS, ems);
	Print_Error(error);

	i=0;
	while(GetSMSData[i] != NULL) {
		free(GetSMSData[i]);
		GetSMSData[i] = NULL;
		i++;
	}

	i=0;
	while(SortedSMS[i] != NULL) {
		smspos++;
		for (j=0;j<SortedSMS[i]->Number;j++) {
			smsnum++;
			if ((j==0) || (j!=0 && SortedSMS[i]->SMS[j].Location != SortedSMS[i]->SMS[j-1].Location)) {
				PrintSMSLocation(&SortedSMS[i]->SMS[j], &folders);
			}
		}
		DisplayMultiSMSInfo(SortedSMS[i], true, ems, BackupPtr);

		free(SortedSMS[i]);
		SortedSMS[i] = NULL;
		i++;
	}

	printf(_("\n%i SMS parts in %i SMS sequences\n"),smsnum,smspos);

	GSM_Terminate();
}

void GetSMSFolders(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_Error error;
	GSM_SMSFolders folders;
	int i;

	GSM_Init(true);

	error=GSM_GetSMSFolders(gsm,&folders);
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

void GetMMSFolders(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_Error error;
	GSM_MMSFolders folders;
	int i;

	GSM_Init(true);

	error=GSM_GetMMSFolders(gsm,&folders);
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
	int				i;
	size_t Pos;
	char				buff[200];
	GSM_EncodedMultiPartMMSInfo 	info;
	GSM_Error			error;
	FILE				*file2;
	GSM_MemoryEntry			pbk;
	GSM_CalendarEntry 		Calendar;
	GSM_ToDoEntry 			ToDo;

	if (num != -1 && answer_yes(_("Do you want to save this MMS file?"))) {
		sprintf(buff,"%i_0",num);
		file2 = fopen(buff,"wb");
		if (fwrite(file->Buffer, 1, file->Used, file2) != file->Used) {
			printf_err(_("Error while saving to file %s!\n"), buff);
		} else {
			printf(_("Saved to file %s\n"),buff);
		}
		fclose(file2);
	}

	for (i=0;i<GSM_MAX_MULTI_MMS;i++) info.Entries[i].File.Buffer = NULL;
	GSM_ClearMMSMultiPart(&info);

	error = GSM_DecodeMMSFileToMultiPart(GSM_GetDebug(gsm), file, &info);
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
			error = GSM_DecodeVCARD(GSM_GetDebug(gsm), info.Entries[i].File.Buffer, &Pos, &pbk, Nokia_VCard21);
			if (error == ERR_NONE) PrintMemoryEntry(&pbk);
		}
		if (!strcmp(DecodeUnicodeString(info.Entries[i].ContentType),"text/x-vCalendar")) {
			Pos = 0;
			printf("\n");
			error = GSM_DecodeVCALENDAR_VTODO(GSM_GetDebug(gsm), info.Entries[i].File.Buffer, &Pos, &Calendar, &ToDo, Nokia_VCalendar, Nokia_VToDo);
			if (error == ERR_NONE) PrintCalendar(&Calendar);
		}
		if (num != -1 && answer_yes(_("Do you want to save this attachment?"))) {
			sprintf(buff,"%i_%i_%s",num,i+1,DecodeUnicodeString(info.Entries[i].File.Name));
			file2 = fopen(buff,"wb");
			if (fwrite(info.Entries[i].File.Buffer, 1, info.Entries[i].File.Used, file2) != info.Entries[i].File.Used) {
				printf_err(_("Error while saving to file %s!\n"), buff);
			} else {
				printf(_("Saved to file %s\n"),buff);
			}
			fclose(file2);
		}

	}

	GSM_ClearMMSMultiPart(&info);
}

void GetEachMMS(int argc, char *argv[])
{
	int FileFolder;
	GSM_File		File;
	bool			start = true;
	GSM_MMSFolders 		folders;
	int			Handle,Size,num = -1;
	GSM_Error error;

	if (argc>2 && strcasecmp(argv[2],"-save") == 0) num=0;

	GSM_Init(true);

	error=GSM_GetMMSFolders(gsm,&folders);
	Print_Error(error);

	File.Buffer = NULL;

	while (1) {
		error = GSM_GetNextMMSFileInfo(gsm,File.ID_FullName,&FileFolder,start);
		if (error == ERR_EMPTY) break;
		Print_Error(error);
		start = false;

		printf(_("Folder %s\n"),DecodeUnicodeConsole(folders.Folder[FileFolder-1].Name));
		printf(LISTFORMAT "\"%s\"\n", _("  File filesystem ID"),DecodeUnicodeConsole(File.ID_FullName));
		if (File.Buffer != NULL) {
			free(File.Buffer);
			File.Buffer = NULL;
		}
		File.Used = 0;
		while (true) {
			error = GSM_GetFilePart(gsm,&File,&Handle,&Size);
			if (error == ERR_EMPTY) break;
			Print_Error(error);
			fprintf(stderr, "\r   ");
			fprintf(stderr, _("Reading: %i percent"), (int)(File.Used*100/Size));
		}
		fprintf(stderr, "%c",13);

		if (GSM_IsPhoneFeatureAvailable(GSM_GetModelInfo(gsm), F_SERIES40_30)) {
			memcpy(File.Buffer,File.Buffer+176,File.Used-176);
			File.Used-=176;
			File.Buffer = realloc(File.Buffer,File.Used);
		}

		DecodeMMSFile(&File,num);
		if (num != -1) num++;
	}

	if (File.Buffer != NULL) free(File.Buffer);

	GSM_Terminate();
}

void DisplaySMSFrame(GSM_SMSMessage *SMS)
{
	GSM_Error 		error;
	int			i, length, current = 0;
	unsigned char		req[1000], buffer[1000], hexreq[1000];
#ifdef OSCAR
        unsigned char           hexmsg[1000], hexudh[1000];
#endif
	error=PHONE_EncodeSMSFrame(gsm,SMS,buffer,PHONE_SMSSubmit,&length,true);
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

GSM_Error SMSStatus;

void SendSMSStatus (GSM_StateMachine *sm, int status, int MessageReference, void *user_data)
{
	smprintf(gsm, "Sent SMS on device: \"%s\"\n", GSM_GetConfig(sm, -1)->Device);
	if (status==0) {
		printf(_("..OK"));
		SMSStatus = ERR_NONE;
	} else {
		printf(_("..error %i"),status);
		SMSStatus = ERR_UNKNOWN;
	}
	printf(_(", message reference=%d\n"),MessageReference);
}

void SendSaveDisplaySMS(int argc, char *argv[])
{
	GSM_Error error;
#ifdef GSM_ENABLE_BACKUP
	GSM_Backup			Backup;
#endif
	int				i,j,z,FramesNum = 0;
	int				Protected = 0;
	GSM_SMSFolders			folders;
	GSM_MultiSMSMessage		sms;
	GSM_Ringtone			ringtone[GSM_MAX_MULTI_SMS];
	GSM_MultiBitmap			bitmap[GSM_MAX_MULTI_SMS],bitmap2;
	GSM_MultiPartSMSInfo		SMSInfo;
	GSM_NetworkInfo			NetInfo;
	GSM_MMSIndicator		MMSInfo;
	FILE 				*ReplaceFile,*f;
	char				ReplaceBuffer2	[200],ReplaceBuffer[200];
	char				InputBuffer	[SEND_SAVE_SMS_BUFFER_SIZE/2+1];
	char				Buffer		[GSM_MAX_MULTI_SMS][SEND_SAVE_SMS_BUFFER_SIZE];
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

	bool SaveSMS = false, SendSMS = false, DisplaySMS = false, SendSMSDSMS = false;

	if (strcasestr(argv[1], "savesms") != NULL) {
		SaveSMS = true;
	} else if (strcasestr(argv[1], "sendsmsdsms") != NULL) {
		SendSMSDSMS = true;
	} else if (strcasestr(argv[1], "sendsms") != NULL) {
		SendSMS = true;
	} else if (strcasestr(argv[1], "displaysms") != NULL) {
		DisplaySMS = true;
	} else {
		return;
	}

	ReplaceBuffer[0] = 0;
	ReplaceBuffer[1] = 0;
	GSM_ClearMultiPartSMSInfo(&SMSInfo);
	SMSInfo.ReplaceMessage		= 0;
	SMSInfo.EntriesNum		= 1;
	Validity.Format = 0;
	Validity.Relative = 0;

	if (SaveSMS) {
		EncodeUnicode(Sender,"Gammu",5);
		Name[0]  	= 0;
		Name[1]  	= 0;
		startarg 	= 0;
	} else {
		EncodeUnicode(Sender,argv[3],strlen(argv[3]));
		startarg 	= 1;
	}

	if (SendSMSDSMS) {
		startarg += 2;
		EncodeUnicode(SMSC,"1234",4);
		SMSCSet	= 0;
	}

	if (strcasecmp(argv[2],"TEXT") == 0) {
		if (isatty(fileno(stdin))) {
			printf("%s\n", _("Enter message text and press ^D:"));
		}
		chars_read = fread(InputBuffer, 1, SEND_SAVE_SMS_BUFFER_SIZE/2, stdin);
		if (chars_read == 0) printf_warn("%s\n", _("0 chars read!"));
		InputBuffer[chars_read] 		= 0x00;
		InputBuffer[chars_read+1] 		= 0x00;
		EncodeUnicode(Buffer[0],InputBuffer,chars_read);
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
		if (SaveSMS) {
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
		if (SaveSMS) {
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
		if (SaveSMS) {
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
		if (SaveSMS) {
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
		if (SaveSMS) {
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
		if (SaveSMS) {
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
		error=GSM_ReadBackupFile(argv[3+startarg],&Backup,GSM_GuessBackupFormat(argv[3+startarg], false));
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
		if (SaveSMS) {
			EncodeUnicode(Sender, "Bookmark",8);
			EncodeUnicode(Name,"WAP Bookmark",12);
		}
		startarg += 5;
	} else if (strcasecmp(argv[2],"WAPSETTINGS") == 0) {
		if (argc<6+startarg) {
			printf("%s\n", _("Where is backup filename and location?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[3+startarg],&Backup,GSM_GuessBackupFormat(argv[3+startarg], false));
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
		if (SaveSMS) {
			EncodeUnicode(Sender, "Settings",8);
			EncodeUnicode(Name,"WAP Settings",12);
		}
		startarg += 6;
	} else if (strcasecmp(argv[2],"MMSSETTINGS") == 0) {
		if (argc<5+startarg) {
			printf("%s\n", _("Where is backup filename and location?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[3+startarg],&Backup,GSM_GuessBackupFormat(argv[3+startarg], false));
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
		if (SaveSMS) {
			EncodeUnicode(Sender, "Settings",8);
			EncodeUnicode(Name,"MMS Settings",12);
		}
		startarg += 5;
	} else if (strcasecmp(argv[2],"CALENDAR") == 0) {
		if (argc<5+startarg) {
			printf("%s\n", _("Where is backup filename and location?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[3+startarg],&Backup,GSM_GuessBackupFormat(argv[3+startarg], false));
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
		if (SaveSMS) {
			EncodeUnicode(Sender, "Calendar",8);
		}
		startarg += 5;
	} else if (strcasecmp(argv[2],"TODO") == 0) {
		if (argc<5+startarg) {
			printf("%s\n", _("Where is backup filename and location?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[3+startarg],&Backup,GSM_GuessBackupFormat(argv[3+startarg], false));
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
		if (SaveSMS) {
			EncodeUnicode(Sender, "ToDo",8);
		}
		startarg += 5;
	} else if (strcasecmp(argv[2],"VCARD10") == 0 || strcasecmp(argv[2],"VCARD21") == 0) {
		if (argc<6+startarg) {
			printf("%s\n", _("Where is backup filename and location and memory type?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[3+startarg],&Backup,GSM_GuessBackupFormat(argv[3+startarg], false));
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
		if (SaveSMS) {
			EncodeUnicode(Sender, "VCARD",5);
			EncodeUnicode(Name, "Phonebook entry",15);
		}
		startarg += 6;
#endif
	} else if (strcasecmp(argv[2],"PROFILE") == 0) {
		SMSInfo.Entries[0].ID = SMS_NokiaProfileLong;
		if (SaveSMS) {
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
			if (SaveSMS || SendSaved) {
				if (strcasecmp(argv[i],"-folder") == 0) {
					nextlong=1;
					continue;
				}
			}
			if (SaveSMS) {
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
			if (SaveSMS) {
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
			if (fread(ReplaceBuffer,1,sizeof(ReplaceBuffer),ReplaceFile) != sizeof(ReplaceBuffer)) {
				printf_err(_("Error while writing file!\n"));
			}
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
			smprintf(gsm, "buffer is \"%s\"\n",DecodeUnicodeConsole(Buffer[SMSInfo.EntriesNum]));
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
			smprintf(gsm, "buffer is \"%s\"\n",DecodeUnicodeConsole(Buffer[SMSInfo.EntriesNum]));
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

	if (DisplaySMS || SendSMSDSMS) {
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
				error=GSM_GetNetworkInfo(gsm,&NetInfo);
				Print_Error(error);
				strcpy(bitmap[0].Bitmap[0].NetworkCode,NetInfo.NetworkCode);
				if (SaveSMS) {
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

	error=GSM_EncodeMultiPartSMS(GSM_GetDebug(gsm), &SMSInfo,&sms);
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
		GSM_Terminate();
		exit(-1);
	}

	if (DisplaySMS) {
		if (SMSCSet != 0) {
			printf("%s\n", _("Use -smscnumber option to give SMSC number"));
			GSM_Terminate();
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
	}
	if (SendSMSDSMS) {
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
		error = SMSDaemonSendSMS(argv[4],argv[5],&sms);
		Print_Error(error);
	}
	if (SaveSMS || SendSaved) {
		error=GSM_GetSMSFolders(gsm, &folders);
		Print_Error(error);

		if (SendSaved)	{
			if (Validity.Format != 0 && SMSCSet != 0) {
				PhoneSMSC.Location = SMSCSet;
				error=GSM_GetSMSC(gsm,&PhoneSMSC);
				Print_Error(error);
				CopyUnicodeString(SMSC,PhoneSMSC.Number);
				SMSCSet = 0;
			}

			GSM_SetSendSMSStatusCallback(gsm, SendSMSStatus, NULL);

			signal(SIGINT, interrupt);
			fprintf(stderr, "%s\n", _("If you want break, press Ctrl+C..."));
		}

		for (i=0;i<sms.Number;i++) {
			printf(_("Saving SMS %i/%i\n"),i+1,sms.Number);
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
			error=GSM_AddSMS(gsm, &sms.SMS[i]);
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
				error=GSM_SendSavedSMS(gsm, 0, sms.SMS[i].Location);
				Print_Error(error);
				printf(_("....waiting for network answer"));
				while (!gshutdown) {
					GSM_ReadDevice(gsm,true);
					if (SMSStatus == ERR_UNKNOWN) {
						GSM_Terminate();
						exit(-1);
					}
					if (SMSStatus == ERR_NONE) break;
				}
			}
		}
	}

	if (SendSMS) {
		if (Validity.Format != 0 && SMSCSet != 0) {
			PhoneSMSC.Location = SMSCSet;
			error=GSM_GetSMSC(gsm,&PhoneSMSC);
			Print_Error(error);
			CopyUnicodeString(SMSC,PhoneSMSC.Number);
			SMSCSet = 0;
		}

		signal(SIGINT, interrupt);
		fprintf(stderr, "%s\n", _("If you want break, press Ctrl+C..."));

		GSM_SetSendSMSStatusCallback(gsm, SendSMSStatus, NULL);

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
			error=GSM_SendSMS(gsm, &sms.SMS[i]);
			Print_Error(error);
			printf(_("....waiting for network answer"));
			fflush(stdout);
			while (!gshutdown) {
				GSM_ReadDevice(gsm,true);
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

void ReadMMSFile(int argc, char *argv[])
{
	GSM_File		File;
	int			num = -1;
	GSM_Error error;

	File.Buffer = NULL;
	error = GSM_ReadFile(argv[2], &File);
	Print_Error(error);

	if (argc>3 && strcasecmp(argv[3],"-save") == 0) num=0;

	DecodeMMSFile(&File,num);

	free(File.Buffer);
}

void AddSMSFolder(int argc UNUSED, char *argv[])
{
	unsigned char buffer[200];
	GSM_Error error;

	GSM_Init(true);

	EncodeUnicode(buffer,argv[2],strlen(argv[2]));
	error=GSM_AddSMSFolder(gsm,buffer);
	Print_Error(error);

	GSM_Terminate();
}

void DeleteAllSMS(int argc, char *argv[])
{
	GSM_MultiSMSMessage 	sms;
	GSM_SMSFolders		folders;
	int			foldernum;
	bool			start = true;
	GSM_Error error;

	GSM_Init(true);

	error=GSM_GetSMSFolders(gsm, &folders);
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
		error=GSM_GetNextSMS(gsm, &sms, start);
		switch (error) {
		case ERR_EMPTY:
			break;
		case ERR_CORRUPTED:
			fprintf(stderr, "\n%s\n", _("Corrupted message, skipping"));
			error = ERR_NONE;
			continue;
		default:
			Print_Error(error);
			if (sms.SMS[0].Folder == foldernum) {
				sms.SMS[0].Folder=0x00;
				error=GSM_DeleteSMS(gsm, &sms.SMS[0]);
				Print_Error(error);
				printf("*");
			}
		}
		start=false;
	}

	printf("\n");

	GSM_Terminate();
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */


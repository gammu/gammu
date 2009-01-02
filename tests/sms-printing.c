/* Code for printing SMSes used in tests */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../common/protocol/protocol.h" /* Needed for GSM_Protocol_Message */
#include "../common/gsmstate.h" /* Needed for state machine internals */
#include "../common/gsmphones.h" /* Phone data */

#include "sms-printing.h"

/* Parts of gammu/message.c {{{ */
void PrintSMSCNumber(unsigned char *number,GSM_Backup *Info)
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

void DisplaySingleSMSInfo(GSM_SMSMessage sms, bool displaytext, bool displayudh, GSM_Backup *Info)
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
		PrintSMSCNumber(sms.Number, Info);
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
		printf("%s\n", _("SMS deliver message"));
		if (sms.State == SMS_UnSent && sms.Memory==MEM_ME) {
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
		if (sms.PDU == SMS_Submit) {
			printf("%s\n", _("SMS submit message"));
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
		if (sms.Coding != 0) {
			printf(LISTFORMAT, _("Coding"));
			switch (sms.Coding) {
				case SMS_Coding_Unicode_No_Compression 	: printf("%s\n", _("Unicode (no compression)"));		break;
				case SMS_Coding_Unicode_Compression 	: printf("%s\n", _("Unicode (compression)"));			break;
				case SMS_Coding_Default_No_Compression 	: printf("%s\n", _("Default GSM alphabet (no compression)"));	break;
				case SMS_Coding_Default_Compression 	: printf("%s\n", _("Default GSM alphabet (compression)"));	break;
				case SMS_Coding_8bit			: printf("%s\n", _("8 bit"));					break;
			}
		}
		if (sms.State==SMS_UnSent && sms.Memory==MEM_ME) {
		} else {
			printf(LISTFORMAT, ngettext("Remote number", "Remote numbers", sms.OtherNumbersNum + 1));
			PrintSMSCNumber(sms.Number, Info);
			for (i=0;i<sms.OtherNumbersNum;i++) {
				printf(", ");
				PrintSMSCNumber(sms.OtherNumbers[i], Info);
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
				if (GSM_DecodeSiemensOTASMS(NULL, &SiemensOTA,&sms)) {
					printf("%s\n", _("Siemens file"));
					break;
				}
				printf("%s\n", _("8 bit SMS, cannot be displayed here"));
			}
		}
		break;
	}
}
/* }}} */

void DisplayTestSMS(GSM_MultiSMSMessage sms)
{
	int i, j;
	GSM_MultiPartSMSInfo	SMSInfo;

	/* Display parts */
	for (j=0;j<sms.Number;j++) {
		DisplaySingleSMSInfo(sms.SMS[j],true,true, NULL);
		printf("\n");
	}

	/* Decode it */
	if (GSM_DecodeMultiPartSMS(NULL, &SMSInfo,&sms,true)) {
		if (SMSInfo.Unknown) printf("%s\n\n", _("Some details were ignored (unknown or not implemented in decoding functions)"));

		for (i=0;i<SMSInfo.EntriesNum;i++) {
			switch (SMSInfo.Entries[i].ID) {
			case SMS_SiemensFile:
				printf(_("Siemens OTA file"));
				printf("\n");
				break;
			case SMS_NokiaRingtone:
				printf(_("Ringtone \"%s\"\n"),DecodeUnicodeConsole(SMSInfo.Entries[i].Ringtone->Name));
				GSM_SaveRingtoneRttl(stdout,SMSInfo.Entries[i].Ringtone);
				printf("\n");
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
				printf("%s\n", _("Not supported printing"));
				break;
#ifndef CHECK_CASES
			default:
#endif
				printf("%s: %d\n", _("Error"), SMSInfo.Entries[i].ID);
				exit(2);
				break;
			}
		}
		printf("\n");
	}

	/* Free allocated memory */
	GSM_FreeMultiPartSMSInfo(&SMSInfo);
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

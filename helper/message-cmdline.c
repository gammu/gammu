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

#include <gammu.h>

#include "../common/misc/locales.h"

#include "formats.h"
#include "printing.h"
#include "message-cmdline.h"

typedef enum {
	COMPOSE_ANIMATION = 1,
	COMPOSE_BOOKMARK,
	COMPOSE_CALENDAR,
	COMPOSE_CALLER,
	COMPOSE_EMS,
	COMPOSE_MMSINDICATOR,
	COMPOSE_MMSSETTINGS,
	COMPOSE_OPERATOR,
	COMPOSE_PICTURE,
	COMPOSE_PROFILE,
	COMPOSE_RINGTONE,
	COMPOSE_SMSTEMPLATE,
	COMPOSE_TEXT,
	COMPOSE_TODO,
	COMPOSE_VCARD10,
	COMPOSE_VCARD21,
	COMPOSE_WAPINDICATOR,
	COMPOSE_WAPSETTINGS
} ComposeType;

typedef struct {
	const char *name;
	const ComposeType type;
} ComposeMapEntry;

ComposeMapEntry ComposeMap[] = {
	{"ANIMATION", COMPOSE_ANIMATION},
	{"BOOKMARK", COMPOSE_BOOKMARK},
	{"CALENDAR", COMPOSE_CALENDAR},
	{"CALLER", COMPOSE_CALLER},
	{"EMS", COMPOSE_EMS},
	{"MMSINDICATOR", COMPOSE_MMSINDICATOR},
	{"MMSSETTINGS", COMPOSE_MMSSETTINGS},
	{"OPERATOR", COMPOSE_OPERATOR},
	{"PICTURE", COMPOSE_PICTURE},
	{"PROFILE", COMPOSE_PROFILE},
	{"RINGTONE", COMPOSE_RINGTONE},
	{"SMSTEMPLATE", COMPOSE_SMSTEMPLATE},
	{"TEXT", COMPOSE_TEXT},
	{"TODO", COMPOSE_TODO},
	{"VCARD10", COMPOSE_VCARD10},
	{"VCARD21", COMPOSE_VCARD21},
	{"WAPINDICATOR", COMPOSE_WAPINDICATOR},
	{"WAPSETTINGS", COMPOSE_WAPSETTINGS},
	{NULL, 0}
};


#define BMP_AUTO_ALLOC(n) \
	if (bitmap[n] == NULL) { \
		bitmap[n] = (GSM_MultiBitmap *)malloc(sizeof(GSM_MultiBitmap)); \
		if (bitmap[n] == NULL) { \
			error = ERR_MOREMEMORY; \
			goto end_compose; \
		} \
		memset(bitmap[n], 0, sizeof(GSM_MultiBitmap)); \
	}

#define RNG_AUTO_ALLOC(n) \
	if (ringtone[n] == NULL) { \
		ringtone[n] = (GSM_Ringtone *)malloc(sizeof(GSM_Ringtone)); \
		if (ringtone[n] == NULL) { \
			error = ERR_MOREMEMORY; \
			goto end_compose; \
		} \
		memset(ringtone[n], 0, sizeof(GSM_Ringtone)); \
	}

#define SEND_SAVE_SMS_BUFFER_SIZE 10000

GSM_Error CreateMessage(GSM_Message_Type *type, GSM_MultiSMSMessage *sms, int argc, int typearg, char *argv[], GSM_StateMachine *sm)
{
	/**
	 * Whether SMSC is set manually (0) or should be read from phone.
	 */
	int SMSCSet = 1;
	int startarg;

	GSM_Error error;
#ifdef GSM_ENABLE_BACKUP
	GSM_Backup			Backup;
#endif
	int				i,j,z,FramesNum = 0;
	int				Protected = 0;
	GSM_Ringtone			*ringtone[GSM_MAX_MULTI_SMS];
	GSM_MultiBitmap			*bitmap[GSM_MAX_MULTI_SMS];
	GSM_MultiBitmap			bitmap2;
	GSM_MultiPartSMSInfo		SMSInfo;
	GSM_NetworkInfo			NetInfo;
	GSM_MMSIndicator		MMSInfo;
	FILE 				*ReplaceFile,*f;
	char				ReplaceBuffer2	[200],ReplaceBuffer[200];
	char				InputBuffer	[SEND_SAVE_SMS_BUFFER_SIZE/2+1];
	unsigned char			Buffer		[GSM_MAX_MULTI_SMS][SEND_SAVE_SMS_BUFFER_SIZE];
	int				chars_read		= 0;
	int 				nextlong		= 0;
	bool				ReplyViaSameSMSC 	= false;
	int				MaxSMS			= -1;
	bool				EMS16Bit		= false;
	int frames_num;

	/* Parameters required only during saving */
	int				Folder			= 1; /*Inbox by default */
	GSM_SMS_State			State			= SMS_Sent;

	/* Required only during sending */
	GSM_SMSValidity			Validity;
	GSM_SMSC		    	PhoneSMSC;
	bool				DeliveryReport		= false;
	/* Whether we already got text for TEXT message */
	bool HasText = false;
	ComposeType compose_type = 0;
	/**
	 * Recipient or sender.
	 */
	unsigned char RemoteNumber[(GSM_MAX_NUMBER_LENGTH + 1) * 2];
	/**
	 * Name of message.
	 */
	unsigned char Name[(GSM_MAX_SMS_NAME_LENGTH + 1) * 2];
	/**
	 * SMSC to use for message.
	 */
	unsigned char SMSC[(GSM_MAX_NUMBER_LENGTH + 1) * 2];

	/* Some defaults */
	Name[0] = 0;
	Name[1] = 0;

	SMSC[0] = 0;
	SMSC[1] = 0;

	ReplaceBuffer[0] = 0;
	ReplaceBuffer[1] = 0;

	for (i = 0; i < GSM_MAX_MULTI_SMS; i++) {
		ringtone[i] = NULL;
		bitmap[i] = NULL;
	}

	EncodeUnicode(RemoteNumber, "Gammu", 5);

	GSM_ClearMultiPartSMSInfo(&SMSInfo);
	SMSInfo.ReplaceMessage		= 0;
	SMSInfo.EntriesNum		= 1;
	Validity.Format = 0;
	Validity.Relative = 0;


	if (*type == SMS_Save) {
		startarg = typearg + 1;
	} else {
		/* When not saving SMS, recipient has to be specified */
		if (typearg + 1 >= argc) {
			printf("%s\n", _("Not enough parameters!"));
			exit(-1);
		}
		EncodeUnicode(RemoteNumber, argv[typearg + 1], strlen(argv[typearg + 1]));
		startarg = typearg + 2;
	}

	if (*type == SMS_SMSD || *type == SMS_Display) {
		/*
		 * We don't care about SMSC number here, SMSD handles this
		 * itself and for displaying we don't really need a SMSC
		 * number.
		 */
		SMSCSet	= 0;
	}

	for (i = 0; ComposeMap[i].type != 0; i++) {
		if (strcasecmp(argv[typearg], ComposeMap[i].name) == 0) {
			compose_type = ComposeMap[i].type;
			break;
		}
	}

	if (compose_type == 0) {
		printf(_("What format of sms (\"%s\") ?\n"),argv[typearg]);
		exit(-1);
	}

	switch (compose_type) {
	case COMPOSE_TEXT:
		/* Text is fed to the buffer later! */
		SMSInfo.Entries[0].Buffer  		= Buffer[0];
		SMSInfo.Entries[0].ID			= SMS_Text;
		SMSInfo.UnicodeCoding   		= false;
		break;
	case COMPOSE_SMSTEMPLATE:
		SMSInfo.UnicodeCoding   		= false;
		SMSInfo.EntriesNum 			= 1;
		Buffer[0][0]				= 0x00;
		Buffer[0][1]				= 0x00;
		SMSInfo.Entries[0].Buffer  		= Buffer[0];
		SMSInfo.Entries[0].ID			= SMS_AlcatelSMSTemplateName;
		break;
	case COMPOSE_EMS:
		SMSInfo.UnicodeCoding   		= false;
		SMSInfo.EntriesNum 			= 0;
		break;
	case COMPOSE_MMSINDICATOR:
		if (argc < 3 + startarg) {
			printf("%s\n", _("Where are parameters?"));
			exit(-1);
		}
		SMSInfo.Entries[0].ID 	 	= SMS_MMSIndicatorLong;
		SMSInfo.Entries[0].MMSIndicator = &MMSInfo;
		if (*type == SMS_Save) {
			EncodeUnicode(RemoteNumber,"MMS Info",8);
		}
		strcpy(MMSInfo.Address,	argv[0 + startarg]);
		strcpy(MMSInfo.Title,	argv[1 + startarg]);
		strcpy(MMSInfo.Sender,	argv[2 + startarg]);
		startarg += 3;
		break;
	case COMPOSE_WAPINDICATOR:
		if (argc < 2 + startarg) {
			printf("%s\n", _("Where are parameters?"));
			exit(-1);
		}
		SMSInfo.Entries[0].ID 	 	= SMS_WAPIndicatorLong;
		SMSInfo.Entries[0].MMSIndicator = &MMSInfo;
		if (*type == SMS_Save) {
			EncodeUnicode(RemoteNumber,"WAP Info",8);
		}
		strcpy(MMSInfo.Address,	argv[0 + startarg]);
		strcpy(MMSInfo.Title,	argv[1 + startarg]);
		startarg += 2;
		break;
	case COMPOSE_RINGTONE:
		if (argc < 1 + startarg) {
			printf("%s\n", _("Where is ringtone filename?"));
			exit(-1);
		}
		RNG_AUTO_ALLOC(0);
		ringtone[0]->Format = RING_NOTETONE;
		error=GSM_ReadRingtoneFile(argv[startarg], ringtone[0]);
		if (error != ERR_NONE) goto end_compose;
		SMSInfo.Entries[0].ID 	 = SMS_NokiaRingtone;
		SMSInfo.Entries[0].Ringtone = ringtone[0];
		if (*type == SMS_Save) {
			CopyUnicodeString(RemoteNumber, ringtone[0]->Name);
			EncodeUnicode(Name,"Ringtone ",9);
			CopyUnicodeString(Name+9*2, ringtone[0]->Name);
		}
		startarg += 1;
		break;
	case COMPOSE_OPERATOR:
		if (argc < 1 + startarg) {
			printf("%s\n", _("Where is logo filename?"));
			exit(-1);
		}
		BMP_AUTO_ALLOC(0);
		bitmap[0]->Bitmap[0].Type = GSM_OperatorLogo;
		error=GSM_ReadBitmapFile(argv[startarg], bitmap[0]);
		if (error != ERR_NONE) goto end_compose;
		strcpy(bitmap[0]->Bitmap[0].NetworkCode,"000 00");
		SMSInfo.Entries[0].ID 	 = SMS_NokiaOperatorLogo;
		SMSInfo.Entries[0].Bitmap   = bitmap[0];
		if (*type == SMS_Save) {
			EncodeUnicode(RemoteNumber, "OpLogo",6);
			EncodeUnicode(Name,"OpLogo ",7);
		}
		startarg += 1;
		break;
	case COMPOSE_CALLER:
		if (argc < 1 + startarg) {
			printf("%s\n", _("Where is logo filename?"));
			exit(-1);
		}
		BMP_AUTO_ALLOC(0);
		bitmap[0]->Bitmap[0].Type = GSM_CallerGroupLogo;
		error=GSM_ReadBitmapFile(argv[startarg], bitmap[0]);
		if (error != ERR_NONE) goto end_compose;
		SMSInfo.Entries[0].ID 	    = SMS_NokiaCallerLogo;
		SMSInfo.Entries[0].Bitmap   = bitmap[0];
		if (*type == SMS_Save) {
			EncodeUnicode(RemoteNumber, "Caller",6);
		}
		startarg += 1;
		break;
	case COMPOSE_ANIMATION:
		SMSInfo.UnicodeCoding   		= false;
		SMSInfo.EntriesNum 			= 1;
		if (argc < 1 + startarg) {
			printf("%s\n", _("Where is number of frames?"));
			exit(-1);
		}
		BMP_AUTO_ALLOC(0);
		bitmap[0]->Number 		= 0;
		i				= 1;
		frames_num = atoi(argv[startarg]);
		if (frames_num > GSM_MAX_MULTI_BITMAP) {
			printf("%s\n", _("Too many animation frames!"));
			exit(-1);
		}
		while (1) {
			bitmap2.Bitmap[0].Type = GSM_StartupLogo;
			error=GSM_ReadBitmapFile(argv[startarg + i],&bitmap2);
			if (error != ERR_NONE) goto end_compose;
			for (j=0;j<bitmap2.Number;j++) {
				if (bitmap[0]->Number == frames_num)
					break;
				memcpy(&bitmap[0]->Bitmap[bitmap[0]->Number],&bitmap2.Bitmap[j],sizeof(GSM_Bitmap));
				bitmap[0]->Number++;
			}
			if (bitmap[0]->Number == frames_num)
				break;
			i++;
		}
		SMSInfo.Entries[0].ID  		= SMS_AlcatelMonoAnimationLong;
		SMSInfo.Entries[0].Bitmap   	= bitmap[0];
		bitmap[0]->Bitmap[0].Text[0]	= 0;
		bitmap[0]->Bitmap[0].Text[1]	= 0;
		startarg += 1 + frames_num;
		break;
	case COMPOSE_PICTURE:
		if (argc < 1 + startarg) {
			printf("%s\n", _("Where is logo filename?"));
			exit(-1);
		}
		BMP_AUTO_ALLOC(0);
		bitmap[0]->Bitmap[0].Type = GSM_PictureImage;
		error=GSM_ReadBitmapFile(argv[startarg], bitmap[0]);
		printf(_("File \"%s\"\n"),argv[startarg]);
		if (error != ERR_NONE) goto end_compose;
		SMSInfo.Entries[0].ID 	 	= SMS_NokiaPictureImageLong;
		SMSInfo.Entries[0].Bitmap   	= bitmap[0];
		SMSInfo.UnicodeCoding 		= false;
		bitmap[0]->Bitmap[0].Text[0]	= 0;
		bitmap[0]->Bitmap[0].Text[1]	= 0;
		if (*type == SMS_Save) {
			EncodeUnicode(RemoteNumber, "Picture",7);
			EncodeUnicode(Name,"Picture Image",13);
		}
		startarg += 1;
		break;
#ifdef GSM_ENABLE_BACKUP
	case COMPOSE_BOOKMARK:
		if (argc < 2 + startarg) {
			printf("%s\n", _("Where is backup filename and location?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[startarg],&Backup,GSM_GuessBackupFormat(argv[startarg], false));
		if (error != ERR_NONE && error != ERR_NOTIMPLEMENTED) goto end_compose;
		i = 0;
		while (Backup.WAPBookmark[i]!=NULL) {
			if (i == atoi(argv[1 + startarg])-1) break;
			i++;
		}
		if (i != atoi(argv[1 + startarg])-1) {
			printf("%s\n", _("Bookmark not found in file"));
			exit(-1);
		}
		SMSInfo.Entries[0].ID 	    = SMS_NokiaWAPBookmarkLong;
		SMSInfo.Entries[0].Bookmark = Backup.WAPBookmark[i];
		if (*type == SMS_Save) {
			EncodeUnicode(RemoteNumber, "Bookmark",8);
			EncodeUnicode(Name,"WAP Bookmark",12);
		}
		startarg += 2;
		break;
	case COMPOSE_WAPSETTINGS:
		if (argc < 3 + startarg) {
			printf("%s\n", _("Where is backup filename and location?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[startarg],&Backup,GSM_GuessBackupFormat(argv[startarg], false));
		if (error != ERR_NONE && error != ERR_NOTIMPLEMENTED) goto end_compose;
		i = 0;
		while (Backup.WAPSettings[i]!=NULL) {
			if (i == atoi(argv[1 + startarg])-1) break;
			i++;
		}
		if (i != atoi(argv[1 + startarg])-1 || Backup.WAPSettings[i] == NULL) {
			printf("%s\n", _("WAP settings not found in file"));
			exit(-1);
		}
		SMSInfo.Entries[0].Settings = NULL;
		for (j=0;j<Backup.WAPSettings[i]->Number;j++) {
			switch (Backup.WAPSettings[i]->Settings[j].Bearer) {
			case WAPSETTINGS_BEARER_GPRS:
				if (strcasecmp(argv[2 + startarg],"GPRS") == 0) {
					SMSInfo.Entries[0].Settings = &Backup.WAPSettings[i]->Settings[j];
					break;
				}
			case WAPSETTINGS_BEARER_DATA:
				if (strcasecmp(argv[2 + startarg],"DATA") == 0) {
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
		if (*type == SMS_Save) {
			EncodeUnicode(RemoteNumber, "Settings",8);
			EncodeUnicode(Name,"WAP Settings",12);
		}
		startarg += 3;
		break;
	case COMPOSE_MMSSETTINGS:
		if (argc < 2 + startarg) {
			printf("%s\n", _("Where is backup filename and location?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[startarg],&Backup,GSM_GuessBackupFormat(argv[startarg], false));
		if (error != ERR_NONE && error != ERR_NOTIMPLEMENTED) goto end_compose;
		i = 0;
		while (Backup.MMSSettings[i]!=NULL) {
			if (i == atoi(argv[1 + startarg])-1) break;
			i++;
		}
		if (i != atoi(argv[1 + startarg])-1 || Backup.WAPSettings[i] == NULL) {
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
		if (*type == SMS_Save) {
			EncodeUnicode(RemoteNumber, "Settings",8);
			EncodeUnicode(Name,"MMS Settings",12);
		}
		startarg += 2;
		break;
	case COMPOSE_CALENDAR:
		if (argc < 2 + startarg) {
			printf("%s\n", _("Where is backup filename and location?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[startarg],&Backup,GSM_GuessBackupFormat(argv[startarg], false));
		if (error != ERR_NONE && error != ERR_NOTIMPLEMENTED) goto end_compose;
		i = 0;
		while (Backup.Calendar[i]!=NULL) {
			if (i == atoi(argv[1 + startarg])-1) break;
			i++;
		}
		if (i != atoi(argv[1 + startarg])-1) {
			printf("%s\n", _("Calendar note not found in file"));
			exit(-1);
		}
		SMSInfo.Entries[0].ID 	    = SMS_NokiaVCALENDAR10Long;
		SMSInfo.Entries[0].Calendar = Backup.Calendar[i];
		if (*type == SMS_Save) {
			EncodeUnicode(RemoteNumber, "Calendar",8);
		}
		startarg += 2;
		break;
	case COMPOSE_TODO:
		if (argc < 2 + startarg) {
			printf("%s\n", _("Where is backup filename and location?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[startarg],&Backup,GSM_GuessBackupFormat(argv[startarg], false));
		if (error != ERR_NONE && error != ERR_NOTIMPLEMENTED) goto end_compose;
		i = 0;
		while (Backup.ToDo[i]!=NULL) {
			if (i == atoi(argv[1 + startarg])-1) break;
			i++;
		}
		if (i != atoi(argv[1 + startarg])-1) {
			printf("%s\n", _("ToDo note not found in file"));
			exit(-1);
		}
		SMSInfo.Entries[0].ID 	 	= SMS_NokiaVTODOLong;
		SMSInfo.Entries[0].ToDo 	= Backup.ToDo[i];
		if (*type == SMS_Save) {
			EncodeUnicode(RemoteNumber, "ToDo",8);
		}
		startarg += 2;
		break;
	case COMPOSE_VCARD10:
	case COMPOSE_VCARD21:
		if (argc < 3 + startarg) {
			printf("%s\n", _("Where is backup filename and location and memory type?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[startarg],&Backup,GSM_GuessBackupFormat(argv[startarg], false));
		if (error != ERR_NONE && error != ERR_NOTIMPLEMENTED) goto end_compose;
		i = 0;
		if (strcasecmp(argv[1 + startarg],"SM") == 0) {
			while (Backup.SIMPhonebook[i]!=NULL) {
				if (i == atoi(argv[2 + startarg])-1) break;
				i++;
			}
			if (i != atoi(argv[2 + startarg])-1) {
				printf("%s\n", _("Phonebook entry not found in file"));
				exit(-1);
			}
			SMSInfo.Entries[0].Phonebook = Backup.SIMPhonebook[i];
		} else if (strcasecmp(argv[1 + startarg],"ME") == 0) {
			while (Backup.PhonePhonebook[i]!=NULL) {
				if (i == atoi(argv[2 + startarg])-1) break;
				i++;
			}
			if (i != atoi(argv[2 + startarg])-1) {
				printf("%s\n", _("Phonebook entry not found in file"));
				exit(-1);
			}
			SMSInfo.Entries[0].Phonebook = Backup.PhonePhonebook[i];
		} else {
			printf(_("Unknown memory type: \"%s\"\n"),argv[1 + startarg]);
			exit(-1);
		}
		if (compose_type == COMPOSE_VCARD10) {
			SMSInfo.Entries[0].ID = SMS_VCARD10Long;
		} else {
			SMSInfo.Entries[0].ID = SMS_VCARD21Long;
		}
		if (*type == SMS_Save) {
			EncodeUnicode(RemoteNumber, "VCARD",5);
			EncodeUnicode(Name, "Phonebook entry",15);
		}
		startarg += 3;
		break;
#endif
	case COMPOSE_PROFILE:
		SMSInfo.Entries[0].ID = SMS_NokiaProfileLong;
		if (*type == SMS_Save) {
			EncodeUnicode(RemoteNumber, "Profile",7);
		}
		break;
	}

	for (i = startarg; i < argc; i++) {
		switch (nextlong) {
		case 0:
			if (*type == SMS_Save || *type == SMS_SendSaved) {
				if (strcasecmp(argv[i],"-folder") == 0) {
					nextlong=1;
					continue;
				}
			}
			if (*type == SMS_Save) {
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
					*type = SMS_SendSaved;
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
			if (compose_type == COMPOSE_RINGTONE) {
				if (strcasecmp(argv[i],"-long") == 0) {
					SMSInfo.Entries[0].ID = SMS_NokiaRingtoneLong;
					break;
				}
				if (strcasecmp(argv[i],"-scale") == 0) {
					RNG_AUTO_ALLOC(0);
					ringtone[0]->NoteTone.AllNotesScale=true;
					break;
				}
			}
			if (compose_type == COMPOSE_TEXT) {
				if (strcasecmp(argv[i],"-text") == 0) {
					nextlong = 26;
					break;
				}
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
			if (compose_type == COMPOSE_PICTURE) {
				if (strcasecmp(argv[i],"-text") == 0) {
					nextlong = 6;
					break;
				}
				if (strcasecmp(argv[i],"-unicode") == 0) {
					SMSInfo.UnicodeCoding = true;
					break;
				}
				if (strcasecmp(argv[i],"-alcatelbmmi") == 0) {
					BMP_AUTO_ALLOC(0);
					bitmap[0]->Bitmap[0].Type = GSM_StartupLogo;
					error = GSM_ReadBitmapFile(argv[startarg-1], bitmap[0]);
					if (error != ERR_NONE) goto end_compose;
					SMSInfo.UnicodeCoding = true;
					SMSInfo.Entries[0].ID = SMS_AlcatelMonoBitmapLong;
					break;
				}
				break;
			}
			if (compose_type == COMPOSE_VCARD10) {
				if (strcasecmp(argv[i],"-nokia") == 0) {
					SMSInfo.Entries[0].ID = SMS_NokiaVCARD10Long;
					break;
				}
				break;
			}
			if (compose_type == COMPOSE_VCARD21) {
				if (strcasecmp(argv[i],"-nokia") == 0) {
					SMSInfo.Entries[0].ID = SMS_NokiaVCARD21Long;
					break;
				}
				break;
			}
			if (compose_type == COMPOSE_PROFILE) {
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
			if (compose_type == COMPOSE_SMSTEMPLATE) {
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
					BMP_AUTO_ALLOC(SMSInfo.EntriesNum);
					bitmap[SMSInfo.EntriesNum]->Number 	= 0;
					nextlong = 16;
					break;
				}
			}
			if (compose_type == COMPOSE_EMS) {
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
					BMP_AUTO_ALLOC(SMSInfo.EntriesNum);
					bitmap[SMSInfo.EntriesNum]->Number 	= 0;
					nextlong = 16;
					break;
				}
			}
			if (compose_type == COMPOSE_OPERATOR) {
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
		case 2: /* Remote number - only during saving SMS */
			EncodeUnicode(RemoteNumber,argv[i],strlen(argv[i]));
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
			BMP_AUTO_ALLOC(0);
			EncodeUnicode(bitmap[0]->Bitmap[0].Text,argv[i],strlen(argv[i]));
			nextlong = 0;
			break;
		case 7:	/* Operator Logo - network code */
			BMP_AUTO_ALLOC(0);
			strncpy(bitmap[0]->Bitmap[0].NetworkCode,argv[i],7);
			if (!strcmp(DecodeUnicodeConsole(GSM_GetNetworkName(bitmap[0]->Bitmap[0].NetworkCode)),"unknown")) {
				printf(_("Unknown GSM network code (\"%s\")\n"),argv[i]);
				exit(-1);
			}
			if (*type == SMS_Save) {
				EncodeUnicode(RemoteNumber, "OpLogo",6);
				EncodeUnicode(RemoteNumber+6*2,bitmap[0]->Bitmap[0].NetworkCode,3);
				EncodeUnicode(RemoteNumber+6*2+3*2,bitmap[0]->Bitmap[0].NetworkCode+4,2);
				if (UnicodeLength(GSM_GetNetworkName(bitmap[0]->Bitmap[0].NetworkCode))<GSM_MAX_SMS_NAME_LENGTH-7) {
					EncodeUnicode(Name,"OpLogo ",7);
					CopyUnicodeString(Name+7*2,GSM_GetNetworkName(bitmap[0]->Bitmap[0].NetworkCode));
				} else {
					CopyUnicodeString(Name,RemoteNumber);
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
			if (ReplaceFile == NULL) {
				error = ERR_CANTOPENFILE;
				goto end_compose;
			}
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
			RNG_AUTO_ALLOC(SMSInfo.EntriesNum);
			ringtone[SMSInfo.EntriesNum]->Format = RING_NOTETONE;
			error=GSM_ReadRingtoneFile(argv[i], ringtone[SMSInfo.EntriesNum]);
			if (error != ERR_NONE) goto end_compose;
			SMSInfo.Entries[SMSInfo.EntriesNum].Ringtone = ringtone[SMSInfo.EntriesNum];
			SMSInfo.EntriesNum++;
			nextlong = 0;
			break;
		case 15:/* EMS bitmap file */
			BMP_AUTO_ALLOC(SMSInfo.EntriesNum);
			bitmap[SMSInfo.EntriesNum]->Bitmap[0].Type = GSM_StartupLogo;
			error=GSM_ReadBitmapFile(argv[i], bitmap[SMSInfo.EntriesNum]);
			if (error != ERR_NONE) goto end_compose;
			SMSInfo.Entries[SMSInfo.EntriesNum].Bitmap = bitmap[SMSInfo.EntriesNum];
			SMSInfo.EntriesNum++;
			nextlong = 0;
			break;
		case 16:/* Number of frames for EMS animation */
			FramesNum = atoi(argv[i]);
			if (FramesNum < 1 || FramesNum > 4) {
				printf(_("You have to give number of EMS frames between 1 and 4 (\"%s\")\n"),argv[i]);
				exit(-1);
			}
			BMP_AUTO_ALLOC(SMSInfo.EntriesNum);
			bitmap[SMSInfo.EntriesNum]->Number = 0;
			nextlong = 17;
			break;
		case 17:/*File for EMS animation */
			BMP_AUTO_ALLOC(SMSInfo.EntriesNum);
			bitmap2.Bitmap[0].Type=GSM_StartupLogo;
			error=GSM_ReadBitmapFile(argv[i],&bitmap2);
			for (j=0;j<bitmap2.Number;j++) {
				if (bitmap[SMSInfo.EntriesNum]->Number == FramesNum) break;
				memcpy(&bitmap[SMSInfo.EntriesNum]->Bitmap[bitmap[SMSInfo.EntriesNum]->Number],&bitmap2.Bitmap[j],sizeof(GSM_Bitmap));
				bitmap[SMSInfo.EntriesNum]->Number++;
			}
			if (bitmap[SMSInfo.EntriesNum]->Number == FramesNum) {
				SMSInfo.Entries[SMSInfo.EntriesNum].Bitmap = bitmap[SMSInfo.EntriesNum];
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
			RNG_AUTO_ALLOC(0);
			ringtone[0]->Format	    = RING_NOTETONE;
			error=GSM_ReadRingtoneFile(argv[i], ringtone[0]);
			if (error != ERR_NONE) goto end_compose;
			SMSInfo.Entries[0].Ringtone = ringtone[0];
			nextlong 		    = 0;
			break;
		case 24:/* profile bitmap */
			BMP_AUTO_ALLOC(0);
			bitmap[0]->Bitmap[0].Type    = GSM_PictureImage;
			error=GSM_ReadBitmapFile(argv[i], bitmap[0]);
			if (error != ERR_NONE) goto end_compose;
			bitmap[0]->Bitmap[0].Text[0] = 0;
			bitmap[0]->Bitmap[0].Text[1] = 0;
			SMSInfo.Entries[0].Bitmap   = bitmap[0];
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
		case 26:/* text from parameter */
			EncodeUnicode(Buffer[0],argv[i],strlen(argv[i]));
			HasText = true;
			nextlong = 0;
			break;
		}
	}
	if (nextlong!=0) {
		printf_err("%s\n", _("Parameter missing!"));
		exit(-1);
	}

	if (compose_type == COMPOSE_EMS && EMS16Bit) {
		for (i=0;i<SMSInfo.EntriesNum;i++) {
			switch (SMSInfo.Entries[i].ID) {
			case SMS_ConcatenatedTextLong:
				SMSInfo.Entries[i].ID = SMS_ConcatenatedTextLong16bit;
			default:
				break;

			}
		}

	}
	if (compose_type == COMPOSE_TEXT) {
		if (! HasText) {
			if (isatty(fileno(stdin))) {
				printf("%s\n", _("Enter message text and press ^D:"));
			}
			chars_read = fread(InputBuffer, 1, SEND_SAVE_SMS_BUFFER_SIZE/2, stdin);
			/* Zero terminate string */
			InputBuffer[chars_read] = 0;
			/* Trim \n at the end of string */
			if (InputBuffer[chars_read - 1] == '\n') {
				chars_read--;
				InputBuffer[chars_read] = 0;
			}
			/* Warn on no input */
			if (chars_read == 0) {
				printf_warn("%s\n", _("No chars read, assuming it is okay!"));
			}

			EncodeUnicode(Buffer[0],InputBuffer,chars_read);
		}

		chars_read = UnicodeLength(Buffer[0]);

		if (chars_read != 0) {
			/* Trim \n at the end of string */
			if (Buffer[0][chars_read*2-1] == '\n' && Buffer[0][chars_read*2-2] == 0)
			{
				Buffer[0][chars_read*2-1] = 0;
			}
		}
	}

	if (*type == SMS_Display || *type == SMS_SMSD) {
		if (compose_type == COMPOSE_OPERATOR) {
			if (bitmap[0]->Bitmap[0].Type==GSM_OperatorLogo && strcmp(bitmap[0]->Bitmap[0].NetworkCode,"000 00")==0) {
				printf("%s\n", _("No network code"));
				exit(-1);
			}
		}
	} else {
		if (sm == NULL) {
			printf_err("%s\n", _("You have to set network code!"));
			exit(-1);
		}
		if (compose_type == COMPOSE_OPERATOR) {
			if (bitmap[0]->Bitmap[0].Type == GSM_OperatorLogo &&
					strcmp(bitmap[0]->Bitmap[0].NetworkCode,"000 00")==0) {
				error = GSM_GetNetworkInfo(sm,&NetInfo);
				if (error != ERR_NONE) goto end_compose;
				strcpy(bitmap[0]->Bitmap[0].NetworkCode,NetInfo.NetworkCode);
				if (*type == SMS_Save) {
					EncodeUnicode(RemoteNumber, "OpLogo",6);
					EncodeUnicode(RemoteNumber+6*2,bitmap[0]->Bitmap[0].NetworkCode,3);
					EncodeUnicode(RemoteNumber+6*2+3*2,bitmap[0]->Bitmap[0].NetworkCode+4,2);
					if (UnicodeLength(GSM_GetNetworkName(bitmap[0]->Bitmap[0].NetworkCode))<GSM_MAX_SMS_NAME_LENGTH-7) {
						EncodeUnicode(Name,"OpLogo ",7);
						CopyUnicodeString(Name+7*2,GSM_GetNetworkName(bitmap[0]->Bitmap[0].NetworkCode));
					} else {
						CopyUnicodeString(Name,RemoteNumber);
					}
				}
			}
		}
	}

	error = GSM_EncodeMultiPartSMS(GSM_GetGlobalDebug(), &SMSInfo, sms);
	if (error != ERR_NONE) goto end_compose;

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

	/* Check whether we did not reach user limit of messages */
	if (MaxSMS != -1 && sms->Number > MaxSMS) {
		printf_err(_("There is %i SMS packed and %i limit. Exiting\n"),sms->Number,MaxSMS);
		error = ERR_MOREMEMORY;
		goto end_compose;
	}

	/*
	 * Are we supposed to read SMSC from phone?
	 */
	if (SMSCSet != 0) {
		if (sm == NULL) {
			printf_err("%s\n", _("Use -smscnumber option to give SMSC number"));
			exit(-1);
		}

		PhoneSMSC.Location = SMSCSet;
		error = GSM_GetSMSC(sm, &PhoneSMSC);
		if (error != ERR_NONE) goto end_compose;

		CopyUnicodeString(SMSC, PhoneSMSC.Number);
		SMSCSet = 0;
	}

	/*
	 * Fill up all messages with flags.
	 */
	for (i=0;i<sms->Number;i++) {
		CopyUnicodeString(sms->SMS[i].Number, RemoteNumber);
		CopyUnicodeString(sms->SMS[i].Name, Name);

		sms->SMS[i].Location = 0;
		sms->SMS[i].Folder = Folder;
		sms->SMS[i].State = State;

		sms->SMS[i].ReplyViaSameSMSC = ReplyViaSameSMSC;

		sms->SMS[i].SMSC.Location = SMSCSet;
		CopyUnicodeString(sms->SMS[i].SMSC.Number, SMSC);

		if (DeliveryReport) {
			sms->SMS[i].PDU	= SMS_Status_Report;
		} else {
			sms->SMS[i].PDU = SMS_Submit;
		}

		if (Validity.Format != 0) {
			sms->SMS[i].SMSC.Validity = Validity;
		}
	}

	error = ERR_NONE;
end_compose:

	for (i = 0; i < GSM_MAX_MULTI_SMS; i++) {
		free(ringtone[i]);
		free(bitmap[i]);
	}

	return error;
}


/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */


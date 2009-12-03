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

#include "../helper/locales.h"

#include "common.h"
#include "message.h"
#include "memory.h"
#include "../helper/formats.h"
#include "calendar.h"
#include "misc.h"

#include "../helper/message-display.h"
#include "../helper/memory-display.h"
#include "../helper/message-cmdline.h"
#include "../helper/printing.h"
#include "../helper/string.h"

volatile gboolean 			wasincomingsms 		= FALSE;

GSM_MultiSMSMessage		IncomingSMSData;

void IncomingSMS(GSM_StateMachine *sm UNUSED, GSM_SMSMessage sms, void *user_data)
{
	printf("%s\n", _("SMS message received"));
	fflush(stdout);

 	if (wasincomingsms) {
 		printf("%s\n", _("We already have one pending, ignoring this one!"));
		fflush(stdout);
 		return;
 	}
 	wasincomingsms = TRUE;
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
			fflush(stdout);
 			break;
 		default:
 			Print_Error(error);
			PrintSMSLocation(&IncomingSMSData.SMS[0], &folders);
 		}
 	}
 	DisplayMultiSMSInfo(&IncomingSMSData,FALSE,FALSE,NULL, gsm);
 	wasincomingsms = FALSE;
}

void IncomingCB(GSM_StateMachine *sm UNUSED, GSM_CBMessage CB, void *user_data)
{
	printf("%s\n", _("CB message received"));
	printf(_("Channel %i, text \"%s\"\n"),CB.Channel,DecodeUnicodeConsole(CB.Text));
	fflush(stdout);
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
	fflush(stdout);
}

void IncomingUSSD2(GSM_StateMachine *sm, GSM_USSDMessage ussd, void * user_data)
{
	IncomingUSSD(sm, ussd, user_data);

	gshutdown = TRUE;
}

void GetUSSD(int argc UNUSED, char *argv[])
{
	GSM_Error error;
	GSM_Init(TRUE);

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));
	fflush(stderr);

	GSM_SetIncomingUSSDCallback(gsm, IncomingUSSD2, NULL);

	error=GSM_SetIncomingUSSD(gsm,TRUE);
	Print_Error(error);

	error=GSM_DialService(gsm, argv[2]);
	/* Fallback to voice call, it can work with some phones */
	if (error == ERR_NOTIMPLEMENTED || error == ERR_NOTSUPPORTED) {
		error=GSM_DialVoice(gsm, argv[2], GSM_CALL_DefaultNumberPresence);
	}
	Print_Error(error);

	while (!gshutdown) GSM_ReadDevice(gsm, FALSE);

	error=GSM_SetIncomingUSSD(gsm, FALSE);
	Print_Error(error);

	GSM_Terminate();
}

void SetSMSC(int argc, char *argv[])
{
	GSM_Error error;
	GSM_SMSC smsc;

	smsc.Location = atoi(argv[2]);
	if (smsc.Location < 1) {
		printf_err(_("Invalid SMSC location: %s\n"), argv[2]);
		return;
	}
	GSM_Init(TRUE);

	error = GSM_GetSMSC(gsm, &smsc);
	Print_Error(error);

	EncodeUnicode(smsc.Number, argv[3], strlen(argv[3]));

	error = GSM_SetSMSC(gsm, &smsc);
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

	GSM_Init(TRUE);

	for (i=start;i<=stop;i++) {
		smsc.Location=i;

		error=GSM_GetSMSC(gsm, &smsc);
		Print_Error(error);

		printf(LISTFORMAT "%d\n", _("Location"), smsc.Location);

		if (UnicodeLength(smsc.Name) != 0) {
			printf(LISTFORMAT "\"%s\"\n", _("Name"),DecodeUnicodeConsole(smsc.Name));
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
		fflush(stdout);
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

	GSM_Init(TRUE);

	error=GSM_GetSMSFolders(gsm, &folders);
	Print_Error(error);

	for (j = start; j <= stop; j++) {
		sms.SMS[0].Folder	= atoi(argv[2]);
		sms.SMS[0].Location	= j;
		sms.Number = 0;
		error=GSM_GetSMS(gsm, &sms);
		switch (error) {
		case ERR_EMPTY:
			printf(_("Location %i\n"),sms.SMS[0].Location);
			printf("%s\n", _("Empty"));
			break;
		default:
			Print_Error(error);
			PrintSMSLocation(&sms.SMS[0], &folders);
			DisplayMultiSMSInfo(&sms,FALSE,FALSE,NULL, gsm);
		}
	}
	fflush(stdout);
	GSM_Terminate();
}

void DeleteSMS(int argc, char *argv[])
{
	GSM_Error error;
	GSM_SMSMessage	sms;
	int		start, stop, i;

	sms.Folder=atoi(argv[2]);

	GetStartStop(&start, &stop, 3, argc, argv);

	GSM_Init(TRUE);

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
	gboolean			start = TRUE;
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

	sms.Number = 0;
	sms.SMS[0].Location = 0;

	GSM_Init(TRUE);

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
						fflush(stdout);
						break;
					}
					*Backup.PhonePhonebook[used]=Pbk;
					used++;
				}
				fprintf(stderr, "\r");
				fprintf(stderr, "%s ", _("Reading phone phonebook:"));
				fprintf(stderr, _("%i percent"),
					used * 100 / MemStatus.MemoryUsed);
				i++;

				if (gshutdown) {
					GSM_Terminate();
					Terminate(4);
				}
			}
			fprintf(stderr, "\n");
			fflush(stderr);
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
			fflush(stderr);
			error = ERR_NONE;
			continue;
		default:
			Print_Error(error);
			PrintSMSLocation(&sms.SMS[0], &folders);
			smspos++;
			smsnum+=sms.Number;
			DisplayMultiSMSInfo(&sms, FALSE, FALSE, BackupPtr, gsm);
		}
		start=FALSE;
	}
	printf("\n\n");
	printf(_("%i SMS parts in %i SMS sequences"),smsnum,smspos);
	printf("\n");
	fflush(stdout);

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
	gboolean			start = TRUE, ems = TRUE;
	GSM_Backup		*BackupPtr = NULL;
#ifdef GSM_ENABLE_BACKUP
	GSM_MemoryStatus	MemStatus;
	GSM_MemoryEntry		Pbk;
	int			used;
	GSM_Backup		Backup;

	GSM_ClearBackup(&Backup);
	BackupPtr = &Backup;
#endif

	sms.Number = 0;
	sms.SMS[0].Location = 0;

	GetSMSData[0] = NULL;

	GSM_Init(TRUE);

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
						fflush(stdout);
						break;
					}
					*Backup.PhonePhonebook[used]=Pbk;
					used++;
				}
				fprintf(stderr, "\r");
				fprintf(stderr, "%s ", _("Reading phone phonebook:"));
				fprintf(stderr, _("%i percent"),
					used * 100 / MemStatus.MemoryUsed);
				fflush(stderr);
				i++;

				if (gshutdown) {
					GSM_Terminate();
					Terminate(4);
				}
			}
			fprintf(stderr, "\n");
			fflush(stderr);
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
		fflush(stderr);
		start=FALSE;
	}
	fprintf(stderr,"\n");
	fflush(stderr);

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
		DisplayMultiSMSInfo(SortedSMS[i], TRUE, ems, BackupPtr, gsm);

		free(SortedSMS[i]);
		SortedSMS[i] = NULL;
		i++;
	}

	printf("\n");
	printf(_("%i SMS parts in %i SMS sequences"),smsnum,smspos);
	printf("\n");
	fflush(stdout);
	GSM_Terminate();
}

void GetSMSFolders(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_Error error;
	GSM_SMSFolders folders;
	int i;

	GSM_Init(TRUE);

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
	fflush(stdout);
	GSM_Terminate();
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
	fflush(stdout);
}

void SendSaveDisplaySMS(int argc, char *argv[])
{
	GSM_Error error;
	GSM_MultiSMSMessage sms;
	GSM_Message_Type type;
	GSM_SMSFolders folders;
	int i;

	if (strcasestr(argv[1], "savesms") != NULL) {
		type = SMS_Save;
	} else if (strcasestr(argv[1], "sendsmsdsms") != NULL) {
		type = SMS_SMSD;
	} else if (strcasestr(argv[1], "sendsms") != NULL) {
		type = SMS_Send;
	} else if (strcasestr(argv[1], "displaysms") != NULL) {
		type = SMS_Display;
	} else {
		return;
	}

	GSM_Init(TRUE);

	error = CreateMessage(&type, &sms, argc, 2, argv, gsm);
	Print_Error(error);

	switch (type) {
		case SMS_Display:
			for (i = 0; i < sms.Number; i++) {
				printf(LISTFORMAT "%i\n", _("Message number"), i);
				DisplaySMSFrame(&sms.SMS[i], gsm);
			}

			printf(LISTFORMAT "%i\n", _("Number of messages"), sms.Number);
			break;
		case SMS_Save:
		case SMS_SendSaved:
			error = GSM_GetSMSFolders(gsm, &folders);
			Print_Error(error);

			if (type == SMS_SendSaved) {
				GSM_SetSendSMSStatusCallback(gsm, SendSMSStatus, NULL);

				signal(SIGINT, interrupt);
				fprintf(stderr, "%s\n", _("If you want break, press Ctrl+C..."));
				fflush(stderr);
			}

			for (i=0;i<sms.Number;i++) {
				printf(_("Saving SMS %i/%i\n"),i+1,sms.Number);
				error = GSM_AddSMS(gsm, &sms.SMS[i]);
				Print_Error(error);
				printf(_("Saved in folder number %d \"%s\", location %i"),
					sms.SMS[i].Folder,
					DecodeUnicodeConsole(folders.Folder[sms.SMS[i].Folder-1].Name),
					sms.SMS[i].Location);
				if (sms.SMS[i].Memory == MEM_SM) {
					printf(", %s\n", _("SIM"));
				} else {
					printf(", %s\n", _("phone"));
				}

				if (type == SMS_SendSaved) {
					printf(_("Sending sms from folder \"%s\", location %i\n"),
						DecodeUnicodeString(folders.Folder[sms.SMS[i].Folder-1].Name),sms.SMS[i].Location);
					fflush(stdout);
					SMSStatus = ERR_TIMEOUT;
					error = GSM_SendSavedSMS(gsm, 0, sms.SMS[i].Location);
					Print_Error(error);
					printf(_("....waiting for network answer"));
					fflush(stdout);

					while (!gshutdown) {
						GSM_ReadDevice(gsm,TRUE);
						if (SMSStatus == ERR_UNKNOWN) {
							Print_Error(SMSStatus);
						}
						if (SMSStatus == ERR_NONE) break;
					}
				}
			}
			break;
		case SMS_Send:
			signal(SIGINT, interrupt);
			fprintf(stderr, "%s\n", _("If you want break, press Ctrl+C..."));
			fflush(stderr);

			GSM_SetSendSMSStatusCallback(gsm, SendSMSStatus, NULL);

			for (i=0;i<sms.Number;i++) {
				printf(_("Sending SMS %i/%i"),i+1,sms.Number);
				fflush(stdout);
				SMSStatus = ERR_TIMEOUT;
				error=GSM_SendSMS(gsm, &sms.SMS[i]);
				Print_Error(error);
				printf(_("....waiting for network answer"));
				fflush(stdout);

				while (!gshutdown) {
					GSM_ReadDevice(gsm,TRUE);

					if (SMSStatus == ERR_UNKNOWN) {
						Print_Error(SMSStatus);
					}
					if (SMSStatus == ERR_NONE) break;
				}
			}
			break;
		default:
			printf_err(_("Something went wrong, uknown message operation!\n"));
	}

	GSM_Terminate();
}

void AddSMSFolder(int argc UNUSED, char *argv[])
{
	unsigned char buffer[200];
	GSM_Error error;

	GSM_Init(TRUE);

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
	gboolean			start = TRUE;
	GSM_Error error;

	GSM_Init(TRUE);

	error=GSM_GetSMSFolders(gsm, &folders);
	Print_Error(error);

	GetStartStop(&foldernum, NULL, 2, argc, argv);

	if (foldernum > folders.Number) {
		printf(_("Too high folder number (max. %i)\n"),folders.Number);
		fflush(stdout);
		GSM_Terminate();
		Terminate(2);
	}

	printf(_("Deleting SMS from \"%s\" folder: "),DecodeUnicodeConsole(folders.Folder[foldernum-1].Name));
	fflush(stdout);

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
		start=FALSE;
	}

	printf("\n");
	fflush(stdout);

	GSM_Terminate();
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

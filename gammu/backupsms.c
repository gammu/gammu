#include "../helper/locales.h"

#include <gammu.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "backup.h"
#include "memory.h"
#include "message.h"
#include "common.h"
#include "../helper/formats.h"

#include "../helper/message-display.h"
#include "../helper/printing.h"
#include "../helper/string.h"

void BackupSMS(int argc UNUSED, char *argv[])
{
	GSM_Error error;
	GSM_SMS_Backup		Backup;
	GSM_MultiSMSMessage 	sms;
	GSM_SMSFolders		folders;
	gboolean			BackupFromFolder[GSM_MAX_SMS_FOLDERS];
	gboolean			start = TRUE;
	gboolean			DeleteAfter = FALSE, askdelete = TRUE;
	int			j, smsnum = 0;

	if (argc == 4) {
		if (strcasecmp(argv[3],"-yes") == 0) {
			always_answer_yes = TRUE;
		}
		if (strcasecmp(argv[3],"-all") == 0) {
			always_answer_yes = TRUE;
			askdelete = FALSE;
			DeleteAfter = FALSE;
		}
	}

	GSM_Init(TRUE);

	Backup.SMS[0] = NULL;
	sms.SMS[0].Location = 0;
	sms.Number = 0;

	error=GSM_GetSMSFolders(gsm, &folders);
	Print_Error(error);

	if (askdelete) {
		DeleteAfter = answer_yes(_("Delete each sms after backup?"));
	}

	for (j=0;j<folders.Number;j++) {
		BackupFromFolder[j] = FALSE;
		if (answer_yes(_("Backup sms from folder \"%s\"%s?"),
				DecodeUnicodeConsole(folders.Folder[j].Name),
				folders.Folder[j].Memory == MEM_SM ? _(" (SIM)") : ""
				))
			BackupFromFolder[j] = TRUE;
	}

	while (error == ERR_NONE) {
		sms.SMS[0].Folder=0x00;
		error=GSM_GetNextSMS(gsm, &sms, start);
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
							Terminate(3);
						}
						*Backup.SMS[smsnum] = sms.SMS[j];
						smsnum++;
						break;
					}
				}
			}
		}
		start=FALSE;
	}

	error = GSM_AddSMSBackupFile(argv[2],&Backup);
	Print_Error(error);

	if (DeleteAfter) {
		for (j=0;j<smsnum;j++) {
			Backup.SMS[j]->Folder = 0;
			error=GSM_DeleteSMS(gsm, Backup.SMS[j]);
			Print_Error(error);
			fprintf(stderr, "\r");
			fprintf(stderr, "%s ", _("Deleting:"));
			fprintf(stderr, _("%i percent"),
				(j + 1) * 100 / smsnum);
		}
	}

	GSM_FreeSMSBackup(&Backup);

	GSM_Terminate();
}

void AddSMS(int argc UNUSED, char *argv[])
{
	GSM_Error error;
	GSM_MultiSMSMessage 	SMS;
	GSM_SMS_Backup		Backup;
	int			smsnum = 0;
	int			folder;

	if (argc == 5 && strcasecmp(argv[4],"-yes") == 0) always_answer_yes = TRUE;

	folder = atoi(argv[2]);

	error = GSM_ReadSMSBackupFile(argv[3], &Backup);
	Print_Error(error);

	GSM_Init(TRUE);

	while (Backup.SMS[smsnum] != NULL) {
		Backup.SMS[smsnum]->Folder = folder;
		Backup.SMS[smsnum]->SMSC.Location = 1;
		SMS.Number = 1;
		SMS.SMS[0] = *Backup.SMS[smsnum];
		DisplayMultiSMSInfo(&SMS,FALSE,FALSE,NULL, gsm);
		if (answer_yes(_("Restore message?"))) {
			error=GSM_AddSMS(gsm, Backup.SMS[smsnum]);
			Print_Error(error);
		}
		smsnum++;
	}

	/* We don't need this anymore */
	GSM_FreeSMSBackup(&Backup);

	GSM_Terminate();
}

void RestoreSMS(int argc, char *argv[])
{
	GSM_Error error;
	GSM_MultiSMSMessage 	SMS;
	GSM_SMS_Backup		Backup;
	GSM_SMSFolders		folders;
	int			smsnum = 0;
	gboolean			restore8bit;

	if (argc == 4 && strcasecmp(argv[3],"-yes") == 0) always_answer_yes = TRUE;

	error = GSM_ReadSMSBackupFile(argv[2], &Backup);
	Print_Error(error);

	restore8bit = answer_yes(_("Do you want to restore binary SMS?"));

	GSM_Init(TRUE);

	error = GSM_GetSMSFolders(gsm, &folders);
	Print_Error(error);

	while (Backup.SMS[smsnum] != NULL) {
		if (restore8bit || Backup.SMS[smsnum]->Coding != SMS_Coding_8bit) {
			SMS.Number = 1;
			memcpy(&SMS.SMS[0],Backup.SMS[smsnum],sizeof(GSM_SMSMessage));
			DisplayMultiSMSInfo(&SMS,FALSE,FALSE,NULL, gsm);
			if (answer_yes(_("Restore %03i sms to folder \"%s\"%s?"),
					smsnum + 1,
					DecodeUnicodeConsole(folders.Folder[Backup.SMS[smsnum]->Folder - 1].Name),
					folders.Folder[Backup.SMS[smsnum]->Folder - 1].Memory == MEM_SM ? _(" (SIM)") : "")) {
				smprintf(gsm, _("saving %i SMS\n"),smsnum);
				error = GSM_AddSMS(gsm, Backup.SMS[smsnum]);
				Print_Error(error);
			}
		}
		smsnum++;
	}

	/* We don't need this anymore */
	GSM_FreeSMSBackup(&Backup);

	GSM_Terminate();
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */


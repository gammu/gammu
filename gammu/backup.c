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
#include "../helper/memory-display.h"
#include "../helper/printing.h"

#ifdef GSM_ENABLE_BACKUP
void SaveFile(int argc, char *argv[])
{
	GSM_Error error;
	GSM_Backup		Backup;
	int			i;
	size_t j;
	FILE			*file;
	unsigned char		Buffer[10000];
	GSM_MemoryEntry		*pbk;

	if (strcasecmp(argv[2],"CALENDAR") == 0) {
		if (argc<5) {
			printf("%s\n", _("Where is backup filename and location?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[4],&Backup,GSM_GuessBackupFormat(argv[4], false));
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
		error = GSM_EncodeVCALENDAR(Buffer, sizeof(Buffer), &j, Backup.Calendar[i],true,Nokia_VCalendar);
		Print_Error(error);
	} else if (strcasecmp(argv[2],"BOOKMARK") == 0) {
		if (argc<5) {
			printf("%s\n", _("Where is backup filename and location?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[4],&Backup,GSM_GuessBackupFormat(argv[4], false));
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
		error=GSM_ReadBackupFile(argv[4],&Backup,GSM_GuessBackupFormat(argv[4], false));
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
		error = GSM_EncodeVNTFile(Buffer, sizeof(Buffer), &j, Backup.Note[i]);
		Print_Error(error);
	} else if (strcasecmp(argv[2],"TODO") == 0) {
		if (argc<5) {
			printf("%s\n", _("Where is backup filename and location?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[4],&Backup,GSM_GuessBackupFormat(argv[4], false));
		if (error!=ERR_NOTIMPLEMENTED) Print_Error(error);
		i = 0;
		while (Backup.ToDo[i]!=NULL) {
			if (i == atoi(argv[5])-1) break;
			i++;
		}
		if (i != atoi(argv[5])-1) {
			printf("%s\n", _("Todo note not found in file"));
			exit(-1);
		}
		j = 0;
		error = GSM_EncodeVTODO(Buffer, sizeof(Buffer), &j, Backup.ToDo[i], true, Nokia_VToDo);
		Print_Error(error);
	} else if (strcasecmp(argv[2],"VCARD10") == 0 || strcasecmp(argv[2],"VCARD21") == 0) {
		if (argc<6) {
			printf("%s\n", _("Where is backup filename and location and memory type?"));
			exit(-1);
		}
		error=GSM_ReadBackupFile(argv[4],&Backup,GSM_GuessBackupFormat(argv[4], false));
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
			error = GSM_EncodeVCARD(GSM_GetDebug(gsm), Buffer, sizeof(Buffer), &j, pbk, true, Nokia_VCard10);
			Print_Error(error);
		} else {
			error = GSM_EncodeVCARD(GSM_GetDebug(gsm), Buffer, sizeof(Buffer), &j, pbk, true, Nokia_VCard21);
			Print_Error(error);
		}
	} else {
		printf(_("Unknown backup format: \"%s\"\n"), argv[2]);
		exit(-1);
	}

	file = fopen(argv[3],"wb");
	if (file == NULL) {
		printf_err(_("Error while opening file for writing!\n"));
		exit(-1);
	}
	if (j != fwrite(Buffer,1,j,file)) {
		printf_err(_("Error while writing file!\n"));
	}
	fclose(file);
}

void DoBackup(int argc, char *argv[])
{
	GSM_Error error;
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
	bool			DoBackupPart;
	bool			UseNext;
	char buffer[GSM_MAX_INFO_LENGTH];

	if (argc == 4 && strcasecmp(argv[3],"-yes") == 0) always_answer_yes = true;

	GSM_ClearBackup(&Backup);
	GSM_GetBackupFormatFeatures(GSM_GuessBackupFormat(argv[2], false),&Info);

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
		Info.UseUnicode=answer_yes(_("Use Unicode subformat of backup file?"));
	}
	if (Info.DateTime) {
		GSM_GetCurrentDateTime (&Backup.DateTime);
		Backup.DateTimeAvailable=true;
	}
	if (Info.Model) {
		error=GSM_GetManufacturer(gsm, Backup.Model);
		Print_Error(error);
		strcat(Backup.Model," ");
		error=GSM_GetModel(gsm, buffer);
		strcat(Backup.Model, buffer);
		if (GSM_GetModelInfo(gsm)->model[0]!=0) {
			strcat(Backup.Model," (");
			strcat(Backup.Model,GSM_GetModelInfo(gsm)->model);
			strcat(Backup.Model,")");
		}
		strcat(Backup.Model," ");
		error=GSM_GetFirmware(gsm, buffer, NULL, NULL);
		strcat(Backup.Model,buffer);
	}
	if (Info.IMEI) {
		error=GSM_GetIMEI(gsm, Backup.IMEI);
		if (error != ERR_NOTSUPPORTED) {
			Print_Error(error);
		} else {
			Backup.IMEI[0] = 0;
		}
	}
	printf("\n");

	DoBackupPart = false;
	if (Info.PhonePhonebook) {
		printf("%s\n", _("Checking phone phonebook"));
		MemStatus.MemoryType = MEM_ME;
		Pbk.MemoryType  = MEM_ME;
		Pbk.Location = 0;
		UseNext = false;
		error = GSM_GetMemoryStatus(gsm, &MemStatus);
		if (error == ERR_NONE) {
			if (MemStatus.MemoryUsed != 0) {
				DoBackupPart = true;
			}
		} else {
			UseNext = true;
			error = GSM_GetNextMemory(gsm, &Pbk, true);
			if (error == ERR_NONE) {
				DoBackupPart = true;
			}

		}
		if (DoBackupPart) {
			if (answer_yes("   %s", _("Backup phone phonebook?"))) DoBackupPart = true;
			else DoBackupPart = false;
		}
	}
	if (DoBackupPart) {
		if (UseNext) {
			used 		= 0;
			fprintf(stderr, LISTFORMAT, _("Reading"));
			while (error == ERR_NONE) {
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
				error = GSM_GetNextMemory(gsm, &Pbk, false);
				fprintf(stderr, "*");
				if (gshutdown) {
					GSM_Terminate();
					exit(0);
				}
			}
			fprintf(stderr, "\n");
		} else {
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
				fprintf(stderr, "\r   ");
				fprintf(stderr, "%s ", _("Reading:"));
				fprintf(stderr, _("%i percent"),
					used * 100 / MemStatus.MemoryUsed);
				i++;
				if (gshutdown) {
					GSM_Terminate();
					exit(0);
				}
			}
			fprintf(stderr, "\n");
		}
	}
	DoBackupPart = false;
	if (Info.SIMPhonebook) {
		printf("%s\n", _("Checking SIM phonebook"));
		MemStatus.MemoryType = MEM_SM;
		error=GSM_GetMemoryStatus(gsm, &MemStatus);
		if (error==ERR_NONE && MemStatus.MemoryUsed != 0) {
			if (answer_yes("   %s", _("Backup SIM phonebook?"))) DoBackupPart=true;
		}
	}
	if (DoBackupPart) {
		Pbk.MemoryType 	= MEM_SM;
		i		= 1;
		used 		= 0;
		while (used != MemStatus.MemoryUsed) {
			Pbk.Location = i;
			error=GSM_GetMemory(gsm, &Pbk);
			if (error != ERR_EMPTY) {
				Print_Error(error);
				if (used < GSM_BACKUP_MAX_SIMPHONEBOOK) {
					Backup.SIMPhonebook[used] = malloc(sizeof(GSM_MemoryEntry));
				        if (Backup.SIMPhonebook[used] == NULL) Print_Error(ERR_MOREMEMORY);
					Backup.SIMPhonebook[used + 1] = NULL;
				} else {
					printf("\n   ");
					printf(_("Only part of data saved, please increase %s.") , "GSM_BACKUP_MAX_SIMPHONEBOOK");
					printf("\n");
					break;
				}
				*Backup.SIMPhonebook[used]=Pbk;
				used++;
			}
			fprintf(stderr, "\r   ");
			fprintf(stderr, "%s ", _("Reading:"));
			fprintf(stderr, _("%i percent"),
				used * 100 / MemStatus.MemoryUsed);
			i++;
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoBackupPart = false;
	if (Info.Calendar) {
		printf("%s\n", _("Checking phone calendar"));
		error=GSM_GetNextCalendar(gsm,&Calendar,true);
		if (error==ERR_NONE) {
			if (answer_yes("   %s", _("Backup phone calendar notes?"))) DoBackupPart = true;
		}
	}
	if (DoBackupPart) {
		used 		= 0;
		fprintf(stderr, LISTFORMAT, _("Reading"));
		while (error == ERR_NONE) {
			if (used < GSM_MAXCALENDARTODONOTES) {
				Backup.Calendar[used] = malloc(sizeof(GSM_CalendarEntry));
			        if (Backup.Calendar[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.Calendar[used+1] = NULL;
			} else {
				printf("\n   ");
				printf(_("Only part of data saved, please increase %s.") , "GSM_MAXCALENDARTODONOTES");
				printf("\n");
				break;
			}
			*Backup.Calendar[used]=Calendar;
			used ++;
			error=GSM_GetNextCalendar(gsm,&Calendar,false);
			fprintf(stderr, "*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoBackupPart = false;
	if (Info.ToDo) {
		printf("%s\n", _("Checking phone todos"));
		error=GSM_GetToDoStatus(gsm,&ToDoStatus);
		if (error == ERR_NONE && ToDoStatus.Used != 0) {
			if (answer_yes("   %s", _("Backup phone todos?"))) DoBackupPart = true;
		}
	}
	if (DoBackupPart) {
		used = 0;
		error=GSM_GetNextToDo(gsm,&ToDo,true);
		while (error == ERR_NONE) {
			if (used < GSM_MAXCALENDARTODONOTES) {
				Backup.ToDo[used] = malloc(sizeof(GSM_ToDoEntry));
				if (Backup.ToDo[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.ToDo[used+1] = NULL;
			} else {
				printf("\n   ");
				printf(_("Only part of data saved, please increase %s.") , "GSM_MAXCALENDARTODONOTES");
				printf("\n");
				break;
			}
			*Backup.ToDo[used]=ToDo;
			used ++;
			error=GSM_GetNextToDo(gsm,&ToDo,false);
			fprintf(stderr, "\r   ");
			fprintf(stderr, "%s ", _("Reading:"));
			fprintf(stderr, _("%i percent"),
				used * 100 / ToDoStatus.Used);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoBackupPart = false;
	if (Info.Note) {
		printf("%s\n", _("Checking phone notes"));
		error=GSM_GetNextNote(gsm,&Note,true);
		if (error==ERR_NONE) {
			if (answer_yes("   %s", _("Backup phone notes?"))) DoBackupPart = true;
		}
	}
	if (DoBackupPart) {
		used 		= 0;
		fprintf(stderr, LISTFORMAT, _("Reading"));
		while (error == ERR_NONE) {
			if (used < GSM_BACKUP_MAX_NOTE) {
				Backup.Note[used] = malloc(sizeof(GSM_NoteEntry));
			        if (Backup.Note[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.Note[used+1] = NULL;
			} else {
				printf("\n   ");
				printf(_("Only part of data saved, please increase %s.") , "GSM_BACKUP_MAX_NOTE");
				printf("\n");
				break;
			}
			*Backup.Note[used]=Note;
			used ++;
			error=GSM_GetNextNote(gsm,&Note,false);
			fprintf(stderr, "*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoBackupPart = false;
	if (Info.CallerLogos) {
		printf("%s\n", _("Checking phone caller logos"));
		Bitmap.Type 	= GSM_CallerGroupLogo;
		Bitmap.Location = 1;
		error=GSM_GetBitmap(gsm,&Bitmap);
		if (error == ERR_NONE) {
			if (answer_yes("   %s", _("Backup phone caller groups and logos?"))) DoBackupPart = true;
		}
	}
	if (DoBackupPart) {
		fprintf(stderr, LISTFORMAT, _("Reading"));
		error = ERR_NONE;
		used  = 0;
		while (error == ERR_NONE) {
			if (used < GSM_BACKUP_MAX_CALLER) {
				Backup.CallerLogos[used] = malloc(sizeof(GSM_Bitmap));
			        if (Backup.CallerLogos[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.CallerLogos[used+1] = NULL;
			} else {
				printf("\n   ");
				printf(_("Only part of data saved, please increase %s.") , "GSM_BACKUP_MAX_CALLER");
				printf("\n");
				break;
			}
			*Backup.CallerLogos[used] = Bitmap;
			used ++;
			Bitmap.Location = used + 1;
			error=GSM_GetBitmap(gsm,&Bitmap);
			fprintf(stderr, "*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoBackupPart = false;
	if (Info.SMSC) {
		printf("%s\n", _("Checking SIM SMS profiles"));
		if (answer_yes("   %s", _("Backup SIM SMS profiles?"))) DoBackupPart = true;
	}
	if (DoBackupPart) {
		used = 0;
		fprintf(stderr, LISTFORMAT, _("Reading"));
		while (true) {
			SMSC.Location = used + 1;
			error = GSM_GetSMSC(gsm,&SMSC);
			if (error != ERR_NONE) break;
			if (used < GSM_BACKUP_MAX_SMSC) {
				Backup.SMSC[used] = malloc(sizeof(GSM_SMSC));
			        if (Backup.SMSC[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.SMSC[used + 1] = NULL;
			} else {
				printf("\n   ");
				printf(_("Only part of data saved, please increase %s.") , "GSM_BACKUP_MAX_SMSC");
				printf("\n");
				break;
			}
			*Backup.SMSC[used]=SMSC;
			used++;
			fprintf(stderr, "*");
		}
		fprintf(stderr, "\n");
	}
	DoBackupPart = false;
	if (Info.StartupLogo) {
		printf("%s\n", _("Checking phone startup text"));
		Bitmap.Type = GSM_WelcomeNote_Text;
		error = GSM_GetBitmap(gsm,&Bitmap);
		if (error == ERR_NONE) {
			if (answer_yes("   %s", _("Backup phone startup logo/text?"))) DoBackupPart = true;
		}
	}
	if (DoBackupPart) {
		Backup.StartupLogo = malloc(sizeof(GSM_Bitmap));
	        if (Backup.StartupLogo == NULL) Print_Error(ERR_MOREMEMORY);
		*Backup.StartupLogo = Bitmap;
		if (Bitmap.Text[0]==0 && Bitmap.Text[1]==0) {
			Bitmap.Type = GSM_StartupLogo;
			error = GSM_GetBitmap(gsm,&Bitmap);
			if (error == ERR_NONE) *Backup.StartupLogo = Bitmap;
		}
	}
	DoBackupPart = false;
	if (Info.OperatorLogo) {
		printf("%s\n", _("Checking phone operator logo"));
		Bitmap.Type = GSM_OperatorLogo;
		error=GSM_GetBitmap(gsm,&Bitmap);
		if (error == ERR_NONE) {
			if (strcmp(Bitmap.NetworkCode,"000 00")!=0) {
				if (answer_yes("   %s", _("Backup phone operator logo?"))) DoBackupPart = true;
			}
		}
	}
	if (DoBackupPart) {
		Backup.OperatorLogo = malloc(sizeof(GSM_Bitmap));
	        if (Backup.OperatorLogo == NULL) Print_Error(ERR_MOREMEMORY);
		*Backup.OperatorLogo = Bitmap;
	}
	DoBackupPart = false;
	if (Info.WAPBookmark) {
		printf("%s\n", _("Checking phone WAP bookmarks"));
		Bookmark.Location = 1;
		error=GSM_GetWAPBookmark(gsm,&Bookmark);
		if (error==ERR_NONE) {
			if (answer_yes("   %s", _("Backup phone WAP bookmarks?"))) DoBackupPart = true;
		}
	}
	if (DoBackupPart) {
		used = 0;
		fprintf(stderr, LISTFORMAT, _("Reading"));
		while (error == ERR_NONE) {
			if (used < GSM_BACKUP_MAX_WAPBOOKMARK) {
				Backup.WAPBookmark[used] = malloc(sizeof(GSM_WAPBookmark));
			        if (Backup.WAPBookmark[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.WAPBookmark[used+1] = NULL;
			} else {
				printf("\n   ");
				printf(_("Only part of data saved, please increase %s.") , "GSM_BACKUP_MAX_WAPBOOKMARK");
				printf("\n");
				break;
			}
			*Backup.WAPBookmark[used]=Bookmark;
			used ++;
			Bookmark.Location = used+1;
			error=GSM_GetWAPBookmark(gsm,&Bookmark);
			fprintf(stderr, "*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoBackupPart = false;
	if (Info.WAPSettings) {
		printf("%s\n", _("Checking phone WAP settings"));
		Settings.Location = 1;
		error=GSM_GetWAPSettings(gsm,&Settings);
		if (error==ERR_NONE) {
			if (answer_yes("   %s", _("Backup phone WAP settings?"))) DoBackupPart = true;
		}
	}
	if (DoBackupPart) {
		used = 0;
		fprintf(stderr, LISTFORMAT, _("Reading"));
		while (error == ERR_NONE) {
			if (used < GSM_BACKUP_MAX_WAPSETTINGS) {
				Backup.WAPSettings[used] = malloc(sizeof(GSM_MultiWAPSettings));
			        if (Backup.WAPSettings[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.WAPSettings[used+1] = NULL;
			} else {
				printf("\n   ");
				printf(_("Only part of data saved, please increase %s.") , "GSM_BACKUP_MAX_WAPSETTINGS");
				printf("\n");
				break;
			}
			*Backup.WAPSettings[used]=Settings;
			used ++;
			Settings.Location = used+1;
			error=GSM_GetWAPSettings(gsm,&Settings);
			fprintf(stderr, "*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoBackupPart = false;
	if (Info.MMSSettings) {
		printf("%s\n", _("Checking phone MMS settings"));
		Settings.Location = 1;
		error=GSM_GetMMSSettings(gsm,&Settings);
		if (error==ERR_NONE) {
			if (answer_yes("   %s", _("Backup phone MMS settings?"))) DoBackupPart = true;
		}
	}
	if (DoBackupPart) {
		used = 0;
		fprintf(stderr, LISTFORMAT, _("Reading"));
		while (error == ERR_NONE) {
			if (used < GSM_BACKUP_MAX_MMSSETTINGS) {
				Backup.MMSSettings[used] = malloc(sizeof(GSM_MultiWAPSettings));
			        if (Backup.MMSSettings[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.MMSSettings[used+1] = NULL;
			} else {
				printf("\n   ");
				printf(_("Only part of data saved, please increase %s.") , "GSM_BACKUP_MAX_MMSSETTINGS");
				printf("\n");
				break;
			}
			*Backup.MMSSettings[used]=Settings;
			used ++;
			Settings.Location = used+1;
			error=GSM_GetMMSSettings(gsm,&Settings);
			fprintf(stderr, "*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoBackupPart = false;
	if (Info.ChatSettings) {
		printf("%s\n", _("Checking phone Chat settings"));
		Chat.Location = 1;
		error=GSM_GetChatSettings(gsm,&Chat);
		if (error==ERR_NONE) {
			if (answer_yes("   %s", _("Backup phone Chat settings?"))) DoBackupPart = true;
		}
	}
	if (DoBackupPart) {
		used = 0;
		fprintf(stderr, LISTFORMAT, _("Reading"));
		while (error == ERR_NONE) {
			if (used < GSM_BACKUP_MAX_CHATSETTINGS) {
				Backup.ChatSettings[used] = malloc(sizeof(GSM_ChatSettings));
			        if (Backup.ChatSettings[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.ChatSettings[used+1] = NULL;
			} else {
				printf("\n   ");
				printf(_("Only part of data saved, please increase %s.") , "GSM_BACKUP_MAX_CHATSETTINGS");
				printf("\n");
				break;
			}
			*Backup.ChatSettings[used]=Chat;
			used ++;
			Chat.Location = used+1;
			error=GSM_GetChatSettings(gsm,&Chat);
			fprintf(stderr, "*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoBackupPart = false;
	if (Info.SyncMLSettings) {
		printf("%s\n", _("Checking phone SyncML settings"));
		SyncML.Location = 1;
		error=GSM_GetSyncMLSettings(gsm,&SyncML);
		if (error==ERR_NONE) {
			if (answer_yes("   %s", _("Backup phone SyncML settings?"))) DoBackupPart = true;
		}
	}
	if (DoBackupPart) {
		used = 0;
		fprintf(stderr, LISTFORMAT, _("Reading"));
		while (error == ERR_NONE) {
			if (used < GSM_BACKUP_MAX_SYNCMLSETTINGS) {
				Backup.SyncMLSettings[used] = malloc(sizeof(GSM_SyncMLSettings));
			        if (Backup.SyncMLSettings[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.SyncMLSettings[used+1] = NULL;
			} else {
				printf("\n   ");
				printf(_("Only part of data saved, please increase %s.") , "GSM_BACKUP_MAX_SYNCMLSETTINGS");
				printf("\n");
				break;
			}
			*Backup.SyncMLSettings[used]=SyncML;
			used ++;
			SyncML.Location = used+1;
			error=GSM_GetSyncMLSettings(gsm,&SyncML);
			fprintf(stderr, "*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoBackupPart = false;
	if (Info.Ringtone) {
		printf("%s\n", _("Checking phone user ringtones"));
		Ringtone.Location 	= 1;
		Ringtone.Format		= 0;
		error=GSM_GetRingtone(gsm,&Ringtone,false);
		if (error==ERR_EMPTY || error == ERR_NONE) {
			if (answer_yes("   %s", _("Backup phone user ringtones?"))) DoBackupPart = true;
		}
	}
	if (DoBackupPart) {
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
					printf("\n   ");
					printf(_("Only part of data saved, please increase %s.") , "GSM_BACKUP_MAX_RINGTONES");
					printf("\n");
					break;
				}
				*Backup.Ringtone[used]=Ringtone;
				used ++;
			}
			i++;
			Ringtone.Location = i;
			Ringtone.Format	  = 0;
			error=GSM_GetRingtone(gsm,&Ringtone,false);
			fprintf(stderr, "*");
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	DoBackupPart = false;
	if (Info.Profiles) {
		printf("%s\n", _("Checking phone profiles"));
		Profile.Location = 1;
		error = GSM_GetProfile(gsm,&Profile);
	        if (error == ERR_NONE) {
			if (answer_yes("   %s", _("Backup phone profiles?"))) DoBackupPart = true;
		}
	}
	if (DoBackupPart) {
		used = 0;
		fprintf(stderr, LISTFORMAT, _("Reading"));
		while (true) {
			Profile.Location = used + 1;
			error = GSM_GetProfile(gsm,&Profile);
			if (error != ERR_NONE) break;
			if (used < GSM_BACKUP_MAX_PROFILES) {
				Backup.Profiles[used] = malloc(sizeof(GSM_Profile));
				if (Backup.Profiles[used] == NULL) Print_Error(ERR_MOREMEMORY);
				Backup.Profiles[used + 1] = NULL;
			} else {
				printf("\n   ");
				printf(_("Only part of data saved, please increase %s.") , "GSM_BACKUP_MAX_PROFILES");
				printf("\n");
				break;
			}
			*Backup.Profiles[used]=Profile;
			used++;
			fprintf(stderr, "*");
		}
		fprintf(stderr, "\n");
	}
	DoBackupPart = false;
 	if (Info.FMStation) {
		printf("%s\n", _("Checking phone FM radio stations"));
 		FMStation.Location = 1;
 		error = GSM_GetFMStation(gsm,&FMStation);
 	        if (error == ERR_NONE || error == ERR_EMPTY) {
 			if (answer_yes("   %s", _("Backup phone FM radio stations?"))) DoBackupPart=true;
		}
	}
	if (DoBackupPart) {
		used	= 0;
		i	= 1;
		fprintf(stderr, LISTFORMAT, _("Reading"));
		while (error == ERR_NONE || error == ERR_EMPTY) {
			error = GSM_GetFMStation(gsm,&FMStation);
			if (error == ERR_NONE) {
 				if (used < GSM_BACKUP_MAX_FMSTATIONS) {
 					Backup.FMStation[used] = malloc(sizeof(GSM_FMStation));
					if (Backup.FMStation[used] == NULL) Print_Error(ERR_MOREMEMORY);
 					Backup.FMStation[used + 1] = NULL;
 				} else {
					printf("\n   ");
					printf(_("Only part of data saved, please increase %s.") , "GSM_BACKUP_MAX_FMSTATIONS");
					printf("\n");
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
	DoBackupPart = false;
 	if (Info.GPRSPoint) {
		printf("%s\n", _("Checking phone GPRS access points"));
 		GPRSPoint.Location = 1;
 		error = GSM_GetGPRSAccessPoint(gsm,&GPRSPoint);
 	        if (error == ERR_NONE || error == ERR_EMPTY) {
 			if (answer_yes("   %s", _("Backup phone GPRS access points?"))) DoBackupPart = true;
		}
	}
	if (DoBackupPart) {
		used	= 0;
		i	= 1;
		fprintf(stderr, LISTFORMAT, _("Reading"));
		while (error == ERR_NONE || error == ERR_EMPTY) {
			error = GSM_GetGPRSAccessPoint(gsm,&GPRSPoint);
 			if (error == ERR_NONE) {
 				if (used < GSM_BACKUP_MAX_GPRSPOINT) {
 					Backup.GPRSPoint[used] = malloc(sizeof(GSM_GPRSAccessPoint));
					if (Backup.GPRSPoint[used] == NULL) Print_Error(ERR_MOREMEMORY);
 					Backup.GPRSPoint[used + 1] = NULL;
 				} else {
					printf("\n   ");
					printf(_("Only part of data saved, please increase %s.") , "GSM_BACKUP_MAX_GPRSPOINT");
					printf("\n");
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

	GSM_SaveBackupFile(argv[2], &Backup, GSM_GuessBackupFormat(argv[2], Info.UseUnicode));
    	GSM_FreeBackup(&Backup);
}

void Restore(int argc, char *argv[])
{
	GSM_Error error;
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

	error=GSM_ReadBackupFile(argv[2],&Backup,GSM_GuessBackupFormat(argv[2], false));
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
		if (strcmp(Backup.MD5Original,Backup.MD5Calculated)) {
			if (!answer_yes(_("Checksum in backup file do not match (original: %s, new: %s). Continue?"), Backup.MD5Original, Backup.MD5Calculated)) return;
		}
	}

	GSM_Init(true);

	printf("%s\n", _("Please note that restoring data will cause existing data in phone to be deleted."));

	DoRestore = false;
	if (Backup.CallerLogos[0] != NULL) {
		Bitmap.Type 	= GSM_CallerGroupLogo;
		Bitmap.Location = 1;
		error=GSM_GetBitmap(gsm,&Bitmap);
		if (error == ERR_NONE) {
			if (answer_yes(_("Restore phone caller groups and logos?"))) DoRestore = true;
		}
	}
	if (DoRestore) {
		max = 0;
		while (Backup.CallerLogos[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			error=GSM_SetBitmap(gsm,Backup.CallerLogos[i]);
			Print_Error(error);
			fprintf(stderr, "\r");
			fprintf(stderr, "%s ", _("Writing:"));
			fprintf(stderr, _("%i percent"),
				(i + 1) * 100 / max);
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
		error=GSM_GetMemoryStatus(gsm, &MemStatus);
		if (error==ERR_NONE) {
			max = 0;
			while (Backup.PhonePhonebook[max]!=NULL) max++;
			fprintf(stderr, _("%i entries in backup file\n"),max);
			if (answer_yes(_("Restore phone phonebook?"))) DoRestore = true;
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
				if (Pbk.EntriesNum != 0) error=GSM_SetMemory(gsm, &Pbk);
				if (error == ERR_PERMISSION && GSM_IsPhoneFeatureAvailable(GSM_GetModelInfo(gsm), F_6230iCALLER)) {
					error=GSM_DeleteMemory(gsm, &Pbk);
					Print_Error(error);
					error=GSM_SetMemory(gsm, &Pbk);
				}
				if (error == ERR_MEMORY && GSM_IsPhoneFeatureAvailable(GSM_GetModelInfo(gsm), F_6230iCALLER)) {
					printf("\n%s\n", _("Error - try to (1) add enough number of/restore caller groups and (2) use --restore again"));
					GSM_TerminateConnection(gsm);
					exit (-1);
				}
				if (Pbk.EntriesNum != 0 && error==ERR_NONE) {
					First = true;
					for (j=0;j<Pbk.EntriesNum;j++) {
			 			if (Pbk.Entries[j].AddError == ERR_NONE) continue;
						if (First) {
							printf("\r");
							printf(_("Location %d"), Pbk.Location);
							printf("%20s\n    ", " ");
							First = false;
						}
						PrintMemorySubEntry(&Pbk.Entries[j], gsm);
						printf("    %s\n", GSM_ErrorString(Pbk.Entries[j].AddError));
					}
				}
			}
			if (Pbk.EntriesNum == 0) {
				/* Delete only when there was some content in phone */
				if (MemStatus.MemoryUsed > 0)
					error=GSM_DeleteMemory(gsm, &Pbk);
			}
			Print_Error(error);
			fprintf(stderr, "\r");
			fprintf(stderr, "%s ", _("Writing:"));
			fprintf(stderr, _("%i percent"),
					(i + 1) * 100 / (MemStatus.MemoryUsed + MemStatus.MemoryFree)
					);
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
		error=GSM_GetMemoryStatus(gsm, &MemStatus);
		if (error==ERR_NONE) {
			max = 0;
			while (Backup.SIMPhonebook[max]!=NULL) max++;
			fprintf(stderr, _("%i entries in backup file\n"),max);
			if (answer_yes(_("Restore SIM phonebook?"))) DoRestore = true;
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
				if (Pbk.EntriesNum != 0) {
					error=GSM_SetMemory(gsm, &Pbk);
					if (error==ERR_NONE) {
						First = true;
						for (j=0;j<Pbk.EntriesNum;j++) {
					 		if (Pbk.Entries[j].AddError == ERR_NONE) continue;
							if (First) {
								printf("\r");
								printf(_("Location %d"), Pbk.Location);
								printf("%20s\n    ", " ");
								First = false;
							}
							PrintMemorySubEntry(&Pbk.Entries[j], gsm);
							printf("    %s\n",GSM_ErrorString(Pbk.Entries[j].AddError));
						}
					}
				}
			}
			if (Pbk.EntriesNum == 0) error=GSM_DeleteMemory(gsm, &Pbk);
			Print_Error(error);
			fprintf(stderr, "\r");
			fprintf(stderr, "%s ", _("Writing:"));
			fprintf(stderr, _("%i percent"),
				(i + 1) * 100 / (MemStatus.MemoryUsed + MemStatus.MemoryFree));
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}

	if (!strcasecmp(GSM_GetConfig(gsm, -1)->SyncTime,"yes") == 0) {
		if (answer_yes(_("Do you want to set phone date/time? (NOTE: in some phones it's required to correctly restore calendar notes and other items)"))) {
			GSM_GetCurrentDateTime(&date_time);

			error=GSM_SetDateTime(gsm, &date_time);
			Print_Error(error);
		}
	}
	DoRestore = false;
	if (Backup.Calendar[0] != NULL) {
		/* N6110 doesn't support getting calendar status */
		error = GSM_GetNextCalendar(gsm,&Calendar,true);
		if (error == ERR_NONE || error == ERR_INVALIDLOCATION || error == ERR_EMPTY) {
			max = 0;
			while (Backup.Calendar[max] != NULL) max++;
			fprintf(stderr, _("%i entries in backup file\n"),max);
			if (answer_yes(_("Restore phone calendar notes?"))) {
				Past    = answer_yes("   %s", _("Restore notes from the past?"));
				DoRestore = true;
			}
		}
	}
	if (DoRestore) {
		fprintf(stderr, "%s ", _("Deleting old notes:"));
		error = GSM_DeleteAllCalendar(gsm);
		if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
 			while (1) {
				error = GSM_GetNextCalendar(gsm,&Calendar,true);
				if (error != ERR_NONE) break;
				error = GSM_DeleteCalendar(gsm,&Calendar);
 				Print_Error(error);
				fprintf(stderr, "*");
			}
			fprintf(stderr, "\n");
		} else {
			fprintf(stderr, "%s\n", _("Done"));
			Print_Error(error);
		}

		for (i=0;i<max;i++) {
			if (!Past && GSM_IsCalendarNoteFromThePast(Backup.Calendar[i])) continue;

			Calendar = *Backup.Calendar[i];
			error=GSM_AddCalendar(gsm,&Calendar);
			Print_Error(error);
			fprintf(stderr, "\r");
			fprintf(stderr, "%s ", _("Writing:"));
			fprintf(stderr, _("%i percent"),
				(i + 1) * 100 / max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}

	DoRestore = false;
	if (Backup.ToDo[0] != NULL) {
		error = GSM_GetToDoStatus(gsm,&ToDoStatus);
		if (error == ERR_NONE) {
			max = 0;
			while (Backup.ToDo[max]!=NULL) max++;
			fprintf(stderr, _("%i entries in backup file\n"),max);

			if (answer_yes(_("Restore phone todo?"))) DoRestore = true;
		}
	}
	if (DoRestore) {
		ToDo  = *Backup.ToDo[0];
		error = GSM_SetToDo(gsm,&ToDo);
	}
	if (DoRestore && (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED)) {
		fprintf(stderr, "%s ", _("Deleting old todos:"));
		error=GSM_DeleteAllToDo(gsm);
		if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
			while (1) {
				error = GSM_GetNextToDo(gsm,&ToDo,true);
				if (error != ERR_NONE) break;
				error = GSM_DeleteToDo(gsm,&ToDo);
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
			error=GSM_AddToDo(gsm,&ToDo);
			Print_Error(error);
			fprintf(stderr, "\r");
			fprintf(stderr, "%s ", _("Writing:"));
			fprintf(stderr, _("%i percent"),
				(i + 1) * 100 / max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	} else if (DoRestore) {
		/* At first delete entries, that were deleted */
		used  = 0;
		error = GSM_GetNextToDo(gsm,&ToDo,true);
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
				error=GSM_DeleteToDo(gsm,&ToDo);
				Print_Error(error);
			}
			error = GSM_GetNextToDo(gsm,&ToDo,false);
			fprintf(stderr, "\r");
			fprintf(stderr, "%s ", _("Writing:"));
			fprintf(stderr, _("%i percent"),
				used * 100 / ToDoStatus.Used);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");

		/* Now write modified/new entries */
		for (i=0;i<max;i++) {
			ToDo  = *Backup.ToDo[i];
			error = GSM_SetToDo(gsm,&ToDo);
			Print_Error(error);
			fprintf(stderr, "\r");
			fprintf(stderr, "%s ", _("Writing:"));
			fprintf(stderr, _("%i percent"),
				(i + 1) * 100 / max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
 	}

	DoRestore = false;
	if (Backup.Note[0] != NULL) {
		error = GSM_GetNotesStatus(gsm,&ToDoStatus);
		if (error == ERR_NONE) {
			max = 0;
			while (Backup.Note[max]!=NULL) max++;
			fprintf(stderr, _("%i entries in backup file\n"),max);

			if (answer_yes(_("Restore phone notes?"))) DoRestore = true;
		}
	}
	if (DoRestore) {
		fprintf(stderr, "%s ", _("Deleting old notes:"));
		while (1) {
			error = GSM_GetNextNote(gsm,&Note,true);
			if (error != ERR_NONE) break;
			error = GSM_DeleteNote(gsm,&Note);
 			Print_Error(error);
			fprintf(stderr, "*");
		}
		fprintf(stderr, "\n");

		for (i=0;i<max;i++) {
			Note 		= *Backup.Note[i];
			Note.Location 	= 0;
			error=GSM_AddNote(gsm,&Note);
			Print_Error(error);
			fprintf(stderr, "\r");
			fprintf(stderr, "%s ", _("Writing:"));
			fprintf(stderr, _("%i percent"),
				(i + 1) * 100 / max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}

	if (Backup.SMSC[0] != NULL && answer_yes(_("Restore SIM SMSC profiles?"))) {
		max = 0;
		while (Backup.SMSC[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			error=GSM_SetSMSC(gsm,Backup.SMSC[i]);
			Print_Error(error);
			fprintf(stderr, "\r");
			fprintf(stderr, "%s ", _("Writing:"));
			fprintf(stderr, _("%i percent"),
				(i + 1) * 100 / max);
			if (gshutdown) {
				GSM_Terminate();
				exit(0);
			}
		}
		fprintf(stderr, "\n");
	}
	if (Backup.StartupLogo != NULL && answer_yes(_("Restore phone startup logo/text?"))) {
		error=GSM_SetBitmap(gsm,Backup.StartupLogo);
		Print_Error(error);
	}
	if (Backup.OperatorLogo != NULL && answer_yes(_("Restore phone operator logo?"))) {
		error=GSM_SetBitmap(gsm,Backup.OperatorLogo);
		Print_Error(error);
	}
	DoRestore = false;
	if (Backup.WAPBookmark[0] != NULL) {
		Bookmark.Location = 1;
		error = GSM_GetWAPBookmark(gsm,&Bookmark);
		if (error == ERR_NONE || error == ERR_INVALIDLOCATION) {
			if (answer_yes(_("Restore phone WAP bookmarks?"))) DoRestore = true;
		}
	}
	if (DoRestore) {
		fprintf(stderr, "%s ", _("Deleting old bookmarks:"));
		/* One thing to explain: DCT4 phones seems to have bug here.
		 * When delete for example first bookmark, phone change
		 * numeration for getting frame, not for deleting. So, we try to
		 * get 1'st bookmark. Inside frame is "correct" location. We use
		 * it later
		 */
		while (error==ERR_NONE) {
			error = GSM_DeleteWAPBookmark(gsm,&Bookmark);
			Bookmark.Location = 1;
			error = GSM_GetWAPBookmark(gsm,&Bookmark);
			fprintf(stderr, "*");
		}
		fprintf(stderr, "\n");
		max = 0;
		while (Backup.WAPBookmark[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			Bookmark 	  = *Backup.WAPBookmark[i];
			Bookmark.Location = 0;
			error=GSM_SetWAPBookmark(gsm,&Bookmark);
			Print_Error(error);
			fprintf(stderr, "\r");
			fprintf(stderr, "%s ", _("Writing:"));
			fprintf(stderr, _("%i percent"),
				(i + 1) * 100 / max);
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
		error = GSM_GetWAPSettings(gsm,&Settings);
		if (error == ERR_NONE) {
			if (answer_yes(_("Restore phone WAP settings?"))) DoRestore = true;
		}
	}
	if (DoRestore) {
		max = 0;
		while (Backup.WAPSettings[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			error=GSM_SetWAPSettings(gsm,Backup.WAPSettings[i]);
			Print_Error(error);
			fprintf(stderr, "\r");
			fprintf(stderr, "%s ", _("Writing:"));
			fprintf(stderr, _("%i percent"),
				(i + 1) * 100 / max);
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
		error = GSM_GetMMSSettings(gsm,&Settings);
		if (error == ERR_NONE) {
			if (answer_yes(_("Restore phone MMS settings?"))) DoRestore = true;
		}
	}
	if (DoRestore) {
		max = 0;
		while (Backup.MMSSettings[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			error=GSM_SetMMSSettings(gsm,Backup.MMSSettings[i]);
			Print_Error(error);
			fprintf(stderr, "\r");
			fprintf(stderr, "%s ", _("Writing:"));
			fprintf(stderr, _("%i percent"),
				(i + 1) * 100 / max);
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
		error = GSM_GetRingtone(gsm,&Ringtone,false);
		if (error == ERR_NONE || error ==ERR_EMPTY) {
			if (answer_yes(_("Delete all phone user ringtones?"))) DoRestore = true;
		}
	}
	if (DoRestore) {
		fprintf(stderr, LISTFORMAT, _("Deleting"));
		error=GSM_DeleteUserRingtones(gsm);
		Print_Error(error);
		fprintf(stderr, "%s\n", _("Done"));
		DoRestore = false;
		if (answer_yes(_("Restore user ringtones?"))) DoRestore = true;
	}
	if (DoRestore) {
		max = 0;
		while (Backup.Ringtone[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			error=GSM_RingtoneConvert(&Ringtone, Backup.Ringtone[i], Ringtone.Format);
			Print_Error(error);
			error=GSM_SetRingtone(gsm,&Ringtone,&i);
			Print_Error(error);
			fprintf(stderr, "\r");
			fprintf(stderr, "%s ", _("Writing:"));
			fprintf(stderr, _("%i percent"),
				(i + 1) * 100 / max);
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
		error = GSM_GetProfile(gsm,&Profile);
		if (error == ERR_NONE) {
			if (answer_yes(_("Restore phone profiles?"))) DoRestore = true;
		}
	}
	if (DoRestore) {
		Profile.Location= 0;
		max = 0;
		while (Backup.Profiles[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			Profile	= *Backup.Profiles[i];
			error=GSM_SetProfile(gsm,&Profile);
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
		error = GSM_GetFMStation(gsm,&FMStation);
		if (error == ERR_NONE || error == ERR_EMPTY) {
			if (answer_yes(_("Restore phone FM radio stations?"))) DoRestore = true;
		}
	}
	if (DoRestore) {
		fprintf(stderr, "%s ", _("Deleting old FM stations:"));
		error=GSM_ClearFMStations(gsm);
		Print_Error(error);
		fprintf(stderr, "%s\n", _("Done"));
		max = 0;
		while (Backup.FMStation[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			FMStation = *Backup.FMStation[i];
			error=GSM_SetFMStation(gsm,&FMStation);
			Print_Error(error);
			fprintf(stderr, "\r");
			fprintf(stderr, "%s ", _("Writing:"));
			fprintf(stderr, _("%i percent"),
				(i + 1) * 100 / max);
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
		error = GSM_GetGPRSAccessPoint(gsm,&GPRSPoint);
		if (error == ERR_NONE || error == ERR_EMPTY) {
			if (answer_yes(_("Restore phone GPRS Points?"))) DoRestore = true;
		}
	}
	if (DoRestore) {
		max = 0;
		while (Backup.GPRSPoint[max]!=NULL) max++;
		for (i=0;i<max;i++) {
			error=GSM_SetGPRSAccessPoint(gsm,Backup.GPRSPoint[i]);
			Print_Error(error);
			fprintf(stderr, "\r");
			fprintf(stderr, "%s ", _("Writing:"));
			fprintf(stderr, _("%i percent"),
				(i + 1) * 100 / max);
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

void AddNew(int argc, char *argv[])
{
	GSM_Error error;
	GSM_Backup		Backup;
	GSM_DateTime 		date_time;
	GSM_MemoryEntry		Pbk;
	GSM_MemoryStatus	MemStatus;
	GSM_ToDoEntry		ToDo;
	GSM_ToDoStatus		ToDoStatus;
	GSM_CalendarEntry	Calendar;
	GSM_WAPBookmark		Bookmark;
	int			i, max;

	error=GSM_ReadBackupFile(argv[2],&Backup,GSM_GuessBackupFormat(argv[2], false));
	if (error!=ERR_NOTIMPLEMENTED) Print_Error(error);

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));

	if (Backup.DateTimeAvailable) 	fprintf(stderr, LISTFORMAT "%s\n", _("Time of backup"),OSDateTime(Backup.DateTime,false));
	if (Backup.Model[0]!=0) 	fprintf(stderr, LISTFORMAT "%s\n", _("Phone"),Backup.Model);
	if (Backup.IMEI[0]!=0) 		fprintf(stderr, LISTFORMAT "%s\n", _("IMEI"),Backup.IMEI);

	if (argc == 4 && strcasecmp(argv[3],"-yes") == 0) always_answer_yes = true;

	GSM_Init(true);

	if (Backup.PhonePhonebook[0] != NULL) {
		MemStatus.MemoryType = MEM_ME;
		error=GSM_GetMemoryStatus(gsm, &MemStatus);
		if (error==ERR_NONE) {
			max = 0;
			while (Backup.PhonePhonebook[max]!=NULL) max++;
			fprintf(stderr, _("%i entries in backup file\n"),max);
			if (MemStatus.MemoryFree < max) {
				fprintf(stderr, _("Memory has only %i free locations.Exiting\n"),MemStatus.MemoryFree);
			} else if (answer_yes(_("Add phone phonebook entries?"))) {
				for (i=0;i<max;i++) {
					Pbk 		= *Backup.PhonePhonebook[i];
					Pbk.MemoryType 	= MEM_ME;
					error=GSM_AddMemory(gsm, &Pbk);
					Print_Error(error);
					fprintf(stderr, "\r");
					fprintf(stderr, "%s ", _("Writing:"));
					fprintf(stderr, _("%i percent"),
						(i + 1) * 100 / max);
					if (gshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				fprintf(stderr, "\n");
			}
		}
	}
	if (Backup.SIMPhonebook[0] != NULL) {
		MemStatus.MemoryType = MEM_SM;
		error=GSM_GetMemoryStatus(gsm, &MemStatus);
		if (error==ERR_NONE) {
			max = 0;
			while (Backup.SIMPhonebook[max]!=NULL) max++;
			fprintf(stderr, _("%i entries in backup file\n"),max);
			if (MemStatus.MemoryFree < max) {
				fprintf(stderr, _("Memory has only %i free locations.Exiting\n"),MemStatus.MemoryFree);
			} else if (answer_yes(_("Add SIM phonebook entries?"))) {
				for (i=0;i<max;i++) {
					Pbk 		= *Backup.SIMPhonebook[i];
					Pbk.MemoryType 	= MEM_SM;
					error=GSM_AddMemory(gsm, &Pbk);
					Print_Error(error);
					fprintf(stderr, "\r");
					fprintf(stderr, "%s ", _("Writing:"));
					fprintf(stderr, _("%i percent"),
						(i + 1) * 100 / max);
					if (gshutdown) {
						GSM_Terminate();
						exit(0);
					}
				}
				fprintf(stderr, "\n");
			}
		}
	}

	if (!strcasecmp(GSM_GetConfig(gsm, -1)->SyncTime,"yes") == 0) {
		if (answer_yes(_("Do you want to set phone date/time? (NOTE: in some phones it's required to correctly restore calendar notes and other items)"))) {
			GSM_GetCurrentDateTime(&date_time);

			error=GSM_SetDateTime(gsm, &date_time);
			Print_Error(error);
		}
	}
	if (Backup.Calendar[0] != NULL) {
		error = GSM_GetNextCalendar(gsm,&Calendar,true);
		if (error == ERR_NONE || error == ERR_INVALIDLOCATION || error == ERR_EMPTY) {
			if (answer_yes(_("Add phone calendar notes?"))) {
				max = 0;
				while (Backup.Calendar[max]!=NULL) max++;
				for (i=0;i<max;i++) {
					Calendar = *Backup.Calendar[i];
					error=GSM_AddCalendar(gsm,&Calendar);
					Print_Error(error);
					fprintf(stderr, "\r");
					fprintf(stderr, "%s ", _("Writing:"));
					fprintf(stderr, _("%i percent"),
						(i + 1) * 100 / max);
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
		error=GSM_GetToDoStatus(gsm,&ToDoStatus);
		if (error == ERR_NONE) {
			if (answer_yes(_("Add phone ToDo?"))) {
				max = 0;
				while (Backup.ToDo[max]!=NULL) max++;
				for (i=0;i<max;i++) {
					ToDo  = *Backup.ToDo[i];
					error = GSM_AddToDo(gsm,&ToDo);
					Print_Error(error);
					fprintf(stderr, "\r");
					fprintf(stderr, "%s ", _("Writing:"));
					fprintf(stderr, _("%i percent"),
						(i + 1) * 100 / max);
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
		error = GSM_GetWAPBookmark(gsm,&Bookmark);
		if (error == ERR_NONE || error == ERR_INVALIDLOCATION) {
			if (answer_yes(_("Add phone WAP bookmarks?"))) {
				max = 0;
				while (Backup.WAPBookmark[max]!=NULL) max++;
				for (i=0;i<max;i++) {
					Bookmark 	  = *Backup.WAPBookmark[i];
					Bookmark.Location = 0;
					error=GSM_SetWAPBookmark(gsm,&Bookmark);
					Print_Error(error);
					fprintf(stderr, "\r");
					fprintf(stderr, "%s ", _("Writing:"));
					fprintf(stderr, _("%i percent"),
						(i + 1) * 100 / max);
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

void BackupSMS(int argc UNUSED, char *argv[])
{
	GSM_Error error;
	GSM_SMS_Backup		Backup;
	GSM_MultiSMSMessage 	sms;
	GSM_SMSFolders		folders;
	bool			BackupFromFolder[GSM_MAX_SMS_FOLDERS];
	bool			start = true;
	bool			DeleteAfter;
	int			j, smsnum = 0;
	char			buffer[200];

	GSM_Init(true);

	error=GSM_GetSMSFolders(gsm, &folders);
	Print_Error(error);

	DeleteAfter=answer_yes(_("Delete each sms after backup?"));

	for (j=0;j<folders.Number;j++) {
		BackupFromFolder[j] = false;
		if (folders.Folder[j].Memory == MEM_SM) strcat(buffer," (SIM)");
		if (answer_yes(_("Backup sms from folder \"%s\"%s?"),
				DecodeUnicodeConsole(folders.Folder[j].Name),
				folders.Folder[j].Memory == MEM_SM ? _(" (SIM)") : ""
				))
			BackupFromFolder[j] = true;
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
			error=GSM_DeleteSMS(gsm, Backup.SMS[j]);
			Print_Error(error);
			fprintf(stderr, "\r");
			fprintf(stderr, "%s ", _("Deleting:"));
			fprintf(stderr, _("%i percent"),
				(j + 1) * 100 / smsnum);
		}
	}

	GSM_Terminate();
}

void AddSMS(int argc UNUSED, char *argv[])
{
	GSM_Error error;
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
		DisplayMultiSMSInfo(&SMS,false,false,NULL, gsm);
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

void RestoreSMS(int argc UNUSED, char *argv[])
{
	GSM_Error error;
	GSM_MultiSMSMessage 	SMS;
	GSM_SMS_Backup		Backup;
	GSM_SMSFolders		folders;
	int			smsnum = 0;
	bool			restore8bit;

	error = GSM_ReadSMSBackupFile(argv[2], &Backup);
	Print_Error(error);

	restore8bit = answer_yes(_("Do you want to restore binary SMS?"));

	GSM_Init(true);

	error = GSM_GetSMSFolders(gsm, &folders);
	Print_Error(error);

	while (Backup.SMS[smsnum] != NULL) {
		if (restore8bit || Backup.SMS[smsnum]->Coding != SMS_Coding_8bit) {
			SMS.Number = 1;
			memcpy(&SMS.SMS[0],Backup.SMS[smsnum],sizeof(GSM_SMSMessage));
			DisplayMultiSMSInfo(&SMS,false,false,NULL, gsm);
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
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */


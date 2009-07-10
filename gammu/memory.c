#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <stdlib.h>
#include <ctype.h>

#include "../helper/locales.h"

#include "common.h"
#include "memory.h"
#include "../helper/formats.h"
#include "../helper/memory-display.h"
#include "../helper/printing.h"
#include "../helper/string.h"

#define GAMMU_CALLER_GROUPS	20

void GetAllMemory(int argc UNUSED, char *argv[])
{
	GSM_MemoryEntry		Entry;
	gboolean			start = TRUE;
	GSM_Error error;

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));

	Entry.MemoryType = GSM_StringToMemoryType(argv[2]);
	Entry.Location = 0;

	if (Entry.MemoryType == 0) {
		printf_err(_("Unknown memory type (\"%s\")\n"),argv[2]);
		exit (-1);
	}

	GSM_Init(TRUE);

	while (!gshutdown) {
		error = GSM_GetNextMemory(gsm, &Entry, start);
		if (error == ERR_EMPTY) break;
		if (error != ERR_NONE && ringtones_info.Ringtone) free(ringtones_info.Ringtone);
		Print_Error(error);
		printf(_("Memory %s, Location %i\n"),argv[2],Entry.Location);
		error = PrintMemoryEntry(&Entry, gsm);
		GSM_FreeMemoryEntry(&Entry);
		Print_Error(error);
 		start = FALSE;
	}

 	if (ringtones_info.Ringtone) free(ringtones_info.Ringtone);

	GSM_Terminate();
}

void GetMemory(int argc, char *argv[])
{
	int			j, start, stop, emptynum = 0, fillednum = 0;
	GSM_MemoryEntry		entry;
	gboolean			empty = TRUE;
	double version;
	GSM_Error error;

	entry.MemoryType = GSM_StringToMemoryType(argv[2]);

	if (entry.MemoryType==0) {
		printf_err(_("Unknown memory type (\"%s\")\n"),argv[2]);
		exit (-1);
	}

	GetStartStop(&start, &stop, 3, argc, argv);

	if (argc > 5 && strcmp(argv[5],"")) {
		if (strcasecmp(argv[5],"-nonempty") == 0) {
			empty = FALSE;
		} else {
			printf_err(_("Unknown parameter (\"%s\")\n"),argv[5]);
			exit (-1);
		}
	}

	GSM_Init(TRUE);

	if (strcmp(GSM_GetModelInfo(gsm)->model, "3310") == 0) {
		error = GSM_GetFirmware(gsm, NULL, NULL, &version);
		if (error == ERR_NONE && version <= 4.06) {
			printf_warn("%s\n", _("You will have null names in entries. Upgrade firmware in phone to higher than 4.06"));
		}
	}
	if (strcmp(GSM_GetModelInfo(gsm)->model, "3210") == 0) {
		error = GSM_GetFirmware(gsm, NULL, NULL, &version);
		if (error == ERR_NONE && version <= 6.00) {
			printf_warn("%s\n", _("You will have null names in entries. Upgrade firmware in phone to higher than 6.00"));
		}
	}

	for (j=start;j<=stop;j++) {
		if (empty) printf(_("Memory %s, Location %i\n"),argv[2],j);

		entry.Location=j;

		error=GSM_GetMemory(gsm, &entry);
		if (error != ERR_EMPTY) {
			if (ringtones_info.Ringtone) free(ringtones_info.Ringtone);
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
			error = PrintMemoryEntry(&entry, gsm);
			GSM_FreeMemoryEntry(&entry);
			Print_Error(error);
		}
	}

	printf(_("%i entries empty, %i entries filled\n"),emptynum,fillednum);

 	if (ringtones_info.Ringtone) free(ringtones_info.Ringtone);

	GSM_Terminate();
}

void DeleteMemory(int argc, char *argv[])
{
	int			j, start, stop;
	GSM_MemoryEntry		entry;
	GSM_Error error;

	entry.MemoryType = GSM_StringToMemoryType(argv[2]);

	if (entry.MemoryType==0) {
		printf_err(_("Unknown memory type (\"%s\")\n"),argv[2]);
		exit (-1);
	}

	GetStartStop(&start, &stop, 3, argc, argv);

	GSM_Init(TRUE);

	for (j=start;j<=stop;j++) {
		printf(LISTFORMAT "%i\n", _("Location"), j);

		entry.Location = j;

		error = GSM_DeleteMemory(gsm, &entry);

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

void DeleteAllMemory(int argc, char *argv[])
{
	GSM_MemoryType MemoryType;
	GSM_Error error;

	MemoryType = GSM_StringToMemoryType(argv[2]);

	if (MemoryType == 0) {
		printf_err(_("Unknown memory type (\"%s\")\n"),argv[2]);
		exit (-1);
	}

	GSM_Init(TRUE);

	error = GSM_DeleteAllMemory(gsm, MemoryType);

	if (error != ERR_NONE) Print_Error(error);

	GSM_Terminate();
}

void SearchOneEntry(GSM_MemoryEntry *Entry, const unsigned char *Text)
{
	int i;
	GSM_Error error;

	for (i=0;i<Entry->EntriesNum;i++) {
		switch (Entry->Entries[i].EntryType) {
			case PBK_Number_General     :
			case PBK_Number_Mobile      :
			case PBK_Number_Mobile_Work :
			case PBK_Number_Mobile_Home :
			case PBK_Number_Work        :
			case PBK_Number_Fax         :
			case PBK_Number_Home        :
			case PBK_Number_Pager       :
			case PBK_Number_Other       :
			case PBK_Number_Messaging:
			case PBK_Text_Note          :
			case PBK_Text_Postal        :
			case PBK_Text_WorkPostal:
			case PBK_Text_Email         :
			case PBK_Text_Email2        :
			case PBK_Text_URL           :
			case PBK_Text_LUID          :
			case PBK_Text_Name          :
			case PBK_Text_LastName      :
			case PBK_Text_FirstName     :
			case PBK_Text_FormalName    :
			case PBK_Text_NickName      :
			case PBK_Text_Company       :
			case PBK_Text_JobTitle      :
			case PBK_Text_StreetAddress :
			case PBK_Text_City          :
			case PBK_Text_State         :
			case PBK_Text_Zip           :
			case PBK_Text_Country       :
			case PBK_Text_WorkStreetAddress :
			case PBK_Text_WorkCity          :
			case PBK_Text_WorkState         :
			case PBK_Text_WorkZip           :
			case PBK_Text_WorkCountry       :
			case PBK_Text_Custom1       :
			case PBK_Text_Custom2       :
			case PBK_Text_Custom3       :
			case PBK_Text_Custom4       :
			case PBK_Text_UserID:
			case PBK_Text_PictureName:
			case PBK_PushToTalkID:
				if (mywstrstr(Entry->Entries[i].Text, Text) != NULL) {
					fprintf(stderr,"\n");
					printf(_("Memory %s, Location %i\n"),GSM_MemoryTypeToString(Entry->MemoryType),Entry->Location);
					error = PrintMemoryEntry(Entry, gsm);
					Print_Error(error);
					return;
				}
				break;
			case PBK_Caller_Group:
			case PBK_Date:
			case PBK_Category:
			case PBK_Private:
			case PBK_RingtoneID:
			case PBK_PictureID:
			case PBK_CallLength:
			case PBK_LastModified:
			case PBK_Photo:
#ifndef CHECK_CASES
			default:
#endif
				break;
		}
	}
}

void SearchOneMemory(GSM_MemoryType MemoryType, const char *Title, const unsigned char *Text)
{
	GSM_MemoryEntry		Entry;
	GSM_MemoryStatus	Status;
	int			i = 0, l = 1;
	GSM_Error error;

	Status.MemoryType = MemoryType;
	Entry.MemoryType  = MemoryType;
	Entry.Location = 0;

	if (GSM_GetMemoryStatus(gsm, &Status) == ERR_NONE) {
		fprintf(stderr,"%c%s: %i%%", 13, Title, (i+1)*100/(Status.MemoryUsed+1));
		error = GSM_GetNextMemory(gsm, &Entry, TRUE);
		if (error != ERR_NOTSUPPORTED && error != ERR_NOTIMPLEMENTED) {
			while (i < Status.MemoryUsed) {
				if (error == ERR_EMPTY) break;
				Print_Error(error);
				SearchOneEntry(&Entry, Text);
				GSM_FreeMemoryEntry(&Entry);
				if (gshutdown) return;
				i++;
				fprintf(stderr,"\r%s: %i%%", Title, (i+1)*100/(Status.MemoryUsed+1));
				error = GSM_GetNextMemory(gsm, &Entry, FALSE);
			}
		} else {
			while (i < Status.MemoryUsed) {
				Entry.Location = l;
				error = GSM_GetMemory(gsm, &Entry);
				if (error != ERR_EMPTY) {
					Print_Error(error);
					i++;
					SearchOneEntry(&Entry, Text);
					GSM_FreeMemoryEntry(&Entry);
				}
				fprintf(stderr,"%c%s: %i%%", 13, Title, (i+1)*100/(Status.MemoryUsed+1));
				l++;
			}
		}
		fprintf(stderr,"\n");
	}
}

void SearchMemory(int argc UNUSED, char *argv[])
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

	GSM_Init(TRUE);

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

void ListMemoryCategoryEntries(int Category)
{
	GSM_MemoryEntry		Entry;
	gboolean			start = TRUE;
	int			j;
	GSM_Error error;

	/* Category can be only for ME stored entries */
	Entry.MemoryType  = MEM_ME;

	while (!gshutdown) {
		error = GSM_GetNextMemory(gsm, &Entry, start);
		if (error == ERR_EMPTY) break;
		Print_Error(error);
		for (j=0;j<Entry.EntriesNum;j++) {
			if (Entry.Entries[j].EntryType == PBK_Category && Entry.Entries[j].Number == Category) {
				printf(_("Memory %s, Location %i\n"),GSM_MemoryTypeToString(Entry.MemoryType),Entry.Location);
				error = PrintMemoryEntry(&Entry, gsm);
				Print_Error(error);
			}
		}
		GSM_FreeMemoryEntry(&Entry);
 		start = FALSE;
	}
}

void ListMemoryCategory(int argc UNUSED, char *argv[])
{
	GSM_Category		Category;
	GSM_CategoryStatus	Status;
	int			j, count;
	GSM_Error error;

	unsigned char		Text[(GSM_MAX_CATEGORY_NAME_LENGTH+1)*2];
	int			Length;
	gboolean			Number = TRUE;;

	GSM_Init(TRUE);

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));

	Length = strlen(argv[2]);
	for (j = 0; j < Length; j++) {
		if (!isdigit((int)argv[2][j])) {
			Number = FALSE;
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

		if (GSM_GetCategoryStatus(gsm, &Status) == ERR_NONE) {
			for (count=0,j=1;count<Status.Used;j++) {
				Category.Location=j;
				error=GSM_GetCategory(gsm, &Category);

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

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */


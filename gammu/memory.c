#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <stdlib.h>
#include <ctype.h>

#include "../common/misc/locales.h"

#include "common.h"
#include "memory.h"
#include "formats.h"

#define GAMMU_CALLER_GROUPS	20

static GSM_Bitmap		caller[GAMMU_CALLER_GROUPS];
static GSM_AllRingtonesInfo 	Info = {0, NULL};
static bool			callerinit[GAMMU_CALLER_GROUPS] = {false, false, false, false, false};
static bool			ringinit = false;

void PrintMemorySubEntry(GSM_SubMemoryEntry *entry)
{
	GSM_Category	Category;
	int		z;
	GSM_Error error;

	switch (entry->EntryType) {
	case PBK_CallLength:
		printf(_("Call length      : %02i:%02i:%02i\n"),entry->CallLength/(60*60),entry->CallLength/60,entry->CallLength%60);
		return;
	case PBK_Date:
		printf(LISTFORMAT "%s\n", _("Date and time"),OSDateTime(entry->Date,false));
		return;
	case PBK_LastModified:
		printf(LISTFORMAT "%s\n", _("Last modified"), OSDateTime(entry->Date,false));
		return;
	case PBK_Category:
		if (entry->Number == -1) {
			printf(LISTFORMAT "\"%s\"\n", _("Category"), DecodeUnicodeConsole(entry->Text));
		} else {
			Category.Location = entry->Number;
			Category.Type = Category_Phonebook;
			error=GSM_GetCategory(gsm, &Category);
			if (error == ERR_NONE) {
				printf(LISTFORMAT "\"%s\" (%i)\n", _("Category"), DecodeUnicodeConsole(Category.Name), entry->Number);
			} else {
				printf(LISTFORMAT "%i\n", _("Category"), entry->Number);
			}
		}
		return;
	case PBK_Private:
		printf(LISTFORMAT "%s\n", _("Private"), entry->Number == 1 ? "Yes" : "No");
		return;
	case PBK_Caller_Group       :
		if (entry->Number >= GAMMU_CALLER_GROUPS) {
			printf(LISTFORMAT "\"%d\"\n", _("Caller group"),entry->Number);
			fprintf(stderr, "%s\n", _("Caller group number too high, please increase buffer in sources!"));
			break;
		}
		if (!callerinit[entry->Number-1]) {
			caller[entry->Number-1].Type	    = GSM_CallerGroupLogo;
			caller[entry->Number-1].Location = entry->Number;
			error=GSM_GetBitmap(gsm,&caller[entry->Number-1]);
			Print_Error(error);
			if (caller[entry->Number-1].DefaultName) {
				NOKIA_GetDefaultCallerGroupName(&caller[entry->Number-1]);
			}
			callerinit[entry->Number-1]=true;
		}
		printf(LISTFORMAT "\"%s\"\n", _("Caller group"),DecodeUnicodeConsole(caller[entry->Number-1].Text));
		return;
	case PBK_RingtoneID	     :
		if (!ringinit) {
			error=GSM_GetRingtonesInfo(gsm,&Info);
			if (error != ERR_NOTSUPPORTED) Print_Error(error);
			if (error == ERR_NONE) ringinit = true;
		}
		if (ringinit) {
			for (z=0;z<Info.Number;z++) {
				if (Info.Ringtone[z].ID == entry->Number) {
					printf(LISTFORMAT "\"%s\"\n", _("Ringtone"),DecodeUnicodeConsole(Info.Ringtone[z].Name));
					break;
				}
			}
		} else {
			printf(LISTFORMAT "%i\n", _("Ringtone ID"),entry->Number);
		}
		return;
	case PBK_Text_UserID:
		printf(LISTFORMAT "%s\n", _("User ID"),DecodeUnicodeString(entry->Text));
		return;
	case PBK_Text_PictureName:
		printf(LISTFORMAT "%s\n", _("Picture name"),DecodeUnicodeString(entry->Text));
		return;
	case PBK_PictureID	     :
		printf(LISTFORMAT "0x%x\n", _("Picture ID"),entry->Number);
		return;
	case PBK_Photo:
		printf(LISTFORMAT "%s\n", _("Photo"), _("Displaying not supported"));
		return;
	case PBK_Number_Messaging   : printf(LISTFORMAT, _("Favorite messaging number")); break;
	case PBK_Number_General     : printf(LISTFORMAT, _("General number")); break;
	case PBK_Number_Mobile      : printf(LISTFORMAT, _("Mobile number")); break;
	case PBK_Number_Work        : printf(LISTFORMAT, _("Work number")); break;
	case PBK_Number_Fax         : printf(LISTFORMAT, _("Fax number")); break;
	case PBK_Number_Home        : printf(LISTFORMAT, _("Home number")); break;
	case PBK_Number_Pager       : printf(LISTFORMAT, _("Pager number")); break;
	case PBK_Number_Other       : printf(LISTFORMAT, _("Other number")); break;
	case PBK_Text_Note          : printf(LISTFORMAT, _("Text")); break;
	case PBK_Text_Postal        : printf(LISTFORMAT, _("Snail address")); break;
	case PBK_Text_WorkPostal    : printf(LISTFORMAT, _("Work snail address")); break;
	case PBK_Text_Email         : printf(LISTFORMAT, _("Email address 1")); break;
	case PBK_Text_Email2        : printf(LISTFORMAT, _("Email address 2")); break;
	case PBK_Text_URL           : printf(LISTFORMAT, _("URL address")); break;
	case PBK_Text_LUID          : printf(LISTFORMAT, _("LUID")); break;
	case PBK_Text_Name          : printf(LISTFORMAT, _("Name")); break;
	case PBK_Text_LastName      : printf(LISTFORMAT, _("Last name")); break;
	case PBK_Text_FirstName     : printf(LISTFORMAT, _("First name")); break;
	case PBK_Text_FormalName     : printf(LISTFORMAT, _("Formal name")); break;
	case PBK_Text_NickName     : printf(LISTFORMAT, _("Nick name")); break;
	case PBK_Text_Company       : printf(LISTFORMAT, _("Company")); break;
	case PBK_Text_JobTitle      : printf(LISTFORMAT, _("Job title")); break;
	case PBK_Text_StreetAddress : printf(LISTFORMAT, _("Street address")); break;
	case PBK_Text_City          : printf(LISTFORMAT, _("City")); break;
	case PBK_Text_State         : printf(LISTFORMAT, _("State")); break;
	case PBK_Text_Zip           : printf(LISTFORMAT, _("Zip code")); break;
	case PBK_Text_Country       : printf(LISTFORMAT, _("Country")); break;
	case PBK_Text_WorkStreetAddress : printf(LISTFORMAT, _("Work street address")); break;
	case PBK_Text_WorkCity      : printf(LISTFORMAT, _("Work city")); break;
	case PBK_Text_WorkState     : printf(LISTFORMAT, _("Work state")); break;
	case PBK_Text_WorkZip       : printf(LISTFORMAT, _("Work zip code")); break;
	case PBK_Text_WorkCountry   : printf(LISTFORMAT, _("Work country")); break;
	case PBK_Text_Custom1       : printf(LISTFORMAT, _("Custom text 1")); break;
	case PBK_Text_Custom2       : printf(LISTFORMAT, _("Custom text 2")); break;
	case PBK_Text_Custom3       : printf(LISTFORMAT, _("Custom text 3")); break;
	case PBK_Text_Custom4       : printf(LISTFORMAT, _("Custom text 4")); break;
	case PBK_PushToTalkID       : printf(LISTFORMAT, _("Push to talk ID")); break;
#ifndef CHECK_CASES
	default:
		printf("%s\n", _("unknown field type"));
		return;
#endif
	}
	printf("\"%s\"\n", DecodeUnicodeConsole(entry->Text));
}

void PrintMemoryEntry(GSM_MemoryEntry *entry)
{
	int i;

	for (i=0;i<entry->EntriesNum;i++) PrintMemorySubEntry(&entry->Entries[i]);
	printf("\n");
}

void GetAllMemory(int argc UNUSED, char *argv[])
{
	GSM_MemoryEntry		Entry;
	bool			start = true;
	GSM_Error error;

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));

	Entry.MemoryType = MemoryTypeFromString(argv[2]);
	Entry.Location = 0;

	if (Entry.MemoryType == 0) {
		printf_err(_("Unknown memory type (\"%s\")\n"),argv[2]);
		exit (-1);
	}

	GSM_Init(true);

	while (!gshutdown) {
		error = GSM_GetNextMemory(gsm, &Entry, start);
		if (error == ERR_EMPTY) break;
		if (error != ERR_NONE && Info.Ringtone) free(Info.Ringtone);
		Print_Error(error);
		printf(_("Memory %s, Location %i\n"),argv[2],Entry.Location);
		PrintMemoryEntry(&Entry);
 		start = false;
	}

 	if (Info.Ringtone) free(Info.Ringtone);

	GSM_Terminate();
}

void GetMemory(int argc, char *argv[])
{
	int			j, start, stop, emptynum = 0, fillednum = 0;
	GSM_MemoryEntry		entry;
	bool			empty = true;
	double version;
	GSM_Error error;

	entry.MemoryType = MemoryTypeFromString(argv[2]);

	if (entry.MemoryType==0) {
		printf_err(_("Unknown memory type (\"%s\")\n"),argv[2]);
		exit (-1);
	}

	GetStartStop(&start, &stop, 3, argc, argv);

	if (argc > 5 && strcmp(argv[5],"")) {
		if (strcasecmp(argv[5],"-nonempty") == 0) {
			empty = false;
		} else {
			printf_err(_("Unknown parameter (\"%s\")\n"),argv[5]);
			exit (-1);
		}
	}

	GSM_Init(true);

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
			if (Info.Ringtone) free(Info.Ringtone);
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
			PrintMemoryEntry(&entry);
		}
	}

	printf(_("%i entries empty, %i entries filled\n"),emptynum,fillednum);

 	if (Info.Ringtone) free(Info.Ringtone);

	GSM_Terminate();
}

void DeleteMemory(int argc, char *argv[])
{
	int			j, start, stop;
	GSM_MemoryEntry		entry;
	GSM_Error error;

	entry.MemoryType = MemoryTypeFromString(argv[2]);

	if (entry.MemoryType==0) {
		printf_err(_("Unknown memory type (\"%s\")\n"),argv[2]);
		exit (-1);
	}

	GetStartStop(&start, &stop, 3, argc, argv);

	GSM_Init(true);

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

	MemoryType = MemoryTypeFromString(argv[2]);

	if (MemoryType == 0) {
		printf_err(_("Unknown memory type (\"%s\")\n"),argv[2]);
		exit (-1);
	}

	GSM_Init(true);

	error = GSM_DeleteAllMemory(gsm, MemoryType);

	if (error != ERR_NONE) Print_Error(error);

	GSM_Terminate();
}

void SearchOneEntry(GSM_MemoryEntry *Entry, unsigned char *Text)
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
			case PBK_Caller_Group       :
				if (mywstrstr(Entry->Entries[i].Text, Text) != NULL) {
					fprintf(stderr,"\n");
					printf(_("Memory %s, Location %i\n"),GSM_MemoryTypeToString(Entry->MemoryType),Entry->Location);
					PrintMemoryEntry(Entry);
					return;
				}
				break;
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

void SearchOneMemory(GSM_MemoryType MemoryType, char *Title, unsigned char *Text)
{
	GSM_MemoryEntry		Entry;
	GSM_MemoryStatus	Status;
	int			i = 0, l = 1;
	bool			start = true;
	GSM_Error error;

	Status.MemoryType = MemoryType;
	Entry.MemoryType  = MemoryType;

	if (GSM_GetMemoryStatus(gsm, &Status) == ERR_NONE) {
		fprintf(stderr,"%c%s: %i%%", 13, Title, (i+1)*100/(Status.MemoryUsed+1));
		error = GSM_GetNextMemory(gsm, &Entry, true);
		if (error != ERR_NOTSUPPORTED && error != ERR_NOTIMPLEMENTED) {
			while (i < Status.MemoryUsed) {
				if (gshutdown) return;
				i++;
				fprintf(stderr,"\r%s: %i%%", Title, (i+1)*100/(Status.MemoryUsed+1));
				error = GSM_GetNextMemory(gsm, &Entry, start);
				if (error == ERR_EMPTY) break;
				Print_Error(error);
				SearchOneEntry(&Entry, Text);
				start = false;
			}
		} else {
			while (i < Status.MemoryUsed) {
				Entry.Location = l;
				error = GSM_GetMemory(gsm, &Entry);
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

	GSM_Init(true);

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
	bool			start = true;
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
				PrintMemoryEntry(&Entry);
			}
		}
 		start = false;
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
	bool			Number = true;;

	GSM_Init(true);

	signal(SIGINT, interrupt);
	fprintf(stderr, "%s\n", _("Press Ctrl+C to break..."));

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


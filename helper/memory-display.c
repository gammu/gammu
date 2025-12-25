#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <stdlib.h>
#include <ctype.h>

#include <gammu.h>

#include "locales.h"

#include "memory-display.h"
#include "formats.h"

#define GAMMU_CALLER_GROUPS	20

static GSM_Bitmap		caller[GAMMU_CALLER_GROUPS];
GSM_AllRingtonesInfo 	ringtones_info = {0, NULL};
static gboolean			callerinit[GAMMU_CALLER_GROUPS] = {FALSE, FALSE, FALSE, FALSE, FALSE};
static gboolean			ringinit = FALSE;

GSM_Error PrintMemorySubEntry(GSM_SubMemoryEntry *entry, GSM_StateMachine *sm)
{
	GSM_Category	Category;
	int		z;
	GSM_Error error;
	gboolean show_location = TRUE;

	switch (entry->EntryType) {
	case PBK_CallLength:
		printf(LISTFORMAT, _("Call length"));
		/* l10n: Call length format string hour:minute:second*/
		printf(_("%02i:%02i:%02i\n"),
			entry->CallLength / (60 * 60),
			entry->CallLength / 60,
			entry->CallLength % 60);
		return ERR_NONE;
	case PBK_Date:
		printf(LISTFORMAT "%s\n", _("Date and time"),OSDateTime(entry->Date,FALSE));
		return ERR_NONE;
	case PBK_LastModified:
		printf(LISTFORMAT "%s\n", _("Last modified"), OSDateTime(entry->Date,FALSE));
		return ERR_NONE;
	case PBK_Category:
		if (entry->Number == -1) {
			printf(LISTFORMAT "\"%s\"\n", _("Category"), DecodeUnicodeConsole(entry->Text));
		} else {
			if (sm == NULL) {
				error = ERR_NOTSUPPORTED;
			} else {
				Category.Location = entry->Number;
				Category.Type = Category_Phonebook;
				error=GSM_GetCategory(sm, &Category);
			}
			if (error == ERR_NONE) {
				printf(LISTFORMAT "\"%s\" (%i)\n", _("Category"), DecodeUnicodeConsole(Category.Name), entry->Number);
			} else {
				printf(LISTFORMAT "%i\n", _("Category"), entry->Number);
			}
		}
		return ERR_NONE;
	case PBK_Private:
		printf(LISTFORMAT "%s\n", _("Private"), entry->Number == 1 ? "Yes" : "No");
		return ERR_NONE;
	case PBK_Caller_Group       :
		if (sm == NULL) {
			printf(LISTFORMAT "\"%d\"\n", _("Caller group"),entry->Number);
			return ERR_NONE;
		}
		if (entry->Number >= GAMMU_CALLER_GROUPS) {
			printf(LISTFORMAT "\"%d\"\n", _("Caller group"),entry->Number);
			fprintf(stderr, "%s\n", _("Caller group number too high, please increase buffer in sources!"));
			return ERR_MOREMEMORY;
		}
		if (!callerinit[entry->Number-1]) {
			caller[entry->Number-1].Type	    = GSM_CallerGroupLogo;
			caller[entry->Number-1].Location = entry->Number;
			error=GSM_GetBitmap(sm,&caller[entry->Number-1]);
			if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
				printf(LISTFORMAT "\"%d\"\n", _("Caller group"), entry->Number);
				return ERR_NONE;
			}
			if (error != ERR_NONE) {
				return error;
			}
			if (caller[entry->Number-1].DefaultName) {
				NOKIA_GetDefaultCallerGroupName(&caller[entry->Number-1]);
			}
			callerinit[entry->Number-1]=TRUE;
		}
		printf(LISTFORMAT "\"%s\"\n", _("Caller group"),DecodeUnicodeConsole(caller[entry->Number-1].Text));
		return ERR_NONE;
	case PBK_RingtoneID	     :
		if (!ringinit && sm != NULL) {
			error=GSM_GetRingtonesInfo(sm,&ringtones_info);
			if (error == ERR_NONE) {
				ringinit = TRUE;
			} else if (error != ERR_NOTSUPPORTED) {
				return error;
			}
		}
		if (ringinit) {
			for (z=0;z<ringtones_info.Number;z++) {
				if (ringtones_info.Ringtone[z].ID == entry->Number) {
					printf(LISTFORMAT "\"%s\"\n", _("Ringtone"),DecodeUnicodeConsole(ringtones_info.Ringtone[z].Name));
					break;
				}
			}
		} else {
			printf(LISTFORMAT "%i\n", _("Ringtone ID"),entry->Number);
		}
		return ERR_NONE;
	case PBK_Text_UserID:
		printf(LISTFORMAT "%s\n", _("User ID"),DecodeUnicodeString(entry->Text));
		return ERR_NONE;
	case PBK_Text_PictureName:
		printf(LISTFORMAT "%s\n", _("Picture name"),DecodeUnicodeString(entry->Text));
		return ERR_NONE;
	case PBK_PictureID	     :
		printf(LISTFORMAT "0x%x\n", _("Picture ID"),entry->Number);
		return ERR_NONE;
	case PBK_Photo:
		printf(LISTFORMAT "%s\n", _("Photo"), _("Displaying not supported"));
		return ERR_NONE;
	case PBK_Number_Messaging   : printf(LISTFORMAT, _("Favorite messaging number")); break;
	case PBK_Number_General     :
		show_location = FALSE;
		switch (entry->Location) {
			case PBK_Location_Home:
				printf(LISTFORMAT, _("Home number"));
				break;
			case PBK_Location_Work:
				printf(LISTFORMAT, _("Work number"));
				break;
			case PBK_Location_Unknown:
				printf(LISTFORMAT, _("General number"));
				break;
		}
		break;
	case PBK_Number_Video     : printf(LISTFORMAT, _("Video number")); break;
	case PBK_Number_Mobile      :
		show_location = FALSE;
		switch (entry->Location) {
			case PBK_Location_Home:
				printf(LISTFORMAT, _("Home mobile number"));
				break;
			case PBK_Location_Work:
				printf(LISTFORMAT, _("Work mobile number"));
				break;
			case PBK_Location_Unknown:
				printf(LISTFORMAT, _("Mobile number"));
				break;
		}
		break;
	case PBK_Number_Fax         :
		show_location = FALSE;
		switch (entry->Location) {
			case PBK_Location_Home:
				printf(LISTFORMAT, _("Home fax number"));
				break;
			case PBK_Location_Work:
				printf(LISTFORMAT, _("Work fax number"));
				break;
			case PBK_Location_Unknown:
				printf(LISTFORMAT, _("Fax number"));
				break;
		}
		break;
	case PBK_Number_Pager       : printf(LISTFORMAT, _("Pager number")); break;
	case PBK_Number_Other       : printf(LISTFORMAT, _("Other number")); break;
	case PBK_Text_Note          : printf(LISTFORMAT, _("Text")); break;
	case PBK_Text_Postal        :
		show_location = FALSE;
		switch (entry->Location) {
			case PBK_Location_Home:
				printf(LISTFORMAT, _("Home address"));
				break;
			case PBK_Location_Work:
				printf(LISTFORMAT, _("Work address"));
				break;
			case PBK_Location_Unknown:
				printf(LISTFORMAT, _("Address"));
				break;
		}
		break;
	case PBK_Text_Email         :
		show_location = FALSE;
		switch (entry->Location) {
			case PBK_Location_Home:
				printf(LISTFORMAT, _("Home email"));
				break;
			case PBK_Location_Work:
				printf(LISTFORMAT, _("Work email"));
				break;
			case PBK_Location_Unknown:
				printf(LISTFORMAT, _("Email"));
				break;
		}
		break;
	case PBK_Text_Email2        : printf(LISTFORMAT, _("Email address 2")); break;
	case PBK_Text_URL           :
		show_location = FALSE;
		switch (entry->Location) {
			case PBK_Location_Home:
				printf(LISTFORMAT, _("Home website"));
				break;
			case PBK_Location_Work:
				printf(LISTFORMAT, _("Work website"));
				break;
			case PBK_Location_Unknown:
				printf(LISTFORMAT, _("Website"));
				break;
		}
		break;
	case PBK_Text_LUID          : printf(LISTFORMAT, _("LUID")); break;
	case PBK_Text_VOIP          : printf(LISTFORMAT, _("VOIP")); break;
	case PBK_Text_SWIS          : printf(LISTFORMAT, _("SWIS")); break;
	case PBK_Text_WVID          : printf(LISTFORMAT, _("WVID")); break;
	case PBK_Text_SIP          : printf(LISTFORMAT, _("SIP")); break;
	case PBK_Text_DTMF          : printf(LISTFORMAT, _("DTMF")); break;
	case PBK_Text_Name          : printf(LISTFORMAT, _("Name")); break;
	case PBK_Text_LastName      : printf(LISTFORMAT, _("Last name")); break;
	case PBK_Text_FirstName     : printf(LISTFORMAT, _("First name")); break;
	case PBK_Text_SecondName     : printf(LISTFORMAT, _("Second name")); break;
	case PBK_Text_FormalName     : printf(LISTFORMAT, _("Formal name")); break;
	case PBK_Text_NamePrefix     : printf(LISTFORMAT, _("Name prefix")); break;
	case PBK_Text_NameSuffix     : printf(LISTFORMAT, _("Name suffix")); break;
	case PBK_Text_NickName     : printf(LISTFORMAT, _("Nick name")); break;
	case PBK_Text_Company       : printf(LISTFORMAT, _("Company")); break;
	case PBK_Text_JobTitle      : printf(LISTFORMAT, _("Job title")); break;
	case PBK_Text_StreetAddress : printf(LISTFORMAT, _("Street address")); break;
	case PBK_Text_City          : printf(LISTFORMAT, _("City")); break;
	case PBK_Text_State         : printf(LISTFORMAT, _("State")); break;
	case PBK_Text_Zip           : printf(LISTFORMAT, _("Zip code")); break;
	case PBK_Text_Country       : printf(LISTFORMAT, _("Country")); break;
	case PBK_Text_Custom1       : printf(LISTFORMAT, _("Custom text 1")); break;
	case PBK_Text_Custom2       : printf(LISTFORMAT, _("Custom text 2")); break;
	case PBK_Text_Custom3       : printf(LISTFORMAT, _("Custom text 3")); break;
	case PBK_Text_Custom4       : printf(LISTFORMAT, _("Custom text 4")); break;
	case PBK_PushToTalkID       : printf(LISTFORMAT, _("Push to talk ID")); break;
#ifndef CHECK_CASES
	default:
		printf("%s\n", _("unknown field type"));
		return ERR_NONE;
#endif
	}
	if (show_location) {
		switch (entry->Location) {
			case PBK_Location_Home:
				printf("[%s]", _("home"));
				break;
			case PBK_Location_Work:
				printf("[%s]", _("work"));
				break;
			case PBK_Location_Unknown:
				break;
		}
	}
	printf("\"%s\"\n", DecodeUnicodeConsole(entry->Text));
	return ERR_NONE;
}

GSM_Error PrintMemoryEntry(GSM_MemoryEntry *entry, GSM_StateMachine *sm)
{
	int i;
	GSM_Error error;

	for (i = 0; i < entry->EntriesNum; i++) {
		error = PrintMemorySubEntry(&entry->Entries[i], sm);
		if (error != ERR_NONE) return error;
	}
	printf("\n");
	return ERR_NONE;
}

GSM_Error PrintMemoryEntryLocation(GSM_MemoryEntry *entry, GSM_StateMachine *sm)
{
	printf(_("Memory %s, Location %i\n"), GSM_MemoryTypeToString(entry->MemoryType), entry->Location);
	return PrintMemoryEntry(entry, sm);
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

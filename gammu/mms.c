#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "../helper/locales.h"

#include "common.h"
#include "message.h"
#include "../helper/formats.h"
#include "calendar.h"

#include "../helper/memory-display.h"
#include "../helper/printing.h"
#include "../helper/string.h"


void GetMMSFolders(int argc UNUSED, char *argv[] UNUSED)
{
	GSM_Error error;
	GSM_MMSFolders folders;
	int i;

	GSM_Init(TRUE);

	error=GSM_GetMMSFolders(gsm,&folders);
	Print_Error(error);

	for (i=0;i<folders.Number;i++) {
		printf("%i. \"%s\"",i+1,DecodeUnicodeConsole(folders.Folder[i].Name));
		if (folders.Folder[i].InboxFolder) printf("%s", _(", Inbox folder"));
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
			case MMSADDRESS_PHONE: printf("%s", _("phone ")); break;
			default: 				 break;
		}
		printf("%s\n",DecodeUnicodeString(info.Source));
	}
	if (UnicodeLength(info.Destination) != 0) {
		printf(LISTFORMAT, _("Recipient"));
		switch (info.DestinationType) {
			case MMSADDRESS_PHONE: printf("%s", _("phone ")); break;
			default: 				 break;
		}
		printf("%s\n",DecodeUnicodeString(info.Destination));
	}
	if (UnicodeLength(info.CC) != 0) {
		printf(LISTFORMAT, _("CC"));
		switch (info.CCType) {
			case MMSADDRESS_PHONE: printf("%s", _("phone ")); break;
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
			if (error == ERR_NONE) PrintMemoryEntry(&pbk, gsm);
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
	gboolean			start = TRUE;
	GSM_MMSFolders 		folders;
	int			Handle,Size,num = -1;
	GSM_Error error;

	if (argc>2 && strcasecmp(argv[2],"-save") == 0) num=0;

	GSM_Init(TRUE);

	error=GSM_GetMMSFolders(gsm,&folders);
	Print_Error(error);

	File.Buffer = NULL;

	while (1) {
		error = GSM_GetNextMMSFileInfo(gsm,File.ID_FullName,&FileFolder,start);
		if (error == ERR_EMPTY) break;
		Print_Error(error);
		start = FALSE;

		printf(_("Folder %s\n"),DecodeUnicodeConsole(folders.Folder[FileFolder-1].Name));
		printf(LISTFORMAT "\"%s\"\n", _("  File filesystem ID"),DecodeUnicodeConsole(File.ID_FullName));
		if (File.Buffer != NULL) {
			free(File.Buffer);
			File.Buffer = NULL;
		}
		File.Used = 0;
		while (TRUE) {
			error = GSM_GetFilePart(gsm,&File,&Handle,&Size);
			if (error == ERR_EMPTY) break;
			Print_Error(error);
			fprintf(stderr, "\r");
			fprintf(stderr, "%s ", _("Reading:"));
			fprintf(stderr, _("%i percent"),
				(int)(File.Used * 100 / Size));
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

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */


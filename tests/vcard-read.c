/**
 * vCard parser testing.
 *
 * First parameter is location of vCard, second location of Gammu backup
 * how it should be parsed.
 *
 * Optional third parameter can be used to generate template backup
 * file.
 */
#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

int main(int argc, char **argv)
{
	size_t pos = 0;
	GSM_MemoryEntry pbk;
	GSM_Error error;
	char buffer[65536];
	char vcard_buffer[65536];
	FILE *f;
	size_t len;
	bool generate = false;
	GSM_Backup backup;
	int i;
	GSM_Debug_Info *debug_info;

	/* Configure debugging */
	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, false, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	/* Check parameters */
	if (argc != 3 && argc != 4) {
		printf("Not enough parameters!\nUsage: vcard-read file.vcf file.backup\n");
		return 1;
	}

	/* Check for generating option */
	if (argc == 4 && strcmp(argv[3], "generate") == 0) {
		generate = true;
	}

	/* Open file */
	f = fopen(argv[1], "r");
	if (f == NULL) {
		printf("Could not open %s\n", argv[1]);
		return 1;
	}

	/* Read data */
	len = fread(buffer, 1, sizeof(buffer) - 1, f);
	if (!feof(f)) {
		printf("Could not read whole file %s\n", argv[1]);
		return 1;
	}

	/* Zero terminate string */
	buffer[len] = 0;

	/* We don't need file any more */
	fclose(f);

	/* Parse vCard */
	error = GSM_DecodeVCARD(NULL, buffer, &pos, &pbk, SonyEricsson_VCard21);
	gammu_test_result(error, "GSM_DecodeVCARD");

	/* Encode vCard back */
	pos = 0;
	error = GSM_EncodeVCARD(NULL, vcard_buffer, sizeof(vcard_buffer), &pos, &pbk, true, SonyEricsson_VCard21);
	test_result(error == ERR_NONE);

	/*
	 * Best would be to compare here, but we never can get
	 * absolutely same as original.
	 */
	printf("ORIGINAL:\n%s\n----\nENCODED:\n%s\n", buffer, vcard_buffer);

	/* Generate file if we should */
	if (generate) {
		GSM_ClearBackup(&backup);
		strcpy(backup.Creator, "vCard tester");
		pbk.Location = 0;
		backup.PhonePhonebook[0] = &pbk;
		backup.PhonePhonebook[1] = NULL;
		if (GSM_SaveBackupFile(argv[2], &backup, true) != ERR_NONE) {
			printf("Error saving backup to %s\n", argv[2]);
			return 1;
		}
	}

	/* Read file content */
	GSM_ClearBackup(&backup);
	error = GSM_ReadBackupFile(argv[2], &backup, GSM_Backup_GammuUCS2);
	gammu_test_result(error, "GSM_ReadBackupFile");

	/* Compare size */
	test_result(pbk.EntriesNum == backup.PhonePhonebook[0]->EntriesNum);

	/* Compare content */
	for (i = 0; i < pbk.EntriesNum; i++) {
		test_result(pbk.Entries[i].EntryType == backup.PhonePhonebook[0]->Entries[i].EntryType);
		printf("Entry type: %d\n", pbk.Entries[i].EntryType);
		switch (pbk.Entries[i].EntryType) {
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
				test_result(mywstrncmp(
					pbk.Entries[i].Text,
					backup.PhonePhonebook[0]->Entries[i].Text,
					0) == true);
				break;
			case PBK_Photo       :
				test_result((pbk.Entries[i].Picture.Length ==
					backup.PhonePhonebook[0]->Entries[i].Picture.Length) &&
					memcmp(
					pbk.Entries[i].Picture.Buffer,
					backup.PhonePhonebook[0]->Entries[i].Picture.Buffer,
					pbk.Entries[i].Picture.Length) == 0);
				free(pbk.Entries[i].Picture.Buffer);
				free(backup.PhonePhonebook[0]->Entries[i].Picture.Buffer);
				break;
			case PBK_Date:
			case PBK_LastModified:
				break;
			case PBK_Category:
			case PBK_Private:
			case PBK_RingtoneID:
			case PBK_PictureID:
			case PBK_CallLength:
			case PBK_Caller_Group       :
				test_result(pbk.Entries[i].Number == backup.PhonePhonebook[0]->Entries[i].Number);
				break;
		}
	}


	/* Free data */
	GSM_FreeBackup(&backup);

	/* We're done */
	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

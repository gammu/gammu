/* (c) 2003-2004 by Marcin Wiacek */

#include <string.h>
#include <ctype.h>

#include <gammu-config.h>

#include "../../misc/coding/coding.h"
#include "../../debug.h"
#include "../gsmlogo.h"
#include "../gsmmisc.h"
#include "backldif.h"

#ifdef GSM_ENABLE_BACKUP

static void SaveLDIFText(FILE *file, const char *Name, const unsigned char *Value)
{
	unsigned char Buffer[1000],Buffer2[1000];

	if (EncodeUTF8(Buffer, Value)) {
		EncodeBASE64(Buffer, Buffer2, strlen(Buffer));
		fprintf(file,"%s:: %s%c%c",Name,Buffer2,13,10);
	} else {
		fprintf(file,"%s: %s%c%c",Name,DecodeUnicodeString(Value),13,10);
	}
}

GSM_Error SaveLDIF(const char *FileName, GSM_Backup *backup)
{
	int 		i, j;
	FILE 		*file;

	file = fopen(FileName, "wb");
	if (file == NULL) return ERR_CANTOPENFILE;

	i=0;
	while (backup->PhonePhonebook[i]!=NULL) {
		for (j=0;j<backup->PhonePhonebook[i]->EntriesNum;j++) {
			switch (backup->PhonePhonebook[i]->Entries[j].EntryType) {
			case PBK_Text_Name:
				SaveLDIFText(file, "dn", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			default:
				break;
			}
		}
		fprintf(file, "objectclass: top%c%c",13,10);
		fprintf(file, "objectclass: person%c%c",13,10);
		fprintf(file, "objectclass: organizationalPerson%c%c",13,10);
		fprintf(file, "objectclass: inetOrgPerson%c%c",13,10);
		fprintf(file, "objectclass: mozillaAbPersonObsolete%c%c",13,10);
		for (j=0;j<backup->PhonePhonebook[i]->EntriesNum;j++) {
			switch (backup->PhonePhonebook[i]->Entries[j].EntryType) {
			case PBK_Text_Postal:
				SaveLDIFText(file, "HomePostalAddress", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_WorkPostal:
				/* This does not conform to standard, but I think it's better to have it saved */
				SaveLDIFText(file, "WorkPostalAddress", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_URL:
				SaveLDIFText(file, "homeurl", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_Name:
				/* Handled above */
				break;
			case PBK_Text_Note:
				SaveLDIFText(file, "Description", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Number_Work:
				/* not exist in Mozilla 1.4 win32 */
				SaveLDIFText(file, "workPhone", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Number_Mobile:
				SaveLDIFText(file, "mobile", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Number_Pager:
				SaveLDIFText(file, "pager", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Number_Messaging:
				SaveLDIFText(file, "messaging", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Number_Fax:
				/* facsimileTelephoneNumber */
				SaveLDIFText(file, "fax", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Number_Home:
				SaveLDIFText(file, "homePhone", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Number_General:
				/* work in Mozilla 1.4 win32 */
				SaveLDIFText(file, "telephoneNumber", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_Email:
				SaveLDIFText(file, "mail", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_Email2:
				SaveLDIFText(file, "mozillaSecondEmail", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_Custom1:
				SaveLDIFText(file, "custom1", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_Custom2:
				SaveLDIFText(file, "custom2", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_Custom3:
				SaveLDIFText(file, "custom3", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_Custom4:
				SaveLDIFText(file, "custom4", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_LUID:
				SaveLDIFText(file, "luid", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_Company:
				SaveLDIFText(file, "o", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_JobTitle:
				SaveLDIFText(file, "title", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_WorkStreetAddress:
				SaveLDIFText(file, "workPostalAddress", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_WorkCity:
				SaveLDIFText(file, "workLocalityName", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_WorkState:
				SaveLDIFText(file, "workState", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_WorkZip:
				SaveLDIFText(file, "workPostalCode", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_WorkCountry:
				SaveLDIFText(file, "workCountryName", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_StreetAddress:
				SaveLDIFText(file, "homePostalAddress", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_City:
				SaveLDIFText(file, "mozillaHomeLocalityName", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_State:
				SaveLDIFText(file, "mozillaHomeState", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_Zip:
				SaveLDIFText(file, "mozillaHomePostalCode", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_Country:
				SaveLDIFText(file, "mozillaHomeCountryName", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_LastName:
				SaveLDIFText(file, "sn", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_FirstName:
				SaveLDIFText(file, "cn", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_NickName:
				SaveLDIFText(file, "nickname", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Text_FormalName:
				SaveLDIFText(file, "cn", backup->PhonePhonebook[i]->Entries[j].Text);
				break;
			case PBK_Number_Other:
			case PBK_Caller_Group:
			case PBK_RingtoneID:
			case PBK_PictureID:
			case PBK_Date:
			case PBK_LastModified:
			case PBK_Text_UserID:
			case PBK_Category:
			case PBK_Private:
			case PBK_CallLength:
			case PBK_Text_PictureName:
			case PBK_PushToTalkID:
			case PBK_Photo:
				dbgprintf(NULL, "Feature missed\n");
				break;
			}
		}
		fprintf(file, "%c%c",13,10);
		i++;
	}
	fclose(file);
	return ERR_NONE;
}

static bool ReadLDIFText(const char *Buffer, const char *Start, char *Value)
{
	unsigned char 	Buffer2[1000],buff[200];
	int 		i;

	Value[0] = 0x00;

	strcpy(buff,Start);
	strcat(buff,":: ");
	if (!strncmp(Buffer,buff,strlen(buff))) {
		i = DecodeBASE64(Buffer+strlen(Start)+3, Buffer2, strlen(Buffer)-(strlen(Start)+3));
		dbgprintf(NULL, "Text after DecodeBASE64 is \"%s\"\n",Buffer2);
		DecodeUTF8(Value, Buffer2, i);
		dbgprintf(NULL, "Text after DecodeUTF8 is \"%s\"\n",DecodeUnicodeString(Value));
		return true;
	}
	strcpy(buff,Start);
	strcat(buff,": ");
	if (!strncmp(Buffer,buff,strlen(buff))) {
		EncodeUnicode(Value,Buffer+strlen(Start)+2,strlen(Buffer)-(strlen(Start)+2));
		dbgprintf(NULL, "Text after EncodeUnicode is \"%s\"\n",DecodeUnicodeString(Value));
		return true;
	}
	return false;
}

static GSM_Error GSM_DecodeLDIFEntry(char *Buffer, size_t *Pos, GSM_MemoryEntry *Pbk)
{
	unsigned char 	Line[2000],Buff[2000],Buff2[2000];
	int		Level = 0;
	GSM_Error	error;

	Buff[0] 	= 0;
	Pbk->EntriesNum = 0;

	while (1) {
		error = MyGetLine(Buffer, Pos, Line, strlen(Buffer), sizeof(Line), false);
		if (error != ERR_NONE) return error;
		if (strlen(Line) == 0) break;
		Pbk->Entries[Pbk->EntriesNum].AddError = ERR_NONE;
		Pbk->Entries[Pbk->EntriesNum].SMSList[0] = 0;
		Pbk->Entries[Pbk->EntriesNum].VoiceTag = 0;
		switch (Level) {
		case 0:
			if (ReadLDIFText(Line, "objectclass", Buff)) {
				sprintf(Buff2,"%s",DecodeUnicodeString(Buff));
				if (!strcmp("mozillaAbPersonObsolete",Buff2)) {
					dbgprintf(NULL, "level1\n");
					Level = 1;
				}
			}
			break;
		case 1:
			if (ReadLDIFText(Line, "dn", Buff)) {
				dbgprintf(NULL, "entries num is %i\n",Pbk->EntriesNum);
				if (Pbk->EntriesNum == 0) return ERR_EMPTY;
				return ERR_NONE;
			}
			if (ReadLDIFText(Line, "givenName", Buff)) {
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Name;
				Pbk->EntriesNum++;
			}
			if (ReadLDIFText(Line, "telephoneNumber", Buff)) {
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_General;
				Pbk->EntriesNum++;
			}
			if (ReadLDIFText(Line, "mobile", Buff)) {
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Mobile;
				Pbk->EntriesNum++;
			}
			if (ReadLDIFText(Line, "pager", Buff)) {
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Pager;
				Pbk->EntriesNum++;
			}
			if (ReadLDIFText(Line, "messaging", Buff)) {
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Messaging;
				Pbk->EntriesNum++;
			}
			if (ReadLDIFText(Line, "workPhone", Buff)) {
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Work;
				Pbk->EntriesNum++;
			}
			if (ReadLDIFText(Line, "fax", Buff)) {
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Fax;
				Pbk->EntriesNum++;
			}
			if (ReadLDIFText(Line, "homePhone",Buff)) {
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Home;
				Pbk->EntriesNum++;
			}
			if (ReadLDIFText(Line, "Description", Buff)) {
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Note;
				Pbk->EntriesNum++;
			}
			if (ReadLDIFText(Line, "HomePostalAddress", Buff)) {
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Postal;
				Pbk->EntriesNum++;
			}
			if (ReadLDIFText(Line, "womePostalAddress", Buff)) {
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_WorkPostal;
				Pbk->EntriesNum++;
			}
			if (ReadLDIFText(Line, "mail", Buff)) {
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Email;
				Pbk->EntriesNum++;
			}
			if (ReadLDIFText(Line, "homeurl", Buff)) {
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_URL;
				Pbk->EntriesNum++;
			}
			if (ReadLDIFText(Line, "luid", Buff)) {
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_LUID;
				Pbk->EntriesNum++;
			}
			/* FIXME: add rest */
			break;
		}
	}

	dbgprintf(NULL, "entries num is %i\n",Pbk->EntriesNum);
	if (Pbk->EntriesNum == 0) return ERR_EMPTY;
	return ERR_NONE;
}

GSM_Error LoadLDIF(const char *FileName, GSM_Backup *backup)
{
	GSM_File 		File;
	GSM_Error		error;
	GSM_MemoryEntry	Pbk;
	int			numPbk = 0;
	size_t Pos = 0;

	File.Buffer = NULL;
	error = GSM_ReadFile(FileName, &File);
	if (error != ERR_NONE) return error;

	while (1) {
		error = GSM_DecodeLDIFEntry(File.Buffer, &Pos, &Pbk);
		if (error == ERR_EMPTY) {
			error = ERR_NONE;
			break;
		}
		if (error != ERR_NONE) break;
		if (numPbk < GSM_BACKUP_MAX_PHONEPHONEBOOK) {
			backup->PhonePhonebook[numPbk] = malloc(sizeof(GSM_MemoryEntry));
		        if (backup->PhonePhonebook[numPbk] == NULL) {
				error = ERR_MOREMEMORY;
				break;
			}
			backup->PhonePhonebook[numPbk + 1] = NULL;
		} else {
			dbgprintf(NULL, "Increase GSM_BACKUP_MAX_PHONEPHONEBOOK\n");
			error = ERR_MOREMEMORY;
			break;
		}
		memcpy(backup->PhonePhonebook[numPbk],&Pbk,sizeof(GSM_MemoryEntry));
		backup->PhonePhonebook[numPbk]->Location 	= numPbk + 1;
		backup->PhonePhonebook[numPbk]->MemoryType 	= MEM_ME;
		numPbk++;
	}

	free(File.Buffer);

	return error;
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

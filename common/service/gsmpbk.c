
#include <string.h>

#include "gsmpbk.h"
#include "../misc/coding.h"

void GSM_PhonebookFindDefaultNameNumberGroup(GSM_PhonebookEntry entry, int *Name, int *Number, int *Group)
{
	int i;

	*Name	= -1;
	*Number = -1;
	*Group	= -1;
	for (i = 0; i < entry.EntriesNum; i++)
	{
		switch (entry.Entries[i].EntryType) {
		case PBK_Number_General : if (*Number	== -1) *Number	= i; break;
		case PBK_Name		: if (*Name	== -1) *Name	= i; break;
		case PBK_Caller_Group	: if (*Group 	== -1) *Group 	= i; break;
		default			:				     break;
		}
	}
}

static void ParseVCardLine(char **pos, char *Name, char *Parameters, char *Value)
{
	int i;

	Name[0] = Parameters[0] = Value[0] = 0;

	if (**pos == 0) return;

	for (i=0; **pos && **pos != ':' && **pos != ';'; i++, (*pos)++) Name[i] = **pos;
	Name[i] = 0;

	//dprintf("ParseVCardLine: name tag = '%s'\n", Name);
	if (**pos == ';') {
                (*pos)++;
		for (i=0; **pos && **pos != ':'; i++, (*pos)++) Parameters[i] = **pos;
                Parameters[i] = ';';
		Parameters[i+1] = 0;
		//dprintf("ParseVCardLine: parameter tag = '%s'\n", Parameters);
	}

	if (**pos != 0) (*pos)++;

	i=0;
	while (**pos) {
		if ((*pos)[0] == '\x0d' && (*pos)[1] == '\x0a') {
			(*pos) += 2;
			if (**pos != '\t' && **pos != ' ') break;
			while (**pos == '\t' || **pos == ' ') (*pos)++;
                        continue;
		}
		Value[i++] = **pos;
                (*pos)++;
	}
        Value[i] = 0;

	//dprintf("ParseVCardLine: value tag = '%s'\n", Value);
}

void DecodeVCARD21Text(char *VCard, GSM_PhonebookEntry *pbk)
{
	char *pos = VCard;
	char Name[32], Parameters[256], Value[1024];

	dprintf("Parsing VCard:\n%s\n", VCard);

	ParseVCardLine(&pos, Name, Parameters, Value);
	if (!mystrncasecmp(Name, "BEGIN", 0) || !mystrncasecmp(Value, "VCARD", 0))
	{
                dprintf("No valid VCARD signature\n");
		return;
	}

	while (1) {
                GSM_SubPhonebookEntry *pbe = &pbk->Entries[pbk->EntriesNum];

		ParseVCardLine(&pos, Name, Parameters, Value);
		if (Name[0] == 0x00 ||
		    (mystrncasecmp(Name, "END", 0) && mystrncasecmp(Value, "VCARD", 0)))
                        return;

		if (mystrncasecmp(Name, "N", 0)) {
			//FIXME: Name is tagged field which should be parsed
			pbe->EntryType = PBK_Name;
			EncodeUnicode(pbe->Text, Value, strlen(Value));
                        pbk->EntriesNum++;
		} else if (mystrncasecmp(Name, "EMAIL", 0)) {
			pbe->EntryType = PBK_Text_Email;
			EncodeUnicode(pbe->Text, Value, strlen(Value));
                        pbk->EntriesNum++;
		} else if (mystrncasecmp(Name, "TEL", 0)) {
			if (strstr(Parameters, "WORK;"))
				pbe->EntryType = PBK_Number_Work;
			else if (strstr(Name, "HOME;"))
				pbe->EntryType = PBK_Number_Home;
			else if (strstr(Name, "FAX;"))
				pbe->EntryType = PBK_Number_Fax;
			else	pbe->EntryType = PBK_Number_General;

			EncodeUnicode(pbe->Text, Value, strlen(Value));
			pbk->EntriesNum++;
		}
	}
}

void NOKIA_EncodeVCARD10SMSText(char *Buffer, int *Length, GSM_PhonebookEntry pbk)
{
	int Name, Number, Group;

	GSM_PhonebookFindDefaultNameNumberGroup(pbk, &Name, &Number, &Group);

	*Length+=sprintf(Buffer+(*Length),"BEGIN:VCARD%c%c",13,10);
	if (Name != -1) {
		*Length+=sprintf(Buffer+(*Length),"N:%s%c%c",DecodeUnicodeString(pbk.Entries[Name].Text),13,10);
	}
	if (Number != -1) {
		*Length +=sprintf(Buffer+(*Length),"TEL:%s%c%c",DecodeUnicodeString(pbk.Entries[Number].Text),13,10);
	}
	*Length+=sprintf(Buffer+(*Length),"END:VCARD%c%c",13,10);
}

void NOKIA_EncodeVCARD21SMSText(char *Buffer, int *Length, GSM_PhonebookEntry pbk)
{
	char	buffer[1000];
	int	Name, Number, i;
	bool	ignore;

	GSM_PhonebookFindDefaultNameNumberGroup(pbk, &Name, &Number, &i);

	*Length+=sprintf(Buffer+(*Length),"BEGIN:VCARD%c%cVERSION:2.1%c%c",13,10,13,10);
	if (Name != -1) {
		EncodeUTF8(buffer,pbk.Entries[Name].Text);
		if (strlen(DecodeUnicodeString(pbk.Entries[Name].Text))==strlen(buffer)) {
			(*Length)+=sprintf(Buffer+(*Length),"N:%s%c%c",DecodeUnicodeString(pbk.Entries[Name].Text),13,10);
		} else {
			(*Length)+=sprintf(Buffer+(*Length),"N;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:%s%c%c",buffer,13,10);
		}
	}
	if (Number != -1) {
		(*Length)+=sprintf(Buffer+(*Length),"TEL;PREF:%s%c%c",DecodeUnicodeString(pbk.Entries[Number].Text),13,10);
	}
	for (i=0; i < pbk.EntriesNum; i++) {
		if (i != Name && i != Number) {
			ignore = false;
			switch(pbk.Entries[i].EntryType) {
			case PBK_Name		:
			case PBK_Date		:
			case PBK_Caller_Group	:
				ignore = true;
				break;
			case PBK_Number_General	:
				*Length+=sprintf(Buffer+(*Length),"TEL");
				break;
			case PBK_Number_Mobile	:
				*Length+=sprintf(Buffer+(*Length),"TEL;CELL");
				break;
			case PBK_Number_Work	:
				*Length+=sprintf(Buffer+(*Length),"TEL;WORK;VOICE");
				break;
			case PBK_Number_Fax	:
				*Length+=sprintf(Buffer+(*Length),"TEL;FAX");
				break;
			case PBK_Number_Home	:
				*Length+=sprintf(Buffer+(*Length),"TEL;HOME;VOICE");
				break;
			case PBK_Text_Note	:
				*Length+=sprintf(Buffer+(*Length),"NOTE");
				break;
			case PBK_Text_Postal	:
				/* Don't ask why. Nokia phones save postal address
				 * double - once like LABEL, second like ADR
				 */
				*Length+=sprintf(Buffer+(*Length),"LABEL");
				EncodeUTF8(buffer,pbk.Entries[i].Text);
				if (strlen(DecodeUnicodeString(pbk.Entries[i].Text))==strlen(buffer)) {
					(*Length)+=sprintf(Buffer+(*Length),":%s%c%c",DecodeUnicodeString(pbk.Entries[i].Text),13,10);
				} else {
					(*Length)+=sprintf(Buffer+(*Length),";CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:%s%c%c",buffer,13,10);
				}
				*Length+=sprintf(Buffer+(*Length),"ADR");
				break;
			case PBK_Text_Email	:
				*Length+=sprintf(Buffer+(*Length),"EMAIL");
				break;
			case PBK_Text_URL	:
				*Length+=sprintf(Buffer+(*Length),"URL");
				break;
			default	:
				ignore = true;
				break;
			}
			if (!ignore) {
				EncodeUTF8(buffer,pbk.Entries[i].Text);
				if (strlen(DecodeUnicodeString(pbk.Entries[i].Text))==strlen(buffer)) {
					(*Length)+=sprintf(Buffer+(*Length),":%s%c%c",DecodeUnicodeString(pbk.Entries[i].Text),13,10);
				} else {
					(*Length)+=sprintf(Buffer+(*Length),";CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:%s%c%c",buffer,13,10);
				}
			}
		}
	}
	*Length+=sprintf(Buffer+(*Length),"END:VCARD%c%c",13,10);
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

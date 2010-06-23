
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

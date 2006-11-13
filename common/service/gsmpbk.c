/* (c) 2001-2005 by Marcin Wiacek, Michal Cihar... */

#include <string.h>

#include "../misc/coding/coding.h"
#include "gsmpbk.h"
#include "gsmmisc.h"

unsigned char *GSM_PhonebookGetEntryName (GSM_MemoryEntry *entry)
{
        /* We possibly store here "LastName, FirstName" so allocate enough memory */
        static char     dest[(GSM_PHONEBOOK_TEXT_LENGTH*2+2+1)*2];
        static char     split[] = { '\0', ',', '\0', ' ', '\0', '\0'};
        int             i;
        int             first = -1, last = -1, name = -1;
        int             len = 0;

        for (i = 0; i < entry->EntriesNum; i++) {
                switch (entry->Entries[i].EntryType) {
                        case PBK_Text_LastName:
                                last = i;
                                break;
                        case PBK_Text_FirstName:
                                first = i;
                                break;
                        case PBK_Text_Name:
                                name = i;
                                break;
                        default:
                                break;
                }
        }

        if (name != -1) {
                CopyUnicodeString(dest, entry->Entries[name].Text);
        } else {
                if (last != -1 && first != -1) {
                        len = UnicodeLength(entry->Entries[last].Text);
                        CopyUnicodeString(dest, entry->Entries[last].Text);
                        CopyUnicodeString(dest + 2*len, split);
                        CopyUnicodeString(dest + 2*len + 4, entry->Entries[first].Text);
                } else if (last != -1) {
                        CopyUnicodeString(dest, entry->Entries[last].Text);
                } else if (first != -1) {
                        CopyUnicodeString(dest, entry->Entries[first].Text);
                } else {
                        return NULL;
                }
        }

        return dest;
}

void GSM_PhonebookFindDefaultNameNumberGroup(GSM_MemoryEntry *entry, int *Name, int *Number, int *Group)
{
        int i;

        *Name   = -1;
        *Number = -1;
        *Group  = -1;
        for (i = 0; i < entry->EntriesNum; i++) {
                switch (entry->Entries[i].EntryType) {
                case PBK_Number_General : if (*Number   == -1) *Number  = i; break;
                case PBK_Text_Name      : if (*Name     == -1) *Name    = i; break;
                case PBK_Caller_Group   : if (*Group    == -1) *Group   = i; break;
                default                 :                                    break;
                }
        }
        if ((*Number) == -1) {
                for (i = 0; i < entry->EntriesNum; i++) {
                        switch (entry->Entries[i].EntryType) {
                                case PBK_Number_Mobile:
                                case PBK_Number_Work:
                                case PBK_Number_Fax:
                                case PBK_Number_Home:
                                case PBK_Number_Pager:
                                case PBK_Number_Other:
                                        *Number = i;
                                        break;
                                default:
                                        break;
                        }
                        if (*Number != -1) break;
                }
        }
        if ((*Name) == -1) {
                for (i = 0; i < entry->EntriesNum; i++) {
			if (entry->Entries[i].EntryType != PBK_Text_LastName) continue;
                        *Name = i;
			break;
                }
        }
        if ((*Name) == -1) {
                for (i = 0; i < entry->EntriesNum; i++) {
                        if (entry->Entries[i].EntryType != PBK_Text_FirstName) continue;
                        *Name = i;
                        break;
		}
	}
}

void GSM_EncodeVCARD(char *Buffer, int *Length, GSM_MemoryEntry *pbk, bool header, GSM_VCardVersion Version)
{
        int     Name, Number, Group, i;
	int	firstname = -1, lastname = -1;
	int	address = -1, city = -1, state = -1, zip = -1, country = -1;
	unsigned char buffer[1024];
	int pos;
        bool    ignore;

        GSM_PhonebookFindDefaultNameNumberGroup(pbk, &Name, &Number, &Group);

        if (Version == Nokia_VCard10) {
                if (header) *Length+=sprintf(Buffer+(*Length),"BEGIN:VCARD%c%c",13,10);
                if (Name != -1) {
                        *Length+=sprintf(Buffer+(*Length),"N:%s%c%c",DecodeUnicodeString(pbk->Entries[Name].Text),13,10);
                }
                if (Number != -1) {
                        *Length +=sprintf(Buffer+(*Length),"TEL:%s%c%c",DecodeUnicodeString(pbk->Entries[Number].Text),13,10);
                }
                if (header) *Length+=sprintf(Buffer+(*Length),"END:VCARD%c%c",13,10);
        } else if (Version == Nokia_VCard21 || Version == SonyEricsson_VCard21) {
                if (header) *Length+=sprintf(Buffer+(*Length),"BEGIN:VCARD%c%cVERSION:2.1%c%c",13,10,13,10);
                for (i=0; i < pbk->EntriesNum; i++) {
			ignore = false;
			switch(pbk->Entries[i].EntryType) {
                                case PBK_Text_Name:
                                        *Length+=sprintf(Buffer+(*Length),"N");
					break;
                                case PBK_Text_FirstName:
					firstname = i;
                                        ignore = true;
					break;
                                case PBK_Text_LastName:
					lastname = i;
                                        ignore = true;
					break;
				case PBK_Text_StreetAddress:
					address = i;
                                        ignore = true;
					break;
				case PBK_Text_City:
					city = i;
                                        ignore = true;
					break;
				case PBK_Text_State:
					state = i;
                                        ignore = true;
					break;
				case PBK_Text_Zip:
					zip = i;
                                        ignore = true;
					break;
				case PBK_Text_Country:
					country = i;
                                        ignore = true;
					break;
                                case PBK_Date:
					SaveVCALDate(Buffer, Length, &(pbk->Entries[i].Date), "BDAY");
                                        ignore = true;
					break;
                                case PBK_Number_General:
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        break;
                                case PBK_Number_Other:
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        break;
                                case PBK_Number_Pager:
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        *Length+=sprintf(Buffer+(*Length),";PAGER");
                                        break;
                                case PBK_Number_Mobile  :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        *Length+=sprintf(Buffer+(*Length),";CELL");
                                        break;
                                case PBK_Number_Work    :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        *Length+=sprintf(Buffer+(*Length),";WORK;VOICE");
                                        break;
                                case PBK_Number_Fax     :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        *Length+=sprintf(Buffer+(*Length),";FAX");
                                        break;
                                case PBK_Number_Home    :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        *Length+=sprintf(Buffer+(*Length),";HOME;VOICE");
                                        break;
                                case PBK_Text_Note      :
                                        *Length+=sprintf(Buffer+(*Length),"NOTE");
                                        break;
                                case PBK_Text_Postal    :
                                        /* Don't ask why. Nokia phones save postal address
                                         * double - once like LABEL, second like ADR
                                         */
                                        SaveVCALText(Buffer, Length, pbk->Entries[i].Text, "LABEL");
                                        *Length+=sprintf(Buffer+(*Length),"ADR");
                                        break;
                                case PBK_Text_Email     :
                                case PBK_Text_Email2    :
                                        *Length+=sprintf(Buffer+(*Length),"EMAIL");
                                        break;
                                case PBK_Text_URL       :
                                        *Length+=sprintf(Buffer+(*Length),"URL");
                                        break;
                                case PBK_Text_LUID      :
                                        *Length+=sprintf(Buffer+(*Length),"X-IRMC-LUID");
                                        break;
				case PBK_Text_JobTitle:
                                        *Length+=sprintf(Buffer+(*Length),"TITLE");
                                        break;
				case PBK_Text_Company:
                                        *Length+=sprintf(Buffer+(*Length),"ORG");
                                        break;
				/* Not supported fields */
                                case PBK_Caller_Group:
				case PBK_Category:
				case PBK_Private:
				case PBK_RingtoneID:
				case PBK_RingtoneFileSystemID:
				case PBK_PictureID:
				case PBK_SMSListID:
				case PBK_Text_UserID:
				case PBK_CallLength:
				case PBK_Text_Custom1:
				case PBK_Text_Custom2:
				case PBK_Text_Custom3:
				case PBK_Text_Custom4:
                                        ignore = true;
                                        break;
                                }
                                if (!ignore) {
                                        SaveVCALText(Buffer, Length, pbk->Entries[i].Text, "");
                                }
                }
		/* Save name if it is composed from parts */
		if (firstname != -1 || lastname != -1) {
			pos = 0;
			if (lastname != -1) {
				CopyUnicodeString(buffer + 2*pos, pbk->Entries[lastname].Text);
				pos += UnicodeLength(pbk->Entries[lastname].Text);
			}
			buffer[2*pos] = 0;
			buffer[2*pos + 1] = ';';
			pos++;
			if (firstname != -1) {
				CopyUnicodeString(buffer + 2*pos, pbk->Entries[firstname].Text);
				pos += UnicodeLength(pbk->Entries[firstname].Text);
			}
			SaveVCALText(Buffer, Length, buffer, "N");
		}
		/* Save address if it is composed from parts */
		if (address != -1 || city != -1 || state != -1 || zip != -1 || country != -1) {
			pos = 0;
			buffer[2*pos] = 0;
			buffer[2*pos + 1] = ';';
			pos++;
			buffer[2*pos] = 0;
			buffer[2*pos + 1] = ';';
			pos++;
			if (address != -1) {
				CopyUnicodeString(buffer + 2*pos, pbk->Entries[address].Text);
				pos += UnicodeLength(pbk->Entries[address].Text);
			}
			buffer[2*pos] = 0;
			buffer[2*pos + 1] = ';';
			pos++;
			if (city != -1) {
				CopyUnicodeString(buffer + 2*pos, pbk->Entries[city].Text);
				pos += UnicodeLength(pbk->Entries[city].Text);
			}
			buffer[2*pos] = 0;
			buffer[2*pos + 1] = ';';
			pos++;
			if (state != -1) {
				CopyUnicodeString(buffer + 2*pos, pbk->Entries[state].Text);
				pos += UnicodeLength(pbk->Entries[state].Text);
			}
			buffer[2*pos] = 0;
			buffer[2*pos + 1] = ';';
			pos++;
			if (zip != -1) {
				CopyUnicodeString(buffer + 2*pos, pbk->Entries[zip].Text);
				pos += UnicodeLength(pbk->Entries[zip].Text);
			}
			buffer[2*pos] = 0;
			buffer[2*pos + 1] = ';';
			pos++;
			if (country != -1) {
				CopyUnicodeString(buffer + 2*pos, pbk->Entries[country].Text);
				pos += UnicodeLength(pbk->Entries[country].Text);
			}
			SaveVCALText(Buffer, Length, buffer, "ADR");
		}
                if (header) *Length+=sprintf(Buffer+(*Length),"END:VCARD%c%c",13,10);
        }
}

GSM_Error GSM_DecodeVCARD(unsigned char *Buffer, int *Pos, GSM_MemoryEntry *Pbk, GSM_VCardVersion Version)
{
        unsigned char   Line[2000],Buff[2000];
        int             Level = 0;
        unsigned char   *s;
	int		pos;
	bool		address = false;

        Buff[0]         = 0;
        Pbk->EntriesNum = 0;

        while (1) {
                MyGetLine(Buffer, Pos, Line, strlen(Buffer), true);
                if (strlen(Line) == 0) break;
                switch (Level) {
                case 0:
                        if (strstr(Line,"BEGIN:VCARD")) Level = 1;
                        break;
                case 1:
			if (Pbk->EntriesNum == GSM_PHONEBOOK_ENTRIES) return ERR_MOREMEMORY;
                        if (strstr(Line,"END:VCARD")) {
                                if (Pbk->EntriesNum == 0) return ERR_EMPTY;
                                return ERR_NONE;
                        }
                        if (ReadVCALText(Line, "N", Buff)) {
				pos = 0;
				s = VCALGetTextPart(Buff, &pos);
				if (s == NULL) {
					CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
					Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Name;
					Pbk->EntriesNum++;
				} else {
					CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text, s);
					/* Skip empty name */
					if (UnicodeLength(Pbk->Entries[Pbk->EntriesNum].Text) > 0) {
						Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_LastName;
						Pbk->EntriesNum++;
					}

					s = VCALGetTextPart(Buff, &pos);
					if (s == NULL) continue;
					if (Pbk->EntriesNum == GSM_PHONEBOOK_ENTRIES) return ERR_MOREMEMORY;
					CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text, s);
					Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_FirstName;
					Pbk->EntriesNum++;
				}
                        }
                        if (ReadVCALText(Line, "TEL",                   Buff) ||
                            ReadVCALText(Line, "TEL;VOICE",             Buff) ||
                            ReadVCALText(Line, "TEL;PREF",              Buff) ||
                            ReadVCALText(Line, "TEL;PREF;VOICE",        Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_General;
				Pbk->Entries[Pbk->EntriesNum].SMSList[0] = 0;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "TEL;CELL",              Buff) ||
                            ReadVCALText(Line, "TEL;CELL;VOICE",        Buff) ||
                            ReadVCALText(Line, "TEL;PREF;CELL",         Buff) ||
                            ReadVCALText(Line, "TEL;PREF;CELL;VOICE",   Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Mobile;
				Pbk->Entries[Pbk->EntriesNum].SMSList[0] = 0;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "TEL;WORK",              Buff) ||
                            ReadVCALText(Line, "TEL;PREF;WORK",         Buff) ||
                            ReadVCALText(Line, "TEL;WORK;VOICE",        Buff) ||
                            ReadVCALText(Line, "TEL;PREF;WORK;VOICE",   Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Work;
				Pbk->Entries[Pbk->EntriesNum].SMSList[0] = 0;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "TEL;FAX",               Buff) ||
                            ReadVCALText(Line, "TEL;PREF;FAX",          Buff) ||
                            ReadVCALText(Line, "TEL;FAX;VOICE",         Buff) ||
                            ReadVCALText(Line, "TEL;PREF;FAX;VOICE",    Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Fax;
				Pbk->Entries[Pbk->EntriesNum].SMSList[0] = 0;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "TEL;HOME",              Buff) ||
                            ReadVCALText(Line, "TEL;PREF;HOME",         Buff) ||
                            ReadVCALText(Line, "TEL;HOME;VOICE",        Buff) ||
                            ReadVCALText(Line, "TEL;PREF;HOME;VOICE",   Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Home;
				Pbk->Entries[Pbk->EntriesNum].SMSList[0] = 0;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "TITLE", Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_JobTitle;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "NOTE", Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Note;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "LABEL", Buff) ||
			    ReadVCALText(Line, "ADR", Buff) ||
                            ReadVCALText(Line, "ADR;HOME", Buff)) {
				if (address) continue;
				address = true;
				pos = 0;
				s = VCALGetTextPart(Buff, &pos); /* PO box, ignore for now */
				if (s == NULL) {
					CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
					Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Postal;
					Pbk->EntriesNum++;
				} else {
					s = VCALGetTextPart(Buff, &pos); /* Don't know ... */

					s = VCALGetTextPart(Buff, &pos);
					if (s == NULL) continue;
					CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text, s);
					Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_StreetAddress;
					Pbk->EntriesNum++;
					if (Pbk->EntriesNum == GSM_PHONEBOOK_ENTRIES) return ERR_MOREMEMORY;

					s = VCALGetTextPart(Buff, &pos);
					if (s == NULL) continue;
					CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text, s);
					Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_City;
					Pbk->EntriesNum++;
					if (Pbk->EntriesNum == GSM_PHONEBOOK_ENTRIES) return ERR_MOREMEMORY;

					s = VCALGetTextPart(Buff, &pos);
					if (s == NULL) continue;
					CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text, s);
					Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_State;
					Pbk->EntriesNum++;
					if (Pbk->EntriesNum == GSM_PHONEBOOK_ENTRIES) return ERR_MOREMEMORY;

					s = VCALGetTextPart(Buff, &pos);
					if (s == NULL) continue;
					CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text, s);
					Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Zip;
					Pbk->EntriesNum++;
					if (Pbk->EntriesNum == GSM_PHONEBOOK_ENTRIES) return ERR_MOREMEMORY;

					s = VCALGetTextPart(Buff, &pos);
					if (s == NULL) continue;
					CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text, s);
					Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Country;
					Pbk->EntriesNum++;
				}
                        }
                        if (ReadVCALText(Line, "EMAIL", Buff) ||
			    ReadVCALText(Line, "EMAIL;INTERNET", Buff) ||
			    ReadVCALText(Line, "EMAIL;INTERNET;PREF", Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Email;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "X-IRMC-LUID", Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_LUID;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "URL", Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_URL;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "ORG", Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Company;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "CATEGORIES", Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].Number = -1;
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Category;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "BDAY", Buff)) {
				if (ReadVCALDateTime(DecodeUnicodeString(Buff), &Pbk->Entries[Pbk->EntriesNum].Date)) {
					Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Date;
					Pbk->EntriesNum++;
				}
                        }
                        break;
                }
        }

        if (Pbk->EntriesNum == 0) return ERR_EMPTY;
        return ERR_NONE;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

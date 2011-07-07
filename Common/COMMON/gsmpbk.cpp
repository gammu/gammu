
#include "stdafx.h"
#include <string.h>

#include "commfun.h"
#include "coding.h"
#include "gsmpbk.h"

void GSM_SemicolonEncode(unsigned char *inputText, unsigned char *outputText)
{
/*	unsigned char uText[2000];
	DecodeUnicode(inputText, uText);
	CString strText = uText;
	strText.Replace(";","\\;");
	sprintf((char *)uText,"%s",strText);
	EncodeUnicode(outputText,uText, strlen((char *)uText));*/
	CopyUnicodeString(outputText,inputText);
	UnicodeReplace(outputText,";","\\;");

}

void GSM_SemicolonDecode(unsigned char *inputText, unsigned char *outputText)
{	
/*	unsigned char uText[2000];
	DecodeUnicode(inputText, uText);
	CString strText = uText;
	strText.Replace("\\;", ";");
	sprintf((char *)uText,"%s",strText);
	EncodeUnicode(outputText,uText, strlen((char *)uText));*/

	CopyUnicodeString(outputText,inputText);
	UnicodeReplace(outputText,"\\;", ";");

}

void SamsungSplitString(char *pString ,int  Split,char* pStr1,char* pStr2)
{
	char *pFind;
	char szTemp[MAX_PATH];
	int length;
	pFind = strchr(pString,Split );
	sprintf(szTemp,"%s",pString);
	if(pFind)
	{
		length = strlen(szTemp) - strlen(pFind);
		while (szTemp[length -1]=='\\')
		{
			pFind = strchr(pFind+1,Split);
			if(pFind)
			{
				length = strlen(szTemp) - strlen(pFind);
			}
			else
			{
				sprintf(pStr1,"%s",pString);
				pStr2[0]='\0';
			}
		}
		sprintf(pStr2	,"%s",pFind+1);
		
		szTemp[strlen(szTemp)-strlen(pFind)] = '\0';
		sprintf(pStr1	,"%s",szTemp);
	}
	else
	{
		sprintf(pStr1,"%s",pString);
		pStr2[0]='\0';
	}
}


void SESaveVCARDText(char *Buffer, int *Length, unsigned char *Text, char *Start,char *beforetext)
{
/*	char buffer[1000];

	if (UnicodeLength(Text) != 0) {
		EncodeCString2UTF8(DecodeUnicodeString(Text),buffer);
		if (UnicodeLength(Text)==strlen(buffer)) {
			*Length+=sprintf(Buffer+(*Length), "%s:;%s%c%c",Start,DecodeUnicodeString(Text),13,10);
		} else {
			*Length+=sprintf(Buffer+(*Length), "%s;CHARSET=UTF-8:%s%c%c",Start,buffer,13,10);
		}
	}*/
	unsigned char buffer[1000];

	if (UnicodeLength(Text) != 0) 
	{
		unsigned char uText[2000];
		DecodeUnicode(Text, uText);
		if(strchr((char*)uText,'\r') || strchr((char*)uText,'\n'))
		{
			EncodeUTF8QuotedPrintable((unsigned char*)buffer,Text); //peggy +	
			*Length+=sprintf(Buffer+(*Length), "%s;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE%s%s%c%c",Start,beforetext,buffer,13,10);
		}
		else
		{
		
		//	EncodeCString2UTF8(DecodeUnicodeString(Text),buffer);
			EncodeUnicode2UTF8(Text,buffer);// for _UNICODE
			if (UnicodeLength(Text)==strlen((char*)buffer)) {
				*Length+=sprintf(Buffer+(*Length), "%s%s%s%c%c",Start,beforetext,DecodeUnicodeString(Text),13,10);
			} else {
				*Length+=sprintf(Buffer+(*Length), "%s;CHARSET=UTF-8%s%s%c%c",Start,beforetext,buffer,13,10);
			}
		}
	}
}

// 9@9u
void SF_SaveVCARDText(char *Buffer, int *Length, unsigned char *Text, char *Start,char *beforetext)
{

	unsigned char buffer[1000];

	if (UnicodeLength(Text) != 0) 
	{
		unsigned char uText[2000];
		DecodeUnicode(Text, uText);
		if(strchr((char*)uText,'\r') || strchr((char*)uText,'\n'))// ???
		{
			EncodeUTF8QuotedPrintable((unsigned char*)buffer,Text); //peggy +	
			*Length+=sprintf(Buffer+(*Length), "%s;ENCODING=QUOTED-PRINTABLE;CHARSET=UTF-8%s%s%c%c%c%c",Start,beforetext,buffer,0x2C,0x2C,0x2C,0x2C);
//			*Length+=sprintf(Buffer+(*Length), "%s;ENCODING=QUOTED-PRINTABLE;CHARSET=UTF-8%s%s%c%c",Start,beforetext,buffer,13,10);
//			*Length+=sprintf(Buffer+(*Length), "%s;ENCODING=QUOTED-PRINTABLE;CHARSET=UTF-8%s%s",Start,beforetext,buffer);
		}
		else // ??? for 9@9u
		{
		
		//	EncodeCString2UTF8(DecodeUnicodeString(Text),buffer);
			EncodeUnicode2UTF8(Text,buffer);// for _UNICODE
			if (UnicodeLength(Text)==strlen((char*)buffer)) {
				*Length+=sprintf(Buffer+(*Length), "%s;ENCODING=QUOTED-PRINTABLE;CHARSET=UTF-8%s%s%c%c%c%c",Start,beforetext,DecodeUnicodeString(Text),0x2C,0x2C,0x2C,0x2C);
			//	*Length+=sprintf(Buffer+(*Length), "%s;ENCODING=QUOTED-PRINTABLE;CHARSET=UTF-8%s%s",Start,beforetext,DecodeUnicodeString(Text));
			} else {
				*Length+=sprintf(Buffer+(*Length), "%s;ENCODING=QUOTED-PRINTABLE;CHARSET=UTF-8%s%s%c%c%c%c",Start,beforetext,buffer,0x2C,0x2C,0x2C,0x2C);
			//	*Length+=sprintf(Buffer+(*Length), "%s;ENCODING=QUOTED-PRINTABLE;CHARSET=UTF-8%s%s",Start,beforetext,buffer);
			}
		}
	}
}



int nSearchstr(unsigned const char *a, unsigned const char *b, int num )
{
	int i,j,k;

	if (a == NULL || b == NULL) return 0;

	if (num == 0)
		return 0;
   	i=0;
	while(1)
	{
		if (a[i] == 0x00 && a[i+1] == 0x00) return 0;
		if(a[i]== b[0])
		{
			k=0;
			for (j = i; j != num+i; j++) 
			{
				if (a[j] == 0x00 || b[k] == 0x00) return 0;

				if (tolower(a[j]) != tolower(b[k]))
					break;
				if(j==num+i-1)
				{
					return i+1;
				}
				k++;
 			}
		}
		i++;
	}
	return 0;
}
bool YYYYMMDDGetDateTime(GSM_DateTime *Date, char * timet)
{
	int num=0 ,timelen=0,op=0;
	char YYYY[5];
	char MM[3];
	char DD[3];
	YYYY[4] =MM[2]= DD[2]= 0;
	timelen= strlen(timet);

	Date->Year	= 0;
	Date->Month	= 0;
	Date->Day	= 0;
	Date->Hour	= 0;
	Date->Minute= 0;
	Date->Second= 0;
	if(timelen < 8) return false;

	memcpy(YYYY,timet,4);
	memcpy(MM,timet+4,2);
	memcpy(DD,timet+6,2);

	Date->Year = atoi(YYYY);
	Date->Month = atoi(MM);
	Date->Day = atoi(DD);
	return true;


}//1976-12-08  ->  GSM_DateTime
bool YYYYMMDD_Get_DateTime(GSM_DateTime *Date, char * timet)
{
	int num=0 ,timelen=0,op=0;
	char YYYY[5];
	char MM[3];
	char DD[3];
	YYYY[4] =MM[2]= DD[2]= 0;

	timelen= strlen(timet);

	Date->Year	= 0;
	Date->Month	= 0;
	Date->Day	= 0;
	Date->Hour	= 0;
	Date->Minute= 0;
	Date->Second= 0;

	num=nSearchstr((unsigned char *)timet,(unsigned char *)"-",1);
	op+=num-5;
	if(op+4> timelen || num<=0)
	{
		char 	*pTemp = strchr(timet,':' );
		if(pTemp)
		{
			return YYYYMMDDGetDateTime(Date,(char*) pTemp+1);
		}
		else	return false;
	}
	memcpy(YYYY,timet+(num-5),4);
	Date->Year	= atoi(YYYY);

	timet+=(num-5);
	timet+=5;
	op+=5;

	num=nSearchstr((unsigned char *)timet,(unsigned char *)"-",1);
	op+=num-3;
	if(op+1> timelen) return false;
	memcpy(MM,timet+(num-3),2);
	Date->Month	= atoi(MM);

	timet+=(num-3);
	timet+=3;
	op+=3;	

	if(op+1> timelen) return false;
	memcpy(DD,timet,2);
	Date->Day	= atoi(DD);

	return true;
}
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
                CopyUnicodeString((unsigned char *)dest, entry->Entries[name].Text);
        } else {
                if (last != -1 && first != -1) {
                        len = UnicodeLength((unsigned char *)entry->Entries[last].Text);
                        CopyUnicodeString((unsigned char *)dest, (unsigned char *)entry->Entries[last].Text);
                        CopyUnicodeString((unsigned char *)dest + 2*len, (unsigned char *)split);
                        CopyUnicodeString((unsigned char *)dest + 2*len + 4, entry->Entries[first].Text);
                } else if (last != -1) {
                        CopyUnicodeString((unsigned char *)dest, entry->Entries[last].Text);
                } else if (first != -1) {
                        CopyUnicodeString((unsigned char *)dest, entry->Entries[first].Text);
                } else {
                        return NULL;
                }
        }

        return (unsigned char *)dest;
}

//Bobby Add Function 8/9/05
void GSM_PhonebookFindDefaultNameNumberGroupSamsung(GSM_MemoryEntry *entry, int *Number, int *FirstName, int *LastName, int *Mobile, int *Home, int *Work, int *Fax, int *Other, int *Email, int *Memo, int *Group, int *Pic, int *PicID, int *Ring, int *RingID)
{
	int i;
	*Number = -1;
	*FirstName = -1;
	*LastName = -1;
	*Mobile = -1;
	*Home = -1;
	*Work = -1;
	*Fax = -1;
	*Other = -1;
	*Email = -1;
	*Memo = -1;
	*Group = -1;
	*Pic = -1;
	*PicID = -1;
	*Ring = -1;
	*RingID = -1;
	for (i = 0; i < entry->EntriesNum; i++) 
	{
		switch (entry->Entries[i].EntryType) 
		{
		case PBK_Number_General : if (*Mobile   == -1) *Mobile  = i; if (*Number   == -1) *Number  = i; break;
		case PBK_Number_Mobile	: if (*Mobile   == -1) *Mobile  = i; if (*Number   == -1) *Number  = i; break;
		case PBK_Text_Name      : if (*FirstName== -1) *FirstName    = i; break;
		case PBK_Text_FirstName	: if (*FirstName== -1) *FirstName    = i; break;
		case PBK_Text_LastName	: if (*LastName	== -1) *LastName= i; break;
		case PBK_Number_Home    : if (*Home		== -1) *Home    = i; break;
		case PBK_Number_Work    : if (*Work     == -1) *Work    = i; break;
		case PBK_Number_Other   : if (*Other	== -1) *Other   = i; break;
		case PBK_Number_Fax		: if (*Fax	    == -1) *Fax		= i; break;
		case PBK_Text_Email		: if (*Email	== -1) *Email	= i; break;
		case PBK_Text_Note		: if (*Memo		== -1) *Memo	= i; break;
		case PBK_Caller_Group	: if (*Group	== -1) *Group	= i; break;
		case PBK_PictureID		: if (*PicID	== -1) *PicID	= i; break;
		case PBK_RingtoneID		: if (*RingID	== -1) *RingID	= i; break;
		case PBK_Text_Ring		: if (*Ring		== -1) *Ring	= i; break;
		case PBK_Text_Picture	: if (*Pic		== -1) *Pic		= i; break;
		default                 :                                    break;
		}
	}
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
void GSM_PhonebookFindDefaultNameNumberGroupSE(GSM_MemoryEntry *entry, int *Name, int *Number, int *Group,int *LastName , int *FirstName)
{
        int i;

        *Name   = -1;
        *Number = -1;
        *Group  = -1;
		*LastName = -1;
		*FirstName = -1;
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
        if ((*LastName) == -1) {
                for (i = 0; i < entry->EntriesNum; i++) {
			if (entry->Entries[i].EntryType != PBK_Text_LastName) continue;
                        *LastName = i;
			break;
                }
        }
        if ((*FirstName) == -1) {
                for (i = 0; i < entry->EntriesNum; i++) {
                        if (entry->Entries[i].EntryType != PBK_Text_FirstName) continue;
                        *FirstName = i;
                        break;
		}
	}
}

void GSM_EncodeVCARD(char *Buffer, int *Length, GSM_MemoryEntry *pbk, bool header, GSM_VCardVersion Version)
{
        int     Name, Number, Group, i;
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
        } else if (Version == Nokia_VCard21) {
                if (header) *Length+=sprintf(Buffer+(*Length),"BEGIN:VCARD%c%cVERSION:2.1%c%c",13,10,13,10);
                if (Name != -1) {
                        SaveVCALText(Buffer, Length, pbk->Entries[Name].Text, "N");
                }
                for (i=0; i < pbk->EntriesNum; i++) {
                        if (i != Name) {
                                ignore = false;
                                switch(pbk->Entries[i].EntryType) {
                                case PBK_Text_Name      :
                                //case PBK_Date           :
                                case PBK_Caller_Group   :
                                        ignore = true;
                                        break;
                                case PBK_Number_General :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
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
								case PBK_Date       :
										*Length+=sprintf(Buffer+(*Length),"BDAY");
										sprintf((char *)pbk->Entries[i].Text,"%s-%s-%s",pbk->Entries[i].Date.Year,pbk->Entries[i].Date.Month,pbk->Entries[i].Date.Day);
                                        break;
                                default :
                                        ignore = true;
                                        break;
                                }
                                if (!ignore) {
                                        SaveVCALText(Buffer, Length, pbk->Entries[i].Text, "");
                                }
                        }
                }
                if (header) *Length+=sprintf(Buffer+(*Length),"END:VCARD%c%c",13,10);
        }
}

void GSM_EncodeSonyEricssonVCARD(char *Buffer, int *Length, GSM_MemoryEntry *pbk, bool header, GSM_VCardVersion Version)
{
        int     Name, Number, Group, i,LastName ,FirstName;
        bool    ignore;
//		char szName[2000];
		unsigned char szFullName[2000];
//		char szLastName[2000];
//		char szFirstName[2000];
		unsigned char szoutput[2000];

        GSM_PhonebookFindDefaultNameNumberGroupSE(pbk, &Name, &Number, &Group,&LastName ,&FirstName);

        if (Version == SonyEricsson_VCard10) {
                if (header) *Length+=sprintf(Buffer+(*Length),"BEGIN:VCARD%c%c",13,10);
                if (Name != -1) {
                        *Length+=sprintf(Buffer+(*Length),"N:%s%c%c",DecodeUnicodeString(pbk->Entries[Name].Text),13,10);
                }
                if (Number != -1) {
                        *Length +=sprintf(Buffer+(*Length),"TEL:%s%c%c",DecodeUnicodeString(pbk->Entries[Number].Text),13,10);
                }
                if (header) *Length+=sprintf(Buffer+(*Length),"END:VCARD%c%c",13,10);
        } else if (Version == SonyEricsson_VCard21) {
                if (header) *Length+=sprintf(Buffer+(*Length),"BEGIN:VCARD%c%cVERSION:2.1%c%c",13,10,13,10);
                if (Name != -1)
				{/*
 						sprintf(szName,"%s",DecodeUnicodeConsole(pbk->Entries[Name].Text));
						CString strName = szName;
						strName.Replace(";","\\;");
						sprintf(szName,strName);

						unsigned char	   	Text[(GSM_PHONEBOOK_TEXT_LENGTH+1)*2];
						EncodeUnicode(Text,(unsigned char *)szName,strlen(szName)+1);
						SESaveVCARDText(Buffer, Length, Text, "N", ":");*/
					//for _UNICODE
						unsigned char	   	Text[(GSM_PHONEBOOK_TEXT_LENGTH+1)*2];
						CopyUnicodeString(Text,pbk->Entries[Name].Text);
						UnicodeReplace(Text,";","\\;");
						SESaveVCARDText(Buffer, Length, Text, "N", ":;");


					//	SESaveVCARDText(Buffer, Length, pbk->Entries[Name].Text, "N", ":");
                }
				else
				{/*
					szLastName[0]='\0';
					szFirstName[0]='\0';
					if(LastName != -1)
					{
						sprintf(szLastName,"%s",DecodeUnicodeString(pbk->Entries[LastName].Text));
						CString strLastName = szLastName;
						strLastName.Replace(";","\\;");
						sprintf(szLastName,strLastName);
					}
					if(FirstName != -1)
					{
						sprintf(szFirstName,"%s",DecodeUnicodeString(pbk->Entries[FirstName].Text));
					
						CString strFirstName = szFirstName;
						strFirstName.Replace(";","\\;");
						sprintf(szFirstName,strFirstName);

					}
					sprintf(szName,"%s;%s",szLastName,szFirstName);
					EncodeUnicode((unsigned char *)szFullName,(unsigned char *)szName,strlen(szName)+1);
					SESaveVCARDText(Buffer, Length, (unsigned char *)szFullName, "N", ":");

					sprintf(szName,"%s %s",szFirstName,szLastName);
					CString strFullname = szName;
					strFullname.Replace("\\;",";");
					sprintf(szName,strFullname);

					EncodeUnicode(pbk->Entries[pbk->EntriesNum].Text,(unsigned char *)szName,strlen(szName)+1);
					pbk->Entries[pbk->EntriesNum].EntryType = PBK_Text_Name;
					pbk->EntriesNum++;*/

					//for _UNICODE
					unsigned char uLastName[4000];
					unsigned char uFirstName[4000];
					unsigned char uTemp[10];
					unsigned char uTemp2[10];

					uLastName[0] =uLastName[1] =0;
					uFirstName[0] =uFirstName[1] =0;
					EncodeUnicode(uTemp,(unsigned char *)";",2);
					EncodeUnicode(uTemp2,(unsigned char *)" ",2);

					if(LastName != -1)
					{
						CopyUnicodeString(uLastName,pbk->Entries[LastName].Text);
						UnicodeReplace(uLastName,";","\\;");
					}
					if(FirstName != -1)
					{
						CopyUnicodeString(uFirstName,pbk->Entries[FirstName].Text);
						UnicodeReplace(uFirstName,";","\\;");
					}
					CopyUnicodeString(szFullName,uLastName);
					UnicodeCat(szFullName,uTemp);
					UnicodeCat(szFullName,uFirstName);

					SESaveVCARDText(Buffer, Length, (unsigned char *)szFullName, "N", ":");

					if(FirstName != -1)
					{
						CopyUnicodeString(pbk->Entries[pbk->EntriesNum].Text,pbk->Entries[FirstName].Text);
						UnicodeCat(pbk->Entries[pbk->EntriesNum].Text,uTemp2);
						if(LastName != -1)
							UnicodeCat(pbk->Entries[pbk->EntriesNum].Text,pbk->Entries[LastName].Text);
						pbk->Entries[pbk->EntriesNum].EntryType = PBK_Text_Name;
						pbk->EntriesNum++;
					}
					else if(LastName != -1)
					{
						CopyUnicodeString(pbk->Entries[pbk->EntriesNum].Text,pbk->Entries[LastName].Text);
						pbk->Entries[pbk->EntriesNum].EntryType = PBK_Text_Name;
						pbk->EntriesNum++;
					}



				}

                for (i=0; i < pbk->EntriesNum; i++) {
                        if (i != Name) {
                                ignore = false;
                                switch(pbk->Entries[i].EntryType) {
								case PBK_Text_Name      :
                                case PBK_Caller_Group   :
                                        ignore = true;
                                        break;
								case PBK_Date       :
									*Length+=sprintf(Buffer+(*Length),"BDAY:%04d%02d%02d%c%c",pbk->Entries[i].Date.Year,pbk->Entries[i].Date.Month,pbk->Entries[i].Date.Day,13,10);
									 ignore = true;
									break;
                                case PBK_Number_General :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        //if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        break;
                                case PBK_Number_Mobile  :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        //if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        *Length+=sprintf(Buffer+(*Length),";CELL");
                                        break;
                                case PBK_Number_Work    :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        //if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        *Length+=sprintf(Buffer+(*Length),";WORK;VOICE");
                                        break;
                                case PBK_Number_Fax     :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        //if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        *Length+=sprintf(Buffer+(*Length),";FAX");
                                        break;
                                case PBK_Number_Home    :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        //if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        *Length+=sprintf(Buffer+(*Length),";HOME;VOICE");
                                        break;
                                case PBK_Number_Other    :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        //if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        //*Length+=sprintf(Buffer+(*Length),";HOME;VOICE");
                                        break;
                               	case PBK_Text_Company:
                                         *Length+=sprintf(Buffer+(*Length),"ORG");
										GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
										SESaveVCARDText(Buffer, Length, szoutput, "",":");
										ignore = true;

                                        break;
                                case PBK_Text_JobTitle:
                                       *Length+=sprintf(Buffer+(*Length),"TITLE");
   										GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
										SESaveVCARDText(Buffer, Length, szoutput, "",":");
										ignore = true;

                                        break;
 								case PBK_Text_Note      :
                                        *Length+=sprintf(Buffer+(*Length),"NOTE");
										GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
										SESaveVCARDText(Buffer, Length, szoutput, "",":");
										ignore = true;

                                        break;
                                case PBK_Text_Postal    :
                                        /* Don't ask why. Nokia phones save postal address
                                         * double - once like LABEL, second like ADR
                                         */
                                        //*Length+=sprintf(Buffer+(*Length),"ADR");
										SESaveVCARDText(Buffer, Length, pbk->Entries[i].Text, "ADR", ";HOME:;");
                                        
										ignore = true;
                                        break;
                                case PBK_Text_Postal_Work    :
										SESaveVCARDText(Buffer, Length, pbk->Entries[i].Text, "ADR", ";WORK:;");
										ignore = true;
                                        break;
                                case PBK_Text_Email     :
                                case PBK_Text_Email2    :
                                        *Length+=sprintf(Buffer+(*Length),"EMAIL");
										GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
										SESaveVCARDText(Buffer, Length, szoutput, "",":");
										ignore = true;

                                        break;
                                case PBK_Text_URL       :
                                        *Length+=sprintf(Buffer+(*Length),"URL");
										GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
										SESaveVCARDText(Buffer, Length, szoutput, "",":");
										ignore = true;

                                        break;
                                default :
                                        ignore = true;
                                        break;
                                }
                                if (!ignore) {
                                        SESaveVCARDText(Buffer, Length, pbk->Entries[i].Text, "",":");
                                }
                        }
                }
                if (header) *Length+=sprintf(Buffer+(*Length),"END:VCARD%c%c",13,10);
        }
}
void GSM_EncodeSiemensVCARD(char *Buffer, int *Length, GSM_MemoryEntry *pbk, bool header, GSM_VCardVersion Version)
{
        int     Name, Number, Group, i,LastName ,FirstName;
        bool    ignore;
//		char szName[2000];
//		char szLastName[2000];
//		char szFirstName[2000];
		unsigned char szFullName[2000];
		unsigned char szoutput[2000];
		int j;
		i=j=0;

//        GSM_PhonebookFindDefaultNameNumberGroup(pbk, &Name, &Number, &Group);
		GSM_PhonebookFindDefaultNameNumberGroupSE(pbk, &Name, &Number, &Group,&LastName ,&FirstName);
        if (Version == Siemens_VCard10) {
                if (header) *Length+=sprintf(Buffer+(*Length),"BEGIN:VCARD%c%c",13,10);
                if (Name != -1) {
                        *Length+=sprintf(Buffer+(*Length),"N:%s%c%c",DecodeUnicodeString(pbk->Entries[Name].Text),13,10);
                }
                if (Number != -1) {
                        *Length +=sprintf(Buffer+(*Length),"TEL:%s%c%c",DecodeUnicodeString(pbk->Entries[Number].Text),13,10);
                }
                if (header) *Length+=sprintf(Buffer+(*Length),"END:VCARD%c%c",13,10);
        } else if (Version == Siemens_VCard21) {
                if (header) *Length+=sprintf(Buffer+(*Length),"BEGIN:VCARD%c%cVERSION:2.1%c%c",13,10,13,10);
                if (Name != -1)
				{/*
//peggy add for ';' +
						sprintf(szName,"%s",DecodeUnicodeConsole(pbk->Entries[Name].Text));
						CString strName = szName;
						strName.Replace(";","\\;");
						sprintf(szName,strName);

						unsigned char	   	Text[(GSM_PHONEBOOK_TEXT_LENGTH+1)*2];
						EncodeUnicode(Text,(unsigned char *)szName,strlen(szName)+1);
//peggy add for ';' -*/
						//for _UNICODE
						unsigned char	   	Text[(GSM_PHONEBOOK_TEXT_LENGTH+1)*2];
						CopyUnicodeString(Text,pbk->Entries[Name].Text);
						UnicodeReplace(Text,";","\\;");

                        SiemensSaveVCARDText(Buffer, Length,(char *)Text, "N","");


                }
				else
				{
				/*	szLastName[0]='\0';
					szFirstName[0]='\0';
					if(LastName != -1)
					{
						sprintf(szLastName,"%s",DecodeUnicodeString(pbk->Entries[LastName].Text));
						CString strLastName = szLastName;
						strLastName.Replace(";","\\;");
						sprintf(szLastName,strLastName);
					}
					if(FirstName != -1)
					{
						sprintf(szFirstName,"%s",DecodeUnicodeString(pbk->Entries[FirstName].Text));
					
						CString strFirstName = szFirstName;
						strFirstName.Replace(";","\\;");
						sprintf(szFirstName,strFirstName);

					}
					sprintf(szName,"%s;%s",szLastName,szFirstName);
					EncodeUnicode((unsigned char *)szFullName,(unsigned char *)szName,strlen(szName)+1);
					SiemensSaveVCARDText(Buffer, Length, szFullName, "N", "");

					sprintf(szName,"%s %s",szFirstName,szLastName);
					CString strFullname = szName;
					strFullname.Replace("\\;",";");
					sprintf(szName,strFullname);

					EncodeUnicode(pbk->Entries[pbk->EntriesNum].Text,(unsigned char *)szName,strlen(szName)+1);
					pbk->Entries[pbk->EntriesNum].EntryType = PBK_Text_Name;
					pbk->EntriesNum++;*/
					//for _UNICODE
					unsigned char uLastName[4000];
					unsigned char uFirstName[4000];
					unsigned char uTemp[10];
					unsigned char uTemp2[10];
					uLastName[0] =uLastName[1] =0;
					uFirstName[0] =uFirstName[1] =0;

					EncodeUnicode(uTemp,(unsigned char *)";",2);
					EncodeUnicode(uTemp2,(unsigned char *)" ",2);

					if(LastName != -1)
					{
						CopyUnicodeString(uLastName,pbk->Entries[LastName].Text);
						UnicodeReplace(uLastName,";","\\;");
					}
					if(FirstName != -1)
					{
						CopyUnicodeString(uFirstName,pbk->Entries[FirstName].Text);
						UnicodeReplace(uFirstName,";","\\;");
					}
					CopyUnicodeString(szFullName,uLastName);
					UnicodeCat(szFullName,uTemp);
					UnicodeCat(szFullName,uFirstName);

					SiemensSaveVCARDText(Buffer, Length, (char *)szFullName, "N", "");

					if(FirstName != -1)
					{
						CopyUnicodeString(pbk->Entries[pbk->EntriesNum].Text,pbk->Entries[FirstName].Text);
						UnicodeCat(pbk->Entries[pbk->EntriesNum].Text,uTemp2);
						if(LastName != -1)
							UnicodeCat(pbk->Entries[pbk->EntriesNum].Text,pbk->Entries[LastName].Text);
						pbk->Entries[pbk->EntriesNum].EntryType = PBK_Text_Name;
						pbk->EntriesNum++;
					}
					else if(LastName != -1)
					{
						CopyUnicodeString(pbk->Entries[pbk->EntriesNum].Text,pbk->Entries[LastName].Text);
						pbk->Entries[pbk->EntriesNum].EntryType = PBK_Text_Name;
						pbk->EntriesNum++;
					}


				}

                for (i=0; i < pbk->EntriesNum; i++) {
                        if (i != Name) {
                                ignore = false;
                                switch(pbk->Entries[i].EntryType) {
								case PBK_Text_Name      :
                              //  case PBK_Date           :
                                case PBK_Caller_Group   :
                                        ignore = true;
                                        break;
                                case PBK_Caller_Group_Text   :
                                        *Length+=sprintf(Buffer+(*Length),"X-ESI-CATEGORIES");
                                        break;
                                case PBK_Number_General :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        //if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        break;
                                case PBK_Number_Mobile  :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        //if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        *Length+=sprintf(Buffer+(*Length),";CELL");
                                        break;
                                case PBK_Number_Mobile_Home  :
                                        *Length+=sprintf(Buffer+(*Length),"TEL;CELL;HOME");
                                        break;
                                case PBK_Number_Mobile_Work  :
                                        *Length+=sprintf(Buffer+(*Length),"TEL;CELL;WORK");
                                        break;
                                case PBK_Number_Work    :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        //if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        *Length+=sprintf(Buffer+(*Length),";WORK");
                                        break;
                                case PBK_Number_Fax     :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        //if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        *Length+=sprintf(Buffer+(*Length),";FAX");
                                        break;
                                case PBK_Number_Fax_Home     :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        //if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        *Length+=sprintf(Buffer+(*Length),";FAX;HOME");
                                        break;
                                case PBK_Number_Fax_Work  :
                                        *Length+=sprintf(Buffer+(*Length),"TEL;FAX;WORK");
                                        break;
                                case PBK_Number_Home    :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        //if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        *Length+=sprintf(Buffer+(*Length),";HOME");
                                        break;
                                case PBK_Number_Other    :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        //if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        //*Length+=sprintf(Buffer+(*Length),";HOME;VOICE");
                                        break;
                               	case PBK_Text_Company:
 										GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
										*Length+=sprintf(Buffer+(*Length),"ORG");
										SiemensSaveVCARDText(Buffer, Length,(char*) szoutput, "","");
										ignore = true;
                                       break;
                                case PBK_Text_JobTitle:
   										GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
										*Length+=sprintf(Buffer+(*Length),"TITLE");
										SiemensSaveVCARDText(Buffer, Length, (char*)szoutput, "","");
										ignore = true;

                                        break;
 								case PBK_Text_Note      :
                                 //       *Length+=sprintf(Buffer+(*Length),"NOTE");
   										GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
										*Length+=sprintf(Buffer+(*Length),"NOTE");
										SiemensSaveVCARDText(Buffer, Length,(char*) szoutput, "","");
										ignore = true;
                                        break;
                                case PBK_Text_Postal    :
                                        /* Don't ask why. Nokia phones save postal address
                                         * double - once like LABEL, second like ADR
                                         */
                                         *Length+=sprintf(Buffer+(*Length),"ADR");
										 SiemensSaveVCARDText(Buffer, Length, (char *)pbk->Entries[i].Text, "",";");
                                         *Length+=sprintf(Buffer+(*Length),"ADR;HOME");
										 SiemensSaveVCARDText(Buffer, Length, (char *)pbk->Entries[i].Text, "",";");
										 ignore = true;
                                        break;
                                case PBK_Text_Email     :
                                case PBK_Text_Email2    :
                                      //  *Length+=sprintf(Buffer+(*Length),"EMAIL;INTERNET");
    									GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
										*Length+=sprintf(Buffer+(*Length),"EMAIL;INTERNET");
										SiemensSaveVCARDText(Buffer, Length, (char*)szoutput, "","");
										ignore = true;
                                       break;
                                case PBK_Text_Email_Home    :
                                        *Length+=sprintf(Buffer+(*Length),"EMAIL;HOME;INTERNET");
    									GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
										SiemensSaveVCARDText(Buffer, Length, (char*)szoutput, "","");
										ignore = true;
                                        break;
                                case PBK_Text_Email_Work    :
                                        *Length+=sprintf(Buffer+(*Length),"EMAIL;WORK;PREF;INTERNET");
    									GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
										SiemensSaveVCARDText(Buffer, Length, (char*)szoutput, "","");
										ignore = true;
                                        break;
                                case PBK_Text_URL       :
                                        *Length+=sprintf(Buffer+(*Length),"URL");
      									GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
										SiemensSaveVCARDText(Buffer, Length,(char*) szoutput, "","");
										ignore = true;
                                      break;
                                case PBK_Text_URL_Home       :
                                        *Length+=sprintf(Buffer+(*Length),"URL;HOME");
      									GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
										SiemensSaveVCARDText(Buffer, Length,(char*) szoutput, "","");
										ignore = true;
                                      break;
                                case PBK_Date       :
                                        *Length+=sprintf(Buffer+(*Length),"BDAY:%04d-%02d-%02d%c%c",pbk->Entries[i].Date.Year,pbk->Entries[i].Date.Month,pbk->Entries[i].Date.Day,13,10);
										 ignore = true;
										break;
                                default :
                                        ignore = true;
                                        break;
                                }
                                if (!ignore) {
                                        SiemensSaveVCARDText(Buffer, Length, (char *)pbk->Entries[i].Text, "","");
                                }
                        }
                }
                if (header) *Length+=sprintf(Buffer+(*Length),"END:VCARD%c%c",13,10);
        }
}
void GSM_EncodeSiemensVCARD_SL75(char *Buffer, int *Length, GSM_MemoryEntry *pbk, bool header, GSM_VCardVersion Version)
{
        int     Name, Number, Group, i,LastName ,FirstName;
        bool    ignore;
//		char szName[2000];
//		char szLastName[2000];
//		char szFirstName[2000];
		unsigned char szFullName[2000];
		unsigned char szoutput[2000];
		int j;
		i=j=0;

//        GSM_PhonebookFindDefaultNameNumberGroup(pbk, &Name, &Number, &Group);
		GSM_PhonebookFindDefaultNameNumberGroupSE(pbk, &Name, &Number, &Group,&LastName ,&FirstName);
        if (Version == Siemens_VCard10) {
                if (header) *Length+=sprintf(Buffer+(*Length),"BEGIN:VCARD%c%c",13,10);
                if (Name != -1) {
                        *Length+=sprintf(Buffer+(*Length),"N:%s%c%c",DecodeUnicodeString(pbk->Entries[Name].Text),13,10);
                }
                if (Number != -1) {
                        *Length +=sprintf(Buffer+(*Length),"TEL:%s%c%c",DecodeUnicodeString(pbk->Entries[Number].Text),13,10);
                }
                if (header) *Length+=sprintf(Buffer+(*Length),"END:VCARD%c%c",13,10);
        } else if (Version == Siemens_VCard21) {
                if (header) *Length+=sprintf(Buffer+(*Length),"BEGIN:VCARD%c%cVERSION:2.1%c%c",13,10,13,10);
                if (Name != -1)
				{/*
//peggy add for ';' +
						sprintf(szName,"%s",DecodeUnicodeConsole(pbk->Entries[Name].Text));
						CString strName = szName;
						strName.Replace(";","\\;");
						sprintf(szName,strName);

						unsigned char	   	Text[(GSM_PHONEBOOK_TEXT_LENGTH+1)*2];
						EncodeUnicode(Text,(unsigned char *)szName,strlen(szName)+1);
//peggy add for ';' -*/
						//for _UNICODE
						unsigned char	   	Text[(GSM_PHONEBOOK_TEXT_LENGTH+1)*2];
						CopyUnicodeString(Text,pbk->Entries[Name].Text);
						UnicodeReplace(Text,";","\\;");

                        SiemensSaveVCARDText(Buffer, Length,(char *)Text, "N","");


                }
				else
				{
			/*		szLastName[0]='\0';
					szFirstName[0]='\0';
					if(LastName != -1)
					{
						sprintf(szLastName,"%s",DecodeUnicodeString(pbk->Entries[LastName].Text));
						CString strLastName = szLastName;
						strLastName.Replace(";","\\;");
						sprintf(szLastName,strLastName);
					}
					if(FirstName != -1)
					{
						sprintf(szFirstName,"%s",DecodeUnicodeString(pbk->Entries[FirstName].Text));
					
						CString strFirstName = szFirstName;
						strFirstName.Replace(";","\\;");
						sprintf(szFirstName,strFirstName);

					}
					sprintf(szName,"%s;%s",szLastName,szFirstName);
					EncodeUnicode((unsigned char *)szFullName,(unsigned char *)szName,strlen(szName)+1);
					SiemensSaveVCARDText(Buffer, Length, szFullName, "N", "");

					sprintf(szName,"%s %s",szFirstName,szLastName);
					CString strFullname = szName;
					strFullname.Replace("\\;",";");
					sprintf(szName,strFullname);

					EncodeUnicode(pbk->Entries[pbk->EntriesNum].Text,(unsigned char *)szName,strlen(szName)+1);
					pbk->Entries[pbk->EntriesNum].EntryType = PBK_Text_Name;
					pbk->EntriesNum++;*/
					//for _UNICODE
					unsigned char uLastName[4000];
					unsigned char uFirstName[4000];
					unsigned char uTemp[10];
					unsigned char uTemp2[10];
					
					uLastName[0] =uLastName[1] =0;
					uFirstName[0] =uFirstName[1] =0;

					EncodeUnicode(uTemp,(unsigned char *)";",2);
					EncodeUnicode(uTemp2,(unsigned char *)" ",2);

					if(LastName != -1)
					{
						CopyUnicodeString(uLastName,pbk->Entries[LastName].Text);
						UnicodeReplace(uLastName,";","\\;");
					}
					if(FirstName != -1)
					{
						CopyUnicodeString(uFirstName,pbk->Entries[FirstName].Text);
						UnicodeReplace(uFirstName,";","\\;");
					}
					CopyUnicodeString(szFullName,uLastName);
					UnicodeCat(szFullName,uTemp);
					UnicodeCat(szFullName,uFirstName);

					SiemensSaveVCARDText(Buffer, Length,(char *) szFullName, "N", "");

					if(FirstName != -1)
					{
						CopyUnicodeString(pbk->Entries[pbk->EntriesNum].Text,pbk->Entries[FirstName].Text);
						UnicodeCat(pbk->Entries[pbk->EntriesNum].Text,uTemp2);
						if(LastName != -1)
							UnicodeCat(pbk->Entries[pbk->EntriesNum].Text,pbk->Entries[LastName].Text);
						pbk->Entries[pbk->EntriesNum].EntryType = PBK_Text_Name;
						pbk->EntriesNum++;
					}
					else if(LastName != -1)
					{
						CopyUnicodeString(pbk->Entries[pbk->EntriesNum].Text,pbk->Entries[LastName].Text);
						pbk->Entries[pbk->EntriesNum].EntryType = PBK_Text_Name;
						pbk->EntriesNum++;
					}

				}

                for (i=0; i < pbk->EntriesNum; i++) {
                        if (i != Name) {
                                ignore = false;
                                switch(pbk->Entries[i].EntryType) {
								case PBK_Text_Name      :
                              //  case PBK_Date           :
                                case PBK_Caller_Group   :
                                        ignore = true;
                                        break;
                                case PBK_Caller_Group_Text   :
                                        *Length+=sprintf(Buffer+(*Length),"X-ESI-CATEGORIES");
                                        break;
                                case PBK_Number_General :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        //if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        break;
                                case PBK_Number_Mobile  :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        //if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        *Length+=sprintf(Buffer+(*Length),";CELL");
                                        break;
                                case PBK_Number_Mobile_Home  :
                                        *Length+=sprintf(Buffer+(*Length),"TEL;CELL;HOME");
                                        break;
                                case PBK_Number_Mobile_Work  :
                                        *Length+=sprintf(Buffer+(*Length),"TEL;CELL;WORK");
                                        break;
                                case PBK_Number_Work    :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        //if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        *Length+=sprintf(Buffer+(*Length),";WORK");
                                        break;
                                case PBK_Number_Fax     :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        //if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        *Length+=sprintf(Buffer+(*Length),";FAX");
                                        break;
                                case PBK_Number_Fax_Home     :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        //if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        *Length+=sprintf(Buffer+(*Length),";FAX;HOME");
                                        break;
                                case PBK_Number_Fax_Work  :
                                        *Length+=sprintf(Buffer+(*Length),"TEL;FAX;WORK");
                                        break;
                                case PBK_Number_Home    :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        //if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        *Length+=sprintf(Buffer+(*Length),";HOME");
                                        break;
                                case PBK_Number_Other    :
                                        *Length+=sprintf(Buffer+(*Length),"TEL");
                                        //if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                        //*Length+=sprintf(Buffer+(*Length),";HOME;VOICE");
                                        break;
                               	case PBK_Text_Company:
 										GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
										*Length+=sprintf(Buffer+(*Length),"ORG");
										SiemensSaveVCARDText(Buffer, Length,(char*) szoutput, "","");
										ignore = true;
                                       break;
                               	case PBK_Text_Nickname:
 										GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
										*Length+=sprintf(Buffer+(*Length),"FN");
										SiemensSaveVCARDText(Buffer, Length,(char*) szoutput, "","");
										ignore = true;
                                       break;
                                case PBK_Text_JobTitle:
   										GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
										*Length+=sprintf(Buffer+(*Length),"TITLE");
										SiemensSaveVCARDText(Buffer, Length, (char*)szoutput, "","");
										ignore = true;

                                        break;
 								case PBK_Text_Note      :
                                 //       *Length+=sprintf(Buffer+(*Length),"NOTE");
   										GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
										*Length+=sprintf(Buffer+(*Length),"NOTE");
										SiemensSaveVCARDText(Buffer, Length,(char*) szoutput, "","");
										ignore = true;
                                        break;
                                case PBK_Text_Postal    :
                                        /* Don't ask why. Nokia phones save postal address
                                         * double - once like LABEL, second like ADR
                                         */
                                         *Length+=sprintf(Buffer+(*Length),"ADR");
										 SiemensSaveVCARDText(Buffer, Length, (char *)pbk->Entries[i].Text, "",";");
                                         *Length+=sprintf(Buffer+(*Length),"ADR;HOME");
										 SiemensSaveVCARDText(Buffer, Length, (char *)pbk->Entries[i].Text, "",";");
										 ignore = true;
                                        break;
                                case PBK_Text_Email     :
                                case PBK_Text_Email2    :
                                      //  *Length+=sprintf(Buffer+(*Length),"EMAIL;INTERNET");
    									GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
										*Length+=sprintf(Buffer+(*Length),"EMAIL;INTERNET");
										SiemensSaveVCARDText(Buffer, Length, (char*)szoutput, "","");
										ignore = true;
                                       break;
                                case PBK_Text_Email_Home    :
                                        *Length+=sprintf(Buffer+(*Length),"EMAIL;HOME;PREF;INTERNET");
    									GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
										SiemensSaveVCARDText(Buffer, Length, (char*)szoutput, "","");
										ignore = true;
                                        break;
                                case PBK_Text_Email_Work    :
                                        *Length+=sprintf(Buffer+(*Length),"EMAIL;WORK;PREF;INTERNET");
    									GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
										SiemensSaveVCARDText(Buffer, Length, (char*)szoutput, "","");
										ignore = true;
                                        break;
                                case PBK_Text_URL       :
                                        *Length+=sprintf(Buffer+(*Length),"URL;WORK");
      									GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
										SiemensSaveVCARDText(Buffer, Length,(char*) szoutput, "","");
										ignore = true;
                                      break;
                                case PBK_Text_URL_Home       :
                                        *Length+=sprintf(Buffer+(*Length),"URL;HOME");
      									GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
										SiemensSaveVCARDText(Buffer, Length,(char*) szoutput, "","");
										ignore = true;
                                      break;
                                case PBK_Date       :
                                        *Length+=sprintf(Buffer+(*Length),"BDAY:%04d-%02d-%02d%c%c",pbk->Entries[i].Date.Year,pbk->Entries[i].Date.Month,pbk->Entries[i].Date.Day,13,10);
										 ignore = true;
										break;
                                default :
                                        ignore = true;
                                        break;
                                }
                                if (!ignore) {
                                        SiemensSaveVCARDText(Buffer, Length, (char *)pbk->Entries[i].Text, "","");
                                }
                        }
                }
                if (header) *Length+=sprintf(Buffer+(*Length),"END:VCARD%c%c",13,10);
        }
}
GSM_Error GSM_DecodePBInfo(unsigned char *Buffer,  int *Pos, int *irmc_no,GSM_MemoryStatus *Status, GSM_VCardVersion Version)
{
        unsigned char   Line[200];
		unsigned char   count[10];
        int             Level = 1;
		//int Pos = 0;
		char   *found;
        if (Buffer == NULL) return ERR_EMPTY;
        while (1) {
                MyGetLine(Buffer, Pos, Line, strlen((char *)Buffer));
                if (strlen((char *)Line) == 0) break;

                if ( strstr((char *)Line,"Total-Records:")) {
					if(found =strchr((char *)Line, ':') )
					{
						memset( count, 0, 10);
						memcpy( count, found+1, strlen(found+1));
						Status->MemoryUsed = atoi((char *)count);//Only for SE T610
						break;
					}
                 }
                if (strstr((char *)Line,"Free-Records:")) {
					if(found =strchr((char *)Line, ':'))
					{
						memset( count, 0, 10);
						memcpy( count, found+1, strlen(found+1));
						Status->MemoryFree = atoi((char *)count);
						break;
					}
                }
                if (strstr((char *)Line,"X-IRMC-LUID:")) {
					if(found =strchr((char *)Line, '='))
					{
						memset( count, 0, 10);
						memcpy( count, found+1, strlen(found+1));
						*irmc_no = atoi((char *)count);
						break;
					}
                }
       }
        return ERR_NONE;
}
GSM_Error GSM_DecodeSharpPBInfo(unsigned char *Buffer, int *Pos, GSM_MemoryStatus *Status)
{
	unsigned char   Line[200];
	unsigned char   count[10];
	int             Level = 1;
	char   *found;

	if (Buffer == NULL) return ERR_EMPTY;

	while (1) {
		MyGetLine(Buffer, Pos, Line, strlen((char *)Buffer));
		if (strlen((char *)Line) == 0) break;
		
		if ( strstr((char *)Line,"Total-Records:")) {
			if(found =strchr((char *)Line, ':') )
			{
				memset( count, 0, 10);
				memcpy( count, found+1, strlen(found+1));
				Status->MemoryUsed = atoi((char *)count);
				break;
			}
		}
		if (strstr((char *)Line,"Maximum-Records:")) {
			if(found =strchr((char *)Line, ':'))
			{
				memset( count, 0, 10);
				memcpy( count, found+1, strlen(found+1));
				Status->MemoryFree = atoi((char *)count) - Status->MemoryUsed;
				break;
			}
		}
	}
	return ERR_NONE;
}
void SplitString(char *pString ,int  Split,char* pStr1,char* pStr2)
{
	char *pFind;
	char szTemp[MAX_PATH];
	pFind = strchr(pString,Split );
	if(pFind)
	{
		sprintf(pStr2	,"%s",pFind+1);
		sprintf(szTemp,"%s",pString);
		szTemp[strlen(szTemp)-strlen(pFind)] = '\0';
		sprintf(pStr1	,"%s",szTemp);
	}
	else
	{
		sprintf(pStr1,"%s",pString);
		pStr2[0]='\0';
	}
}
void SplitStringEx(unsigned char *pString ,char*  Split,unsigned char* pStr1,unsigned char* pStr2)
{
	unsigned char uTemp[10];
	unsigned char uTempSlash[10];
	EncodeUnicode(uTemp,(unsigned char *)Split,2);
	EncodeUnicode(uTempSlash,(unsigned char *)"\\",2);
	wchar_t* pwSplit = (unsigned short *)uTemp;
	wchar_t* pwSlash = (unsigned short *)uTempSlash;

	wchar_t* pwstr = (unsigned short *)pString;

	int nLen = wcslen(pwstr);
	for(int i = 0; i<nLen ;i++)
	{
		if(*(pwstr+i) ==*pwSplit)
		{
			if(i==0)
			{
				pStr1[0]='\0';
				pStr1[1]='\0';
				CopyUnicodeString(pStr2,pString+2);
				break;
			}
			else if(*(pwstr+i-1) != *pwSlash)
			{
				CopyUnicodeString(pStr1,pString);
				CopyUnicodeString(pStr2,pString+(i+1)*2);

				pStr1[(nLen-UnicodeLength(pStr2)-1)*2]='\0';
				pStr1[(nLen-UnicodeLength(pStr2)-1)*2+1]='\0';
				break;
			}
		}
	}
	if(i==nLen)
	{
		pStr2[0]='\0';
		pStr2[1]='\0';
		CopyUnicodeString(pStr1,pString);
	}
}
GSM_Error NokiaS6_DecodeVCARD(unsigned char *Buffer,int nBufferSize, int *Pos, GSM_MemoryEntry *Pbk)
{
        unsigned char   Line[2000],Buff[2000];
        int             Level = 0;//,num;
        unsigned char   Line2[2000]; //peggy
		int npos = *Pos;//peggy

//		char  str[30];
		//peggy add 02242005 +
//		char 	*pName;
		unsigned char uTemp[2000];
		unsigned char uTemp1[2000];
		unsigned char uLastName[2000];
		unsigned char uFirstName[2000];
		unsigned char uMiddleName[2000];
		unsigned char uTitle[2000];
		unsigned char uSuffix[2000];
		unsigned char uName[2000];
		unsigned char uTempSpace[10];
		EncodeUnicode(uTempSpace,(unsigned char*)" ",2);
//		char szName1[2000];
		int i,j ;
		i=j=0;

		uLastName[0]='\0';
		uFirstName[0]='\0';
		uMiddleName[0]='\0';
		uTitle[0]='\0';
		uSuffix[0]='\0';
		uLastName[1]='\0';
		uFirstName[1]='\0';
		uMiddleName[1]='\0';
		uTitle[1]='\0';
		uSuffix[1]='\0';
		uName[0] =uName[1] = '\0';

		//peggy add 02242005 -
        Buff[0]         = 0;
        Pbk->EntriesNum = 0;
        if (Buffer == NULL) return ERR_EMPTY;

        while (1) {
                MyGetLine(Buffer, Pos, Line, nBufferSize);
                if (strlen((char *)Line) == 0)
				{
					(*Pos)++;
					if(*Pos >=nBufferSize)
						break;

				}
				//Modify for SE K750i
				//=E6=88=91=E9=96=80=E6=98=A=..F=E4=B8=80=E5=AE=B6=E4=BA=BA..
				npos = *Pos;
				memcpy(Line2,Line,strlen((char*)Line)+1);
				while(Line2[strlen((char*)Line2)-1] == '=')
				{
					if(Buffer[npos+2] == 0x0d && Buffer[npos+3] == 0x0A)
					{
						if(Line[strlen((char*)Line)-1] == '=')
							Line[strlen((char*)Line)-1]= '\0';
						break;
					}
					MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
					if(Line2[0] == '=')
					{
						strcat((char *)Line,(char *)Line2+1);
					}
					else
					{
						Line[strlen((char*)Line)-1]= '\0';
						strcat((char *)Line,(char *)Line2);
					}
				}
				MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
				while(Line2[0] == 0x20 && Line2[1] == 0x20)
				{
					strcat((char *)Line,(char *)Line2);
					MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
				}

/* 				npos = *Pos;//peggy
                MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
    				//peggy +
				while(Line2[0] == '=')
				{
					strcat((char *)Line,(char *)Line2+1);

					if(Line2[strlen((char *)Line2) -1] == '=')
						MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
					else 
						break;
				}
				//peggy +*/
              switch (Level) {
                case 0:
                        if (strstr((char *)Line,"BEGIN:VCARD")) Level = 1;
                        break;
                case 1:
                        if (strstr((char *)Line,"END:VCARD")) {
                                if (Pbk->EntriesNum == 0) return ERR_EMPTY;
                                return ERR_NONE;
                        }
                      if (ReadVCALText(Line, "N", Buff))
					  {
							char 	*pTemp = strchr((char*)Line,':' );
							if(pTemp[1] == ';')
							{

								SplitStringEx(Buff,";",uFirstName,uTemp);
								SplitStringEx(uTemp,";",uMiddleName,uTemp1);
								SplitStringEx(uTemp1,";",uTitle,uSuffix);
								
								if(UnicodeLength(uFirstName)>0)
								{
									UnicodeReplace(uFirstName,"\\;",";");
									CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uFirstName);
									Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Name;
									Pbk->EntriesNum++;

									CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uFirstName);
									Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_FirstName;
									Pbk->EntriesNum++;
								}
							}
							else
							{
								SplitStringEx(Buff,";",uLastName,uTemp);
								SplitStringEx(uTemp,";",uFirstName,uTemp1);
								SplitStringEx(uTemp1,";",uMiddleName,uTemp);
								SplitStringEx(uTemp,";",uTitle,uSuffix);

								if(UnicodeLength(uLastName) >0)
								{
									UnicodeReplace(uLastName,"\\;",";");

									if(UnicodeLength(uFirstName) > 0)
									{
										UnicodeReplace(uFirstName,"\\;",";");
										CopyUnicodeString(uName,uFirstName);
										UnicodeCat(uName,uTempSpace);
										UnicodeCat(uName,uLastName);
									}
									else
										CopyUnicodeString(uName,uLastName);
								}
								else if(UnicodeLength(uFirstName)>0)
								{
									UnicodeReplace(uFirstName,"\\;",";");
									CopyUnicodeString(uName,uFirstName);
								}
								if(UnicodeLength(uName)>0)
								{
									CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uName);
									Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Name;
									Pbk->EntriesNum++;
								}
								if(UnicodeLength(uFirstName)>0)
								{
									CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uFirstName);
									Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_FirstName;
									Pbk->EntriesNum++;
								}
								if(UnicodeLength(uLastName) >0)
								{
									CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uLastName);
									Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_LastName;
									Pbk->EntriesNum++;
								}
							}
							if(UnicodeLength(uMiddleName) > 0)
							{
								UnicodeReplace(uMiddleName,"\\;",";");
								CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uMiddleName);
								Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_MiddleName;
								Pbk->EntriesNum++;
							}
							if(UnicodeLength(uTitle) > 0)
							{
								UnicodeReplace(uTitle,"\\;",";");
								CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uTitle);
								Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Title;
								Pbk->EntriesNum++;
							}
							if(UnicodeLength(uSuffix) > 0)
							{
								UnicodeReplace(uSuffix,"\\;",";");
								CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uSuffix);
								Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Suffix;
								Pbk->EntriesNum++;
							}



                        }
                       if (ReadVCALText(Line, "X-EPOCSECONDNAME",  Buff) ) 
					   {
						    GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                          //  CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Nickname;
                            Pbk->EntriesNum++;
                        }
					   if (ReadVCALText(Line, "TEL",                   Buff) ||
                            ReadVCALText(Line, "TEL;VOICE",             Buff) ||
                            ReadVCALText(Line, "TEL;PREF",              Buff) ||
                            ReadVCALText(Line, "TEL;PREF;VOICE",        Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
								Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_General;

								Pbk->EntriesNum++;
                        }
                       if (ReadVCALText(Line, "TEL;CELL;WORK",  Buff) ||
						   ReadVCALText(Line, "TEL;WORK;CELL",  Buff) ||
						   ReadVCALText(Line, "TEL;WORK;VOICE;CELL",  Buff) ||
						   ReadVCALText(Line, "TEL;VOICE;WORK;CELL",  Buff)) 
					   {
                            CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Mobile_Work;
                            Pbk->EntriesNum++;
                        }      
                       if (ReadVCALText(Line, "TEL;CELL;HOME",  Buff)||
						   ReadVCALText(Line, "TEL;HOME;CELL",  Buff)||
						   ReadVCALText(Line, "TEL;HOME;VOICE;CELL",  Buff) ||
						   ReadVCALText(Line, "TEL;VOICE;HOME;CELL",  Buff)) 
					   {
                            CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Mobile_Home;
                            Pbk->EntriesNum++;
                        }    
					   
                       if (ReadVCALText(Line, "TEL;HOME;PAGER",  Buff) ||
						   ReadVCALText(Line, "TEL;PAGER;HOME",  Buff)) 
					   {
                            CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Pager_Home;
                            Pbk->EntriesNum++;
                        }      
                       if (ReadVCALText(Line, "TEL;PAGER;WORK",  Buff) ||
						   ReadVCALText(Line, "TEL;WORK;PAGER",  Buff) ) 
					   {
                            CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Pager_Work;
                            Pbk->EntriesNum++;
                        }      
                       if (ReadVCALText(Line, "PAGER",  Buff) ||
						   ReadVCALText(Line, "TEL;PAGER",  Buff)) 
					   {
                            CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Pager;
                            Pbk->EntriesNum++;
                        }      

                       if (ReadVCALText(Line, "TEL;VIDEO;HOME",  Buff)||
						   ReadVCALText(Line, "TEL;HOME;VIDEO",  Buff)) 
					   {
                            CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_VideoCall_Home;
                            Pbk->EntriesNum++;
                        }      
                       if (ReadVCALText(Line, "TEL;VIDEO;WORK",  Buff)||
						   ReadVCALText(Line, "TEL;WORK;VIDEO",  Buff)) 
					   {
                            CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_VideoCall_Work;
                            Pbk->EntriesNum++;
                        }      
                       if (ReadVCALText(Line, "TEL;VIDEO",  Buff) ) 
					   {
                            CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_VideoCall;
                            Pbk->EntriesNum++;
                        }      

                        if (ReadVCALText(Line, "TEL;CELL",              Buff) ||
                            ReadVCALText(Line, "TEL;CELL;VOICE",        Buff) ||
							ReadVCALText(Line, "TEL;VOICE;CELL",        Buff) ||
							 ReadVCALText(Line, "TEL;PREF;CELL",         Buff) ||
                            ReadVCALText(Line, "TEL;PREF;CELL;VOICE",   Buff)) 
						{
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Mobile;
                                Pbk->EntriesNum++;
                        }

						  if (ReadVCALText(Line, "TEL;VOICE;WORK",  Buff) ||
							  ReadVCALText(Line, "TEL;WORK",              Buff) ||
                            ReadVCALText(Line, "TEL;PREF;WORK",         Buff) ||
                            ReadVCALText(Line, "TEL;WORK;VOICE",        Buff) ||
                            ReadVCALText(Line, "TEL;PREF;WORK;VOICE",   Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Work;
                                Pbk->EntriesNum++;
                        }

                        if (ReadVCALText(Line, "TEL;FAX;WORK",               Buff)||
							ReadVCALText(Line, "TEL;WORK;FAX",               Buff))
						{
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Fax_Work;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "TEL;FAX;HOME",               Buff)||
							ReadVCALText(Line, "TEL;HOME;FAX",               Buff))
						{
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Fax_Home;
                                Pbk->EntriesNum++;
                        }

                        if (ReadVCALText(Line, "TEL;FAX",               Buff) ||
                            ReadVCALText(Line, "TEL;PREF;FAX",          Buff) ||
                            ReadVCALText(Line, "TEL;FAX;VOICE",         Buff) ||
							ReadVCALText(Line, "TEL;PREF;FAX;VOICE",    Buff) ) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Fax;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "TEL;VOICE;HOME",  Buff)||
							ReadVCALText(Line, "TEL;HOME",              Buff) ||
                            ReadVCALText(Line, "TEL;PREF;HOME",         Buff) ||
                            ReadVCALText(Line, "TEL;HOME;VOICE",        Buff) ||
                            ReadVCALText(Line, "TEL;PREF;HOME;VOICE",   Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Home;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "NOTE", Buff)) {
								GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                              //  CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Note;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText_ADR(Line, "ADR;HOME", Buff) ) 
						{
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Postal_Home;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText_ADR(Line, "ADR;WORK", Buff) ) 
						{
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Postal_Work;
                                Pbk->EntriesNum++;
                        }

                        if (ReadVCALText_ADR(Line, "ADR", Buff)||
							ReadVCALText_ADR(Line, "ADR;PREF", Buff)) 
						{
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Postal;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "ORG", Buff)) {
							/*
 								sprintf(szTemp,"%s",DecodeUnicodeConsole(Buff));
								if(szTemp[strlen(szTemp)-1] ==';')
									szTemp[strlen(szTemp)-1] = '\0';

								EncodeUnicode(Buff,(unsigned char *)szTemp,strlen(szTemp)+1);*/
								//for _UNICODE
								char szTemp[2000];
 								sprintf(szTemp,"%s",DecodeUnicodeConsole(Buff));
								if(szTemp[strlen(szTemp)-1] ==';')
								{
									int nLen = UnicodeLength(Buff);
									Buff[(nLen-1)*2] = 0;
									Buff[(nLen-1)*2+1] = 0;

								}

								GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);

                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Company;
                                Pbk->EntriesNum++;

                        }
                         if (ReadVCALText(Line, "TITLE", Buff)) {
								 GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                              //  CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_JobTitle;
                                Pbk->EntriesNum++;
                        }
                      if (ReadVCALText(Line, "EMAIL;INTERNET;HOME", Buff)||
						  ReadVCALText(Line, "EMAIL;HOME;INTERNET", Buff)) 
					  {
                             //   CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
								 GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);

                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Email_Home;
                                Pbk->EntriesNum++;
                       }
                      if (ReadVCALText(Line, "EMAIL;INTERNET;WORK", Buff)||
						  ReadVCALText(Line, "EMAIL;WORK;INTERNET", Buff)) 
					  {
                              //  CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
								  GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);

                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Email_Work;
                                Pbk->EntriesNum++;
                        }

                      if (ReadVCALText(Line, "EMAIL", Buff)||
                            ReadVCALText(Line, "EMAIL;INTERNET;PREF",  Buff) ||
							ReadVCALText(Line, "EMAIL;INTERNET",  Buff)) {
							  GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);

                                //CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Email;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "URL;WORK",         Buff)) 
						{
                         //       CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
								GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);

                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_URL_Work;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "URL;HOME",         Buff)) 
						{
                              //  CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
								GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);

                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_URL_Home;
                                Pbk->EntriesNum++;
                        }

                        if (ReadVCALText(Line, "URL", Buff))
						{
                              //  CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
								GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);

                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_URL;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "X-SIP;VOIP", Buff))
						{
								GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Custom1;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "X-SIP;VOIP;HOME", Buff))
						{
								GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Custom2;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "X-SIP;VOIP;WORK", Buff))
						{
								GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Custom3;
                                Pbk->EntriesNum++;
                        }


						if (ReadVCALText(Line, "BDAY", Buff))
						{
							char 	*pTemp = strchr((char*)Line,':' );
							if(pTemp)
							{
							YYYYMMDDGetDateTime(&Pbk->Entries[Pbk->EntriesNum].Date,(char*) pTemp+1);
								
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Date;
                            Pbk->EntriesNum++;
							}
                        }
						if (ReadVCALText(Line, "X-ESI-CATEGORIES", Buff))
						{
							CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Caller_Group_Text;
                            Pbk->EntriesNum++;
                        }
                        break;
                }
        }

        if (Pbk->EntriesNum == 0) return ERR_EMPTY;
        return ERR_NONE;
}
GSM_Error GSM_SiemensDecodeVCARD(unsigned char *Buffer, int *Pos, GSM_MemoryEntry *Pbk, GSM_VCardVersion Version)
{
        unsigned char   Line[2000],Buff[2000];
        int             Level = 0,num;
        unsigned char   Line2[2000]; //peggy
		int npos = *Pos;//peggy

		char  str[30];
		//peggy add 02242005 +
		unsigned char uLastName[2000];
		unsigned char uFirstName[2000];
		unsigned char uName[2000];
//		char szName1[2000];
		unsigned char uTempSpace[10];
		EncodeUnicode(uTempSpace,(unsigned char*)" ",2);
		int i,j ;
		i=j=0;

		uLastName[0]='\0';
		uFirstName[0]='\0';
		uLastName[1]='\0';
		uFirstName[1]='\0';
		uName[0] =uName[1] = '\0';
		//peggy add 02242005 -
        Buff[0]         = 0;
        Pbk->EntriesNum = 0;
        if (Buffer == NULL) return ERR_EMPTY;

        while (1) {
                MyGetLine(Buffer, Pos, Line, strlen((char *)Buffer));
                if (strlen((char *)Line) == 0) break;
 	
				//Modify for SE K750i
				//=E6=88=91=E9=96=80=E6=98=A=..F=E4=B8=80=E5=AE=B6=E4=BA=BA..
				npos = *Pos;
				memcpy(Line2,Line,strlen((char*)Line)+1);
				while(Line2[strlen((char*)Line2)-1] == '=')
				{
					if(Buffer[npos+2] == 0x0d && Buffer[npos+3] == 0x0A)
					{
						if(Line[strlen((char*)Line)-1] == '=')
							Line[strlen((char*)Line)-1]= '\0';
						break;
					}
					MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
					if(Line2[0] == '=')
					{
						strcat((char *)Line,(char *)Line2+1);
					}
					else
					{
						Line[strlen((char*)Line)-1]= '\0';
						strcat((char *)Line,(char *)Line2);
					}
				}
				MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
				while(Line2[0] == 0x20 && Line2[1] == 0x20)
				{
					strcat((char *)Line,(char *)Line2);
					MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
				}
	/*			npos = *Pos;//peggy
                MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
  				//peggy +
				while(Line2[0] == '=')
				{
					strcat((char *)Line,(char *)Line2+1);

					if(Line2[strlen((char *)Line2) -1] == '=')
						MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
					else 
						break;
				}*/
				//peggy +
                switch (Level) {
                case 0:
                        if (strstr((char *)Line,"BEGIN:VCARD")) Level = 1;
                        break;
                case 1:
                        if (strstr((char *)Line,"END:VCARD")) {
                                if (Pbk->EntriesNum == 0) return ERR_EMPTY;
                                return ERR_NONE;
                        }
                      if (ReadVCALText(Line, "N", Buff))
					  {
							 if(Version == Siemens_VCard21)
							{
//////////
								char 	*pTemp = strchr((char*)Line,':' );
								if(pTemp[1] == ';')
								{
									CopyUnicodeString(uName,Buff);
									UnicodeReplace(uName,"\\;",";");
								
									CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uName);
									Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Name;
									Pbk->EntriesNum++;

									CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uName);
									Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_FirstName;
									Pbk->EntriesNum++;

								}
								else
								{
									SplitStringEx(Buff,";",uLastName,uFirstName);

									if(UnicodeLength(uLastName)>0) UnicodeReplace(uLastName,"\\;",";");
									if(UnicodeLength(uFirstName) >0) UnicodeReplace(uFirstName,"\\;",";");

//									unsigned char uFullName[MAX_PATH];
									if(UnicodeLength(uFirstName) >0)
									{
										if(UnicodeLength(uLastName) > 0)
										{
											CopyUnicodeString(uName,uFirstName);
											UnicodeCat(uName,uTempSpace);
											UnicodeCat(uName,uLastName);
										}
										else
											CopyUnicodeString(uName,uFirstName);
									}
									else if(UnicodeLength(uLastName)>0)
									{
										CopyUnicodeString(uName,uLastName);
									}
									if(UnicodeLength(uName)>0)
									{
										CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uName);
										Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Name;
										Pbk->EntriesNum++;
									}
									if(UnicodeLength(uFirstName)>0)
									{
										CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uFirstName);
										Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_FirstName;
										Pbk->EntriesNum++;
									}
									if(UnicodeLength(uLastName) >0)
									{
										CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uLastName);
										Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_LastName;
										Pbk->EntriesNum++;
									}
								}

/////////
							}
							else
							{
 								CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
								Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Name;
								Pbk->EntriesNum++;
							}

                        }
                      if (ReadVCALText(Line, "FN", Buff))
					  {
							GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);

                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Nickname;
                            Pbk->EntriesNum++;
					  }

                       if (ReadVCALText(Line, "TEL",                   Buff) ||
                            ReadVCALText(Line, "TEL;VOICE",             Buff) ||
                            ReadVCALText(Line, "TEL;PREF",              Buff) ||
                            ReadVCALText(Line, "TEL;PREF;VOICE",        Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
								if( Version == Nokia_VCard21 || Version ==Siemens_VCard21)
									Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_General;
                                else
									Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Other;

								Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "TEL;CELL",              Buff) ||
                            ReadVCALText(Line, "TEL;CELL;VOICE",        Buff) ||
                            ReadVCALText(Line, "TEL;PREF;CELL",         Buff) ||
                            ReadVCALText(Line, "TEL;PREF;CELL;VOICE",   Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Mobile;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "TEL;CELL;HOME",   Buff) )
						{
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Mobile_Home;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "TEL;CELL;WORK",   Buff) )//SL75
						{
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Mobile_Work;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "TEL;WORK",              Buff) ||
                            ReadVCALText(Line, "TEL;PREF;WORK",         Buff) ||
                            ReadVCALText(Line, "TEL;WORK;VOICE",        Buff) ||
                            ReadVCALText(Line, "TEL;PREF;WORK;VOICE",   Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Work;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "TEL;FAX",               Buff) ||
                            ReadVCALText(Line, "TEL;PREF;FAX",          Buff) ||
                            ReadVCALText(Line, "TEL;FAX;VOICE",         Buff) ||
					//		ReadVCALText(Line, "TEL;FAX;HOME",         Buff) || //S55
                            ReadVCALText(Line, "TEL;PREF;FAX;VOICE",    Buff) ) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Fax;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "TEL;FAX;HOME",         Buff) )//S55
						{
                            CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Fax_Home;
                            Pbk->EntriesNum++;
						}
                        if (ReadVCALText(Line, "TEL;FAX;WORK",         Buff) )//SL75
						{
                            CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Fax_Work;
                            Pbk->EntriesNum++;
						}
                        if (ReadVCALText(Line, "TEL;HOME",              Buff) ||
                            ReadVCALText(Line, "TEL;PREF;HOME",         Buff) ||
                            ReadVCALText(Line, "TEL;HOME;VOICE",        Buff) ||
                            ReadVCALText(Line, "TEL;PREF;HOME;VOICE",   Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Home;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "NOTE", Buff)) {
                               // CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
								GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);

                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Note;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "ADR", Buff)||
                            ReadVCALText(Line, "ADR;HOME", Buff) ) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Postal;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "ORG", Buff)) {
                             //   CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
								GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);

                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Company;
                                Pbk->EntriesNum++;
                        }
                         if (ReadVCALText(Line, "TITLE", Buff)) {
                           //     CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
								 GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);

                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_JobTitle;
                                Pbk->EntriesNum++;
                        }
                      if (ReadVCALText(Line, "EMAIL", Buff)||
				//		  ReadVCALText(Line, "EMAIL;HOME;INTERNET", Buff)|| //S55
                            ReadVCALText(Line, "EMAIL;INTERNET;PREF",  Buff) ||
							ReadVCALText(Line, "EMAIL;INTERNET",  Buff)) {
                           //     CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
								 GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);

                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Email;
                                Pbk->EntriesNum++;
                        }
                      if (ReadVCALText(Line, "EMAIL;HOME;INTERNET", Buff) ||//S55
						  ReadVCALText(Line, "EMAIL;HOME;PREF;INTERNET", Buff))//SL75
					  {
                          //  CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
							  GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);

                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Email_Home;
                            Pbk->EntriesNum++;
					  }
                      if (ReadVCALText(Line, "EMAIL;WORK;INTERNET", Buff) ||//S55
						  ReadVCALText(Line, "EMAIL;WORK;PREF;INTERNET", Buff))//SL75
					  {
                          //  CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
							  GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);

                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Email_Work;
                            Pbk->EntriesNum++;
					  }
                        if (ReadVCALText(Line, "URL", Buff)||
                            ReadVCALText(Line, "URL;WORK",         Buff)) {
                         //       CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
								GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);

                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_URL;
                                Pbk->EntriesNum++;
                        }
                        if (   ReadVCALText(Line, "URL;HOME",         Buff)) //SL75
						{
                         //       CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
								GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);

                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_URL_Home;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "X-IRMC-LUID", Buff)) 
						{
							if (Version == Nokia_VCard21 || Version ==Siemens_VCard21)
							{
								num=0;
								num=Searchstr(Line,(unsigned char *)"X-IRMC-LUID",11);
								if(num)
								{
									memcpy(str,&Line[num+11],30);
									wsprintf(Pbk->szIndex,str);
								//	Pbk->Location=atoi(str);
								}
							}
							else
							{
							//	Pbk->Location = strtoul((char*)DecodeUnicodeString(Buff), NULL, 16);//atof(DecodeUnicodeString(Buff));
								wsprintf(Pbk->szIndex,(char*)DecodeUnicodeString(Buff));
							}
							
                        }
						if (ReadVCALText(Line, "BDAY", Buff))
						{
							YYYYMMDD_Get_DateTime(&Pbk->Entries[Pbk->EntriesNum].Date,(char*) Line);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Date;
                            Pbk->EntriesNum++;
                        }
						if (ReadVCALText(Line, "X-ESI-CATEGORIES", Buff))
						{
							CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Caller_Group_Text;
                            Pbk->EntriesNum++;
                        }
                        break;
                }
        }

        if (Pbk->EntriesNum == 0) return ERR_EMPTY;
        return ERR_NONE;
}
GSM_Error GSM_DecodeVCARD(unsigned char *Buffer, int *Pos, GSM_MemoryEntry *Pbk, GSM_VCardVersion Version)
{
        unsigned char   Line[2000],Buff[2000];
        int             Level = 0,num;
        unsigned char   Line2[2000]; //peggy
		int npos = *Pos;//peggy

		char  str[30];
		//peggy add 02242005 +
		unsigned char uLastName[2000];
		unsigned char uFirstName[2000];
		unsigned char uName[2000];
//		char szName1[2000];
		unsigned char uTempSpace[10];
		EncodeUnicode(uTempSpace,(unsigned char*)" ",2);
		int i,j ;
		i=j=0;

		uLastName[0]='\0';
		uFirstName[0]='\0';
		uLastName[1]='\0';
		uFirstName[1]='\0';
		uName[0] =uName[1] = '\0';
		//peggy add 02242005 -
        Buff[0]         = 0;
        Pbk->EntriesNum = 0;
        if (Buffer == NULL) return ERR_EMPTY;

        while (1) {
                MyGetLine(Buffer, Pos, Line, strlen((char *)Buffer));
                if (strlen((char *)Line) == 0) break;
 	
				//Modify for SE K750i
				//=E6=88=91=E9=96=80=E6=98=A=..F=E4=B8=80=E5=AE=B6=E4=BA=BA..
				npos = *Pos;
				memcpy(Line2,Line,strlen((char*)Line)+1);
				while(Line2[strlen((char*)Line2)-1] == '=')
				{
					if(Buffer[npos+2] == 0x0d && Buffer[npos+3] == 0x0A)
					{
						if(Line[strlen((char*)Line)-1] == '=')
							Line[strlen((char*)Line)-1]= '\0';
						break;
					}
					MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
					if(Line2[0] == '=')
					{
						strcat((char *)Line,(char *)Line2+1);
					}
					else
					{
						Line[strlen((char*)Line)-1]= '\0';
						strcat((char *)Line,(char *)Line2);
					}
				}
				MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
				while(Line2[0] == 0x20 && Line2[1] == 0x20)
				{
					strcat((char *)Line,(char *)Line2);
					MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
				}
	/*			npos = *Pos;//peggy
                MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
  				//peggy +
				while(Line2[0] == '=')
				{
					strcat((char *)Line,(char *)Line2+1);

					if(Line2[strlen((char *)Line2) -1] == '=')
						MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
					else 
						break;
				}*/
				//peggy +
                switch (Level) {
                case 0:
                        if (strstr((char *)Line,"BEGIN:VCARD")) Level = 1;
                        break;
                case 1:
                        if (strstr((char *)Line,"END:VCARD")) {
                                if (Pbk->EntriesNum == 0) return ERR_EMPTY;
                                return ERR_NONE;
                        }
                      if (ReadVCALText(Line, "N", Buff))
					  {
                            //peggy 02242005 modify +
						/*	if (Version == SonyEricsson_VCard21)
							{
								if(Line[2] == ';')
								{
									CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
									Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Name;
									Pbk->EntriesNum++;

									CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
									Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_FirstName;
									Pbk->EntriesNum++;

								}
								else
								{

									sprintf(szName,"%s",DecodeUnicodeConsole(Buff));

									pName = strrchr(szName,';' );
									if(pName)
									{
										sprintf(szFirstName	,"%s",pName+1);
										sprintf(szTemp,"%s",szName);
										szTemp[strlen(szTemp)-strlen(pName)] = '\0';
										sprintf(szLastName	,"%s",szTemp);
									
										sprintf(szName,"%s %s",szFirstName ,szLastName);

										EncodeUnicode(Pbk->Entries[Pbk->EntriesNum].Text,(unsigned char *)szName,strlen(szName)+1);
										Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Name;
										Pbk->EntriesNum++;

										EncodeUnicode(Pbk->Entries[Pbk->EntriesNum].Text,(unsigned char *)szFirstName,strlen(szFirstName)+1);
										Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_FirstName;
										Pbk->EntriesNum++;

										EncodeUnicode(Pbk->Entries[Pbk->EntriesNum].Text,(unsigned char *)szLastName,strlen(szLastName)+1);
										Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_LastName;
										Pbk->EntriesNum++;
									}
									else
									{
										CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
										Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Name;
										Pbk->EntriesNum++;
									}
								}
							}
							//peggy add for ';' 
							else if(Version == Siemens_VCard21)*/
						  if(Version == SonyEricsson_VCard21 ||Version == Siemens_VCard21 )
							{
//////////
								char 	*pTemp = strchr((char*)Line,':' );
								if(pTemp[1] == ';')
								{
									CopyUnicodeString(uName,Buff);
									UnicodeReplace(uName,"\\;",";");
				
									CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uName);
									Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Name;
									Pbk->EntriesNum++;

									CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uName);
									Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_FirstName;
									Pbk->EntriesNum++;

								}
								else
								{
									SplitStringEx(Buff,";",uLastName,uFirstName);
									if(UnicodeLength(uLastName)>0) UnicodeReplace(uLastName,"\\;",";");
									if(UnicodeLength(uFirstName) >0) UnicodeReplace(uFirstName,"\\;",";");

								//	char szFullName[MAX_PATH];
									if(UnicodeLength(uFirstName) >0)
									{
										if(UnicodeLength(uLastName) > 0)
										{
											CopyUnicodeString(uName,uFirstName);
											UnicodeCat(uName,uTempSpace);
											UnicodeCat(uName,uLastName);
										}
										else
											CopyUnicodeString(uName,uFirstName);
									}
									else if(UnicodeLength(uLastName)>0)
										CopyUnicodeString(uName,uLastName);
									if(UnicodeLength(uName)>0)
									{
										CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uName);
										Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Name;
										Pbk->EntriesNum++;
									}
									if(UnicodeLength(uFirstName)>0)
									{
										CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uFirstName);
										Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_FirstName;
										Pbk->EntriesNum++;
									}
									if(UnicodeLength(uLastName) >0)
									{
										CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uLastName);
										Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_LastName;
										Pbk->EntriesNum++;
									}
								}


/////////
							}
							else
							{
 								CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
								Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Name;
								Pbk->EntriesNum++;
							}
 						//	CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
							//	Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Name;
							//	Pbk->EntriesNum++;
                           //peggy 02242005 modify -


                        }
                       if (ReadVCALText(Line, "TEL",                   Buff) ||
                            ReadVCALText(Line, "TEL;VOICE",             Buff) ||
                            ReadVCALText(Line, "TEL;PREF",              Buff) ||
                            ReadVCALText(Line, "TEL;PREF;VOICE",        Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
								if( Version == Nokia_VCard21 || Version ==Siemens_VCard21)
									Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_General;
                                else
									Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Other;

								Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "TEL;CELL",              Buff) ||
                            ReadVCALText(Line, "TEL;CELL;VOICE",        Buff) ||
                            ReadVCALText(Line, "TEL;PREF;CELL",         Buff) ||
                            ReadVCALText(Line, "TEL;PREF;CELL;VOICE",   Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Mobile;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "TEL;WORK",              Buff) ||
                            ReadVCALText(Line, "TEL;PREF;WORK",         Buff) ||
                            ReadVCALText(Line, "TEL;WORK;VOICE",        Buff) ||
                            ReadVCALText(Line, "TEL;PREF;WORK;VOICE",   Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Work;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "TEL;FAX",               Buff) ||
                            ReadVCALText(Line, "TEL;PREF;FAX",          Buff) ||
                            ReadVCALText(Line, "TEL;FAX;VOICE",         Buff) ||
							ReadVCALText(Line, "TEL;FAX;HOME",         Buff) || //S55
                            ReadVCALText(Line, "TEL;PREF;FAX;VOICE",    Buff) ) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Fax;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "TEL;HOME",              Buff) ||
                            ReadVCALText(Line, "TEL;PREF;HOME",         Buff) ||
                            ReadVCALText(Line, "TEL;HOME;VOICE",        Buff) ||
                            ReadVCALText(Line, "TEL;PREF;HOME;VOICE",   Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Home;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "NOTE", Buff)) {
								GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                               // CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Note;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "ADR", Buff)||
                            ReadVCALText(Line, "ADR;HOME", Buff) ) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Postal;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "ADR;WORK", Buff) ) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Postal_Work;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "ORG", Buff)) {
								GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                             //   CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Company;
                                Pbk->EntriesNum++;
                        }
                         if (ReadVCALText(Line, "TITLE", Buff)) {
								 GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                          //      CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_JobTitle;
                                Pbk->EntriesNum++;
                        }
                      if (ReadVCALText(Line, "EMAIL", Buff)||
						  ReadVCALText(Line, "EMAIL;HOME;INTERNET", Buff)|| //S55
                            ReadVCALText(Line, "EMAIL;INTERNET;PREF",  Buff) ||
							ReadVCALText(Line, "EMAIL;INTERNET",  Buff)) {
								  GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
								 if( UnicodeLength(Buff) > 0)
								 {
									CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
									Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Email;
									Pbk->EntriesNum++;
								 }
                        }
                        if (ReadVCALText(Line, "URL", Buff)||
                            ReadVCALText(Line, "URL;WORK",         Buff)) {
								GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_URL;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "X-IRMC-LUID", Buff)) 
						{
							if (Version == Nokia_VCard21 || Version ==Siemens_VCard21)
							{
								num=0;
								num=Searchstr(Line,(unsigned char *)"X-IRMC-LUID",11);
								if(num)
								{
									memcpy(str,&Line[num+11],30);
									wsprintf(Pbk->szIndex,str);
								//	Pbk->Location=atoi(str);
								}
							}
							else
							{
							//	Pbk->Location = strtoul((char*)DecodeUnicodeString(Buff), NULL, 16);//atof(DecodeUnicodeString(Buff));
								wsprintf(Pbk->szIndex,(char*)DecodeUnicodeString(Buff));
							}
							
                        }
						if (ReadVCALText(Line, "BDAY", Buff))
						{
							YYYYMMDD_Get_DateTime(&Pbk->Entries[Pbk->EntriesNum].Date,(char*) Line);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Date;
                            Pbk->EntriesNum++;
                        }
						if (ReadVCALText(Line, "X-ESI-CATEGORIES", Buff))
						{
							CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Caller_Group_Text;
                            Pbk->EntriesNum++;
                        }
                        break;
                }
        }

        if (Pbk->EntriesNum == 0) return ERR_EMPTY;
        return ERR_NONE;
}

/* -------------- OLD functions (c) by Timo Teras -------------------------- */

#ifndef ENABLE_LGPL

static void ParseVCardLine(char **pos, char *Name, char *Parameters, char *Value)
{
        int i;

        Name[0] = Parameters[0] = Value[0] = 0;

        if (**pos == 0) return;

        for (i=0; **pos && **pos != ':' && **pos != ';'; i++, (*pos)++) Name[i] = **pos;
        Name[i] = 0;

        //dbgprintf("ParseVCardLine: name tag = '%s'\n", Name);
        if (**pos == ';') {
                (*pos)++;
                for (i=0; **pos && **pos != ':'; i++, (*pos)++) Parameters[i] = **pos;
                Parameters[i] = ';';
                Parameters[i+1] = 0;
                //dbgprintf("ParseVCardLine: parameter tag = '%s'\n", Parameters);
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

        //dbgprintf("ParseVCardLine: value tag = '%s'\n", Value);
}

void DecodeVCARD21Text(char *VCard, GSM_MemoryEntry *pbk)
{
        char *pos = VCard;
        char Name[32], Parameters[256], Value[1024];

        dbgprintf("Parsing VCard:\n%s\n", VCard);

        ParseVCardLine(&pos, Name, Parameters, Value);
        if (!mystrncasecmp((unsigned char *)Name, (unsigned char *)"BEGIN", 0) || !mystrncasecmp((unsigned char *)Value,(unsigned char *) "VCARD", 0)) {
                dbgprintf("No valid VCARD signature\n");
                return;
        }

        while (1) {
                GSM_SubMemoryEntry *pbe = &pbk->Entries[pbk->EntriesNum];

                ParseVCardLine(&pos, Name, Parameters, Value);
                if (Name[0] == 0x00 ||
                    (mystrncasecmp((unsigned char *)Name, (unsigned char *)"END", 0) && mystrncasecmp((unsigned char *)Value, (unsigned char *)"VCARD", 0)))
                        return;

                if (mystrncasecmp((unsigned char *)Name, (unsigned char *)"N", 0)) {
                        //FIXME: Name is tagged field which should be parsed
                        pbe->EntryType = PBK_Text_Name;
                        EncodeUnicode(pbe->Text, (unsigned char *)Value, strlen(Value));
                        pbk->EntriesNum++;
                } else if (mystrncasecmp((unsigned char *)Name, (unsigned char *)"EMAIL", 0)) {
                        pbe->EntryType = PBK_Text_Email;
                        EncodeUnicode(pbe->Text, (unsigned char *)Value, strlen(Value));
                        pbk->EntriesNum++;
                } else if (mystrncasecmp((unsigned char *)Name, (unsigned char *)"TEL", 0)) {
                        if (strstr(Parameters, "WORK;"))
                                pbe->EntryType = PBK_Number_Work;
                        else if (strstr(Name, "HOME;"))
                                pbe->EntryType = PBK_Number_Home;
                        else if (strstr(Name, "FAX;"))
                                pbe->EntryType = PBK_Number_Fax;
                        else    pbe->EntryType = PBK_Number_General;

                        EncodeUnicode(pbe->Text, (unsigned char *)Value, strlen(Value));
                        pbk->EntriesNum++;
                }
        }
}
void GSM_PhonebookFindDefaultNameNumberGroupNS6(GSM_MemoryEntry *entry, int *Name, int *Number, int *Group,int *LastName , 
												int *FirstName,int *MiddleName,int *Title,int *Suffix)
{
        int i;

        *Name   = -1;
        *Number = -1;
        *Group  = -1;
		*LastName = -1;
		*FirstName = -1;
		*MiddleName = -1;
		*Title = -1;
		*Suffix = -1;

        for (i = 0; i < entry->EntriesNum; i++)
		{
                switch (entry->Entries[i].EntryType)
				{
                case PBK_Number_General : 
					if (*Number   == -1) *Number  = i; 
					break;
                case PBK_Text_Name      :
					if (*Name     == -1) *Name    = i;
					break;
                case PBK_Caller_Group   : 
					if (*Group    == -1) *Group   = i;
					break;
                case PBK_Text_LastName   : 
					if (*LastName    == -1) *LastName   = i;
					break;
                case PBK_Text_FirstName   : 
					if (*FirstName    == -1) *FirstName   = i;
					break;
                case PBK_Text_MiddleName   : 
					if (*MiddleName    == -1) *MiddleName   = i;
					break;
                case PBK_Text_Title   : 
					if (*Title    == -1) *Title   = i;
					break;
                case PBK_Text_Suffix   : 
					if (*Suffix    == -1) *Suffix   = i;
					break;
             default                 :           
					break;
                }

        }
        if ((*Number) == -1)
		{
                for (i = 0; i < entry->EntriesNum; i++) 
				{
                        switch (entry->Entries[i].EntryType)
						{
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

}

GSM_Error Samsung_DecodeVCARD(unsigned char *Buffer,int nBufferSize, int *Pos, GSM_MemoryEntry *Pbk)
{
	unsigned char   Line[2000],Buff[2000];
	int             Level = 0;//,num;
	unsigned char   Line2[2000]; //peggy
	int npos = *Pos;//peggy
	
	//		char  str[30];
	//peggy add 02242005 +
//	char 	*pName;
	unsigned char uTemp[2000];
	unsigned char uTemp1[2000];
	unsigned char uLastName[500];
	unsigned char uFirstName[500];
	char szMiddleName[500];
//	char szCompany[500];
//	char szDepartment[500];
	char szTitle[2000];
	char szSuffix[2000];
	unsigned char uName[2000];
	char szVCardText[12]="BEGIN:VCARD";
	unsigned char uTempSpace[10];
	EncodeUnicode(uTempSpace,(unsigned char*)" ",2);
//	char szName1[2000];
	int i,j ;
	i=j=0;
	
	uLastName[0]='\0';
	uFirstName[0]='\0';
	szMiddleName[0]='\0';
	szTitle[0]='\0';
	szSuffix[0]='\0';
	uLastName[1]='\0';
	uFirstName[1]='\0';
	uName[0] =uName[1] = '\0';	
	
	//peggy add 02242005 -
	Buff[0]         = 0;
	Pbk->EntriesNum = 0;
	if (Buffer == NULL) return ERR_EMPTY;
	while (1) {
		MyGetLine(Buffer, Pos, Line, nBufferSize);
		if (strlen((char *)Line) == 0)
		{
			(*Pos)++;
			if(*Pos >=nBufferSize)
				break;
			
		}
		//Modify for SE K750i
		//=E6=88=91=E9=96=80=E6=98=A=..F=E4=B8=80=E5=AE=B6=E4=BA=BA..
		npos = *Pos;
		memcpy(Line2,Line,strlen((char*)Line)+1);
		while(Line2[strlen((char*)Line2)-1] == '=')
		{
			if(Buffer[npos+2] == 0x0d && Buffer[npos+3] == 0x0A)
			{
				if(Line[strlen((char*)Line)-1] == '=')
					Line[strlen((char*)Line)-1]= '\0';
				break;
			}
			MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
			if(Line2[0] == '=')
			{
				strcat((char *)Line,(char *)Line2+1);
			}
			else
			{
				Line[strlen((char*)Line)-1]= '\0';
				strcat((char *)Line,(char *)Line2);
			}
		}
		MyGetLine(Buffer, &npos, Line2, 180);//peggy
		while(Line2[0] == 0x20 && Line2[1] == 0x20)
		{
			strcat((char *)Line,(char *)Line2);
			MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
		}
		switch (Level) {
		case 0:
		/*	if (strstr((char *)Line,"<![CDATA["))
			{
				CString lineRemoval = Line;
				lineRemoval.Replace("<![CDATA[","");
				sprintf((char *)Line, lineRemoval);
			}*/
			if (strstr((char *)Line,"BEGIN:VCARD")) Level = 1;
			if ((strlen((char *)Line)) <11) //VCard or :VCard
			{
				int nLength = strlen((char *)Line);
				for (int i = 0;nLength>=0;)
				{
					if(Line[i]==szVCardText[11-nLength])
					{
						i++;
						nLength--;
						Level = 1;
					}
					else
					{
						Level = 0;
						break;
					}
				}
			}

			break;
		case 1:
			if (strstr((char *)Line,"END:VCARD")) {
				if (Pbk->EntriesNum == 0) return ERR_EMPTY;
				return ERR_NONE;
			}
			if (ReadVCALText(Line, "N", Buff))
			{
				char 	*pTemp = strchr((char*)Line,':' );
				if(pTemp[1] == ';')
				{
				//	CopyUnicodeString(uName,Buff);
				//	UnicodeReplace(uName,"\\;",";");

					SplitStringEx(Buff,";",uFirstName,uTemp);
					if(UnicodeLength(uFirstName)>0)
					{
						UnicodeReplace(uFirstName,"\\;",";");
						CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uFirstName);
						Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Name;
						Pbk->EntriesNum++;
						
						CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uFirstName);
						Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_FirstName;
						Pbk->EntriesNum++;
					}
				}
				else
				{
					SplitStringEx(Buff,";",uLastName,uTemp);
					SplitStringEx(uTemp,";",uFirstName,uTemp1);
					if(UnicodeLength(uLastName)>0) UnicodeReplace(uLastName,"\\;",";");
					if(UnicodeLength(uFirstName) >0) UnicodeReplace(uFirstName,"\\;",";");

					if(UnicodeLength(uLastName) >0)
					{
						if(UnicodeLength(uFirstName) > 0)
						{
							CopyUnicodeString(uName,uFirstName);
							UnicodeCat(uName,uTempSpace);
							UnicodeCat(uName,uLastName);
						}
						else
							CopyUnicodeString(uName,uLastName);
					}
					else if(UnicodeLength(uFirstName)>0)
					{
						CopyUnicodeString(uName,uFirstName);
					}
				/*	if(strlen(szName)>0)
					{
						EncodeUnicode(Pbk->Entries[Pbk->EntriesNum].Text,(unsigned char *)szName,strlen(szName)+1);
						Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Name;
						Pbk->EntriesNum++;
					}*/
					if(UnicodeLength(uFirstName)>0)
					{
						CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uFirstName);
						Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_FirstName;
						Pbk->EntriesNum++;
					}
					if(UnicodeLength(uLastName) >0)
					{
						CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uLastName);
						Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_LastName;
						Pbk->EntriesNum++;
					}
				}
			}
			if (ReadVCALText(Line, "X-EPOCSECONDNAME",  Buff) ) 
			{
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Nickname;
				Pbk->EntriesNum++;
			}
			
			if (ReadVCALText(Line, "TEL",                   Buff) ||
				ReadVCALText(Line, "TEL;VOICE",             Buff) ||
				ReadVCALText(Line, "TEL;PREF",              Buff) ||
				ReadVCALText(Line, "TEL;TYPE=CELL; VOICE",  Buff) ||
				ReadVCALText(Line, "TEL;PREF;VOICE",        Buff)) {
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Mobile;
				Pbk->EntriesNum++;
			}
			if (ReadVCALText(Line, "TEL;CELL;WORK",  Buff) ||
				ReadVCALText(Line, "TEL;WORK;CELL",  Buff) ||
				ReadVCALText(Line, "TEL;WORK;VOICE;CELL",  Buff) ||
				ReadVCALText(Line, "TEL;VOICE;WORK;CELL",  Buff)) 
			{
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Mobile_Work;
				Pbk->EntriesNum++;
			}      
			if (ReadVCALText(Line, "TEL;CELL;HOME",  Buff)||
				ReadVCALText(Line, "TEL;HOME;CELL",  Buff)||
				ReadVCALText(Line, "TEL;HOME;VOICE;CELL",  Buff) ||
				ReadVCALText(Line, "TEL;VOICE;HOME;CELL",  Buff)) 
			{
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Mobile_Home;
				Pbk->EntriesNum++;
			}    
			
			if (ReadVCALText(Line, "TEL;HOME;PAGER",  Buff) ||
				ReadVCALText(Line, "TEL;PAGER;HOME",  Buff)) 
			{
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Pager_Home;
				Pbk->EntriesNum++;
			}      
			if (ReadVCALText(Line, "TEL;PAGER;WORK",  Buff) ||
				ReadVCALText(Line, "TEL;WORK;PAGER",  Buff) ) 
			{
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Pager_Work;
				Pbk->EntriesNum++;
			}      
			if (ReadVCALText(Line, "PAGER",  Buff) ||
				ReadVCALText(Line, "TEL;PAGER",  Buff)) 
			{
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Pager;
				Pbk->EntriesNum++;
			}      
			
			if (ReadVCALText(Line, "TEL;VIDEO;HOME",  Buff)||
				ReadVCALText(Line, "TEL;HOME;VIDEO",  Buff)) 
			{
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_VideoCall_Home;
				Pbk->EntriesNum++;
			}      
			if (ReadVCALText(Line, "TEL;VIDEO;WORK",  Buff)||
				ReadVCALText(Line, "TEL;WORK;VIDEO",  Buff)) 
			{
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_VideoCall_Work;
				Pbk->EntriesNum++;
			}      
			if (ReadVCALText(Line, "TEL;VIDEO",  Buff) ) 
			{
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_VideoCall;
				Pbk->EntriesNum++;
			}      
			
			if (ReadVCALText(Line, "TEL;CELL",              Buff) ||
				ReadVCALText(Line, "TEL;CELL;VOICE",        Buff) ||
				ReadVCALText(Line, "TEL;VOICE;CELL",        Buff) ||
				ReadVCALText(Line, "TEL;PREF;CELL",         Buff) ||
				ReadVCALText(Line, "TEL;PREF;CELL;VOICE",   Buff)) 
			{
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Mobile;
				Pbk->EntriesNum++;
			}
			
			if (ReadVCALText(Line, "TEL;VOICE;WORK",  Buff) ||
				ReadVCALText(Line, "TEL;WORK",              Buff) ||
				ReadVCALText(Line, "TEL;PREF;WORK",         Buff) ||
				ReadVCALText(Line, "TEL;WORK;VOICE",        Buff) ||
				ReadVCALText(Line, "TEL;TYPE=WORK; VOICE",  Buff) ||
				ReadVCALText(Line, "TEL;PREF;WORK;VOICE",   Buff)) {
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Work;
				Pbk->EntriesNum++;
			}
			
			if (ReadVCALText(Line, "TEL;FAX;WORK",               Buff)||
				ReadVCALText(Line, "TEL;WORK;FAX",               Buff))
			{
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Fax_Work;
				Pbk->EntriesNum++;
			}
			if (ReadVCALText(Line, "TEL;FAX;HOME",               Buff)||
				ReadVCALText(Line, "TEL;HOME;FAX",               Buff))
			{
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Fax_Home;
				Pbk->EntriesNum++;
			}
			
			if (ReadVCALText(Line, "TEL;FAX",               Buff) ||
				ReadVCALText(Line, "TEL;PREF;FAX",          Buff) ||
				ReadVCALText(Line, "TEL;FAX;VOICE",         Buff) ||
				ReadVCALText(Line, "TEL;TYPE=FAX",         Buff) ||
				ReadVCALText(Line, "TEL;PREF;FAX;VOICE",    Buff) ) {
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Fax;
				Pbk->EntriesNum++;
			}
			if (ReadVCALText(Line, "TEL;VOICE;HOME",  Buff)||
				ReadVCALText(Line, "TEL;HOME",              Buff) ||
				ReadVCALText(Line, "TEL;PREF;HOME",         Buff) ||
				ReadVCALText(Line, "TEL;HOME;VOICE",        Buff) ||
				ReadVCALText(Line, "TEL;TYPE=HOME; VOICE",  Buff) ||
				ReadVCALText(Line, "TEL;PREF;HOME;VOICE",   Buff)) {
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Home;
				Pbk->EntriesNum++;
			}
			if (ReadVCALText(Line, "TEL;PAGER",				Buff)||
				ReadVCALText(Line, "TEL;TYPE=VOICE",		Buff))
			{
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Other;
				Pbk->EntriesNum++;
			}
			
			if (ReadVCALText(Line, "NOTE", Buff)) {
				GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
			//	CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Note;
				Pbk->EntriesNum++;
			}
			if (ReadVCALText_ADR(Line, "ADR;HOME", Buff) ) 
			{
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Postal_Home;
				Pbk->EntriesNum++;
			}
			if (ReadVCALText_ADR(Line, "ADR;WORK", Buff) ) 
			{
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Postal_Work;
				Pbk->EntriesNum++;
			}
			
			if (ReadVCALText_ADR(Line, "ADR", Buff)||
				ReadVCALText_ADR(Line, "ADR;PREF", Buff)) 
			{
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Postal;
				Pbk->EntriesNum++;
			}
			if (ReadVCALText(Line, "ORG", Buff)) 
			{
				char 	*pTemp = strchr((char*)Line,':' );
				unsigned char uDepartment[2000];
				unsigned char uCompany[2000];
				if(pTemp[1] == ';')
				{
					CopyUnicodeString(uDepartment,Buff);
					UnicodeReplace(uDepartment,"\\;",";");

					CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uDepartment);
					Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Custom1;
					Pbk->EntriesNum++;
				}
				else
				{
					SplitStringEx(Buff,";",uCompany,uDepartment);
					if(UnicodeLength(uCompany)>0) UnicodeReplace(uCompany,"\\;",";");
					if(UnicodeLength(uDepartment) >0) UnicodeReplace(uDepartment,"\\;",";");
					if(UnicodeLength(uCompany)>0)
					{
						CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uCompany);
						Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Company;
						Pbk->EntriesNum++;
					}
					if(UnicodeLength(uDepartment)>0)
					{
						CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uDepartment);
						Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Custom1;
						Pbk->EntriesNum++;
					}
				}

			/*	sprintf(szTemp,"%s",DecodeUnicodeConsole(Buff));
				if(szTemp[1] == ';')
				{
				//	EncodeUnicode(Pbk->Entries[Pbk->EntriesNum].Text,(unsigned char *)szTemp,strlen(szTemp)+1);
					CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
					Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Custom1;	//Department, No Comp
					Pbk->EntriesNum++;
				}
				else if(szTemp[strlen(szTemp)-1] ==';')
				{
				//	szTemp[strlen(szTemp)-1] = '\0';
				//	EncodeUnicode(Pbk->Entries[Pbk->EntriesNum].Text,(unsigned char *)szTemp,strlen(szTemp)+1);
					int nLen = UnicodeLength(Buff);
					Buff[(nLen-1)*2] = 0;
					Buff[(nLen-1)*2+1] = 0;
					Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Company;
					Pbk->EntriesNum++;
				}
				else
				{
					if(strstr(szTemp,";")==NULL)
					{
						szTemp[strlen(szTemp)-1] = '\0';
						EncodeUnicode(Pbk->Entries[Pbk->EntriesNum].Text,(unsigned char *)szTemp,strlen(szTemp)+1);
						Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Custom1;
						Pbk->EntriesNum++;
					}
					else
					{
						SamsungSplitString(szTemp,';',szCompany,szDepartment);
						EncodeUnicode(Pbk->Entries[Pbk->EntriesNum].Text,(unsigned char *)szCompany,strlen(szCompany)+1);
						Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Company;
						Pbk->EntriesNum++;

						EncodeUnicode(Pbk->Entries[Pbk->EntriesNum].Text,(unsigned char *)szDepartment,strlen(szDepartment)+1);
						Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Custom1;
						Pbk->EntriesNum++;
					}
				}*/
				
			}
			if (ReadVCALText(Line, "TITLE", Buff)) {
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_JobTitle;
				Pbk->EntriesNum++;
			}
			if (ReadVCALText(Line, "EMAIL;INTERNET;HOME", Buff)||
				ReadVCALText(Line, "EMAIL;HOME;INTERNET", Buff)) 
			{
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Email_Home;
				Pbk->EntriesNum++;
			}
			if (ReadVCALText(Line, "NICKNAME", Buff))
			{
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Nickname;
				Pbk->EntriesNum++;
			}
			
			if (ReadVCALText(Line, "EMAIL", Buff)||
				ReadVCALText(Line, "EMAIL;INTERNET;PREF",  Buff) ||
				ReadVCALText(Line, "EMAIL;INTERNET",  Buff)	||
				ReadVCALText(Line, "EMAIL;TYPE=INTERNET",  Buff)	||
				ReadVCALText(Line, "EMAIL;PREF;INTERNET", Buff))
			{
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Email;
				Pbk->EntriesNum++;
			}
			if (ReadVCALText(Line, "BDAY", Buff))
			{
				char 	*pTemp = strchr((char*)Line,':' );
				if(pTemp)
				{
//					YYYYMMDDGetDateTime(&Pbk->Entries[Pbk->EntriesNum].Date,(char*) pTemp+1);
					if(YYYYMMDD_Get_DateTime(&Pbk->Entries[Pbk->EntriesNum].Date,(char*) pTemp+1) == false) //Z-serial
						YYYYMMDDGetDateTime(&Pbk->Entries[Pbk->EntriesNum].Date,(char*) pTemp+1);
					
					Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Date;
					Pbk->EntriesNum++;
				}
			}
			if (ReadVCALText(Line, "URL;TYPE=WORK", Buff))
			{
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_URL;
				Pbk->EntriesNum++;
			}
			if (ReadVCALText(Line, "X-ESI-CATEGORIES", Buff))
			{
				CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Caller_Group_Text;
				Pbk->EntriesNum++;
			}
			break;
                }
				
		}
        if (Pbk->EntriesNum == 0) return ERR_EMPTY;
        return ERR_NONE;
}

void GSM_PhonebookFindDefaultNameCompanySamsungZ(GSM_MemoryEntry *entry, int *Name, int *LastName , 
												int *FirstName, int *Company, int *Department)
{
        int i;

        *Name   = -1;
		*LastName = -1;
		*FirstName = -1;
		*Company = -1;
		*Department = -1;

        for (i = 0; i < entry->EntriesNum; i++)
		{
                switch (entry->Entries[i].EntryType)
				{
                case PBK_Text_Name      :
					if (*Name     == -1) *Name    = i;
					break;
                case PBK_Text_LastName   : 
					if (*LastName    == -1) *LastName   = i;
					break;
                case PBK_Text_FirstName   : 
					if (*FirstName    == -1) *FirstName   = i;
					break;
                case PBK_Text_Company   : 
					if (*Company    == -1) *Company   = i;
					break;
                case PBK_Text_Custom1   : 
					if (*Department    == -1) *Department   = i;
					break;
             default                 :           
					break;
                }

        }
}

void GSM_EncodeSamsungVCARD(char *Buffer, int *Length, GSM_MemoryEntry *pbk, bool header)
{
    int     Name, i, LastName, FirstName, Number, Group, MiddleName, Title, Suffix;
	bool    ignore;
	CString csEncodeSemi;
//	char szEncoded[2000];
	char szName[2000];
	unsigned char szFullName[2000];
	unsigned char szoutput[2000];

	GSM_PhonebookFindDefaultNameNumberGroupNS6(pbk, &Name, &Number, &Group,&LastName ,&FirstName,&MiddleName,&Title,&Suffix);

    if (header) *Length+=sprintf(Buffer+(*Length),"BEGIN:VCARD%c%cVERSION:2.1%c%c",13,10,13,10);
	szName[0] = '\0';

	//for _UNICODE
	ZeroMemory(szFullName,2000);
	unsigned char uTemp[10];
	unsigned char uTempName[2000];
	EncodeUnicode(uTemp,(unsigned char *)";",2);

	if(LastName == -1 && FirstName ==-1)
	{
/*		strcat(szName,";");
		if (Name != -1)
		{
			csEncodeSemi = DecodeUnicodeString(pbk->Entries[Name].Text);
			csEncodeSemi.Replace(";","\\;");
			sprintf(szEncoded, "%s", csEncodeSemi);
			strcat(szName, (char *)szEncoded);
		}
		strcat(szName,";");*/
		//for _UNICODE
		CopyUnicodeString(szFullName,uTemp);
		if (Name != -1)
		{
			CopyUnicodeString(uTempName,pbk->Entries[Name].Text);
			UnicodeReplace(uTempName,";","\\;");
			UnicodeCat(szFullName,uTempName);
		}
		UnicodeCat(szFullName,uTemp);
	}
	else
	{
		if(LastName != -1)
		{
			CopyUnicodeString(uTempName,pbk->Entries[LastName].Text);
			UnicodeReplace(uTempName,";","\\;");
			UnicodeCat(szFullName,uTempName);
		}
		UnicodeCat(szFullName,uTemp);

		if(FirstName != -1)
		{
			CopyUnicodeString(uTempName,pbk->Entries[FirstName].Text);
			UnicodeReplace(uTempName,";","\\;");
			UnicodeCat(szFullName,uTempName);
		}
		UnicodeCat(szFullName,uTemp);
	}

	UnicodeCat(szFullName,uTemp);
	UnicodeCat(szFullName,uTemp);

//	EncodeUnicode((unsigned char *)szFullName,(unsigned char *)szName,strlen(szName)+1);
	
	SESaveVCARDText(Buffer, Length, (unsigned char *)szFullName, "N", ":");
	
    for (i=0; i < pbk->EntriesNum; i++) 
	{
		if (i != Name && i != LastName&& i != FirstName) 
		{
			ignore = false;
			switch(pbk->Entries[i].EntryType) {
			case PBK_Text_Name      :
			case PBK_Caller_Group   :
				ignore = true;
				break;
			case PBK_Date       :
				*Length+=sprintf(Buffer+(*Length),"BDAY:%04d%02d%02d%c%c",pbk->Entries[i].Date.Year,pbk->Entries[i].Date.Month,pbk->Entries[i].Date.Day,13,10);
				ignore = true;
				break;
			case PBK_Number_General :
				*Length+=sprintf(Buffer+(*Length),"TEL;CELL");
				pbk->Entries[i].EntryType = PBK_Number_Mobile;
				break;
			case PBK_Number_Work    :
				*Length+=sprintf(Buffer+(*Length),"TEL;WORK");
				break;
			case PBK_Number_Home    :
				*Length+=sprintf(Buffer+(*Length),"TEL;HOME");
				break;
			case PBK_Number_Mobile  :
				*Length+=sprintf(Buffer+(*Length),"TEL;CELL");
				break;
			case PBK_Number_Mobile_Home  :
				*Length+=sprintf(Buffer+(*Length),"TEL;CELL;HOME");
				break;
			case PBK_Number_Mobile_Work  :
				*Length+=sprintf(Buffer+(*Length),"TEL;CELL;WORK");
				break;
			case PBK_Number_Fax     :
				*Length+=sprintf(Buffer+(*Length),"TEL;FAX");
				break;
			case PBK_Number_Fax_Home     :
				*Length+=sprintf(Buffer+(*Length),"TEL;FAX;HOME");
				break;
			case PBK_Number_Fax_Work     :
				*Length+=sprintf(Buffer+(*Length),"TEL;FAX;WORK");
				break;
			case PBK_Number_Pager     :
				*Length+=sprintf(Buffer+(*Length),"TEL;PAGER");
				break;
			case PBK_Number_Pager_Home     :
				*Length+=sprintf(Buffer+(*Length),"TEL;HOME;PAGER");
				break;
			case PBK_Number_Pager_Work     :
				*Length+=sprintf(Buffer+(*Length),"TEL;PAGER;WORK");
				break;
			case PBK_Number_VideoCall     :
				*Length+=sprintf(Buffer+(*Length),"TEL;VIDEO");
				break;
			case PBK_Number_VideoCall_Home     :
				*Length+=sprintf(Buffer+(*Length),"TEL;VIDEO;HOME");
				break;
			case PBK_Number_VideoCall_Work     :
				*Length+=sprintf(Buffer+(*Length),"TEL;VIDEO;WORK");
				break;
			case PBK_Number_Other	:
				*Length+=sprintf(Buffer+(*Length),"TEL;PAGER");
				break;
			case PBK_Text_Nickname:
				SESaveVCARDText(Buffer, Length, pbk->Entries[i].Text, "X-EPOCSECONDNAME", ":");
				ignore = true;
				break;
			case PBK_Text_JobTitle:
				*Length+=sprintf(Buffer+(*Length),"TITLE");
				break;
			case PBK_Text_Note     :
				GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
				*Length+=sprintf(Buffer+(*Length),"NOTE");
				SESaveVCARDText(Buffer, Length, szoutput, "",":");
				ignore = true;
				break;
			case PBK_Text_Company:
				*Length+=sprintf(Buffer+(*Length),"ORG");
				break;
			case PBK_Text_Postal    :
				//         *Length+=sprintf(Buffer+(*Length),"ADR;PREF");
				*Length+=sprintf(Buffer+(*Length),"ADR");
				break;
			case PBK_Text_Postal_Home    :
				*Length+=sprintf(Buffer+(*Length),"ADR;HOME");
				break;
			case PBK_Text_Postal_Work    :
				*Length+=sprintf(Buffer+(*Length),"ADR;WORK");
				break;
			case PBK_Text_Email     :
				*Length+=sprintf(Buffer+(*Length),"EMAIL;INTERNET");
				break;
			case PBK_Text_Email_Home     :
				*Length+=sprintf(Buffer+(*Length),"EMAIL;INTERNET;HOME");
				break;
			case PBK_Text_Email_Work     :
				*Length+=sprintf(Buffer+(*Length),"EMAIL;INTERNET;Work");
				break;
			case PBK_Text_URL       :
				*Length+=sprintf(Buffer+(*Length),"URL");
				break;
			case PBK_Text_URL_Home       :
				*Length+=sprintf(Buffer+(*Length),"URL;HOME");
				break;
			case PBK_Text_URL_Work       :
				*Length+=sprintf(Buffer+(*Length),"URL;WORK");
				break;
			default :
				ignore = true;
				break;
			}
			if (!ignore) {
				SESaveVCARDText(Buffer, Length, pbk->Entries[i].Text, "",":");
			}
			}
    }
    if (header) *Length+=sprintf(Buffer+(*Length),"END:VCARD%c%c",13,10);
}


void GSM_EncodeSamsungZVCARD(char *Buffer, int *Length, GSM_MemoryEntry *pbk, bool header)
{
    int     Name, i, LastName, FirstName, Company, Department;
	bool    ignore;
	CString csEncodeSemi;
//	char szEncoded[2000];
	char szName[2000];
	unsigned char szFullName[2000];
//	char szCompany[1000];
	unsigned char szFullCompany[2000];
//	unsigned char szoutput[2000];

   GSM_PhonebookFindDefaultNameCompanySamsungZ(pbk, &Name,&LastName ,&FirstName,&Company,&Department);

    if (header) *Length+=sprintf(Buffer+(*Length),"BEGIN:VCARD%c%cVERSION:2.1%c%c",13,10,13,10);
	szName[0] = '\0';

	//for _UNICODE
	ZeroMemory(szFullName,2000);
	unsigned char uTemp[10];
	unsigned char uTempName[2000];
	EncodeUnicode(uTemp,(unsigned char *)";",2);

	if(LastName == -1 && FirstName ==-1)
	{
	/*	strcat(szName,";");
		if (Name != -1)
		{
			csEncodeSemi = DecodeUnicodeString(pbk->Entries[Name].Text);
			csEncodeSemi.Replace(";","\\;");
			sprintf(szEncoded, "%s", csEncodeSemi);
			strcat(szName, (char *)szEncoded);
				//strcat(szName,(char*)DecodeUnicodeString(pbk->Entries[Name].Text));
		}
			//strcat(szName,(char*)DecodeUnicodeString(pbk->Entries[Name].Text));
		strcat(szName,";");*/
		//for _UNICODE
		CopyUnicodeString(szFullName,uTemp);
		if (Name != -1)
		{
			CopyUnicodeString(uTempName,pbk->Entries[Name].Text);
			UnicodeReplace(uTempName,";","\\;");
			UnicodeCat(szFullName,uTempName);
		}
		UnicodeCat(szFullName,uTemp);

	}
	else
	{
	/*	if(LastName != -1)
		{
			csEncodeSemi =DecodeUnicodeString(pbk->Entries[LastName].Text);
			csEncodeSemi.Replace(";","\\;");
			sprintf(szEncoded, "%s", csEncodeSemi);
			strcat(szName, (char *)szEncoded);
		}
		strcat(szName,";");

		if(FirstName != -1)
		{
			csEncodeSemi =DecodeUnicodeString(pbk->Entries[FirstName].Text);
			csEncodeSemi.Replace(";","\\;");
			sprintf(szEncoded, "%s", csEncodeSemi);
			strcat(szName, (char *)szEncoded);
		}
		strcat(szName,";");*/
		//for _UNICODE
		if(LastName != -1)
		{
			CopyUnicodeString(uTempName,pbk->Entries[LastName].Text);
			UnicodeReplace(uTempName,";","\\;");
			UnicodeCat(szFullName,uTempName);
		}
		UnicodeCat(szFullName,uTemp);

		if(FirstName != -1)
		{
			CopyUnicodeString(uTempName,pbk->Entries[FirstName].Text);
			UnicodeReplace(uTempName,";","\\;");
			UnicodeCat(szFullName,uTempName);
		}
		UnicodeCat(szFullName,uTemp);
	}
	UnicodeCat(szFullName,uTemp);
	UnicodeCat(szFullName,uTemp);
//	strcat(szName,";");
//	strcat(szName,";");
//	EncodeUnicode((unsigned char *)szFullName,(unsigned char *)szName,strlen(szName)+1);
	SESaveVCARDText(Buffer, Length, (unsigned char *)szFullName, "N", ":");

	if(Company != -1 || Department!=-1)
	{
	/*	sprintf(szCompany,"");
		if(Company != -1)
		{
			sprintf(szEncoded, "%s", DecodeUnicodeString(pbk->Entries[Company].Text));
			strcat(szCompany, (char *)szEncoded);
		}
		strcat(szCompany, ";");
		if(Department!=-1)
		{
			sprintf(szEncoded, "%s", DecodeUnicodeString(pbk->Entries[Department].Text));
			strcat(szCompany, (char *)szEncoded);
		}
			EncodeUnicode((unsigned char *)szFullCompany,(unsigned char *)szCompany,strlen(szCompany)+1);*/
		//for _UNICODE
		ZeroMemory(szFullCompany,2000);
		if(Company != -1)
		{
			CopyUnicodeString(szFullCompany,pbk->Entries[Company].Text);
		}
		UnicodeCat(szFullCompany,uTemp);

		if(Department!=-1)
		{
			UnicodeCat(szFullCompany,pbk->Entries[Department].Text);
		}
//			EncodeUnicode((unsigned char *)szFullCompany,(unsigned char *)szCompany,strlen(szCompany)+1);


		SESaveVCARDText(Buffer, Length, (unsigned char *)szFullCompany, "ORG", ":");
	}

	
    for (i=0; i < pbk->EntriesNum; i++) 
	{
		if (i != Name && i != LastName&& i != FirstName && i != Department && i != Company) 
		{
			ignore = false;
			switch(pbk->Entries[i].EntryType) {
			case PBK_Text_Name      :
			case PBK_Caller_Group   :
				ignore = true;
				break;
			case PBK_Date       :
				*Length+=sprintf(Buffer+(*Length),"BDAY:%04d-%02d-%02dT00:00:00Z%c%c",pbk->Entries[i].Date.Year,pbk->Entries[i].Date.Month,pbk->Entries[i].Date.Day,13,10);
		//		ignore = true;
				break;
			case PBK_Number_General :
				*Length+=sprintf(Buffer+(*Length),"TEL;TYPE=CELL; VOICE");
				pbk->Entries[i].EntryType = PBK_Number_Mobile;
				break;
			case PBK_Number_Work    :
				*Length+=sprintf(Buffer+(*Length),"TEL;TYPE=WORK; VOICE");
				break;
			case PBK_Number_Home    :
				*Length+=sprintf(Buffer+(*Length),"TEL;TYPE=HOME; VOICE");
				break;
			case PBK_Number_Mobile  :
				*Length+=sprintf(Buffer+(*Length),"TEL;TYPE=CELL; VOICE");
				break;
			case PBK_Number_Fax     :
				*Length+=sprintf(Buffer+(*Length),"TEL;TYPE=FAX");
				break;
			case PBK_Number_Other	:
				*Length+=sprintf(Buffer+(*Length),"TEL;TYPE=VOICE");
				break;
			case PBK_Text_Nickname:
				*Length+=sprintf(Buffer+(*Length),"NICKNAME");
				break;;
			case PBK_Text_JobTitle:
				*Length+=sprintf(Buffer+(*Length),"TITLE");
				break;
			case PBK_Text_Note     :
				*Length+=sprintf(Buffer+(*Length),"NOTE");
				break;
			case PBK_Text_Email     :
				*Length+=sprintf(Buffer+(*Length),"EMAIL;TYPE=INTERNET");
				break;
			case PBK_Text_URL       :
				*Length+=sprintf(Buffer+(*Length),"URL");
				break;
			default :
				ignore = true;
				break;
			}
			if (!ignore) {
				SESaveVCARDText(Buffer, Length, pbk->Entries[i].Text, "",":");
			}
			}
    }
    if (header) *Length+=sprintf(Buffer+(*Length),"END:VCARD");
}

void GSM_EncodeNokiaS6VCARD(char *Buffer, int *Length, GSM_MemoryEntry *pbk, bool header)
{
    int     Name, Number, Group, i,LastName ,FirstName;
	int MiddleName,Title,Suffix;
	bool    ignore;
	char szName[2000];
	unsigned char szFullName[2000];
	unsigned char szoutput[2000];
	unsigned char uTemp[10];
	szFullName[0] = szFullName[1] = 0;
	szoutput[0] = szoutput[1] = 0;
//	char szLastName[2000];
//	char szFirstName[2000];
	EncodeUnicode(uTemp,(unsigned char *)";",2);
    GSM_PhonebookFindDefaultNameNumberGroupNS6(pbk, &Name, &Number, &Group,&LastName ,&FirstName,&MiddleName,&Title,&Suffix);

    if (header) *Length+=sprintf(Buffer+(*Length),"BEGIN:VCARD%c%cVERSION:2.1%c%c",13,10,13,10);
	szName[0] = '\0';
/*
	if(LastName == -1 && FirstName ==-1)
	{
		if (Name != -1)
		{
			GSM_SemicolonEncode(pbk->Entries[Name].Text,szoutput);
			strcat(szName,(char*)DecodeUnicodeString(szoutput));
		}
		strcat(szName,";");
	}
	else
	{
		if(LastName != -1)
		{
			GSM_SemicolonEncode(pbk->Entries[LastName].Text,szoutput);
			strcat(szName,(char*)DecodeUnicodeString(szoutput));
		}
		strcat(szName,";");

		if(FirstName != -1)
		{
			GSM_SemicolonEncode(pbk->Entries[FirstName].Text,szoutput);
			strcat(szName,(char*)DecodeUnicodeString(szoutput));
		}
		strcat(szName,";");
	}

	if(MiddleName != -1)
	{
		GSM_SemicolonEncode(pbk->Entries[MiddleName].Text,szoutput);
		strcat(szName,(char*)DecodeUnicodeString(szoutput));
	}
	strcat(szName,";");

	if(Title != -1)
	{
		GSM_SemicolonEncode(pbk->Entries[Title].Text,szoutput);
		strcat(szName,(char*)DecodeUnicodeString(szoutput));
	}
	strcat(szName,";");

	if(Suffix != -1)
	{
		GSM_SemicolonEncode(pbk->Entries[Suffix].Text,szoutput);
		strcat(szName,(char*)DecodeUnicodeString(szoutput));
	}

	EncodeUnicode((unsigned char *)szFullName,(unsigned char *)szName,strlen(szName)+1);
*/
	//for _UNICODE
	if(LastName == -1 && FirstName ==-1)
	{
		if (Name != -1)
		{
			GSM_SemicolonEncode(pbk->Entries[Name].Text,szoutput);
			UnicodeCat(szFullName,szoutput);
		}
		UnicodeCat(szFullName,uTemp);
	}
	else
	{
		if(LastName != -1)
		{
			GSM_SemicolonEncode(pbk->Entries[LastName].Text,szoutput);
			UnicodeCat(szFullName,szoutput);
		}
		UnicodeCat(szFullName,uTemp);

		if(FirstName != -1)
		{
			GSM_SemicolonEncode(pbk->Entries[FirstName].Text,szoutput);
			UnicodeCat(szFullName,szoutput);
		}
		UnicodeCat(szFullName,uTemp);
	}

	if(MiddleName != -1)
	{
		GSM_SemicolonEncode(pbk->Entries[MiddleName].Text,szoutput);
		UnicodeCat(szFullName,szoutput);
	}
	UnicodeCat(szFullName,uTemp);

	if(Title != -1)
	{
		GSM_SemicolonEncode(pbk->Entries[Title].Text,szoutput);
		UnicodeCat(szFullName,szoutput);
	}
	UnicodeCat(szFullName,uTemp);

	if(Suffix != -1)
	{
		GSM_SemicolonEncode(pbk->Entries[Suffix].Text,szoutput);
		UnicodeCat(szFullName,szoutput);
	}

	SESaveVCARDText(Buffer, Length, (unsigned char *)szFullName, "N", ":");
/*


	if(LastName != -1 && FirstName!=-1)
	{
		sprintf(szLastName,"%s",DecodeUnicodeString(pbk->Entries[LastName].Text));
		sprintf(szFirstName,"%s",DecodeUnicodeString(pbk->Entries[FirstName].Text));
		sprintf(szName,"%s;%s",szLastName,szFirstName);
        
		EncodeUnicode((unsigned char *)szFullName,(unsigned char *)szName,strlen(szName)+1);

		SESaveVCARDText(Buffer, Length, (unsigned char *)szFullName, "N", ":");

	}
 	else if(LastName != -1)
	{
         SESaveVCARDText(Buffer, Length, pbk->Entries[LastName].Text, "N", ":");
	}
	else if(FirstName != -1)
	{
		sprintf(szName,";%s",DecodeUnicodeString(pbk->Entries[FirstName].Text));
        
		EncodeUnicode((unsigned char *)szFullName,(unsigned char *)szName,strlen(szName)+1);

		SESaveVCARDText(Buffer, Length, (unsigned char *)szFullName, "N", ":");
	}
   else if (Name != -1)
	{
 		sprintf(szName,";%s",DecodeUnicodeString(pbk->Entries[Name].Text));
        
		EncodeUnicode((unsigned char *)szFullName,(unsigned char *)szName,strlen(szName)+1);

		SESaveVCARDText(Buffer, Length, (unsigned char *)szFullName, "N", ":");
    }
*/

    for (i=0; i < pbk->EntriesNum; i++) 
	{
            if (i != Name && i != LastName&& i != FirstName) 
			{
                    ignore = false;
                    switch(pbk->Entries[i].EntryType) {
					case PBK_Text_Name      :
                    case PBK_Caller_Group   :
                            ignore = true;
                            break;
                    case PBK_Date       :
                            *Length+=sprintf(Buffer+(*Length),"BDAY:%04d%02d%02d%c%c",pbk->Entries[i].Date.Year,pbk->Entries[i].Date.Month,pbk->Entries[i].Date.Day,13,10);
							 ignore = true;
							break;
                    case PBK_Number_General :
                            *Length+=sprintf(Buffer+(*Length),"TEL;VOICE");
                            break;
                   case PBK_Number_Work    :
                            *Length+=sprintf(Buffer+(*Length),"TEL;VOICE;WORK");
                            break;
                    case PBK_Number_Home    :
                            *Length+=sprintf(Buffer+(*Length),"TEL;VOICE;HOME");
                            break;
                    case PBK_Number_Mobile  :
                            *Length+=sprintf(Buffer+(*Length),"TEL;CELL");
                             break;
                     case PBK_Number_Mobile_Home  :
                            *Length+=sprintf(Buffer+(*Length),"TEL;CELL;HOME");
                             break;
                     case PBK_Number_Mobile_Work  :
                            *Length+=sprintf(Buffer+(*Length),"TEL;CELL;WORK");
                             break;
                    case PBK_Number_Fax     :
                            *Length+=sprintf(Buffer+(*Length),"TEL;FAX");
                            break;
                    case PBK_Number_Fax_Home     :
                            *Length+=sprintf(Buffer+(*Length),"TEL;FAX;HOME");
                            break;
                    case PBK_Number_Fax_Work     :
                            *Length+=sprintf(Buffer+(*Length),"TEL;FAX;WORK");
                            break;
                    case PBK_Number_Pager     :
                            *Length+=sprintf(Buffer+(*Length),"TEL;PAGER");
                            break;
                    case PBK_Number_Pager_Home     :
                            *Length+=sprintf(Buffer+(*Length),"TEL;HOME;PAGER");
                            break;
                    case PBK_Number_Pager_Work     :
                            *Length+=sprintf(Buffer+(*Length),"TEL;PAGER;WORK");
                            break;
                    case PBK_Number_VideoCall     :
                            *Length+=sprintf(Buffer+(*Length),"TEL;VIDEO");
                            break;
                    case PBK_Number_VideoCall_Home     :
                            *Length+=sprintf(Buffer+(*Length),"TEL;VIDEO;HOME");
                            break;
                    case PBK_Number_VideoCall_Work     :
                            *Length+=sprintf(Buffer+(*Length),"TEL;VIDEO;WORK");
                            break;
                  case PBK_Text_Nickname:
							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "X-EPOCSECONDNAME", ":");
							ignore = true;
                            break;
                  case PBK_Text_Company:
                             *Length+=sprintf(Buffer+(*Length),"ORG");
							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "",":");
							ignore = true;

                            break;
                    case PBK_Text_JobTitle:
                           *Length+=sprintf(Buffer+(*Length),"TITLE");
 							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "",":");
							ignore = true;
                           break;
 					case PBK_Text_Note      :
                            *Length+=sprintf(Buffer+(*Length),"NOTE");
							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "",":");
							ignore = true;
                            break;
                    case PBK_Text_Postal    :
                   //         *Length+=sprintf(Buffer+(*Length),"ADR;PREF");
                            *Length+=sprintf(Buffer+(*Length),"ADR");
                           break;
                    case PBK_Text_Postal_Home    :
                            *Length+=sprintf(Buffer+(*Length),"ADR;HOME");
                            break;
                    case PBK_Text_Postal_Work    :
                            *Length+=sprintf(Buffer+(*Length),"ADR;WORK");
                            break;
                    case PBK_Text_Email     :
                            *Length+=sprintf(Buffer+(*Length),"EMAIL;INTERNET");
							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "",":");
							ignore = true;
                            break;
                    case PBK_Text_Email_Home     :
                            *Length+=sprintf(Buffer+(*Length),"EMAIL;INTERNET;HOME");
							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "",":");
							ignore = true;
                            break;
                     case PBK_Text_Email_Work     :
                            *Length+=sprintf(Buffer+(*Length),"EMAIL;INTERNET;Work");
 							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "",":");
							ignore = true;
                           break;
                   case PBK_Text_URL       :
                            *Length+=sprintf(Buffer+(*Length),"URL");
 							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "",":");
							ignore = true;
                           break;
                   case PBK_Text_URL_Home       :
                            *Length+=sprintf(Buffer+(*Length),"URL;HOME");
  							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "",":");
							ignore = true;
                          break;
                   case PBK_Text_URL_Work       :
                            *Length+=sprintf(Buffer+(*Length),"URL;WORK");
 							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "",":");
							ignore = true;
                           break;
                    case PBK_Text_Custom1       :
                            *Length+=sprintf(Buffer+(*Length),"X-SIP;VOIP");
 							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "",":");
							ignore = true;
                           break;
                    case PBK_Text_Custom2       :
                            *Length+=sprintf(Buffer+(*Length),"X-SIP;VOIP;HOME");
 							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "",":");
							ignore = true;
                           break;
                    case PBK_Text_Custom3       :
                            *Length+=sprintf(Buffer+(*Length),"X-SIP;VOIP;WORK");
 							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "",":");
							ignore = true;
                           break;
                   default :
                            ignore = true;
                            break;
                    }
                    if (!ignore) {
                            SESaveVCARDText(Buffer, Length, pbk->Entries[i].Text, "",":");
                    }
            }
    }
    if (header) *Length+=sprintf(Buffer+(*Length),"END:VCARD%c%c",13,10);
}
GSM_Error SAGEM_DecodeVCARD(unsigned char *Buffer, int *Pos, GSM_MemoryEntry *Pbk)
{
        unsigned char   Line[2000],Buff[2000];
        int             Level = 0;//,num;
        unsigned char   Line2[2000]; //peggy
		int npos = *Pos;//peggy

//		char  str[30];
		//peggy add 02242005 +
		unsigned char uLastName[2000];
		unsigned char uFirstName[2000];
		unsigned char uName[2000];
//		char szName1[2000];
		unsigned char uTempSpace[10];
		EncodeUnicode(uTempSpace,(unsigned char*)" ",2);
		int i,j ;
		i=j=0;

		uLastName[0]='\0';
		uFirstName[0]='\0';
		uLastName[1]='\0';
		uFirstName[1]='\0';
		uName[0] =uName[1] = '\0';
		//peggy add 02242005 -
        Buff[0]         = 0;
        Pbk->EntriesNum = 0;
        if (Buffer == NULL) return ERR_EMPTY;

        while (1) {
                MyGetLine(Buffer, Pos, Line, strlen((char *)Buffer));
                if (strlen((char *)Line) == 0) break;
 	
				//Modify for SE K750i
				//=E6=88=91=E9=96=80=E6=98=A=..F=E4=B8=80=E5=AE=B6=E4=BA=BA..
				npos = *Pos;
				memcpy(Line2,Line,strlen((char*)Line)+1);
				while(Line2[strlen((char*)Line2)-1] == '=')
				{
					if(Buffer[npos+2] == 0x0d && Buffer[npos+3] == 0x0A)
					{
						if(Line[strlen((char*)Line)-1] == '=')
							Line[strlen((char*)Line)-1]= '\0';
						break;
					}
					MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
					if(Line2[0] == '=')
					{
						strcat((char *)Line,(char *)Line2+1);
					}
					else
					{
						Line[strlen((char*)Line)-1]= '\0';
						strcat((char *)Line,(char *)Line2);
					}
				}
				MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
				while(Line2[0] == 0x20 && Line2[1] == 0x20)
				{
					strcat((char *)Line,(char *)Line2);
					MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
				}
                switch (Level) {
                case 0:
                        if (strstr((char *)Line,"BEGIN:VCARD")) Level = 1;
                        break;
                case 1:
                        if (strstr((char *)Line,"END:VCARD")) {
                                if (Pbk->EntriesNum == 0) return ERR_EMPTY;
                                return ERR_NONE;
                        }
                      if (ReadVCALText(Line, "N", Buff))
					  {

								char 	*pTemp = strchr((char*)Line,':' );
								if(pTemp[1] == ';')
								{
									CopyUnicodeString(uName,Buff);
									UnicodeReplace(uName,"\\;",";");
				
									CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uName);
									Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Name;
									Pbk->EntriesNum++;

									CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uName);
									Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_FirstName;
									Pbk->EntriesNum++;

								}
								else
								{
									SplitStringEx(Buff,";",uLastName,uFirstName);
									if(UnicodeLength(uLastName)>0) UnicodeReplace(uLastName,"\\;",";");
									if(UnicodeLength(uFirstName) >0) UnicodeReplace(uFirstName,"\\;",";");

									if(UnicodeLength(uFirstName) >0)
									{
										if(UnicodeLength(uLastName) > 0)
										{
											CopyUnicodeString(uName,uFirstName);
											UnicodeCat(uName,uTempSpace);
											UnicodeCat(uName,uLastName);
										}
										else
											CopyUnicodeString(uName,uFirstName);
									}
									else if(UnicodeLength(uLastName)>0)
										CopyUnicodeString(uName,uLastName);

									if(UnicodeLength(uName)>0)
									{
										CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uName);
										Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Name;
										Pbk->EntriesNum++;
									}
									if(UnicodeLength(uFirstName)>0)
									{
										CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uFirstName);
										Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_FirstName;
										Pbk->EntriesNum++;
									}
									if(UnicodeLength(uLastName) >0)
									{
										CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uLastName);
										Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_LastName;
										Pbk->EntriesNum++;
									}
								}

                        }
                       if (ReadVCALText(Line, "TEL",                   Buff) ||
                            ReadVCALText(Line, "TEL;VOICE",             Buff) ||
                            ReadVCALText(Line, "TEL;PREF",              Buff) ||
                            ReadVCALText(Line, "TEL;PREF;VOICE",        Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
								Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_General;
								Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "TEL;CELL",              Buff) ||
                            ReadVCALText(Line, "TEL;CELL;VOICE",        Buff) ||
                            ReadVCALText(Line, "TEL;PREF;CELL",         Buff) ||
							ReadVCALText(Line, "TEL;TYPE=CELL; VOICE",  Buff) ||
                            ReadVCALText(Line, "TEL;PREF;CELL;VOICE",   Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Mobile;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "TEL;WORK",              Buff) ||
                            ReadVCALText(Line, "TEL;PREF;WORK",         Buff) ||
                            ReadVCALText(Line, "TEL;WORK;VOICE",        Buff) ||
                            ReadVCALText(Line, "TEL;PREF;WORK;VOICE",   Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Work;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "TEL;FAX",               Buff) ||
                            ReadVCALText(Line, "TEL;PREF;FAX",          Buff) ||
                            ReadVCALText(Line, "TEL;FAX;VOICE",         Buff) ||
							ReadVCALText(Line, "TEL;FAX;HOME",         Buff) || //S55
                            ReadVCALText(Line, "TEL;PREF;FAX;VOICE",    Buff) ) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Fax;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "TEL;HOME",              Buff) ||
                            ReadVCALText(Line, "TEL;PREF;HOME",         Buff) ||
                            ReadVCALText(Line, "TEL;HOME;VOICE",        Buff) ||
                            ReadVCALText(Line, "TEL;PREF;HOME;VOICE",   Buff)) {
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_General;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "NOTE", Buff)) {
								GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                               // CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Note;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "ADR", Buff)||
                            ReadVCALText(Line, "ADR;HOME", Buff) )
						{
/*							char szTemp[5000];
							char szTempPostal[5000];
							char szTempPostal1[5000];
							sprintf(szTempPostal,"%s",DecodeUnicodeConsole(Buff));
							SplitStringEx(szTempPostal,';',szTempPostal1,szTemp);
							SplitStringEx(szTemp,';',szTempPostal1,szTemp);
							if(strlen(szTempPostal1) >0)
							{
								EncodeUnicode(Pbk->Entries[Pbk->EntriesNum].Text,szTempPostal1,strlen(szTempPostal1));
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_StreetAddress;
                                Pbk->EntriesNum++;
							}
							SplitStringEx(szTemp,';',szTempPostal1,szTemp);
							if(strlen(szTempPostal1) >0)
							{
								EncodeUnicode(Pbk->Entries[Pbk->EntriesNum].Text,szTempPostal1,strlen(szTempPostal1));
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_City;
                                Pbk->EntriesNum++;
							}

							SplitStringEx(szTemp,';',szTempPostal1,szTemp);
							if(strlen(szTempPostal1) >0)
							{
								EncodeUnicode(Pbk->Entries[Pbk->EntriesNum].Text,szTempPostal1,strlen(szTempPostal1));
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_City;
                                Pbk->EntriesNum++;
							}*/

                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Postal;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "ORG", Buff)) {
								GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                             //   CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Company;
                                Pbk->EntriesNum++;
                        }
                         if (ReadVCALText(Line, "TITLE", Buff)) {
								 GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                          //      CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_JobTitle;
                                Pbk->EntriesNum++;
                        }
						  if ( ReadVCALText(Line, "EMAIL;PREF;INTERNET", Buff))
						  {
							  GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Email;
                                Pbk->EntriesNum++;
                        }
						  if ( ReadVCALText(Line, "EMAIL;INTERNET", Buff))
						  {
								  GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Email2;
                                Pbk->EntriesNum++;
                        }
                        if (ReadVCALText(Line, "URL", Buff)||
                            ReadVCALText(Line, "URL;WORK",         Buff)) {
								GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                                CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                                Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_URL;
                                Pbk->EntriesNum++;
                        }
						if (ReadVCALText(Line, "BDAY", Buff))
						{
							YYYYMMDD_Get_DateTime(&Pbk->Entries[Pbk->EntriesNum].Date,(char*) Line);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Date;
                            Pbk->EntriesNum++;
                        }
						if (ReadVCALText(Line, "X-ESI-CATEGORIES", Buff))
						{
							CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Caller_Group_Text;
                            Pbk->EntriesNum++;
                        }
                        break;
                }
        }

        if (Pbk->EntriesNum == 0) return ERR_EMPTY;
        return ERR_NONE;
}
void SAGEM_EncodeVCARD(char *Buffer, int *Length, GSM_MemoryEntry *pbk, bool header)
{
        int     Name, Number, Group, i,LastName ,FirstName;
        bool    ignore;
//		char szName[2000];
//		char szLastName[2000];
//		char szFirstName[2000];
		unsigned char szFullName[2000];
		unsigned char szoutput[2000];
		int j;
		i=j=0;

//        GSM_PhonebookFindDefaultNameNumberGroup(pbk, &Name, &Number, &Group);
		GSM_PhonebookFindDefaultNameNumberGroupSE(pbk, &Name, &Number, &Group,&LastName ,&FirstName);


        if (header) *Length+=sprintf(Buffer+(*Length),"BEGIN:VCARD%c%cVERSION:2.1%c%c",13,10,13,10);
        if (Name != -1)
		{
			/*	sprintf(szName,"%s",DecodeUnicodeConsole(pbk->Entries[Name].Text));
				CString strName = szName;
				strName.Replace(";","\\;");
				sprintf(szName,strName);

				unsigned char	   	Text[(GSM_PHONEBOOK_TEXT_LENGTH+1)*2];
				EncodeUnicode(Text,(unsigned char *)szName,strlen(szName)+1);*/
				unsigned char	   	Text[(GSM_PHONEBOOK_TEXT_LENGTH+1)*2];
				CopyUnicodeString(Text,pbk->Entries[Name].Text);
				UnicodeReplace(Text,";","\\;");

                SiemensSaveVCARDText(Buffer, Length,(char *)Text, "N","");


        }
		else
		{
	/*		szLastName[0]='\0';
			szFirstName[0]='\0';
			if(LastName != -1)
			{
				sprintf(szLastName,"%s",DecodeUnicodeString(pbk->Entries[LastName].Text));
				CString strLastName = szLastName;
				strLastName.Replace(";","\\;");
				sprintf(szLastName,strLastName);
			}
			if(FirstName != -1)
			{
				sprintf(szFirstName,"%s",DecodeUnicodeString(pbk->Entries[FirstName].Text));
			
				CString strFirstName = szFirstName;
				strFirstName.Replace(";","\\;");
				sprintf(szFirstName,strFirstName);

			}
			sprintf(szName,"%s;%s",szLastName,szFirstName);
			EncodeUnicode((unsigned char *)szFullName,(unsigned char *)szName,strlen(szName)+1);
			SiemensSaveVCARDText(Buffer, Length, szFullName, "N", "");

			sprintf(szName,"%s %s",szFirstName,szLastName);
			CString strFullname = szName;
			strFullname.Replace("\\;",";");
			sprintf(szName,strFullname);

			EncodeUnicode(pbk->Entries[pbk->EntriesNum].Text,(unsigned char *)szName,strlen(szName)+1);
			pbk->Entries[pbk->EntriesNum].EntryType = PBK_Text_Name;
			pbk->EntriesNum++;*/

			//for _UNICODE
			unsigned char uLastName[4000];
			unsigned char uFirstName[4000];
			unsigned char uTemp[10];
			unsigned char uTemp2[10];
		
			uLastName[0] =uLastName[1] =0;
			uFirstName[0] =uFirstName[1] =0;

			EncodeUnicode(uTemp,(unsigned char *)";",2);
			EncodeUnicode(uTemp2,(unsigned char *)" ",2);

			if(LastName != -1)
			{
				CopyUnicodeString(uLastName,pbk->Entries[LastName].Text);
				UnicodeReplace(uLastName,";","\\;");
			}
			if(FirstName != -1)
			{
				CopyUnicodeString(uFirstName,pbk->Entries[FirstName].Text);
				UnicodeReplace(uFirstName,";","\\;");
			}
			CopyUnicodeString(szFullName,uLastName);
			UnicodeCat(szFullName,uTemp);
			UnicodeCat(szFullName,uFirstName);

			SiemensSaveVCARDText(Buffer, Length, (char *)szFullName, "N", "");

			if(FirstName != -1)
			{
				CopyUnicodeString(pbk->Entries[pbk->EntriesNum].Text,pbk->Entries[FirstName].Text);
				UnicodeCat(pbk->Entries[pbk->EntriesNum].Text,uTemp2);
				if(LastName != -1)
					UnicodeCat(pbk->Entries[pbk->EntriesNum].Text,pbk->Entries[LastName].Text);
				pbk->Entries[pbk->EntriesNum].EntryType = PBK_Text_Name;
				pbk->EntriesNum++;
			}
			else if(LastName != -1)
			{
				CopyUnicodeString(pbk->Entries[pbk->EntriesNum].Text,pbk->Entries[LastName].Text);
				pbk->Entries[pbk->EntriesNum].EntryType = PBK_Text_Name;
				pbk->EntriesNum++;
			}


		}
        for (i=0; i < pbk->EntriesNum; i++) {
                if (i != Name) {
                        ignore = false;
                        switch(pbk->Entries[i].EntryType) {
                        case PBK_Text_Name      :
                        //case PBK_Date           :
                        case PBK_Caller_Group   :
                                ignore = true;
                                break;
                        case PBK_Number_General :
                                *Length+=sprintf(Buffer+(*Length),"TEL");
                                if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                *Length+=sprintf(Buffer+(*Length),";HOME");
                                break;
                        case PBK_Number_Mobile  :
                                *Length+=sprintf(Buffer+(*Length),"TEL");
                                if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                *Length+=sprintf(Buffer+(*Length),";CELL");
                                break;
                        case PBK_Number_Work    :
                                *Length+=sprintf(Buffer+(*Length),"TEL");
                                if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                *Length+=sprintf(Buffer+(*Length),";WORK");
                                break;
                        case PBK_Number_Fax     :
                                *Length+=sprintf(Buffer+(*Length),"TEL");
                                if (Number == i) (*Length)+=sprintf(Buffer+(*Length),";PREF");
                                *Length+=sprintf(Buffer+(*Length),";FAX");
                                break;
                        case PBK_Text_Note      :
								*Length+=sprintf(Buffer+(*Length),"NOTE");
								GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
								SESaveVCARDText(Buffer, Length, szoutput, "",":");
								ignore = true;
                            break;
                               break;
                        case PBK_Text_Postal    :
								*Length+=sprintf(Buffer+(*Length),"ADR");
                                  SESaveVCARDText(Buffer, Length, pbk->Entries[i].Text, "",":;");
 								ignore = true;
                             break;
                        case PBK_Text_Email     :
                                 *Length+=sprintf(Buffer+(*Length),"EMAIL;PREF;INTERNET");
								GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
								SESaveVCARDText(Buffer, Length, szoutput, "",":");
								ignore = true;
                                break;
                       case PBK_Text_Email2    :
                                *Length+=sprintf(Buffer+(*Length),"EMAIL;INTERNET");
								GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
								SESaveVCARDText(Buffer, Length, szoutput, "",":");
								ignore = true;
                                break;
                        case PBK_Text_URL       :
                                *Length+=sprintf(Buffer+(*Length),"URL");
  								GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
								SESaveVCARDText(Buffer, Length, szoutput, "",":");
								ignore = true;
                              break;
						case PBK_Date       :
								*Length+=sprintf(Buffer+(*Length),"BDAY");
								sprintf((char *)pbk->Entries[i].Text,"%s-%s-%s",pbk->Entries[i].Date.Year,pbk->Entries[i].Date.Month,pbk->Entries[i].Date.Day);
                                break;
						  case PBK_Text_Company:
								 *Length+=sprintf(Buffer+(*Length),"ORG");
								GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
								SESaveVCARDText(Buffer, Length, szoutput, "",":");
								ignore = true;

                            break;
                       default :
                                ignore = true;
                                break;
                        }
                        if (!ignore) {
                                SESaveVCARDText(Buffer, Length, pbk->Entries[i].Text, "",":");
                        }
                }
        }
        if (header) *Length+=sprintf(Buffer+(*Length),"END:VCARD%c%c",13,10);
}
GSM_Error MOTOE2_DecodeVCARD(unsigned char *Buffer,int nBufferSize, int *Pos, GSM_MemoryEntry *Pbk)
{
    unsigned char   Line[2000],Buff[2000];
    int             Level = 0;
    unsigned char   Line2[2000]; 
	int npos = *Pos;

//	char szTemp[2000];
//	char szTemp1[2000];
	unsigned char uLastName[2000];
	unsigned char uFirstName[2000];
	char szMiddleName[2000];
	char szTitle[2000];
	char szSuffix[2000];
	unsigned char uName[2000];
//	char szName1[2000];
	unsigned char uTempSpace[10];
	EncodeUnicode(uTempSpace,(unsigned char*)" ",2);
	int i,j ;
	i=j=0;
	uLastName[0]='\0';
	uFirstName[0]='\0';
	uLastName[1]='\0';
	uFirstName[1]='\0';
	uName[0] =uName[1] = '\0';
	szMiddleName[0]='\0';
	szTitle[0]='\0';
	szSuffix[0]='\0';

    Buff[0]         = 0;
    Pbk->EntriesNum = 0;
    if (Buffer == NULL) return ERR_EMPTY;

    while (1) {
            MyGetLine(Buffer, Pos, Line, nBufferSize);
            if (strlen((char *)Line) == 0)
			{
				(*Pos)++;
				if(*Pos >=nBufferSize)
					break;

			}
			//Modify for SE K750i
			//=E6=88=91=E9=96=80=E6=98=A=..F=E4=B8=80=E5=AE=B6=E4=BA=BA..
			npos = *Pos;
			memcpy(Line2,Line,strlen((char*)Line)+1);
			while(Line2[strlen((char*)Line2)-1] == '=')
			{
				if(Buffer[npos+2] == 0x0d && Buffer[npos+3] == 0x0A)
				{
					if(Line[strlen((char*)Line)-1] == '=')
						Line[strlen((char*)Line)-1]= '\0';
					break;
				}
				MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
				if(Line2[0] == '=')
				{
					strcat((char *)Line,(char *)Line2+1);
				}
				else
				{
					Line[strlen((char*)Line)-1]= '\0';
					strcat((char *)Line,(char *)Line2);
				}
			}
			MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
			while(Line2[0] == 0x20 && Line2[1] == 0x20)
			{
				strcat((char *)Line,(char *)Line2);
				MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
			}
          switch (Level) {
            case 0:
                    if (strstr((char *)Line,"BEGIN:VCARD")) Level = 1;
                    break;
            case 1:
                    if (strstr((char *)Line,"END:VCARD")) {
                            if (Pbk->EntriesNum == 0) return ERR_EMPTY;
                            return ERR_NONE;
                    }
                  if (ReadVCALText(Line, "N", Buff))
				  {
						char 	*pTemp = strchr((char*)Line,':' );
						if(pTemp[1] == ';')
						{
							CopyUnicodeString(uName,Buff);
							UnicodeReplace(uName,"\\;",";");
		
							CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uName);
							Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Name;
							Pbk->EntriesNum++;

							CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uName);
							Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_FirstName;
							Pbk->EntriesNum++;

						}
						else
						{
							SplitStringEx(Buff,";",uLastName,uFirstName);
							if(UnicodeLength(uLastName)>0) UnicodeReplace(uLastName,"\\;",";");
							if(UnicodeLength(uFirstName) >0) UnicodeReplace(uFirstName,"\\;",";");

							if(UnicodeLength(uFirstName) >0)
							{
								if(UnicodeLength(uLastName) > 0)
								{
									CopyUnicodeString(uName,uFirstName);
									UnicodeCat(uName,uTempSpace);
									UnicodeCat(uName,uLastName);
								}
								else
									CopyUnicodeString(uName,uFirstName);
							}
							else if(UnicodeLength(uLastName)>0)
								CopyUnicodeString(uName,uLastName);

							if(UnicodeLength(uName)>0)
							{
								CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uName);
								Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Name;
								Pbk->EntriesNum++;
							}
							if(UnicodeLength(uFirstName)>0)
							{
								CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uFirstName);
								Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_FirstName;
								Pbk->EntriesNum++;
							}
							if(UnicodeLength(uLastName) >0)
							{
								CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,uLastName);
								Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_LastName;
								Pbk->EntriesNum++;
							}
						}

                }
                   if (ReadVCALText(Line, "X-MOT-NICKNAME",  Buff) ) 
				   {
						GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                      //  CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                        Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Nickname;
                        Pbk->EntriesNum++;
                    }
				   if (ReadVCALText(Line, "TEL",                   Buff) ||
                        ReadVCALText(Line, "TEL;VOICE",             Buff) ||
                        ReadVCALText(Line, "TEL;PREF",              Buff) ||
                        ReadVCALText(Line, "TEL;PREF;VOICE",        Buff)) {
                            CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
							Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_General;

							Pbk->EntriesNum++;
                    }

					if (ReadVCALText(Line, "TEL;CELL;HOME",  Buff)||
						ReadVCALText(Line, "TEL;HOME;CELL",  Buff)||
						ReadVCALText(Line, "TEL;HOME;VOICE;CELL",  Buff) ||
						ReadVCALText(Line, "TEL;VOICE;HOME;CELL",  Buff) ||
						ReadVCALText(Line, "TEL;CELL;WORK",  Buff) ||
						ReadVCALText(Line, "TEL;CELL",              Buff))
					{
						CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
						Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Mobile_Home;
						Pbk->EntriesNum++;
					}    

					  if (ReadVCALText(Line, "TEL;VOICE;WORK",  Buff) ||
						  ReadVCALText(Line, "TEL;WORK",              Buff) ||
                        ReadVCALText(Line, "TEL;PREF;WORK",         Buff) ||
                        ReadVCALText(Line, "TEL;WORK;VOICE",        Buff) ||
                        ReadVCALText(Line, "TEL;PREF;WORK;VOICE",   Buff)) {
                            CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Work;
                            Pbk->EntriesNum++;
                    }
/*
                    if (ReadVCALText(Line, "TEL;FAX;WORK",               Buff)||
						ReadVCALText(Line, "TEL;WORK;FAX",               Buff))
					{
                            CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Fax_Work;
                            Pbk->EntriesNum++;
                    }*/
                    if (ReadVCALText(Line, "TEL;FAX;HOME",               Buff)||
						ReadVCALText(Line, "TEL;HOME;FAX",               Buff) ||
						
						ReadVCALText(Line, "TEL;FAX",               Buff) ||
                        ReadVCALText(Line, "TEL;PREF;FAX",          Buff) ||
                        ReadVCALText(Line, "TEL;FAX;VOICE",         Buff) ||
						ReadVCALText(Line, "TEL;PREF;FAX;VOICE",    Buff) ||

						ReadVCALText(Line, "TEL;FAX;WORK",               Buff)||
						ReadVCALText(Line, "TEL;WORK;FAX",               Buff))
					{
                            CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Fax_Home;
                            Pbk->EntriesNum++;
                    }

               /*     if (ReadVCALText(Line, "TEL;FAX",               Buff) ||
                        ReadVCALText(Line, "TEL;PREF;FAX",          Buff) ||
                        ReadVCALText(Line, "TEL;FAX;VOICE",         Buff) ||
						ReadVCALText(Line, "TEL;PREF;FAX;VOICE",    Buff) ) {
                            CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Fax;
                            Pbk->EntriesNum++;
                    }*/
                    if (ReadVCALText(Line, "TEL;VOICE;HOME",  Buff)||
						ReadVCALText(Line, "TEL;HOME",              Buff) ||
                        ReadVCALText(Line, "TEL;PREF;HOME",         Buff) ||
                        ReadVCALText(Line, "TEL;HOME;VOICE",        Buff) ||
                        ReadVCALText(Line, "TEL;PREF;HOME;VOICE",   Buff)) {
                            CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Home;
                            Pbk->EntriesNum++;
                    }
                    if (ReadVCALText(Line, "NOTE", Buff)) {
							GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                          //  CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Note;
                            Pbk->EntriesNum++;
                    }
                    if (ReadVCALText(Line, "ADR;HOME", Buff) ) 
					{
                            CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Postal_Home;
                            Pbk->EntriesNum++;
                    }
                    if (ReadVCALText(Line, "ADR;WORK", Buff) ) 
					{
                            CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Postal_Work;
                            Pbk->EntriesNum++;
                    }

                    if (ReadVCALText(Line, "ADR", Buff)||
						ReadVCALText(Line, "ADR;PREF", Buff)) 
					{
                            CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Postal;
                            Pbk->EntriesNum++;
                    }
                    if (ReadVCALText(Line, "ORG", Buff)) {
							char szTemp[2000];
 							sprintf(szTemp,"%s",DecodeUnicodeConsole(Buff));
							if(szTemp[strlen(szTemp)-1] ==';')
							{
								int nLen = UnicodeLength(Buff);
								Buff[(nLen-1)*2] = 0;
								Buff[(nLen-1)*2+1] = 0;

							}

							GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);

                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Company;
                            Pbk->EntriesNum++;


                    }
                     if (ReadVCALText(Line, "TITLE", Buff)) {
							 GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                          //  CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_JobTitle;
                            Pbk->EntriesNum++;
                    }
                  if (ReadVCALText(Line, "EMAIL;INTERNET;HOME", Buff)||
					  ReadVCALText(Line, "EMAIL;HOME;INTERNET", Buff) ||
					  ReadVCALText(Line, "EMAIL;HOME", Buff) ) 
				  {
                         //   CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
							 GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);

                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Email_Home;
                            Pbk->EntriesNum++;
                   }
                  if (ReadVCALText(Line, "EMAIL;INTERNET;WORK", Buff)||
					  ReadVCALText(Line, "EMAIL;WORK;INTERNET", Buff) ||
					  ReadVCALText(Line, "EMAIL;WORK", Buff)) 
				  {
                          //  CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
							  GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);

                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Email_Work;
                            Pbk->EntriesNum++;
                    }

                  if (ReadVCALText(Line, "EMAIL", Buff)||
                        ReadVCALText(Line, "EMAIL;INTERNET;PREF",  Buff) ||
						ReadVCALText(Line, "EMAIL;INTERNET",  Buff)) {
						  GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);

                            //CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Email;
                            Pbk->EntriesNum++;
                    }
                    if (ReadVCALText(Line, "URL;WORK",         Buff)) 
					{
                     //       CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
							GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);

                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_URL_Work;
                            Pbk->EntriesNum++;
                    }
                    if (ReadVCALText(Line, "URL;HOME",         Buff)) 
					{
                          //  CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
							GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);

                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_URL_Home;
                            Pbk->EntriesNum++;
                    }

                    if (ReadVCALText(Line, "URL", Buff))
					{
                          //  CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
							GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);

                            Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_URL;
                            Pbk->EntriesNum++;
                    }
                    if (ReadVCALText(Line, "X-MOT-MANAGER", Buff))
					{
						GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                        Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Manager_Name;
                        Pbk->EntriesNum++;
                    }
                    if (ReadVCALText(Line, "X-MOT-ASSISTANT", Buff))
					{
						GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                        Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Assistant_Name;
                        Pbk->EntriesNum++;
                    }
                    if (ReadVCALText(Line, "X-MOT-SPOUSE", Buff))
					{
						GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                        Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Spouse_Name;
                        Pbk->EntriesNum++;
                    }
                    if (ReadVCALText(Line, "X-MOT-CHILD", Buff))
					{
						GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                        Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Children;
                        Pbk->EntriesNum++;
                    }
                    if (ReadVCALText(Line, "X-MOT-IM", Buff))
					{
						GSM_SemicolonDecode(Buff,Pbk->Entries[Pbk->EntriesNum].Text);
                        Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_IMID;
                        Pbk->EntriesNum++;
                    }
					if (ReadVCALText(Line, "BDAY", Buff))
					{
						char 	*pTemp = strchr((char*)Line,':' );
						if(pTemp)
						{
							YYYYMMDD_Get_DateTime(&Pbk->Entries[Pbk->EntriesNum].Date,(char*) pTemp+1);
							Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Date;
							Pbk->EntriesNum++;
						}
                    }
					if (ReadVCALText(Line, "X-MOT-ANNIVERSARY", Buff))
					{
						char 	*pTemp = strchr((char*)Line,':' );
						if(pTemp)
						{
							YYYYMMDD_Get_DateTime(&Pbk->Entries[Pbk->EntriesNum].Date,(char*) pTemp+1);
							Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Date_Anniversary;
							Pbk->EntriesNum++;
						}
                    }
					if (ReadVCALText(Line, "X-MOT-CAT", Buff))
					{
						CopyUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text,Buff);
                        Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Caller_Group_Text;
                        Pbk->EntriesNum++;
                    }
                    break;
            }
    }

    if (Pbk->EntriesNum == 0) return ERR_EMPTY;
    return ERR_NONE;
}
void MOTOE2_EncodeVCARD(char *Buffer, int *Length, GSM_MemoryEntry *pbk, bool header)
{
    int     Name, Number, Group, i,LastName ,FirstName;
	bool    ignore;
//	char szName[2000];
	unsigned char szFullName[2000];
//	char szLastName[2000];
//	char szFirstName[2000];
	unsigned char szoutput[2000];
//	char szLastName[2000];
//	char szFirstName[2000];
	GSM_PhonebookFindDefaultNameNumberGroupSE(pbk, &Name, &Number, &Group,&LastName ,&FirstName);

    if (header) *Length+=sprintf(Buffer+(*Length),"BEGIN:VCARD%c%cVERSION:2.1%c%c",13,10,13,10);
   if (Name != -1)
	{
/* 			sprintf(szName,"%s",DecodeUnicodeConsole(pbk->Entries[Name].Text));
			CString strName = szName;
			strName.Replace(";","\\;");
			sprintf(szName,strName);

			unsigned char	   	Text[(GSM_PHONEBOOK_TEXT_LENGTH+1)*2];
			EncodeUnicode(Text,(unsigned char *)szName,strlen(szName)+1);*/
	   		unsigned char	   	Text[(GSM_PHONEBOOK_TEXT_LENGTH+1)*2];
			CopyUnicodeString(Text,pbk->Entries[Name].Text);
			UnicodeReplace(Text,";","\\;");

			SESaveVCARDText(Buffer, Length, Text, "N", ":");
    }
	else
	{
/*		szLastName[0]='\0';
		szFirstName[0]='\0';
		if(LastName != -1)
		{
			sprintf(szLastName,"%s",DecodeUnicodeString(pbk->Entries[LastName].Text));
			CString strLastName = szLastName;
			strLastName.Replace(";","\\;");
			sprintf(szLastName,strLastName);
		}
		if(FirstName != -1)
		{
			sprintf(szFirstName,"%s",DecodeUnicodeString(pbk->Entries[FirstName].Text));
		
			CString strFirstName = szFirstName;
			strFirstName.Replace(";","\\;");
			sprintf(szFirstName,strFirstName);

		}
		sprintf(szName,"%s;%s",szLastName,szFirstName);
		EncodeUnicode((unsigned char *)szFullName,(unsigned char *)szName,strlen(szName)+1);
		SESaveVCARDText(Buffer, Length, (unsigned char *)szFullName, "N", ":");

		sprintf(szName,"%s %s",szFirstName,szLastName);
		CString strFullname = szName;
		strFullname.Replace("\\;",";");
		sprintf(szName,strFullname);

		EncodeUnicode(pbk->Entries[pbk->EntriesNum].Text,(unsigned char *)szName,strlen(szName)+1);
		pbk->Entries[pbk->EntriesNum].EntryType = PBK_Text_Name;
		pbk->EntriesNum++;*/
		//for _UNICODE
		unsigned char uLastName[4000];
		unsigned char uFirstName[4000];
		unsigned char uTemp[10];
		unsigned char uTemp2[10];
		uLastName[0] =uLastName[1] =0;
		uFirstName[0] =uFirstName[1] =0;

		EncodeUnicode(uTemp,(unsigned char *)";",2);
		EncodeUnicode(uTemp2,(unsigned char *)" ",2);

		if(LastName != -1)
		{
			CopyUnicodeString(uLastName,pbk->Entries[LastName].Text);
			UnicodeReplace(uLastName,";","\\;");
		}
		if(FirstName != -1)
		{
			CopyUnicodeString(uFirstName,pbk->Entries[FirstName].Text);
			UnicodeReplace(uFirstName,";","\\;");
		}
		CopyUnicodeString(szFullName,uLastName);
		UnicodeCat(szFullName,uTemp);
		UnicodeCat(szFullName,uFirstName);

		SESaveVCARDText(Buffer, Length, (unsigned char *)szFullName, "N", ":");

		if(FirstName != -1)
		{
			CopyUnicodeString(pbk->Entries[pbk->EntriesNum].Text,pbk->Entries[FirstName].Text);
			UnicodeCat(pbk->Entries[pbk->EntriesNum].Text,uTemp2);
			if(LastName != -1)
				UnicodeCat(pbk->Entries[pbk->EntriesNum].Text,pbk->Entries[LastName].Text);
			pbk->Entries[pbk->EntriesNum].EntryType = PBK_Text_Name;
			pbk->EntriesNum++;
		}
		else if(LastName != -1)
		{
			CopyUnicodeString(pbk->Entries[pbk->EntriesNum].Text,pbk->Entries[LastName].Text);
			pbk->Entries[pbk->EntriesNum].EntryType = PBK_Text_Name;
			pbk->EntriesNum++;
		}


	}
    for (i=0; i < pbk->EntriesNum; i++) 
	{
            if (i != Name && i != LastName&& i != FirstName) 
			{
                    ignore = false;
                    switch(pbk->Entries[i].EntryType) {
					case PBK_Text_Name      :
                    case PBK_Caller_Group   :
                            ignore = true;
                            break;
                    case PBK_Date       :
                            *Length+=sprintf(Buffer+(*Length),"BDAY:%04d-%02d-%02d%c%c",pbk->Entries[i].Date.Year,pbk->Entries[i].Date.Month,pbk->Entries[i].Date.Day,13,10);
							 ignore = true;
							break;
                   case PBK_Date_Anniversary       :
                            *Length+=sprintf(Buffer+(*Length),"X-MOT-ANNIVERSARY:%04d-%02d-%02d%c%c",pbk->Entries[i].Date.Year,pbk->Entries[i].Date.Month,pbk->Entries[i].Date.Day,13,10);
							 ignore = true;
							break;
                    case PBK_Number_General :
                            *Length+=sprintf(Buffer+(*Length),"TEL;VOICE");
                            break;
                   case PBK_Number_Work    :
                            *Length+=sprintf(Buffer+(*Length),"TEL;VOICE;WORK");
                            break;
                    case PBK_Number_Home    :
                            *Length+=sprintf(Buffer+(*Length),"TEL;VOICE;HOME");
                            break;
                    case PBK_Number_Mobile  :
                            *Length+=sprintf(Buffer+(*Length),"TEL;CELL");
                             break;
                     case PBK_Number_Mobile_Home  :
                            *Length+=sprintf(Buffer+(*Length),"TEL;CELL;HOME");
                             break;
                     case PBK_Number_Mobile_Work  :
                            *Length+=sprintf(Buffer+(*Length),"TEL;CELL;WORK");
                             break;
                    case PBK_Number_Fax     :
                            *Length+=sprintf(Buffer+(*Length),"TEL;FAX");
                            break;
                    case PBK_Number_Fax_Home     :
                            *Length+=sprintf(Buffer+(*Length),"TEL;FAX;HOME");
                            break;
                    case PBK_Number_Fax_Work     :
                            *Length+=sprintf(Buffer+(*Length),"TEL;FAX;WORK");
                            break;
                    case PBK_Number_Pager     :
                            *Length+=sprintf(Buffer+(*Length),"TEL;PAGER");
                            break;
                    case PBK_Number_Pager_Home     :
                            *Length+=sprintf(Buffer+(*Length),"TEL;HOME;PAGER");
                            break;
                    case PBK_Number_Pager_Work     :
                            *Length+=sprintf(Buffer+(*Length),"TEL;PAGER;WORK");
                            break;
                    case PBK_Number_VideoCall     :
                            *Length+=sprintf(Buffer+(*Length),"TEL;VIDEO");
                            break;
                    case PBK_Number_VideoCall_Home     :
                            *Length+=sprintf(Buffer+(*Length),"TEL;VIDEO;HOME");
                            break;
                    case PBK_Number_VideoCall_Work     :
                            *Length+=sprintf(Buffer+(*Length),"TEL;VIDEO;WORK");
                            break;
                  case PBK_Text_Nickname:
							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "X-MOT-NICKNAME", ":");
							ignore = true;
                            break;
                  case PBK_Text_Manager_Name:
							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "X-MOT-MANAGER", ":");
							ignore = true;
                            break;
                  case PBK_Text_Assistant_Name:
							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "X-MOT-ASSISTANT", ":");
							ignore = true;
                            break;
                  case PBK_Text_Spouse_Name:
							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "X-MOT-SPOUSE", ":");
							ignore = true;
                            break;
                  case PBK_Text_Children:
							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "X-MOT-CHILD", ":");
							ignore = true;
                            break;
                  case PBK_Text_IMID:
							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "X-MOT-IM", ":");
							ignore = true;
                            break;
                  case PBK_Caller_Group_Text:
							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "X-MOT-CAT", ":");
							ignore = true;
                            break;
                  case PBK_Text_Company:
                             *Length+=sprintf(Buffer+(*Length),"ORG");
							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "",":");
							ignore = true;

                            break;
                    case PBK_Text_JobTitle:
                           *Length+=sprintf(Buffer+(*Length),"TITLE");
 							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "",":");
							ignore = true;
                           break;
 					case PBK_Text_Note      :
                            *Length+=sprintf(Buffer+(*Length),"NOTE");
							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "",":");
							ignore = true;
                            break;
                    case PBK_Text_Postal    :
                   //         *Length+=sprintf(Buffer+(*Length),"ADR;PREF");
                            *Length+=sprintf(Buffer+(*Length),"ADR");
 							SiemensSaveVCARDText(Buffer, Length, (char *)pbk->Entries[i].Text, "",";");
							ignore = true;
                          break;
                    case PBK_Text_Postal_Home    :
                            *Length+=sprintf(Buffer+(*Length),"ADR;HOME");
							SiemensSaveVCARDText(Buffer, Length, (char *)pbk->Entries[i].Text, "",";");
							ignore = true;
                            break;
                    case PBK_Text_Postal_Work    :
                            *Length+=sprintf(Buffer+(*Length),"ADR;WORK");
 							SiemensSaveVCARDText(Buffer, Length, (char *)pbk->Entries[i].Text, "",";");
							ignore = true;
                           break;
                    case PBK_Text_Email     :
                            *Length+=sprintf(Buffer+(*Length),"EMAIL");
							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "",":");
							ignore = true;
                            break;
                    case PBK_Text_Email_Home     :
                            *Length+=sprintf(Buffer+(*Length),"EMAIL;HOME");
							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "",":");
							ignore = true;
                            break;
                     case PBK_Text_Email_Work     :
                            *Length+=sprintf(Buffer+(*Length),"EMAIL;WORK");
 							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "",":");
							ignore = true;
                           break;
                   case PBK_Text_URL       :
                            *Length+=sprintf(Buffer+(*Length),"URL");
 							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "",":");
							ignore = true;
                           break;
                   case PBK_Text_URL_Home       :
                            *Length+=sprintf(Buffer+(*Length),"URL;HOME");
  							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "",":");
							ignore = true;
                          break;
                   case PBK_Text_URL_Work       :
                            *Length+=sprintf(Buffer+(*Length),"URL;WORK");
 							GSM_SemicolonEncode(pbk->Entries[i].Text,szoutput );
							SESaveVCARDText(Buffer, Length, szoutput, "",":");
							ignore = true;
                           break;
                   default :
                            ignore = true;
                            break;
                    }
                    if (!ignore) {
                            SESaveVCARDText(Buffer, Length, pbk->Entries[i].Text, "",":");
                    }
            }
    }
    if (header) *Length+=sprintf(Buffer+(*Length),"END:VCARD%c%c",13,10);
}
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

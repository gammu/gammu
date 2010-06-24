/* (c) 2001-2005 by Marcin Wiacek, Michal Cihar... */





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











#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

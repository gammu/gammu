/* (c) 2002-2004 by Marcin Wiacek, Walek and Michal Cihar */

#include <string.h>
#include <ctype.h>

#include "../../phone/nokia/nfunc.h"
#include "../../phone/nokia/dct3/n7110.h"
#include "../../misc/cfg.h"
#include "../../misc/coding/coding.h"
#include "../../misc/coding/md5.h"
#include "../gsmlogo.h"
#include "../gsmmisc.h"
#include "backtext.h"

#ifdef GSM_ENABLE_BACKUP

GSM_Error FindBackupChecksum(char *FileName, bool UseUnicode, char *checksum)
{
	INI_Section		*file_info, *h;
	INI_Entry		*e;
	char			*buffer = NULL,buff[100];
	int			len=0;

	//int i;

	file_info = INI_ReadFile(FileName, UseUnicode);

	if (UseUnicode) {
	        for (h = file_info; h != NULL; h = h->Next) {
			EncodeUnicode(buff,"Checksum",8);
			if (mywstrncasecmp(buff, h->SectionName, 8)) continue;

			buffer = (unsigned char *)realloc(buffer,len+UnicodeLength(h->SectionName)*2+2);
			CopyUnicodeString(buffer+len,h->SectionName);
			len+=UnicodeLength(h->SectionName)*2;
//			dbgprintf("[%s]\n",DecodeUnicodeConsole(h->SectionName));

		        for (e = h->SubEntries; e != NULL; e = e->Next) {
				buffer = (unsigned char *)realloc(buffer,len+UnicodeLength(e->EntryName)*2+2);
				CopyUnicodeString(buffer+len,e->EntryName);
				len+=UnicodeLength(e->EntryName)*2;
				buffer = (unsigned char *)realloc(buffer,len+UnicodeLength(e->EntryValue)*2+2);
				CopyUnicodeString(buffer+len,e->EntryValue);
				len+=UnicodeLength(e->EntryValue)*2;
//				dbgprintf("\"%s\"",DecodeUnicodeConsole(e->EntryName));
//				dbgprintf("=\"%s\"\n",DecodeUnicodeConsole(e->EntryValue));
			}
		}
	} else {
	        for (h = file_info; h != NULL; h = h->Next) {
	            	if (mystrncasecmp("Checksum", h->SectionName, 8)) continue;

			buffer = (unsigned char *)realloc(buffer,len+strlen(h->SectionName)+1);
			strcpy(buffer+len,h->SectionName);
			len+=strlen(h->SectionName);

		        for (e = h->SubEntries; e != NULL; e = e->Next) {
//				dbgprintf("%s=%s\n",e->EntryName,e->EntryValue);
				buffer = (unsigned char *)realloc(buffer,len+strlen(e->EntryName)+1);
				strcpy(buffer+len,e->EntryName);
				len+=strlen(e->EntryName);
				buffer = (unsigned char *)realloc(buffer,len+strlen(e->EntryValue)+1);
				strcpy(buffer+len,e->EntryValue);
				len+=strlen(e->EntryValue);
			}
		}
	}

	//for (i=0;i<len;i++) printf("%02x",buffer[i]);
	CalculateMD5(buffer, len, checksum);
	free(buffer);

	return ERR_NONE;
}

static unsigned char *ReadCFGText(INI_Section *cfg, unsigned char *section, unsigned char *key, bool Unicode)
{
	unsigned char Buffer[500],Buffer2[500],*retval;

	if (Unicode) {
		EncodeUnicode(Buffer2,key,strlen(key));
		retval = INI_GetValue(cfg,section,Buffer2,Unicode);
		if (retval != NULL) return DecodeUnicodeString(retval);
		return NULL;
	} else {
		strcpy(Buffer,section);
		strcpy(Buffer2,key);
		return INI_GetValue(cfg,section,key,Unicode);
	}
}

static void SaveLinkedBackupText(FILE *file, char *myname, char *myvalue, bool UseUnicode)
{
	int 		w,current;
	unsigned char 	buffer2[1000],buffer3[1000];

	current = strlen(myvalue); w = 0;
	while (true) {
		if (current > 200) {
			memcpy(buffer2,myvalue+(strlen(myvalue)-current),200);
			buffer2[200] = 0;
			current = current - 200;
		} else {
			memcpy(buffer2,myvalue+(strlen(myvalue)-current),current);
			buffer2[current] = 0;
			current = 0;
		}
		if (UseUnicode) {
			sprintf(buffer3,"%s%02i = %s%c%c",myname,w,buffer2,13,10);
			EncodeUnicode(buffer2,buffer3,strlen(buffer3));
			fwrite(buffer2,1,strlen(buffer3)*2,file);
		} else {
			fprintf(file,"%s%02i = %s%c%c",myname,w,buffer2,13,10);
		}
		if (current == 0) break;
		w++;
	}		
}

static void ReadLinkedBackupText(INI_Section *file_info, char *section, char *myname, char *myvalue, bool UseUnicode)
{
	unsigned char		buffer2[300];
	char			*readvalue;
	int			i;

	i=0;
	myvalue[0] = 0;
	while (true) {
		sprintf(buffer2,"%s%02i",myname,i);
		readvalue = ReadCFGText(file_info, section, buffer2, UseUnicode);
		if (readvalue!=NULL) {
			myvalue[strlen(myvalue)+strlen(readvalue)]=0;
			memcpy(myvalue+strlen(myvalue),readvalue,strlen(readvalue));
		} else break;
		i++;
	}
}

static void SaveBackupText(FILE *file, char *myname, char *myvalue, bool UseUnicode)
{
	unsigned char buffer[10000], buffer2[10000];

	if (myname[0] == 0x00) {
		if (UseUnicode) {
			EncodeUnicode(buffer,myvalue,strlen(myvalue));
			fwrite(buffer,1,strlen(myvalue)*2,file);
		} else fprintf(file,"%s",myvalue);
	} else {
		if (UseUnicode) {
			sprintf(buffer,"%s = \"",myname);
			EncodeUnicode(buffer2,buffer,strlen(buffer));
			fwrite(buffer2,1,strlen(buffer)*2,file);

			fwrite(EncodeUnicodeSpecialChars(myvalue),1,UnicodeLength(EncodeUnicodeSpecialChars(myvalue))*2,file);

			sprintf(buffer,"\"%c%c",13,10);
			EncodeUnicode(buffer2,buffer,strlen(buffer));
			fwrite(buffer2,1,strlen(buffer)*2,file);
		} else {
			sprintf(buffer,"%s = \"%s\"%c%c",myname,EncodeSpecialChars(DecodeUnicodeString(myvalue)),13,10);
			fprintf(file,"%s",buffer);

			EncodeHexBin(buffer,myvalue,UnicodeLength(myvalue)*2);
			fprintf(file,"%sUnicode = %s%c%c",myname,buffer,13,10);
		}
	}
}

static bool ReadBackupText(INI_Section *file_info, char *section, char *myname, char *myvalue, bool UseUnicode)
{
	unsigned char paramname[10000],*readvalue;

	if (UseUnicode) {
		EncodeUnicode(paramname,myname,strlen(myname));
		readvalue = INI_GetValue(file_info, section, paramname, UseUnicode);
		if (readvalue!=NULL) {
			CopyUnicodeString(myvalue,DecodeUnicodeSpecialChars(readvalue+2));
			myvalue[UnicodeLength(myvalue)*2-2]=0;
			myvalue[UnicodeLength(myvalue)*2-1]=0;

			dbgprintf("%s\n",DecodeUnicodeString(readvalue));
		} else {
			myvalue[0]=0;
			myvalue[1]=0;
			return false;
		}
	} else {
		strcpy(paramname,myname);
		strcat(paramname,"Unicode");
		readvalue = ReadCFGText(file_info, section, paramname, UseUnicode);
		if (readvalue!=NULL) {
			dbgprintf("%s %i\n",readvalue,strlen(readvalue));
			DecodeHexBin (myvalue, readvalue, strlen(readvalue));
			myvalue[strlen(readvalue)/2]=0;
			myvalue[strlen(readvalue)/2+1]=0;
			dbgprintf("%s\n",DecodeUnicodeString(myvalue));
		} else {
			strcpy(paramname,myname);
			readvalue = ReadCFGText(file_info, section, paramname, UseUnicode);
			if (readvalue!=NULL) {
				EncodeUnicode(myvalue,DecodeSpecialChars(readvalue+1),strlen(DecodeSpecialChars(readvalue+1))-1);
			} else {
				myvalue[0]=0;
				myvalue[1]=0;
				return false;
			}
		}
	}
	return true;
}

static void SaveVCalDateTime(FILE *file, GSM_DateTime *dt, bool UseUnicode)
{
	unsigned char 	buffer[100];
	int		Length = 3;

	sprintf(buffer, " = ");
	SaveVCALDateTime(buffer, &Length, dt, NULL);
	SaveBackupText(file, "", buffer, UseUnicode);
}

static void SaveVCalDate(FILE *file, GSM_DateTime *dt, bool UseUnicode)
{
	unsigned char buffer[100];

	sprintf(buffer, " = %04d%02d%02d%c%c", dt->Year, dt->Month, dt->Day,13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
}

/* ---------------------- backup files ------------------------------------- */

static void SavePbkEntry(FILE *file, GSM_MemoryEntry *Pbk, bool UseUnicode)
{
	bool	text;
	char	buffer[1000];
	int	j, i;

	sprintf(buffer,"Location = %03i%c%c",Pbk->Location,13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
	for (j=0;j<Pbk->EntriesNum;j++) {
		text = true;
		switch (Pbk->Entries[j].EntryType) {
			case PBK_Number_General:
				sprintf(buffer,"Entry%02iType = NumberGeneral%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Number_Mobile:
				sprintf(buffer,"Entry%02iType = NumberMobile%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Number_Work:
				sprintf(buffer,"Entry%02iType = NumberWork%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Number_Fax:
				sprintf(buffer,"Entry%02iType = NumberFax%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Number_Home:
				sprintf(buffer,"Entry%02iType = NumberHome%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Number_Pager:
				sprintf(buffer,"Entry%02iType = NumberPager%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Number_Other:
				sprintf(buffer,"Entry%02iType = NumberOther%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Text_Note:
				sprintf(buffer,"Entry%02iType = Note%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Text_Postal:
				sprintf(buffer,"Entry%02iType = Postal%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Text_Email:
				sprintf(buffer,"Entry%02iType = Email%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Text_Email2:
				sprintf(buffer,"Entry%02iType = Email2%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Text_URL:
				sprintf(buffer,"Entry%02iType = URL%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Text_Name:
				sprintf(buffer,"Entry%02iType = Name%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Caller_Group:
				sprintf(buffer,"Entry%02iType = CallerGroup%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				sprintf(buffer,"Entry%02iNumber = %i%c%c",j,Pbk->Entries[j].Number,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				text = false;
				break;
			case PBK_RingtoneID:
				sprintf(buffer,"Entry%02iType = RingtoneID%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				sprintf(buffer,"Entry%02iNumber = %i%c%c",j,Pbk->Entries[j].Number,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				text = false;
				break;
			case PBK_PictureID:
				sprintf(buffer,"Entry%02iType = PictureID%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				sprintf(buffer,"Entry%02iNumber = %i%c%c",j,Pbk->Entries[j].Number,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				text = false;
				break;
			case PBK_Text_UserID:
				sprintf(buffer,"Entry%02iType = UserID%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Category:
				sprintf(buffer,"Entry%02iType = Category%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				sprintf(buffer,"Entry%02iNumber = %i%c%c",j,Pbk->Entries[j].Number,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				text = false;
				break;
			case PBK_Private:
				sprintf(buffer,"Entry%02iType = Private%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				sprintf(buffer,"Entry%02iNumber = %i%c%c",j,Pbk->Entries[j].Number,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				text = false;
				break;
			case PBK_Text_LastName:
				sprintf(buffer,"Entry%02iType = LastName%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Text_FirstName:
				sprintf(buffer,"Entry%02iType = FirstName%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Text_Company:
				sprintf(buffer,"Entry%02iType = Company%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Text_JobTitle:
				sprintf(buffer,"Entry%02iType = JobTitle%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Text_StreetAddress:
				sprintf(buffer,"Entry%02iType = Address%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Text_City:
				sprintf(buffer,"Entry%02iType = City%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Text_State:
				sprintf(buffer,"Entry%02iType = State%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Text_Zip:
				sprintf(buffer,"Entry%02iType = Zip%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Text_Country:
				sprintf(buffer,"Entry%02iType = Country%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Text_Custom1:
				sprintf(buffer,"Entry%02iType = Custom1%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Text_Custom2:
				sprintf(buffer,"Entry%02iType = Custom2%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Text_Custom3:
				sprintf(buffer,"Entry%02iType = Custom3%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_Text_Custom4:
				sprintf(buffer,"Entry%02iType = Custom4%c%c",j,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				break;
			case PBK_SMSListID:
			case PBK_RingtoneFileSystemID:
			case PBK_Date:
				break;
        	}
		if (text) {
			sprintf(buffer,"Entry%02iText",j);
			SaveBackupText(file,buffer,Pbk->Entries[j].Text, UseUnicode);
		}
		switch (Pbk->Entries[j].EntryType) {
			case PBK_Number_General:
			case PBK_Number_Mobile:
			case PBK_Number_Work:
			case PBK_Number_Fax:
			case PBK_Number_Home:
			case PBK_Number_Other:
			case PBK_Number_Pager:
				if (Pbk->Entries[j].VoiceTag!=0) {
					sprintf(buffer,"Entry%02iVoiceTag = %i%c%c",j,Pbk->Entries[j].VoiceTag,13,10);
					SaveBackupText(file, "", buffer, UseUnicode);
				}
				i = 0;
				while (Pbk->Entries[j].SMSList[i]!=0) {
					sprintf(buffer,"Entry%02iSMSList%02i = %i%c%c",j,i,Pbk->Entries[j].SMSList[i],13,10);
					SaveBackupText(file, "", buffer, UseUnicode);
					i++;
				}
				break;
			default:
				break;
		}
	}
	sprintf(buffer,"%c%c",13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
}

static void SaveNoteEntry(FILE *file, GSM_NoteEntry *Note, bool UseUnicode)
{
	char buffer[1000];

	sprintf(buffer,"Location = %d%c%c", Note->Location,13,10);
	SaveBackupText(file, "", buffer, UseUnicode);	
	SaveBackupText(file, "Text", Note->Text, UseUnicode);
	sprintf(buffer, "%c%c",13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
}

static void SaveCalendarEntry(FILE *file, GSM_CalendarEntry *Note, bool UseUnicode)
{
	int 	i;
	char	buffer[1000];

	sprintf(buffer,"Location = %d%c%c", Note->Location,13,10);
	SaveBackupText(file, "", buffer, UseUnicode);	
	SaveBackupText(file, "", "Type = ", UseUnicode);
	switch (Note->Type) {
		case GSM_CAL_REMINDER 	: sprintf(buffer,"Reminder%c%c", 		13,10); break;
		case GSM_CAL_CALL     	: sprintf(buffer,"Call%c%c", 			13,10); break;
		case GSM_CAL_MEETING  	: sprintf(buffer,"Meeting%c%c", 		13,10); break;
		case GSM_CAL_BIRTHDAY 	: sprintf(buffer,"Birthday%c%c", 		13,10); break;
		case GSM_CAL_TRAVEL  	: sprintf(buffer,"Travel%c%c", 			13,10); break;
		case GSM_CAL_VACATION 	: sprintf(buffer,"Vacation%c%c", 		13,10); break;
		case GSM_CAL_MEMO	: sprintf(buffer,"Memo%c%c", 			13,10); break;
		case GSM_CAL_ALARM    	: sprintf(buffer,"Alarm%c%c", 			13,10); break;
		case GSM_CAL_DAILY_ALARM: sprintf(buffer,"DailyAlarm%c%c", 		13,10); break;
		case GSM_CAL_T_ATHL   	: sprintf(buffer,"Training/Athletism%c%c", 	13,10); break;
       		case GSM_CAL_T_BALL   	: sprintf(buffer,"Training/BallGames%c%c", 	13,10); break;
                case GSM_CAL_T_CYCL   	: sprintf(buffer,"Training/Cycling%c%c", 	13,10); break;
                case GSM_CAL_T_BUDO   	: sprintf(buffer,"Training/Budo%c%c", 		13,10); break;
                case GSM_CAL_T_DANC   	: sprintf(buffer,"Training/Dance%c%c", 		13,10); break;
                case GSM_CAL_T_EXTR   	: sprintf(buffer,"Training/ExtremeSports%c%c", 	13,10); break;
                case GSM_CAL_T_FOOT   	: sprintf(buffer,"Training/Football%c%c", 	13,10); break;
                case GSM_CAL_T_GOLF   	: sprintf(buffer,"Training/Golf%c%c", 		13,10); break;
                case GSM_CAL_T_GYM    	: sprintf(buffer,"Training/Gym%c%c", 		13,10); break;
                case GSM_CAL_T_HORS   	: sprintf(buffer,"Training/HorseRaces%c%c", 	13,10); break;
                case GSM_CAL_T_HOCK   	: sprintf(buffer,"Training/Hockey%c%c", 	13,10); break;
                case GSM_CAL_T_RACE   	: sprintf(buffer,"Training/Races%c%c", 		13,10); break;
                case GSM_CAL_T_RUGB   	: sprintf(buffer,"Training/Rugby%c%c", 		13,10); break;
                case GSM_CAL_T_SAIL   	: sprintf(buffer,"Training/Sailing%c%c", 	13,10); break;
                case GSM_CAL_T_STRE   	: sprintf(buffer,"Training/StreetGames%c%c",	13,10); break;
                case GSM_CAL_T_SWIM   	: sprintf(buffer,"Training/Swimming%c%c", 	13,10); break;
                case GSM_CAL_T_TENN   	: sprintf(buffer,"Training/Tennis%c%c", 	13,10); break;
                case GSM_CAL_T_TRAV   	: sprintf(buffer,"Training/Travels%c%c", 	13,10); break;
                case GSM_CAL_T_WINT   	: sprintf(buffer,"Training/WinterGames%c%c", 	13,10); break;
	}
	SaveBackupText(file, "", buffer, UseUnicode);	
	for (i=0;i<Note->EntriesNum;i++) {
		switch (Note->Entries[i].EntryType) {
		case CAL_START_DATETIME:
			SaveBackupText(file, "", "StartTime", UseUnicode);
			SaveVCalDateTime(file, &Note->Entries[i].Date, UseUnicode);
			break;
		case CAL_END_DATETIME:
			SaveBackupText(file, "", "StopTime", UseUnicode);
			SaveVCalDateTime(file, &Note->Entries[i].Date, UseUnicode);
			break;
		case CAL_ALARM_DATETIME:
			SaveBackupText(file, "", "Alarm", UseUnicode);
			SaveVCalDateTime(file, &Note->Entries[i].Date, UseUnicode);
			sprintf(buffer,"AlarmType = Tone%c%c",13,10);
			SaveBackupText(file, "", buffer, UseUnicode);
			break;
		case CAL_SILENT_ALARM_DATETIME:
			SaveBackupText(file, "", "Alarm", UseUnicode);
			SaveVCalDateTime(file, &Note->Entries[i].Date, UseUnicode);
			sprintf(buffer,"AlarmType = Silent%c%c",13,10);
			SaveBackupText(file, "", buffer, UseUnicode);
			break;
		case CAL_PRIVATE:
			sprintf(buffer, "Private = %d%c%c",Note->Entries[i].Number,13,10);
			SaveBackupText(file, "", buffer, UseUnicode);
			break;
		case CAL_LOCATION:
			SaveBackupText(file, "EventLocation", Note->Entries[i].Text, UseUnicode);
			break;
		case CAL_CONTACTID:
			sprintf(buffer, "ContactID = %d%c%c",Note->Entries[i].Number,13,10);
			SaveBackupText(file, "", buffer, UseUnicode);
			break;
		case CAL_RECURRANCE:
			sprintf(buffer, "Recurrance = %d%c%c",Note->Entries[i].Number/24,13,10);
			SaveBackupText(file, "", buffer, UseUnicode);
			break;
		case CAL_TEXT:
			SaveBackupText(file, "Text", Note->Entries[i].Text, UseUnicode);
			break;
		case CAL_PHONE:
			SaveBackupText(file, "Phone", Note->Entries[i].Text, UseUnicode);
			break;               
		case CAL_REPEAT_STOPDATE:
			SaveBackupText(file, "", "RepeatStopDate", UseUnicode);
			SaveVCalDate(file, &Note->Entries[i].Date, UseUnicode);
			break;
		case CAL_REPEAT_STARTDATE:
			SaveBackupText(file, "", "RepeatStartDate", UseUnicode);
			SaveVCalDate(file, &Note->Entries[i].Date, UseUnicode);
			break;
		case CAL_REPEAT_DAYOFWEEK:
			sprintf(buffer, "RepeatDayOfWeek = %d%c%c",Note->Entries[i].Number,13,10);
			SaveBackupText(file, "", buffer, UseUnicode);
			break;
		case CAL_REPEAT_DAY:
			sprintf(buffer, "RepeatDay = %d%c%c",Note->Entries[i].Number,13,10);
			SaveBackupText(file, "", buffer, UseUnicode);
			break;
		case CAL_REPEAT_WEEKOFMONTH:
			sprintf(buffer, "RepeatWeekOfMonth = %d%c%c",Note->Entries[i].Number,13,10);
			SaveBackupText(file, "", buffer, UseUnicode);
			break;
		case CAL_REPEAT_MONTH:
			sprintf(buffer, "RepeatMonth = %d%c%c",Note->Entries[i].Number,13,10);
			SaveBackupText(file, "", buffer, UseUnicode);
			break;
		case CAL_REPEAT_FREQUENCY:
			sprintf(buffer, "RepeatFrequency = %d%c%c",Note->Entries[i].Number,13,10);
			SaveBackupText(file, "", buffer, UseUnicode);
			break;
		}
	}
	sprintf(buffer, "%c%c",13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
}

static void SaveWAPSettingsEntry(FILE *file, GSM_MultiWAPSettings *settings, bool UseUnicode)
{
	int 	i;
	char 	buffer[10000];

	if (settings->Active) {
		sprintf(buffer,"Active = Yes%c%c",13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
	}
	switch (settings->ActiveBearer) {
		case WAPSETTINGS_BEARER_SMS : sprintf(buffer,"Bearer = SMS%c%c",13,10);  break;
		case WAPSETTINGS_BEARER_GPRS: sprintf(buffer,"Bearer = GPRS%c%c",13,10); break;
		case WAPSETTINGS_BEARER_DATA: sprintf(buffer,"Bearer = Data%c%c",13,10); break;
		case WAPSETTINGS_BEARER_USSD: sprintf(buffer,"Bearer = USSD%c%c",13,10);
	}	
	SaveBackupText(file, "", buffer, UseUnicode);
	if (settings->ReadOnly) {
		sprintf(buffer,"ReadOnly = Yes%c%c",13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
	}
	sprintf(buffer,"Proxy");
	SaveBackupText(file, buffer, settings->Proxy, UseUnicode);
	sprintf(buffer,"ProxyPort = %i%c%c",settings->ProxyPort,13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
	sprintf(buffer,"Proxy2");
	SaveBackupText(file, buffer, settings->Proxy2, UseUnicode);
	sprintf(buffer,"Proxy2Port = %i%c%c",settings->Proxy2Port,13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
	sprintf(buffer,"%c%c",13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
	for (i=0;i<settings->Number;i++) {
		sprintf(buffer,"Title%02i",i);
		SaveBackupText(file, buffer, settings->Settings[i].Title, UseUnicode);
		sprintf(buffer,"HomePage%02i",i);
		SaveBackupText(file, buffer, settings->Settings[i].HomePage, UseUnicode);
		if (settings->Settings[i].IsContinuous) {
			sprintf(buffer,"Type%02i = Continuous%c%c",i,13,10);
		} else {
			sprintf(buffer,"Type%02i = Temporary%c%c",i,13,10);
		}
		SaveBackupText(file, "", buffer, UseUnicode);
		if (settings->Settings[i].IsSecurity) {
			sprintf(buffer,"Security%02i = On%c%c",i,13,10);
		} else {
			sprintf(buffer,"Security%02i = Off%c%c",i,13,10);
		}
		SaveBackupText(file, "", buffer, UseUnicode);
		switch (settings->Settings[i].Bearer) {
			case WAPSETTINGS_BEARER_SMS:
				sprintf(buffer,"Bearer%02i = SMS%c%c",i,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				sprintf(buffer,"Server%02i",i);
				SaveBackupText(file, buffer, settings->Settings[i].Server, UseUnicode);
				sprintf(buffer,"Service%02i",i);
				SaveBackupText(file, buffer, settings->Settings[i].Service, UseUnicode);
				break;
			case WAPSETTINGS_BEARER_GPRS:
				sprintf(buffer,"Bearer%02i = GPRS%c%c",i,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				sprintf(buffer,"IP%02i",i);
				SaveBackupText(file, buffer, settings->Settings[i].IPAddress, UseUnicode);
			case WAPSETTINGS_BEARER_DATA:
				if (settings->Settings[i].Bearer == WAPSETTINGS_BEARER_DATA) {
					sprintf(buffer,"Bearer%02i = Data%c%c",i,13,10);
					SaveBackupText(file, "", buffer, UseUnicode);
					if (settings->Settings[i].IsISDNCall) {
						sprintf(buffer,"CallType%02i = ISDN%c%c",i,13,10);
					} else {
						sprintf(buffer,"CallType%02i = Analogue%c%c",i,13,10);
					}
					SaveBackupText(file, "", buffer, UseUnicode);
					sprintf(buffer,"IP%02i",i);
					SaveBackupText(file, buffer, settings->Settings[i].IPAddress, UseUnicode);
				}
				sprintf(buffer,"Number%02i",i);
				SaveBackupText(file, buffer, settings->Settings[i].DialUp, UseUnicode);
				if (settings->Settings[i].ManualLogin) {
					sprintf(buffer,"Login%02i = Manual%c%c",i,13,10);
				} else {
					sprintf(buffer,"Login%02i = Automatic%c%c",i,13,10);
				}
				SaveBackupText(file, "", buffer, UseUnicode);
				if (settings->Settings[i].IsNormalAuthentication) {
					sprintf(buffer,"Authentication%02i = Normal%c%c",i,13,10);
				} else {
					sprintf(buffer,"Authentication%02i = Secure%c%c",i,13,10);
				}
				SaveBackupText(file, "", buffer, UseUnicode);
				switch (settings->Settings[i].Speed) {
					case WAPSETTINGS_SPEED_9600 : sprintf(buffer,"CallSpeed%02i = 9600%c%c" ,i,13,10); break;
					case WAPSETTINGS_SPEED_14400: sprintf(buffer,"CallSpeed%02i = 14400%c%c",i,13,10); break;
					case WAPSETTINGS_SPEED_AUTO : sprintf(buffer,"CallSpeed%02i = auto%c%c" ,i,13,10); break;
				}
				switch (settings->Settings[i].Speed) {
					case WAPSETTINGS_SPEED_9600 :
					case WAPSETTINGS_SPEED_14400:
					case WAPSETTINGS_SPEED_AUTO :
						SaveBackupText(file, "", buffer, UseUnicode);
					default:
						break;
				}
				sprintf(buffer,"User%02i",i);
				SaveBackupText(file, buffer, settings->Settings[i].User, UseUnicode);
				sprintf(buffer,"Password%02i",i);
				SaveBackupText(file, buffer, settings->Settings[i].Password, UseUnicode);
				break;
			case WAPSETTINGS_BEARER_USSD:
				sprintf(buffer,"Bearer%02i = USSD%c%c",i,13,10);
				SaveBackupText(file, "", buffer, UseUnicode);
				sprintf(buffer,"ServiceCode%02i",i);
				SaveBackupText(file, buffer, settings->Settings[i].Code, UseUnicode);
				if (settings->Settings[i].IsIP) {
					sprintf(buffer,"IP%02i",i);
				} else {
					sprintf(buffer,"Number%02i",i);
				}
				SaveBackupText(file, buffer, settings->Settings[i].Service, UseUnicode);
		}
		sprintf(buffer,"%c%c",13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
	}
}

static void SaveChatSettingsEntry(FILE *file, GSM_ChatSettings *settings, bool UseUnicode)
{
	char buffer[10000];

	sprintf(buffer,"HomePage");
	SaveBackupText(file, buffer, settings->HomePage, UseUnicode);
	sprintf(buffer,"User");
	SaveBackupText(file, buffer, settings->User, UseUnicode);
	sprintf(buffer,"Password");
	SaveBackupText(file, buffer, settings->Password, UseUnicode);
	SaveWAPSettingsEntry(file, &settings->Connection, UseUnicode);
}

static void SaveSyncMLSettingsEntry(FILE *file, GSM_SyncMLSettings *settings, bool UseUnicode)
{
	char buffer[10000];

	sprintf(buffer,"User");
	SaveBackupText(file, buffer, settings->User, UseUnicode);
	sprintf(buffer,"Password");
	SaveBackupText(file, buffer, settings->Password, UseUnicode);
	sprintf(buffer,"PhonebookDB");
	SaveBackupText(file, buffer, settings->PhonebookDataBase, UseUnicode);
	sprintf(buffer,"CalendarDB");
	SaveBackupText(file, buffer, settings->CalendarDataBase, UseUnicode);
	sprintf(buffer,"Server");
	SaveBackupText(file, buffer, settings->Server, UseUnicode);
	if (settings->SyncPhonebook) {
		sprintf(buffer,"SyncPhonebook = True%c%c",13,10);
	} else {
		sprintf(buffer,"SyncPhonebook = False%c%c",13,10);
	}
	SaveBackupText(file, "", buffer, UseUnicode);
	if (settings->SyncCalendar) {
		sprintf(buffer,"SyncCalendar = True%c%c",13,10);
	} else {
		sprintf(buffer,"SyncCalendar = False%c%c",13,10);
	}
	SaveBackupText(file, "", buffer, UseUnicode);
	SaveWAPSettingsEntry(file, &settings->Connection, UseUnicode);
}

static void SaveBitmapEntry(FILE *file, GSM_Bitmap *bitmap, bool UseUnicode)
{
	unsigned char 	buffer[10000],buffer2[10000];
	int		x,y;

	sprintf(buffer,"Width = %i%c%c",bitmap->BitmapWidth,13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
	sprintf(buffer,"Height = %i%c%c",bitmap->BitmapHeight,13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
	for (y=0;y<bitmap->BitmapHeight;y++) {
		for (x=0;x<bitmap->BitmapWidth;x++) {
			buffer[x] = ' ';
			if (GSM_IsPointBitmap(bitmap,x,y)) buffer[x]='#';
		}
		buffer[bitmap->BitmapWidth] = 0;
		sprintf(buffer2,"Bitmap%02i = \"%s\"%c%c",y,buffer,13,10);
		SaveBackupText(file, "", buffer2, UseUnicode);
	}
}

static void SaveCallerEntry(FILE *file, GSM_Bitmap *bitmap, bool UseUnicode)
{
	unsigned char buffer[1000];

	sprintf(buffer,"Location = %03i%c%c",bitmap->Location,13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
	if (!bitmap->DefaultName) SaveBackupText(file, "Name", bitmap->Text, UseUnicode);
	if (!bitmap->DefaultRingtone) 	{
		if (bitmap->FileSystemRingtone) {
			sprintf(buffer,"FileRingtone = %02x%c%c",bitmap->RingtoneID,13,10);
		} else {
			sprintf(buffer,"Ringtone = %02x%c%c",bitmap->RingtoneID,13,10);
		}
		SaveBackupText(file, "", buffer, UseUnicode);
	}
	if (bitmap->BitmapEnabled) {
		sprintf(buffer,"Enabled = True%c%c",13,10);
	} else {
		sprintf(buffer,"Enabled = False%c%c",13,10);
	}
	SaveBackupText(file, "", buffer, UseUnicode);
	if (!bitmap->DefaultBitmap) SaveBitmapEntry(file, bitmap, UseUnicode);
	sprintf(buffer,"%c%c",13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
}

static void SaveWAPBookmarkEntry(FILE *file, GSM_WAPBookmark *bookmark, bool UseUnicode)
{
	unsigned char buffer[1000];

	SaveBackupText(file, "URL", bookmark->Address, UseUnicode);
	SaveBackupText(file, "Title", bookmark->Title, UseUnicode);
	sprintf(buffer,"%c%c",13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
}

static void SaveStartupEntry(FILE *file, GSM_Bitmap *bitmap, bool UseUnicode)
{
	unsigned char buffer[1000];

	sprintf(buffer,"[Startup]%c%c",13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
	if (bitmap->Type == GSM_WelcomeNote_Text) {
		SaveBackupText(file, "Text", bitmap->Text, UseUnicode);
	}
	if (bitmap->Type == GSM_StartupLogo) {
		SaveBitmapEntry(file, bitmap, UseUnicode);
	}
	sprintf(buffer,"%c%c",13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
}

static void SaveSMSCEntry(FILE *file, GSM_SMSC *SMSC, bool UseUnicode)
{
	unsigned char buffer[1000];

	sprintf(buffer,"Location = %03i%c%c",SMSC->Location,13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
	SaveBackupText(file, "Name", SMSC->Name, UseUnicode);
	SaveBackupText(file, "Number", SMSC->Number, UseUnicode);
	SaveBackupText(file, "DefaultNumber", SMSC->DefaultNumber, UseUnicode);
	SaveBackupText(file, "", "Format = ", UseUnicode);
	switch (SMSC->Format) {
		case SMS_FORMAT_Text	: sprintf(buffer,"Text");  break;
		case SMS_FORMAT_Fax	: sprintf(buffer,"Fax");   break;
		case SMS_FORMAT_Email	: sprintf(buffer,"Email"); break;
		case SMS_FORMAT_Pager	: sprintf(buffer,"Pager"); break;
	}
	SaveBackupText(file, "", buffer, UseUnicode);
	sprintf(buffer,"%c%cValidity = ",13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
	switch (SMSC->Validity.Relative) {
		case SMS_VALID_1_Hour	: sprintf(buffer, "1hour"	); break;
		case SMS_VALID_6_Hours 	: sprintf(buffer, "6hours"	); break;
		case SMS_VALID_1_Day	: sprintf(buffer, "24hours"	); break;
		case SMS_VALID_3_Days	: sprintf(buffer, "72hours"	); break;
		case SMS_VALID_1_Week  	: sprintf(buffer, "1week"	); break;
		case SMS_VALID_Max_Time	:
		default			: sprintf(buffer,"MaximumTime"	); break;
	}
	SaveBackupText(file, "", buffer, UseUnicode);
	sprintf(buffer,"%c%c%c%c",13,10,13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
}

static void SaveRingtoneEntry(FILE *file, GSM_Ringtone *ringtone, bool UseUnicode)
{
	unsigned char buffer[45000];

	sprintf(buffer,"Location = %i%c%c",ringtone->Location,13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
	SaveBackupText(file, "Name", ringtone->Name, UseUnicode);
	switch (ringtone->Format) {
	case RING_NOKIABINARY:
		EncodeHexBin(buffer,ringtone->NokiaBinary.Frame,ringtone->NokiaBinary.Length);
		SaveLinkedBackupText(file, "NokiaBinary", buffer, UseUnicode);
		break;
	case RING_MIDI:
		EncodeHexBin(buffer,ringtone->NokiaBinary.Frame,ringtone->NokiaBinary.Length);
		SaveLinkedBackupText(file, "Pure Midi", buffer, UseUnicode);
		break;
	case RING_MMF:
		EncodeHexBin(buffer,ringtone->NokiaBinary.Frame,ringtone->NokiaBinary.Length);
		SaveLinkedBackupText(file, "SMAF", buffer, UseUnicode);
		break;
	case RING_NOTETONE:
		break;
	}
	sprintf(buffer,"%c%c",13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
}

static void SaveOperatorEntry(FILE *file, GSM_Bitmap *bitmap, bool UseUnicode)
{
	unsigned char buffer[1000];

	sprintf(buffer,"[Operator]%c%c",13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
	sprintf(buffer,"Network = \"%s\"%c%c", bitmap->NetworkCode,13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
	SaveBitmapEntry(file, bitmap, UseUnicode);
	sprintf(buffer,"%c%c",13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
}

static void SaveToDoEntry(FILE *file, GSM_ToDoEntry *ToDo, bool UseUnicode)
{
	unsigned char 	buffer[1000];
    	int 		j;
    	
	sprintf(buffer,"Location = %i%c%c",ToDo->Location,13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
	switch (ToDo->Priority) {
	case GSM_Priority_High:
		sprintf(buffer,"Priority = High%c%c",13,10);
		break;
	case GSM_Priority_Medium:
		sprintf(buffer,"Priority = Medium%c%c",13,10);
		break;
	case GSM_Priority_Low:
		sprintf(buffer,"Priority = Low%c%c",13,10);
		break;
	}
	SaveBackupText(file, "", buffer, UseUnicode);
	
	for (j=0;j<ToDo->EntriesNum;j++) {
        switch (ToDo->Entries[j].EntryType) {
	    case TODO_END_DATETIME:
		SaveBackupText(file, "", "DueTime", UseUnicode);
                SaveVCalDateTime(file, &ToDo->Entries[j].Date, UseUnicode);
                break;
            case TODO_COMPLETED:
	        sprintf(buffer,"Completed = %s%c%c",ToDo->Entries[j].Number == 1 ? "yes" : "no" ,13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
                break;
            case TODO_ALARM_DATETIME:
		SaveBackupText(file, "", "Alarm", UseUnicode);
                SaveVCalDateTime(file, &ToDo->Entries[j].Date, UseUnicode);
                break;
            case TODO_SILENT_ALARM_DATETIME:
		SaveBackupText(file, "", "SilentAlarm", UseUnicode);
                SaveVCalDateTime(file, &ToDo->Entries[j].Date, UseUnicode);
                break;
            case TODO_TEXT:
	        SaveBackupText(file, "Text", ToDo->Entries[j].Text, UseUnicode);
                break;
            case TODO_PRIVATE:
	        sprintf(buffer,"Private = %i%c%c",ToDo->Entries[j].Number,13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
                break;
            case TODO_CATEGORY:
	        sprintf(buffer,"Category = %i%c%c",ToDo->Entries[j].Number,13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
                break;
            case TODO_CONTACTID:
	        sprintf(buffer,"ContactID = %i%c%c",ToDo->Entries[j].Number,13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
                break;
            case TODO_PHONE:
	        SaveBackupText(file, "Phone", ToDo->Entries[j].Text, UseUnicode);
                break;
        }
    }
    sprintf(buffer,"%c%c",13,10);
    SaveBackupText(file, "", buffer, UseUnicode);
}

static void SaveProfileEntry(FILE *file, GSM_Profile *Profile, bool UseUnicode)
{
	int			j,k;
	bool			special;
	unsigned char 		buffer[1000];
		
	sprintf(buffer,"Location = %i%c%c",Profile->Location,13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
	SaveBackupText(file, "Name",Profile->Name, UseUnicode);

	if (Profile->DefaultName) {
		sprintf(buffer,"DefaultName = true%c%c",13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
	}
	if (Profile->HeadSetProfile) {
		sprintf(buffer,"HeadSetProfile = true%c%c",13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
	}
	if (Profile->CarKitProfile) {
		sprintf(buffer,"CarKitProfile = true%c%c",13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
	}

	for (j=0;j<Profile->FeaturesNumber;j++) {
		sprintf(buffer,"Feature%02i = ",j);
		SaveBackupText(file, "", buffer, UseUnicode);
		special = false;
		switch (Profile->FeatureID[j]) {
		case Profile_MessageToneID:
		case Profile_RingtoneID:
			special = true;
			if (Profile->FeatureID[j] == Profile_RingtoneID) {
				sprintf(buffer,"RingtoneID%c%c",13,10);
			} else {
				sprintf(buffer,"MessageToneID%c%c",13,10);
			}
			SaveBackupText(file, "", buffer, UseUnicode);
			sprintf(buffer,"Value%02i = %i%c%c",j,Profile->FeatureValue[j],13,10);
			SaveBackupText(file, "", buffer, UseUnicode);
			break;	
		case Profile_CallerGroups:
			special = true;
			sprintf(buffer,"CallerGroups%c%c",13,10);
			SaveBackupText(file, "", buffer, UseUnicode);
			sprintf(buffer,"Value%02i = ",j);
			SaveBackupText(file, "", buffer, UseUnicode);
			for (k=0;k<5;k++) {
				if (Profile->CallerGroups[k]) {
					sprintf(buffer,"%i",k);
					SaveBackupText(file, "", buffer, UseUnicode);
				}
			}
			sprintf(buffer,"%c%c",13,10);
			SaveBackupText(file, "", buffer, UseUnicode);
			break;
		case Profile_ScreenSaverNumber:
			special = true;
			sprintf(buffer,"ScreenSaverNumber%c%c",13,10);
			SaveBackupText(file, "", buffer, UseUnicode);
			sprintf(buffer,"Value%02i = %i%c%c",j,Profile->FeatureValue[j],13,10);
			SaveBackupText(file, "", buffer, UseUnicode);
			break;
		case Profile_CallAlert  	: sprintf(buffer,"IncomingCallAlert%c%c",13,10); 		break;
		case Profile_RingtoneVolume 	: sprintf(buffer,"RingtoneVolume%c%c",13,10); 			break;
		case Profile_Vibration		: sprintf(buffer,"Vibrating%c%c",13,10); 			break;
		case Profile_MessageTone	: sprintf(buffer,"MessageTone%c%c",13,10); 			break;
		case Profile_KeypadTone		: sprintf(buffer,"KeypadTones%c%c",13,10); 			break;
		case Profile_WarningTone	: sprintf(buffer,"WarningTones%c%c",13,10); 			break;
		case Profile_ScreenSaver	: sprintf(buffer,"ScreenSaver%c%c",13,10); 			break;
		case Profile_ScreenSaverTime	: sprintf(buffer,"ScreenSaverTimeout%c%c",13,10); 		break;
		case Profile_AutoAnswer		: sprintf(buffer,"AutomaticAnswer%c%c",13,10); 			break;
		case Profile_Lights		: sprintf(buffer,"Lights%c%c",13,10); 				break;
		default				: special = true;
		}
		if (!special) {
			SaveBackupText(file, "", buffer, UseUnicode);
			sprintf(buffer,"Value%02i = ",j);
			SaveBackupText(file, "", buffer, UseUnicode);
			switch (Profile->FeatureValue[j]) {
			case PROFILE_VOLUME_LEVEL1 		:
			case PROFILE_KEYPAD_LEVEL1 		: sprintf(buffer,"Level1%c%c",13,10); 		break;
			case PROFILE_VOLUME_LEVEL2 		:
			case PROFILE_KEYPAD_LEVEL2 		: sprintf(buffer,"Level2%c%c",13,10);		break;
			case PROFILE_VOLUME_LEVEL3 		:
			case PROFILE_KEYPAD_LEVEL3 		: sprintf(buffer,"Level3%c%c",13,10); 		break;
			case PROFILE_VOLUME_LEVEL4 		: sprintf(buffer,"Level4%c%c",13,10); 		break;
			case PROFILE_VOLUME_LEVEL5 		: sprintf(buffer,"Level5%c%c",13,10); 		break;
			case PROFILE_MESSAGE_NOTONE 		:
			case PROFILE_AUTOANSWER_OFF		:
			case PROFILE_LIGHTS_OFF  		:
			case PROFILE_SAVER_OFF			:
			case PROFILE_WARNING_OFF		:
			case PROFILE_CALLALERT_OFF	 	:
			case PROFILE_VIBRATION_OFF 		:
			case PROFILE_KEYPAD_OFF	   		: sprintf(buffer,"Off%c%c",13,10);	  	break;
			case PROFILE_CALLALERT_RINGING   	: sprintf(buffer,"Ringing%c%c",13,10);		break;
			case PROFILE_CALLALERT_RINGONCE  	: sprintf(buffer,"RingOnce%c%c",13,10);		break;
			case PROFILE_CALLALERT_ASCENDING 	: sprintf(buffer,"Ascending%c%c",13,10);        break;
			case PROFILE_CALLALERT_CALLERGROUPS	: sprintf(buffer,"CallerGroups%c%c",13,10);	break;
			case PROFILE_MESSAGE_STANDARD 		: sprintf(buffer,"Standard%c%c",13,10);  	break;
			case PROFILE_MESSAGE_SPECIAL 		: sprintf(buffer,"Special%c%c",13,10);	 	break;
			case PROFILE_MESSAGE_BEEPONCE		:
			case PROFILE_CALLALERT_BEEPONCE  	: sprintf(buffer,"BeepOnce%c%c",13,10);		break;
			case PROFILE_MESSAGE_ASCENDING		: sprintf(buffer,"Ascending%c%c",13,10); 	break;
			case PROFILE_MESSAGE_PERSONAL		: sprintf(buffer,"Personal%c%c",13,10);		break;
			case PROFILE_AUTOANSWER_ON		:
			case PROFILE_WARNING_ON			:
			case PROFILE_SAVER_ON			:
			case PROFILE_VIBRATION_ON 		: sprintf(buffer,"On%c%c",13,10);  		break;
			case PROFILE_VIBRATION_FIRST 		: sprintf(buffer,"VibrateFirst%c%c",13,10);	break;
			case PROFILE_LIGHTS_AUTO 		: sprintf(buffer,"Auto%c%c",13,10); 		break;
			case PROFILE_SAVER_TIMEOUT_5SEC	 	: sprintf(buffer,"5Seconds%c%c",13,10); 	break;
			case PROFILE_SAVER_TIMEOUT_20SEC 	: sprintf(buffer,"20Seconds%c%c",13,10); 	break;
			case PROFILE_SAVER_TIMEOUT_1MIN	 	: sprintf(buffer,"1Minute%c%c",13,10);		break;
			case PROFILE_SAVER_TIMEOUT_2MIN	 	: sprintf(buffer,"2Minutes%c%c",13,10);		break;
			case PROFILE_SAVER_TIMEOUT_5MIN	 	: sprintf(buffer,"5Minutes%c%c",13,10);		break;
			case PROFILE_SAVER_TIMEOUT_10MIN 	: sprintf(buffer,"10Minutes%c%c",13,10);	break;
			default					: sprintf(buffer,"UNKNOWN%c%c",13,10);
			}	
			SaveBackupText(file, "", buffer, UseUnicode);
		}
	}
	sprintf(buffer,"%c%c",13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
}

static void SaveFMStationEntry(FILE *file, GSM_FMStation *FMStation, bool UseUnicode)
{
	unsigned char buffer[1000];

 	sprintf(buffer,"Location = %i%c%c",FMStation->Location,13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
        SaveBackupText(file, "StationName", FMStation->StationName, UseUnicode);
        sprintf(buffer,"Frequency = %f%c%c",FMStation->Frequency,13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
	sprintf(buffer,"%c%c",13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
}

static void SaveGPRSPointEntry(FILE *file, GSM_GPRSAccessPoint *GPRSPoint, bool UseUnicode)
{
	unsigned char buffer[1000];

 	sprintf(buffer,"Location = %i%c%c",GPRSPoint->Location,13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
        SaveBackupText(file, "Name", GPRSPoint->Name, UseUnicode);
        SaveBackupText(file, "URL", GPRSPoint->URL, UseUnicode);
	if (GPRSPoint->Active) {
		sprintf(buffer,"Active = Yes%c%c",13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
	}
	sprintf(buffer,"%c%c",13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
}

GSM_Error SaveBackup(char *FileName, GSM_Backup *backup, bool UseUnicode)
{
	int 		i;
	unsigned char 	buffer[1000],checksum[200];
	FILE 		*file;
 
	file = fopen(FileName, "wb");      
	if (file == NULL) return ERR_CANTOPENFILE;

	if (UseUnicode) {
		sprintf(buffer,"%c%c", 0xFE, 0xFF);
		SaveBackupText(file, "", buffer, false);
	}

	sprintf(buffer,"# Format of this file was designed for Gammu (see www.mwiacek.com)%c%c%c%c",13,10,13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
	sprintf(buffer,"[Backup]%c%c",13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
	sprintf(buffer,"IMEI = \"%s\"%c%c",backup->IMEI,13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
	sprintf(buffer,"Phone = \"%s\"%c%c",backup->Model,13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
	if (backup->Creator[0] != 0) {
		sprintf(buffer,"Creator = \"%s\"%c%c",backup->Creator,13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
	}
	if (backup->DateTimeAvailable) {
		SaveBackupText(file, "", "DateTime", UseUnicode);
		SaveVCalDateTime(file, &backup->DateTime, UseUnicode);
	}
	sprintf(buffer,"Format = 1.03%c%c",13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
	sprintf(buffer,"%c%c",13,10);
	SaveBackupText(file, "", buffer, UseUnicode);

	i=0;
	while (backup->PhonePhonebook[i]!=NULL) {
		sprintf(buffer,"[PhonePBK%03i]%c%c",i+1,13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
		SavePbkEntry(file, backup->PhonePhonebook[i], UseUnicode);
		i++;
	}
	i=0;
	while (backup->SIMPhonebook[i]!=NULL) {
		sprintf(buffer,"[SIMPBK%03i]%c%c",i+1,13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
		SavePbkEntry(file, backup->SIMPhonebook[i], UseUnicode);
		i++;
	}
	i=0;
	while (backup->Calendar[i]!=NULL) {
		sprintf(buffer,"[Calendar%03i]%c%c",i+1,13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
		SaveCalendarEntry(file, backup->Calendar[i], UseUnicode);
		i++;
	}
	i=0;
	while (backup->Note[i]!=NULL) {
		sprintf(buffer,"[Note%03i]%c%c",i+1,13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
		SaveNoteEntry(file, backup->Note[i], UseUnicode);
		i++;
	}
	i=0;
	while (backup->CallerLogos[i]!=NULL) {
		sprintf(buffer,"[Caller%03i]%c%c",i+1,13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
		SaveCallerEntry(file, backup->CallerLogos[i], UseUnicode);
		i++;
	}
	i=0;
	while (backup->SMSC[i]!=NULL) {
		sprintf(buffer,"[SMSC%03i]%c%c",i+1,13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
		SaveSMSCEntry(file, backup->SMSC[i], UseUnicode);
		i++;
	}
	i=0;
	while (backup->WAPBookmark[i]!=NULL) {
		sprintf(buffer,"[WAPBookmark%03i]%c%c",i+1,13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
		SaveWAPBookmarkEntry(file, backup->WAPBookmark[i], UseUnicode);
		i++;
	}
	i=0;
	while (backup->WAPSettings[i]!=NULL) {
		sprintf(buffer,"[WAPSettings%03i]%c%c",i+1,13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
		SaveWAPSettingsEntry(file, backup->WAPSettings[i], UseUnicode);
		i++;
	}
	i=0;
	while (backup->MMSSettings[i]!=NULL) {
		sprintf(buffer,"[MMSSettings%03i]%c%c",i+1,13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
		SaveWAPSettingsEntry(file, backup->MMSSettings[i], UseUnicode);
		i++;
	}
	i=0;
	while (backup->SyncMLSettings[i]!=NULL) {
		sprintf(buffer,"[SyncMLSettings%03i]%c%c",i+1,13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
		SaveSyncMLSettingsEntry(file, backup->SyncMLSettings[i], UseUnicode);
		i++;
	}
	i=0;
	while (backup->ChatSettings[i]!=NULL) {
		sprintf(buffer,"[ChatSettings%03i]%c%c",i+1,13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
		SaveChatSettingsEntry(file, backup->ChatSettings[i], UseUnicode);
		i++;
	}
	i=0;
	while (backup->Ringtone[i]!=NULL) {
		sprintf(buffer,"[Ringtone%03i]%c%c",i+1,13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
		SaveRingtoneEntry(file, backup->Ringtone[i], UseUnicode);
		i++;
	}
	i=0;
	while (backup->ToDo[i]!=NULL) {
		sprintf(buffer,"[TODO%03i]%c%c",i+1,13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
		SaveToDoEntry(file, backup->ToDo[i], UseUnicode);
		i++;
	}
	i=0;
	while (backup->Profiles[i]!=NULL) {
		sprintf(buffer,"[Profile%03i]%c%c",i+1,13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
		SaveProfileEntry(file, backup->Profiles[i], UseUnicode);
		i++;
	}
 	i=0;
 	while (backup->FMStation[i]!=NULL) {
 		sprintf(buffer,"[FMStation%03i]%c%c",i+1,13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
 		SaveFMStationEntry(file, backup->FMStation[i], UseUnicode);
 		i++;
 	}
 	i=0;
 	while (backup->GPRSPoint[i]!=NULL) {
 		sprintf(buffer,"[GPRSPoint%03i]%c%c",i+1,13,10);
		SaveBackupText(file, "", buffer, UseUnicode);
 		SaveGPRSPointEntry(file, backup->GPRSPoint[i], UseUnicode);
 		i++;
 	}

	if (backup->StartupLogo!=NULL) {
		SaveStartupEntry(file, backup->StartupLogo, UseUnicode);
	}
	if (backup->OperatorLogo!=NULL) {
		SaveOperatorEntry(file, backup->OperatorLogo, UseUnicode);
	}

	fclose(file);

	FindBackupChecksum(FileName, UseUnicode, checksum);

	file = fopen(FileName, "ab");      
	if (file == NULL) return ERR_CANTOPENFILE;
	sprintf(buffer,"[Checksum]%c%c",13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
	sprintf(buffer,"MD5=%s%c%c",checksum,13,10);
	SaveBackupText(file, "", buffer, UseUnicode);
	fclose(file);

	return ERR_NONE;
}

static void ReadPbkEntry(INI_Section *file_info, char *section, GSM_MemoryEntry *Pbk, bool UseUnicode)
{
	unsigned char		buffer[10000];
	char			*readvalue;
	int			num,i;
	INI_Entry		*e;

	Pbk->EntriesNum = 0;
	e = INI_FindLastSectionEntry(file_info, section, UseUnicode);
	
	while (e != NULL) {
		num = -1;
		if (UseUnicode) {
			sprintf(buffer,"%s",DecodeUnicodeString(e->EntryName));
		} else {
			sprintf(buffer,"%s",e->EntryName);
		}
		if (strlen(buffer) == 11) {
			if (mystrncasecmp("Entry", buffer,   5) &&
			    mystrncasecmp("Type",  buffer+7, 4)) {
				num = atoi(buffer+5);
			}
		}
		e = e->Prev;
		if (num != -1) {
			sprintf(buffer,"Entry%02iType",num);
			readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
			if (mystrncasecmp(readvalue,"NumberGeneral",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_General;
			} else if (mystrncasecmp(readvalue,"NumberMobile",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Mobile;
			} else if (mystrncasecmp(readvalue,"NumberWork",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Work;
			} else if (mystrncasecmp(readvalue,"NumberFax",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Fax;
			} else if (mystrncasecmp(readvalue,"NumberHome",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Home;
			} else if (mystrncasecmp(readvalue,"NumberOther",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Other;
			} else if (mystrncasecmp(readvalue,"NumberPager",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Number_Pager;
			} else if (mystrncasecmp(readvalue,"Note",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Note;
			} else if (mystrncasecmp(readvalue,"Postal",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Postal;
			} else if (mystrncasecmp(readvalue,"Email",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Email;
			} else if (mystrncasecmp(readvalue,"Email2",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Email2;
			} else if (mystrncasecmp(readvalue,"URL",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_URL;
			} else if (mystrncasecmp(readvalue,"FirstName",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_FirstName;
			} else if (mystrncasecmp(readvalue,"LastName",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_LastName;
			} else if (mystrncasecmp(readvalue,"Company",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Company;
			} else if (mystrncasecmp(readvalue,"JobTitle",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_JobTitle;
			} else if (mystrncasecmp(readvalue,"Address",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_StreetAddress;
			} else if (mystrncasecmp(readvalue,"City",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_City;
			} else if (mystrncasecmp(readvalue,"State",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_State;
			} else if (mystrncasecmp(readvalue,"Zip",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Zip;
			} else if (mystrncasecmp(readvalue,"Country",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Country;
			} else if (mystrncasecmp(readvalue,"Custom1",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Custom1;
			} else if (mystrncasecmp(readvalue,"Custom2",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Custom2;
			} else if (mystrncasecmp(readvalue,"Custom3",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Custom3;
			} else if (mystrncasecmp(readvalue,"Custom4",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Custom4;
			} else if (mystrncasecmp(readvalue,"Name",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_Name;
			} else if (mystrncasecmp(readvalue,"Category",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Category;
				Pbk->Entries[Pbk->EntriesNum].Number = 0;
				sprintf(buffer,"Entry%02iNumber",num);
				readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
				if (readvalue!=NULL) {
					Pbk->Entries[Pbk->EntriesNum].Number = atoi(readvalue);
				}
				Pbk->EntriesNum ++;
				continue;
			} else if (mystrncasecmp(readvalue,"Private",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Private;
				Pbk->Entries[Pbk->EntriesNum].Number = 0;
				sprintf(buffer,"Entry%02iNumber",num);
				readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
				if (readvalue!=NULL) {
					Pbk->Entries[Pbk->EntriesNum].Number = atoi(readvalue);
				}
				Pbk->EntriesNum ++;
				continue;
			} else if (mystrncasecmp(readvalue,"CallerGroup",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Caller_Group;
				Pbk->Entries[Pbk->EntriesNum].Number = 0;
				sprintf(buffer,"Entry%02iNumber",num);
				readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
				if (readvalue!=NULL) {
					Pbk->Entries[Pbk->EntriesNum].Number = atoi(readvalue);
				}
				Pbk->EntriesNum ++;
				continue;
			} else if (mystrncasecmp(readvalue,"RingtoneID",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_RingtoneID;
				Pbk->Entries[Pbk->EntriesNum].Number = 0;
				sprintf(buffer,"Entry%02iNumber",num);
				readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
				if (readvalue!=NULL) {
					Pbk->Entries[Pbk->EntriesNum].Number = atoi(readvalue);
				}
				Pbk->EntriesNum ++;
				continue;
			} else if (mystrncasecmp(readvalue,"PictureID",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_PictureID;
				Pbk->Entries[Pbk->EntriesNum].Number = 0;
				sprintf(buffer,"Entry%02iNumber",num);
				readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
				if (readvalue!=NULL) {
					Pbk->Entries[Pbk->EntriesNum].Number = atoi(readvalue);
				}
				Pbk->EntriesNum ++;
				continue;
			} else if (mystrncasecmp(readvalue,"UserID",0)) {
				Pbk->Entries[Pbk->EntriesNum].EntryType = PBK_Text_UserID;
			}
			sprintf(buffer,"Entry%02iText",num);
			ReadBackupText(file_info, section, buffer, Pbk->Entries[Pbk->EntriesNum].Text,UseUnicode);
			dbgprintf("text \"%s\", type %i\n",DecodeUnicodeString(Pbk->Entries[Pbk->EntriesNum].Text),Pbk->Entries[Pbk->EntriesNum].EntryType);
			Pbk->Entries[Pbk->EntriesNum].VoiceTag = 0;
			sprintf(buffer,"Entry%02iVoiceTag",num);
			readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
			if (readvalue!=NULL) {
				Pbk->Entries[Pbk->EntriesNum].VoiceTag = atoi(readvalue);
			}
			i = 0;
			while (1) {
				Pbk->Entries[Pbk->EntriesNum].SMSList[i] = 0;
				sprintf(buffer,"Entry%02iSMSList%02i",num,i);
				readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
				if (readvalue==NULL) break;
				Pbk->Entries[Pbk->EntriesNum].SMSList[i] = atoi(readvalue);
				i++;
			}
			Pbk->EntriesNum ++;
		}
	}
}

static void ReadCalendarEntry(INI_Section *file_info, char *section, GSM_CalendarEntry *note, bool UseUnicode)
{
	unsigned char		buffer[10000];
	char			*readvalue;

	sprintf(buffer,"Location");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) note->Location = atoi(readvalue);

	sprintf(buffer,"Type");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	note->Type = GSM_CAL_REMINDER;
	if (readvalue!=NULL)
	{
		if (mystrncasecmp(readvalue,"Call",0)) {
			note->Type = GSM_CAL_CALL;
		} else if (mystrncasecmp(readvalue,"Meeting",0)) {
			note->Type = GSM_CAL_MEETING;
		} else if (mystrncasecmp(readvalue,"Birthday",0)) {
			note->Type = GSM_CAL_BIRTHDAY;
		} else if (mystrncasecmp(readvalue,"Memo",0)) {
			note->Type = GSM_CAL_MEMO;
		} else if (mystrncasecmp(readvalue,"Travel",0)) {
			note->Type = GSM_CAL_TRAVEL;
		} else if (mystrncasecmp(readvalue,"Vacation",0)) {
			note->Type = GSM_CAL_VACATION;
		} else if (mystrncasecmp(readvalue,"DailyAlarm",0)) {
			note->Type = GSM_CAL_DAILY_ALARM;
		} else if (mystrncasecmp(readvalue,"Alarm",0)) {
			note->Type = GSM_CAL_ALARM;
		} else if (mystrncasecmp(readvalue,"Training/Athletism",0)) {
			note->Type = GSM_CAL_T_ATHL;
		} else if (mystrncasecmp(readvalue,"Training/BallGames",0)) {
			note->Type = GSM_CAL_T_BALL;
		} else if (mystrncasecmp(readvalue,"Training/Cycling",0)) {
			note->Type = GSM_CAL_T_CYCL;
		} else if (mystrncasecmp(readvalue,"Training/Budo",0)) {
			note->Type = GSM_CAL_T_BUDO;
		} else if (mystrncasecmp(readvalue,"Training/Dance",0)) {
			note->Type = GSM_CAL_T_DANC;
		} else if (mystrncasecmp(readvalue,"Training/ExtremeSports",0)) {
			note->Type = GSM_CAL_T_EXTR;
		} else if (mystrncasecmp(readvalue,"Training/Football",0)) {
			note->Type = GSM_CAL_T_FOOT;
		} else if (mystrncasecmp(readvalue,"Training/Golf",0)) {
			note->Type = GSM_CAL_T_GOLF;
		} else if (mystrncasecmp(readvalue,"Training/Gym",0)) {
			note->Type = GSM_CAL_T_GYM;
		} else if (mystrncasecmp(readvalue,"Training/HorseRaces",0)) {
			note->Type = GSM_CAL_T_HORS;
		} else if (mystrncasecmp(readvalue,"Training/Hockey",0)) {
			note->Type = GSM_CAL_T_HOCK;
		} else if (mystrncasecmp(readvalue,"Training/Races",0)) {
			note->Type = GSM_CAL_T_RACE;
		} else if (mystrncasecmp(readvalue,"Training/Rugby",0)) {
			note->Type = GSM_CAL_T_RUGB;
		} else if (mystrncasecmp(readvalue,"Training/Sailing",0)) {
			note->Type = GSM_CAL_T_SAIL;
		} else if (mystrncasecmp(readvalue,"Training/StreetGames",0)) {
			note->Type = GSM_CAL_T_STRE;
		} else if (mystrncasecmp(readvalue,"Training/Swimming",0)) {
			note->Type = GSM_CAL_T_SWIM;
		} else if (mystrncasecmp(readvalue,"Training/Tennis",0)) {
			note->Type = GSM_CAL_T_TENN;
		} else if (mystrncasecmp(readvalue,"Training/Travels",0)) {
			note->Type = GSM_CAL_T_TRAV;
		} else if (mystrncasecmp(readvalue,"Training/WinterGames",0)) {
			note->Type = GSM_CAL_T_WINT;
		}
	}
	note->EntriesNum = 0;
	sprintf(buffer,"Text");
	if (ReadBackupText(file_info, section, buffer, note->Entries[note->EntriesNum].Text,UseUnicode)) {
		note->Entries[note->EntriesNum].EntryType = CAL_TEXT;
		note->EntriesNum++;
	}
	sprintf(buffer,"Phone");
	if (ReadBackupText(file_info, section, buffer, note->Entries[note->EntriesNum].Text,UseUnicode)) {
		note->Entries[note->EntriesNum].EntryType = CAL_PHONE;
		note->EntriesNum++;
	}
	sprintf(buffer,"Private");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		note->Entries[note->EntriesNum].Number 	  = atoi(readvalue);
		note->Entries[note->EntriesNum].EntryType = CAL_PRIVATE;
		note->EntriesNum++;
	}
	sprintf(buffer,"EventLocation");
	if (ReadBackupText(file_info, section, buffer, note->Entries[note->EntriesNum].Text,UseUnicode)) {
		note->Entries[note->EntriesNum].EntryType = CAL_LOCATION;
		note->EntriesNum++;
	}
	sprintf(buffer,"ContactID");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		note->Entries[note->EntriesNum].Number 	  = atoi(readvalue);
		note->Entries[note->EntriesNum].EntryType = CAL_CONTACTID;
		note->EntriesNum++;
	}
	sprintf(buffer,"Recurrance");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		note->Entries[note->EntriesNum].Number 	  = atoi(readvalue) * 24;
		note->Entries[note->EntriesNum].EntryType = CAL_RECURRANCE;
		note->EntriesNum++;
	}
	sprintf(buffer,"StartTime");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		ReadVCALDateTime(readvalue, &note->Entries[note->EntriesNum].Date);
		note->Entries[note->EntriesNum].EntryType = CAL_START_DATETIME;
		note->EntriesNum++;
	}
	sprintf(buffer,"StopTime");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		ReadVCALDateTime(readvalue, &note->Entries[note->EntriesNum].Date);
		note->Entries[note->EntriesNum].EntryType = CAL_END_DATETIME;
		note->EntriesNum++;
	}
	sprintf(buffer,"Alarm");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL)
	{
		ReadVCALDateTime(readvalue, &note->Entries[note->EntriesNum].Date);
		note->Entries[note->EntriesNum].EntryType = CAL_ALARM_DATETIME;
		sprintf(buffer,"AlarmType");
		readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
		if (readvalue!=NULL)
		{
			if (mystrncasecmp(readvalue,"Silent",0)) {
				note->Entries[note->EntriesNum].EntryType = CAL_SILENT_ALARM_DATETIME;
			}
		}
		note->EntriesNum++;
	}
	sprintf(buffer,"RepeatStartDate");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		ReadVCALDateTime(readvalue, &note->Entries[note->EntriesNum].Date);
		note->Entries[note->EntriesNum].EntryType = CAL_REPEAT_STARTDATE;
		note->EntriesNum++;
	}
	sprintf(buffer,"RepeatStopDate");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		ReadVCALDateTime(readvalue, &note->Entries[note->EntriesNum].Date);
		note->Entries[note->EntriesNum].EntryType = CAL_REPEAT_STOPDATE;
		note->EntriesNum++;
	}
	sprintf(buffer,"RepeatDayOfWeek");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		note->Entries[note->EntriesNum].Number 	  = atoi(readvalue);
		note->Entries[note->EntriesNum].EntryType = CAL_REPEAT_DAYOFWEEK;
		note->EntriesNum++;
	}
	sprintf(buffer,"RepeatDay");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		note->Entries[note->EntriesNum].Number 	  = atoi(readvalue);
		note->Entries[note->EntriesNum].EntryType = CAL_REPEAT_DAY;
		note->EntriesNum++;
	}
	sprintf(buffer,"RepeatWeekOfMonth");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		note->Entries[note->EntriesNum].Number 	  = atoi(readvalue);
		note->Entries[note->EntriesNum].EntryType = CAL_REPEAT_WEEKOFMONTH;
		note->EntriesNum++;
	}
	sprintf(buffer,"RepeatMonth");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		note->Entries[note->EntriesNum].Number 	  = atoi(readvalue);
		note->Entries[note->EntriesNum].EntryType = CAL_REPEAT_MONTH;
		note->EntriesNum++;
	}
	sprintf(buffer,"RepeatFrequency");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		note->Entries[note->EntriesNum].Number 	  = atoi(readvalue);
		note->Entries[note->EntriesNum].EntryType = CAL_REPEAT_FREQUENCY;
		note->EntriesNum++;
	}
}

static void ReadToDoEntry(INI_Section *file_info, char *section, GSM_ToDoEntry *ToDo, bool UseUnicode)
{
	unsigned char		buffer[10000];
	char			*readvalue;

    	ToDo->EntriesNum = 0;
	
	sprintf(buffer,"Location");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) ToDo->Location = atoi(readvalue);

	ToDo->Priority = GSM_Priority_High;
	sprintf(buffer,"Priority");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		if (!strcmp(readvalue,"3") || !strcmp(readvalue,"Low")) {
			ToDo->Priority = GSM_Priority_Low;
		}
		if (!strcmp(readvalue,"2") || !strcmp(readvalue,"Medium")) {
			ToDo->Priority = GSM_Priority_Medium;
		}
	}

	sprintf(buffer,"Text");
	if (ReadBackupText(file_info, section, buffer, ToDo->Entries[ToDo->EntriesNum].Text,UseUnicode)) {
  	      	ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_TEXT;
        	ToDo->EntriesNum++;
    	}

	sprintf(buffer,"Phone");
	if (ReadBackupText(file_info, section, buffer, ToDo->Entries[ToDo->EntriesNum].Text,UseUnicode)) {
        	ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_PHONE;
        	ToDo->EntriesNum++;
    	}
    
	sprintf(buffer,"Private");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
        	ToDo->Entries[ToDo->EntriesNum].Number 		= atoi(readvalue);
        	ToDo->Entries[ToDo->EntriesNum].EntryType 	= TODO_PRIVATE;
        	ToDo->EntriesNum++;
    	}
    
	sprintf(buffer,"Completed");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		if (strncmp(readvalue, "yes", 3) == 0) {
			ToDo->Entries[ToDo->EntriesNum].Number 	= 1;
		} else {
			ToDo->Entries[ToDo->EntriesNum].Number 	= 0;
		}
        	ToDo->Entries[ToDo->EntriesNum].EntryType 	= TODO_COMPLETED;
        	ToDo->EntriesNum++;
    	}
    
	sprintf(buffer,"Category");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
        	ToDo->Entries[ToDo->EntriesNum].Number		= atoi(readvalue);
        	ToDo->Entries[ToDo->EntriesNum].EntryType 	= TODO_CATEGORY;
        	ToDo->EntriesNum++;
    	}

	sprintf(buffer,"ContactID");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
       	 	ToDo->Entries[ToDo->EntriesNum].Number 		= atoi(readvalue);
        	ToDo->Entries[ToDo->EntriesNum].EntryType 	= TODO_CONTACTID;
        	ToDo->EntriesNum++;
    	}

	sprintf(buffer,"DueTime");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		ReadVCALDateTime(readvalue, &ToDo->Entries[ToDo->EntriesNum].Date);
        	ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_END_DATETIME;
        	ToDo->EntriesNum++;
   	}
    
	sprintf(buffer,"Alarm");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		ReadVCALDateTime(readvalue, &ToDo->Entries[ToDo->EntriesNum].Date);
        	ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_ALARM_DATETIME;
        	ToDo->EntriesNum++;
    	}

	sprintf(buffer,"SilentAlarm");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		ReadVCALDateTime(readvalue, &ToDo->Entries[ToDo->EntriesNum].Date);
        	ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_SILENT_ALARM_DATETIME;
        	ToDo->EntriesNum++;
    	}
}

static bool ReadBitmapEntry(INI_Section *file_info, char *section, GSM_Bitmap *bitmap, bool UseUnicode)
{
	char		*readvalue;
	unsigned char	buffer[10000];
	unsigned char 	Width, Height;
	int 		x, y;

	GSM_GetMaxBitmapWidthHeight(bitmap->Type, &Width, &Height);
	sprintf(buffer,"Width");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue==NULL) bitmap->BitmapWidth = Width; else bitmap->BitmapWidth = atoi(readvalue);
	sprintf(buffer,"Height");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue==NULL) bitmap->BitmapHeight = Height; else bitmap->BitmapHeight = atoi(readvalue);
	GSM_ClearBitmap(bitmap);
	for (y=0;y<bitmap->BitmapHeight;y++) {
		sprintf(buffer,"Bitmap%02i",y);
		readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
		if (readvalue!=NULL) {
			for (x=0;x<bitmap->BitmapWidth;x++) {
				if (readvalue[x+1]=='#') GSM_SetPointBitmap(bitmap,x,y);
			}
		} else return false;
	}
	return true;
}

static void ReadCallerEntry(INI_Section *file_info, char *section, GSM_Bitmap *bitmap, bool UseUnicode)
{
	unsigned char		buffer[10000];
	char			*readvalue;

	bitmap->Type 		= GSM_CallerGroupLogo;
	bitmap->DefaultBitmap 	= !ReadBitmapEntry(file_info, section, bitmap, UseUnicode);
	if (bitmap->DefaultBitmap) {
		bitmap->BitmapWidth  = 72;
		bitmap->BitmapHeight = 14;
		GSM_ClearBitmap(bitmap);
	}
	sprintf(buffer,"Name");
	ReadBackupText(file_info, section, buffer, bitmap->Text,UseUnicode);
	if (bitmap->Text[0] == 0x00 && bitmap->Text[1] == 0x00) {
		bitmap->DefaultName = true;
	} else {
		bitmap->DefaultName = false;
	}
	sprintf(buffer,"Ringtone");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue==NULL) {
		sprintf(buffer,"FileRingtone");
		readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
		if (readvalue==NULL) {
			bitmap->DefaultRingtone = true;
		} else {
			DecodeHexBin (&bitmap->RingtoneID, readvalue, 2);
			bitmap->DefaultRingtone 	= false;
			bitmap->FileSystemRingtone 	= true;
		}
	} else {
		DecodeHexBin (&bitmap->RingtoneID, readvalue, 2);
		bitmap->DefaultRingtone 	= false;
		bitmap->FileSystemRingtone 	= false;
	}
	sprintf(buffer,"Enabled");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
        bitmap->BitmapEnabled = true;
	if (readvalue!=NULL) {
		if (mystrncasecmp(readvalue,"False",0)) bitmap->BitmapEnabled = false;
	}
}

static void ReadStartupEntry(INI_Section *file_info, char *section, GSM_Bitmap *bitmap, bool UseUnicode)
{
	unsigned char buffer[10000];

	sprintf(buffer,"Text");
	ReadBackupText(file_info, section, buffer, bitmap->Text,UseUnicode);
	if (bitmap->Text[0]!=0 || bitmap->Text[1]!=0) {
		bitmap->Type = GSM_WelcomeNote_Text;
	} else {
		bitmap->Type 	 = GSM_StartupLogo;
		bitmap->Location = 1;
		ReadBitmapEntry(file_info, section, bitmap, UseUnicode);
#ifdef DEBUG
		if (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE) GSM_PrintBitmap(di.df,bitmap);
#endif
	}
}

static void ReadWAPBookmarkEntry(INI_Section *file_info, char *section, GSM_WAPBookmark *bookmark, bool UseUnicode)
{
	unsigned char		buffer[10000];

	sprintf(buffer,"URL");
	ReadBackupText(file_info, section, buffer, bookmark->Address,UseUnicode);
	sprintf(buffer,"Title");
	ReadBackupText(file_info, section, buffer, bookmark->Title,UseUnicode);
}

static void ReadOperatorEntry(INI_Section *file_info, char *section, GSM_Bitmap *bitmap, bool UseUnicode)
{
	unsigned char		buffer[10000];
	char			*readvalue;

	sprintf(buffer,"Network");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	memcpy(bitmap->NetworkCode, readvalue + 1, 6);
	bitmap->NetworkCode[6] = 0;
	bitmap->Type = GSM_OperatorLogo;
	ReadBitmapEntry(file_info, section, bitmap, UseUnicode);
}

static void ReadSMSCEntry(INI_Section *file_info, char *section, GSM_SMSC *SMSC, bool UseUnicode)
{
	unsigned char		buffer[10000];
	char			*readvalue;

	sprintf(buffer,"Name");
	ReadBackupText(file_info, section, buffer, SMSC->Name,UseUnicode);
	sprintf(buffer,"Number");
	ReadBackupText(file_info, section, buffer, SMSC->Number,UseUnicode);
	sprintf(buffer,"DefaultNumber");
	ReadBackupText(file_info, section, buffer, SMSC->DefaultNumber,UseUnicode);
	sprintf(buffer,"Format");
	SMSC->Format = SMS_FORMAT_Text;
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		if (mystrncasecmp(readvalue,"Fax",0)) {
			SMSC->Format = SMS_FORMAT_Fax;
		} else if (mystrncasecmp(readvalue,"Email",0)) {
			SMSC->Format = SMS_FORMAT_Email;
		} else if (mystrncasecmp(readvalue,"Pager",0)) {
			SMSC->Format = SMS_FORMAT_Pager;
		}
	}
	sprintf(buffer,"Validity");
	SMSC->Validity.Relative = SMS_VALID_Max_Time;
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		if (mystrncasecmp(readvalue,"1hour",0)) {
			SMSC->Validity.Relative = SMS_VALID_1_Hour;
		} else if (mystrncasecmp(readvalue,"6hours",0)) {
			SMSC->Validity.Relative = SMS_VALID_6_Hours;
		} else if (mystrncasecmp(readvalue,"24hours",0)) {
			SMSC->Validity.Relative = SMS_VALID_1_Day;
		} else if (mystrncasecmp(readvalue,"72hours",0)) {
			SMSC->Validity.Relative = SMS_VALID_3_Days;
		} else if (mystrncasecmp(readvalue,"1week",0)) {
			SMSC->Validity.Relative = SMS_VALID_1_Week;
		}
	}
}

static void ReadWAPSettingsEntry(INI_Section *file_info, char *section, GSM_MultiWAPSettings *settings, bool UseUnicode)
{
	unsigned char		buffer[10000], *readvalue;
	int			num;
	INI_Entry		*e;

	settings->ActiveBearer = WAPSETTINGS_BEARER_DATA;
	sprintf(buffer,"Bearer");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		if (mystrncasecmp(readvalue,"SMS",0)) {
			settings->ActiveBearer = WAPSETTINGS_BEARER_SMS;
		} else if (mystrncasecmp(readvalue,"GPRS",0)) {
			settings->ActiveBearer = WAPSETTINGS_BEARER_GPRS;
		} else if (mystrncasecmp(readvalue,"USSD",0)) {
			settings->ActiveBearer = WAPSETTINGS_BEARER_USSD;
		}
	}

	settings->Active = false;
	sprintf(buffer,"Active");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		if (mystrncasecmp(readvalue,"Yes",0)) settings->Active = true;
	}

	settings->ReadOnly = false;
	sprintf(buffer,"ReadOnly");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		if (mystrncasecmp(readvalue,"Yes",0)) settings->ReadOnly = true;
	}

	sprintf(buffer,"Proxy");
	ReadBackupText(file_info, section, buffer, settings->Proxy,UseUnicode);
	sprintf(buffer,"ProxyPort");
	settings->ProxyPort = 8080;
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) settings->ProxyPort = atoi(readvalue);
	sprintf(buffer,"Proxy2");
	ReadBackupText(file_info, section, buffer, settings->Proxy2,UseUnicode);
	sprintf(buffer,"Proxy2Port");
	settings->Proxy2Port = 8080;
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) settings->Proxy2Port = atoi(readvalue);

	settings->Number = 0;
	e = INI_FindLastSectionEntry(file_info, section, UseUnicode);
	while (e != NULL) {
		num = -1;
		if (UseUnicode) {
			sprintf(buffer,"%s",DecodeUnicodeString(e->EntryName));
		} else {
			sprintf(buffer,"%s",e->EntryName);
		}
		if (strlen(buffer) == 7) {
			if (mystrncasecmp("Title", buffer,5)) num = atoi(buffer+5);
		}
		e = e->Prev;
		if (num != -1) {
			sprintf(buffer,"Title%02i",num);
			ReadBackupText(file_info, section, buffer, settings->Settings[settings->Number].Title,UseUnicode);
			sprintf(buffer,"HomePage%02i",num);
			ReadBackupText(file_info, section, buffer, settings->Settings[settings->Number].HomePage,UseUnicode);
			sprintf(buffer,"Type%02i",num);
			settings->Settings[settings->Number].IsContinuous = true;
			readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
			if (readvalue!=NULL) {
				if (mystrncasecmp(readvalue,"Temporary",0)) settings->Settings[settings->Number].IsContinuous = false;
			}
			sprintf(buffer,"Security%02i",num);
			settings->Settings[settings->Number].IsSecurity = true;
			readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
			if (readvalue!=NULL) 
			{
				if (mystrncasecmp(readvalue,"Off",0)) settings->Settings[settings->Number].IsSecurity = false;
			}
			sprintf(buffer,"Bearer%02i",num);
			readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
			if (readvalue!=NULL) 
			{
				if (mystrncasecmp(readvalue,"SMS",0)) {
					settings->Settings[settings->Number].Bearer = WAPSETTINGS_BEARER_SMS;
					sprintf(buffer,"Server%02i",num);
					ReadBackupText(file_info, section, buffer, settings->Settings[settings->Number].Server,UseUnicode);
					sprintf(buffer,"Service%02i",num);
					ReadBackupText(file_info, section, buffer, settings->Settings[settings->Number].Service,UseUnicode);
				} else if ((mystrncasecmp(readvalue,"Data",0) || mystrncasecmp(readvalue,"GPRS",0))) {
					settings->Settings[settings->Number].Bearer = WAPSETTINGS_BEARER_DATA;
					if (mystrncasecmp(readvalue,"GPRS",0)) settings->Settings[settings->Number].Bearer = WAPSETTINGS_BEARER_GPRS;
					sprintf(buffer,"Number%02i",num);
					ReadBackupText(file_info, section, buffer, settings->Settings[settings->Number].DialUp,UseUnicode);
					sprintf(buffer,"IP%02i",num);
					ReadBackupText(file_info, section, buffer, settings->Settings[settings->Number].IPAddress,UseUnicode);
					sprintf(buffer,"User%02i",num);
					ReadBackupText(file_info, section, buffer, settings->Settings[settings->Number].User,UseUnicode);
					sprintf(buffer,"Password%02i",num);
					ReadBackupText(file_info, section, buffer, settings->Settings[settings->Number].Password,UseUnicode);
					sprintf(buffer,"Authentication%02i",num);
					settings->Settings[settings->Number].IsNormalAuthentication = true;
					readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
					if (readvalue!=NULL) 
					{
						if (mystrncasecmp(readvalue,"Secure",0)) settings->Settings[settings->Number].IsNormalAuthentication = false;
					}
					sprintf(buffer,"CallSpeed%02i",num);
					settings->Settings[settings->Number].Speed = WAPSETTINGS_SPEED_14400;
					readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
					if (readvalue!=NULL) 
					{
						if (mystrncasecmp(readvalue,"9600",0)) settings->Settings[settings->Number].Speed = WAPSETTINGS_SPEED_9600;
						if (mystrncasecmp(readvalue,"auto",0)) settings->Settings[settings->Number].Speed = WAPSETTINGS_SPEED_AUTO;
					}
					sprintf(buffer,"Login%02i",num);
					settings->Settings[settings->Number].ManualLogin = false;
					readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
					if (readvalue!=NULL) 
					{
						if (mystrncasecmp(readvalue,"Manual",0)) settings->Settings[settings->Number].ManualLogin = true;
					}	
					sprintf(buffer,"CallType%02i",num);
					settings->Settings[settings->Number].IsISDNCall = true;
					readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
					if (readvalue!=NULL) 
					{
						if (mystrncasecmp(readvalue,"Analogue",0)) settings->Settings[settings->Number].IsISDNCall = false;
					}
				} else if (mystrncasecmp(readvalue,"USSD",0)) {
					settings->Settings[settings->Number].Bearer = WAPSETTINGS_BEARER_USSD;
					sprintf(buffer,"ServiceCode%02i",num);
					ReadBackupText(file_info, section, buffer, settings->Settings[settings->Number].Code,UseUnicode);
					sprintf(buffer,"IP%02i",num);
					readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
					if (readvalue!=NULL) {
						settings->Settings[settings->Number].IsIP = true;
						sprintf(buffer,"IP%02i",num);
					} else {
						settings->Settings[settings->Number].IsIP = false;
						sprintf(buffer,"Number%02i",num);
					}
					ReadBackupText(file_info, section, buffer, settings->Settings[settings->Number].Service,UseUnicode);
				}
			}
			settings->Number++;
		}
	}
}

static void ReadRingtoneEntry(INI_Section *file_info, char *section, GSM_Ringtone *ringtone, bool UseUnicode)
{
	unsigned char buffer[10000], buffer2[10000], *readvalue;

	sprintf(buffer,"Name");
	ReadBackupText(file_info, section, buffer, ringtone->Name,UseUnicode);
	ringtone->Location = 0;
	sprintf(buffer,"Location");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) ringtone->Location = atoi(readvalue);
	sprintf(buffer,"NokiaBinary00");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		ringtone->Format = RING_NOKIABINARY;
		ReadLinkedBackupText(file_info, section, "NokiaBinary", buffer2, UseUnicode);
		DecodeHexBin (ringtone->NokiaBinary.Frame, buffer2, strlen(buffer2));
		ringtone->NokiaBinary.Length = strlen(buffer2)/2;
	}
	sprintf(buffer,"Pure Midi00");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		ringtone->Format = RING_MIDI;
		ReadLinkedBackupText(file_info, section, "Pure Midi", buffer2, UseUnicode);
		DecodeHexBin (ringtone->NokiaBinary.Frame, buffer2, strlen(buffer2));
		ringtone->NokiaBinary.Length = strlen(buffer2)/2;
	}

}

static void ReadProfileEntry(INI_Section *file_info, char *section, GSM_Profile *Profile, bool UseUnicode)
{
	unsigned char		buffer[10000];
	char			*readvalue;
	bool			unknown;
	int			num,j;
	INI_Entry		*e;

	sprintf(buffer,"Name");
	ReadBackupText(file_info, section, buffer, Profile->Name,UseUnicode);

	sprintf(buffer,"Location");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	Profile->Location = atoi(readvalue);

	Profile->DefaultName = false;
	sprintf(buffer,"DefaultName");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL && mystrncasecmp(buffer,"true",0)) Profile->DefaultName = true;

	Profile->HeadSetProfile = false;
	sprintf(buffer,"HeadSetProfile");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL && mystrncasecmp(buffer,"true",0)) Profile->HeadSetProfile = true;

	Profile->CarKitProfile = false;
	sprintf(buffer,"CarKitProfile");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL && mystrncasecmp(buffer,"true",0)) Profile->CarKitProfile = true;

	Profile->FeaturesNumber = 0;
	e = INI_FindLastSectionEntry(file_info, section, UseUnicode);
	while (e != NULL) {
		num = -1;
		if (UseUnicode) {
			sprintf(buffer,"%s",DecodeUnicodeString(e->EntryName));
		} else {
			sprintf(buffer,"%s",e->EntryName);
		}
		if (strlen(buffer) == 9) {
			if (mystrncasecmp("Feature", buffer, 7)) num = atoi(buffer+7);
		}
		e = e->Prev;
		if (num != -1) {
			sprintf(buffer,"Feature%02i",num);
			readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
			if (readvalue==NULL) break;
			unknown = true;
			if (mystrncasecmp(readvalue,"RingtoneID",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_RingtoneID;
				sprintf(buffer,"Value%02i",num);
				readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
				Profile->FeatureValue[Profile->FeaturesNumber]=atoi(readvalue);
				Profile->FeaturesNumber++;
			} else if (mystrncasecmp(readvalue,"MessageToneID",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_MessageToneID;
				sprintf(buffer,"Value%02i",num);
				readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
				Profile->FeatureValue[Profile->FeaturesNumber]=atoi(readvalue);
				Profile->FeaturesNumber++;
			} else if (mystrncasecmp(readvalue,"ScreenSaverNumber",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_ScreenSaverNumber;
				sprintf(buffer,"Value%02i",num);
				readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
				Profile->FeatureValue[Profile->FeaturesNumber]=atoi(readvalue);
				Profile->FeaturesNumber++;
			} else if (mystrncasecmp(readvalue,"CallerGroups",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_CallerGroups;
				sprintf(buffer,"Value%02i",num);
				readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
				for (j=0;j<5;j++) {
					Profile->CallerGroups[j]=false;
					if (strstr(readvalue,"1"+j)!=NULL) Profile->CallerGroups[j]=true;
				}
				Profile->FeaturesNumber++;
			} else if (mystrncasecmp(readvalue,"IncomingCallAlert",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_CallAlert;
				unknown = false;
			} else if (mystrncasecmp(readvalue,"RingtoneVolume",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_RingtoneVolume;
				unknown = false;
			} else if (mystrncasecmp(readvalue,"Vibrating",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_Vibration;
				unknown = false;
			} else if (mystrncasecmp(readvalue,"MessageTone",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_MessageTone;
				unknown = false;
			} else if (mystrncasecmp(readvalue,"KeypadTones",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_KeypadTone;
				unknown = false;
			} else if (mystrncasecmp(readvalue,"WarningTones",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_WarningTone;
				unknown = false;
			} else if (mystrncasecmp(readvalue,"ScreenSaver",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_ScreenSaver;
				unknown = false;
			} else if (mystrncasecmp(readvalue,"ScreenSaverTimeout",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_ScreenSaverTime;
				unknown = false;
			} else if (mystrncasecmp(readvalue,"AutomaticAnswer",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_AutoAnswer;
				unknown = false;
			} else if (mystrncasecmp(readvalue,"Lights",0)) {
				Profile->FeatureID[Profile->FeaturesNumber]=Profile_Lights;
				unknown = false;
			}
			if (!unknown) {
				sprintf(buffer,"Value%02i",num);
				readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
				if (mystrncasecmp(readvalue,"Level1",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_VOLUME_LEVEL1;
					if (Profile->FeatureID[Profile->FeaturesNumber]==Profile_KeypadTone) {
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_KEYPAD_LEVEL1;
					}
				} else if (mystrncasecmp(readvalue,"Level2",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_VOLUME_LEVEL2;
					if (Profile->FeatureID[Profile->FeaturesNumber]==Profile_KeypadTone) {
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_KEYPAD_LEVEL2;
					}
				} else if (mystrncasecmp(readvalue,"Level3",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_VOLUME_LEVEL3;
					if (Profile->FeatureID[Profile->FeaturesNumber]==Profile_KeypadTone) {
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_KEYPAD_LEVEL3;
					}
				} else if (mystrncasecmp(readvalue,"Level4",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_VOLUME_LEVEL4;
				} else if (mystrncasecmp(readvalue,"Level5",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_VOLUME_LEVEL5;
				} else if (mystrncasecmp(readvalue,"Off",0)) {
					switch (Profile->FeatureID[Profile->FeaturesNumber]) {
					case Profile_MessageTone:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_MESSAGE_NOTONE;
						break;
					case Profile_AutoAnswer:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_AUTOANSWER_OFF;
						break;
					case Profile_Lights:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_LIGHTS_OFF;
						break;
					case Profile_ScreenSaver:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_SAVER_OFF;
						break;
					case Profile_WarningTone:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_WARNING_OFF;
						break;
					case Profile_CallAlert:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_CALLALERT_OFF;
						break;
					case Profile_Vibration:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_VIBRATION_OFF;
						break;
					default:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_KEYPAD_OFF;
						break;
					}
				} else if (mystrncasecmp(readvalue,"Ringing",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_CALLALERT_RINGING;
				} else if (mystrncasecmp(readvalue,"BeepOnce",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_CALLALERT_BEEPONCE;
					if (Profile->FeatureID[Profile->FeaturesNumber]==Profile_MessageTone) {
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_MESSAGE_BEEPONCE;
					}
				} else if (mystrncasecmp(readvalue,"RingOnce",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_CALLALERT_RINGONCE;
				} else if (mystrncasecmp(readvalue,"Ascending",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_CALLALERT_ASCENDING;
				} else if (mystrncasecmp(readvalue,"CallerGroups",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_CALLALERT_CALLERGROUPS;
				} else if (mystrncasecmp(readvalue,"Standard",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_MESSAGE_STANDARD;
				} else if (mystrncasecmp(readvalue,"Special",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_MESSAGE_SPECIAL;
				} else if (mystrncasecmp(readvalue,"Ascending",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_MESSAGE_ASCENDING;
				} else if (mystrncasecmp(readvalue,"Personal",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_MESSAGE_PERSONAL;
				} else if (mystrncasecmp(readvalue,"VibrateFirst",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_VIBRATION_FIRST;
				} else if (mystrncasecmp(readvalue,"Auto",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_LIGHTS_AUTO;
				} else if (mystrncasecmp(readvalue,"5Seconds",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_SAVER_TIMEOUT_5SEC;
				} else if (mystrncasecmp(readvalue,"20Seconds",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_SAVER_TIMEOUT_20SEC;
				} else if (mystrncasecmp(readvalue,"1Minute",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_SAVER_TIMEOUT_1MIN;
				} else if (mystrncasecmp(readvalue,"2Minutes",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_SAVER_TIMEOUT_2MIN;
				} else if (mystrncasecmp(readvalue,"5Minutes",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_SAVER_TIMEOUT_5MIN;
				} else if (mystrncasecmp(readvalue,"10Minutes",0)) {
					Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_SAVER_TIMEOUT_10MIN;
				} else if (mystrncasecmp(readvalue,"On",0)) {
					switch (Profile->FeatureID[Profile->FeaturesNumber]) {
					case Profile_AutoAnswer:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_AUTOANSWER_ON;
						break;
					case Profile_WarningTone:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_WARNING_ON;
						break;
					case Profile_ScreenSaver:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_SAVER_ON;
						break;
					default:
						Profile->FeatureValue[Profile->FeaturesNumber]=PROFILE_VIBRATION_ON;
						break;
					}
				} else unknown = true;
			}
			if (!unknown) Profile->FeaturesNumber++;
		}
	}
}

static void ReadFMStationEntry(INI_Section *file_info, char *section, GSM_FMStation *FMStation, bool UseUnicode)
{
	unsigned char buffer[10000], *readvalue;

	FMStation->Location  = 0;
	FMStation->Frequency = 0;

	sprintf(buffer,"Location");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) FMStation->Location = atoi(readvalue);

	sprintf(buffer,"StationName");
	ReadBackupText(file_info, section, buffer, FMStation->StationName,UseUnicode);

	sprintf(buffer,"Frequency");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) StringToDouble(readvalue, &FMStation->Frequency);
}

static void ReadGPRSPointEntry(INI_Section *file_info, char *section, GSM_GPRSAccessPoint *GPRSPoint, bool UseUnicode)
{
	unsigned char buffer[10000], *readvalue;

	GPRSPoint->Name[0]  = 0;
	GPRSPoint->Name[1]  = 0;
	GPRSPoint->URL[0]   = 0;
	GPRSPoint->URL[1]   = 0;
	GPRSPoint->Location = 0;

	GPRSPoint->Active = false;
	sprintf(buffer,"Active");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) {
		if (mystrncasecmp(readvalue,"Yes",0)) GPRSPoint->Active = true;
	}

	sprintf(buffer,"Location");
	readvalue = ReadCFGText(file_info, section, buffer, UseUnicode);
	if (readvalue!=NULL) GPRSPoint->Location = atoi(readvalue);

	sprintf(buffer,"Name");
	ReadBackupText(file_info, section, buffer, GPRSPoint->Name,UseUnicode);

	sprintf(buffer,"URL");
	ReadBackupText(file_info, section, buffer, GPRSPoint->URL,UseUnicode);
}

static void ReadNoteEntry(INI_Section *file_info, char *section, GSM_NoteEntry *Note, bool UseUnicode)
{
	unsigned char buffer[100];

	sprintf(buffer,"Text");
	ReadBackupText(file_info, section, buffer, Note->Text,UseUnicode);
}

GSM_Error LoadBackup(char *FileName, GSM_Backup *backup, bool UseUnicode)
{
	INI_Section		*file_info, *h;
	char			buffer[100], *readvalue;
	int			num;
	GSM_MemoryEntry 	PBK;
	bool			found;

	file_info = INI_ReadFile(FileName, UseUnicode);

	sprintf(buffer,"Backup");
	if (UseUnicode) EncodeUnicode(buffer,"Backup",6);

	readvalue = ReadCFGText(file_info, buffer, "Format", UseUnicode);
	/* Did we read anything? */
	if (readvalue == NULL) return ERR_FILENOTSUPPORTED;
	/* Is this format version supported ? */
	if (strcmp(readvalue,"1.01")!=0 && strcmp(readvalue,"1.02")!=0 &&
            strcmp(readvalue,"1.03")!=0) return ERR_FILENOTSUPPORTED;

	readvalue = ReadCFGText(file_info, buffer, "IMEI", UseUnicode);
	if (readvalue!=NULL) strcpy(backup->IMEI,readvalue);
	readvalue = ReadCFGText(file_info, buffer, "Phone", UseUnicode);
	if (readvalue!=NULL) strcpy(backup->Model,readvalue);
	readvalue = ReadCFGText(file_info, buffer, "Creator", UseUnicode);
	if (readvalue!=NULL) strcpy(backup->Creator,readvalue);
	readvalue = ReadCFGText(file_info, buffer, "DateTime", UseUnicode);
	if (readvalue!=NULL) {
		ReadVCALDateTime(readvalue, &backup->DateTime);
		backup->DateTimeAvailable = true;
	}

	sprintf(buffer,"Checksum");
	if (UseUnicode) EncodeUnicode(buffer,"Checksum",8);
	readvalue = ReadCFGText(file_info, buffer, "MD5", UseUnicode);
	if (readvalue!=NULL) strcpy(backup->MD5Original,readvalue);

	num = 0;
        for (h = file_info; h != NULL; h = h->Next) {
		found = false;
		if (UseUnicode) {
			EncodeUnicode(buffer,"Profile",7);
			if (mywstrncasecmp(buffer, h->SectionName, 7)) found = true;
		} else {
	                if (mystrncasecmp("Profile", h->SectionName, 7)) found = true;
		}
		if (found) {
			readvalue = ReadCFGText(file_info, h->SectionName, "Location", UseUnicode);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_PROFILES) {
				backup->Profiles[num] = malloc(sizeof(GSM_Profile));
			        if (backup->Profiles[num] == NULL) return ERR_MOREMEMORY;
				backup->Profiles[num + 1] = NULL;
			} else {
				dbgprintf("Increase GSM_BACKUP_MAX_PROFILES\n");
				return ERR_MOREMEMORY;
			}
			ReadProfileEntry(file_info, h->SectionName, backup->Profiles[num], UseUnicode);
			num++;
                }
        }
	num = 0;
        for (h = file_info; h != NULL; h = h->Next) {
		found = false;
		if (UseUnicode) {
			EncodeUnicode(buffer,"PhonePBK",8);
			if (mywstrncasecmp(buffer, h->SectionName, 8)) found = true;
		} else {
	                if (mystrncasecmp("PhonePBK", h->SectionName, 8)) found = true;
		}
                if (found) {
			readvalue = ReadCFGText(file_info, h->SectionName, "Location", UseUnicode);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_PHONEPHONEBOOK) {
				backup->PhonePhonebook[num] = malloc(sizeof(GSM_MemoryEntry));
			        if (backup->PhonePhonebook[num] == NULL) return ERR_MOREMEMORY;
				backup->PhonePhonebook[num + 1] = NULL;
			} else {
				dbgprintf("Increase GSM_BACKUP_MAX_PHONEPHONEBOOK\n");
				return ERR_MOREMEMORY;
			}
			backup->PhonePhonebook[num]->Location	= atoi (readvalue);
			backup->PhonePhonebook[num]->MemoryType	= MEM_ME;
			ReadPbkEntry(file_info, h->SectionName, backup->PhonePhonebook[num],UseUnicode);
			dbgprintf("number of entries = %i\n",backup->PhonePhonebook[num]->EntriesNum);
			num++;
                }
        }
	num = 0;
	while (0) {
		if (backup->PhonePhonebook[num] == NULL) break;
		if (backup->PhonePhonebook[num+1] != NULL) {
			if (backup->PhonePhonebook[num+1]->Location < backup->PhonePhonebook[num]->Location) {
				memcpy(&PBK,backup->PhonePhonebook[num+1],sizeof(GSM_MemoryEntry));
				memcpy(backup->PhonePhonebook[num+1],backup->PhonePhonebook[num],sizeof(GSM_MemoryEntry));
				memcpy(backup->PhonePhonebook[num],&PBK,sizeof(GSM_MemoryEntry));
				num = 0;
				continue;
			}
		}
		num++;
	}
	num = 0;
        for (h = file_info; h != NULL; h = h->Next) {
		found = false;
		if (UseUnicode) {
			EncodeUnicode(buffer,"SIMPBK",6);
			if (mywstrncasecmp(buffer, h->SectionName, 6)) found = true;
		} else {
	                if (mystrncasecmp("SIMPBK", h->SectionName, 6)) found = true;
		}
                if (found) {
			readvalue = ReadCFGText(file_info, h->SectionName, "Location", UseUnicode);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_SIMPHONEBOOK) {
				backup->SIMPhonebook[num] = malloc(sizeof(GSM_MemoryEntry));
			        if (backup->SIMPhonebook[num] == NULL) return ERR_MOREMEMORY;
				backup->SIMPhonebook[num + 1] = NULL;
			} else {
				dbgprintf("Increase GSM_BACKUP_MAX_SIMPHONEBOOK\n");
				return ERR_MOREMEMORY;
			}
			backup->SIMPhonebook[num]->Location	= atoi (readvalue);
			backup->SIMPhonebook[num]->MemoryType	= MEM_SM;
			ReadPbkEntry(file_info, h->SectionName, backup->SIMPhonebook[num],UseUnicode);
			num++;
                }
        }
	num = 0;
	while (0) {
		if (backup->SIMPhonebook[num] == NULL) break;
		if (backup->SIMPhonebook[num+1] != NULL) {
			if (backup->SIMPhonebook[num+1]->Location < backup->SIMPhonebook[num]->Location) {
				memcpy(&PBK,backup->SIMPhonebook[num+1],sizeof(GSM_MemoryEntry));
				memcpy(backup->SIMPhonebook[num+1],backup->SIMPhonebook[num],sizeof(GSM_MemoryEntry));
				memcpy(backup->SIMPhonebook[num],&PBK,sizeof(GSM_MemoryEntry));
				num = 0;
				continue;
			}
		}
		num++;
	}
	num = 0;
        for (h = file_info; h != NULL; h = h->Next) {
		found = false;
		if (UseUnicode) {
			EncodeUnicode(buffer,"Calendar",8);
			if (mywstrncasecmp(buffer, h->SectionName, 8)) found = true;
		} else {
	                if (mystrncasecmp("Calendar", h->SectionName, 8)) found = true;
		}
                if (found) {
			readvalue = ReadCFGText(file_info, h->SectionName, "Type", UseUnicode);
			if (readvalue==NULL) break;
			if (num < GSM_MAXCALENDARTODONOTES) {
				backup->Calendar[num] = malloc(sizeof(GSM_CalendarEntry));
			        if (backup->Calendar[num] == NULL) return ERR_MOREMEMORY;
				backup->Calendar[num + 1] = NULL;
			} else {
				dbgprintf("Increase GSM_MAXCALENDARTODONOTES\n");
				return ERR_MOREMEMORY;
			}
			backup->Calendar[num]->Location = num + 1;
			ReadCalendarEntry(file_info, h->SectionName, backup->Calendar[num],UseUnicode);
			num++;
                }
        }
	num = 0;
        for (h = file_info; h != NULL; h = h->Next) {
		found = false;
		if (UseUnicode) {
			EncodeUnicode(buffer,"Caller",6);
			if (mywstrncasecmp(buffer, h->SectionName, 6)) found = true;
		} else {
	                if (mystrncasecmp("Caller", h->SectionName, 6)) found = true;
		}
                if (found) {
			readvalue = ReadCFGText(file_info, h->SectionName, "Location", UseUnicode);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_CALLER) {
				backup->CallerLogos[num] = malloc(sizeof(GSM_Bitmap));
			        if (backup->CallerLogos[num] == NULL) return ERR_MOREMEMORY;
				backup->CallerLogos[num + 1] = NULL;
			} else {
				dbgprintf("Increase GSM_BACKUP_MAX_CALLER\n");
				return ERR_MOREMEMORY;
			}
			backup->CallerLogos[num]->Location = atoi (readvalue);
			ReadCallerEntry(file_info, h->SectionName, backup->CallerLogos[num],UseUnicode);
			num++;
                }
        }
	num = 0;
        for (h = file_info; h != NULL; h = h->Next) {
		found = false;
		if (UseUnicode) {
			EncodeUnicode(buffer,"SMSC",4);
			if (mywstrncasecmp(buffer, h->SectionName, 4)) found = true;
		} else {
	                if (mystrncasecmp("SMSC", h->SectionName, 4)) found = true;
		}
                if (found) {
			readvalue = ReadCFGText(file_info, h->SectionName, "Location", UseUnicode);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_SMSC) {
				backup->SMSC[num] = malloc(sizeof(GSM_SMSC));
			        if (backup->SMSC[num] == NULL) return ERR_MOREMEMORY;
				backup->SMSC[num + 1] = NULL;
			} else {
				dbgprintf("Increase GSM_BACKUP_MAX_SMSC\n");
				return ERR_MOREMEMORY;
			}
			backup->SMSC[num]->Location = atoi (readvalue);
			ReadSMSCEntry(file_info, h->SectionName, backup->SMSC[num],UseUnicode);
			num++;
                }
        }
	num = 0;
        for (h = file_info; h != NULL; h = h->Next) {
		found = false;
		if (UseUnicode) {
			EncodeUnicode(buffer,"WAPBookmark",11);
			if (mywstrncasecmp(buffer, h->SectionName, 11)) found = true;
			if (!found) {
				EncodeUnicode(buffer,"Bookmark",8);
				if (mywstrncasecmp(buffer, h->SectionName, 8)) found = true;
			}
		} else {
	                if (mystrncasecmp("WAPBookmark", h->SectionName, 11)) found = true;
			if (!found) {
				if (mystrncasecmp("Bookmark", h->SectionName, 8)) found = true;
			}
		}
                if (found) {
			readvalue = ReadCFGText(file_info, h->SectionName, "URL", UseUnicode);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_WAPBOOKMARK) {
				backup->WAPBookmark[num] = malloc(sizeof(GSM_WAPBookmark));
			        if (backup->WAPBookmark[num] == NULL) return ERR_MOREMEMORY;
				backup->WAPBookmark[num + 1] = NULL;
			} else {
				dbgprintf("Increase GSM_BACKUP_MAX_WAPBOOKMARK\n");
				return ERR_MOREMEMORY;
			}
			backup->WAPBookmark[num]->Location = num + 1;
			ReadWAPBookmarkEntry(file_info, h->SectionName, backup->WAPBookmark[num],UseUnicode);
			num++;
                }
        }
	num = 0;
        for (h = file_info; h != NULL; h = h->Next) {
		found = false;
		if (UseUnicode) {
			EncodeUnicode(buffer,"WAPSettings",11);
			if (mywstrncasecmp(buffer, h->SectionName, 11)) found = true;
			if (!found) {
				EncodeUnicode(buffer,"Settings",8);
				if (mywstrncasecmp(buffer, h->SectionName, 8)) found = true;
			}
		} else {
	                if (mystrncasecmp("WAPSettings", h->SectionName, 11)) found = true;
			if (!found) {
		                if (mystrncasecmp("Settings", h->SectionName, 8)) found = true;
			}
		}
                if (found) {
			readvalue = ReadCFGText(file_info, h->SectionName, "Title00", UseUnicode);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_WAPSETTINGS) {
				backup->WAPSettings[num] = malloc(sizeof(GSM_MultiWAPSettings));
			        if (backup->WAPSettings[num] == NULL) return ERR_MOREMEMORY;
				backup->WAPSettings[num + 1] = NULL;
			} else {
				dbgprintf("Increase GSM_BACKUP_MAX_WAPSETTINGS\n");
				return ERR_MOREMEMORY;
			}
			backup->WAPSettings[num]->Location = num + 1;
			dbgprintf("reading wap settings\n");
			ReadWAPSettingsEntry(file_info, h->SectionName, backup->WAPSettings[num],UseUnicode);
			num++;
                }
        }
	num = 0;
        for (h = file_info; h != NULL; h = h->Next) {
		found = false;
		if (UseUnicode) {
			EncodeUnicode(buffer,"MMSSettings",8);
			if (mywstrncasecmp(buffer, h->SectionName, 8)) found = true;
		} else {
	                if (mystrncasecmp("MMSSettings", h->SectionName, 8)) found = true;
		}
                if (found) {
			readvalue = ReadCFGText(file_info, h->SectionName, "Title00", UseUnicode);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_MMSSETTINGS) {
				backup->MMSSettings[num] = malloc(sizeof(GSM_MultiWAPSettings));
			        if (backup->MMSSettings[num] == NULL) return ERR_MOREMEMORY;
				backup->MMSSettings[num + 1] = NULL;
			} else {
				dbgprintf("Increase GSM_BACKUP_MAX_MMSSETTINGS\n");
				return ERR_MOREMEMORY;
			}
			backup->MMSSettings[num]->Location = num + 1;
			dbgprintf("reading mms settings\n");
			ReadWAPSettingsEntry(file_info, h->SectionName, backup->MMSSettings[num],UseUnicode);
			num++;
                }
        }
	num = 0;
        for (h = file_info; h != NULL; h = h->Next) {
		found = false;
		if (UseUnicode) {
			EncodeUnicode(buffer,"Ringtone",8);
			if (mywstrncasecmp(buffer, h->SectionName, 8)) found = true;
		} else {
	                if (mystrncasecmp("Ringtone", h->SectionName, 8)) found = true;
		}
                if (found) {
			readvalue = ReadCFGText(file_info, h->SectionName, "Location", UseUnicode);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_RINGTONES) {
				backup->Ringtone[num] = malloc(sizeof(GSM_Ringtone));
			        if (backup->Ringtone[num] == NULL) return ERR_MOREMEMORY;
				backup->Ringtone[num + 1] = NULL;
			} else {
				dbgprintf("Increase GSM_BACKUP_MAX_RINGTONES\n");
				return ERR_MOREMEMORY;
			}
			ReadRingtoneEntry(file_info, h->SectionName, backup->Ringtone[num],UseUnicode);
			num++;
                }
        }
	num = 0;
        for (h = file_info; h != NULL; h = h->Next) {
		found = false;
		if (UseUnicode) {
			EncodeUnicode(buffer,"TODO",4);
			if (mywstrncasecmp(buffer, h->SectionName, 4)) found = true;
		} else {
	                if (mystrncasecmp("TODO", h->SectionName, 4)) found = true;
		}
                if (found) {
			readvalue = ReadCFGText(file_info, h->SectionName, "Location", UseUnicode);
			if (readvalue==NULL) break;
			if (num < GSM_MAXCALENDARTODONOTES) {
				backup->ToDo[num] = malloc(sizeof(GSM_ToDoEntry));
			        if (backup->ToDo[num] == NULL) return ERR_MOREMEMORY;
				backup->ToDo[num + 1] = NULL;
			} else {
				dbgprintf("Increase GSM_MAXCALENDARTODONOTES\n");
				return ERR_MOREMEMORY;
			}
			backup->ToDo[num]->Location = num + 1;
			ReadToDoEntry(file_info, h->SectionName, backup->ToDo[num],UseUnicode);
			num++;
                }
        }
	sprintf(buffer,"Startup");
	readvalue = ReadCFGText(file_info, buffer, "Text", UseUnicode);
	if (readvalue==NULL) {
		readvalue = ReadCFGText(file_info, buffer, "Width", UseUnicode);
	}
	if (readvalue!=NULL) {
		backup->StartupLogo = malloc(sizeof(GSM_Bitmap));
	        if (backup->StartupLogo == NULL) return ERR_MOREMEMORY;
		ReadStartupEntry(file_info, buffer, backup->StartupLogo,UseUnicode);
	}
	sprintf(buffer,"Operator");
	readvalue = ReadCFGText(file_info, buffer, "Network", UseUnicode);
	if (readvalue!=NULL) {
		backup->OperatorLogo = malloc(sizeof(GSM_Bitmap));
	        if (backup->OperatorLogo == NULL) return ERR_MOREMEMORY;
		ReadOperatorEntry(file_info, buffer, backup->OperatorLogo,UseUnicode);
	}
	num = 0;
        for (h = file_info; h != NULL; h = h->Next) {
		found = false;
		if (UseUnicode) {
			EncodeUnicode(buffer,"FMStation",9);
			if (mywstrncasecmp(buffer, h->SectionName, 9)) found = true;
		} else {
	                if (mystrncasecmp("FMStation", h->SectionName, 9)) found = true;
		}
                if (found) {
			readvalue = ReadCFGText(file_info, h->SectionName, "Location", UseUnicode);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_FMSTATIONS) {
				backup->FMStation[num] = malloc(sizeof(GSM_FMStation));
			        if (backup->FMStation[num] == NULL) return ERR_MOREMEMORY;
				backup->FMStation[num + 1] = NULL;
			} else {
				dbgprintf("Increase GSM_BACKUP_MAX_FMSTATIONS\n");
				return ERR_MOREMEMORY;
			}
			backup->FMStation[num]->Location = num + 1;
			ReadFMStationEntry(file_info, h->SectionName, backup->FMStation[num],UseUnicode);
			num++;
                }
        }
	num = 0;
        for (h = file_info; h != NULL; h = h->Next) {
		found = false;
		if (UseUnicode) {
			EncodeUnicode(buffer,"GPRSPoint",9);
			if (mywstrncasecmp(buffer, h->SectionName, 9)) found = true;
		} else {
	                if (mystrncasecmp("GPRSPoint", h->SectionName, 9)) found = true;
		}
                if (found) {
			readvalue = ReadCFGText(file_info, h->SectionName, "Location", UseUnicode);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_GPRSPOINT) {
				backup->GPRSPoint[num] = malloc(sizeof(GSM_GPRSAccessPoint));
			        if (backup->GPRSPoint[num] == NULL) return ERR_MOREMEMORY;
				backup->GPRSPoint[num + 1] = NULL;
			} else {
				dbgprintf("Increase GSM_BACKUP_MAX_GPRSPOINT\n");
				return ERR_MOREMEMORY;
			}
			backup->GPRSPoint[num]->Location = num + 1;
			ReadGPRSPointEntry(file_info, h->SectionName, backup->GPRSPoint[num],UseUnicode);
			num++;
                }
        }
	num = 0;
        for (h = file_info; h != NULL; h = h->Next) {
		found = false;
		if (UseUnicode) {
			EncodeUnicode(buffer,"Note",4);
			if (mywstrncasecmp(buffer, h->SectionName, 4)) found = true;
		} else {
	                if (mystrncasecmp("Note", h->SectionName, 4)) found = true;
		}
                if (found) {
			readvalue = ReadCFGText(file_info, h->SectionName, "Text", UseUnicode);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_NOTE) {
				backup->Note[num] = malloc(sizeof(GSM_NoteEntry));
			        if (backup->Note[num] == NULL) return ERR_MOREMEMORY;
				backup->Note[num + 1] = NULL;
			} else {
				dbgprintf("Increase GSM_BACKUP_MAX_NOTE\n");
				return ERR_MOREMEMORY;
			}
			ReadNoteEntry(file_info, h->SectionName, backup->Note[num],UseUnicode);
			num++;
                }
        }
	if (backup->MD5Original[0]!=0) {
		FindBackupChecksum(FileName, UseUnicode, backup->MD5Calculated);
	}
        for (h = file_info; h != NULL; h = h->Next) {
		found = false;
		if (UseUnicode) {
			EncodeUnicode(buffer,"Backup",4);
			if (mywstrncasecmp(buffer, h->SectionName, 6)) found = true;
		} else {
	                if (mystrncasecmp("Backup", h->SectionName, 6)) found = true;
		}
		if (UseUnicode) {
			EncodeUnicode(buffer,"Checksum",4);
			if (mywstrncasecmp(buffer, h->SectionName, 8)) found = true;
		} else {
	                if (mystrncasecmp("Checksum", h->SectionName, 8)) found = true;
		}
		if (UseUnicode) {
			EncodeUnicode(buffer,"Profile",7);
			if (mywstrncasecmp(buffer, h->SectionName, 7)) found = true;
		} else {
	                if (mystrncasecmp("Profile", h->SectionName, 7)) found = true;
		}
		if (UseUnicode) {
			EncodeUnicode(buffer,"PhonePBK",8);
			if (mywstrncasecmp(buffer, h->SectionName, 8)) found = true;
		} else {
	                if (mystrncasecmp("PhonePBK", h->SectionName, 8)) found = true;
		}
		if (UseUnicode) {
			EncodeUnicode(buffer,"SIMPBK",6);
			if (mywstrncasecmp(buffer, h->SectionName, 6)) found = true;
		} else {
	                if (mystrncasecmp("SIMPBK", h->SectionName, 6)) found = true;
		}
		if (UseUnicode) {
			EncodeUnicode(buffer,"Calendar",8);
			if (mywstrncasecmp(buffer, h->SectionName, 8)) found = true;
		} else {
	                if (mystrncasecmp("Calendar", h->SectionName, 8)) found = true;
		}
		if (UseUnicode) {
			EncodeUnicode(buffer,"Caller",6);
			if (mywstrncasecmp(buffer, h->SectionName, 6)) found = true;
		} else {
	                if (mystrncasecmp("Caller", h->SectionName, 6)) found = true;
		}
		if (UseUnicode) {
			EncodeUnicode(buffer,"SMSC",4);
			if (mywstrncasecmp(buffer, h->SectionName, 4)) found = true;
		} else {
	                if (mystrncasecmp("SMSC", h->SectionName, 4)) found = true;
		}
		if (UseUnicode) {
			EncodeUnicode(buffer,"WAPBookmark",11);
			if (mywstrncasecmp(buffer, h->SectionName, 11)) found = true;
			if (!found) {
				EncodeUnicode(buffer,"Bookmark",8);
				if (mywstrncasecmp(buffer, h->SectionName, 8)) found = true;
			}
		} else {
	                if (mystrncasecmp("WAPBookmark", h->SectionName, 11)) found = true;
			if (!found) {
				if (mystrncasecmp("Bookmark", h->SectionName, 8)) found = true;
			}
		}
		if (UseUnicode) {
			EncodeUnicode(buffer,"WAPSettings",11);
			if (mywstrncasecmp(buffer, h->SectionName, 11)) found = true;
			if (!found) {
				EncodeUnicode(buffer,"Settings",8);
				if (mywstrncasecmp(buffer, h->SectionName, 8)) found = true;
			}
		} else {
	                if (mystrncasecmp("WAPSettings", h->SectionName, 11)) found = true;
			if (!found) {
		                if (mystrncasecmp("Settings", h->SectionName, 8)) found = true;
			}
		}
		if (UseUnicode) {
			EncodeUnicode(buffer,"MMSSettings",8);
			if (mywstrncasecmp(buffer, h->SectionName, 8)) found = true;
		} else {
	                if (mystrncasecmp("MMSSettings", h->SectionName, 8)) found = true;
		}
		if (UseUnicode) {
			EncodeUnicode(buffer,"Ringtone",8);
			if (mywstrncasecmp(buffer, h->SectionName, 8)) found = true;
		} else {
	                if (mystrncasecmp("Ringtone", h->SectionName, 8)) found = true;
		}
		if (UseUnicode) {
			EncodeUnicode(buffer,"TODO",4);
			if (mywstrncasecmp(buffer, h->SectionName, 4)) found = true;
		} else {
	                if (mystrncasecmp("TODO", h->SectionName, 4)) found = true;
		}
		if (UseUnicode) {
			EncodeUnicode(buffer,"Startup",7);
			if (mywstrncasecmp(buffer, h->SectionName, 7)) found = true;
		} else {
	                if (mystrncasecmp("Startup", h->SectionName, 7)) found = true;
		}
		if (UseUnicode) {
			EncodeUnicode(buffer,"Operator",7);
			if (mywstrncasecmp(buffer, h->SectionName, 8)) found = true;
		} else {
	                if (mystrncasecmp("Operator", h->SectionName, 8)) found = true;
		}
		if (UseUnicode) {
			EncodeUnicode(buffer,"FMStation",9);
			if (mywstrncasecmp(buffer, h->SectionName, 9)) found = true;
		} else {
	                if (mystrncasecmp("FMStation", h->SectionName, 9)) found = true;
		}
		if (UseUnicode) {
			EncodeUnicode(buffer,"GPRSPoint",9);
			if (mywstrncasecmp(buffer, h->SectionName, 9)) found = true;
		} else {
	                if (mystrncasecmp("GPRSPoint", h->SectionName, 9)) found = true;
		}
		if (UseUnicode) {
			EncodeUnicode(buffer,"Note",4);
			if (mywstrncasecmp(buffer, h->SectionName, 4)) found = true;
		} else {
	                if (mystrncasecmp("Note", h->SectionName, 4)) found = true;
		}
		if (!found) return ERR_NOTIMPLEMENTED;
        }
	return ERR_NONE;
}

/* ---------------------- backup files for SMS ----------------------------- */

static void ReadSMSBackupEntry(INI_Section *file_info, char *section, GSM_SMSMessage *SMS)
{
	unsigned char buffer[10000], *readvalue;

	GSM_SetDefaultSMSData(SMS);

	SMS->PDU = SMS_Submit;
	SMS->SMSC.Location = 0;
	sprintf(buffer,"SMSC");
	ReadBackupText(file_info, section, buffer, SMS->SMSC.Number, false);
	sprintf(buffer,"ReplySMSC");
	SMS->ReplyViaSameSMSC = false;
	readvalue = ReadCFGText(file_info, section, buffer, false);
	if (readvalue!=NULL) {
		if (mystrncasecmp(readvalue,"True",0)) SMS->ReplyViaSameSMSC = true;
	}
	sprintf(buffer,"Class");
	SMS->Class = -1;
	readvalue = ReadCFGText(file_info, section, buffer, false);
	if (readvalue!=NULL) SMS->Class = atoi(readvalue);
	sprintf(buffer,"Sent");
	readvalue = ReadCFGText(file_info, section, buffer, false);
	if (readvalue!=NULL) {
		ReadVCALDateTime(readvalue, &SMS->DateTime);
		SMS->PDU = SMS_Deliver;
	}
	sprintf(buffer,"RejectDuplicates");
	SMS->RejectDuplicates = false;
	readvalue = ReadCFGText(file_info, section, buffer, false);
	if (readvalue!=NULL) {
		if (mystrncasecmp(readvalue,"True",0)) SMS->RejectDuplicates = true;
	}
	sprintf(buffer,"ReplaceMessage");
	SMS->ReplaceMessage = 0;
	readvalue = ReadCFGText(file_info, section, buffer, false);
	if (readvalue!=NULL) SMS->ReplaceMessage = atoi(readvalue);
	sprintf(buffer,"MessageReference");
	SMS->MessageReference = 0;
	readvalue = ReadCFGText(file_info, section, buffer, false);
	if (readvalue!=NULL) SMS->MessageReference = atoi(readvalue);
	sprintf(buffer,"State");
	SMS->State = SMS_UnRead;
	readvalue = ReadCFGText(file_info, section, buffer, false);
	if (readvalue!=NULL) {
		if (mystrncasecmp(readvalue,"Read",0))		SMS->State = SMS_Read;
		else if (mystrncasecmp(readvalue,"Sent",0))	SMS->State = SMS_Sent;
		else if (mystrncasecmp(readvalue,"UnSent",0))	SMS->State = SMS_UnSent;
	}
	sprintf(buffer,"Number");
	ReadBackupText(file_info, section, buffer, SMS->Number, false);
	sprintf(buffer,"Name");
	ReadBackupText(file_info, section, buffer, SMS->Name, false);
	sprintf(buffer,"Length");
	SMS->Length = 0;
	readvalue = ReadCFGText(file_info, section, buffer, false);
	if (readvalue!=NULL) SMS->Length = atoi(readvalue);
	sprintf(buffer,"Coding");
	SMS->Coding = SMS_Coding_Default;
	readvalue = ReadCFGText(file_info, section, buffer, false);
	if (readvalue!=NULL) {
		if (mystrncasecmp(readvalue,"Unicode",0)) {
			SMS->Coding = SMS_Coding_Unicode;
		} else if (mystrncasecmp(readvalue,"8bit",0)) {
			SMS->Coding = SMS_Coding_8bit;
		}
	}
	ReadLinkedBackupText(file_info, section, "Text", buffer, false);
	DecodeHexBin (SMS->Text, buffer, strlen(buffer));
	SMS->Text[strlen(buffer)/2]	= 0;
	SMS->Text[strlen(buffer)/2+1] 	= 0;
	sprintf(buffer,"Folder");
	readvalue = ReadCFGText(file_info, section, buffer, false);
	if (readvalue!=NULL) SMS->Folder = atoi(readvalue);
	SMS->UDH.Type		= UDH_NoUDH;
	SMS->UDH.Length 	= 0;
	SMS->UDH.ID8bit	  	= -1;
	SMS->UDH.ID16bit	= -1;
	SMS->UDH.PartNumber	= -1;
	SMS->UDH.AllParts	= -1;
	sprintf(buffer,"UDH");
	readvalue = ReadCFGText(file_info, section, buffer, false);
	if (readvalue!=NULL) {
		DecodeHexBin (SMS->UDH.Text, readvalue, strlen(readvalue));
		SMS->UDH.Length = strlen(readvalue)/2;
		GSM_DecodeUDHHeader(&SMS->UDH);
	}
}

static GSM_Error GSM_ReadSMSBackupTextFile(char *FileName, GSM_SMS_Backup *backup)
{
	INI_Section	*file_info, *h;
	char		*readvalue;
	int		num;

	backup->SMS[0] = NULL;

	file_info = INI_ReadFile(FileName, false);

	num = 0;
        for (h = file_info; h != NULL; h = h->Next) {
                if (mystrncasecmp("SMSBackup", h->SectionName, 9)) {
			readvalue = ReadCFGText(file_info, h->SectionName, "Number", false);
			if (readvalue==NULL) break;
			if (num < GSM_BACKUP_MAX_SMS) {
				backup->SMS[num] = malloc(sizeof(GSM_SMSMessage));
			        if (backup->SMS[num] == NULL) return ERR_MOREMEMORY;
				backup->SMS[num + 1] = NULL;
			} else {
				dbgprintf("Increase GSM_BACKUP_MAX_SMS\n");
				return ERR_MOREMEMORY;
			}
			backup->SMS[num]->Location = num + 1;
			ReadSMSBackupEntry(file_info, h->SectionName, backup->SMS[num]);
			num++;
		}
        }
	return ERR_NONE;
}

GSM_Error GSM_ReadSMSBackupFile(char *FileName, GSM_SMS_Backup *backup)
{
	FILE *file;

	backup->SMS[0] = NULL;

	file = fopen(FileName, "rb");
	if (file ==  NULL) return(ERR_CANTOPENFILE);

	fclose(file);

	return GSM_ReadSMSBackupTextFile(FileName, backup);
}

static GSM_Error SaveSMSBackupTextFile(FILE *file, GSM_SMS_Backup *backup)
{
	int 		i,w,current;
	unsigned char 	buffer[10000];
	GSM_DateTime	DT;

	fprintf(file,"\n# File created by Gammu (www.mwiacek.com) version %s\n",VERSION);
	GSM_GetCurrentDateTime (&DT);
	fprintf(file,"# Saved ");
	fprintf(file, "%04d%02d%02dT%02d%02d%02d",
			DT.Year, DT.Month, DT.Day,
			DT.Hour, DT.Minute, DT.Second);
	fprintf(file," (%s)\n\n",OSDateTime(DT,false));

	i=0;
	while (backup->SMS[i]!=NULL) {
		fprintf(file,"[SMSBackup%03i]\n",i);
		switch (backup->SMS[i]->Coding) {
			case SMS_Coding_Unicode:
			case SMS_Coding_Default:
				sprintf(buffer,"%s",DecodeUnicodeString(backup->SMS[i]->Text));
				fprintf(file,"#");
				current = 0;
				for (w=0;w<(int)(strlen(buffer));w++) {
					switch (buffer[w]) {
						case 10:
							fprintf(file,"\n#");
							current = 0;
							break;
						case 13:
							break;
						default:
							if (isprint(buffer[w])) {
								fprintf(file,"%c",buffer[w]);
								current ++;
							}
							if (current == 75) {
								fprintf(file,"\n#");
								current = 0;
							}
					}
				}
				fprintf(file,"\n");	
				break;
			default:
				break;
		}
		if (backup->SMS[i]->PDU == SMS_Deliver) {
			SaveBackupText(file, "SMSC", backup->SMS[i]->SMSC.Number, false);
			if (backup->SMS[i]->ReplyViaSameSMSC) fprintf(file,"SMSCReply = true\n");
			fprintf(file,"Sent");
			SaveVCalDateTime(file,&backup->SMS[i]->DateTime, false);
		}
		fprintf(file,"State = ");
		switch (backup->SMS[i]->State) {
			case SMS_UnRead	: fprintf(file,"UnRead\n");	break;
			case SMS_Read	: fprintf(file,"Read\n");	break;
			case SMS_Sent	: fprintf(file,"Sent\n");	break;
			case SMS_UnSent	: fprintf(file,"UnSent\n");	break;
		}
		SaveBackupText(file, "Number", backup->SMS[i]->Number, false);
		SaveBackupText(file, "Name", backup->SMS[i]->Name, false);
		if (backup->SMS[i]->UDH.Type != UDH_NoUDH) {
			EncodeHexBin(buffer,backup->SMS[i]->UDH.Text,backup->SMS[i]->UDH.Length);
			fprintf(file,"UDH = %s\n",buffer);
		}
		switch (backup->SMS[i]->Coding) {
			case SMS_Coding_Unicode:
			case SMS_Coding_Default:
				EncodeHexBin(buffer,backup->SMS[i]->Text,backup->SMS[i]->Length*2);
				break;
			default:
				EncodeHexBin(buffer,backup->SMS[i]->Text,backup->SMS[i]->Length);
				break;
		}
		SaveLinkedBackupText(file, "Text", buffer, false);
		switch (backup->SMS[i]->Coding) {
			case SMS_Coding_Unicode	: fprintf(file,"Coding = Unicode\n"); 	break;
			case SMS_Coding_Default	: fprintf(file,"Coding = Default\n"); 	break;
			case SMS_Coding_8bit	: fprintf(file,"Coding = 8bit\n"); 	break;
		}
		fprintf(file,"Folder = %i\n",backup->SMS[i]->Folder);
		fprintf(file,"Length = %i\n",backup->SMS[i]->Length);
		fprintf(file,"Class = %i\n",backup->SMS[i]->Class);
		fprintf(file,"ReplySMSC = ");
		if (backup->SMS[i]->ReplyViaSameSMSC) fprintf(file,"True\n"); else fprintf(file,"False\n");
		fprintf(file,"RejectDuplicates = ");
		if (backup->SMS[i]->RejectDuplicates) fprintf(file,"True\n"); else fprintf(file,"False\n");
		fprintf(file,"ReplaceMessage = %i\n",backup->SMS[i]->ReplaceMessage);
		fprintf(file,"MessageReference = %i\n",backup->SMS[i]->MessageReference);
		fprintf(file,"\n");
		i++;
	}
	return ERR_NONE;
}

GSM_Error GSM_AddSMSBackupFile(char *FileName, GSM_SMS_Backup *backup)
{
	FILE *file;
  
	file = fopen(FileName, "ab");      
	if (file == NULL) return(ERR_CANTOPENFILE);

	SaveSMSBackupTextFile(file,backup);

	fclose(file);
   
	return ERR_NONE;
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

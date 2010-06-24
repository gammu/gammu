/* (c) 2002-2004 by Marcin Wiacek */



time_t Fill_Time_T(GSM_DateTime DT, int TZ)
{
	struct tm 	tm_starttime;
	unsigned char 	buffer[30];

	dbgprintf("  StartTime  : %02i-%02i-%04i %02i:%02i:%02i\n",
		DT.Day,DT.Month,DT.Year,DT.Hour,DT.Minute,DT.Second);

	if (TZ != 0) {
#if defined(WIN32) || defined(__SVR4)
	    sprintf((char*)buffer,"TZ=PST+%i",TZ);
	    putenv((char*)buffer);
#else
	    sprintf((char*)buffer,"PST+%i",TZ);
	    setenv("TZ",buffer,1);
#endif
	}
	tzset();

	memset(&tm_starttime, 0, sizeof(tm_starttime));
	tm_starttime.tm_year 	= DT.Year - 1900;
	tm_starttime.tm_mon  	= DT.Month - 1;
	tm_starttime.tm_mday 	= DT.Day;
	tm_starttime.tm_hour 	= DT.Hour;
	tm_starttime.tm_min  	= DT.Minute;
	tm_starttime.tm_sec  	= DT.Second;
	tm_starttime.tm_isdst	= 0;
	
	return mktime(&tm_starttime);
}
void GetTimeDifference(unsigned long diff, GSM_DateTime *DT, bool Plus, int multi)
{
	time_t t_time;

	t_time = Fill_Time_T(*DT,8);

	if (Plus) {
		t_time 		+= diff*multi;
	} else {
		t_time 		-= diff*multi;
	}

	Fill_GSM_DateTime(DT, t_time);
	DT->Year = DT->Year + 1900;
	dbgprintf("  EndTime    : %02i-%02i-%04i %02i:%02i:%02i\n",
		DT->Day,DT->Month,DT->Year,DT->Hour,DT->Minute,DT->Second);
}
void SaveVCALDateTime(char *Buffer, int *Length, GSM_DateTime *Date, char *Start)
{
	if (Start != NULL) {
		*Length+=sprintf(Buffer+(*Length), "%s:",Start);
	}
	*Length+=sprintf(Buffer+(*Length), "%04d%02d%02dT%02d%02d%02dZ%c%c",
			Date->Year, Date->Month, Date->Day,
			Date->Hour, Date->Minute, Date->Second,13,10);
}


void ReadVCALDateTime(char *Buffer, GSM_DateTime *dt,int nTimeZone)
{
	char year[5]="", month[3]="", day[3]="", hour[3]="", minute[3]="", second[3]="";

	memset(dt,0,sizeof(dt));

	strncpy(year, 	Buffer, 	4);
	strncpy(month, 	Buffer+4, 	2);
	strncpy(day, 	Buffer+6, 	2);
	strncpy(hour, 	Buffer+9,	2);
	strncpy(minute, Buffer+11,	2);
	strncpy(second, Buffer+13,	2);



	/* FIXME: Should check ranges... */
	dt->Year	= atoi(year);
	dt->Month	= atoi(month);
	dt->Day		= atoi(day);
	dt->Hour	= atoi(hour);
	dt->Minute	= atoi(minute);
	dt->Second	= atoi(second);
	/* FIXME */
	if(nTimeZone!=0  && strlen(Buffer)>=16 && (Buffer[15] =='Z' ||Buffer[15] =='z'))
	{
		if(nTimeZone>0)
			GetTimeDifference(nTimeZone*15, dt, true, 60);
		else 
			GetTimeDifference(-nTimeZone*15, dt, false, 60);

	}

	dt->Timezone	= 0;
}


bool IsCalendarNoteFromThePast(GSM_CalendarEntry *note)
{
	bool 		Past = true;
	int		i;
	GSM_DateTime	DT;

	GSM_GetCurrentDateTime (&DT);
	for (i = 0; i < note->EntriesNum; i++) {
		switch (note->Entries[i].EntryType) {
		case CAL_RECURRANCE:
			Past = false;
			break;
		case CAL_START_DATETIME :
			if (note->Entries[i].Date.Year > DT.Year) Past = false;
			if (note->Entries[i].Date.Year == DT.Year &&
			    note->Entries[i].Date.Month > DT.Month) Past = false;
			if (note->Entries[i].Date.Year == DT.Year &&
			    note->Entries[i].Date.Month == DT.Month &&
			    note->Entries[i].Date.Day > DT.Day) Past = false;
			break;
		default:
			break;
		}
		if (!Past) break;
	}
	switch (note->Type) {
		case GSM_CAL_BIRTHDAY:
			Past = false;
			break;
		default:
			break;
	}
	return Past;
}


void GSM_ToDoFindDefaultTextTimeAlarmCompleted(GSM_ToDoEntry *entry, int *Text, int *Alarm, int *Completed, int *EndTime, int *Phone)
{
	int i;

	*Text		= -1;
	*EndTime	= -1;
	*Alarm		= -1;
	*Completed	= -1;
	*Phone		= -1;
	for (i = 0; i < entry->EntriesNum; i++) {
		switch (entry->Entries[i].EntryType) {
		case TODO_END_DATETIME :
			if (*EndTime == -1) *EndTime = i;
			break;
		case TODO_ALARM_DATETIME :
		case TODO_SILENT_ALARM_DATETIME:
			if (*Alarm == -1) *Alarm = i;
			break;
		case TODO_TEXT:
			if (*Text == -1) *Text = i;
			break;
		case TODO_COMPLETED:
			if (*Completed == -1) *Completed = i;
			break;
		case TODO_PHONE:
			if (*Phone == -1) *Phone = i;
			break;
		default:
			break;
		}
	}
}



GSM_Error GSM_EncodeVTODO(char *Buffer, int *Length, GSM_ToDoEntry *note, bool header, GSM_VToDoVersion Version,int Timezone)
{
 	int Text, Alarm, Completed, StartTime, EndTime, Phone;
	int Recurrance,RecurranceFreq ,RepeatEndDate,Description,DayofweekMask,CompletedDate;

	if (Version == Samsung_VToDo)
		GSM_ToDoFindFindEntryIndexEx(note, &Text, &Alarm, &Completed,&StartTime, &EndTime, &Phone,&Recurrance,&RecurranceFreq ,&RepeatEndDate,&Description,&DayofweekMask,&CompletedDate);
	else
		GSM_ToDoFindFindEntryIndex(note, &Text, &Alarm, &Completed, &EndTime, &Phone,&Recurrance,&RecurranceFreq ,&RepeatEndDate,&Description,&DayofweekMask,&CompletedDate);

	if (header) {
		*Length+=sprintf(Buffer, "BEGIN:VCALENDAR%c%c",13,10);
		*Length+=sprintf(Buffer+(*Length), "VERSION:1.0%c%c",13,10);
	}

	*Length+=sprintf(Buffer+(*Length), "BEGIN:VTODO%c%c",13,10);

	if (Text == -1) return ERR_UNKNOWN;
	unsigned char utemp[(MAX_TODO_TEXT_LENGTH + 1)*2];
	CopyUnicodeString(utemp,note->Entries[Text].Text);
//	UnicodeReplace(utemp,";","\\;");

	if(Version == SonyEricsson_VToDo) //Encode with utf7
		SESaveVCALText(Buffer, Length, utemp, "SUMMARY",":");
	else
	{
		if (Version == Samsung_VToDo)
			UnicodeReplace(utemp,";","\\;");
		SESaveVCARDText(Buffer, Length, utemp, "SUMMARY",":");
	}
	if (Version == Samsung_VToDo)	//Summary = no use, Decription = title for samsung	//bobby 10.5.05
	{
		if(StartTime != -1)
		{
			GSM_DateTime gsmDateTime;
			memcpy(&gsmDateTime,&note->Entries[StartTime].Date,sizeof(GSM_DateTime));

			if(Timezone>0)
				GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
			else
				GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);
			if(Timezone == 0)
				SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &gsmDateTime, "DTSTART");
			else
				SaveVCALDateTime(Buffer, Length, &gsmDateTime, "DTSTART");
		}
		else
		{	
			GSM_DateTime gsmDateTime;
			memcpy(&gsmDateTime,&note->Entries[EndTime].Date,sizeof(GSM_DateTime));
			if(Timezone>0)
				GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
			else
				GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);
			if(Timezone == 0)
				SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &gsmDateTime, "DTSTART");
			else
				SaveVCALDateTime(Buffer, Length, &gsmDateTime, "DTSTART");
		}
	}

	if (EndTime != -1) 
	{
		if(Version == SonyEricsson_VToDo)
		{
			note->Entries[CompletedDate].Date.Hour = 23;
			note->Entries[CompletedDate].Date.Minute = 59;
			SaveVCALDateTime(Buffer, Length, &note->Entries[EndTime].Date, "DUE");
		}
		else
		{
			GSM_DateTime gsmDateTime;
			memcpy(&gsmDateTime,&note->Entries[EndTime].Date,sizeof(GSM_DateTime));

			if(Timezone>0)
				GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
			else
				GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);

			if(Timezone == 0)
				SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &gsmDateTime, "DUE");
			else
				SaveVCALDateTime(Buffer, Length, &gsmDateTime, "DUE");
		}
	}
	if (CompletedDate != -1) 
	{
		GSM_DateTime gsmDateTime;
		memcpy(&gsmDateTime,&note->Entries[CompletedDate].Date,sizeof(GSM_DateTime));
		if(Timezone>0)
			GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
		else
			GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);
		if(Timezone == 0)
			SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &gsmDateTime, "COMPLETED");
		else
			SaveVCALDateTime(Buffer, Length, &gsmDateTime, "COMPLETED");
	}

	if(Version == SonyEricsson_VToDo)
	{
		switch (note->Type) {
		case GSM_CAL_MEETING:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "MISCELLANEOUS%c%c",13,10);
			break;
		case GSM_CAL_CALL:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "PHONECALL%c%c",13,10);
			break;
		case GSM_CAL_REMINDER:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "MESSAGE%c%c",13,10);
			break;
		case GSM_CAL_MEMO:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "TASK%c%c",13,10);
			break;
		case GSM_CAL_TRAVEL:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "PHONE CALL%c%c",13,10);
		//		*Length+=sprintf(Buffer+(*Length), "APPOINTMENT%c%c",13,10);
		break;
		}

	}
	else if(Version == Sagem_VToDo)
	{
		switch (note->Type) {
		case GSM_CAL_MEETING:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "UNKNOWN%c%c",13,10);
			break;
		case GSM_CAL_CALL:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "SHOPPING LIST%c%c",13,10);
			break;
		case GSM_CAL_REMINDER:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "BUSINESS%c%c",13,10);
			break;
		case GSM_CAL_MEMO:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "PERSONAL%c%c",13,10);
			break;
		case GSM_CAL_TRAVEL:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "PHONE CALL%c%c",13,10);
			break;
		case GSM_CAL_BUSINESS:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "GIFT%c%c",13,10);
			break;
		case GSM_CAL_VACATION:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "IDEA%c%c",13,10);
			break;
		}
	}
	*Length+=sprintf(Buffer+(*Length), "PRIORITY:%d%c%c",note->Priority,13,10);

	bool bCompleted = false;
	if (Completed != -1 ) 
		if(note->Entries[Completed].Number) bCompleted = true;

	if (bCompleted == false ) 
		*Length+=sprintf(Buffer+(*Length), "STATUS:NEEDS ACTION%c%c",13,10);
	else 
		*Length+=sprintf(Buffer+(*Length), "STATUS:COMPLETED%c%c",13,10);


	
	if (Alarm != -1) 
	{
		GSM_DateTime gsmDateTime;
		memcpy(&gsmDateTime,&note->Entries[Alarm].Date,sizeof(GSM_DateTime));
		if(Timezone>0)
			GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
		else
			GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);

		if(Timezone == 0)
			SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &gsmDateTime, "AALARM");
		else
			SaveVCALDateTime(Buffer, Length, &gsmDateTime, "AALARM");
	}


	if (Description != -1) 
	{
		CopyUnicodeString(utemp,note->Entries[Description].Text);
//		UnicodeReplace(utemp,";","\\;");

		if(Version == SonyEricsson_VToDo) //Encode with utf7
			SESaveVCALText(Buffer, Length, utemp, "DESCRIPTION",":");
		else
		{
			if (Version == Samsung_VToDo)
				UnicodeReplace(utemp,";","\\;");
			SESaveVCARDText(Buffer, Length, utemp, "DESCRIPTION",":");
		}
	}

	if (Recurrance != -1) 
	{
		int nFreq = 1;
		if(RecurranceFreq!= -1) nFreq = note->Entries[RecurranceFreq].Number;
		switch(note->Entries[Recurrance].Number)
		{
			case CAL_REPEAT_DAILY	 : 
				*Length+=sprintf(Buffer+(*Length), "RRULE:D%d #0",nFreq);
				break;
			case CAL_REPEAT_WEEKLY	 : 
				if(DayofweekMask == -1)
					*Length+=sprintf(Buffer+(*Length), "RRULE:W%d #0",nFreq);
				else 
				{
					char szDay[MAX_PATH];
					int nMask = note->Entries[DayofweekMask].Number;
					szDay[0] = '\0';
					if(nMask & 1) strcat(szDay ,"SU ");
					if(nMask & 2) strcat(szDay ,"MO ");
					if(nMask & 4) strcat(szDay ,"TU ");
					if(nMask & 8) strcat(szDay ,"WE ");
					if(nMask & 16) strcat(szDay ,"TH ");
					if(nMask & 32) strcat(szDay ,"FR ");
					if(nMask & 64) strcat(szDay ,"SA ");
					*Length+=sprintf(Buffer+(*Length), "RRULE:W%d %s#0",nFreq,szDay);

				}
				break;
			case CAL_REPEAT_MONTHLY	 : 
				*Length+=sprintf(Buffer+(*Length), "RRULE:MD%d #0",nFreq);
				break;
			case CAL_REPEAT_MONTH_WEEKLY	 : 
				{
					UINT whichWeek,  whichDay;
					GetwhichWeekDay(note->Entries[EndTime].Date,whichWeek,whichDay);
					char szday[MAX_PATH];
					switch(whichDay)
					{
					case 0:
						wsprintf(szday,"SU");
						break;
					case 1:
						wsprintf(szday,"MO");
						break;
					case 2:
						wsprintf(szday,"TU");
						break;
					case 3:
						wsprintf(szday,"WE");
						break;
					case 4:
						wsprintf(szday,"TH");
						break;
					case 5:
						wsprintf(szday,"FR");
						break;
					case 6:
						wsprintf(szday,"SA");
						break;
					}

					if(whichWeek == 5) 
						*Length+=sprintf(Buffer+(*Length), "RRULE:MP%d %d- %s #0",nFreq,1,szday);
					else
						*Length+=sprintf(Buffer+(*Length), "RRULE:MP%d %d+ %s #0",nFreq,whichWeek,szday);
				}
				break;
			case CAL_REPEAT_YEARLY : 
				*Length+=sprintf(Buffer+(*Length), "RRULE:YM%d #0",nFreq);
				break;
		}
		if(RepeatEndDate!=-1)
		{
			*Length+=sprintf(Buffer+(*Length), " %04d%02d%02dT%02d%02d%02d",
				note->Entries[RepeatEndDate].Date.Year,note->Entries[RepeatEndDate].Date.Month,note->Entries[RepeatEndDate].Date.Day,
				note->Entries[RepeatEndDate].Date.Hour,note->Entries[RepeatEndDate].Date.Minute,note->Entries[RepeatEndDate].Date.Second);
		}
		*Length+=sprintf(Buffer+(*Length), "%c%c",13,10);

		SaveVCALDateTime(Buffer, Length, &note->Entries[EndTime].Date, "DTSTART");

//DTEND
		if(RepeatEndDate!=-1)
			SaveVCALDateTime(Buffer, Length, &note->Entries[RepeatEndDate].Date, "DTEND");
		else
		{
			GSM_DateTime time;
			time.Year = 2100;
			time.Month = 12;
			time.Day = 31;
			time.Hour = time.Minute = time.Second = 0;
			SaveVCALDateTime(Buffer, Length, &time, "DTEND");
		}


	}	
	*Length+=sprintf(Buffer+(*Length), "END:VTODO%c%c",13,10);

	if (header) {
		*Length+=sprintf(Buffer+(*Length), "END:VCALENDAR%c%c",13,10);
	}
	return ERR_NONE;
}
GSM_Error GSM_DecodeVCALENDAR_VTODO(unsigned char *Buffer, int *Pos, GSM_CalendarEntry *Calendar, GSM_ToDoEntry *ToDo, GSM_VCalendarVersion CalVer, GSM_VToDoVersion ToDoVer,int Timezone,bool bSupportDescription)
{
	unsigned char 	Line[2000],Buff[2000],	Line2[2000];
    int             Level = 0,num,npos;
	char szCalType[500];
	char  str[30];
//	time_t time;
	unsigned char	   	DescriptionText[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
	DescriptionText [0] =0;
	bool bSummary = false;
	bool bDescription = false;

	Calendar->EntriesNum 	= 0;
	ToDo->EntriesNum 	= 0;

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

 /*		npos = *Pos;//peggy
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
			if (strstr((char *)Line,"BEGIN:VEVENT")) {
				Calendar->Type 	= GSM_CAL_MEMO;
				Level 		= 1;
			}
			if (strstr((char *)Line,"BEGIN:VTODO")) {
				ToDo->Priority 	= 2;//GSM_Priority_Low;
				Level 		= 2;
			}
			break;
		case 1: /* Calendar note */
			if (strstr((char *)Line,"END:VEVENT")) {
				if (Calendar->EntriesNum == 0) return ERR_EMPTY;
				if(bSupportDescription)
				{
					if(bDescription)
					{
						Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_DESCRIPTION;
						CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,DescriptionText);
						UnicodeReplace(Calendar->Entries[Calendar->EntriesNum].Text,"\\;",";");
						Calendar->EntriesNum++;
					}
				}
				else
				{
					if(bSummary == false && bDescription)
					{
						Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_TEXT;
						CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,DescriptionText);
						UnicodeReplace(Calendar->Entries[Calendar->EntriesNum].Text,"\\;",";");
						Calendar->EntriesNum++;
					}
				}
				return ERR_NONE;
			}
	/*		if (strstr(Line,"CATEGORIES:REMINDER")) 	Calendar->Type = GSM_CAL_REMINDER;
			if (strstr(Line,"CATEGORIES:DATE"))	 	Calendar->Type = GSM_CAL_REMINDER;//SE
			if (strstr(Line,"CATEGORIES:TRAVEL"))	 	Calendar->Type = GSM_CAL_TRAVEL;  //SE
			if (strstr(Line,"CATEGORIES:VACATION"))	 	Calendar->Type = GSM_CAL_VACATION;//SE
			if (strstr(Line,"CATEGORIES:MISCELLANEOUS")) 	Calendar->Type = GSM_CAL_MEMO;
			if (strstr(Line,"CATEGORIES:PHONE CALL")) 	Calendar->Type = GSM_CAL_CALL;
			if (strstr(Line,"CATEGORIES:SPECIAL OCCASION")) Calendar->Type = GSM_CAL_BIRTHDAY;
			if (strstr(Line,"CATEGORIES:ANNIVERSARY")) 	Calendar->Type = GSM_CAL_BIRTHDAY;
			if (strstr(Line,"CATEGORIES:MEETING")) 		Calendar->Type = GSM_CAL_MEETING;
			if (strstr(Line,"CATEGORIES:APPOINTMENT")) 	Calendar->Type = GSM_CAL_MEETING;
*/	////////// 
  			//peggy +
/*			if(Line2[0] == '=')
			{
				strcat((char *)Line,(char *)Line2+1);
			}*/
			//peggy +
			if (ReadVCALText(Line, "CATEGORIES", Buff) )
			{

				DecodeUnicode(Buff,(unsigned char *)szCalType);
				if (strstr(szCalType,"REMINDER")) 	Calendar->Type = GSM_CAL_REMINDER;
				if (strstr(szCalType,"DATE"))	 	Calendar->Type = GSM_CAL_REMINDER;//SE
				if (strstr(szCalType,"TRAVEL"))	 	Calendar->Type = GSM_CAL_TRAVEL;  //SE
				if (strstr(szCalType,"VACATION"))	 	Calendar->Type = GSM_CAL_VACATION;//SE
				if (strstr(szCalType,"MISCELLANEOUS")) 	Calendar->Type = GSM_CAL_MEMO;
				if (strstr(szCalType,"PHONE CALL")) 	Calendar->Type = GSM_CAL_CALL;
				if (strstr(szCalType,"SPECIAL OCCASION")) Calendar->Type = GSM_CAL_BIRTHDAY;
				if (strstr(szCalType,"ANNIVERSARY")) 	Calendar->Type = GSM_CAL_BIRTHDAY;
				if (strstr(szCalType,"MEETING")) 		Calendar->Type = GSM_CAL_MEETING;
				if (strstr(szCalType,"APPOINTMENT")) 	Calendar->Type = GSM_CAL_MEETING;

				if (strstr(szCalType,"Meeting")) 		Calendar->Type = GSM_CAL_MEETING;
				if (strstr(szCalType,"Memo")) 	Calendar->Type = GSM_CAL_MEMO;
				if (strstr(szCalType,"Birthday")) Calendar->Type = GSM_CAL_BIRTHDAY;
				if (strstr(szCalType,"Call")) Calendar->Type = GSM_CAL_REMINDER;
				if (strstr(szCalType,"Voice memo")) Calendar->Type =GSM_CAL_CALL ;
				if (strstr(szCalType,"Holiday")) Calendar->Type = GSM_CAL_VACATION;
				if (strstr(szCalType,"語音備忘")) 	Calendar->Type = GSM_CAL_MEMO;
				if (strstr(szCalType,"假期"))	 	Calendar->Type = GSM_CAL_VACATION;//SE
				if (strstr(szCalType,"文字提示")) 	Calendar->Type = GSM_CAL_REMINDER;
				if (strstr(szCalType,"撥叫")) 	Calendar->Type = GSM_CAL_CALL;
				if (strstr(szCalType,"生日")) Calendar->Type = GSM_CAL_BIRTHDAY;
				if (strstr(szCalType,"會議")) 		Calendar->Type = GSM_CAL_MEETING;
			}
			
            if (ReadVCALText(Line, "X-IRMC-LUID", Buff)) 
			{
				num=0;
				num=Searchstr(Line,(unsigned char *)"X-IRMC-LUID",11);
				if(num)
				{
					memcpy(str,&Line[num+11],30);
					if(CalVer == SonyEricsson_VCalendar)
					//	Calendar->Location=strtoul((char *)DecodeUnicodeString(Buff), NULL, 16);
						wsprintf(Calendar->szIndex,(char *)DecodeUnicodeString(Buff));
					else
					//	Calendar->Location=atoi(str);
						wsprintf(Calendar->szIndex,str);
				}
				
            }
	////////		
			if (strstr((char *)Line,"RRULE:D1")) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_DAILY ;//1*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = 1 ;
				Calendar->EntriesNum++;
			}
			if ((strstr((char *)Line,"RRULE:W1")) || (strstr((char *)Line,"RRULE:D7"))) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_WEEKLY;//7*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = 1 ;
				Calendar->EntriesNum++;
			}
			if (strstr((char *)Line,"RRULE:W2")) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_WEEKLY;//14*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = 2 ;
				Calendar->EntriesNum++;
			}
			if (strstr((char *)Line,"RRULE:MD1")) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_MONTHLY;//30*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = 1 ;
				Calendar->EntriesNum++;
			}
			if (strstr((char *)Line,"RRULE:YD1")) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_YEARLY;//365*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = 1 ;
				Calendar->EntriesNum++;
			}


		/*	if ((ReadVCALText(Line, "SUMMARY", Buff)) || (ReadVCALText(Line, "DESCRIPTION", Buff))) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_TEXT;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
				Calendar->EntriesNum++;
				bSummary = true;
			}*/
			if ((ReadVCALText(Line, "SUMMARY", Buff))){// || (ReadVCALText(Line, "DESCRIPTION", Buff))) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_TEXT;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
				UnicodeReplace(Calendar->Entries[Calendar->EntriesNum].Text,"\\;",";");
				Calendar->EntriesNum++;
				bSummary = true;
			}
			if ((ReadVCALText(Line, "DESCRIPTION", Buff))) 
			{
				CopyUnicodeString(DescriptionText,Buff);
				bDescription = true;
			}

			if (ReadVCALText(Line, "LOCATION", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_LOCATION;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
				UnicodeReplace(Calendar->Entries[Calendar->EntriesNum].Text,"\\;",";");
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "DTSTART", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_START_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);

				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "DTEND", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_END_DATETIME;
				ReadVCALDateTime((char *)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);
			
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "DALARM", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_SILENT_ALARM_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);
			
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "AALARM", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_ALARM_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);
			
				Calendar->EntriesNum++;
			}
			break;
		case 2: /* ToDo note */
			if (strstr((char *)Line,"END:VTODO")) {
				if (ToDo->EntriesNum == 0) return ERR_EMPTY;
				return ERR_NONE;
			}
			if (ReadVCALText(Line, "DUE", Buff)) 
			{
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_END_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,Timezone);

				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "COMPLETED", Buff)) 
			{
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_COMPLETEDDATE;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,Timezone);
				ToDo->EntriesNum++;
			}

			if (ReadVCALText(Line, "DALARM", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_SILENT_ALARM_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,Timezone);
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "AALARM", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_ALARM_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,Timezone);
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "SUMMARY", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_TEXT;
				CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,Buff);
				UnicodeReplace(ToDo->Entries[ToDo->EntriesNum].Text,"\\;",";");
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "PRIORITY", Buff)) {
			/*	if (ToDoVer == SonyEricsson_VToDo) {
					ToDo->Priority = GSM_Priority_Low;
					if (atoi((char *)DecodeUnicodeString(Buff))==2) ToDo->Priority = GSM_Priority_Medium;
					if (atoi((char *)DecodeUnicodeString(Buff))==1) ToDo->Priority = GSM_Priority_High;
					dbgprintf("atoi is %i %s\n",atoi((char *)DecodeUnicodeString(Buff)),DecodeUnicodeString(Buff));
				} else if (ToDoVer == Nokia_VToDo) {
					ToDo->Priority = GSM_Priority_Low;
					if (atoi((char *)DecodeUnicodeString(Buff))==2) ToDo->Priority = GSM_Priority_Medium;
					if (atoi((char *)DecodeUnicodeString(Buff))==3) ToDo->Priority = GSM_Priority_High;
				}*/
				 ToDo->Priority  = atoi((char *)DecodeUnicodeString(Buff));
			}
			if (strstr((char *)Line,"STATUS:COMPLETED")) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_COMPLETED;
				ToDo->Entries[ToDo->EntriesNum].Number	  = 1;
				ToDo->EntriesNum++;
			}
			if (strstr((char *)Line,"STATUS:NEEDS ACTION")) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_COMPLETED;
				ToDo->Entries[ToDo->EntriesNum].Number	  = 0;
				ToDo->EntriesNum++;
			}
   			if (ReadVCALText(Line, "CATEGORIES", Buff) )
			{
				DecodeUnicode(Buff,(unsigned char *)szCalType);
				if (strstr(szCalType,"MISCELLANEOUS")) 	ToDo->Type = GSM_CAL_MEETING;
				else if (strstr(szCalType,"PHONECALL")) 	ToDo->Type = GSM_CAL_CALL;
				else if (strstr(szCalType,"PHONE CALL")) 	ToDo->Type = GSM_CAL_TRAVEL;
				else if (strstr(szCalType,"MESSAGE")) 	ToDo->Type = GSM_CAL_REMINDER;
				else if (strstr(szCalType,"TASK")) 	ToDo->Type = GSM_CAL_MEMO;
				else ToDo->Type = GSM_CAL_MEETING;
			}
		     if (ReadVCALText(Line, "X-IRMC-LUID", Buff)) 
			{
				num=0;
				num=Searchstr(Line,(unsigned char *)"X-IRMC-LUID",11);
				if(num)
				{
					memcpy(str,&Line[num+11],30);
					wsprintf(ToDo->Location,str);
				/*	if(CalVer == SonyEricsson_VCalendar)
						ToDo->Location=strtoul((char *)DecodeUnicodeString(Buff), NULL, 16);
					else
						ToDo->Location=atoi(str);*/
				}
				
            }
			break;
		}
	}

	if (Calendar->EntriesNum == 0 && ToDo->EntriesNum == 0) return ERR_EMPTY;
	return ERR_NONE;
}


GSM_Error GSM_EncodeVNTFile(unsigned char *Buffer, int *Length, GSM_NoteEntry *Note)
{
	*Length+=sprintf((char *)Buffer+(*Length), "BEGIN:VNOTE%c%c",13,10);
	*Length+=sprintf((char *)Buffer+(*Length), "VERSION:1.1%c%c",13,10);
	SaveVCALText((char*)Buffer, Length,(unsigned char *) Note->Text, "BODY");
	*Length+=sprintf((char *)Buffer+(*Length), "END:VNOTE%c%c",13,10);

	return ERR_NONE;
}










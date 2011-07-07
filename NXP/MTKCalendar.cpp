#include "stdafx.h"
#include "MTK.h"
#include "commfun.h"
#include "Nfundef.h"
#include "MTKfundef.h"
#include "gsmcal.h"
#include "coding.h"
extern CMTKApp theApp;
extern GSM_Reply_MsgType ReplymsgType;


GSM_Error WINAPI InitCalendarGetEx(GSM_CalendarStatus *Status,int *totalnum,int (*pGetStatusfn)(int nCur,int nTotal));
GSM_Error SF_ReplyGetCalendar(GSM_Protocol_Message msg);
GSM_Error SF_ReplyGetTalNum(GSM_Protocol_Message msg);
GSM_Error SF_ReplyAddCal(GSM_Protocol_Message msg);
void NXP_CalendarFindEntryIndex(GSM_CalendarEntry *entry, int *Text, int *Time, int *Alarm, int *Phone, int *Recurrance, int *RecurranceFreq, int *EndTime, int *Location ,int *RepeatEndDate,int *Description,int *DayofweekMask, int *Priority);

extern GSM_Error SF_GetReplyStatue(GSM_Protocol_Message	*msg,GSM_ATReplayInfo* pReplynfo);
 char Base64AlphabetCalendar[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};
unsigned char GetBase64IndexCalendar(char ch)
{
	unsigned char i, v_Return = -1;
	for(i = 0; i < 64; i++)
	{
		if(Base64AlphabetCalendar[(int)i] == ch)
		{
			v_Return = i;
			break;
		}
	}
	return v_Return;
}
int DecodeCalendar(const char * szDecoding, char * szOutput)
{
	char			Ch;
	char			chCurrentIndex;
	unsigned int	i,nCurrentBits;
	unsigned char	chTargetChar;
	char			*pl_DeData;
	nCurrentBits = 0;
	chTargetChar = '\0';
	pl_DeData = szOutput;
	for(i = 0; i < strlen(szDecoding); i++)
	{
		Ch = szDecoding[i];
		if( Ch == '=')
		{
			nCurrentBits = 0;
	continue;
		}
		chCurrentIndex = GetBase64IndexCalendar(Ch);
		if(chCurrentIndex < 0) 	return 0;
		if((nCurrentBits+6) >= 8)
		{
			chTargetChar |= (chCurrentIndex >> (6-(8-nCurrentBits)));
			*pl_DeData = chTargetChar;
			pl_DeData++;
			nCurrentBits = nCurrentBits - 2;
			chTargetChar = chCurrentIndex << (8 - nCurrentBits);
		}
		else
		{
			chTargetChar |= (chCurrentIndex << (2-nCurrentBits));
			nCurrentBits += 6;
		}
	}
	return pl_DeData-szOutput;
}

int EncodeCalendar(const unsigned char *pp_Data, int v_DataLen,unsigned char *pp_EncodeData)
{
	unsigned char	v_FirstByte,v_SecondByte,v_ThirdByte, v_FourthByte;
	unsigned char	*pl_EnData;
	int 			i,v_Left;	
	pl_EnData = (unsigned char *)pp_EncodeData;
	for(i = 0; i < v_DataLen / 3; i++)
	{
		v_FirstByte	= pp_Data[0] >> 2;
		v_SecondByte	= ((pp_Data[0] & 0x03) << 4) | (pp_Data[1] >> 4) ;
		v_ThirdByte	= ((pp_Data[1] & 0x0F) << 2) | (pp_Data[2] >> 6);
		v_FourthByte	= pp_Data[2] & 0x3F;
		pl_EnData[0] = Base64AlphabetCalendar[(int)v_FirstByte];
        pl_EnData[1] = Base64AlphabetCalendar[(int)v_SecondByte];
		pl_EnData[2] = Base64AlphabetCalendar[(int)v_ThirdByte];
		pl_EnData[3] = Base64AlphabetCalendar[(int)v_FourthByte];

		pp_Data += 3;
		pl_EnData += 4;	 
	}
	v_Left = v_DataLen % 3;
	if(v_Left == 1)
	{
		v_FirstByte = pp_Data[0] >> 2;
		v_SecondByte = (pp_Data[0] & 0x03) << 4;
		pl_EnData[0] = Base64AlphabetCalendar[(int)v_FirstByte];
		pl_EnData[1] = Base64AlphabetCalendar[(int)v_SecondByte];
		pl_EnData[2] = '=';
		pl_EnData[3] = '=';
		pp_Data += 1;
		pl_EnData += 4;
	}
	else if(v_Left == 2)
	{
		v_FirstByte	= pp_Data[0] >> 2;
		v_SecondByte	= ((pp_Data[0] & 0x03) << 4) | ((pp_Data[1] & 0xF0) >> 4);
		v_ThirdByte	= (pp_Data[1] & 0x0F) << 2; 
		pl_EnData[0] = Base64AlphabetCalendar[(int)v_FirstByte];
		pl_EnData[1] = Base64AlphabetCalendar[(int)v_SecondByte];
		pl_EnData[2] = Base64AlphabetCalendar[(int)v_ThirdByte];
		pl_EnData[3] = '=';
		pp_Data += 2;
		pl_EnData += 4;	 
	}
	
	return pl_EnData - pp_EncodeData ;
}
void bcd2asciiCalendar( char *p_bcd,char *p_ascii,int ascii_len)
{
	unsigned char i,j,len_char,temp_number;
	memset(p_ascii,'\0',ascii_len);
	len_char=*p_bcd;
	j=1;
	if(len_char%2)
	{
		for(i=0;i<len_char-1;i=i+2)
		{
				temp_number=*(p_bcd+j);
				p_ascii[i]=((temp_number & 0xf0)>>4)+'0';
				p_ascii[i+1]=(temp_number & 0x0f)+'0';
				j++;
		}
		temp_number=*(p_bcd+j);
		p_ascii[len_char-1]=temp_number;
	}
	else
	{
		for(i=0;i<len_char;i=i+2)
		{
				temp_number=*(p_bcd+j);
				p_ascii[i]=((temp_number & 0xf0)>>4)+'0';
				p_ascii[i+1]=(temp_number & 0x0f)+'0';
				j++;
		}
	}
	p_ascii[len_char]='\0';
	len_char=strlen(p_ascii);
	for(i=0;i<len_char;i++)
	{
		if(p_ascii[i]==(10+'0'))
		{
			p_ascii[i]='p';
		}
		if(p_ascii[i]==(11+'0'))
		{
			p_ascii[i]='w';
		}
		if(p_ascii[i]==(12+'0'))
		{
			p_ascii[i]='+';
		}
		if(p_ascii[i]==(13+'0'))
		{
			p_ascii[i]='*';
		}
		if(p_ascii[i]==(14+'0'))
		{
			p_ascii[i]='#';
		}
	}
}
void  ascii2bcdCalendar(char *p_ascii,char *p_bcdcode,int bcdcode_len)
{
	unsigned char i,j,len_char;
	memset(p_bcdcode,'\0',bcdcode_len);
	len_char=strlen(p_ascii);
	for(i=0;i<len_char;i++)
	{
		if((*(p_ascii+i))=='p')
		{
			*(p_ascii+i)=(10+'0');
		}
		else if((*(p_ascii+i))=='w')
		{
			*(p_ascii+i)=(11+'0');
		}
		else if((*(p_ascii+i))=='+')
		{
			*(p_ascii+i)=(12+'0');
		}
		else if((*(p_ascii+i))=='*')
		{
			*(p_ascii+i)=(13+'0');
		}
		else if((*(p_ascii+i))=='#')
		{
			*(p_ascii+i)=(14+'0');
		}
	}
	j=1;
	p_bcdcode[0]=len_char;

	if(len_char%2)
	{
		for(i=0;i<len_char-1;i=i+2)
		{
			p_bcdcode[j]=(*(p_ascii+i)-'0')*16+(*(p_ascii+i+1)-'0');
			j++;
		}
		p_bcdcode[j]=*(p_ascii+i);

	}
	else
	{
		for(i=0;i<len_char-1;i=i+2)
		{
			p_bcdcode[j]=(*(p_ascii+i)-'0')*16+(*(p_ascii+i+1)-'0');
			j++;
		}
	}
}
char CheckSumCalendar(char* pSrc, int nLen)
{

       if(nLen == 0)return 0;
       BYTE ch = *pSrc;
       for(int i = 1; i < nLen; i++)
       {
              ch = ch ^ (*(pSrc+i));
       }
       return ch;
}

GSM_Error WINAPI GetCalendarStatus(GSM_CalendarStatus *Status)
{
//	return ERR_NOTSUPPORTED;
	Status->Used = -1 ;
	return ERR_NONE ;
}

GSM_Error WINAPI GetNextCalendar(GSM_CalendarEntry *Note, bool start)
{
	 FILE *	file;
    if (start) 
        theApp.m_LastCalendarPos = 1;
    else
        theApp.m_LastCalendarPos++;
    int nIndex = theApp.m_LastCalendarPos ;
	smprintf(theApp.m_pDebuginfo, "Getting calendar note %i\n",nIndex);
	if((nIndex-1) < theApp.m_CalendarNum )
	{
		file = fopen(theApp.m_szCalendarDataFileTempFile,"r+b");
		if(file)
		{
			fseek(file,sizeof(GSM_CalendarEntry)*(nIndex-1),SEEK_SET);
			fread(Note,1,sizeof(GSM_CalendarEntry),file);
			fclose(file);
			return ERR_NONE;
		}
		else 
		{
			Note->EntriesNum = 0;
			return ERR_EMPTY;
		}
	}
	else
	{
		Note->EntriesNum = 0;
		return ERR_EMPTY;
	}
	return ERR_EMPTY;



}
void NXP_CalendarFindEntryIndex(GSM_CalendarEntry *entry, int *Text, int *Time, int *Alarm, int *Phone, int *Recurrance, int *RecurranceFreq, int *EndTime, int *Location ,int *RepeatEndDate,int *Description,int *DayofweekMask, int *Priority)
{
	int i;
	*Text		= -1;
	*Time		= -1;
	*Alarm		= -1;
	*Phone		= -1;
	*Recurrance	= -1;
	*EndTime	= -1;
	*Location	= -1;
	*RecurranceFreq	= -1;
	*RepeatEndDate = -1;
	*Description = *DayofweekMask = -1;
	*Priority = -1;

	for (i = 0; i < entry->EntriesNum; i++) {
		switch (entry->Entries[i].EntryType) {
		case CAL_START_DATETIME :
			if (*Time == -1) *Time = i;
			break;
		case CAL_END_DATETIME :
			if (*EndTime == -1) *EndTime = i;
			break;
		case CAL_ALARM_DATETIME :
		case CAL_SILENT_ALARM_DATETIME:
			if (*Alarm == -1) *Alarm = i;
			break;
		case CAL_RECURRANCE:
			if (*Recurrance == -1) *Recurrance = i;
			break;
		case CAL_RECURRANCEFREQUENCY:
			if (*RecurranceFreq == -1) *RecurranceFreq = i;
			break;
		case CAL_TEXT:
			if (*Text == -1) *Text = i;
			break;
		case CAL_PHONE:
			if (*Phone == -1) *Phone = i;
			break;
		case CAL_LOCATION:
			if (*Location == -1) *Location = i;
			break;
		case CAL_REPEAT_STOPDATE:
			if (*RepeatEndDate == -1) *RepeatEndDate = i;
			break;
		case CAL_DESCRIPTION:
			if (*Description == -1) *Description = i;
			break;
		case CAL_REPEAT_DAYOFWEEK:
			if (*DayofweekMask == -1) *DayofweekMask = i;
			break;
		case CAL_PRIVATE:
			if (*Priority == -1) *Priority = i;
			break;
		default:
			break;
		}
	}
}
GSM_Error WINAPI  AddCalendar(GSM_CalendarEntry *Note, int Control)
{
	GSM_Error 		error;
	if( Control == Type_End ) return ERR_NONE;
	int nLocation = atoi(Note->szIndex);
	if (nLocation < 0) return ERR_INVALIDLOCATION; 
	nLocation--;
	if(Control ==Type_Start)
	ATGEN_GetDateTime(&theApp.m_MobileDate_time,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo);
	t_org_AllData org_data;
	ZeroMemory(&org_data,sizeof(t_org_AllData));
	FILE *	file;
	file = fopen(theApp.m_szCalendarIndexFileTempFile,"r+b");
	if(file)
	{
		fseek(file,0,SEEK_SET);
		fread(&org_data,1,sizeof(t_org_AllData),file); 
	    fclose(file);
	}
	org_data.v_OrganizerRecords.a_Records[nLocation].v_Enable=1;
	if (Note->Type==GSM_CAL_REMINDER)
	{
	org_data.v_OrganizerRecords.a_Records[nLocation].v_EventType=1;
	}
	else if (Note->Type==GSM_CAL_MEETING)
	{
      org_data.v_OrganizerRecords.a_Records[nLocation].v_EventType=0;
	}
	int 	Text, Time, Alarm, Phone, Recurrance, RecurranceFreq, EndTime, Location,RepeatEndDate,Description,Dayofweek, Priority;
    GSM_DateTime     start_Date , end_Date, alarm_Date;
	NXP_CalendarFindEntryIndex(Note, &Text, &Time, &Alarm, &Phone, &Recurrance,&RecurranceFreq, &EndTime, &Location,&RepeatEndDate,&Description,&Dayofweek, &Priority);
	if ( Time == -1 ) 
		return ERR_UNKNOWN;
	else
	{
	start_Date  = Note->Entries[Time].Date;
	org_data.v_OrganizerRecords.a_Records[nLocation].v_StartTime.v_YearBitField=start_Date.Year;
	org_data.v_OrganizerRecords.a_Records[nLocation].v_StartTime.v_MonthBitField=start_Date.Month;
	org_data.v_OrganizerRecords.a_Records[nLocation].v_StartTime.v_DayBitField=start_Date.Day;
	org_data.v_OrganizerRecords.a_Records[nLocation].v_StartTime.v_HourBitField=start_Date.Hour;
	org_data.v_OrganizerRecords.a_Records[nLocation].v_StartTime.v_MinutesBitField=start_Date.Minute;
	}
	if (EndTime!=-1)
	{
	end_Date  = Note->Entries[EndTime].Date;
	org_data.v_OrganizerRecords.a_Records[nLocation].v_EndTime.v_YearBitField=end_Date.Year;
	org_data.v_OrganizerRecords.a_Records[nLocation].v_EndTime.v_MonthBitField=end_Date.Month;
	org_data.v_OrganizerRecords.a_Records[nLocation].v_EndTime.v_DayBitField=end_Date.Day;
	org_data.v_OrganizerRecords.a_Records[nLocation].v_EndTime.v_HourBitField=end_Date.Hour;
	org_data.v_OrganizerRecords.a_Records[nLocation].v_EndTime.v_MinutesBitField=end_Date.Minute;

	}
	if(Text != -1)
	{
		EncodeUnicode2UTF8(Note->Entries[Text].Text, (unsigned char *)org_data.v_OrganizerRecords.a_Records[nLocation].a_Text);
	}
	if (Recurrance!=-1)
	{
		if (Note->Entries[Recurrance].Number==5)
		{
			org_data.v_OrganizerRecords.a_Records[nLocation].v_RepeatType=4;
		}
		else
		org_data.v_OrganizerRecords.a_Records[nLocation].v_RepeatType=Note->Entries[Recurrance].Number;	
	}
	if (Alarm !=-1)
	{
		alarm_Date = Note->Entries[Alarm].Date;
// 		if ((start_Date.Day!=alarm_Date.Day)&&(start_Date.Minute==alarm_Date.Minute))
// 		{
// 		org_data.v_OrganizerRecords.a_Records[nLocation].v_ReminderType=4;
// 
// 		}
// 		else
// 		{
// 		org_data.v_OrganizerRecords.a_Records[nLocation].v_ReminderType=5;
// 
// 		}

		unsigned long diff;
		diff = GetDurationDifferenceTime(&start_Date, &alarm_Date);	//Difference in time determined in Seconds
		if (diff == 0 ) // 0 min
			org_data.v_OrganizerRecords.a_Records[nLocation].v_InformType=0;
		else if (diff == 5) // 5 min
			org_data.v_OrganizerRecords.a_Records[nLocation].v_InformType=1;
		else if (diff == 10) // 10 min
			org_data.v_OrganizerRecords.a_Records[nLocation].v_InformType=2;
		else if (diff == 60) // 1 hou
			org_data.v_OrganizerRecords.a_Records[nLocation].v_InformType=3;
		else if (diff ==1440)
		{
			org_data.v_OrganizerRecords.a_Records[nLocation].v_InformType=4;
		}
		else if (diff ==10080)
		{
			org_data.v_OrganizerRecords.a_Records[nLocation].v_InformType=5;
		}
	}
	else 
	{
	org_data.v_OrganizerRecords.a_Records[nLocation].v_InformType=6; // off
		
	}
	remove(theApp.m_szCalendarIndexFileTempFile);
	file = fopen(theApp.m_szCalendarIndexFileTempFile,"a+b");
	if(file)
	{
		int len = fwrite(&org_data,1,30844,file);
		fclose(file);
	}
	char req[500];
	sprintf(req, "AT*STARTDL=/app/org/dt/data,%d\r", 30844);
	ReplymsgType.nCount = 2;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*STARTDL=");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*STARTDL");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;
	error=theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	if (error != ERR_NONE&& error != ERR_EMPTY) return error;
		unsigned char DataBuffer[200];
	unsigned char EncodeDataBuffer[200];
	unsigned char checkSum[10];
	file = fopen(theApp.m_szCalendarIndexFileTempFile,"r+b");
    for (int i=0;i<285;i++)
    {
		memset(DataBuffer,0,sizeof(DataBuffer));
		memset(EncodeDataBuffer,0,sizeof(EncodeDataBuffer));
		memset(checkSum,0,sizeof(checkSum));
		if(file)
		{
			fseek(file,/*DecodeLength/4*/108*i,SEEK_SET);
			if (i==284)
			{
			fread(DataBuffer,1,/*DecodeLength/4*/64,file);
			}
			else
			{
			fread(DataBuffer,1,/*DecodeLength/4*/108,file);
			}

		}
		

		if (i==284)
		{
			char check=CheckSumCalendar((char *)DataBuffer,64);
			EncodeCalendar((unsigned char*)&check,1,checkSum);
			EncodeCalendar(DataBuffer,64,EncodeDataBuffer);
			sprintf(req,"AT*PUT=%d,%d,%s,%s\r",i,64,EncodeDataBuffer,checkSum);
		}
		else
		{
			char check=CheckSumCalendar((char *)DataBuffer,108);
			EncodeCalendar((unsigned char*)&check,1,checkSum);
			EncodeCalendar(DataBuffer,108,EncodeDataBuffer);
			
			sprintf(req,"AT*PUT=%d,%d,%s,%s\r",i,108,EncodeDataBuffer,checkSum);
			
		}
	ReplymsgType.nCount = 2;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*PUT=");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*PUT");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;
	error=theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	if (error != ERR_NONE) 
	{
		fclose(file);//081027libaoliu
		return error;
	}
		
    }
	fclose(file);
	sprintf(req,"at*end\r");
	ReplymsgType.nCount = 2;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"at*end");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*end:");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;
	error = theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	if (error != ERR_NONE) 
		return error;
	sprintf(req,"AT*SYNCC\r");
	ReplymsgType.nCount = 2;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*SYNCC");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*SYNCC:");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;
	error = theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	if (error != ERR_NONE&& error != ERR_EMPTY) return error;
	return error;

}
GSM_Error WINAPI InitCalendarGet(GSM_CalendarStatus *Status,int *totalnum)
{
//	return ERR_NOTSUPPORTED;
	return InitCalendarGetEx(Status,totalnum,NULL);
}
GSM_Error WINAPI DelCalendar(GSM_CalendarEntry *Note, int control)
{
//	return ERR_NOTSUPPORTED;

	GSM_Error 		error;
	int nLocation = atoi(Note->szIndex);
	if (nLocation < 0) return ERR_INVALIDLOCATION; 
	if (nLocation >0) 
		nLocation --;
	if( control==Type_End)
	{
		return ERR_NONE; 

	}
	t_org_AllData org_data;
	ZeroMemory(&org_data,sizeof(t_org_AllData));
	FILE *	file;
	file = fopen(theApp.m_szCalendarIndexFileTempFile,"r+b");
	if(file)
	{
		fseek(file,0,SEEK_SET);
		fread(&org_data,1,sizeof(t_org_AllData),file); 
	    fclose(file);
	}
	org_data.v_OrganizerRecords.a_Records[nLocation].v_Enable=0;
	remove(theApp.m_szCalendarIndexFileTempFile);
	file = fopen(theApp.m_szCalendarIndexFileTempFile,"a+b");
	if(file)
	{
		int len = fwrite(&org_data,1,30844,file);
		fclose(file);
	}
	char req[500];
	sprintf(req, "AT*STARTDL=/app/org/dt/data,%d\r", 30844);
	ReplymsgType.nCount = 2;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*STARTDL=");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*STARTDL");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;
	error=theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	if (error != ERR_NONE&& error != ERR_EMPTY) return error;
		unsigned char DataBuffer[200];
	unsigned char EncodeDataBuffer[200];
	unsigned char checkSum[10];
	file = fopen(theApp.m_szCalendarIndexFileTempFile,"r+b");
    for (int i=0;i<285;i++)
    {
		memset(DataBuffer,0,sizeof(DataBuffer));
		memset(EncodeDataBuffer,0,sizeof(EncodeDataBuffer));
		memset(checkSum,0,sizeof(checkSum));
		if(file)
		{
			fseek(file,/*DecodeLength/4*/108*i,SEEK_SET);
			if (i==284)
			{
			fread(DataBuffer,1,/*DecodeLength/4*/64,file);
			}
			else
			{
			fread(DataBuffer,1,/*DecodeLength/4*/108,file);
			}

		}
		

		if (i==284)
		{
			char check=CheckSumCalendar((char *)DataBuffer,64);
			EncodeCalendar((unsigned char*)&check,1,checkSum);
			EncodeCalendar(DataBuffer,64,EncodeDataBuffer);
			sprintf(req,"AT*PUT=%d,%d,%s,%s\r",i,64,EncodeDataBuffer,checkSum);
		}
		else
		{
			char check=CheckSumCalendar((char *)DataBuffer,108);
			EncodeCalendar((unsigned char*)&check,1,checkSum);
			EncodeCalendar(DataBuffer,108,EncodeDataBuffer);
			
			sprintf(req,"AT*PUT=%d,%d,%s,%s\r",i,108,EncodeDataBuffer,checkSum);
			
		}
	ReplymsgType.nCount = 2;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*PUT=");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*PUT");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;
	error=theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	if (error != ERR_NONE)
	{
		fclose(file);//081027libaoliu
		return error;
	}
		
    }
	fclose(file);
	sprintf(req,"at*end\r");
	ReplymsgType.nCount = 2;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"at*end");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*end:");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;
	error = theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	if (error != ERR_NONE) 
		return error;
	sprintf(req,"AT*SYNCC\r");

	ReplymsgType.nCount = 2;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*SYNCC");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*SYNCC:");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;
	error = theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	if (error != ERR_NONE&& error != ERR_EMPTY) return error;
	return error;
}

GSM_Error NXP_ReplyGetCalendarInfo(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	MTK_GetReplyStatue(&msg,&Replynfo);
	char 			*pos;
	long Length=0;
 	switch (Replynfo.ReplyState) 
	{
 	case AT_Reply_OK:
		smprintf(theApp.m_pDebuginfo, "Memory info received\n");
		pos =(char*)msg.Buffer;
		if (!pos) return ERR_UNKNOWN;
		pos = strstr((char*)pos, "*STARTUL:");
		pos=pos+9;
		Length=atol(pos);
		Length=Length*2;
		if (theApp.m_unDecodeBase64)
		{
			delete theApp.m_unDecodeBase64;
			theApp.m_unDecodeBase64=NULL;
		}
		else
			theApp.m_unDecodeBase64=new unsigned char[Length];

		return ERR_NONE;
	case AT_Reply_Error:
		return ERR_UNKNOWN;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(Replynfo);
 	default:
		return ERR_UNKNOWNRESPONSE;
	}


}
GSM_Error NXP_ReplyGetCalendarDataMemory(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	GSM_Protocol_Message tempmsg;
	tempmsg.Buffer = (unsigned char *)malloc(101);
	if(msg.Length < 100)
	{
		memcpy(tempmsg.Buffer,msg.Buffer,msg.Length);
		tempmsg.Length = msg.Length;
		tempmsg.BufferUsed = msg.Length+1;
	}
	else
	{
		memcpy(tempmsg.Buffer,msg.Buffer+msg.Length -100,100);
		if(tempmsg.Buffer[99] == 0x00)
		{
			tempmsg.Length = 99;
			tempmsg.BufferUsed =100;
		}
		else
		{
			tempmsg.Length = 100;
			tempmsg.BufferUsed =101;
		}
	}
	MTK_GetReplyStatue(&tempmsg,&Replynfo);
	free(tempmsg.Buffer);
	char			*pos;
	char			*pos2;
	unsigned char		buffer[2000]/*,buffer2[2000]*/;
	long zero=0;
	switch (Replynfo.ReplyState) 
	{
	case AT_Reply_Continue: 
	case AT_Reply_OK:
 		smprintf(theApp.m_pDebuginfo, "Phonebook entry received\n");
		pos = (char*)msg.Buffer;
		pos=strstr((char*)pos,"*GET:");
		pos=pos+5;
		zero=atol(pos);
		if (zero)
		{
			pos2 = strstr((char*)pos, ",");
			pos=pos2+1;
			ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
			
			if (!theApp.isFirst)
			{
				_mbscpy(theApp.m_unDecodeBase64,buffer);
				theApp.isFirst=true;
			}
			else
				_mbscat(theApp.m_unDecodeBase64,buffer);
		}
		else
		{
			theApp.isGetZero=true;
		}
		return ERR_NONE;
		break;
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(Replynfo);
	case AT_Reply_Error:
 		smprintf(theApp.m_pDebuginfo, "Error - too high location ?\n");
		return ERR_INVALIDLOCATION;
	case AT_Reply_CMSError:
 	     return ATGEN_HandleCMSError(Replynfo);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}
GSM_Error WINAPI InitCalendarGetEx(GSM_CalendarStatus *Status,int *totalnum,int (*pGetStatusfn)(int nCur,int nTotal))
{
	GSM_Error   error;
    //int         max_no;
	int			start;
//	int         end;
	char			req[200];
	ATGEN_GetDateTime(&theApp.m_MobileDate_time,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo);
    *totalnum = -1;
	Status->Used =0;
    theApp.m_nCalFirstInde = 0;
	theApp.m_CalendarNum = 0;
    start = 0;

    theApp.m_Caltotal_entries = 30;  //080612libaoliu
	*totalnum = theApp.m_Caltotal_entries;

//	remove(theApp.m_szCalendarTempFile);
	ZeroMemory(&theApp.m_CalendarEntry,sizeof(GSM_CalendarEntry));
	sprintf(req, "AT*STARTUL=%s\r", "/app/org/dt/data");
	theApp.isFirst=false;
	theApp.isGetZero=false;
	ReplymsgType.nCount = 2;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*STARTUL=");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*STARTUL");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;
	error = theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,NXP_ReplyGetCalendarInfo);
	if (error != ERR_NONE&& error != ERR_EMPTY) return error;
	sprintf(req,"at*get\r");
	ReplymsgType.nCount = 2;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"at*get");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*get:");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;
	while (1)
	{
		if (!theApp.isGetZero)
		{
			error = theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,NXP_ReplyGetCalendarDataMemory);
		}
		else
			break;
	}
	if (theApp.isGetZero)
	{
	sprintf(req,"at*end\r");
	ReplymsgType.nCount = 2;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"at*end");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*end:");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;
	error = theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	if (error != ERR_NONE&& error != ERR_EMPTY) 
		return error;
	}
	int Length=strlen((char *)theApp.m_unDecodeBase64);
	unsigned char *encodebuffer=new unsigned char[Length*2];
	int encodeLength=DecodeCalendar((char *)theApp.m_unDecodeBase64,(char *)encodebuffer);
	remove(theApp.m_szCalendarIndexFileTempFile);
	FILE *	file;
	file = fopen(theApp.m_szCalendarIndexFileTempFile,"a+b");
	if(file)
	{
		int len = fwrite(encodebuffer,1,encodeLength,file);
		fclose(file);
	}
	file = fopen(theApp.m_szCalendarIndexFileTempFile,"r+b");
	t_org_AllData org_data;
	ZeroMemory(&org_data,sizeof(t_org_AllData));
	if(file)
	{
		fseek(file,0,SEEK_SET);
		fread(&org_data,1,sizeof(t_org_AllData),file); 
	    fclose(file);
	}
	    GSM_CalendarEntry   *Calendar = &theApp.m_CalendarEntry; 
		GSM_DateTime tempDate;
	    GSM_DateTime startDate;
		int alarm_time_f= 6;
	remove(theApp.m_szCalendarDataFileTempFile);
	for(int i=0 ;i<30 ;i++)
	{
	     memset(&tempDate, 0, sizeof(GSM_DateTime));
	     memset(&startDate, 0, sizeof(GSM_DateTime));

		if(org_data.v_OrganizerRecords.a_Records[i].v_Enable ==1) 
		{
			 theApp.m_CalendarNum++;
			 Calendar->EntriesNum = 0;
			 itoa(i+1,Calendar->szIndex,10);
			 if (org_data.v_OrganizerRecords.a_Records[i].v_EventType==0)
			 {
			   Calendar->Type = GSM_CAL_MEETING; 
			 }
			 else
			 {
                Calendar->Type =GSM_CAL_REMINDER; 
			 }
			 int Repeattype=org_data.v_OrganizerRecords.a_Records[i].v_RepeatType;
			 if (Repeattype)
			 {
				 
				 Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				 Calendar->Entries[Calendar->EntriesNum].Number    = 1;
				 Calendar->EntriesNum++;
				 
				 
				 if (Repeattype==1)
				 {
					 Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
					 Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_DAILY;
					 Calendar->EntriesNum++;
					 
				 }
				 else if (Repeattype==2)
				 {
					 Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
					 Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_WEEKLY;
					 Calendar->EntriesNum++;
					 
				 }
				 else if (Repeattype==3)
				 {
					 Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
					 Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_MONTHLY;//30*24;
					 Calendar->EntriesNum++;
					 
				 }
				 else if (Repeattype==4)
				 {
					 Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
					 Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_YEARLY;//30*24;
					 Calendar->EntriesNum++;
					 
				 }
//  				 else
// 					 return ERR_UNKNOWN;080612libaoliu
			 }
		/* Note */
		Calendar->Entries[Calendar->EntriesNum].EntryType=CAL_TEXT;	
      //  unsigned char tempNote[MAX_PATH];
		DecodeUTF8ToUnicode(Calendar->Entries[Calendar->EntriesNum].Text, (unsigned char *)org_data.v_OrganizerRecords.a_Records[i].a_Text, strlen(org_data.v_OrganizerRecords.a_Records[i].a_Text));
		if(UnicodeLength(Calendar->Entries[Calendar->EntriesNum].Text)>0)					
		  Calendar->EntriesNum++;
		   /* start time */
		
          Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_START_DATETIME;
		  // Year
		  Calendar->Entries[Calendar->EntriesNum].Date.Year =org_data.v_OrganizerRecords.a_Records[i].v_StartTime.v_YearBitField;
		  startDate.Year = org_data.v_OrganizerRecords.a_Records[i].v_StartTime.v_YearBitField;
		  //Month
		  Calendar->Entries[Calendar->EntriesNum].Date.Month = org_data.v_OrganizerRecords.a_Records[i].v_StartTime.v_MonthBitField;
		  startDate.Month = org_data.v_OrganizerRecords.a_Records[i].v_StartTime.v_MonthBitField;
		   //Day
		 
          Calendar->Entries[Calendar->EntriesNum].Date.Day =org_data.v_OrganizerRecords.a_Records[i].v_StartTime.v_DayBitField;
		  startDate.Day = org_data.v_OrganizerRecords.a_Records[i].v_StartTime.v_DayBitField;
		  // Hour
		
          Calendar->Entries[Calendar->EntriesNum].Date.Hour = org_data.v_OrganizerRecords.a_Records[i].v_StartTime.v_HourBitField;
		  startDate.Hour = org_data.v_OrganizerRecords.a_Records[i].v_StartTime.v_HourBitField;
		  //Minute
          Calendar->Entries[Calendar->EntriesNum].Date.Minute =org_data.v_OrganizerRecords.a_Records[i].v_StartTime.v_MinutesBitField;
		  startDate.Minute = org_data.v_OrganizerRecords.a_Records[i].v_StartTime.v_MinutesBitField;
		  Calendar->EntriesNum++;
		  /* end time */
		
          Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_END_DATETIME;
		  // Year
		  Calendar->Entries[Calendar->EntriesNum].Date.Year =org_data.v_OrganizerRecords.a_Records[i].v_EndTime.v_YearBitField;
		  //Month
		  Calendar->Entries[Calendar->EntriesNum].Date.Month = org_data.v_OrganizerRecords.a_Records[i].v_EndTime.v_MonthBitField;
		   //Day
		 
          Calendar->Entries[Calendar->EntriesNum].Date.Day =org_data.v_OrganizerRecords.a_Records[i].v_EndTime.v_DayBitField;
		  // Hour
		
          Calendar->Entries[Calendar->EntriesNum].Date.Hour = org_data.v_OrganizerRecords.a_Records[i].v_EndTime.v_HourBitField;
		  //Minute
          Calendar->Entries[Calendar->EntriesNum].Date.Minute =org_data.v_OrganizerRecords.a_Records[i].v_EndTime.v_MinutesBitField;
		  Calendar->EntriesNum++;

		  
		  /* Alarm Time flag */
		  alarm_time_f=org_data.v_OrganizerRecords.a_Records[i].v_InformType;
		  if (alarm_time_f !=6)
		  {
			  COleDateTime oleStartDateTime(startDate.Year,startDate.Month,startDate.Day,startDate.Hour,startDate.Minute,startDate.Second);
			  COleDateTime  dtAlarm;
			  COleDateTimeSpan dtAlarmLeadTimeSpan;

			  Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_ALARM_DATETIME;
			  if ( alarm_time_f == 0 )
			  {

				  Calendar->Entries[Calendar->EntriesNum].Date.Year = startDate.Year;
				  Calendar->Entries[Calendar->EntriesNum].Date.Month = startDate.Month;
				  Calendar->Entries[Calendar->EntriesNum].Date.Day = startDate.Day;
				  Calendar->Entries[Calendar->EntriesNum].Date.Hour = startDate.Hour;
				  Calendar->Entries[Calendar->EntriesNum].Date.Minute = startDate.Minute;
				  Calendar->Entries[Calendar->EntriesNum].Date.Second = startDate.Second;
			  }
			  else if (alarm_time_f == 1)
			  {
				  dtAlarmLeadTimeSpan.SetDateTimeSpan(0,0,5,0);	
		          dtAlarm = oleStartDateTime - dtAlarmLeadTimeSpan;	

				  Calendar->Entries[Calendar->EntriesNum].Date.Year = dtAlarm.GetYear();
				  Calendar->Entries[Calendar->EntriesNum].Date.Month = dtAlarm.GetMonth();
				  Calendar->Entries[Calendar->EntriesNum].Date.Day = dtAlarm.GetDay();
// 				  if (startDate.Minute<5)
// 				  {
// 					  if (startDate.Hour<1)
// 					  {
// 						  startDate.Hour=23;
// 						  Calendar->Entries[Calendar->EntriesNum].Date.Day = startDate.Day-1;
// 	
// 					  }
// 					  else
// 					  startDate.Hour=startDate.Hour-1;
// 					  startDate.Minute=55+startDate.Minute;
// 					  
// 					  
// 				  }
// 				  else
// 					 startDate.Minute=startDate.Minute-5;
				  Calendar->Entries[Calendar->EntriesNum].Date.Hour = dtAlarm.GetHour();
				  Calendar->Entries[Calendar->EntriesNum].Date.Minute = dtAlarm.GetMinute();
				  Calendar->Entries[Calendar->EntriesNum].Date.Second = dtAlarm.GetSecond();
				  
				  
			  }
			  else if (alarm_time_f == 2)
			  {
				  dtAlarmLeadTimeSpan.SetDateTimeSpan(0,0,10,0);	
		          dtAlarm = oleStartDateTime - dtAlarmLeadTimeSpan;	

				   Calendar->Entries[Calendar->EntriesNum].Date.Year = dtAlarm.GetYear();
				  Calendar->Entries[Calendar->EntriesNum].Date.Month = dtAlarm.GetMonth();
				  Calendar->Entries[Calendar->EntriesNum].Date.Day = dtAlarm.GetDay();
// 				  if (startDate.Minute<10)
// 				  {
// 					  if (startDate.Hour<1)
// 					  {
// 						  startDate.Hour=23;
// 						  Calendar->Entries[Calendar->EntriesNum].Date.Day = startDate.Day-1;
// 
// 					  }
// 					  else
// 						  startDate.Hour=startDate.Hour-1;
// 					  startDate.Minute=50+startDate.Minute;
// 				  }
// 				  else
// 					  startDate.Minute=startDate.Minute-10;
			      Calendar->Entries[Calendar->EntriesNum].Date.Hour = dtAlarm.GetHour();
				  Calendar->Entries[Calendar->EntriesNum].Date.Minute = dtAlarm.GetMinute();
				  Calendar->Entries[Calendar->EntriesNum].Date.Second = dtAlarm.GetSecond();
				  
				  
			  }
			  else if (alarm_time_f == 3)
			  {
				  dtAlarmLeadTimeSpan.SetDateTimeSpan(0,0,60,0);	
		          dtAlarm = oleStartDateTime - dtAlarmLeadTimeSpan;	

				   Calendar->Entries[Calendar->EntriesNum].Date.Year = dtAlarm.GetYear();
				  Calendar->Entries[Calendar->EntriesNum].Date.Month = dtAlarm.GetMonth();
				  Calendar->Entries[Calendar->EntriesNum].Date.Day = dtAlarm.GetDay();
				  
// 				  
// 					  if (startDate.Hour<1)
// 					  {
// 						  startDate.Hour=23;
// 						  Calendar->Entries[Calendar->EntriesNum].Date.Day = startDate.Day-1;
// 
// 					  }
// 					  else
// 						  startDate.Hour=startDate.Hour-1;
				  Calendar->Entries[Calendar->EntriesNum].Date.Hour = dtAlarm.GetHour();
				  Calendar->Entries[Calendar->EntriesNum].Date.Minute = dtAlarm.GetMinute();
				  Calendar->Entries[Calendar->EntriesNum].Date.Second = dtAlarm.GetSecond();
				  
				  
			  }
			  else if (alarm_time_f == 4)
			  {
				  dtAlarmLeadTimeSpan.SetDateTimeSpan(0,0,1440,0);	
		          dtAlarm = oleStartDateTime - dtAlarmLeadTimeSpan;	

				   Calendar->Entries[Calendar->EntriesNum].Date.Year = dtAlarm.GetYear();
				  Calendar->Entries[Calendar->EntriesNum].Date.Month = dtAlarm.GetMonth();
				  Calendar->Entries[Calendar->EntriesNum].Date.Day = dtAlarm.GetDay();
// 				  if (startDate.Day-1==0)
// 				  {
// 				   Calendar->Entries[Calendar->EntriesNum].Date.Month = startDate.Month-1;
// 				   if ((startDate.Month-1)==4 ||(startDate.Month-1)==6 ||(startDate.Month-1)==9|| (startDate.Month-1)==11)
// 				   {
// 					Calendar->Entries[Calendar->EntriesNum].Date.Day =30;
// 
// 				   }
// 				   else if ((startDate.Month-1)==2)
// 				   {
// 					 Calendar->Entries[Calendar->EntriesNum].Date.Day =28;
// 
// 				   }
// 				   else
// 					  Calendar->Entries[Calendar->EntriesNum].Date.Day =31;
// 
// 				  }
		           Calendar->Entries[Calendar->EntriesNum].Date.Hour = dtAlarm.GetHour();
				  Calendar->Entries[Calendar->EntriesNum].Date.Minute = dtAlarm.GetMinute();
				  Calendar->Entries[Calendar->EntriesNum].Date.Second = dtAlarm.GetSecond();
				  
			  }
			  else if (alarm_time_f == 5)
			  {
				  dtAlarmLeadTimeSpan.SetDateTimeSpan(0,0,10080,0);	
		          dtAlarm = oleStartDateTime - dtAlarmLeadTimeSpan;	

				   Calendar->Entries[Calendar->EntriesNum].Date.Year = dtAlarm.GetYear();
				  Calendar->Entries[Calendar->EntriesNum].Date.Month = dtAlarm.GetMonth();
				  Calendar->Entries[Calendar->EntriesNum].Date.Day = dtAlarm.GetDay();
// 				  if (startDate.Day-7==0)
// 				  {
// 					  Calendar->Entries[Calendar->EntriesNum].Date.Month = startDate.Month-1;
// 					  if ((startDate.Month-1)==4 ||(startDate.Month-1)==6 ||(startDate.Month-1)==9|| (startDate.Month-1)==11)
// 					  {
// 						Calendar->Entries[Calendar->EntriesNum].Date.Day =30;
// 					  }
// 					  else if ((startDate.Month-1)==2)
// 					  {
// 						Calendar->Entries[Calendar->EntriesNum].Date.Day =28;
// 					  }
// 					  else
// 						  Calendar->Entries[Calendar->EntriesNum].Date.Day =31;
// 					  
// 				  }
// 				  if (startDate.Day<7)
// 				  {
// 					  Calendar->Entries[Calendar->EntriesNum].Date.Month = startDate.Month-1;
// 					  if ((startDate.Month-1)==4 ||(startDate.Month-1)==6 ||(startDate.Month-1)==9|| (startDate.Month-1)==11)
// 					  {
// 						Calendar->Entries[Calendar->EntriesNum].Date.Day =23+startDate.Day;
// 					  }
// 					  else if ((startDate.Month-1)==2)
// 					  {
// 						Calendar->Entries[Calendar->EntriesNum].Date.Day =21+startDate.Day;
// 					  }
// 					  else
// 						  Calendar->Entries[Calendar->EntriesNum].Date.Day =24+startDate.Day;
// 					  
// 				  }

				  Calendar->Entries[Calendar->EntriesNum].Date.Hour = dtAlarm.GetHour();
				  Calendar->Entries[Calendar->EntriesNum].Date.Minute = dtAlarm.GetMinute();
				  Calendar->Entries[Calendar->EntriesNum].Date.Second = dtAlarm.GetSecond();
				  
			  }
			  Calendar->EntriesNum++;
		  }
		 file = fopen(theApp.m_szCalendarDataFileTempFile,"a+b");
		 if(file)
		{
			fwrite(Calendar,1,sizeof(GSM_CalendarEntry),file);
			fclose(file);
		}
	     ZeroMemory(&theApp.m_CalendarEntry,sizeof(GSM_CalendarEntry));

		}
		//080811libaoliu
// 		file = fopen(theApp.m_szCalendarDataFileTempFile,"a+b");
// 		if(file)
// 		{
// 			fwrite(Calendar,1,sizeof(GSM_CalendarEntry),file);
// 			fclose(file);
// 		}
// 	     ZeroMemory(&theApp.m_CalendarEntry,sizeof(GSM_CalendarEntry));

	}
	if (encodebuffer)
	{
		delete encodebuffer;
	}
	if (theApp.m_unDecodeBase64)
	{
		delete theApp.m_unDecodeBase64;
		theApp.m_unDecodeBase64=NULL;
	}
	Status->Used = theApp.m_CalendarNum;
	return ERR_NONE;

}

GSM_Error SF_ReplyGetCalendar(GSM_Protocol_Message msg)
{
    GSM_Error 		error;
	int nIndex = 0;
//	FILE *	file;

	GSM_ATReplayInfo Replynfo;
	SF_GetReplyStatue(&msg,&Replynfo);
//	if(Replynfo.ReplyState == AT_Reply_Connect)
//		return ERR_NEEDANOTHEDATA;

//	char			*pos;
//	unsigned char		buffer[500];

	switch (Replynfo.ReplyState) {
//	case AT_Reply_Continue:
	case AT_Reply_OK:
		{
		int nPos = 0;
		GSM_ToDoEntry		ToDo;

		theApp.m_MobileDate_time.Timezone = 0;
		error = SF_DecodeVCALENDAR_VTODO(msg.Buffer,&nPos, theApp.m_CalNote, &ToDo,theApp.m_MobileDate_time.Timezone);
        return error;
		/*
		error = SF_DecodeVCALENDAR_VTODO(msg.Buffer,&nPos, &theApp.m_CalendarEntry, &ToDo,theApp.m_MobileDate_time.Timezone);

		file = fopen(theApp.m_szCalendarTempFile,"a+b");
		if(file)
		 {
			fwrite(&theApp.m_CalendarEntry,1,sizeof(GSM_CalendarEntry),file);
			fclose(file);
		 }

		ZeroMemory(&theApp.m_CalendarEntry,sizeof(GSM_CalendarEntry));
	
        return error;
		*/
		}
		break;
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(Replynfo);
	case AT_Reply_Error:
 		smprintf(theApp.m_pDebuginfo, "Error - too high location ?\n");
		return ERR_INVALIDLOCATION;
	case AT_Reply_CMSError:
 	    return ATGEN_HandleCMSError(Replynfo);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error SF_ReplyGetTalNum(GSM_Protocol_Message msg)
{

	int nIndex = 0;
	GSM_ATReplayInfo Replynfo;
	SF_GetReplyStatue(&msg,&Replynfo);
	char			*pos;
//	unsigned char		buffer[500];

	switch (Replynfo.ReplyState) {
//	case AT_Reply_Continue:
	case AT_Reply_OK:
		{
			pos = strstr((char*)msg.Buffer, "+VCALR=:");
			if (pos == NULL)
			{
				if(Replynfo.ReplyState == AT_Reply_Continue)
					return ERR_UNKNOWN;
				else return ERR_NONE; 
			}
			pos += 8; 

            while (*pos && !isdigit(*pos)) pos++;
 			nIndex = atoi(pos) ;
			theApp.m_CalendarNum = nIndex;

			 return ERR_NONE; 
		}
		break;
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(Replynfo);
	case AT_Reply_Error:
 		smprintf(theApp.m_pDebuginfo, "Error - too high location ?\n");
		return ERR_INVALIDLOCATION;
	case AT_Reply_CMSError:
 	        return ATGEN_HandleCMSError(Replynfo);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error SF_ReplyAddCal(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	SF_GetReplyStatue(&msg,&Replynfo);
	char			*pos;
//	unsigned char		buffer[500];
	switch (Replynfo.ReplyState) 
	{
	case AT_Reply_OK:
		{
			pos = strstr((char*)msg.Buffer, "+VCALW=:");
			if (pos == NULL)
				return ERR_UNKNOWN;

			pos += 8; 
			while (*pos && !isdigit(*pos)) pos++;
 			int nIndex = atoi(pos) ;
			itoa(nIndex,theApp.m_CalNote->szIndex,10);
		}
		return ERR_NONE;
		break;
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(Replynfo);
	case AT_Reply_Error:
 		smprintf(theApp.m_pDebuginfo, "Error - too high location ?\n");
		return ERR_INVALIDLOCATION;
	case AT_Reply_CMSError:
 	        return ATGEN_HandleCMSError(Replynfo);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}
GSM_Error WINAPI Calendar_NeedCreateIndexTable(BOOL &bNeedCreate)
{
	bNeedCreate = true;
	return ERR_NONE;
}
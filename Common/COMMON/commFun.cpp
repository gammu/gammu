#include "stdafx.h"
#include "commfun.h"
#include "Common.h"
#include "coding.h"
#include "gsmcal.h"
#include "anwmobile.h"
extern CCommonApp theApp;

Debug_Info di = {DL_TEXTALL,NULL,""};
int WINAPI smfprintf(FILE *f, Debug_Level dl, const char *format, ...)
{
        va_list 		argp;
	int 			result=0;
	static unsigned char 	prevline[2000] = "", nextline[2000]="";
	static unsigned int 	linecount=0;
	unsigned char		buffer[2000];
	GSM_DateTime 		date_time;

	if (f == NULL) return 0;
	va_start(argp, format);
	result = vsprintf((char *)buffer, format, argp);
	strcat((char *)nextline, (char *)buffer);
	if (strstr((char *)buffer, "\n")) {
		if (ftell(f) < 5000000) {
			GSM_GetCurrentDateTime(&date_time);
			if (linecount > 0) {
				if (dl == DL_TEXTALLDATE || dl == DL_TEXTERRORDATE || dl == DL_TEXTDATE) {
			                fprintf(f,"%s %4d/%02d/%02d %02d:%02d:%02d: <%i> %s",
			                        DayOfWeek(date_time.Year, date_time.Month, date_time.Day),
			                        date_time.Year, date_time.Month, date_time.Day,
			                        date_time.Hour, date_time.Minute, date_time.Second,linecount,prevline);
				} else {
			                fprintf(f,"%s",prevline);
				}
			}
			linecount=0;
			if (dl == DL_TEXTALLDATE || dl == DL_TEXTERRORDATE || dl == DL_TEXTDATE) {
		                fprintf(f,"%s %4d/%02d/%02d %02d:%02d:%02d: %s",
		                        DayOfWeek(date_time.Year, date_time.Month, date_time.Day),
		                        date_time.Year, date_time.Month, date_time.Day,
		                        date_time.Hour, date_time.Minute, date_time.Second,nextline);
			} else {
		                fprintf(f,"%s",nextline);
			}
			strcpy((char *)prevline, (char *)nextline);
		}
		strcpy((char *)nextline, "");
		fflush(f);
	}
	va_end(argp);
	return result;
}

int WINAPI smprintf(Debug_Info *debugInfo, const char *format, ...)
{
	va_list		argp;
	int 		result=0;
	unsigned char	buffer[8000];
	Debug_Level	dl;
	FILE		*df;

	va_start(argp, format);
	if (debugInfo == NULL) {
		dl = di.dl;
		df = di.df;
	} else {
		dl = debugInfo->dl;
	/*	if (debugInfo->use_global)
		{
			df = di.df;
		} else*/
		{
			df = debugInfo->df;
		}
	}

	if (dl != 0) {
		result = vsprintf((char *)buffer, format, argp);
		result = smfprintf(df, dl, "%s", buffer);
	}

	va_end(argp);
	return result;
}

int WINAPI dbgprintf(const char *format, ...)
{
#ifdef _DEBUG
	va_list			argp;
	int 			result;
	static unsigned char 	nextline[2000]="";
	unsigned char		buffer[2000];
	GSM_DateTime 		date_time;

	if (di.df != NULL && (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE)) {
		va_start(argp, format);
		result = vsprintf((char *)buffer, format, argp);
		strcat((char *)nextline, (char *)buffer);
		if (strstr((char *)buffer, "\n")) {
			if (di.dl == DL_TEXTALLDATE) {
				GSM_GetCurrentDateTime(&date_time);
				fprintf(di.df,"%s %4d/%02d/%02d %02d:%02d:%02d: %s",
		                	DayOfWeek(date_time.Year, date_time.Month, date_time.Day),
		                	date_time.Year, date_time.Month, date_time.Day,
		                	date_time.Hour, date_time.Minute, date_time.Second,nextline);
			} else {
				fprintf(di.df,"%s",nextline);
			}
			strcpy((char *)nextline, "");
		}
		fflush(di.df);
		va_end(argp);
		return result;
	}
#endif
	return 0;
}

void WINAPI GSM_OSErrorInfo(Debug_Info *debugInfo, char *description)
{
	int 		i;
	unsigned char 	*lpMsgBuf;

	if(debugInfo == NULL) return;
	/* We don't use errno in win32 - GetLastError gives better info */
	if (GetLastError()!=-1) {
		if (debugInfo->dl == DL_TEXTERROR || debugInfo->dl == DL_TEXT || debugInfo->dl == DL_TEXTALL ||
		    debugInfo->dl == DL_TEXTERRORDATE || debugInfo->dl == DL_TEXTDATE || debugInfo->dl == DL_TEXTALLDATE) {
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
			);
			for (i=0;i<(int)strlen((char *)lpMsgBuf);i++) {
				if (lpMsgBuf[i] == 13 || lpMsgBuf[i] == 10) {
					lpMsgBuf[i] = ' ';
				}
			}
			smprintf(debugInfo,"[System error     - %s, %i, \"%s\"]\n",description,GetLastError(),(LPCTSTR)lpMsgBuf);
			LocalFree(lpMsgBuf);
		}
	}
	return;

	if (errno!=-1) {
		if (debugInfo->dl == DL_TEXTERROR || debugInfo->dl == DL_TEXT || debugInfo->dl == DL_TEXTALL ||
		    debugInfo->dl == DL_TEXTERRORDATE || debugInfo->dl == DL_TEXTDATE || debugInfo->dl == DL_TEXTALLDATE) {
			smprintf(debugInfo,"[System error     - %s, %i, \"%s\"]\n",description,errno,strerror(errno));
		}
	}
}
void WINAPI Fill_GSM_DateTime(GSM_DateTime *Date, time_t timet)
{
	struct tm *now;

	now  		= localtime(&timet);
	if(now!=NULL && now>0)  //by karl
	{
		Date->Year	= now->tm_year;
		Date->Month	= now->tm_mon+1;
		Date->Day	= now->tm_mday;
		Date->Hour	= now->tm_hour;
		Date->Minute	= now->tm_min;
		Date->Second	= now->tm_sec;
	}
	else
	{
		Date->Year	= 0;
		Date->Month	= 0;
		Date->Day	= 0;
		Date->Hour	= 0;
		Date->Minute	= 0;
		Date->Second	= 0;
	}
}

void WINAPI GSM_GetCurrentDateTime (GSM_DateTime *Date)
{
	Fill_GSM_DateTime(Date, time(NULL));
	if (Date->Year<1900) {
		if (Date->Year>90) Date->Year = Date->Year+1900;
			      else Date->Year = Date->Year+2000;
	}
}
char *WINAPI DayOfWeek (int year, int month, int day)
{
	int 		p,q,r,w;
	static char 	DayOfWeekChar[10];

	p=(14-month) / 12;
	q=month+12*p-2;
	r=year-p;
	w=(day+(31*q) / 12 + r + r / 4 - r / 100 + r / 400) % 7;
	strcpy(DayOfWeekChar,"");
	switch (w) {
		case 0: strcpy(DayOfWeekChar,"Sun"); break;
		case 1: strcpy(DayOfWeekChar,"Mon"); break;
		case 2: strcpy(DayOfWeekChar,"Tue"); break;
		case 3: strcpy(DayOfWeekChar,"Wed"); break;
		case 4: strcpy(DayOfWeekChar,"Thu"); break;
		case 5: strcpy(DayOfWeekChar,"Fri"); break;
		case 6: strcpy(DayOfWeekChar,"Sat"); break;
	}
	return DayOfWeekChar;
}
GSM_Error WINAPI CheckReplyFunctions(GSM_Reply_MsgType ReplyCheckType,GSM_Protocol_Message		*msg)
{
//	GSM_Phone_Data			*Data	  = &s->Phone.Data;
	bool				execute;
//	bool				available = false;

	execute=false;
	if(ReplyCheckType.nCount <=0) 
		return ERR_NONE;
	/* Binary frames like in Nokia */
	for(int i = 0 ;i<ReplyCheckType.nCount ; i++)
	{
		if (strlen((char*)ReplyCheckType.CheckInfo[i].msgtype) < 2)
		{
			if(msg->Type == 00)
			{
				if(ReplyCheckType.CheckInfo[i].msgtype[0] == msg->Buffer[0])
				{
					if (ReplyCheckType.CheckInfo[i].subtypechar!=0) 
					{
						if (ReplyCheckType.CheckInfo[i].subtypechar<=msg->Length) 
						{
							if (msg->Buffer[ReplyCheckType.CheckInfo[i].subtypechar]==ReplyCheckType.CheckInfo[i].subtype)
								execute=true;
						}			
					} 
					else execute=true;
				}
			}
			else
			{
				if (ReplyCheckType.CheckInfo[i].msgtype[0]==msg->Type) 
				{
					if (ReplyCheckType.CheckInfo[i].subtypechar!=0) 
					{
						if (ReplyCheckType.CheckInfo[i].subtypechar<=msg->Length) 
						{
							if (msg->Buffer[ReplyCheckType.CheckInfo[i].subtypechar]==ReplyCheckType.CheckInfo[i].subtype)
								execute=true;
						}			
					} 
					else execute=true;
				}
			}
		} 
		else 
		{
	//			printf("msg length %i %i\n",strlen(Reply[i].msgtype),msg->Length);
			if ((int)strlen((char*)ReplyCheckType.CheckInfo[i].msgtype)<msg->Length)
			{
				if (strncmp((char*)ReplyCheckType.CheckInfo[i].msgtype,(char*)msg->Buffer,strlen((char*)ReplyCheckType.CheckInfo[i].msgtype))==0)
					execute=true;
			}
		}
	}

	if (execute)
	{
		return ERR_NONE;
	}
	else 
	{
		return ERR_UNKNOWNFRAME;
	}
}
void WINAPI SplitLines(unsigned char *message, int messagesize, GSM_Lines *lines, unsigned char *whitespaces, int spaceslen, bool eot)
{
	int 	i,number=0,j,q;
	bool 	whitespace=true, nowwhite;

	for (i=0;i<MAX_LINES*2;i++) lines->numbers[i]=0;

	for (i=0;i<messagesize;i++) 
	{
		nowwhite = false;
	/*	for (j=0;j<spaceslen;j++) {
			if (whitespaces[j] == message[i]) {
				nowwhite = true;
				break;
			}
		}*/
		j = 0;
		q=i;
		while(j<spaceslen)
		{
			if (whitespaces[j] == message[q]) 
			{
				j++;
				q++;
			}
			else 
				break;
		}
		if (j == spaceslen)
		{
				nowwhite = true;
			//	i = i-1;
		}

		if (whitespace) {			
			if (!nowwhite) {
				lines->numbers[number]=i;
				number++;
				whitespace=false;
			}
			else
			{
			//	lines->numbers[number]=i;
			//	number++;
			//	lines->numbers[number]=i;
			//	number++;
				whitespace=true;
				i += (spaceslen-1);

			}
		} else {
			if (nowwhite) {
				lines->numbers[number]=i;
				number++;
				whitespace=true;
				i=q-1;
			}

		}
	}
    	if (eot && !whitespace) lines->numbers[number]=messagesize;
}

char *WINAPI GetLineString(unsigned char *message, GSM_Lines lines, int start)
{
	static char retval[8000];

	memcpy(retval,message + lines.numbers[start*2-2],lines.numbers[start*2-2+1]-lines.numbers[start*2-2]);
	retval[lines.numbers[start*2-2+1]-lines.numbers[start*2-2]]=0;

	return retval;
}

void WINAPI CopyLineString(unsigned char *dest, unsigned char *src, GSM_Lines lines, int start)
{
	memcpy(dest,GetLineString(src, lines, start),strlen(GetLineString(src, lines, start)));
	dest[strlen(GetLineString(src, lines, start))] = 0;
}
BOOL WINAPI GetModelData(char *model, char *number, char *irdamodel,OnePhoneModel *pMobileInfo)
{
	POSITION pos = theApp.m_SupportModelList.GetTailPosition();
	while(pos)
	{
		OnePhoneModel* pSupportMobileInfo = (OnePhoneModel*)theApp.m_SupportModelList.GetPrev(pos);
		if (model !=NULL) {
			if (strcmp (model, pSupportMobileInfo->model) == 0) 
			{
				memcpy(pMobileInfo,pSupportMobileInfo,sizeof(OnePhoneModel));
				return TRUE;
			}
		}
		if (number !=NULL) {
			if (strcmp (number, pSupportMobileInfo->number) == 0)
			{
				memcpy(pMobileInfo,pSupportMobileInfo,sizeof(OnePhoneModel));
				return TRUE;
			}
		}
		if (irdamodel !=NULL) {
			if (strcmp (irdamodel, pSupportMobileInfo->irdamodel) == 0) 
			{
				memcpy(pMobileInfo,pSupportMobileInfo,sizeof(OnePhoneModel));
				return TRUE;
			}
		}


	}
	return FALSE;
}
BOOL WINAPI GetModelDataEx(char *model, char *number, char *irdamodel,OnePhoneModel *pMobileInfo,int nCompanyID)
{
	char szCompany[MAX_PATH];
	szCompany[0] = '\0';
	switch(nCompanyID)
	{
		case Nokia:
			wsprintf(szCompany,"Nokia");
			break;
		case SonyEricsson:
			wsprintf(szCompany,"SE");
			break;
		case Siemens:
			wsprintf(szCompany,"Siemens");
			break; 
		case ASUS:
			wsprintf(szCompany,"ASUS");
			break; 
		case MOTO:
			wsprintf(szCompany,"MOTO");
			break;
		case Samsung:// v1.2.0.0
			wsprintf(szCompany,"Samsung");
			break; 
		case Sharp:
			wsprintf(szCompany,"Sharp");
			break; 
		case Panasonic:
			wsprintf(szCompany,"Panasonic");
			break; 
		case LG:
			wsprintf(szCompany,"LG");
			break; 
		case SAGEM:
			wsprintf(szCompany,"SAGEM");
			break; 
		case Toshiba:
			wsprintf(szCompany,"Toshiba");
			break;
		case MTK:
			wsprintf(szCompany,"MTK");
			break;
		case NXP:
			wsprintf(szCompany,"PHILIPS");
			break;
		default:
			break;

	}
	POSITION pos = theApp.m_SupportModelList.GetHeadPosition();
	while(pos)
	{
		OnePhoneModel* pSupportMobileInfo = (OnePhoneModel*)theApp.m_SupportModelList.GetNext(pos);
		if(_stricmp(pSupportMobileInfo->Manufacturer,szCompany)==0)
		{
			if (model !=NULL) 
			{
				if (strcmp (model, pSupportMobileInfo->model) == 0) 
				{
					memcpy(pMobileInfo,pSupportMobileInfo,sizeof(OnePhoneModel));
					return TRUE;
				}
			}
			if (number !=NULL) 
			{
				if (strcmp (number, pSupportMobileInfo->number) == 0)
				{
					memcpy(pMobileInfo,pSupportMobileInfo,sizeof(OnePhoneModel));
					return TRUE;
				}
			}
			if (irdamodel !=NULL) 
			{
				if (strcmp (irdamodel, pSupportMobileInfo->irdamodel) == 0) 
				{
					memcpy(pMobileInfo,pSupportMobileInfo,sizeof(OnePhoneModel));
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}
bool WINAPI IsPhoneFeatureAvailable(OnePhoneModel *model, Feature feature)
{
	int	i	= 0;
	bool	retval  = false;

	while (model->features[i] != F_NONE) {
		if (model->features[i] == feature) {
			retval = true;
			break;
		}
		i++;
	}
	return retval;
}

bool WINAPI ReadVCALText_ADR(unsigned char *Buffer, char *Start,unsigned char *Value)
{
    char buff[2000];
    char buff2[2000];
	int begin = 0;
	int /*len1, len2,*/i; // v2.0.0.3 ; by mingfa for UTF-8 EU special char
//    unsigned char *tempValue; // v2.0.0.3 
	int nStartchar = 0;
	int nMax= 0 ;
	Value[0] = 0x00;
	Value[1] = 0x00;

	strcpy(buff,Start);
	strcat(buff,":");
	if (!strncmp((char *)Buffer,buff,strlen(buff))) {
	/*	if( *(Buffer+strlen(Start)+1) == ';' )
			begin = 1;
		else*/
		begin = 0;
		EncodeUnicode(Value,Buffer+strlen(Start)+1+begin,strlen((char *)Buffer)-(strlen(Start)+1+begin));
//		dbgprintf("ReadVCalText is \"%s\"\n",DecodeUnicodeConsole(Value));
		return true;
	}
	/* SE T68i */
	strcpy(buff,Start);
	strcat(buff,";ENCODING=QUOTED-PRINTABLE:");
	if (!strncmp((char*)Buffer,buff,strlen(buff)))
	{
		nStartchar = 27;
		nMax = strlen((char *)Buffer)-(strlen(Start));
		begin = 0;
		while(*(Buffer+strlen(Start)+nStartchar) == ';' && nStartchar <nMax)
		{
			begin++;
			nStartchar++;

		}
		/* v2.0.0.3 marked by mingfa for UTF-8 EU special char
		DecodeUTF8QuotedPrintable(Value,Buffer+strlen(Start)+27+begin,strlen((char*)Buffer)-(strlen(Start)+27+begin));
	
		DecodeUTF2String(buff,Value,strlen((char*)Value));//peggy +
		EncodeUnicode(Value,(unsigned char*)buff,strlen(buff)+1);//peggy +
		*/
		// v2.0.0.3 added by mingfa for UTF-8 EU special char ( ASCII specila code )
        DecodeUTF8QuotedPrintable((unsigned char *)buff,Buffer+strlen(Start)+27+begin,strlen((char *)Buffer)-(strlen(Start)+27+begin));
		buff2[0]='\0';
		for(i = 0 ;i<begin;i++)
		{
			strcat(buff2,";");
		}
		strcat(buff2,buff);
	/*	len1 = strlen(buff2);
		tempValue = Value;
		for ( len2 = 0 ; len2 < len1 ; len2++ )
		{
         *tempValue = 0x00;
		 tempValue++;
		 *tempValue = buff2[len2];
		 tempValue++;
		}
		// null terminate
        *tempValue = 0x00;
	    tempValue++;
        *tempValue = 0x00;
        // Mingfa--
*/
		EncodeUnicode(Value,(unsigned char*)buff2,strlen(buff2)+1);//peggy +
		return true;
	}
	strcpy(buff,Start);
	strcat(buff,";CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:");
	if (!strncmp((char*)Buffer,buff,strlen(buff)))
	{
		nStartchar = 41;
		nMax = strlen((char *)Buffer)-(strlen(Start));
		begin = 0;
		while(*(Buffer+strlen(Start)+nStartchar) == ';' && nStartchar <nMax)
		{
			begin++;
			nStartchar++;

		}

		DecodeUTF8QuotedPrintable(Value,Buffer+strlen(Start)+41+begin,strlen((char*)Buffer)-(strlen(Start)+41+begin));
		//memcpy(	buff , Value , strlen(Value)+2);
		/*
		DecodeUTF2String(buff,Value,strlen((char*)Value));
		buff2[0]='\0';
		for(i = 0 ;i<begin;i++)
		{
			strcat(buff2,";");
		}
		strcat(buff2,buff);
		EncodeUnicode(Value,(unsigned char*)buff2,strlen(buff2)+1);*/
		//for _UNICODE

		unsigned char ubuff[2000];
		unsigned char ubuff2[2000];

		DecodeUTF8ToUnicode(ubuff,Value,strlen((char*)Value));
		buff2[0]='\0';
		for(i = 0 ;i<begin;i++)
		{
			strcat(buff2,";");
		}
		if(strlen(buff2)>0)
		{
			EncodeUnicode(ubuff2,(unsigned char*)buff2,strlen(buff2)+1);
			UnicodeCat(ubuff2,ubuff);
			CopyUnicodeString(Value,ubuff2);
		}
		else
			CopyUnicodeString(Value,ubuff);
		return true;
	}
	//add for Nokis S6 +
	strcpy(buff,Start);
	strcat(buff,";ENCODING=QUOTED-PRINTABLE;CHARSET=UTF-8:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		nStartchar = 41;
		nMax = strlen((char *)Buffer)-(strlen(Start));
		begin = 0;
		while(*(Buffer+strlen(Start)+nStartchar) == ';' && nStartchar <nMax)
		{
			begin++;
			nStartchar++;

		}
		DecodeUTF8QuotedPrintable(Value,Buffer+strlen(Start)+41+begin,strlen((char*)Buffer)-(strlen(Start)+41)+begin);
		//memcpy(	buff , Value , strlen(Value)+2);
		/*
		DecodeUTF2String(buff,Value,strlen((char*)Value));
		buff2[0]='\0';
		for(i = 0 ;i<begin;i++)
		{
			strcat(buff2,";");
		}
		strcat(buff2,buff);
		EncodeUnicode(Value,(unsigned char*)buff2,strlen(buff2)+1);*/
		//for _UNICODE
		unsigned char ubuff[2000];
		unsigned char ubuff2[2000];

		DecodeUTF8ToUnicode(ubuff,Value,strlen((char*)Value));
		buff2[0]='\0';
		for(i = 0 ;i<begin;i++)
		{
			strcat(buff2,";");
		}
		if(strlen(buff2)>0)
		{
			EncodeUnicode(ubuff2,(unsigned char*)buff2,strlen(buff2)+1);
			UnicodeCat(ubuff2,ubuff);
			CopyUnicodeString(Value,ubuff2);
		}
		else
			CopyUnicodeString(Value,ubuff);

		return true;
	}
	//add for Nokis S6 -
	strcpy(buff,Start);
	strcat(buff,";CHARSET=UTF-8:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		nStartchar = 15;
		nMax = strlen((char *)Buffer)-(strlen(Start));
		begin = 0;
		while(*(Buffer+strlen(Start)+nStartchar) == ';' && nStartchar <nMax)
		{
			begin++;
			nStartchar++;

		}
//		DecodeUTF8(Value,Buffer+strlen(Start)+15+begin,strlen((char*)Buffer)-(strlen(Start)+15+begin));
//		dbgprintf("ReadVCalText is \"%s\"\n",DecodeUnicodeConsole(Value));
/*
		DecodeUTF2String(buff,Buffer+strlen(Start)+15+begin,strlen((char*)Buffer)-(strlen(Start)+15+begin));
		buff2[0]='\0';
		for(i = 0 ;i<begin;i++)
		{
			strcat(buff2,";");
		}
		strcat(buff2,buff);
		EncodeUnicode(Value,(unsigned char*)buff2,strlen(buff2)+1);*/
		//for _UNICODE
		unsigned char ubuff[2000];
		unsigned char ubuff2[2000];

		DecodeUTF8ToUnicode(ubuff,Buffer+strlen(Start)+15+begin,strlen((char*)Buffer)-(strlen(Start)+15+begin));
		buff2[0]='\0';
		for(i = 0 ;i<begin;i++)
		{
			strcat(buff2,";");
		}
		if(strlen(buff2)>0)
		{
			EncodeUnicode(ubuff2,(unsigned char*)buff2,strlen(buff2)+1);
			UnicodeCat(ubuff2,ubuff);
			CopyUnicodeString(Value,ubuff2);
		}
		else
			CopyUnicodeString(Value,ubuff);
		return true;
	}
	strcpy(buff,Start);
	strcat(buff,";CHARSET=UTF-7:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+15) == ';' )
			begin = 1;
		else
			begin = 0;
		DecodeUTF7(Value,Buffer+strlen(Start)+15+begin,strlen((char*)Buffer)-(strlen(Start)+15+begin));
		dbgprintf("ReadVCalText is \"%s\"\n",DecodeUnicodeConsole(Value));
		return true;
	}
	strcpy(buff,Start);
	strcat(buff,";CHARSET=UTF-7;ENCODING=QUOTED-PRINTABLE:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+41) == ';' )
			begin = 1;
		else
			begin = 0;
		DecodeUTF7(Value,Buffer+strlen(Start)+41+begin,strlen((char*)Buffer)-(strlen(Start)+41+begin));
		dbgprintf("ReadVCalText is \"%s\"\n",DecodeUnicodeConsole(Value));
		return true;
	}
	
	strcpy(buff,Start);
	strcat(buff,";CHARSET=UTF-7;HOME:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+20) == ';' )
			begin = 1;
		else
			begin = 0;
		DecodeUTF7(Value,Buffer+strlen(Start)+20+begin,strlen((char*)Buffer)-(strlen(Start)+20+begin));
		dbgprintf("ReadVCalText is \"%s\"\n",DecodeUnicodeConsole(Value));
		return true;
	}
//for Sagem +

	strcpy(buff,Start);
	strcat(buff,";CHARSET=US-ASCII:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+18) == ';' )
			begin = 1;
		else
			begin = 0;
		EncodeUnicode(Value,Buffer+strlen(Start)+18+begin,strlen((char *)Buffer)-(strlen(Start)+18+begin));
		return true;
	}
	strcpy(buff,Start);
	strcat(buff,";CHARSET=US-ASCII;ENCODING=QUOTED-PRINTABLE:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+44) == ';' )
			begin = 1;
		else
			begin = 0;
		CString csCharConversion = Buffer;
		csCharConversion.Replace("=3D","=");
		csCharConversion.Replace("=0D","\r");
		csCharConversion.Replace("=0A","\n");
		sprintf((char *)Buffer,"%s",csCharConversion);
		EncodeUnicode(Value,Buffer+strlen(Start)+44+begin,strlen((char *)Buffer)-(strlen(Start)+44+begin));
		return true;
	}
//for Sagem -

	return false;
}
bool WINAPI SAGEM_ReadVCALText(unsigned char *Buffer, char *Start,unsigned char *Value,BOOL bSkipFirst)
{
    char buff[2000];
	int begin = 0;
//	int len1, len2; // v2.0.0.3 ; by mingfa for UTF-8 EU special char
//    unsigned char *tempValue; // v2.0.0.3 

	Value[0] = 0x00;
	Value[1] = 0x00;

	strcpy(buff,Start);
	strcat(buff,":");
	if (!strncmp((char *)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+1) == ';' )
			begin = 1;
		else
			begin = 0;
		if(bSkipFirst == false) begin =0;
		EncodeUnicode(Value,Buffer+strlen(Start)+1+begin,strlen((char *)Buffer)-(strlen(Start)+1+begin));
//		dbgprintf("ReadVCalText is \"%s\"\n",DecodeUnicodeConsole(Value));
		return true;
	}

   // added for 9@9u  by Mingfa
	strcpy(buff,Start);
	strcat(buff,";ENCODING=QUOTED-PRINTABLE;CHARSET=UTF-8:"); // len = 41
	if (!strncmp((char*)Buffer,buff,strlen(buff))) 
	{
	//	if( *(Buffer+strlen(Start)+41) == '=' && *(Buffer+strlen(Start)+44) == '=')
			DecodeUTF8QuotedPrintable(Value,Buffer+strlen(Start)+41,strlen((char*)Buffer)-(strlen(Start)+41));
//		else
//			memcpy(Value,Buffer+strlen(Start)+41,1+strlen((char*)Buffer)-(strlen(Start)+41));
//		DecodeUTF2String(buff,Value,strlen((char*)Value));
//		EncodeUnicode(Value,(unsigned char*)buff,strlen(buff)+1);
		//for _UNICODE
		unsigned char ubuff[4000];
		DecodeUTF8ToUnicode(ubuff,Value,strlen((char*)Value));
		CopyUnicodeString(Value,ubuff);
		return true;
	}
	// end add


	/* SE T68i */
	strcpy(buff,Start);
	strcat(buff,";ENCODING=QUOTED-PRINTABLE:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+27) == ';' )
			begin = 1;
		else
			begin = 0;		
		if(bSkipFirst == false) begin =0;
        DecodeUTF8QuotedPrintable((unsigned char *)buff,Buffer+strlen(Start)+27+begin,strlen((char *)Buffer)-(strlen(Start)+27+begin));
       
		EncodeUnicode(Value,(unsigned char*)buff,strlen(buff)+1);//peggy +

		return true;
	}
 


	strcpy(buff,Start);
	strcat(buff,";CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) 
	{
	//	if( *(Buffer+strlen(Start)+41) == '=' && *(Buffer+strlen(Start)+44) == '=')
			DecodeUTF8QuotedPrintable(Value,Buffer+strlen(Start)+41,strlen((char*)Buffer)-(strlen(Start)+41));
//		else
//			memcpy(Value,Buffer+strlen(Start)+41,1+strlen((char*)Buffer)-(strlen(Start)+41));
//		DecodeUTF2String(buff,Value,strlen((char*)Value));
//		EncodeUnicode(Value,(unsigned char*)buff,strlen(buff)+1);
		//for _UNICODE
		unsigned char ubuff[4000];
		DecodeUTF8ToUnicode(ubuff,Value,strlen((char*)Value));
		CopyUnicodeString(Value,ubuff);
		return true;
	}
	//add for Nokis S6 +
	strcpy(buff,Start);
	strcat(buff,";ENCODING=QUOTED-PRINTABLE;CHARSET=UTF-8:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
//		if( *(Buffer+strlen(Start)+41) == '=' && *(Buffer+strlen(Start)+44) == '=')
			DecodeUTF8QuotedPrintable(Value,Buffer+strlen(Start)+41,strlen((char*)Buffer)-(strlen(Start)+41));
//		else
//			memcpy(Value,Buffer+strlen(Start)+41,1+strlen((char*)Buffer)-(strlen(Start)+41));
//		DecodeUTF2String(buff,Value,strlen((char*)Value));
//		EncodeUnicode(Value,(unsigned char*)buff,strlen(buff)+1);
		//for _UNICODE
		unsigned char ubuff[4000];
		DecodeUTF8ToUnicode(ubuff,Value,strlen((char*)Value));
		CopyUnicodeString(Value,ubuff);

		return true;
	}
	//add for Nokis S6 -
	strcpy(buff,Start);
	strcat(buff,";CHARSET=UTF-8:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+15) == ';' )
			begin = 1;
		else
			begin = 0;		
		if(bSkipFirst == false) begin =0;
//		DecodeUTF2String(buff,Buffer+strlen(Start)+15+begin,strlen((char*)Buffer)-(strlen(Start)+15+begin));
//		EncodeUnicode(Value,(unsigned char*)buff,strlen(buff)+1);//peggy +
		//for _UNICODE
		unsigned char ubuff[4000];
		DecodeUTF8ToUnicode(ubuff,Buffer+strlen(Start)+15+begin,strlen((char*)Buffer)-(strlen(Start)+15+begin));
		CopyUnicodeString(Value,ubuff);

		return true;
	}
	strcpy(buff,Start);
	strcat(buff,";CHARSET=UTF-7:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+15) == ';' )
			begin = 1;
		else
			begin = 0;
		if(bSkipFirst == false) begin =0;
		DecodeUTF7(Value,Buffer+strlen(Start)+15+begin,strlen((char*)Buffer)-(strlen(Start)+15+begin));
		dbgprintf("ReadVCalText is \"%s\"\n",DecodeUnicodeConsole(Value));
		return true;
	}
	strcpy(buff,Start);
	strcat(buff,";CHARSET=UTF-7;ENCODING=QUOTED-PRINTABLE:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+41) == ';' )
			begin = 1;
		else
			begin = 0;
		if(bSkipFirst == false) begin =0;
		DecodeUTF7(Value,Buffer+strlen(Start)+41+begin,strlen((char*)Buffer)-(strlen(Start)+41+begin));
		dbgprintf("ReadVCalText is \"%s\"\n",DecodeUnicodeConsole(Value));
		return true;
	}
	
	strcpy(buff,Start);
	strcat(buff,";CHARSET=UTF-7;HOME:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+20) == ';' )
			begin = 1;
		else
			begin = 0;
		if(bSkipFirst == false) begin =0;
		DecodeUTF7(Value,Buffer+strlen(Start)+20+begin,strlen((char*)Buffer)-(strlen(Start)+20+begin));
		dbgprintf("ReadVCalText is \"%s\"\n",DecodeUnicodeConsole(Value));
		return true;
	}

	//Bobby 10.18.05 For ISO--8859 used in samsung when there an = sign
	strcpy(buff,Start);
	strcat(buff,";CHARSET=ISO-8859-1;ENCODING=QUOTED-PRINTABLE:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+46) == ';' )
			begin = 1;
		else
			begin = 0;
		if(bSkipFirst == false) begin =0;
		CString csCharConversion = Buffer;
		csCharConversion.Replace("=3D","=");
		csCharConversion.Replace("=0D","\r");
		csCharConversion.Replace("=0A","\n");
		sprintf((char *)Buffer,"%s",csCharConversion);
		EncodeUnicode(Value,Buffer+strlen(Start)+46+begin,strlen((char *)Buffer)-(strlen(Start)+46+begin));
		return true;
	}
//for Sagem +
	strcpy(buff,Start);
	strcat(buff,";CHARSET=US-ASCII:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+18) == ';' )
			begin = 1;
		else
			begin = 0;
		if(bSkipFirst == false) begin =0;
		EncodeUnicode(Value,Buffer+strlen(Start)+18+begin,strlen((char *)Buffer)-(strlen(Start)+18+begin));
		return true;
	}

	strcpy(buff,Start);
	strcat(buff,";CHARSET=US-ASCII;ENCODING=QUOTED-PRINTABLE:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+44) == ';' )
			begin = 1;
		else
			begin = 0;
		if(bSkipFirst == false) begin =0;
		CString csCharConversion = Buffer;
		csCharConversion.Replace("=3D","=");
		csCharConversion.Replace("=0D","\r");
		csCharConversion.Replace("=0A","\n");
		sprintf((char *)Buffer,"%s",csCharConversion);
		EncodeUnicode(Value,Buffer+strlen(Start)+44+begin,strlen((char *)Buffer)-(strlen(Start)+44+begin));
		return true;
	}
//for Sagem -

	return false;
}
bool WINAPI ReadVCALText(unsigned char *Buffer, char *Start,unsigned char *Value,BOOL bSkipFirst)
{
    char buff[2000];
	int begin = 0;
//	int len1, len2; // v2.0.0.3 ; by mingfa for UTF-8 EU special char
//    unsigned char *tempValue; // v2.0.0.3 

	Value[0] = 0x00;
	Value[1] = 0x00;

	strcpy(buff,Start);
	strcat(buff,":");
	if (!strncmp((char *)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+1) == ';' )
			begin = 1;
		else
			begin = 0;
		if(bSkipFirst == false) begin =0;
		EncodeUnicode(Value,Buffer+strlen(Start)+1+begin,strlen((char *)Buffer)-(strlen(Start)+1+begin));
//		dbgprintf("ReadVCalText is \"%s\"\n",DecodeUnicodeConsole(Value));
		return true;
	}
	/* SE T68i */
	strcpy(buff,Start);
	strcat(buff,";ENCODING=QUOTED-PRINTABLE:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+27) == ';' )
			begin = 1;
		else
			begin = 0;		
		if(bSkipFirst == false) begin =0;
		/* v2.0.0.3 marked by mingfa for UTF-8 EU special char
		DecodeUTF8QuotedPrintable(Value,Buffer+strlen(Start)+27+begin,strlen((char*)Buffer)-(strlen(Start)+27+begin));
	
		DecodeUTF2String(buff,Value,strlen((char*)Value));//peggy +
		EncodeUnicode(Value,(unsigned char*)buff,strlen(buff)+1);//peggy +
		*/
		// v2.0.0.3 added by mingfa for UTF-8 EU special char ( ASCII specila code )
        DecodeUTF8QuotedPrintable((unsigned char *)buff,Buffer+strlen(Start)+27+begin,strlen((char *)Buffer)-(strlen(Start)+27+begin));
       
/*		len1 = strlen(buff);
		tempValue = Value;
		for ( len2 = 0 ; len2 < len1 ; len2++ )
		{
         *tempValue = 0x00;
		 tempValue++;
		 *tempValue = buff[len2];
		 tempValue++;
		}
		// null terminate
        *tempValue = 0x00;
	    tempValue++;
        *tempValue = 0x00;
        // Mingfa--*/
		EncodeUnicode(Value,(unsigned char*)buff,strlen(buff)+1);//peggy +

		return true;
	}
	strcpy(buff,Start);
	strcat(buff,";CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:");
	if (!_strnicmp((char*)Buffer,buff,strlen(buff))) {

		if( *(Buffer+strlen(Start)+41) == ';' )
			begin = 1;
		else
			begin = 0;		
		if(bSkipFirst == false) begin =0;

		DecodeUTF8QuotedPrintable(Value,Buffer+strlen(Start)+41+begin,strlen((char*)Buffer)-(strlen(Start)+41+begin));
		//memcpy(	buff , Value , strlen(Value)+2);
	//	DecodeUTF2String(buff,Value,strlen((char*)Value));
	//	EncodeUnicode(Value,(unsigned char*)buff,strlen(buff)+1);
		//for _UNICODE
		unsigned char ubuff[4000];
		DecodeUTF8ToUnicode(ubuff,Value,strlen((char*)Value));
		CopyUnicodeString(Value,ubuff);

		return true;
	}
	//add for Nokis S6 +
	strcpy(buff,Start);
	strcat(buff,";ENCODING=QUOTED-PRINTABLE;CHARSET=UTF-8:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+41) == ';' )
			begin = 1;
		else
			begin = 0;		
		if(bSkipFirst == false) begin =0;

		DecodeUTF8QuotedPrintable(Value,Buffer+strlen(Start)+41+begin,strlen((char*)Buffer)-(strlen(Start)+41+begin));
		//memcpy(	buff , Value , strlen(Value)+2);
	//	DecodeUTF2String(buff,Value,strlen((char*)Value));
	//	EncodeUnicode(Value,(unsigned char*)buff,strlen(buff)+1);
		//for _UNICODE
		unsigned char ubuff[4000];
		DecodeUTF8ToUnicode(ubuff,Value,strlen((char*)Value));
		CopyUnicodeString(Value,ubuff);

		return true;
	}
	//add for Nokis S6 -
	strcpy(buff,Start);
	strcat(buff,";CHARSET=UTF-8:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+15) == ';' )
			begin = 1;
		else
			begin = 0;		
		if(bSkipFirst == false) begin =0;
//		DecodeUTF2String(buff,Buffer+strlen(Start)+15+begin,strlen((char*)Buffer)-(strlen(Start)+15+begin));
//		EncodeUnicode(Value,(unsigned char*)buff,strlen(buff)+1);
		//for _UNICODE
		unsigned char ubuff[4000];
		DecodeUTF8ToUnicode(ubuff,Buffer+strlen(Start)+15+begin,strlen((char*)Buffer)-(strlen(Start)+15+begin));
		CopyUnicodeString(Value,ubuff);

		return true;
	}
	strcpy(buff,Start);
	strcat(buff,";CHARSET=UTF-7:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+15) == ';' )
			begin = 1;
		else
			begin = 0;
		if(bSkipFirst == false) begin =0;
		DecodeUTF7(Value,Buffer+strlen(Start)+15+begin,strlen((char*)Buffer)-(strlen(Start)+15+begin));
		dbgprintf("ReadVCalText is \"%s\"\n",DecodeUnicodeConsole(Value));
		return true;
	}
	strcpy(buff,Start);
	strcat(buff,";CHARSET=UTF-7;ENCODING=QUOTED-PRINTABLE:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+41) == ';' )
			begin = 1;
		else
			begin = 0;
		if(bSkipFirst == false) begin =0;
		DecodeUTF7(Value,Buffer+strlen(Start)+41+begin,strlen((char*)Buffer)-(strlen(Start)+41+begin));
		dbgprintf("ReadVCalText is \"%s\"\n",DecodeUnicodeConsole(Value));
		return true;
	}
	
	strcpy(buff,Start);
	strcat(buff,";CHARSET=UTF-7;HOME:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+20) == ';' )
			begin = 1;
		else
			begin = 0;
		if(bSkipFirst == false) begin =0;
		DecodeUTF7(Value,Buffer+strlen(Start)+20+begin,strlen((char*)Buffer)-(strlen(Start)+20+begin));
		dbgprintf("ReadVCalText is \"%s\"\n",DecodeUnicodeConsole(Value));
		return true;
	}

	//Bobby 10.18.05 For ISO--8859 used in samsung when there an = sign
	strcpy(buff,Start);
	strcat(buff,";CHARSET=ISO-8859-1;ENCODING=QUOTED-PRINTABLE:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+46) == ';' )
			begin = 1;
		else
			begin = 0;
		if(bSkipFirst == false) begin =0;
		CString csCharConversion = Buffer;
		csCharConversion.Replace("=3D","=");
		csCharConversion.Replace("=0D","\r");
		csCharConversion.Replace("=0A","\n");
		sprintf((char *)Buffer,"%s",csCharConversion);
		EncodeUnicode(Value,Buffer+strlen(Start)+46+begin,strlen((char *)Buffer)-(strlen(Start)+46+begin));
		return true;
	}
//for Sagem +
	strcpy(buff,Start);
	strcat(buff,";CHARSET=US-ASCII:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+18) == ';' )
			begin = 1;
		else
			begin = 0;
		if(bSkipFirst == false) begin =0;
		EncodeUnicode(Value,Buffer+strlen(Start)+18+begin,strlen((char *)Buffer)-(strlen(Start)+18+begin));
		return true;
	}

	strcpy(buff,Start);
	strcat(buff,";CHARSET=US-ASCII;ENCODING=QUOTED-PRINTABLE:");
	if (!strncmp((char*)Buffer,buff,strlen(buff))) {
		if( *(Buffer+strlen(Start)+44) == ';' )
			begin = 1;
		else
			begin = 0;
		if(bSkipFirst == false) begin =0;
		CString csCharConversion = Buffer;
		csCharConversion.Replace("=3D","=");
		csCharConversion.Replace("=0D","\r");
		csCharConversion.Replace("=0A","\n");
		sprintf((char *)Buffer,"%s",csCharConversion);
		EncodeUnicode(Value,Buffer+strlen(Start)+44+begin,strlen((char *)Buffer)-(strlen(Start)+44+begin));
		return true;
	}
//for Sagem -

	return false;
}
void WINAPI SaveVCALText(char *Buffer, int *Length,unsigned char *Text, char *Start)
{
	char buffer[1000];
//	char strUTF8[1000];

	if (UnicodeLength(Text) != 0) {
		EncodeUTF8QuotedPrintable((unsigned char*)buffer,Text); //peggy +
	//	EncodeCString2UTF8(Text, strUTF8);//peggy -
	//	EncodeQuotedPrintable(buffer,strUTF8);//peggy -
		if (UnicodeLength(Text)==strlen(buffer)) {
			*Length+=sprintf(Buffer+(*Length), "%s:%s%c%c",Start,DecodeUnicodeString(Text),13,10);
		} else {
			*Length+=sprintf(Buffer+(*Length), "%s;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:%s%c%c",Start,buffer,13,10);
		}
	}	    
}
void WINAPI SiemensSaveVCARDText(char *Buffer, int *Length, char *Text, char *Start,char *beforetext)
{
	char buffer[1000];

	if (UnicodeLength((unsigned char *)Text) != 0)
	{
		unsigned char uText[2000];
		DecodeUnicode((unsigned char *)Text, uText);
		if(strchr((char*)uText,'\r') || strchr((char*)uText,'\n'))
		{
			EncodeUTF8QuotedPrintable((unsigned char*)buffer,(unsigned char *)Text); //peggy +	
			*Length+=sprintf(Buffer+(*Length), "%s;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:%s%s%c%c",Start,beforetext,buffer,13,10);
		}
		else
		{
			//	EncodeCString2UTF8(DecodeUnicodeString((unsigned char *)Text),(unsigned char *)buffer);
			EncodeUnicode2UTF8((unsigned char *)Text,(unsigned char *)buffer);// for _UNICODE
			if (UnicodeLength((unsigned char *)Text)==strlen(buffer))
			{
				*Length+=sprintf(Buffer+(*Length), "%s:%s%s%c%c",Start,beforetext,DecodeUnicodeString((unsigned char *)Text),13,10);
			} else {
				*Length+=sprintf(Buffer+(*Length), "%s;CHARSET=UTF-8:%s%s%c%c",Start,beforetext,buffer,13,10);
			}
		}
	}	    
}
void WINAPI SharpSaveVCARDText(char *Buffer, int *Length, unsigned char *Text, char *Start,char *beforetext, char* endtext)
{
	unsigned char buffer[1000];

	if (UnicodeLength(Text) != 0) 
	{
		unsigned char uText[2000];
		DecodeUnicode(Text, uText);
		if(strchr((char*)uText,'\r') || strchr((char*)uText,'\n'))
		{
			EncodeUTF8QuotedPrintable((unsigned char*)buffer,Text); //peggy +	
			*Length+=sprintf(Buffer+(*Length), "%s;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE%s%s%s%c%c",Start,beforetext,buffer,endtext,13,10);
		}
		else
		{
	//		EncodeCString2UTF8(DecodeUnicodeString(Text),buffer);
			EncodeUnicode2UTF8(Text,buffer);// for _UNICODE
			if (UnicodeLength(Text)==strlen((char*)buffer)) {
				*Length+=sprintf(Buffer+(*Length), "%s%s%s%s%c%c",Start,beforetext,DecodeUnicodeString(Text), endtext,13,10);
			} else {
				*Length+=sprintf(Buffer+(*Length), "%s;CHARSET=UTF-8;ENCODING=QUOTABLE-PRINTABL%s%s%s%c%c",Start,beforetext,buffer,endtext,13,10);
			}
		}
	}
}
int WINAPI ATGEN_ExtractOneParameter(unsigned char *input, unsigned char *output)
{
	int position=0;

	while (*input!=',' && *input!=0x0d && *input!=0x00) 
	{
		*output=*input;
		input	++;
		output	++;
		position++;
	}
	*output=0;
	position++;
	return position;
}

int WINAPI ATGEN_ExtractOneParameterEx(unsigned char *input, unsigned char *output)
{
	int position=0;

	if(*input == '"')
	{
		*output=*input;
		input	++;
		output	++;
		position++;
		while(*input!='"')
		{
			*output=*input;
			input	++;
			output	++;
			position++;
		}
	}

	while (*input!=',' && *input!=0x0d && *input!=0x00) 
	{
		*output=*input;
		input	++;
		output	++;
		position++;
	}
	*output=0;
	position++;
	return position;
}

int WINAPI ATGEN_ExtractOneParameterEx2(unsigned char *input, unsigned char *output)
{
	int position=0;

	if(*input == '"')
	{
		*output=*input;
		input	++;
		output	++;
		position++;
		while(*input!='"')
		{
			*output=*input;
			input	++;
			output	++;
			position++;
		}
	}

	while (*input!='"' && *input!=0x0d && *input!=0x00) 
	{
		*output=*input;
		input	++;
		output	++;
		position++;
	}
	*output=0;
	position++;
	return position;
}

bool WINAPI MMDDYY_Get_DateTime(GSM_DateTime *Date, char * timet)
{
	int num=0 ,timelen=0;
	char 	*pTemp;
	char szTemp[30];
	int nY;
	GSM_DateTime CurrentDate;
	timelen= strlen(timet);

	Date->Year	= 0;
	Date->Month	= 0;
	Date->Day	= 0;
	Date->Hour	= 0;
	Date->Minute= 0;
	Date->Second= 0;

	pTemp = strchr(timet,'-' );
	if(pTemp == NULL) return false;
	sprintf(szTemp,"%s",timet);
	sprintf(timet	,"%s",pTemp+1);
	szTemp[strlen(szTemp)-strlen(pTemp)] = '\0';
	Date->Month	= atoi(szTemp);

	pTemp = strchr (timet,'-' );
	if(pTemp == NULL) return false;
	sprintf(szTemp,"%s",timet);
	sprintf(timet	,"%s",pTemp+1);
	szTemp[strlen(szTemp)-strlen(pTemp)] = '\0';
	Date->Day	= atoi(szTemp);

	nY = atoi(timet);

	if(nY < 1000)
	{
		GSM_GetCurrentDateTime(&CurrentDate);
		Date->Year	=nY-(nY/100)*100 +2000;
		if(Date->Year > CurrentDate.Year)
			Date->Year -=100;
		else if(Date->Year == CurrentDate.Year)
		{
			if(Date->Month > CurrentDate.Month)
				Date->Year -=100;
			else if(Date->Month == CurrentDate.Month)
			{
				if(Date->Day > CurrentDate.Day)
					Date->Year -=100;

			}

		}
	}
	else
		Date->Year =nY;

	return true;
}
bool WINAPI HHMM_Get_DateTime(GSM_DateTime *Date, char * timet)
{
	int num=0 ,timelen=0;
	char 	*pTemp;
	char szTemp[30];
	timelen= strlen(timet);

	Date->Year	= 0;
	Date->Month	= 0;
	Date->Day	= 0;
	Date->Hour	= 0;
	Date->Minute= 0;
	Date->Second= 0;

	pTemp = strchr(timet,':' );
	if(pTemp == NULL) return false;
	sprintf(szTemp,"%s",timet);
	sprintf(timet	,"%s",pTemp+1);
	szTemp[strlen(szTemp)-strlen(pTemp)] = '\0';
	Date->Hour	= atoi(szTemp);


	Date->Minute = atoi(timet);



	return true;
}
int WINAPI GetDurationDifferenceTime(GSM_DateTime *end_DT, GSM_DateTime *start_DT)
{
	time_t start_time;
	time_t end_time;
	double duration=0;
    int    result=0;

	start_time = Fill_Time_T(*start_DT,8);
	end_time   = Fill_Time_T(*end_DT,8);

    duration = difftime(end_time , start_time);// get seconds

	if (duration < 0) 
	{
	  duration 		= 0;
	}
	else
	{
     duration = duration / 60;
	}


	result = (int)duration;

	return result;

}

int WINAPI VCARD_ParseAddress(unsigned char* buffer, unsigned char* postal, unsigned char* street, unsigned char* city, unsigned char* state, unsigned char* zipcode, unsigned char* country)
{
	// ;postal;street;city;state;zipcode;country
	int pos, start=-1, end=-1, len, count=0, index=5;
	unsigned char* temp[6]={0,0,0,0,0,0};
	unsigned char line[2000], unicode[2000];

	DecodeUnicode(buffer, line);
	len=strlen((char*)line);
	temp[0]=postal, temp[1]=street, temp[2]=city, temp[3]=state, temp[4]=zipcode, temp[5]=country;

	pos=len-1;
	end=pos;

	while(pos>=0){
		if(*(line+pos)==';'){
			start=pos;
		}
		if(start!=-1){
			memcpy(temp[index], line+start+1, end-start);
			memset(temp[index]+end-start, 0, 1);
			end=start-1; start=-1;
			index--;
		}
		pos--;
	}
	
	EncodeUnicode(unicode, postal, strlen((char*)postal)+1);
	CopyUnicodeString(postal,unicode);

	EncodeUnicode(unicode, street, strlen((char*)street)+1);
	CopyUnicodeString(street,unicode);

	EncodeUnicode(unicode, city, strlen((char*)city)+1);
	CopyUnicodeString(city,unicode);

	EncodeUnicode(unicode, state, strlen((char*)state)+1);
	CopyUnicodeString(state,unicode);

	EncodeUnicode(unicode, zipcode, strlen((char*)zipcode)+1);
	CopyUnicodeString(zipcode,unicode);

	EncodeUnicode(unicode, country, strlen((char*)country)+1);
	CopyUnicodeString(country,unicode);
	return 6-index;
}

void WINAPI VCARD_AddressCat(GSM_MemoryEntry* pbk, unsigned char* value)
{
	// the address format:
	// ;postal;street;city;state;zipcode;country

	unsigned char postal[MAX_PATH*2]={0}, 
				street[MAX_PATH*2]={0}, 
				city[MAX_PATH*2]={0}, 
				state[MAX_PATH*2]={0}, 
				zipcode[MAX_PATH*2]={0},
				country[MAX_PATH*2]={0},
				Semicolon[10];

	memset(value, 0, 2);
	memset(postal, 0, 2);

	for(int i=0; i<pbk->EntriesNum; i++){
		if(PBK_Text_Postal==pbk->Entries[i].EntryType){
			CopyUnicodeString(postal,pbk->Entries[i].Text);

		}else if(PBK_Text_StreetAddress==pbk->Entries[i].EntryType){
			CopyUnicodeString(street,pbk->Entries[i].Text);

		}else if(PBK_Text_City==pbk->Entries[i].EntryType){
			CopyUnicodeString(city,pbk->Entries[i].Text);

		}else if(PBK_Text_State==pbk->Entries[i].EntryType){
			CopyUnicodeString(state,pbk->Entries[i].Text);

		}else if(PBK_Text_Zip==pbk->Entries[i].EntryType){
			CopyUnicodeString(zipcode,pbk->Entries[i].Text);

		}else if(PBK_Text_Country==pbk->Entries[i].EntryType){
			CopyUnicodeString(country,pbk->Entries[i].Text);
		}
	}

	EncodeUnicode(Semicolon, (unsigned char*)";", strlen(";"));
	UnicodeCat(value, Semicolon);
	UnicodeCat(value, postal);

	UnicodeCat(value, Semicolon);
	UnicodeCat(value, street);

	UnicodeCat(value, Semicolon);
	UnicodeCat(value, city);

	UnicodeCat(value, Semicolon);
	UnicodeCat(value, state);

	UnicodeCat(value, Semicolon);
	UnicodeCat(value, zipcode);

	UnicodeCat(value, Semicolon);
	UnicodeCat(value, country);
}

void WINAPI GSM_DecodeSharpDevInfo(unsigned char* Buffer, char* pszManufacturer, char* pszModel, char* pszIMEI)
{
	unsigned char   Line[1000];//,Buff[1000];

	int pos=0, i=0;

	while (1) {
		MyGetLine(Buffer, &pos, Line, strlen((char *)Buffer));
		if (strlen((char *)Line) == 0) break;
		if(pszManufacturer){
			if(strstr((char*)Line, "MANU:")){
				if(strncmp((char*)Line, "MANU:", 5)==0)
					strcpy(pszManufacturer, (char*)Line+5);
			}
		}
		if(pszModel){
			if(strstr((char*)Line, "MOD:")){
				if(strncmp((char*)Line, "MOD:", 4)==0)
					strcpy(pszModel, (char*)Line+4);
			}
		}
		if(pszIMEI){
			if(strstr((char*)Line, "SN:")){
				if(strncmp((char*)Line, "SN:", 3)==0)
					strcpy(pszIMEI, (char*)Line+3);
			}
		}
	}// while
}
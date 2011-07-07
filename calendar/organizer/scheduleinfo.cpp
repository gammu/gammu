// ScheduleInfo.cpp: implementation of the CScheduleInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScheduleInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CString ParseOleDateTime(COleDateTimeEx DateTimeEx);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CString CScheduleInfo::GuidToString(GUID guid)
{
	CString string;
	string.Format(_T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"),
				guid.Data1,
				guid.Data2,
				guid.Data3,
				guid.Data4[0],
				guid.Data4[1],
				guid.Data4[2],
				guid.Data4[3],
				guid.Data4[4],
				guid.Data4[5],
				guid.Data4[6],
				guid.Data4[7]);
	return string;
}

void CScheduleInfo::StringToGuid(CString szStr, GUID& inputGUID)
{
	GUID guid;
	TCHAR buf[MAX_PATH];
	lstrcpy(buf,szStr.GetBuffer(MAX_PATH));
	szStr.ReleaseBuffer();
	_stscanf(buf,_T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"),
				&guid.Data1,
				&guid.Data2,
				&guid.Data3,
				&guid.Data4[0],
				&guid.Data4[1],
				&guid.Data4[2],
				&guid.Data4[3],
				&guid.Data4[4],
				&guid.Data4[5],
				&guid.Data4[6],
				&guid.Data4[7]);

	memcpy(&inputGUID, &guid, sizeof(GUID));
	return;

}

CScheduleInfo::CScheduleInfo()
{
	OrgId = IID_NONE;
	csReasonForReply=_T("");
	dtStartDate = COleDateTime::GetCurrentTime();

	//kerm add 2.15
 	SYSTEMTIME st;
    GetSystemTime(&st);
 	dtStartDate.SetDate(st.wYear, st.wMonth, st.wDay);

	hiRealStartTime.SetData(8,0,0);
	hiStartTime = hiRealStartTime;
	hiStopTime = hiStartTime;
	hiStopTime.MoveToNextHalfHour();
	hiRealStopTime = hiStopTime;

	rowIndex = 0;
	n30MinBlockCnt = 0;
	bScheduleStart = FALSE;
	csTitle = _T("");
	csMemo = _T("");
	haveRepeatUntil = FALSE;  //m_bRepeatUntil
	utilWhichDate.SetDate(dtStartDate.GetYear(),dtStartDate.GetMonth(),dtStartDate.GetDay());
	telnumber = _T(""); //telephone number to be reminded on : m_AlarmPhoneNo
	haveAlarm = FALSE; //true or false  : m_bAlarm
//	alarmLeadtime = 1; //Similar to Reminder option of outlook. must be in minutes. : m_AlarmLeadtime

	nRepeatId = -1;
	originalData = TRUE;
	haveRepeat = FALSE; //true or false : m_bRepeat
	repeatType = DAILY; //Daily , weekly, Monthly, MonthlyWeekly and Yearly : m_RepeatType
	repeatFrequency = 1; // every 1 day/week/month/year : m_RepeatFrequency
	rfWhichWeek = FIRST; //First , Second, Third, Fourth or Last week : m_RepeatWhichWeek
	rfWhichDay = SUNDAY; //Sunday, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday : m_WeekDay
	nDayofWeekMask = 0;
	//Calendar
//	uiIndex = 0;
	itoa(0,szuiIndex,10);
	cuLocation = _T(""); //Location of the shcedule
	uiNodeType = 0;//NodeType of the shcedule
	uiPrivate = 2;//int uiPrivate			//use for 9@9u priority   kerm 07.12.12 初始化为2 默认为 中
	bhaveTone = false;
    dtAlarmDateTime = COleDateTime::GetCurrentTime();
	//kerm test 2.19 
	dtStopDate = COleDateTime::GetCurrentTime();
	//dtStopDate = dtStartDate;
	bEndDateSameToStartDate = TRUE;
}

CScheduleInfo::~CScheduleInfo()
{

}

CScheduleInfo::CScheduleInfo(CScheduleInfo &si) {
	SetScheduleInfoData(si);
}
void CScheduleInfo::SetScheduleInfoData(const CScheduleInfo &si) {

	
	OrgId = si.OrgId;
	csReasonForReply=si.csReasonForReply;
	dtStartDate = si.dtStartDate;
	rowIndex = si.rowIndex;
	n30MinBlockCnt = si.n30MinBlockCnt;
	bScheduleStart = si.bScheduleStart;

	hiStartTime = si.hiStartTime;
	hiRealStartTime = si.hiRealStartTime;
	hiStopTime = si.hiStopTime;
	hiRealStopTime = si.hiRealStopTime;

//	Schedule_Time_Info hiStopTime;
	
	csTitle = si.csTitle;

	csMemo = si.csMemo;
	haveRepeatUntil = si.haveRepeatUntil;  //m_bRepeatUntil
	utilWhichDate.SetDate(si.utilWhichDate._yy,si.utilWhichDate._mm,si.utilWhichDate._dd);    //m_UntilWhichDate
	telnumber = si.telnumber; //telephone number to be reminded on : m_AlarmPhoneNo
	haveAlarm = si.haveAlarm; //true or false  : m_bAlarm
//	alarmLeadtime = si.alarmLeadtime; //Similar to Reminder option of outlook. must be in minutes. : m_AlarmLeadtime

	dtAlarmDateTime = si.dtAlarmDateTime;
	bEndDateSameToStartDate = si.bEndDateSameToStartDate;

	nRepeatId = si.nRepeatId;
	originalData = si.originalData;
	haveRepeat = si.haveRepeat; //true or false : m_bRepeat
	repeatType = si.repeatType; //Daily , weekly, Monthly, MonthlyWeekly and Yearly : m_RepeatType
	repeatFrequency = si.repeatFrequency; // every 1 day/week/month/year : m_RepeatFrequency
	rfWhichWeek = si.rfWhichWeek; //First , Second, Third, Fourth or Last week : m_RepeatWhichWeek
	rfWhichDay = si.rfWhichDay; //Sunday, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday : m_WeekDay
	nDayofWeekMask = si.nDayofWeekMask;
	//Calendar
//	uiIndex = si.uiIndex;
	sprintf(szuiIndex,si.szuiIndex);
	cuLocation = si.cuLocation; //Location of the shcedule
	uiNodeType = si.uiNodeType;//NodeType of the shcedule
	uiPrivate = si.uiPrivate;//int uiPrivate
	bhaveTone = si.bhaveTone;

	dtStopDate = si.dtStopDate;

/*	//kerm change for 9a9u 08.1.02
	if(repeatType == WEEKDAYS)
	{
    	repeatType = WEEKLY;
		nDayofWeekMask= 62;
	}
	else if(repeatType == WEEKEND)
	{
		repeatType = WEEKLY;
		nDayofWeekMask= 65;
	}*/

}
/*
CString CScheduleInfo::SetAlarmLeadDatetime()
{
	//Calendar
	//void SetDateTimeSpan( long lDays, int nHours, int nMins, int nSecs );
  	int nHour=0, nMin=0;
	hiStartTime.Get24HourTime(nHour, nMin);

	COleDateTime dtStartDate24Hur;
	dtStartDate24Hur.SetDateTime(dtStartDate.GetYear(), dtStartDate.GetMonth(), dtStartDate.GetDay(),nHour,nMin,0);

	dtAlarmLeadTimeSpan.SetDateTimeSpan(0,0,_ttoi(getAlarmLeadtime()),0);
	dtAlarmDateTime = dtStartDate24Hur - dtAlarmLeadTimeSpan;

//	cuAlarmLeadDatetime = ParseOleDateTime(dtAlarmDateTime);

	return ParseOleDateTime(dtAlarmDateTime);
}*/
/*
CString CScheduleInfo::getAlarmLeadtime() {
	if (0 == alarmLeadtime)  {
		return _T("");
	}else 
	{
		alarmLeadtime - 1;
	}

	CString leadTime[]={"0","5","10","15","30","60","120","180","240","300","360","420","480","540","600","660","720","1440","2880"};

	return leadTime[alarmLeadtime-1];
}

CString CScheduleInfo::getAlarmLeadtime(int nIndexAlarmLeadtime) {
	if (0 == nIndexAlarmLeadtime)  {
		return _T("");
	}else 
	{
		nIndexAlarmLeadtime--;
	}

	CString leadTime[]={"0","5","10","15","30","60","120","180","240","300","360","420","480","540","600","660","720","1440","2880"};

	return leadTime[nIndexAlarmLeadtime];
}
*/
/*
void CScheduleInfo::SetAlarmLeadtime(CString leadtime) {
	CString leadTime[]={"0","5","10","15","30","60","120","180","240","300","360","420","480","540","600","660","720","1440","2880"};
	if (leadtime.IsEmpty()) {
		alarmLeadtime = 1;
		return;
	}
	for (int i=0;i<19;i++) {
		if (leadTime[i] == leadtime) {
			//Test
            CString csText = (LPCTSTR)leadtime;

			alarmLeadtime = i+1;
			return;
		}
	}

	//alarmLeadtime = 1;
	alarmLeadtime = 0;
}*/
CString CScheduleInfo::GetStartTimeString() {
	return hiStartTime.GetTimeString();
}
CString CScheduleInfo::GetStartDateString()
{
	CString str;

	if(dtStartDate.GetStatus() == COleDateTime ::valid)
		str.Format(_T("%02d/%02d/%04d"),dtStartDate.GetMonth(),dtStartDate.GetDay(),dtStartDate.GetYear());
	else
		str.Format(_T("%02d/%02d/%02d"),0,0,0);
	return str;
}
CString CScheduleInfo::GetStopDateString()
{
	CString str;
	if(dtStopDate.GetStatus() == COleDateTime ::valid)
		str.Format(_T("%02d/%02d/%04d"),dtStopDate.GetMonth(),dtStopDate.GetDay(),dtStopDate.GetYear());
	else
		str.Format(_T("%02d/%02d/%02d"),0,0,0);
	return str;
}

CString CScheduleInfo::GetStopTimeString() {
	return hiStopTime.GetTimeString();
}

CString CScheduleInfo::GetRealStartTimeString() {
	return hiRealStartTime.GetTimeString();
}

CString CScheduleInfo::GetRealStopTimeString() {
	return hiRealStopTime.GetTimeString();
}

CString CScheduleInfo::toString(){
	CString strRet;
	strRet = GuidToString(OrgId);

	strRet += dtStartDate.Format(VAR_DATEVALUEONLY);
	strRet += hiRealStartTime.toString();
	strRet += hiRealStopTime.toString();
	CString strTemp;
	strTemp.Format(_T("%d"),n30MinBlockCnt);
	strRet += strTemp;
	strRet += csTitle;
	strRet += csMemo;
    if (haveAlarm) {
		strRet += _T("haveAlarm");
//		strRet += getAlarmLeadtime();
		strRet += telnumber;
	}
	if (haveRepeat) {
		strRet += _T("haveRepeat");
		strTemp.Format(_T("%d"),repeatType);
		strRet += strTemp;
		strTemp.Format(_T("%d"),repeatFrequency);
		strRet += strTemp;
		strTemp.Format(_T("%d"),rfWhichWeek);
		strRet += strTemp;
		strTemp.Format(_T("%d"),rfWhichDay);
		strRet += strTemp;
		strTemp.Format(_T("%d"),nDayofWeekMask);
		strRet += strTemp;
	}
    if (haveRepeatUntil) {
		strRet += _T("haveRepeatUntil");
		strTemp.Format(_T("%d"),utilWhichDate._yy);
		strRet += strTemp;
		strTemp.Format(_T("%d"),utilWhichDate._mm);
		strRet += strTemp;
		strTemp.Format(_T("%d"),utilWhichDate._dd);
		strRet += strTemp;
	}

	if (originalData) {
		strRet += _T("originalData");
	}
	return strRet;
}
/*
BOOL CScheduleInfo::ParseString(CString inStr) {
	int pos1=0,pos2=0;
	if ((pos2 = inStr.Find(delimitor,pos1)) == -1) {
		return FALSE;
	}
	hiStartTime.ParseString(inStr.Mid(pos1,pos2-pos1));
	pos1 = pos2 + sizeof(delimitor);

	if ((pos2 = inStr.Find(delimitor,pos1)) == -1) {
		return FALSE;
	}
	n30MinBlockCnt = _ttoi(inStr.Mid(pos1,pos2-pos1));
	//		hiStopTime.ParseString(inStr.Mid(pos1,pos2-pos1));
	pos1 = pos2 + sizeof(delimitor);

	if ((pos2 = inStr.Find(delimitor,pos1)) == -1) {
		return FALSE;
	}
	csTitle =  inStr.Mid(pos1,pos2-pos1);
	pos1 = pos2 + sizeof(delimitor);

	if ((pos2 = inStr.Find(delimitor,pos1)) == -1) {
		return FALSE;
	}
	csMemo =  inStr.Mid(pos1,pos2-pos1);
	pos1 = pos2 + sizeof(delimitor);

	return TRUE;
}*/
void CScheduleInfo::Reset() {
	OrgId = IID_NONE;
	csReasonForReply=_T("");
	n30MinBlockCnt = 1;
	hiRealStartTime = hiStartTime;
	hiStopTime = hiStartTime;
	hiStopTime.MoveToNextHalfHour();
	hiRealStopTime = hiStopTime;
	csTitle = _T("");
	csMemo = _T("");
	bScheduleStart = FALSE;

	haveRepeatUntil = FALSE;  //m_bRepeatUntil
	//utilWhichDate = _T("");    //m_UntilWhichDate
	telnumber = _T(""); //telephone number to be reminded on : m_AlarmPhoneNo
	haveAlarm = FALSE; //true or false  : m_bAlarm
//	alarmLeadtime = 1; //Similar to Reminder option of outlook. must be in minutes. : m_AlarmLeadtime

	originalData = TRUE;
	haveRepeat = FALSE; //true or false : m_bRepeat
	repeatType = 0; //Daily , weekly, Monthly, MonthlyWeekly and Yearly : m_RepeatType
	repeatFrequency = 1; // every 1 day/week/month/year : m_RepeatFrequency
	rfWhichWeek = 0; //First , Second, Third, Fourth or Last week : m_RepeatWhichWeek
	rfWhichDay = 0; //Sunday, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday : m_WeekDay
	nDayofWeekMask = 0;
	//Calendar 
//	uiIndex = 0;
	itoa(0,szuiIndex,10);
	cuLocation = _T(""); //Location of the shcedule
	uiNodeType = 0;//NodeType of the shcedule
	uiPrivate = 0;//int uiPrivate
	bhaveTone = false;
	bEndDateSameToStartDate = TRUE;
}
int CScheduleInfo::GetStopRow() {
	return rowIndex + n30MinBlockCnt;
}

void CScheduleInfo::SetData(SchedulesRecord &sr) {
	int year=0,month=0,day=0;
	int hour=0,min=0,sec=0;
	int year2=0,month2=0,day2=0;
	int hour2=0,min2=0,sec2=0;
	int year3=0,month3=0,day3=0;
	int hour3=0,min3=0,sec3=0;

	OrgId = sr.GetPCID();
	csReasonForReply = sr.reasonForReply;

	CString starttime = sr.starttime;
	CString endtime = sr.endtime;
	ParseDateTimeString(starttime,year,month,day,hour,min,sec);
	ParseDateTimeString(endtime,year2,month2,day2,hour2,min2,sec2);

	dtStartDate.SetDateTime(year,month,day,hour,min,sec);
	//
	if(year2 == 0)
		dtStopDate.SetDateTime(year,month,day,hour,min,sec);
	else
		dtStopDate.SetDateTime(year2,month2,day2,hour2,min2,sec2);

	csTitle = sr.title;
	csMemo = sr.memo;
	hiRealStartTime.SetData(hour,min,sec);
	hiStartTime = hiRealStartTime;
	hiStartTime.MoveToNearestStartTime();
	hiRealStopTime.SetData(hour2,min2,sec2);
	hiStopTime = hiRealStopTime;
	hiStopTime.MoveToNearestStopTime();

	dtAlarmDateTime = sr.dtAlarmTime;


	//	n30MinBlockCnt = (hour2*2 + min2/30) - (hour*2 + min/30);
	//	if ((hour2 == 23) && (min2 == 59)) {
	//		n30MinBlockCnt = n30MinBlockCnt + 1;
	//	}
	n30MinBlockCnt = hiStopTime.GetHalfHourCount() - hiStartTime.GetHalfHourCount();

	bScheduleStart = TRUE;
	rowIndex = (hour*2 + min/30);

	//Get AlarmLeadTime
//	char szBuffer[10];
	dtAlarmLeadTimeSpan = dtStartDate - dtAlarmDateTime;
//	if(dtAlarmLeadTimeSpan.GetStatus() == COleDateTimeSpan::valid)
//	{
//		CString cuTotalMinute_alarmLeadtime = itoa(dtAlarmLeadTimeSpan.GetTotalMinutes(), szBuffer, 10);
//		SetAlarmLeadtime(cuTotalMinute_alarmLeadtime);
//	}
//	else 
//		SetAlarmLeadtime("0");

//	SetAlarmLeadDatetime();
	//SetAlarmLeadtime(sr.alarmLeadtime);
	haveAlarm = sr.haveAlarm;

	originalData = TRUE;
	haveRepeat = sr.haveRepeat;
	repeatFrequency = max(sr.repeatFrequency,1);
	repeatType = sr.repeatType;
	rfWhichDay = sr.rfWhichDay;
	nDayofWeekMask = sr.nDayofWeekMask;
	rfWhichWeek = sr.rfWhichWeek;
	telnumber = sr.telnumber;
	haveRepeatUntil = sr.haveUntil;
	utilWhichDate.SetDate(&sr.untilDate);

	//Calendar
//	uiIndex = sr.uiIndex;
	sprintf(szuiIndex,sr.szuiIndex);
	cuLocation = sr.cuLocation; //Location of the shcedule
	uiNodeType = sr.uiNodeType;//NodeType of the shcedule
	uiPrivate = sr.uiPrivate;//int uiPrivate
	bhaveTone = sr.bhaveTone;
	bEndDateSameToStartDate= sr.bEndDateSameToStartDate;

	// kerm change for 9a9u 1.05
	//为什么去掉之后出错????
/**/	if(repeatType == WEEKDAYS)
	{
		repeatType = WEEKLY;
		//nDayofWeekMask= 62;	//kerm change for 9a9u 1.16
	}
	else if(repeatType == WEEKEND)
	{
		repeatType = WEEKLY;
		nDayofWeekMask= 65;
	}
}

void CScheduleInfo::ParseDateTimeString(CString &cs,int &year,int &month,int &day,
		                   int &hour,int &min,int &sec) {
	if (cs.GetLength() < 16) {
		return;
	}
	ParseDateString(cs,year,month,day);
	CString ts = cs.Right(5);
	ParseTimeString(ts,hour,min,sec);
}

void CScheduleInfo::ParseDateString(CString &cs,int &year,int &month,int &day) {
	if (cs.GetLength() < 10) {
		return;
	}
	month = _ttoi(cs.Left(2));
	day = _ttoi(cs.Mid(3,2));
	year = _ttoi(cs.Mid(6,4));
}

BOOL CScheduleInfo::ParseTimeString(CString &inStr,int &hour,int &min,int &sec) {
	if (inStr.GetLength() != 5) {
		return FALSE;
	}
	hour = _ttoi(inStr.Mid(0,2));
	min = _ttoi(inStr.Mid(3,2));
//		sec = _ttoi(inStr.Mid(6,2));
	return TRUE;
}

BOOL CScheduleInfo::IsStartDateFirstDateToDisplay() {
	if (repeatType != WEEKLYMONTHLY) {
		return TRUE;
	}

	int dayOfWeek = dtStartDate.GetDayOfWeek();
	if ((dayOfWeek-1) == rfWhichDay) {
		int weekOfMonth = dtStartDate.GetWeekOfMonthByWeekDay();
		if ((weekOfMonth) == rfWhichWeek) {
			return TRUE;
		}
	}
	return FALSE;
}

int CScheduleInfo::GetFirstExtendDate(COleDateTimeEx &dt,COleDateTimeEx &dtExtendStopDate,int year,int month,int day,COleDateTime StartEndDateSpin) 
{
	int nRet = 1;
//	COleDateTimeEx dtExtendStopDate;
	dt = dtStartDate;
	if(dtStopDate.GetStatus() == COleDateTime::invalid)
		dtExtendStopDate = dtStartDate;
	else
		dtExtendStopDate = dtStopDate;
	if (IsStartDateFirstDateToDisplay())
	{
		while (dt.GetYear() < year && !IsDateExcessEndDate(dt)) 
		{
			nRet = GetNextExtendDate(dt,dtExtendStopDate,day,StartEndDateSpin);
		}
		while ((dt.GetYear() == year) && (dt.GetMonth() < month)&& !IsDateExcessEndDate(dt)) 
		{
			nRet = GetNextExtendDate(dt,dtExtendStopDate,day,StartEndDateSpin);
		}
		return nRet;
	}

	dt.MoveToFirstMonthlyWeeklyDate(rfWhichWeek,rfWhichDay,day);
	if (dt.GetStatus() == COleDateTime::invalid) {
//		AfxMessageBox("invalid");
	}

		while (dt.GetYear() < year && !IsDateExcessEndDate(dt)) 
		{
			nRet = GetNextExtendDate(dt,dtExtendStopDate,day,StartEndDateSpin);
		}
		while ((dt.GetYear() == year) && (dt.GetMonth() < month)&& !IsDateExcessEndDate(dt)) 
		{
			nRet = GetNextExtendDate(dt,dtExtendStopDate,day,StartEndDateSpin);
		}
		return nRet;
}

int CScheduleInfo::GetNextExtendDate(COleDateTimeEx &dt,COleDateTimeEx &dtStop,int day,COleDateTime StartEndDateSpin)
{
	COleDateTimeSpan dts;
	COleDateTimeEx dtNextExtendStopDate = dtStop;
	COleDateTimeEx dt2,dt3,dttemp;
	dttemp.SetDateTime(dt.GetYear(),dt.GetMonth(),dt.GetDay(),0,0,0);
	int i=0;
	if(bEndDateSameToStartDate == FALSE)
	{
		dttemp.MoveToNextDayDate(1);
		if(dtStop >= dttemp)
		{
			dt = dttemp;
			return 0;
		}
		switch (repeatType) 
		{
		case DAILY:
		//	dt.MoveToNextDayDate(repeatFrequency);
		//	dtStop.MoveToNextDayDate(repeatFrequency);
			dt.MoveToNextDayDate(repeatFrequency);
			if(StartEndDateSpin.GetStatus() == COleDateTime::valid)
			{
				dtNextExtendStopDate = dt;
				dt = dt - StartEndDateSpin;
				if(dtStop >dt)
				{
					dt = dtStop;
					dt.MoveToNextDayDate(1);
					dtStop =(COleDateTime) dt +StartEndDateSpin;
				}
				else
					dtStop =(COleDateTime) dt +StartEndDateSpin;
			}
			return 0;
			break;
		case WEEKLY:
			//dt.MoveToNextWeekDate(repeatFrequency);
			//kerm change for 9a9u 1.07
			dt.MoveToNextWeekDate(1);
			if(StartEndDateSpin.GetStatus() == COleDateTime::valid)
			{
				dtNextExtendStopDate = dt;
				int nY = dtNextExtendStopDate.GetYear();
				int nM = dtNextExtendStopDate.GetMonth();
				int nD = dtNextExtendStopDate.GetDay();
				dt = dt - StartEndDateSpin;
				if(dtStop >=dt)
				{
					dt = dtStop;
					dt.MoveToNextDayDate(1);
	
					dtStop = dt ;
					return 0;
				}
			}
			break;
		case TWOWEEKLY://Added by nono, 2004_1119
			repeatFrequency =2;
			dt.MoveToNextWeekDate(repeatFrequency);
			if(StartEndDateSpin.GetStatus() == COleDateTime::valid)
			{
				dtNextExtendStopDate = dt;
				dt = dt - StartEndDateSpin;
				if(dtStop >=dt)
				{
					dt = dtStop;
					dt.MoveToNextDayDate(1);
	
					dtStop = dt ;
					return 0;
				}
			}
			break;
		case MONTHLY:
			dt.MoveToNextMonthDate(repeatFrequency,day);
			if(StartEndDateSpin.GetStatus() == COleDateTime::valid)
			{
				dtNextExtendStopDate.m_dt = dt.m_dt + StartEndDateSpin.m_dt;
				if(dtStop >=dt)
				{
			//		dt = dtStop;
					dt.MoveToNextDayDate(1);
	
					dtStop = dt;
					return 0;
				}
			}
			break;
		case YEARLY:
			dt.MoveToNextYearDate(repeatFrequency,day);
			if(StartEndDateSpin.GetStatus() == COleDateTime::valid)
			{
				dtNextExtendStopDate.m_dt = dt.m_dt + StartEndDateSpin.m_dt;
				if(dtStop >=dt)
				{
			//		dt = dtStop;
					dt.MoveToNextDayDate(1);
	
					dtStop = dt;
					return 0;
				}
			}
			break;
		case WEEKLYMONTHLY:
			dt.MoveToNextMonthlyWeeklyDate(repeatFrequency,rfWhichWeek,rfWhichDay,day);
			if(StartEndDateSpin.GetStatus() == COleDateTime::valid)
			{
				dtNextExtendStopDate.m_dt = dt.m_dt + StartEndDateSpin.m_dt;
				if(dtStop >=dt)
				{
			//		dt = dtStop;
					dt.MoveToNextDayDate(1);
	
					dtStop = dt;
					return 0;
				}
			}
			break;
		default:
			dt = dtStopDate ;
			dt.MoveToNextDayDate(1);
			break;
		}
		COleDateTime dtTEMP ;
		dtTEMP.m_dt = dt.m_dt;

		dtStop = dtTEMP + StartEndDateSpin ;
		return 1;
	}
	else
	{
		switch (repeatType) 
		{
		case DAILY:
			dt.MoveToNextDayDate(repeatFrequency);
			break;
		case WEEKLY:
			//dt.MoveToNextWeekDate(repeatFrequency);
			//kerm change for 9a9u 1.07
			dt.MoveToNextWeekDate(1);
			break;
		case TWOWEEKLY://Added by nono, 2004_1119
			if (2!=repeatFrequency) {
			}
			dt.MoveToNextWeekDate(repeatFrequency);
			break;
		case MONTHLY:
			dt.MoveToNextMonthDate(repeatFrequency,day);
			break;
		case YEARLY:
			dt.MoveToNextYearDate(repeatFrequency,day);
			break;
		case WEEKLYMONTHLY:
			dt.MoveToNextMonthlyWeeklyDate(repeatFrequency,rfWhichWeek,rfWhichDay,day);
			break;
		default:
			dt = dtStopDate ;
			dt.MoveToNextDayDate(1);
			break;
		}
	}
	return 1 ;
}
const CScheduleInfo& CScheduleInfo::operator =(CScheduleInfo &si)
{
	SetScheduleInfoData(si);

	return *this;
}




BOOL operator ==(CScheduleInfo const &info1,CScheduleInfo const &info2)
{
	CScheduleInfo i1;
	i1.SetScheduleInfoData(info1);
	
	CScheduleInfo i2;
	i2.SetScheduleInfoData(info2);

	CString s1 = i1.toString();
	CString s2 = i2.toString();

	if (s1 != s2) {
		return FALSE;
	}
	return TRUE;
}

CString CScheduleInfo::GetClipboardString() {
	CString cpStr;
	CString sep = Member_Seperator;
	CString strTmp;
	cpStr = sep;

	cpStr += dtStartDate.Format(VAR_DATEVALUEONLY);
	cpStr += sep;

	strTmp.Format(_T("%d"),rowIndex);
	cpStr += strTmp;
	cpStr += sep;

	strTmp.Format(_T("%d"),n30MinBlockCnt);
	cpStr += strTmp;
	cpStr += sep;

	strTmp.Format(_T("%d"),(bScheduleStart?1:0));
	cpStr += strTmp;
	cpStr += sep;

	cpStr += hiStartTime.GetTimeString();
	cpStr += sep;

	cpStr += hiRealStartTime.GetTimeString();
	cpStr += sep;
	
	cpStr += hiStopTime.GetTimeString();
	cpStr += sep;

	cpStr += hiRealStopTime.GetTimeString();
	cpStr += sep;

	cpStr += csTitle;
	cpStr += sep;

	cpStr += csMemo;
	cpStr += sep;

	strTmp.Format(_T("%d"),(haveRepeatUntil?1:0));
	cpStr += strTmp;
	cpStr += sep;

	if (utilWhichDate._yy == 0) {
		utilWhichDate.SetDate(dtStartDate.GetYear(),dtStartDate.GetMonth(),dtStartDate.GetDay());
	}
	strTmp.Format(_T("%d"),utilWhichDate._yy);
	cpStr += strTmp;
	strTmp.Format(_T("%d"),utilWhichDate._mm);
	if (strTmp.GetLength() == 1) {
		strTmp = _T("0") + strTmp;
	}
	cpStr += strTmp;
	strTmp.Format(_T("%d"),utilWhichDate._dd);
	if (strTmp.GetLength() == 1) {
		strTmp = _T("0") + strTmp;
	}
	cpStr += strTmp;
	cpStr += sep;

	cpStr += telnumber;
	cpStr += sep;

	strTmp.Format(_T("%d"),(haveAlarm?1:0));
	cpStr += strTmp;
	cpStr += sep;

//	strTmp.Format("%d",alarmLeadtime);
	cpStr += strTmp;
	cpStr += sep;

	strTmp.Format(_T("%d"),nRepeatId);
	cpStr += strTmp;
	cpStr += sep;

	strTmp.Format(_T("%d"),(originalData?1:0));
	cpStr += strTmp;
	cpStr += sep;

	strTmp.Format(_T("%d"),(haveRepeat?1:0));
	cpStr += strTmp;
	cpStr += sep;

	strTmp.Format(_T("%d"),repeatType);
	cpStr += strTmp;
	cpStr += sep;

	strTmp.Format(_T("%d"),repeatFrequency);
	cpStr += strTmp;
	cpStr += sep;

	strTmp.Format(_T("%d"),rfWhichWeek);
	cpStr += strTmp;
	cpStr += sep;

	strTmp.Format(_T("%d"),rfWhichDay);
	cpStr += strTmp;
	cpStr += sep;

	//strTmp.Format(_T("%d"),uiIndex);
	strTmp.Format(_T("%s"),szuiIndex);
	cpStr += strTmp;
	cpStr += sep;

	cpStr += cuLocation;
	cpStr += sep;

	strTmp.Format(_T("%d"),uiNodeType);
	cpStr += strTmp;
	cpStr += sep;

	strTmp.Format(_T("%d"),uiPrivate);
	cpStr += strTmp;
	cpStr += sep;


	strTmp.Format(_T("%d"),bhaveTone);
	cpStr += strTmp;
	cpStr += sep;
	

	return cpStr;
}

void CScheduleInfo::SetScheduleValue(int idx,CString str) {
	int bVal=0;
	int h,m,s;
	switch (idx) {
	case 0:
		dtStartDate.ParseDateTime(str,VAR_DATEVALUEONLY);
		break;
	case 1:
		rowIndex = _ttoi(str);
		break;
	case 2:
		n30MinBlockCnt = _ttoi(str);
		break;
	case 3:
		bVal = _ttoi(str);
		bScheduleStart = bVal == 1;
		break;
	case 4:
		hiStartTime.ParseTimeString(str,h,m,s);
		hiStartTime.SetData(h,m,s);
		break;
	case 5:
		hiRealStartTime.ParseTimeString(str,h,m,s);
		hiRealStartTime.SetData(h,m,s);
		break;
	case 6:
		hiStopTime.ParseTimeString(str,h,m,s);
		hiStopTime.SetData(h,m,s);
		break;
	case 7:
		hiRealStopTime.ParseTimeString(str,h,m,s);
		hiRealStopTime.SetData(h,m,s);
		break;
	case 8:
		csTitle = str;
		break;
	case 9:
		csMemo = str;
		break;
	case 10:
		bVal = _ttoi(str);
		haveRepeatUntil = (bVal == 1);
		break;
	case 11:
		if (str.IsEmpty()) {
			utilWhichDate.SetDate(dtStartDate.GetYear(),dtStartDate.GetMonth(),dtStartDate.GetDay());
		} else {
			utilWhichDate.SetDate(_ttoi(str.Mid(0,4)),_ttoi(str.Mid(4,2)),_ttoi(str.Mid(6,2)));
		}
		break;
	case 12:
		telnumber = str;
		break;
	case 13:
		bVal = _ttoi(str);
		haveAlarm = (bVal == 1);
		break;
	case 14:
	//	alarmLeadtime = _ttoi(str);
		break;
	case 15:
		nRepeatId = _ttoi(str);
		break;
	case 16:
		bVal = _ttoi(str);
		originalData = (bVal == 1);
		break;
	case 17:
		bVal = _ttoi(str);
		haveRepeat = (bVal == 1);
		break;
	case 18:
		repeatType = _ttoi(str);
		break;
	case 19:
		repeatFrequency = _ttoi(str);
		break;
	case 20:
		rfWhichWeek = _ttoi(str);
		break;
	case 21:
		rfWhichDay = _ttoi(str);
		break;
	case 22://Calendar
	//	uiIndex = _ttoi(str);
#ifdef _UNICODE
		{
		USES_CONVERSION;
		WCHAR *wstr = str.GetBuffer(MAX_PATH);
		str.ReleaseBuffer();
		char* sz = W2A(wstr);
		sprintf(szuiIndex,sz);
		}
#else
		wsprintf(szuiIndex,str);
#endif
		break;
	case 23://Calendar
		cuLocation = str;
		break;
	case 24://Calendar
		uiNodeType = _ttoi(str);
		break;
	case 25://Calendar
		uiPrivate = _ttoi(str);
		break;
	case 26://Calendar
		bhaveTone = _ttoi(str);
		break;
	}
}
BOOL CScheduleInfo::ParseClipboardString(CString cpStr) {
	CString sep = Member_Seperator;
	int pos=0,pos2=0;
	int idx=0;
	pos2 = cpStr.Find(sep,pos);
	if (pos2 < 0) {
		return FALSE;
	}
	pos2 += sep.GetLength();
	while (pos2 > 0) {
		pos = pos2;
		pos2 = cpStr.Find(sep,pos);
		SetScheduleValue(idx,cpStr.Mid(pos,pos2-pos));
		pos2 += sep.GetLength();
		if (pos2 >= cpStr.GetLength()) {
			break;
		}
		idx++;
		if (idx > 26) {//Calendar
//		if (idx > 21) {
			break;
		}
	}
	if (idx == 26) {//Calendar
//	if (idx == 21) {
		return TRUE;
	}
	return FALSE;
}

void CScheduleInfo::MergeAndValidateSchedule(CScheduleInfo si) {

	if ((rowIndex == si.rowIndex) && (n30MinBlockCnt == si.n30MinBlockCnt)) {
		si.dtStartDate = dtStartDate;
		si.nRepeatId = -1;
		SetScheduleInfoData(si);
	} else {
		si.dtStartDate = dtStartDate;
		si.hiRealStartTime = hiRealStartTime;
		si.hiStartTime = hiStartTime;
		si.rowIndex = rowIndex;

		if ((si.n30MinBlockCnt + si.rowIndex) > 48) {
			si.n30MinBlockCnt = 48 - si.rowIndex;
		}
		si.hiRealStopTime.SetTimeInfoByHourCount(si.n30MinBlockCnt+si.rowIndex);
		si.hiStopTime = si.hiRealStopTime;
			//		if (haveRepeat) {
			//			if (haveRepeatUntil) {
			//				COleDateTimeEx dt;
			//				dt.SetDate(utilWhichDate._yy,utilWhichDate._mm,utilWhichDate._dd);
			//				if (dt < dtStartDate) {
			//					si.utilWhichDate.SetDate(dtStartDate.GetYear(),dtStartDate.GetMonth(),dtStartDate.GetDay());
			//				}
			//			}
			//		}
		si.nRepeatId = -1;
		SetScheduleInfoData(si);
	}
//	si.uiIndex = uiIndex;//Index assigned Mobile.
	sprintf(si.szuiIndex,szuiIndex);
	si.cuLocation = cuLocation;//Index assigned Mobile.
	si.uiNodeType = uiNodeType;//Index assigned Mobile.
	si.uiPrivate = uiPrivate;//Index assigned Mobile.
	si.bhaveTone = bhaveTone;

}


void CScheduleInfo::DumpScheduleConntent()
{
}

CString ParseOleDateTime(COleDateTimeEx DateTimeEx)
{
    //record.starttime format:  "10/20/2004 08:00"
	CString cuoutputString(_T(""));

	int nLen=0;
	char cBuffer[12];

	//Month
	itoa(DateTimeEx.GetMonth() ,cBuffer,10);
	nLen = strlen(cBuffer);
	if (2>nLen) {
    	cuoutputString += "0";
	}
   	cuoutputString += cBuffer;
	cuoutputString += "/";

	//Day
	itoa(DateTimeEx.GetDay() ,cBuffer,10);
	nLen = strlen(cBuffer);
    if (2>nLen) {
    	cuoutputString += "0";
	}
   	cuoutputString += cBuffer;
	cuoutputString += "/";

	//Year
	itoa(DateTimeEx.GetYear() ,cBuffer,10);
	nLen = strlen(cBuffer);
    if (4>nLen) {
    	cuoutputString += "0";
	}
   	cuoutputString += cBuffer;
	cuoutputString += " ";

	//Hour
	itoa(DateTimeEx.GetHour()  ,cBuffer,10);
	nLen = strlen(cBuffer);
	if (2>nLen) {
    	cuoutputString += "0";
	}
    cuoutputString += cBuffer;

	//Seperator
	cuoutputString += ":";

	//Minute
	itoa(DateTimeEx.GetMinute()  ,cBuffer,10);
	nLen = strlen(cBuffer);
	if (2>nLen) {
    	cuoutputString += "0";
	}
   	cuoutputString += cBuffer;


#ifdef _DEBUG
	//Check result
	CString temp;
	temp = (LPCTSTR) cuoutputString;
	temp.ReleaseBuffer();
#endif
/*
typedef struct {
	// The difference between local time and GMT in hours
	int			Timezone;

	unsigned int		Second;
	unsigned int 		Minute;
	unsigned int		Hour;

	unsigned int 		Day;

	// January = 1, February = 2, etc.
	unsigned int 		Month;

	// Complete year number. Not 03, but 2003
	unsigned int		Year;
} GSM_DateTime;
*/

	return cuoutputString;
}

DWORD CScheduleInfo::GetAlarmBeforeMin(COleDateTime strStartTime, COleDateTime strAlarmtime)
{

	COleDateTimeSpan  dt = strStartTime-strAlarmtime;
	DWORD ltime =(long)(dt.GetTotalSeconds() / 60);
	return ltime;


}
BOOL CScheduleInfo::IsDateExcessEndDate(COleDateTimeEx dt)
{
	if(haveRepeat == FALSE)
	{
		if(dtStopDate < dt)
			return TRUE;
	}
	return FALSE;
}

DWORD CScheduleInfo::GetDifferFlag(CScheduleInfo diffSi)
{
	DWORD dwFlag = 0;
/*	if (uiNodeType != diffSi.uiNodeType) 
		dwFlag |= 1;
	if (dtStartDate != diffSi.dtStartDate)
		dwFlag |= 2;

	if (dtStopDate != diffSi.dtStopDate) 
		dwFlag |= 4;

	if (hiStartTime != diffSi.hiStartTime) 
		dwFlag |= 8;
	if (hiStopTime != diffSi.hiStopTime) 
		dwFlag |= 16;

	if (csTitle.Compare(diffSi.csTitle) != 0) 
		dwFlag |= 32;

	
	//Alarm
	if (haveAlarm != diffSi.haveAlarm) 
		dwFlag |= 64;
	else if(haveAlarm)
	{

	}

	if (m_bAlarm) {
		if (m_ScheduleInfo.alarmLeadtime != m_AlarmLeadTime) {
			return TRUE;
		}
		if (m_ScheduleInfo.bhaveTone != m_bTone) {
			return TRUE;
		}

		COleDateTime dtAlarm;
		if(m_AlarmLeadTime !=0)
		{
  			int nHour=0, nMin=0;
			m_ScheduleInfo.hiStartTime.Get24HourTime(nHour, nMin);

			COleDateTime dtStartDate24Hur;
			dtStartDate24Hur.SetDateTime(m_ScheduleInfo.dtStartDate.GetYear(), m_ScheduleInfo.dtStartDate.GetMonth(), m_ScheduleInfo.dtStartDate.GetDay(),nHour,nMin,0);

			COleDateTimeSpan dtAlarmLeadTimeSpan;
			dtAlarmLeadTimeSpan.SetDateTimeSpan(0,0,_ttoi(m_ScheduleInfo.getAlarmLeadtime()),0);
			dtAlarm = dtStartDate24Hur - dtAlarmLeadTimeSpan;
		}
		else
		{
			int nNoteType = m_ArrayofSupportNoteType[m_cbIndex_NoteType];
			int nY,nM,nD;
			if(nNoteType == BIRTHDAY || nNoteType == MEMO)
			{
				nY = m_Alarmday.GetYear();
				nM = m_Alarmday.GetMonth();
				nD = m_Alarmday.GetDay();
			}
			else
			{

				nY = m_ScheduleInfo.dtStartDate.GetYear();
				nM = m_ScheduleInfo.dtStartDate.GetMonth();
				nD = m_ScheduleInfo.dtStartDate.GetDay();
			}
			dtAlarm.SetDateTime(nY,nM,nD,m_AlarmTime.GetHour(), m_AlarmTime.GetMinute(), m_AlarmTime.GetSecond());

		}
		if (m_ScheduleInfo.dtAlarmDateTime != dtAlarm) {
			return TRUE;
		}






	}

	//Calendar
	//telnumber
		if (m_ScheduleInfo.telnumber.GetLength() != m_ExtraBytes.GetLength()+m_AlarmPhoneNo.GetLength())
			return TRUE;
		if (m_AlarmPhoneNo.GetLength() != 0) {
			if (m_ScheduleInfo.telnumber != m_ExtraBytes+m_AlarmPhoneNo) {
				return TRUE;
			}
		}
	//Location
	CString	_csLocation(m_csLocation);
	if (m_ScheduleInfo.cuLocation.GetLength()!=_csLocation.GetLength()) {
		return TRUE;
	}else if (m_ScheduleInfo.cuLocation.GetLength()!=0
	     	&& (m_ScheduleInfo.cuLocation) != _csLocation) {
		return TRUE;
	}

	//Repeat
	if (m_ScheduleInfo.haveRepeat != m_bRepeat) {
		return TRUE;
	}
	if (m_bRepeat) {
		CString str;
		str = m_RepeatFrequency;
		if (m_ScheduleInfo.repeatFrequency != _ttoi(str)) {
			return TRUE;
		}
		if (m_ScheduleInfo.repeatType != m_RepeatType) {
			return TRUE;
		}
		if (m_ScheduleInfo.rfWhichDay != m_WhichDay) {
			return TRUE;
		}
		if (m_ScheduleInfo.rfWhichWeek != m_RepeatWhichWeek) {
			return TRUE;
		}
	}

	//Repeat until
	if (m_ScheduleInfo.haveRepeatUntil != m_bRepeatUntil) {
		return TRUE;
	}
	if (m_bRepeatUntil) {
		if (m_ScheduleInfo.utilWhichDate._yy != m_UntilDate.GetYear()) {
			return TRUE;
		}
		if (m_ScheduleInfo.utilWhichDate._mm != m_UntilDate.GetMonth()) {
			return TRUE;
		}
		if (m_ScheduleInfo.utilWhichDate._dd != m_UntilDate.GetDay()) {
			return TRUE;
		}
	}
	*/return dwFlag;
}

void CScheduleInfo::UpdateData(CScheduleInfo diffSi, DWORD dwUpdateFlag)
{
	
}

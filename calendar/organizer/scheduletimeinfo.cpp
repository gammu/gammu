// ScheduleTimeInfo.cpp: implementation of the CScheduleTimeInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScheduleTimeInfo.h"
#include "..\loadstring.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScheduleTimeInfo::CScheduleTimeInfo()
{
	hour = 12; min = 0; halfDay = HALFDAY_AM;
}

CScheduleTimeInfo::CScheduleTimeInfo(CScheduleTimeInfo &sti)
{
	hour = sti.hour; 
	min = sti.min; 
	halfDay = sti.halfDay;
}

CScheduleTimeInfo::~CScheduleTimeInfo()
{

}

void CScheduleTimeInfo::SetTimeInfoByHourCount(int hhc) {
	if (hhc == 48) {
		hour = 11;
		min = 59;
		halfDay = HALFDAY_PM;
		return;
	}

	if ((hhc % 2) == 0) {
		min = 0;
	} else {
		min = 30;
	}
	hhc /= 2;
	if (hhc >= 12) {
		halfDay = HALFDAY_PM;
		if (hhc > 12) {
			hhc -= 12;
		}
	} else {
		halfDay = HALFDAY_AM;
		if (hhc == 0) {
			hhc = 12;  //12:00 AM //Modifiwd by NONO, 2004_1128
		}
	}
	hour = hhc;
}
int CScheduleTimeInfo::GetHalfHourCount() {
	int h=hour;
	int m=min;
	if (h == 12) {
		if (halfDay == HALFDAY_AM) {
			h = 0; //Modifiwd by NONO, 2004_1128
		} 
	} else {
		if (halfDay == HALFDAY_PM) {
			h += 12;
		} 
	}
	if (m == 59) {
		m = 60; //Modifiwd by NONO, 2004_1128
	}
	return h*2 + m/30;
}
void CScheduleTimeInfo::SetData(int h,int m,int s) {
	if (h == 0) {   //Modified by NONO, revise the value about zero-hour-time.
		hour = 12;
	} else if (h > 12) {
		hour = h - 12;
	} else {
		hour = h;
	}
	min = m;
	if (h >= 12) {
		halfDay = HALFDAY_PM;
/*		if (h == 12 && m == 0) {//Modifiwd by NONO, 2004_1128
	    	halfDay = HALFDAY_AM;
		}else {
		    halfDay = HALFDAY_PM;
		}
*/	} else {
		halfDay = HALFDAY_AM;
	}
}
CString CScheduleTimeInfo::FormatString() {
	CString strRet;
	if (hour == 12) 
	{
		if (halfDay == HALFDAY_AM) 
		{
			if (min == 0) 
			{
				return "12:00am";
			} else 
			{
			//	return "12:30pm";
				//halfDay == HALFDAY_PM;//Added by NONO, 2004_1128
			}
		} else 
		{
			if (min == 0)
			{
				return _T("12:00pm");
				//return "12:00am";//Modifiwd by NONO, 2004_1128
				//halfDay == HALFDAY_AM;//Added by NONO, 2004_1128
			} else {
				;//return "12:30pm";//Modifiwd by NONO, 2004_1128
			}
		}
	}
	//	strRet.Format(_T("%d"),hour + (halfDay == HALFDAY_AM?0:12));
	strRet.Format(_T("%d"),hour);
	if (strRet.GetLength() == 1) {
		strRet = "0" + strRet;
	}
	CString strMin;
	strMin.Format(_T("%d"),min);
	if (strMin.GetLength() == 1) {
		strMin = "0" + strMin;
	}
	strRet = strRet + ":";
	strRet = strRet + strMin;
	strRet += (halfDay == HALFDAY_AM)?"am":"pm";

	return strRet;
}
CString CScheduleTimeInfo::toString() {
	//total 5 TCHAR
	CString strRet;
	CString strTemp;
	strTemp.Format(_T("%d"),hour);
	if (strTemp.GetLength() == 1) {
		strTemp = "0" + strTemp;
	}
	strRet = strTemp; //hour

	strTemp.Format(_T("%d"),min);
	if (strTemp.GetLength() == 1) {
		strTemp = "0" + strTemp;
	}
	strRet += strTemp; //min

	strTemp.Format(_T("%d"),halfDay);
	strRet += strTemp;  //Length == 1

	return strRet;
}
CString CScheduleTimeInfo::GetTimeString() 
{
	//total 5 TCHAR
	CString strRet;
	CString strTemp;
	int h=hour;
	int m=min;
	if (halfDay == HALFDAY_AM) {
		if (h == 12) {
			h = 0;
		}
	} else if (halfDay == HALFDAY_PM) {
		if (h != 12) {
			h += 12;
		}
	}
	strTemp.Format(_T("%d"),h);
	if (strTemp.GetLength() == 1) {
		strTemp = "0" + strTemp;
	}
	strRet = strTemp; //hour

	strTemp.Format(_T("%d"),m);
	if (strTemp.GetLength() == 1) {
		strTemp = "0" + strTemp;
	}
	strRet += ":";
	strRet += strTemp;  //min

//		strRet += ":00"; //sec
	return strRet;
}
BOOL CScheduleTimeInfo::ParseTimeString(CString inStr,int &hour,int &min,int &sec) {
	if (inStr.GetLength() != 5) {
		return FALSE;
	}
	hour = _ttoi(inStr.Mid(0,2));
	min = _ttoi(inStr.Mid(3,2));
//		sec = _ttoi(inStr.Mid(6,2));
	return TRUE;
}
/*
BOOL CScheduleTimeInfo::ParseString(CString inStr) {
	if (inStr.GetLength() != 5) {
		return FALSE;
	}
	hour = _ttoi(inStr.Mid(0,2));
	min = _ttoi(inStr.Mid(2,2));
	halfDay = _ttoi(inStr.Mid(4,1)) == 0 ? HALFDAY_AM : HALFDAY_PM;
	return TRUE;
}
*/
BOOL CScheduleTimeInfo::ParseAMPMTimeString(CString inStr)
 {
	CString strAM = LoadStringFromFile(_T("public"),_T("AM"));
	CString strPM = LoadStringFromFile(_T("public"),_T("PM"));
	int nAMLen = strAM.GetLength() + 6;
	int nPMLen = strPM.GetLength() + 6;

	if (inStr.GetLength() != 8 && inStr.GetLength() != nAMLen && inStr.GetLength() != nPMLen)
	{
		return FALSE;
	}
	if (inStr.Mid(0,2).CompareNoCase(_T("AM")) == 0 || inStr.Mid(0,strAM.GetLength()).CompareNoCase(strAM) == 0) {
		halfDay = HALFDAY_AM;
	} else if (inStr.Mid(0,2).CompareNoCase(_T("PM")) == 0 || inStr.Mid(0,strPM.GetLength()).CompareNoCase(strPM) == 0) {
		halfDay = HALFDAY_PM;
	} else {
		return FALSE;
	}
	hour = _ttoi(inStr.Mid(3,2));
	if ((hour < 0) || (hour > 12)) {
		return FALSE;
	}
	if (hour == 0) {
		hour = 12;// Modified by nono, 2004_1128
	}

	min = _ttoi(inStr.Mid(6,2));
	if ((min < 0) || (min > 59)) {
		return FALSE;
	}
//		sec = _ttoi(inStr.Mid(6,2));
	return TRUE;
}

CString CScheduleTimeInfo::GetAMPMTimeString() {
	CString str;

	if (halfDay == HALFDAY_AM) {
		str = "AM ";
	} else {
		str = "PM ";
	}
	CString strTmp;
	strTmp.Format(_T("%d"),hour);
	if (strTmp.GetLength() == 1) {
		strTmp = "0" + strTmp;
	}
	str += strTmp;

	strTmp.Format(_T("%d"),min);
	if (strTmp.GetLength() == 1) {
		strTmp = "0" + strTmp;
	}
	str += ":";
	str += strTmp;

	return str;
}

void CScheduleTimeInfo::SetData(CScheduleTimeInfo &sti)
{
	hour = sti.hour; 
	min = sti.min; 
	halfDay = sti.halfDay;
}

const CScheduleTimeInfo& CScheduleTimeInfo::operator =(const CScheduleTimeInfo &sti)
{
	hour = sti.hour; 
	min = sti.min; 
	halfDay = sti.halfDay;

	return *this;
}

const BOOL CScheduleTimeInfo::operator <=(CScheduleTimeInfo &sti)
{
	int nH1,nH2,nM1,nM2;
	Get24HourTime(nH1,nM1);
	sti.Get24HourTime(nH2,nM2);
	COleDateTimeSpan dt1(0,nH1,nM1,0);;
	COleDateTimeSpan dt2(0,nH2,nM2,0);;
	if(dt1<=dt2)
		return TRUE;
	return FALSE;
/*	if (halfDay != sti.halfDay) {
		if (halfDay == HALFDAY_PM) {
			return FALSE;
		}
	}
	if ((hour != 12) && (sti.hour != 12)) {
		if (hour > sti.hour) {
			return FALSE;
		} else if (hour == sti.hour) {
			if (min > sti.min) {
				return FALSE;
			}
		}
		return TRUE;
	}
	if ((hour != 12) && (sti.hour == 12)) {
		return FALSE;
	}
	if ((hour == 12) && (sti.hour != 12)) {
		return TRUE;
	}
	if ((hour == 12) && (sti.hour == 12)) {
		if (min > sti.min) {
			return FALSE;
		}
	}
	return TRUE;*/
}

const BOOL CScheduleTimeInfo::operator < (CScheduleTimeInfo &sti)
{
	int nH1,nH2,nM1,nM2;
	Get24HourTime(nH1,nM1);
	sti.Get24HourTime(nH2,nM2);
	COleDateTimeSpan dt1(0,nH1,nM1,0);;
	COleDateTimeSpan dt2(0,nH2,nM2,0);;
	if(dt1<dt2)
		return TRUE;
	return FALSE;
	

/*	if (halfDay != sti.halfDay) {
		if (halfDay == HALFDAY_PM) {
			return FALSE;
		}
	}
	if ((hour != 12) && (sti.hour != 12))
	{
		if (hour > sti.hour) 
		{
			return FALSE;
		}
		else if (hour == sti.hour)
		{
			if (min >= sti.min)
			{
				return FALSE;
			}
		}
		return TRUE;
	}
	if ((hour != 12) && (sti.hour == 12)) {
		return FALSE;
	}
	if ((hour == 12) && (sti.hour != 12)) {
		return TRUE;
	}
	if ((hour == 12) && (sti.hour == 12)) {
		if (min >= sti.min) {
			return FALSE;
		}
	}
	return TRUE;*/
}

const BOOL CScheduleTimeInfo::operator !=(CScheduleTimeInfo &sti)
{
	if (halfDay != sti.halfDay) {
		return TRUE;
	}
	if (hour != sti.hour) {
		return TRUE;
	}
	if (min != sti.min) {
		return TRUE;
	}
	return FALSE;
}

void CScheduleTimeInfo::MoveToNearestStartTime()
{
	if (min < 30) {
		min = 0;
	}
}

void CScheduleTimeInfo::MoveToNearestStopTime()
{
	if (min < 30) {
		min = 0;
	}
}

void CScheduleTimeInfo::Reset()
{
	hour = 0; min = 0; halfDay = HALFDAY_AM;
}

void CScheduleTimeInfo::MoveToNextHalfHour()
{
	if (min == 0) {
		min = 30;
		return;
	}
	if ((hour == 12) && (min == 30) && (halfDay == HALFDAY_AM)) {
		hour = 1;
		min = 0;
		halfDay = HALFDAY_AM;
		return;
	}
	if ((hour == 11) && (min == 30) && (halfDay == HALFDAY_AM)) {
		hour = 12;
		min = 0;
		halfDay = HALFDAY_PM;
		return;
	}
	if ((hour == 12) && (min == 30) && (halfDay == HALFDAY_PM)) {
		hour = 1;
		min = 0;
		halfDay = HALFDAY_PM;
		return;
	}
	if ((hour == 11) && (min == 59) && (halfDay == HALFDAY_PM)) {
		return;
	}
	if ((hour == 11) && (min == 30) && (halfDay == HALFDAY_PM)) {
		min = 59;
		return;
	}
	if (min == 30) {
		min = 0;
		hour++;
		return;
	}
}

void CScheduleTimeInfo::Get24HourTime(int& nHour, int& nMinute)
{
	if (halfDay != HALFDAY_AM) 
	{
		if (12 != hour && hour <12) 
		{
    		nHour = hour+12;
		}
		else 
		{
	    	nHour = hour;
		}
	}
	else 
	{
		if (12 != hour) 
		{
    		nHour = hour;
		}
		else 
		{
	    	nHour = 0;
	    	//nHour = hour; //Modified by NONO, 2004_1128
		}
	}
    nMinute = min;
	
}
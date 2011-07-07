// ScheduleAllRepeatInfo.cpp: implementation of the CScheduleAllRepeatInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScheduleAllRepeatInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScheduleAllRepeatInfo::CScheduleAllRepeatInfo()
{

}

CScheduleAllRepeatInfo::~CScheduleAllRepeatInfo()
{

}

void CScheduleAllRepeatInfo::AddRepeatScheduleInfo(CScheduleInfo &si)
{
	if (si.haveRepeat || !si.bEndDateSameToStartDate) {
		si.nRepeatId = m_SiList.GetCount();
		m_SiList.AddTail(si);
	}
}

void CScheduleAllRepeatInfo::RemoveRepeatScheduleInfo(CScheduleInfo &si)
{
	POSITION pos = m_SiList.Find(si,NULL);
	if (!pos) {
		return;
	}
	m_SiList.RemoveAt(pos);

	return;
}

void CScheduleAllRepeatInfo::RemoveAllRepeatScheduleInfo()
{
	m_SiList.RemoveAll();

}

void CScheduleAllRepeatInfo::LoadScheduleMonthData(CScheduleMonthInfo& mi) {
	CScheduleInfo si,si2;
	POSITION pos = m_SiList.GetHeadPosition();
	CList<CScheduleInfo,CScheduleInfo> extendList;
	POSITION pos2;

	for (int i=0;i<m_SiList.GetCount();i++) {
		si = m_SiList.GetNext(pos);

		if (si.repeatFrequency < 1) {
			continue;
		}
		extendList.RemoveAll();
		if(si.repeatType  != WEEKLY && si.repeatType  != TWOWEEKLY)
		   si.nDayofWeekMask = 0;
		 
		if(si.nDayofWeekMask == 0)
		{
			ExtendRepeatInfo(si,mi.CurrentYear,mi.CurrentMonth,extendList);
			pos2 = extendList.GetHeadPosition();
			for (int j=0;j<extendList.GetCount();j++) {
				si2 = extendList.GetNext(pos2);

				mi.SetScheduleDate(si2.dtStartDate); //set for day
				mi.AddScheduleInfo(si2);
			}
		}
		else
		{
			CScheduleInfo si3;
			int nDay = si.dtStartDate.GetDayOfWeek() -1;
			for(int i = 0; i<7 ;i++)
			{
				si3 = si;
				if(((si3.nDayofWeekMask >> i) &  1))
				{
					COleDateTimeEx dtTemp;
					if(nDay >i)
					{
						dtTemp.SetDate(si3.dtStartDate.GetYear(),si3.dtStartDate.GetMonth(),si3.dtStartDate.GetDay());
						dtTemp.MoveToPrevDayDate(nDay-i);
 						//dtTemp.MoveToNextWeekDate(si.repeatFrequency);
						dtTemp.MoveToNextWeekDate(1);	//kerm change for 9a9u 1.07
						si3.dtStartDate.SetDate(dtTemp.GetYear(),dtTemp.GetMonth(),dtTemp.GetDay());
					
						dtTemp.SetDate(si3.dtStopDate.GetYear(),si3.dtStopDate.GetMonth(),si3.dtStopDate.GetDay());
						dtTemp.MoveToPrevDayDate(nDay-i);
						//dtTemp.MoveToNextWeekDate(si.repeatFrequency);
						dtTemp.MoveToNextWeekDate(1);	//kerm change for 9a9u 1.07
						si3.dtStopDate.SetDate(dtTemp.GetYear(),dtTemp.GetMonth(),dtTemp.GetDay());
					
						dtTemp.SetDate(si3.dtAlarmDateTime.GetYear(),si3.dtAlarmDateTime.GetMonth(),si3.dtAlarmDateTime.GetDay());
						dtTemp.MoveToPrevDayDate(nDay-i);
						//dtTemp.MoveToNextWeekDate(si.repeatFrequency);
						dtTemp.MoveToNextWeekDate(1);	//kerm change for 9a9u 1.07
						si3.dtAlarmDateTime.SetDate(dtTemp.GetYear(),dtTemp.GetMonth(),dtTemp.GetDay());

					}
					else if(nDay <i)
					{
						dtTemp.SetDate(si3.dtStartDate.GetYear(),si3.dtStartDate.GetMonth(),si3.dtStartDate.GetDay());
						dtTemp.MoveToNextDayDate(i-nDay);
						si3.dtStartDate.SetDate(dtTemp.GetYear(),dtTemp.GetMonth(),dtTemp.GetDay());
					
						dtTemp.SetDate(si3.dtStopDate.GetYear(),si3.dtStopDate.GetMonth(),si3.dtStopDate.GetDay());
						dtTemp.MoveToNextDayDate(i-nDay);
						si3.dtStopDate.SetDate(dtTemp.GetYear(),dtTemp.GetMonth(),dtTemp.GetDay());
					
						dtTemp.SetDate(si3.dtAlarmDateTime.GetYear(),si3.dtAlarmDateTime.GetMonth(),si3.dtAlarmDateTime.GetDay());
						dtTemp.MoveToNextDayDate(i-nDay);
						si3.dtAlarmDateTime.SetDate(dtTemp.GetYear(),dtTemp.GetMonth(),dtTemp.GetDay());

					}
					extendList.RemoveAll();

					ExtendRepeatInfo(si3,mi.CurrentYear,mi.CurrentMonth,extendList);
					pos2 = extendList.GetHeadPosition();
					for (int j=0;j<extendList.GetCount();j++) {
						si2 = extendList.GetNext(pos2);

						mi.SetScheduleDate(si2.dtStartDate); //set for day
						mi.AddScheduleInfo(si2 ,true);
					}
				}
			}
		}
	}
	extendList.RemoveAll();
}
BOOL CScheduleAllRepeatInfo::IsDateExcessEndDate(CScheduleInfo &si,COleDateTimeEx dt)
{
	if(si.haveRepeat == FALSE)
	{
		if(si.dtStopDate < dt)
			return TRUE;
	}
	return FALSE;
}
void CScheduleAllRepeatInfo::ExtendRepeatInfo(CScheduleInfo &si,int year,int month,CList<CScheduleInfo,CScheduleInfo> &extendList) {

//	bool butilDateShift= false;
	COleDateTime untilDate;
	untilDate.SetDate(si.utilWhichDate._yy,si.utilWhichDate._mm,si.utilWhichDate._dd);
//	if((int)si.dtStopDate.m_dt > (int)untilDate.m_dt)
	if(si.nDayofWeekMask == 0)
	{
		untilDate.m_dt +=((int)si.dtStopDate.m_dt -(int)si.dtStartDate.m_dt) ;
//		butilDateShift = true;
	}
	else
	{
		int nDay = untilDate.GetDayOfWeek()-1;
		if((si.nDayofWeekMask >> nDay) &  1)
			untilDate.m_dt +=((int)si.dtStopDate.m_dt -(int)si.dtStartDate.m_dt) ;
		else
		{
			COleDateTimeSpan dts;
			COleDateTime oletemp;
			dts.SetDateTimeSpan(1,0,0,0);
			
			int nShiftDay = ((int)si.dtStopDate.m_dt -(int)si.dtStartDate.m_dt);
			oletemp = untilDate;
			oletemp -= dts;
			for( int n = nShiftDay ; n>0 ;n--)
			{
				nDay = oletemp.GetDayOfWeek()-1;
				if((si.nDayofWeekMask >> nDay) &  1)
				{
					untilDate.m_dt =((int)oletemp.m_dt )+nShiftDay ;
					break;
				}
				oletemp -= dts;

			}
		}
	}

	if (si.haveRepeatUntil) {
		if (untilDate.GetYear() < year) {
			return;
		} else if (untilDate.GetYear() == year) {
			if (untilDate.GetMonth() < month) {
				return;
			}
		}
	}

	if (si.dtStartDate.GetYear() > year) {
		return;
	} else if (si.dtStartDate.GetYear() == year) {
		if (si.dtStartDate.GetMonth() > month) {
			return;
		}
	}
////

	int nHour=0, nMin=0;
	si.hiStartTime.Get24HourTime(nHour, nMin);
	COleDateTime oleStartData,oleStopData,oleDayspain,oleExtendStartTime,oleAlarmtime,oleNewStartDateTime,oleStartDateTime;
	oleStartData.SetDateTime(si.dtStartDate.GetYear(),si.dtStartDate.GetMonth(),si.dtStartDate.GetDay(),0,0,0);
	oleStopData.SetDateTime(si.dtStopDate.GetYear(),si.dtStopDate.GetMonth(),si.dtStopDate.GetDay(),0,0,0);
	oleDayspain = oleStopData - oleStartData;
	oleStartDateTime.SetDateTime(si.dtStartDate.GetYear(),si.dtStartDate.GetMonth(),si.dtStartDate.GetDay(),nHour,nMin,0);

	COleDateTime oleAlarmspain;
	BOOL bAlarmBeforeStart = TRUE;
	oleAlarmspain.m_dt = 0;
	if(si.haveAlarm)
	{
		oleAlarmtime.SetDateTime(si.dtAlarmDateTime.GetYear(),si.dtAlarmDateTime.GetMonth(),si.dtAlarmDateTime.GetDay(),
			si.dtAlarmDateTime.GetHour(),si.dtAlarmDateTime.GetMinute(),si.dtAlarmDateTime.GetSecond());

		if(oleStartDateTime > oleAlarmtime)
		{
			oleAlarmspain = oleStartDateTime - oleAlarmtime ;
			bAlarmBeforeStart = TRUE;
		}
		else
		{
			oleAlarmspain = oleAlarmtime - oleStartDateTime;
			bAlarmBeforeStart = FALSE;
		}


	}
	
//////
	int nRet = 1;
	CScheduleInfo si2;
	si2.SetScheduleInfoData(si);
//	COleDateTime untilDate;
//	untilDate.SetDate(si.utilWhichDate._yy,si.utilWhichDate._mm,si.utilWhichDate._dd);
//	untilDate = untilDate+oleDayspain;
	COleDateTimeEx dt,dtExtendStopDate;
	nRet =si2.GetFirstExtendDate(dt,si2.dtStopDate,year,month,si.dtStartDate.GetDay(),oleDayspain);
	while ((dt.GetYear() == year) && (dt.GetMonth() == month) && !IsDateExcessEndDate(si,dt)) 
	{

		int nY = dt.GetYear() ;
		int nM = dt.GetMonth();
		int nD = dt.GetDay();
		si2.originalData = FALSE;
		si2.dtStartDate = dt;
		if(nRet)
		{
			oleExtendStartTime.SetDateTime(si2.dtStartDate.GetYear(),si2.dtStartDate.GetMonth(),si2.dtStartDate.GetDay(),0,0,0);
			oleStopData = oleExtendStartTime + oleDayspain;
			si2.dtStopDate.SetDateTime(oleStopData.GetYear(),oleStopData.GetMonth(),oleStopData.GetDay(),0,0,0);
		
			oleNewStartDateTime.SetDateTime(si2.dtStartDate.GetYear(),si2.dtStartDate.GetMonth(),si2.dtStartDate.GetDay(),nHour,nMin,0);

			if(si.haveAlarm)
			{
				if(oleAlarmspain.m_dt >=0 )
				{
					if(bAlarmBeforeStart)
						oleAlarmtime = oleNewStartDateTime - oleAlarmspain;
					else
						oleAlarmtime = oleNewStartDateTime + oleAlarmspain;

					si2.dtAlarmDateTime = oleAlarmtime;
				}
				else
					si2.dtAlarmDateTime = oleNewStartDateTime;
			}
		}

		if (si.haveRepeatUntil) 
		{
	/*		if(butilDateShift == false && (int)si2.dtStopDate.m_dt > (int)untilDate.m_dt)
			{
				untilDate.m_dt +=((int)si.dtStopDate.m_dt -(int)si.dtStartDate.m_dt) ;
				butilDateShift = true;
			}*/
			if ((int)si2.dtStartDate.m_dt >(int) untilDate.m_dt) {
				break;
			}
		//	if (si2.dtStopDate > untilDate)
		//		break;
		}
		extendList.AddTail(si2);
	//	dtExtendStopDate = si2.dtStopDate;
		nRet = si2.GetNextExtendDate(dt,si2.dtStopDate,si.dtStartDate.GetDay(),oleDayspain);
	}
	
}
void CScheduleAllRepeatInfo::GetRepeatOriginalInfo(CScheduleInfo &si)
 {
	POSITION pos = m_SiList.GetHeadPosition();
	CScheduleInfo si2;
	for (int i=0;i<m_SiList.GetCount();i++) 
	{
		si2 = m_SiList.GetNext(pos);
		if (si2.nRepeatId == si.nRepeatId)
		{
			si.SetScheduleInfoData(si2);
			break;
		}
	}


}

void CScheduleAllRepeatInfo::RemoveScheduleInfo(CScheduleInfo &si)
{

	CScheduleInfo sr;
	POSITION delpos;
	POSITION pos = m_SiList.GetHeadPosition();
	while(pos)
	{
		delpos = pos;
		sr = m_SiList.GetNext(pos);
//		if(sr.uiIndex == si.uiIndex)
		if(_stricmp(sr.szuiIndex,si.szuiIndex) == 0)
		{
			m_SiList.RemoveAt(delpos);
			break;
		}

	}

}

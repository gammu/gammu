// ScheduleAllInfo.cpp: implementation of the CScheduleAllInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScheduleAllInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScheduleAllInfo::CScheduleAllInfo()
{
}

CScheduleAllInfo::~CScheduleAllInfo()
{
	if (!m_ScheduleRecord.IsEmpty()) {
		m_ScheduleRecord.RemoveAll();
//		for (i=0;i<m_ScheduleRecord.GetCount();i++) {
//			m_ScheduleRecord.RemoveTail();
//		}
	}
/*	if (pScheduleRecord) {
		if (recordCount > 1) {
			//delete [] pScheduleRecord;
			//pScheduleRecord = NULL;
		} else if (recordCount == 1) {
			//delete &pScheduleRecord[0];
			//pScheduleRecord = NULL;
		}
	}
*/
}

//void DumpInsertSchedule(SchedulesRecord *psr,int recordCount);

void CScheduleAllInfo::SetScheduleData(SchedulesRecord *psr,int rc) {
/*	if (recordCount > 0) {
		if (pScheduleRecord) {
			delete [] pScheduleRecord;
			pScheduleRecord = NULL;
		}
	}
*/
//	recordCount = rc;
//	pScheduleRecord = psr;

	int i=0;
	if (!m_ScheduleRecord.IsEmpty()) {
		m_ScheduleRecord.RemoveAll();
//		for (i=0;i<m_ScheduleRecord.GetCount();i++) {
//			m_ScheduleRecord.RemoveTail();
//		}
	}

	for (i=0;i<rc;i++) 
	{
		m_ScheduleRecord.AddTail(psr[i]);
      
	}

//#ifdef _DEBUG
	DumpInsertSchedule(psr,rc);

//#endif

}
void CScheduleAllInfo::LoadScheduleMonthData(CScheduleMonthInfo& mi) {
//	if (!pScheduleRecord) {
//		return;
//	}
	int year,month,day;
	int hour,min,sec=0;
	int year2,month2,day2;
	int hour2,min2,sec2;
	CScheduleInfo si;
	SchedulesRecord sr;
	POSITION pos = m_ScheduleRecord.GetHeadPosition();
	for (int i=0;i<m_ScheduleRecord.GetCount();i++) 
	{
		sr = m_ScheduleRecord.GetNext(pos);
		CString starttime = sr.starttime;
		CString endtime = sr.endtime;
		mi.ParseDateTimeString(starttime,year,month,day,hour,min,sec);
		mi.ParseDateTimeString(endtime,year2,month2,day2,hour2,min2,sec2);
		if (mi.IsSameYearMonth(year,month)/* && mi.IsSameYearMonth(year2,month2)*/) {//Modified by Peggy
//		if (mi.IsSameYearMonth(year,month) && mi.IsSameYearMonth(year2,month2)) {
			si.SetData(sr);

			COleDateTimeEx dt;
			dt.SetDate(year,month,day);
			mi.SetScheduleDate(dt); //set for day
			mi.AddScheduleInfo(si);
		}
	}
}
void CScheduleAllInfo::GetScheduleData(SchedulesRecord *psr,int &rc) {
	ASSERT(psr != NULL);
	SchedulesRecord sr;
	POSITION pos = m_ScheduleRecord.GetHeadPosition();
	for (int i=0;i<m_ScheduleRecord.GetCount();i++) {
		sr = m_ScheduleRecord.GetNext(pos);
		psr[i].SetRecord(&sr);
		
		psr[i].SetPCID(sr.GetPCID());
	}
//	*psr = pScheduleRecord;
	rc = m_ScheduleRecord.GetCount();
//#ifdef _DEBUG
    DumpInsertSchedule(psr,rc);
//#endif
}
void CScheduleAllInfo::GetMonthDataCount(CScheduleMonthInfo& mi,int &cnt) {
	int year,month,day;
	int hour,min,sec;
	cnt = 0;
	SchedulesRecord sr;
	POSITION pos = m_ScheduleRecord.GetHeadPosition();
	for (int i=0;i<m_ScheduleRecord.GetCount();i++) {
		sr = m_ScheduleRecord.GetNext(pos);
		CString starttime = sr.starttime;
		mi.ParseDateTimeString(starttime,year,month,day,hour,min,sec);
		if (mi.IsSameYearMonth(year,month)) {
			cnt++;
		}
	}
}
void CScheduleAllInfo::MoveMonthBeforeScheduleData(CScheduleMonthInfo& mi,CList<SchedulesRecord,SchedulesRecord&> &pSrNew,CList<SchedulesRecord,SchedulesRecord&> &pSrOld) {
	int year,month,day;
	int hour,min,sec;
	SchedulesRecord sr;
	pSrNew.RemoveAll();
	POSITION pos = pSrOld.GetHeadPosition();
	for (int i=0;i<pSrOld.GetCount();i++) {
		sr = pSrOld.GetNext(pos);
		CString starttime = sr.starttime;
#ifdef _DEBUG
		CString csText = (LPCTSTR) starttime;
#endif
		mi.ParseDateTimeString(starttime,year,month,day,hour,min,sec);
		if(year < 0 )
		{

			continue;
		}
		if (mi.CurrentYear > year) {
			pSrNew.AddTail(sr);
			continue;
		}
		if (mi.CurrentYear == year) {
			if (mi.CurrentMonth > month) {
				pSrNew.AddTail(sr);
				continue;
			}
		}
	}
}
void CScheduleAllInfo::MoveMonthAfterScheduleData(CScheduleMonthInfo& mi,CList<SchedulesRecord,SchedulesRecord&> &pSrNew,CList<SchedulesRecord,SchedulesRecord&> &pSrOld) {
	int year,month,day;
	int hour,min,sec;
	SchedulesRecord sr;
	POSITION pos = pSrOld.GetHeadPosition();
	for (int i=0;i<pSrOld.GetCount();i++) {
		sr = pSrOld.GetNext(pos);
		CString starttime = sr.starttime;
		mi.ParseDateTimeString(starttime,year,month,day,hour,min,sec);
		if (mi.CurrentYear == year) {
			if (mi.CurrentMonth < month) {
				pSrNew.AddTail(sr);
				continue;
			}
		}
		if (mi.CurrentYear < year) {
			pSrNew.AddTail(sr);
			continue;
		}
	}
}

void CScheduleAllInfo::SaveScheduleMonthData(CScheduleMonthInfo& mi) {

	DumpInsertSchedule(&m_ScheduleRecord);

	CList<SchedulesRecord,SchedulesRecord&> srList;
	MoveMonthBeforeScheduleData(mi,srList,m_ScheduleRecord);
	//
	DumpInsertSchedule(&srList);

	mi.SaveScheduleMonthData(srList);
	//
	DumpInsertSchedule(&srList);

	MoveMonthAfterScheduleData(mi,srList,m_ScheduleRecord);
	//
	DumpInsertSchedule(&srList);



	int i=0;
	if (!m_ScheduleRecord.IsEmpty()) {
		m_ScheduleRecord.RemoveAll();
	}
	m_ScheduleRecord.AddTail(&srList);

	if (!srList.IsEmpty()) {
		srList.RemoveAll();
	}
}

void CScheduleAllInfo::LoadAllRepeatScheduleData(CScheduleAllRepeatInfo &ari) {
	CScheduleInfo si;
	SchedulesRecord sr;
	CScheduleMonthInfo mi;
	POSITION pos = m_ScheduleRecord.GetHeadPosition();
	for (int i=0;i<m_ScheduleRecord.GetCount();i++) {
		sr = m_ScheduleRecord.GetNext(pos);
		if (sr.haveRepeat || !sr.bEndDateSameToStartDate) {
			si.SetData(sr);
			ari.AddRepeatScheduleInfo(si);
		}
	}
}
void DumpInsertSchedule(SchedulesRecord *psr,int recordCount)
{

}

void DumpInsertSchedule(CList<SchedulesRecord,SchedulesRecord&> *psr)
{
}

void CScheduleAllInfo::RemoveScheduleInfo(CScheduleInfo &si)
{
	SchedulesRecord sr;
	POSITION delpos;
	POSITION pos = m_ScheduleRecord.GetHeadPosition();
	while(pos)
	{
		delpos = pos;
		sr = m_ScheduleRecord.GetNext(pos);
//		if(sr.uiIndex == si.uiIndex)
		if(_stricmp(sr.szuiIndex,si.szuiIndex) == 0)
		{
			m_ScheduleRecord.RemoveAt(delpos);
			break;
		}

	}
}

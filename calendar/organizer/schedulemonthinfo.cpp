// ScheduleMonthInfo.cpp: implementation of the CScheduleMonthInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScheduleMonthInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScheduleMonthInfo::CScheduleMonthInfo()
{
	COleDateTimeEx dtTemp = COleDateTime::GetCurrentTime();
	CurrentYear = dtTemp.GetYear();
	CurrentMonth = dtTemp.GetMonth();
	CurrentDay = dtTemp.GetDay();
}

CScheduleMonthInfo::~CScheduleMonthInfo()
{

}

CScheduleMonthInfo::CScheduleMonthInfo(CScheduleMonthInfo &si) {
	CurrentYear = si.CurrentYear;
	CurrentMonth = si.CurrentMonth;
	CurrentDay = si.CurrentMonth;
	for (int i=0;i<31;i++) {
		sDayInfo[i] = si.sDayInfo[i];
	}
}

COleDateTimeEx CScheduleMonthInfo::GetCurrentDate() {
	COleDateTimeEx dt;
	dt.SetDate(CurrentYear,CurrentMonth,CurrentDay);
	return dt;
}
void CScheduleMonthInfo::Reset() {
	for (int i=0;i<31;i++) {
		sDayInfo[i].Reset();
	}
}
void CScheduleMonthInfo::SaveScheduleMonthData(CList<SchedulesRecord,SchedulesRecord&> &srList) {
	CString dateString;
	int currentDayBackup = CurrentDay;
	for (int i=0;i<31;i++) {
		CurrentDay = i + 1;
		GetDateString(dateString);
		sDayInfo[i].SaveScheduleMonthData(dateString,srList);
	}
	CurrentDay = currentDayBackup;
}
void CScheduleMonthInfo::GetDataCount(int &cnt) {
	cnt = 0;
	int c;
	for (int i=0;i<31;i++) {
		sDayInfo[i].GetDataCount(c);
		cnt += c;
	}
}

void CScheduleMonthInfo::GetRowCount(int &cnt) {
	cnt = 0;
	int c;
	for (int i=0;i<31;i++) {
		sDayInfo[i].GetRowCount(c);
		cnt += c;
	}
}

void CScheduleMonthInfo::GetDateString(CString &cs) {
	CString ss = _T("");
	ss.Format(_T("%d"),CurrentMonth);
	if (ss.GetLength() == 1) {
		ss = _T("0") + ss;
	}
	cs = ss;
	
	ss.Format(_T("%d"),CurrentDay);
	if (ss.GetLength() == 1) {
		ss = "0" + ss;
	}
	cs += _T("/");
	cs += ss;

	ss.Format(_T("%d"),CurrentYear);
	cs += _T("/");
	cs += ss;
}
void CScheduleMonthInfo::ParseDateTimeString(CString &cs,int &year,int &month,int &day,
		                   int &hour,int &min,int &sec) {
	if (cs.GetLength() < 16) {
		return;
	}
	ParseDateString(cs,year,month,day);
	CString ts = cs.Right(5);
	ParseTimeString(ts,hour,min,sec);
}
void CScheduleMonthInfo::ParseDateString(CString &cs,int &year,int &month,int &day) {
	if (cs.GetLength() < 10) {
		return;
	}
	month = _ttoi(cs.Left(2));
	day = _ttoi(cs.Mid(3,2));
	year = _ttoi(cs.Mid(6,4));
}
BOOL CScheduleMonthInfo::IsSameYearMonth(int year,int month) {
	if (year != CurrentYear) {
		return FALSE;
	}
	if (month != CurrentMonth) {
		return FALSE;
	}
	return TRUE;
}
BOOL CScheduleMonthInfo::IsSameDate(int year,int month,int day) {
	if (year != CurrentYear) {
		return FALSE;
	}
	if (month != CurrentMonth) {
		return FALSE;
	}
	if (day != CurrentDay) {
		return FALSE;
	}
	return TRUE;
}
BOOL CScheduleMonthInfo::ParseTimeString(CString &inStr,int &hour,int &min,int &sec) {
	if (inStr.GetLength() != 5) {
		return FALSE;
	}
	hour = _ttoi(inStr.Mid(0,2));
	min = _ttoi(inStr.Mid(3,2));
//		sec = atoi(inStr.Mid(6,2));
	return TRUE;
}

void CScheduleMonthInfo::SetScheduleDate(COleDateTimeEx dt) {
	CurrentYear = dt.GetYear();
	CurrentMonth = dt.GetMonth();
	CurrentDay = dt.GetDay();
}
CScheduleDayInfo CScheduleMonthInfo::GetCurrentDayInfo() {
	return sDayInfo[CurrentDay-1];
}

CScheduleMultiInfo CScheduleMonthInfo::GetInfo(int row) {
	CScheduleMultiInfo si;
	if ((row >=0) && (row < 48)) {
		si = sDayInfo[CurrentDay-1].sInfo[row];
	} else {
		for (int i=0;i<48;i++) {
			if (!sDayInfo[CurrentDay-1].bDirtyFlag[i]) {
				si = sDayInfo[CurrentDay-1].sInfo[i];
				break;
			}
		}
	}
	return si;
}
void CScheduleMonthInfo::GetCurrentDayRowCountBeforeTime(int nTimeRow,int &cnt) {
	cnt = 0;
//	int c;
	for (int i=0;i<nTimeRow;i++) 
		cnt += sDayInfo[CurrentDay-1].sInfo[i].GetCount();
	
}

void CScheduleMonthInfo::GetScheduleInfo(int row,CScheduleInfo &si) {
	int cnt=0;
	int i=0 ;
/*	for (i=0;i<48;i++) {
		for (int j=0;j<sDayInfo[CurrentDay-1].sInfo[i].GetCount()+1;j++) 
		{
			int nCount = sDayInfo[CurrentDay-1].sInfo[i].GetCount();
			si = sDayInfo[CurrentDay-1].sInfo[i].GetScheduleInfo(j);
			if (si.haveRepeat|| !si.bEndDateSameToStartDate)
			{
				if (si.originalData) 
				{
					continue;
				}
			}
			if (cnt == row) 
			{
				if(si.dtStartDate.m_dt == 0)
				{
					si.dtStartDate.SetDateTime(CurrentYear,CurrentMonth,CurrentDay,((si).hiStartTime).hour,
						((si).hiStartTime).min,0);

				}
				return;
			}
			cnt++;
		}
	}*/
cnt=0;
	int nInfoCount = 0  ,nOldInfoCount = 0 ;
	for(i = 0 ; i<48 ;i++)
	{
		nInfoCount += sDayInfo[CurrentDay-1].sInfo[i].GetRowCount() +1;
		if(nInfoCount >= row)
			break;
		nOldInfoCount = nInfoCount;
	}
	if(i<48)
	{
		cnt += nOldInfoCount;
		for (i;i<48;i++) 
		{
			for (int j=0;j<sDayInfo[CurrentDay-1].sInfo[i].GetCount()+1;j++)
			{
				si = sDayInfo[CurrentDay-1].sInfo[i].GetScheduleInfo(j);
				if (si.haveRepeat|| !si.bEndDateSameToStartDate)
				{
					if (si.originalData)
					{
						continue;
					}
				}
				if (cnt == row) 
				{
					if(si.dtStartDate.m_dt == 0)
					{
						si.dtStartDate.SetDateTime(CurrentYear,CurrentMonth,CurrentDay,((si).hiStartTime).hour,
							((si).hiStartTime).min,0);
						si.dtStopDate = si.dtStartDate;

					}
					return;
				}
				cnt++;
			}
		}
	}
}

//CScheduleInfo CScheduleMonthInfo::GetInfo(int row) {
//	CScheduleInfo si;
//	if ((row >=0) && (row < 48)) {
//		si = sDayInfo[CurrentDay-1].sInfo[row];
//	} else {
//		for (int i=0;i<48;i++) {
//			if (!sDayInfo[CurrentDay-1].bDirtyFlag[i]) {
//				si = sDayInfo[CurrentDay-1].sInfo[i];
//				break;
//			}
//		}
//	}
//	return si;
//}

BOOL CScheduleMonthInfo::GetFirstWorkingHourInfo(CScheduleInfo &si) {
	for (int i=16;i<48;i++) {
		//		if (sDayInfo[CurrentDay-1].sInfo[i].IsEmpty()) {
			si.hiStartTime = sDayInfo[CurrentDay-1].sInfo[i].hiStartTime;
			si.rowIndex = sDayInfo[CurrentDay-1].sInfo[i].rowIndex;
			si.dtStartDate.SetDate(CurrentYear,CurrentMonth,CurrentDay);
			if(si.dtStopDate <si.dtStartDate)
				si.dtStopDate = si.dtStartDate;
			si.Reset();
			return TRUE;
			//		}
	}
	return FALSE;
}

//BOOL CScheduleMonthInfo::GetFirstEmptyInfo(CScheduleInfo &si) {
//	for (int i=0;i<48;i++) {
//		if (!sDayInfo[CurrentDay-1].sInfo[i].bScheduleStart) {
//			if (!sDayInfo[CurrentDay-1].bDirtyFlag[i]) {
//				si = sDayInfo[CurrentDay-1].sInfo[i];
//				return TRUE;
//			}
//		}
//	}
//	return FALSE;
//}

BOOL CScheduleMonthInfo::GetUpdatedInfo(int selectedIndex,CScheduleInfo &si) {
	int cnt=0;
	int i;
	if ((selectedIndex < 0) && (selectedIndex >= 48)) {
		return FALSE;
	}
/*
	for (i=0;i<48;i++) {
		for (int j=0;j<sDayInfo[CurrentDay-1].sInfo[i].GetCount();j++) {
			si = sDayInfo[CurrentDay-1].sInfo[i].GetScheduleInfo(j);
			if (si.haveRepeat|| !si.bEndDateSameToStartDate) {
				if (si.originalData) {
					continue;
				}
			}
			if (cnt == selectedIndex) {
				return TRUE;
			}
			cnt++;
		}
	}*/
	int nInfoCount = 0  ,nOldInfoCount = 0 ;
	for(i = 0 ; i<48 ;i++)
	{
	//	nInfoCount += sDayInfo[CurrentDay-1].sInfo[i].GetRowCount() +1;
		nInfoCount += sDayInfo[CurrentDay-1].sInfo[i].GetRowCount() ;
		if(nInfoCount >= selectedIndex)
			break;
		nOldInfoCount = nInfoCount;
	}
	if(i<48)
	{
		cnt += nOldInfoCount;
		for (i;i<48;i++) 

//	for (i=0;i<48;i++) {
		for (int j=0;j<sDayInfo[CurrentDay-1].sInfo[i].GetCount();j++) {
			si = sDayInfo[CurrentDay-1].sInfo[i].GetScheduleInfo(j);
			if (si.haveRepeat|| !si.bEndDateSameToStartDate) {
				if (si.originalData) {
					continue;
				}
			}
			if (cnt == selectedIndex) {
				return TRUE;
			}
			cnt++;
		}
	}
	return GetFirstWorkingHourInfo(si);
}

//BOOL CScheduleMonthInfo::GetUpdatedInfo(int selectedIndex,CScheduleInfo &si) {
//	int cnt=-1;
//	int i;
//	if ((selectedIndex < 0) && (selectedIndex >= 48)) {
//		return FALSE;
//	}
//
//	for (i=0;i<48;i++) {
//		if (sDayInfo[CurrentDay-1].sInfo[i].bScheduleStart) {
//			cnt++;
//			if (cnt == selectedIndex) {
//				si = sDayInfo[CurrentDay-1].sInfo[i];
//				return TRUE;
//			}
//		}
//	}
//	return GetFirstEmptyInfo(si);
//}
//CScheduleInfo CScheduleMonthInfo::GetInfoAndResetInfo(int row) {
//	if ((row >=0) && (row < 48)) {
//		for (int i= sDayInfo[CurrentDay-1].sInfo[row].rowIndex;i<sDayInfo[CurrentDay-1].sInfo[row].rowIndex + sDayInfo[CurrentDay-1].sInfo[row].n30MinBlockCnt; i++) {
//			sDayInfo[CurrentDay-1].bDirtyFlag[i] = FALSE;
//		}
//		sDayInfo[CurrentDay-1].sInfo[row].Reset();
//	}
//	return GetInfo(row);
//}
void CScheduleMonthInfo::ResetScheduleInfo(int row) {
	if ((row >=0) && (row < 48)) {
		sDayInfo[CurrentDay-1].sInfo[row].Reset();
	}
}
//void CScheduleMonthInfo::ResetScheduleInfo(int row) {
//	if ((row >=0) && (row < 48)) {
//		CScheduleInfo si = sDayInfo[CurrentDay-1].sInfo[row];
//		sDayInfo[CurrentDay-1].sInfo[row].Reset();
//		for (int i=si.rowIndex;i<si.rowIndex+si.n30MinBlockCnt;i++) {
//			sDayInfo[CurrentDay-1].bDirtyFlag[i] = FALSE;
//		}
//	}
//}

BOOL CScheduleMonthInfo::AddScheduleInfo(CScheduleInfo &si,bool bExtend) {

	si.DumpScheduleConntent();
	sDayInfo[CurrentDay-1].sInfo[si.rowIndex].AddScheduleInfo(si,bExtend);

	return TRUE;
}

BOOL CScheduleMonthInfo::AddScheduleInfoBeforeSame(CScheduleInfo &si) {

	si.DumpScheduleConntent();
	sDayInfo[CurrentDay-1].sInfo[si.rowIndex].AddScheduleInfoBeforeSame(si);

	return TRUE;
}

BOOL CScheduleMonthInfo::RemoveScheduleInfo(CScheduleInfo &si) {

	si.DumpScheduleConntent();
	return sDayInfo[CurrentDay-1].sInfo[si.rowIndex].RemoveScheduleInfo(si);

}

//BOOL CScheduleMonthInfo::SetInfo(CScheduleInfo &si) {
//	if (CheckDirty(si)) {
//		return FALSE;
//	}
//
//	sDayInfo[CurrentDay-1].sInfo[si.rowIndex] = si;
//	sDayInfo[CurrentDay-1].sInfo[si.rowIndex].bScheduleStart = TRUE;
//	SetDirtyInfo(si,TRUE);
//	
//	return TRUE;
//}
//void CScheduleMonthInfo::SetDirtyInfo(CScheduleInfo si,BOOL bDirty) {
//	for (int i= si.rowIndex;i<si.rowIndex + si.n30MinBlockCnt; i++) {
//		sDayInfo[CurrentDay-1].bDirtyFlag[i] = bDirty;
//	}
//}
//BOOL CScheduleMonthInfo::CheckDirty(CScheduleInfo si) {
//	for (int i= si.rowIndex;i<si.rowIndex + si.n30MinBlockCnt; i++) {
//		if (i<48) {
//			if (sDayInfo[CurrentDay-1].bDirtyFlag[i]) {
//				return TRUE;
//			}
//		}
//	}
//	return FALSE;
//}
CScheduleTimeInfo CScheduleMonthInfo::GetStopTime(CScheduleInfo sti) {
	return sDayInfo[CurrentDay-1].GetStopTime(sti);
}
void CScheduleMonthInfo::SetDate(int year,int month) {
	CurrentYear = year;
	CurrentMonth = month;
	for (int i=0;i<31;i++) {
		sDayInfo[i].Reset();
	}
}
void CScheduleMonthInfo::SetNextMonth() {
	if (CurrentMonth == 12) {
		CurrentMonth = 1;
		CurrentYear++;
	} else {
		CurrentMonth++;
	}
	SetDate(CurrentYear,CurrentMonth);
}
void CScheduleMonthInfo::SetPreMonth() {
	if (CurrentMonth == 1) {
		CurrentMonth = 12;
		CurrentYear--;
	} else {
		CurrentMonth--;
	}
	SetDate(CurrentYear,CurrentMonth);
}

//BOOL CScheduleMonthInfo::CheckConflict(CScheduleInfo sti,int orgRow) {
//	int i =0;
//	for (i=sti.rowIndex;i<sti.GetStopRow();i++) {
//		if (GetCurrentDayInfo().bDirtyFlag[i]) {
//			if ((orgRow != -1) && (orgRow <= i) && (i < GetCurrentDayInfo().sInfo[orgRow].GetStopRow())) {
//				continue;
//			} else {
//				return TRUE;
//			}
//		}
//	}
//
//	return FALSE;
//}
int CScheduleMonthInfo::GetInfoSize() {
	return GetCurrentDayInfo().GetInfoSize();
}

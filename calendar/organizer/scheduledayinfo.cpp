// ScheduleDayInfo.cpp: implementation of the CScheduleDayInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScheduleDayInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScheduleDayInfo::CScheduleDayInfo()
{
	//		CurrentInfo = 0;
	for (int i=0;i<48;i++) {
		sInfo[i].hiStartTime = xGetScheduleTime(i);
		sInfo[i].rowIndex = i;
		sInfo[i].Reset();

		//		bDirtyFlag[i] = FALSE;
	}
}

CScheduleDayInfo::~CScheduleDayInfo()
{

}

CScheduleDayInfo::CScheduleDayInfo(CScheduleDayInfo &sdi) {
	for (int i=0;i<48;i++) {
		//		bDirtyFlag[i] = sdi.bDirtyFlag[i];
		sInfo[i] = sdi.sInfo[i];
	}
}
void CScheduleDayInfo::SaveScheduleMonthData(CString dateString,CList<SchedulesRecord,SchedulesRecord&> &srList) {
	SchedulesRecord sr;
	CScheduleInfo si;
	for (int i=0;i<48;i++) {
		for (int j=0;j<sInfo[i].GetCount();j++) {
			si = sInfo[i].GetScheduleInfo(j);
			if (si.originalData)
			{
				CString starttime;
				starttime = si.GetStartDateString();
				starttime += " ";
				//				starttime += si.GetStartTimeString();
				starttime += si.GetRealStartTimeString();
				CString endtime;
				endtime = si.GetStopDateString();
				endtime += " ";
				//				endtime += GetStopTime(si).GetTimeString();
				endtime += si.GetRealStopTimeString();
				if (si.repeatType == WEEKLYMONTHLY) {
					sr.SetRecord(si.csTitle,starttime,endtime,
						si.csMemo,si.telnumber,si.haveAlarm,si.haveRepeat,
						si.haveRepeatUntil,si.repeatType, si.repeatFrequency
//						,si.rfWhichWeek,si.rfWhichDay,si.utilWhichDate,si.csReasonForReply); 
						,si.rfWhichWeek,si.rfWhichDay,si.nDayofWeekMask,si.utilWhichDate,si.csReasonForReply,
						si.dtAlarmDateTime,si.bEndDateSameToStartDate,
						si.szuiIndex, si.cuLocation, si.uiNodeType, si.uiPrivate, si.bhaveTone);//,
						//si.bhaveTone, si.csHaveAlarm, si.csHaveRepeat, si.alarmLeadDatetime); 
				} else {
					sr.SetRecord(si.csTitle,starttime,endtime,
						si.csMemo,si.telnumber,si.haveAlarm,si.haveRepeat,
						si.haveRepeatUntil,si.repeatType, si.repeatFrequency
//						,0,0,si.utilWhichDate,si.csReasonForReply);
						,0,0,si.nDayofWeekMask,si.utilWhichDate,si.csReasonForReply,
						si.dtAlarmDateTime,si.bEndDateSameToStartDate,
						si.szuiIndex, si.cuLocation, si.uiNodeType, si.uiPrivate, si.bhaveTone);//,
						//si.bhaveTone, si.csHaveAlarm, si.csHaveRepeat, si.alarmLeadDatetime); 
				}
				sr.SetPCID(si.OrgId);

				srList.AddTail(sr);
			}
		}
	}
}
void CScheduleDayInfo::GetDataCount(int &cnt) {
	cnt = 0;
	for (int i=0;i<48;i++) {
		//		if (sInfo[i].bScheduleStart) {
			cnt += sInfo[i].GetCount();
			//		}
	}
}

void CScheduleDayInfo::GetRowCount(int &cnt) {
	cnt = 0;
	for (int i=0;i<48;i++) {
		//		if (sInfo[i].bScheduleStart) {
			cnt += sInfo[i].GetRowCount() + 1;
			//		}
	}
}

CScheduleTimeInfo CScheduleDayInfo::xGetScheduleTime(int tiIndex) {
	CScheduleTimeInfo sti;
	tiIndex %= 48;
	int hourIndex[24] = {12,1,2,3,4,5,6,7,8,9,10,11,12,1,2,3,4,5,6,7,8,9,10,11};
	int minIndex[2] = {0,30};
	sti.hour = hourIndex[tiIndex / 2];
	sti.min =  minIndex[tiIndex % 2];
	if (tiIndex < 24) {
		sti.halfDay = HALFDAY_AM;
	} else {
		sti.halfDay = HALFDAY_PM;
	}
	
	return sti;
}
CScheduleTimeInfo CScheduleDayInfo::GetStopTime(CScheduleInfo sti) {
	int rowIndex = (sti.rowIndex + sti.n30MinBlockCnt);
	if (rowIndex == 48) {
		CScheduleTimeInfo sti;
		sti.SetData(23,59,0);
		return sti;
	} else {
		rowIndex %= 48;
		return sInfo[rowIndex].hiStartTime;
	}
}
void CScheduleDayInfo::Reset() {
	for (int i=0;i<48;i++) {
		//		bDirtyFlag[i] = FALSE;
		sInfo[i].Reset();
	}
}
int CScheduleDayInfo::GetInfoSize() {
	return 48;
}

void CScheduleDayInfo::SetData(CScheduleDayInfo &sdi)
{
	int i;
	for (i=0;i<48;i++) {
		sInfo[i] = sdi.sInfo[i];
		//		bDirtyFlag[i] = sdi.bDirtyFlag[i];
	}
}


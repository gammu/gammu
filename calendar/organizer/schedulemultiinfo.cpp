// ScheduleMultiInfo.cpp: implementation of the CScheduleMultiInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScheduleMultiInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScheduleMultiInfo::CScheduleMultiInfo()
{

}
CScheduleMultiInfo::CScheduleMultiInfo(CScheduleMultiInfo &info)
{
	hiStartTime = info.hiStartTime;
	rowIndex = info.rowIndex;

	POSITION pos = info.m_ScheduleInfoList.FindIndex(0);
	while (pos) {
		m_ScheduleInfoList.AddTail(info.m_ScheduleInfoList.GetNext(pos));
	}
}

CScheduleMultiInfo::~CScheduleMultiInfo()
{

}

void CScheduleMultiInfo::AddScheduleInfo(CScheduleInfo &info,bool bExtend)
{
	POSITION pos = m_ScheduleInfoList.FindIndex(0);
	if (pos == NULL) {
		m_ScheduleInfoList.AddHead(info);
		return;
	}
	CScheduleInfo si;
	POSITION posPre;
	while (pos) {
		posPre = pos;
		si = m_ScheduleInfoList.GetNext(pos);
		if (CompareData(si,info) > 0) {
			m_ScheduleInfoList.InsertBefore(posPre,info);
			return;
		}
		else if (/*bExtend && */si == info 
			&& /*si.uiIndex == info.uiIndex*/_stricmp(si.szuiIndex,info.szuiIndex) == 0 )
		{
			return;

		}
	}
	m_ScheduleInfoList.AddTail(info);
}

void CScheduleMultiInfo::AddScheduleInfoBeforeSame(CScheduleInfo &info)
{
	POSITION pos = m_ScheduleInfoList.FindIndex(0);
	if (pos == NULL) {
		m_ScheduleInfoList.AddHead(info);
		return;
	}
	CScheduleInfo si;
	POSITION posPre;
	while (pos) {
		posPre = pos;
		si = m_ScheduleInfoList.GetNext(pos);
		if (CompareData(si,info) >= 0) {
			m_ScheduleInfoList.InsertBefore(posPre,info);
			return;
		}
	}
	m_ScheduleInfoList.AddTail(info);
}
BOOL CScheduleMultiInfo::RemoveScheduleInfo(int idx)
{
	if ((idx<0) || (idx >= m_ScheduleInfoList.GetCount())) {
		return FALSE;
	}
	POSITION pos = m_ScheduleInfoList.FindIndex(idx);
	m_ScheduleInfoList.RemoveAt(pos);

	return TRUE;
}

BOOL CScheduleMultiInfo::RemoveScheduleInfo(CScheduleInfo &info)
{
/*	CScheduleInfo first;
	if(m_ScheduleInfoList.GetCount() >0)
	{
		first = m_ScheduleInfoList.GetHead();
	}*/
	POSITION delpos;
//	POSITION pos = m_ScheduleInfoList.Find(info,NULL);
//	if (!pos)
	{
//		return FALSE;
//		if(info.uiIndex >0)
		if(strlen(info.szuiIndex) >0)
		{
			POSITION infopos = m_ScheduleInfoList.GetHeadPosition();
			while(infopos)
			{
				delpos = infopos;
				CScheduleInfo si  = m_ScheduleInfoList.GetNext(infopos);
			//	if(si.uiIndex = info.uiIndex)
				if(_stricmp(si.szuiIndex,info.szuiIndex) == 0)
				{
					m_ScheduleInfoList.RemoveAt(delpos);
					return TRUE;
				}
			}
		}

	}
//	else
//		m_ScheduleInfoList.RemoveAt(pos);

	return TRUE;
}

BOOL CScheduleMultiInfo::ReplaceScheduleInfo(int idx, CScheduleInfo &info)
{
	if ((idx<0) || (idx >= m_ScheduleInfoList.GetCount())) {
		return FALSE;
	}
	
	POSITION pos = m_ScheduleInfoList.FindIndex(idx);
	m_ScheduleInfoList.SetAt(pos,info);

	return TRUE;
}

CScheduleInfo CScheduleMultiInfo::GetScheduleInfo(int idx)
{
	CScheduleInfo si;
	if ((idx<0) || (idx >= m_ScheduleInfoList.GetCount())) {
		si.hiStartTime = hiStartTime;
		si.hiStopTime = hiStartTime;
		si.hiStopTime.MoveToNextHalfHour();
		si.hiRealStartTime = si.hiStartTime;
		si.hiRealStopTime = si.hiStopTime;

		si.n30MinBlockCnt = 1;
		si.rowIndex = rowIndex;
		si.uiNodeType = 0;
		si.originalData = TRUE;
		si.dtStartDate.m_dt = 0 ;
		return si;
	}

	POSITION pos = m_ScheduleInfoList.FindIndex(idx);
	return m_ScheduleInfoList.GetAt(pos);
}

BOOL CScheduleMultiInfo::IsScheduleInfoExisted(CScheduleInfo &info)
{

	POSITION pos = m_ScheduleInfoList.Find(info);

	if (pos == NULL) {
		return FALSE;
	}
	return TRUE;
}

int CScheduleMultiInfo::CompareData(CScheduleInfo &info1,CScheduleInfo &info2)
{
	if (info1.n30MinBlockCnt > info2.n30MinBlockCnt) {
		return 1;
	}
	if (info1.n30MinBlockCnt == info2.n30MinBlockCnt) {
		return 0;
	}
	if (info1.n30MinBlockCnt < info2.n30MinBlockCnt) {
		return -1;
	}
	return 0;
}


void CScheduleMultiInfo::Reset()
{
	m_ScheduleInfoList.RemoveAll();
}

void CScheduleMultiInfo::SetData(CScheduleMultiInfo &info)
{
	hiStartTime = info.hiStartTime;
	rowIndex = info.rowIndex;

/*	POSITION pos = info.m_ScheduleInfoList.FindIndex(0);
	while (pos) {
		m_ScheduleInfoList.AddTail(info.m_ScheduleInfoList.GetNext(pos));
	}*/
	m_ScheduleInfoList.AddTail(&info.m_ScheduleInfoList);
}

int CScheduleMultiInfo::GetCount()
{
	return m_ScheduleInfoList.GetCount();
}

int CScheduleMultiInfo::GetRowCount()
{
	int cnt=0;
	CScheduleInfo si;
	POSITION pos = m_ScheduleInfoList.GetHeadPosition();
	for (int i=0;i<m_ScheduleInfoList.GetCount();i++) {
		si = m_ScheduleInfoList.GetNext(pos);
		if (si.haveRepeat|| !si.bEndDateSameToStartDate) {
			if (!si.originalData) {
				cnt++;
			}
		} else {
			cnt++;
		}
	}
	return cnt;
}

BOOL CScheduleMultiInfo::IsEmpty()
{
	if (GetCount() < 1) {
		return TRUE;
	}
	return FALSE;
}

const CScheduleMultiInfo& CScheduleMultiInfo::operator =(CScheduleMultiInfo &smui)
{
	SetData(smui);

	return *this;
}

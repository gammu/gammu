// PhoneData.h: interface for the CPhoneData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PHONEDATA_H__47FE8D5A_5871_4A51_8A37_78A430C59CC7__INCLUDED_)
#define AFX_PHONEDATA_H__47FE8D5A_5871_4A51_8A37_78A430C59CC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "stdafx.h"

class CPhoneData  
{
public:
	void initialize();
	void GetGSMSubMemoryEntry(GSM_SubMemoryEntry &Entry);
	CPhoneData(const GSM_SubMemoryEntry &Entry);
	CPhoneData(const CPhoneData& pn);
	CPhoneData();
	virtual ~CPhoneData();
	//get data
	GSM_EntryType GetType(){return m_iType;}
	LPCTSTR GetDisplayName(){return m_sDisplayName.c_str();}
	LPCTSTR GetText(){return m_sText.c_str();} 
	int GetNumber(){return m_iNumber;}
	void GetTime(COleDateTime &tm){tm = m_tm;}
//	void GetTime(time_t &tm){tm = m_tm.GetTime();}

	//operator
	const CPhoneData& operator=(const CPhoneData& pn){
		m_iType = pn.m_iType;
		m_sDisplayName = pn.m_sDisplayName;
		m_sText = pn.m_sText;
		m_iNumber = pn.m_iNumber;
		m_tm = pn.m_tm;
		m_iTimeZone = pn.m_iTimeZone;
		return *this;
	}

	const CPhoneData& operator=(const GSM_SubMemoryEntry& Entry){
		m_sText = Tstring(reinterpret_cast<const TCHAR*>(Entry.Text));
		m_iNumber = Entry.Number;
		m_tm = COleDateTime(Entry.Date.Year,Entry.Date.Month,Entry.Date.Day,Entry.Date.Hour,
			Entry.Date.Minute,Entry.Date.Second);
		m_iTimeZone = Entry.Date.Timezone;
		SetType(Entry.EntryType);
		return *this;
	}
	//set data
	//auto convert type to display name
	void SetType(const GSM_EntryType iType);
	void SetText(LPCTSTR lpszText){m_sText = lpszText;}
	void SetNumber(const int iNumber){m_iNumber = iNumber;}
	void SetDate(const COleDateTime tm){m_tm = tm;}
//	void SetDisplayName(LPCSTR lpszName){m_sDisplayName = lpszName;}
protected:
	//data type
	GSM_EntryType m_iType;
	//data display name
	Tstring m_sDisplayName;
	//text data
	Tstring m_sText;
	//int data
	int m_iNumber;
	//date data
	COleDateTime m_tm;
	//integer to store time zone
	int	m_iTimeZone;
};

#endif // !defined(AFX_PHONEDATA_H__47FE8D5A_5871_4A51_8A37_78A430C59CC7__INCLUDED_)

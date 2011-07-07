// ListData.h: interface for the CListData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LISTDATA_H__FDE453B5_D2CC_4DE8_9EFD_8F07D15871CD__INCLUDED_)
#define AFX_LISTDATA_H__FDE453B5_D2CC_4DE8_9EFD_8F07D15871CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PhoneData.h"

#define DEQPHONEDATA	deque<CPhoneData>

class CListData  
{
public:
	//only use by csv 
	void GetDataDeque(deque<Tstring> &deqData);
	void SetDataDeque(deque<Tstring> &deqData);
	void SetGSMMemoryEntry(GSM_MemoryEntry Entry);

	void GetGSMMemoryEntry(GSM_MemoryEntry &Entry);
	CListData(const GSM_MemoryEntry &Entry);
	CListData(const CListData &data);
	CListData();
	virtual ~CListData();

	//get data
	DEQPHONEDATA* GetPhoneData(){return &m_deqData;}
	LPSTR GetLocation()
	{
	//	CString strLocation;
	//	strLocation.Format("%s",m_szLocation);
		return m_szLocation;
	}
	const GSM_MemoryType GetMemoryType(){return m_iMemoryType;}
	LPCSTR GetStorageType(){return m_sType.c_str();}

	//get special data
	bool GetName(LPTSTR lpszName);
	bool GetGroupText(LPTSTR lpszGroup);
	int GetGroup();

	//Set data
	void SetPhoneData(DEQPHONEDATA &deqData);
//	void SetLocation(int iLocation){ m_iLocation = iLocation;}
	void SetLocation(LPCSTR  strLocation)
	{ 
		sprintf(m_szLocation,"%s",strLocation);
	}

	void SetMemoryType(GSM_MemoryType iMemoryType){ m_iMemoryType = iMemoryType;}
	void SetStorageType(LPCSTR lpszType){ m_sType = lpszType;}
	void SetStorageType(string &sType){ m_sType = sType;}

	//operation
	const CListData& CListData::operator =(const CListData &ld);
	const CListData& CListData::operator =(const GSM_MemoryEntry &Entry);
	
protected:
	//entry to store data
	DEQPHONEDATA m_deqData;
	//location of data
//	int m_iLocation;
	char m_szLocation[100];
	//memory type of data entry
	GSM_MemoryType m_iMemoryType;
	//memory type of storage
	string m_sType;
};


#endif // !defined(AFX_LISTDATA_H__FDE453B5_D2CC_4DE8_9EFD_8F07D15871CD__INCLUDED_)

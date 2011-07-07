// SupportEntry.h: interface for the CSupportEntry class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SUPPORTENTRY_H__0E0A4985_2C65_461C_9E7F_055C894EED7D__INCLUDED_)
#define AFX_SUPPORTENTRY_H__0E0A4985_2C65_461C_9E7F_055C894EED7D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PhBoundary.h"

#define DEQPHBOUNDARY	deque<CPhBoundary>

class CSupportEntry  
{
public:
	//Construction
	CSupportEntry(const GSM_Phone_Support_Entry &Entry);
	CSupportEntry(const CSupportEntry &se);
	CSupportEntry();
	//Destruction
	virtual ~CSupportEntry();
	//operator
	const CSupportEntry& operator =(const CSupportEntry &se);
	const CSupportEntry& operator =(const GSM_Phone_Support_Entry &Entry);
	//get data
	GSM_MemoryType GetMemoryType(){return m_iMemoryType;}
	DEQPHBOUNDARY* GetPhBoundary(){return &m_deqBoundary;}
	//set data
	void SetMemoryType(GSM_MemoryType iType){ m_iMemoryType = iType;}
	void SetBoundary(DEQPHBOUNDARY &deqBoundary){ m_deqBoundary = deqBoundary;}
protected:
	GSM_MemoryType m_iMemoryType;
	DEQPHBOUNDARY	m_deqBoundary;
};

#endif // !defined(AFX_SUPPORTENTRY_H__0E0A4985_2C65_461C_9E7F_055C894EED7D__INCLUDED_)

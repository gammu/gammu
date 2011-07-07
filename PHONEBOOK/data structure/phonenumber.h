// PhoneNumber.h: interface for the CPhoneNumber class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PHONENUMBER_H__F3236346_7BA7_48E1_AE45_0ACBA12AFF4D__INCLUDED_)
#define AFX_PHONENUMBER_H__F3236346_7BA7_48E1_AE45_0ACBA12AFF4D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPhoneNumber  
{
public:
	CPhoneNumber();
	virtual ~CPhoneNumber();
	//get data
	int GetType(){return m_iType;}
	LPCTSTR GetDisplayName(){return m_sDisplayName.c_str();}
	LPCTSTR GetNumber(){return m_sNumber.c_str();} 
	//operator
	const CPhoneNumber& operator=(const CPhoneNumber& pn){
		m_iType = pn.m_iType;
		m_sDisplayName = pn.m_sDisplayName;
		m_sNumber = pn.m_sNumber;
	}
	//set data
	//auto convert type to display name
	void SetType(int iType);
	void SetNumber(LPCTSTR lpszNumber){m_sNumber = lpszNumber;}
//	void SetDisplayName(LPCSTR lpszName){m_sDisplayName = lpszName;}
protected:
	int m_iType;
	Tstring m_sDisplayName;
	Tstring m_sNumber;
};

#endif // !defined(AFX_PHONENUMBER_H__F3236346_7BA7_48E1_AE45_0ACBA12AFF4D__INCLUDED_)

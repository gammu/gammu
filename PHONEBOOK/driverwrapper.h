// DriverWrapper.h: interface for the CDriverWrapper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DRIVERWRAPPER_H__451530D9_7779_4B9B_AFB8_3DCC9F21C120__INCLUDED_)
#define AFX_DRIVERWRAPPER_H__451530D9_7779_4B9B_AFB8_3DCC9F21C120__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Data Structure\CardPanel.h"
#include "Data Structure\SupportEntry.h"

class CDriverWrapper  
{
public:
	//Open the organize funciton,must call this before any other funtion
	bool Open(int iMobileCompany , TCHAR *szPhone, TCHAR *szPort,TCHAR *szConnectMode, int (*ConnectStatusCallBack)(int iState));
	//Close the organiz,must call this when you won't use dirver	
	void Close();
	//Get the space to store the contracts 
	bool GetContactInfo(int &iSIMUsedNum, int &iSIMFreeNum, int &iMEUsedNum, int &iMEFreeNum);
	bool GetSMContactInfo(int &iSIMUsedNum, int &iSIMFreeNum);
	bool GetMEContactInfo(int &iMEUsedNum, int &iMEFreeNum);
	//Add data
	bool AddContactData(CCardPanel &data,Control_Type Type,int &iErr);
	//update data
	bool UpdateContactData(CCardPanel &data,CCardPanel &Olddata);
	//Get group list	
	bool GetGroupList(deque<Tstring>	&deqGroup);
	//Delete one contract data
	bool DeleteContactData(CCardPanel &data,Control_Type Type,int& iErr);
	//Get the support entry column , ColumnType is a unsigned short array
	bool IsSupportPhonebookDirectSMS();
	bool GetSupportCol(CSupportEntry &se);
public:
	bool EndAddContactData(LPCSTR lpszStorageType);
	bool InitialMemory(LPCSTR lpszType,int& iUsed,int& iFree);
	bool EndDeleteContactData(LPCSTR lpszStorageType);
	bool init();
	//get contact data from mobile
	bool GetContactData(char *lpszMemoryType,CCardPanel &data,bool bStart);
	void FreeDriver();
//	bool GetContactData(TCHAR *lpszMemoryType,DATALIST &lsData);
	//DLL CONTRUCT
	// A Condition to Free the AnwMobile Library 
    void FreeLibrary(HMODULE m_hDriver);

	CDriverWrapper();
	virtual ~CDriverWrapper();

	void SetOpenFlag(bool bOpen){ m_bOpened = bOpen;}
	void xErrorHandling(int iErr);
	int SendSMSData(int MemType, int SMSfolderType, SMS_Data_Strc  * SMS_Data_Strc);
protected:
	bool xInitialLibrary();
	bool m_bOpened;
	bool m_bInit;

protected:
	void xCheckDataType(CCardPanel &data );
	HMODULE m_hDriver;
};

#endif // !defined(AFX_DRIVERWRAPPER_H__451530D9_7779_4B9B_AFB8_3DCC9F21C120__INCLUDED_)

// PhoneData.cpp: implementation of the CPhoneData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\phonebook.h"
#include "PhoneData.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPhoneData::CPhoneData()
{

}


CPhoneData::CPhoneData(const CPhoneData &pn):m_iType(pn.m_iType),m_sDisplayName(pn.m_sDisplayName),
	m_sText(pn.m_sText),m_iNumber(pn.m_iNumber),m_tm(pn.m_tm),m_iTimeZone(pn.m_iTimeZone)
{
}

CPhoneData::CPhoneData(const GSM_SubMemoryEntry &Entry)
{
	m_iNumber = 0;
	m_iTimeZone = 0;
	//set number
	if(Entry.EntryType == PBK_Caller_Group || Entry.EntryType == PBK_Category ||
		Entry.EntryType == PBK_Private || Entry.EntryType == PBK_Number_Sex|| 
		Entry.EntryType == PBK_Number_Light){
		m_iNumber = Entry.Number;
	}
	//set date
	else if(Entry.EntryType == PBK_Date|| Entry.EntryType == PBK_Date_Anniversary){
		m_tm = COleDateTime(Entry.Date.Year,Entry.Date.Month,Entry.Date.Day,Entry.Date.Hour,
			Entry.Date.Minute,Entry.Date.Second);
		
	}
	//set text
	else{
//#ifdef _UNICODE
		m_sText = Tstring(reinterpret_cast<const TCHAR*>(Entry.Text));
/*
#else
		USES_CONVERSION;
		m_sText = Tstring(reinterpret_cast<const TCHAR*>(W2A((WCHAR*)Entry.Text)));

#endif*/
	}
	SetType(Entry.EntryType);
}

CPhoneData::~CPhoneData()
{

}

//set type and display name
void CPhoneData::SetType(const GSM_EntryType iType)
{
//	m_sText = "";
//	m_sDisplayName = "";

	m_iType = iType;
	TCHAR szText[MAX_PATH];
	switch(m_iType)
	{
	case PBK_Number_General:
		al_GetSettingString(_T("public"),_T("IDS_N_GENERAL"),theApp.m_szRes,szText);
		break;
	case PBK_Number_Mobile:  
		if(theApp.m_iMobileCompany == CO_ASUS&& (_tcscmp(theApp.m_szPhone,_T("J202")) != 0))
			al_GetSettingString(_T("public"),_T("IDS_A_TEL1"),theApp.m_szRes,szText);
		else /*if(theApp.m_iMobileCompany == CO_NOKIA || theApp.m_iMobileCompany == CO_MOTO)*/
			al_GetSettingString(_T("public"),_T("IDS_N_MOBILENO"),theApp.m_szRes,szText);
		break;
	case PBK_Number_Work:         
		if(theApp.m_iMobileCompany == CO_NOKIA || theApp.m_iMobileCompany == CO_SIMENS)
			al_GetSettingString(_T("public"),_T("IDS_N_COMPANYNO"),theApp.m_szRes,szText);
		else if(theApp.m_iMobileCompany == CO_ASUS&& (_tcscmp(theApp.m_szPhone,_T("J202")) != 0))
			al_GetSettingString(_T("public"),_T("IDS_A_TEL3"),theApp.m_szRes,szText);
		else if(theApp.m_iMobileCompany == CO_MOTO)
			al_GetSettingString(_T("public"),_T("IDS_M_COMPANY"),theApp.m_szRes,szText);
		else if(theApp.m_iMobileCompany == CO_SONY_ERISON)
			al_GetSettingString(_T("public"),_T("IDS_SE_WORK"),theApp.m_szRes,szText);	
		else if(theApp.m_iMobileCompany == CO_SAMSUNG)
			al_GetSettingString(_T("public"),_T("IDS_N_COMPANYNO"),theApp.m_szRes,szText);	
		else
			al_GetSettingString(_T("public"),_T("IDS_N_COMPANYNO"),theApp.m_szRes,szText);	
		break;
	case PBK_Number_Fax:          
		if(theApp.m_iMobileCompany == CO_ASUS&& (_tcscmp(theApp.m_szPhone,_T("J202")) != 0))
			al_GetSettingString(_T("public"),_T("IDS_A_FAX"),theApp.m_szRes,szText);
		else/* if(theApp.m_iMobileCompany == CO_NOKIA || theApp.m_iMobileCompany == CO_MOTO)*/
			al_GetSettingString(_T("public"),_T("IDS_N_FAX"),theApp.m_szRes,szText);
		break;
	case PBK_Number_Home:         
		if(theApp.m_iMobileCompany == CO_ASUS && (_tcscmp(theApp.m_szPhone,_T("J202")) != 0))
			al_GetSettingString(_T("public"),_T("IDS_A_TEL2"),theApp.m_szRes,szText);
		else/* if(theApp.m_iMobileCompany == CO_NOKIA || theApp.m_iMobileCompany == CO_MOTO)*/
			al_GetSettingString(_T("public"),_T("IDS_N_HOMENO"),theApp.m_szRes,szText);
		break;
	case PBK_Number_Pager:	//B.B.Call
	//	if(theApp.m_iMobileCompany == CO_MOTO)
	//		al_GetSettingString(_T("public"),_T("IDS_M_GENERAL",theApp.m_szRes,szText);
	//	else
			al_GetSettingString(_T("public"),_T("IDS_N_PAGER"),theApp.m_szRes,szText);
		break;
	case PBK_Number_Other:
		if(theApp.m_iMobileCompany == CO_ASUS && (_tcscmp(theApp.m_szPhone,_T("J202")) != 0))
			al_GetSettingString(_T("public"),_T("IDS_A_TEL4"),theApp.m_szRes,szText);
		else if(theApp.m_iMobileCompany == CO_MOTO)
			al_GetSettingString(_T("public"),_T("IDS_M_VIDEO"),theApp.m_szRes,szText);
		else
			al_GetSettingString(_T("public"),_T("IDS_N_OTHER"),theApp.m_szRes,szText);
		break;
	case PBK_Text_Note:
		al_GetSettingString(_T("public"),_T("IDS_N_NOTE"),theApp.m_szRes,szText);
		break;
	case PBK_Text_Postal:	//address
		al_GetSettingString(_T("public"),_T("IDS_N_POSTAL"),theApp.m_szRes,szText);
		break;
	case PBK_Text_Email: 
		al_GetSettingString(_T("public"),_T("IDS_N_EMAIL"),theApp.m_szRes,szText);
		break;
	case PBK_Text_Email2:         
		al_GetSettingString(_T("public"),_T("IDS_N_EMAIL2"),theApp.m_szRes,szText);
		break;
	case PBK_Text_URL:          
		al_GetSettingString(_T("public"),_T("IDS_N_URL"),theApp.m_szRes,szText);
		break;
	case PBK_Date:
		al_GetSettingString(_T("public"),_T("IDS_N_DATE"),theApp.m_szRes,szText);
		break;
	case PBK_Caller_Group:
		al_GetSettingString(_T("public"),_T("IDS_N_GROUP"),theApp.m_szRes,szText);
		break;
	case PBK_Text_Name:          
		al_GetSettingString(_T("public"),_T("IDS_N_NAME"),theApp.m_szRes,szText);
		break;
	case PBK_Text_LastName:          
		al_GetSettingString(_T("public"),_T("IDS_N_LASTNAME"),theApp.m_szRes,szText);
		break;
	case PBK_Text_FirstName:         
		al_GetSettingString(_T("public"),_T("IDS_N_FIRSTNAME"),theApp.m_szRes,szText);
		break;
	case PBK_Text_Company:        
		al_GetSettingString(_T("public"),_T("IDS_N_COMPANY"),theApp.m_szRes,szText);
		break;
	case PBK_Text_JobTitle:          
		al_GetSettingString(_T("public"),_T("IDS_N_JPBTITLE"),theApp.m_szRes,szText);
		break;
	case PBK_Category:  //branch
		al_GetSettingString(_T("public"),_T("IDS_N_CATEGORY"),theApp.m_szRes,szText);
		break;
	case PBK_Private:	
		al_GetSettingString(_T("public"),_T("IDS_N_PRIVATE"),theApp.m_szRes,szText);
		break;
	case PBK_Text_StreetAddress:
		al_GetSettingString(_T("public"),_T("IDS_N_STREETADDRESS"),theApp.m_szRes,szText);
		break;
	case PBK_Text_City:      
		al_GetSettingString(_T("public"),_T("IDS_N_CITY"),theApp.m_szRes,szText);
		break;
	case PBK_Text_State:        
		al_GetSettingString(_T("public"),_T("IDS_N_STATE"),theApp.m_szRes,szText);
		break;
	case PBK_Text_Zip:
		al_GetSettingString(_T("public"),_T("IDS_N_ZIP"),theApp.m_szRes,szText);
		break;
	case PBK_Text_Country:
		al_GetSettingString(_T("public"),_T("IDS_N_COUNTRY"),theApp.m_szRes,szText);
		break;    
	case PBK_Number_Sex:
		al_GetSettingString(_T("public"),_T("IDS_A_SEX"),theApp.m_szRes,szText);
		break;
	case PBK_Number_Light:
		al_GetSettingString(_T("public"),_T("IDS_A_LED"),theApp.m_szRes,szText);
		break;
	case PBK_Number_VideoCall:
		al_GetSettingString(_T("public"),_T("IDS_M_VIDEO"),theApp.m_szRes,szText);
		break;
	case PBK_Number_VideoCall_Home:
		al_GetSettingString(_T("public"),_T("IDS_M_VIDEO_HOME"),theApp.m_szRes,szText);
		break;
	case PBK_Number_VideoCall_Work:
		al_GetSettingString(_T("public"),_T("IDS_M_VIDEO_WORK"),theApp.m_szRes,szText);
		break;
	case PBK_Number_Pager_Home:	//B.B.Call
		al_GetSettingString(_T("public"),_T("IDS_N_PAGER_HOME"),theApp.m_szRes,szText);
		break;
	case PBK_Number_Pager_Work:	//B.B.Call
		al_GetSettingString(_T("public"),_T("IDS_N_PAGER_WORK"),theApp.m_szRes,szText);
		break;
	case PBK_Number_Mobile_Home:
		al_GetSettingString(_T("public"),_T("IDS_MOBLE_HOME"),theApp.m_szRes,szText);
		break;
	case PBK_Number_Mobile_Work:
		al_GetSettingString(_T("public"),_T("IDS_MOBLE_WORK"),theApp.m_szRes,szText);
		break;
	case PBK_Number_Fax_Home:
		al_GetSettingString(_T("public"),_T("IDS_FAX_HOME"),theApp.m_szRes,szText);
		break;
	case PBK_Number_Fax_Work:
		al_GetSettingString(_T("public"),_T("IDS_FAX_WORK"),theApp.m_szRes,szText);
		break;
	case PBK_Text_Email_Home:
		al_GetSettingString(_T("public"),_T("IDS_EMAIL_HOME"),theApp.m_szRes,szText);
		break;
	case PBK_Text_Email_Work:
		al_GetSettingString(_T("public"),_T("IDS_EMAIL_WORK"),theApp.m_szRes,szText);
		break;
	case PBK_Text_Email_Mobile:
		al_GetSettingString(_T("public"),_T("IDS_N_EMAIL"),theApp.m_szRes,szText);
		break;
	case PBK_Text_Email_Unknown:
		al_GetSettingString(_T("public"),_T("IDS_N_EMAIL"),theApp.m_szRes,szText);
		break;
	case PBK_Text_URL_Home:
		al_GetSettingString(_T("public"),_T("IDS_N_URL_HOME"),theApp.m_szRes,szText);
		break;
	case PBK_Text_URL_Work:
		al_GetSettingString(_T("public"),_T("IDS_N_URL_WORK"),theApp.m_szRes,szText);
		break;
	case PBK_Text_Postal_Home:
		al_GetSettingString(_T("public"),_T("IDS_N_POSTAL_HOME"),theApp.m_szRes,szText);
		break;
	case PBK_Text_Postal_Work:
		al_GetSettingString(_T("public"),_T("IDS_N_POSTAL_WORK"),theApp.m_szRes,szText);
		break;
	case PBK_Text_Nickname:
		al_GetSettingString(_T("public"),_T("IDS_N_NICKNAME"),theApp.m_szRes,szText);
		break;
	case PBK_Text_MiddleName:
		al_GetSettingString(_T("public"),_T("IDS_N_MIDDLENAME"),theApp.m_szRes,szText);
		break;
	case PBK_Text_Suffix:
		al_GetSettingString(_T("public"),_T("IDS_N_SUFFIX"),theApp.m_szRes,szText);
		break;
	case PBK_Text_Title:
		al_GetSettingString(_T("public"),_T("IDS_A_TITLE"),theApp.m_szRes,szText);
		break;
	}
	m_sDisplayName = szText;
}



void CPhoneData::GetGSMSubMemoryEntry(GSM_SubMemoryEntry &Entry)
{
	memset(&Entry,0,sizeof(GSM_SubMemoryEntry));
	Entry.EntryType = m_iType;

	if(Entry.EntryType == PBK_Caller_Group || Entry.EntryType == PBK_Category ||
		Entry.EntryType == PBK_Private || Entry.EntryType == PBK_Number_Sex || 
		Entry.EntryType == PBK_Number_Light){
		Entry.Number = m_iNumber;
	}
	//set date
	else if(Entry.EntryType == PBK_Date|| Entry.EntryType == PBK_Date_Anniversary){
		//set time
		Entry.Date.Timezone = m_iTimeZone;
		Entry.Date.Year = m_tm.GetYear();
		Entry.Date.Month = m_tm.GetMonth();
		Entry.Date.Day = m_tm.GetDay();
		Entry.Date.Hour = m_tm.GetHour();
		Entry.Date.Minute = m_tm.GetMinute();
		Entry.Date.Second = m_tm.GetSecond();
	}
	//set text
	else{
//#ifdef _UNICODE
		_tcscpy(reinterpret_cast<TCHAR*>(Entry.Text),m_sText.c_str());
/*#else
		USES_CONVERSION;
		swprintf((WCHAR*)(Entry.Text),L"%s",A2W(m_sText.c_str()));

#endif*/
	}
}

void CPhoneData::initialize()
{
	m_iNumber = 0;
	m_iTimeZone = 0;
	m_sDisplayName = _T("");
	m_sText = _T("");
}

// ListData.cpp: implementation of the CListData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\phonebook.h"
#include "ListData.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CListData::CListData()
{

}

CListData::~CListData()
{

}

void CListData::SetPhoneData(DEQPHONEDATA &deqData)
{
	m_deqData.clear();
	m_deqData = deqData;
}
bool CListData::GetGroupText(LPTSTR lpszGroup)
{
	for(DEQPHONEDATA::iterator iter = m_deqData.begin() ; iter != m_deqData.end() ; iter ++){
		if((*iter).GetType() == PBK_Caller_Group_Text){
			_tcscpy(lpszGroup,(*iter).GetText());
			return true;
		}
	}
	return false;
}

bool CListData::GetName(LPTSTR lpszName)
{
	if(!lpszName)
		return false;
	Tstring sFName,sLName;
	for(DEQPHONEDATA::iterator iter = m_deqData.begin() ; iter != m_deqData.end() ; iter ++){
		if((*iter).GetType() == PBK_Text_Name){
			_tcscpy(lpszName,(*iter).GetText());
			return true;
		}
		else if((*iter).GetType() == PBK_Text_FirstName){
			sFName = (*iter).GetText();
		}
		else if((*iter).GetType() == PBK_Text_LastName){
			sLName = (*iter).GetText();
		}
	}
	if(!sFName.empty() || !sLName.empty()){
		if(!sLName.empty() && !sFName.empty()){
		/*	if(theApp.m_iMobileCompany == CO_NOKIA)	
			{
				_tcscpy(lpszName,sLName.c_str());
				_tcscat(lpszName," ");
				_tcscat(lpszName,sFName.c_str());
			}
			else*/
			{
				_tcscpy(lpszName,sFName.c_str());
				_tcscat(lpszName,_T(" "));
				_tcscat(lpszName,sLName.c_str());
			}
		}
		else if(sLName.empty())
			_tcscpy(lpszName,sFName.c_str());
		else
			_tcscpy(lpszName,sLName.c_str());

		return true;
	}
	return false;
}

int CListData::GetGroup()
{
	for(DEQPHONEDATA::iterator iter = m_deqData.begin() ; iter != m_deqData.end() ; iter ++){
		if((*iter).GetType() == PBK_Caller_Group){
			return (*iter).GetNumber();
		}
	}
	return 0;
}


const CListData& CListData::operator =(const CListData &ld)
{
//	m_iLocation = ld.m_iLocation;
	sprintf(m_szLocation,ld.m_szLocation);
	m_iMemoryType = ld.m_iMemoryType;
	m_deqData.clear();
	m_deqData = ld.m_deqData;	
	m_sType = ld.m_sType;
	return *this;
}

const CListData& CListData::operator =(const GSM_MemoryEntry &Entry)
{
//	m_iLocation = Entry.Location;
	sprintf(m_szLocation,Entry.szIndex);
	m_iMemoryType = Entry.MemoryType;
	for(int i = 0 ; i < Entry.EntriesNum ; i ++){
		CPhoneData data(Entry.Entries[i]);
		m_deqData.push_back(data);
	}

	return *this;
}


CListData::CListData(const CListData &data):/*m_iLocation(data.m_iLocation),*/m_iMemoryType(data.m_iMemoryType)
	,m_deqData(data.m_deqData),m_sType(data.m_sType)
{
	sprintf(m_szLocation,data.m_szLocation);

}

CListData::CListData(const GSM_MemoryEntry &Entry):/*m_iLocation(Entry.Location),*/	m_iMemoryType(Entry.MemoryType)
{
	sprintf(m_szLocation,Entry.szIndex);
	for(int i = 0 ; i < Entry.EntriesNum ; i ++){
		CPhoneData data(Entry.Entries[i]);
		m_deqData.push_back(data);
	}	
}

void CListData::GetGSMMemoryEntry(GSM_MemoryEntry &Entry)
{
	Entry.MemoryType = m_iMemoryType;
//	Entry.Location = m_iLocation;
	sprintf(Entry.szIndex,m_szLocation);

	Entry.EntriesNum = m_deqData.size();
	//get every subentry item
	int i = 0;
	for(DEQPHONEDATA::iterator iter = m_deqData.begin() ; iter != m_deqData.end() ; iter ++ , i++){
		//get subentry
		GSM_SubMemoryEntry SubEntry;
		memset(&SubEntry,0,sizeof(GSM_SubMemoryEntry));
		(*iter).GetGSMSubMemoryEntry(SubEntry);
		//set subentry to Entry
		memcpy(&(Entry.Entries[i]),&SubEntry,sizeof(GSM_SubMemoryEntry));
	}
}


void CListData::GetDataDeque(deque<Tstring> &deqData)
{
	//push name
	TCHAR szName[MAX_PATH];
	memset(szName,0,sizeof(TCHAR) * MAX_PATH);
	GetName(szName);
	deqData.push_back(szName);
	for(DEQPHONEDATA::iterator iter = m_deqData.begin() ; iter != m_deqData.end() ; iter ++){
		if((*iter).GetType() == PBK_Date || (*iter).GetType() == PBK_Date_Anniversary){	// date
			//push id
			TCHAR szFormat[MAX_PATH];
			wsprintf(szFormat,_T("%d"),(*iter).GetType());
			deqData.push_back(szFormat);
			//push date
//			time_t tm;
			COleDateTime tm;
			(*iter).GetTime(tm);
		//	tm.Format(szFormat);
//			sprintf(szFormat,"%d",tm);
	//		deqData.push_back(szFormat);
			CString strszFormat = tm.Format(_T("%d/%m/%Y %H:%M:%S"));
			deqData.push_back(strszFormat.GetBuffer(MAX_PATH));
			strszFormat.ReleaseBuffer();

		}
		else if((*iter).GetType() == PBK_Number_Sex || (*iter).GetType() == PBK_Number_Light
			|| (*iter).GetType() == PBK_Caller_Group || (*iter).GetType() == PBK_Category
			|| (*iter).GetType() == PBK_Private || (*iter).GetType() == PBK_RingtoneID
			|| (*iter).GetType() == PBK_RingtoneFileSystemID || (*iter).GetType() == PBK_PictureID
			|| (*iter).GetType() == PBK_SMSListID ){	//number 
			//push id
			TCHAR szFormat[MAX_PATH];
			wsprintf(szFormat,_T("%d"),(*iter).GetType());
			deqData.push_back(szFormat);
			//push number
			wsprintf(szFormat,_T("%d"),(*iter).GetNumber());
			deqData.push_back(szFormat);
		}
		else if((*iter).GetType() != PBK_Text_Name){	//text
			//push id
			TCHAR szFormat[MAX_PATH];
			wsprintf(szFormat,_T("%d"),(*iter).GetType());
			deqData.push_back(szFormat);
			//push text
			deqData.push_back((*iter).GetText());
		}
	}
}

void CListData::SetDataDeque(deque<Tstring> &deqData)
{
	int i = 0 ;
	for(deque<Tstring>::iterator iter = deqData.begin() ; iter != deqData.end() ; iter ++ , i ++){
		CPhoneData data;
		if( i == 0 ) {	//name
			data.SetType(PBK_Text_Name);
			data.SetText((*iter).c_str());
			m_deqData.push_back(data);
		}
		else if( (i + 1) % 2 == 0 ){	//ID and data
			int iType = _ttoi((*iter).c_str());
			if(iType == PBK_Date|| iType== PBK_Date_Anniversary){	//date
		//		time_t tm = atol((*(iter + 1)).c_str());
		//		COleDateTime time(tm);
				COleDateTime time;
				CString str;
				str.Format(_T("%s"),(*(iter + 1)).c_str());
				time.ParseDateTime(str);
				data.SetType((GSM_EntryType)(iType));
				data.SetDate(time);
				m_deqData.push_back(data);
			}
			else if(iType == PBK_Caller_Group || iType == PBK_Category || iType == PBK_Private
				|| iType == PBK_RingtoneID || iType == PBK_RingtoneFileSystemID || iType == PBK_PictureID
				|| iType == PBK_SMSListID || iType == PBK_Number_Sex || iType == PBK_Number_Light){	//number
				int iNum = _ttoi((*(iter + 1)).c_str());
				data.SetType((GSM_EntryType)(iType));
				data.SetNumber(iNum);
				m_deqData.push_back(data);
			}
			else if(iType != 0){	//text
				data.SetType((GSM_EntryType)(iType));
				data.SetText((*(iter + 1)).c_str());
				m_deqData.push_back(data);
			}
		}//end of ID and data
	}
}
void CListData::SetGSMMemoryEntry(GSM_MemoryEntry Entry)
{
	m_deqData.clear();
	m_iMemoryType = Entry.MemoryType;
	sprintf(m_szLocation,Entry.szIndex);
	for(int i = 0 ; i < Entry.EntriesNum ; i ++){
		CPhoneData data(Entry.Entries[i]);
		m_deqData.push_back(data);
	}	
}

// DriverWrapper.cpp: implementation of the CDriverWrapper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PhoneBook.h"
#include "DriverWrapper.h"
#include "CommUIExportFun.h"

//Include the mobile driver

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

OpenPhoneBookProc OpenPhoneBookfn;
ClosePhoneBookProc ClosePhoneBookfn;
GetPhoneBookInfoProc GetPhoneBookInfofn;
GetPhoneBookStartDataProc GetPhoneBookStartDatafn;
GetPhoneBookNextDataProc GetPhoneBookNextDatafn;
DeletePhoneBookDataProc DeletePhoneBookDatafn;
GetGroupNameProc GetGroupNamefn;
GetGroupNumProc GetGroupNumfn;
GetMobileNumProc GetMobileNumfn;
GetMobileListProc GetMobileListfn;
AddPhoneBookDataProc AddPhoneBookDatafn;
GetAllGroupProc GetAllGroupfn;
UpdatePhoneBookDataProc UpdatePhoneBookDatafn;
InitPhoneBookProc			InitPhoneBookfn;
TerminatePhoneBookProc		TerminatePhoneBookfn;
GetPhoneBookSupportColumnProc GetPhoneBookSupportColumnnfn;
InitialMemoryStatusProc		InitialMemoryStatusfn;
GetPhoneBookInfoExProc GetPhoneBookInfoExfn;
IsSupportPhonebookDirectSMSProc IsSupportPhonebookDirectSMSfn;

anwSendSMSData		ANWSendSMSData;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDriverWrapper::CDriverWrapper()
{
	xInitialLibrary();
	m_bOpened = false;
	m_bInit = false;
}

CDriverWrapper::~CDriverWrapper()
{
}

//must call this function 
bool CDriverWrapper::Open(int iMobileCompany, TCHAR *szPhone, TCHAR *szPort, TCHAR *szConnectMode, int (__cdecl *ConnectStatusCallBack)(int State))
{
	if(!m_bOpened){
		int iErr = OpenPhoneBookfn(iMobileCompany,szPhone,szConnectMode,szPort,theApp.m_szIMEI,ConnectStatusCallBack);
		xErrorHandling(iErr);
		return  iErr == 1 ? true : false;
	}
	return true;
}

void CDriverWrapper::Close()
{
	TerminatePhoneBookfn();
	ClosePhoneBookfn();
	m_bOpened = false;
}
bool CDriverWrapper::GetSMContactInfo(int &iSIMUsedNum, int &iSIMFreeNum)
{
	if(!m_bOpened)
		return false;

	int nUsed,nFree;
	int iErr = GetPhoneBookInfoExfn("SM",&nUsed,&nFree);
	// v2.0.1.0 ; for unknow error ; by mingfa
	//NOTE: This is a temp solution for release, we need to modified code in the future.
     if ( iErr == Anw_UNKNOW_ERROR )
		iErr = 9999;

    // mingfa--

	xErrorHandling(iErr);
	if( iErr == 1)
	{
		iSIMFreeNum = nFree;
		iSIMUsedNum = nUsed;
		return true;
	}
	
	return false;
}
bool CDriverWrapper::GetMEContactInfo(int &iMEUsedNum, int &iMEFreeNum)
{
	if(!m_bOpened)
		return false;

	int nUsed,nFree;
	int iErr = GetPhoneBookInfoExfn("ME",&nUsed,&nFree);
	// v2.0.1.0 ; for unknow error ; by mingfa
	//NOTE: This is a temp solution for release, we need to modified code in the future.
     if ( iErr == Anw_UNKNOW_ERROR )
		iErr = 9999;

    // mingfa--

	xErrorHandling(iErr);
	if( iErr == 1){
		iMEFreeNum = nFree;
		iMEUsedNum = nUsed;
		return true;
	}
	
	return false;
}

bool CDriverWrapper::GetContactInfo(int &iSIMUsedNum, int &iSIMFreeNum, int &iMEUsedNum, int &iMEFreeNum)
{
	if(!m_bOpened)
		return false;

	Contact_Tal_Num tal_num;
	int iErr = GetPhoneBookInfofn(&tal_num);
	// v2.0.1.0 ; for unknow error ; by mingfa
	//NOTE: This is a temp solution for release, we need to modified code in the future.
     if ( iErr == Anw_UNKNOW_ERROR )
		iErr = 9999;

    // mingfa--

	xErrorHandling(iErr);
	if( iErr == 1){
		iSIMFreeNum = tal_num.SIMContactTotalNum - tal_num.SIMContactUsedNum;
		iSIMUsedNum = tal_num.SIMContactUsedNum;
		iMEFreeNum = tal_num.MEContactTatalNum - tal_num.MEContactUsedNum;
		iMEUsedNum = tal_num.MEContactUsedNum;
		return true;
	}
	
	return false;
}

bool CDriverWrapper::AddContactData(CCardPanel &data,Control_Type Type,int& iErr)
{
	if(!m_bOpened)
		return false;
	//DLL CONTRUCT
	if(afxContextIsDLL)
	  ::SetCursor(LoadCursor(NULL, IDC_WAIT));
	else 
	  CWaitCursor ww;
	//get mobile space and check it enough to add item
/*	int iSIMUsed = 0, iSIMFree = 0 , iMEUsed = 0 , iMEFree = 0 , iFree = 0;
	
	int iErr = GetContactInfo(iSIMUsed,iSIMFree,iMEUsed,iMEFree);
	xErrorHandling(iErr);
	if( iErr != 1)
		return false;

	//check data storage type 
	if(strcmp(data.GetStorageType(),_T("ME")) == 0)
		iFree = iMEFree;
	else if(strcmp(data.GetStorageType(),_T("SM")) == 0)
		iFree = iSIMFree;
	
	//if no space ,show error msg
	if(iFree <= 0){
		TCHAR szMsg[MAX_PATH];
		al_GetSettingString(_T("public"),_T("IDS_ERR_NOSPACE"),theApp.m_szRes,szMsg);
		AfxMessageBox(szMsg);
		return false;
	}*/
	//check is SIM,if true,make correct format

	CCardPanel dataFormat = data;
	if(strcmp(data.GetStorageType(),SIM_NAME) == 0)
	{
		if(!dataFormat.GetSIMForamt(data))
		{
		//	TCHAR szMsg[MAX_PATH];
		//	al_GetSettingString(_T("public"),_T("IDS_ERR_INVALIDDATA"),theApp.m_szRes,szMsg);
		//	AfxMessageBox(szMsg);
			iErr = Anw_INVALIDDATA;
			return false;
		}
	}

	//add Contact_Data_Struct to mobile and return the new item location
	//get GSM_MemoryEntry object from CCardPanel
	GSM_MemoryEntry Entry;
	memset(&Entry,0,sizeof(GSM_MemoryEntry));
	if(strcmp(data.GetStorageType(),MEMORY_NAME) == 0)
		xCheckDataType(data);
	data.GetGSMMemoryEntry(Entry);
	//add data to mobile or SIM card
//	int 
	iErr = AddPhoneBookDatafn(const_cast<char *>(data.GetStorageType()),&Entry,Type);

	//check add sim card data error
	if((iErr == Anw_NOTSUPPORTED || iErr == Anw_INVALIDDATA || iErr == Anw_UNKNOW_ERROR) && strcmp(data.GetStorageType(),SIM_NAME) == 0){
		//if add error and memory type is SIM Card,cut string length 
		Tstring sNameTmp,sNumTmp;
		for(int i = 0 ; i < Entry.EntriesNum ; i ++){
			if(Entry.Entries[i].EntryType == PBK_Text_Name){
				Entry.Entries[i].Text[12*sizeof(TCHAR)] = 0;
				sNameTmp = reinterpret_cast<TCHAR *>(Entry.Entries[i].Text);
			}
			else if(Entry.Entries[i].EntryType == PBK_Number_General){
				Entry.Entries[i].Text[20*sizeof(TCHAR)] = 0;
				sNumTmp = reinterpret_cast<TCHAR *>(Entry.Entries[i].Text);
			}
		}
		//add this data once again ; v2.0.1.1
		if(Type == Type_Start)
			Type = Type_Continune;
		iErr = AddPhoneBookDatafn(const_cast<char *>(data.GetStorageType()),&Entry,Type);
		if(iErr == Anw_NOTSUPPORTED || iErr == Anw_INVALIDDATA || iErr == Anw_UNKNOW_ERROR)
		{
			for(int i = 0 ; i < Entry.EntriesNum ; i ++)
			{
				if(Entry.Entries[i].EntryType == PBK_Text_Name)
				{
					Entry.Entries[i].Text[10*sizeof(TCHAR)] = 0;
					sNameTmp = reinterpret_cast<TCHAR *>(Entry.Entries[i].Text);
				}
			}
			iErr = AddPhoneBookDatafn(const_cast<char *>(data.GetStorageType()),&Entry,Type);

			if(iErr == Anw_NOTSUPPORTED || iErr == Anw_INVALIDDATA|| iErr == Anw_UNKNOW_ERROR)
			{
				for(int i = 0 ; i < Entry.EntriesNum ; i ++)
				{
					if(Entry.Entries[i].EntryType == PBK_Text_Name)
					{
						Entry.Entries[i].Text[8*sizeof(TCHAR)] = 0;
						sNameTmp = reinterpret_cast<TCHAR *>(Entry.Entries[i].Text);
					}
				}
				iErr = AddPhoneBookDatafn(const_cast<char *>(data.GetStorageType()),&Entry,Type);
			}

		}
		//set the fixed string back to data
		if(iErr == 1){
			DEQPHONEDATA *pdeqData;
			pdeqData = data.GetPhoneData();
			for(DEQPHONEDATA::iterator iter = pdeqData->begin() ; iter != pdeqData->end() ; iter ++ ){
				if((*iter).GetType() == PBK_Text_Name)
					(*iter).SetText(sNameTmp.c_str());
				else if((*iter).GetType() == PBK_Number_General)
					(*iter).SetText(sNumTmp.c_str());
			}
//			data.SetPhoneData(*pdeqData);
		}
	}

//	xErrorHandling(iErr);
	if( iErr == 1){
		//Set the location in mobile to CCardPanel
//		data.SetLocation(Entry.Location);
		data.SetGSMMemoryEntry(Entry);
		data.SetLocation(Entry.szIndex);
		return true;
	}
	return true;
//	return false;
}
bool CDriverWrapper::UpdateContactData(CCardPanel &data,CCardPanel &Olddata)
{
	bool bSuc = true;
	if(!m_bOpened)
		return false;
	//Convert CCardPanel to Contact_Data_Struct
	//DLL CONTRUCT
	if(afxContextIsDLL)
	  ::SetCursor(LoadCursor(NULL, IDC_WAIT));
	else 
	  CWaitCursor ww;
	
	Control_Type type(Type_Start);
	int iErr =1,iErr2 =1;
	bool bDelSuc= DeleteContactData(data,type,iErr);
	if(bDelSuc && iErr ==1)
	{
		type = Type_Continune;
		bSuc = AddContactData(data,type,iErr);
		if(!bSuc || (bSuc && iErr !=1))
		{
			AddContactData(Olddata,type,iErr2);
			bSuc = false;
		}
	}
	else 
		bSuc = false;
	type = Type_Continune;
	EndAddContactData(const_cast<char *>(data.GetStorageType()));
	xErrorHandling(iErr);

	return bSuc;
}
/*
//Do not work 0921
bool CDriverWrapper::UpdateContactData(CCardPanel &data)
{
	if(!m_bOpened)
		return false;
	//Convert CCardPanel to Contact_Data_Struct
	//DLL CONTRUCT
	if(afxContextIsDLL)
	  ::SetCursor(LoadCursor(NULL, IDC_WAIT));
	else 
	  CWaitCursor ww;
	GSM_MemoryEntry Entry;
	memset(&Entry,0,sizeof(GSM_MemoryEntry));
	data.GetGSMMemoryEntry(Entry);
	//Update Contact_Data_Struct to mobile
	int iErr = UpdatePhoneBookDatafn(const_cast<char *>(data.GetStorageType()),&Entry);

	//check add sim card data error
	if((iErr == Anw_NOTSUPPORTED || iErr == Anw_INVALIDDATA) && strcmp(data.GetStorageType(),SIM_NAME) == 0){
//	if(iErr == Anw_NOTSUPPORTED && strcmp(data.GetStorageType(),SIM_NAME) == 0){
		//if add error and memory type is SIM Card,cut string length 
		string sNameTmp,sNumTmp;
		for(int i = 0 ; i < Entry.EntriesNum ; i ++){
			if(Entry.Entries[i].EntryType == PBK_Text_Name){
				Entry.Entries[i].Text[12] = 0;
				sNameTmp = reinterpret_cast<LPCSTR>(Entry.Entries[i].Text);
			}
			else if(Entry.Entries[i].EntryType == PBK_Number_General){
				Entry.Entries[i].Text[20] = 0;
				sNumTmp = reinterpret_cast<LPCSTR>(Entry.Entries[i].Text);
			}
		}
		//add this data once again
		iErr = UpdatePhoneBookDatafn(const_cast<char *>(data.GetStorageType()),&Entry);
		//set the fixed string back to data
		if(iErr == 1){
			DEQPHONEDATA *pdeqData;
			pdeqData = data.GetPhoneData();
			for(DEQPHONEDATA::iterator iter = pdeqData->begin() ; iter != pdeqData->end() ; iter ++ ){
				if((*iter).GetType() == PBK_Text_Name)
					(*iter).SetText(sNameTmp.c_str());
				else if((*iter).GetType() == PBK_Number_General)
					(*iter).SetText(sNumTmp.c_str());
			}
//			data.SetPhoneData(*pdeqData);
		}
	}

	xErrorHandling(iErr);
	if( iErr == 1)
	{
//		data.SetLocation(Entry.Location);
		data.SetGSMMemoryEntry(Entry);
		data.SetLocation(Entry.szIndex);
		return true;
	}

	return false;
}*/

bool CDriverWrapper::GetGroupList(deque<Tstring>	&deqGroup)
{
	if(!m_bOpened)
		return false;
	//get group list size
	int nCount = 0;
	int iErr = GetGroupNumfn(&nCount);
	if(iErr == Anw_NOTSUPPORTED || nCount == 0)
	{
// 		TCHAR szCompany[MAX_PATH];
// 		szCompany[0] = '\0';
// 		switch(theApp.m_iMobileCompany )
// 		{
// 			case CO_NOKIA:
// 				wsprintf(szCompany,_T("Nokia"));
// 				break;
// 			case CO_SONY_ERISON:
// 				wsprintf(szCompany,_T("SE"));
// 				break;
// 			case CO_SIMENS:
// 				wsprintf(szCompany,_T("Siemens"));
// 				break;
// 			case CO_ASUS:
// 				wsprintf(szCompany,_T("ASUS"));
// 				break;
// 			case CO_MOTO:
// 				wsprintf(szCompany,_T("MOTO"));
// 				break;
// 			case CO_SAMSUNG:
// 				wsprintf(szCompany,_T("Samsung"));
// 				break;
// 			case CO_SHARP:
// 				wsprintf(szCompany,_T("Sharp"));
// 				break;
// 			case CO_LG:
// 				wsprintf(szCompany,_T("LG"));
// 				break;
// 			case CO_PANASONIC:
// 				wsprintf(szCompany,_T("Panasonic"));
// 				break;
// 			case CO_SAGEM:
// 				wsprintf(szCompany,_T("SAGEM"));
// 				break;
// 			case CO_TOSHIBA:
// 				wsprintf(szCompany,_T("Toshiba"));
// 				break;
// 			case CO_MTK:
// 				wsprintf(szCompany,_T("MTK"));
// 				break;
// 		}
		//get profile path
		Tstring sProfile(theApp.m_szSkin);
		
// 		TCHAR szProfile[MAX_PATH];
// 		if(_tcslen(szCompany)>0)
// 			wsprintf(szProfile,_T("%sOrganize\\%s %s.ini"),theApp.m_szSkin,szCompany,theApp.m_szPhone);
// 		else
// 			wsprintf(szProfile,_T("%sOrganize\\%s.ini"),theApp.m_szSkin,theApp.m_szPhone);
// 		if(al_IsFileExist(szProfile))
// 		{
// 			TCHAR szReference[MAX_PATH];
// 			if(al_GetSettingString(_T("Dlg"),_T("ReferenceTO"),szProfile,szReference))
// 			{
// 				wsprintf(szProfile,_T("%sOrganize\\%s"), theApp.m_szSkin,szReference);
// 			}
// 			sProfile = Tstring(szProfile);
// 		}
// 		else
// 		{
// 			wsprintf(szProfile,_T("%sOrganize\\%s.ini"),theApp.m_szSkin,szCompany);
// 			if(al_IsFileExist(szProfile))
// 			{
// 				TCHAR szReference[MAX_PATH];
// 				if(al_GetSettingString(_T("Dlg"),_T("ReferenceTO"),szProfile,szReference))
// 				{
// 					wsprintf(szProfile,_T("%sOrganize\\%s"), theApp.m_szSkin,szReference);
// 				}
// 				sProfile = Tstring(szProfile);
// 			}
// 			else
// 				sProfile += Tstring(_T("Organize\\Default.ini"));
// 		}
          sProfile += Tstring(_T("Organize\\M5511Dlg.ini"));
		int iCount = 0;
		TCHAR szKey[MAX_PATH];
		TCHAR szTextID[MAX_PATH];
		TCHAR szText[MAX_PATH];
		if(!al_GetSettingInt(_T("Group"),_T("nCount"),const_cast<TCHAR*>(sProfile.c_str()),iCount))
			return false;
	
		for(int i = 0 ; i <= 5 ; i ++)
		{
			wsprintf(szKey,_T("Group%d") ,i);
			if(al_GetSettingString(_T("Group"),szKey,const_cast<TCHAR*>(sProfile.c_str()),szTextID))
			{
				al_GetSettingString(_T("public"),szTextID,theApp.m_szRes,szText);
				Tstring str(szText);
			/*	AfxMessageBox(str.c_str());*/
				deqGroup.push_back(str);
			}
		}
		return true;

	}
	xErrorHandling(iErr);
	if( iErr != 1)
		return false;

	deqGroup.clear();

	//add the first "no group" into deque
	if(theApp.m_iMobileCompany == CO_NOKIA){
		Tstring sFirst(_T(" "));
		deqGroup.push_back(sFirst);
	}

	//use a array to recursive 
	TCHAR **pGroup;
	pGroup = new TCHAR*[nCount + 1];
	for(int j = 0 ; j < nCount + 1; j ++){
		pGroup[j] = new TCHAR[MAX_PATH];
		memset(pGroup[j],0,sizeof(TCHAR) * MAX_PATH);
	}

	iErr = GetAllGroupfn(pGroup);
	xErrorHandling(iErr);
	if( iErr == 1){
		for(int i = 0 ; i < nCount ; i ++){
			Tstring str(pGroup[i]);
			deqGroup.push_back(str);
		}

		for(j = 0 ; j < nCount + 1; j ++){
			delete [] pGroup[j];
		}
		SAFE_DELPTR(pGroup);
		
		return true;
	}

	for(j = 0 ; j < nCount + 1; j ++){
		delete [] pGroup[j];
	}
	SAFE_DELPTR(pGroup);

	return false;
}

bool CDriverWrapper::DeleteContactData(CCardPanel &data,Control_Type Type,int& iErr)
{
	if(!m_bOpened)
		return false;
//	int iErr = DeletePhoneBookDatafn(const_cast<TCHAR*>(data.GetStorageType()),data.GetLocation(),Type);
//	int 
	iErr = DeletePhoneBookDatafn(const_cast<char*>(data.GetStorageType()),data.GetLocation(),Type);
//	xErrorHandling(iErr);
//	return  iErr == 1 ? true : false;
	return true;
}

bool CDriverWrapper::IsSupportPhonebookDirectSMS()
{
	if(!m_bOpened)
		return false;
	return IsSupportPhonebookDirectSMSfn();
}

bool CDriverWrapper::GetSupportCol(CSupportEntry &se)
{
	if(!m_bOpened)
		return false;
	//Get column type array to the array which driver provide
	int iErr = 1;
	GSM_Phone_Support_Entry Entry;
	memset(&Entry,0,sizeof(GSM_Phone_Support_Entry));
	Entry.MemoryType = se.GetMemoryType();
	iErr = GetPhoneBookSupportColumnnfn(&Entry);
	xErrorHandling(iErr);
	//convert the array to column type which we provide
	if(iErr == 1){
		se = Entry;
		return true;
	}
	return false;
}

bool CDriverWrapper::GetContactData(char *lpszMemoryType,CCardPanel &data,bool bStart)
{
	if(!m_bOpened)
		return false;

	//error code flag
	int iErr = 1;
	GSM_MemoryEntry Entry;
	//initialize the contract data
	memset(&Entry,0,sizeof(GSM_MemoryEntry));
	//check flag to get data
	if(bStart == true)
		iErr = GetPhoneBookStartDatafn(lpszMemoryType,&Entry);
	else
		iErr = GetPhoneBookNextDatafn(lpszMemoryType,&Entry,bStart);
	xErrorHandling(iErr);

	//if success ,do something
	if(iErr == 1){
		//convert Entry to CCArdPanel
		data = Entry;
		//set memory type to CCardPanel
		data.SetStorageType(lpszMemoryType);
		return true;
	}
	return false;
}

bool CDriverWrapper::xInitialLibrary()
{
	//get mobile_setting profile path
	Tstring sProfile(theApp.m_szModule);
	sProfile += _T("mobile_setting.ini");
	//get convert engine dll name
	TCHAR szDLL[MAX_PATH];
	// DLL CONTRUCT
	if(afxContextIsDLL)
	{
	  m_hDriver = ((CPhoneBookApp*)afxGetApp())->m_hInstAnwMobile;
	}
    else
	{
 	  if(al_GetSettingString(_T("lib"),_T("control_dll"),const_cast<TCHAR*>(sProfile.c_str()),szDLL))
	  {
		//load dll
		 m_hDriver = LoadLibrary(szDLL);
	  }
	  else
	  {
#ifdef _UNICODE
		 m_hDriver = LoadLibrary(_T("AnwMobileU.dll"));
#else
		 m_hDriver = LoadLibrary(_T("AnwMobile.dll"));
#endif
	  }
	}

	if (!m_hDriver) 
	{
		MessageBox(NULL,_T("Load AnwMobile.dll error"),_T("Error"),MB_ICONERROR|MB_OK);
		return false;
	}
	
	
	OpenPhoneBookfn = (OpenPhoneBookProc)GetProcAddress(m_hDriver,"OpenPhoneBook");
	if (OpenPhoneBookfn == NULL)
	{
		MessageBox(NULL,_T("Can't Get OpenPhoneBookfn()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hDriver);
		return false; 
	}
	
	ClosePhoneBookfn = (ClosePhoneBookProc)GetProcAddress(m_hDriver,"ClosePhoneBook");
	if (ClosePhoneBookfn == NULL)
	{
		MessageBox(NULL,_T("Can't Get ClosePhoneBookfn()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hDriver);
		return false; 
	}
	
	GetPhoneBookInfofn = (GetPhoneBookInfoProc)GetProcAddress(m_hDriver,"GetPhoneBookInfo");
	if (GetPhoneBookInfofn == NULL)
	{
		MessageBox(NULL,_T("Can't Get GetPhoneBookInfofn()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hDriver);
		return false; 
	}
	GetPhoneBookInfoExfn = (GetPhoneBookInfoExProc)GetProcAddress(m_hDriver,"GetPhoneBookInfoEx");
	
	GetPhoneBookStartDatafn = (GetPhoneBookStartDataProc)GetProcAddress(m_hDriver,"GetPhoneBookStartData");
	if (GetPhoneBookStartDatafn == NULL)
	{
		MessageBox(NULL,_T("Can't Get GetPhoneBookStartDatafn()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hDriver);
		return false; 
	}
	
	GetPhoneBookNextDatafn = (GetPhoneBookNextDataProc)GetProcAddress(m_hDriver,"GetPhoneBookNextData");
	if (GetPhoneBookNextDatafn == NULL)
	{
		MessageBox(NULL,_T("Can't Get GetPhoneBookNextDatafn()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hDriver);
		return false; 
	}
	
	DeletePhoneBookDatafn = (DeletePhoneBookDataProc)GetProcAddress(m_hDriver,"DeletePhoneBookData");
	if (DeletePhoneBookDatafn == NULL)
	{
		MessageBox(NULL,_T("Can't Get DeletePhoneBookDatafn()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hDriver);
		return false; 
	}
	
	GetGroupNamefn = (GetGroupNameProc)GetProcAddress(m_hDriver,"GetGroupName");
	if (GetGroupNamefn == NULL)
	{
		MessageBox(NULL,_T("Can't Get GetGroupNamefn()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hDriver);
		return false; 
	}
	
	GetGroupNumfn = (GetGroupNumProc)GetProcAddress(m_hDriver,"GetGroupNum");
	if (GetGroupNumfn == NULL)
	{
		MessageBox(NULL,_T("Can't Get GetGroupNumfn()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hDriver);
		return false; 
	}
	
	GetMobileNumfn = (GetMobileNumProc)GetProcAddress(m_hDriver,"GetMobileNum");
	if (GetMobileNumfn == NULL)
	{
		MessageBox(NULL,_T("Can't Get GetMobileNumfn()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hDriver);
		return false; 
	}
	
	GetMobileListfn = (GetMobileListProc)GetProcAddress(m_hDriver,"GetMobileList");
	if(GetMobileListfn == NULL)
	{
		MessageBox(NULL,_T("Can't Get GetMobileListfn()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hDriver);
		return false; 
	}
	
	AddPhoneBookDatafn = (AddPhoneBookDataProc)GetProcAddress(m_hDriver,"AddPhoneBookData");
	if(AddPhoneBookDatafn == NULL)
	{
		MessageBox(NULL,_T("Can't Get AddPhoneBookDatafn()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hDriver);
		return false; 
	}

	GetAllGroupfn = (GetAllGroupProc)GetProcAddress(m_hDriver,"GetAllGroup");
	if(GetAllGroupfn == NULL)
	{
		MessageBox(NULL,_T("Can't Get GetAllGroupfn()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hDriver);
		return false; 
	}
	IsSupportPhonebookDirectSMSfn = (IsSupportPhonebookDirectSMSProc)GetProcAddress(m_hDriver,"IsSupportPhonebookDirectSMS"); 
	if(IsSupportPhonebookDirectSMSfn == NULL)
	{
		MessageBox(NULL,_T("Can't Get IsSupportPhonebookDirectSMSfn()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hDriver);
		return false; 
	}
	GetPhoneBookSupportColumnnfn = (GetPhoneBookSupportColumnProc)GetProcAddress(m_hDriver,"GetPhoneBookSupportColumn"); 
	if(GetPhoneBookSupportColumnnfn == NULL)
	{
		MessageBox(NULL,_T("Can't Get GetPhoneBookSupportColumnnfn()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hDriver);
		return false; 
	}

	UpdatePhoneBookDatafn = (UpdatePhoneBookDataProc)GetProcAddress(m_hDriver,"UpdatePhoneBookData");
	if(UpdatePhoneBookDatafn == NULL)
	{
		MessageBox(NULL,_T("Can't Get UpdatePhoneBookDatafn()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hDriver);
		return false; 
	}

	InitPhoneBookfn = (InitPhoneBookProc)GetProcAddress(m_hDriver,"InitPhoneBook");
	if(InitPhoneBookfn == NULL)
	{
		MessageBox(NULL,_T("Can't Get InitPhoneBookfn()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hDriver);
		return false; 
	}

	TerminatePhoneBookfn = (TerminatePhoneBookProc)GetProcAddress(m_hDriver,"TerminatePhoneBook");
	if(TerminatePhoneBookfn == NULL)
	{
		MessageBox(NULL,_T("Can't Get TerminatePhoneBookfn()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hDriver);
		return false; 
	}

	InitialMemoryStatusfn = (InitialMemoryStatusProc)GetProcAddress(m_hDriver,"InitialMemoryStatus");
	if(InitialMemoryStatusfn == NULL)
	{
		MessageBox(NULL,_T("Can't Get InitialMemoryStatusfn()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hDriver);
		return false; 
	}

	ANWSendSMSData = (anwSendSMSData)::GetProcAddress(m_hDriver,"SendSMSData");
	if(ANWSendSMSData == NULL)
	{
		MessageBox(NULL,_T("Can't Get ANWSendSMSData()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hDriver);
		return false; 
	}



/*	GetPhonebookSupportfn = (GetPhoneBookSupportColumnProc)::GetProcAddress(m_hDriver,"GetPhoneBookSupportColumn");
	if(!GetPhonebookSupportfn)
	{
		MessageBox(NULL,"Can't Get GetPhonebookSupportfn()",_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hDriver);
		return false; 
	}*/
	return true;
	
}

void CDriverWrapper::FreeDriver()
{
   if(!afxContextIsDLL)
	::FreeLibrary(m_hDriver);
}

// DLL CONTRUCT
void CDriverWrapper::FreeLibrary(HMODULE m_hDriver)
{
   if(!afxContextIsDLL)
	   ::FreeLibrary(m_hDriver);
}


bool CDriverWrapper::init()
{
	if(!m_bInit){
		int iErr = InitPhoneBookfn();
		xErrorHandling(iErr);
		m_bInit = iErr == 1 ? true : false;
		return  m_bInit;
	}
	return true;
}

void CDriverWrapper::xErrorHandling(int iErr)
{
	if(iErr == Anw_SUCCESS || iErr == Anw_EMPTY)
		return;

	TCHAR szMsg[MAX_PATH*10];
	switch(iErr)
	{
	case Anw_TRIAL_LIMITEDFUNCTION:
		{
			TCHAR szText[MAX_PATH];
			al_GetSettingString(_T("public"),_T("IDS_TITLE"),theApp.m_szRes,szText);
			BusyMsgDlg(theApp.m_pMainWnd->m_hWnd,szText);
		}
		return ;
	case Anw_MOBILENOTRESPONSE:
		GetPrivateProfileString(_T("public"),_T("IDS_ERROR_MOBILENORESPOSE"),_T(""),szMsg,MAX_PATH*10,theApp.m_szRes);
		break;
	case Anw_MOBILE_BUSY:
		if(theApp.m_iMobileCompany == CO_SAMSUNG || theApp.m_iMobileCompany == CO_SHARP )
		{
			if(!al_GetSettingString(_T("public"),_T("IDS_ERR_OPERATIONFAIL_SAMSUNG"),theApp.m_szRes,szMsg))
				al_GetSettingString(_T("public"),_T("IDS_ERR_MOBILEBUSY"),theApp.m_szRes,szMsg);
		}
		else
			al_GetSettingString(_T("public"),_T("IDS_ERR_MOBILEBUSY"),theApp.m_szRes,szMsg);
		break;
	case Anw_TIMEOUT:
		al_GetSettingString(_T("public"),_T("IDS_ERR_TIMEOUT"),theApp.m_szRes,szMsg);
		break;
	case Anw_MOBILE_CONNECT_FAILED:
		al_GetSettingString(_T("public"),_T("IDS_ERR_CONNECTFAILED"),theApp.m_szRes,szMsg);
		break;
	case Anw_MOBILE_WRITE_FAILED:
		al_GetSettingString(_T("public"),_T("IDS_ERR_WRITEFAILED"),theApp.m_szRes,szMsg);
		break;
	case Anw_MOBILE_READ_FAILED:
		al_GetSettingString(_T("public"),_T("IDS_ERR_READFAILED"),theApp.m_szRes,szMsg);
		break;
	case Anw_MOBILE_DELETE_FAILED:
		al_GetSettingString(_T("public"),_T("IDS_ERR_DELFAILED"),theApp.m_szRes,szMsg);
		break;
	case Anw_ALLOCATE_MEMORY_FAILED:
		al_GetSettingString(_T("public"),_T("IDS_ERR_ALLOCATEMEMORY"),theApp.m_szRes,szMsg);
		break;
/*	case Anw_PARAMETER_CHECK_ERROR:
		al_GetSettingString(_T("public"),_T(""),theApp.m_szRes,szMsg);
		break;
	case Anw_MEMORY_RW_FAILED:
		al_GetSettingString(_T("public"),_T("IDS_ERR_MEMORYREADWRITEFAILED"),theApp.m_szRes,szMsg);
		break;
	case Anw_MOBILE_POWER_OFF:
		al_GetSettingString(_T("public"),_T("IDS_ERR_POWREOFF"),theApp.m_szRes,szMsg);
		break;
	case Anw_NOT_FOUNT_WINASPI:
		al_GetSettingString(_T("public"),_T("IDS_ERR_NOTFOUNDWINASPI"),theApp.m_szRes,szMsg);
		break;
	case Anw_NOT_FOUND_MOBILE:
		al_GetSettingString(_T("public"),_T("IDS_ERR_NOTFOUNDMOBILE"),theApp.m_szRes,szMsg);
		break;
	case Anw_NOT_LINK_DRV:
		al_GetSettingString(_T("public"),_T(""),theApp.m_szRes,szMsg);
		break;
	case Anw_EMPTY:
		al_GetSettingString(_T("public"),_T("IDS_ERR_EMPTY"),theApp.m_szRes,szMsg);
		break;*/
	case Anw_NOTSUPPORTED:
		al_GetSettingString(_T("public"),_T("IDS_ERR_NOTSUPPORTED"),theApp.m_szRes,szMsg);
		break;
	case Anw_CANTOPENFILE:
		al_GetSettingString(_T("public"),_T("IDS_ERR_CANTOPENFILE"),theApp.m_szRes,szMsg);
		break;
	case Anw_FILENOTSUPPORTED:
		al_GetSettingString(_T("public"),_T("IDS_ERR_FILENOTSUPPORTED"),theApp.m_szRes,szMsg);
		break;
	case Anw_INVALIDDATETIME:
		al_GetSettingString(_T("public"),_T("IDS_ERR_INVALIDDATETIME"),theApp.m_szRes,szMsg);
		break;
	case Anw_Phone_MEMORY:
		al_GetSettingString(_T("public"),_T("IDS_ERR_PHONE_MEMORY"),theApp.m_szRes,szMsg);
		break;
	case Anw_INVALIDDATA:
		al_GetSettingString(_T("public"),_T("IDS_ERR_INVALIDDATA"),theApp.m_szRes,szMsg);
		break;
	case Anw_NOT_LINK_MBdrv:
		al_GetSettingString(_T("public"),_T("IDS_ERR_NOTLINKDLL"),theApp.m_szRes,szMsg);
		break;
	case Anw_FileName_FAILED:
		al_GetSettingString(_T("public"),_T("IDS_ERR_FILENAMEFAILED"),theApp.m_szRes,szMsg);
		break;
	case Anw_Thread_Terminate:
		al_GetSettingString(_T("public"),_T("IDS_ERR_THREADTERMINATE"),theApp.m_szRes,szMsg);
		break;
	// v2.0.1.0 , temp solution for release , we need to modified in the future
	case 9999:
        if(al_GetSettingString(_T("public"),_T("IDS_ERR_UNKNOW_2"),theApp.m_szRes,szMsg) == false)
			al_GetSettingString(_T("public"),_T("IDS_ERR_UNKNOW"),theApp.m_szRes,szMsg);

		break;
	// mingfa--
	default:
		al_GetSettingString(_T("public"),_T("IDS_ERR_UNKNOW"),theApp.m_szRes,szMsg);
		break;
	}
	AfxMessageBox(szMsg);

}

bool CDriverWrapper::EndDeleteContactData(LPCSTR lpszStorageType)
{
	if(!m_bOpened)
		return false;
	int iErr = DeletePhoneBookDatafn(const_cast<char*>(lpszStorageType),"0",Type_End);
//	xErrorHandling(iErr);
	return  iErr == 1 ? true : false;

}

bool CDriverWrapper::InitialMemory(LPCSTR lpszType, int &iUsed, int &iFree)
{
	if(!m_bOpened)
		return false;
	int iErr = InitialMemoryStatusfn(const_cast<char*>(lpszType),&iUsed,&iFree);
	xErrorHandling(iErr);
	return  iErr == 1 ? true : false;
}

bool CDriverWrapper::EndAddContactData(LPCSTR lpszStorageType)
{
	if(!m_bOpened)
		return false;
	int iErr = AddPhoneBookDatafn(const_cast<char*>(lpszStorageType),0,Type_End);
//	xErrorHandling(iErr);
	return  iErr == 1 ? true : false;

}

void CDriverWrapper::xCheckDataType(CCardPanel &data)
{
	bool bRet = false,bSE = false;
	if(theApp.m_iMobileCompany == CO_SONY_ERISON && (_tcscmp(theApp.m_szPhone,_T("T68"))== 0 ||  _tcscmp(theApp.m_szPhone,_T("T39"))== 0))
		bSE = true;
	TCHAR szName[MAX_PATH];
	deque<CPhoneData> *pdeqData = data.GetPhoneData();
	for(deque<CPhoneData>::iterator iter = pdeqData->begin(); iter != pdeqData->end() ; iter ++){
		if((*iter).GetType() == PBK_Number_General){
			if(theApp.m_iMobileCompany == CO_SONY_ERISON ||theApp.m_iMobileCompany == CO_LG||theApp.m_iMobileCompany == CO_MTK || 
				(theApp.m_iMobileCompany == CO_ASUS &&(_tcscmp(theApp.m_szPhone,_T("J202")) == 0)))	//convert general to mobile
				(*iter).SetType(PBK_Number_Mobile);
			else if(theApp.m_iMobileCompany == CO_SIMENS)	//convert general to home
				(*iter).SetType(PBK_Number_Home);
		}

		if(bSE){
			if((*iter).GetType() == PBK_Text_FirstName || (*iter).GetType() == PBK_Text_LastName){
				bRet = true;
			}
			else if((*iter).GetType() == PBK_Text_Name){
				_tcscpy(szName,(*iter).GetText());	
			}
		}
	}

	if(bSE && !bRet){
		for(deque<CPhoneData>::iterator iter = pdeqData->begin(); iter != pdeqData->end() ; iter ++){
			if((*iter).GetType() == PBK_Text_Name){
				(*iter).SetType(PBK_Text_FirstName);
			}
		}

	}
}
int CDriverWrapper::SendSMSData(int MemType, int SMSfolderType, SMS_Data_Strc  * SMS_Data_Strc)
{
	if(!m_bOpened)
		return false;
	int iErr = ANWSendSMSData(MemType,  SMSfolderType, SMS_Data_Strc);
	xErrorHandling(iErr);
	return  iErr == 1 ? true : false;

}

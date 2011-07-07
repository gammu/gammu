
//implementation file

#include "stdafx.h"
#include "CAMMSync.h"
#include "ConvertEngine.h"
#include "..\loadstring.h"
#include "..\MainFrm.h"
#include "CommUIExportFun.h"

//
#include "ConvertEngine.h"
Calendar_Data_Struct CalendarDataStAnW;//Temporary, nono[2004_1029]
void DumpMobileCalendar(Calendar_Data_Struct CalendarDataFromMobile);//Implement in the below.
CString TranslateErrorCode(int nErrorCode);

static Calendar_Data_Struct _Calendar_Data_Strc_;
static bool _static_bMobileConneted = false;
static	CalendarSupportInfo _static_SupportInfo;
//
anwOpenCalendar				ANWOpenCalendar;
anwCloseCalendar			ANWCloseCalendar;
anwGetMobileScheduleStrc	ANWGetMobileScheduleStrc;
anwGetMobileScheduleInfo	ANWGetMobileScheduleInfo;
anwGetScheduleStartData		ANWGetScheduleStartData;
anwGetScheduleNextData		ANWGetScheduleNextData;
anwAddScheduleData			ANWAddScheduleData;
anwDeleteScheduleData		ANWDeleteScheduleData;
anwDeleteAllScheduleData	ANWDeleteAllScheduleData;
anwInitCalendar             ANWInitCalendar;
anwGetCalendarSupport       ANWGetCalendarSupport;
anwGetMobileDateTime		ANWGetMobileDateTime;
anwInitialCalendarStatus		ANWInitialCalendarStatus;

//


/*
CString GuidToString(GUID guid)
{
	CString string;
	string.Format("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,",
				guid.Data1,
				guid.Data2,
				guid.Data3,
				guid.Data4[0],
				guid.Data4[1],
				guid.Data4[2],
				guid.Data4[3],
				guid.Data4[4],
				guid.Data4[5],
				guid.Data4[6],
				guid.Data4[7]);
	return string;
}

void StringToGuid(CString szStr, GUID& inputGUID)
{
	GUID guid;
	char buf[MAX_PATH];
	lstrcpy(buf,szStr.GetBuffer(MAX_PATH));
	szStr.ReleaseBuffer();
	sscanf(buf,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,",
				&guid.Data1,
				&guid.Data2,
				&guid.Data3,
				&guid.Data4[0],
				&guid.Data4[1],
				&guid.Data4[2],
				&guid.Data4[3],
				&guid.Data4[4],
				&guid.Data4[5],
				&guid.Data4[6],
				&guid.Data4[7]);

	memcpy(&inputGUID, &guid, sizeof(GUID));
	return;

}
*/

CCAMMSync::CCAMMSync()
{
//	xLoadLibrary();

	m_bOpened = false;
    m_hMODLL = NULL;
//	m_hWnd = this->m_hWnd;


	//LoadandOpenCalendarDriver();//nono, 2004_1101

}

CCAMMSync::~CCAMMSync()
{
//	xFreeLibrary();

//	ANWCloseCalendar();
//	FreeCalendarDriver();//nono, 2004_1101

}

/*
BOOL CCAMMSync::xLoadLibrary()
{

	TCHAR path[MAX_PATH+1];
	CString strPath;
	memset(path,0x00,sizeof(TCHAR)*(MAX_PATH+1));

	DWORD type =0;
	DWORD size=MAX_PATH;
	BOOL bKeyExist = FALSE;
	HKEY hKey;
	if(::RegOpenKeyEx(HKEY_LOCAL_MACHINE,gstrRegistryPath,NULL,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS)
	{
		if(::RegQueryValueEx(hKey,LPCTSTR("InstallDir"),NULL,&type,(LPBYTE)path,&size) == ERROR_SUCCESS)
		{
			bKeyExist = TRUE;
		}
		::RegCloseKey(hKey);
	}

	if(bKeyExist)
	{
		strPath.Format(path);
	}
	else
	{
	}
	strPath+="\\";
	strPath += OKWAPMO;


	m_hMODLL = ::LoadLibrary(strPath);
	if( m_hMODLL!=NULL )
	{
		MoOpenMyOrganizer = (_MoOpenMyOrganizer) ::GetProcAddress(m_hMODLL, "MoOpenMyOrganizer");
        if(!MoOpenMyOrganizer) {
            xFreeLibrary();
            return FALSE;
		}

		MoCloseMyOrganizer = (_MoCloseMyOrganizer) ::GetProcAddress(m_hMODLL, "MoCloseMyOrganizer");
        if(!MoCloseMyOrganizer) {
            xFreeLibrary();
            return FALSE;
		}

		MoOpenFile = (_MoOpenFile) ::GetProcAddress(m_hMODLL, "MoOpenFile");
        if(!MoOpenFile) {
            xFreeLibrary();
            return FALSE;
		}

		MoCloseFile = (_MoCloseFile) ::GetProcAddress(m_hMODLL, "MoCloseFile");
        if(!MoCloseFile) {
            xFreeLibrary();
            return FALSE;
		}

		MoOpenDatabase = (_MoOpenDatabase) ::GetProcAddress(m_hMODLL, "MoOpenDatabase");
        if(!MoOpenDatabase) {
            xFreeLibrary();
            return FALSE;
		}

		MoCloseDatabase = (_MoCloseDatabase) ::GetProcAddress(m_hMODLL, "MoCloseDatabase");
        if(!MoCloseDatabase) {
            xFreeLibrary();
            return FALSE;
		}

		MoCreateRecord = (_MoCreateRecord) ::GetProcAddress(m_hMODLL, "MoCreateRecord");
        if(!MoCreateRecord) {
            xFreeLibrary();
            return FALSE;
		}

		MoGetRecordFieldNameByID = (_MoGetRecordFieldNameByID) ::GetProcAddress(m_hMODLL, "MoGetRecordFieldNameByID");
        if(!MoGetRecordFieldNameByID) {
            xFreeLibrary();
            return FALSE;
		}

		MoSetRecordField = (_MoSetRecordField) ::GetProcAddress(m_hMODLL, "MoSetRecordField");
        if(!MoSetRecordField) {
            xFreeLibrary();
            return FALSE;
		}
	
		MoGetRecordField = (_MoGetRecordField) ::GetProcAddress(m_hMODLL, "MoGetRecordField");
        if(!MoGetRecordField) {
            xFreeLibrary();
            return FALSE;
		}
	
		MoCloseRecord = (_MoCloseRecord) ::GetProcAddress(m_hMODLL, "MoCloseRecord");
        if(!MoCloseRecord) {
            xFreeLibrary();
            return FALSE;
		}
		MoGetRecordCount = (_MoGetRecordCount) ::GetProcAddress(m_hMODLL, "MoGetRecordCount");
        if(!MoGetRecordCount) {
            xFreeLibrary();
            return FALSE;
		}
		pfnMoSetGeneralRecordRecurInfo = (_MoSetGeneralRecordRecurInfo) ::GetProcAddress(m_hMODLL, "MoSetGeneralRecordRecurInfo");
        if(!pfnMoSetGeneralRecordRecurInfo) {
            xFreeLibrary();
            return FALSE;
		}
		pfnMoGetGeneralRecordRecurInfo = (_MoGetGeneralRecordRecurInfo) ::GetProcAddress(m_hMODLL, "MoGetGeneralRecordRecurInfo");
        if(!pfnMoGetGeneralRecordRecurInfo) {
            xFreeLibrary();
            return FALSE;
		}


	}

	return FALSE;
}


void CCAMMSync::xFreeLibrary()
{

    if (m_hMODLL)
	{	::FreeLibrary(m_hMODLL);
		m_hMODLL = NULL;
	}

}
*/
//////////////////// Begin Contacts //////////////////////////////////////////////////////////////
/*CAMMError CCAMMSync::IsValidOrgnizerFile(CString dbPath)
{
	if (dbPath.IsEmpty()) {
		return CAMM_ERROR_INVALIDARG;
	}
	if (xCheckFileExisted(dbPath) != CAMM_ERROR_SUCCESS) {
		return CAMM_ERROR_INVALIDARG;
	}

	long lRet=0L;
	CAMMError ce=CAMM_ERROR_SUCCESS;
	CString szProductPath;
	xGetProductPath(szProductPath,dbPath);

	
/*	lRet =	MoOpenMyOrganizer(TRUE, szProductPath);
	lRet =	MoOpenFile(dbPath);

	if(lRet == FALSE)
	{
		ce = CAMM_ERROR_FAILURE;
	}

	lRet = 	MoCloseFile(FALSE);

	lRet = MoCloseMyOrganizer();
*/
/*	


	return ce;
}*/
/*
CAMMError CCAMMSync::GetRecordCount(int nType, CString csDBPath,int &recordCount)
{
	if (csDBPath.IsEmpty()) {
		return CAMM_ERROR_INVALIDARG;
	}
	if (xCheckFileExisted(csDBPath) != CAMM_ERROR_SUCCESS) {
		recordCount = 0;
		return CAMM_ERROR_SUCCESS;
	}

	CString szProductPath;
	xGetProductPath(szProductPath,csDBPath);
	return xGetRecordCount(nType,szProductPath,csDBPath,recordCount);
}

CAMMError CCAMMSync::xGetRecordCount(int nType,CString szProductPath, CString csDBPath,int &recordCount)
{
	long lRet = 0L;
	
/*	lRet =	MoOpenMyOrganizer(TRUE, szProductPath);
	lRet =	MoOpenFile(csDBPath);
	lRet =	MoOpenDatabase(nType);
	
	DWORD recCount = 0;
	lRet =	MoGetRecordCount(recCount);
	recordCount = recCount;
	
	lRet = MoCloseDatabase();

	lRet = 	MoCloseFile(FALSE);

	lRet = MoCloseMyOrganizer();
*/
/*	

	return CAMM_ERROR_SUCCESS;
}*/
/*
CAMMError CCAMMSync::LoadContact(ContactsRecord record[],CString csDBPath, int &nSize)
{
	if (csDBPath.IsEmpty()) {
		return CAMM_ERROR_INVALIDARG;
	}

	if (xCheckFileExisted(csDBPath) != CAMM_ERROR_SUCCESS) {
		return CAMM_ERROR_INVALIDFILE;
	}

	CString szProductPath;
	xGetProductPath(szProductPath,csDBPath);

	CAMMError ret = xLoadContact(record,szProductPath,csDBPath,nSize);

	return ret;
}

CAMMError CCAMMSync::xLoadContact(ContactsRecord record[],CString szProductPath,CString csDBPath, int &nSize)
{
	long lRet = 0L;

	
/*	lRet =	MoOpenMyOrganizer(TRUE, szProductPath);
	
	lRet =	MoOpenFile(csDBPath);
	lRet =	MoOpenDatabase(MO_ADR);
	
	TCHAR lpBuffer[512];
	DWORD dwLength=0;
	
	ContactsRecordWrapper crw;
	CString storage[SupportFieldNum];
	DWORD recCount = 0;
	CString csPCID ; 
	GUID pcid = IID_NONE;

	lRet =	MoGetRecordCount(recCount);

	nSize = recCount;
	for(int x=0; x< nSize;x++)
	{	
		memset(lpBuffer,0x00,sizeof(TCHAR)*512);
		sprintf(lpBuffer,"%d",x+1);
		long l = MoCreateRecord(MO_OPEN_EXISTING_BY_INDEX, lpBuffer, 512, dwLength);

		crw.Init();
		
		TCHAR MoStrBuffer[512];
		DWORD uBytes=0;
		TCHAR FieldBuffer[FIELD_BUFFER_LEN];
		DWORD dwBytesWritten=0;
		int n;
		for(n=0; n<SupportFieldNum; n++)
		{
			
			dwBytesWritten = 0;
			memset(FieldBuffer,0x00,sizeof(TCHAR)*FIELD_BUFFER_LEN);
			lRet = MoGetRecordFieldNameByID(MO_ADR, crw.GetOutputFieldId(n), MoStrBuffer, 512, uBytes);
					
			lRet = MoGetRecordField(MoStrBuffer, FieldBuffer,FIELD_BUFFER_LEN,dwBytesWritten);
			if(n==0)
			{
				csPCID.Format("%s",FieldBuffer);
			}
			else
			{
				storage[n-1].Format("%s",FieldBuffer);
			}
		}
		StringToGuid(csPCID,pcid);
		record[x].SetPCID(pcid); 
		record[x].SetRecord(storage[1],storage[0],storage[10],storage[2],storage[5],storage[6],storage[7],storage[8],storage[9],storage[3],storage[4],storage[12]);
		record[x].SetSound(storage[11]);

		lRet = MoCloseRecord(FALSE, lpBuffer);
	
	}
	//End of Loop
	
	lRet = MoCloseDatabase();

	lRet = 	MoCloseFile(FALSE);

	lRet = MoCloseMyOrganizer();
*/	
/*
	
	return CAMM_ERROR_SUCCESS;
}*/
/*
CAMMError CCAMMSync::SaveContact(ContactsRecord record[],CString csDBPath,int nSize)
{
	if (csDBPath.IsEmpty()) {
		return CAMM_ERROR_INVALIDARG;
	}

	if (xCheckFileExisted(csDBPath) == CAMM_ERROR_INVALIDFILE) {
		if (xCreateEmptyNewFile(csDBPath) != CAMM_ERROR_SUCCESS) {
			AfxMessageBox("Can't save");
			return CAMM_ERROR_FAILURE;
		}
	}
	CString szProductPath;
	xGetProductPath(szProductPath,csDBPath);
/*
	CString TempSavePath = szProductPath + "temp.iog";
	if (CopyFile(csDBPath, TempSavePath, FALSE)) {
        DWORD dwAttrs = GetFileAttributes(TempSavePath); 
        SetFileAttributes(TempSavePath,dwAttrs | FILE_ATTRIBUTE_NORMAL); 
    } 
    else 
    { 
        return CAMM_ERROR_FAILURE;
    } 
	
	xRemoveOrganizer(MO_ADR,szProductPath,csDBPath);
*/
//	CAMMError ret = xSaveContact(record,szProductPath,csDBPath,nSize);
/*
	if (ret != CAMM_ERROR_SUCCESS) {
		CopyFile(TempSavePath, csDBPath, FALSE);
	}
	try{
		CFile::Remove(TempSavePath);
	}
	catch(...){}
*/	
/*	return ret;
}

CAMMError CCAMMSync::xSaveContact(ContactsRecord record[],CString szProductPath,CString csDBPath,int nSize)
{

	long lRet = 0;
	
/*	lRet =	MoOpenMyOrganizer(TRUE, szProductPath);
	
	lRet =	MoOpenFile(csDBPath);
	lRet =	MoOpenDatabase(MO_ADR);
	
	TCHAR lpBuffer[512];
	DWORD dwLength=0;
	memset(lpBuffer,0x00,sizeof(TCHAR)*512);
	
	ContactsRecordWrapper crw;
	
	for(int x=0; x< nSize;x++)
	{
		memset(lpBuffer,0x00,sizeof(TCHAR)*512);
		long l = MoCreateRecord(MO_NEW_CREATE, lpBuffer, 512, dwLength);

		crw.Init();
	
        crw.PrepareRecord(record[x]);
		int MBLen;
		TCHAR MoStrBuffer[512];
		DWORD uBytes=0;
		TCHAR WriteFieldBuffer[FIELD_BUFFER_LEN];
		int n;
		for(n=SupportFieldNum-1; n>=0; n--)
		{
			//			MBLen = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK|WC_SEPCHARS, 
			//				crw.GetInputField(n), crw.GetInputFieldLen(n), WriteFieldBuffer, 
			//				FIELD_BUFFER_LEN, NULL, NULL);
			if(crw.GetInputField(n) != NULL)
			{
				MBLen = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK|WC_SEPCHARS, 
					crw.GetInputField(n), -1, WriteFieldBuffer, 
					FIELD_BUFFER_LEN, NULL, NULL);
				
				lRet = MoGetRecordFieldNameByID(MO_ADR, crw.GetOutputFieldId(n), MoStrBuffer, 512, uBytes);
		
				lRet = MoSetRecordField(MoStrBuffer, WriteFieldBuffer);
				
				memset(WriteFieldBuffer,0x00,sizeof(TCHAR)*FIELD_BUFFER_LEN);
			}
		}
		lRet = MoCloseRecord(TRUE, lpBuffer);
	
	}
	//End of Loop

	lRet = MoCloseDatabase();

	lRet = 	MoCloseFile(TRUE);

	lRet = MoCloseMyOrganizer();
*/	
/*	

	return CAMM_ERROR_SUCCESS;

}*/
//////////////////// End of Contacts //////////////////////////////////////////////////////////////


//////////////////// Begin of Notes //////////////////////////////////////////////////////////////
/*CAMMError CCAMMSync::LoadNotes(NotesRecord record[],CString csDBPath, int &nSize)
{
	if (csDBPath.IsEmpty()) {
		return CAMM_ERROR_INVALIDARG;
	}

	if (xCheckFileExisted(csDBPath) != CAMM_ERROR_SUCCESS) {
		return CAMM_ERROR_INVALIDFILE;
	}

	CString szProductPath;
	xGetProductPath(szProductPath,csDBPath);

	CAMMError ret = xLoadNotes(record,szProductPath,csDBPath,nSize);

	return ret;
}

CAMMError CCAMMSync::xLoadNotes(NotesRecord record[],CString szProductPath,CString csDBPath,int &nSize)
{
	
	long lRet = 0L;

	
/*	lRet =	MoOpenMyOrganizer(TRUE, szProductPath);
	lRet =	MoOpenFile(csDBPath);
	lRet =	MoOpenDatabase(MO_NOTE);
	
	TCHAR lpBuffer[512];
	DWORD dwLength=0;
	
	NotesRecordWrapper crw;

	CString storage[SupportFieldNumNotes];
	DWORD recCount = 0;
	CString csPCID ; 
	GUID pcid = IID_NONE;
	
	lRet =	MoGetRecordCount(recCount);

	nSize = recCount;
	for(int x=0; x< recCount;x++)
	{
		memset(lpBuffer,0x00,sizeof(TCHAR)*512);
		sprintf(lpBuffer,"%d",x+1);
	
		long l = MoCreateRecord(MO_OPEN_EXISTING_BY_INDEX, lpBuffer, 512, dwLength);

		crw.Init();
		
		TCHAR MoStrBuffer[512];
		DWORD uBytes=0;
		TCHAR FieldBuffer[FIELD_BUFFER_LEN];
		DWORD dwBytesWritten=0;
		int n;
		for(n=0; n<SupportFieldNumNotes; n++)
		{
			
			dwBytesWritten = 0;
			memset(FieldBuffer,0x00,sizeof(TCHAR)*FIELD_BUFFER_LEN);
			lRet = MoGetRecordFieldNameByID(MO_NOTE, crw.GetOutputFieldId(n), MoStrBuffer, 512, uBytes);

			lRet = MoGetRecordField(MoStrBuffer, FieldBuffer,FIELD_BUFFER_LEN,dwBytesWritten);
					
			if(n==0)
			{
				csPCID.Format("%s",FieldBuffer);
			}
			else
			{
				storage[n-1].Format("%s",FieldBuffer);
			}
		}
		StringToGuid(csPCID,pcid);
		record[x].SetPCID(pcid); 
		record[x].SetRecord(storage[0],storage[1]);

		lRet = MoCloseRecord(FALSE, lpBuffer);
	
	}
	//End of Loop
	
	lRet = MoCloseDatabase();

	lRet = 	MoCloseFile(FALSE);

	lRet = MoCloseMyOrganizer();
*/	
/*

	return CAMM_ERROR_SUCCESS;
}
CAMMError CCAMMSync::SaveNotes(NotesRecord record[],CString csDBPath,int nSize)
{
	if (csDBPath.IsEmpty()) {
		return CAMM_ERROR_INVALIDARG;
	}
	if (xCheckFileExisted(csDBPath) == CAMM_ERROR_INVALIDFILE) {
		if (xCreateEmptyNewFile(csDBPath) != CAMM_ERROR_SUCCESS) {
			AfxMessageBox("Can't save");
			return CAMM_ERROR_FAILURE;
		}
	}
	CString szProductPath;
	xGetProductPath(szProductPath,csDBPath);
/*
	CString TempSavePath = szProductPath + "temp.iog";
	if (CopyFile(csDBPath, TempSavePath, FALSE)) {
        DWORD dwAttrs = GetFileAttributes(TempSavePath); 
        SetFileAttributes(TempSavePath,dwAttrs | FILE_ATTRIBUTE_NORMAL); 
    } 
    else 
    { 
        return CAMM_ERROR_FAILURE;
    } 
	
	xRemoveOrganizer(MO_NOTE,szProductPath,csDBPath);
*/
//	CAMMError ret = xSaveNotes(record,szProductPath,csDBPath,nSize);
/*
	if (ret != CAMM_ERROR_SUCCESS) {
		CopyFile(TempSavePath, csDBPath, FALSE);
	}
	try{
		CFile::Remove(TempSavePath);
	}
	catch(...){}
*/
/*	return ret;
}

CAMMError CCAMMSync::xSaveNotes(NotesRecord record[],CString szProductPath,CString csDBPath,int nSize)
{

	long lRet = 0;
	
/*	lRet =	MoOpenMyOrganizer(TRUE, szProductPath);
	lRet =	MoOpenFile(csDBPath);
	lRet =	MoOpenDatabase(MO_NOTE);
	
	TCHAR lpBuffer[512];
	DWORD dwLength=0;
	memset(lpBuffer,0x00,sizeof(TCHAR)*512);
	
	NotesRecordWrapper crw;
	
	for(int x=0; x< nSize;x++)
	{
		memset(lpBuffer,0x00,sizeof(TCHAR)*512);

		long l = MoCreateRecord(MO_NEW_CREATE, lpBuffer, 512, dwLength);

		crw.Init();
		crw.PrepareRecord(record[x]);
		int MBLen;
		TCHAR MoStrBuffer[512];
		DWORD uBytes=0;
		TCHAR WriteFieldBuffer[FIELD_BUFFER_LEN];
		int n;
		for(n=SupportFieldNumNotes-1; n>=0; n--)
		{
			//			MBLen = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK|WC_SEPCHARS, 
			//				crw.GetInputField(n), crw.GetInputFieldLen(n), WriteFieldBuffer, 
			//				FIELD_BUFFER_LEN, NULL, NULL);
			if(crw.GetInputField(n) != NULL)
			{
				MBLen = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK|WC_SEPCHARS, 
					crw.GetInputField(n), -1, WriteFieldBuffer, 
					FIELD_BUFFER_LEN, NULL, NULL);
				
				lRet = MoGetRecordFieldNameByID(MO_NOTE, crw.GetOutputFieldId(n), MoStrBuffer, 512, uBytes);

				lRet = MoSetRecordField(MoStrBuffer, WriteFieldBuffer);

				memset(WriteFieldBuffer,0x00,sizeof(TCHAR)*FIELD_BUFFER_LEN);
			}
		}

		lRet = MoCloseRecord(TRUE, lpBuffer);
	
	}
	//End of Loop

	lRet = MoCloseDatabase();

	lRet = 	MoCloseFile(TRUE);

	lRet = MoCloseMyOrganizer();
*/	
	
/*	return CAMM_ERROR_SUCCESS;

}*/


//////////////////// End of Notes //////////////////////////////////////////////////////////////


//////////////////// Begin of Schedule //////////////////////////////////////////////////////////////

/////CAMMagic Mobile 2.0
int CCAMMSync::GetCalendarCount()
{
	int nCalendarCount=0;
	Schedule_Tal_Num tal_num;
	tal_num.ScheduleUsedNum = 0;

	int nRet = ANWGetMobileScheduleInfo(&tal_num);
	if(tal_num.ScheduleUsedNum == -1)
	{
		nRet =ANWInitialCalendarStatus(&tal_num);
	}
	xErrorHandling(nRet);
	if(nRet != Anw_SUCCESS)
		tal_num.ScheduleUsedNum = -1;
//	if (nRet == Anw_SUCCESS) 
//	{
 //   	g_numTotalCalendars = tal_num.ScheduleUsedNum;
 //       ::SendMessage(afxGetMainWnd()->GetSafeHwnd(), WM_REFRESH_LEFT_PANEL, (DWORD)true, NULL);
//	}

	return tal_num.ScheduleUsedNum;
}
/*
CAMMError CCAMMSync::LoadCalendarfromMobile(SchedulesRecord record[], int nRecordCount)
{
	CAMMError ret;
	if (!record || !nRecordCount) {
		//nErrorCode = 2;
		return CAMM_ERROR_INVALIDARG;
	}

	//Call private function
	ret = xLoadCalendarfromMobile(record,nRecordCount);

	return ret;
}


CAMMError CCAMMSync::xLoadCalendarfromMobile(SchedulesRecord record[], int nRecordCount)
{
	CAMMError ret=CAMM_ERROR_SUCCESS;
	int nErrorCode=0;

	


	//Double check the size of Calendar_Data_Struct
	Schedule_Tal_Num tal_num;
	int nRet = ANWGetMobileScheduleInfo(&tal_num);
	if (Anw_SUCCESS!=nRet||tal_num.ScheduleUsedNum != nRecordCount) {
		if (Anw_MOBILE_CONNECT_FAILED == nRet) {
			_static_bMobileConneted = false;
			AfxMessageBox(TranslateErrorCode(nRet));
       		PostMessage(afxGetMainWnd()->GetSafeHwnd(), WM_MOBILECONNECTED, (DWORD)false, NULL);
		}
		//nErrorCode = nRet;
		return CAMM_ERROR_INVALIDFILE;
	}

	//Set the limitation of total number of Calendar.
	CalendarSupportInfo SupportInfo;
    if (Anw_SUCCESS == ANWGetCalendarSupport(&SupportInfo) ) {
     	g_nMAX_SCHEDULE_COUNT = SupportInfo.TotalCalendar;
	}
	
	int RealCount=0, NeedCount=1;
	//initialize Calendar_Data_Struct
	memset(&_Calendar_Data_Strc_, 0, sizeof(Calendar_Data_Struct));

	nRet = ANWGetScheduleStartData(NeedCount, &_Calendar_Data_Strc_, RealCount);
	if (Anw_SUCCESS==nRet) {
		//Call converter
    	ret = ConvertMoblieDataToPC(_Calendar_Data_Strc_, &(record[0]), nErrorCode);
		if (ret != CAMM_ERROR_SUCCESS) {
			;
		}
#ifdef _DEBUG
       	DumpMobileCalendar(_Calendar_Data_Strc_);
#endif
	}else {
		AfxMessageBox(TranslateErrorCode(nRet));
		if (Anw_MOBILE_CONNECT_FAILED == nRet) {
			_static_bMobileConneted = false;
       		PostMessage(afxGetMainWnd()->GetSafeHwnd(), WM_MOBILECONNECTED, (DWORD)false, NULL);
		}
		nErrorCode = nRet;
		return CAMM_ERROR_INVALIDFILE;
	}

	//Access the other Calendar_Data_Struct.
	for( int i = 0; i < tal_num.ScheduleUsedNum -1; i++ )
	{
		RealCount=0;
    	//initialize Calendar_Data_Struct
    	memset(&_Calendar_Data_Strc_, 0, sizeof(Calendar_Data_Struct));
		nRet = ANWGetScheduleNextData(NeedCount, &_Calendar_Data_Strc_, RealCount);
		if (Anw_SUCCESS==nRet) {
	     	//Call converter
        	ret = ConvertMoblieDataToPC(_Calendar_Data_Strc_, &(record[i+1]), nErrorCode);
	    	if (ret != CAMM_ERROR_SUCCESS) {
			}
#ifdef _DEBUG
         	DumpMobileCalendar(_Calendar_Data_Strc_);
#endif
		}else {
			AfxMessageBox(TranslateErrorCode(nRet));
	    	if (Anw_MOBILE_CONNECT_FAILED == nRet) {
	    		_static_bMobileConneted = false;
         		PostMessage(afxGetMainWnd()->GetSafeHwnd(), WM_MOBILECONNECTED, (DWORD)false, NULL);
			}
            nErrorCode = nRet;
            return CAMM_ERROR_INVALIDFILE;
		}
	}
	
	
	
	return ret;
}
*/
int CCAMMSync::WrapperGSMDateTime(COleDateTimeEx dtStartDate, GSM_DateTime& gsmDateTime)
{
	int nRet=1;

	gsmDateTime.Year  = (unsigned int) dtStartDate.GetYear();
	gsmDateTime.Month = (unsigned int) dtStartDate.GetMonth();
	gsmDateTime.Day   = (unsigned int) dtStartDate.GetDay();
	gsmDateTime.Hour  = (unsigned int) dtStartDate.GetHour();
	gsmDateTime.Minute   = (unsigned int) dtStartDate.GetMinute();
	gsmDateTime.Second   = (unsigned int) dtStartDate.GetSecond();

	return nRet;
}

int CCAMMSync::WrapperGSMDateTime(COleDateTimeEx dtStartDate, CScheduleTimeInfo st, GSM_DateTime& gsmDateTime)
{
	int nRet=1;
	int nHour=0, nMin=0;
	st.Get24HourTime(nHour, nMin);


	
	

	gsmDateTime.Year  = (unsigned int) dtStartDate.GetYear();
	gsmDateTime.Month = (unsigned int) dtStartDate.GetMonth();
	gsmDateTime.Day   = (unsigned int) dtStartDate.GetDay();
	gsmDateTime.Hour  = (unsigned int) nHour;//dtStartDate.GetHour();
	gsmDateTime.Minute   = (unsigned int) nMin;//dtStartDate.GetMinute();
	gsmDateTime.Second   = (unsigned int) dtStartDate.GetSecond();

	return nRet;
}
bool ParseGSMDateTime(GSM_DateTime gsmDateTime, CString& cuoutputString)
{
    //record.starttime format:  "10/20/2004 08:00"
	cuoutputString.Empty();

	int nLen=0;
	char cBuffer[12];

	//Month
	itoa(gsmDateTime.Month ,cBuffer,10);
	nLen = strlen(cBuffer);
	if (2>nLen) {
    	cuoutputString += "0";
	}
   	cuoutputString += cBuffer;
	cuoutputString += "/";

	//Day
	itoa(gsmDateTime.Day ,cBuffer,10);
	nLen = strlen(cBuffer);
    if (2>nLen) {
    	cuoutputString += "0";
	}
   	cuoutputString += cBuffer;
	cuoutputString += "/";

	//Year
	itoa(gsmDateTime.Year ,cBuffer,10);
	nLen = strlen(cBuffer);
    if (4>nLen) {
    	cuoutputString += "0";
	}
   	cuoutputString += cBuffer;
	cuoutputString += " ";

	//Hour
	itoa(gsmDateTime.Hour ,cBuffer,10);
	nLen = strlen(cBuffer);
	if (2>nLen) {
    	cuoutputString += "0";
	}
    cuoutputString += cBuffer;

	//Seperator
	cuoutputString += ":";

	//Minute
	itoa(gsmDateTime.Minute ,cBuffer,10);
	nLen = strlen(cBuffer);
	if (2>nLen) {
    	cuoutputString += "0";
	}
   	cuoutputString += cBuffer;



	CString temp;
	temp = (LPCTSTR) cuoutputString;
	temp.ReleaseBuffer();
/*
typedef struct {
	// The difference between local time and GMT in hours
	int			Timezone;

	unsigned int		Second;
	unsigned int 		Minute;
	unsigned int		Hour;

	unsigned int 		Day;

	// January = 1, February = 2, etc.
	unsigned int 		Month;

	// Complete year number. Not 03, but 2003
	unsigned int		Year;
} GSM_DateTime;
*/
	return true;
}

int DecodeRepeatType(int nRecurrence, int& nRepeatFrequency) 
{//typedef enum {DAILY=0, WEEKLY=1, TWOWEEKLY, MONTHLY, YEARLY, WEEKLYMONTHLY} RepeatType;

	RepeatType nRepeatType;

	if (!nRecurrence) return NOREPEAT;

	if (24 == nRecurrence) {
		nRepeatType = DAILY;
		nRepeatFrequency = 1;
	}else if (168 == nRecurrence) {
		nRepeatType = WEEKLY;
		nRepeatFrequency = 1;
	}else if (336 == nRecurrence) {
		nRepeatType = TWOWEEKLY;
		nRepeatFrequency = 2;
	}else if (65534 == nRecurrence) {
		nRepeatType = MONTHLY;
		nRepeatFrequency = 1;
	}else if (8760 == nRecurrence) {
		nRepeatType = YEARLY;
		nRepeatFrequency = 1;
	}

	return nRepeatType;
}

int EncodeRepeatType(int nRepeatType, int nRepeatFrequency) 
{//typedef enum {NOREPEAT=-1, DAILY=0, WEEKLY=1, TWOWEEKLY, MONTHLY, YEARLY, WEEKLYMONTHLY} RepeatType;


	int nRecurrence=0;
	if (0 > nRepeatType) return nRecurrence;


	if (DAILY == nRepeatType) {
		nRecurrence = 24;
	}else if (WEEKLY == nRepeatType) {
    		nRecurrence = 168;
	}else if (TWOWEEKLY == nRepeatType) {
    		nRecurrence = 336;
	}else if (MONTHLY == nRepeatType) {
		nRecurrence = 65534;
	}else if (YEARLY == nRepeatType) {
		nRecurrence = 8760;
	}

	return nRecurrence;
}

bool IsEmptyDateTime(GSM_DateTime gsmDateTime)
{
	bool bRet=true;

	if (gsmDateTime.Year && gsmDateTime.Month && gsmDateTime.Month >=1 && gsmDateTime.Month <=12)
		bRet = false;
    else if (gsmDateTime.Day )
		bRet = false;
    else if (gsmDateTime.Hour)
		bRet = false;
    else if (gsmDateTime.Minute)
		bRet = false;
    else if (gsmDateTime.Second)
		bRet = false;
	
	return bRet;
}

bool CCAMMSync::GetCalendarSupport(int& nSupportNum, CArray<NoteTypeSupportInfo,NoteTypeSupportInfo>& ArraySupportType)
{
	int nRet=0;

	nSupportNum = 0;
	ArraySupportType.RemoveAll();

	int nArraySize=0;
	nArraySize = m_ArrayofSupportNoteType.GetSize();
	if(nArraySize) 
	{
		ArraySupportType.SetSize(nArraySize);
		for (int i=0; i<nArraySize; i++) 
		{	//定义局部结构体 notetypeinfo 传递给dlg中的notetype数组
			NoteTypeSupportInfo NoteTypeInfo;
			ZeroMemory(&NoteTypeInfo,sizeof(NoteTypeSupportInfo));
			//下面这个函数很重要，通过notetype的id 在ini中读出该type的信息 付给dlg中的结构体数组ArraySupportType[] 
			//如果要修改文件名，也要在这个函数中作改动
			GetNoteTypeSupportInfo(m_ArrayofSupportNoteType[i],&NoteTypeInfo);
			ArraySupportType[i] = NoteTypeInfo;
		}
	}
	nSupportNum = m_SizeofSupportNoteType;

/*
	CalendarSupportInfo _SupportInfo;
	memset(&_SupportInfo,0,sizeof(CalendarSupportInfo));

	ANWGetCalendarSupport(&_SupportInfo);
	if (Anw_SUCCESS != nRet) {
		//AfxMessageBox(TranslateErrorCode(nRet));
		return false;
	}
	nSupportNum = _SupportInfo.SupportNodeNumber;

	if (0 >= nSupportNum) {
		return false;
	}else {
		ArraySupportType.SetSize(nSupportNum);
	}
    	//record->uiNodeType = REMIND;

	int nLimitation=0;
	for (int i=0; i<GSM_CALENDAR_ENTRIES; i++) {
		//typedef enum {REMIND=1, CALL, MEETING, BIRTHDAY, MEMO} NOTETYPE;//Calendar
        if (nSupportNum <= nLimitation) {
			return false;
		}

     	bool bSupportNoteType=false;
		if (0 == i) {
         	//ArraySupportType[i] =_SupportInfo.NodeFormat & 0x00000001;
         	if(_SupportInfo.NodeFormat & 0x00000001){
        		ArraySupportType[nLimitation++] = i;
			}
		}else if (1 == i) {
        	//ArraySupportType[i] =_SupportInfo.NodeFormat & 0x00000002;
         	if(_SupportInfo.NodeFormat & 0x00000002){
        		ArraySupportType[nLimitation++] = i;
			}
		}else if (2 == i) {
        	//ArraySupportType[i] =_SupportInfo.NodeFormat & 0x00000004;
         	if(_SupportInfo.NodeFormat & 0x00000004){
        		ArraySupportType[nLimitation++] = i;
			}
 		}else if (3 == i) {
        	//ArraySupportType[i] =_SupportInfo.NodeFormat & 0x00000008;
			if(_SupportInfo.NodeFormat & 0x00000008){
        		ArraySupportType[nLimitation++] = i;
			}
 		}else if (4 == i) {
         	//ArraySupportType[i] =_SupportInfo.NodeFormat & 0x00000010;
			if (_SupportInfo.NodeFormat & 0x00000010) {
        		ArraySupportType[nLimitation++] = i;
			}
		}
	}

*/
/*
typedef enum {MEMO=1, CALL, MEETING, BIRTHDAY, REMIND} NOTETYPE;//Calendar
			// Reminder or Date								0x00000001
			//Call											0x00000002
			//Meeting										0x00000004
			//Birthday or Anniversary or Special Occasion	0x00000008
			// Memo or Miscellaneous						0x00000010
*/
	return true;
}

void CCAMMSync::GetCalendarSupport(CalendarSupportInfo& SupportInfo)
{
    SupportInfo =  _static_SupportInfo;

    return ;
}
bool CCAMMSync::GetCalendarSupport()
{
	int nRet=0;

	memset(&_static_SupportInfo,0,sizeof(CalendarSupportInfo));
	//底层，完成静态变量的初始化工作
	ANWGetCalendarSupport(&_static_SupportInfo);
	//根据底层传上来的calendar信息，作下面几个变量的初始化工作
	//支持的类型数量
	m_SizeofSupportNoteType = _static_SupportInfo.SupportNodeNumber;

	if (0 >= m_SizeofSupportNoteType)
		return false;
	else
	{
		m_ArrayofSupportNoteType.RemoveAll();
		m_ArrayofSupportNoteType.SetSize(m_SizeofSupportNoteType);
	}
    	//record->uiNodeType = REMIND;
	
	//根据底层提供的nodeformat 将类型名称加载到类型数组m_ArrayofSupportNoteType[]中
	//注意 这个数组是int型的 只是用来存储类型的id号
	int nLimitation=0;
	if (m_SizeofSupportNoteType >= nLimitation)
	{
        if(_static_SupportInfo.NodeFormat & 0x00000004)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_MEETING;	//meeting
	
        if(_static_SupportInfo.NodeFormat & 0x00000001)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_REMINDER;	//remainder
     
		if(_static_SupportInfo.NodeFormat & 0x00000002)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_CALL;		//call

		if(_static_SupportInfo.NodeFormat & 0x00000008)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_BIRTHDAY;	//Anniversary 周年纪念

		if (_static_SupportInfo.NodeFormat & 0x00000010)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_MEMO;		//date in 9@9u 约会

		if (_static_SupportInfo.NodeFormat & 0x00000020)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_TRAVEL;	//course

		if (_static_SupportInfo.NodeFormat & 0x00000040)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_VACATION;

		if (_static_SupportInfo.NodeFormat & 0x00000080)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_T_ATHL;

		if (_static_SupportInfo.NodeFormat & 0x00000100)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_T_BALL;

		if (_static_SupportInfo.NodeFormat & 0x00000200)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_T_CYCL;

		if (_static_SupportInfo.NodeFormat & 0x00000400)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_T_BUDO;


		if (_static_SupportInfo.NodeFormat & 0x00000800)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_T_DANC;

		if (_static_SupportInfo.NodeFormat & 0x00001000)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_T_EXTR;

		if (_static_SupportInfo.NodeFormat & 0x00002000)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_T_FOOT;

		if (_static_SupportInfo.NodeFormat & 0x00004000)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_T_GOLF;

		if (_static_SupportInfo.NodeFormat & 0x00008000)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_T_GYM;

		if (_static_SupportInfo.NodeFormat & 0x00010000)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_T_HORS;

		if (_static_SupportInfo.NodeFormat & 0x00020000)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_T_HOCK;

		if (_static_SupportInfo.NodeFormat & 0x00040000)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_T_RACE;

		if (_static_SupportInfo.NodeFormat & 0x00080000)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_T_RUGB;

		if (_static_SupportInfo.NodeFormat & 0x00100000)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_T_SAIL;

		if (_static_SupportInfo.NodeFormat & 0x00200000)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_T_STRE;

		if (_static_SupportInfo.NodeFormat & 0x00400000)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_T_SWIM;

		if (_static_SupportInfo.NodeFormat & 0x00800000)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_T_TENN;

		if (_static_SupportInfo.NodeFormat & 0x01000000)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_T_TRAV;
	
		if (_static_SupportInfo.NodeFormat & 0x02000000)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_T_WINT;

		if (_static_SupportInfo.NodeFormat & 0x04000000)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_ALARM;

		if (_static_SupportInfo.NodeFormat & 0x08000000)
        	m_ArrayofSupportNoteType[nLimitation++] = GSM_CAL_DAILY_ALARM;
	}

	return true;
}


CAMMError CCAMMSync::ConvertMoblieDataToPC(									   
	Calendar_Data_Struct CalendarDataFromMobile,
	SchedulesRecord* record,
	int& nErrorCode)
{
//	char cText[514];



	COleDateTime	tNow = COleDateTime::GetCurrentTime();
	CAMMError ret=CAMM_ERROR_SUCCESS;

	if( CalendarDataFromMobile.Start_DateTime.Year==0 )
		CalendarDataFromMobile.Start_DateTime.Year = tNow.GetYear();
	if( CalendarDataFromMobile.Start_DateTime.Month>12 || 
		CalendarDataFromMobile.Start_DateTime.Month<1 )
		CalendarDataFromMobile.Start_DateTime.Month = tNow.GetMonth();
	if( CalendarDataFromMobile.Start_DateTime.Day>31 || 
		CalendarDataFromMobile.Start_DateTime.Day<1 )
		CalendarDataFromMobile.Start_DateTime.Day = tNow.GetDay();

	if( CalendarDataFromMobile.End_DateTime.Year==0 )
		CalendarDataFromMobile.End_DateTime.Year = tNow.GetYear();
	if( CalendarDataFromMobile.End_DateTime.Month>12 || 
		CalendarDataFromMobile.End_DateTime.Month<1 )
		CalendarDataFromMobile.End_DateTime.Month = tNow.GetMonth();
	if( CalendarDataFromMobile.End_DateTime.Day>31 || 
		CalendarDataFromMobile.End_DateTime.Day<1 )
		CalendarDataFromMobile.End_DateTime.Day = tNow.GetDay();

 

	COleDateTime dtEnd,dtStart;
	dtStart.SetDateTime(CalendarDataFromMobile.Start_DateTime.Year, CalendarDataFromMobile.Start_DateTime.Month,CalendarDataFromMobile.Start_DateTime.Day,
		CalendarDataFromMobile.Start_DateTime.Hour,CalendarDataFromMobile.Start_DateTime.Minute,0);

	dtEnd.SetDateTime(CalendarDataFromMobile.End_DateTime.Year, CalendarDataFromMobile.End_DateTime.Month,CalendarDataFromMobile.End_DateTime.Day,
		CalendarDataFromMobile.End_DateTime.Hour,CalendarDataFromMobile.End_DateTime.Minute,0);
	if(dtStart > dtEnd)
	{
		CalendarDataFromMobile.End_DateTime.Year = CalendarDataFromMobile.Start_DateTime.Year;
		CalendarDataFromMobile.End_DateTime.Month = CalendarDataFromMobile.Start_DateTime.Month;
		CalendarDataFromMobile.End_DateTime.Day = CalendarDataFromMobile.Start_DateTime.Day;
		CalendarDataFromMobile.End_DateTime.Hour = CalendarDataFromMobile.Start_DateTime.Hour;
		CalendarDataFromMobile.End_DateTime.Minute = CalendarDataFromMobile.Start_DateTime.Minute;

	}

	//在这里使用dayofweek
	if(CalendarDataFromMobile.Repeat_DayOfWeekMask)
	{
		int nMoveDays = 0;
		COleDateTimeEx dtoleStart ;
		dtoleStart.SetDate(CalendarDataFromMobile.Start_DateTime.Year,CalendarDataFromMobile.Start_DateTime.Month,CalendarDataFromMobile.Start_DateTime.Day);

		int nDay = dtoleStart.GetDayOfWeek() -1;
		while(((CalendarDataFromMobile.Repeat_DayOfWeekMask >> nDay) &  1) == 0)
		{
			dtoleStart.MoveToNextDayDate(1);
			nDay ++ ;
			nMoveDays++;
			if(nDay>6) nDay = 0;
		}
		CalendarDataFromMobile.Start_DateTime.Year = dtoleStart.GetYear();
		CalendarDataFromMobile.Start_DateTime.Month = dtoleStart.GetMonth();
		CalendarDataFromMobile.Start_DateTime.Day = dtoleStart.GetDay();
		if(nMoveDays>0)
		{
			COleDateTimeEx dtoleEnd;
			dtoleEnd.SetDate(CalendarDataFromMobile.End_DateTime.Year,CalendarDataFromMobile.End_DateTime.Month,CalendarDataFromMobile.End_DateTime.Day);
			dtoleEnd.MoveToNextDayDate(nMoveDays);
			CalendarDataFromMobile.End_DateTime.Year = dtoleEnd.GetYear();
			CalendarDataFromMobile.End_DateTime.Month = dtoleEnd.GetMonth();
			CalendarDataFromMobile.End_DateTime.Day = dtoleEnd.GetDay();

		}

	}

	
	NoteTypeSupportInfo NoteTypeInfo;
	ZeroMemory(&NoteTypeInfo,sizeof(NoteTypeSupportInfo));
	GetNoteTypeSupportInfo(CalendarDataFromMobile.NodeType,&NoteTypeInfo);
   	record->uiNodeType = CalendarDataFromMobile.NodeType;
	if(NoteTypeInfo.bPhoneNoText2)
		record->telnumber = CalendarDataFromMobile.Location;
	else
		record->cuLocation = CalendarDataFromMobile.Location;
	
	// 1. ** GSM_DateTime Start_DateTime
	int nStartHour,nStartMin,nStartSec;
	nStartHour=nStartMin=nStartSec = 0;
	if( NoteTypeInfo.dwSupportFlag & 0x00000008 ) //startTime
	{
     	ParseGSMDateTime(CalendarDataFromMobile.Start_DateTime, record->starttime);
		nStartHour = CalendarDataFromMobile.Start_DateTime.Hour;
		nStartMin = CalendarDataFromMobile.Start_DateTime.Minute;
		nStartSec = CalendarDataFromMobile.Start_DateTime.Second;
	}
	else
	{
		GSM_DateTime gsmtime;
		gsmtime.Year = CalendarDataFromMobile.Start_DateTime.Year;
		gsmtime.Month = CalendarDataFromMobile.Start_DateTime.Month;
		gsmtime.Day = CalendarDataFromMobile.Start_DateTime.Day;
		if (!IsEmptyDateTime(CalendarDataFromMobile.Alarm_DateTime))
		{
			gsmtime.Hour =CalendarDataFromMobile.Alarm_DateTime.Hour;
			gsmtime.Minute = CalendarDataFromMobile.Alarm_DateTime.Minute;
			gsmtime.Second= 0;
		}
/*	原来就没有
	else if (!IsEmptyDateTime(CalendarDataFromMobile.Silent_Alarm_DateTime))
		{
			gsmtime.Hour =CalendarDataFromMobile.Silent_Alarm_DateTime.Hour;
			gsmtime.Minute = CalendarDataFromMobile.Silent_Alarm_DateTime.Minute;
			gsmtime.Second= 0;
		}*/
		else
		{
			gsmtime.Hour =9;
			gsmtime.Minute = 0;
			gsmtime.Second= 0;
		}
  		nStartHour = gsmtime.Hour;
		nStartMin = gsmtime.Minute;
		nStartSec = gsmtime.Second;
	  	ParseGSMDateTime(gsmtime, record->starttime);
	}

	if(CalendarDataFromMobile.End_DateTime.Year == 0)
		record->endtime = record->starttime;
	else 
	{
		if(!(NoteTypeInfo.dwSupportFlag & 0x00000020)) ////StopTime
		{
			CalendarDataFromMobile.End_DateTime.Hour = nStartHour;
			CalendarDataFromMobile.End_DateTime.Minute = nStartMin;
			CalendarDataFromMobile.End_DateTime.Second = nStartSec;
		}
		ParseGSMDateTime(CalendarDataFromMobile.End_DateTime, record->endtime);
	}

	// 2. ** GSM_DateTime End_DateTime

	if(CalendarDataFromMobile.Start_DateTime.Year != CalendarDataFromMobile.End_DateTime.Year ||
		CalendarDataFromMobile.Start_DateTime.Month != CalendarDataFromMobile.End_DateTime.Month ||
		CalendarDataFromMobile.Start_DateTime.Day != CalendarDataFromMobile.End_DateTime.Day)
		record->bEndDateSameToStartDate = FALSE;
	else record->bEndDateSameToStartDate = TRUE;


	bool bhaveAlarm=false;
	// 3. ** GSM_DateTime Alarm_DateTime
	if (!IsEmptyDateTime(CalendarDataFromMobile.Alarm_DateTime))
	{
    	bhaveAlarm = true;
//    	record->bhaveTone = true;
    //	ParseGSMDateTime(CalendarDataFromMobile.Alarm_DateTime, record->alarmLeadDatetime);
    	GSMTimetoDATE(CalendarDataFromMobile.Alarm_DateTime, record->dtAlarmTime);
	}
/*	else 
	{
		// 4. ** GSM_DateTime Silent_Alarm_DateTime
		if (!IsEmptyDateTime(CalendarDataFromMobile.Silent_Alarm_DateTime)){
    		bhaveAlarm = true;
	   		record->bhaveTone = false;
		//	ParseGSMDateTime(CalendarDataFromMobile.Silent_Alarm_DateTime, record->alarmLeadDatetime);
    		GSMTimetoDATE(CalendarDataFromMobile.Silent_Alarm_DateTime, record->dtAlarmTime);
		}
	}*/
	record->bhaveTone = CalendarDataFromMobile.haveTone;
	record->haveAlarm = bhaveAlarm;

	// 8. ** char Text[514]

 	record->title = CalendarDataFromMobile.Text;


    // 9. ** int Recurrence;
	int nRepeatFrequency=CalendarDataFromMobile.RecurrenceFrequency;
//	int nRecurrence = DecodeRepeatType(CalendarDataFromMobile.Recurrence, nRepeatFrequency);
	int nRepeatType;

	//在这里测试CalendarDataFromMobile.RecurrenceFrequency
 //	CString st;
  //	st.Format(L"底层传递的RecurrenceFrequency %d",nRepeatFrequency);
  //	AfxMessageBox(st);
// 	//测试各种类型的dayofweek值
 //	st.Format(L"底层传递的dayofweek %d",CalendarDataFromMobile.Repeat_DayOfWeekMask);
  //	AfxMessageBox(st);

	//在这里进行recurrencetyep到repeat type的转换
	switch(CalendarDataFromMobile.RecurrenceType)
	{
	case CAL_REPEAT_NONE:
		nRepeatType = NOREPEAT;
		break;
	case CAL_REPEAT_DAILY:
		nRepeatType = DAILY;
		break;
	case CAL_REPEAT_WEEKLY:
		nRepeatType = WEEKLY;
		break;
	case CAL_REPEAT_MONTHLY:
		nRepeatType = MONTHLY;
		break;
	case CAL_REPEAT_MONTH_WEEKLY:
		nRepeatType = WEEKLYMONTHLY;
		GetwhichWeekDay(CalendarDataFromMobile.Start_DateTime,record->rfWhichWeek,record->rfWhichDay);
		break;
	case CAL_REPEAT_YEARLY:
		nRepeatType = YEARLY;
		break;
	}


	//下面两个if是用来判断 twoweekly 暂时不用 kerm change for 9a9u 1.04
/*	if(nRepeatType == WEEKLY && nRepeatFrequency == 2)
		nRepeatType = TWOWEEKLY;

	if(nRepeatType != WEEKLY && nRepeatType != TWOWEEKLY)
		record->nDayofWeekMask = 0;
	else
		record->nDayofWeekMask = CalendarDataFromMobile.Repeat_DayOfWeekMask;
*/

	//在此处修改dayofweek的生成条件，只有 days 类型才使用，其他情况为0 kerm add for 9a9u 1.04
	record->nDayofWeekMask = CalendarDataFromMobile.Repeat_DayOfWeekMask;

//	itoa(-1==nRecurrence ? 0 : nRecurrence, cText, 10);
	//itoa(CalendarDataFromMobile.Recurrence, cText, 10);
//	record->csHaveRepeat = cText;
	if(nRepeatType == NOREPEAT)
		record->haveRepeat = FALSE;
	else
		record->haveRepeat = TRUE;
	record->repeatType = nRepeatType;
	if(record->haveRepeat)
	{
		if(CalendarDataFromMobile.Repeat_EndDate.Year > 0 && CalendarDataFromMobile.Repeat_EndDate.Month >0 &&
			CalendarDataFromMobile.Repeat_EndDate.Month<=12 && CalendarDataFromMobile.Repeat_EndDate.Day >0 &&
			CalendarDataFromMobile.Repeat_EndDate.Day<=31)
			record->haveUntil = true;

   		if(record->haveUntil)
		{
			record->untilDate.SetDate(CalendarDataFromMobile.Repeat_EndDate.Year,CalendarDataFromMobile.Repeat_EndDate.Month,
				CalendarDataFromMobile.Repeat_EndDate.Day);
		}
	}

//	record->haveRepeat = nRecurrence>=0 ? true : false;
//	record->repeatType = nRecurrence;

    //10. ** int Private;
 	record->uiPrivate = CalendarDataFromMobile.Private;

    //11. ** int ContactID;

    //12. ** int Index;
   //	record->uiIndex = CalendarDataFromMobile.Index;
	sprintf(record->szuiIndex,CalendarDataFromMobile.szIndex);

	//No need, Nono[2004_1105]
    //13. ** int Repeat_DayOfWeek;
    //14. ** int Repeat_Day;
    //15. ** int Repeat_WeekOfMonth;
    //16. ** int Repeat_Month;

    //17. ** int Repeat_Frequency;
	record->repeatFrequency = nRepeatFrequency;//receive the value of "nRepeatFrequency" from step 9--"bool Recurrence"

    //18. ** GSM_DateTime Repeat_StartDate;
    //19. ** GSM_DateTime Repeat_EndDate;

	record->memo.Format(_T("%s"),CalendarDataFromMobile.szDescription);

//
#ifdef _DEBUG
	DumpMobileCalendar(_Calendar_Data_Strc_);
#endif

	return ret;
}
CAMMError  CCAMMSync::ConvertPCDataToMobile(CScheduleInfo Record,Calendar_Data_Struct *pCalendarData,int& nErrorCode)
{
	CAMMError ret = ConvertPCDataToMobile(Record,nErrorCode);
	memcpy(pCalendarData ,&_Calendar_Data_Strc_,sizeof(Calendar_Data_Struct));
	return ret;
}

CAMMError CCAMMSync::ConvertPCDataToMobile(
	CScheduleInfo Record,
	int& nErrorCode)
{
	int nRet=0;
	CAMMError ret=CAMM_ERROR_SUCCESS;
	int nStartYear = theApp.m_nStartYear ; 
	int nShiftYear = 0;
//	
  
	memset(&_Calendar_Data_Strc_, 0, sizeof(_Calendar_Data_Strc_));
  
	NoteTypeSupportInfo NoteTypeInfo;
	ZeroMemory(&NoteTypeInfo,sizeof(NoteTypeSupportInfo));
	GetNoteTypeSupportInfo(Record.uiNodeType,&NoteTypeInfo);


	// 5. ** int NodeType
//typedef enum {REMIND=0, CALL, MEETING, BIRTHDAY, MEMO} NOTETYPE;//Calendar
	_Calendar_Data_Strc_.NodeType = Record.uiNodeType;

	if( !(NoteTypeInfo.dwSupportFlag & 0x00000008) ) //startTime
	{
		if (Record.haveAlarm) 
			Record.hiStartTime.SetData(Record.dtAlarmDateTime.GetHour(),Record.dtAlarmDateTime.GetMinute(),0);
		else
			Record.hiStartTime.SetData(9,0,0);
	}

	if( !(NoteTypeInfo.dwSupportFlag & 0x00000020)) //StopTime
	{
		Record.hiStopTime = Record.hiStartTime;
	}

	if( Record.dtStartDate.GetYear() <nStartYear)
	{
		nShiftYear = nStartYear -Record.dtStartDate.GetYear();
		 Record.dtStartDate.SetDateTime(nStartYear,Record.dtStartDate.GetMonth(),Record.dtStartDate.GetDay(),
			Record.dtStartDate.GetHour(),Record.dtStartDate.GetMinute(),Record.dtStartDate.GetSecond());
	}

	if( !(NoteTypeInfo.dwSupportFlag & 0x00000010)) //StopDate
	{
		Record.dtStopDate.SetDateTime(Record.dtStartDate.GetYear(),Record.dtStartDate.GetMonth(),Record.dtStartDate.GetDay(),
				Record.dtStopDate.GetHour(),Record.dtStopDate.GetMinute(),Record.dtStopDate.GetSecond());
	}
	else
	{
		if(nShiftYear > 0)
		{
			 Record.dtStopDate.SetDateTime(Record.dtStopDate.GetYear() + nShiftYear ,Record.dtStopDate.GetMonth(),Record.dtStopDate.GetDay(),
				Record.dtStopDate.GetHour(),Record.dtStopDate.GetMinute(),Record.dtStopDate.GetSecond());
		}
	/*	if( Record.dtStopDate.GetYear() <nStartYear)
		{
			 Record.dtStopDate.SetDateTime(nStartYear,Record.dtStopDate.GetMonth(),Record.dtStopDate.GetDay(),
				Record.dtStopDate.GetHour(),Record.dtStopDate.GetMinute(),Record.dtStopDate.GetSecond());
		}*/
	}

	// 1. ** GSM_DateTime Start_DateTime


  
		
	if (!WrapperGSMDateTime(Record.dtStartDate, Record.hiRealStartTime, _Calendar_Data_Strc_.Start_DateTime))
	{
	}
   	if (!WrapperGSMDateTime(Record.dtStopDate, Record.hiRealStopTime,  _Calendar_Data_Strc_.End_DateTime))
	{
	}
	    
     //080825LIBAOLIU
	 
// 	if (!WrapperGSMDateTime(Record.dtStartDate, Record.hiStartTime, _Calendar_Data_Strc_.Start_DateTime))
// 	{
// 	}
//    	if (!WrapperGSMDateTime(Record.dtStopDate, Record.hiStopTime,  _Calendar_Data_Strc_.End_DateTime))
// 	{
// 	}
	    

	if (Record.haveAlarm) 
	{ 
		_Calendar_Data_Strc_.haveTone = Record.bhaveTone ? 1 : 0;
		if(nShiftYear > 0)
		{
			 Record.dtAlarmDateTime.SetDateTime(Record.dtAlarmDateTime.GetYear() + nShiftYear ,Record.dtAlarmDateTime.GetMonth(),Record.dtAlarmDateTime.GetDay(),
				Record.dtAlarmDateTime.GetHour(),Record.dtAlarmDateTime.GetMinute(),Record.dtAlarmDateTime.GetSecond());
		}

        WrapperGSMDateTime(Record.dtAlarmDateTime, _Calendar_Data_Strc_.Alarm_DateTime);
/*		if (Record.bhaveTone) 
		{
     	// 3. ** GSM_DateTime Alarm_DateTime
        	if (!WrapperGSMDateTime(Record.dtAlarmDateTime, _Calendar_Data_Strc_.Alarm_DateTime))
			{
			}
		}else {
    	// 4. ** GSM_DateTime Silent_Alarm_DateTime
        	if (!WrapperGSMDateTime(Record.dtAlarmDateTime, _Calendar_Data_Strc_.Silent_Alarm_DateTime))
			{
			}
		}*/
	}
	

	if (NoteTypeInfo.bPhoneNoText2)
    	_tcscpy(_Calendar_Data_Strc_.Location, (LPCTSTR)Record.telnumber);
	else 
    	_tcscpy(_Calendar_Data_Strc_.Location, (LPCTSTR)Record.cuLocation);
    

	// 8. ** char Text[514]
	_tcscpy(_Calendar_Data_Strc_.Text, (LPCTSTR)Record.csTitle);

    // 9. ** int Recurrence;
	if (Record.haveRepeat)
	{
   //      _Calendar_Data_Strc_.Recurrence = EncodeRepeatType((int) Record.repeatType, Record.repeatFrequency);
		_Calendar_Data_Strc_.RecurrenceFrequency = Record.repeatFrequency;
// 		CString st;
// 		st.Format(L"ConvertPCDataToMobile中看看repeat type\nRecord.repeatFrequency= %d\nRecord.repeatType=%d ", Record.repeatFrequency,Record.repeatType );
// 		AfxMessageBox(st);
		switch(Record.repeatType)
		{
		case NOREPEAT:
			_Calendar_Data_Strc_.RecurrenceType = CAL_REPEAT_NONE;
			break;
		case DAILY:
			_Calendar_Data_Strc_.RecurrenceType = CAL_REPEAT_DAILY;
			break;
		case WEEKLY:
		case WEEKDAYS:	//kerm add for 9a9u 1.04
			_Calendar_Data_Strc_.RecurrenceType = CAL_REPEAT_WEEKLY;
			break;
		case TWOWEEKLY:
			_Calendar_Data_Strc_.RecurrenceType = CAL_REPEAT_WEEKLY;
			_Calendar_Data_Strc_.RecurrenceFrequency  = 2;
			break;
		case MONTHLY:
			_Calendar_Data_Strc_.RecurrenceType = CAL_REPEAT_MONTHLY;
			break;
		case WEEKLYMONTHLY:
			_Calendar_Data_Strc_.RecurrenceType = CAL_REPEAT_MONTH_WEEKLY;
			break;
		case YEARLY:
			_Calendar_Data_Strc_.RecurrenceType = CAL_REPEAT_YEARLY;
			break;
		}
		if(Record.haveRepeatUntil)
		{
		//      WrapperGSMDateTime(Record.utilWhichDate, _Calendar_Data_Strc_.Repeat_EndDate);
			  _Calendar_Data_Strc_.Repeat_EndDate.Year = Record.utilWhichDate._yy;
			  _Calendar_Data_Strc_.Repeat_EndDate.Month = Record.utilWhichDate._mm;
			  _Calendar_Data_Strc_.Repeat_EndDate.Day = Record.utilWhichDate._dd;
		}
		if(_Calendar_Data_Strc_.RecurrenceType == CAL_REPEAT_WEEKLY)
			_Calendar_Data_Strc_.Repeat_DayOfWeekMask = Record.nDayofWeekMask ;
		else 
			//除了WEEKLY 其他的都是0 
			_Calendar_Data_Strc_.Repeat_DayOfWeekMask = 0;		  
			//_Calendar_Data_Strc_.Repeat_DayOfWeekMask = Record.nDayofWeekMask;

	}

    //10. ** int Private;
	_Calendar_Data_Strc_.Private = (int) Record.uiPrivate;

    //11. ** int ContactID;

    //12. ** int Index;
//	_Calendar_Data_Strc_.Index = (int) Record.uiIndex;
	sprintf(_Calendar_Data_Strc_.szIndex,Record.szuiIndex);


	if(Record.csMemo.GetLength()>0)
		wsprintf(_Calendar_Data_Strc_.szDescription,_T("%s"),Record.csMemo);

  
	return ret;
}

/////
/*
CAMMError CCAMMSync::LoadSchedule(SchedulesRecord record[],CString csDBPath, int &nSize)
{
	if (csDBPath.IsEmpty()) {
		return CAMM_ERROR_INVALIDARG;
	}

	if (xCheckFileExisted(csDBPath) != CAMM_ERROR_SUCCESS) {
		return CAMM_ERROR_INVALIDFILE;
	}

	CString szProductPath;
	xGetProductPath(szProductPath,csDBPath);

	CAMMError ret = xLoadSchedule(record,szProductPath,csDBPath,nSize);

	return ret;
}

CAMMError CCAMMSync::xLoadSchedule(SchedulesRecord record[],CString szProductPath,CString csDBPath,int &nSize)
{


	long lRet = 0L;

	
/*	lRet =	MoOpenMyOrganizer(TRUE, szProductPath);
	
	lRet =	MoOpenFile(csDBPath);

	lRet =	MoOpenDatabase(MO_SCH);
	
	TCHAR lpBuffer[512];
	DWORD dwLength=0;
	
	ScheduleRecordWrapper crw;

	CString storage[SupportFieldNumSch];
	CString dummy = "Not Impl.";
	SYTR_REPEAT structRepeat;
	BOOL bIsRpt;
	DWORD recCount = 0;
	CString csPCID ; 
	GUID pcid = IID_NONE;
	
	lRet =	MoGetRecordCount(recCount);

	nSize = recCount;
	for(int x=0; x< recCount;x++)
	{
		memset(lpBuffer,0x00,sizeof(TCHAR)*512);
		sprintf(lpBuffer,"%d",x+1);
		
		long l = MoCreateRecord(MO_OPEN_EXISTING_BY_INDEX, lpBuffer, 512, dwLength);

		crw.Init();
		
		TCHAR MoStrBuffer[512];
		DWORD uBytes=0;
		TCHAR FieldBuffer[FIELD_BUFFER_LEN];
		DWORD dwBytesWritten=0;
		int n;
		for(n=0; n<SupportFieldNumSch; n++)
		{
			
			dwBytesWritten = 0;
			memset(FieldBuffer,0x00,sizeof(TCHAR)*FIELD_BUFFER_LEN);
			lRet = MoGetRecordFieldNameByID(MO_SCH, crw.GetOutputFieldId(n), MoStrBuffer, 512, uBytes);

			lRet = MoGetRecordField(MoStrBuffer, FieldBuffer,FIELD_BUFFER_LEN,dwBytesWritten);
					
			if(n==0)
			{
				csPCID.Format("%s",FieldBuffer);
			}
			else
			{
				storage[n-1].Format("%s",FieldBuffer);
			}
		}
		StringToGuid(csPCID,pcid);
		record[x].SetPCID(pcid); 

		//Repead Code
		memset(&structRepeat, 0x00, sizeof(SYTR_REPEAT));

		lRet=MoGetGeneralRecordRecurInfo(structRepeat, bIsRpt);
		CString cus;
		cus.Format("0x%x",structRepeat);


		if(bIsRpt == TRUE)
		{
			BOOL bHaveUntil = FALSE;
			ScheduleDate sd;
			
			bHaveUntil = !structRepeat.noEndDate;
			if(bHaveUntil)
			{
				COleDateTime DateTime;
				DateTime = structRepeat.endDate;
				sd.SetDate(DateTime.GetYear(),DateTime.GetMonth(),DateTime.GetDay());
			}
				  
			// _title, _st,_et,_memo,_telephone,_haveAlarm,_alarmLeadtime,_haveRepeat,_haveUntil,_repeatType,_repeatFreq,_whichweek,_whichday,_untilDate
			int nTransfer=0;
			nTransfer = atoi(storage[5]);

			//Add by Peter for change from SYTR_XXX to XXX
			int nType=0;
			switch(structRepeat.type)
			{
				case SYTR_DAILY : //Daily
					nType = DAILY;
					break;
				case SYTR_WEEKLY_DAYS : //Weekly
					nType = WEEKLY;
					break;
				case SYTR_MONTHLY : //Monthly
					nType = MONTHLY;
					break;
				case SYTR_MONTHLY_BY_POS : //Weekly Monthly
					nType = WEEKLYMONTHLY;
					break;
				case SYTR_YEARLY : //Yearly
					nType = YEARLY;
					break;
			}
			//Peter

			//Peter
			//Comment by Peter because change from storage[6] to storage[7] for alarm leadtime
			//			record[x].SetRecord(storage[2],storage[0],storage[1],storage[3],storage[4],(BOOL)nTransfer,storage[6],bIsRpt,
			//				!structRepeat.noEndDate,nType,structRepeat.frequency,structRepeat.weekOfMonth,structRepeat.dayOfWeek,sd);
			record[x].SetRecord(storage[2],storage[0],storage[1],storage[3],storage[4],(BOOL)nTransfer,storage[7],bIsRpt,
				bHaveUntil,nType,structRepeat.frequency,structRepeat.weekOfMonth,structRepeat.dayOfWeek,sd,storage[8]);
			//Peter
		
			for(int n=0;n < SupportFieldNumSch; n++)
			{
			}
		}
		else
		{
			ScheduleDate sd;
			sd.SetDate(0,0,0);
			int nTransfer=0;
			nTransfer = atoi(storage[5]);

			//sanjeev2411
			record[x].SetRecord(storage[2],storage[0],storage[1],storage[3],storage[4],(BOOL)nTransfer,storage[7],bIsRpt,
				0,0,0,0,0,sd,storage[8]);
			//sanjeev2411
			for(int n=0;n < SupportFieldNumSch; n++)
			{
			}
		}

		//End of repeat code



		// Change this function to 
		//record[x].SetRecord(storage[2],storage[0],storage[1],storage[3],storage[4],storage[5],storage[6],storage[7],storage[8],storage[9],storage[10],storage[11]);
		//record[x].SetRecord(storage[2],storage[0],storage[1],storage[3],storage[4],storage[5],storage[6],storage[7],dummy,dummy,dummy,dummy);

		lRet = MoCloseRecord(FALSE, lpBuffer);
	
	}
	//End of Loop
	
	lRet = MoCloseDatabase();
	lRet = 	MoCloseFile(FALSE);

	lRet = MoCloseMyOrganizer();
*/	

/*
	return CAMM_ERROR_SUCCESS;
}

CAMMError CCAMMSync::SaveSchedule(SchedulesRecord record[],CString csDBPath,int nSize)
{
	if (csDBPath.IsEmpty()) {
		return CAMM_ERROR_INVALIDARG;
	}
	if (xCheckFileExisted(csDBPath) == CAMM_ERROR_INVALIDFILE) {
		if (xCreateEmptyNewFile(csDBPath) != CAMM_ERROR_SUCCESS) {
			AfxMessageBox("Can't save");
			return CAMM_ERROR_FAILURE;
		}
	}
	CString szProductPath;
	xGetProductPath(szProductPath,csDBPath);
/*
	CString TempSavePath = szProductPath + "temp.iog";
	if (CopyFile(csDBPath, TempSavePath, FALSE)) {
        DWORD dwAttrs = GetFileAttributes(TempSavePath); 
		//   if ((dwAttrs & FILE_ATTRIBUTE_READONLY)) 
		//  { 
		SetFileAttributes(TempSavePath,dwAttrs | FILE_ATTRIBUTE_NORMAL); 
        //} 
    } 
    else 
    { 
        return CAMM_ERROR_FAILURE;
    } 
	
	
	xRemoveOrganizer(MO_SCH,szProductPath,csDBPath);
*/
//	CAMMError ret = xSaveSchedule(record,szProductPath,csDBPath,nSize);
/*
	if (ret != CAMM_ERROR_SUCCESS) {
		CopyFile(TempSavePath, csDBPath, FALSE);
	}
	try{
		CFile::Remove(TempSavePath);
	}
	catch(...){}
*/
/*	return ret;
}

CAMMError CCAMMSync::xSaveSchedule(SchedulesRecord record[],CString szProductPath,CString csDBPath,int nSize)
{

	long lRet = 0;
	
/*	lRet =	MoOpenMyOrganizer(TRUE, szProductPath);
	lRet =	MoOpenFile(csDBPath);
	lRet =	MoOpenDatabase(MO_SCH);

	TCHAR lpBuffer[512];
	DWORD dwLength=0;
	memset(lpBuffer,0x00,sizeof(TCHAR)*512);
	
	ScheduleRecordWrapper crw;
	
	for(int x=0; x< nSize;x++)
	{
		memset(lpBuffer,0x00,sizeof(TCHAR)*512);
		long l = 0;
		
		l = MoCreateRecord(MO_NEW_CREATE, lpBuffer, 512, dwLength);

		crw.Init();
		crw.PrepareRecord(record[x]);
		int MBLen;
		TCHAR MoStrBuffer[512];
		DWORD uBytes=0;
		TCHAR WriteFieldBuffer[FIELD_BUFFER_LEN];
		// Repeat Code
		SYTR_REPEAT structRepeat;
		memset(&structRepeat, 0x00, sizeof(SYTR_REPEAT));
		if(record[x].haveRepeat == TRUE)
		{
			switch(record[x].repeatType)
			{
				case DAILY : //Daily
					structRepeat.type = SYTR_DAILY;
					structRepeat.frequency = record[x].repeatFrequency; // in days
					break;
				case WEEKLY : //Weekly
					structRepeat.type = SYTR_WEEKLY_DAYS;
					structRepeat.frequency = record[x].repeatFrequency; // in weeks
					break;
				case MONTHLY : //Monthly
					structRepeat.type = SYTR_MONTHLY;
					structRepeat.frequency = record[x].repeatFrequency; // in months
					break;
				case WEEKLYMONTHLY : //Weekly Monthly
					structRepeat.type = SYTR_MONTHLY_BY_POS;
					structRepeat.frequency = record[x].repeatFrequency; // in months
					structRepeat.weekOfMonth = record[x].rfWhichWeek; //First , Second, Third, Fourth or Last week
					break;
				case YEARLY : //Yearly
					structRepeat.type = SYTR_YEARLY;
					structRepeat.frequency = record[x].repeatFrequency;
					break;
			}

			if(record[x].haveUntil)
			{
				COleDateTime DateTime;
				DateTime.SetDate(record[x].untilDate._yy, record[x].untilDate._mm,	record[x].untilDate._dd);
				structRepeat.endDate = DateTime;
			}
			
			structRepeat.noEndDate = !record[x].haveUntil;
			structRepeat.dayOfWeek = record[x].rfWhichDay;
		}
		// End of Repeat code
	
		lRet= MoSetGeneralRecordRecurInfo(structRepeat);
		CString cus;
		cus.Format("0x%x",structRepeat);

		int n;
		for(n=SupportFieldNumSch-1; n>=0; n--)
		{
			//			MBLen = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK|WC_SEPCHARS, 
			//				crw.GetInputField(n), crw.GetInputFieldLen(n), WriteFieldBuffer, 
			//				FIELD_BUFFER_LEN, NULL, NULL);
			if(crw.GetInputField(n) != NULL)
			{
				MBLen = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK|WC_SEPCHARS, 
					crw.GetInputField(n), -1, WriteFieldBuffer, 
					FIELD_BUFFER_LEN, NULL, NULL);
				
				lRet = MoGetRecordFieldNameByID(MO_SCH, crw.GetOutputFieldId(n), MoStrBuffer, 512, uBytes);
				lRet = MoSetRecordField(MoStrBuffer, WriteFieldBuffer);

				memset(WriteFieldBuffer,0x00,sizeof(TCHAR)*FIELD_BUFFER_LEN);
			}
		}
		lRet = MoCloseRecord(TRUE, lpBuffer);
	
	}
	//End of Loop

	lRet = MoCloseDatabase();
	lRet = 	MoCloseFile(TRUE);

	lRet = MoCloseMyOrganizer();
*/	
/*
	
	return CAMM_ERROR_SUCCESS;
	
}


//Add by peter 
CAMMError CCAMMSync::xCreateEmptyNewFile(CString csDBPath)
{
	CFile f;
	CFileException e;
	if( !f.Open( csDBPath, CFile::modeCreate | CFile::modeReadWrite, &e ) )
	{
		return CAMM_ERROR_INVALIDFILE;
	}
	f.Close();
	return CAMM_ERROR_SUCCESS;

	//	HANDLE hFile; 
	// 
	//	if (xCheckFileExisted(csDBPath) == CAMM_ERROR_SUCCESS) {
	//		DeleteFile(csDBPath);
	//	}
	//	
	//	hFile = CreateFile(csDBPath,           // open MYFILE.TXT 
	//					GENERIC_READ,              // open for reading 
	//					FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,           // share for reading 
	//					NULL,                      // no security 
	//					CREATE_NEW,             // existing file only 
	//					FILE_ATTRIBUTE_NORMAL,     // normal file 
	//					NULL);                     // no attr. template 
	// 
	//	if (hFile == INVALID_HANDLE_VALUE) 
	//	{ 
	//			return CAMM_ERROR_INVALIDFILE;
	//	}
	//
	//	CloseHandle(hFile);
	//
	//	return CAMM_ERROR_SUCCESS;

}

CAMMError CCAMMSync::xCheckFileExisted(CString csDBPath)
{
	if (csDBPath.IsEmpty()) {
		return CAMM_ERROR_INVALIDARG;
	}

	CFileFind finder;
	BOOL fStatus = finder.FindFile(csDBPath);
	if(!fStatus) {
		return CAMM_ERROR_INVALIDFILE;
	}
	return CAMM_ERROR_SUCCESS;
}

CAMMError CCAMMSync::xGetSystemTempPath(CString &szTempPath)
{
	TCHAR tpath[MAX_PATH];
	memset(tpath,0x00,sizeof(TCHAR)*MAX_PATH);

	DWORD result=GetTempPath( 200,(LPTSTR)tpath);	//get temp folder from system

	if(result!=0) {
		szTempPath.Format("%s",tpath);
	} else {
		szTempPath = "C:\\";
	}

	return CAMM_ERROR_SUCCESS;
}

CAMMError CCAMMSync::xGetProductPath(CString &szProductPath,CString csDBPath)
{
/*	int pos=0,pos2=0;
	TCHAR fol ='\\';

	pos2 = csDBPath.Find(fol);
	if (pos2 <= 0) {
		szProductPath = "";
		return CAMM_ERROR_NONE;
	}
	while (pos2 > 0) {
		pos = pos2;
		pos2 = csDBPath.Find(fol,pos+1);
	}

	szProductPath = csDBPath.Left(pos+1);
*/
/*	TCHAR szPath[MAX_PATH];
	memset(szPath,0x00,sizeof(TCHAR) * MAX_PATH);
	GetModuleFileName(NULL,szPath,MAX_PATH);
	CString strPath;
	strPath = szPath;
	if(!strPath.IsEmpty())
	{
		int pos = strPath.ReverseFind('\\');
		strPath = strPath.Left(pos+1);
		szProductPath = CString(strPath);
	}

	return CAMM_ERROR_SUCCESS;
}

CAMMError CCAMMSync::xRemoveOrganizer(int nType,CString szProductPath,CString csDBPath)
{

	CString TempSavePath = szProductPath + "tempR.iog";
	xCreateEmptyNewFile(TempSavePath);
/*
	if (nType != MO_SCH) {
		xCopySchedule(szProductPath,csDBPath,TempSavePath);
	}

	if (nType != MO_ADR) {
		xCopyContact(szProductPath,csDBPath,TempSavePath);
	}

	if (nType != MO_NOTE) {
		xCopyNote(szProductPath,csDBPath,TempSavePath);
	}
*/
/*	CopyFile(TempSavePath,csDBPath,FALSE);
	try{
		CFile::Remove(TempSavePath);
	}
	catch(...){}

	
	return CAMM_ERROR_SUCCESS;
	
}
/*
CAMMError CCAMMSync::xCopyContact(CString szProductPath,CString csSrcDBPath,CString csTgtDBPath)
{
	CAMMError ret = CAMM_ERROR_SUCCESS;

	//Load 
	int recCount = 0;
	ContactsRecord *record;
//	GetRecordCount(MO_ADR,csSrcDBPath,recCount);

	if (recCount > 0) {
		record = new ContactsRecord[recCount];
		xLoadContact(record,szProductPath,csSrcDBPath,recCount);

		//Save
		ret = xSaveContact(record,szProductPath,csTgtDBPath,recCount);

		delete [] record;
	} //if recCount > 0

	return ret;
}

CAMMError CCAMMSync::xCopyNote(CString szProductPath,CString csSrcDBPath,CString csTgtDBPath)
{
	CAMMError ret = CAMM_ERROR_SUCCESS;
	//Load 
	int recCount = 0;
	NotesRecord *record;
//	GetRecordCount(MO_NOTE,csSrcDBPath,recCount);

	if (recCount > 0) {
		record = new NotesRecord[recCount];
		xLoadNotes(record,szProductPath,csSrcDBPath,recCount);

		//Save
		ret = xSaveNotes(record,szProductPath,csTgtDBPath,recCount);

		delete [] record;
	} //if recCount > 0

	return ret;
}

CAMMError CCAMMSync::xCopySchedule(CString szProductPath,CString csSrcDBPath,CString csTgtDBPath)
{
	CAMMError ret = CAMM_ERROR_SUCCESS;

	//Load 
	int recCount = 0;
	SchedulesRecord *record;
//	GetRecordCount(MO_SCH,csSrcDBPath,recCount);

	if (recCount > 0) {
		record = new SchedulesRecord[recCount];
		xLoadSchedule(record,szProductPath,csSrcDBPath,recCount);

		//Save
		ret = xSaveSchedule(record,szProductPath,csTgtDBPath,recCount);

		delete [] record;
	}  //if recordCount > 0

	return ret;
}
*/

//Add by peter 


int GetScheduleData( void );

int CCAMMSync::ConnectStatusCallBack(int State)
{
	theApp.m_bOpenDriverFinal = TRUE;
	if( State == Anw_MOBILE_CONNECT_FAILED )
	{
//		if (_static_bMobileConneted){
    		//Transfer the message of failure to connect mobile to the main frame.  
    		PostMessage(afxGetMainWnd()->GetSafeHwnd(), WM_MOBILECONNECTED, (DWORD)false, NULL);
	//		_static_bMobileConneted = false;
	//	}
	}else if(  State == Anw_SUCCESS )
	{
	//	if (!_static_bMobileConneted){
    		//Transfer the message of succeeding to connect mobile to the main frame.  
	    	PostMessage(afxGetMainWnd()->GetSafeHwnd(), WM_MOBILECONNECTED, (DWORD)true, NULL);
    //		_static_bMobileConneted = true;
	//	}
	}

	CMainFrame* pFrame=(CMainFrame*)afxGetMainWnd();
	//DLL CONTRUCT
	if(pFrame == NULL)
	  return 0;
	if(pFrame->m_bCapture)
	{
		SetCursor( LoadCursor(NULL, IDC_ARROW) );
		ReleaseCapture();
		pFrame->m_bCapture = FALSE;
	}
	

  return 0;
}

int GetScheduleData( void )
{
	int NeedCount = 1;
	Calendar_Data_Struct schedule_Data_Strc;
	memset(&schedule_Data_Strc, 0, sizeof(Calendar_Data_Struct));
	int RealCount = 0;
	Schedule_Tal_Num tal_num;
	int ret = ANWGetMobileScheduleInfo(&tal_num);
	ret = ANWGetScheduleStartData(NeedCount, &schedule_Data_Strc, RealCount);
	for( int i = 0; i < tal_num.ScheduleUsedNum -1; i++ )
	{
		RealCount=0;
		ret = ANWGetScheduleNextData(NeedCount, &schedule_Data_Strc, RealCount);
		if (Anw_SUCCESS!=ret) {
	    	if (Anw_MOBILE_CONNECT_FAILED == ret) {
	    		_static_bMobileConneted = false;
         		PostMessage(afxGetMainWnd()->GetSafeHwnd(), WM_MOBILECONNECTED, (DWORD)false, NULL);
			}
		//	AfxMessageBox("Lack of this Schedule-Data!!");
			return ret=0;
		}
/*		if (i==1) {
			//Test Add Schedule-Data
			schedule_Data_Strc.Index = 0;
			ret = ANWAddScheduleData(&schedule_Data_Strc);
			if (!ret) {
				AfxMessageBox("Failed to Add Schedule-Data!!");
			}else {
				AfxMessageBox("Succeed to Add Schedule-Data!!");
			}

			//Test Delete Schedule-Data
			int nIndexDeleted=0;
			nIndexDeleted = schedule_Data_Strc.Index;
			ret = ANWDeleteScheduleData(nIndexDeleted, &schedule_Data_Strc);
			if (!ret) {
				AfxMessageBox("Failed to delete Schedule-Data!!");
			}else {
				AfxMessageBox("Succeed to delete Schedule-Data!!");
               	ret = ANWGetMobileScheduleInfo(&tal_num);
			}

		}
*/
	}
//	ret = ANWAddScheduleData(&schedule_Data_Strc);
	if(ret != 1)
	{
//	    AfxMessageBox("ANWGetMobileScheduleInfo!");
		return ret;
	}
	ANWCloseCalendar();
	return ret;
}

void CCAMMSync::FreeCalendarDriver()
{
	if (m_hMODLL) {
    	FreeLibrary(m_hMODLL);
		m_hMODLL = NULL;
	}
}

bool CCAMMSync::InitCalendarDriver()
{
	bool bRet=false;

    int nRet = ANWInitCalendar();
	if (Anw_SUCCESS == nRet) {
		bRet = true;
	}

	if (Anw_MOBILE_CONNECT_FAILED == nRet) {
		_static_bMobileConneted = false;
     	PostMessage(afxGetMainWnd()->GetSafeHwnd(), WM_MOBILECONNECTED, (DWORD)false, NULL);
	}

	return bRet;
}

bool CCAMMSync::CloseCalendarDriver()
{
	bool bRet=false;
	if(!ANWCloseCalendar)
		return bRet;
	int nRet = ANWCloseCalendar();
	if (Anw_SUCCESS == nRet) {
		bRet = true;
	}

	if (Anw_MOBILE_CONNECT_FAILED == nRet) {
			_static_bMobileConneted = false;
         	PostMessage(afxGetMainWnd()->GetSafeHwnd(), WM_MOBILECONNECTED, (DWORD)false, NULL);
	}

	FreeCalendarDriver();//nono, 2004_1101

	return bRet;
}

//DLL CONTRUCT
// A Condition to Free the AnwMobile Library 
void  CCAMMSync::FreeLibrary(HMODULE m_hDriver)
{
   if(!afxContextIsDLL)
	   ::FreeLibrary(m_hDriver);
}

//下面这个函数不知道在那里被调用的，在传输数据之前，估计是底层调用了
bool CCAMMSync::LoadandOpenCalendarDriver()
{
	bool bRet=true;
//	if (!m_bOpened) {

	m_bOpened = false;

	if (m_hMODLL) FreeLibrary(m_hMODLL);
    
	// DLL CONTRUCT
	if(afxContextIsDLL)
	{
	    m_hMODLL = ((CCalendarApp*)afxGetApp())->m_hInstAnwMobile;
	}
    else
	{
       //    m_hMODLL = LoadLibrary("AnwMobile.dll");
        m_hMODLL = LoadLibrary(theApp.m_szDriverDllName);
	}


	if (!m_hMODLL) 
	{

		return bRet = false;
	}

	ANWOpenCalendar = (anwOpenCalendar)::GetProcAddress(m_hMODLL,"OpenCalendar");
	ANWCloseCalendar = (anwCloseCalendar)::GetProcAddress(m_hMODLL,"CloseCalendar");
	ANWGetMobileScheduleInfo = (anwGetMobileScheduleInfo)::GetProcAddress(m_hMODLL,"GetMobileScheduleInfo");
	ANWGetScheduleStartData = (anwGetScheduleStartData)::GetProcAddress(m_hMODLL,"GetScheduleStartData");
	ANWGetScheduleNextData = (anwGetScheduleNextData)::GetProcAddress(m_hMODLL,"GetScheduleNextData");
	ANWAddScheduleData = (anwAddScheduleData)::GetProcAddress(m_hMODLL,"AddScheduleData");
	ANWDeleteScheduleData = (anwDeleteScheduleData)::GetProcAddress(m_hMODLL,"DeleteScheduleData");
	ANWDeleteAllScheduleData = (anwDeleteAllScheduleData)::GetProcAddress(m_hMODLL,"DeleteAllScheduleData");
    ANWGetCalendarSupport = (anwGetCalendarSupport)::GetProcAddress(m_hMODLL,"GetCalendarSupport");
	ANWGetMobileDateTime = (anwGetMobileDateTime)GetProcAddress(m_hMODLL,"GetMobileDateTime");
	ANWInitialCalendarStatus = (anwInitialCalendarStatus)GetProcAddress(m_hMODLL,"InitialCalendarStatus");

//	char cBuffer[12];
//	TCHAR szPhone[MAX_PATH];
//	strcpy(szPhone,itoa(theApp.m_iPhone,cBuffer,10));
    int ret=0;
	_static_bMobileConneted = false;

	CMainFrame* pFrame=(CMainFrame*)afxGetMainWnd();
	::SetCapture(pFrame->m_hWnd);
	SetCursor( LoadCursor(NULL, IDC_WAIT) );
	pFrame->m_bCapture = TRUE;

	ret = ANWOpenCalendar(theApp.m_iMobileCompany, theApp.m_szPhone, theApp.m_szConnectMode, theApp.m_szPortName, _tcslen(theApp.m_szIMEI) >0 ? theApp.m_szIMEI :NULL, CCAMMSync::ConnectStatusCallBack);

	if(ret != Anw_SUCCESS)
	{
		xErrorHandling(ret);
		return false;
	}

    //Get infomation og Calendar-Support.
	//ANWGetCalendarSupport(&_static_SupportInfo);
	//CalendarSupportInfo _SupportInfo;
	//下面函数完成对静态结构体的初始化工作
	GetCalendarSupport();

//	CheckMobileSetDateTime();
/*
	if (ret == Anw_SUCCESS) {
		m_bOpened=true;
		Schedule_Tal_Num tal_num;
     	int nRet = ANWGetMobileScheduleInfo(&tal_num);
	}*/
//	}
	

	return bRet;
}

BOOL CCAMMSync::CheckMobileSetDateTime()
{
	if(theApp.m_bMobileSetDateTime == FALSE )
	{
		if(ANWGetMobileDateTime)
		{
			GSM_DateTime DateTime;
			int ret =ANWGetMobileDateTime(&DateTime);
			if(ret ==  Anw_SUCCESS)
			{
				if(DateTime.Year == 0)
					theApp.m_bMobileSetDateTime = FALSE;
				else
					theApp.m_bMobileSetDateTime = TRUE;

			}
		}
		else theApp.m_bMobileSetDateTime = TRUE;
	}
	return theApp.m_bMobileSetDateTime;
}

/*
typedef enum {
		Type_Start	= 1,	      
		Type_Continune,
		Type_End,
}Control_Type;
*/
int CCAMMSync::AddCalendar(CScheduleInfo& Record,Control_Type& nType,SchedulesRecord* pSR)
{
	
	int nRet=1, nErrorCode=0;
	CString msg(_T(""));
	if(nType == Type_End)
	{
		ANWAddScheduleData(&_Calendar_Data_Strc_,Type_End);
		return true;
	}
#ifdef _DEBUG
	Record.DumpScheduleConntent();
#endif

	nRet = ConvertPCDataToMobile(Record, nErrorCode);
	
    
//#ifdef _DEBUG
	DumpMobileCalendar(_Calendar_Data_Strc_);
//#endif
	if (!nRet || !nErrorCode) 
	{
		if ( CheckMobileSetDateTime() == FALSE)
		{
			CalendarMessageBox(afxGetMainWnd()->m_hWnd,_T("MSG_ERROR_MOBILETIME"));
			return false;
		}
	   //DLL CONTRUCT
	   if(afxContextIsDLL)
	      ::SetCursor(LoadCursor(NULL, IDC_WAIT));
	   else 
	      CWaitCursor ww;

///////////////
	CalendarSupportInfo SupportInfo;
	GetCalendarSupport(SupportInfo);


	if(SupportInfo.Title_Boundry > 0 || SupportInfo.Location_Boundry >0)
	{
		int nTitleBytes = SupportInfo.Title_Boundry;
		int nLocationBytes = SupportInfo.Location_Boundry ;

#ifdef _UNICODE
		if(SupportInfo.CharType == 1)
		{
			USES_CONVERSION;
			int nTextLen =  _tcslen(_Calendar_Data_Strc_.Text);
			int nLocationLen =  _tcslen(_Calendar_Data_Strc_.Location);

			char* pszText = new char[(nTextLen+1)*2];
			char* pszLocation = new char[(nLocationLen+1)*2];
			sprintf(pszText,"%s",W2A(_Calendar_Data_Strc_.Text));
			sprintf(pszLocation,"%s",W2A(_Calendar_Data_Strc_.Location));

			int nAnsiTextLen = strlen(pszText);
			int nAnsiLocationLen = strlen(pszLocation);

			
			if(nTitleBytes > 0 && nAnsiTextLen > nTitleBytes)
			{
				pszText[nTitleBytes] ='\0'; 
				swprintf(_Calendar_Data_Strc_.Text,_T("%s"),A2W(pszText));
			}
			if(nLocationBytes > 0 && nAnsiLocationLen> nLocationBytes)
			{
				pszLocation[nLocationBytes] ='\0';
				swprintf(_Calendar_Data_Strc_.Location,_T("%s"),A2W(pszLocation));
			}
			delete[] pszText;
			delete[] pszLocation;

		}
		else
		{
			if(wcslen(_Calendar_Data_Strc_.Text) > nTitleBytes)
				_Calendar_Data_Strc_.Text[nTitleBytes] = '\0';
			if(wcslen(_Calendar_Data_Strc_.Location) > nLocationBytes)
				_Calendar_Data_Strc_.Location[nLocationBytes] = '\0';
		}
#else
		if(SupportInfo.CharType == 1)
		{
			if(nTitleBytes > 0 && _tcslen(_Calendar_Data_Strc_.Text) > nTitleBytes)
				_Calendar_Data_Strc_.Text[nTitleBytes] ='\0'; 
			if(nLocationBytes > 0 && _tcslen(_Calendar_Data_Strc_.Location)> nLocationBytes)
				_Calendar_Data_Strc_.Location[nLocationBytes] ='\0';

		}
		else
		{
			int nTitleBytes = SupportInfo.Title_Boundry;
			int nLocationBytes = SupportInfo.Location_Boundry ;
			WCHAR				szLocationW[_MAX_PATH*2];
			WCHAR				szTextW[_MAX_PATH*2];
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, _Calendar_Data_Strc_.Location, -1,
								szLocationW, MAX_PATH*2);
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, _Calendar_Data_Strc_.Text, -1,
								szTextW, MAX_PATH*2);
			if(wcslen(szTextW) > nTitleBytes)
				szTextW[nTitleBytes] = '\0';
			if(wcslen(szLocationW) > nLocationBytes)
				szLocationW[nLocationBytes] = '\0';
			WideCharToMultiByte( CP_ACP, 0, szLocationW , -1, _Calendar_Data_Strc_.Location, sizeof(_Calendar_Data_Strc_.Location), NULL, NULL );
			WideCharToMultiByte( CP_ACP, 0, szTextW , -1, _Calendar_Data_Strc_.Text, sizeof(_Calendar_Data_Strc_.Text), NULL, NULL );
		}
#endif
	}/////////
/////////////////
     	nRet = ANWAddScheduleData(&_Calendar_Data_Strc_,nType);
		
		if(nType == Type_Start)
			nType = Type_Continune;
//#ifdef _DEBUG
	DumpMobileCalendar(_Calendar_Data_Strc_);
//#endif
	
    	if (Anw_SUCCESS != nRet) 
		{
			int nReturn = nRet;
	    	if (Anw_MOBILE_CONNECT_FAILED == nRet)
			{
	    		_static_bMobileConneted = false;
         		PostMessage(afxGetMainWnd()->GetSafeHwnd(), WM_MOBILECONNECTED, (DWORD)false, NULL);
				nReturn = false;
			}
			else if(Anw_ALLOCATE_MEMORY_FAILED == nRet || Anw_TRIAL_LIMITEDFUNCTION == nRet)
				nReturn = false;
			else 
				nReturn = -1;
			ANWAddScheduleData(&_Calendar_Data_Strc_,Type_End);
		
			bool bshowMsg = true;
			if(theApp.m_iMobileCompany == CO_LG && nRet == Anw_INVALIDDATA)
			{
				GSM_DateTime DateTime;
				int ret =ANWGetMobileDateTime(&DateTime);
				if(ret ==  Anw_SUCCESS)
				{
					if(DateTime.Year != 0)
					{
						COleDateTime dtPhone,dtStart;
						dtStart.SetDateTime(_Calendar_Data_Strc_.Start_DateTime.Year, _Calendar_Data_Strc_.Start_DateTime.Month,_Calendar_Data_Strc_.Start_DateTime.Day,
							_Calendar_Data_Strc_.Start_DateTime.Hour,_Calendar_Data_Strc_.Start_DateTime.Minute,0);
						dtPhone.SetDateTime(DateTime.Year, DateTime.Month,DateTime.Day,
							DateTime.Hour,DateTime.Minute,0);

						if(dtPhone > dtStart)
						{
							CalendarMessageBox(afxGetMainWnd()->m_hWnd,_T("IDS_ERR_STARTTIME_INVALIDATE"));
							bshowMsg = false;
						}
					}
				}

			}
		
		
		
			
		     
			if(bshowMsg)
			{
			xErrorHandling(nRet);
			bshowMsg=false;////081027libaoliu



			}
		
			
			nType = Type_Start;
			nRet = nReturn;
		}
		else
		{
			if(pSR)
			{
				ConvertMoblieDataToPC(_Calendar_Data_Strc_,pSR,nErrorCode);
			}
			sprintf(Record.szuiIndex,_Calendar_Data_Strc_.szIndex);
		}
        
	}
	else 
		nRet = -1;
	
	return nRet;
}
//可能是向低层赋值
int CCAMMSync::AddCalendar(CScheduleInfo& Record)
{

	int nRet=1, nErrorCode=0;
//	int nRecordCount=GetCalendarCount();
	CString msg(_T(""));

	

	//_Calendar_Data_Strc_.Index = 0;
	//nRet = ANWAddScheduleData(&_Calendar_Data_Strc_);
#ifdef _DEBUG
	Record.DumpScheduleConntent();
#endif

// 	CString stx1;
// 	stx1.Format(L"看看dayofweekmask的值（之前）  %d",Record.nDayofWeekMask );
// 	AfxMessageBox(stx1);

	nRet = ConvertPCDataToMobile(Record, nErrorCode);

//#ifdef _DEBUG
	DumpMobileCalendar(_Calendar_Data_Strc_);
	
//#endif
	if (!nRet || !nErrorCode) 
	{
		if ( CheckMobileSetDateTime() == FALSE)
		{
			CalendarMessageBox(afxGetMainWnd()->m_hWnd,_T("MSG_ERROR_MOBILETIME"));
			return false;
		}
	   //DLL CONTRUCT
	   if(afxContextIsDLL)
	      ::SetCursor(LoadCursor(NULL, IDC_WAIT));
	   else 
	      CWaitCursor ww;
	    
//  	   CString st;
//  	   st.Format(L"看看start time的值（之前）  %d-%d-%d  %d:%d",
// 		   _Calendar_Data_Strc_.Start_DateTime.Year,
// 		   _Calendar_Data_Strc_.Start_DateTime.Month,
// 		   _Calendar_Data_Strc_.Start_DateTime.Day,
// 		   _Calendar_Data_Strc_.Start_DateTime.Hour, 
// 		   _Calendar_Data_Strc_.Start_DateTime.Minute);
//  	   AfxMessageBox(st);
	  
       
// 	    CString stx;
// 	    stx.Format(L"看看dayofweekmask的值（之中）  %d",_Calendar_Data_Strc_.Repeat_DayOfWeekMask);
// 	    AfxMessageBox(stx);

     	nRet = ANWAddScheduleData(&_Calendar_Data_Strc_,Type_Start);
		ANWAddScheduleData(&_Calendar_Data_Strc_,Type_End);


	    //CString st;
// 		st.Format(L"看看start time的值（之后）  %d-%d-%d  %d:%d",
// 			_Calendar_Data_Strc_.Start_DateTime.Year,
// 			_Calendar_Data_Strc_.Start_DateTime.Month,
// 			_Calendar_Data_Strc_.Start_DateTime.Day,
// 			_Calendar_Data_Strc_.Start_DateTime.Hour, 
// 			_Calendar_Data_Strc_.Start_DateTime.Minute);
//  	   AfxMessageBox(st);

// 		CString st;
// 		st.Format(L"看看dayofweekmask的值（之后）  %d",_Calendar_Data_Strc_.Repeat_DayOfWeekMask);
// 		AfxMessageBox(st);


//#ifdef _DEBUG
	DumpMobileCalendar(_Calendar_Data_Strc_);
//#endif
    	if (Anw_SUCCESS != nRet) 
		{
	    	if (Anw_MOBILE_CONNECT_FAILED == nRet) {
	    		_static_bMobileConneted = false;
         		PostMessage(afxGetMainWnd()->GetSafeHwnd(), WM_MOBILECONNECTED, (DWORD)false, NULL);
			}
			bool bshowMsg = true;
			if(theApp.m_iMobileCompany == CO_LG && nRet == Anw_INVALIDDATA)
			{
				GSM_DateTime DateTime;
				int ret =ANWGetMobileDateTime(&DateTime);
				if(ret ==  Anw_SUCCESS)
				{
					if(DateTime.Year != 0)
					{
						COleDateTime dtPhone,dtStart;
						dtStart.SetDateTime(_Calendar_Data_Strc_.Start_DateTime.Year, _Calendar_Data_Strc_.Start_DateTime.Month,_Calendar_Data_Strc_.Start_DateTime.Day,
							_Calendar_Data_Strc_.Start_DateTime.Hour,_Calendar_Data_Strc_.Start_DateTime.Minute,0);
						dtPhone.SetDateTime(DateTime.Year, DateTime.Month,DateTime.Day,
							DateTime.Hour,DateTime.Minute,0);

						if(dtPhone > dtStart)
						{
							CalendarMessageBox(afxGetMainWnd()->m_hWnd,_T("IDS_ERR_STARTTIME_INVALIDATE"));
							bshowMsg = false;
						}
					}
				}

			}
		

			if(bshowMsg) xErrorHandling(nRet);
		//	xErrorHandling(nRet);
			nRet = false;
		}
		else
		{
		/*	if (nRecordCount == GetCalendarCount())
			{
				CalendarMessageBox(afxGetMainWnd()->m_hWnd,"MSG_ERROR_ADDFAIL");
				msg.ReleaseBuffer();
		    	nRet = false;
			}
			else*/
			{
//	    		Record.uiIndex = _Calendar_Data_Strc_.Index;
				SchedulesRecord sRecord;
				ConvertMoblieDataToPC(_Calendar_Data_Strc_,&sRecord,nErrorCode);
// 				CString st;
// 				st.Format(L"看看repeattyep的值（ConvertMoblieDataToPC之后）  %d",_Calendar_Data_Strc_.RecurrenceType);
//  				AfxMessageBox(st);

				Record.SetData(sRecord);

// 				CString st;
// 				st.Format(L"看看repeattyep的值（setdata之后）  %d",Record.repeatType);
// 	  			AfxMessageBox(st);

				sprintf(Record.szuiIndex,_Calendar_Data_Strc_.szIndex);
			}
		}
	}else {
		nRet = false;
	}
	//AfxMessageBox(TranslateErrorCode(nRet));

	


	return nRet;
}

int CCAMMSync::EditCalendar(CScheduleInfo& Record)
{
	int nRet=1, nErrorCode=0;

	//DLL CONTRUCT
	if(afxContextIsDLL)
	   ::SetCursor(LoadCursor(NULL, IDC_WAIT));
	else 
	   CWaitCursor ww;


	//_Calendar_Data_Strc_.Index = 0;
//#ifdef _DEBUG
	Record.DumpScheduleConntent();
//#endif
	nRet = ConvertPCDataToMobile(Record, nErrorCode);
//#ifdef _DEBUG
	DumpMobileCalendar(_Calendar_Data_Strc_);
//#endif
	if (!nRet || !nErrorCode) {
		//Firstly, Delete the original Calendar in mobile.
		nRet = DeleteCalendar(Record,Type_Start);
		if(nRet ==0) return nRet;
	//	int nIndexDeleted = (int)Record.uiIndex;
		int ret=0;
    		//And then, Add the revised Calendar to mobile.
       	nRet = ANWAddScheduleData(&_Calendar_Data_Strc_,Type_Continune);
        	if (Anw_SUCCESS != nRet) 
			{
	        	if (Anw_MOBILE_CONNECT_FAILED == nRet)
				{
	        		_static_bMobileConneted = false;
         	     	PostMessage(afxGetMainWnd()->GetSafeHwnd(), WM_MOBILECONNECTED, (DWORD)false, NULL);
				}
				nRet = DeleteCalendar(Record,Type_End);
 				xErrorHandling(nRet);
        		nRet = 0;
			}
			else 
			{
//	    		Record.uiIndex = _Calendar_Data_Strc_.Index;
				sprintf(Record.szuiIndex,_Calendar_Data_Strc_.szIndex);
				nRet = DeleteCalendar(Record,Type_End);
			}

//		}else  {
//			nRet = 0;
//		}
	}else {
		nRet = 0;
	}

	

	return nRet;
}

int CCAMMSync::DeleteCalendar(CScheduleInfo& Record,int Control)
{
	int nRet=1, nErrorCode=0;
	if(Control == Type_End)
	{
		ANWDeleteScheduleData("0",NULL,Type_End);
		return true;
	}
	//_Calendar_Data_Strc_.Index = 0;
	nRet = ConvertPCDataToMobile(Record, nErrorCode);
//#ifdef _DEBUG
		DumpMobileCalendar(_Calendar_Data_Strc_);
//#endif
	if (!nRet || !nErrorCode) 
	{
		//Firstly, Delete the original Calendar in mobile.
	//	int nIndexDeleted = (int)Record.uiIndex;
		int ret=0;

// 		CString st = Record.szuiIndex;
// 		AfxMessageBox(L"准备删除 索引是多少？");
// 		AfxMessageBox(st);
       
		ret = ANWDeleteScheduleData(Record.szuiIndex, &_Calendar_Data_Strc_,Control);
    	if (Anw_MOBILE_DELETE_FAILED == ret || Anw_SUCCESS != ret) 
		{
			int nReturn = ret;
	       	if (Anw_MOBILE_CONNECT_FAILED == ret) 
			{
	       		_static_bMobileConneted = false;
         		PostMessage(afxGetMainWnd()->GetSafeHwnd(), WM_MOBILECONNECTED, (DWORD)false, NULL);
				nReturn = false;
			}
			else if(Anw_ALLOCATE_MEMORY_FAILED == ret || Anw_TRIAL_LIMITEDFUNCTION == ret|| Anw_MOBILE_BUSY == ret)
				nReturn = false;
			else 
				nReturn = -1;
			ANWDeleteScheduleData("0",NULL,Type_End);
			if(Anw_ALLOCATE_MEMORY_FAILED == ret || Anw_TRIAL_LIMITEDFUNCTION == ret || Anw_MOBILE_BUSY == ret)
			{
				xErrorHandling(ret);
			}
			else
			{
				CString msg = LoadStringFromFile(_T("message"),_T("MSG_ERROR_DELETEFAIL"));
				CString Errmsg;
				Errmsg.Format(msg,_Calendar_Data_Strc_.Text);


			//	msg.Format("Failed to delete the Schedule-Data selected!! [Index of item deleted: %d ;ErrorCode: %d ]", nIndexDeleted, ret);
			//	AfxMessageBox(msg);
			
				CalendarStringMessageBox(afxGetMainWnd()->m_hWnd,Errmsg);
			}

         	nRet = nReturn;
		} 
	}
	else 
	{
		nRet = -1;
	}
	return nRet;
}

void DumpMobileCalendar(Calendar_Data_Struct CalendarDataFromMobile)
{
/*
	CString cuTemp(_T(""));

	// 1. ** GSM_DateTime Start_DateTime
	ParseGSMDateTime(CalendarDataFromMobile.Start_DateTime, cuTemp);

	// 2. ** GSM_DateTime End_DateTime
	ParseGSMDateTime(CalendarDataFromMobile.End_DateTime, cuTemp);

	// 3. ** GSM_DateTime Alarm_DateTime
	ParseGSMDateTime(CalendarDataFromMobile.Alarm_DateTime, cuTemp);

	// 4. ** GSM_DateTime Silent_Alarm_DateTime
	ParseGSMDateTime(CalendarDataFromMobile.Silent_Alarm_DateTime, cuTemp);

	// 5. ** int NodeType

	// 6. ** char Location[514]


    // 9. ** bool Recurrence;

    //10. ** int Private;

    //11. ** int ContactID;

    //18. ** GSM_DateTime Repeat_StartDate;
	ParseGSMDateTime(CalendarDataFromMobile.Repeat_StartDate, cuTemp);

    //19. ** GSM_DateTime Repeat_EndDate;
	ParseGSMDateTime(CalendarDataFromMobile.Repeat_EndDate, cuTemp);
*/

}

CString TranslateErrorCode(int nErrorCode)
{
	CString csOutput(_T(""));
	
	if (Anw_SUCCESS == nErrorCode) {
		csOutput = "[Success]";
	}else if (Anw_MOBILE_BUSY == nErrorCode) {
		csOutput = "[MOBILE Busy]";
	}else if (Anw_TIMEOUT == nErrorCode) {
		csOutput = "[Time Out]";
	}else if (Anw_MOBILE_CONNECT_FAILED == nErrorCode) {
		csOutput = "[Mobile Connect Failed]";
	}else if (Anw_MOBILE_WRITE_FAILED == nErrorCode) {
		csOutput = "[Mobile WRITE Failed]";
	}else if (Anw_MOBILE_READ_FAILED == nErrorCode) {
		csOutput = "[Mobile READ Failed]";
	}else if (Anw_MOBILE_DELETE_FAILED == nErrorCode) {
		csOutput = "[Mobile DELETE Failed]";
	}else if (Anw_PARAMETER_CHECK_ERROR == nErrorCode) {
		csOutput = "[Unknown connection type given by user]";
	}else if (Anw_ALLOCATE_MEMORY_FAILED == nErrorCode) {
		csOutput = "[Allocate Memory Failed]";
	}else if (Anw_NOT_LINK_DRV == nErrorCode) {
		csOutput = "[Can't LINK DRIVER]";
	}else if (Anw_NOTSUPPORTED == nErrorCode) {
		csOutput = "[Not supported by phone]";
	}else if (Anw_EMPTY == nErrorCode) {
		csOutput = "[Empty phonebook entry, ...]";
	}else if (Anw_CANTOPENFILE == nErrorCode) {
		csOutput = "[Error during opening file]";
	}else if (Anw_FILENOTSUPPORTED == nErrorCode) {
		csOutput = "[File format not supported by SyncDrv]";
	}else if (Anw_INVALIDDATETIME == nErrorCode) {
		csOutput = "[Invalid date/time]";
	}else if (Anw_Phone_MEMORY == nErrorCode) {
		csOutput = "[Phone memory error, maybe it is read only]";
	}else if (Anw_INVALIDDATA == nErrorCode) {
		csOutput = "[Invalid data]";
	}else if (Anw_NOT_LINK_MBdrv == nErrorCode) {
		csOutput = "[Can't LINK MBdrv.DLL.]";
	}else if (Anw_FileName_FAILED == nErrorCode) {
		csOutput = "[File Name FAILED.]";
	}else if (Anw_Thread_Terminate == nErrorCode) {
		csOutput = "[Anw_Thread_Terminate]";
	}else if (Anw_UNKNOW_ERROR == nErrorCode) {
		csOutput = "[unknow error]";
	}else {
		csOutput = "[None matched error-code!]";
	}


	return csOutput;
}

void CCAMMSync::DATEtoGSMTime(COleDateTime oleDateTime, GSM_DateTime &gsmtime)
{
	gsmtime.Second = oleDateTime.GetSecond();
	gsmtime.Minute = oleDateTime.GetMinute();
	gsmtime.Hour = oleDateTime.GetHour();
	gsmtime.Day = oleDateTime.GetDay();
	gsmtime.Month = oleDateTime.GetMonth();
	gsmtime.Year = oleDateTime.GetYear();
}

void CCAMMSync::GSMTimetoDATE(GSM_DateTime gsmtime, COleDateTime &date)
{
	COleDateTime oleDateTime(gsmtime.Year,gsmtime.Month,gsmtime.Day,gsmtime.Hour,gsmtime.Minute,gsmtime.Second);
	date=oleDateTime;
}
void CCAMMSync::xErrorHandling(int iErr)
 {
	if(iErr == Anw_SUCCESS)
		return;
	CString strMsgKeyID;
	switch(iErr)
	{
	case Anw_TRIAL_LIMITEDFUNCTION:
		{
			TCHAR szText[MAX_PATH];
			al_GetSettingString(_T("public"),_T("IDS_TITLE"),theApp.m_szRes,szText);
			BusyMsgDlg(theApp.m_pMainWnd->m_hWnd,szText);
		}
		return ;
		break;
	case Anw_MOBILENOTRESPONSE:
		strMsgKeyID = _T("IDS_ERROR_MOBILENORESPOSE");
		break;
	case Anw_MOBILE_BUSY:
		if(theApp.m_iMobileCompany == CO_SAMSUNG || theApp.m_iMobileCompany == CO_SHARP )
		{
			TCHAR szMsg[MAX_PATH];
			if(al_GetSettingString(_T("message"),_T("IDS_ERR_OPERATIONFAIL_SAMSUNG"),theApp.m_szRes,szMsg))
				strMsgKeyID = _T("IDS_ERR_OPERATIONFAIL_SAMSUNG");
			else
				strMsgKeyID = _T("IDS_ERR_MOBILEBUSY");
		}
		else
		strMsgKeyID = _T("IDS_ERR_MOBILEBUSY");
		break;
	case Anw_TIMEOUT:
		strMsgKeyID = _T("IDS_ERR_TIMEOUT");
		break;
	case Anw_MOBILE_CONNECT_FAILED:
		strMsgKeyID = _T("IDS_ERR_CONNECTFAILED");
		break;
	case Anw_MOBILE_WRITE_FAILED:
		strMsgKeyID = _T("IDS_ERR_WRITEFAILED");
		break;
	case Anw_MOBILE_READ_FAILED:
		strMsgKeyID = _T("IDS_ERR_READFAILED");
		break;
	case Anw_MOBILE_DELETE_FAILED:
		strMsgKeyID = _T("IDS_ERR_DELFAILED");
		break;
	case Anw_ALLOCATE_MEMORY_FAILED:
		strMsgKeyID = _T("IDS_ERR_ALLOCATEMEMORY");
		break;
	case Anw_NOT_LINK_MBdrv:
		strMsgKeyID = _T("IDS_ERR_NOTLINKDLL");
		break;
	case Anw_FileName_FAILED:
		strMsgKeyID = _T("IDS_ERR_FILENAMEFAILED");
		break;
	case Anw_Thread_Terminate:
		strMsgKeyID = _T("IDS_ERR_THREADTERMINATE");
		break;
	case Anw_NOTSUPPORTED:
		strMsgKeyID = _T("Anw_NOTSUPPORTED");
		break;
	case Anw_EMPTY:
		strMsgKeyID = _T("Anw_EMPTY");
		break;
	case Anw_CANTOPENFILE:
		strMsgKeyID = _T("Anw_CANTOPENFILE");
		break;
	case Anw_FILENOTSUPPORTED:
		strMsgKeyID = _T("Anw_FILENOTSUPPORTED");
		break;
	case Anw_INVALIDDATETIME:
		strMsgKeyID = _T("Anw_INVALIDDATETIME");
		break;
	case Anw_Phone_MEMORY:
		strMsgKeyID = _T("Anw_Phone_MEMORY");
		break;
	case Anw_INVALIDDATA:
		strMsgKeyID = _T("Anw_INVALIDDATA");
		break;
//	case Anw_UNKNOW_ERROR:
	default:
		strMsgKeyID = _T("IDS_ERR_UNKNOW");
		break;
	}
//	AfxMessageBox(szMsg);
	CalendarMessageBox(afxGetMainWnd()->m_hWnd,strMsgKeyID);

}

void AnaString(LPTSTR szString,CStringList *pstrlist )
{
	CString strall(szString);
	int  i, j;
	TCHAR szStr[128];
	strall +=',';
	pstrlist->RemoveAll();

	for( i = 0, j = 0; i < strall.GetLength(); i++ )
	{
	    if (strall[i] == ',')
	    {
			szStr[j] = '\0';
		//	if(strlen(szStr)>0)
			pstrlist->AddTail(szStr);
			j = 0;
	    }
	    else
			szStr[j++] = strall[i];
	}

}
BOOL CCAMMSync::GetNoteTypeSupportInfo(int nNoteType,NoteTypeSupportInfo *pSupportInfo)
{
	//合成路径 skin\default\calendar.ini
	TCHAR szProfile[MAX_PATH];
	wsprintf(szProfile, _T("%s%sAddDlg_%s.ini"), theApp.m_szSkin, _T("Calendar\\"),theApp.m_szMobileName_ori);
// 	CString strll;
// 	strll.Format(L"%s", theApp.m_szMobileName_ori);
// 	AfxMessageBox(strll);
	if(al_IsFileExist(szProfile) == FALSE)
	{
		CString str = theApp.m_szMobileName_ori;
		int nth = str.Find(' ');
		CString strCompanyName = str.Left(nth);
		wsprintf(szProfile, _T("%s%sAddDlg_%s.ini"), theApp.m_szSkin, _T("Calendar\\"),strCompanyName);
		if(strCompanyName.CompareNoCase(_T("Nokia")) == 0 && _tcsncicmp(theApp.m_szConnectMode,_T("SYNCML"),6) == 0)
		{
			wsprintf(szProfile, _T("%s%sAddDlg_%sS60.ini"), theApp.m_szSkin, _T("Calendar\\"),strCompanyName);
		}
		if(al_IsFileExist(szProfile) == FALSE)
			wsprintf(szProfile, _T("%s%s"), theApp.m_szSkin, _T("Calendar\\AddDlg.ini"));
	}

	if(al_IsFileExist(szProfile) == FALSE)
		return 0;

	TCHAR szReference[MAX_PATH];
	if(al_GetSettingString(_T("Dlg"),_T("ReferenceTO"),szProfile,szReference))
	{
		wsprintf(szProfile,_T("%sCalendar\\%s"), theApp.m_szSkin,szReference);
	}


	TCHAR szSection[MAX_PATH];
	TCHAR szDefaultSection[MAX_PATH];
	//下面代码根据notetype来确定搜索的段名称 lang\calendar.ini
	//如果要增加支持的类型，要在上面的ini中修改之
	pSupportInfo->nNodeType = nNoteType;
	switch(nNoteType)
	{
	case GSM_CAL_REMINDER:
		wsprintf(szSection, _T("Reminder"));
		break;
	case GSM_CAL_CALL:
		wsprintf(szSection, _T("Call"));
		break;
	case GSM_CAL_MEETING:
		wsprintf(szSection, _T("Meeting"));
		break;
	case GSM_CAL_BIRTHDAY:
		wsprintf(szSection, _T("Birthday"));	//use for Anniversary in 9@9u
		break;
	case GSM_CAL_MEMO:
		wsprintf(szSection, _T("Memo"));		//use for date in 9@9u
		break;
	case GSM_CAL_TRAVEL:
		//wsprintf(szSection, _T("Travel"));
		wsprintf(szSection, _T("Course"));		//kerm change for 9@9u 
		break;
	case GSM_CAL_VACATION:
		wsprintf(szSection, _T("Vacation"));
		break;
	case GSM_CAL_T_ATHL:
		wsprintf(szSection, _T("Athl"));
		break;
	case GSM_CAL_T_CYCL:
		wsprintf(szSection, _T("Cycl"));
		break;
	case GSM_CAL_T_BUDO:
		wsprintf(szSection, _T("Budo"));
		break;
	case GSM_CAL_T_DANC:
		wsprintf(szSection, _T("Danc"));
		break;
	case GSM_CAL_T_EXTR:
		wsprintf(szSection, _T("Extr"));
		break;
	case GSM_CAL_T_FOOT:
		wsprintf(szSection, _T("Foot"));
		break;
	case GSM_CAL_T_GOLF:
		wsprintf(szSection, _T("Golf"));
		break;
	case GSM_CAL_T_GYM:
		wsprintf(szSection, _T("GYM"));
		break;
	case GSM_CAL_T_HORS:
		wsprintf(szSection, _T("Hors"));
		break;
	case GSM_CAL_T_HOCK:
		wsprintf(szSection, _T("Hock"));
		break;
	case GSM_CAL_T_RACE:
		wsprintf(szSection, _T("Race"));
		break;
	case GSM_CAL_T_RUGB:
		wsprintf(szSection, _T("Rugb"));
		break;
	case GSM_CAL_T_SAIL:
		wsprintf(szSection, _T("sail"));
		break;
	case GSM_CAL_T_STRE:
		wsprintf(szSection, _T("Stre"));
		break;
	case GSM_CAL_T_SWIM:
		wsprintf(szSection, _T("swim"));
		break;
	case GSM_CAL_T_TENN:
		wsprintf(szSection, _T("Tenn"));
		break;
	case GSM_CAL_T_TRAV:
		wsprintf(szSection, _T("Trav"));
		break;
	case GSM_CAL_T_WINT:
		wsprintf(szSection, _T("Wint"));
		break;
	case GSM_CAL_ALARM:
		wsprintf(szSection, _T("Alarm"));
		break;
	case GSM_CAL_DAILY_ALARM:
		wsprintf(szSection, _T("DailyAlarm"));
		break;
	default:
		wsprintf(szSection, _T("GeneralType"));
		break;
	}
	//默认的段名
	wsprintf(szDefaultSection, _T("GeneralType"));

	int bSupport = 0;
	pSupportInfo->dwSupportFlag = 0 ;
                  
	if(al_GetSettingInt(szSection, _T("text1"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000001;
	}
	else if(al_GetSettingInt(szDefaultSection, _T("text1"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000001;
	}

	if(al_GetSettingInt(szSection, _T("text2"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000002;
	}
	else if(al_GetSettingInt(szDefaultSection, _T("text2"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000002;
	}

	if(al_GetSettingInt(szSection,_T("startDate"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000004;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("startDate"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000004;
	}

	if(al_GetSettingInt(szSection,_T("startTime"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000008;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("startTime"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000008;
	}

	if(al_GetSettingInt(szSection,_T("endDate"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000010;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("endDate"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000010;
	}

	if(al_GetSettingInt(szSection,_T("endTime"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000020;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("endTime"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000020;
	}

	if(al_GetSettingInt(szSection,_T("alarmDate"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000040;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("alarmDate"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000040;
	}

	if(al_GetSettingInt(szSection,_T("alarmTime"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000080;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("alarmTime"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000080;
	}

	if(al_GetSettingInt(szSection,_T("alarmTone"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000100;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("alarmTone"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000100;
	}

	if(al_GetSettingInt(szSection,_T("repeat"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000200;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("repeat"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000200;
	}

	if(al_GetSettingInt(szSection,_T("repeatEndDate"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000400;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("repeatEndDate"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000400;
	}

	if(al_GetSettingInt(szSection,_T("alarmLeadTime"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000800;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("alarmLeadTime"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00000800;
	}

	if(al_GetSettingInt(szSection,_T("text3"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00001000;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("text3"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00001000;
	}

	if(al_GetSettingInt(szSection,_T("repeatdayofweekmask"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00002000;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("repeatdayofweekmask"),szProfile,bSupport))
	{
		if(bSupport) pSupportInfo->dwSupportFlag |= 0x00002000;
	}



	if(al_GetSettingInt(szSection,_T("bPhoneNo_Text2"),szProfile,bSupport))
	{
		pSupportInfo->bPhoneNoText2 = bSupport;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("bPhoneNo_Text2"),szProfile,bSupport))
	{
		pSupportInfo->bPhoneNoText2 = bSupport;
	}

	if(al_GetSettingInt(szSection,_T("repeatDefaultType"),szProfile,bSupport))
	{
		pSupportInfo->nRepeatdefaulType = bSupport;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("repeatDefaultType"),szProfile,bSupport))
	{
		pSupportInfo->nRepeatdefaulType = bSupport;
	}

	if(al_GetSettingInt(szSection,_T("EndTimeIntervalMax"),szProfile,bSupport))
	{
		pSupportInfo->nEndTimeIntervalMax = bSupport;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("EndTimeIntervalMax"),szProfile,bSupport))
	{
		pSupportInfo->nEndTimeIntervalMax = bSupport;
	}

	if(al_GetSettingInt(szSection,_T("bDisableEndDateWhenRepeat"),szProfile,bSupport))
	{
		pSupportInfo->bDisableEndDateWhenRepeat = bSupport;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("bDisableEndDateWhenRepeat"),szProfile,bSupport))
	{
		pSupportInfo->bDisableEndDateWhenRepeat = bSupport;
	}
	else pSupportInfo->bDisableEndDateWhenRepeat = 0;

	if(al_GetSettingInt(szSection,_T("AlarmTimeIntervalMax"),szProfile,bSupport))
	{
		pSupportInfo->nAlarmTimeIntervalMax = bSupport;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("AlarmTimeIntervalMax"),szProfile,bSupport))
	{
		pSupportInfo->nAlarmTimeIntervalMax = bSupport;
	}
//////////
	if(al_GetSettingInt(szSection,_T("AlarmTimeIntervalMax_Hour"),szProfile,bSupport))
	{
		pSupportInfo->nAlarmTimeIntervalMax_Hour = bSupport;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("AlarmTimeIntervalMax_Hour"),szProfile,bSupport))
	{
		pSupportInfo->nAlarmTimeIntervalMax_Hour = bSupport;
	}
	else
		pSupportInfo->nAlarmTimeIntervalMax_Hour=pSupportInfo->nAlarmTimeIntervalMax;

	if(al_GetSettingInt(szSection,_T("AlarmTimeIntervalMax_Min"),szProfile,bSupport))
	{
		pSupportInfo->nAlarmTimeIntervalMax_Min = bSupport;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("AlarmTimeIntervalMax_Min"),szProfile,bSupport))
	{
		pSupportInfo->nAlarmTimeIntervalMax_Min = bSupport;
	}
	else
		pSupportInfo->nAlarmTimeIntervalMax_Min=pSupportInfo->nAlarmTimeIntervalMax;


	if(al_GetSettingInt(szSection,_T("AlarmTimeIntervalMax_Sec"),szProfile,bSupport))
	{
		pSupportInfo->nAlarmTimeIntervalMax_Sec = bSupport;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("AlarmTimeIntervalMax_Sec"),szProfile,bSupport))
	{
		pSupportInfo->nAlarmTimeIntervalMax_Sec = bSupport;
	}
	else 
		pSupportInfo->nAlarmTimeIntervalMax_Sec=pSupportInfo->nAlarmTimeIntervalMax;

	if(al_GetSettingInt(szSection,_T("AlarmTimeIntervalMax_Day"),szProfile,bSupport))
	{
		pSupportInfo->nAlarmTimeIntervalMax_Day = bSupport;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("AlarmTimeIntervalMax_Day"),szProfile,bSupport))
	{
		pSupportInfo->nAlarmTimeIntervalMax_Day = bSupport;
	}
	else 
		pSupportInfo->nAlarmTimeIntervalMax_Day=pSupportInfo->nAlarmTimeIntervalMax;

	if(al_GetSettingInt(szSection,_T("AlarmTimeIntervalMax_Week"),szProfile,bSupport))
	{
		pSupportInfo->nAlarmTimeIntervalMax_Week = bSupport;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("AlarmTimeIntervalMax_Week"),szProfile,bSupport))
	{
		pSupportInfo->nAlarmTimeIntervalMax_Week = bSupport;
	}
	else 
		pSupportInfo->nAlarmTimeIntervalMax_Week=pSupportInfo->nAlarmTimeIntervalMax;


	if(al_GetSettingInt(szSection,_T("AlarmTimeIntervalMax_Month"),szProfile,bSupport))
	{
		pSupportInfo->nAlarmTimeIntervalMax_Month = bSupport;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("AlarmTimeIntervalMax_Month"),szProfile,bSupport))
	{
		pSupportInfo->nAlarmTimeIntervalMax_Month = bSupport;
	}
	else 
		pSupportInfo->nAlarmTimeIntervalMax_Month=pSupportInfo->nAlarmTimeIntervalMax;

	if(al_GetSettingInt(szSection,_T("AlarmTimeIntervalMax_Year"),szProfile,bSupport))
	{
		pSupportInfo->nAlarmTimeIntervalMax_Year = bSupport;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("AlarmTimeIntervalMax_Year"),szProfile,bSupport))
	{
		pSupportInfo->nAlarmTimeIntervalMax_Year = bSupport;
	}
	else
		pSupportInfo->nAlarmTimeIntervalMax_Year=pSupportInfo->nAlarmTimeIntervalMax;

//////////
	if(al_GetSettingInt(szSection,_T("repeatIntervalYear"),szProfile,bSupport))
	{
		pSupportInfo->nRepeatIntervalYear = bSupport;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("repeatIntervalYear"),szProfile,bSupport))
	{
		pSupportInfo->nRepeatIntervalYear = bSupport;
	}
	else
		pSupportInfo->nRepeatIntervalYear = 1;

	if(al_GetSettingInt(szSection,_T("AlarmCheckdefault"),szProfile,bSupport))
	{
		pSupportInfo->nDefaultAlarmcheck = bSupport;
	}
	else if(al_GetSettingInt(szDefaultSection,_T("AlarmCheckdefault"),szProfile,bSupport))
	{
		pSupportInfo->nDefaultAlarmcheck = bSupport;
	}
	else
		pSupportInfo->nDefaultAlarmcheck = -1;

	GetSupportFlag(szProfile,szSection,szDefaultSection,_T("alarmLeadTimeType"),pSupportInfo->dwAlarmLeadTimeType);
	GetSupportFlag(szProfile,szSection,szDefaultSection,_T("repeatType"),pSupportInfo->dwRepeatType);
	GetSupportFlag(szProfile,szSection,szDefaultSection,_T("EndTimeIntervalType"),pSupportInfo->dwEndTimeIntervalType);
	GetSupportFlag(szProfile,szSection,szDefaultSection,_T("AlarmTimeIntervalType"),pSupportInfo->dwAlarmTimeIntervalType);

	TCHAR szTemp[MAX_PATH];
	if(al_GetSettingString(szSection,_T("stText1ID"),szProfile,szTemp))
		wsprintf(pSupportInfo->stText1ID,_T("%s"),szTemp);
	else if(al_GetSettingString(szDefaultSection,_T("stText1ID"),szProfile,szTemp))
		wsprintf(pSupportInfo->stText1ID,_T("%s"),szTemp);

	if(al_GetSettingString(szSection,_T("stText2ID"),szProfile,szTemp))
		wsprintf(pSupportInfo->stText2ID,_T("%s"),szTemp);
	else if(al_GetSettingString(szDefaultSection,_T("stText2ID"),szProfile,szTemp))
		wsprintf(pSupportInfo->stText2ID,_T("%s"),szTemp);

	if(al_GetSettingString(szSection,_T("stText3ID"),szProfile,szTemp))
		wsprintf(pSupportInfo->stText3ID,_T("%s"),szTemp);
	else if(al_GetSettingString(szDefaultSection,_T("stText3ID"),szProfile,szTemp))
		wsprintf(pSupportInfo->stText3ID,_T("%s"),szTemp);

	if(al_GetSettingString(szSection,_T("stTypeID"),szProfile,szTemp))
		wsprintf(pSupportInfo->stTypeID,_T("%s"),szTemp);
	else if(al_GetSettingString(szDefaultSection,_T("stTypeID"),szProfile,szTemp))
		wsprintf(pSupportInfo->stTypeID,_T("%s"),szTemp);

	if(al_GetSettingString(szSection,_T("icon"),szProfile,szTemp))
		wsprintf(pSupportInfo->szIconfileName,_T("%s"),szTemp);
	else if(al_GetSettingString(szDefaultSection,_T("icon"),szProfile,szTemp))
		wsprintf(pSupportInfo->szIconfileName,_T("%s"),szTemp);

	return TRUE;

}

BOOL CCAMMSync::GetSupportFlag(LPCTSTR lpProfile ,LPCTSTR lpszSection, LPCTSTR lpszDefaultSection,LPCTSTR lpszKey, DWORD &dwFlag)
{
	TCHAR szTemp[MAX_PATH];
	BOOL bGetValue = FALSE;
	TCHAR szSection[MAX_PATH];
	TCHAR szDefaultSection[MAX_PATH];
	TCHAR szKey[MAX_PATH];
	TCHAR szProfile[MAX_PATH];
	wsprintf(szSection,_T("%s"),lpszSection);
	wsprintf(szDefaultSection,_T("%s"),lpszDefaultSection);
	wsprintf(szKey,_T("%s"),lpszKey);
	wsprintf(szProfile,_T("%s"),lpProfile);

	if(al_GetSettingString(szSection,szKey,szProfile,szTemp))
		bGetValue = TRUE;
	else if(al_GetSettingString(szDefaultSection,szKey,szProfile,szTemp))
		bGetValue = TRUE;
	if(bGetValue)
	{
		dwFlag = 0;
		CStringList strlist;
		AnaString(szTemp,&strlist );
		CString str;
		POSITION pos=strlist.GetHeadPosition();
		while(pos)
		{
			str = strlist.GetNext(pos);
			int n = _ttoi(str.GetBuffer(MAX_PATH));
			str.ReleaseBuffer();
			if(n<32)
				dwFlag |=1<<n;

		}
	}
	return bGetValue;

}


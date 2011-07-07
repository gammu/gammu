#include "stdAfx.h"
#include "commfun.h"
#include "MobileInfo.h"
void _EncryptDecrypt(BYTE *pMemIn, BYTE *pMemOut, int Size)
{
	for(int i=0; i<Size; i++)
		pMemOut[i]=pMemIn[i]^(0x69^(i&0xFF));
}


BOOL ConvertFile(char*  lpInFileName,char*  lpOutFileName)
{
	CFile File_In,File_Out;
	LPBYTE lpInData,lpOutData;
	File_In.Open(lpInFileName, CFile::modeRead|CFile::typeBinary);
	File_Out.Open(lpOutFileName, CFile::modeWrite| CFile::modeCreate|CFile::typeBinary);

	int FileSize = File_In.GetLength();
	HGLOBAL hInMem = ::GlobalAlloc(GMEM_MOVEABLE, FileSize);
 	HGLOBAL hOutMem = ::GlobalAlloc(GMEM_MOVEABLE, FileSize);
   if (hInMem ==  NULL || hOutMem == NULL)
	{
		File_In.Close();
		File_Out.Close();
		return FALSE;
	}
   
   lpInData = (LPBYTE)::GlobalLock(hInMem);		
   lpOutData = (LPBYTE)::GlobalLock(hOutMem);		
   if (lpInData ==  NULL || lpOutData == NULL)
	{
		File_In.Close();
		File_Out.Close();
		::GlobalFree(hInMem);
		::GlobalFree(hOutMem);
		return FALSE;
	}
	File_In.Read((void*)lpInData, FileSize);


	_EncryptDecrypt(lpInData,lpOutData, FileSize);
	File_Out.WriteHuge(lpOutData, FileSize);
	File_In.Close();
	File_Out.Close();
	::GlobalUnlock(hInMem);
	::GlobalUnlock(hOutMem);
	::GlobalFree(hInMem);
	::GlobalFree(hOutMem);
	return TRUE ; 

}
BOOL  _IsFileExist(LPCTSTR lpszFile)
{
    ASSERT(lpszFile != NULL);
    DWORD dwAttr = ::GetFileAttributes(lpszFile);

    if (dwAttr == 0xffffffff)    return FALSE;

    return TRUE;
}
void AnaString(LPTSTR szString,CStringList *pstrlist )
{
	CString strall(szString);
	int  i, j;
	char szStr[128];
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
void GetFeature(CString szFeatures,OnePhoneModel *Mobileinfo)
{
	int nindex = 0;
	Feature	feature;
	CStringList strlist;
	AnaString(szFeatures.GetBuffer(MAX_PATH),&strlist );
	szFeatures.ReleaseBuffer();
	CString str;
	POSITION pos=strlist.GetHeadPosition();
	while(pos)
	{
		str = strlist.GetNext(pos);
		feature = F_NONE;
//		if(str.CompareNoCase("F_CAL33") == 0) feature = F_CAL33;
//		else if(str.CompareNoCase("F_CAL52") == 0) feature = F_CAL52;
//		else if(str.CompareNoCase("F_CAL82") == 0) feature = F_CAL82;
//		else if(str.CompareNoCase("F_RING_SM") == 0) feature = F_RING_SM;
//		else if(str.CompareNoCase("F_NORING") == 0) feature = F_NORING;
//		else if(str.CompareNoCase("F_NOPBKUNICODE") == 0) feature = F_NOPBKUNICODE;
//		else if(str.CompareNoCase("F_NOWAP") == 0) feature = F_NOWAP;
//		else if(str.CompareNoCase("F_NOCALLER") == 0) feature = F_NOCALLER;
//		else if(str.CompareNoCase("F_NOPICTURE") == 0) feature = F_NOPICTURE;
//		else if(str.CompareNoCase("F_NOPICTUREUNI") == 0) feature = F_NOPICTUREUNI;
//		else if(str.CompareNoCase("F_NOSTARTUP") == 0) feature = F_NOSTARTUP;
//		else if(str.CompareNoCase("F_NOCALENDAR") == 0) feature = F_NOCALENDAR;
//		else if(str.CompareNoCase("F_NOSTARTANI") == 0) feature = F_NOSTARTANI;
//		else if(str.CompareNoCase("F_POWER_BATT") == 0) feature = F_POWER_BATT;
//		else if(str.CompareNoCase("F_PROFILES33") == 0) feature = F_PROFILES33;
//		else if(str.CompareNoCase("F_PROFILES51") == 0) feature = F_PROFILES51;
//		else if(str.CompareNoCase("F_MAGICBYTES") == 0) feature = F_MAGICBYTES;
//		else if(str.CompareNoCase("F_NODTMF") == 0) feature = F_NODTMF;
//		else if(str.CompareNoCase("F_DISPSTATUS") == 0) feature = F_DISPSTATUS;
//		else if(str.CompareNoCase("F_NOCALLINFO") == 0) feature = F_NOCALLINFO;
//		else if(str.CompareNoCase("F_DAYMONTH") == 0) feature = F_DAYMONTH;
		if(str.CompareNoCase("F_PBK35") == 0) feature = F_PBK35;
		else if(str.CompareNoCase("F_PBKIMG") == 0) feature = F_PBKIMG;
	//	else if(str.CompareNoCase("F_PBKTONEGAL") == 0) feature = F_PBKTONEGAL;
	//	else if(str.CompareNoCase("F_PBKSMSLIST") == 0) feature = F_PBKSMSLIST;
		else if(str.CompareNoCase("F_PBKUSER") == 0) feature = F_PBKUSER;
	//	else if(str.CompareNoCase("F_RADIO") == 0) feature = F_RADIO;
		else if(str.CompareNoCase("F_TODO63") == 0) feature = F_TODO63;
		else if(str.CompareNoCase("F_TODO66") == 0) feature = F_TODO66;
	//	else if(str.CompareNoCase("F_NOMIDI") == 0) feature = F_NOMIDI;
	//	else if(str.CompareNoCase("F_BLUETOOTH") == 0) feature = F_BLUETOOTH;
		else if(str.CompareNoCase("F_NOFILESYSTEM") == 0) feature = F_NOFILESYSTEM;
		else if(str.CompareNoCase("F_NOMMS") == 0) feature = F_NOMMS;
		else if(str.CompareNoCase("F_NOGPRSPOINT") == 0) feature = F_NOGPRSPOINT;
		else if(str.CompareNoCase("F_CAL35") == 0) feature = F_CAL35;
		else if(str.CompareNoCase("F_CAL65") == 0) feature = F_CAL65;
		else if(str.CompareNoCase("F_WAPMMSPROXY") == 0) feature = F_WAPMMSPROXY;
		else if(str.CompareNoCase("F_VOICETAGS") == 0) feature = F_VOICETAGS;
		else if(str.CompareNoCase("F_CAL62") == 0) feature = F_CAL62;
	//	else if(str.CompareNoCase("F_NOTES") == 0) feature = F_NOTES;
		else if(str.CompareNoCase("F_PBKPUSHTALK") == 0) feature = F_PBKPUSHTALK;
		else if(str.CompareNoCase("F_SMSONLYSENT") == 0) feature = F_SMSONLYSENT;
	//	else if(str.CompareNoCase("F_BROKENCPBS") == 0) feature = F_BROKENCPBS;
		else if(str.CompareNoCase("F_M20SMS") == 0) feature = F_M20SMS;
		else if(str.CompareNoCase("F_SLOWWRITE") == 0) feature = F_SLOWWRITE;
		else if(str.CompareNoCase("F_SMSME900") == 0) feature = F_SMSME900;
	//	else if(str.CompareNoCase("F_ALCATEL") == 0) feature = F_ALCATEL;
		else if(str.CompareNoCase("F_SETSERIAL") == 0) feature = F_SETSERIAL;
		else if(str.CompareNoCase("F_ATPHONE_SMS") == 0) feature = F_ATPHONE_SMS;
		else if(str.CompareNoCase("F_ATPHONE_PHONEBOOK_ME") == 0) feature = F_ATPHONE_PHONEBOOK_ME;
		else if(str.CompareNoCase("F_ATPHONE_PHONEBOOK_SM") == 0) feature = F_ATPHONE_PHONEBOOK_SM;
		else if(str.CompareNoCase("F_NOKIA_PHONEBOOK_NOSM") == 0) feature = F_NOKIA_PHONEBOOK_NOSM;
		else if(str.CompareNoCase("F_MOTO_PHONEBOOK_2") == 0) feature = F_MOTO_PHONEBOOK_2;
		else if(str.CompareNoCase("F_MOTO_PHONEBOOK_3") == 0) feature = F_MOTO_PHONEBOOK_3;
		else if(str.CompareNoCase("F_ATPHONE_PHONEBOOK_SYNCML") == 0) feature = F_ATPHONE_PHONEBOOK_SYNCML;
		else if(str.CompareNoCase("F_NEWSERIAL40") == 0) feature = F_NEWSERIAL40;
		else if(str.CompareNoCase("F_SLOWOBEX") == 0) feature = F_SLOWOBEX;
		else if(str.CompareNoCase("F_PIM_SYNCML_SERVER") == 0) feature = F_PIM_SYNCML_SERVER;
		else if(str.CompareNoCase("F_CDMA_PHONE") == 0) feature = F_CDMA_PHONE;
		else if(str.CompareNoCase("F_SAMSUNG_PHONEBOOK_D") == 0) feature = F_SAMSUNG_PHONEBOOK_D;
		else if(str.CompareNoCase("F_ATPHONE_PHONEBOOK_WITHOUTSPACE") == 0) feature = F_ATPHONE_PHONEBOOK_WITHOUTSPACE;
		else if(str.CompareNoCase("F_CAL_DESCRIPTION") == 0) feature = F_CAL_DESCRIPTION;
		else if(str.CompareNoCase("F_SMS_SETMAXMEINDEX") == 0) feature = F_SMS_SETMAXMEINDEX;
		else if(str.CompareNoCase("F_SMS_SHIFTMEINDEX") == 0) feature = F_SMS_SHIFTMEINDEX;
		else if(str.CompareNoCase("F_ATPHONE_PHONEBOOK_WITHOUTINDEX") == 0) feature = F_ATPHONE_PHONEBOOK_WITHOUTINDEX;
		else if(str.CompareNoCase("F_MOTO_CALENDAR_2") == 0) feature = F_MOTO_CALENDAR_2;

		if(feature != F_NONE && nindex<12)
		{
			Mobileinfo->features[nindex] = feature;
			nindex++;
		}

	}
}
BOOL WINAPI IsCheckLimitedTrial(bool* checkKey ,bool* checkStack)
{
	*checkKey  = true;
	*checkStack  = true;
	TCHAR szInFileName[MAX_PATH];
	TCHAR szOutFileName[MAX_PATH];
	TCHAR szTempPath[MAX_PATH];
	TCHAR drive[_MAX_PATH], dir[_MAX_PATH], fname[_MAX_PATH], ext[_MAX_PATH];

	GetModuleFileName(AfxGetInstanceHandle(), szInFileName, _MAX_PATH);
	_splitpath(szInFileName, drive, dir, fname, ext);
	_makepath(szInFileName, drive, dir, "triset", "anw");

	GetTempPath(MAX_PATH,szTempPath);
	wsprintf(szOutFileName,"%s~triset.bin",szTempPath);

	if(_IsFileExist(szInFileName) == FALSE) return FALSE;

	if(ConvertFile(szInFileName,szOutFileName) == FALSE) return FALSE;

	if(_IsFileExist(szOutFileName) == FALSE) return FALSE;

	*checkKey =  ::GetPrivateProfileInt("Limited", "CheckLicenseKey", 1,szOutFileName);
	*checkStack =  ::GetPrivateProfileInt("Limited", "CheckStack", 1,szOutFileName);
	DeleteFile(szOutFileName);
	return TRUE;
}
BOOL GetSupportMobileInfoList(CPtrList *pSupportModelList)
{
	TCHAR szInFileName[MAX_PATH];
	TCHAR szOutFileName[MAX_PATH];
	TCHAR szTempPath[MAX_PATH];
	TCHAR drive[_MAX_PATH], dir[_MAX_PATH], fname[_MAX_PATH], ext[_MAX_PATH];

	GetModuleFileName(AfxGetInstanceHandle(), szInFileName, _MAX_PATH);
	_splitpath(szInFileName, drive, dir, fname, ext);
	_makepath(szInFileName, drive, dir, "Mobileinfo", "anw");

	GetTempPath(MAX_PATH,szTempPath);
	wsprintf(szOutFileName,"%s~Mobileinfo.bin",szTempPath);

	pSupportModelList->RemoveAll();
	if(_IsFileExist(szInFileName) == FALSE) return FALSE;

	if(ConvertFile(szInFileName,szOutFileName) == FALSE) return FALSE;

	if(_IsFileExist(szOutFileName) == FALSE) return FALSE;

	int nSupportCount =  ::GetPrivateProfileInt("Info", "Count", 0,szOutFileName);

	char szSection[MAX_PATH];
	char szFeatures[MAX_PATH];
	for(int i=1 ;i<=nSupportCount ;i++)
	{
		OnePhoneModel *pMobileinfo = new OnePhoneModel;
		wsprintf(szSection,"Mobile%d",i);
		::GetPrivateProfileString(szSection, "MBDrv_ID", "", pMobileinfo->model, MAX_PATH, szOutFileName);
		if(strlen(pMobileinfo->model)>0)
		{
			::GetPrivateProfileString(szSection, "MBDrv_Model", "", pMobileinfo->number, MAX_PATH, szOutFileName);
			::GetPrivateProfileString(szSection, "MBDrv_IrdaID", "", pMobileinfo->irdamodel, MAX_PATH, szOutFileName);
			::GetPrivateProfileString(szSection, "MBDrv_Manufacturer", "", pMobileinfo->Manufacturer, MAX_PATH, szOutFileName);
		
			::GetPrivateProfileString(szSection, "MBDrv_phone", "", pMobileinfo->PhoneDllName, MAX_PATH, szOutFileName);
			::GetPrivateProfileString(szSection, "MBDrv_Protocol", "", pMobileinfo->ProtocolDllName, MAX_PATH, szOutFileName);
			::GetPrivateProfileString(szSection, "MBDrv_Device", "", pMobileinfo->DeviceDllName, MAX_PATH, szOutFileName);

			::GetPrivateProfileString(szSection, "MBDrv_Features", "", szFeatures, MAX_PATH, szOutFileName);
		
			for(int j = 0 ;j<12 ;j++)
				pMobileinfo->features[j] = F_NONE;
			
			if(strlen(szFeatures)>0)
				GetFeature(szFeatures,pMobileinfo);
			pSupportModelList->AddTail(pMobileinfo);
		}
	}
	DeleteFile(szOutFileName);
	return TRUE;
}

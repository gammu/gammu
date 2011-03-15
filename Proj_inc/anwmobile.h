//#include "phonebook.h"
#ifndef _anwmobile_
#define _anwmobile_

#pragma once
#include "GSM_DATATYPE.h"
#include "gsmnet.h"
#include "pcscDevice.h"

extern TCHAR g_sProfilePath[_MAX_PATH];
extern int MobileCompanyID;
extern TCHAR g_ConnectMode[300];

typedef int (* FUNC_PROGRESSSTEP)(int);

typedef struct
{
	TCHAR MobileID[32];
	TCHAR MobileName[32];
} MobileModel;

typedef enum {
	Nokia=1,
	SonyEricsson,
	Siemens,
	ASUS,
	MOTO,
	Samsung,
	Sharp,
	Panasonic,
	LG,
	SAGEM,
	Toshiba,
	MTK,
}MobileCompany;

typedef struct
{
  char MMS;        //MMS
  char SMS;        //SMS
  char EMS;        
  char Photo;      // PHOTO => File Manager Flag
  char Ringtone;   // RINGTONE
  char Video;      // VIDEO
  char PhoneBook;  // PhoneBook  
  char Calendar;   // Calendar
  char Memo;       
  char Todolist;   // Todolist ( Work)
  char Mail;       
  char Sync;	   // Sync
  char BackupRestore; 
  char DownloadJava; 
  char GPRS;        
  char PhotoManager; // For Photo only ; 

  char syncPhonebook; 
  char syncCalendar;
  char syncTodolist; 
  char syncMemo; 
  char syncMail; 

  char Reserved[3];
}Support_Mobile_Function;


int ChangeGSMErrCode (int error);

int WINAPI GetMobileNum (int MobileCompany );


bool WINAPI GetMobileList (int MobileCompany,
						   int ArrayNum,
						   MobileModel  *MobileList
						   );


int WINAPI OpenSyncDrv(int MobileCompany , TCHAR *MobileID , TCHAR *ConnectMode , TCHAR *ConnectPortName ,TCHAR *IMEI, int (*ConnectStatusCallBack)(int State) );

void WINAPI CloseSyncDrv( );
int WINAPI ResetMobile(void);
int WINAPI GetSupportFunction(Support_Mobile_Function *ColumnList);

void WINAPI SetProfilePath(TCHAR* Path);

//ex: char * VidPid="Vid_067b&Pid_2303";
int WINAPI SearchUSBPort(char * VidPid,PortNameList *PortList);
void WINAPI MBdrvThreadTerminate(void);

int WINAPI GetIMEI(char *IMEI);
int WINAPI GetModelName(char *MODELNAME);


int WINAPI CheckSIM(void);

//function pointer to AnwMobile.dll
typedef int (WINAPI* OpenSyncDrvProc)(int MobileCompany , TCHAR *MobileID , TCHAR *ConnectMode , TCHAR *ConnectPortName  ,TCHAR *IMEI, int (*ConnectStatusCallBack)(int State) );
typedef void (WINAPI* CloseSyncDrvProc)(void);
typedef void (WINAPI* SetProfilePathProc)(TCHAR* Path);

typedef int (WINAPI* GetMobileNumProc)(int MobileCompany);
typedef bool (WINAPI* GetMobileListProc)(int MobileCompany, int ArrayNum, MobileModel *MobileList);
typedef int  (WINAPI* GetSupportFunctionProc)(Support_Mobile_Function *ColumnList);
typedef int  (WINAPI* GetSupportFunctionExProc)(Support_Mobile_Function *ColumnList,TCHAR* BTstack);

typedef int (WINAPI* SearchUSBPortProc)(TCHAR * VidPid,PortNameList *PortList);						   

typedef void (WINAPI* MBdrvThreadTerminateProc)(void);
typedef int (WINAPI* GetIMEIProc)(TCHAR *IMEI);
typedef int (WINAPI* GetModelNameProc)(TCHAR *MODELNAME);
typedef int (WINAPI* GetSignalQualityInfoProc)(GSM_SignalQuality *sig);
typedef int (WINAPI* GetBatteryChargeInfoProc)(GSM_BatteryCharge *bat);
typedef int (WINAPI* GetOperatorInfoProc)(GSM_NetworkInfo *NetworkInfo);

typedef int (WINAPI* CheckSIMProc)(void);


extern OpenSyncDrvProc OpenSyncDrvfn;
extern CloseSyncDrvProc CloseSyncDrvfn;
extern SetProfilePathProc	SetProfilePathfn;

extern GetMobileNumProc GetMobileNumfn;
extern GetMobileListProc GetMobileListfn;
extern GetSupportFunctionProc GetSupportFunctionfn;
extern GetSupportFunctionExProc GetSupportFunctionExfn;

extern SearchUSBPortProc SearchUSBPortfn;
extern MBdrvThreadTerminateProc	MBdrvThreadTerminatefn;
extern GetIMEIProc	GetIMEIfn;
extern GetModelNameProc	GetModelNamefn;

extern CheckSIMProc CheckSIMfn;
extern GetSignalQualityInfoProc GetSignalQualityInfofn;
extern GetBatteryChargeInfoProc GetBatteryChargeInfofn;
extern GetOperatorInfoProc GetOperatorInfofn;

#endif
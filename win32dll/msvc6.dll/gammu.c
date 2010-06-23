
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <process.h>

#include <ctype.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#  include <windows.h>
#endif
#include <locale.h>
#include <signal.h>

#include "../../common/gsmstate.h"
#include "../../common/gsmcomon.h"
#include "../../common/misc/coding.h"

typedef struct {
	GSM_StateMachine 	s;
	void 			(*PhoneCallBack) 	(int s, boolean connected);
	void 			(*SecurityCallBack) 	(int s, GSM_SecurityCodeType State);
	void 			(*SMSCallBack) 		(int s);
	HANDLE       		hCommWatchThread;
	DWORD         		dwThreadID;
    	HANDLE 		 	Mutex;
	bool			ThreadTerminate;
	int 		 	number;
	int			errors;
	GSM_SecurityCodeType 	SecurityStatus;
	GSM_Error		SendSMSStatus;
} GSM_StateMachineExt;

GSM_StateMachineExt s[10];

void SetErrorCounter(int i, GSM_Error error)
{
	if (error == GE_TIMEOUT) {
		s[i].errors++;
	} else {
		if (s[i].errors > 0) s[i].errors--;
	}
}

BOOL LoopProc(int *i)
{
	GSM_Error		error;
	GSM_SecurityCodeType 	SecurityStatus;
	GSM_SMSMemoryStatus	SMSStatus;

	while(1) {
		if (s[*i].errors > 1) {
		        WaitForSingleObject(s[*i].Mutex, INFINITE );
			if (s[*i].ThreadTerminate) break;
			if (s[*i].errors != 255) {
				s[*i].PhoneCallBack(*i,false);
				GSM_TerminateConnection(&s[*i].s);
			}
			if (s[*i].ThreadTerminate) break;
			error=GSM_InitConnection(&s[*i].s,2);
			if (s[*i].ThreadTerminate) break;
			if (error == GE_NONE) {
				s[*i].errors = 0;
				s[*i].PhoneCallBack(*i,true);
				if (s[*i].ThreadTerminate) break;
				error=s[*i].s.Phone.Functions->SetAutoNetworkLogin(&s[*i].s);
				SetErrorCounter(*i, error);
			}
		        ReleaseMutex(s[*i].Mutex);
		} else {
		        WaitForSingleObject(s[*i].Mutex, INFINITE );
			if (s[*i].ThreadTerminate) break;
			error=s[*i].s.Phone.Functions->GetSecurityStatus(&s[*i].s,&SecurityStatus);
		        ReleaseMutex(s[*i].Mutex);
			SetErrorCounter(*i, error);
			if (error == GE_NONE) {
				if (SecurityStatus != s[*i].SecurityStatus) {
					if (s[*i].ThreadTerminate) break;
					s[*i].SecurityStatus = SecurityStatus;
					s[*i].SecurityCallBack(*i,SecurityStatus);
				}
			        WaitForSingleObject(s[*i].Mutex, INFINITE );
				if (s[*i].ThreadTerminate) break;
				error=s[*i].s.Phone.Functions->GetSMSStatus(&s[*i].s,&SMSStatus);
			        ReleaseMutex(s[*i].Mutex);
				SetErrorCounter(*i, error);
				if (error == GE_NONE) {
					if (s[*i].ThreadTerminate) break;
					if (SMSStatus.SIMUsed+SMSStatus.PhoneUsed != 0) s[*i].SMSCallBack(*i);
				}
			};
			if (s[*i].ThreadTerminate) break;
			mili_sleep(500);
			if (s[*i].ThreadTerminate) break;
		}
	}

	s[*i].hCommWatchThread 	= NULL;
	s[*i].dwThreadID 	= 0;

	return(TRUE);
}

static void CreatePhoneThread(int *phone,
			      void (*PhoneCallBack)    (int s, boolean connected),
			      void (*SecurityCallBack) (int s, GSM_SecurityCodeType State),
			      void (*SMSCallBack)      (int s))
{
	s[*phone].Mutex = CreateMutex( NULL, FALSE, NULL );
	s[*phone].errors 		= 0;
	s[*phone].SecurityStatus	= GSCT_None;
	s[*phone].number 		= *phone;
	s[*phone].PhoneCallBack		= PhoneCallBack;	
	s[*phone].SecurityCallBack	= SecurityCallBack;	
	s[*phone].SMSCallBack		= SMSCallBack;
	s[*phone].ThreadTerminate 	= false;
	s[*phone].hCommWatchThread = CreateThread((LPSECURITY_ATTRIBUTES) NULL,
				  0,
				  (LPTHREAD_START_ROUTINE) LoopProc,
				  (LPVOID) &s[*phone].number,
				  0, &s[*phone].dwThreadID);
}

static void CheckConnectionType(int  *phone,
				char *connection,
				char *connection_to_check,
				GSM_Error *error,
				GSM_Error *error2,
			        void (*PhoneCallBack)    (int s, boolean connected),
			        void (*SecurityCallBack) (int s, GSM_SecurityCodeType State),
			        void (*SMSCallBack)      (int s))
{
		s[*phone].s.CFGConnection = connection_to_check;
		*error=GSM_InitConnection(&s[*phone].s,2);
		switch (*error) {
		case GE_NONE:
			strcpy(connection,connection_to_check);
			CreatePhoneThread(phone,PhoneCallBack,SecurityCallBack,SMSCallBack);
		case GE_DEVICEOPENERROR:
			break;
		default:
			*error2 = GSM_TerminateConnection(&s[*phone].s);
		}
}

GSM_Error WINAPI mystartconnection(int *phone,
				   char *device,
				   char *connection,
				   char *model,
				   char *logfile,
				   char *logfiletype,
				   void (*PhoneCallBack)    (int s, boolean connected),
				   void (*SecurityCallBack) (int s, GSM_SecurityCodeType State),
				   void (*SMSCallBack) 	    (int s))
{
	int 		i;
	GSM_Error 	error,error2;

#ifndef WIN32
	setlocale(LC_ALL, "");
#else
	setlocale(LC_ALL, ".OCP");
#endif

	*phone = 0;
	for (i=1;i<10;i++) {
		if (!s[i].s.opened) {
			s[i].s.opened 	= true;
			*phone		= i;
			break;
		}
	}
	if (*phone == 0) return GE_MOREMEMORY;

	s[*phone].s.msg	 		= NULL;
        s[*phone].s.CFGLocalize 	= "";
	s[*phone].s.CFGSyncTime		= "";
	s[*phone].s.CFGDebugFile 	= malloc( strlen(logfile)+1 );
	if (s[*phone].s.CFGDebugFile == NULL) return GE_MOREMEMORY;
	strcpy(s[*phone].s.CFGDebugFile,logfile);
	strcpy(s[*phone].s.CFGDebugLevel,logfiletype);
	s[*phone].s.CFGLockDevice	= "";
	s[*phone].s.CFGDevice 		= malloc( strlen(device)+1 );
	if (s[*phone].s.CFGDevice == NULL) return GE_MOREMEMORY;
	strcpy(s[*phone].s.CFGDevice,device);
	strcpy(s[*phone].s.CFGModel,model);

	if (connection[0] == 0) {
		CheckConnectionType(phone,connection,"at115200",&error,&error2,PhoneCallBack,SecurityCallBack,SMSCallBack);
		switch (error) {
			case GE_NONE:
			case GE_DEVICEOPENERROR:
				return error;
			default:
				if (error2 != GE_NONE) return error2;
		}
		CheckConnectionType(phone,connection,"at19200",&error,&error2,PhoneCallBack,SecurityCallBack,SMSCallBack);
		switch (error) {
			case GE_NONE:
			case GE_DEVICEOPENERROR:
				return error;
			default:
				if (error2 != GE_NONE) return error2;
		}
		CheckConnectionType(phone,connection,"fbus",&error,&error2,PhoneCallBack,SecurityCallBack,SMSCallBack);
		switch (error) {
			case GE_NONE:
			case GE_DEVICEOPENERROR:
				return error;
			default:
				if (error2 != GE_NONE) return error2;
		}
		strcpy(connection,"");
		return GE_NOTCONNECTED;
	} else {
		s[*phone].s.CFGConnection = malloc( strlen(connection)+1 );
		if (s[*phone].s.CFGConnection == NULL) return GE_MOREMEMORY;
		strcpy(s[*phone].s.CFGConnection,connection);
		CreatePhoneThread(phone,PhoneCallBack,SecurityCallBack,SMSCallBack);
		return GE_NONE;
	}
}

GSM_Error WINAPI myendconnection(int phone)
{
	s[phone].ThreadTerminate = true;
	while (1) {
		if (s[phone].dwThreadID == 0) break;
	}
	if (s[phone].s.opened) {
		s[phone].s.opened = false;
		return GSM_TerminateConnection(&s[phone].s);
	} else {
		return GE_NONE;
	}
}

GSM_Error WINAPI mygetnetworkinfo (int phone, GSM_NetworkInfo *NetworkInfo)
{
	GSM_Error error;

	if (!s[phone].s.opened) return GE_NOTCONNECTED;

        WaitForSingleObject(s[phone].Mutex, INFINITE );
	error=s[phone].s.Phone.Functions->GetNetworkInfo(&s[phone].s,NetworkInfo);
	SetErrorCounter(phone, error);
        ReleaseMutex(s[phone].Mutex);
	return error;
}

GSM_Error WINAPI mygetnextsmsmessage (int phone, GSM_MultiSMSMessage *sms, bool start)
{
	GSM_Error 	error;
	int		i;

	if (!s[phone].s.opened) return GE_NOTCONNECTED;

        WaitForSingleObject(s[phone].Mutex, INFINITE );
	error=s[phone].s.Phone.Functions->GetNextSMSMessage(&s[phone].s,sms,start);
	SetErrorCounter(phone, error);
        ReleaseMutex(s[phone].Mutex);
	if (error == GE_NONE) {
		for (i=0;i<sms->Number;i++) {
			if (sms->SMS[i].PDU == SMS_Deliver || sms->SMS[i].PDU == SMS_Status_Report) {
				ReverseUnicodeString(sms->SMS[i].SMSC.Number);
			}
			ReverseUnicodeString(sms->SMS[i].Number);
			if (sms->SMS[i].PDU != SMS_Status_Report) {
				ReverseUnicodeString(sms->SMS[i].Name);
			}
			if (sms->SMS[i].Coding == GSM_Coding_Unicode || sms->SMS[i].Coding == GSM_Coding_Default) {
				ReverseUnicodeString(sms->SMS[i].Text);
			}
		}
	}
	return error;
}

GSM_Error WINAPI myentersecuritycode(int phone, GSM_SecurityCode *Code)
{
	GSM_Error error;

	if (!s[phone].s.opened) return GE_NOTCONNECTED;

        WaitForSingleObject(s[phone].Mutex, INFINITE );
	error=s[phone].s.Phone.Functions->EnterSecurityCode(&s[phone].s,*Code);
	SetErrorCounter(phone, error);
        ReleaseMutex(s[phone].Mutex);
	return error;
}

GSM_Error WINAPI mydeletesmsmessage (int phone, GSM_SMSMessage *sms)
{
	GSM_Error error;

	if (!s[phone].s.opened) return GE_NOTCONNECTED;

        WaitForSingleObject(s[phone].Mutex, INFINITE );
	error=s[phone].s.Phone.Functions->DeleteSMS(&s[phone].s,sms);
	SetErrorCounter(phone, error);
        ReleaseMutex(s[phone].Mutex);
	return error;
}

void SendSMSStatus (char *Device, int status)
{
	int i;

	for (i=0;i<10;i++) {
		if (s[i].s.opened) {
			if (strcmp(s[i].s.CFGDevice,Device)==0) {
				if (status == 0) {
					s[i].SendSMSStatus = GE_NONE;
				} else {
					s[i].SendSMSStatus = GE_UNKNOWN;
				}
			}
		}
	}
}

GSM_Error WINAPI mysendsmsmessage (int phone, GSM_SMSMessage *sms, int timeout)
{
	GSM_Error 	error;
	GSM_DateTime	Date;
	int		i,j;
	GSM_SMSMessage	sms2;

	if (!s[phone].s.opened) return GE_NOTCONNECTED;

	memcpy(&sms2,sms,sizeof(GSM_SMSMessage));
	if (sms2.SMSC.Location == 0) ReverseUnicodeString(sms2.SMSC.Number);
	ReverseUnicodeString(sms2.Number);
	ReverseUnicodeString(sms2.Name);
	if (sms2.Coding == GSM_Coding_Unicode || sms2.Coding == GSM_Coding_Default) {
		ReverseUnicodeString(sms2.Text);
	}

        WaitForSingleObject(s[phone].Mutex, INFINITE );
	s[phone].s.User.SendSMSStatus 	= SendSMSStatus;
	s[phone].SendSMSStatus 		= GE_TIMEOUT;
	error=s[phone].s.Phone.Functions->SendSMSMessage(&s[phone].s,&sms2);
	SetErrorCounter(phone, error);
	if (error != GE_NONE) {
	        ReleaseMutex(s[phone].Mutex);
		s[phone].s.User.SendSMSStatus = NULL;
		return error;
	}
	GSM_GetCurrentDateTime (&Date);
	for (j=0;j<timeout;j++) {
		i=Date.Second;
		while (i==Date.Second) {
			GSM_ReadDevice(&s[phone].s);
			mili_sleep(10);
			GSM_GetCurrentDateTime(&Date);
		}
		if (s[phone].SendSMSStatus != GE_TIMEOUT) break;
	}

	s[phone].s.User.SendSMSStatus = NULL;
	ReleaseMutex(s[phone].Mutex);
	return s[phone].SendSMSStatus;
}

GSM_Error WINAPI mygetsmsstatus (int phone, GSM_SMSMemoryStatus *status)
{
	GSM_Error error;

	if (!s[phone].s.opened) return GE_NOTCONNECTED;

        WaitForSingleObject(s[phone].Mutex, INFINITE );
	error=s[phone].s.Phone.Functions->GetSMSStatus(&s[phone].s,status);
	SetErrorCounter(phone, error);
        ReleaseMutex(s[phone].Mutex);
	return error;
}

GSM_Error WINAPI mysavesmsmessage (int phone, GSM_SMSMessage *sms)
{
	GSM_Error 	error;
	GSM_SMSMessage	sms2;

	if (!s[phone].s.opened) return GE_NOTCONNECTED;

	memcpy(&sms2,sms,sizeof(GSM_SMSMessage));
	if (sms2.SMSC.Location == 0) ReverseUnicodeString(sms2.SMSC.Number);
	ReverseUnicodeString(sms2.Number);
	ReverseUnicodeString(sms2.Name);
	if (sms2.Coding == GSM_Coding_Unicode || sms2.Coding == GSM_Coding_Default) {
		ReverseUnicodeString(sms2.Text);
	}

        WaitForSingleObject(s[phone].Mutex, INFINITE );
	error=s[phone].s.Phone.Functions->SaveSMSMessage(&s[phone].s,&sms2);
	SetErrorCounter(phone, error);
        ReleaseMutex(s[phone].Mutex);
	return error;
}

void WINAPI mygetnetworkname(char *NetworkCode, char *NetworkName)
{
	memcpy(NetworkName,GSM_GetNetworkName(NetworkCode),strlen(DecodeUnicodeString(GSM_GetNetworkName(NetworkCode)))*2+2);
	ReverseUnicodeString(NetworkName);
}

void WINAPI mygetgammuversion(char *version)
{
	sprintf(version, "%s",VERSION);
}

GSM_Error WINAPI mygetimei(int phone, char *IMEI)
{
	GSM_Error error;

	if (!s[phone].s.opened) return GE_NOTCONNECTED;

        WaitForSingleObject(s[phone].Mutex, INFINITE );
	error=s[phone].s.Phone.Functions->GetIMEI(&s[phone].s,IMEI);
	SetErrorCounter(phone, error);
        ReleaseMutex(s[phone].Mutex);
	return error;
}

GSM_Error WINAPI mygetmanufacturer(int phone, char *manufacturer)
{
	GSM_Error error;

	if (!s[phone].s.opened) return GE_NOTCONNECTED;

        WaitForSingleObject(s[phone].Mutex, INFINITE );
	error=s[phone].s.Phone.Functions->GetManufacturer(&s[phone].s,manufacturer);
	SetErrorCounter(phone, error);
        ReleaseMutex(s[phone].Mutex);
	if (error == GE_NONE) ReverseUnicodeString(manufacturer);
	return error;
}

GSM_Error WINAPI mygetmodel(int phone, char *model)
{
	if (!s[phone].s.opened) return GE_NOTCONNECTED;
	sprintf(model, "%s",s[phone].s.Model);
	return GE_NONE;
}

GSM_Error WINAPI mygetfirmwareversion(int phone, double *version)
{
	if (!s[phone].s.opened) return GE_NOTCONNECTED;
	*version = s[phone].s.VerNum;
	return GE_NONE;
}

GSM_Error WINAPI myreset(int phone, bool hard)
{
	GSM_Error error;

	if (!s[phone].s.opened) return GE_NOTCONNECTED;

        WaitForSingleObject(s[phone].Mutex, INFINITE );
	error=s[phone].s.Phone.Functions->Reset(&s[phone].s,hard);
	SetErrorCounter(phone, error);
        ReleaseMutex(s[phone].Mutex);
	return error;

}

BOOL WINAPI DllMain  ( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved )
{
	int i;

	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			for (i=0;i<10;i++) {
				s[i].s.opened 	= false;
				s[i].dwThreadID = 0;
			}
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			for (i=0;i<10;i++) {
				if (s[i].s.opened) GSM_TerminateConnection(&s[i].s);
			}
			break;
    	}
 	return TRUE;
}

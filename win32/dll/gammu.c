
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <process.h>
#include <ctype.h>
#include <locale.h>
#include <signal.h>

#include "../../common/gsmstate.h"
#include "../../common/gsmcomon.h"
#include "../../common/misc/coding/coding.h"

typedef struct {
	GSM_StateMachine 	s;
	void 			(**PhoneCallBack) 	(int x, int s, boolean connected);
	void 			(**SecurityCallBack) 	(int x, int s, GSM_SecurityCodeType State);
	void 			(**SMSCallBack) 	(int x, int s);
	HANDLE       		hCommWatchThread;
	DWORD         		dwThreadID;
    	HANDLE 		 	Mutex;
	bool			ThreadTerminate;
	bool			Used;
	int 		 	number;
	int			errors;
	GSM_SecurityCodeType 	SecurityStatus;
	GSM_Error		SendSMSStatus;
} GSM_StateMachineExt;

GSM_StateMachineExt s[10];

bool SetErrorCounter(int i, GSM_Error error)
{
	switch (error) {
	case GE_TIMEOUT:
		s[i].errors++;
		break;
	case GE_NOTSUPPORTED:
		break;
	default:
		if (s[i].errors > 0) s[i].errors--;
	}
	if (s[i].errors > 1) return true;
	return false;
}

BOOL LoopProc(int *i)
{
	GSM_Error		error;
	GSM_SecurityCodeType 	SecurityStatus;
	GSM_SMSMemoryStatus	SMSStatus;
	void 			(*PhoneCall) 	(int x, int s, boolean connected);
	void 			(*SecurityCall) (int x, int s, GSM_SecurityCodeType State);
	void 			(*SMSCall) 	(int x, int s);

	while(1) {
		if (s[*i].errors > 1) {
		        WaitForSingleObject(s[*i].Mutex, INFINITE );
			if (s[*i].ThreadTerminate) break;
			if (s[*i].errors < 250) {
				PhoneCall = *s[*i].PhoneCallBack;
				PhoneCall(1,*i,false);
			}
			if (s[*i].errors != 250) {
				GSM_TerminateConnection(&s[*i].s);
			}
			s[*i].errors = 251;
			if (s[*i].ThreadTerminate) break;
			error=GSM_InitConnection(&s[*i].s,2);
			if (s[*i].ThreadTerminate) break;
//			if (error == GE_DEVICEOPENERROR) break;
			if (error == GE_NONE) {
				s[*i].errors = 0;
				PhoneCall = *s[*i].PhoneCallBack;
				PhoneCall(1,*i,true);
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
			if (SetErrorCounter(*i, error)) continue;
			switch (error) {
			case GE_NONE:
				if (SecurityStatus != s[*i].SecurityStatus) {
					if (s[*i].ThreadTerminate) break;
					s[*i].SecurityStatus = SecurityStatus;
					SecurityCall = *s[*i].SecurityCallBack;
					SecurityCall(1,*i,SecurityStatus);
				}
				break;
			case GE_NOTSUPPORTED:
				if (s[*i].ThreadTerminate) break;
				SecurityStatus		= 0;
				s[*i].SecurityStatus 	= 0;
				SecurityCall = *s[*i].SecurityCallBack;
				SecurityCall(1,*i,SecurityStatus);
			default:
				break;
			}
			WaitForSingleObject(s[*i].Mutex, INFINITE );
			if (s[*i].ThreadTerminate) break;
			error=s[*i].s.Phone.Functions->GetSMSStatus(&s[*i].s,&SMSStatus);
			ReleaseMutex(s[*i].Mutex);
			if (SetErrorCounter(*i, error)) continue;
			if (error == GE_NONE) {
				if (SMSStatus.SIMUsed+SMSStatus.PhoneUsed != 0) {
					if (s[*i].ThreadTerminate) break;
					SMSCall = *s[*i].SMSCallBack;
					SMSCall(1,*i);
				}
			}
			if (s[*i].ThreadTerminate) break;
			my_sleep(500);
			if (s[*i].ThreadTerminate) break;
		}
	}

	s[*i].Used		= false;
	s[*i].hCommWatchThread 	= NULL;
	s[*i].dwThreadID 	= 0;

	return(TRUE);
}

static void CreatePhoneThread(int *phone,
			      void (**PhoneCallBack)    (int x, int s, boolean connected),
			      void (**SecurityCallBack) (int x, int s, GSM_SecurityCodeType State),
			      void (**SMSCallBack)      (int x, int s))
{
	s[*phone].Mutex = CreateMutex( NULL, FALSE, NULL );
	s[*phone].SecurityStatus	= 0;
	s[*phone].number 		= *phone;
	
	s[*phone].PhoneCallBack 	= PhoneCallBack;
	s[*phone].SecurityCallBack 	= SecurityCallBack;
	s[*phone].SMSCallBack 		= SMSCallBack;
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
			        void (**PhoneCallBack)    (int x, int s, boolean connected),
			        void (**SecurityCallBack) (int x, int s, GSM_SecurityCodeType State),
			        void (**SMSCallBack)      (int x, int s))
{
	void (*PhoneCall) (int x, int s, boolean connected);

	s[*phone].s.CurrentConfig->Connection = connection_to_check;
	*error=GSM_InitConnection(&s[*phone].s,2);
	switch (*error) {
	case GE_NONE:
		strcpy(connection,connection_to_check);
		s[*phone].errors = 0;
		CreatePhoneThread(phone,PhoneCallBack,SecurityCallBack,SMSCallBack);
		PhoneCall = *s[*phone].PhoneCallBack;
		PhoneCall(1,*phone,true);
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
				   void (**PhoneCallBack)    (int x, int s, boolean connected),
				   void (**SecurityCallBack) (int x, int s, GSM_SecurityCodeType State),
				   void (**SMSCallBack)	     (int x, int s))
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
		if (!s[i].Used) {
			s[i].Used 	= true;
			*phone		= i;
			break;
		}
	}
	if (*phone == 0) return GE_MOREMEMORY;

	s[*phone].s.ConfigNum			= 1;
	s[*phone].s.msg	 			= NULL;
	s[*phone].s.CurrentConfig		= &s[*phone].s.Config[0];
    	s[*phone].s.CurrentConfig->Localize 	= "";
	s[*phone].s.CurrentConfig->SyncTime	= "";
	s[*phone].s.CurrentConfig->DebugFile 	= malloc( strlen(logfile)+1 );
	if (s[*phone].s.CurrentConfig->DebugFile == NULL) return GE_MOREMEMORY;
	strcpy(s[*phone].s.CurrentConfig->DebugFile,logfile);
	strcpy(s[*phone].s.CurrentConfig->DebugLevel,logfiletype);
	s[*phone].s.CurrentConfig->LockDevice	= "";
	s[*phone].s.CurrentConfig->StartInfo	= "yes";
	s[*phone].s.CurrentConfig->Device 	= malloc( strlen(device)+1 );
	if (s[*phone].s.CurrentConfig->Device == NULL) return GE_MOREMEMORY;
	strcpy(s[*phone].s.CurrentConfig->Device,device);
	strcpy(s[*phone].s.CurrentConfig->Model,model);

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
		s[*phone].s.CurrentConfig->Connection = malloc( strlen(connection)+1 );
		if (s[*phone].s.CurrentConfig->Connection == NULL) return GE_MOREMEMORY;
		strcpy(s[*phone].s.CurrentConfig->Connection,connection);
		s[*phone].errors = 250;
		CreatePhoneThread(phone,PhoneCallBack,SecurityCallBack,SMSCallBack);
		return GE_NONE;
	}
}

GSM_Error WINAPI myendconnection(int phone)
{
	GSM_Error error=GE_NONE;

	if (s[phone].Used) {
		s[phone].ThreadTerminate = true;
		if (s[phone].s.opened) {
		        WaitForSingleObject(s[phone].Mutex, INFINITE );
			error = GSM_TerminateConnection(&s[phone].s);
		        ReleaseMutex(s[phone].Mutex);
		}
	}
	return error;
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
	error=s[phone].s.Phone.Functions->GetNextSMS(&s[phone].s,sms,start);
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
		if (s[i].s.opened && s[i].Used) {
			if (strcmp(s[i].s.CurrentConfig->Device,Device)==0) {
				if (status == 0) {
					s[i].SendSMSStatus = GE_NONE;
				} else {
					s[i].SendSMSStatus = GE_UNKNOWN;
				}
			}
		}
	}
}

GSM_Error WINAPI mysendsmsmessage (int phone, GSM_SMSMessage *sms, unsigned int timeout)
{
	GSM_Error 	error;
	GSM_DateTime	Date;
	unsigned int	i,j;
	GSM_SMSMessage	sms2;

	if (!s[phone].s.opened) return GE_NOTCONNECTED;

	memcpy(&sms2,sms,sizeof(GSM_SMSMessage));

	if (sms2.SMSC.Location == 0) ReverseUnicodeString(sms2.SMSC.Number);
	ReverseUnicodeString(sms2.Number);
//	ReverseUnicodeString(sms2.Name);
	if (sms2.Coding == GSM_Coding_Unicode || sms2.Coding == GSM_Coding_Default) {
		ReverseUnicodeString(sms2.Text);
	}

        WaitForSingleObject(s[phone].Mutex, INFINITE );
	s[phone].s.User.SendSMSStatus 	= SendSMSStatus;
	s[phone].SendSMSStatus 		= GE_TIMEOUT;

	error=s[phone].s.Phone.Functions->SendSMS(&s[phone].s,&sms2);
	SetErrorCounter(phone, error);
	if (error != GE_NONE) {
	        ReleaseMutex(s[phone].Mutex);
		s[phone].s.User.SendSMSStatus = NULL;
		return error;
	}
	for (j=0;j<timeout;j++) {
		GSM_GetCurrentDateTime(&Date);
		i=Date.Second;
		while (i==Date.Second) {
			GSM_ReadDevice(&s[phone].s,false);
			if (s[phone].SendSMSStatus != GE_TIMEOUT) break;
			GSM_GetCurrentDateTime(&Date);
			my_sleep(20);
		}
		if (s[phone].SendSMSStatus != GE_TIMEOUT) break;
	}
	ReleaseMutex(s[phone].Mutex);
	SetErrorCounter(phone, s[phone].SendSMSStatus);
	s[phone].s.User.SendSMSStatus = NULL;
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

GSM_Error WINAPI myaddsmsmessage (int phone, GSM_SMSMessage *sms)
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
	error=s[phone].s.Phone.Functions->AddSMS(&s[phone].s,&sms2);
	SetErrorCounter(phone, error);
        ReleaseMutex(s[phone].Mutex);
	return error;
}

void WINAPI mygetnetworkname(char *NetworkCode, char *NetworkName)
{
	memcpy(NetworkName,GSM_GetNetworkName(NetworkCode),UnicodeLength(GSM_GetNetworkName(NetworkCode))*2+2);
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
	error=s[phone].s.Phone.Functions->GetIMEI(&s[phone].s);
	SetErrorCounter(phone, error);
        ReleaseMutex(s[phone].Mutex);
	if (error == GE_NONE) strcpy(IMEI,s[phone].s.Phone.Data.IMEI);
	return error;
}

GSM_Error WINAPI mygetmanufacturer(int phone, char *manufacturer)
{
	GSM_Error error;

	if (!s[phone].s.opened) return GE_NOTCONNECTED;

        WaitForSingleObject(s[phone].Mutex, INFINITE );
	error=s[phone].s.Phone.Functions->GetManufacturer(&s[phone].s);
	SetErrorCounter(phone, error);
        ReleaseMutex(s[phone].Mutex);
	if (error == GE_NONE) strcpy(manufacturer,s[phone].s.Phone.Data.Manufacturer);
	return error;
}

GSM_Error WINAPI mygetmodel(int phone, char *model)
{
	if (!s[phone].s.opened) return GE_NOTCONNECTED;
	strcpy(model,s[phone].s.Phone.Data.Model);
	return GE_NONE;
}

GSM_Error WINAPI mygetmodelname(int phone, char *model)
{
	if (!s[phone].s.opened) return GE_NOTCONNECTED;
	strcpy(model,s[phone].s.Phone.Data.ModelInfo->model);
	return GE_NONE;
}

GSM_Error WINAPI mygetfirmwareversion(int phone, double *version)
{
	if (!s[phone].s.opened) return GE_NOTCONNECTED;
	*version = s[phone].s.Phone.Data.VerNum;
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

GSM_Error WINAPI mysmscounter(int 		MessageLength,
			      unsigned char 	*MessageBuffer,
		    	      GSM_UDH	 	UDH,
		    	      GSM_Coding_Type 	Coding,
		    	      int 		*SMSNum,
		    	      int 		*CharsLeft)
{
	/* FIXME. Reverse MessageBuffer */
	GSM_SMSCounter(MessageLength,MessageBuffer,UDH,Coding,SMSNum,CharsLeft);
	return GE_NONE;
}

GSM_Error WINAPI mymakemultipartsms(unsigned char		*MessageBuffer,
			  	    int				MessageLength,
			  	    GSM_UDH			UDHType,
			  	    GSM_Coding_Type		Coding,
			  	    int				Class,
			  	    unsigned char		ReplaceMessage,
				    GSM_MultiSMSMessage		*SMS)
{
	int i;

	/* FIXME. Reverse MessageBuffer */
	GSM_MakeMultiPartSMS(SMS,MessageBuffer,MessageLength,UDHType,Coding,Class,ReplaceMessage);
	for (i=0;i<SMS->Number;i++) {
		if (SMS->SMS[i].Coding == GSM_Coding_Unicode || SMS->SMS[i].Coding == GSM_Coding_Default) {
			ReverseUnicodeString(SMS->SMS[i].Text);
		}
	}
	return GE_NONE;
}

int WINAPI mygetstructuresize(int i)
{
	switch (i) {
		case 0: return sizeof(GSM_SMSMessage);
		case 1: return sizeof(GSM_SMSC);
		case 2: return sizeof(GSM_SMS_State);
		case 3: return sizeof(GSM_UDHHeader);
		case 4: return sizeof(bool);
		case 5: return sizeof(GSM_DateTime);
		case 6: return sizeof(int);
		case 7: return sizeof(GSM_NetworkInfo);
		case 8: return sizeof(GSM_UDH);
	}
	return 0;
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
				s[i].Used	= false;
				s[i].dwThreadID = 0;
			}
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			for (i=0;i<10;i++) {
				myendconnection(i);
			}
			break;
    	}
 	return TRUE;
}

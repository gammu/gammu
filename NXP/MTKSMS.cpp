#include "stdafx.h"
#include "MTK.h"
#include "commfun.h"
#include "MTKfundef.h"
extern CMTKApp theApp;

GSM_Error WINAPI GetSMSStatus(GSM_SMSMemoryStatus *status)
{
	GSM_Error error;
	error =  ATGEN_GetSMSStatus(status,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo );
	return error;
}
GSM_Error WINAPI GetSMSMessage(GSM_MultiSMSMessage *sms)
{
	GSM_Error error;
	error =  ATGEN_GetSMS(theApp.m_pMobileInfo,sms,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo );
	return error;
}
GSM_Error WINAPI GetNextSMSMessage(GSM_MultiSMSMessage *sms, bool start)
{
	GSM_Error error;

	error = ATGEN_GetNextSMS(theApp.m_pMobileInfo,sms,start,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo );
	return error;
}
GSM_Error WINAPI SetSMS(GSM_SMSMessage *sms)
{
	return	ERR_NOTSUPPORTED;
	//	return ATGEN_SendSMS(sms,theApp.m_pSetATProtocolDatafn,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo );
}
GSM_Error WINAPI AddSMS(GSM_SMSMessage *sms)
{
	GSM_Error error;

	error = ATGEN_AddSMS(theApp.m_pMobileInfo,sms,theApp.m_pSetATProtocolDatafn,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo );
	Sleep(1000);
	return error;
}
GSM_Error WINAPI DeleteSMSMessage(GSM_SMSMessage *sms)
{
	GSM_Error error;

	error = ATGEN_DeleteSMS(theApp.m_pMobileInfo,sms,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo );
	return error;
}
GSM_Error WINAPI SendSMSMessage(GSM_SMSMessage *sms)
{
	GSM_Error error;
	//error = MTK_SetCharset(false);

	error = ATGEN_SendSMS(sms,theApp.m_pSetATProtocolDatafn,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo );
	//error = MTK_SetCharset(true);
	return error;
}

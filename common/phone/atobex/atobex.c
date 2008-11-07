/* (c) 2006 by Michal Cihar */

/**
 * \file atobex.c
 * @author Michal Čihař
 */
/**
 * @addtogroup Phone
 * @{
 */
/**
 * \defgroup SEPhone Sony-Ericsson phones communication
 * High level functions for communication with Sony-Ericsson phones.
 *
 * This module heavily uses \ref ATPhone and \ref OBEXPhone modules.
 *
 * @{
 */

#include "../../gsmstate.h"

#ifdef GSM_ENABLE_ATOBEX
#ifdef GSM_ENABLE_ATGEN

#include <string.h>
#include <time.h>

#include "../../gsmcomon.h"
#include "../../misc/coding/coding.h"
#include "../../misc/misc.h"
#include "../pfunc.h"
#include "../at/atfunc.h"
#include "../obex/obexfunc.h"
#include "atobex.h"

extern GSM_Reply_Function ATOBEXReplyFunctions[];


/**
 * Ensures phone and Gammu protocol are switched to AT commands mode.
 */
GSM_Error ATOBEX_SetATMode(GSM_StateMachine *s)
{
	GSM_Phone_ATOBEXData	*Priv = &s->Phone.Data.Priv.ATOBEX;
	GSM_Error		error;

	/* Aren't we in OBEX mode? */
	if (Priv->Mode == ATOBEX_ModeAT) return ERR_NONE;

	dbgprintf ("Terminating OBEX\n");

	/* Disconnect from OBEX service */
	error = OBEXGEN_Disconnect(s);
	if (error != ERR_NONE) return error;

	/* Terminate OBEX protocol */
	error = s->Protocol.Functions->Terminate(s);
	if (error != ERR_NONE) return error;

	/* Switch to AT protocol */
	dbgprintf ("Changing protocol to AT\n");
	s->Protocol.Functions			= &ATProtocol;
	s->Phone.Functions->ReplyFunctions	= ATGENReplyFunctions;
	Priv->Mode				= ATOBEX_ModeAT;

	/* Terminate SQWE Obex mode */
	if (Priv->HasOBEX == ATOBEX_OBEX_SQWE) {
		error = s->Protocol.Functions->WriteMessage(s, "+++", 3, 0x00);
		if (error != ERR_NONE) return error;
		error = s->Device.Functions->DeviceSetDtrRts(s, false, false);
		if (error != ERR_NONE) return error;
	}

	/* Initialise AT protocol */
	error = s->Protocol.Functions->Initialise(s);
	if (error != ERR_NONE) return error;

	return ERR_NONE;
}


/**
 * Ensures phone and Gammu protocol are in OBEX mode, in IrMC service
 * if requrested.
 */
GSM_Error ATOBEX_SetOBEXMode(GSM_StateMachine *s, OBEX_Service service)
{
	GSM_Phone_ATOBEXData	*Priv = &s->Phone.Data.Priv.ATOBEX;
	GSM_Error		error;

	/* Is OBEX mode supported? */
	if (Priv->HasOBEX == ATOBEX_OBEX_None) {
		return ERR_NOTSUPPORTED;
	}

	/* Are we already in OBEX mode? */
	if (Priv->Mode == ATOBEX_ModeOBEX) {
		/* We can not safely switch service, we need to disconnect instead */
		if (s->Phone.Data.Priv.OBEXGEN.Service == service) {
			return ERR_NONE;
		} else {
			error = ATOBEX_SetATMode(s);
		}
		if (error != ERR_NONE) return error;
	}

	dbgprintf ("Changing to OBEX mode\n");

	/* Switch phone to OBEX */
	error = ERR_NOTSUPPORTED;
	switch (Priv->HasOBEX) {
		case ATOBEX_OBEX_CPROT0:
			/* 3GPP TS 27.007 standard */
			error = GSM_WaitFor (s, "AT+CPROT=0\r", 11, 0x00, 20, ID_SetOBEX);
			break;
		case ATOBEX_OBEX_EOBEX:
			/* Sony-Ericsson extension */
			error = GSM_WaitFor (s, "AT*EOBEX\r", 9, 0x00, 20, ID_SetOBEX);
			break;
		case ATOBEX_OBEX_MODE22:
			/* Motorola extension */
			error = GSM_WaitFor (s, "AT+MODE=22\r", 11, 0x00, 20, ID_SetOBEX);
			break;
		case ATOBEX_OBEX_XLNK:
			/* Sharp extension */
			error = GSM_WaitFor (s, "AT+XLNK\r", 8, 0x00, 20, ID_SetOBEX);
			break;
		case ATOBEX_OBEX_SQWE:
			/* Siemens extension */
			error = GSM_WaitFor (s, "AT^SQWE=3\r", 10, 0x00, 20, ID_SetOBEX);
			break;
		case ATOBEX_OBEX_None:
			break;
	}
	if (error != ERR_NONE) return error;

	/* Tell OBEX module it has no service selected */
	s->Phone.Data.Priv.OBEXGEN.Service = 0;

	smprintf(s, "Changing protocol to OBEX\n");

	/* Stop AT protocol */
	error = s->Protocol.Functions->Terminate(s);
	if (error != ERR_NONE) return error;

	/* Need some sleep before starting talk in OBEX */
	sleep(1);

	/* Switch to OBEX protocol and initialise it */
	s->Protocol.Functions = &OBEXProtocol;
	s->Phone.Functions->ReplyFunctions	= OBEXGENReplyFunctions;

	/* Initialise protocol */
	error = s->Protocol.Functions->Initialise(s);
	if (error != ERR_NONE) {
		/* Revert back to AT */
		s->Protocol.Functions = &ATProtocol;
		return error;
	}

	/* Remember our state */
	Priv->Mode				= ATOBEX_ModeOBEX;

	/* Choose appropriate connection type (we need different for filesystem and for IrMC) */
	smprintf(s, "Setting service %d\n", service);
	error = OBEXGEN_Connect(s, service);
	if (error != ERR_NONE) return error;

	return ERR_NONE;
}


/**
 * Initialises Sony-Ericsson module internals and calls AT module init.
 */
GSM_Error ATOBEX_Initialise(GSM_StateMachine *s)
{
	GSM_Phone_ATOBEXData	*Priv = &s->Phone.Data.Priv.ATOBEX;
	GSM_Phone_ATGENData	*PrivAT = &s->Phone.Data.Priv.ATGEN;
	GSM_Error		error;

	Priv->Mode				= ATOBEX_ModeAT;
	Priv->EBCAFailed = false;

	/* We might receive incoming event */
	s->Phone.Data.BatteryCharge = NULL;

	/* Init OBEX module also */
	error = OBEXGEN_InitialiseVars(s);
	if (error != ERR_NONE) return error;

	/* This can be filled in by AT module init */
	Priv->HasOBEX = ATOBEX_OBEX_None;

	/* Init AT module */
	/* This also enables ATOBEX_OBEX_CPROT0 if available */
	error = ATGEN_Initialise(s);
	if (error != ERR_NONE) return error;

	/* Does phone have support for AT+MODE=22 switching? */
	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_MODE22)) {
		Priv->HasOBEX = ATOBEX_OBEX_MODE22;
	} else if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_XLNK)) {
		Priv->HasOBEX = ATOBEX_OBEX_XLNK;
	} else if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SQWE)) {
		Priv->HasOBEX = ATOBEX_OBEX_SQWE;
	} else if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_CPROT)) {
		Priv->HasOBEX = ATOBEX_OBEX_CPROT0;
	} else {
		if (PrivAT->Mode) {
			smprintf(s, "Guessed mode style switching\n");
			Priv->HasOBEX = ATOBEX_OBEX_MODE22;
		}
	}

	/* Do we have OBEX capability? */
	if (Priv->HasOBEX == ATOBEX_OBEX_None) {
		error = GSM_WaitFor (s, "AT*EOBEX=?\r", 11, 0x00, 4, ID_SetOBEX);
		if (error == ERR_NONE) {
			Priv->HasOBEX = ATOBEX_OBEX_EOBEX;
		}
	}

	return ERR_NONE;
}

/**
 * Switch to AT mode and calls AT module termination procedure.
 */
GSM_Error ATOBEX_Terminate(GSM_StateMachine *s)
{
	GSM_Error 		error;

	error = ATOBEX_SetATMode(s);
	OBEXGEN_FreeVars(s);
	return ATGEN_Terminate(s);
}


/**
 * Dispatches message to correct dispatcher according to active protocol
 */
GSM_Error ATOBEX_DispatchMessage(GSM_StateMachine *s)
{
	if (s->Phone.Data.Priv.ATOBEX.Mode == ATOBEX_ModeOBEX) {
		return GSM_DispatchMessage(s);
	} else {
		return ATGEN_DispatchMessage(s);
	}
}


/**
 * We receive product code over AT commands, so we can easily use it
 */
GSM_Error ATOBEX_GetProductCode(GSM_StateMachine *s, char *value)
{
       strcpy(value, s->Phone.Data.Model);
       return ERR_NONE;
}


/**
 * \defgroup SEAT Wrapper functions for using AT module functionality
 * \ingroup SEPhone
 * @{
 */

GSM_Error ATOBEX_GetIMEI (GSM_StateMachine *s)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetIMEI(s);
}

GSM_Error ATOBEX_GetFirmware(GSM_StateMachine *s)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetFirmware(s);
}

GSM_Error ATOBEX_GetModel(GSM_StateMachine *s)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetModel(s);
}

GSM_Error ATOBEX_GetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetDateTime(s, date_time);
}

GSM_Error ATOBEX_GetSMS(GSM_StateMachine *s, GSM_MultiSMSMessage *sms)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetSMS(s, sms);
}

GSM_Error ATOBEX_DeleteSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_DeleteSMS(s, sms);
}

GSM_Error ATOBEX_AddSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_AddSMS(s, sms);
}

GSM_Error ATOBEX_GetSignalStrength(GSM_StateMachine *s, GSM_SignalQuality *sig)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetSignalQuality(s, sig);
}

GSM_Error ATOBEX_GetSMSFolders(GSM_StateMachine *s, GSM_SMSFolders *folders)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetSMSFolders(s, folders);
}

GSM_Error ATOBEX_GetNextSMS(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, bool start)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetNextSMS(s, sms, start);
}

GSM_Error ATOBEX_GetSMSStatus(GSM_StateMachine *s, GSM_SMSMemoryStatus *status)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetSMSStatus(s, status);
}

GSM_Error ATOBEX_DialVoice(GSM_StateMachine *s, char *number, GSM_CallShowNumber ShowNumber)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_DialVoice(s, number, ShowNumber);
}

GSM_Error ATOBEX_DialService(GSM_StateMachine *s, char *number)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_DialService(s, number);
}

GSM_Error ATOBEX_AnswerCall(GSM_StateMachine *s, int ID, bool all)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_AnswerCall(s,ID,all);
}

GSM_Error ATOBEX_GetNetworkInfo(GSM_StateMachine *s, GSM_NetworkInfo *netinfo)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetNetworkInfo(s, netinfo);
}

GSM_Error ATOBEX_GetDisplayStatus(GSM_StateMachine *s, GSM_DisplayFeatures *features)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetDisplayStatus(s, features);
}

GSM_Error ATOBEX_SetAutoNetworkLogin(GSM_StateMachine *s)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetAutoNetworkLogin(s);
}

GSM_Error ATOBEX_PressKey(GSM_StateMachine *s, GSM_KeyCode Key, bool Press)
{
	GSM_Error error;

	/**
	 * @todo Implement completely using AT*EKEY
	 */
	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_PressKey(s, Key, Press);
}

GSM_Error ATOBEX_Reset(GSM_StateMachine *s, bool hard)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_Reset(s, hard);
}

GSM_Error ATOBEX_CancelCall(GSM_StateMachine *s, int ID, bool all)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_CancelCall(s,ID,all);
}

GSM_Error ATOBEX_SendSavedSMS(GSM_StateMachine *s, int Folder, int Location)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SendSavedSMS(s, Folder, Location);
}

GSM_Error ATOBEX_SendSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SendSMS(s, sms);
}

GSM_Error ATOBEX_SetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetDateTime(s, date_time);
}

GSM_Error ATOBEX_SetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetSMSC(s, smsc);
}

GSM_Error ATOBEX_GetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetSMSC(s, smsc);
}

GSM_Error ATOBEX_EnterSecurityCode(GSM_StateMachine *s, GSM_SecurityCode Code)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_EnterSecurityCode(s, Code);
}

GSM_Error ATOBEX_GetSecurityStatus(GSM_StateMachine *s, GSM_SecurityCodeType *Status)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetSecurityStatus(s, Status);
}

GSM_Error ATOBEX_ResetPhoneSettings(GSM_StateMachine *s, GSM_ResetSettingsType Type)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_ResetPhoneSettings(s, Type);
}

GSM_Error ATOBEX_SendDTMF(GSM_StateMachine *s, char *sequence)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SendDTMF(s, sequence);
}

GSM_Error ATOBEX_GetSIMIMSI(GSM_StateMachine *s, char *IMSI)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetSIMIMSI(s, IMSI);
}

GSM_Error ATOBEX_SetIncomingCall (GSM_StateMachine *s, bool enable)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetIncomingCall(s, enable);
}

GSM_Error ATOBEX_SetIncomingCB (GSM_StateMachine *s, bool enable)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetIncomingCB(s, enable);
}

GSM_Error ATOBEX_SetIncomingSMS (GSM_StateMachine *s, bool enable)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetIncomingSMS(s, enable);
}

GSM_Error ATOBEX_SetFastSMSSending(GSM_StateMachine *s, bool enable)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetFastSMSSending(s, enable);
}

GSM_Error ATOBEX_GetManufacturer(GSM_StateMachine *s)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetManufacturer(s);
}

GSM_Error ATOBEX_GetAlarm(GSM_StateMachine *s, GSM_Alarm *Alarm)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetAlarm(s, Alarm);
}

GSM_Error ATOBEX_SetAlarm(GSM_StateMachine *s, GSM_Alarm *Alarm)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetAlarm(s, Alarm);
}

GSM_Error ATOBEX_SetIncomingUSSD(GSM_StateMachine *s, bool enable)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetIncomingUSSD(s, enable);
}

GSM_Error ATOBEX_GetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, bool PhoneRingtone)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetRingtone(s, Ringtone, PhoneRingtone);
}

GSM_Error ATOBEX_SetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, int *maxlength)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetRingtone(s, Ringtone, maxlength);
}

GSM_Error ATOBEX_GetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetBitmap(s, Bitmap);
}

GSM_Error ATOBEX_SetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetBitmap(s, Bitmap);
}

/*@}*/
/**
 * \defgroup SEOBEX OBEX native functions for filesystem
 * \ingroup SEPhone
 * @{
 */

GSM_Error ATOBEX_AddFilePart(GSM_StateMachine *s, GSM_File *File, int *Pos, int *Handle)
{
	GSM_Error error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_BrowsingFolders))!= ERR_NONE) return error;
	return OBEXGEN_AddFilePart(s, File, Pos, Handle);
}

GSM_Error ATOBEX_SendFilePart(GSM_StateMachine *s, GSM_File *File, int *Pos, int *Handle)
{
	GSM_Error error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_None))!= ERR_NONE) return error;
	return OBEXGEN_SendFilePart(s, File, Pos, Handle);
}

GSM_Error ATOBEX_GetFilePart(GSM_StateMachine *s, GSM_File *File, int *Handle, int *Size)
{
	GSM_Error error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_BrowsingFolders))!= ERR_NONE) return error;
	return OBEXGEN_GetFilePart(s, File, Handle, Size);
}

GSM_Error ATOBEX_GetNextFileFolder(GSM_StateMachine *s, GSM_File *File, bool start)
{
	GSM_Error error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_BrowsingFolders))!= ERR_NONE) return error;
	return OBEXGEN_GetNextFileFolder(s, File, start);
}

GSM_Error ATOBEX_DeleteFile(GSM_StateMachine *s, unsigned char *ID)
{
	GSM_Error error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_BrowsingFolders))!= ERR_NONE) return error;
	return OBEXGEN_DeleteFile(s, ID);
}

GSM_Error ATOBEX_AddFolder(GSM_StateMachine *s, GSM_File *File)
{
	GSM_Error error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_BrowsingFolders))!= ERR_NONE) return error;
	return OBEXGEN_AddFolder(s, File);
}

/*@}*/
/**
 * \defgroup SEATIrMC Mixed AT mode/IrMC functions
 * \ingroup SEPhone
 * @{
 */

GSM_Error ATOBEX_GetMemoryStatus(GSM_StateMachine *s, GSM_MemoryStatus *Status)
{
	GSM_Error 		error;

	if (Status->MemoryType == MEM_ME) {
		if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE)
			goto atgen;
		return OBEXGEN_GetMemoryStatus(s, Status);
	}

atgen:
	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetMemoryStatus(s, Status);
}

GSM_Error ATOBEX_GetMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	GSM_Error 		error;

	if (entry->MemoryType == MEM_ME) {
		if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE)
			goto atgen;
		return OBEXGEN_GetMemory(s, entry);
	}

atgen:
	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetMemory(s, entry);
}

GSM_Error ATOBEX_GetNextMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry, bool start)
{
	GSM_Error 		error;

	if (entry->MemoryType == MEM_ME) {
		if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE)
			goto atgen;
		return OBEXGEN_GetNextMemory(s, entry, start);
	}

atgen:
	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetNextMemory(s, entry, start);
}

GSM_Error ATOBEX_SetMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	GSM_Error 		error;

	if (entry->MemoryType == MEM_ME) {
		if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE)
			goto atgen;
		return OBEXGEN_SetMemory(s, entry);
	}

atgen:
	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetMemory(s, entry);
}

GSM_Error ATOBEX_AddMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	GSM_Error 		error;

	if (entry->MemoryType == MEM_ME) {
		if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE)
			goto atgen;
		return OBEXGEN_AddMemory(s, entry);
	}

atgen:
	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_AddMemory(s, entry);
}

GSM_Error ATOBEX_DeleteMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	GSM_Error 		error;

	if (entry->MemoryType == MEM_ME) {
		if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE)
			goto atgen;
		return OBEXGEN_DeleteMemory(s, entry);
	}

atgen:
	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_DeleteMemory(s, entry);
}

GSM_Error ATOBEX_DeleteAllMemory(GSM_StateMachine *s, GSM_MemoryType type)
{
	GSM_Error 		error;

	if (type == MEM_ME) {
		if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE)
			goto atgen;
		return OBEXGEN_DeleteAllMemory(s, type);
	}

atgen:
	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_DeleteAllMemory(s, type);
}

/*@}*/
/**
 * \defgroup SEIrMC Native IrMC functions using OBEX connection
 * \ingroup SEPhone
 * @{
 */

GSM_Error ATOBEX_GetToDoStatus(GSM_StateMachine *s, GSM_ToDoStatus *status)
{
	GSM_Error 		error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE) return error;
	return OBEXGEN_GetTodoStatus(s, status);
}

GSM_Error ATOBEX_GetToDo (GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	GSM_Error 		error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE) return error;
	return OBEXGEN_GetTodo(s, ToDo);
}

GSM_Error ATOBEX_GetNextToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo, bool start)
{
	GSM_Error 		error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE) return error;
	return OBEXGEN_GetNextTodo(s, ToDo, start);
}

GSM_Error ATOBEX_DeleteAllToDo (GSM_StateMachine *s)
{
	GSM_Error 		error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE) return error;
	return OBEXGEN_DeleteAllTodo(s);
}

GSM_Error ATOBEX_AddToDo (GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	GSM_Error 		error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE) return error;
	return OBEXGEN_AddTodo(s, ToDo);
}

GSM_Error ATOBEX_SetToDo (GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	GSM_Error 		error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE) return error;
	return OBEXGEN_SetTodo(s, ToDo);
}

GSM_Error ATOBEX_DeleteToDo (GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	GSM_Error 		error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE) return error;
	return OBEXGEN_DeleteTodo(s, ToDo);
}

GSM_Error ATOBEX_GetCalendarStatus(GSM_StateMachine *s, GSM_CalendarStatus *status)
{
	GSM_Error 		error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE) return error;
	return OBEXGEN_GetCalendarStatus(s, status);
}

GSM_Error ATOBEX_GetCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	GSM_Error 		error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE) return error;
	return OBEXGEN_GetCalendar(s, Note);
}

GSM_Error ATOBEX_GetNextCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool start)
{
	GSM_Error 		error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE) return error;
	return OBEXGEN_GetNextCalendar(s, Note, start);
}

GSM_Error ATOBEX_DeleteCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	GSM_Error 		error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE) return error;
	return OBEXGEN_DeleteCalendar(s, Note);
}

GSM_Error ATOBEX_AddCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	GSM_Error 		error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE) return error;
	return OBEXGEN_AddCalendar(s, Note);
}

GSM_Error ATOBEX_SetCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	GSM_Error 		error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE) return error;
	return OBEXGEN_SetCalendar(s, Note);
}

GSM_Error ATOBEX_DeleteAllCalendar (GSM_StateMachine *s)
{
	GSM_Error 		error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE) return error;
	return OBEXGEN_DeleteAllCalendar(s);
}

GSM_Error ATOBEX_GetNoteStatus(GSM_StateMachine *s, GSM_ToDoStatus *status)
{
	GSM_Error 		error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE) return error;
	return OBEXGEN_GetNoteStatus(s, status);
}

GSM_Error ATOBEX_GetNote (GSM_StateMachine *s, GSM_NoteEntry *Note)
{
	GSM_Error 		error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE) return error;
	return OBEXGEN_GetNote(s, Note);
}

GSM_Error ATOBEX_GetNextNote(GSM_StateMachine *s, GSM_NoteEntry *Note, bool start)
{
	GSM_Error 		error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE) return error;
	return OBEXGEN_GetNextNote(s, Note, start);
}

GSM_Error ATOBEX_DeleteAllNotes(GSM_StateMachine *s)
{
	GSM_Error 		error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE) return error;
	return OBEXGEN_DeleteAllNotes(s);
}

GSM_Error ATOBEX_AddNote (GSM_StateMachine *s, GSM_NoteEntry *Note)
{
	GSM_Error 		error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE) return error;
	return OBEXGEN_AddNote(s, Note);
}

GSM_Error ATOBEX_SetNote (GSM_StateMachine *s, GSM_NoteEntry *Note)
{
	GSM_Error 		error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE) return error;
	return OBEXGEN_SetNote(s, Note);
}

GSM_Error ATOBEX_DeleteNote (GSM_StateMachine *s, GSM_NoteEntry *Note)
{
	GSM_Error 		error;

	if ((error = ATOBEX_SetOBEXMode(s, OBEX_IRMC))!= ERR_NONE) return error;
	return OBEXGEN_DeleteNote(s, Note);
}


/*@}*/
/**
 * \defgroup SENativeAT Native AT mode functions
 * \ingroup SEPhone
 * @{
 */

/**
 * \author Peter Ondraska, based on code by Marcin Wiacek and Michal Cihar
 *
 * License: Whatever the current maintainer of gammulib chooses, as long as there
 * is an easy way to obtain the source under GPL, otherwise the author's parts
 * of this function are GPL 2.0.
 */
GSM_Error ATOBEX_ReplyGetDateLocale(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Locale	*locale = s->Phone.Data.Locale;
	int		format;
	char		*pos;

	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Date settings received\n");
		pos = strstr(msg.Buffer, "*ESDF:");
		if (pos == NULL) return ERR_UNKNOWNRESPONSE;
		format = atoi(pos + 7);
		switch (format) {
			case 0: locale->DateFormat 	= GSM_Date_OFF;
				locale->DateSeparator 	= 0;
				break;
			case 1: locale->DateFormat 	= GSM_Date_DDMMMYY;
				locale->DateSeparator 	= '-';
				break;
			case 2: locale->DateFormat 	= GSM_Date_DDMMYY;
				locale->DateSeparator 	= '-';
				break;
			case 3: locale->DateFormat 	= GSM_Date_MMDDYY;
				locale->DateSeparator 	= '/';
				break;
			case 4: locale->DateFormat 	= GSM_Date_DDMMYY;
				locale->DateSeparator 	= '/';
				break;
			case 5: locale->DateFormat 	= GSM_Date_DDMMYY;
				locale->DateSeparator 	= '.';
				break;
			case 6: locale->DateFormat 	= GSM_Date_YYMMDD;
				locale->DateSeparator 	= 0;
				break;
			case 7: locale->DateFormat 	= GSM_Date_YYMMDD;
				locale->DateSeparator 	= '-';
				break;
			default:return ERR_UNKNOWNRESPONSE;
		}
		return ERR_NONE;
	default:return ERR_NOTSUPPORTED;
	}
}

/**
 * \author Peter Ondraska, based on code by Marcin Wiacek and Michal Cihar
 *
 * License: Whatever the current maintainer of gammulib chooses, as long as there
 * is an easy way to obtain the source under GPL, otherwise the author's parts
 * of this function are GPL 2.0.
 */
GSM_Error ATOBEX_ReplyGetTimeLocale(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int		format;
	char		*pos;

	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
		case AT_Reply_OK:
			smprintf(s, "Time settings received\n");
			pos = strstr(msg.Buffer, "*ESTF:");
			if (pos == NULL) return ERR_UNKNOWNRESPONSE;
			format = atoi(pos + 7);
			switch (format) {
				case 1:
				case 2: s->Phone.Data.Locale->AMPMTime=(format==2);
					return ERR_NONE;
				default:return ERR_UNKNOWNRESPONSE;
			}
		default: return ERR_NOTSUPPORTED;
	}
}

/**
 * \author Peter Ondraska, based on code by Marcin Wiacek and Michal Cihar
 *
 * License: Whatever the current maintainer of gammulib chooses, as long as there
 * is an easy way to obtain the source under GPL, otherwise the author's parts
 * of this function are GPL 2.0.
 */
GSM_Error ATOBEX_GetLocale(GSM_StateMachine *s, GSM_Locale *locale)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;


	s->Phone.Data.Locale = locale;

	smprintf(s, "Getting date format\n");
	error=GSM_WaitFor (s, "AT*ESDF?\r", 9, 0x00, 3, ID_GetLocale);
	if (error!=ERR_NONE) return error;

	smprintf(s, "Getting time format\n");
	return GSM_WaitFor (s, "AT*ESTF?\r", 9, 0x00, 3, ID_GetLocale);
}

/**
 * \author Peter Ondraska, based on code by Marcin Wiacek and Michal Cihar
 *
 * License: Whatever the current maintainer of gammulib chooses, as long as there
 * is an easy way to obtain the source under GPL, otherwise the author's parts
 * of this function are GPL 2.0.
 */
GSM_Error ATOBEX_SetLocale(GSM_StateMachine *s, GSM_Locale *locale)
{
	/* this is not yet supported by gammu.c */
	int	format=0;
	char	req[12];
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;

	if (locale->DateFormat==GSM_Date_OFF) { format=0; } else
	if ((locale->DateFormat==GSM_Date_DDMMMYY)&&(locale->DateSeparator=='-')) { format=1; } else
	if ((locale->DateFormat==GSM_Date_DDMMYY)&&(locale->DateSeparator=='-')) { format=2; } else
	if ((locale->DateFormat==GSM_Date_MMDDYY)&&(locale->DateSeparator=='/')) { format=3; } else
	if ((locale->DateFormat==GSM_Date_DDMMYY)&&(locale->DateSeparator=='/')) { format=4; } else
	if ((locale->DateFormat==GSM_Date_DDMMYY)&&(locale->DateSeparator=='.')) { format=5; } else
	if ((locale->DateFormat==GSM_Date_YYMMDD)&&(locale->DateSeparator==0)) { format=6; } else
	if ((locale->DateFormat==GSM_Date_YYMMDD)&&(locale->DateSeparator=='-')) { format=7; }
	else { return ERR_NOTSUPPORTED; } /* ERR_WRONGINPUT */

	sprintf(req,"AT*ESDF=%i\r",format);
	smprintf(s, "Setting date format\n");
	error = GSM_WaitFor (s, req, strlen(req), 0x00, 3, ID_SetLocale);
	if (error!=ERR_NONE) return error;

	if (locale->AMPMTime) { format=2; } else { format=1; }
	sprintf(req,"AT*ESTF=%i\r",format);
	smprintf(s, "Setting time format\n");
	return GSM_WaitFor (s, req, strlen(req), 0x00, 3, ID_SetLocale);
}

GSM_Error ATOBEX_ReplyGetFileSystemStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Error error;

	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
		case AT_Reply_OK:
			error = ATGEN_ParseReply(s,
					GetLineString(msg.Buffer, &s->Phone.Data.Priv.ATGEN.Lines, 2),
					"*EMEM: @i, @i, @i, @i, @i",
					&s->Phone.Data.FileSystemStatus->Free,
					&s->Phone.Data.FileSystemStatus->Used,
					&s->Phone.Data.FileSystemStatus->UsedImages,
					&s->Phone.Data.FileSystemStatus->UsedSounds,
					&s->Phone.Data.FileSystemStatus->UsedThemes
					);

			if (error == ERR_NONE) {
				/* This is actually total */
				s->Phone.Data.FileSystemStatus->Used -= s->Phone.Data.FileSystemStatus->Free;
				return ERR_NONE;
			}

			return error;
		default:
			return ERR_NOTSUPPORTED;
	}
}

GSM_Error ATOBEX_GetFileSystemStatus(GSM_StateMachine *s, GSM_FileSystemStatus *Status)
{
	GSM_Error error;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;

	s->Phone.Data.FileSystemStatus = Status;

	return GSM_WaitFor (s, "AT*EMEM\r", 8, 0x00, 3, ID_FileSystemStatus);
}

GSM_Error ATOBEX_ReplyGetBatteryCharge(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int tmp, ncapacity, method, state;
	int vbat1, vbat2, vbat3, vbat4;
	GSM_Error error;
	GSM_BatteryCharge *bat = s->Phone.Data.BatteryCharge;

	if (bat == NULL) {
		smprintf(s, "Battery status received, but not requested right now\n");
		return ERR_NONE;
	}
	smprintf(s, "Battery status received\n");

	/* Parse version 4 reply */
	error = ATGEN_ParseReply(s,
				msg.Buffer,
				"*EBCA: @i, @i, @i, @i, @i, @i, @i, @i, @i, @i, @i, @i, @i, @i\xd\xa",
				&bat->BatteryVoltage, /* vbat */
				&bat->ChargeVoltage, /* dcio */
				&bat->ChargeCurrent, /* iocharge */
				&bat->PhoneCurrent, /* iphone */
				&bat->BatteryTemperature, /* tempbattery */
				&bat->PhoneTemperature, /* tempphone */
				&method, /* chargingmethod, used to determine battery type */
				&state, /* chargestate */
				&bat->BatteryCapacity, /* remcapacity */
				&bat->BatteryPercent, /* remcapacitypercent */
				&tmp, /* powerdissipation */
				&tmp, /* noccycles */
				&tmp, /* nosostimer */
				&tmp /* suspensioncause */
				);
	/* Did we suceed? */
	if (error == ERR_NONE) {
		/* Posprocess fields that need it */
		bat->ChargeCurrent /= 10;
		bat->PhoneCurrent /= 10;
		switch (method) {
			case 0:
				bat->BatteryType = GSM_BatteryLiPol;
				break;
			case 1:
				bat->BatteryType = GSM_BatteryLiIon;
				break;
			case 2:
				bat->BatteryType = GSM_BatteryNiMH;
				break;
			default:
				bat->BatteryType = GSM_BatteryUnknown;
				break;
		}
		switch (state) {
			case 7:
				bat->ChargeState = GSM_BatteryPowered;
				break;
			case 2:
				bat->ChargeState = GSM_BatteryCharging;
				break;
			case 0:
			case 3:
			case 4:
			case 5:
			case 15:
				bat->ChargeState = GSM_BatteryConnected;
				break;
			case 8:
				bat->ChargeState = GSM_BatteryFull;
				break;
		}
		s->Phone.Data.BatteryCharge = NULL;
		return ERR_NONE;
	}

	/* Parse version 2 reply */
	error = ATGEN_ParseReply(s,
				msg.Buffer,
				"*EBCA: @i, @i, @i, @i, @i, @i, @i, @i, @i, @i, @i, @i, @i, @i, @i, @i, @i, @i, @i, @i, @i, @i, @i, @i, @i, @i, @i\xd\xa",
				&vbat1, /* vbat1 */
				&vbat2, /* vbat2 */
				&vbat3, /* vbat3 */
				&vbat4, /* vbat4 */
				&method, /* btype */
				&bat->ChargeVoltage, /* dcio */
				&bat->ChargeCurrent, /* iocharge */
				&bat->PhoneCurrent, /* iphone */
				&tmp, /* acapacity */
				&tmp, /* ccapacity */
				&tmp, /* pcapacity */
				&ncapacity, /* ncapacity */
				&bat->BatteryTemperature, /* tempbattery */
				&bat->PhoneTemperature, /* tempphone */
				&state, /* chargestate */
				&bat->BatteryPercent, /* remcapacitypercent */
				&tmp, /* cycles */
				&tmp, /* ipulse */
				&tmp, /* ibattery */
				&tmp, /* ChTempMax */
				&tmp, /* ChTempMin */
				&tmp, /* MainChTempMax */
				&tmp, /* MainChTempMin */
				&tmp, /* FlatVTimer */
				&tmp, /* DV */
				&tmp, /* DT */
				&tmp /* D2V */
				);
	/* Did we suceed? */
	if (error == ERR_NONE) {
		/* Posprocess fields that need it */
		if (vbat4 > 0) {
			bat->BatteryVoltage = vbat4;
		} else if (vbat3 > 0) {
			bat->BatteryVoltage = vbat3;
		} else if (vbat2 > 0) {
			bat->BatteryVoltage = vbat2;
		} else if (vbat1 > 0) {
			bat->BatteryVoltage = vbat1;
		}
		bat->ChargeVoltage *= 10;
		switch (method) {
			case 0:
				bat->BatteryType = GSM_BatteryLiPol;
				bat->BatteryType = GSM_BatteryNiMH;
				break;
			case 1:
				bat->BatteryType = GSM_BatteryLiIon;
				break;
			case 2:
			default:
				bat->BatteryType = GSM_BatteryUnknown;
				break;
		}
		switch (state) {
			case 3:
				bat->ChargeState = GSM_BatteryPowered;
				break;
			case 0:
			case 1:
			case 2:
				bat->ChargeState = GSM_BatteryCharging;
				break;
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
				bat->ChargeState = GSM_BatteryFull;
				break;
		}
		/* Calculate remaining capacity */
		bat->BatteryCapacity = 1000 * ncapacity / bat->BatteryPercent;
		s->Phone.Data.BatteryCharge = NULL;
		return ERR_NONE;
	}

	smprintf(s, "Unsupported battery status format, you're welcome to help with implementation\n");
	s->Phone.Data.BatteryCharge = NULL;
	return ERR_NOTIMPLEMENTED;
}

GSM_Error ATOBEX_GetBatteryCharge(GSM_StateMachine *s, GSM_BatteryCharge *bat)
{
	GSM_Error error, error2;
	int	i = 0;
	GSM_Phone_ATOBEXData	*Priv = &s->Phone.Data.Priv.ATOBEX;

	s->Phone.Data.BatteryCharge = bat;

	if ((error = ATOBEX_SetATMode(s))!= ERR_NONE) return error;

	/* Go to AT if EBCA does not work */
	if (Priv->EBCAFailed) {
		return ATGEN_GetBatteryCharge(s, bat);
	}

	/* Now try ericsson extended reporting */
	error = GSM_WaitFor (s, "AT*EBCA=1\r", 10, 0x00, 3, ID_GetBatteryCharge);
	if (error != ERR_NONE) {
		Priv->EBCAFailed = true;
		/* Ty ATGEN state */
		return ATGEN_GetBatteryCharge(s, bat);
	}
	/* Wait for async phone reply */
	while (s->Phone.Data.BatteryCharge != NULL) {
		error = GSM_WaitFor (s, "AT\r", 3, 0x00, 3, ID_GetBatteryCharge);
		smprintf(s, "Loop %d, error %d\n", i, error);
		if (i == 20) break;
		if (error != ERR_NONE) {
			break;
		}
		i++;
	}
	/* Disable reading information */
	error2 = GSM_WaitFor (s, "AT*EBCA=0\r", 10, 0x00, 3, ID_GetBatteryCharge);
	if (error2 != ERR_NONE) {
		return error;
	}
	/* If something failed, do AT way */
	if (error != ERR_NONE) {
		return ATGEN_GetBatteryCharge(s, bat);
	}
	/* Did we timeout? */
	if (i == 20) return ERR_TIMEOUT;
	return error;
}

/*@}*/


GSM_Phone_Functions ATOBEXPhone = {
	/* There is much more SE phones which support this! */
	"sonyericsson|ericsson|atobex",
	ATGENReplyFunctions,
	ATOBEX_Initialise,
	ATOBEX_Terminate,
	ATOBEX_DispatchMessage,
	NOTSUPPORTED,			/* 	ShowStartInfo		*/
	ATOBEX_GetManufacturer,
	ATOBEX_GetModel,
	ATOBEX_GetFirmware,
	ATOBEX_GetIMEI,
	NOTSUPPORTED,			/* 	GetOriginalIMEI		*/
	NOTSUPPORTED,			/* 	GetManufactureMonth	*/
        ATOBEX_GetProductCode,
	NOTSUPPORTED,			/* 	GetHardware		*/
	NOTSUPPORTED,			/* 	GetPPM			*/
	ATOBEX_GetSIMIMSI,
	ATOBEX_GetDateTime,
	ATOBEX_SetDateTime,
	ATOBEX_GetAlarm,
	ATOBEX_SetAlarm,
	ATOBEX_GetLocale,
	ATOBEX_SetLocale,
	ATOBEX_PressKey,
	ATOBEX_Reset,
	ATOBEX_ResetPhoneSettings,
	ATOBEX_EnterSecurityCode,
	ATOBEX_GetSecurityStatus,
	ATOBEX_GetDisplayStatus,
	ATOBEX_SetAutoNetworkLogin,
	ATOBEX_GetBatteryCharge,
	ATOBEX_GetSignalStrength,
	ATOBEX_GetNetworkInfo,
 	NOTSUPPORTED,       		/*  	GetCategory 		*/
 	NOTSUPPORTED,       		/*  	AddCategory 		*/
 	NOTSUPPORTED,        		/*  	GetCategoryStatus 	*/
	ATOBEX_GetMemoryStatus,
	ATOBEX_GetMemory,
	ATOBEX_GetNextMemory,
	ATOBEX_SetMemory,
	ATOBEX_AddMemory,
	ATOBEX_DeleteMemory,
	ATOBEX_DeleteAllMemory,
	NOTSUPPORTED,			/* 	GetSpeedDial		*/
	NOTSUPPORTED,			/* 	SetSpeedDial		*/
	ATOBEX_GetSMSC,
	ATOBEX_SetSMSC,
	ATOBEX_GetSMSStatus,
	ATOBEX_GetSMS,
	ATOBEX_GetNextSMS,
	NOTSUPPORTED,			/*	SetSMS			*/
	ATOBEX_AddSMS,
	ATOBEX_DeleteSMS,
	ATOBEX_SendSMS,
	ATOBEX_SendSavedSMS,
	ATOBEX_SetFastSMSSending,
	ATOBEX_SetIncomingSMS,
	ATOBEX_SetIncomingCB,
	ATOBEX_GetSMSFolders,
 	NOTSUPPORTED,			/* 	AddSMSFolder		*/
 	NOTSUPPORTED,			/* 	DeleteSMSFolder		*/
	ATOBEX_DialVoice,
	ATOBEX_DialService,
	ATOBEX_AnswerCall,
	ATOBEX_CancelCall,
 	NOTSUPPORTED,			/* 	HoldCall 		*/
 	NOTSUPPORTED,			/* 	UnholdCall 		*/
 	NOTSUPPORTED,			/* 	ConferenceCall 		*/
 	NOTSUPPORTED,			/* 	SplitCall		*/
 	NOTSUPPORTED,			/* 	TransferCall		*/
 	NOTSUPPORTED,			/* 	SwitchCall		*/
 	NOTSUPPORTED,			/* 	GetCallDivert		*/
 	NOTSUPPORTED,			/* 	SetCallDivert		*/
 	NOTSUPPORTED,			/* 	CancelAllDiverts	*/
	ATOBEX_SetIncomingCall,
	ATOBEX_SetIncomingUSSD,
	ATOBEX_SendDTMF,
	ATOBEX_GetRingtone,
	ATOBEX_SetRingtone,
	NOTSUPPORTED,			/* 	GetRingtonesInfo	*/
	NOTSUPPORTED,			/* 	DeleteUserRingtones	*/
	NOTSUPPORTED,			/* 	PlayTone		*/
	NOTSUPPORTED,			/* 	GetWAPBookmark		*/
	NOTSUPPORTED,			/* 	SetWAPBookmark		*/
	NOTSUPPORTED,			/* 	DeleteWAPBookmark	*/
	NOTSUPPORTED,			/* 	GetWAPSettings		*/
	NOTSUPPORTED,			/* 	SetWAPSettings		*/
	NOTSUPPORTED,			/*	GetSyncMLSettings	*/
	NOTSUPPORTED,			/*	SetSyncMLSettings	*/
	NOTSUPPORTED,			/*	GetChatSettings		*/
	NOTSUPPORTED,			/*	SetChatSettings		*/
	NOTSUPPORTED,			/* 	GetMMSSettings		*/
	NOTSUPPORTED,			/* 	SetMMSSettings		*/
	NOTSUPPORTED,			/*	GetMMSFolders		*/
	NOTSUPPORTED,			/*	GetNextMMSFileInfo	*/
	ATOBEX_GetBitmap,
	ATOBEX_SetBitmap,
	ATOBEX_GetToDoStatus,
	ATOBEX_GetToDo,
	ATOBEX_GetNextToDo,
	ATOBEX_SetToDo,
	ATOBEX_AddToDo,
	ATOBEX_DeleteToDo,
	ATOBEX_DeleteAllToDo,
	ATOBEX_GetCalendarStatus,
	ATOBEX_GetCalendar,
	ATOBEX_GetNextCalendar,
	ATOBEX_SetCalendar,
	ATOBEX_AddCalendar,
	ATOBEX_DeleteCalendar,
	ATOBEX_DeleteAllCalendar,
	NOTSUPPORTED,			/* 	GetCalendarSettings	*/
	NOTSUPPORTED,			/* 	SetCalendarSettings	*/
	ATOBEX_GetNoteStatus,
	ATOBEX_GetNote,
	ATOBEX_GetNextNote,
	ATOBEX_SetNote,
	ATOBEX_AddNote,
	ATOBEX_DeleteNote,
	ATOBEX_DeleteAllNotes,
	NOTSUPPORTED,			/* 	GetProfile		*/
	NOTSUPPORTED,			/* 	SetProfile		*/
	NOTSUPPORTED,			/* 	GetFMStation		*/
	NOTSUPPORTED,			/* 	SetFMStation		*/
	NOTSUPPORTED,			/* 	ClearFMStations		*/
	ATOBEX_GetNextFileFolder,
	NOTSUPPORTED,			/*	GetFolderListing	*/
	NOTSUPPORTED,			/*	GetNextRootFolder	*/
	NOTSUPPORTED,			/*	SetFileAttributes	*/
	ATOBEX_GetFilePart,
	ATOBEX_AddFilePart,
	ATOBEX_SendFilePart,
	ATOBEX_GetFileSystemStatus,
	ATOBEX_DeleteFile,
	ATOBEX_AddFolder,
	ATOBEX_DeleteFile,	/* 	DeleteFolder		*/
	NOTSUPPORTED,			/* 	GetGPRSAccessPoint	*/
	NOTSUPPORTED			/* 	SetGPRSAccessPoint	*/
};

#endif
#endif
/*@}*/
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

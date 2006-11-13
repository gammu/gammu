/* (c) 2006 by Michal Cihar */

/**
 * High level functions for communication with Sony-Ericsson phones.
 */

#include "../../gsmstate.h"

#ifdef GSM_ENABLE_SONYERICSSON
#ifdef GSM_ENABLE_ATGEN

#include <string.h>
#include <time.h>

#include "../../gsmcomon.h"
#include "../../misc/coding/coding.h"
#include "../../misc/misc.h"
#include "../../service/sms/gsmsms.h"
#include "../pfunc.h"
#include "../at/atgen-functions.h"
#include "../obex/obexgen-functions.h"
#include "sonyericsson.h"

extern GSM_Reply_Function SONYERICSSONReplyFunctions[];


/**
 * Ensures phone and Gammu protocol are switched to AT commands mode.
 */
GSM_Error SONYERICSSON_SetATMode(GSM_StateMachine *s)
{
	GSM_Phone_SONYERICSSONData	*Priv = &s->Phone.Data.Priv.SONYERICSSON;
	GSM_Error		error;

	/* Aren't we in OBEX mode? */
	if (Priv->Mode == SONYERICSSON_ModeAT) return ERR_NONE;

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
	Priv->Mode				= SONYERICSSON_ModeAT;

	/* Initialise AT protocol */
	error = s->Protocol.Functions->Initialise(s);
	if (error != ERR_NONE) return error;

	return ERR_NONE;
}


/**
 * Ensures phone and Gammu protocol are in OBEX mode, in IrMC service
 * if requrested.
 */
GSM_Error SONYERICSSON_SetOBEXMode(GSM_StateMachine *s, bool irmc)
{
	GSM_Phone_SONYERICSSONData	*Priv = &s->Phone.Data.Priv.SONYERICSSON;
	GSM_Error		error;

	/* Is OBEX mode supported? */
	if (!Priv->HasOBEX) {
		return ERR_NOTSUPPORTED;
	}

	/* Are we already in OBEX mode? */
	if (Priv->Mode == SONYERICSSON_ModeOBEX) {
		/* We can not safely switch service, we need to disconnect instead */
		if (irmc) {
			if (s->Phone.Data.Priv.OBEXGEN.Service == OBEX_IRMC) {
				return ERR_NONE;
			} else {
				error = SONYERICSSON_SetATMode(s);
			}
		} else {
			if (s->Phone.Data.Priv.OBEXGEN.Service == OBEX_BrowsingFolders) {
				return ERR_NONE;
			} else {
				error = SONYERICSSON_SetATMode(s);
			}
		}
		if (error != ERR_NONE) return error;
	}

	dbgprintf ("Changing to OBEX mode\n");

	/* Switch phone to OBEX */
	error=GSM_WaitFor (s, "AT*EOBEX\r", 9, 0x00, 4, ID_SetOBEX);
	if (error != ERR_NONE) return error;

	/* Tell OBEX module it has no service selected */
	s->Phone.Data.Priv.OBEXGEN.Service = 0;

	dbgprintf ("Changing protocol to OBEX\n");

	/* Stop AT protocol */
	error = s->Protocol.Functions->Terminate(s);
	if (error != ERR_NONE) return error;

	/* Need some sleep before starting talk in OBEX */
	my_sleep(100);

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
	Priv->Mode				= SONYERICSSON_ModeOBEX;

	/* Choose appropriate connection type (we need different for filesystem and for IrMC) */
	if (irmc) {
		error = OBEXGEN_Connect(s, OBEX_IRMC);
	} else {
		error = OBEXGEN_Connect(s, OBEX_BrowsingFolders);
	}
	if (error != ERR_NONE) return error;

	return ERR_NONE;
}


/**
 * Initialises Sony-Ericsson module internals and calls AT module init.
 */
GSM_Error SONYERICSSON_Initialise(GSM_StateMachine *s)
{
	GSM_Phone_SONYERICSSONData	*Priv = &s->Phone.Data.Priv.SONYERICSSON;
	GSM_Error		error;

	Priv->Mode				= SONYERICSSON_ModeAT;

	/* Init OBEX module also */
	error = OBEXGEN_InitialiseVars(s);
	if (error != ERR_NONE) return error;

	/* Init AT module */
	error = ATGEN_Initialise(s);
	if (error != ERR_NONE) return error;

	/* Do we have OBEX capability? */
	error = GSM_WaitFor (s, "AT*EOBEX=?\r", 11, 0x00, 4, ID_SetOBEX);
	if (error == ERR_NONE) {
		Priv->HasOBEX = true;
	} else {
		Priv->HasOBEX = false;
	}

	return ERR_NONE;
}

/**
 * Switch to AT mode and calls AT module termination procedure.
 */
GSM_Error SONYERICSSON_Terminate(GSM_StateMachine *s)
{
	GSM_Error 		error;

	error = SONYERICSSON_SetATMode(s);
	return ATGEN_Terminate(s);
}


/**
 * Dispatches message to correct dispatcher according to active protocol
 */
GSM_Error SONYERICSSON_DispatchMessage(GSM_StateMachine *s)
{
	if (s->Phone.Data.Priv.SONYERICSSON.Mode == SONYERICSSON_ModeOBEX) {
		return GSM_DispatchMessage(s);
	} else {
		return ATGEN_DispatchMessage(s);
	}
}


/**
 * We receive product code over AT commands, so we can easily use it
 */
GSM_Error SONYERICSSON_GetProductCode(GSM_StateMachine *s, char *value)
{
       strcpy(value, s->Phone.Data.Model);
       return ERR_NONE;
}


/* Wrapper functions for using AT module functionality */

GSM_Error SONYERICSSON_GetIMEI (GSM_StateMachine *s)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetIMEI(s);
}

GSM_Error SONYERICSSON_GetFirmware(GSM_StateMachine *s)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetFirmware(s);
}

GSM_Error SONYERICSSON_GetModel(GSM_StateMachine *s)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetModel(s);
}

GSM_Error SONYERICSSON_GetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetDateTime(s, date_time);
}

GSM_Error SONYERICSSON_GetSMS(GSM_StateMachine *s, GSM_MultiSMSMessage *sms)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetSMS(s, sms);
}

GSM_Error SONYERICSSON_DeleteSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_DeleteSMS(s, sms);
}

GSM_Error SONYERICSSON_AddSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_AddSMS(s, sms);
}

GSM_Error SONYERICSSON_GetBatteryCharge(GSM_StateMachine *s, GSM_BatteryCharge *bat)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetBatteryCharge(s, bat);
}

GSM_Error SONYERICSSON_GetSignalStrength(GSM_StateMachine *s, GSM_SignalQuality *sig)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetSignalQuality(s, sig);
}

GSM_Error SONYERICSSON_GetSMSFolders(GSM_StateMachine *s, GSM_SMSFolders *folders)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetSMSFolders(s, folders);
}

GSM_Error SONYERICSSON_GetNextSMS(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, bool start)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetNextSMS(s, sms, start);
}

GSM_Error SONYERICSSON_GetSMSStatus(GSM_StateMachine *s, GSM_SMSMemoryStatus *status)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetSMSStatus(s, status);
}

GSM_Error SONYERICSSON_DialVoice(GSM_StateMachine *s, char *number, GSM_CallShowNumber ShowNumber)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_DialVoice(s, number, ShowNumber);
}

GSM_Error SONYERICSSON_AnswerCall(GSM_StateMachine *s, int ID, bool all)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_AnswerCall(s,ID,all);
}

GSM_Error SONYERICSSON_GetNetworkInfo(GSM_StateMachine *s, GSM_NetworkInfo *netinfo)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetNetworkInfo(s, netinfo);
}

GSM_Error SONYERICSSON_GetDisplayStatus(GSM_StateMachine *s, GSM_DisplayFeatures *features)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetDisplayStatus(s, features);
}

GSM_Error SONYERICSSON_SetAutoNetworkLogin(GSM_StateMachine *s)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetAutoNetworkLogin(s);
}

GSM_Error SONYERICSSON_PressKey(GSM_StateMachine *s, GSM_KeyCode Key, bool Press)
{
	GSM_Error error;

	/* @todo: Implement completely using AT*EKEY */
	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_PressKey(s, Key, Press);
}

GSM_Error SONYERICSSON_Reset(GSM_StateMachine *s, bool hard)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_Reset(s, hard);
}

GSM_Error SONYERICSSON_CancelCall(GSM_StateMachine *s, int ID, bool all)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_CancelCall(s,ID,all);
}

GSM_Error SONYERICSSON_SendSavedSMS(GSM_StateMachine *s, int Folder, int Location)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SendSavedSMS(s, Folder, Location);
}

GSM_Error SONYERICSSON_SendSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SendSMS(s, sms);
}

GSM_Error SONYERICSSON_SetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetDateTime(s, date_time);
}

GSM_Error SONYERICSSON_SetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetSMSC(s, smsc);
}

GSM_Error SONYERICSSON_GetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetSMSC(s, smsc);
}

GSM_Error SONYERICSSON_EnterSecurityCode(GSM_StateMachine *s, GSM_SecurityCode Code)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_EnterSecurityCode(s, Code);
}

GSM_Error SONYERICSSON_GetSecurityStatus(GSM_StateMachine *s, GSM_SecurityCodeType *Status)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetSecurityStatus(s, Status);
}

GSM_Error SONYERICSSON_ResetPhoneSettings(GSM_StateMachine *s, GSM_ResetSettingsType Type)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_ResetPhoneSettings(s, Type);
}

GSM_Error SONYERICSSON_SendDTMF(GSM_StateMachine *s, char *sequence)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SendDTMF(s, sequence);
}

GSM_Error SONYERICSSON_GetSIMIMSI(GSM_StateMachine *s, char *IMSI)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetSIMIMSI(s, IMSI);
}

GSM_Error SONYERICSSON_SetIncomingCB (GSM_StateMachine *s, bool enable)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetIncomingCB(s, enable);
}

GSM_Error SONYERICSSON_SetIncomingSMS (GSM_StateMachine *s, bool enable)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetIncomingSMS(s, enable);
}

GSM_Error SONYERICSSON_SetFastSMSSending(GSM_StateMachine *s, bool enable)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetFastSMSSending(s, enable);
}

GSM_Error SONYERICSSON_GetManufacturer(GSM_StateMachine *s)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetManufacturer(s);
}

GSM_Error SONYERICSSON_GetAlarm(GSM_StateMachine *s, GSM_Alarm *alarm)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetAlarm(s, alarm);
}

GSM_Error SONYERICSSON_SetAlarm(GSM_StateMachine *s, GSM_Alarm *alarm)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetAlarm(s, alarm);
}

GSM_Error SONYERICSSON_SetIncomingUSSD(GSM_StateMachine *s, bool enable)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetIncomingUSSD(s, enable);
}

GSM_Error SONYERICSSON_GetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, bool PhoneRingtone)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetRingtone(s, Ringtone, PhoneRingtone);
}

GSM_Error SONYERICSSON_SetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, int *maxlength)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetRingtone(s, Ringtone, maxlength);
}

GSM_Error SONYERICSSON_GetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_GetBitmap(s, Bitmap);
}

GSM_Error SONYERICSSON_SetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
	return ATGEN_SetBitmap(s, Bitmap);
}

/* OBEX native functions for filesystem */

GSM_Error SONYERICSSON_AddFilePart(GSM_StateMachine *s, GSM_File *File, int *Pos, int *Handle)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetOBEXMode(s, false))!= ERR_NONE) return error;
	return OBEXGEN_AddFilePart(s, File, Pos, Handle);
}

GSM_Error SONYERICSSON_GetFilePart(GSM_StateMachine *s, GSM_File *File, int *Handle, int *Size)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetOBEXMode(s, false))!= ERR_NONE) return error;
	return OBEXGEN_GetFilePart(s, File, Handle, Size);
}

GSM_Error SONYERICSSON_GetNextFileFolder(GSM_StateMachine *s, GSM_File *File, bool start)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetOBEXMode(s, false))!= ERR_NONE) return error;
	return OBEXGEN_GetNextFileFolder(s, File, start);
}

GSM_Error SONYERICSSON_DeleteFile(GSM_StateMachine *s, unsigned char *ID)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetOBEXMode(s, false))!= ERR_NONE) return error;
	return OBEXGEN_DeleteFile(s, ID);
}

GSM_Error SONYERICSSON_AddFolder(GSM_StateMachine *s, GSM_File *File)
{
	GSM_Error error;

	if ((error = SONYERICSSON_SetOBEXMode(s, false))!= ERR_NONE) return error;
	return OBEXGEN_AddFolder(s, File);
}

/* Mixed AT mode/IrMC functions */

GSM_Error SONYERICSSON_GetMemoryStatus(GSM_StateMachine *s, GSM_MemoryStatus *Status)
{
	GSM_Error 		error;

	if (Status->MemoryType == MEM_ME) {
		if ((error = SONYERICSSON_SetOBEXMode(s, true))!= ERR_NONE) return error;
		return OBEXGEN_GetMemoryStatus(s, Status);
	} else {
		if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
		return ATGEN_GetMemoryStatus(s, Status);
	}
}

GSM_Error SONYERICSSON_GetMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	GSM_Error 		error;

	if (entry->MemoryType == MEM_ME) {
		if ((error = SONYERICSSON_SetOBEXMode(s, true))!= ERR_NONE) return error;
		return OBEXGEN_GetMemory(s, entry);
	} else {
		if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
		return ATGEN_GetMemory(s, entry);
	}
}

GSM_Error SONYERICSSON_GetNextMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry, bool start)
{
	GSM_Error 		error;

	if (entry->MemoryType == MEM_ME) {
		if ((error = SONYERICSSON_SetOBEXMode(s, true))!= ERR_NONE) return error;
		return OBEXGEN_GetNextMemory(s, entry, start);
	} else {
		if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
		return ATGEN_GetNextMemory(s, entry, start);
	}
}

GSM_Error SONYERICSSON_SetMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	GSM_Error 		error;

	if (entry->MemoryType == MEM_ME) {
		return ERR_NOTIMPLEMENTED;
	} else {
		if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
		return ATGEN_SetMemory(s, entry);
	}
}

GSM_Error SONYERICSSON_AddMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	GSM_Error 		error;

	if (entry->MemoryType == MEM_ME) {
		return ERR_NOTIMPLEMENTED;
	} else {
		if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
		return ATGEN_AddMemory(s, entry);
	}
}

GSM_Error SONYERICSSON_DeleteMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	GSM_Error 		error;

	if (entry->MemoryType == MEM_ME) {
		return ERR_NOTIMPLEMENTED;
	} else {
		if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
		return ATGEN_DeleteMemory(s, entry);
	}
}

GSM_Error SONYERICSSON_DeleteAllMemory(GSM_StateMachine *s, GSM_MemoryType type)
{
	GSM_Error 		error;

	if (type == MEM_ME) {
		return ERR_NOTIMPLEMENTED;
	} else {
		if ((error = SONYERICSSON_SetATMode(s))!= ERR_NONE) return error;
		return ATGEN_DeleteAllMemory(s, type);
	}
}
/* Native IrMC functions using OBEX connection */

GSM_Error SONYERICSSON_GetToDoStatus(GSM_StateMachine *s, GSM_ToDoStatus *status)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error SONYERICSSON_GetToDo (GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error SONYERICSSON_GetNextToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo, bool start)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error SONYERICSSON_DeleteAllToDo (GSM_StateMachine *s)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error SONYERICSSON_AddToDo (GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error SONYERICSSON_SetToDo (GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error SONYERICSSON_DeleteToDo (GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error SONYERICSSON_GetCalendarStatus(GSM_StateMachine *s, GSM_CalendarStatus *status)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error SONYERICSSON_GetCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error SONYERICSSON_GetNextCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool start)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error SONYERICSSON_DeleteCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error SONYERICSSON_AddCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error SONYERICSSON_SetCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error SONYERICSSON_DeleteAllCalendar (GSM_StateMachine *s)
{
	return ERR_NOTIMPLEMENTED;
}

/* Native AT mode functions */

GSM_Error SONYERICSSON_ReplyGetDateLocale(GSM_Protocol_Message msg, GSM_StateMachine *s)
{ /*	Author: Peter Ondraska, based on code by Marcin Wiacek and Michal Cihar
	License: Whatever the current maintainer of gammulib chooses, as long as there
	is an easy way to obtain the source under GPL, otherwise the author's parts
	of this function are GPL 2.0.
  */
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

GSM_Error SONYERICSSON_ReplyGetTimeLocale(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
/*	Author: Peter Ondraska
	License: Whatever the current maintainer of gammulib chooses, as long as there
	is an easy way to obtain the source under GPL, otherwise the author's parts
	of this function are GPL 2.0.
 */
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

GSM_Error SONYERICSSON_GetLocale(GSM_StateMachine *s, GSM_Locale *locale)
{
	GSM_Error error;

	s->Phone.Data.Locale = locale;

	smprintf(s, "Getting date format\n");
	error=GSM_WaitFor (s, "AT*ESDF?\r", 9, 0x00, 3, ID_GetLocale);
	if (error!=ERR_NONE) return error;

	smprintf(s, "Getting time format\n");
	return GSM_WaitFor (s, "AT*ESTF?\r", 9, 0x00, 3, ID_GetLocale);
}

GSM_Error SONYERICSSON_SetLocale(GSM_StateMachine *s, GSM_Locale *locale)
{
/*	Author: Peter Ondraska
	License: Whatever the current maintainer of gammulib chooses, as long as there
	is an easy way to obtain the source under GPL, otherwise the author's parts
	of this function are GPL 2.0.
 */
	/* this is not yet supported by gammu.c */
	int	format=0;
	char	req[12];
	GSM_Error error;

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


GSM_Phone_Functions SONYERICSSONPhone = {
	/* There is much more SE phones which support this! */
	"sonyericsson|ericsson|sony|k750|k750i",
	ATGENReplyFunctions,
	SONYERICSSON_Initialise,
	SONYERICSSON_Terminate,
	SONYERICSSON_DispatchMessage,
	NOTSUPPORTED,			/* 	ShowStartInfo		*/
	SONYERICSSON_GetManufacturer,
	SONYERICSSON_GetModel,
	SONYERICSSON_GetFirmware,
	SONYERICSSON_GetIMEI,
	NOTSUPPORTED,			/* 	GetOriginalIMEI		*/
	NOTSUPPORTED,			/* 	GetManufactureMonth	*/
        SONYERICSSON_GetProductCode,
	NOTSUPPORTED,			/* 	GetHardware		*/
	NOTSUPPORTED,			/* 	GetPPM			*/
	SONYERICSSON_GetSIMIMSI,
	SONYERICSSON_GetDateTime,
	SONYERICSSON_SetDateTime,
	SONYERICSSON_GetAlarm,
	SONYERICSSON_SetAlarm,
	SONYERICSSON_GetLocale,
	SONYERICSSON_SetLocale,
	SONYERICSSON_PressKey,
	SONYERICSSON_Reset,
	SONYERICSSON_ResetPhoneSettings,
	SONYERICSSON_EnterSecurityCode,
	SONYERICSSON_GetSecurityStatus,
	SONYERICSSON_GetDisplayStatus,
	SONYERICSSON_SetAutoNetworkLogin,
	SONYERICSSON_GetBatteryCharge,
	SONYERICSSON_GetSignalStrength,
	SONYERICSSON_GetNetworkInfo,
 	NOTSUPPORTED,       		/*  	GetCategory 		*/
 	NOTSUPPORTED,       		/*  	AddCategory 		*/
 	NOTSUPPORTED,        		/*  	GetCategoryStatus 	*/
	SONYERICSSON_GetMemoryStatus,
	SONYERICSSON_GetMemory,
	SONYERICSSON_GetNextMemory,
	SONYERICSSON_SetMemory,
	SONYERICSSON_AddMemory,
	SONYERICSSON_DeleteMemory,
	SONYERICSSON_DeleteAllMemory,
	NOTSUPPORTED,			/* 	GetSpeedDial		*/
	NOTSUPPORTED,			/* 	SetSpeedDial		*/
	SONYERICSSON_GetSMSC,
	SONYERICSSON_SetSMSC,
	SONYERICSSON_GetSMSStatus,
	SONYERICSSON_GetSMS,
	SONYERICSSON_GetNextSMS,
	NOTSUPPORTED,			/*	SetSMS			*/
	SONYERICSSON_AddSMS,
	SONYERICSSON_DeleteSMS,
	SONYERICSSON_SendSMS,
	SONYERICSSON_SendSavedSMS,
	SONYERICSSON_SetFastSMSSending,
	SONYERICSSON_SetIncomingSMS,
	SONYERICSSON_SetIncomingCB,
	SONYERICSSON_GetSMSFolders,
 	NOTSUPPORTED,			/* 	AddSMSFolder		*/
 	NOTSUPPORTED,			/* 	DeleteSMSFolder		*/
	SONYERICSSON_DialVoice,
	SONYERICSSON_AnswerCall,
	SONYERICSSON_CancelCall,
 	NOTSUPPORTED,			/* 	HoldCall 		*/
 	NOTSUPPORTED,			/* 	UnholdCall 		*/
 	NOTSUPPORTED,			/* 	ConferenceCall 		*/
 	NOTSUPPORTED,			/* 	SplitCall		*/
 	NOTSUPPORTED,			/* 	TransferCall		*/
 	NOTSUPPORTED,			/* 	SwitchCall		*/
 	NOTSUPPORTED,			/* 	GetCallDivert		*/
 	NOTSUPPORTED,			/* 	SetCallDivert		*/
 	NOTSUPPORTED,			/* 	CancelAllDiverts	*/
	NONEFUNCTION,			/* 	SetIncomingCall		*/
	SONYERICSSON_SetIncomingUSSD,
	SONYERICSSON_SendDTMF,
	SONYERICSSON_GetRingtone,
	SONYERICSSON_SetRingtone,
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
	SONYERICSSON_GetBitmap,
	SONYERICSSON_SetBitmap,
	SONYERICSSON_GetToDoStatus,
	SONYERICSSON_GetToDo,
	SONYERICSSON_GetNextToDo,
	SONYERICSSON_SetToDo,
	SONYERICSSON_AddToDo,
	SONYERICSSON_DeleteToDo,
	SONYERICSSON_DeleteAllToDo,
	SONYERICSSON_GetCalendarStatus,
	SONYERICSSON_GetCalendar,
	SONYERICSSON_GetNextCalendar,
	SONYERICSSON_SetCalendar,
	SONYERICSSON_AddCalendar,
	SONYERICSSON_DeleteCalendar,
	SONYERICSSON_DeleteAllCalendar,
	NOTSUPPORTED,			/* 	GetCalendarSettings	*/
	NOTSUPPORTED,			/* 	SetCalendarSettings	*/
	NOTSUPPORTED,			/*	GetNoteStatus		*/
	NOTSUPPORTED,			/*	GetNote			*/
	NOTSUPPORTED,			/*	GetNextNote		*/
	NOTSUPPORTED,			/*	SetNote			*/
	NOTSUPPORTED,			/*	AddNote			*/
	NOTSUPPORTED,			/* 	DeleteNote		*/
	NOTSUPPORTED,			/*	DeleteAllNotes		*/
	NOTSUPPORTED,			/* 	GetProfile		*/
	NOTSUPPORTED,			/* 	SetProfile		*/
	NOTSUPPORTED,			/* 	GetFMStation		*/
	NOTSUPPORTED,			/* 	SetFMStation		*/
	NOTSUPPORTED,			/* 	ClearFMStations		*/
	SONYERICSSON_GetNextFileFolder,
	NOTSUPPORTED,			/*	GetFolderListing	*/
	NOTSUPPORTED,			/*	GetNextRootFolder	*/
	NOTSUPPORTED,			/*	SetFileAttributes	*/
	SONYERICSSON_GetFilePart,
	SONYERICSSON_AddFilePart,
	NOTSUPPORTED, 			/* 	GetFileSystemStatus	*/
	SONYERICSSON_DeleteFile,
	SONYERICSSON_AddFolder,
	NOTSUPPORTED,			/* 	DeleteFolder		*/
	NOTSUPPORTED,			/* 	GetGPRSAccessPoint	*/
	NOTSUPPORTED			/* 	SetGPRSAccessPoint	*/
};

#endif
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

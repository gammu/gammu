#include <string.h>

#include <gammu.h>
#include "gsmstate.h"
#include "debug.h"

#ifdef __FUNCTION__WORKING
#define PRINT_FUNCTION_START smprintf(s, "Entering %s\n", __FUNCTION__);
#define PRINT_FUNCTION_END smprintf(s, "Leaving %s\n", __FUNCTION__);
#else
#define PRINT_FUNCTION_START smprintf(s, "Entering %s:%d\n", __FILE__, __LINE__);
#define PRINT_FUNCTION_END smprintf(s, "Leaving %s:%d\n", __FILE__, __LINE__);
#endif
#define PRINT_MEMORY_INFO() smprintf(s, "Location = %d, Memory type = %s\n", entry->Location, GSM_MemoryTypeToString(entry->MemoryType));
#define PRINT_TODO_INFO() smprintf(s, "Location = %d\n", ToDo->Location);
#define PRINT_CALENDAR_INFO() smprintf(s, "Location = %d\n", Note->Location);
#define PRINT_NOTE_INFO() smprintf(s, "Location = %d\n", Note->Location);
#define PRINT_MSMS_INFO() smprintf(s, "Number = %d, Location = %d, Folder = %d\n", sms->Number, sms->SMS[0].Location, sms->SMS[0].Folder);
#define PRINT_SMS_INFO() smprintf(s, "Location = %d, Folder = %d\n", sms->Location, sms->Folder);
#define PRINT_START() if (start) smprintf(s, "Starting reading!\n");

/**
 * Prints error message (if any) to debug log.
 *
 * \param err Error code to check.
 */
#define PRINT_LOG_ERROR(err) \
{ \
	GSM_LogError(s, __FUNCTION__, err); \
	PRINT_FUNCTION_END \
}

/**
 * Checks whether we are connected to phone, fails with error
 * otherwise.
 */
#define CHECK_PHONE_CONNECTION() \
{ \
	PRINT_FUNCTION_START \
	if (!GSM_IsConnected(s)) { \
		return ERR_NOTCONNECTED; \
	} \
}

/**
 * Tries the command a couple of times to see if it can complete
 * without an ERR_BUSY return.
 */
#define RUN_RESTARTABLE(return_value, function_call) \
{ \
	int restarts; \
	for (restarts = 0; restarts < 10; ++restarts) { \
		unsigned useconds = 10000 << restarts; \
		return_value = (function_call); \
		if (return_value != ERR_BUSY) \
			break; \
		smprintf(s, "Sleeping %d ms before retrying the last command\n", useconds / 1000); \
		usleep(useconds); \
	} \
}

/**
 * Reads manufacturer from phone.
 */
GSM_Error GSM_GetManufacturer(GSM_StateMachine *s, char *value)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	s->Phone.Data.Manufacturer[0] = '\0';
	err = s->Phone.Functions->GetManufacturer(s);
	if (value != NULL) {
		strcpy(value, s->Phone.Data.Manufacturer);
	}

	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Reads model from phone.
 */
GSM_Error GSM_GetModel(GSM_StateMachine *s, char *value)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	s->Phone.Data.Model[0] = '\0';
	err = s->Phone.Functions->GetModel(s);
	if (value != NULL) {
		strcpy(value, s->Phone.Data.Model);
	}

	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Reads firmware information from phone.
 */
GSM_Error GSM_GetFirmware(GSM_StateMachine *s, char *value, char *date, double *num)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	s->Phone.Data.Version[0] = '\0';
	err = s->Phone.Functions->GetFirmware(s);
	if (value != NULL) {
		strcpy(value, s->Phone.Data.Version);
	}
	if (date != NULL) {
		strcpy(date, s->Phone.Data.VerDate);
	}
	if (num != NULL) {
		*num = s->Phone.Data.VerNum;
	}

	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Reads IMEI/serial number from phone.
 */
GSM_Error GSM_GetIMEI(GSM_StateMachine *s, char *value)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	s->Phone.Data.IMEI[0] = '\0';
	err = s->Phone.Functions->GetIMEI(s);
	if (value != NULL) {
		strcpy(value, s->Phone.Data.IMEI);
	}

	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Gets date and time from phone.
 */
GSM_Error GSM_GetOriginalIMEI(GSM_StateMachine *s, char *value)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetOriginalIMEI(s, value);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Gets month when device was manufactured.
 */
GSM_Error GSM_GetManufactureMonth(GSM_StateMachine *s, char *value)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetManufactureMonth(s, value);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Gets product code of device.
 */
GSM_Error GSM_GetProductCode(GSM_StateMachine *s, char *value)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetProductCode(s, value);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Gets hardware information about device.
 */
GSM_Error GSM_GetHardware(GSM_StateMachine *s, char *value)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetHardware(s, value);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Gets PPM (Post Programmable Memory) info from phone
 * (in other words for Nokia get, which language pack is in phone)
 */
GSM_Error GSM_GetPPM(GSM_StateMachine *s, char *value)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetPPM(s, value);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Gets SIM IMSI from phone.
 */
GSM_Error GSM_GetSIMIMSI(GSM_StateMachine *s, char *IMSI)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetSIMIMSI(s, IMSI);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Reads date and time from phone.
 */
GSM_Error GSM_GetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetDateTime(s, date_time);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Sets date and time in phone.
 */
GSM_Error GSM_SetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetDateTime(s, date_time);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Reads alarm set in phone.
 */
GSM_Error GSM_GetAlarm(GSM_StateMachine *s, GSM_Alarm *Alarm)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetAlarm(s, Alarm);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Sets alarm in phone.
 */
GSM_Error GSM_SetAlarm(GSM_StateMachine *s, GSM_Alarm *Alarm)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetAlarm(s, Alarm);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Gets locale from phone.
 */
GSM_Error GSM_GetLocale(GSM_StateMachine *s, GSM_Locale *locale)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetLocale(s, locale);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Sets locale of phone.
 */
GSM_Error GSM_SetLocale(GSM_StateMachine *s, GSM_Locale *locale)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetLocale(s, locale);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Emulates key press or key release.
 */
GSM_Error GSM_PressKey(GSM_StateMachine *s, GSM_KeyCode Key, gboolean Press)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->PressKey(s, Key, Press);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Performs phone reset.
 */
GSM_Error GSM_Reset(GSM_StateMachine *s, gboolean hard)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->Reset(s, hard);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Resets phone settings.
 */
GSM_Error GSM_ResetPhoneSettings(GSM_StateMachine *s, GSM_ResetSettingsType Type)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->ResetPhoneSettings(s, Type);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Enters security code (PIN, PUK,...) .
 */
GSM_Error GSM_EnterSecurityCode(GSM_StateMachine *s, GSM_SecurityCode Code)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->EnterSecurityCode(s, Code);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Queries whether some security code needs to be entered./
 */
GSM_Error GSM_GetSecurityStatus(GSM_StateMachine *s, GSM_SecurityCodeType *Status)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetSecurityStatus(s, Status);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Acquired display status.
 */
GSM_Error GSM_GetDisplayStatus(GSM_StateMachine *s, GSM_DisplayFeatures *features)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetDisplayStatus(s, features);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Enables network auto login.
 */
GSM_Error GSM_SetAutoNetworkLogin(GSM_StateMachine *s)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetAutoNetworkLogin(s);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Gets information about batery charge and phone charging state.
 */
GSM_Error GSM_GetBatteryCharge(GSM_StateMachine *s, GSM_BatteryCharge *bat)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

    memset(bat, 0, sizeof(GSM_BatteryCharge));

	err = s->Phone.Functions->GetBatteryCharge(s, bat);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Reads signal quality (strength and error rate).
 */
GSM_Error GSM_GetSignalQuality(GSM_StateMachine *s, GSM_SignalQuality *sig)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetSignalQuality(s, sig);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Gets network information.
 */
GSM_Error GSM_GetNetworkInfo(GSM_StateMachine *s, GSM_NetworkInfo *netinfo)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetNetworkInfo(s, netinfo);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Reads category from phone.
 */
GSM_Error GSM_GetCategory(GSM_StateMachine *s, GSM_Category *Category)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetCategory(s, Category);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Adds category to phone.
 */
GSM_Error GSM_AddCategory(GSM_StateMachine *s, GSM_Category *Category)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->AddCategory(s, Category);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Reads category status (number of used entries) from phone.
 */
GSM_Error GSM_GetCategoryStatus(GSM_StateMachine *s, GSM_CategoryStatus *Status)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetCategoryStatus(s, Status);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Gets memory (phonebooks or calls) status (eg. number of used and
 * free entries).
 */
GSM_Error GSM_GetMemoryStatus(GSM_StateMachine *s, GSM_MemoryStatus *status)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetMemoryStatus(s, status);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Reads entry from memory (phonebooks or calls). Which entry should
 * be read is defined in entry.
 */
GSM_Error GSM_GetMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_MEMORY_INFO();

	err = s->Phone.Functions->GetMemory(s, entry);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Reads entry from memory (phonebooks or calls). Which entry should
 * be read is defined in entry. This can be easily used for reading all entries.
 */
GSM_Error GSM_GetNextMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry, gboolean start)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_START();
	PRINT_MEMORY_INFO();

	err = s->Phone.Functions->GetNextMemory(s, entry, start);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Sets memory (phonebooks or calls) entry.
 */
GSM_Error GSM_SetMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_MEMORY_INFO();

	RUN_RESTARTABLE(err, s->Phone.Functions->SetMemory(s, entry));
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Deletes memory (phonebooks or calls) entry.
 */
GSM_Error GSM_AddMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_MEMORY_INFO();

	err = s->Phone.Functions->AddMemory(s, entry);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Deletes memory (phonebooks or calls) entry.
 */
GSM_Error GSM_DeleteMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_MEMORY_INFO();

	RUN_RESTARTABLE(err, s->Phone.Functions->DeleteMemory(s, entry));
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Deletes all memory (phonebooks or calls) entries of specified type.
 */
GSM_Error GSM_DeleteAllMemory(GSM_StateMachine *s, GSM_MemoryType MemoryType)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->DeleteAllMemory(s, MemoryType);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Gets speed dial.
 */
GSM_Error GSM_GetSpeedDial(GSM_StateMachine *s, GSM_SpeedDial *Speed)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetSpeedDial(s, Speed);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Sets speed dial.
 */
GSM_Error GSM_SetSpeedDial(GSM_StateMachine *s, GSM_SpeedDial *Speed)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetSpeedDial(s, Speed);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Gets SMS Service Center number and SMS settings.
 */
GSM_Error GSM_GetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetSMSC(s, smsc);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Sets SMS Service Center number and SMS settings.
 */
GSM_Error GSM_SetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetSMSC(s, smsc);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Gets information about SMS memory (read/unread/size of memory for
 * both SIM and phone).
 */
GSM_Error GSM_GetSMSStatus(GSM_StateMachine *s, GSM_SMSMemoryStatus *status)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetSMSStatus(s, status);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Reads SMS message.
 */
GSM_Error GSM_GetSMS(GSM_StateMachine *s, GSM_MultiSMSMessage *sms)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_MSMS_INFO();

	err = s->Phone.Functions->GetSMS(s, sms);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Reads next (or first if start set) SMS message. This might be
 * faster for some phones than using @ref GSM_GetSMS for each message.
 */
GSM_Error GSM_GetNextSMS(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, gboolean start)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_START();
	PRINT_MSMS_INFO();

	err = s->Phone.Functions->GetNextSMS(s, sms, start);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Sets SMS.
 */
GSM_Error GSM_SetSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_SMS_INFO();

	err = s->Phone.Functions->SetSMS(s, sms);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Adds SMS to specified folder.
 */
GSM_Error GSM_AddSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_SMS_INFO();

	err = s->Phone.Functions->AddSMS(s, sms);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Deletes SMS.
 */
GSM_Error GSM_DeleteSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_SMS_INFO();

	err = s->Phone.Functions->DeleteSMS(s, sms);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Sends SMS.
 */
GSM_Error GSM_SendSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SendSMS(s, sms);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Sends SMS already saved in phone.
 */
GSM_Error GSM_SendSavedSMS(GSM_StateMachine *s, int Folder, int Location)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	smprintf(s, "Location = %d, Folder = %d\n", Location, Folder);

	err = s->Phone.Functions->SendSavedSMS(s, Folder, Location);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Configures fast SMS sending.
 */
GSM_Error GSM_SetFastSMSSending(GSM_StateMachine *s, gboolean enable)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetFastSMSSending(s, enable);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Enable/disable notification on incoming SMS.
 */
GSM_Error GSM_SetIncomingSMS(GSM_StateMachine *s, gboolean enable)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetIncomingSMS(s, enable);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Gets network information from phone.
 */
GSM_Error GSM_SetIncomingCB(GSM_StateMachine *s, gboolean enable)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetIncomingCB(s, enable);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Returns SMS folders information.
 */
GSM_Error GSM_GetSMSFolders(GSM_StateMachine *s, GSM_SMSFolders *folders)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetSMSFolders(s, folders);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Creates SMS folder.
 */
GSM_Error GSM_AddSMSFolder(GSM_StateMachine *s, unsigned char *name)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->AddSMSFolder(s, name);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Deletes SMS folder.
 */
GSM_Error GSM_DeleteSMSFolder(GSM_StateMachine *s, int ID)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->DeleteSMSFolder(s, ID);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Dials number and starts voice call.
 */
GSM_Error GSM_DialVoice(GSM_StateMachine *s, char *Number, GSM_CallShowNumber ShowNumber)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->DialVoice(s, Number, ShowNumber);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Dials service number (usually for USSD).
 */
GSM_Error GSM_DialService(GSM_StateMachine *s, char *Number)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->DialService(s, Number);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Accept current incoming call.
 */
GSM_Error GSM_AnswerCall(GSM_StateMachine *s, int ID, gboolean all)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->AnswerCall(s, ID, all);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Deny current incoming call.
 */
GSM_Error GSM_CancelCall(GSM_StateMachine *s, int ID, gboolean all)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->CancelCall(s, ID, all);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Holds call.
 */
GSM_Error GSM_HoldCall(GSM_StateMachine *s, int ID)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->HoldCall(s, ID);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Unholds call.
 */
GSM_Error GSM_UnholdCall(GSM_StateMachine *s, int ID)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->UnholdCall(s, ID);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Initiates a conference call.
 */
GSM_Error GSM_ConferenceCall(GSM_StateMachine *s, int ID)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->ConferenceCall(s, ID);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Splits call.
 */
GSM_Error GSM_SplitCall(GSM_StateMachine *s, int ID)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SplitCall(s, ID);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Transfers call.
 */
GSM_Error GSM_TransferCall(GSM_StateMachine *s, int ID, gboolean next)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->TransferCall(s, ID, next);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Switches call.
 */
GSM_Error GSM_SwitchCall(GSM_StateMachine *s, int ID, gboolean next)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SwitchCall(s, ID, next);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Gets call diverts.
 */
GSM_Error GSM_GetCallDivert(GSM_StateMachine *s, GSM_MultiCallDivert *divert)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetCallDivert(s, divert);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Sets call diverts.
 */
GSM_Error GSM_SetCallDivert(GSM_StateMachine *s, GSM_MultiCallDivert *divert)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetCallDivert(s, divert);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Cancels all diverts.
 */
GSM_Error GSM_CancelAllDiverts(GSM_StateMachine *s)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->CancelAllDiverts(s);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Activates/deactivates noticing about incoming calls.
 */
GSM_Error GSM_SetIncomingCall(GSM_StateMachine *s, gboolean enable)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetIncomingCall(s, enable);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Activates/deactivates noticing about incoming USSDs (UnStructured Supplementary Services).
 */
GSM_Error GSM_SetIncomingUSSD(GSM_StateMachine *s, gboolean enable)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetIncomingUSSD(s, enable);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Sends DTMF (Dual Tone Multi Frequency) tone.
 */
GSM_Error GSM_SendDTMF(GSM_StateMachine *s, char *sequence)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SendDTMF(s, sequence);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Gets ringtone from phone.
 */
GSM_Error GSM_GetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, gboolean PhoneRingtone)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetRingtone(s, Ringtone, PhoneRingtone);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Sets ringtone in phone.
 */
GSM_Error GSM_SetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, int *maxlength)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetRingtone(s, Ringtone, maxlength);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Acquires ringtone informaiton.
 */
GSM_Error GSM_GetRingtonesInfo(GSM_StateMachine *s, GSM_AllRingtonesInfo *Info)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetRingtonesInfo(s, Info);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Deletes user defined ringtones from phone.
 */
GSM_Error GSM_DeleteUserRingtones(GSM_StateMachine *s)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->DeleteUserRingtones(s);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Plays tone.
 */
GSM_Error GSM_PlayTone(GSM_StateMachine *s, int Herz, unsigned char Volume, gboolean start)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->PlayTone(s, Herz, Volume, start);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Reads WAP bookmark.
 */
GSM_Error GSM_GetWAPBookmark(GSM_StateMachine *s, GSM_WAPBookmark *bookmark)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetWAPBookmark(s, bookmark);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Sets WAP bookmark.
 */
GSM_Error GSM_SetWAPBookmark(GSM_StateMachine *s, GSM_WAPBookmark *bookmark)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetWAPBookmark(s, bookmark);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Deletes WAP bookmark.
 */
GSM_Error GSM_DeleteWAPBookmark(GSM_StateMachine *s, GSM_WAPBookmark *bookmark)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->DeleteWAPBookmark(s, bookmark);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Acquires WAP settings.
 */
GSM_Error GSM_GetWAPSettings(GSM_StateMachine *s, GSM_MultiWAPSettings *settings)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetWAPSettings(s, settings);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Changes WAP settings.
 */
GSM_Error GSM_SetWAPSettings(GSM_StateMachine *s, GSM_MultiWAPSettings *settings)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetWAPSettings(s, settings);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Acquires SyncML settings.
 */
GSM_Error GSM_GetSyncMLSettings(GSM_StateMachine *s, GSM_SyncMLSettings *settings)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetSyncMLSettings(s, settings);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Changes SyncML settings.
 */
GSM_Error GSM_SetSyncMLSettings(GSM_StateMachine *s, GSM_SyncMLSettings *settings)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetSyncMLSettings(s, settings);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Acquires chat/presence settings.
 */
GSM_Error GSM_GetChatSettings(GSM_StateMachine *s, GSM_ChatSettings *settings)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetChatSettings(s, settings);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Changes chat/presence settings.
 */
GSM_Error GSM_SetChatSettings(GSM_StateMachine *s, GSM_ChatSettings *settings)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetChatSettings(s, settings);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Acquires MMS settings.
 */
GSM_Error GSM_GetMMSSettings(GSM_StateMachine *s, GSM_MultiWAPSettings *settings)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetMMSSettings(s, settings);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Changes MMS settings.
 */
GSM_Error GSM_SetMMSSettings(GSM_StateMachine *s, GSM_MultiWAPSettings *settings)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetMMSSettings(s, settings);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Lists MMS folders.
 */
GSM_Error GSM_GetMMSFolders(GSM_StateMachine *s, GSM_MMSFolders *folders)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetMMSFolders(s, folders);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Retrieves next part of MMS file information.
 */
GSM_Error GSM_GetNextMMSFileInfo(GSM_StateMachine *s, unsigned char *FileID, int *MMSFolder, gboolean start)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_START();

	err = s->Phone.Functions->GetNextMMSFileInfo(s, FileID, MMSFolder, start);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Gets bitmap.
 */
GSM_Error GSM_GetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetBitmap(s, Bitmap);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Sets bitmap.
 */
GSM_Error GSM_SetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetBitmap(s, Bitmap);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Gets status of ToDos (count of used entries).
 */
GSM_Error GSM_GetToDoStatus(GSM_StateMachine *s, GSM_ToDoStatus *status)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetToDoStatus(s, status);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Reads ToDo from phone.
 */
GSM_Error GSM_GetToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_TODO_INFO();

	err = s->Phone.Functions->GetToDo(s, ToDo);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Reads ToDo from phone.
 */
GSM_Error GSM_GetNextToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo, gboolean start)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_START();
	PRINT_TODO_INFO();

	err = s->Phone.Functions->GetNextToDo(s, ToDo, start);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Sets ToDo in phone.
 */
GSM_Error GSM_SetToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_TODO_INFO();

	err = s->Phone.Functions->SetToDo(s, ToDo);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Adds ToDo in phone.
 */
GSM_Error GSM_AddToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_TODO_INFO();

	err = s->Phone.Functions->AddToDo(s, ToDo);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Deletes ToDo entry in phone.
 */
GSM_Error GSM_DeleteToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_TODO_INFO();

	err = s->Phone.Functions->DeleteToDo(s, ToDo);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Deletes all todo entries in phone.
 */
GSM_Error GSM_DeleteAllToDo(GSM_StateMachine *s)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->DeleteAllToDo(s);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Retrieves calendar status (number of used entries).
 */
GSM_Error GSM_GetCalendarStatus(GSM_StateMachine *s, GSM_CalendarStatus *Status)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetCalendarStatus(s, Status);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Retrieves calendar entry.
 */
GSM_Error GSM_GetCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_CALENDAR_INFO();

	err = s->Phone.Functions->GetCalendar(s, Note);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Retrieves calendar entry. This is useful for continuous reading of all
 * calendar entries.
 */
GSM_Error GSM_GetNextCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note, gboolean start)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_START();
	PRINT_CALENDAR_INFO();

	err = s->Phone.Functions->GetNextCalendar(s, Note, start);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Sets calendar entry
 */
GSM_Error GSM_SetCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_CALENDAR_INFO();

	err = s->Phone.Functions->SetCalendar(s, Note);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Adds calendar entry.
 */
GSM_Error GSM_AddCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_CALENDAR_INFO();

	err = s->Phone.Functions->AddCalendar(s, Note);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Deletes calendar entry.
 */
GSM_Error GSM_DeleteCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_CALENDAR_INFO();

	err = s->Phone.Functions->DeleteCalendar(s, Note);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Deletes all calendar entries.
 */
GSM_Error GSM_DeleteAllCalendar(GSM_StateMachine *s)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->DeleteAllCalendar(s);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Reads calendar settings.
 */
GSM_Error GSM_GetCalendarSettings(GSM_StateMachine *s, GSM_CalendarSettings *settings)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetCalendarSettings(s, settings);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Sets calendar settings.
 */
GSM_Error GSM_SetCalendarSettings(GSM_StateMachine *s, GSM_CalendarSettings *settings)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetCalendarSettings(s, settings);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Retrieves notes status (number of used entries).
 */
GSM_Error GSM_GetNotesStatus(GSM_StateMachine *s, GSM_ToDoStatus *status)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetNotesStatus(s, status);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Retrieves notes entry.
 */
GSM_Error GSM_GetNote(GSM_StateMachine *s, GSM_NoteEntry *Note)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_NOTE_INFO();

	err = s->Phone.Functions->GetNote(s, Note);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Retrieves note entry. This is useful for continuous reading of all
 * notes entries.
 */
GSM_Error GSM_GetNextNote(GSM_StateMachine *s, GSM_NoteEntry *Note, gboolean start)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_START();
	PRINT_NOTE_INFO();

	err = s->Phone.Functions->GetNextNote(s, Note, start);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Sets note entry
 */
GSM_Error GSM_SetNote(GSM_StateMachine *s, GSM_NoteEntry *Note)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_NOTE_INFO();

	err = s->Phone.Functions->SetNote(s, Note);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Adds note entry.
 */
GSM_Error GSM_AddNote(GSM_StateMachine *s, GSM_NoteEntry *Note)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_NOTE_INFO();

	err = s->Phone.Functions->AddNote(s, Note);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Deletes note entry.
 */
GSM_Error GSM_DeleteNote(GSM_StateMachine *s, GSM_NoteEntry *Note)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_NOTE_INFO();

	err = s->Phone.Functions->DeleteNote(s, Note);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Deletes all notes entries.
 */
GSM_Error GSM_DeleteAllNotes(GSM_StateMachine *s)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->DeleteAllNotes(s);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Reads profile.
 */
GSM_Error GSM_GetProfile(GSM_StateMachine *s, GSM_Profile *Profile)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetProfile(s, Profile);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Updates profile.
 */
GSM_Error GSM_SetProfile(GSM_StateMachine *s, GSM_Profile *Profile)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetProfile(s, Profile);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Reads FM station.
 */
GSM_Error GSM_GetFMStation(GSM_StateMachine *s, GSM_FMStation *FMStation)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetFMStation(s, FMStation);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Sets FM station.
 */
GSM_Error GSM_SetFMStation(GSM_StateMachine *s, GSM_FMStation *FMStation)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetFMStation(s, FMStation);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Clears defined FM stations.
 */
GSM_Error GSM_ClearFMStations(GSM_StateMachine *s)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->ClearFMStations(s);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Gets next filename from filesystem.
 */
GSM_Error GSM_GetNextFileFolder(GSM_StateMachine *s, GSM_File *File, gboolean start)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_START();

	err = s->Phone.Functions->GetNextFileFolder(s, File, start);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Gets file part from filesystem.
 */
GSM_Error GSM_GetFolderListing(GSM_StateMachine *s, GSM_File *File, gboolean start)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();
	PRINT_START();

	err = s->Phone.Functions->GetFolderListing(s, File, start);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Gets next root folder.
 */
GSM_Error GSM_GetNextRootFolder(GSM_StateMachine *s, GSM_File *File)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetNextRootFolder(s, File);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Sets file system attributes.
 */
GSM_Error GSM_SetFileAttributes(GSM_StateMachine *s, GSM_File *File)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetFileAttributes(s, File);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Retrieves file part.
 */
GSM_Error GSM_GetFilePart(GSM_StateMachine *s, GSM_File *File, int *Handle, int *Size)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetFilePart(s, File, Handle, Size);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Adds file part to filesystem.
 */
GSM_Error GSM_AddFilePart(GSM_StateMachine *s, GSM_File *File, int *Pos, int *Handle)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->AddFilePart(s, File, Pos, Handle);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Sends file to phone, it's up to phone to decide what to do with it.
 */
GSM_Error GSM_SendFilePart(GSM_StateMachine *s, GSM_File *File, int *Pos, int *Handle)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SendFilePart(s, File, Pos, Handle);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Acquires filesystem status.
 */
GSM_Error GSM_GetFileSystemStatus(GSM_StateMachine *s, GSM_FileSystemStatus *Status)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetFileSystemStatus(s, Status);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Deletes file from filessytem.
 */
GSM_Error GSM_DeleteFile(GSM_StateMachine *s, unsigned char *ID)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->DeleteFile(s, ID);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Adds folder to filesystem.
 */
GSM_Error GSM_AddFolder(GSM_StateMachine *s, GSM_File *File)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->AddFolder(s, File);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Deletes folder from filesystem.
 */
GSM_Error GSM_DeleteFolder(GSM_StateMachine *s, unsigned char *ID)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->DeleteFolder(s, ID);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Gets GPRS access point.
 */
GSM_Error GSM_GetGPRSAccessPoint(GSM_StateMachine *s, GSM_GPRSAccessPoint *point)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->GetGPRSAccessPoint(s, point);
	PRINT_LOG_ERROR(err);
	return err;
}
/**
 * Sets GPRS access point.
 */
GSM_Error GSM_SetGPRSAccessPoint(GSM_StateMachine *s, GSM_GPRSAccessPoint *point)
{
	GSM_Error err;

	CHECK_PHONE_CONNECTION();

	err = s->Phone.Functions->SetGPRSAccessPoint(s, point);
	PRINT_LOG_ERROR(err);
	return err;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

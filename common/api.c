#include <gammu.h>
#include "gsmstate.h"

/**
 * Reads manufacturer from phone.
 */
GSM_Error GAMMU_GetManufacturer(GSM_StateMachine *s)
{
	return s->Phone.Functions->GetManufacturer(s);
}
/**
 * Reads model from phone.
 */
GSM_Error GAMMU_GetModel(GSM_StateMachine *s)
{
	return s->Phone.Functions->GetModel(s);
}
/**
 * Reads firmware information from phone.
 */
GSM_Error GAMMU_GetFirmware(GSM_StateMachine *s)
{
	return s->Phone.Functions->GetFirmware(s);
}
/**
 * Reads IMEI/serial number from phone.
 */
GSM_Error GAMMU_GetIMEI(GSM_StateMachine *s, char *value)
{
	if (value != NULL) {
		value = s->Phone.Data.IMEI;
	}
	s->Phone.Data.IMEI[0] = 0;
	return s->Phone.Functions->GetIMEI(s);
}
/**
 * Gets date and time from phone.
 */
GSM_Error GAMMU_GetOriginalIMEI(GSM_StateMachine *s, char *value)
{
	return s->Phone.Functions->GetOriginalIMEI(s, value);
}
/**
 * Gets month when device was manufactured.
 */
GSM_Error GAMMU_GetManufactureMonth(GSM_StateMachine *s, char *value)
{
	return s->Phone.Functions->GetManufactureMonth(s, value);
}
/**
 * Gets product code of device.
 */
GSM_Error GAMMU_GetProductCode(GSM_StateMachine *s, char *value)
{
	return s->Phone.Functions->GetProductCode(s, value);
}
/**
 * Gets hardware information about device.
 */
GSM_Error GAMMU_GetHardware(GSM_StateMachine *s, char *value)
{
	return s->Phone.Functions->GetHardware(s, value);
}
/**
 * Gets PPM (Post Programmable Memory) info from phone
 * (in other words for Nokia get, which language pack is in phone)
 */
GSM_Error GAMMU_GetPPM(GSM_StateMachine *s, char *value)
{
	return s->Phone.Functions->GetPPM(s, value);
}
/**
 * Gets SIM IMSI from phone.
 */
GSM_Error GAMMU_GetSIMIMSI(GSM_StateMachine *s, char *IMSI)
{
	return s->Phone.Functions->GetSIMIMSI(s, IMSI);
}
/**
 * Reads date and time from phone.
 */
GSM_Error GAMMU_GetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	return s->Phone.Functions->GetDateTime(s, date_time);
}
/**
 * Sets date and time in phone.
 */
GSM_Error GAMMU_SetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	return s->Phone.Functions->SetDateTime(s, date_time);
}
/**
 * Reads alarm set in phone.
 */
GSM_Error GAMMU_GetAlarm(GSM_StateMachine *s, GSM_Alarm	*alarm)
{
	return s->Phone.Functions->GetAlarm(s, alarm);
}
/**
 * Sets alarm in phone.
 */
GSM_Error GAMMU_SetAlarm(GSM_StateMachine *s, GSM_Alarm *alarm)
{
	return s->Phone.Functions->SetAlarm(s, alarm);
}
/**
 * Gets locale from phone.
 */
GSM_Error GAMMU_GetLocale(GSM_StateMachine *s, GSM_Locale *locale)
{
	return s->Phone.Functions->GetLocale(s, locale);
}
/**
 * Sets locale of phone.
 */
GSM_Error GAMMU_SetLocale(GSM_StateMachine *s, GSM_Locale *locale)
{
	return s->Phone.Functions->SetLocale(s, locale);
}
/**
 * Emulates key press or key release.
 */
GSM_Error GAMMU_PressKey(GSM_StateMachine *s, GSM_KeyCode Key, bool Press)
{
	return s->Phone.Functions->PressKey(s, Key, Press);
}
/**
 * Performs phone reset.
 */
GSM_Error GAMMU_Reset(GSM_StateMachine *s, bool hard)
{
	return s->Phone.Functions->Reset(s, hard);
}
/**
 * Resets phone settings.
 */
GSM_Error GAMMU_ResetPhoneSettings(GSM_StateMachine *s, GSM_ResetSettingsType Type)
{
	return s->Phone.Functions->ResetPhoneSettings(s, Type);
}
/**
 * Enters security code (PIN, PUK,...) .
 */
GSM_Error GAMMU_EnterSecurityCode(GSM_StateMachine *s, GSM_SecurityCode Code)
{
	return s->Phone.Functions->EnterSecurityCode(s, Code);
}
/**
 * Queries whether some security code needs to be entered./
 */
GSM_Error GAMMU_GetSecurityStatus(GSM_StateMachine *s, GSM_SecurityCodeType *Status)
{
	return s->Phone.Functions->GetSecurityStatus(s, Status);
}
/**
 * Acquired display status.
 */
GSM_Error GAMMU_GetDisplayStatus(GSM_StateMachine *s, GSM_DisplayFeatures *features)
{
	return s->Phone.Functions->GetDisplayStatus(s, features);
}
/**
 * Enables network auto login.
 */
GSM_Error GAMMU_SetAutoNetworkLogin(GSM_StateMachine *s)
{
	return s->Phone.Functions->SetAutoNetworkLogin(s);
}
/**
 * Gets information about batery charge and phone charging state.
 */
GSM_Error GAMMU_GetBatteryCharge(GSM_StateMachine *s, GSM_BatteryCharge *bat)
{
	return s->Phone.Functions->GetBatteryCharge(s, bat);
}
/**
 * Reads signal quality (strength and error rate).
 */
GSM_Error GAMMU_GetSignalQuality(GSM_StateMachine *s, GSM_SignalQuality *sig)
{
	return s->Phone.Functions->GetSignalQuality(s, sig);
}
/**
 * Gets network information.
 */
GSM_Error GAMMU_GetNetworkInfo(GSM_StateMachine *s, GSM_NetworkInfo *netinfo)
{
	return s->Phone.Functions->GetNetworkInfo(s, netinfo);
}
/**
 * Reads category from phone.
 */
GSM_Error GAMMU_GetCategory(GSM_StateMachine *s, GSM_Category *Category)
{
	return s->Phone.Functions->GetCategory(s, Category);
}
/**
 * Adds category to phone.
 */
GSM_Error GAMMU_AddCategory(GSM_StateMachine *s, GSM_Category *Category)
{
	return s->Phone.Functions->AddCategory(s, Category);
}
/**
 * Reads category status (number of used entries) from phone.
 */
GSM_Error GAMMU_GetCategoryStatus(GSM_StateMachine *s, GSM_CategoryStatus *Status)
{
	return s->Phone.Functions->GetCategoryStatus(s, Status);
}
/**
 * Gets memory (phonebooks or calls) status (eg. number of used and
 * free entries).
 */
GSM_Error GAMMU_GetMemoryStatus(GSM_StateMachine *s, GSM_MemoryStatus *status)
{
	return s->Phone.Functions->GetMemoryStatus(s, status);
}
/**
 * Reads entry from memory (phonebooks or calls). Which entry should
 * be read is defined in entry.
 */
GSM_Error GAMMU_GetMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	return s->Phone.Functions->GetMemory(s, entry);
}
/**
 * Reads entry from memory (phonebooks or calls). Which entry should
 * be read is defined in entry. This can be easily used for reading all entries.
 */
GSM_Error GAMMU_GetNextMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry, bool start)
{
	return s->Phone.Functions->GetNextMemory(s, entry, start);
}
/**
 * Sets memory (phonebooks or calls) entry.
 */
GSM_Error GAMMU_SetMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	return s->Phone.Functions->SetMemory(s, entry);
}
/**
 * Deletes memory (phonebooks or calls) entry.
 */
GSM_Error GAMMU_AddMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	return s->Phone.Functions->AddMemory(s, entry);
}
/**
 * Deletes memory (phonebooks or calls) entry.
 */
GSM_Error GAMMU_DeleteMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	return s->Phone.Functions->DeleteMemory(s, entry);
}
/**
 * Deletes all memory (phonebooks or calls) entries of specified type.
 */
GSM_Error GAMMU_DeleteAllMemory(GSM_StateMachine *s, GSM_MemoryType MemoryType)
{
	return s->Phone.Functions->DeleteAllMemory(s, MemoryType);
}
/**
 * Gets speed dial.
 */
GSM_Error GAMMU_GetSpeedDial(GSM_StateMachine *s, GSM_SpeedDial *Speed)
{
	return s->Phone.Functions->GetSpeedDial(s, Speed);
}
/**
 * Sets speed dial.
 */
GSM_Error GAMMU_SetSpeedDial(GSM_StateMachine *s, GSM_SpeedDial *Speed)
{
	return s->Phone.Functions->SetSpeedDial(s, Speed);
}
/**
 * Gets SMS Service Center number and SMS settings.
 */
GSM_Error GAMMU_GetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	return s->Phone.Functions->GetSMSC(s, smsc);
}
/**
 * Sets SMS Service Center number and SMS settings.
 */
GSM_Error GAMMU_SetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	return s->Phone.Functions->SetSMSC(s, smsc);
}
/**
 * Gets information about SMS memory (read/unread/size of memory for
 * both SIM and phone).
 */
GSM_Error GAMMU_GetSMSStatus(GSM_StateMachine *s, GSM_SMSMemoryStatus *status)
{
	return s->Phone.Functions->GetSMSStatus(s, status);
}
/**
 * Reads SMS message.
 */
GSM_Error GAMMU_GetSMS(GSM_StateMachine *s, GSM_MultiSMSMessage *sms)
{
	return s->Phone.Functions->GetSMS(s, sms);
}
/**
 * Reads next (or first if start set) SMS message. This might be
 * faster for some phones than using @ref GetSMS for each message.
 */
GSM_Error GAMMU_GetNextSMS(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, bool start)
{
	return s->Phone.Functions->GetNextSMS(s, sms, start);
}
/**
 * Sets SMS.
 */
GSM_Error GAMMU_SetSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	return s->Phone.Functions->SetSMS(s, sms);
}
/**
 * Adds SMS to specified folder.
 */
GSM_Error GAMMU_AddSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	return s->Phone.Functions->AddSMS(s, sms);
}
/**
 * Deletes SMS.
 */
GSM_Error GAMMU_DeleteSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	return s->Phone.Functions->DeleteSMS(s, sms);
}
/**
 * Sends SMS.
 */
GSM_Error GAMMU_SendSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	return s->Phone.Functions->SendSMS(s, sms);
}
/**
 * Sends SMS already saved in phone.
 */
GSM_Error GAMMU_SendSavedSMS(GSM_StateMachine *s, int Folder, int Location)
{
	return s->Phone.Functions->SendSavedSMS(s, Folder, Location);
}
/**
 * Configures fast SMS sending.
 */
GSM_Error GAMMU_SetFastSMSSending(GSM_StateMachine *s, bool enable)
{
	return s->Phone.Functions->SetFastSMSSending(s, enable);
}
/**
 * Enable/disable notification on incoming SMS.
 */
GSM_Error GAMMU_SetIncomingSMS(GSM_StateMachine *s, bool enable)
{
	return s->Phone.Functions->SetIncomingSMS(s, enable);
}
/**
 * Gets network information from phone.
 */
GSM_Error GAMMU_SetIncomingCB(GSM_StateMachine *s, bool enable)
{
	return s->Phone.Functions->SetIncomingCB(s, enable);
}
/**
 * Returns SMS folders information.
 */
GSM_Error GAMMU_GetSMSFolders(GSM_StateMachine *s, GSM_SMSFolders *folders)
{
	return s->Phone.Functions->GetSMSFolders(s, folders);
}
/**
 * Creates SMS folder.
 */
GSM_Error GAMMU_AddSMSFolder(GSM_StateMachine *s, unsigned char *name)
{
	return s->Phone.Functions->AddSMSFolder(s, name);
}
/**
 * Deletes SMS folder.
 */
GSM_Error GAMMU_DeleteSMSFolder(GSM_StateMachine *s, int ID)
{
	return s->Phone.Functions->DeleteSMSFolder(s, ID);
}
/**
 * Dials number and starts voice call.
 */
GSM_Error GAMMU_DialVoice(GSM_StateMachine *s, char *Number, GSM_CallShowNumber ShowNumber)
{
	return s->Phone.Functions->DialVoice(s, Number, ShowNumber);
}
/**
 * Dials service number (usually for USSD).
 */
GSM_Error GAMMU_DialService(GSM_StateMachine *s, char *Number)
{
	return s->Phone.Functions->DialService(s, Number);
}
/**
 * Accept current incoming call.
 */
GSM_Error GAMMU_AnswerCall(GSM_StateMachine *s, int ID, bool all)
{
	return s->Phone.Functions->AnswerCall(s, ID, all);
}
/**
 * Deny current incoming call.
 */
GSM_Error GAMMU_CancelCall(GSM_StateMachine *s, int ID, bool all)
{
	return s->Phone.Functions->CancelCall(s, ID, all);
}
/**
 * Holds call.
 */
GSM_Error GAMMU_HoldCall(GSM_StateMachine *s, int ID)
{
	return s->Phone.Functions->HoldCall(s, ID);
}
/**
 * Unholds call.
 */
GSM_Error GAMMU_UnholdCall(GSM_StateMachine *s, int ID)
{
	return s->Phone.Functions->UnholdCall(s, ID);
}
/**
 * Initiates conference call.
 */
GSM_Error GAMMU_ConferenceCall(GSM_StateMachine *s, int ID)
{
	return s->Phone.Functions->ConferenceCall(s, ID);
}
/**
 * Splits call.
 */
GSM_Error GAMMU_SplitCall(GSM_StateMachine *s, int ID)
{
	return s->Phone.Functions->SplitCall(s, ID);
}
/**
 * Transfers call.
 */
GSM_Error GAMMU_TransferCall(GSM_StateMachine *s, int ID, bool next)
{
	return s->Phone.Functions->TransferCall(s, ID, next);
}
/**
 * Switches call.
 */
GSM_Error GAMMU_SwitchCall(GSM_StateMachine *s, int ID, bool next)
{
	return s->Phone.Functions->SwitchCall(s, ID, next);
}
/**
 * Gets call diverts.
 */
GSM_Error GAMMU_GetCallDivert(GSM_StateMachine *s, GSM_MultiCallDivert *divert)
{
	return s->Phone.Functions->GetCallDivert(s, divert);
}
/**
 * Sets call diverts.
 */
GSM_Error GAMMU_SetCallDivert(GSM_StateMachine *s, GSM_MultiCallDivert *divert)
{
	return s->Phone.Functions->SetCallDivert(s, divert);
}
/**
 * Cancels all diverts.
 */
GSM_Error GAMMU_CancelAllDiverts(GSM_StateMachine *s)
{
	return s->Phone.Functions->CancelAllDiverts(s);
}
/**
 * Activates/deactivates noticing about incoming calls.
 */
GSM_Error GAMMU_SetIncomingCall(GSM_StateMachine *s, bool enable)
{
	return s->Phone.Functions->SetIncomingCall(s, enable);
}
/**
 * Activates/deactivates noticing about incoming USSDs (UnStructured Supplementary Services).
 */
GSM_Error GAMMU_SetIncomingUSSD(GSM_StateMachine *s, bool enable)
{
	return s->Phone.Functions->SetIncomingUSSD(s, enable);
}
/**
 * Sends DTMF (Dual Tone Multi Frequency) tone.
 */
GSM_Error GAMMU_SendDTMF(GSM_StateMachine *s, char *sequence)
{
	return s->Phone.Functions->SendDTMF(s, sequence);
}
/**
 * Gets ringtone from phone.
 */
GSM_Error GAMMU_GetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, bool PhoneRingtone)
{
	return s->Phone.Functions->GetRingtone(s, Ringtone, PhoneRingtone);
}
/**
 * Sets ringtone in phone.
 */
GSM_Error GAMMU_SetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, int *maxlength)
{
	return s->Phone.Functions->SetRingtone(s, Ringtone, maxlength);
}
/**
 * Acquires ringtone informaiton.
 */
GSM_Error GAMMU_GetRingtonesInfo(GSM_StateMachine *s, GSM_AllRingtonesInfo *Info)
{
	return s->Phone.Functions->GetRingtonesInfo(s, Info);
}
/**
 * Deletes user defined ringtones from phone.
 */
GSM_Error GAMMU_DeleteUserRingtones(GSM_StateMachine *s)
{
	return s->Phone.Functions->DeleteUserRingtones(s);
}
/**
 * Plays tone.
 */
GSM_Error GAMMU_PlayTone(GSM_StateMachine *s, int Herz, unsigned char Volume, bool start)
{
	return s->Phone.Functions->PlayTone(s, Herz, Volume, start);
}
/**
 * Reads WAP bookmark.
 */
GSM_Error GAMMU_GetWAPBookmark(GSM_StateMachine *s, GSM_WAPBookmark *bookmark)
{
	return s->Phone.Functions->GetWAPBookmark(s, bookmark);
}
/**
 * Sets WAP bookmark.
 */
GSM_Error GAMMU_SetWAPBookmark(GSM_StateMachine *s, GSM_WAPBookmark *bookmark)
{
	return s->Phone.Functions->SetWAPBookmark(s, bookmark);
}
/**
 * Deletes WAP bookmark.
 */
GSM_Error GAMMU_DeleteWAPBookmark(GSM_StateMachine *s, GSM_WAPBookmark *bookmark)
{
	return s->Phone.Functions->DeleteWAPBookmark(s, bookmark);
}
/**
 * Acquires WAP settings.
 */
GSM_Error GAMMU_GetWAPSettings(GSM_StateMachine *s, GSM_MultiWAPSettings *settings)
{
	return s->Phone.Functions->GetWAPSettings(s, settings);
}
/**
 * Changes WAP settings.
 */
GSM_Error GAMMU_SetWAPSettings(GSM_StateMachine *s, GSM_MultiWAPSettings *settings)
{
	return s->Phone.Functions->SetWAPSettings(s, settings);
}
/**
 * Acquires SyncML settings.
 */
GSM_Error GAMMU_GetSyncMLSettings(GSM_StateMachine *s, GSM_SyncMLSettings *settings)
{
	return s->Phone.Functions->GetSyncMLSettings(s, settings);
}
/**
 * Changes SyncML settings.
 */
GSM_Error GAMMU_SetSyncMLSettings(GSM_StateMachine *s, GSM_SyncMLSettings *settings)
{
	return s->Phone.Functions->SetSyncMLSettings(s, settings);
}
/**
 * Acquires chat/presence settings.
 */
GSM_Error GAMMU_GetChatSettings(GSM_StateMachine *s, GSM_ChatSettings *settings)
{
	return s->Phone.Functions->GetChatSettings(s, settings);
}
/**
 * Changes chat/presence settings.
 */
GSM_Error GAMMU_SetChatSettings(GSM_StateMachine *s, GSM_ChatSettings *settings)
{
	return s->Phone.Functions->SetChatSettings(s, settings);
}
/**
 * Acquires MMS settings.
 */
GSM_Error GAMMU_GetMMSSettings(GSM_StateMachine *s, GSM_MultiWAPSettings *settings)
{
	return s->Phone.Functions->GetMMSSettings(s, settings);
}
/**
 * Changes MMS settings.
 */
GSM_Error GAMMU_SetMMSSettings(GSM_StateMachine *s, GSM_MultiWAPSettings *settings)
{
	return s->Phone.Functions->SetMMSSettings(s, settings);
}
/**
 * Lists MMS folders.
 */
GSM_Error GAMMU_GetMMSFolders(GSM_StateMachine *s, GSM_MMSFolders *folders)
{
	return s->Phone.Functions->GetMMSFolders(s, folders);
}
/**
 * Retrieves next part of MMS file information.
 */
GSM_Error GAMMU_GetNextMMSFileInfo(GSM_StateMachine *s, unsigned char *FileID, int *MMSFolder, bool start)
{
	return s->Phone.Functions->GetNextMMSFileInfo(s, FileID, MMSFolder, start);
}
/**
 * Gets bitmap.
 */
GSM_Error GAMMU_GetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	return s->Phone.Functions->GetBitmap(s, Bitmap);
}
/**
 * Sets bitmap.
 */
GSM_Error GAMMU_SetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	return s->Phone.Functions->SetBitmap(s, Bitmap);
}
/**
 * Gets status of ToDos (count of used entries).
 */
GSM_Error GAMMU_GetToDoStatus(GSM_StateMachine *s, GSM_ToDoStatus *status)
{
	return s->Phone.Functions->GetToDoStatus(s, status);
}
/**
 * Reads ToDo from phone.
 */
GSM_Error GAMMU_GetToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	return s->Phone.Functions->GetToDo(s, ToDo);
}
/**
 * Reads ToDo from phone.
 */
GSM_Error GAMMU_GetNextToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo, bool start)
{
	return s->Phone.Functions->GetNextToDo(s, ToDo, start);
}
/**
 * Sets ToDo in phone.
 */
GSM_Error GAMMU_SetToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	return s->Phone.Functions->SetToDo(s, ToDo);
}
/**
 * Adds ToDo in phone.
 */
GSM_Error GAMMU_AddToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	return s->Phone.Functions->AddToDo(s, ToDo);
}
/**
 * Deletes ToDo entry in phone.
 */
GSM_Error GAMMU_DeleteToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	return s->Phone.Functions->DeleteToDo(s, ToDo);
}
/**
 * Deletes all todo entries in phone.
 */
GSM_Error GAMMU_DeleteAllToDo(GSM_StateMachine *s)
{
	return s->Phone.Functions->DeleteAllToDo(s);
}
/**
 * Retrieves calendar status (number of used entries).
 */
GSM_Error GAMMU_GetCalendarStatus(GSM_StateMachine *s, GSM_CalendarStatus *Status)
{
	return s->Phone.Functions->GetCalendarStatus(s, Status);
}
/**
 * Retrieves calendar entry.
 */
GSM_Error GAMMU_GetCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	return s->Phone.Functions->GetCalendar(s, Note);
}
/**
 * Retrieves calendar entry. This is useful for continuous reading of all
 * calendar entries.
 */
GSM_Error GAMMU_GetNextCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool start)
{
	return s->Phone.Functions->GetNextCalendar(s, Note, start);
}
/**
 * Sets calendar entry
 */
GSM_Error GAMMU_SetCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	return s->Phone.Functions->SetCalendar(s, Note);
}
/**
 * Adds calendar entry.
 */
GSM_Error GAMMU_AddCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	return s->Phone.Functions->AddCalendar(s, Note);
}
/**
 * Deletes calendar entry.
 */
GSM_Error GAMMU_DeleteCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	return s->Phone.Functions->DeleteCalendar(s, Note);
}
/**
 * Deletes all calendar entries.
 */
GSM_Error GAMMU_DeleteAllCalendar(GSM_StateMachine *s)
{
	return s->Phone.Functions->DeleteAllCalendar(s);
}
/**
 * Reads calendar settings.
 */
GSM_Error GAMMU_GetCalendarSettings(GSM_StateMachine *s, GSM_CalendarSettings *settings)
{
	return s->Phone.Functions->GetCalendarSettings(s, settings);
}
/**
 * Sets calendar settings.
 */
GSM_Error GAMMU_SetCalendarSettings(GSM_StateMachine *s, GSM_CalendarSettings *settings)
{
	return s->Phone.Functions->SetCalendarSettings(s, settings);
}
/**
 * Retrieves notes status (number of used entries).
 */
GSM_Error GAMMU_GetNotesStatus(GSM_StateMachine *s, GSM_ToDoStatus *status)
{
	return s->Phone.Functions->GetNotesStatus(s, status);
}
/**
 * Retrieves notes entry.
 */
GSM_Error GAMMU_GetNote(GSM_StateMachine *s, GSM_NoteEntry *Note)
{
	return s->Phone.Functions->GetNote(s, Note);
}
/**
 * Retrieves note entry. This is useful for continuous reading of all
 * notes entries.
 */
GSM_Error GAMMU_GetNextNote(GSM_StateMachine *s, GSM_NoteEntry *Note, bool start)
{
	return s->Phone.Functions->GetNextNote(s, Note, start);
}
/**
 * Sets note entry
 */
GSM_Error GAMMU_SetNote(GSM_StateMachine *s, GSM_NoteEntry *Note)
{
	return s->Phone.Functions->SetNote(s, Note);
}
/**
 * Adds note entry.
 */
GSM_Error GAMMU_AddNote(GSM_StateMachine *s, GSM_NoteEntry *Note)
{
	return s->Phone.Functions->AddNote(s, Note);
}
/**
 * Deletes note entry.
 */
GSM_Error GAMMU_DeleteNote(GSM_StateMachine *s, GSM_NoteEntry *Note)
{
	return s->Phone.Functions->DeleteNote(s, Note);
}
/**
 * Deletes all notes entries.
 */
GSM_Error GAMMU_DeleteAllNotes(GSM_StateMachine *s)
{
	return s->Phone.Functions->DeleteAllNotes(s);
}
/**
 * Reads profile.
 */
GSM_Error GAMMU_GetProfile(GSM_StateMachine *s, GSM_Profile *Profile)
{
	return s->Phone.Functions->GetProfile(s, Profile);
}
/**
 * Updates profile.
 */
GSM_Error GAMMU_SetProfile(GSM_StateMachine *s, GSM_Profile *Profile)
{
	return s->Phone.Functions->SetProfile(s, Profile);
}
/**
 * Reads FM station.
 */
GSM_Error GAMMU_GetFMStation(GSM_StateMachine *s, GSM_FMStation *FMStation)
{
	return s->Phone.Functions->GetFMStation(s, FMStation);
}
/**
 * Sets FM station.
 */
GSM_Error GAMMU_SetFMStation(GSM_StateMachine *s, GSM_FMStation *FMStation)
{
	return s->Phone.Functions->SetFMStation(s, FMStation);
}
/**
 * Clears defined FM stations.
 */
GSM_Error GAMMU_ClearFMStations(GSM_StateMachine *s)
{
	return s->Phone.Functions->ClearFMStations(s);
}
/**
 * Gets next filename from filesystem.
 */
GSM_Error GAMMU_GetNextFileFolder(GSM_StateMachine *s, GSM_File *File, bool start)
{
	return s->Phone.Functions->GetNextFileFolder(s, File, start);
}
/**
 * Gets file part from filesystem.
 */
GSM_Error GAMMU_GetFolderListing(GSM_StateMachine *s, GSM_File *File, bool start)
{
	return s->Phone.Functions->GetFolderListing(s, File, start);
}
/**
 * Gets next root folder.
 */
GSM_Error GAMMU_GetNextRootFolder(GSM_StateMachine *s, GSM_File *File)
{
	return s->Phone.Functions->GetNextRootFolder(s, File);
}
/**
 * Sets file system attributes.
 */
GSM_Error GAMMU_SetFileAttributes(GSM_StateMachine *s, GSM_File *File)
{
	return s->Phone.Functions->SetFileAttributes(s, File);
}
/**
 * Retrieves file part.
 */
GSM_Error GAMMU_GetFilePart(GSM_StateMachine *s, GSM_File *File, int *Handle, int *Size)
{
	return s->Phone.Functions->GetFilePart(s, File, Handle, Size);
}
/**
 * Adds file part to filesystem.
 */
GSM_Error GAMMU_AddFilePart(GSM_StateMachine *s, GSM_File *File, int *Pos, int *Handle)
{
	return s->Phone.Functions->AddFilePart(s, File, Pos, Handle);
}
/**
 * Sends file to phone, it's up to phone to decide what to do with it.
 */
GSM_Error GAMMU_SendFilePart(GSM_StateMachine *s, GSM_File *File, int *Pos, int *Handle)
{
	return s->Phone.Functions->SendFilePart(s, File, Pos, Handle);
}
/**
 * Acquires filesystem status.
 */
GSM_Error GAMMU_GetFileSystemStatus(GSM_StateMachine *s, GSM_FileSystemStatus *Status)
{
	return s->Phone.Functions->GetFileSystemStatus(s, Status);
}
/**
 * Deletes file from filessytem.
 */
GSM_Error GAMMU_DeleteFile(GSM_StateMachine *s, unsigned char *ID)
{
	return s->Phone.Functions->DeleteFile(s, ID);
}
/**
 * Adds folder to filesystem.
 */
GSM_Error GAMMU_AddFolder(GSM_StateMachine *s, GSM_File *File)
{
	return s->Phone.Functions->AddFolder(s, File);
}
/**
 * Deletes folder from filesystem.
 */
GSM_Error GAMMU_DeleteFolder(GSM_StateMachine *s, unsigned char *ID)
{
	return s->Phone.Functions->DeleteFolder(s, ID);
}
/**
 * Gets GPRS access point.
 */
GSM_Error GAMMU_GetGPRSAccessPoint(GSM_StateMachine *s, GSM_GPRSAccessPoint *point)
{
	return s->Phone.Functions->GetGPRSAccessPoint(s, point);
}
/**
 * Sets GPRS access point.
 */
GSM_Error GAMMU_SetGPRSAccessPoint(GSM_StateMachine *s, GSM_GPRSAccessPoint *point)
{
	return s->Phone.Functions->SetGPRSAccessPoint(s, point);
}

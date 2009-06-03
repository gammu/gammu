/* (c) 2002-2004 by Marcin Wiacek */

#ifndef phone_nokia_h
#define phone_nokia_h

#include <gammu-nokia.h>

#include "ncommon.h"
#include "../../gsmstate.h"

extern unsigned char N71_65_MEMORY_TYPES[];
extern GSM_Profile_PhoneTableValue Profile71_65[];

GSM_Error NOKIA_ReplyGetPhoneString		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N71_65_ReplyGetMemoryError		(unsigned char error     , GSM_StateMachine *s);
GSM_Error N71_65_ReplyWritePhonebook		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N71_65_ReplyGetCalendarInfo1		(GSM_Protocol_Message msg, GSM_StateMachine *s, GSM_NOKIACalToDoLocations *LastCalendar);
GSM_Error N71_65_ReplyGetNextCalendar1		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N71_65_ReplyGetCalendarNotePos1	(GSM_Protocol_Message msg, GSM_StateMachine *s,int *FirstCalendarPos);
GSM_Error N71_65_ReplyAddCalendar1		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N71_65_ReplyAddCalendar2		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N71_65_ReplyDelCalendar		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N71_65_ReplyCallInfo			(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N71_65_ReplyUSSDInfo			(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3DCT4_ReplyCallDivert		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3DCT4_ReplyGetActiveConnectSet	(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3DCT4_ReplySetActiveConnectSet	(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3DCT4_ReplyGetWAPBookmark		(GSM_Protocol_Message msg, GSM_StateMachine *s, gboolean FullLength);
GSM_Error DCT3DCT4_ReplySetWAPBookmark		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3DCT4_ReplyDelWAPBookmark		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3DCT4_ReplyEnableConnectFunc	(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3DCT4_ReplyDisableConnectFunc      (GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT3DCT4_ReplyGetModelFirmware	(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N71_65_ReplySendDTMF			(GSM_Protocol_Message msg, GSM_StateMachine *s);

GSM_Error NOKIA_GetManufacturer			(GSM_StateMachine *s);
GSM_Error NOKIA_GetPhoneString			(GSM_StateMachine *s, const unsigned char *msgframe, int msglen, unsigned char msgtype, char *retvalue, GSM_Phone_RequestID request, int startresponse);
GSM_Error NOKIA_SetIncomingSMS			(GSM_StateMachine *s, gboolean enable);
GSM_Error NOKIA_SetIncomingCall			(GSM_StateMachine *s, gboolean enable);
GSM_Error NOKIA_SetIncomingUSSD			(GSM_StateMachine *s, gboolean enable);
GSM_Error N71_65_EnableFunctions		(GSM_StateMachine *s, const char *buff,int len);
GSM_Error N71_65_GetNextCalendar1		(GSM_StateMachine *s, GSM_CalendarEntry *Note, gboolean start, GSM_NOKIACalToDoLocations *LastCalendar, int *LastCalendarYear, int *LastCalendarPos);
GSM_Error N71_65_AddCalendar2			(GSM_StateMachine *s, GSM_CalendarEntry *Note);
GSM_Error N71_65_AddCalendar1			(GSM_StateMachine *s, GSM_CalendarEntry *Note, int *FirstCalendarPos);
GSM_Error N71_65_DelCalendar			(GSM_StateMachine *s, GSM_CalendarEntry *Note);
GSM_Error N71_65_GetCalendarInfo1		(GSM_StateMachine *s, GSM_NOKIACalToDoLocations *LastCalendar);
GSM_Error DCT3DCT4_EnableWAPFunctions		(GSM_StateMachine *s);
GSM_Error DCT3DCT4_SendDTMF			(GSM_StateMachine *s, char *sequence);
GSM_Error DCT3DCT4_DeleteWAPBookmarkPart	(GSM_StateMachine *s, GSM_WAPBookmark *bookmark);
GSM_Error DCT3DCT4_GetWAPBookmarkPart		(GSM_StateMachine *s, GSM_WAPBookmark *bookmark);
GSM_Error DCT3DCT4_DisableConnectionFunctions	(GSM_StateMachine *s);
GSM_Error DCT3DCT4_GetModel			(GSM_StateMachine *s);
GSM_Error DCT3DCT4_GetFirmware			(GSM_StateMachine *s);
GSM_Error DCT3DCT4_AnswerCall			(GSM_StateMachine *s, int ID);
GSM_Error DCT3DCT4_CancelCall			(GSM_StateMachine *s, int ID);
GSM_Error DCT3DCT4_GetActiveConnectSet		(GSM_StateMachine *s);
GSM_Error DCT3DCT4_SetActiveConnectSet		(GSM_StateMachine *s, GSM_MultiWAPSettings *settings);
GSM_Error DCT3DCT4_CancelAllDiverts		(GSM_StateMachine *s);
GSM_Error DCT3DCT4_SetCallDivert		(GSM_StateMachine *s, GSM_MultiCallDivert *divert);
GSM_Error DCT3DCT4_GetCallDivert		(GSM_StateMachine *s, GSM_MultiCallDivert *divert);

GSM_CalendarNoteType N71_65_FindCalendarType(GSM_CalendarNoteType Type, GSM_PhoneModel *model);
int 		NOKIA_SetUnicodeString		(GSM_StateMachine *s, unsigned char *dest, unsigned char *string, gboolean FullLength);
void 		NOKIA_GetUnicodeString		(GSM_StateMachine *s, int *current, unsigned char *input, unsigned char *output, gboolean FullLength);
GSM_MemoryType 	NOKIA_GetMemoryType 		(GSM_StateMachine *s, GSM_MemoryType memory_type, unsigned char *ID);
void 		NOKIA_DecodeSMSState		(GSM_StateMachine *s, unsigned char state, GSM_SMSMessage *sms);
void		NOKIA_EncodeDateTime		(GSM_StateMachine *s, unsigned char* buffer, GSM_DateTime *datetime);
void NOKIA_DecodeDateTime(GSM_StateMachine *s, unsigned char* buffer, GSM_DateTime *datetime, gboolean seconds, gboolean DayMonthReverse);
void 		NOKIA_SortSMSFolderStatus	(GSM_StateMachine *s, GSM_NOKIASMSFolder *Folder);
void NOKIA_FindFeatureValue(GSM_StateMachine		*s,
			    GSM_Profile_PhoneTableValue ProfileTable[],
  			    unsigned char 		ID,
  			    unsigned char 		Value,
  			    GSM_Phone_Data 		*Data,
  			    gboolean			CallerGroups);
gboolean NOKIA_FindPhoneFeatureValue(GSM_StateMachine		*s,
				 GSM_Profile_PhoneTableValue 	ProfileTable[],
  				 GSM_Profile_Feat_ID		FeatureID,
  				 GSM_Profile_Feat_Value		FeatureValue,
  			    	 unsigned char 			*PhoneID,
  			    	 unsigned char 			*PhoneValue);

GSM_Error N71_65_DecodePhonebook		(GSM_StateMachine	*s,
						 GSM_MemoryEntry 	*entry,
  				 		 GSM_Bitmap 		*bitmap,
  				 		 GSM_SpeedDial 		*speed,
  				 		 unsigned char 		*MessageBuffer,
  				 		 int 			MessageLength,
						 gboolean			DayMonthReverse);
size_t 		N71_65_EncodePhonebookFrame	(GSM_StateMachine *s, unsigned char *req, GSM_MemoryEntry *entry, size_t *block2, gboolean DCT4, gboolean VoiceTag);
size_t 		N71_65_PackPBKBlock		(GSM_StateMachine *s, int id, size_t size, int no, unsigned char *buf, unsigned char *block);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

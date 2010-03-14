
#ifndef phone_nokia_h
#define phone_nokia_h

#include "ncommon.h"
#include "../../gsmcomon.h"
#include "../../gsmstate.h"

extern unsigned char N71_65_MEMORY_TYPES[];
extern GSM_Profile_PhoneTableValue Profile71_65[];

GSM_Error NOKIA_ReplyGetPhoneString		(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User);
GSM_Error N71_65_ReplyGetMemoryError		(unsigned char error,	   GSM_Phone_Data *Data);
GSM_Error N71_65_ReplyWritePhonebook		(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User);
GSM_Error N71_65_ReplyGetNextNote		(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User);
GSM_Error N71_65_ReplyGetCalendarInfo		(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User, GSM_NOKIACalendarLocations *LastCalendar);
GSM_Error N71_65_ReplyGetCalendarNote		(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User);
GSM_Error N71_65_ReplyDeleteCalendarNote	(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User);
GSM_Error N71_65_ReplySetCalendarNote		(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User);
GSM_Error N71_65_ReplyGetCalendarNotePos	(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User, int *FirstCalendarPos);
GSM_Error N71_65_ReplyDeleteMemory		(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User);
GSM_Error N71_65_ReplyCallInfo			(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User);
GSM_Error DCT3DCT4_ReplyGetWAPBookmark		(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User, bool FullLength);
GSM_Error DCT3DCT4_ReplySetWAPBookmark		(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User);
GSM_Error DCT3DCT4_ReplyDelWAPBookmark		(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User);
GSM_Error DCT3DCT4_ReplyEnableWAP		(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User);
GSM_Error DCT3DCT4_ReplyGetModelFirmware	(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User);

GSM_Error NOKIA_GetManufacturer			(GSM_StateMachine *s, char		  *manufacturer );
GSM_Error NOKIA_GetPhoneString			(GSM_StateMachine *s, unsigned char *msgframe, int msglen, unsigned char msgtype, char *retvalue, GSM_Phone_RequestID request, int startresponse);
GSM_Error NOKIA_SetIncomingSMS			(GSM_StateMachine *s, bool enable);
GSM_Error N71_65_GetCalendarInfo		(GSM_StateMachine *s);
GSM_Error N71_65_GetCalendarNote		(GSM_StateMachine *s, GSM_CalendarNote *Note, bool start, GSM_NOKIACalendarLocations *LastCalendar);
GSM_Error N71_65_DelCalendarNote		(GSM_StateMachine *s, GSM_CalendarNote *Note, GSM_NOKIACalendarLocations *LastCalendar);
GSM_Error N71_65_SetCalendarNote		(GSM_StateMachine *s, GSM_CalendarNote *Note,int *FirstCalendarPos);
GSM_Error N71_65_GetNextCalendarNote		(GSM_StateMachine *s, GSM_CalendarNote *Note, bool start, int *LastCalendarYear, int *LastCalendarPos);
GSM_Error N71_65_DeleteMemory			(GSM_StateMachine *s, GSM_PhonebookEntry *entry, unsigned char *memory);
GSM_Error DCT3DCT4_DeleteWAPBookmark		(GSM_StateMachine *s, GSM_WAPBookmark 	  *bookmark);
GSM_Error DCT3DCT4_EnableWAP			(GSM_StateMachine *s);
GSM_Error DCT3DCT4_GetModel			(GSM_StateMachine *s);
GSM_Error DCT3DCT4_GetFirmware			(GSM_StateMachine *s);
GSM_Error DCT3DCT4_GetWAPBookmark		(GSM_StateMachine *s, GSM_WAPBookmark	  *bookmark	);

int 		NOKIA_SetUnicodeString		(unsigned char *dest, unsigned char *string, bool FullLength);
void 		NOKIA_GetUnicodeString		(int *current, unsigned char *input, unsigned char *output, bool FullLength);
GSM_MemoryType 	NOKIA_GetMemoryType 		(GSM_MemoryType memory_type, unsigned char *ID);
void 		NOKIA_DecodeSMSState		(unsigned char state, GSM_SMSMessage *sms);
void		NOKIA_EncodeDateTime		(unsigned char* buffer, GSM_DateTime *datetime);
void		NOKIA_DecodeDateTime		(unsigned char* buffer, GSM_DateTime *datetime);
void 		NOKIA_SortSMSFolderStatus	(GSM_NOKIASMSFolder *Folder);
void 		NOKIA_GetDefaultCallerGroupName	(GSM_StateMachine *s, GSM_Bitmap *Bitmap);
void 		NOKIA_GetDefaultProfileName	(GSM_StateMachine *s, GSM_Profile *Profile);
void NOKIA_FindFeatureValue(GSM_Profile_PhoneTableValue ProfileTable[],
			    unsigned char 		ID,
			    unsigned char 		Value,
			    GSM_Phone_Data 		*Data,
			    bool			CallerGroups);
bool NOKIA_FindPhoneFeatureValue(GSM_Profile_PhoneTableValue 	ProfileTable[],
				 GSM_Profile_Feat_ID		FeatureID,
				 GSM_Profile_Feat_Value		FeatureValue,
			    	 unsigned char 			*PhoneID,
			    	 unsigned char 			*PhoneValue);

GSM_Error N71_65_DecodePhonebook		(GSM_PhonebookEntry 	*entry,
				 		 GSM_Bitmap 		*bitmap,
				 		 GSM_SpeedDial 		*speed,
				 		 unsigned char 		*MessageBuffer,
				 		 int 			MessageLength);
int 		N71_65_EncodePhonebookFrame	(unsigned char *req, GSM_PhonebookEntry entry, int *block2, bool URL);
int 		N71_65_PackPBKBlock		(int id, int size, int no, unsigned char *buf, unsigned char *block);

#endif

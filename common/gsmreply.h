/* (c) 2002-2009 by Marcin Wiacek & Michal Cihar */

/**
 * @file gsmreply.h
 * @author Michal Čihař
 * @author Marcin Wiacek
 */

/**
 * \addtogroup StateMachine
 *
 * @{
 */

#ifndef __gsm_reply_h
#define __gsm_reply_h

#include "protocol/protocol.h"
#include <gammu.h>

/**
 * Phone requests identificators, these are used for internally identifying
 * which operation is being performed.
 */
typedef enum {
	ID_None=1,
	ID_GetModel,
	ID_GetFirmware,
	ID_EnableSecurity,
	ID_OpenFile,
	ID_CloseFile,
	ID_GetIMEI,
	ID_GetDateTime,
	ID_GetAlarm,
	ID_GetMemory,
	ID_GetMemoryStatus,
	ID_GetSMSC,
	ID_GetSMSMessage,
	ID_EnableEcho,
	ID_EnableErrorInfo,
	ID_SetOBEX,
	ID_SetUSSD,
	ID_GetUSSD,
	ID_GetNote,
	ID_SetNote,
	ID_GetSignalQuality,
	ID_GetBatteryCharge,
	ID_GetSMSFolders,
	ID_GetSMSFolderStatus,
	ID_GetSMSStatus,
	ID_AddSMSFolder,
	ID_GetNetworkInfo,
	ID_GetNetworkCode,
	ID_GetNetworkName,
	ID_GetRingtone,
	ID_DialVoice,
	ID_GetCalendarNotesInfo,
	ID_GetCalendarNote,
	ID_GetSecurityCode,
	ID_GetWAPBookmark,
	ID_GetBitmap,
	ID_GetCRC,
	ID_SetAttrib,
	ID_SaveSMSMessage,
	ID_CancelCall,
	ID_SetDateTime,
	ID_SetAlarm,
	ID_DisableConnectFunc,
	ID_EnableConnectFunc,
	ID_AnswerCall,
	ID_SetBitmap,
	ID_SetRingtone,
	ID_DeleteSMSMessage,
	ID_DeleteCalendarNote,
	ID_SetPath,
	ID_SetSMSC,
	ID_SetProfile,
	ID_SetMemory,
	ID_DeleteMemory,
	ID_SetCalendarNote,
	ID_SetIncomingSMS,
	ID_SetIncomingCB,
	ID_SetIncomingCall,
	ID_GetCNMIMode,
	ID_GetCalendarNotePos,
	ID_Initialise,
	ID_GetConnectSet,
	ID_SetWAPBookmark,
	ID_GetLocale,
	ID_SetLocale,
	ID_GetCalendarSettings,
	ID_SetCalendarSettings,
	ID_GetGPRSPoint,
	ID_SetGPRSPoint,
	ID_EnableGPRSPoint,
	ID_DeleteWAPBookmark,
	ID_Netmonitor,
	ID_HoldCall,
	ID_UnholdCall,
	ID_ConferenceCall,
	ID_SplitCall,
	ID_TransferCall,
	ID_SwitchCall,
	ID_GetManufactureMonth,
	ID_GetProductCode,
	ID_GetOriginalIMEI,
	ID_GetHardware,
	ID_GetPPM,
	ID_GetSMSMode,
	ID_GetSMSMemories,
	ID_GetManufacturer,
	ID_SetMemoryType,
	ID_GetMemoryCharset,
	ID_SetMemoryCharset,
	ID_SetSMSParameters,
	ID_GetFMStation,
	ID_SetFMStation,
	ID_GetLanguage,
	ID_SetFastSMSSending,
	ID_Reset,
	ID_GetToDo,
	ID_PressKey,
	ID_DeleteAllToDo,
	ID_SetLight,
	ID_Divert,
	ID_SetToDo,
	ID_PlayTone,
	ID_GetChatSettings,
	ID_GetSyncMLSettings,
	ID_GetSyncMLName,
	ID_GetSecurityStatus,
	ID_EnterSecurityCode,
	ID_GetProfile,
	ID_GetRingtonesInfo,
	ID_MakeAuthentication,
	ID_GetSpeedDial,
	ID_ResetPhoneSettings,
	ID_SendDTMF,
	ID_GetDisplayStatus,
	ID_SetAutoNetworkLogin,
	ID_SetConnectSet,
	ID_GetSIMIMSI,
	ID_GetFileInfo,
	ID_FileSystemStatus,
	ID_GetFile,
	ID_AddFile,
	ID_AddFolder,
	ID_DeleteFolder,
	ID_DeleteFile,
	ID_ModeSwitch,
	ID_GetProtocol,

    	/* Alcatel AT mode */
    	ID_SetFlowControl,
    	ID_AlcatelConnect,
	ID_AlcatelProtocol,

    	/* Alcatel Binary mode */
    	ID_AlcatelAttach,
    	ID_AlcatelDetach,
    	ID_AlcatelCommit,
    	ID_AlcatelCommit2,
	ID_AlcatelEnd,
    	ID_AlcatelClose,
   	ID_AlcatelStart,
    	ID_AlcatelSelect1,
    	ID_AlcatelSelect2,
    	ID_AlcatelSelect3,
    	ID_AlcatelBegin1,
   	ID_AlcatelBegin2,
    	ID_AlcatelGetIds1,
    	ID_AlcatelGetIds2,
        ID_AlcatelGetCategories1,
        ID_AlcatelGetCategories2,
        ID_AlcatelGetCategoryText1,
        ID_AlcatelGetCategoryText2,
        ID_AlcatelAddCategoryText1,
        ID_AlcatelAddCategoryText2,
    	ID_AlcatelGetFields1,
    	ID_AlcatelGetFields2,
    	ID_AlcatelGetFieldValue1,
    	ID_AlcatelGetFieldValue2,
   	ID_AlcatelDeleteItem1,
   	ID_AlcatelDeleteItem2,
   	ID_AlcatelDeleteField,
	ID_AlcatelCreateField,
	ID_AlcatelUpdateField,

	ID_IncomingFrame,

	ID_User1,
	ID_User2,
	ID_User3,
	ID_User4,
	ID_User5,
	ID_User6,
	ID_User7,
	ID_User8,
	ID_User9,
	ID_User10,

	ID_EachFrame
} GSM_Phone_RequestID;

/**
 * Structure for defining reply functions.
 *
 * Function is called when requestID matches current operation or is
 * ID_IncomingFrame and msgtype matches start message and (if msgtype is just
 * one character) subtypechar is zero or subtypechar-th character of message
 * matches subtype.
 *
 * Should be used in array with last element containing ID_None as requestID.
 */
typedef struct {
	/**
	 * Pointer to function that should be executed.
	 */
	GSM_Error (*Function)	(GSM_Protocol_Message msg, GSM_StateMachine *s);
	/**
	 * Message type, if it is longer than 1 character, it disables subtype
	 * checking.
	 */
	const unsigned char		*msgtype;
	/**
	 * Which character of message should be checked as subtype. Zero to
	 * disable subtype checking.
	 */
	const size_t			subtypechar;
	/**
	 * Subtype to be checked.
	 */
	const unsigned char		subtype;
	/**
	 * Phone request when this can be called, use ID_IncomingFrame when
	 * you want to use this in any state.
	 */
	const GSM_Phone_RequestID	requestID;
} GSM_Reply_Function;

#endif
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

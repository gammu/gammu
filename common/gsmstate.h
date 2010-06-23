
#ifndef gsm_statemachine_h
#define gsm_statemachine_h

#include <time.h>

#include "../cfg/config.h"
#include "misc/cfg.h"

#ifdef GSM_ENABLE_NOKIA6110
#  include "phone/nokia/dct3/n6110.h"
#endif
#ifdef GSM_ENABLE_NOKIA6510
#  include "phone/nokia/dct4/n6510.h"
#endif
#ifdef GSM_ENABLE_NOKIA7110
#  include "phone/nokia/dct3/n7110.h"
#endif
#ifdef GSM_ENABLE_NOKIA9210
#  include "phone/nokia/dct3/n9210.h"
#endif
#ifdef GSM_ENABLE_ATGEN
#  include "phone/at/atgen.h"
#endif
#ifdef GSM_ENABLE_ALCATEL
#  include "phone/alcatel/alcatel.h"
#endif
#ifdef GSM_ENABLE_OBEXGEN
#  include "phone/obex/obexgen.h"
#endif

#ifndef GSM_USED_MBUS2
#  undef GSM_ENABLE_MBUS2
#endif
#ifndef GSM_USED_FBUS2
#  undef GSM_ENABLE_FBUS2
#endif
#ifndef GSM_USED_FBUS2DLR3
#  undef GSM_ENABLE_FBUS2DLR3
#endif
#ifndef GSM_USED_FBUS2BLUE
#  undef GSM_ENABLE_FBUS2BLUE
#endif
#ifndef GSM_USED_FBUS2IRDA
#  undef GSM_ENABLE_FBUS2IRDA
#endif
#ifndef GSM_USED_PHONETBLUE
#  undef GSM_ENABLE_PHONETBLUE
#endif
#ifndef GSM_USED_AT
#  undef GSM_ENABLE_AT
#endif
#ifndef GSM_USED_IRDAOBEX
#  undef GSM_ENABLE_IRDAOBEX
#endif
#ifndef GSM_USED_BLUEOBEX
#  undef GSM_ENABLE_BLUEOBEX
#endif
#ifndef GSM_USED_ALCABUS
#  undef GSM_ENABLE_ALCABUS
#endif
#ifndef GSM_USED_IRDAPHONET
#  undef GSM_ENABLE_IRDAPHONET
#endif
#ifndef GSM_USED_BLUEFBUS2
#  undef GSM_ENABLE_BLUEFBUS2
#endif
#ifndef GSM_USED_BLUEPHONET
#  undef GSM_ENABLE_BLUEPHONET
#endif
#ifndef GSM_USED_BLUEAT
#  undef GSM_ENABLE_BLUEAT
#endif
#ifndef GSM_USED_IRDAAT
#  undef GSM_ENABLE_IRDAAT
#endif

#if defined(GSM_ENABLE_NOKIA6110) || defined(GSM_ENABLE_NOKIA7110) || defined(GSM_ENABLE_NOKIA9210)
#  define GSM_ENABLE_NOKIA_DCT3
#endif
#if defined(GSM_ENABLE_NOKIA6510)
#  define GSM_ENABLE_NOKIA_DCT4
#endif

#include "protocol/protocol.h"
#if defined(GSM_ENABLE_FBUS2) || defined(GSM_ENABLE_FBUS2IRDA) || defined(GSM_ENABLE_FBUS2DLR3) || defined(GSM_ENABLE_FBUS2BLUE) || defined(GSM_ENABLE_BLUEFBUS2)
#  include "protocol/nokia/fbus2.h"
#endif
#ifdef GSM_ENABLE_MBUS2
#  include "protocol/nokia/mbus2.h"
#endif
#if defined(GSM_ENABLE_PHONETBLUE) || defined(GSM_ENABLE_IRDAPHONET) || defined(GSM_ENABLE_BLUEPHONET)
#  include "protocol/nokia/phonet.h"
#endif
#if defined(GSM_ENABLE_AT) || defined(GSM_ENABLE_BLUEAT) || defined(GSM_ENABLE_IRDAAT)
#  include "protocol/at/at.h"
#endif
#ifdef GSM_ENABLE_ALCABUS
#  include "protocol/alcatel/alcabus.h"
#endif
#if defined(GSM_ENABLE_IRDAOBEX) || defined(GSM_ENABLE_BLUEOBEX)
#  include "protocol/obex/obex.h"
#endif

#define GSM_ENABLE_SERIALDEVICE
#ifndef GSM_USED_SERIALDEVICE
#  undef GSM_ENABLE_SERIALDEVICE
#endif
#define GSM_ENABLE_IRDADEVICE
#ifndef GSM_USED_IRDADEVICE
#  undef GSM_ENABLE_IRDADEVICE
#endif
#define GSM_ENABLE_BLUETOOTHDEVICE
#ifndef GSM_USED_BLUETOOTHDEVICE
#  undef GSM_ENABLE_BLUETOOTHDEVICE
#endif

#ifdef DJGPP
#  undef GSM_ENABLE_IRDADEVICE
#  undef GSM_ENABLE_FBUS2IRDA
#  undef GSM_ENABLE_IRDAPHONET
#  undef GSM_ENABLE_BLUETOOTHDEVICE
#  undef GSM_ENABLE_BLUEFBUS2
#  undef GSM_ENABLE_BLUEPHONET
#  undef GSM_ENABLE_BLUEAT
#  undef GSM_ENABLE_IRDAAT
#  undef GSM_ENABLE_PHONETBLUE
#  undef GSM_ENABLE_FBUS2BLUE
#  undef GSM_ENABLE_IRDAOBEX
#  undef GSM_ENABLE_BLUEOBEX
#endif

#include "device/serial/ser_w32.h"
#include "device/serial/ser_unx.h"
#include "device/serial/ser_djg.h"
#include "device/irda/irda.h"
#include "device/bluetoth/bluetoth.h"

#include "service/gsmpbk.h"
#include "service/gsmsms.h"
#include "service/gsmnet.h"
#include "service/gsmring.h"
#include "service/gsmcal.h"
#include "service/gsmwap.h"
#include "service/gsmlogo.h"
#include "service/gsmmisc.h"
#include "service/gsmprof.h"
#include "service/gsmcall.h"

typedef struct _GSM_StateMachine GSM_StateMachine;
typedef struct _GSM_User	 GSM_User;
typedef struct _OnePhoneModel	 OnePhoneModel;

/* ------------------------- Device layer ---------------------------------- */

typedef struct {
	GSM_Error (*OpenDevice)        (GSM_StateMachine *s);
	GSM_Error (*CloseDevice)       (GSM_StateMachine *s);
	GSM_Error (*DeviceSetParity)   (GSM_StateMachine *s, bool parity);
	GSM_Error (*DeviceSetDtrRts)   (GSM_StateMachine *s, bool dtr, bool rts);
	GSM_Error (*DeviceSetSpeed)    (GSM_StateMachine *s, int speed);
	int       (*ReadDevice)        (GSM_StateMachine *s, void *buf, size_t nbytes);
	int       (*WriteDevice)       (GSM_StateMachine *s, void *buf, size_t nbytes);
} GSM_Device_Functions;

#ifdef GSM_ENABLE_SERIALDEVICE
	extern GSM_Device_Functions SerialDevice;
#endif
#ifdef GSM_ENABLE_IRDADEVICE
	extern GSM_Device_Functions IrdaDevice;
#endif
#ifdef GSM_ENABLE_BLUETOOTHDEVICE
	extern GSM_Device_Functions BlueToothDevice;
#endif

typedef struct {
	union {
		char fake;
#ifdef GSM_ENABLE_SERIALDEVICE
		GSM_Device_SerialData		Serial;
#endif
#ifdef GSM_ENABLE_IRDADEVICE
		GSM_Device_IrdaData		Irda;
#endif
#ifdef GSM_ENABLE_BLUETOOTHDEVICE
		GSM_Device_BlueToothData	BlueTooth;
#endif
	} Data;
	GSM_Device_Functions *Functions;
} GSM_Device;

/* ---------------------- Protocol layer ----------------------------------- */

typedef struct {
	GSM_Error (*WriteMessage) (GSM_StateMachine *s, unsigned char *buffer,
				   int length, unsigned char type);
	GSM_Error (*StateMachine) (GSM_StateMachine *s, unsigned char rx_byte);
	GSM_Error (*Initialise)   (GSM_StateMachine *s);
	GSM_Error (*Terminate)    (GSM_StateMachine *s);
} GSM_Protocol_Functions;

#ifdef GSM_ENABLE_MBUS2
	extern GSM_Protocol_Functions MBUS2Protocol;
#endif
#if defined(GSM_ENABLE_FBUS2) || defined(GSM_ENABLE_FBUS2IRDA) || defined(GSM_ENABLE_FBUS2DLR3) || defined(GSM_ENABLE_FBUS2BLUE) || defined(GSM_ENABLE_BLUEFBUS2)
	extern GSM_Protocol_Functions FBUS2Protocol;
#endif
#if defined(GSM_ENABLE_PHONETBLUE) || defined(GSM_ENABLE_IRDAPHONET) || defined(GSM_ENABLE_BLUEPHONET)
	extern GSM_Protocol_Functions PHONETProtocol;
#endif
#if defined(GSM_ENABLE_AT) || defined(GSM_ENABLE_BLUEAT) || defined(GSM_ENABLE_IRDAAT)
	extern GSM_Protocol_Functions ATProtocol;
#endif
#ifdef GSM_ENABLE_ALCABUS
	extern GSM_Protocol_Functions ALCABUSProtocol;
#endif
#if defined(GSM_ENABLE_IRDAOBEX) || defined(GSM_ENABLE_BLUEOBEX)
	extern GSM_Protocol_Functions OBEXProtocol;
#endif

typedef struct {
	struct {
		char fake;
#ifdef GSM_ENABLE_MBUS2
		GSM_Protocol_MBUS2Data		MBUS2;
#endif
#if defined(GSM_ENABLE_FBUS2) || defined(GSM_ENABLE_FBUS2IRDA) || defined(GSM_ENABLE_FBUS2DLR3) || defined(GSM_ENABLE_FBUS2BLUE) || defined(GSM_ENABLE_BLUEFBUS2)
		GSM_Protocol_FBUS2Data		FBUS2;
#endif
#if defined(GSM_ENABLE_PHONETBLUE) || defined(GSM_ENABLE_IRDAPHONET) || defined(GSM_ENABLE_BLUEPHONET)
		GSM_Protocol_PHONETData		PHONET;
#endif
#if defined(GSM_ENABLE_AT) || defined(GSM_ENABLE_BLUEAT) || defined(GSM_ENABLE_IRDAAT)
		GSM_Protocol_ATData		AT;
#endif
#ifdef GSM_ENABLE_ALCABUS
		GSM_Protocol_ALCABUSData	ALCABUS;
#endif
#if defined(GSM_ENABLE_IRDAOBEX) || defined(GSM_ENABLE_BLUEOBEX)
		GSM_Protocol_OBEXData		OBEX;
#endif
	} Data;
	GSM_Protocol_Functions *Functions;
} GSM_Protocol;

/* -------------------------- Phone layer ---------------------------------- */

typedef enum {
	ID_None=1,
	ID_GetModel,
	ID_GetFirmware,
	ID_EnableSecurity,
	ID_GetIMEI,
	ID_GetDateTime,
	ID_GetAlarm,
	ID_GetMemory,
	ID_GetMemoryStatus,
	ID_GetSMSC,
	ID_GetSMSMessage,
	ID_EnableEcho,
	ID_GetSignalQuality,
	ID_GetBatteryCharge,
	ID_GetSMSFolders,
	ID_GetSMSFolderStatus,
	ID_GetSMSStatus,
	ID_AddSMSFolder,
	ID_GetNetworkInfo,
	ID_GetRingtone,
	ID_DialVoice,
	ID_GetCalendarNotesInfo,
	ID_GetCalendarNote,
	ID_GetSecurityCode,
	ID_EnableWAP,
	ID_GetWAPBookmark,
	ID_GetBitmap,
	ID_SaveSMSMessage,
	ID_CancelCall,
	ID_SetDateTime,
	ID_SetAlarm,
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
	ID_GetCalendarNotePos,
	ID_Initialise,
	ID_GetWAPSettings,
	ID_SetWAPBookmark,
	ID_GetLocale,
	ID_SetLocale,
	ID_GetCalendarSettings,
	ID_SetCalendarSettings,
	ID_GetGPRSPoint,
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
	ID_SetMemoryCharset,
	ID_GetMMSSettings,
	ID_SetSMSParameters,
	ID_GetFMStation,
	ID_SetFMStation,
	ID_GetLanguage,
	ID_Reset,
	ID_GetToDo,
	ID_PressKey,
	ID_DeleteAllToDo,
	ID_SetLight,
	ID_Divert,
	ID_SetToDo,
	ID_PlayTone,
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
	ID_SetWAPSettings,
	ID_SetMMSSettings,
	ID_GetSIMIMSI,
	ID_GetFileInfo,
	ID_FileSystemStatus,
	ID_GetFile,
	ID_AddFile,
	ID_AddFolder,
	ID_DeleteFile,
#ifdef GSM_ENABLE_ALCATEL
    	/* AT mode */
    	ID_SetFlowControl,
    	ID_AlcatelConnect,
	ID_AlcatelProtocol,
    
    	/* Binary mode */
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
    	ID_AlcatelGetFields1,
    	ID_AlcatelGetFields2,
    	ID_AlcatelGetFieldValue1,
    	ID_AlcatelGetFieldValue2,    
   	ID_AlcatelDeleteItem1,
   	ID_AlcatelDeleteItem2,
   	ID_AlcatelDeleteField,
	ID_AlcatelCreateField,
	ID_AlcatelUpdateField,
#endif
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

typedef struct {
	char			IMEI[MAX_IMEI_LENGTH];			/* IMEI 				*/
	char			Manufacturer[MAX_MANUFACTURER_LENGTH];	/* Real connected phone manufacturer	*/
	char			Model[MAX_MODEL_LENGTH];		/* Real connected phone model 		*/
	OnePhoneModel		*ModelInfo;				/* Model information			*/
	char			Version[MAX_VERSION_LENGTH];		/* Real connected phone version 	*/
	char			VerDate[MAX_VERSION_DATE_LENGTH];	/* Version date				*/
	double			VerNum;					/* Phone version as number 		*/
	/* Some modules can cache these variables */
	char			HardwareCache[50];			/* Hardware version			*/
	char			ProductCodeCache[50];			/* Product code version			*/

	int			StartInfoCounter;

	GSM_GPRSAccessPoint	*GPRSPoint;
	GSM_SpeedDial		*SpeedDial;
	GSM_DateTime		*DateTime;
	GSM_DateTime		*Alarm;
	GSM_PhonebookEntry	*Memory;
	GSM_MemoryStatus	*MemoryStatus;
	GSM_SMSC		*SMSC;
	GSM_MultiSMSMessage	*GetSMSMessage;
	GSM_SMSMessage		*SaveSMSMessage;
	GSM_SMSMemoryStatus	*SMSStatus;
	GSM_SMSFolders		*SMSFolders;
	int                 	*VoiceRecord;
	int			CallID;
	GSM_SignalQuality	*SignalQuality;
	GSM_BatteryCharge	*BatteryCharge;
	GSM_NetworkInfo		*NetworkInfo;
	GSM_Ringtone		*Ringtone;
	GSM_CalendarEntry	*Cal;
	unsigned char		*SecurityCode;
	GSM_WAPBookmark		*WAPBookmark;
	GSM_MultiWAPSettings	*WAPSettings;
	GSM_Bitmap		*Bitmap;
	unsigned char		*Netmonitor;
	GSM_MultiCallDivert	*Divert;
	GSM_ToDoEntry		*ToDo;
	bool			PressKey;
	GSM_SecurityCodeType	*SecurityStatus;
	GSM_Profile		*Profile;
	GSM_AllRingtonesInfo	*RingtonesInfo;
	GSM_DisplayFeatures	*DisplayFeatures;
	GSM_FMStation		*FMStation;
	GSM_Locale		*Locale;
	GSM_CalendarSettings	*CalendarSettings;
	unsigned char		*PhoneString;
	int			StartPhoneString;
	GSM_File		*FileInfo;
	GSM_File		*File;
	GSM_FileSystemStatus	*FileSystemStatus;

	bool			EnableIncomingCall;
	bool			EnableIncomingSMS;	/* notify about incoming sms ? 	*/
	bool			EnableIncomingCB;	/* notify about incoming cb ?	*/
	bool			EnableIncomingUSSD;

	GSM_Protocol_Message	*RequestMsg;		/* last frame from phone	*/
	unsigned int		RequestID;		/* what operation is done now	*/
	GSM_Error		DispatchError;		/* error returned by function	*/
							/* in phone module		*/
	struct {
		int			fake;
#ifdef GSM_ENABLE_NOKIA6110
		GSM_Phone_N6110Data	N6110;
#endif
#ifdef GSM_ENABLE_NOKIA6510
		GSM_Phone_N6510Data	N6510;
#endif
#ifdef GSM_ENABLE_NOKIA7110
		GSM_Phone_N7110Data	N7110;
#endif
#ifdef GSM_ENABLE_ATGEN
		GSM_Phone_ATGENData	ATGEN;
#endif
#ifdef GSM_ENABLE_ALCATEL
		GSM_Phone_ALCATELData	ALCATEL;
#endif
#ifdef GSM_ENABLE_OBEXGEN
		GSM_Phone_OBEXGENData	OBEXGEN;
#endif
	} Priv;
} GSM_Phone_Data;

typedef struct {
	GSM_Error (*Function)	(GSM_Protocol_Message msg, GSM_StateMachine *s);
	unsigned char		*msgtype;
	int			subtypechar;
	unsigned char		subtype;
	GSM_Phone_RequestID	requestID;
} GSM_Reply_Function;

typedef struct {
	char				*models;
	GSM_Reply_Function		*ReplyFunctions;
	GSM_Error (*Initialise) 	(GSM_StateMachine *s);
	GSM_Error (*Terminate)  	(GSM_StateMachine *s);
	GSM_Error (*DispatchMessage)	(GSM_StateMachine *s);
	GSM_Error (*GetModel)		(GSM_StateMachine *s);
	GSM_Error (*GetFirmware)	(GSM_StateMachine *s);
	GSM_Error (*GetIMEI)            (GSM_StateMachine *s);
	GSM_Error (*GetDateTime)	(GSM_StateMachine *s, GSM_DateTime	    *date_time	);
	GSM_Error (*GetAlarm)		(GSM_StateMachine *s, GSM_DateTime	    *alarm,	int	alarm_number    );
	GSM_Error (*GetMemory)		(GSM_StateMachine *s, GSM_PhonebookEntry    *entry	);
	GSM_Error (*GetMemoryStatus)	(GSM_StateMachine *s, GSM_MemoryStatus	    *status	);
	GSM_Error (*GetSMSC)		(GSM_StateMachine *s, GSM_SMSC		    *smsc	);
	GSM_Error (*GetSMS)		(GSM_StateMachine *s, GSM_MultiSMSMessage   *sms	);
	GSM_Error (*GetSMSFolders)	(GSM_StateMachine *s, GSM_SMSFolders	    *folders	);
	GSM_Error (*GetManufacturer)	(GSM_StateMachine *s);
	GSM_Error (*GetNextSMS)		(GSM_StateMachine *s, GSM_MultiSMSMessage   *sms,		bool	start		);
	GSM_Error (*GetSMSStatus)	(GSM_StateMachine *s, GSM_SMSMemoryStatus   *status	);
	GSM_Error (*SetIncomingSMS)	(GSM_StateMachine *s, bool		    enable	);
	GSM_Error (*GetNetworkInfo)	(GSM_StateMachine *s, GSM_NetworkInfo	    *netinfo	);
	GSM_Error (*Reset)		(GSM_StateMachine *s, bool		    hard	);
	GSM_Error (*DialVoice)		(GSM_StateMachine *s, char		    *Number,	GSM_CallShowNumber ShowNumber);
	GSM_Error (*AnswerCall)		(GSM_StateMachine *s, int ID, bool all);                     
	GSM_Error (*CancelCall)		(GSM_StateMachine *s, int ID, bool all);
	GSM_Error (*GetRingtone)	(GSM_StateMachine *s, GSM_Ringtone 	    *Ringtone,	bool 	PhoneRingtone);
	GSM_Error (*GetWAPBookmark)	(GSM_StateMachine *s, GSM_WAPBookmark	    *bookmark	);
	GSM_Error (*GetBitmap)		(GSM_StateMachine *s, GSM_Bitmap	    *Bitmap	);
	GSM_Error (*SetRingtone)	(GSM_StateMachine *s, GSM_Ringtone	    *Ringtone,	int 	*maxlength	);
	GSM_Error (*SetSMS)		(GSM_StateMachine *s, GSM_SMSMessage	    *sms	);
	GSM_Error (*SendSMS)		(GSM_StateMachine *s, GSM_SMSMessage	    *sms	);
	GSM_Error (*SetDateTime)	(GSM_StateMachine *s, GSM_DateTime	    *date_time	);
	GSM_Error (*SetAlarm)		(GSM_StateMachine *s, GSM_DateTime	    *alarm,	int	alarm_number	);
	GSM_Error (*SetBitmap)		(GSM_StateMachine *s, GSM_Bitmap	    *Bitmap	);
	GSM_Error (*SetMemory)		(GSM_StateMachine *s, GSM_PhonebookEntry    *entry	);
	GSM_Error (*DeleteSMS)		(GSM_StateMachine *s, GSM_SMSMessage	    *sms	);
	GSM_Error (*SetWAPBookmark)	(GSM_StateMachine *s, GSM_WAPBookmark	    *bookmark	);
	GSM_Error (*DeleteWAPBookmark)	(GSM_StateMachine *s, GSM_WAPBookmark	    *bookmark	);
	GSM_Error (*GetWAPSettings)	(GSM_StateMachine *s, GSM_MultiWAPSettings  *settings	);
	GSM_Error (*SetIncomingCB)	(GSM_StateMachine *s, bool		    enable	);
	GSM_Error (*SetSMSC)		(GSM_StateMachine *s, GSM_SMSC		    *smsc	);
	GSM_Error (*GetManufactureMonth)(GSM_StateMachine *s, char		    *value	);
	GSM_Error (*GetProductCode)	(GSM_StateMachine *s, char		    *value	);
	GSM_Error (*GetOriginalIMEI)	(GSM_StateMachine *s, char		    *value	);
	GSM_Error (*GetHardware)	(GSM_StateMachine *s, char		    *value	);
	GSM_Error (*GetPPM)		(GSM_StateMachine *s, char		    *value	);
	GSM_Error (*PressKey)		(GSM_StateMachine *s, GSM_KeyCode	    Key,	bool 	Press);
	GSM_Error (*GetToDo)		(GSM_StateMachine *s, GSM_ToDoEntry	    *ToDo,	bool	refresh);
	GSM_Error (*DeleteAllToDo)	(GSM_StateMachine *s);
	GSM_Error (*SetToDo)		(GSM_StateMachine *s, GSM_ToDoEntry	    *ToDo	);
	GSM_Error (*GetToDoStatus)	(GSM_StateMachine *s, GSM_ToDoStatus	    *status	);
	GSM_Error (*PlayTone)		(GSM_StateMachine *s, int 		    Herz, 	unsigned char Volume, bool start);
	GSM_Error (*EnterSecurityCode)	(GSM_StateMachine *s, GSM_SecurityCode 	    Code	);
	GSM_Error (*GetSecurityStatus)	(GSM_StateMachine *s, GSM_SecurityCodeType  *Status	);
	GSM_Error (*GetProfile)		(GSM_StateMachine *s, GSM_Profile	    *Profile	);
	GSM_Error (*GetRingtonesInfo)	(GSM_StateMachine *s, GSM_AllRingtonesInfo  *Info	);
	GSM_Error (*SetWAPSettings)	(GSM_StateMachine *s, GSM_MultiWAPSettings  *settings	);
	GSM_Error (*GetSpeedDial)	(GSM_StateMachine *s, GSM_SpeedDial 	    *Speed	);
	GSM_Error (*SetSpeedDial)	(GSM_StateMachine *s, GSM_SpeedDial 	    *Speed	);
	GSM_Error (*ResetPhoneSettings)	(GSM_StateMachine *s, GSM_ResetSettingsType Type	);
	GSM_Error (*SendDTMF)		(GSM_StateMachine *s, char		    *sequence	);
	GSM_Error (*GetDisplayStatus)	(GSM_StateMachine *s, GSM_DisplayFeatures   *features	);
	GSM_Error (*SetAutoNetworkLogin)(GSM_StateMachine *s);
	GSM_Error (*SetProfile)		(GSM_StateMachine *s, GSM_Profile	    *Profile	);
	GSM_Error (*GetSIMIMSI)		(GSM_StateMachine *s, char		    *IMSI	);
	GSM_Error (*SetIncomingCall)	(GSM_StateMachine *s, bool		    enable	);
	GSM_Error (*GetNextCalendar)	(GSM_StateMachine *s, GSM_CalendarEntry	    *Note,	bool	start		);
	GSM_Error (*DeleteCalendar)	(GSM_StateMachine *s, GSM_CalendarEntry     *Note	);
	GSM_Error (*AddCalendar)	(GSM_StateMachine *s, GSM_CalendarEntry	    *Note, 	bool Past);
	GSM_Error (*GetBatteryCharge)	(GSM_StateMachine *s, GSM_BatteryCharge     *bat	);
	GSM_Error (*GetSignalQuality)	(GSM_StateMachine *s, GSM_SignalQuality     *sig	);
 	GSM_Error (*GetCategory)	(GSM_StateMachine *s, GSM_Category	    *Category	);
 	GSM_Error (*GetCategoryStatus)	(GSM_StateMachine *s, GSM_CategoryStatus    *Status	);
	GSM_Error (*GetFMStation)	(GSM_StateMachine *s, GSM_FMStation	    *FMStation	);
	GSM_Error (*SetFMStation)	(GSM_StateMachine *s, GSM_FMStation	    *FMStation	);	GSM_Error (*ClearFMStations)	(GSM_StateMachine *s);
	GSM_Error (*SetIncomingUSSD)	(GSM_StateMachine *s, bool		    enable	);
	GSM_Error (*DeleteUserRingtones)(GSM_StateMachine *s);
	GSM_Error (*ShowStartInfo)	(GSM_StateMachine *s, bool 		    enable	);
	GSM_Error (*GetNextFileFolder)	(GSM_StateMachine *s, GSM_File		    *File, 	bool start);
	GSM_Error (*GetFilePart)	(GSM_StateMachine *s, GSM_File		    *File	);
	GSM_Error (*AddFilePart)	(GSM_StateMachine *s, GSM_File		    *File, 	int *Pos);
	GSM_Error (*GetFileSystemStatus)(GSM_StateMachine *s, GSM_FileSystemStatus  *Status	);
	GSM_Error (*DeleteFile)		(GSM_StateMachine *s, unsigned char *ID);
	GSM_Error (*AddFolder)		(GSM_StateMachine *s, GSM_File		    *File	);
	GSM_Error (*GetMMSSettings)	(GSM_StateMachine *s, GSM_MultiWAPSettings  *settings	);
	GSM_Error (*SetMMSSettings)	(GSM_StateMachine *s, GSM_MultiWAPSettings  *settings	);
	GSM_Error (*HoldCall)		(GSM_StateMachine *s, int ID);
	GSM_Error (*UnholdCall)		(GSM_StateMachine *s, int ID);
	GSM_Error (*ConferenceCall)	(GSM_StateMachine *s, int ID);
	GSM_Error (*SplitCall)		(GSM_StateMachine *s, int ID);
	GSM_Error (*TransferCall)	(GSM_StateMachine *s, int ID, bool next);
	GSM_Error (*SwitchCall)		(GSM_StateMachine *s, int ID, bool next);
	GSM_Error (*GetCallDivert)	(GSM_StateMachine *s, GSM_MultiCallDivert *divert);
	GSM_Error (*SetCallDivert)	(GSM_StateMachine *s, GSM_MultiCallDivert *divert);
	GSM_Error (*CancelAllDiverts)	(GSM_StateMachine *s);
	GSM_Error (*AddSMSFolder)	(GSM_StateMachine *s, unsigned char *name);
	GSM_Error (*DeleteSMSFolder)	(GSM_StateMachine *s, int ID);
	GSM_Error (*GetGPRSAccessPoint)	(GSM_StateMachine *s, GSM_GPRSAccessPoint   *point	);
	GSM_Error (*SetGPRSAccessPoint)	(GSM_StateMachine *s, GSM_GPRSAccessPoint   *point	);
	GSM_Error (*GetLocale)		(GSM_StateMachine *s, GSM_Locale	    *locale	);
	GSM_Error (*SetLocale)		(GSM_StateMachine *s, GSM_Locale	    *locale	);
	GSM_Error (*GetCalendarSettings)(GSM_StateMachine *s, GSM_CalendarSettings  *settings	);
	GSM_Error (*SetCalendarSettings)(GSM_StateMachine *s, GSM_CalendarSettings  *settings	);
} GSM_Phone_Functions;

	extern GSM_Phone_Functions NAUTOPhone;
#ifdef GSM_ENABLE_NOKIA6110
	extern GSM_Phone_Functions N6110Phone;
#endif
#ifdef GSM_ENABLE_NOKIA6510
	extern GSM_Phone_Functions N6510Phone;
#endif
#ifdef GSM_ENABLE_NOKIA7110
	extern GSM_Phone_Functions N7110Phone;
#endif
#ifdef GSM_ENABLE_NOKIA9210
	extern GSM_Phone_Functions N9210Phone;
#endif
#ifdef GSM_ENABLE_ATGEN
	extern GSM_Phone_Functions ATGENPhone;
#endif
#ifdef GSM_ENABLE_ALCATEL
	extern GSM_Phone_Functions ALCATELPhone;
#endif
#ifdef GSM_ENABLE_OBEXGEN
	extern GSM_Phone_Functions OBEXGENPhone;
#endif

typedef struct {
	GSM_Phone_Data		 Data;
	GSM_Phone_Functions	*Functions;
} GSM_Phone;

/* --------------------------- User layer ---------------------------------- */

struct _GSM_User {
	GSM_Reply_Function		*UserReplyFunctions;

	void	  (*IncomingCall)	(char *Device, GSM_Call	      call);
	void 	  (*IncomingSMS)	(char *Device, GSM_SMSMessage sms);
	void 	  (*IncomingCB)		(char *Device, GSM_CBMessage  cb);
	void 	  (*IncomingUSSD)	(char *Device, char	      *Text);
	void 	  (*SendSMSStatus)	(char *Device, int 	      status);
};

/* --------------------------- Statemachine layer -------------------------- */

typedef enum {
	GCT_MBUS2=1,
	GCT_FBUS2,
	GCT_FBUS2DLR3,
	GCT_FBUS2BLUE,
	GCT_FBUS2IRDA,
	GCT_PHONETBLUE,
	GCT_AT,

	GCT_IRDAOBEX,
	GCT_IRDAAT,
	GCT_IRDAPHONET,
	GCT_BLUEFBUS2,
	GCT_BLUEAT,
	GCT_BLUEPHONET,
	GCT_BLUEOBEX
} GSM_ConnectionType;

typedef struct {
	/* Config file (or Registry or...) variables */
	char			Model[50];	   /* Model from config file 		*/
	char			DebugLevel[50];    /* Debug level			*/
	char 			*Device;	   /* Device name from config file 	*/
	char			*Connection;	   /* Connection type as string		*/
	char			*SyncTime;	   /* Synchronize time on startup? 	*/
	char			*LockDevice;	   /* Lock device ? (Unix)		*/
	char			*DebugFile;        /* Name of debug file		*/
	char 			*Localize;	   /* Name of localisation file		*/
	char			*StartInfo;	   /* Display something during start ?  */
	bool			UseGlobalDebugFile;/* Should we use global debug file?	*/
	bool			DefaultModel;
	bool			DefaultDebugLevel;
	bool			DefaultDevice;
	bool			DefaultConnection;
	bool			DefaultSyncTime;
	bool			DefaultLockDevice;
	bool			DefaultDebugFile;
	bool			DefaultLocalize;
	bool			DefaultStartInfo;
} GSM_Config;

struct _GSM_StateMachine {
	GSM_ConnectionType 	ConnectionType;				/* Type of connection as int			*/
	char			*LockFile;				/* Lock file name for Unix 			*/
	Debug_Info		di;					/* Debug information				*/
	bool			opened;					/* Is connection opened ?			*/
	GSM_Config		Config[5];
	GSM_Config		*CurrentConfig;				/* Config file (or Registry or...) variables 	*/
	int			ConfigNum;
	CFG_Header 		*msg;					/* Localisation strings structure    		*/
	int			ReplyNum;				/* How many times make sth. 			*/
	int			Speed;					/* For some protocols used speed		*/

	GSM_Device		Device;
	GSM_Protocol		Protocol;
	GSM_Phone		Phone;
	GSM_User		User;
};

/* ------------------------ Other general definitions ---------------------- */

GSM_Error GSM_RegisterAllPhoneModules	(GSM_StateMachine *s);

GSM_Error GSM_InitConnection		(GSM_StateMachine *s, int ReplyNum);
GSM_Error GSM_TerminateConnection	(GSM_StateMachine *s);

int 	  GSM_ReadDevice		(GSM_StateMachine *s, bool wait);

GSM_Error GSM_WaitForOnce		(GSM_StateMachine *s, unsigned char *buffer,
			  		 int length, unsigned char type, int time);

GSM_Error GSM_WaitFor			(GSM_StateMachine *s, unsigned char *buffer,
		       			 int length, unsigned char type, int time,
					 GSM_Phone_RequestID request);

GSM_Error GSM_DispatchMessage		(GSM_StateMachine *s);

CFG_Header 				*CFG_FindGammuRC();

bool 	  CFG_ReadConfig		(CFG_Header *cfg_info, GSM_Config *cfg, int num);

void 	  GSM_DumpMessageLevel2		(GSM_StateMachine *s, unsigned char *message, int messagesize, int type);
void 	  GSM_DumpMessageLevel3		(GSM_StateMachine *s, unsigned char *message, int messagesize, int type);

/* ---------------------- Phone features ----------------------------------- */

typedef enum {
	/* n6110.c */
	F_CAL33 = 1,	/* Calendar,3310 style - 10 reminders, Unicode, 3 coding types	*/
	F_CAL52,	/* Calendar,5210 style - full Unicode, etc.			*/
	F_CAL82,	/* Calendar,8250 style - "normal", but with Unicode		*/
	F_RING_SM,	/* Ringtones returned in SM format - 33xx			*/
	F_NORING,	/* No ringtones							*/
	F_NOPBKUNICODE,	/* No phonebook in Unicode					*/
	F_NOWAP,      	/* No WAP							*/
	F_NOCALLER,	/* No caller groups						*/
	F_NOPICTURE,	/* No Picture Images						*/
	F_NOPICTUREUNI,	/* No Picture Images text in Unicode				*/
	F_NOSTARTUP,	/* No startup logo						*/
	F_NOCALENDAR,	/* No calendar							*/
	F_NOSTARTANI,	/* Startup logo is not animated 				*/
	F_POWER_BATT,	/* Network and battery level get from netmonitor		*/
	F_PROFILES33,	/* Phone profiles in 3310 style					*/
	F_PROFILES51,	/* Phone profiles in 5110 style					*/
	F_MAGICBYTES,	/* Phone can make authentication with magic bytes		*/
	F_NODTMF,	/* Phone can't send DTMF					*/
	F_DISPSTATUS,	/* Phone return display status					*/
	F_NOCALLINFO,

	/* n6510.c */
	F_PBK35,	/* Phonebook in 3510 style with ringtones ID			*/
	F_RADIO,	/* Phone with FM radio						*/
	F_TODO63,	/* ToDo in 6310 style - 0x55 msg type				*/
	F_TODO66,	/* ToDo in 6610 style - like calendar, with date and other	*/
	F_NOMIDI,	/* No ringtones in MIDI						*/
	F_BLUETOOTH,	/* Bluetooth support						*/
	F_NOFILESYSTEM,	/* No images, ringtones, java saved in special filesystem	*/
	F_NOMMS,	/* No MMS sets in phone						*/
	F_NOGPRSPOINT,	/* GPRS point are not useable					*/
	F_CAL35,	/* Calendar,3510 style - Reminder,Call,Birthday			*/
	F_CAL65,	/* Calendar,6510 style - CBMM, method 3				*/

	/* n6510.c && n7110.c */
	F_VOICETAGS,	/* Voice tags available						*/
	F_CAL62,	/* Calendar,6210 style - Call,Birthday,Memo,Meeting		*/

	/* AT modules */
	F_SMSONLYSENT,	/* Phone supports only sent/unsent messages			*/
	F_BROKENCPBS, 	/* CPBS on some memories can hang phone				*/
	F_M20SMS	/* Siemens M20 like SMS handling				*/
} Feature;

/* For models table */
struct _OnePhoneModel {
	char		*model;
	char		*number;
	char		*irdamodel;
	int		features[12];
};

bool 		IsPhoneFeatureAvailable	(OnePhoneModel *model, int feature);
OnePhoneModel 	*GetModelData		(char *model, char *number, char *irdamodel);

int smprintf(GSM_StateMachine *s, const char *format, ...);

void GSM_OSErrorInfo(GSM_StateMachine *s, char *description);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

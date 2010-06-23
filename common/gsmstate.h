
#ifndef gsm_statemachine_h
#define gsm_statemachine_h

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

#ifndef GSM_USED_FBUS2
#  undef GSM_ENABLE_FBUS2
#endif
#ifndef GSM_USED_MBUS2
#  undef GSM_ENABLE_MBUS2
#endif
#ifndef GSM_USED_DLR3AT
#  undef GSM_ENABLE_DLR3AT
#endif
#ifndef GSM_USED_IRDA
#  undef GSM_ENABLE_IRDA
#endif
#ifndef GSM_USED_INFRARED
#  undef GSM_ENABLE_INFRARED
#endif
#ifndef GSM_USED_AT
#  undef GSM_ENABLE_AT
#endif
#ifndef GSM_USED_ATBLUETOOTH
#  undef GSM_ENABLE_ATBLUETOOTH
#endif
#ifndef GSM_USED_DLR3BLUETOOTH
#  undef GSM_ENABLE_DLR3BLUETOOTH
#endif
#ifndef GSM_USED_ALCABUS
#  undef GSM_ENABLE_ALCABUS
#endif

#if defined(GSM_ENABLE_NOKIA6110) || defined(GSM_ENABLE_NOKIA7110) || defined(GSM_ENABLE_NOKIA9210)
#  define GSM_ENABLE_NOKIA_DCT3
#endif
#if defined(GSM_ENABLE_NOKIA6510)
#  define GSM_ENABLE_NOKIA_DCT4
#endif

#include "protocol/protocol.h"
#if defined(GSM_ENABLE_FBUS2) || defined(GSM_ENABLE_INFRARED) || defined(GSM_ENABLE_DLR3AT) || defined(GSM_ENABLE_DLR3BLUETOOTH)
#  include "protocol/nokia/fbus2.h"
#endif
#ifdef GSM_ENABLE_MBUS2
#  include "protocol/nokia/mbus2.h"
#endif
#ifdef GSM_ENABLE_IRDA
#  include "protocol/nokia/fbusirda.h"
#endif
#if defined(GSM_ENABLE_AT) || defined(GSM_ENABLE_ATBLUETOOTH)
#  include "protocol/at/at.h"
#endif
#ifdef GSM_ENABLE_ALCABUS
#  include "protocol/alcatel/alcabus.h"
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
#  undef GSM_ENABLE_IRDA
#  undef GSM_ENABLE_BLUETOOTHDEVICE
#  undef GSM_ENABLE_ATBLUETOOTH
#  undef GSM_ENABLE_DLR3BLUETOOTH
#endif
#ifdef WIN32
#  undef GSM_ENABLE_BLUETOOTHDEVICE
#  undef GSM_ENABLE_ATBLUETOOTH
#  undef GSM_ENABLE_DLR3BLUETOOTH
#endif

#include "device/serial/win32.h"
#include "device/serial/unix.h"
#include "device/serial/djgpp.h"
#include "device/irda/irda.h"
#include "device/bluetoth/unixblue.h"

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

#if defined(GSM_ENABLE_FBUS2) || defined(GSM_ENABLE_INFRARED) || defined(GSM_ENABLE_DLR3AT) || defined(GSM_ENABLE_DLR3BLUETOOTH)
	extern GSM_Protocol_Functions FBUS2Protocol;
#endif
#ifdef GSM_ENABLE_MBUS2
	extern GSM_Protocol_Functions MBUS2Protocol;
#endif
#ifdef GSM_ENABLE_IRDA
	extern GSM_Protocol_Functions FBUS2IRDAProtocol;
#endif
#if defined(GSM_ENABLE_AT) || defined(GSM_ENABLE_ATBLUETOOTH)
	extern GSM_Protocol_Functions ATProtocol;
#endif
#ifdef GSM_ENABLE_ALCABUS
	extern GSM_Protocol_Functions ALCABUSProtocol;
#endif

typedef struct {
	union {
		char fake;
#ifdef GSM_ENABLE_MBUS2
		GSM_Protocol_MBUS2Data		MBUS2;
#endif
#if defined(GSM_ENABLE_FBUS2) || defined(GSM_ENABLE_INFRARED) || defined(GSM_ENABLE_DLR3AT) || defined(GSM_ENABLE_DLR3BLUETOOTH)
		GSM_Protocol_FBUS2Data		FBUS2;
#endif
#ifdef GSM_ENABLE_IRDA
		GSM_Protocol_FBUS2IRDAData	FBUS2IRDA;
#endif
#if defined(GSM_ENABLE_AT) || defined(GSM_ENABLE_ATBLUETOOTH)
		GSM_Protocol_ATData		AT;
#endif
#ifdef GSM_ENABLE_ALCABUS
		GSM_Protocol_ALCABUSData	ALCABUS;
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
	ID_GetNetworkLevel,
	ID_GetBatteryLevel,
	ID_GetSMSFolders,
	ID_GetSMSFolderStatus,
	ID_GetSMSStatus,
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
	ID_SetSMSC,
	ID_SetProfile,
	ID_SetMemory,
	ID_DeleteMemory,
	ID_SetCalendarNote,
	ID_SetIncomingSMS,
	ID_SetIncomingCB,
	ID_GetCalendarNotePos,
	ID_GetWAPSettings,
	ID_SetWAPBookmark,
	ID_DeleteWAPBookmark,
	ID_Netmonitor,
	ID_GetManufactureMonth,
	ID_GetProductCode,
	ID_GetOriginalIMEI,
	ID_GetHardware,
	ID_GetPPM,
	ID_GetSMSMode,
	ID_GetManufacturer,
	ID_SetMemoryType,
	ID_SetMemoryCharset,
	ID_SetSMSParameters,
	ID_Reset,
	ID_GetToDo,
	ID_PressKey,
	ID_DeleteAllToDo,
	ID_SetToDo,
	ID_PlayTone,
	ID_GetSecurityStatus,
	ID_EnterSecurityCode,
	ID_GetProfile,
	ID_GetRingtonesInfo,
#ifdef GSM_ENABLE_6110_AUTHENTICATION
	ID_MakeAuthentication,
#endif
	ID_GetSpeedDial,
	ID_ResetPhoneSettings,
	ID_SendDTMF,
	ID_GetDisplayStatus,
	ID_SetAutoNetworkLogin,
	ID_SetWAPSettings,
	ID_GetSIMIMSI,

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
	char			*IMEI;
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
	int			*NetworkLevel;
	int			*BatteryLevel;
	GSM_NetworkInfo		*NetworkInfo;
	GSM_Ringtone		*Ringtone;
	GSM_CalendarNote	*Calendar;
	char			*SecurityCode;
	GSM_WAPBookmark		*WAPBookmark;
	GSM_MultiWAPSettings	*WAPSettings;
	GSM_Bitmap		*Bitmap;
	char			*Netmonitor;
	GSM_TODO		*ToDo;
	bool			PressKey;
	GSM_SecurityCodeType	*SecurityStatus;
	GSM_Profile		*Profile;
	GSM_AllRingtonesInfo	*RingtonesInfo;
	GSM_DisplayFeatures	*DisplayFeatures;

	char			*PhoneString;
	int			StartPhoneString;

	bool			EnableIncomingSMS;	/* notify about incoming sms ? 	*/
	bool			EnableIncomingCB;	/* notify about incoming cb ?	*/
	char			*Model;			/* model codename string	*/
	char			*Version;		/* version of firmware		*/
	char			*VersionDate;		/* version date			*/
	double			*VersionNum;		/* firmware version as number	*/
	char			*Device;		/* device name			*/
	GSM_Protocol_Message	*RequestMsg;		/* last frame from phone	*/
	int			RequestID;		/* what operation is done now	*/
	GSM_Error		DispatchError;		/* error returned by function	*/
							/* in phone module		*/

	/* Some modules can cache these variables */
	char			IMEICache[50];		/* IMEI				*/
	char			HardwareCache[50];	/* Hardware version		*/
	char			ProductCodeCache[50];	/* Product code version		*/

	union {
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
	} Priv;
} GSM_Phone_Data;

typedef struct {
	GSM_Error (*Function)	(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User);
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
	GSM_Error (*GetIMEI)            (GSM_StateMachine *s, unsigned char	    *imei	);
	GSM_Error (*GetDateTime)	(GSM_StateMachine *s, GSM_DateTime	    *date_time	);
	GSM_Error (*GetAlarm)		(GSM_StateMachine *s, GSM_DateTime	    *alarm,	int	alarm_number    );
	GSM_Error (*GetMemory)		(GSM_StateMachine *s, GSM_PhonebookEntry    *entry	);
	GSM_Error (*GetMemoryStatus)	(GSM_StateMachine *s, GSM_MemoryStatus	    *status	);
	GSM_Error (*GetSMSC)		(GSM_StateMachine *s, GSM_SMSC		    *smsc	);
	GSM_Error (*GetSMSMessage)	(GSM_StateMachine *s, GSM_MultiSMSMessage   *sms	);
	GSM_Error (*GetBatteryLevel)	(GSM_StateMachine *s, int		    *level	);
	GSM_Error (*GetNetworkLevel)	(GSM_StateMachine *s, int		    *level	);
	GSM_Error (*GetSMSFolders)	(GSM_StateMachine *s, GSM_SMSFolders	    *folders	);
	GSM_Error (*GetManufacturer)	(GSM_StateMachine *s, char		    *manufacturer);
	GSM_Error (*GetNextSMSMessage)	(GSM_StateMachine *s, GSM_MultiSMSMessage   *sms,		bool	start		);
	GSM_Error (*GetSMSStatus)	(GSM_StateMachine *s, GSM_SMSMemoryStatus   *status	);
	GSM_Error (*SetIncomingSMS)	(GSM_StateMachine *s, bool		    enable	);
	GSM_Error (*GetNetworkInfo)	(GSM_StateMachine *s, GSM_NetworkInfo	    *netinfo	);
	GSM_Error (*Reset)		(GSM_StateMachine *s, bool		    hard	);
	GSM_Error (*DialVoice)		(GSM_StateMachine *s, char		    *Number	);
	GSM_Error (*AnswerCall)		(GSM_StateMachine *s);                     
	GSM_Error (*CancelCall)		(GSM_StateMachine *s);
	GSM_Error (*GetRingtone)	(GSM_StateMachine *s, GSM_Ringtone 	    *Ringtone,	bool 	PhoneRingtone);
	GSM_Error (*GetCalendarNote)	(GSM_StateMachine *s, GSM_CalendarNote	    *Note,	bool	start		);
	GSM_Error (*GetWAPBookmark)	(GSM_StateMachine *s, GSM_WAPBookmark	    *bookmark	);
	GSM_Error (*GetBitmap)		(GSM_StateMachine *s, GSM_Bitmap	    *Bitmap	);
	GSM_Error (*SetRingtone)	(GSM_StateMachine *s, GSM_Ringtone	    *Ringtone,	int 	*maxlength	);
	GSM_Error (*SaveSMSMessage)	(GSM_StateMachine *s, GSM_SMSMessage	    *sms	);
	GSM_Error (*SendSMSMessage)	(GSM_StateMachine *s, GSM_SMSMessage	    *sms	);
	GSM_Error (*SetDateTime)	(GSM_StateMachine *s, GSM_DateTime	    *date_time	);
	GSM_Error (*SetAlarm)		(GSM_StateMachine *s, GSM_DateTime	    *alarm,	int	alarm_number	);
	GSM_Error (*SetBitmap)		(GSM_StateMachine *s, GSM_Bitmap	    *Bitmap	);
	GSM_Error (*SetMemory)		(GSM_StateMachine *s, GSM_PhonebookEntry    *entry	);
	GSM_Error (*DeleteSMS)		(GSM_StateMachine *s, GSM_SMSMessage	    *sms	);
	GSM_Error (*DeleteCalendarNote)	(GSM_StateMachine *s, GSM_CalendarNote	    *Note	);
	GSM_Error (*SetCalendarNote)	(GSM_StateMachine *s, GSM_CalendarNote	    *Note	);
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
	GSM_Error (*GetToDo)		(GSM_StateMachine *s, GSM_TODO		    *ToDo,	bool	refresh);
	GSM_Error (*DeleteAllToDo)	(GSM_StateMachine *s);
	GSM_Error (*SetToDo)		(GSM_StateMachine *s, GSM_TODO		    *ToDo	);
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
	void 	  (*SendSMSStatus)	(char *Device, int 	      status);
};

/* --------------------------- Statemachine layer -------------------------- */

typedef enum {
	GCT_FBUS2=1,
	GCT_MBUS2,
	GCT_INFRARED,
	GCT_DLR3AT,
	GCT_IRDA,
	GCT_AT19200,
	GCT_AT115200,
	GCT_ATBLUE,
	GCT_DLR3BLUE,
	GCT_ALCABUS
} GSM_ConnectionType;

struct _GSM_StateMachine {
	GSM_ConnectionType 	connectiontype;    /* Type of connection as int		*/
	char			*lockfile; 	   /* Lock file name for Unix 		*/
	Debug_Info		di;
	bool			opened;		   /* Is connection opened ?		*/
	char			Model[50];	   /* Real connected phone model 	*/
	char			Ver[50];	   /* Real connected phone version 	*/
	char			VerDate[50];	   /* Version date			*/
	double			VerNum;		   /* Phone version as number 		*/

	/* Config file (or Registry or...) variables */
	char			CFGModel[50];	   /* Model from config file 		*/
	char 			*CFGDevice;	   /* Device name from config file 	*/
	char			*CFGConnection;	   /* Connection type as string		*/
	char			*CFGSyncTime;	   /* Synchronize time on startup? 	*/
	char			*CFGLockDevice;	   /* Lock device ? (Unix)		*/
	char			*CFGDebugFile;     /* Name of debug file		*/
	char			CFGDebugLevel[50]; /* Debug level			*/
	char 			*CFGLocalize;	   /* Name of localisation file		*/

	CFG_Header 		*msg;		   /* Localisation strings structure    */

	int			ReplyNum;	   /* How many times make sth. 		*/

	GSM_Device		Device;
	GSM_Protocol		Protocol;
	GSM_Phone		Phone;
	GSM_User		User;
};

/* ------------------------ Other general definitions ---------------------- */

GSM_Error GSM_RegisterAllPhoneModules	(GSM_StateMachine *s);

GSM_Error GSM_InitConnection		(GSM_StateMachine *s, int ReplyNum);
GSM_Error GSM_TerminateConnection	(GSM_StateMachine *s);

int 	  GSM_ReadDevice		(GSM_StateMachine *s);

GSM_Error GSM_WaitForOnce		(GSM_StateMachine *s, unsigned char *buffer,
			  		 int length, unsigned char type, int time);

GSM_Error GSM_WaitFor			(GSM_StateMachine *s, unsigned char *buffer,
		       			 int length, unsigned char type, int time,
					 GSM_Phone_RequestID request);

GSM_Error GSM_DispatchMessage		(GSM_StateMachine *s);

CFG_Header 				*CFG_FindGammuRC();

void      CFG_ReadConfig		(CFG_Header *cfg_info, char **model, char **port,
					 char **connection, char **synchronizetime, char **debugfile,
					 char **debuglevel, char **lockdevice);

void 	  GSM_DumpMessageLevel2		(GSM_StateMachine *s, unsigned char *message, int messagesize, int type);
void 	  GSM_DumpMessageLevel3		(GSM_StateMachine *s, unsigned char *message, int messagesize, int type);

/* ---------------------- Phone features ----------------------------------- */

typedef enum {
	F_CAL33 = 1,	/* Calendar,3310 style - 10 reminders, Unicode, 3 coding types	*/
	F_CAL52,	/* Calendar,5210 style - full Unicode, etc.			*/
	F_CAL82,	/* Calendar,8250 style - "normal", but with Unicode		*/
	F_RING_SM,	/* Ringtones returned in SM format - 33xx			*/
	F_NORING,	/* No ringtones							*/
	F_NOPBKUNICODE,	/* No phonebook in Unicode					*/
	F_NOWAP,      	/* No WAP							*/
	F_NOCALLER,	/* No caller groups						*/
	F_NOPICTURE,	/* No Picture Images						*/
	F_NOSTARTUP,	/* No startup logo						*/
	F_NOCALENDAR,	/* No calendar							*/
	F_NOSTARTANI,	/* Startup logo is not animated 				*/
	F_POWER_BATT,	/* Network and battery level get from netmonitor		*/
	F_PROFILES33,	/* Phone profiles in 3310 style					*/
	F_PROFILES51,	/* Phone profiles in 5110 style					*/
	F_MAGICBYTES,	/* Phone can make authentication with magic bytes		*/
	F_DTMF,		/* Phone can send DTMF						*/
	F_DISPSTATUS	/* Phone return display status					*/
} Feature6110;

/* For models table */
typedef struct {
	char		*model;
	char		*number;
	char		*irdamodel;
	int		features[12];
} OnePhoneModel;

bool 		IsPhoneFeatureAvailable	(char *model, int feature);
OnePhoneModel 	*GetModelData		(char *model, char *number, char *irdamodel);

int smprintf(GSM_StateMachine *s, const char *format, ...);

void GSM_OSErrorInfo(GSM_StateMachine *s, char *description);

#endif


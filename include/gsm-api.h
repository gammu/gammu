/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.
	
  Header file for the various functions, definitions etc. used to implement
  the handset interface.  See gsm-api.c for more details.

*/

#ifndef __gsm_api_h
#define __gsm_api_h
 
#include "misc.h"
#include "gsm-common.h"
#include "data/rlp-common.h"
#include "devices/device.h"
#include "gsm-phonebook.h"
#include "gsm-networks.h"
#include "gsm-bitmaps.h"
#include "gsm-datetime.h"
#include "gsm-calendar.h"
#include "gsm-sms.h"
#include "gsm-ringtones.h"
#include "gsm-wap.h"

/* Undefined functions in fbus/mbus files */
extern GSM_Error Unimplemented(void);
#define UNIMPLEMENTED (void *) Unimplemented
extern GSM_Error NotSupported(void);
#define NOTSUPPORTED (void *) NotSupported

/* Define the structure used to hold pointers to the various API functions.
   This is in effect the master list of functions provided by the gnokii API.
   Modules containing the model specific code each contain one of these
   structures which is "filled in" with the corresponding function within the
   model specific code.  If a function is not supported or not implemented, a
   generic not implemented function is used to return a GE_NOTIMPLEMENTED
   error code. */

typedef struct {

  /* FIXME: comment this. */

  GSM_Error (*Initialise)( char *port_device, char *initlength,
                           GSM_ConnectionType connection,
                           void (*rlp_callback)(RLP_F96Frame *frame));

  void      (*DispatchMessage) (u16 MessageLength, u8 *MessageBuffer, u8 MessageType); 

  void      (*Terminate)(void);	

  void      (*KeepAlive)(void);
  
  GSM_Error (*GetMemoryLocation)( GSM_PhonebookEntry *entry );

  GSM_Error (*WritePhonebookLocation)( GSM_PhonebookEntry *entry );

  GSM_Error (*GetSpeedDial)( GSM_SpeedDial *entry);

  GSM_Error (*SetSpeedDial)( GSM_SpeedDial *entry);

  GSM_Error (*GetMemoryStatus)( GSM_MemoryStatus *Status);

  GSM_Error (*GetSMSStatus)( GSM_SMSStatus *Status);

  GSM_Error (*GetSMSCenter)( GSM_MessageCenter *MessageCenter );

  GSM_Error (*SetSMSCenter)( GSM_MessageCenter *MessageCenter );

  GSM_Error (*GetSMSMessage)( GSM_SMSMessage *Message );

  GSM_Error (*DeleteSMSMessage)( GSM_SMSMessage *Message );

  GSM_Error (*SendSMSMessage)( GSM_SMSMessage *Message);

  GSM_Error (*SaveSMSMessage)( GSM_SMSMessage *Message);

    /* If units is set to a valid GSM_RFUnits value, the code
       will return level in these units if it is able.  Otherwise
       value will be returned as GRF_Arbitary.  If phone doesn't
       support GetRFLevel, function returns GE_NOTSUPPORTED */
  GSM_Error (*GetRFLevel)( GSM_RFUnits *units, float *level );

    /* Works the same as GetRFLevel, except returns battery
       level if known. */
  GSM_Error (*GetBatteryLevel)( GSM_BatteryUnits *units, float *level );

  GSM_Error (*GetPowerSource)( GSM_PowerSource *source);

  GSM_Error (*GetDisplayStatus)( int *Status);

  GSM_Error (*EnterSecurityCode)( GSM_SecurityCode Code);

  GSM_Error (*GetSecurityCodeStatus)( int *Status );

  GSM_Error (*GetSecurityCode)( GSM_SecurityCode *Code);
  
  GSM_Error (*GetIMEI)( char *imei );

  GSM_Error (*GetRevision)( char *revision );

  GSM_Error (*GetModel)( char *model );

  GSM_Error (*GetDateTime)( GSM_DateTime *date_time);

  GSM_Error (*SetDateTime)( GSM_DateTime *date_time);

  GSM_Error (*GetAlarm)( int alarm_number, GSM_DateTime *date_time );

  GSM_Error (*SetAlarm)( int alarm_number, GSM_DateTime *date_time );

  GSM_Error (*DialVoice)( char *Number);

  GSM_Error (*DialData)( char *Number, char type, void (* callpassup)(char c));

  GSM_Error (*GetIncomingCallNr)( char *Number );

  GSM_Error (*GetNetworkInfo) ( GSM_NetworkInfo *NetworkInfo );

  GSM_Error (*GetCalendarNote) ( GSM_CalendarNote *CalendarNote);

  GSM_Error (*WriteCalendarNote) ( GSM_CalendarNote *CalendarNote);

  GSM_Error (*DeleteCalendarNote) ( GSM_CalendarNote *CalendarNote);

  GSM_Error (*NetMonitor) ( unsigned char mode, char *Screen );

  GSM_Error (*SendDTMF) ( char *String );

  GSM_Error (*GetBitmap) ( GSM_Bitmap *Bitmap );
  
  GSM_Error (*SetBitmap) ( GSM_Bitmap *Bitmap );

  GSM_Error (*SetRingtone) ( GSM_Ringtone *ringtone , int *maxlength);

  GSM_Error (*SetBinRingtone) ( GSM_BinRingtone *ringtone);
  
  GSM_Error (*GetBinRingtone) ( GSM_BinRingtone *ringtone);
  
  GSM_Error (*Reset) ( unsigned char type );

  GSM_Error (*GetProfile) ( GSM_Profile *Profile );

  GSM_Error (*SetProfile) ( GSM_Profile *Profile );
  
  bool      (*SendRLPFrame) ( RLP_F96Frame *frame, bool out_dtx );

  GSM_Error (*CancelCall) ();
     
  GSM_Error (*PressKey) (int key, int event);
  
  GSM_Error (*EnableDisplayOutput) ();
  
  GSM_Error (*DisableDisplayOutput) ();
 
  GSM_Error (*EnableCellBroadcast) ();

  GSM_Error (*DisableCellBroadcast) ();

  GSM_Error (*ReadCellBroadcast) ( GSM_CBMessage *Message );

  GSM_Error (*PlayTone) (int Herz, u8 Volume);

  GSM_Error (*GetProductProfileSetting) ( GSM_PPS *PPS);

  GSM_Error (*SetProductProfileSetting) ( GSM_PPS *PPS);

  GSM_Error (*GetOperatorName) ( GSM_Network *operator);  

  GSM_Error (*SetOperatorName) ( GSM_Network *operator);    

  GSM_Error (*GetVoiceMailbox) ( GSM_PhonebookEntry *entry);  

  GSM_Error (*PhoneTests) ();    

  GSM_Error (*SimlockInfo) ( GSM_AllSimlocks *siml);    

  GSM_Error (*GetCalendarNotesInfo) (GSM_NotesInfo *NotesInfo);

  GSM_Error (*GetSMSFolders) ( GSM_SMSFolders *folders);      

  GSM_Error (*ResetPhoneSettings) ();      

  GSM_Error (*GetWAPBookmark) ( GSM_WAPBookmark *bookmark);      

  GSM_Error (*SetWAPBookmark) ( GSM_WAPBookmark *bookmark);      

  GSM_Error (*GetWAPSettings) ( GSM_WAPSettings *settings);      

  GSM_Error (*CallDivert) ( GSM_CallDivert *cd );      

  GSM_Error (*AnswerCall) (char s);

  GSM_Error (*GetManufacturer)( char *manufacturer );

} GSM_Functions;

typedef struct {

  /* FIXME: comment this. */

  GSM_Error (*Initialise)( char *port_device, char *initlength,
                           GSM_ConnectionType connection,
                           void (*rlp_callback)(RLP_F96Frame *frame));

  int (*SendMessage) (u16 message_length, u8 message_type, u8 *buffer);

  int (*SendFrame)   (u16 message_length, u8 message_type, u8 *buffer); 

  int (*WritePhone)  (u16 length, u8 *buffer); 

  void (*Terminate)(void);	

  void (*StateMachine)(unsigned char rx_byte);

} GSM_Protocol;

/* This structure is provided to allow common information about the particular
   model to be looked up in a model independant way. Some of the values here
   define minimum and maximum levels for values retrieved by the various Get
   functions for example battery level. They are not defined as constants to
   allow model specific code to set them during initialisation */

typedef struct {
	 	
  char *FBUSModels;     /* Models covered by this type, pipe '|' delimited. */  
  char *MBUSModels;     /* Models covered by this type, pipe '|' delimited. */
  char *InfraredModels; /* Models covered by this type, pipe '|' delimited. */  
  char *DLR3Models;     /* Models covered by this type, pipe '|' delimited. */
  char *ATModels;       /* Models covered by this type, pipe '|' delimited. */
  char *IrdaModels;     /* Models covered by this type, pipe '|' delimited. */
  char *TekramModels;   /* Models covered by this type, pipe '|' delimited. */  
  char *FBUS3110Models; /* Models covered by this type, pipe '|' delimited. */  
  
/* Minimum and maximum levels for RF signal strength. Units are as per the
   setting of RFLevelUnits.  The setting of RFLevelUnits indicates the 
   default or "native" units used.  In the case of the 3110 and 6110 series
   these are arbitrary, ranging from 0 to 4. */

  float MaxRFLevel;
  float MinRFLevel;
  GSM_RFUnits RFLevelUnits;

/* Minimum and maximum levels for battery level. Again, units are as per the
   setting of GSM_BatteryLevelUnits.  The value that BatteryLevelUnits is set
   to indicates the "native" or default value that the phone supports.  In the
   case of the 3110 and 6110 series these are arbitrary, ranging from 0 to 4. */

  float MaxBatteryLevel;
  float MinBatteryLevel;
  GSM_BatteryUnits BatteryLevelUnits;

/* Information about date, time and alarm support. In case of alarm
   information we provide value for the number of alarms supported. */

  GSM_DateTimeSupport DateTimeSupport;
  GSM_DateTimeSupport AlarmSupport;
  int MaximumAlarms;
} GSM_Information;

void NULL_Terminate(void);
void NULL_KeepAlive();
bool NULL_WritePhone (u16 length, u8 *buffer);
void NULL_TX_DisplayMessage(u16 MessageLength, u8 *MessageBuffer);

GSM_Error NULL_WaitUntil (int time, GSM_Error *value);
GSM_Error NULL_SendMessageSequence (int time, GSM_Error *value,
               u16 message_length, u8 message_type, u8 *buffer);

GSM_ConnectionType GetConnectionTypeFromString(char *Connection);

bool GetMemoryTypeString(char *memorytext, GSM_MemoryType *type);
bool GetMemoryTypeID(char *memorytext, GSM_MemoryType *type);

/* Define these as externs so that app code can pick them up. */

extern bool *GSM_LinkOK;
extern GSM_Information *GSM_Info;
extern GSM_Functions *GSM;
extern GSM_Protocol *Protocol;

GSM_PhonebookEntry *CurrentPhonebookEntry;
GSM_Error          CurrentPhonebookError;

GSM_SpeedDial      *CurrentSpeedDialEntry;
GSM_Error          CurrentSpeedDialError;

unsigned char      Current_IMEI[GSM_MAX_IMEI_LENGTH];
unsigned char      Current_Revision[GSM_MAX_REVISION_LENGTH];
unsigned char      Current_Model[GSM_MAX_MODEL_LENGTH];

GSM_SMSMessage     *CurrentSMSMessage;
GSM_Error          CurrentSMSMessageError;
int                CurrentSMSPointer;

GSM_SMSFolders      *CurrentSMSFolders;
GSM_Error          CurrentSMSFoldersError;
int                CurrentSMSFoldersCount;

GSM_OneSMSFolder   CurrentSMSFolder;
GSM_Error          CurrentSMSFolderError;
int                CurrentSMSFolderID;

GSM_MemoryStatus   *CurrentMemoryStatus;
GSM_Error          CurrentMemoryStatusError;

GSM_NetworkInfo    *CurrentNetworkInfo;
GSM_Error          CurrentNetworkInfoError;

GSM_SMSStatus      *CurrentSMSStatus;
GSM_Error          CurrentSMSStatusError;

GSM_MessageCenter  *CurrentMessageCenter;
GSM_Error          CurrentMessageCenterError;

int                *CurrentSecurityCodeStatus;
GSM_Error          CurrentSecurityCodeError;
GSM_SecurityCode   *CurrentSecurityCode;

GSM_DateTime       *CurrentDateTime;
GSM_Error          CurrentDateTimeError;

GSM_Error          CurrentResetPhoneSettingsError;

GSM_DateTime       *CurrentAlarm;
GSM_Error          CurrentAlarmError;

GSM_CalendarNote   *CurrentCalendarNote;
GSM_Error          CurrentCalendarNoteError;

GSM_NotesInfo      CurrentCalendarNotesInfo,*CurrentCalendarNotesInfo2;
GSM_Error          CurrentCalendarNotesInfoError;

int                *CurrentFirstCalendarFreePos;
GSM_Error          CurrentFirstCalendarFreePosError;

GSM_Error          CurrentSetDateTimeError;
GSM_Error          CurrentSetAlarmError;

GSM_Error          CurrentEnableExtendedCommandsError;

int                CurrentRFLevel,
                   CurrentBatteryLevel,
                   CurrentPowerSource;

int                CurrentDisplayStatus;
GSM_Error          CurrentDisplayStatusError;

char               *CurrentNetmonitor;
GSM_Error          CurrentNetmonitorError;

GSM_Bitmap         *CurrentGetBitmap;
GSM_Error          CurrentGetBitmapError;

GSM_Error          CurrentSetBitmapError;

GSM_Error          CurrentSendDTMFError;

GSM_Profile        *CurrentProfile;
GSM_Error          CurrentProfileError;

GSM_Error          CurrentDisplayOutputError;

GSM_CBMessage      *CurrentCBMessage;
GSM_Error          CurrentCBError;

int                CurrentPressKeyEvent;
GSM_Error          CurrentPressKeyError;

GSM_Error          CurrentPlayToneError;

GSM_Error          CurrentDialVoiceError;

GSM_Error          CurrentGetOperatorNameError;
GSM_Network        *CurrentGetOperatorNameNetwork;
GSM_Error          CurrentSetOperatorNameError;

GSM_Error          CurrentGetIMEIError;

GSM_Error          CurrentGetHWError;

unsigned char      CurrentPPS[4];
GSM_Error          CurrentProductProfileSettingsError;

char               CurrentIncomingCall[20];

GSM_Error          CurrentBinRingtoneError;
GSM_BinRingtone    *CurrentGetBinRingtone;

GSM_Error          CurrentRingtoneError; 

GSM_Error          CurrentMagicError;

GSM_Error          CurrentSimlockInfoError;
GSM_AllSimlocks    *CurrentSimLock;

GSM_Error          CurrentGetWAPBookmarkError;
GSM_Error          CurrentSetWAPBookmarkError;
GSM_WAPBookmark    *WAPBookmark;

GSM_Error          CurrentGetWAPSettingsError;
GSM_WAPSettings    *WAPSettings;

GSM_Error          CurrentCallDivertError;
GSM_CallDivert    *CurrentCallDivert;

char              *CurrentManufacturer;

/* This is the connection type used in gnokii. */
GSM_ConnectionType CurrentConnectionType;

/* Pointer to a callback function used to return changes to a calls status */
/* This saves unreliable polling */
void (*CurrentCallPassup)(char c);

/* Pointer to callback function in user code to be called when RLP frames
   are received. */
void (*CurrentRLP_RXCallback)(RLP_F96Frame *frame);

/* Used to disconnect the call */
u8 CurrentCallSequenceNumber;

bool CurrentLinkOK;

bool CurrentRequestTerminate;

bool CurrentDisableKeepAlive;

/* Prototype for the functions actually provided by gsm-api.c. */

GSM_Error GSM_Initialise(char *model, char *device, char *initlength, GSM_ConnectionType connection, void (*rlp_callback)(RLP_F96Frame *frame), char* SynchronizeTime);

bool CheckModel (GSM_Information InfoToCheck, char *model, GSM_ConnectionType connection);

char *GetMygnokiiVersion();

typedef enum {
  F_CAL61=1,/*Calendar in 6110 style - 4 types,...*/
  F_CAL33,  /*Calendar in 3310 style - 10 reminders, Unicode, 3 coding types*/
  F_CAL71,  /*Calendar in 7110 style - 4 types, recurrance, etc.*/

  F_PBK33SIM,/*Phonebook in Unicode(3310 style).Name and number.No internal*/
  F_PBK33INT,/*Phonebook in Unicode(3310 style).Name and number.Internal available*/
  F_PBK71INT,/*Extended phonebook-multiple numbers,Unicode(7110 style).Internal available*/
  F_PBK61INT,/*Name and number,no Unicode(6110 style).Internal available*/

  F_DATA61, /*We can make data calls using gnokiid. Like in 6110*/
  F_DATA71, /*Datacalls using AT commands*/

  F_SMS,    /*SMS with Inbox, Outbox*/ /*10*/
  F_SMS71,  /*SMS with SMS folders like in 7110*/ 

  F_AUTH,   /*We can make authentication and have "Accessory connected" like in 6110*/
  F_NETMON, /*Netmonitor available*/ 
  F_CALER61,/*Caller groups in 6110 style - 5 groups with members*/
  F_KEYB,   /*We can press keys*/
  F_SMSCDEF,/*SMSC can have default recipient*/
  F_SPEED,  /*We can handle speed dials*/
  F_SCRSAV, /*We have screen savers*/
  F_DTMF,   /*We can handle DTMF sequences*/
  F_NOPOWER,/*We must get power/battery info from netmonitor*/ /*20*/

  F_STANIM, /*Startup logo animated or static*/ 
  F_STA   , /*Startup logo static*/
  F_STA62 , /*Startup logo static size 6210*/ 
  F_STA71 , /*Startup logo static size 7110*/

  F_PROF61, /*Profiles numbered like in N6110*/
  F_PROF51, /*Profiles numbered like in N5110*/
  F_PROF33, /*Profiles numbered like in N3310.Unicode names*/

  F_RING_SM,/*Binary ringtone in Smart Messaging*/
  F_RINGBIN,/*Binary ringtone in binary format*/

  F_WAP     /*WAP functions available*/ /*30*/
} feat_index;

typedef enum {
  FN_CALENDAR=0,
  FN_NETMONITOR,
  FN_CALLERGROUPS,
  FN_PHONEBOOK,
  FN_AUTHENTICATION,
  FN_DATACALLS,
  FN_KEYPRESS,
  FN_SMSCDEFAULT,
  FN_SPEEDDIAL,
  FN_SCREENSAVER,
  FN_DTMF, /*10*/
  FN_SMS,
  FN_NOPOWERFRAME,
  FN_STARTUP,
  FN_PROFILES,
  FN_RINGTONES,
  FN_WAP,
  FN_RINGNUMBER
} featnum_index;

/* For models table */
typedef struct {
  char *model;
  char *number;
  feat_index features[18];  
} OnePhoneModel;

extern char *GetModelName ();
int GetModelFeature (featnum_index num);

extern bool AppendLog(u8 *buffer, int length,bool format);
extern bool AppendLogText(u8 *buffer,bool format);

typedef struct {
  int                SIMPhonebookUsed;
  int                SIMPhonebookSize;
  GSM_PhonebookEntry SIMPhonebook[250];

  int                PhonePhonebookUsed;
  int                PhonePhonebookSize;
  GSM_PhonebookEntry PhonePhonebook[500];

  bool               CallerAvailable;
  GSM_Bitmap         CallerGroups[5];

  bool               SpeedAvailable;
  GSM_SpeedDial      SpeedDials[8];

  bool               OperatorLogoAvailable;
  GSM_Bitmap         OperatorLogo;

  bool               StartupLogoAvailable;
  GSM_Bitmap         StartupLogo;

  GSM_Bitmap         StartupText;
} GSM_Backup;

#endif	/* __gsm_api_h */

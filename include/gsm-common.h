/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.
	
  Header file for the definitions, enums etc. that are used by all models of
  handset.

*/

#ifndef __gsm_common_h
#define __gsm_common_h

#include "misc.h"

#define MYGNOKII

/* Maximum length of SMS center name */
#define GSM_MAX_SMS_CENTER_NAME_LENGTH	(20)

/* Limits of SMS messages. */
#define GSM_MAX_SMS_CENTER_LENGTH  (40)
#define GSM_MAX_RECIPIENT_LENGTH   (40)
#define GSM_MAX_SENDER_LENGTH      (40)
#define GSM_MAX_DESTINATION_LENGTH (40)
#define GSM_MAX_SMS_LENGTH         (160)
#define GSM_MAX_SMS_8_BIT_LENGTH   (140)
#define GSM_MAX_USER_DATA_HEADER_LENGTH (140)
/* The maximum length of an uncompressed concatenated short message is
   255 * 153 = 39015 default alphabet characters */
#define GSM_MAX_CONCATENATED_SMS_LENGTH	(39015)

#define GSM_MAX_CB_MESSAGE         (160)

/* Correct for phones in fbus-6110.c */
/* For other translation could be required */
#define PHONEKEY_1              0x01
#define PHONEKEY_2              0x02
#define PHONEKEY_3              0x03
#define PHONEKEY_4              0x04
#define PHONEKEY_5              0x05
#define PHONEKEY_6              0x06
#define PHONEKEY_7              0x07
#define PHONEKEY_8              0x08
#define PHONEKEY_9              0x09
#define PHONEKEY_0              0x0a
#define PHONEKEY_HASH           0x0b /* # */
#define PHONEKEY_ASTERISK       0x0c /* * */
#define PHONEKEY_POWER          0x0d
#define PHONEKEY_INCREASEVOLUME 0x10 /* doesn't available in some phones as separate button: ie. N5110 */
#define PHONEKEY_DECREASEVOLUME 0x11 /* doesn't available in some phones as separate button: ie. N5110 */
#define PHONEKEY_UP             0x17   
#define PHONEKEY_DOWN           0x18 
#define PHONEKEY_MENU           0x19
#define PHONEKEY_NAMES          0x1a /* doesn't available in some phone: ie. N5110 */
#define PHONEKEY_GREEN          0x0e /* in some phone ie. N5110 sometimes works identical to POWER */
#define PHONEKEY_RED            0x0f /* (c) key in some phone: ie. N5110 */

/* Correct for phones in fbus-6110.c */
/* For other translation could be required */
#define PRESSPHONEKEY   0x01
#define RELEASEPHONEKEY 0x02

/* Limits for IMEI, Revision and Model string storage. */
#define GSM_MAX_IMEI_LENGTH     (20)
#define GSM_MAX_REVISION_LENGTH (20)
#define GSM_MAX_MODEL_LENGTH    (10)

/* In 6210 test 57 there seems to be 75 chars */
#define NM_MAX_SCREEN_WIDTH 75

/* Power source types */
typedef enum {
  GPS_ACDC=1, /* AC/DC powered (charging) */
  GPS_BATTERY /* Internal battery */
} GSM_PowerSource;

/* Definition of security codes. */
typedef enum {
  GSCT_SecurityCode = 0x01, /* Security code. */
  GSCT_Pin,                 /* PIN. */
  GSCT_Pin2,                /* PIN 2. */
  GSCT_Puk,                 /* PUK. */
  GSCT_Puk2,                /* PUK 2. */
  GSCT_None                 /* Code not needed. */
} GSM_SecurityCodeType;

/* Security code definition. */
typedef struct {
  GSM_SecurityCodeType Type; /* Type of the code. */
  char Code[10];             /* Actual code. */
} GSM_SecurityCode;

/* Define an enum for specifying memory types for retrieving phonebook
   entries, SMS messages etc. This type is not mobile specific - the model
   code should take care of translation to mobile specific numbers - see 6110
   code.
   01/07/99:  Two letter codes follow GSM 07.07 release 6.2.0
*/
typedef enum {
  GMT_ME, /* Internal memory of the mobile equipment */
  GMT_SM, /* SIM card memory */
  GMT_FD, /* Fixed dial numbers */
  GMT_ON, /* Own numbers */
  GMT_EN, /* Emergency numbers */
  GMT_DC, /* Dialled numbers */
  GMT_RC, /* Received numbers */
  GMT_MC, /* Missed numbers */
  GMT_LD, /* Last dialed */
  GMT_MT, /* combined ME and SIM phonebook */
  GMT_TA, /* for compatibility only: TA=computer memory */
  GMT_CB, /* Currently selected memory */
  GMT_CG, /* Caller groups */
  GMT_XX = 0xff	/* Error code for unknown memory type (returned by fbus-xxxx functions. */
} GSM_MemoryType;

/* This define speed dialing entries. */
typedef struct {
  int Number;                /* Which number is used to dialing? */
  GSM_MemoryType MemoryType; /* Memory type of the number. */
  int Location;              /* Location of the number in MemoryType. */
} GSM_SpeedDial;

/* Define enums for Battery units. */
typedef enum {
  GBU_Arbitrary,
  GBU_Volts,
  GBU_Minutes,
  GBU_Percentage
} GSM_BatteryUnits;

/* This enum is used for display status. */

typedef enum {
  DS_Call_In_Progress, /* Call in progress. */
  DS_Unknown,          /* The meaning is unknown now :-( */
  DS_Unread_SMS,       /* There is Unread SMS. */
  DS_Voice_Call,       /* Voice call active. */
  DS_Fax_Call,         /* Fax call active. */
  DS_Data_Call,        /* Data call active. */
  DS_Keyboard_Lock,    /* Keyboard lock status. */
  DS_SMS_Storage_Full  /* Full SMS Memory. */
} DisplayStatusEntity;

/* Constants for Profiles. */

#define PROFILE_MESSAGE_NOTONE     0x00
#define PROFILE_MESSAGE_STANDARD   0x01
#define PROFILE_MESSAGE_SPECIAL    0x02
#define PROFILE_MESSAGE_BEEPONCE   0x03
#define PROFILE_MESSAGE_ASCENDING  0x04

#define PROFILE_WARNING_OFF  0xff
#define PROFILE_WARNING_ON   0x04

#define PROFILE_CALLALERT_RINGING       0x01
#define PROFILE_CALLALERT_BEEPONCE      0x02
#define PROFILE_CALLALERT_OFF           0x04
#define PROFILE_CALLALERT_RINGONCE      0x05
#define PROFILE_CALLALERT_ASCENDING     0x06
#define PROFILE_CALLALERT_CALLERGROUPS  0x07

#define PROFILE_KEYPAD_OFF     0xff
#define PROFILE_KEYPAD_LEVEL1  0x00
#define PROFILE_KEYPAD_LEVEL2  0x01
#define PROFILE_KEYPAD_LEVEL3  0x02
//in 5110 I had also once 0x03

#define PROFILE_VOLUME_LEVEL1  0x06
#define PROFILE_VOLUME_LEVEL2  0x07
#define PROFILE_VOLUME_LEVEL3  0x08
#define PROFILE_VOLUME_LEVEL4  0x09
#define PROFILE_VOLUME_LEVEL5  0x0a

#define PROFILE_CALLERGROUPS_ALL         0xff
#define PROFILE_CALLERGROUPS_FAMILY      0x01
#define PROFILE_CALLERGROUPS_VIP         0x02
#define PROFILE_CALLERGROUPS_FRIENDS     0x04
#define PROFILE_CALLERGROUPS_COLLEAGUES  0x08
#define PROFILE_CALLERGROUPS_OTHERS      0x10

#define PROFILE_VIBRATION_OFF 0x00
#define PROFILE_VIBRATION_ON  0x01
#define PROFILE_VIBRATION_FIRST  0x02

/* Structure to hold profile entries. */

typedef struct {
  int Number;          /* The number of the profile. */
  char Name[40];       /* The name of the profile. */
  int DefaultName;     /* 0-6, when default name is used, -1, when not */
  int KeypadTone;      /* Volumen level for keypad tones. */
  int Lights;          /* Lights on/off. */
  int CallAlert;       /* Incoming call alert. */
  int Ringtone;        /* Ringtone for incoming call alert. */
  int Volume;          /* Volume of the ringing. */
  int MessageTone;     /* The tone for message indication. */
  int WarningTone;     /* The tone for warning messages. */
  int Vibration;       /* Vibration? */
  int CallerGroups;    /* CallerGroups. */
  int ScreenSaver;     /* ScreenSaver */
  int AutomaticAnswer; /* Does the phone auto-answer incoming call? */
} GSM_Profile;

typedef enum {
  PPS_ALS=0,
  PPS_HRData,
  PPS_14400Data,
  PPS_LCDContrast,
  PPS_EFR,
  PPS_FR,
  PPS_HR,
  PPS_GamesMenu,   /* N3210 5.26 and higher */
  PPS_VibraMenu    /* N3210 */
} GSM_PPS_Settings;

typedef struct {
  GSM_PPS_Settings Name;
  bool bool_value;
  int int_value;
} GSM_PPS;

typedef struct {
    char data[10+1];//detailed info about this simlock
    bool enabled;   //is enabled or not ?
    int counter;    //how many times user tried to disable simlock using keypad
    bool factory;   //is factory simlock ?
} GSM_OneSimlock;

typedef struct {
    GSM_OneSimlock simlocks[4];
} GSM_AllSimlocks;

/* Data structures for the call divert */
typedef enum {
	GSM_CDV_Busy = 0x01,     /* Divert when busy */ 
	GSM_CDV_NoAnswer,        /* Divert when not answered */
	GSM_CDV_OutOfReach,      /* Divert when phone off or no coverage */
	GSM_CDV_AllTypes         /* Divert all calls without ringing */
} GSM_CDV_DivertTypes;

typedef enum {
	GSM_CDV_VoiceCalls = 0x01,
	GSM_CDV_FaxCalls,
	GSM_CDV_DataCalls,
	GSM_CDV_AllCalls
} GSM_CDV_CallTypes;

/* See GSM 02.82 for diverts types */
typedef enum {
	GSM_CDV_Disable  = 0x00,
	GSM_CDV_Enable   = 0x01,
	GSM_CDV_Query    = 0x02, /* Is concrete divert enabled ? */
	GSM_CDV_Register = 0x03, /* Sets divert */
	GSM_CDV_Erasure  = 0x04  /* Erase concrete divert */
} GSM_CDV_Opers;

typedef struct {
	GSM_CDV_DivertTypes DType;
	GSM_CDV_CallTypes   CType;
	GSM_CDV_Opers       Operation;
        char                Number[GSM_MAX_SENDER_LENGTH + 1];
	unsigned int        Timeout;
	bool		    Enabled;
} GSM_CallDivert;

/* Define standard GSM error/return code values. These codes are also used for
   some internal functions such as SIM read/write in the model specific code. */

typedef enum {
  GE_NONE = 0,              /* No error. */
  GE_DEVICEOPENFAILED,	    /* Couldn't open specified serial device. */
  GE_UNKNOWNMODEL,          /* Model specified isn't known/supported. */
  GE_NOTSUPPORTED,          /* We are sure, that function not supported by phone model */
  GE_NOLINK,                /* Couldn't establish link with phone. */
  GE_TIMEOUT,               /* Command timed out. */
  GE_TRYAGAIN,              /* Try again. */
  GE_INVALIDSECURITYCODE,   /* Invalid Security code. */
  GE_NOTIMPLEMENTED,        /* Command called isn't implemented in model. */
  GE_INVALIDSMSLOCATION,    /* Invalid SMS location. */
  GE_INVALIDPHBOOKLOCATION, /* Invalid phonebook location. */ /*10*/
  GE_INVALIDMEMORYTYPE,     /* Invalid type of memory. */
  GE_INVALIDSPEEDDIALLOCATION, /* Invalid speed dial location. */
  GE_INVALIDCALNOTELOCATION,/* Invalid calendar note location. */
  GE_INVALIDDATETIME,       /* Invalid date, time or alarm specification. */
  GE_EMPTYSMSLOCATION,      /* SMS location is empty. */
  GE_PHBOOKNAMETOOLONG,     /* Phonebook name is too long. */
  GE_PHBOOKNUMBERTOOLONG,   /* Phonebook number is too long. */
  GE_PHBOOKWRITEFAILED,     /* Phonebook write failed. */
  GE_SMSSENDOK,             /* SMS was send correctly. */
  GE_SMSSENDFAILED,         /* SMS send fail. */ /*20*/
  GE_SMSWAITING,            /* Waiting for the next part of SMS. */
  GE_SMSTOOLONG,            /* SMS message too long. */
  GE_NONEWCBRECEIVED,       /* Attempt to read CB when no new CB received */
  GE_INTERNALERROR,         /* Problem occured internal to model specific code. */
  GE_CANTOPENFILE,          /* Can't open file with bitmap/ringtone */
  GE_WRONGNUMBEROFCOLORS,   /* Wrong number of colors in specified bitmap file */
  GE_WRONGCOLORS,           /* Wrong colors in bitmap file */
  GE_INVALIDFILEFORMAT,     /* Invalid format of file */
  GE_SUBFORMATNOTSUPPORTED, /* Subformat of file not supported */
  GE_TOOSHORT,              /* Too short file, structure, etc. to read */ /*30*/
  GE_TOOLONG,               /* Too long file, structure, etc. to read */
  GE_INVALIDIMAGESIZE,      /* Invalid size of bitmap (in file, etc.) */
  GE_BUSY,                  /* Command is still being executed. */
  GE_UNKNOWN,               /* Unknown error - well better than nothing!! */ /*34*/
  GE_NOACCESS,
  GE_USERCANCELED,
  GE_MEMORYFULL,
  GE_INVALIDRINGLOCATION,
  GE_INSIDEBOOKMARKSMENU,
  GE_INVALIDBOOKMARKLOCATION,
  GE_INSIDESETTINGSMENU,
  GE_INVALIDSETTINGSLOCATION,
  GE_EMPTYSMSC,
  
  /* The following are here in anticipation of data call requirements. */

  GE_LINEBUSY,              /* Outgoing call requested reported line busy */
  GE_NOCARRIER,             /* No Carrier error during data call setup ? */
  GE_SMSSAVEFAILED,	    /* save sms failed */
  GE_SMSISINMEM,	    /* sms is already in memory */
  GE_BUSYSMSLOCATION	    /* there is a sms at the requested sms location */
} GSM_Error;

#endif	/* __gsm_common_h */

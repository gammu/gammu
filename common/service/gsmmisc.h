#ifndef __gsm_misc_h
#define __gsm_misc_h

typedef enum {
	GSM_RESET_PHONESETTINGS = 1,
	GSM_RESET_USERINTERFACE,
	GSM_RESET_USERINTERFACE_PHONESETTINGS,
	GSM_RESET_DEVICE,
	GSM_RESET_FULLFACTORY
} GSM_ResetSettingsType;

/* Definition of security codes. */
typedef enum {
	GSCT_SecurityCode = 0x01, /* Security code. 	*/
	GSCT_Pin,                 /* PIN. 		*/
	GSCT_Pin2,                /* PIN 2. 		*/
	GSCT_Puk,                 /* PUK. 		*/
	GSCT_Puk2,                /* PUK 2. 		*/
	GSCT_None                 /* Code not needed. 	*/
} GSM_SecurityCodeType;

/* Security code definition. */
typedef struct {
	GSM_SecurityCodeType 	Type; 		/* Type of the code. 	*/
	char 			Code[10];       /* Actual code. 	*/
} GSM_SecurityCode;

typedef enum {
	GSM_KEY_NONE = 0x00,
	GSM_KEY_1 = 0x01,
	GSM_KEY_2,
	GSM_KEY_3,
	GSM_KEY_4,
	GSM_KEY_5,
	GSM_KEY_6,
	GSM_KEY_7,
	GSM_KEY_8,
	GSM_KEY_9,
	GSM_KEY_0,
	GSM_KEY_HASH,		/* # */
	GSM_KEY_ASTERISK,	/* * */
	GSM_KEY_POWER,
	GSM_KEY_GREEN,		/* in some phone ie. N5110 sometimes works identical to POWER */
	GSM_KEY_RED,		/* (c) key in some phone: ie. N5110 */
	GSM_KEY_INCREASEVOLUME,	/* doesn't available in some phones as separate button: ie. N5110 */
	GSM_KEY_DECREASEVOLUME,	/* doesn't available in some phones as separate button: ie. N5110 */
	GSM_KEY_UP = 0x17,
	GSM_KEY_DOWN,
	GSM_KEY_MENU,
	GSM_KEY_NAMES		/* doesn't available in some phone: ie. N5110 */
} GSM_KeyCode;

typedef enum {
	GSM_CallActive = 1,
	GSM_UnreadSMS,
	GSM_VoiceCall,
	GSM_FaxCall,
	GSM_DataCall,
	GSM_KeypadLocked,
	GSM_SMSMemoryFull	/* blinking envelope */
} GSM_DisplayFeature;

typedef struct {
	int 			Number;
	GSM_DisplayFeature 	Feature[7];
} GSM_DisplayFeatures;

typedef struct {
    /* All these should be -1 when unknown */
    int 	SignalStrength; /* Signal strength in dBm */
    int 	SignalPercent;  /* Signal strength in percent */
    int 	BitErrorRate;   /* Bit error rate in percent */
} GSM_SignalQuality;

typedef enum {
    GSM_BatteryPowered = 1,
    GSM_BatteryConnected,
    GSM_BatteryNotConnected,
    GSM_PowerFault
} GSM_ChargeState;

typedef struct {
    int 		BatteryPercent; /* Signal strength in percent, -1 = unknown */
    GSM_ChargeState 	ChargeState;    /* Charge state */
} GSM_BatteryCharge;

#define GSM_MAX_CATEGORY_NAME_LENGTH 50

typedef enum {
    Category_ToDo,
    Category_Phonebook
} GSM_CategoryType;

typedef struct {
    GSM_CategoryType    Type;
    int                 Location;
    unsigned char       Name[(GSM_MAX_CATEGORY_NAME_LENGTH + 1)*2];
} GSM_Category;

typedef struct {
    GSM_CategoryType    Type;
    int                 Used;
} GSM_CategoryStatus;

#define	GSM_MAX_FMSTATION_LENGTH 12
#define	GSM_MAX_FM_STATION 	 20

typedef struct {
	int			Location;
        char 			StationName [(GSM_MAX_FMSTATION_LENGTH+1)*2];
	int			Frequency;
} GSM_FMStation;

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

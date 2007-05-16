/* (c) 2001-2005 by Marcin Wiacek, Walek and Michal Cihar */

#ifndef __gsm_misc_h
#define __gsm_misc_h

#include "../gsmcomon.h"

/**
 * Enum defines ID for various phone and SIM memories.
 * Phone modules can translate them to values specific for concrete models
 * Two letter codes (excluding VM and SL) are from GSM 07.07
 */
typedef enum {
	/**
	 * Internal memory of the mobile equipment
	 */
	MEM_ME=1,
	/**
	 * SIM card memory
	 */
	MEM_SM,
	/**
	 * Own numbers
	 */
	MEM_ON,
	/**
	 * Dialled calls
	 */
	MEM_DC,
	/**
	 * Received calls
	 */
	MEM_RC,
	/**
	 * Missed calls
	 */
	MEM_MC,
	/**
	 * Combined ME and SIM phonebook
	 */
	MEM_MT,
	/**
	 * Fixed dial
	 */
	MEM_FD,

	/**
	 * Voice mailbox
	 */
	MEM_VM,

	/**
	 * Sent SMS logs
	 */
	MEM_SL
} GSM_MemoryType;

/* --------------------------- resetting phone settings  ------------------- */

typedef enum {
	GSM_RESET_PHONESETTINGS = 1,
	GSM_RESET_USERINTERFACE,
	GSM_RESET_USERINTERFACE_PHONESETTINGS,
	GSM_RESET_DEVICE,
	GSM_RESET_FULLFACTORY
} GSM_ResetSettingsType;

/* --------------------------- security codes ------------------------------ */

/**
 * Definition of security codes.
 */
typedef enum {
	/**
	 * Security code.
	 */
	SEC_SecurityCode = 0x01,
	/**
	 * PIN.
	 */
	SEC_Pin,
	/**
	 * PIN 2.
	 */
	SEC_Pin2,
	/**
	 * PUK.
	 */
	SEC_Puk,
	/**
	 * PUK 2.
	 */
	SEC_Puk2,
	/**
	 * Code not needed.
	 */
	SEC_None
} GSM_SecurityCodeType;

#define GSM_SECURITY_CODE_LEN	15

/**
 * Security code definition.
 */
typedef struct {
	/**
	 * Actual code.
	 */
	char 			Code[GSM_SECURITY_CODE_LEN+1];
	/**
	 * Type of the code.
	 */
	GSM_SecurityCodeType 	Type;
} GSM_SecurityCode;

/* ---------------------------- keyboard ----------------------------------- */

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
	/**
	 * #
	 */
	GSM_KEY_HASH,
	/**
	 * *
	 */
	GSM_KEY_ASTERISK,
	GSM_KEY_POWER,
	/**
         * in some phone ie. N5110 sometimes works identical to POWER
	 */
	GSM_KEY_GREEN,
	/**
	 * (c) key in some phone: ie. N5110
	 */
	GSM_KEY_RED,
	/**
	 * doesn't available in some phones as separate button: ie. N5110
	 */
	GSM_KEY_INCREASEVOLUME,
	/**
	 * doesn't available in some phones as separate button: ie. N5110
	 */
	GSM_KEY_DECREASEVOLUME,
	GSM_KEY_UP = 0x17,
	GSM_KEY_DOWN,
	GSM_KEY_MENU,
	/**
	 * doesn't available in some phone: ie. N5110
	 */
	GSM_KEY_NAMES,
	/**
	 * Left arrow
	 */
	GSM_KEY_LEFT,
	/**
	 * Right arrow
	 */
	GSM_KEY_RIGHT,
	/**
	 * Software key which has assigned mening on display.
	 */
	GSM_KEY_SOFT1,
	/**
	 * Software key which has assigned mening on display.
	 */
	GSM_KEY_SOFT2,
	/**
	 * Button on headset
	 */
	GSM_KEY_HEADSET,
	/**
	 * Joystick pressed
	 */
	GSM_KEY_JOYSTICK,
	/**
	 * Camera button pressed
	 */
	GSM_KEY_CAMERA,
	/**
	 * Media player button
	 */
	GSM_KEY_MEDIA,
	/**
	 * Multi function key, desktop
	 */
	GSM_KEY_DESKTOP,
	/**
	 * Operator button
	 */
	GSM_KEY_OPERATOR,
	/**
	 * Return button
	 */
	GSM_KEY_RETURN,
	/**
	 * Clear button
	 */
	GSM_KEY_CLEAR,
} GSM_KeyCode;

GSM_Error MakeKeySequence(char *text, GSM_KeyCode *KeyCode, int *Length);

/* ------------------------------- display features ------------------------ */

typedef enum {
	GSM_CallActive = 1,
	/**
	 * blinking envelope
	 */
	GSM_SMSMemoryFull,
	GSM_FaxCall,
	GSM_UnreadSMS,
	GSM_DataCall,
	GSM_VoiceCall,
	GSM_KeypadLocked
} GSM_DisplayFeature;

typedef struct {
	int 			Number;
	GSM_DisplayFeature 	Feature[7];
} GSM_DisplayFeatures;

/**
 * Power source
 */
typedef enum {
	GSM_BatteryPowered = 1, /**< Powered from battery */
	GSM_BatteryConnected, /**< Powered from AC, battery connected */
	GSM_BatteryCharging, /**< Powered from AC, battery is charging */
	GSM_BatteryNotConnected, /**< Powered from AC, no battery */
	GSM_BatteryFull, /**< Powered from AC, battery is fully charged */
	GSM_PowerFault, /**< Power failure */
} GSM_ChargeState;

/**
 * Power source
 */
typedef enum {
	GSM_BatteryUnknown = 0, /**< Unknown battery */
	GSM_BatteryNiMH = 1, /**< NiMH battery */
	GSM_BatteryLiIon, /**< Lithium Ion battery */
	GSM_BatteryLiPol, /**< Lithium Polymer battery */
} GSM_BatteryType;

typedef struct {
	/**
	 * Signal strength in percent, -1 = unknown
	 */
	int BatteryPercent;
	/**
	 * Charge state
	 */
	GSM_ChargeState ChargeState;
	/**
	 * Current battery voltage (in mV).
	 */
	int BatteryVoltage;
	/**
	 * Voltage from charger (in mV)
	 */
	int ChargeVoltage;
	/**
	 * Current from charger (in mA)
	 */
	int ChargeCurrent;
	/**
	 * Phone current consumption (in mA)
	 */
	int PhoneCurrent;
	/**
	 * Battery temperature (in degrees Celsius)
	 */
	int BatteryTemperature;
	/**
	 * Phone temperature (in degrees Celsius)
	 */
	int PhoneTemperature;
	/**
	 * Remaining battery capacity (in mAh)
	 */
	int BatteryCapacity;
	/**
	 * Battery type
	 */
	GSM_BatteryType BatteryType;

} GSM_BatteryCharge;

/**
 * Resets all members of GSM_BatteryCharge structure.
 */
void GSM_ClearBatteryCharge(GSM_BatteryCharge *bat);

/* ------------------------------ categories ------------------------------- */

#define GSM_MAX_CATEGORY_NAME_LENGTH 60

typedef enum {
	Category_ToDo = 1,
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

/* ------------------- radio FM stations ---------------------------------- */

#define	GSM_MAX_FMSTATION_LENGTH 18
#define	GSM_MAX_FM_STATION 	 20

typedef struct {
	int			Location;
        char 			StationName [(GSM_MAX_FMSTATION_LENGTH+1)*2];
	double			Frequency;
} GSM_FMStation;

/* ----------------------- filesystem ------------------------------------- */

typedef enum {
	GSM_File_Java_JAR = 1,
	GSM_File_Image_JPG,
	GSM_File_Image_BMP,
	GSM_File_Image_GIF,
	GSM_File_Image_PNG,
	GSM_File_Image_WBMP,
        GSM_File_Video_3GP,
        GSM_File_Sound_AMR,
	GSM_File_Sound_NRT,		/* DCT4 binary format   */
        GSM_File_Sound_MIDI,
#ifdef DEVELOP
	GSM_File_MMS,
#endif
	GSM_File_Other
} GSM_FileType;

typedef struct {
	int		Used;		  /* how many bytes used */
	unsigned char 	Name[300];	  /* Name in Unicode	 */
	bool 		Folder;		  /* true, when folder 	 */
	int		Level;
	GSM_FileType	Type;
	unsigned char	ID_FullName[800]; /* ID in Unicode 	 */
	unsigned char	*Buffer;

	GSM_DateTime	Modified;
	bool		ModifiedEmpty;

	/* File attributes */
	bool		Protected;
	bool		ReadOnly;
	bool		Hidden;
	bool		System;
} GSM_File;

GSM_Error GSM_ReadFile(char *FileName, GSM_File *File);

GSM_Error GSM_JADFindData(GSM_File File, char *Vendor, char *Name, char *JAR, char *Version, int *Size);

void GSM_IdentifyFileFormat(GSM_File *File);

typedef struct {
	int		Free;
	int		Used;
	int		UsedImages;
	int		UsedSounds;
	int		UsedThemes;
} GSM_FileSystemStatus;

bool GSM_ReadHTTPFile(unsigned char *server, unsigned char *filename, GSM_File *file);

/* ----------------------------- GPRS access points ----------------------- */

typedef struct {
	int		Location;
	unsigned char 	Name[300];
	unsigned char 	URL[500];
	bool		Active;
} GSM_GPRSAccessPoint;

/* ------------------------------------------------------------------------ */

typedef enum {
	GSM_Date_DDMMYYYY = 1,
	GSM_Date_MMDDYYYY,
	GSM_Date_YYYYMMDD,
	GSM_Date_DDMMMYY,
	GSM_Date_MMDDYY,
	GSM_Date_DDMMYY,
	GSM_Date_YYMMDD,
	GSM_Date_OFF
} GSM_DateFormat;

typedef struct {
	unsigned char 	DateSeparator;
	GSM_DateFormat	DateFormat;
	bool		AMPMTime;
} GSM_Locale;

/* ------------------------------------------------------------------------ */

bool ReadVCALDateTime(const char *Buffer, GSM_DateTime *dt);
void SaveVCALDateTime(char *Buffer, int *Length, GSM_DateTime *Date, char *Start);

bool ReadVCALDate(char *Buffer, char *Start, GSM_DateTime *Date, bool *is_date_only);
void SaveVCALDate(char *Buffer, int *Length, GSM_DateTime *Date, char *Start);

/**
 * Stores text in vCalendar/vCard formatted buffer.
 *
 * @param Buffer Buffer where output will be stored.
 * @param Length Current position in output buffer (will be updated).
 * @param Text Value to be stored.
 * @param Start Name of field which is being stored.
 * @param UTF8 Whether text should be stored in UTF-8 without prefix.
 */
void SaveVCALText(char *Buffer, int *Length, unsigned char *Text, char *Start, bool UTF8);

/**
 * Reads text from vCalendar/vCard formatted buffer.
 *
 * @param Buffer Buffer where input is stored.
 * @param Start Name of field which is being read.
 * @param Value Storage for read value.
 * @param UTF8 Whether text should be read in UTF-8 without prefix.
 */
bool ReadVCALText(char *Buffer, char *Start, unsigned char *Value, bool UTF8);

bool ReadVCALInt(char *Buffer, char *Start, int *Value);

unsigned char *VCALGetTextPart(unsigned char *Buff, int *pos);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

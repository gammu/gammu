/* (c) 2001-2004 by Marcin Wiacek, Walek and Michal Cihar */

#ifndef __gsm_misc_h
#define __gsm_misc_h

#include "../gsmcomon.h"

/**
 * Enum defines ID for various phone and SIM memories.
 * Phone modules can translate them to values specific for concrete models
 * Two letter codes (excluding VM) are from GSM 07.07
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
	MEM_VM
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
	GSM_KEY_NAMES
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

/* ----------------------------- power source ------------------------------ */

typedef enum {
	GSM_BatteryPowered = 1,
	GSM_BatteryConnected,
	GSM_BatteryNotConnected,
	GSM_PowerFault
} GSM_ChargeState;

typedef struct {
	/**
	 * Signal strength in percent, -1 = unknown
	 */
	int BatteryPercent;
	/**
	 * Charge state
	 */
	GSM_ChargeState ChargeState;
} GSM_BatteryCharge;

/* ------------------------------ categories ------------------------------- */

#define GSM_MAX_CATEGORY_NAME_LENGTH 50

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

#define	GSM_MAX_FMSTATION_LENGTH 12
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
	int		Used;		/* how many bytes used 	*/
	unsigned char 	Name[300];	/* Name			*/
	bool 		Folder;		/* true, when folder 	*/
	int		Level;
	GSM_FileType	Type;
	unsigned char	ID_FullName[400];
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
} GSM_FileSystemStatus;

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

void ReadVCALDateTime(char *Buffer, GSM_DateTime *dt);
void SaveVCALDateTime(char *Buffer, int *Length, GSM_DateTime *Date, char *Start);

void SaveVCALText(char *Buffer, int *Length, char *Text, char *Start);
bool ReadVCALText(char *Buffer, char *Start, char *Value);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */

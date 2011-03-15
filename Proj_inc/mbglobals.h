// All Type 
#ifndef __gsm_globals_h
#define __gsm_globals_h

#include <stdio.h>
#ifndef WIN32
#  include <unistd.h>
#endif
#define MAX_MANUFACTURER_LENGTH		50
#define MAX_MODEL_LENGTH		100 

/* GSM_Error is used to notify about errors */
typedef enum {
// 1	
	ERR_NONE=1,
	ERR_DEVICEOPENERROR,		// Error during opening device
	ERR_DEVICELOCKED,		// Device locked
	ERR_DEVICENOTEXIST,
	ERR_DEVICEBUSY,
	ERR_DEVICENOPERMISSION,
	ERR_DEVICENODRIVER,
	ERR_DEVICENOTWORK,
	ERR_DEVICEDTRRTSERROR,		// Error during setting DTR/RTS in device
//10	
	ERR_DEVICECHANGESPEEDERROR,	// Error during changing speed in device
	ERR_DEVICEWRITEERROR,		// Error during writing device
	ERR_DEVICEREADERROR,		// Error during reading device
	ERR_DEVICEPARITYERROR,		// Can't set parity on device
	ERR_TIMEOUT,			// Command timed out
	ERR_FRAMENOTREQUESTED,		// Frame handled, but not requested in this moment
	ERR_UNKNOWNRESPONSE,		// Response not handled by MBdrv
	ERR_UNKNOWNFRAME,		// Frame not handled by MBdrv
	ERR_UNKNOWNCONNECTIONTYPESTRING,// Unknown connection type given by user
	ERR_UNKNOWNMODELSTRING,		// Unknown model given by user
//20	
	ERR_SOURCENOTAVAILABLE,		// Some functions not compiled in your OS
	ERR_NOTSUPPORTED,		// Not supported by phone
	ERR_EMPTY,			// Empty phonebook entry, ... 
	ERR_SECURITYERROR,		// Not allowed
	ERR_INVALIDLOCATION,		// Too high or too low location...
	ERR_NOTIMPLEMENTED,		// Function not implemented
	ERR_FULL,			// Memory is full
	ERR_UNKNOWN,
	ERR_CANTOPENFILE, 		// Error during opening file
	ERR_MOREMEMORY,			// More memory required
//30	
	ERR_PERMISSION,			// No permission
	ERR_EMPTYSMSC,			// SMSC number is empty
	ERR_INSIDEPHONEMENU,		// Inside phone menu - can't make something
	ERR_NOTCONNECTED,		// Phone NOT connected - can't make something
	ERR_WORKINPROGRESS,		// Work in progress
	ERR_PHONEOFF,			// Phone is disabled and connected to charger
	ERR_FILENOTSUPPORTED,		// File format not supported by MBdrv
	ERR_BUG,                  	// Found bug in implementation or phone
    ERR_CANCELED,                 	// Action was canceled by user
	ERR_NEEDANOTHERANSWER,   	// Inside MBdrv: phone module need to send another answer frame
//40
	ERR_OTHERCONNECTIONREQUIRED,
	ERR_WRONGCRC,
	ERR_INVALIDDATETIME,		// Invalid date/time
	ERR_MEMORY,			// Phone memory error, maybe it is read only
	ERR_INVALIDDATA,			// Invalid data
	ERR_NEEDANOTHEDATA,   	// Inside MBdrv: phone module need to send another answer frame
	ERR_OBEXMODE,
	ERR_NOSIMCARD,
	ERR_FILEALREADYEXIST,
	ERR_TRIAL_LIMITEDFUNCTION,

	ERR_UNSUPPORTEDCARD,
	ERR_UNSUPPORTEDREADER,	
	ERR_FILEIDNOTFOUND,
	ERR_PINPUKNOTINIT,
	ERR_SERVICEERROR, 
	ERR_MOBILENOTRESPONSE,

} GSM_Error;

/**
 * Coding type of SMS.
 */
typedef enum {
	// Unicode
	SMS_Coding_Unicode = 1,
	
	// Default GSM aplhabet.
	SMS_Coding_Default,

	// 8-bit.
	SMS_Coding_8bit
} GSM_Coding_Type;


/**
 * Enum defines ID for various phone and SIM memories.
 * Phone modules can translate them to values specific for concrete models
 * Two letter codes (excluding VM) are from GSM 07.07
 */
typedef enum {
	// Internal memory of the mobile equipment
	MEM_ERROR=0,
	MEM_ME=1,

	// SIM card memory
	MEM_SM,

	// Own numbers
	MEM_ON,

	// Dialled calls
	MEM_DC,

	// Received calls
	MEM_RC,

	// Missed calls
	MEM_MC,

	// Combined ME and SIM phonebook
	MEM_MT,

	// Fixed dial
	MEM_FD,

	// Voice mailbox
	MEM_VM,

	MEM7110_CG		 = 0xf0, /* Caller groups memory 		*/
	MEM7110_SP		 = 0xf1	 /* Speed dial memory 			*/
} GSM_MemoryType;
typedef struct {
	/**
	 * Number of used entries
	 */
	int	     	MemoryUsed;
	/**
	 * Memory type
	 */
	GSM_MemoryType  MemoryType;
	/**
	 * Number of free entries
	 */
	int	     	MemoryFree;
} GSM_MemoryStatus;

/**
 * Type of specific phonebook entry. In parenthesis is specified in which
 * member of @ref GSM_SubMemoryEntry value is stored.
 */
typedef enum {
	// General number. (Text)
	PBK_Number_General = 1,			//1

	// Mobile number. (Text)
	PBK_Number_Mobile,				//2

	// Work number. (Text)
	PBK_Number_Work,				//3

	// Fax number. (Text)
	PBK_Number_Fax,					//4

	// Home number. (Text)
	PBK_Number_Home,				//5

	// Pager number. (Text)
	PBK_Number_Pager,				//6

	// Other number. (Text)
	PBK_Number_Other,				//7

	// Note. (Text)
	PBK_Text_Note,					//8

	// Complete postal address. (Text)
	PBK_Text_Postal,				//9

	// Email. (Text)
	PBK_Text_Email,					//10

	// Second email. (Text)
	PBK_Text_Email2,				//11

	// URL (Text)
	PBK_Text_URL,					//12

	// Date and time. FIXME: describe better (Date)
	PBK_Date,						//13

	// Caller group. (Text)
	PBK_Caller_Group,				//14

	// Name (Text)
	PBK_Text_Name,					//15

	// Last name. (Text)
	PBK_Text_LastName,				//16

	// First name. (Text)
	PBK_Text_FirstName,				//17

	// Company. (Text)
	PBK_Text_Company,				//18

	// Job title. (Text)
	PBK_Text_JobTitle,				//19

	// Category. (Number)
	PBK_Category,					//20

	// Whether entry is private. (Number)
	PBK_Private,					//21

	// Street address. (Text)
	PBK_Text_StreetAddress,			//22

	// City. (Text)
	PBK_Text_City,					//23

	// State. (Text)
	PBK_Text_State,					//24

	// Zip code. (Text)
	PBK_Text_Zip,					//25

	// Country. (Text)
	PBK_Text_Country,				//26

	// Custom information 1. (Text)
	PBK_Text_Custom1,				//27

	// Custom information 2. (Text)
	PBK_Text_Custom2,				//28

	// Custom information 3. (Text)
	PBK_Text_Custom3,				//29

	// Custom information 4. (Text)
	PBK_Text_Custom4,				//30

	// Ringtone ID. (Number)
	PBK_RingtoneID,					//31

	// Ringtone ID in phone filesystem. (Number)
	PBK_RingtoneFileSystemID,		//32

	// Picture ID. (Number)
	PBK_PictureID,					//33
	PBK_SMSListID,					//34

	// User ID. (Text)
	PBK_Text_UserID,				//35

	// PictureName (Text)
	PBK_Text_Picture,				//36

	// RingName (Text)
	PBK_Text_Ring,					//37

	// Sex
	PBK_Number_Sex,					//38

	// Sex
	PBK_Number_Light,				//39
		// Caller group. (Text)

	//Compare : Push Talk. (TEXT) 
    PBK_Push_Talk,					//40


	PBK_Caller_Group_Text,			//41

	// Street address. (Text)
	PBK_Text_StreetAddress2,		//42

	//Nickname (Text)
	PBK_Text_Nickname,				//43

	PBK_Number_Mobile_Home,			//44
	PBK_Number_Mobile_Work,			//45
	PBK_Number_Fax_Home,			//46
	PBK_Number_Fax_Work,			//47
	PBK_Text_Email_Home,			//48
	PBK_Text_Email_Work,			//49
	PBK_Text_URL_Home,				//50
	PBK_Text_URL_Work,				//51
	PBK_Text_Postal_Home,			//52
	PBK_Text_Postal_Work,			//53
	PBK_Number_Pager_Home,			//54
	PBK_Number_Pager_Work,			//55
	PBK_Number_VideoCall,			//56
	PBK_Number_VideoCall_Home,		//57
	PBK_Number_VideoCall_Work,		//58
	PBK_Text_MiddleName,			//59
	PBK_Text_Suffix,				//60
	PBK_Text_Title,					//61
	PBK_Text_Email_Mobile,			//62
	PBK_Text_Email_Unknown,			//63

//new add
	PBK_Number_Assistant ,			//64
	PBK_Number_Business ,			//65
	PBK_Number_Callback,			//66
	PBK_Number_Car,					//67
	PBK_Number_ISDN,				//68
	PBK_Number_Primary,				//69
	PBK_Number_Radio,				//70
	PBK_Number_Telix,				//71
	PBK_Number_TTYTDD,				//72

	PBK_Text_Department,			//73
	PBK_Text_Office,				//74
	PBK_Text_Profession,			//75
	PBK_Text_Manager_Name,			//76
	PBK_Text_Assistant_Name,		//77
	PBK_Text_Spouse_Name,			//78
	PBK_Date_Anniversary,			//79
	PBK_Text_Directory_Server,		//80
	PBK_Text_Email_alias,			//81
	PBK_Text_Internet_Address,		//82
	PBK_Text_Children,				//83

	PBK_Text_StreetAddress_Work,	//84
	PBK_Text_City_Work,				//85
	PBK_Text_State_Work,			//86
	PBK_Text_Zip_Work,				//87
	PBK_Text_Country_Work,			//88

	PBK_Text_StreetAddress_Home,	//89
	PBK_Text_City_Home,				//90
	PBK_Text_State_Home,			//91
	PBK_Text_Zip_Home,				//92
	PBK_Text_Country_Home,			//93
	PBK_Text_IMID					//94

} GSM_EntryType;

/**
 * Structure used for saving date and time 
 */
typedef struct {
	// The difference between local time and GMT in hours
	int			Timezone;

	unsigned int		Second;
	unsigned int 		Minute;
	unsigned int		Hour;

	unsigned int 		Day;

	// January = 1, February = 2, etc.
	unsigned int 		Month;

	// Complete year number. Not 03, but 2003
	unsigned int		Year;
} GSM_DateTime;


#define GSM_PHONEBOOK_TEXT_LENGTH       200
#define GSM_PHONEBOOK_ENTRIES	   	26
/**
 * One value of phonebook memory entry.
 */
typedef struct {
	// Type of entry.
	GSM_EntryType	   	EntryType;

	// Text of entry (if applicable, see @ref GSM_EntryType).
	unsigned char	   	Text[(GSM_PHONEBOOK_TEXT_LENGTH+1)*2];

	// Text of entry (if applicable, see @ref GSM_EntryType).
	GSM_DateTime	    	Date;

	// Number of entry (if applicable, see @ref GSM_EntryType).
	int		     	Number;

	// Voice dialling tag.
	int		     	VoiceTag;
	int			SMSList[20];
} GSM_SubMemoryEntry;


/**
 * Structure for saving phonebook entries
 */
typedef struct {
	// Used memory for phonebook entry
	GSM_MemoryType	  	MemoryType;

	// Used location for phonebook entry
//	int		     	Location;
	char			szIndex[100];

	// Number of SubEntries in Entries table.
	int		     	EntriesNum;

	// Values of SubEntries.
	GSM_SubMemoryEntry      Entries[GSM_PHONEBOOK_ENTRIES];
} GSM_MemoryEntry;

typedef enum
{
	COMMAND_ADD,
	COMMAND_DELETE,
	COMMAND_REPLACE
}COMMAND;

typedef struct {
	GSM_MemoryEntry*	  	MemoryEntry;
	COMMAND		     	command;
	int					Result;
} GSM_MemoryUpdateInfo;
typedef struct {
	char szvcardfilename[MAX_PATH];
	int nLocation;
	COMMAND		     	command;
	int					Result;
} GSM_SYNCMLUpdateInfo;

typedef struct {
	char szvcfFileName[MAX_PATH];
	int					Location;
} GSM_Memoryinfo;

/* ---------------------------- calendar ----------------------------------- */

#define GSM_CALENDAR_ENTRIES	    	16
#define MAX_CALENDAR_TEXT_LENGTH	300 /* In 6310 max. 256 chars */

/**
 * Enum defines types of calendar notes
 */
typedef enum {
	/**
	 * Reminder or Date
	 */
	GSM_CAL_REMINDER=1,
	/**
	 * Call
	 */
	GSM_CAL_CALL,
	/**
	 * Meeting
	 */
	GSM_CAL_MEETING,
	/**
	 * Birthday or Anniversary or Special Occasion
	 */
	GSM_CAL_BIRTHDAY,
	/**
	 * Memo or Miscellaneous
	 */
	GSM_CAL_MEMO,
	/**
	 * Travel
	 */
	GSM_CAL_TRAVEL,
	/**
	 * Vacation
	 */
	GSM_CAL_VACATION,
	/**
	 * Training - Athletism
	 */
	GSM_CAL_T_ATHL,
	/**
	 * Training - Ball Games
	 */
	GSM_CAL_T_BALL,
	/**
	 * Training - Cycling
	 */
	GSM_CAL_T_CYCL,
	/**
	 * Training - Budo
	 */
	GSM_CAL_T_BUDO,
	/**
	 * Training - Dance
	 */
	GSM_CAL_T_DANC,
	/**
	 * Training - Extreme Sports
	 */
	GSM_CAL_T_EXTR,
	/**
	 * Training - Football
	 */
	GSM_CAL_T_FOOT,
	/**
	 * Training - Golf
	 */
	GSM_CAL_T_GOLF,
	/**
	 * Training - Gym
	 */
	GSM_CAL_T_GYM,
	/**
	 * Training - Horse Race
	 */
	GSM_CAL_T_HORS,
	/**
	 * Training - Hockey
	 */
	GSM_CAL_T_HOCK,
	/**
	 * Training - Races
	 */
	GSM_CAL_T_RACE,
	/**
	 * Training - Rugby
	 */
	GSM_CAL_T_RUGB,
	/**
	 * Training - Sailing
	 */
	GSM_CAL_T_SAIL,
	/**
	 * Training - Street Games
	 */
	GSM_CAL_T_STRE,
	/**
	 * Training - Swimming
	 */
	GSM_CAL_T_SWIM,
	/**
	 * Training - Tennis
	 */
	GSM_CAL_T_TENN,
	/**
	 * Training - Travels
	 */
	GSM_CAL_T_TRAV,
	/**
	 * Training - Winter Games
	 */
	GSM_CAL_T_WINT,
	/**
	 * Alarm
	 */
	GSM_CAL_ALARM,
	/**
	 * Alarm repeating each day.
	 */
	GSM_CAL_DAILY_ALARM,
	// LOW
	GSM_CAL_LOW,
	// Normal
	GSM_CAL_NORMAL,
	// High
	GSM_CAL_HIGH,
	// Important
	GSM_CAL_IMPORTANT,
	// business
	GSM_CAL_BUSINESS
} GSM_CalendarNoteType;

typedef enum
{
	CAL_REPEAT_NONE			= 0,
	CAL_REPEAT_DAILY		= 1,
	CAL_REPEAT_WEEKLY		= 2,
	CAL_REPEAT_MONTHLY		= 3,
	CAL_REPEAT_MONTH_WEEKLY = 4,
	CAL_REPEAT_YEARLY		= 5,
	CAL_REPEAT_YEAR_WEEKLY		= 6,
};
/**
 * One value of calendar event.
 */
typedef enum {
	/**
	 * Date and time of event start.
	 */
	CAL_START_DATETIME = 1,
	/**
	 * Date and time of event end.
	 */
	CAL_END_DATETIME,
	/**
	 * Alarm date and time.
	 */
	CAL_ALARM_DATETIME,
	/**
	 * Date and time of silent alarm.
	 */
	CAL_SILENT_ALARM_DATETIME,
	/**
	 * Recurrance.
	 */
	CAL_RECURRANCE,
	/**
	 * 	RecurrenceFrequency
	 */
	CAL_RECURRANCEFREQUENCY,
	/**
	 * Text.
	 */
	CAL_TEXT,
	/**
	 * Location.
	 */
	CAL_LOCATION,
	/**
	 * Phone number.
	 */
	CAL_PHONE,
	/**
	 * Whether this entry is private.
	 */
	 CAL_PRIVATE,
	/**
	 * Related contact id.
	 */
	CAL_CONTACTID,
	/**
	 * Repeat each x'th day of week.
	 */
	CAL_REPEAT_DAYOFWEEK,
	/**
	 * Repeat each x'th day of month.
	 */
	CAL_REPEAT_DAY,
	/**
	 * Repeat x'th week of month.
	 */
	CAL_REPEAT_WEEKOFMONTH,
	/**
	 * Repeat x'th month.
	 */
	CAL_REPEAT_MONTH,
	/**
	 * Repeating frequency.
	 */
	CAL_REPEAT_FREQUENCY,
	/**
	 * Repeating start.
	 */
	CAL_REPEAT_STARTDATE,
	/**
	 * Repeating end.
	 */
	CAL_REPEAT_STOPDATE,
	CAL_DESCRIPTION
} GSM_CalendarType;


/**
 * One value of calendar event.
 */
typedef struct {
	/**
	 * Type of value.
	 */
	 GSM_CalendarType	EntryType;
	/**
	 * Text of value, if applicable.
	 */
	 unsigned char	   	Text[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
	/**
	 * Date and time of value, if applicable.
	 */
	 GSM_DateTime	    	Date;
	/**
	 * Number of value, if applicable.
	 */
	 unsigned int	    	Number;
} GSM_SubCalendarEntry;

/**
 * Calendar note values.
 */
typedef struct {
	/**
	 * Type of calendar note.
	 */
	GSM_CalendarNoteType    Type;
	/**
	 * Location in memory.
	 */
//	int		     	Location;
	char szIndex[100];
	/**
	 * Number of entries.
	 */
	int		     	EntriesNum;
	/**
	 * Values of entries.
	 */
	GSM_SubCalendarEntry    Entries[GSM_CALENDAR_ENTRIES];
} GSM_CalendarEntry;


/**
 * Structure for saving PortName
 */
typedef struct 
{
	int		     	Number;
	unsigned char*  PortName[20];
} PortNameList;


/**
 * Number of possible SMS folders.
 */
#define GSM_MAX_SMS_FOLDERS	     	24
/**
 * Maximal length of SMS folder name.
 */
#define GSM_MAX_SMS_FOLDER_NAME_LEN     20

/**
 * Information about SMS folder.
 */
typedef struct {
	/**
	 * Whether it is really inbox.
	 */
	int		    	InboxFolder;
	/**
	 * Where exactly it's saved
	 */
	GSM_MemoryType		Memory;
        /**
  	 * Name for SMS folder.
	 */
        char	    Name[(GSM_MAX_SMS_FOLDER_NAME_LEN+1)*2];
} GSM_OneSMSFolder;

/**
 * List of SMS folders.
 */
typedef struct {
        /**
	 * Array of structures holding information about each folder.
	 */
        GSM_OneSMSFolder	Folder[GSM_MAX_SMS_FOLDERS];
        /**
 	 * Number of SMS folders.
	 */
        unsigned char	   	Number;
} GSM_SMSFolders;

/**
 * Structure for saving speed dials
 */
typedef struct {
	/**
	 * Number of speed dial: 2,3..,8,9
	 */
	int	     		Location;
	/**
	 * ID of phone number used in phonebook entry
	 */
	int	     		MemoryNumberID;
	/**
	 * Memory, where is saved used phonebook entry
	 */
	GSM_MemoryType  	MemoryType;
	/**
	 * Location in memory, where is saved used phonebook entry
	 */
	int	     		MemoryLocation;
} GSM_SpeedDial;

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

//#ifdef DEVELOP
	GSM_File_MMS,
//#endif	
	GSM_File_Other,

	GSM_File_Image_TIF,
    GSM_File_Video_ASF,
    GSM_File_Video_AVI,
    GSM_File_Video_MOV,
    GSM_File_Video_MPG,
    GSM_File_Video_MPEG4,
    GSM_File_Sound_MMF,
    GSM_File_Sound_MP3,
    GSM_File_Sound_MONO,
    GSM_File_Sound_WAV,
    GSM_File_Sound_AAC,
} GSM_FileType;

typedef struct {
	int		Used;		/* how many bytes used 	*/
	unsigned char 	Name[800];	/* Name			*/
	bool 		Folder;		/* true, when folder 	*/
	int		Level;
	GSM_FileType	Type;
	unsigned char	ID_FullName[800];
	unsigned char	*Buffer;

	GSM_DateTime	Modified;
	bool		ModifiedEmpty;

	/* File attributes */
	bool		Protected;
	bool		ReadOnly;
	bool		Hidden;
	bool		System;
	int       FileSize;
	unsigned char		wFileFullPathName[800];
} GSM_File;
typedef struct {
	int		Free;
	int		Used;
} GSM_FileSystemStatus;

typedef enum {
	OBEX_Init = 0,
	OBEX_None = 1,
	OBEX_BrowsingFolders,
	OBEX_SYNCML
} OBEX_Service;


//int WINAPI DeleteData(char* MemoryType, int Location);*/
typedef enum {
	Type_Start	= 1,	      
	Type_Continune,
	Type_End,
}Control_Type;
/**
 * Types of to do values. In parenthesis is member of @ref GSM_SubToDoEntry,
 * where value is stored.
 */
typedef enum {
	/**
	 * Due date. (Date)
	 */
	TODO_START_DATETIME = 1,
	TODO_END_DATETIME ,
	/**
	 * Whether is completed. (Number)
	 */
	TODO_COMPLETED,
	/**
	 * When should alarm be fired (Date).
	 */
	TODO_ALARM_DATETIME,
	/**
	 * When should silent alarm be fired (Date).
	 */
	TODO_SILENT_ALARM_DATETIME,
	/**
	 * Text of to do (Text).
	 */
	TODO_TEXT,
	/**
	 * Whether entry is private (Number).
	 */
	TODO_PRIVATE,
	/**
	 * Category of entry (Number).
	 */
	TODO_CATEGORY,
	/**
	 * Related contact ID (Number).
	 */
	TODO_CONTACTID,
	/**
	 * Number to call (Text).
	 */
	TODO_PHONE,
	TODO_RECURRANCE,
	TODO_RECURRANCEFREQUENCY,
	TODO_REPEAT_STARTDATE,
	TODO_REPEAT_STOPDATE,
	TODO_PRIORITY,
	TODO_DESCRIPTION,
	TODO_REPEAT_DAYOFWEEK,
	TODO_COMPLETEDDATE
} GSM_ToDoType;

/**
 * Priority of to do.
 */
typedef enum {
	GSM_Priority_High = 1,
	GSM_Priority_Medium,
	GSM_Priority_Low
} GSM_ToDo_Priority;

typedef struct
{
	char szLast[MAX_PATH];
	char szNext[MAX_PATH];
	char szSource[MAX_PATH];
	char szTarget[MAX_PATH];
	char szServerNextAnchor[MAX_PATH];
	bool bSlowSync;
	int nCount;
}SyncML_SyncInfo;
/*typedef struct
{
	bool bSlowSync;
	int nCount;
}SyncInfo;
*/
enum
{
	SYNCDATA_UPDATE_NONE = 0,
	SYNCDATA_UPDATE_ADD,
	SYNCDATA_UPDATE_DELETE,
	SYNCDATA_UPDATE_MODIFY,
	SYNCDATA_SYNC_OK

};
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
#endif// All Type 


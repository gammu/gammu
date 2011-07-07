#ifndef __SYNCDLL_INTERFACE_DEF_H_
#define __SYNCDLL_INTERFACE_DEF_H_

#include <windows.h>

#define LLD_INTERFACE_VERSION		L"V0.6"


//[Connect Status]
//Description: used in connect status callback function
//********************************************************************
#define	LLD_STATUS_CONNECTED	1
#define LLD_STATUS_DISCONNECT   2002


//[Comport Config Field]
//Description: used to describe the field mask in config
//********************************************************************
#define LLD_CONFIG_ALL		    0xFFFFFFFF
#define LLD_CONFIG_BAUDRATE	    0x00000001
#define LLD_CONFIG_PARITY		0x00000002
#define LLD_CONFIG_STOPBITS		0x00000004
#define LLD_CONFIG_BYTESIZE		0x00000008
#define LLD_CONFIG_OVERLAPPEDIO	0x00000010
#define LLD_CONFIG_TIMEOUT		0x00000020
#define LLD_CONFIG_TRACEFLAG	0x00000100
#define LLD_CONFIG_TRACE_FILENAME	0x00000200
#define LLD_CONFIG_TRACE_ON		0x00000400
#define LLD_CONFIG_ATC_MAX_LEN	0x00010000
#define LLD_CONFIG_RECOVERFIRSTCH	0x00020000
#define LLD_CONFIG_RETRY			0x00040000


//[Phone Model String]
//Description: Used to describe the phone model
//********************************************************************
#define LLD_MODEL_M303		"M303"
#define LLD_MODEL_J101		"J101"
#define LLD_MODEL_J102		"J102"
#define LLD_MODEL_J103		"J103"
#define LLD_MODEL_J105		"J105"
#define LLD_MODEL_V9000		"V9000"



////// Return code ///////////////////////////////////
#define	LLD_RETURN_ALREADY_CONNECTTED	2
#define LLD_RETURN_OK			1
#define LLD_RETURN_ERROR		0
#define LLD_RETURN_CONFIG_ERROR		-1
#define LLD_RETURN_TIMEOUT		-2
#define LLD_RETURN_NOT_CONNECTED    -3
#define LLD_RETURN_CALL_ERROR		-100
#define LLD_RETURN_NO_DATA		-101

/////// Trace Log /////////////////////////////////////
#define LLD_TRACE_FLAG_ALL			0xFFFFFFFF
#define LLD_TRACE_FLAG_NONE			0x00000000
#define LLD_TRACE_FLAG_DEBUG		0x00000001
#define	LLD_TRACE_FLAG_SERIAL		0x00000002
#define LLD_TRACE_FLAG_SERIAL_CONTENT	0x00000004
#define LLD_TRACE_FLAG_UTIL			0x00000010
#define LLD_TRACE_FLAG_TRANSPORT	0x00000100
#define LLD_TRACE_FLAG_TRANSPORT_CONTENT	0x00000200
#define LLD_TRACE_FLAG_ATCGEN		0x00001000
#define LLD_TRACE_FLAG_ATCPARSER    0x00004000
#define LLD_TRACE_FLAG_ATTEST		0x00010000
#define LLD_TRACE_FLAG_ATPB			0x00020000
#define LLD_TRACE_FLAG_INTERFACE	0x10000000
#define LLD_TRACE_FLAG_PRINTF		0x20000000
#define LLD_TRACE_FLAG_AT   		0x40000000

/////// Length definition /////////////////////////////
#define LLD_IMEI_LENGTH		20
#define LLD_VENDOR_LENGTH	24
#define LLD_MODEL_LENGTH	24
#define LLD_NAME_LENGTH		20
#define LLD_TELNUM_LENGTH   16
#define LLD_TITLE_LENGTH	20
#define LLD_EMAIL_LENGTH		40
#define LLD_COMPANY_LENGTH	20
#define LLD_NOTE_LENGTH	402
#define LLD_TITLE_LENGTH	20   // 職稱
#define LLD_CALENDAR_TITLE_LENGTH		30
#define LLD_CALENDAR_CONTENT_LENGTH		54
#define LLD_MEMO_TITLE_LENGTH	30
#define LLD_MEMO_TEXT_LENGTH	140
#define LLD_ADDRESS_LENGTH	60
#define LLD_MAXFILE_NUM		200
#define LLD_FILE_NAME_LENGTH		256
#define LLD_GROUP_NAME_LENGTH	16
#define LLD_MAX_INDEXES			4096
#define LLD_MAX_SMS_LENGTH		160
#define LLD_REMARK_LENGTH		20
#define LLD_PC_MAX_FILENAME_LEN		_MAX_PATH


//[Folder List]
//Description: Used to indicate folders that used in SMS, MMS, RingTone and ImageGallery
//********************************************************************
#define LLD_FOLDER_INBOX			0x00000001
#define LLD_FOLDER_OUTBOX		0x00000002
#define LLD_FOLDER_FACTORY_RT	0x00000010 	//Factory Ring Tone
#define LLD_FOLDER_COMPOSER		0x00000020	//Compose
#define LLD_FOLDER_DL_RT			0x00000040 	//Download Ring Tone
#define LLD_FOLDER_MY_ALBUM		0x00000100
#define LLD_FOLDER_MY_VIDEO		0x00000200
#define LLD_FOLDER_MAIN_WALLPAPER	0x00000400
#define LLD_FOLDER_SUB_WALLPAPER	0x00000800
#define LLD_FOLDER_FRAME			0x00001000
#define LLD_FOLDER_SMS_F1			0x00010000
#define LLD_FOLDER_SMS_F2			0x00020000	
#define LLD_FOLDER_SMS_F3			0x00040000
#define LLD_FOLDER_SMS_F4			0x00080000
#define LLD_FOLDER_SMS_F5			0x00100000


//[File Extension List]
//Description: Used to indicate file extension
//********************************************************************
#define LLD_FILE_EXT_JAD			41
#define LLD_FILE_EXT_JAR			42
#define LLD_FILE_EXT_MID			71
#define LLD_FILE_EXT_MMF			72
#define LLD_FILE_EXT_MP3			73
#define LLD_FILE_EXT_JPG			81
#define LLD_FILE_EXT_GIF			82
#define LLD_FILE_EXT_PNG			83
#define LLD_FILE_EXT_BMP			84
#define LLD_FILE_EXT_WBMP			85
#define LLD_FILE_EXT_AVI96			86
#define LLD_FILE_EXT_AVI160			87


//[Folder List Name]
//Description: Used to indicate folders name that used in SMS, MMS, RingTone and ImageGallery
//********************************************************************
#define LLD_FOLDER_INBOX_STR			"In Box"
#define LLD_FOLDER_OUTBOX_STR		"Out Box"
#define LLD_FOLDER_FACTORY_RT_STR	"Factory Ring Tone" 	//Factory Ring Tone
#define LLD_FOLDER_COMPOSER_STR		"Composer"	//Compose
#define LLD_FOLDER_DL_RT_STR			"Download Ring Tone" 	//Download Ring Tone
#define LLD_FOLDER_MY_ALBUM_STR		"My Album"
#define LLD_FOLDER_MY_VIDEO_STR		"My Video"
#define LLD_FOLDER_MAIN_WALLPAPER_STR	"Main Wallpaper"
#define LLD_FOLDER_SUB_WALLPAPER_STR	"Sub Wallpaper"
#define LLD_FOLDER_FRAME_STR			"Frame"
#define LLD_FOLDER_SMS_F1_STR			"Folder 1"
#define LLD_FOLDER_SMS_F2_STR			"Folder 2"	
#define LLD_FOLDER_SMS_F3_STR			"Folder 3"
#define LLD_FOLDER_SMS_F4_STR			"Folder 4"
#define LLD_FOLDER_SMS_F5_STR			"Folder 5" 


//[COM port number]
//Description: Used to indicate comport number
//********************************************************************
enum
{
	LLD_COM_1 = 1,
	LLD_COM_2,
	LLD_COM_3,
	LLD_COM_4,
	LLD_COM_5,
	LLD_COM_6,
	LLD_COM_7,
	LLD_COM_8,
	LLD_COM_9,
	LLD_COM_10,
	LLD_COM_11,
	LLD_COM_12,
	LLD_COM_13,
	LLD_COM_14,
	LLD_COM_15,
};


//[Memory Type]
//Description: Used to indicate the targeted memory type
//********************************************************************
enum 
{
	LLD_MEMORY_TYPE_SIM = 1,
	LLD_MEMORY_TYPE_HANDSET,
	LLD_MEMORY_TYPE_PC,
};


//[Calendar Record Type]
//Description: Used to indicate the calendar record type
//********************************************************************
enum 
{
	LLD_CALENDAR_REC_TYPE_NORMAL = 1,
	LLD_CALENDAR_REC_TYPE_LOW,
	LLD_CALENDAR_REC_TYPE_HIGH,
	LLD_CALENDAR_REC_TYPE_IMPORTANT,
	LLD_CALENDAR_REC_TYPE_BIRTHDAY,
	LLD_CALENDAR_REC_TYPE_MEETING,
	LLD_CALENDAR_REC_TYPE_BUSINESS,
};


//[Calendar Repeat Type]
//Description: Used to indicate the repeat type for a calendar record
//********************************************************************
enum 
{
	LLD_CALENDAR_REPEAT_TYPE_ONCE = 1,
	LLD_CALENDAR_REPEAT_TYPE_EVERYDAY,
	LLD_CALENDAR_REPEAT_TYPE_EVERYWEEK,
	LLD_CALENDAR_REPEAT_TYPE_EVERYMONTH,
	LLD_CALENDAR_REPEAT_TYPE_EVERYYEAR,
};


//[Phone Number Type]
//Description: Used to indicate the phone number type
//********************************************************************
enum 
{
	LLD_PHONENUMBER_TYPE_INVALID = 0,		// Indicate the phone structure is not used
	LLD_PHONENUMBER_TYPE_VALID,		// Indicate structure is used
};


//[Structure lld_Date]
//Description: Used to store the date for birthday etc.
//********************************************************************
typedef struct
{
	unsigned short		year;
	unsigned char		month;
	unsigned char		day;
} lld_Date;


//[Structure lld_DateTime]
//Description: Used to store the date and time for birthday etc.
//********************************************************************
typedef struct
{
	unsigned short		year;
	unsigned char		month;
	unsigned char		day;
	unsigned char		hour;
	unsigned char		minute;
} lld_DateTime;



//[Structure lld_PhoneNumber]
//Description: Used to store the phone number and Fax number
//********************************************************************
typedef struct
{
	unsigned short		PhoneType; // Indicate the phone number type, refer to enumerate section
	unsigned char		len;		// Indicate the digital length
	char		digitals [ LLD_TELNUM_LENGTH + 1];  // Stored in ASCII
} lld_PhoneNumber;


//[Structure lld_PhoneGroup]
//Description: Used to store the phone group information
//********************************************************************
typedef struct
{
	int GroupID;
	WCHAR GroupName [ LLD_GROUP_NAME_LENGTH ];
} lld_PhoneGroup;


//[Structure lld_ContactData]
//Description: Used to store the contact information
//********************************************************************

typedef struct 
{
	int Index;			// File index on mobile phone
	WCHAR Name [ LLD_NAME_LENGTH ];			// Name
	lld_PhoneNumber 		PhoneNumber1;		// Phone number 1
	lld_PhoneNumber 		PhoneNumber2;		// Phone number 2
	lld_PhoneNumber 		PhoneNumber3;		// Phone number 3
	lld_PhoneNumber 		PhoneNumber4;		// Phone number 4
	lld_PhoneNumber 		FaxNumber;		// Fax number 
	WCHAR Company [ LLD_COMPANY_LENGTH ];		// Company
	WCHAR Title	[ LLD_TITLE_LENGTH ];			// Title
	WCHAR Address [ LLD_ADDRESS_LENGTH];	// Address
	WCHAR Email [ LLD_EMAIL_LENGTH];			// Email
	WCHAR	PictureName [ LLD_FILE_NAME_LENGTH ];		// Filename for picture, if all 0 means no picture attached
	WCHAR RingName [ LLD_FILE_NAME_LENGTH ];	// Filename for ring, if all 0 means no picture attached
	lld_Date	birthday;	// Birthday
	unsigned char Sex; // 1 for male, 0 for female
	lld_PhoneGroup Group; 	// Group
	WCHAR Remark [LLD_REMARK_LENGTH];
    unsigned char VoiceTag;
	unsigned char LedId;
} lld_ContactData;



//[lld_FileIndexList]
//Description: Used to store file indexes when explore folder in mobile phone
//********************************************************************
typedef struct
{
	unsigned int IndexFlag [ LLD_MAX_INDEXES ]; // store index list
} lld_FileIndexList;


//[lld_FileExtList]
//Description: Used to store file extension type of each index when explore folder in mobile phone, always used in SMS/MMS, Java and multimedia folders
//********************************************************************
typedef struct
{
	BYTE ExtFlag [ LLD_MAX_INDEXES ]; // Indicate the file extension type of each relevant file index
} lld_FileExtList;


//[lld_FileList]
//Description: Used to store file names when explore folder in mobile phone
//********************************************************************
typedef struct
{
	int FileNum;  // indicates how many files name stored in the structure
	WCHAR** FileNameA;  // Store wchar pointer array to each file name string
		// ! Be careful, each wchar pointer should point to a allocated buffer with len of LLD_FILE_NAME_LENGTH
} lld_FileList;


//[lld_CalendarData]
//Description: Used to store calendar record data
//********************************************************************
typedef struct
{
	int Index;   // File index in mobile phone
	lld_DateTime	StartTime;
	lld_DateTime	EndTime;
	lld_DateTime	AlarmTime;
	int RecordType;	// Indicate the type of calendar record, refer to enumerate sction
	int RepeatType;	//Indicate the repeat type, refer to enumerate section
	BOOL  bTrigger; // Indicate whether or not to play ringtone
	WCHAR RingName [ LLD_FILE_NAME_LENGTH ];	// Filename for ring, if all 0 means no picture attached
	//lld_PhoneNumber 		PhoneNumber;		// Phone number to date with
	WCHAR CalendarTitle [LLD_CALENDAR_TITLE_LENGTH ];
	WCHAR Content [LLD_CALENDAR_CONTENT_LENGTH ];
} lld_CalendarData;


//[lld_MemoData]
//Description: Used to store memo data
//********************************************************************
typedef struct
{
  WCHAR Title[LLD_MEMO_TITLE_LENGTH ];
  WCHAR Text[LLD_MEMO_TEXT_LENGTH ];
  int Index;
} lld_MemoData;


//[lld_SMSInfo]
//Description: Used to store SMS information, including usage and existing folders
//********************************************************************
typedef struct
{
	int  SIMSMSFreeMem;
	int  SIMSMSUsedMem;			// -1 代表 N/A : Driver 無法取得
	int  MESMSFreeMem;
	int  MESMSUsedMem ;			// -1 代表 N/A : Driver 無法取得
	int  SIMSMSFolderTypeMask;	 // indicates which folders in SIM SMS
	int  MESMSFolderTypeMask;	 // indicates which folders in ME SMS
} lld_SMSInfo;


//[lld_MMSInfo]
//Description: Used to store MMS information, including usage and existing folders
//********************************************************************
typedef struct
{
	int MEMMSFreeMem;
	int MEMMSUsedMem;
	int MEMMSFolderTypeMask;
}lld_MMSInfo;


//[lld_ImageGalleryInfo]
//Description: Used to store ImageGallery information, including usage and existing folders
//********************************************************************
typedef struct
{
	int MEImageGalleryFreeMem;
	int MEImageGalleryUsedMem;
	int MEImageGalleryFolderTypeMask;
}lld_ImageGalleryInfo;


//[lld_RingToneInfo]
//Description: Used to store Ring Tone information, including usage and existing folders
//********************************************************************
typedef struct
{
	int MERingToneFreeMem;
	int MERingToneUsedMem;
	int MERingToneFolderTypeMask;
}lld_RingToneInfo;


//[lld_JavaInfo]
//Description: Used to store Java information
//********************************************************************
typedef struct
{
	int MEJavaFreeMem;
	int MEJavaUsedMem;
}lld_JavaInfo;

//[lld_ComCfg]
//Description: Used to store configuration for serial communication
//	(Currently, flow control settings are reserved for future)
//	(The following setting refer to the DCB structure in Win32 definition)
//********************************************************************
typedef struct
{
	unsigned int	field_flag;   // indicates which fields following are valid
	unsigned int 	BaudRate; // default is CBR_115200
	BYTE 	Parity;  // default is NOPARITY
	BYTE 	StopBits;  // default is ONESTOPBIT
	BYTE 	ByteSize;	  // default is 8
	BOOL	OverlappedIO;  // default is TRUE
	unsigned int	timeout;  // in ms, default is 2000
} lld_ComCfg;


//[lld_TraceCfg]
//********************************************************************
typedef struct
{
	unsigned int	field_flag;   // indicates which fields following are valid
	unsigned int 	trace_flag;   // indicates which modules in driver will be logged, default is LLD_TRACE_FLAG_ALL
	WCHAR*		filename;   // indicates the log file, default is "STDOUT"
	BOOL		bTrace;	// True for enable, false for disable, default is TRUE
} lld_TraceCfg;


//[lld_AtcCfg]
//Description: Used to store configuration for AT commands
//********************************************************************
typedef struct
{
	unsigned int	field_flag;   // indicates which fields following are valid
	unsigned int 	atc_max_length;	// indicate the allowed maximum length for an AT command, default is 255
	BOOL	bReoveryFirstCH;	// enable the automatically error recovery for first character that lost by power saving mode, default is TRUE
	unsigned int	retry; // indicates the retry times for a AT transaction, default is 3
} lld_AtcCfg;


//[lld_Config]
//Description: Used to store configuration for low level driver
//********************************************************************
typedef struct
{
	lld_ComCfg		*comcfg; // Store configuration for serial communications
	lld_TraceCfg	*tracecfg;	// Store configuration for trace log
	lld_AtcCfg		*atccfg; // Store configuration for AT command
} lld_Config;



#endif